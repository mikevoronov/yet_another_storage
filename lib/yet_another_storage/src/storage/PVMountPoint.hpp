#pragma once
#include "PVManager.hpp"
#include "IStorage.hpp"
#include <memory>
#include <string>

namespace yas {
namespace storage {

template <typename CharType, typename OffsetType, typename Device = DefaultDevice<OffsetType>>
class PVMountPointManagerAdapter : public IStorage<CharType> {
  using Path = std::basic_string<CharType>;
  using VolumeManager = PVManager<CharType, OffsetType, Device>;

 public:
  using key_type = IStorage<CharType>::key_type;

  PVMountPointManagerAdapter(Path mount_point, std::shared_ptr<VolumeManager> volume_manager)
      : mount_point_(std::move(mount_point)),
        volume_manager_(volume_manager)
  {}

  ~PVMountPointManager() = default;

  virtual nonstd::expected<bool, StorageErrorDescriptor> Put(key_type key, std::any value) ovveride {
    if (!volume_manager_) {
      return nonstd::make_unexpected({"PVMountPointManagerAdapter: manager isn't ready", 
          kPVMountPointManagerAdapterNotReady });
    }

    return volume_manager_->Put(adjust(key), value);
  }

  virtual nonstd::expected<std::any, StorageErrorDescriptor> Get(key_type key) const ovveride {
    if (!volume_manager_) {
      return nonstd::make_unexpected({ "PVMountPointManagerAdapter: manager isn't ready",
          kPVMountPointManagerAdapterNotReady });
    }

    return volume_manager_->Get(adjust(key));
  }

  virtual nonstd::expected<bool, StorageErrorDescriptor> HasKey(key_type key) const ovveride {
    if (!volume_manager_) {
      return nonstd::make_unexpected({ "PVMountPointManagerAdapter: manager isn't ready",
          kPVMountPointManagerAdapterNotReady });
    }

    return volume_manager_->HasKey(adjust(key));
  }

  virtual nonstd::expected<bool, StorageErrorDescriptor> Delete(key_type key) ovveride {
    if (!volume_manager_) {
      return nonstd::make_unexpected({ "PVMountPointManagerAdapter: manager isn't ready",
          kPVMountPointManagerAdapterNotReady });
    }

    return volume_manager_->Delete(adjust(key));
  }

  virtual nonstd::expected<bool, StorageErrorDescriptor> SetExpiredDate(key_type key, time_t expired) ovveride {
    if (!volume_manager_) {
      return nonstd::make_unexpected({ "PVMountPointManagerAdapter: manager isn't ready",
          kPVMountPointManagerAdapterNotReady });
    }

    return volume_manager_->SetExpiredDate(adjust(key), expired);
  }

  virtual nonstd::expected<time_t, StorageErrorDescriptor> GetExpiredDate(key_type key) ovveride {
    if (!volume_manager_) {
      return nonstd::make_unexpected({ "PVMountPointManagerAdapter: manager isn't ready",
          kPVMountPointManagerAdapterNotReady });
    }

    return volume_manager_->GetExpiredDate(adjust(key));
  }

  PVMountPointManagerAdapter(const PVMountPointManagerAdapter&) = delete;
  PVMountPointManagerAdapter(PVMountPointManagerAdapter&&) = delete;
  PVMountPointManagerAdapter& operator=(const PVMountPointManagerAdapter&) = delete;
  PVMountPointManagerAdapter& operator=(PVMountPointManagerAdapter&&) = delete;

 private:
  std::shared_ptr<PVVolumeManager> volume_manager_;
  // represent the catalog name on which this device mounted 
  Path mount_point_; 

  Path adjust(const Path &current) {
    return mount_point_ + current;
  }
};

} // namespace storage
} // namespace fs
