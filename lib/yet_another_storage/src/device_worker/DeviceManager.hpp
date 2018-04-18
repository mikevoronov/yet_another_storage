#pragma once

#include "../external/filesystem.h"
#include "devices/IDevice.hpp"

namespace yas {

template <typename Device, typename OffsetType>
class DeviceManager {
 public:
  DeviceManager(fs::path file_path)
    : device_(file_path) {
    if (!device_.IsOpen()) {
      throw 1;
    }
  }

  template <typename ValueType>
  ValueType Read(OffsetType offset) {
    return Read<ValueType>(std::is_integral<ValueType>, offset);
  }

  template <typename ValueType>
  void Write(OffsetType offset, ValueType &&type) {
    Write<ValueType>(std::is_integral<ValueType>, offset, std::forward(type));
  }

 private:
   Device device_;

  // read complex type
  template <typename ValueType>
  ValueType Read(std::true_type, OffsetType offset) {
  }

  // read trivially type
  template <typename ValueType>
  ValueType Read(std::false_type, OffsetType offset) {
  }

  // write complex type
  template <typename ValueType>
  void Write(std::true_type, OffsetType offset, ValueType &&type) {

  }

  // write trivially type
  template <typename ValueType>
  void Write(std::false_type, OffsetType offset, ValueType &&type) {

  }

  uint64_t read(OffsetType offset, char* buf, size_t buf_size);
  uint64_t write(OffsetType offset, const char* buf, size_t buf_size);
};

} // namespace yas
