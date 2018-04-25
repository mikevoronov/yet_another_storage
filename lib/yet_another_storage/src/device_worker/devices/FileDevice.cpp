#include "FileDevice.hpp"
#include "../../exceptions/YASException.h"

using namespace yas::storage;

namespace yas {
namespace devices {

FileDevice::FileDevice(fs::path path) 
    : path_(path) {
  Open();
}

ByteVector FileDevice::Read(uint64_t position,
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

uint64_t FileDevice::Write(uint64_t position, std::vector<uint8_t> &data) {
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

bool FileDevice::IsOpen() const {
  return device_.is_open() && device_.good();
}

FileDevice::~FileDevice() {
  Close();
}

void FileDevice::Open() {
  device_.rdbuf()->pubsetbuf(0, 0);     // in Windows it is also desirable to disable NTFS write cache
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
