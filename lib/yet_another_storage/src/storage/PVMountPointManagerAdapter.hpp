#pragma once
#include "../common/settings.hpp"
#include "PVMountPointManager.hpp"
#include <memory>
#include <string>
#include <mutex>

namespace yas {
namespace storage {

template <typename CharType>
class PVMountPointManagerAdapter {
  using Path = std::basic_string<CharType>;
 public:
  PVMountPointManager(Path mount_point) 
      : mount_point_(std::move(mount_point))
  {}
  ~PVMountPointManager() = default;

  template <typename ValueType>
  nonstd::expected<bool, StorageErrorDescriptor> Put(std::string_view key, ValueType value) {
    std::lock_guard<std::mutex> lock(manager_guard_);
    try {
    }
    catch (const std::exception &exception) {

    }
  }

  template <typename ValueType>
  nonstd::expected<ValueType, StorageErrorDescriptor> Get(std::string_view key) const;

  bool HasKey(std::string_view key) const noexcept;

  template <typename ValueType>
  nonstd::expected<bool, StorageErrorDescriptor> Delete(std::string_view key);


  PVMountPointManager(const PVMountPointManager&) = delete;
  PVMountPointManager(PVMountPointManager&&) = delete;
  PVMountPointManager operator=(const PVMountPointManager&) = delete;

 private:
  PVMountPointManager<CharType> manager_;
  // represent the catalog name on which this device mounted - this would be used then in inverted index
  Path mount_point_; 
  std::mutex manager_guard_;
};

}
} // namespace fs
