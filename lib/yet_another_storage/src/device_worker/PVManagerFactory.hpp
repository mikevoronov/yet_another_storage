#pragma once

#include "PVManager.hpp"
#include "../external/filesystem.h"
#include <mutex>
#include <memory>

namespace yas_fs {

// factory 
class PhysicalVolumeWorkerFactory {
 public:
   static std::unique_ptr<IPhysicalVolumeWorker> Create(physical_volume_version version) {
     switch(version.major)
     {
     case 1:
       return std::make_unique<PhysicalVolumeWorkerVersion_1_1>();
     }
  }

  PhysicalVolumeWorkerFactory() = delete;
  ~PhysicalVolumeWorkerFactory() = delete;
  PhysicalVolumeWorkerFactory(const PhysicalVolumeWorkerFactory&) = delete;
  PhysicalVolumeWorkerFactory(PhysicalVolumeWorkerFactory&&) = delete;
  PhysicalVolumeWorkerFactory operator=(const PhysicalVolumeWorkerFactory&) = delete;
};

} // namespace yas_fs
