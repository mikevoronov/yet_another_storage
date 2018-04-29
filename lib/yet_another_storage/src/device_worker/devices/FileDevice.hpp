#pragma once
#include "../../external/filesystem.h"
#include "../../external/expected.h"
#include "../../common/common.h"
#include <cstdint>
#include <fstream>
#include <filesystem>

namespace yas {
namespace devices {

template <typename OffsetType>
class FileDevice {
 public:
  FileDevice(fs::path path);
  ~FileDevice();
  FileDevice(const FileDevice&);
   
  ByteVector Read(OffsetType position, uint64_t data_size);
  uint64_t Write(OffsetType position, std::vector<uint8_t> &data);
  bool IsOpen() const;
  bool Close();

  FileDevice operator=(const FileDevice&) = delete;
  FileDevice operator=(FileDevice&&) = delete;
  FileDevice(FileDevice&&) = delete;

 protected:
  void Open();

 private:
  mutable std::fstream device_;     // IsOpen should be "logically" const
  fs::path path_;
};

} // namespace devices
} // namespace yas
