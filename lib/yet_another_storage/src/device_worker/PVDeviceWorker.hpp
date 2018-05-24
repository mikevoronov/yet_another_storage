#pragma once
#include "../common/filesystem.h"
#include "../physical_volume/pv_layout_headers.h"
#include "../utils/serialization_utils.h"

using namespace yas::pv_layout_headers;

namespace yas {
namespace device_worker {

// the main pupose of this class is to proper read and write to file
template <typename Device, typename OffsetType>
class PVDeviceWorker {
 public:
   PVDeviceWorker(fs::path file_path)
      : device_(file_path) {
    if (!device_.IsOpen()) {
      throw(exception::YASException("The device hasn't been opened", StorageError::kDeviceGeneralError));
    }
  }

  template <typename Header>
  Header Read(OffsetType offset) {
    static_assert(std::is_trivially_copyable_v<ValueType>, "Could read only POD types");
    // we know that PVHeader always placed at the file beginning 
    auto bytes = device_.Read(offset, sizeof(PVHeader));
    Header header;
    serialization_utils::LoadFromBytes(std::cbegin(bytes), std::cend(bytes), &header);
  }
  
  // 3rd objects type could 
  template<>
  CommonTypeHeader Read(OffsetType offset) {

  }

  template <typename Header>
  void Write(OffsetType offset, ) {
    static_assert(std::is_trivially_copyable_v<ValueType>, "Could read only POD types");
    // we know that PVHeader always placed at the file beginning 
    auto bytes = device_.Read(offset, sizeof(PVHeader));
    Header header;
    serialization_utils::LoadFromBytes(std::cbegin(bytes), std::cend(bytes), &header);
  }


  template <>
  void Write(OffsetType offset, const ComplexTypeHeader &type) {

  }

  PVDeviceWorker(const PVDeviceWorker&) = delete;
  PVDeviceWorker(PVDeviceWorker&&) = delete;
  PVDeviceWorker& operator=(const PVDeviceWorker&) = delete;
  PVDeviceWorker& operator=(PVDeviceWorker&&) = delete;

 private:
   Device device_;

  template <typename ValueType>
  ValueType Read4BytesType(OffsetType offset) {
    Simple4TypeHeader header;
    auto bytes = Read<Simple4TypeHeader>(offset, sizeof(Simple4TypeHeader));
    serialization_utils::LoadFromBytes(std::cbegin(bytes), std::cend(bytes), &header);
    return header.body_.value;
  }

  template <typename ValueType>
  ValueType Read8BytesType(OffsetType offset) {
  }

  template <typename ValueType>
  ValueType ReadComplexType(OffsetType offset) {
  }

  template <typename ValueType>
  void WriteComplexType(OffsetType offset, const ValueType &type) {
  }
};

} // namespace device_worker
} // namespace yas
