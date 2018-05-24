#pragma once
#include "../../common/filesystem.h"
#include "../../common/common.h"
#include "../../exceptions/YASException.h"
#include <fstream>

namespace yas {
namespace devices {

template <typename OffsetType>
class FileDevice {
 public:
   explicit FileDevice(fs::path path)
     : path_(path) 
  {}

  ~FileDevice() {
    Close();
  }

  FileDevice(const FileDevice &other)
      : path_(other.path_) {
    Open();
  }

  template <typename Iterator>
  void Read(uint64_t position, Iterator begin, Iterator end) {
    if (!IsOpen()) {
      throw(exception::YASException("Raw device read error: the device hasn't been opened during read", 
          StorageError::kDeviceReadError));
    }

    device_.seekg(position);
    if (device_.eof()) {
      throw(exception::YASException("Raw device read error: the device's get cursor position mismath", 
          StorageError::kDeviceReadError));
    }

    const auto read_size = std::distance(begin, end);
    device_.read(static_cast<char*>(begin), data_size);
    if (device_.eof()) {
      throw(exception::YASException("Raw device read error: read after the file end", StorageError::kDeviceReadError));
    }

    return data;
  }

   template <typename Iterator>
  uint64_t Write(uint64_t position, Iterator begin, Iterator end) {
    if (!IsOpen()) {
      throw(exception::YASException("Raw device write error: The device hasn't been opened during write", 
          StorageError::kDeviceWriteError));
    }

    device_.seekp(position);
    if (device_.eof()) {
      throw(exception::YASException("Raw device write error: the device's put cursor position mismath", 
          StorageError::kDeviceWriteError));
    }

    const auto write_size = std::distance(begin, end);
    device_.write(static_cast<char*>(begin), data_size);
    if (!device_.good()) {
      throw(exception::YASException("Raw device write error: something bad happened during device write", 
          StorageError::kDeviceWriteError));
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

 private:
  mutable std::fstream device_;     // IsOpen should be "logically" const
  fs::path path_;

  void Open() {
    device_.rdbuf()->pubsetbuf(0, 0);     // in Windows it is also desirable to disable NTFS write cache
    device_.open(path_, std::ios_base::in | std::ios_base::out | std::ios_base::binary);
  }
};

} // namespace devices
} // namespace yas
