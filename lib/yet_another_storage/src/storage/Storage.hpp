#pragma once
#include "PVManagerFactory.hpp"
#include "IStorage.hpp"
#include "../inverted_index_helper/InvertedIndexHelper.hpp"
#include <shared_mutex>

using namespace yas::index_helper;

namespace yas {
namespace storage {

// should be class with basic exception safety garantee
// all methods should catch inner class exception and transform it to StorageError class wrapper to nonstd::expected
template<typename CharType>
class Storage : public IStorage<CharType> {
  using StringType = std::basic_string<CharType>;
 public:
  using pv_path_type = PVManagerFactory::pv_path_type;
  using key_type = typename IStorage<CharType>::key_type;
  
  Storage() = default;
  ~Storage() = default;

  virtual StorageErrorDescriptor Put(key_type key, std::any value) {
    std::shared_lock<std::shared_mutex> lock(mutex_);

    const auto max_subkey_length = virtual_storage_index_.FindMaxSubKey(key);
    StringType storage_key(key.substr(0, max_subkey_length));
    const auto volume_group_id = virtual_storage_index_.Get(storage_key);
    
    if (!leaf_traits<uint32_t>::IsExistValue(volume_group_id)) {
      return { "Storage: there aren't any physical volume corresponds to specified path", 
          StorageError::kCatalogNotFoundError };
    }
    const auto &volume_group = virtual_storage_[volume_group_id];

    StringType catalog_key(key.substr(max_subkey_length));
    for (auto it = std::crbegin(volume_group), end = std::crend(volume_group); it != end; ++it) {
      const auto new_key = it->mount_catalog_ + catalog_key;
      if (StorageError::kSuccess == it->pv_manager_->Put(new_key, std::move(value)).error_code_) {
        return { "", StorageError::kSuccess };
      }
    }

    return { "", StorageError::kKeyNotFound };
  }

  virtual nonstd::expected<std::any, StorageErrorDescriptor> Get(key_type key) {
    std::shared_lock<std::shared_mutex> lock(mutex_);

    const auto max_subkey_length = virtual_storage_index_.FindMaxSubKey(key);
    StringType storage_key(key.substr(0, max_subkey_length));
    const auto volume_group_id = virtual_storage_index_.Get(storage_key);

    if (!leaf_traits<uint32_t>::IsExistValue(volume_group_id)) {
      return nonstd::make_unexpected(StorageErrorDescriptor( "Storage: there aren't any physical volume corresponds to\
        specified path", StorageError::kCatalogNotFoundError));
    }
    const auto &volume_group = virtual_storage_[volume_group_id];

    StringType catalog_key(key.substr(max_subkey_length));
    for (auto it = std::crbegin(volume_group), end = std::crend(volume_group); it != end; ++it) {
      const auto new_key = it->mount_catalog_ + catalog_key;
      const auto result = it->pv_manager_->Get(new_key);
      if (result.has_value()) {
        return result.value();
      }
    }
    return nonstd::make_unexpected(StorageErrorDescriptor("", StorageError::kKeyNotFound));
 }

  virtual StorageErrorDescriptor HasKey(key_type key) {
    std::shared_lock<std::shared_mutex> lock(mutex_);

    const auto max_subkey_length = virtual_storage_index_.FindMaxSubKey(key);
    StringType storage_key(key.substr(0, max_subkey_length));
    const auto volume_group_id = virtual_storage_index_.Get(storage_key);

    if (!leaf_traits<uint32_t>::IsExistValue(volume_group_id)) {
      return { "Storage: there aren't any physical volume corresponds to specified path",
        StorageError::kCatalogNotFoundError };
    }
    const auto &volume_group = virtual_storage_[volume_group_id];

    StringType catalog_key(key.substr(max_subkey_length));
    for (auto it = std::crbegin(volume_group), end = std::crend(volume_group); it != end; ++it) {
      const auto new_key = it->mount_catalog_ + catalog_key;
      if (StorageError::kSuccess == it->pv_manager_->HasKey(new_key).error_code_) {
        return { "", StorageError::kSuccess };
      }
    }

    return { "", StorageError::kKeyNotFound };
  }

  virtual StorageErrorDescriptor HasCatalog(key_type key) {
    std::shared_lock<std::shared_mutex> lock(mutex_);

    const auto max_subkey_length = virtual_storage_index_.FindMaxSubKey(key);
    StringType storage_key(key.substr(0, max_subkey_length));
    const auto volume_group_id = virtual_storage_index_.Get(storage_key);

    if (!leaf_traits<uint32_t>::IsExistValue(volume_group_id)) {
      return { "Storage: there aren't any physical volume corresponds to specified path",
        StorageError::kCatalogNotFoundError };
    }
    const auto &volume_group = virtual_storage_[volume_group_id];

    StringType catalog_key(key.substr(max_subkey_length));
    for (auto it = std::crbegin(volume_group), end = std::crend(volume_group); it != end; ++it) {
      const auto new_key = it->mount_catalog_ + catalog_key;
      if (StorageError::kSuccess == it->pv_manager_->HasCatalog(new_key).error_code_) {
        return { "", StorageError::kSuccess };
      }
    }

    return { "", StorageError::kKeyNotFound };
  }

  virtual StorageErrorDescriptor Delete(key_type key) {
    std::shared_lock<std::shared_mutex> lock(mutex_);

    const auto max_subkey_length = virtual_storage_index_.FindMaxSubKey(key);
    StringType storage_key(key.substr(0, max_subkey_length));
    const auto volume_group_id = virtual_storage_index_.Get(storage_key);

    if (!leaf_traits<uint32_t>::IsExistValue(volume_group_id)) {
      return { "Storage: there aren't any physical volume corresponds to specified path",
        StorageError::kCatalogNotFoundError };
    }
    const auto &volume_group = virtual_storage_[volume_group_id];

    StringType catalog_key(key.substr(max_subkey_length));
    for (auto it = std::crbegin(volume_group), end = std::crend(volume_group); it != end; ++it) {
      const auto new_key = it->mount_catalog_ + catalog_key;
      if (StorageError::kSuccess == it->pv_manager_->Delete(new_key).error_code_) {
        return { "", StorageError::kSuccess };
      }
    }

    return { "", StorageError::kKeyNotFound };
  }

  virtual StorageErrorDescriptor SetExpiredDate(key_type key, time_t expired) {
    std::shared_lock<std::shared_mutex> lock(mutex_);

    const auto max_subkey_length = virtual_storage_index_.FindMaxSubKey(key);
    StringType storage_key(key.substr(0, max_subkey_length));
    const auto volume_group_id = virtual_storage_index_.Get(storage_key);

    if (!leaf_traits<uint32_t>::IsExistValue(volume_group_id)) {
      return { "Storage: there aren't any physical volume corresponds to specified path",
        StorageError::kCatalogNotFoundError };
    }
    const auto &volume_group = virtual_storage_[volume_group_id];

    StringType catalog_key(key.substr(max_subkey_length));
    for (auto it = std::crbegin(volume_group), end = std::crend(volume_group); it != end; ++it) {
      const auto new_key = it->mount_catalog_ + catalog_key;
      if (StorageError::kSuccess == it->pv_manager_->SetExpiredDate(new_key, expired).error_code_) {
        return { "", StorageError::kSuccess };
      }
    }

    return { "", StorageError::kKeyNotFound };
  }

  virtual nonstd::expected<time_t, StorageErrorDescriptor> GetExpiredDate(key_type key) {
    std::shared_lock<std::shared_mutex> lock(mutex_);

    const auto max_subkey_length = virtual_storage_index_.FindMaxSubKey(key);
    StringType storage_key(key.substr(0, max_subkey_length));
    const auto volume_group_id = virtual_storage_index_.Get(storage_key);

    if (!leaf_traits<uint32_t>::IsExistValue(volume_group_id)) {
      return nonstd::make_unexpected(StorageErrorDescriptor("Storage: there aren't any physical volume corresponds to\
        specified path", StorageError::kCatalogNotFoundError));
    }
    const auto &volume_group = virtual_storage_[volume_group_id];

    StringType catalog_key(key.substr(max_subkey_length));
    for (auto it = std::crbegin(volume_group), end = std::crend(volume_group); it != end; ++it) {
      const auto new_key = it->mount_catalog_ + catalog_key;
      const auto result = it->pv_manager_->GetExpiredDate(new_key);
      if (result.has_value()) {
        return result.value();
      }
    }
    return nonstd::make_unexpected(StorageErrorDescriptor("", StorageError::kKeyNotFound));
  }


  StorageErrorDescriptor Mount(pv_path_type pv_path, StringType storage_mount_catalog, StringType pv_mount_catalog) {
    std::unique_lock<std::shared_mutex> lock(mutex_);

    auto &factory = storage::PVManagerFactory::Instance();
    auto manager = factory.Create(pv_path, kMaximumSupportedVersion);
    if (!manager) {
      return manager.error();
    }
    auto pv_manager = manager.value();
    if (StorageError::kSuccess != pv_manager->HasCatalog(pv_mount_catalog).error_code_) {
      return {"Specified PV doesn't have such catalog", StorageError::kCatalogNotFoundError };
    }

    const auto priority = pv_manager->priority();
    PVMountPoint mount_point(std::move(pv_manager), std::move(pv_mount_catalog), priority);
    return addNewMountPoint(mount_point, storage_mount_catalog);
  }

  Storage(const Storage&) = delete;
  Storage(Storage &&) = delete;
  Storage& operator=(const Storage&) = delete;
  Storage& operator=(Storage&&) = delete;

 private:
  struct PVMountPoint {
    PVMountPoint(std::shared_ptr<PVManagerFactory::manager_type> pv_manager, std::basic_string<CharType> 
        mount_catalog, uint32_t priority)
        : pv_manager_(std::move(pv_manager)),
          mount_catalog_(mount_catalog),
          priority_(priority)
    {}

    std::shared_ptr<PVManagerFactory::manager_type> pv_manager_;
    StringType mount_catalog_;
    uint32_t priority_;
  };
  using VolumeGroup = std::vector<PVMountPoint>;

  std::vector<VolumeGroup> virtual_storage_;
  index_helper::InvertedIndexHelper<CharType, uint32_t> virtual_storage_index_;
  std::shared_mutex mutex_;     // we have expensive and frequent "read" operation -> r/w lock that's we need

  StorageErrorDescriptor addNewMountPoint(const PVMountPoint &mount_point, StringType &storage_mount_catalog) {
    const auto volume_group_id = virtual_storage_index_.Get(storage_mount_catalog);
    if (!leaf_traits<uint32_t>::IsExistValue(volume_group_id)) {
      virtual_storage_index_.Insert(storage_mount_catalog, static_cast<uint32_t>(virtual_storage_.size()));
      virtual_storage_.push_back({ mount_point });
      return { "", StorageError::kSuccess };
    }

    if (volume_group_id > virtual_storage_.size()) {
      return { "Index is diverse with storage", StorageError::kUnknownError };
    }
    
    auto &volume_group = virtual_storage_[volume_group_id];
    auto insert_place = std::lower_bound(std::cbegin(volume_group), std::cend(volume_group), mount_point.priority_, [](
        const typename VolumeGroup::value_type &left,
        const uint32_t priority) {
      return left.priority_ < priority;
    });
    if (std::cend(volume_group) == insert_place) {
      volume_group.push_back(mount_point);
      return {"", StorageError::kSuccess};
    }
    volume_group.insert(insert_place, mount_point);

    return { "", StorageError::kSuccess };
  }
};

} // namespace storage
} // namespace yas
