#pragma once
#include "../common/filesystem.h"
#include "../physical_volume/pv_layout_headers.h"
#include "../utils/serialization_utils.h"
#include "devices/FileDevice.hpp"
#include "../common/common.h"
#include "../common/offset_type_traits.hpp"
#include "../storage/IStorage.hpp"
#include <string_view>

using namespace yas::pv_layout_headers;
using namespace yas::storage;

namespace yas {
namespace device_worker {

// class can read and write control headers of device layout
template <typename OffsetType, typename Device>
class PVDeviceDataReaderWriter {
 public:
  using pv_device_type = PVDeviceDataReaderWriter<OffsetType, Device>;

  explicit PVDeviceDataReaderWriter(fs::path &file_path, uint32_t cluster_size = kDefaultClusterSize)
    : device_(file_path),
      cluster_size_(cluster_size) {
    if (!device_.IsOpen()) {
      throw(exception::YASException("Device worker error: the device hasn't been opened correctly",
        StorageError::kDeviceGeneralError));
    }
  }

   ~PVDeviceDataReaderWriter() {
     // (!) need to be very accurate - we also need also to save inverted index first 
     device_.Close();
   }

   template <typename ValueType>
   ValueType Read(OffsetType offset) {
     static_assert(std::is_trivially_copyable_v<ValueType>, "PVDeviceDataReaderWriter::Read<Type>: type should be POD");

     ByteVector raw_bytes(sizeof(ValueType));
     device_.Read(offset, std::begin(raw_bytes), std::end(raw_bytes));

     ValueType type;
     serialization_utils::LoadFromBytes(std::cbegin(raw_bytes), std::cend(raw_bytes), &type);
     return type;
   }

   template <typename ValueType>
   void Write(OffsetType position, ValueType &type) {
     static_assert(std::is_trivially_copyable_v<ValueType>, "PVDeviceDataReaderWriter::Write<Type>: type should be POD");

     ByteVector data(sizeof(ValueType));
     serialization_utils::SaveAsBytes(std::begin(data), std::end(data), &type);
     device_.Write(position, std::cbegin(data), std::cend(data));
   }

   ByteVector ReadComplexType(OffsetType offset) {
     ComplexTypeHeader type_header = Read<ComplexTypeHeader>(offset);
     CheckComplexTypeHeader(type_header, true);

     ByteVector complex_data(type_header.overall_size_);
     offset += sizeof(ComplexTypeHeader);
     auto read_cursor_begin = std::begin(complex_data);
     auto read_cursor_end = std::begin(complex_data);
     std::advance(read_cursor_end, type_header.chunk_size_);
     device_.Read(offset, read_cursor_begin, read_cursor_end);

     while (type_header.sequel_offset_ != offset_traits<OffsetType>::NonExistValue()) {
       type_header = Read<ComplexTypeHeader>(offset);
       CheckComplexTypeHeader(type_header, false);

       read_cursor_begin = read_cursor_end;
       std::advance(read_cursor_end, type_header.chunk_size_);
     }

     return complex_data;
   }

   template <typename Iterator>
   void WriteComplexType(OffsetType offset, Iterator begin, Iterator end) {
     ComplexTypeHeader type_header = Read<ComplexTypeHeader>(offset);
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

  void CheckComplexTypeHeader(const ComplexTypeHeader &complex_header, bool is_first_header) const {
    if (is_first_header && complex_header.value_type_ != PVType::kComplexBegin) {
      // read complex types is only possible from the beggining of sequence
      throw exception::YASException("Read complex type error: kComplexBegin type expected",
        StorageError::kReadComplexTypeError);
    }
    else if (!is_first_header && complex_header.value_type_ != PVType::kComplexSequel) {
      throw exception::YASException("Read complex type error: kComplexSequel type expected",
        StorageError::kReadComplexTypeError);
    }
    else if (complex_header.chunk_size_ > cluster_size_) {
      throw exception::YASException("Read complex type error: chunk size is bigger than device cluster size",
        StorageError::kReadComplexTypeError);
    }
    else if (complex_header.overall_size_ > kMaximumTypeSize) {
      throw exception::YASException("Read complex type error: chunk size is bigger than device cluster size",
        StorageError::kReadComplexTypeError);
    }
  }
};

} // namespace device_worker
} // namespace yas
