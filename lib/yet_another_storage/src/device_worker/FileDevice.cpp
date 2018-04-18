#include "FileDevice.hpp"

namespace yas {
namespace devices {

FileDevice::FileDevice(fs::path path) 
    : path_(path) {
  Open();
}

nonstd::expected<std::vector<uint8_t>, IDevice::DeviceErrorCode> FileDevice::Read(uint64_t position, 
    uint64_t data_size) {
  if (!IsOpen()) {
    return nonstd::make_unexpected(IDevice::DeviceErrorCode::DeviceOpenError);
  }

  device_.seekg(position);
  if (device_.eof()) {
    return nonstd::make_unexpected(IDevice::DeviceErrorCode::DevicePositionMismatch);
  }

  std::vector<uint8_t> data(data_size);
  device_.read((char *)data.data(), data_size);
  if (device_.eof()) {
    return nonstd::make_unexpected(IDevice::DeviceErrorCode::DeviceLengthMismatch);
  }

  return data;
}

nonstd::expected<uint64_t, IDevice::DeviceErrorCode> FileDevice::Write(uint64_t position, std::vector<uint8_t> &data) {
  if (!IsOpen()) {
    return nonstd::make_unexpected(IDevice::DeviceErrorCode::DeviceOpenError);
  }

  device_.seekp(position);
  if (device_.eof()) {
    return nonstd::make_unexpected(IDevice::DeviceErrorCode::DeviceWriteError);
  }

  device_.write((char *)data.data(), data.size());
  if (!device_.good()) {
    return nonstd::make_unexpected(IDevice::DeviceErrorCode::DeviceWriteError);
  }

  return true;
}

bool FileDevice::IsOpen() const {
  return device_.is_open() && device_.good();
}

FileDevice::~FileDevice() {
  Close();
}

void FileDevice::Open() {
  device_.rdbuf()->pubsetbuf(0, 0);     // in Windows case its also desirable to disable NTFS write cache
  device_.open(path_, std::ios_base::in | std::ios_base::out | std::ios_base::binary);
}

bool FileDevice::Close() {
  device_.flush();
  device_.close();
  return !device_.fail();
}

FileDevice::FileDevice(const FileDevice &other) 
    : path_(other.path_) {
  Open();
}

} // namespace devices
} // namespace yas
