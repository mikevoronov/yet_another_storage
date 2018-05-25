#include "device_worker/PVDeviceDataReaderWriter.hpp"
#include "device_worker/devices/FileDevice.hpp"
#include "common/filesystem.h"

using namespace yas;

int main() {
  fs::path pv1_path = "D:/additional_activity/jetbrains_testtask/result/pv1";

  device_worker::PVDeviceDataReaderWriter<uint64_t, devices::FileDevice<uint64_t>>::Create(pv1_path, utils::Version(1,1), 5);


  return 0;
}
