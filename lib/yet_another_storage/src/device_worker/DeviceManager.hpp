#pragma once
#include "../external/filesystem.h"

namespace yas {

template <typename Device, typename OffsetType>
class DeviceManager {
 public:
  DeviceManager(fs::path file_path)
    : device_(file_path) {
    if (!device_.IsOpen()) {
      throw(exception::YASException("The device hasn't been opened", StorageError::kDeviceGeneralError));
    }
  }

  template <typename ValueType>
  ValueType Read(OffsetType offset) {
    if constexpr (4 >= sizeof(ValueType)) {
      return Read4BytesType(offset);
    }
    if constexpr (8 >= sizeof(ValueType)) {
      return Read8BytesType(offset);
    }
    return ReadComplexType(offset);
  }

  template <typename ValueType>
  void Write(OffsetType offset, const ValueType &type) {
      if constexpr (4 >= sizeof(ValueType)) {
        return Write4BytesType(offset, type);
      }
      if constexpr (8 >= sizeof(ValueType)) {
        return Write8BytesType(offset, type);
      }
      return WriteComplexType(offset, type);
    }

 private:
   Device device_;

  template <typename ValueType>
  ValueType Read4BytesType(OffsetType offset) {
  }

  template <typename ValueType>
  ValueType Read8BytesType(OffsetType offset) {
  }

  template <typename ValueType>
  ValueType ReadComplexType(OffsetType offset) {
  }

  template <typename ValueType>
  void Write4BytesType(OffsetType offset, const ValueType &type) {
  }

  template <typename ValueType>
  void Write8BytesType(OffsetType offset, const ValueType &type) {
  }

  template <typename ValueType>
  void WriteComplexType(OffsetType offset, const ValueType &type) {
  }

  uint64_t read(OffsetType offset, char* buf, size_t buf_size);
  uint64_t write(OffsetType offset, const char* buf, size_t buf_size);
};

} // namespace yas
