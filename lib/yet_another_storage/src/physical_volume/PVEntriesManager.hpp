#pragma once
#include "../utils/Time.hpp"
#include "PVDeviceDataReaderWriter.hpp"
#include "../common/common.h"
#include "../common/offset_type_traits.hpp"
#include "FreelistHelper.hpp"
#include "EntriesTypeConverter.hpp"
#include <string_view>
#include <any>

using namespace yas::pv_layout_headers;
using namespace yas::freelist_helper;

namespace yas {
namespace pv {

template <typename OffsetType, typename Device>
class PVEntriesManager {
 public:
  using offset_type = OffsetType;
  using pv_entries_manager_type = PVEntriesManager<OffsetType, Device>;
  using data_reader_writer_type = PVDeviceDataReaderWriter<OffsetType, Device>;
  using freelist_header_type = FreelistHeader<OffsetType>;

  explicit PVEntriesManager(fs::path &file_path, utils::Version version, uint32_t cluster_size = kDefaultClusterSize)
      : data_reader_writer_(file_path, cluster_size),
        cluster_size_(cluster_size),
        version_(version) {
  }

  ~PVEntriesManager() = default;

  OffsetType LoadStartSections() {
    OffsetType current_cursor = 0;
    PVHeader pv_header = data_reader_writer_.Read<PVHeader>(current_cursor);
    PVHeader default_header;
    if (0 != memcmp(default_header.signature_, pv_header.signature_, sizeof default_header.signature_)) {
      throw exception::YASException("PV header parsing: corrupted signature", StorageError::kInvalidPVSignatureError);
    }
    else if (pv_header.version_ > version_) {
      throw exception::YASException("PV header parsing: unsupported version", StorageError::kInvalidPVSignatureError);
    }
    else if (!offset_traits<OffsetType>::IsExistValue(pv_header.inverted_index_offset_)) {
      throw exception::YASException("PV header parsing: invalid inverted index offset",
        StorageError::kInvalidPVSignatureError);
    }

    current_cursor += sizeof(PVHeader);
    freelist_header_type freelist_header = data_reader_writer_.Read<freelist_header_type>(current_cursor);
    freelist_helper_.SetBins(freelist_header);
    device_end_ = pv_header.pv_size_;
    cluster_size_ = pv_header.cluster_size_;

    return pv_header.inverted_index_offset_;
  }

  void CreateStartSections(const OffsetType index_offset) {
    PVHeader pv_header;
    pv_header.version_ = version_;
    pv_header.priority_ = priority_;
    pv_header.pv_size_ = device_end_;
    pv_header.cluster_size_ = cluster_size_;
    pv_header.inverted_index_offset_ = index_offset;
    data_reader_writer_.Write<PVHeader>(0, pv_header);
    data_reader_writer_.Write<freelist_header_type>(sizeof(PVHeader), freelist_helper_.GetBins());
    device_end_ = sizeof(PVHeader) + sizeof(freelist_header_type);
  }

  OffsetType CreateNewEntryValue(const std::any &content) {
    const PVType pv_type = type_converter_.ConvertToPVType(content);

    switch (pv_type) {
    case PVType::kInt8:
      return createNewEntryValue<Simple4TypeHeader>(PVType::kInt8, std::any_cast<int8_t>(content));
      break;
    case PVType::kUint8:
      return createNewEntryValue<Simple4TypeHeader>(PVType::kUint8, std::any_cast<uint8_t>(content));
      break;
    case PVType::kInt16:
      return createNewEntryValue<Simple4TypeHeader>(PVType::kInt16, std::any_cast<int16_t>(content));
      break;
    case PVType::kUint16:
      return createNewEntryValue<Simple4TypeHeader>(PVType::kUint16, std::any_cast<uint16_t>(content));
      break;
    case PVType::kInt32:
      return createNewEntryValue<Simple4TypeHeader>(PVType::kInt32, std::any_cast<int32_t>(content));
      break;
    case PVType::kUint32:
      return createNewEntryValue<Simple4TypeHeader>(PVType::kUint32, std::any_cast<uint32_t>(content));
      break;
    case PVType::kFloat: {
      static_assert(std::numeric_limits<float>::is_iec559, "The code requires using of IEEE 754 floating point format for binary serialization of floats and doubles.");
      const float float_value = std::any_cast<float>(content);
      return createNewEntryValue<Simple4TypeHeader>(PVType::kFloat, *(reinterpret_cast<const uint32_t*>(&float_value)));
      break;
    }
    case PVType::kDouble: {
      static_assert(std::numeric_limits<double>::is_iec559, "The code requires using of IEEE 754 floating point format for binary serialization of floats and doubles.");
      const double double_value = std::any_cast<double>(content);
      return createNewEntryValue<Simple8TypeHeader>(PVType::kDouble, *(reinterpret_cast<const uint64_t*>(&double_value)));
      break;
    }
    case PVType::kInt64:
      return createNewEntryValue<Simple8TypeHeader>(PVType::kInt64, std::any_cast<int64_t>(content));
      break;
    case PVType::kUint64:
      return createNewEntryValue<Simple8TypeHeader>(PVType::kUint64, std::any_cast<uint64_t>(content));
      break;
    case PVType::kString:
    case PVType::kBlob:
      return createNewEntryValue<ComplexTypeHeader>(content);
      break;
    default:
      throw (exception::YASException("Corrupted storage header type: unsupported type",
          StorageError::kCorruptedHeaderError));
    }

    return offset_traits<OffsetType>::NonExistValue();
  }

  std::any GetEntryContent(OffsetType offset) {
    const PVType pv_type = getRecordType(offset);
    if (pv_type < PVType::k4TypeMax) {
      return getEntryContent<Simple4TypeHeader>(offset);
    }
    else if (pv_type < PVType::k8TypeMax) {
      return getEntryContent<Simple8TypeHeader>(offset);
    }
    else if (pv_type < PVType::kComplexMax) {
      return getEntryContent<ComplexTypeHeader>(offset);
    }

    throw (exception::YASException("Corrupted storage header type: unsupported value type",
        StorageError::kCorruptedHeaderError));
  }

  void DeleteEntry(OffsetType offset) {
    const PVType pv_type = getRecordType(offset);
    if (pv_type < PVType::k4TypeMax) {
      return deleteEntry<Simple4TypeHeader>(offset);
    }
    else if (pv_type < PVType::k8TypeMax) {
      return deleteEntry<Simple8TypeHeader>(offset);
    }
    else if (pv_type < PVType::kComplexMax) {
      return deleteEntry<ComplexTypeHeader>(offset);
    }
    throw (exception::YASException("Corrupted storage header type: unsupported type",
        StorageError::kCorruptedHeaderError));
  }

  bool GetEntryExpiredDate(OffsetType offset, utils::Time &expired_date) {
    const PVState pv_state = data_reader_writer_.Read<PVState>(offset);
    if (pv_state.value_type_ < PVType::k4TypeMax) {
      return getEntryExpiredDate<Simple4TypeHeader>(offset, expired_date);
    }
    else if (pv_state.value_type_ < PVType::k8TypeMax) {
      return getEntryExpiredDate<Simple8TypeHeader>(offset, expired_date);
    }
    else if (pv_state.value_type_ < PVType::kComplexMax) {
      return getEntryExpiredDate<ComplexTypeHeader>(offset, expired_date);
    }

    throw (exception::YASException("Corrupted storage header type: unsupported type",
      StorageError::kCorruptedHeaderError));
  }

  void SetEntryExpiredDate(OffsetType offset, utils::Time &expired_date) {
    const PVType pv_type = getRecordType(offset);
    if (pv_type < PVType::k4TypeMax) {
      return setEntryExpiredDate<Simple4TypeHeader>(offset, expired_date);
    }
    else if (pv_type < PVType::k8TypeMax) {
      return setEntryExpiredDate<Simple8TypeHeader>(offset, expired_date);
    }
    else if (pv_type < PVType::kComplexMax) {
      return setEntryExpiredDate<ComplexTypeHeader>(offset, expired_date);
    }
    throw (exception::YASException("Corrupted storage header type: unsupported type",
        StorageError::kCorruptedHeaderError));
  }

  uint32_t priority() const { return priority; }

 private:
  PVDeviceDataReaderWriter<OffsetType, Device> data_reader_writer_;
  freelist_helper::FreelistHelper<OffsetType> freelist_helper_;
  EntriesTypeConverter type_converter_;
  OffsetType device_end_;
  uint32_t cluster_size_;
  utils::Version version_;
  uint32_t priority_;

  template<typename HeaderType, typename ValueType>
  OffsetType createNewEntryValue(PVType pv_type, ValueType content) {
    auto new_entry_offset = getFreeOffset(sizeof(HeaderType));

    HeaderType header;
    header.value_type_ = pv_type;
    header.value_state_ = PVTypeState::kEmpty;
    header.value_ = content;
    data_reader_writer_.Write<HeaderType>(new_entry_offset, header);

    return new_entry_offset;
  }

  template<typename HeaderType>
  OffsetType createNewEntryValue(const std::any &content) {
    const auto pv_type = type_converter_.ConvertToPVType(content);
    switch (pv_type) {
    case PVType::kString: {
      auto &string_value = std::any_cast<const std::string&>(content);
      return writeComplexType(PVType::kString, std::cbegin(string_value), std::cend(string_value));
      break;
    }
    case PVType::kBlob: {
      auto &vector_value = std::any_cast<const ByteVector&>(content);
      return writeComplexType(PVType::kBlob, std::cbegin(vector_value), std::cend(vector_value));
      break;
    }
    default:
      return offset_traits<OffsetType>::NonExistValue();
    }
  }

  template<typename HeaderType>
  std::any getEntryContent(OffsetType offset) {
    HeaderType header = data_reader_writer_.Read<HeaderType>(offset);
    if (header.value_state_ & PVTypeState::kIsExpired) {
      utils::Time value_time(header.expired_time_low_, header.expired_time_high_);
      if (value_time.IsExpired()) {
        throw (exception::YASException("The key has been expired but doesn't delete from storage",
          StorageError::kKeyExpired));
      }
    }

    return type_converter_.ConvertToUserType(header.value_type_, header.value_);
  }

  template<>
  std::any getEntryContent<ComplexTypeHeader>(OffsetType offset) {
    ComplexTypeHeader header = data_reader_writer_.Read<ComplexTypeHeader>(offset);
    if (header.value_state_ & PVTypeState::kIsExpired) {
      utils::Time value_time(header.expired_time_low_, header.expired_time_high_);
      if (value_time.IsExpired()) {
        throw (exception::YASException("The key has been expired but doesn't delete from storage",
          StorageError::kKeyExpired));
      }
    }

    const auto data = data_reader_writer_.ReadComplexType(offset);
    return type_converter_.ConvertToUserType(header.value_type_, std::cbegin(data), std::cend(data));
  }

  template<typename HeaderType>
  bool getEntryExpiredDate(OffsetType offset, utils::Time &expired_date) {
    HeaderType header = data_reader_writer_.Read<HeaderType>(offset);
    if (!(header.value_state_ & PVTypeState::kIsExpired)) {
      return false;
    }
    expired_date = utils::Time(header.expired_time_low_, header.expired_time_high_);
    return true;
  }

  template<typename HeaderType>
  void deleteEntry(OffsetType offset) {
    HeaderType header = data_reader_writer_.Read<HeaderType>(offset);
    header.value_type_ = (sizeof(HeaderType) == sizeof(Simple4TypeHeader) ? PVType::kEmpty4Simple : PVType::kEmpty8Simple);
    header.value_state_ = PVTypeState::kEmpty;
    header.next_free_entry_offset_ = freelist_helper_.GetFreeEntry(sizeof(HeaderType));
    data_reader_writer_.Write<HeaderType>(offset, header);
    freelist_helper_.PushFreeEntry(offset, sizeof(HeaderType));
  }

  template<>
  void deleteEntry<ComplexTypeHeader>(OffsetType offset) {
    ComplexTypeHeader header = data_reader_writer_.Read<ComplexTypeHeader>(offset);
    const auto overall_size = header.overall_size_;
    auto next_entry_offset = header.sequel_offset_;

    header.value_type_ = PVType::kEmptyComplex;
    header.value_state_ = PVTypeState::kComplexBegin;
    header.overall_size_ = header.chunk_size_;
    header.next_free_entry_offset_ = freelist_helper_.GetFreeEntry(header.chunk_size_);
    data_reader_writer_.Write<ComplexTypeHeader>(offset, header);
    freelist_helper_.PushFreeEntry(offset, header.chunk_size_ + offsetof(ComplexTypeHeader, data_));

    auto deleted = header.chunk_size_;
    while (deleted < overall_size && offset_traits<OffsetType>::IsExistValue(next_entry_offset)) {
      const auto saved_next_entry_offset = next_entry_offset;
      header = data_reader_writer_.Read<ComplexTypeHeader>(next_entry_offset);

      next_entry_offset = header.sequel_offset_;
      header.value_type_ = PVType::kEmptyComplex;
      header.value_state_ = PVTypeState::kComplexBegin;
      header.overall_size_ = header.chunk_size_;
      header.next_free_entry_offset_ = freelist_helper_.GetFreeEntry(header.chunk_size_);
      data_reader_writer_.Write<ComplexTypeHeader>(saved_next_entry_offset, header);
      freelist_helper_.PushFreeEntry(saved_next_entry_offset, header.chunk_size_
        + offsetof(ComplexTypeHeader, data_));
      deleted += header.chunk_size_;
    }
  }

  template<typename HeaderType>
  void setEntryExpiredDate(OffsetType offset, utils::Time &expired_date) {
    HeaderType header = data_reader_writer_.Read<HeaderType>(offset);
    header.value_type_ = header.value_type_;
    header.value_state_ = PVTypeState::kIsExpired;
    header.expired_time_high_ = expired_date.expired_time_high();
    header.expired_time_low_ = expired_date.expired_time_low();
    data_reader_writer_.Write<HeaderType>(offset, header);
  }

  PVType getRecordType(OffsetType offset) {
    const auto pv_state = data_reader_writer_.Read<PVState>(offset);
    return pv_state.value_type_;
  }

  template<typename Iterator>
  OffsetType writeComplexType(PVType value_type, const Iterator begin, const Iterator end) {
    const OffsetType data_size = std::distance(begin, end);
    auto free_offset = getFreeOffset(data_size + sizeof(ComplexTypeHeader));
    auto first_free_offset = free_offset;

    bool is_first = true;
    OffsetType written = 0;
    OffsetType overall_written = 0;
    while (overall_written < data_size) {
      auto new_begin = begin;
      std::advance(new_begin, overall_written);
      auto next_free_offset = getFreeOffset(data_size - written + sizeof(ComplexTypeHeader));
      written = data_reader_writer_.WriteComplexType(free_offset, value_type, is_first, next_free_offset,
        new_begin, end);
      is_first = false;

      free_offset = next_free_offset;
      overall_written += written;
    }

    return first_free_offset;
  }

  OffsetType getFreeOffset(OffsetType entry_size) {
    if (0 == entry_size) {
      return offset_traits<OffsetType>::NonExistValue();
    }

    auto offset = freelist_helper_.PopFreeEntryOffset(entry_size);
    if (!offset_traits<OffsetType>::IsExistValue(offset)) {
      expandPV();
      offset = freelist_helper_.PopFreeEntryOffset(entry_size);
      if (!offset_traits<OffsetType>::IsExistValue(offset)) {
        throw (exception::YASException("Error during physical volume size extended", StorageError::kDeviceExpandError));
      }
    }

    const auto value_type = getRecordType(offset);
    if (PVType::kEmptyComplex != value_type) {
      if (PVType::kEmpty4Simple == value_type) {
        recoverAndPushNextEntry<Simple4TypeHeader>(offset);
      }
      else if (PVType::kEmpty8Simple == value_type) {
        recoverAndPushNextEntry<Simple8TypeHeader>(offset);
      }
      // difference between sizeof Simple header isn't much to split
      return offset;
    }

    // split headers
    const auto header = data_reader_writer_.Read<ComplexTypeHeader>(offset);
    recoverAndPushNextEntry<ComplexTypeHeader>(offset);
    if (entry_size > header.overall_size_) {
      return offset;
    }

    const auto split_size = header.overall_size_ - entry_size;
    const auto split_offset = offset + entry_size;
    if (split_size > sizeof(ComplexTypeHeader)) {
      ComplexTypeHeader header;
      header.value_type_ = PVType::kEmptyComplex;
      header.next_free_entry_offset_ = freelist_helper_.GetFreeEntry(split_size);
      header.overall_size_ = split_size;
      header.chunk_size_ = split_size;
      data_reader_writer_.Write<ComplexTypeHeader>(split_offset, header);
      freelist_helper_.PushFreeEntry(split_offset, split_size);
    }
    else if (split_size >= sizeof(Simple4TypeHeader) && split_size < sizeof(Simple8TypeHeader)) {
      Simple4TypeHeader header;
      header.next_free_entry_offset_ = freelist_helper_.GetFreeEntry(split_size);
      header.value_type_ = PVType::kEmpty4Simple;
      data_reader_writer_.Write<Simple4TypeHeader>(split_offset, header);
      freelist_helper_.PushFreeEntry(split_offset, split_size);
    }
    else if (split_size >= sizeof(Simple8TypeHeader)) {
      Simple8TypeHeader header;
      header.next_free_entry_offset_ = freelist_helper_.GetFreeEntry(split_size);
      header.value_type_ = PVType::kEmpty8Simple;
      data_reader_writer_.Write<Simple8TypeHeader>(split_offset, header);
      freelist_helper_.PushFreeEntry(split_offset, split_size);
    }

    return offset;
  }

  template<typename HeaderType>
  bool recoverAndPushNextEntry(OffsetType offset) {
    HeaderType header = data_reader_writer_.Read<HeaderType>(offset);
    const auto next_free_entry = header.next_free_entry_offset_;
    if (offset_traits<OffsetType>::IsExistValue(next_free_entry)) {
      return false;
    }

    freelist_helper_.PushFreeEntry(next_free_entry, sizeof(HeaderType));
    return true;
  }

  template<>
  bool recoverAndPushNextEntry<ComplexTypeHeader>(OffsetType offset) {
    ComplexTypeHeader header = data_reader_writer_.Read<ComplexTypeHeader>(offset);
    const auto next_free_entry = header.next_free_entry_offset_;
    if (!offset_traits<OffsetType>::IsExistValue(next_free_entry)) {
      return false;
    }

    header = data_reader_writer_.Read<ComplexTypeHeader>(next_free_entry);
    freelist_helper_.PushFreeEntry(next_free_entry, header.overall_size_);
    return true;
  }

  void expandPV() {
    // write several new clusters - note that in future it should be good to add some strategy to choose the count of
    // simultaneously added clusters
    constexpr int clusters_count = 5;
    constexpr ByteVector::value_type debug_filler = 0xAA;

    ByteVector new_cluster(cluster_size_, debug_filler);

    ComplexTypeHeader header;
    header.overall_size_ = cluster_size_ - offsetof(ComplexTypeHeader, data_);
    header.chunk_size_ = cluster_size_ - offsetof(ComplexTypeHeader, data_);
    header.value_type_ = PVType::kEmptyComplex;

    for (int new_cluster_id = 0; new_cluster_id < clusters_count; ++new_cluster_id) {
      const auto next_free_entry_offset = freelist_helper_.GetFreeEntry(cluster_size_);
      header.next_free_entry_offset_ = next_free_entry_offset;
      serialization_utils::SaveAsBytes(std::begin(new_cluster), std::end(new_cluster), &header);

      data_reader_writer_.RawWrite(device_end_, std::cbegin(new_cluster), std::cend(new_cluster));
      freelist_helper_.PushFreeEntry(device_end_, cluster_size_);
      device_end_ += cluster_size_;
    }
  }
};

} // namespace pv
} // namespace yas
