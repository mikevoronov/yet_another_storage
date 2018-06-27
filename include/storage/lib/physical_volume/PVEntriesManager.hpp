#pragma once
#include "../utils/Time.hpp"
#include "PVDeviceDataReaderWriter.hpp"
#include "FreelistHelper.hpp"
#include "EntriesTypeConverter.hpp"
#include "PVEntriesAllocator.hpp"
#include <type_traits>
#include <variant>
#include <cstring>
#include <optional>

namespace yas {
namespace pv {

using namespace yas::pv_layout_headers;
using namespace yas::pv::entries_types;
using namespace yas::storage;

template <typename OffsetType, typename Device>
class PVEntriesManager {
  using FreelistHeaderType = pv_layout_headers::FreelistHeader<OffsetType>;
  using PVPathType = typename Device::path_type;
 
 public:
  PVEntriesManager(const PVPathType &file_path, utils::Version version, int32_t priority = 0,
      int32_t cluster_size = kDefaultClusterSize)
      : data_reader_writer_(file_path, cluster_size),
        version_(version),
        priority_(priority),
        cluster_size_(cluster_size),
        entries_allocator_(cluster_size) {
  }

  ~PVEntriesManager() = default;

  OffsetType LoadStartEntries() {
    OffsetType current_cursor = 0;
    PVHeader pv_header = data_reader_writer_.template Read<PVHeader>(current_cursor);
    PVHeader default_header;
    if (0 != std::memcmp(default_header.signature_, pv_header.signature_, sizeof default_header.signature_)) {
      throw exception::YASException("PV header parsing: corrupted signature", StorageError::kInvalidPVSignatureError);
    }
    else if (pv_header.version_ > version_) {
      throw exception::YASException("PV header parsing: unsupported version", StorageError::kInvalidPVSignatureError);
    }
    else if (!offset_traits<OffsetType>::IsExistValue(pv_header.inverted_index_offset_)) {
      throw exception::YASException("PV header parsing: invalid inverted index offset",
          StorageError::kInvalidPVSignatureError);
    }

    current_cursor += sizeof pv_header;
    FreelistHeaderType freelist_header = data_reader_writer_.template Read<FreelistHeaderType>(current_cursor);
    freelist_helper_.SetBins(freelist_header);
    cluster_size_ = pv_header.cluster_size_;
    priority_ = pv_header.priority_;
    entries_allocator_.device_end(pv_header.pv_size_);

    return pv_header.inverted_index_offset_;
  }

  void SaveStartEntries(OffsetType index_offset) {
    PVHeader pv_header;
    pv_header.version_ = version_;
    pv_header.priority_ = priority_;
    pv_header.pv_size_ = entries_allocator_.device_end();
    pv_header.cluster_size_ = cluster_size_;
    pv_header.inverted_index_offset_ = index_offset;
    data_reader_writer_.template Write<PVHeader>(0, pv_header);
    data_reader_writer_.template Write<FreelistHeaderType>(sizeof pv_header, freelist_helper_.GetBins());

    entries_allocator_.device_end(sizeof pv_header + sizeof(FreelistHeaderType));
  }

  OffsetType CreateNewEntryValue(const storage_value_type &value) {
    return VisitEntryTypes(value,
        [this](int8_t value) { return createNewEntryValue(Int8_EntryType(value)); },
        [this](uint8_t value) { return createNewEntryValue(UInt8_EntryType(value)); },
        [this](int16_t value) { return createNewEntryValue(Int16_EntryType(value)); },
        [this](uint16_t value) { return createNewEntryValue(UInt16_EntryType(value)); },
        [this](int32_t value) { return createNewEntryValue(Int32_EntryType(value)); },
        [this](uint32_t value) { return createNewEntryValue(UInt32_EntryType(value)); },
        [this](float value) {
            static_assert(std::numeric_limits<float>::is_iec559, "The YAS requires using of IEEE 754 floating point format for binary serialization of floats");
            return createNewEntryValue(Float_EntryType(value)); },
        [this](int64_t value) { return createNewEntryValue(Int64_EntryType(value)); },
        [this](uint64_t value) { return createNewEntryValue(UInt64_EntryType(value)); },
        [this](double value) { 
              static_assert(std::numeric_limits<double>::is_iec559, "The YAS requires using of IEEE 754 floating point format for binary serialization of doubles");
              return createNewEntryValue(Double_EntryType(value)); },
        [this](const std::string &value) { return createNewEntryValue(String_EntryType(value)); },
        [this](const ByteVector &value) { return createNewEntryValue(Blob_EntryType(value)); });
  }

  storage_value_type GetEntryContent(OffsetType offset) {
    const PVType pv_type = getRecordType(offset);
    const EntryType storage_type = EntriesTypeConverter::ConvertToEntryType(pv_type);
    return std::visit([this, offset](auto &&value) {
        auto &&storage_result = getEntryContent<typename std::decay_t<decltype(value)>::HeaderType>(offset);
        return EntriesTypeConverter::ConvertToUserType(std::move(storage_result));
    }, storage_type);
  }

  void DeleteEntry(OffsetType offset) {
    const PVType pv_type = getRecordType(offset);
    const EntryType storage_type = EntriesTypeConverter::ConvertToEntryType(pv_type);
    std::visit([this, offset](auto &&value) {
      return deleteEntry<typename std::decay_t<decltype(value)>::HeaderType>(offset);
    }, storage_type);
  }

  std::optional<utils::Time> GetEntryExpiredDate(OffsetType offset) {
    const PVType pv_type = getRecordType(offset);
    const EntryType storage_type = EntriesTypeConverter::ConvertToEntryType(pv_type);
    return std::visit([this, offset](auto &&value) {
      return getEntryExpiredDate<typename std::decay_t<decltype(value)>::HeaderType>(offset);
    }, storage_type);
  }

  void SetEntryExpiredDate(OffsetType offset, const utils::Time &expired_date) {
    const PVType pv_type = getRecordType(offset);
    const EntryType storage_type = EntriesTypeConverter::ConvertToEntryType(pv_type);
    std::visit([this, offset, &expired_date](auto &&value) {
      return setEntryExpiredDate<typename std::decay_t<decltype(value)>::HeaderType>(offset, expired_date);
    }, storage_type);
  }

  int32_t priority() const { return priority_; }

 private:
  PVDeviceDataReaderWriter<OffsetType, Device> data_reader_writer_;
  freelist_helper::FreelistHelper<OffsetType> freelist_helper_;
  PVEntriesAllocator<OffsetType> entries_allocator_;
  utils::Version version_;    // there could be some parsing issues depends on version
  int32_t cluster_size_;
  int32_t priority_;

  template<class EntryType>
  OffsetType createNewEntryValue(EntryType entry_value) {
    using HeaderType = typename EntryType::HeaderType;
    if constexpr(std::is_same_v<HeaderType, ComplexTypeHeader>) {
      return writeComplexType(entry_value.pv_type_, std::cbegin(entry_value.value_), std::cend(entry_value.value_));
    }
    else {
      const auto new_entry_offset = getFreeEntryOffset(sizeof(HeaderType));
      HeaderType header;
      header.value_type_ = entry_value.pv_type_;
      header.value_state_ = PVTypeState::kEmpty;
      header.value_ = *(reinterpret_cast<decltype(header.value_) *>(&entry_value.value_));
      data_reader_writer_.template Write<HeaderType>(new_entry_offset, header);
      return new_entry_offset;
    }
  }

  template<typename HeaderType>
  EntryType getEntryContent(OffsetType offset) {
    HeaderType header = data_reader_writer_.template Read<HeaderType>(offset);
    if constexpr(std::is_same_v<HeaderType, ComplexTypeHeader>) {
      auto &&data = data_reader_writer_.ReadComplexType(offset);
      return EntriesTypeConverter::ConvertToEntryType<ByteVector>(header.value_type_, std::move(data));
    }
    else {
      const auto aligned_value = header.value_;
      return EntriesTypeConverter::ConvertToEntryType(header.value_type_, aligned_value);
    }
  }

  template<typename HeaderType>
  std::optional<utils::Time> getEntryExpiredDate(OffsetType offset) {
    HeaderType header = data_reader_writer_.template Read<HeaderType>(offset);
    if (!(header.value_state_ & PVTypeState::kIsExpired)) {
      return {};
    }
    return utils::Time(header.expired_time_low_, header.expired_time_high_);
  }

  template<typename HeaderType>
  void deleteEntry(OffsetType offset) {
    HeaderType header = data_reader_writer_.template Read<HeaderType>(offset);
    if constexpr(!std::is_same_v<HeaderType, ComplexTypeHeader>) {
      header.value_type_ = (sizeof header == sizeof(Simple4TypeHeader) ? PVType::kEmpty4Simple : PVType::kEmpty8Simple);
      header.value_state_ = PVTypeState::kEmpty;
      header.next_free_entry_offset_ = freelist_helper_.GetFreeEntry(sizeof header);
      data_reader_writer_.template Write<HeaderType>(offset, header);
      freelist_helper_.PushFreeEntry(offset, sizeof header);
      return;
    }
    else {
      const auto overall_size = header.overall_size_;
      auto next_entry_offset = header.sequel_offset_;

      header.value_type_ = PVType::kEmptyComplex;
      header.value_state_ = PVTypeState::kComplexBegin;
      header.overall_size_ = header.chunk_size_;
      header.next_free_entry_offset_ = freelist_helper_.GetFreeEntry(header.chunk_size_);
      data_reader_writer_.template Write<ComplexTypeHeader>(offset, header);
      freelist_helper_.PushFreeEntry(offset, header.chunk_size_ + serialization_utils::offset_of(&ComplexTypeHeader::data_));

      auto deleted = header.chunk_size_;
      while (deleted < overall_size && offset_traits<OffsetType>::IsExistValue(next_entry_offset)) {
        const auto saved_next_entry_offset = next_entry_offset;
        header = data_reader_writer_.template Read<ComplexTypeHeader>(next_entry_offset);

        next_entry_offset = header.sequel_offset_;
        header.value_type_ = PVType::kEmptyComplex;
        header.value_state_ = PVTypeState::kComplexBegin;
        header.overall_size_ = header.chunk_size_;
        header.next_free_entry_offset_ = freelist_helper_.GetFreeEntry(header.chunk_size_);
        data_reader_writer_.template Write<ComplexTypeHeader>(saved_next_entry_offset, header);
        freelist_helper_.PushFreeEntry(saved_next_entry_offset, header.chunk_size_
            + serialization_utils::offset_of(&ComplexTypeHeader::data_));
        deleted += header.chunk_size_;
        return;
      }
    }
  }

  template<typename HeaderType>
  void setEntryExpiredDate(OffsetType offset, const utils::Time &expired_date) {
    HeaderType header = data_reader_writer_.template Read<HeaderType>(offset);
    header.value_type_ = header.value_type_;
    header.value_state_ = PVTypeState::kIsExpired;
    header.expired_time_high_ = expired_date.expired_time_high();
    header.expired_time_low_ = expired_date.expired_time_low();
    data_reader_writer_.template Write<HeaderType>(offset, header);
  }

  PVType getRecordType(OffsetType offset) {
    // In version 1.3 it is good to optimize read operations count by add std::aligned_union<headers...>
    // It gives us with a possibility to read this union in this method and decrease by one read operations.
    // But because of read/write cache in most devices there aren't any important performance issues at the moment.
    // Also it is important to add checks that (readed_offset + sizeof(union)) < device_end
    const auto pv_state = data_reader_writer_.template Read<PVState>(offset);
    return pv_state.value_type_;
  }

  template<typename Iterator>
  OffsetType writeComplexType(PVType value_type, const Iterator begin, const Iterator end) {
    const OffsetType data_size = std::distance(begin, end);
    auto free_offset = getFreeEntryOffset(data_size + sizeof(ComplexTypeHeader));
    auto first_free_offset = free_offset;

    bool is_first = true;
    OffsetType written = 0;
    OffsetType overall_written = 0;
    while (overall_written < data_size) {
      auto new_begin = begin;
      std::advance(new_begin, overall_written);
      auto next_free_offset = getFreeEntryOffset(data_size - overall_written + sizeof(ComplexTypeHeader));
      written = data_reader_writer_.WriteComplexType(free_offset, value_type, is_first, next_free_offset,
          new_begin, end);
      is_first = false;

      free_offset = next_free_offset;
      overall_written += written;
    }

    return first_free_offset;
  }

  OffsetType getFreeEntryOffset(OffsetType entry_size) {
    const auto offset = getFreeOffset(entry_size);

    OffsetType split_size = 0;
    switch (getRecordType(offset)) {
    case PVType::kEmpty4Simple:
      recoverAndPushNextEntry<Simple4TypeHeader>(offset);
      return offset;
    case PVType::kEmpty8Simple:
      recoverAndPushNextEntry<Simple8TypeHeader>(offset);
      return offset;
    case PVType::kEmptyComplex:
      const ComplexTypeHeader header = data_reader_writer_.template Read<ComplexTypeHeader>(offset);
      recoverAndPushNextEntry<ComplexTypeHeader>(offset);
      if (entry_size > header.overall_size_) {
        return offset;
      }
      split_size = header.overall_size_ - entry_size;
    }

    splitEntries(offset + entry_size, split_size);
    return offset;
  }

  OffsetType getFreeOffset(OffsetType entry_size) {
    auto offset = freelist_helper_.PopFreeEntryOffset(entry_size);
    if (!offset_traits<OffsetType>::IsExistValue(offset)) {
      const auto free_entry_offset = entries_allocator_.ExpandPV(data_reader_writer_,
          freelist_helper_.GetFreeEntry(cluster_size_));
      freelist_helper_.PushFreeEntry(free_entry_offset, cluster_size_);
      offset = freelist_helper_.PopFreeEntryOffset(entry_size);
      if (!offset_traits<OffsetType>::IsExistValue(offset)) {
        throw (exception::YASException("Error during physical volume size extended",
            StorageError::kDeviceExpandError));
      }
    }

    return offset;
  }

  void splitEntries(OffsetType split_offset, OffsetType split_size) {
    if (split_size > sizeof(ComplexTypeHeader)) {
      ComplexTypeHeader header;
      header.value_type_ = PVType::kEmptyComplex;
      header.next_free_entry_offset_ = freelist_helper_.GetFreeEntry(split_size);
      header.overall_size_ = split_size;
      header.chunk_size_ = split_size;
      data_reader_writer_.template Write<ComplexTypeHeader>(split_offset, header);
      freelist_helper_.PushFreeEntry(split_offset, split_size);
    }
    else if (split_size >= sizeof(Simple4TypeHeader) && split_size < sizeof(Simple8TypeHeader)) {
      Simple4TypeHeader header;
      header.next_free_entry_offset_ = freelist_helper_.GetFreeEntry(split_size);
      header.value_type_ = PVType::kEmpty4Simple;
      data_reader_writer_.template Write<Simple4TypeHeader>(split_offset, header);
      freelist_helper_.PushFreeEntry(split_offset, split_size);
    }
    else if (split_size >= sizeof(Simple8TypeHeader)) {
      Simple8TypeHeader header;
      header.next_free_entry_offset_ = freelist_helper_.GetFreeEntry(split_size);
      header.value_type_ = PVType::kEmpty8Simple;
      data_reader_writer_.template Write<Simple8TypeHeader>(split_offset, header);
      freelist_helper_.PushFreeEntry(split_offset, split_size);
    }
  }

  // recover next_free_offset from Header and push it to the freelist
  template<typename HeaderType>
  bool recoverAndPushNextEntry(OffsetType offset) {
    HeaderType header = data_reader_writer_.template Read<HeaderType>(offset);
    const auto next_free_entry = header.next_free_entry_offset_;
    if (!offset_traits<OffsetType>::IsExistValue(next_free_entry)) {
      return false;
    }

    auto next_free_entry_size = sizeof header;
    if constexpr(std::is_same_v<HeaderType, ComplexTypeHeader>){
      header = data_reader_writer_.template Read<ComplexTypeHeader>(next_free_entry);
      next_free_entry_size = header.overall_size_ + serialization_utils::offset_of(&ComplexTypeHeader::data_);
    }
    freelist_helper_.PushFreeEntry(next_free_entry, next_free_entry_size);
    return true;
  }
};

} // namespace pv
} // namespace yas
