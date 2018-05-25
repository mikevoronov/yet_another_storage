#pragma once
#include "../common/filesystem.h"
#include "../physical_volume/pv_layout_headers.h"
#include "../utils/serialization_utils.h"
#include "devices/FileDevice.hpp"
#include "../common/common.h"
#include "utils/Version.hpp"
#include "../storage/storage_errors.hpp"

using namespace yas::pv_layout_headers;
using namespace yas::storage;

class Version;

namespace yas {
namespace device_worker {

template<typename OffsetType>
using DefaultDevice = devices::FileDevice<OffsetType>;

// the main purpose of this class is to proper read and write to file data of every type
template <typename OffsetType, typename Device = DefaultDevice<OffsetType>>
class PVDeviceDataReaderWriter {
 public:
   using pv_device_type = PVDeviceDataReaderWriter<OffsetType, Device>;

   static std::unique_ptr<pv_device_type> Load(fs::path &file_path, Version version) {
    auto device_worker = std::unique_ptr<pv_device_type>(new pv_device_type(file_path, cluster_size));
    PVHeader pv_header = device_worker->Read<PVHeader>(0);

    PVHeader default_header;
    if (0 != memcmp(default_header.signature, pv_header.signature, sizeof signature.signature)) {
      throw exception::YASException("Invalid PV header: corrupted signature", ErrorCode::kInvalidSignatureError);
    }
    else if (version < pv_header.version) {
      throw exception::YASException("Invalid PV header: unsupported version", ErrorCode::kInvalidSignatureError);
    }

    device_end_ = pv_header.pv_size_;
    return device_worker;
  }

  static std::unique_ptr<pv_device_type> Create(fs::path &file_path, utils::Version version,
      uint32_t priority, uint32_t cluster_size = kDefaultClusterSize) {
    const OffsetType device_end = (sizeof(PVHeader) + sizeof(FreelistHeader<OffsetType>) + kDefaultClusterSize);
    if (fs::exists(file_path)) {
      fs::resize_file(file_path, device_end);
    }

    // can't use std::make_unique because of it needs access to class ctor
    auto device_worker = std::unique_ptr<pv_device_type>(new pv_device_type(file_path, cluster_size));

    PVHeader pv_header;
    pv_header.version = version;
    pv_header.priority_ = priority;
    pv_header.pv_size_ = device_end;
    pv_header.cluster_size_ = cluster_size;
    device_worker->Write(0, pv_header);

    return device_worker;
  }

  ByteVector ReadComplex(OffsetType offset) {
    auto bytes = device_.Read<ComplexTypeHeader>(offset);
    ComplexTypeHeader type_header;
    serialization_utils::LoadFromBytes(std::cbegin(raw_bytes), std::cend(raw_bytes), &type_header);
    CheckComplexTypeHeader(type_header, true);

    ByteVector data(type_header.overall_size_);
    offset += sizeof(ComplexTypeHeader);

    read_cursor_begin = std::begin(data);
    read_cursor_end = std::begin(data);
    std::advance(read_cursor_end, type_header.chunk_size_);

    device_.Read(offset, read_cursor_begin, read_cursor_end);
//    while()

    return data;
  }

  template <typename Iterator>
  void WriteComplex(OffsetType offset, Iterator begin, Iterator end) {
    static_assert(std::is_trivially_copyable_v<ValueType>, "Could read only POD types");
  }

#ifdef UNIT_TEST
  Device& GetDevice() const { return device_; }
#endif

  PVDeviceDataReaderWriter(const PVDeviceDataReaderWriter&) = delete;
  PVDeviceDataReaderWriter(PVDeviceDataReaderWriter&&) = default;
  PVDeviceDataReaderWriter& operator=(const PVDeviceDataReaderWriter&) = delete;
  PVDeviceDataReaderWriter& operator=(PVDeviceDataReaderWriter&&) = delete;

 private:
  Device device_;
  uint32_t cluster_size_;
  OffsetType device_end_;

  explicit PVDeviceDataReaderWriter(fs::path file_path, uint32_t cluster_size = kDefaultClusterSize)
      : device_(file_path),
    cluster_size_(cluster_size) {
    if (!device_.IsOpen()) {
      throw(exception::YASException("Device worker error: the device hasn't been opened correctly",
        StorageError::kDeviceGeneralError));
    }
  }

  template <typename ValueType>
  ValueType Read(OffsetType offset) {
    static_assert(std::is_trivially_copyable_v<Type>, "PVDeviceDataReaderWriter::Read<Type>: Type should be POD");

    ByteVector data(sizeof(Type));
    device_.Read(offset, std::begin(data), std::end(data));
    ValueType type;
    serialization_utils::LoadFromBytes(std::cbegin(raw_bytes), std::cend(raw_bytes), &type);
    return type;
  }

   // TODO : universal link
   template <typename ValueType>
   void Write(OffsetType offset, ValueType &type) {
     static_assert(std::is_trivially_copyable_v<ValueType>, "Could read only POD types");
     // we know that PVHeader always placed at the file beginning 
     //auto bytes = device_.Read(offset, sizeof(ValueType));
     //ValueType header;
     //serialization_utils::LoadFromBytes(std::cbegin(bytes), std::cend(bytes), &header);
   }

   void CheckComplexTypeHeader(const ComplexTypeHeader &complex_header, bool is_first_header) const {
     if (is_first_header && complex_type_header.value_type_ != ValueType::kComplexBegin) {
       // read complex types is only possible from the beggining of sequence
       throw exception::YASException("Read complex type error: kComplexBegin type expected", kReadComplexTypeError);
     }
     else if (is_first_header && complex_type_header.value_type_ != ValueType::kComplexSequel) {
       throw exception::YASException("Read complex type error: kComplexSequel type expected", kReadComplexTypeError);
     }
     else if (complex_type_header.chunk_size_ > cluster_size_) {
       throw exception::YASException("Read complex type error: chunk size is bigger than device cluster size", 
          kReadComplexTypeError);
     }
     else if (complex_type_header.overall_size_ > kMaximumTypeSize) {
       throw exception::YASException("Read complex type error: chunk size is bigger than device cluster size",
         kReadComplexTypeError);
     }
   }
};

} // namespace device_worker
} // namespace yas
