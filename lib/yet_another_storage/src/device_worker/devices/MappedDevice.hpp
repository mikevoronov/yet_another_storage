#pragma once

#pragma once

#include "IDevice.hpp"
#include "../../external/filesystem.h"
#include "../../external/expected.h"
//#include <boost/interprocess/file_mapping.hpp>

#include <cstdint>
#include <vector>
#include <fstream>
#include <filesystem>

namespace yas {
namespace devices {

class MappedDevice : public IDevice {
 public:
  virtual nonstd::expected<std::vector<uint8_t>, DeviceErrorCode> Read(uint64_t position, uint64_t data_size) override;
  virtual nonstd::expected<uint64_t, DeviceErrorCode> Write(uint64_t position, std::vector<uint8_t> &data) override;
  virtual bool IsOpen() const override;
  virtual bool Close() override;

  MappedDevice(fs::path path);
  virtual ~MappedDevice() override;
  MappedDevice(const MappedDevice&);

  // compiler shouldn't add these 3 but just in case
  MappedDevice operator=(const MappedDevice&) = delete;
  MappedDevice operator=(MappedDevice&&) = delete;
  MappedDevice(MappedDevice&&) = delete;

 protected:
  virtual void Open() override;

 private:
//  mutable boost::interprocess::file_mapping device_;     // IsOpen should be "logically" const
  fs::path path_;
  uintptr_t last_address_;
  uintptr_t last_size_;
};

} // namespace devices

} // namespace yas_fs
