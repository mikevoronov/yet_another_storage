#pragma once
#include "../common/filesystem.h"
#include "../physical_volume/pv_layout_headers.h"
#include "../utils/serialization_utils.h"
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
using namespace yas::storage;

namespace yas {
namespace device_worker {

template<typename OffsetType>
using DefaultDevice = devices::FileDevice<OffsetType>;

// this class represent 
template <typename CharType=DefaultCharType, typename OffsetType=DefaultOffsetType, typename Device=DefaultDevice<OffsetType>>
class PVVolumeManager : public IStorage<CharType> {
  using FreelistHeaderType = FreelistHeader<OffsetType>;

 public:
  using pv_volume_manager_type = PVVolumeManager<CharType, OffsetType, Device>;
  using key_type = IStorage<CharType>::key_type;

  ~PVVolumeManager() {
    close();
  }

  static std::unique_ptr<pv_volume_manager_type> Load(fs::path &file_path, utils::Version version) {
    auto pv_volume_manager = std::unique_ptr<pv_volume_manager_type>(new pv_volume_manager_type(file_path));

    OffsetType current_cursor = 0;
    PVHeader pv_header = pv_volume_manager->data_reader_writer_.Read<PVHeader>(current_cursor);
    PVHeader default_header;
    if (0 != memcmp(default_header.signature_, pv_header.signature_, sizeof default_header.signature_)) {
      throw exception::YASException("Invalid PV header: corrupted signature", StorageError::kInvalidSignatureError);
    }
    else if (version < pv_header.version_) {
      throw exception::YASException("Invalid PV header: unsupported version", StorageError::kInvalidSignatureError);
    }

    current_cursor += sizeof(PVHeader);
    FreelistHeaderType freelist_header = pv_volume_manager->data_reader_writer_.Read<FreelistHeaderType>(sizeof(PVHeader));
    pv_volume_manager->freelist_helper_.SetBins(freelist_header);

    current_cursor += sizeof(FreelistHeaderType);
    ByteVector deserialize_indexer = pv_volume_manager->data_reader_writer_.ReadComplexType(current_cursor);
    
    
    pv_volume_manager->device_end_ = pv_header.pv_size_;
    pv_volume_manager->cluster_size_ = pv_header.cluster_size_;
    return pv_volume_manager;
  }

  static std::unique_ptr<pv_volume_manager_type> Create(fs::path &file_path, utils::Version version,
      uint32_t priority, uint32_t cluster_size = kDefaultClusterSize) {
    const OffsetType device_end = (sizeof(PVHeader) + sizeof(FreelistHeader<OffsetType>) + kDefaultClusterSize);
    if (fs::exists(file_path)) {
      fs::resize_file(file_path, device_end);
    }
    else {
      std::ofstream out(file_path, std::ios_base::out);
    }

    // std::make_unique needs access to the class ctor
    auto pv_volume_manager = std::unique_ptr<pv_volume_manager_type>(new pv_volume_manager_type(file_path, cluster_size));

    PVHeader pv_header;
    pv_header.version_ = version;
    pv_header.priority_ = priority;
    pv_header.pv_size_ = device_end;
    pv_header.cluster_size_ = cluster_size;
    pv_volume_manager->data_reader_writer_.Write<PVHeader>(0, pv_header);

    return pv_volume_manager;
  }

  virtual nonstd::expected<bool, StorageErrorDescriptor> Put(key_type key, std::any value) override {
    std::lock_guard<std::mutex> lock(manager_guard_mutex_);
    try {
      return true;
    }
    catch (...) {
      return nonstd::make_unexpected(exception::YASExceptionHandler(std::current_exception()));
    }
  }

  virtual nonstd::expected<std::any, StorageErrorDescriptor> Get(key_type key) const override {
    std::lock_guard<std::mutex> lock(manager_guard_mutex_);
    try {
      return true;
    }
    catch (...) {
      return nonstd::make_unexpected(exception::YASExceptionHandler(std::current_exception()));
    }
  }

  virtual nonstd::expected<bool, StorageErrorDescriptor> HasKey(key_type key) const override {
    std::lock_guard<std::mutex> lock(manager_guard_mutex_);
    try {
      return inverted_index_.HasKey(key);
    }
    catch (...) {
      return nonstd::make_unexpected(exception::YASExceptionHandler(std::current_exception()));
    }
  }
  
  virtual nonstd::expected<bool, StorageErrorDescriptor> Delete(key_type key) override {
    std::lock_guard<std::mutex> lock(manager_guard_mutex_);
    try {
      if (!inverted_index_.HasKey(key)) {
        return nonstd::make_unexpected(StorageErrorDescriptor( "Delete key: key hasn't been found", StorageError::kKeyNotFound));
      }
      const auto leaf = inverted_index_.Get(key);
      inverted_index_.Delete(key);
      deleteRecord(leaf);
      return true;
    }
    catch (...) {
      return nonstd::make_unexpected(exception::YASExceptionHandler(std::current_exception()));
    }
  }

  virtual nonstd::expected<bool, StorageErrorDescriptor> SetExpiredDate(key_type key, time_t expired) override {
    std::lock_guard<std::mutex> lock(manager_guard_mutex_);
    try {
      if (!inverted_index_.HasKey(key)) {
        return nonstd::make_unexpected(StorageErrorDescriptor("SetExpiredDate key: key hasn't been found", StorageError::kKeyNotFound));
      }
      const auto leaf = inverted_index_.Get(key);
      inverted_index_.Delete(key);
      setExpiredDate(leaf, expired);

      return true;
    }
    catch (...) {
      return nonstd::make_unexpected(exception::YASExceptionHandler(std::current_exception()));
    }
  }

  virtual nonstd::expected<time_t, StorageErrorDescriptor> GetExpiredDate(key_type key) override {
    std::lock_guard<std::mutex> lock(manager_guard_mutex_);
    try {
      return true;
    }
    catch (...) {
      return nonstd::make_unexpected(exception::YASExceptionHandler(std::current_exception()));
    }
  }

#ifdef UNIT_TEST
  Device& GetDevice() const { return device_; }
#endif

  PVVolumeManager(const PVVolumeManager&) = delete;
  PVVolumeManager(PVVolumeManager&&) = default;
  PVVolumeManager& operator=(const PVVolumeManager&) = delete;
  PVVolumeManager& operator=(PVVolumeManager&&) = delete;

 private:
  device_worker::PVDeviceDataReaderWriter<OffsetType, Device> data_reader_writer_;
  index_helper::InvertedIndexHelper<CharType, OffsetType> inverted_index_;
  freelist_helper::FreelistHelper<OffsetType> freelist_helper_;
  types::TypeConverter type_converter_;
  mutable std::mutex manager_guard_mutex_;        // std::lock_guard recieves the argument by non-const ref
  OffsetType device_end_;
  uint32_t cluster_size_;
  utils::Version version_;

  explicit PVVolumeManager(fs::path &file_path, uint32_t cluster_size = kDefaultClusterSize)
      : data_reader_writer_(file_path, cluster_size),
        cluster_size_(cluster_size) {
  }

  void close() {
 //   data_reader_writer_.Write<FreelistHeaderType>(sizeof(PVHeader), freelist_helper_.GetBins());

    // TODO : exception during serailization
    const auto serialized_index = inverted_index_.Serialize<OffsetType>(version_);
    data_reader_writer_.WriteComplexType(sizeof(PVHeader) + sizeof(FreelistHeaderType), 
        std::cbegin(serialized_index), std::cend(serialized_index));
  }

  void deleteRecord(OffsetType offset) {
    PVType pv_type = getRecordType(offset);
    //deleteRecord<type_converter_.ConvertToStorageType(pv_type)>(offset);
  }

  template <typename Type>
  void deleteRecord(OffsetType offset) {
    static_assert(false, "Something went wrong with type mapping");
  }

  template<>
  void deleteRecord<types::Simple4Type>(OffsetType offset) {

  }

  template<>
  void deleteRecord<types::Simple8Type>(OffsetType offset) {

  }

  template<>
  void deleteRecord<types::ComplexType>(OffsetType offset) {

  }

  void setExpiredDate(OffsetType offset, time_t expired_time) {

  }

  PVType getRecordType(OffsetType offset) {
    return data_reader_writer_.Read<PVType>(offset);
  }
};

} // namespace device_worker
} // namespace yas
