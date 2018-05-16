#pragma once
#include "../exceptions/YASExceptionHandler.h"
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
   PVMountPointManagerAdapter(Path mount_point)
      : mount_point_(std::move(mount_point))
  {}
  ~PVMountPointManager() = default;

  template <typename ValueType>
  nonstd::expected<bool, StorageErrorDescriptor> Put(const Path &key, ValueType value) {
    std::lock_guard<std::mutex> lock(manager_guard_);
    try {
      return manager_.Put(adjust(key), value);
    }
    catch (const std::exception &exception) {
      return nonstd::make_unexpected(exception::YASExceptionHandler(exception));
    }
  }

  template <typename ValueType>
  nonstd::expected<ValueType, StorageErrorDescriptor> Get(const Path &key) const {
    std::lock_guard<std::mutex> lock(manager_guard_);
    try {
      return manager_.Get(adjust(key));
    }
    catch (const std::exception &exception) {
      return nonstd::make_unexpected(exception::YASExceptionHandler(exception));
    }
  }

  bool HasKey(const Path &key) const noexcept {
    std::lock_guard<std::mutex> lock(manager_guard_);
    try {
      return manager_.HasKey(adjust(key));
    }
    catch (const std::exception &exception) {
      return nonstd::make_unexpected(exception::YASExceptionHandler(exception));
    }
  }

  template <typename ValueType>
  nonstd::expected<bool, StorageErrorDescriptor> Delete(const Path &key) {
    std::lock_guard<std::mutex> lock(manager_guard_);
    try {
      return manager_.Get(adjust(key));
    }
    catch (const std::exception &exception) {
      return nonstd::make_unexpected(exception::YASExceptionHandler(exception));
    }
  }

  PVMountPointManagerAdapter(const PVMountPointManagerAdapter&) = delete;
  PVMountPointManagerAdapter(PVMountPointManagerAdapter&&) = delete;
  PVMountPointManagerAdapter operator=(const PVMountPointManagerAdapter&) = delete;

 private:
  PVMountPointManager<CharType> manager_;
  // represent the catalog name on which this device mounted - this would be used then in inverted index
  Path mount_point_; 
  std::mutex manager_guard_;

  Path adjust(const Path &current) {
    return mount_point_ + current;
  }
};

} // namespace storage
} // namespace fs
