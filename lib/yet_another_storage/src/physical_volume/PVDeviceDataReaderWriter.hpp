#pragma once
#include "pv_layout_headers.h"
#include "../utils/serialization_utils.h"
#include "../devices/FileDevice.hpp"
#include <string_view>

using namespace yas::pv_layout_headers;
using namespace yas::storage;

namespace yas {
namespace pv {

// class can read and write control headers of device layout
template <typename OffsetType, typename Device>
class PVDeviceDataReaderWriter {
  using PVPathType = typename Device::path_type;

 public:
  explicit PVDeviceDataReaderWriter(const PVPathType &file_path, uint32_t cluster_size = kDefaultClusterSize)
      : device_(file_path),
        cluster_size_(cluster_size) {
    if (!device_.IsOpen()) {
      throw(exception::YASException("Device worker error: the device hasn't been opened correctly",
          StorageError::kDeviceGeneralError));
    }
  }

  ~PVDeviceDataReaderWriter() {
    device_.Close();
  }

  template <typename ValueType>
  ValueType Read(OffsetType offset) {
    static_assert(std::is_trivially_copyable_v<ValueType>, "PVDeviceDataReaderWriter::Read<Type>: Type should be POD");

    ByteVector raw_bytes(sizeof(ValueType));
    device_.Read(offset, std::begin(raw_bytes), std::end(raw_bytes));

    ValueType type;
    serialization_utils::LoadFromBytes(std::cbegin(raw_bytes), std::cend(raw_bytes), &type);
    return type;
  }

  template <typename ValueType>
  void Write(OffsetType position, const ValueType &type) {
    static_assert(std::is_trivially_copyable_v<ValueType>, "PVDeviceDataReaderWriter::Write<Type>: Type should be POD");

    ByteVector data(sizeof(ValueType));
    serialization_utils::SaveAsBytes(std::begin(data), std::end(data), &type);
    device_.Write(position, std::cbegin(data), std::cend(data));
  }

  ByteVector ReadComplexType(OffsetType offset) {
    ComplexTypeHeader type_header = Read<ComplexTypeHeader>(offset);
    CheckComplexTypeHeader(type_header, true);

    ByteVector complex_data(type_header.overall_size_);
    OffsetType readed_size = 0;
    const OffsetType overall_size = type_header.overall_size_;

    offset += offsetof(ComplexTypeHeader, data_);
    auto read_cursor_begin = std::begin(complex_data);
    auto read_cursor_end = std::begin(complex_data);
    std::advance(read_cursor_end, type_header.chunk_size_);
    device_.Read(offset, read_cursor_begin, read_cursor_end);
    readed_size += type_header.chunk_size_;

    while (readed_size < overall_size) {
      offset = type_header.sequel_offset_;
      type_header = Read<ComplexTypeHeader>(type_header.sequel_offset_);
      CheckComplexTypeHeader(type_header, false);
      offset += offsetof(ComplexTypeHeader, data_);

      read_cursor_begin = read_cursor_end;
      std::advance(read_cursor_end, type_header.chunk_size_);
      readed_size += type_header.chunk_size_;
      device_.Read(offset, read_cursor_begin, read_cursor_end);
    }

    return complex_data;
  }

  template <typename Iterator>
  OffsetType WriteComplexType(OffsetType offset, PVType pv_type, bool is_first, OffsetType next_free_offset,
        Iterator begin, Iterator end) {
    ComplexTypeHeader header = Read<ComplexTypeHeader>(offset);
    const auto data_size = std::distance(begin, end);
    const auto written = std::min<OffsetType>(header.chunk_size_, data_size);

    header.overall_size_ = data_size;
    header.value_type_ = pv_type;
    header.value_state_ = (is_first ? PVTypeState::kComplexBegin : PVTypeState::kComplexSequel);
    header.chunk_size_ = written;
    header.sequel_offset_ = next_free_offset;

    auto new_end = begin;
    std::advance(new_end, written);
    // at first try to write data 
    Write<ComplexTypeHeader>(offset, header);
    device_.Write(offset + offsetof(ComplexTypeHeader, data_), begin, new_end);
    // and only then header to don't loose original header if data writing will fails with exception

    return written;
  }

  ByteVector RawRead(OffsetType offset, OffsetType size) {
    ByteVector data(size);
    device_.Read(offset, std::begin(data), std::end(data));

    return data;
  }

  template <typename Iterator>
  OffsetType RawWrite(OffsetType offset, const Iterator begin, const Iterator end) {
    return device_.Write(offset, begin, end);
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
    if (is_first_header && !(complex_header.value_state_ & PVTypeState::kComplexBegin)) {
      // read complex types is only possible from the beggining of sequence
      throw exception::YASException("Read complex type error: kComplexBegin type expected",
          StorageError::kCorruptedHeaderError);
    }
    else if (!is_first_header && !(complex_header.value_state_ & PVTypeState::kComplexSequel)) {
      throw exception::YASException("Read complex type error: kComplexSequel type expected",
          StorageError::kCorruptedHeaderError);
    }
    else if (complex_header.chunk_size_ > cluster_size_) {
      throw exception::YASException("Read complex type error: chunk size is bigger than device cluster size",
          StorageError::kCorruptedHeaderError);
    }
    else if (complex_header.overall_size_ > kMaximumTypeSize) {
      throw exception::YASException("Read complex type error: chunk size is bigger than device cluster size",
          StorageError::kCorruptedHeaderError);
    }
  }
};

} // namespace pv
} // namespace yas
