#pragma once
#include "../common/filesystem.h"
#include "../common/common.h"
#include "../exceptions/YASException.hpp"
#include <fstream>

namespace yas {
namespace devices {

template <typename OffsetType>
class FileDevice {
 public:
  using path_type = fs::path;

  explicit FileDevice(const path_type &path)
      : path_(path) {
    Open();
  }

  ~FileDevice() {
    Close();
  }

  FileDevice(const FileDevice &other)
      : path_(other.path_) {
    Open();
  }

  template <typename Iterator>
  void Read(OffsetType position, Iterator begin, Iterator end) {
    if (!IsOpen()) {
      throw(exception::YASException("Raw device read error: the device hasn't been opened during read",
          storage::StorageError::kDeviceReadError));
    }

    device_.seekg(position);
    if (device_.eof()) {
      throw(exception::YASException("Raw device read error: the device's get cursor position mismath",
          storage::StorageError::kDeviceReadError));
    }

    const auto read_size = std::distance(begin, end);
    device_.read(reinterpret_cast<char*>(&(*begin)), read_size);
    if (device_.eof()) {
      throw(exception::YASException("Raw device read error: read after the file end", 
          storage::StorageError::kDeviceReadError));
    }
  }

  template <typename Iterator>
  OffsetType Write(OffsetType position, const Iterator begin, const Iterator end) {
    if (!IsOpen()) {
      throw(exception::YASException("Raw device write error: the device hasn't been opened during write",
          storage::StorageError::kDeviceWriteError));
    }

    device_.seekp(position);
    if (device_.eof()) {
      throw(exception::YASException("Raw device write error: the device's put cursor position mismath",
          storage::StorageError::kDeviceWriteError));
    }

    const auto write_size = std::distance(begin, end);
    device_.write(reinterpret_cast<const char*>(&(*begin)), write_size);
    if (!device_.good()) {
      throw(exception::YASException("Raw device write error: something bad happened during device write",
          storage::StorageError::kDeviceWriteError));
    }

    return write_size;
  }

  bool IsOpen() const noexcept {
    return device_.is_open() && device_.good();
  }

  bool Close() noexcept {
    device_.flush();
    device_.close();
    return !device_.fail();
  }

  static bool Exists(const path_type &pv_path) {
    return fs::exists(pv_path);
  }

  static void CreateEmpty(const path_type &pv_path) {
    std::ofstream out(pv_path, std::ios_base::out);
  }

  static path_type Canonical(const path_type &pv_path) {
    return fs::canonical(pv_path);
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
