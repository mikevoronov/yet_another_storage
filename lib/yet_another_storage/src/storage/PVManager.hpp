#pragma once
#include "../common/filesystem.h"
#include "../physical_volume/pv_layout_headers.h"
#include "../utils/Time.hpp"
#include "../device_worker/devices/FileDevice.hpp"
#include "../common/common.h"
#include "../utils/Version.hpp"
#include "../storage/storage_errors.hpp"
#include "../physical_volume/freelist_helper/FreelistHelper.hpp"
#include "../physical_volume/inverted_index_helper/InvertedIndexHelper.hpp"
#include "../common/offset_type_traits.hpp"
#include "../storage/IStorage.hpp"
#include "../exceptions/YASExceptionHandler.h"
#include "../device_worker/PVDeviceDataReaderWriter.hpp"
#include "StorageTypeConverter.hpp"
#include <mutex>

using namespace yas::pv_layout_headers;

namespace yas {
namespace storage {

template<typename OffsetType>
using DefaultDevice = devices::FileDevice<OffsetType>;

// this class represent 
template <typename CharType=CharType, typename OffsetType=OffsetType, typename Device=DefaultDevice<OffsetType>>
class PVManager : public IStorage<CharType> {
  using FreelistHeaderType = FreelistHeader<OffsetType>;
  using InvertedIndexType = index_helper::InvertedIndexHelper<CharType, OffsetType>;

 public:
  using pv_volume_manager_type = PVManager<CharType, OffsetType, Device>;
  using key_type = IStorage<CharType>::key_type;

  ~PVManager() {
    close();
  }

  static std::unique_ptr<pv_volume_manager_type> Load(fs::path &file_path, utils::Version version) {
    auto pv_volume_manager = std::unique_ptr<pv_volume_manager_type>(new pv_volume_manager_type(file_path));

    OffsetType current_cursor = 0;
    PVHeader pv_header = pv_volume_manager->data_reader_writer_.Read<PVHeader>(current_cursor);
    PVHeader default_header;
    if (0 != memcmp(default_header.signature_, pv_header.signature_, sizeof default_header.signature_)) {
      throw exception::YASException("PV header parsing: corrupted signature", StorageError::kInvalidPVSignatureError);
    }
    else if (pv_header.version_ > version) {
      throw exception::YASException("PV header parsing: unsupported version", StorageError::kInvalidPVSignatureError);
    }
    else if (!offset_traits<OffsetType>::IsExistValue(pv_header.inverted_index_offset_)) {
      throw exception::YASException("PV header parsing: invalid inverted index offset", 
          StorageError::kInvalidPVSignatureError);
    }

    current_cursor += sizeof(PVHeader);
    FreelistHeaderType freelist_header = pv_volume_manager->data_reader_writer_.Read<FreelistHeaderType>(current_cursor);
    pv_volume_manager->freelist_helper_.SetBins(freelist_header);
    ByteVector deserialized_indexer = 
        pv_volume_manager->data_reader_writer_.ReadComplexType(pv_header.inverted_index_offset_);
    auto indexer = InvertedIndexType::Deserialize<InvertedIndexType::leaf_type>(std::cbegin(deserialized_indexer),
        std::cend(deserialized_indexer), pv_header.version_);

    pv_volume_manager->inverted_index_ = std::move(indexer);
    pv_volume_manager->device_end_ = pv_header.pv_size_;
    pv_volume_manager->cluster_size_ = pv_header.cluster_size_;
    return pv_volume_manager;
  }

  static std::unique_ptr<pv_volume_manager_type> Create(fs::path &file_path, utils::Version version,
      uint32_t priority, uint32_t cluster_size = kDefaultClusterSize) {
    const OffsetType device_end = (sizeof(PVHeader) + sizeof(FreelistHeader<OffsetType>));
    if (fs::exists(file_path)) {
      fs::resize_file(file_path, device_end);
    }
    else {
      std::ofstream out(file_path, std::ios_base::out);
    }

    // std::make_unique needs access to the class ctor
    auto pv_volume_manager = std::unique_ptr<pv_volume_manager_type>(new pv_volume_manager_type(file_path, cluster_size));
    pv_volume_manager->inverted_index_.reset(new InvertedIndexType());

    PVHeader pv_header;
    pv_header.version_ = version;
    pv_header.priority_ = priority;
    pv_header.pv_size_ = device_end;
    pv_header.cluster_size_ = cluster_size;
    pv_header.inverted_index_offset_ = offset_traits<OffsetType>::NonExistValue();
    pv_volume_manager->data_reader_writer_.Write<PVHeader>(0, pv_header);
    pv_volume_manager->device_end_ = sizeof(PVHeader) + sizeof(FreelistHeaderType);
    pv_volume_manager->expandPV();

    return pv_volume_manager;
  }

  virtual nonstd::expected<bool, StorageErrorDescriptor> Put(key_type key, std::any value) override {
    std::lock_guard<std::mutex> lock(manager_guard_mutex_);
    try {
      if (!value.has_value()) {
        return nonstd::make_unexpected(StorageErrorDescriptor("std::any doesn't contain any value", 
            StorageError::kValueNotFound));
      }
      if (inverted_index_->HasKey(key)) {
        return nonstd::make_unexpected(StorageErrorDescriptor("The storage already has current key, please remove it \
            first", StorageError::kKeyAlreadyCreated));
      }

      const auto offset = setEntryValue(value);
      inverted_index_->Insert(key, offset);
      return true;
    }
    catch (...) {
      return nonstd::make_unexpected(exception::YASExceptionHandler(std::current_exception()));
    }
  }

  virtual nonstd::expected<std::any, StorageErrorDescriptor> Get(key_type key) override {
    std::lock_guard<std::mutex> lock(manager_guard_mutex_);
    try {
      const auto entry_offset = inverted_index_->Get(key);
      if (!offset_traits<OffsetType>::IsExistValue(entry_offset)) {
        return nonstd::make_unexpected(StorageErrorDescriptor("Delete key: key hasn't been found", StorageError::kKeyNotFound));
      }
      return getEntryValue(entry_offset);
    }
    catch (...) {
      return nonstd::make_unexpected(exception::YASExceptionHandler(std::current_exception()));
    }
  }

  virtual nonstd::expected<bool, StorageErrorDescriptor> HasKey(key_type key) override {
    std::lock_guard<std::mutex> lock(manager_guard_mutex_);
    try {
      return inverted_index_->HasKey(key);
    }
    catch (...) {
      return nonstd::make_unexpected(exception::YASExceptionHandler(std::current_exception()));
    }
  }
  
  virtual nonstd::expected<bool, StorageErrorDescriptor> Delete(key_type key) override {
    std::lock_guard<std::mutex> lock(manager_guard_mutex_);
    try {
      const auto entry_offset = inverted_index_->Get(key);
      if (!offset_traits<OffsetType>::IsExistValue(entry_offset)) {
        return nonstd::make_unexpected(StorageErrorDescriptor( "Delete key: key hasn't been found", StorageError::kKeyNotFound));
      }
      deleteEntry(entry_offset);
      inverted_index_->Delete(key);
      return true;
    }
    catch (...) {
      return nonstd::make_unexpected(exception::YASExceptionHandler(std::current_exception()));
    }
  }

  virtual nonstd::expected<bool, StorageErrorDescriptor> SetExpiredDate(key_type key, time_t expired) override {
    std::lock_guard<std::mutex> lock(manager_guard_mutex_);
    try {
      const auto entry_offset = inverted_index_->Get(key);
      if (!offset_traits<OffsetType>::IsExistValue(entry_offset)) {
        return nonstd::make_unexpected(StorageErrorDescriptor("SetExpiredDate key: key hasn't been found", StorageError::kKeyNotFound));
      }
      utils::Time expired_time(expired);
      setExpiredDate(entry_offset, expired_time);
      return true;
    }
    catch (...) {
      return nonstd::make_unexpected(exception::YASExceptionHandler(std::current_exception()));
    }
  }

  virtual nonstd::expected<time_t, StorageErrorDescriptor> GetExpiredDate(key_type key) override {
    std::lock_guard<std::mutex> lock(manager_guard_mutex_);
    try {
      const auto entry_offset = inverted_index_->Get(key);
      if (!offset_traits<OffsetType>::IsExistValue(entry_offset)) {
        return nonstd::make_unexpected(StorageErrorDescriptor("SetExpiredDate key: key hasn't been found", StorageError::kKeyNotFound));
      }
      const auto expired_date = getExpiredDate(entry_offset);
      return expired_date.GetTime();
    }
    catch (...) {
      return nonstd::make_unexpected(exception::YASExceptionHandler(std::current_exception()));
    }
  }

  uint32_t priority() const { return priority_; }

#ifdef UNIT_TEST
  Device& GetDevice() const { return device_; }
#endif

  PVManager(const PVManager&) = delete;
  PVManager(PVManager&&) = default;
  PVManager& operator=(const PVManager&) = delete;
  PVManager& operator=(PVManager&&) = delete;

 private:
  device_worker::PVDeviceDataReaderWriter<OffsetType, Device> data_reader_writer_;
  std::unique_ptr<InvertedIndexType> inverted_index_;
  freelist_helper::FreelistHelper<OffsetType> freelist_helper_;
  TypeConverter type_converter_;
  mutable std::mutex manager_guard_mutex_;        // std::lock_guard recieves the argument by non-const ref
  OffsetType device_end_;
  uint32_t cluster_size_;
  uint32_t priority_;
  utils::Version version_;

  explicit PVManager(fs::path &file_path, uint32_t cluster_size = kDefaultClusterSize)
      : data_reader_writer_(file_path, cluster_size),
        cluster_size_(cluster_size) {
  }

  void close() {
    // TODO : exception during serailization 
    const auto serialized_index = inverted_index_->Serialize<OffsetType>(version_);

    PVHeader header = data_reader_writer_.Read<PVHeader>(0);
    data_reader_writer_.Write<FreelistHeaderType>(sizeof(PVHeader), freelist_helper_.GetBins());

    if (offset_traits<OffsetType>::IsExistValue(header.inverted_index_offset_)) {
      deleteEntry(header.inverted_index_offset_);
    }

    const auto inverted_index_offset = 
        writeComplexType(PVType::kInvertedIndex, std::cbegin(serialized_index), std::cend(serialized_index));
    header.inverted_index_offset_ = inverted_index_offset;
    header.cluster_size_ = cluster_size_;
    header.pv_size_ = device_end_;
    header.priority_ = priority_;
    data_reader_writer_.Write<PVHeader>(0, header);
  }

  OffsetType setEntryValue(std::any &value) {
    const PVType pv_type = type_converter_.ConvertToPVType(value);

    switch (pv_type) {
    case PVType::kInt8:
      return setEntryValue<Simple4TypeHeader>(PVType::kInt8, std::any_cast<int8_t>(value));
      break;
    case PVType::kUint8:
      return setEntryValue<Simple4TypeHeader>(PVType::kUint8, std::any_cast<uint8_t>(value));
      break;
    case PVType::kInt16:
      return setEntryValue<Simple4TypeHeader>(PVType::kInt16, std::any_cast<int16_t>(value));
      break;
    case PVType::kUint16:
      return setEntryValue<Simple4TypeHeader>(PVType::kUint16, std::any_cast<uint16_t>(value));
      break;
    case PVType::kInt32:
      return setEntryValue<Simple4TypeHeader>(PVType::kInt32, std::any_cast<int32_t>(value));
      break;
    case PVType::kUint32:
      return setEntryValue<Simple4TypeHeader>(PVType::kUint32, std::any_cast<uint32_t>(value));
      break;
    case PVType::kFloat: {
      static_assert(std::numeric_limits<float>::is_iec559, "The code requires using of IEEE 754 floating point format for binary serialization of floats and doubles.");
      const float float_value = std::any_cast<float>(value);
      return setEntryValue<Simple4TypeHeader>(PVType::kFloat, *(reinterpret_cast<const uint32_t*>(&float_value)));
      break;
    }
    case PVType::kDouble: {
      static_assert(std::numeric_limits<double>::is_iec559, "The code requires using of IEEE 754 floating point format for binary serialization of floats and doubles.");
      const double double_value = std::any_cast<double>(value);
      return setEntryValue<Simple8TypeHeader>(PVType::kDouble, *(reinterpret_cast<const uint64_t*>(&double_value)));
      break;
    }
    case PVType::kInt64:
      return setEntryValue<Simple8TypeHeader>(PVType::kInt64, std::any_cast<int64_t>(value));
      break;
    case PVType::kUint64:
      return setEntryValue<Simple8TypeHeader>(PVType::kUint64, std::any_cast<uint64_t>(value));
      break;
    case PVType::kString:
    case PVType::kBlob:
      return setEntryValue<ComplexTypeHeader>(value);
      break;
    default:
      throw (exception::YASException("Corrupted storage header type: unsupported type",
          StorageError::kCorruptedHeaderError));
    }

    return offset_traits<OffsetType>::NonExistValue();
  }

  template<typename HeaderType, typename ValueType>
  OffsetType setEntryValue(PVType pv_type, ValueType value) {
    auto new_entry_offset = getFreeOffset(sizeof(HeaderType));

    HeaderType header;
    header.value_type_ = pv_type;
    header.value_state_ = PVTypeState::kEmpty;
    header.value_ = value;
    data_reader_writer_.Write<HeaderType>(new_entry_offset, header);

    return new_entry_offset;
  }

  template<typename HeaderType>
  OffsetType setEntryValue(const std::any &value) {
    const auto pv_type = type_converter_.ConvertToPVType(value);
    switch (pv_type) {
    case PVType::kString: {
      auto &string_value = std::any_cast<const std::string&>(value);
      return writeComplexType(PVType::kString, std::cbegin(string_value), std::cend(string_value));
      break;
    }
    case PVType::kBlob: {
      auto &vector_value = std::any_cast<const ByteVector&>(value);
      return writeComplexType(PVType::kBlob, std::cbegin(vector_value), std::cend(vector_value));
      break;
    }
    default:
      return offset_traits<OffsetType>::NonExistValue();
    }
  }

  std::any getEntryValue(OffsetType offset) {
    const PVType pv_type = getRecordType(offset);
    if (pv_type < PVType::k4TypeMax) {
      return getEntryValue<Simple4TypeHeader>(offset);
    }
    else if (pv_type < PVType::k8TypeMax) {
      return getEntryValue<Simple8TypeHeader>(offset);
    }
    else if (pv_type < PVType::kComplexMax) {
      return getEntryValue<ComplexTypeHeader>(offset);
    }

    throw (exception::YASException("Corrupted storage header type: unsupported value type",
        StorageError::kCorruptedHeaderError));
  }

  template<typename HeaderType>
  std::any getEntryValue(OffsetType offset) {
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
  std::any getEntryValue<ComplexTypeHeader>(OffsetType offset) {
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

  utils::Time getExpiredDate(OffsetType offset) {
    const PVState pv_state = data_reader_writer_.Read<PVState>(offset);
    if (!(pv_state.value_state_ & PVTypeState::kIsExpired)) {
      throw (exception::YASException("Expired date hasn't been setted for this value yet",
          StorageError::kKeyDoesntExpired));
    }

    if (pv_state.value_type_ < PVType::k4TypeMax) {
      return getExpiredDate<Simple4TypeHeader>(offset);
    }
    else if (pv_state.value_type_ < PVType::k8TypeMax) {
      return getExpiredDate<Simple8TypeHeader>(offset);
    }
    else if (pv_state.value_type_ < PVType::kComplexMax) {
      return getExpiredDate<ComplexTypeHeader>(offset);
    }

    throw (exception::YASException("Corrupted storage header type: unsupported type", 
        StorageError::kCorruptedHeaderError));
  }

  template<typename HeaderType>
  utils::Time getExpiredDate(OffsetType offset) {
    HeaderType header = data_reader_writer_.Read<HeaderType>(offset);
    return utils::Time(header.expired_time_low_, header.expired_time_high_);
  }

  void deleteEntry(OffsetType offset) {
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

  void setExpiredDate(OffsetType offset, utils::Time &expired_time) {
    const PVType pv_type = getRecordType(offset);
    if (pv_type < PVType::k4TypeMax) {
      return setExpiredDate<Simple4TypeHeader>(offset, expired_time);
    }
    else if (pv_type < PVType::k8TypeMax) {
      return setExpiredDate<Simple8TypeHeader>(offset, expired_time);
    }
    else if (pv_type < PVType::kComplexMax) {
      return setExpiredDate<ComplexTypeHeader>(offset, expired_time);
    }
    throw (exception::YASException("Corrupted storage header type: unsupported type",
        StorageError::kCorruptedHeaderError));
  }

  template<typename HeaderType>
  void setExpiredDate(OffsetType offset, utils::Time &expired_time) {
    HeaderType header = data_reader_writer_.Read<HeaderType>(offset);
    header.value_type_ = header.value_type_;
    header.value_state_ = PVTypeState::kIsExpired;
    header.expired_time_high_ = expired_time.expired_time_high();
    header.expired_time_low_ = expired_time.expired_time_low();
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

      // at first write througth the device
      data_reader_writer_.RawWrite(device_end_, std::cbegin(new_cluster), std::cend(new_cluster));
      // and only then update free lists for exception safety
      freelist_helper_.PushFreeEntry(device_end_, cluster_size_);
      device_end_ += cluster_size_;
    }
  }
};

} // namespace device_worker
} // namespace yas
