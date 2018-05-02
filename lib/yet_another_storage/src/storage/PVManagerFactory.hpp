#pragma once
#include "PVMountPointManagerAdapter.hpp"
#include "../utils/Version.hpp"
#include "../external/filesystem.h"
#include <mutex>
#include <memory>
#include <string>
#include <unordered_map>

namespace yas {
namespace storage {

  // factory 
class PVManagerFactory {
  using Manager = PVMountPointManagerAdapter<CharType>;

 public:
  static std::shared_ptr<Manager> Create(utils::Version version, const std::string &mount_point) {
    // can be spin lock on atomics
    std::lock_guard<std::mutex> lock(factory_mutex_);

    if (!managers_.count(mount_point)) {
      managers_[mount_point].reset(new Manager());
    }

    return managers_[mount_point];
  }

  PVManagerFactory() = delete;
  ~PVManagerFactory() = delete;
  PVManagerFactory(const PVManagerFactory&) = delete;
  PVManagerFactory(PVManagerFactory&&) = delete;
  PVManagerFactory operator=(const PVManagerFactory&) = delete;

 private:
  static std::unordered_map<std::string, std::shared_ptr<Manager>> managers_;
  static std::mutex factory_mutex_;
};

} // namespace pv_manager
} // namespace yas
