#pragma once
#include "../external/filesystem.h"
#include "../physical_volume/physical_volume_layout/pv_layout_headers.h"
#include "../physical_volume/physical_volume_layout/pv_layout_types_headers.h"

using namespace yas::pv_layout_headers;
using namespace yas::pv_layout_types_headers;

namespace yas {
namespace device_worker {

// TODO : need to refactoring - too many similar functions
template <typename Device, typename OffsetType>
class PVDeviceWorker {
 public:
   PVDeviceWorker(fs::path file_path)
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

  template<>
  PVHeader Read(OffsetType offset) {
    device_.Read
  }
  
  template<>
  Simple4TypeHeader Read(OffsetType offset) {

  }

  template<>
  Simple8TypeHeader Read(OffsetType offset) {

  }

  template<>
  CommonTypeHeader Read(OffsetType offset) {

  }

  template<>
  FreelistHeader<OffsetType> Read(OffsetType offset) {

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

  template <>
  void Write(OffsetType offset, const PVHeader &type) {

  }

  template <>
  void Write(OffsetType offset, const Simple4TypeHeader &type) {

  }

  template <>
  void Write(OffsetType offset, const Simple8TypeHeader &type) {

  }

  template <>
  void Write(OffsetType offset, const ComplexTypeHeader &type) {

  }

  template <>
  void Write(OffsetType offset, const FreelistHeader<OffsetType> &type) {

  }

  PVDeviceWorker(const PVDeviceWorker&) = delete;
  PVDeviceWorker(PVDeviceWorker&&) = delete;
  PVDeviceWorker& operator=(const PVDeviceWorker&) = delete;
  PVDeviceWorker& operator=(PVDeviceWorker&&) = delete;

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

} // namespace device_worker
} // namespace yas
