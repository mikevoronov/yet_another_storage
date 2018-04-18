#pragma once

#include "IDevice.hpp"
#include "../external/filesystem.h"

#include <cstdint>
#include <vector>
#include <fstream>
#include <filesystem>

namespace yas {
namespace devices {

class FileDevice : public IDevice {
 public:
  virtual nonstd::expected<std::vector<uint8_t>, DeviceErrorCode> Read(uint64_t position, uint64_t data_size) override;
  virtual nonstd::expected<uint64_t, DeviceErrorCode> Write(uint64_t position, std::vector<uint8_t> &data) override;
  virtual bool IsOpen() const override;
  virtual bool Close() override;

  FileDevice(fs::path path);
  virtual ~FileDevice() override;
  FileDevice(const FileDevice&);

  // compiler shouldn't add these 3 but just in case
  FileDevice operator=(const FileDevice&) = delete;
  FileDevice operator=(FileDevice&&) = delete;
  FileDevice(FileDevice&&) = delete;

 protected:
  virtual void Open() override;

 private:
  mutable std::fstream device_;     // IsOpen should be "logically" const
  fs::path path_;
};

} // namespace devices
} // namespace yas
