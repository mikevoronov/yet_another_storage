#pragma once
#include "../device_worker/PVDeviceWorker.hpp"
#include "../device_worker/devices/FileDevice.hpp"
#include "../common/settings.hpp"

namespace yas {
namespace storage {

template<typename CharType>
class PVMountPointManager {
 public:
  PVMountPointManager() = delete;
  ~PVMountPointManager() = delete;
  PVMountPointManager(const PVMountPointManager&) = delete;
  PVMountPointManager(PVMountPointManager&&) = delete;
  PVMountPointManager operator=(const PVMountPointManager&) = delete;

private:
  device_worker::PVDeviceWorker<devices::FileDevice<OffsetType>, OffsetType> device_worker_;
};

} // namespace storage
} // namespace yas
