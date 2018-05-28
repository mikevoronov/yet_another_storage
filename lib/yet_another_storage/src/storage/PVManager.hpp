#pragma once
#include "../common/filesystem.h"
#include "../physical_volume/pv_layout_headers.h"
#include "../utils/Time.hpp"
#include "../devices/FileDevice.hpp"
#include "../common/common.h"
#include "../utils/Version.hpp"
#include "../storage/storage_errors.hpp"
#include "../common/offset_type_traits.hpp"
#include "../storage/IStorage.hpp"
#include "../exceptions/YASExceptionHandler.h"
#include "../physical_volume/PVDeviceDataReaderWriter.hpp"
#include "../inverted_index_helper/InvertedIndexHelper.hpp"
#include "../physical_volume/PVEntriesManager.hpp"
#include <mutex>

using namespace yas::pv_layout_headers;

namespace yas {
namespace storage {

template<typename OffsetType>
using DefaultDevice = devices::FileDevice<OffsetType>;

// this class represent 
template <typename CharType=CharType, typename OffsetType=OffsetType, typename Device=DefaultDevice<OffsetType>>
class PVManager : public IStorage<CharType> {
  using InvertedIndexType = index_helper::InvertedIndexHelper<CharType, OffsetType>;
  using PVEntriesManagerType = pv::PVEntriesManager<OffsetType, Device>;
public:
  using pv_manager_type = PVManager<CharType, OffsetType, Device>;
  using key_type = IStorage<CharType>::key_type;

  virtual ~PVManager() {
    close();
  }

  static std::unique_ptr<pv_manager_type> Load(fs::path &file_path, utils::Version version) {
    auto pv_volume_manager = std::unique_ptr<pv_manager_type>(new pv_manager_type(file_path, version));

    pv_volume_manager->inverted_index_offset_ = pv_volume_manager->entries_manager_.LoadStartSections();
    const auto serialized_index = pv_volume_manager->entries_manager_.GetEntryContent(pv_volume_manager->inverted_index_offset_);
    const auto &vector_serialized_index = std::any_cast<const ByteVector &>(serialized_index);
    auto indexer = InvertedIndexType::Deserialize<InvertedIndexType::leaf_type>(std::cbegin(vector_serialized_index),
        std::cend(vector_serialized_index), version);

    pv_volume_manager->inverted_index_ = std::move(indexer);
    return pv_volume_manager;
  }

  static std::unique_ptr<pv_manager_type> Create(fs::path &file_path, utils::Version version,
      uint32_t priority, uint32_t cluster_size = kDefaultClusterSize) {
    const OffsetType device_end = (sizeof(PVHeader) + sizeof(FreelistHeader<OffsetType>));
    if (fs::exists(file_path)) {
      fs::resize_file(file_path, device_end);
    }
    else {
      std::ofstream out(file_path, std::ios_base::out);
    }

    // std::make_unique needs access to the class ctor
    auto pv_volume_manager = std::unique_ptr<pv_manager_type>(new pv_manager_type(file_path, version, cluster_size));
    pv_volume_manager->entries_manager_.CreateStartSections(offset_traits<OffsetType>::NonExistValue());
    pv_volume_manager->inverted_index_.reset(new InvertedIndexType());
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

      const auto offset = entries_manager_.CreateNewEntryValue(value);
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
      return entries_manager_.GetEntryContent(entry_offset);
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
      entries_manager_.DeleteEntry(entry_offset);
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
      entries_manager_.SetEntryExpiredDate(entry_offset, expired_time);
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
      const auto expired_date = entries_manager_.GetEntryExpiredDate(entry_offset);
      return expired_date.GetTime();
    }
    catch (...) {
      return nonstd::make_unexpected(exception::YASExceptionHandler(std::current_exception()));
    }
  }

  uint32_t priority() const { return entries_manager_.priority(); }

#ifdef UNIT_TEST
  Device& GetDevice() const { return device_; }
#endif

  PVManager(const PVManager&) = delete;
  PVManager(PVManager&&) = default;
  PVManager& operator=(const PVManager&) = delete;
  PVManager& operator=(PVManager&&) = delete;

 private:
  std::unique_ptr<InvertedIndexType> inverted_index_;
  OffsetType inverted_index_offset_;
  PVEntriesManagerType entries_manager_;
  mutable std::mutex manager_guard_mutex_;        // std::lock_guard recieves the argument by non-const ref
  utils::Version version_;

  explicit PVManager(fs::path &file_path, utils::Version version, uint32_t cluster_size = kDefaultClusterSize)
      : entries_manager_(file_path, version, cluster_size),
        version_(version) {
  }

  void close() {
    // TODO : exception during serailization 
    const auto serialized_index = inverted_index_->Serialize<OffsetType>(version_);
    if (offset_traits<OffsetType>::IsExistValue(inverted_index_offset_)) {
      entries_manager_.DeleteEntry(inverted_index_offset_);
    }
    const auto new_index_offset = entries_manager_.CreateNewEntryValue(serialized_index);
    entries_manager_.CreateStartSections(new_index_offset);
  }

};

} // namespace device_worker
} // namespace yas
