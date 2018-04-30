#pragma once
#include "PVDeviceWorker.hpp"
#include "devices/FileDevice.hpp"
#include <memory>
#include <string>
#include <mutex>

namespace yas {
namespace pv_manager {

template <typename CharType>
class PVMountPointManager {
  using Path = std::basic_string<CharType>;
 public:
  PVMountPointManager() = default;
  ~PVMountPointManager() = default;

  template <typename ValueType>
  nonstd::expected<bool, StorageErrorDescriptor> Put(std::string_view key, ValueType value);

  template <typename ValueType>
  nonstd::expected<ValueType, StorageErrorDescriptor> Get(std::string_view key) const;

  bool HasKey(std::string_view key) const noexcept;

  template <typename ValueType>
  nonstd::expected<bool, StorageErrorDescriptor> Delete(std::string_view key);


  PVMountPointManager(const PVMountPointManager&) = delete;
  PVMountPointManager(PVMountPointManager&&) = delete;
  PVMountPointManager operator=(const PVMountPointManager&) = delete;

 private:
  device_worker::PVDeviceWorker<devices::FileDevice<OffsetType>, OffsetType> device_wroker_;
  // represent the catalog name on which this device mounted - this would be used then in inverted index
  Path mount_point; 
  std::mutex manager_guard_;
};

}
} // namespace fs
