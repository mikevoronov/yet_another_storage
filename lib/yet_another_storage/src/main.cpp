#include "storage/PVVolumeManager.hpp"
#include "device_worker/devices/FileDevice.hpp"
#include "common/filesystem.h"

using namespace yas;

int main() {
  fs::path pv1_path = "D:/additional_activity/jetbrains_testtask/result/pv1";

  device_worker::PVVolumeManager<char, uint64_t, devices::FileDevice<uint64_t>>::Create(pv1_path, utils::Version(1,1), 5);
  device_worker::PVVolumeManager<char, uint64_t, devices::FileDevice<uint64_t>>::Load(pv1_path, utils::Version(1, 1));

  return 0;
}
