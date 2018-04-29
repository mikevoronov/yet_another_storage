#pragma once
#include "PVDeviceWorker.hpp"
#include "devices/FileDevice.hpp"
#include <memory>
#include <string>
#include <mutex>

namespace yas {
namespace pv_manager {

// just simple factory
template <typename CharType>
class PVManager {
  using Path = std::basic_string<CharType>;
 public:
  PVManager() = default;
  ~PVManager() = default;

  template <typename ValueType>
  nonstd::expected<bool, StorageErrorDescriptor> Put(std::string_view key, ValueType value);

  template <typename ValueType>
  nonstd::expected<ValueType, StorageErrorDescriptor> Get(std::string_view key) const;

  bool HasKey(std::string_view key) const noexcept;

  template <typename ValueType>
  nonstd::expected<bool, StorageErrorDescriptor> Delete(std::string_view key);


  PVManager(const PVManager&) = delete;
  PVManager(PVManager&&) = delete;
  PVManager operator=(const PVManager&) = delete;

 private:
  device_worker::PVDeviceWorker<devices::FileDevice<OffsetType>, OffsetType> device_wroker_;
  // represent the catalog name on which this device mounted - this would be used then in inverted index
  Path mount_point; 
  std::mutex manager_guard_;
};

}
} // namespace fs
