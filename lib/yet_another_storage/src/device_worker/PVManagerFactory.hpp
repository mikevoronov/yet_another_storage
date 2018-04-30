#pragma once
#include "PVMountPointManager.hpp"
#include "../utils/Version.hpp"
#include "../external/filesystem.h"
#include <mutex>
#include <memory>
#include <string>
#include <unordered_map>

namespace yas {
namespace pv_manager {

  // factory 
class PhysicalVolumeWorkerFactory {
public:
  using Manager = std::shared_ptr<PVMountPointManager<char>>;

  // TODO - add 
  static std::shared_ptr<Manager> Create(utils::Version version, std::string mount_point) {
    // can be spin lock on atomics
    std::lock_guard<std::mutex> lock(factory_mutex_);

    if (!managers_.count(mount_point)) {
      managers_[mount_point].reset(new PVMountPointManager());
    }

    return managers_[mount_point];
  }

  PhysicalVolumeWorkerFactory() = delete;
  ~PhysicalVolumeWorkerFactory() = delete;
  PhysicalVolumeWorkerFactory(const PhysicalVolumeWorkerFactory&) = delete;
  PhysicalVolumeWorkerFactory(PhysicalVolumeWorkerFactory&&) = delete;
  PhysicalVolumeWorkerFactory operator=(const PhysicalVolumeWorkerFactory&) = delete;

private:
  static std::unordered_map<std::string, Manager> managers_;
  static std::mutex factory_mutex_;
};

} // namespace pv_manager
} // namespace yas
