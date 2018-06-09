#pragma once
#include "PVManagerFactory.hpp"
#include "IStorage.hpp"
#include "lib/inverted_index/InvertedIndexHelper.hpp"
#include <shared_mutex>

using namespace yas::index_helper;

namespace yas {
namespace storage {

/**
*    \brief The class represents virtual volume of key-value storage with possbility to mount physical volumes
*
*    This class is thread-safe with read-write blocks on shared_mutex. "Read" operation is all from IStorage
*    interface, "write" is only mount operation which modifies this class inner structures. Take into account
*    that this based on assumptions that PVManager is also thead-safe.
*/
class Storage : public IStorage<DCharType> {
  using CharType = DCharType;
  using StringType = std::basic_string<CharType>;

 public:
  using pv_path_type = PVManagerFactory::pv_path_type;
  using key_type = typename IStorage<CharType>::key_type;

  Storage() = default;
  virtual ~Storage() = default;

  StorageErrorDescriptor Put(key_type key, storage_value_type value) noexcept override {
    std::shared_lock<std::shared_mutex> lock(mutex_);

    const auto vg_range = getVolumeGroupRange(key);
    if (!vg_range.has_value()) {
      return vg_range.error();
    }

    StringType catalog_key(key.substr(virtual_storage_index_.FindMaxSubKey(key)));
    for (auto &&it : vg_range.value()) {
      const auto adjusted_key = it.mount_catalog_ + catalog_key;
      if (StorageError::kSuccess == it.pv_manager_->Put(adjusted_key, std::move(value)).error_code_) {
        return { std::string(), StorageError::kSuccess };
      }
    }

    return { std::string(), StorageError::kKeyNotFound };
  }

  nonstd::expected<storage_value_type, StorageErrorDescriptor> Get(key_type key) noexcept override {
    std::shared_lock<std::shared_mutex> lock(mutex_);

    const auto vg_range = getVolumeGroupRange(key);
    if (!vg_range.has_value()) {
      return nonstd::make_unexpected(std::move(vg_range.error()));
    }

    StringType catalog_key(key.substr(virtual_storage_index_.FindMaxSubKey(key)));
    for (auto &&it : vg_range.value()) {
      const auto adjusted_key = it.mount_catalog_ + catalog_key;
      const auto result = it.pv_manager_->Get(adjusted_key);
      if (result.has_value()) {
        return result.value();
      }
    }
    return nonstd::make_unexpected(StorageErrorDescriptor(std::string(), StorageError::kKeyNotFound));
 }

  StorageErrorDescriptor HasKey(key_type key) noexcept override {
    std::shared_lock<std::shared_mutex> lock(mutex_);

    const auto vg_range = getVolumeGroupRange(key);
    if (!vg_range.has_value()) {
      return vg_range.error();
    }

    StringType catalog_key(key.substr(virtual_storage_index_.FindMaxSubKey(key)));
    for (auto &&it : vg_range.value()) {
      const auto adjusted_key = it.mount_catalog_ + catalog_key;
      if (StorageError::kSuccess == it.pv_manager_->HasKey(adjusted_key).error_code_) {
        return { std::string(), StorageError::kSuccess };
      }
    }

    return { std::string(), StorageError::kKeyNotFound };
  }

  StorageErrorDescriptor HasCatalog(key_type key) noexcept override {
    std::shared_lock<std::shared_mutex> lock(mutex_);

    const auto vg_range = getVolumeGroupRange(key);
    if (!vg_range.has_value()) {
      return vg_range.error();
    }

    StringType catalog_key(key.substr(virtual_storage_index_.FindMaxSubKey(key)));
    for (auto &&it : vg_range.value()) {
      const auto adjusted_key = it.mount_catalog_ + catalog_key;
      if (StorageError::kSuccess == it.pv_manager_->HasCatalog(adjusted_key).error_code_) {
        return { std::string(), StorageError::kSuccess };
      }
    }

    return { std::string(), StorageError::kKeyNotFound };
  }

  StorageErrorDescriptor Delete(key_type key) noexcept override {
    std::shared_lock<std::shared_mutex> lock(mutex_);

    const auto vg_range = getVolumeGroupRange(key);
    if (!vg_range.has_value()) {
      return vg_range.error();
    }

    StringType catalog_key(key.substr(virtual_storage_index_.FindMaxSubKey(key)));
    for (auto &&it : vg_range.value()) {
      const auto adjusted_key = it.mount_catalog_ + catalog_key;
      if (StorageError::kSuccess == it.pv_manager_->Delete(adjusted_key).error_code_) {
        return { std::string(), StorageError::kSuccess };
      }
    }

    return { std::string(), StorageError::kKeyNotFound };
  }

  StorageErrorDescriptor SetExpiredDate(key_type key, time_t expired) noexcept override {
    std::shared_lock<std::shared_mutex> lock(mutex_);

    const auto vg_range = getVolumeGroupRange(key);
    if (!vg_range.has_value()) {
      return vg_range.error();
    }

    StringType catalog_key(key.substr(virtual_storage_index_.FindMaxSubKey(key)));
    for (auto &&it : vg_range.value()) {
      const auto adjusted_key = it.mount_catalog_ + catalog_key;
      if (StorageError::kSuccess == it.pv_manager_->SetExpiredDate(adjusted_key, expired).error_code_) {
        return { std::string(), StorageError::kSuccess };
      }
    }

    return { std::string(), StorageError::kKeyNotFound };
  }

  nonstd::expected<time_t, StorageErrorDescriptor> GetExpiredDate(key_type key) noexcept override {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    const auto vg_range = getVolumeGroupRange(key);
    if (!vg_range.has_value()) {
      return nonstd::make_unexpected(std::move(vg_range.error()));
    }

    StringType catalog_key(key.substr(virtual_storage_index_.FindMaxSubKey(key)));
    for (auto &&it : vg_range.value()) {
      const auto adjusted_key = it.mount_catalog_ + catalog_key;
      const auto result = it.pv_manager_->GetExpiredDate(adjusted_key);
      if (result.has_value()) {
        return result.value();
      }
    }
    return nonstd::make_unexpected(StorageErrorDescriptor(std::string(), StorageError::kKeyNotFound));
  }

  ///  \brief mounts the specified catalog of PVManager to specified location in virtual volume
  ///
  ///  \param pv_path - path to PVManager. This method gets PVManager instances from PVManagerFactory by pv_path.
  ///  \param storage_mount_catalog - the catalog of virtual storage that would be used for PV mounting.
  ///  \param pv_mount_catalog - the catalog of mounted PV that would be mounted to the virtual storage
  ///  \return - descriptor of error
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

  struct VGReversedRange {
    using Iterator = typename VolumeGroup::reverse_iterator;
    Iterator begin_;
    Iterator end_;
    Iterator begin() noexcept { return begin_; }
    Iterator end() noexcept { return end_; }
    const Iterator begin() const noexcept { return begin_; }
    const Iterator end() const noexcept { return end_; }

    VGReversedRange(VolumeGroup &container)
        : begin_(std::rbegin(container)),
          end_(std::rend(container))
    {}
  };


  std::vector<VolumeGroup> virtual_storage_;
  index_helper::InvertedIndexHelper<CharType, uint32_t> virtual_storage_index_;
  std::shared_mutex mutex_;     // we have expensive and frequent "read" operation -> r/w lock that's we need

  nonstd::expected<VGReversedRange, StorageErrorDescriptor> getVolumeGroupRange(key_type key) {
    const auto max_subkey_length = virtual_storage_index_.FindMaxSubKey(key);
    StringType storage_key(key.substr(0, max_subkey_length));
    const auto volume_group_id = virtual_storage_index_.Get(storage_key);

    if (!leaf_traits<uint32_t>::IsExistValue(volume_group_id)) {
      return nonstd::make_unexpected(StorageErrorDescriptor("Storage: there aren't any physical volume corresponds to\
          specified path", StorageError::kCatalogNotFoundError));
    }

    return VGReversedRange(virtual_storage_[volume_group_id]);
  }

  StorageErrorDescriptor addNewMountPoint(const PVMountPoint &mount_point, StringType &storage_mount_catalog) {
    const auto volume_group_id = virtual_storage_index_.Get(storage_mount_catalog);
    if (!leaf_traits<uint32_t>::IsExistValue(volume_group_id)) {
      virtual_storage_index_.Insert(storage_mount_catalog, static_cast<uint32_t>(virtual_storage_.size()));
      virtual_storage_.push_back({ mount_point });
      return { std::string(), StorageError::kSuccess };
    }

    if (volume_group_id > virtual_storage_.size()) {
      return { "Index is diversed with storage", StorageError::kUnknownError };
    }
    
    auto &volume_group = virtual_storage_[volume_group_id];
    auto insert_place = std::lower_bound(std::cbegin(volume_group), std::cend(volume_group), mount_point.priority_, [](
        const typename VolumeGroup::value_type &left,
        const uint32_t priority) {
      return left.priority_ < priority;
    });
    if (std::cend(volume_group) == insert_place) {
      volume_group.push_back(mount_point);
      return { std::string(), StorageError::kSuccess};
    }
    volume_group.insert(insert_place, mount_point);

    return { std::string(), StorageError::kSuccess };
  }
};

} // namespace storage
} // namespace yas
