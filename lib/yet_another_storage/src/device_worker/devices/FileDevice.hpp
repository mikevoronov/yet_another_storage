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
  FileDevice(fs::path path)
     : path_(path) 
  {}

  ~FileDevice() {
    Close();
  }

  FileDevice(const FileDevice &other)
      : path_(other.path_) {
    Open();
  }

  ByteVector Read(uint64_t position,
    uint64_t data_size) {
    if (!IsOpen()) {
      throw(exception::YASException("The device hasn't been opened during read", StorageError::kDeviceReadError));
    }

    device_.seekg(position);
    if (device_.eof()) {
      throw(exception::YASException("The device's get cursor position mismath", StorageError::kDeviceReadError));
    }

    std::vector<uint8_t> data(data_size);
    device_.read((char *)data.data(), data_size);
    if (device_.eof()) {
      throw(exception::YASException("Read after file end", StorageError::kDeviceReadError));
    }

    return data;
  }

  uint64_t Write(uint64_t position, std::vector<uint8_t> &data) {
    if (!IsOpen()) {
      throw(exception::YASException("The device hasn't been opened during write", StorageError::kDeviceWriteError));
    }

    device_.seekp(position);
    if (device_.eof()) {
      throw(exception::YASException("The device's put cursor position mismath", StorageError::kDeviceWriteError));
    }

    device_.write((char *)data.data(), data.size());
    if (!device_.good()) {
      throw(exception::YASException("Smth happened during device write", StorageError::kDeviceWriteError));
    }

    return data.size();
  }

  bool IsOpen() const noexcept {
    return device_.is_open() && device_.good();
  }

  bool Close() noexcept {
    device_.flush();
    device_.close();
    return !device_.fail();
  }

  FileDevice operator=(const FileDevice&) = delete;
  FileDevice operator=(FileDevice&&) = delete;
  FileDevice(FileDevice&&) = delete;

 protected:
  void Open() {
    device_.rdbuf()->pubsetbuf(0, 0);     // in Windows it is also desirable to disable NTFS write cache
    device_.open(path_, std::ios_base::in | std::ios_base::out | std::ios_base::binary);
  }

 private:
  mutable std::fstream device_;     // IsOpen should be "logically" const
  fs::path path_;
};

} // namespace devices
} // namespace yas
