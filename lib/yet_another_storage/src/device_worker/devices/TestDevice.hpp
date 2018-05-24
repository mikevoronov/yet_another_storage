#pragma once
#include "../../common/filesystem.h"
#include "../../common/common.h"
#include "../../exceptions/YASException.h"
#include <algorithm>
#include <cstdint>
#include <fstream>

namespace yas {
namespace devices {

// this simple device based on std::vector
template <typename OffsetType>
class TestDevice {
 public:
  explicit TestDevice(fs::path path)
  {}

  ~TestDevice() = default;
  TestDevice(const TestDevice &other) = default;

  ByteVector Read(uint64_t position, uint64_t data_size) {
    // possible overflow but it is just a test device
    if ((position + data_size) < storage_.size()) {
      throw(exception::YASException("The device's get cursor position mismath", StorageError::kDeviceReadError));
    }

    auto begin = std::cbegin(storage_);
    auto end = std::cbegin(storage_);
    std::advance(begin, position);
    std::advance(end, position + data_size);

    return { begin, end };
  }

  template<typename Iterator>
  uint64_t Write(uint64_t position, const Iterator begin, const Iterator end) {
    if (position > storage_.size()) {
      throw(exception::YASException("The device hasn't been opened during write", StorageError::kDeviceWriteError));
    }

    const auto data_size = std::distance(begin, end);
    storage_.reserve(position + data_size);
    if (position == storage_.size()) {
      // case one: add new content strictly to the end of file
      for (auto it = begin; it != end; ++it) {
        storage_.push_back(*it);
      }
      return data_size;
    }

    auto storage_begin = std::begin(storage_);
    auto storage_end = std::cbegin(storage_);
    std::advance(storage_begin, position);
    std::advance(storage_end, ((position + data_size) > storage_.size() ? storage_.size() : position + data_size));

    auto data_it = begin;
    for (; storage_begin != storage_end; ++storage_begin) {
      *storage_begin = *data_it;
      ++data_it;
    }

    if (storage_end != std::cend(storage_)) {
      return data_size;
    }

    for (; data_it != end; ++data_it) {
      storage_.push_back(*data_it);
    }

    return data_size;
  }

  bool IsOpen() const noexcept {
    return true;
  }

  bool Close() noexcept {
    return true;
  }

  ByteVector GetStorageContent() const {
    return storage_;
  }

  template<typename Iterator>
  void SetStorageContent(Iterator begin, Iterator end) {
    ByteVector tmp;
    storage_.swap(tmp);     // to reduce capacity

    storage_.assign(begin, end);
  }

  TestDevice operator=(const TestDevice&) = delete;
  TestDevice operator=(TestDevice&&) = delete;
  TestDevice(TestDevice&&) = delete;
 
 private:
  ByteVector storage_;
};

} // namespace devices
} // namespace yas
