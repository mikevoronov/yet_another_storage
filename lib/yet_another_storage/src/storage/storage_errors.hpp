#pragma once
#include <cstdint>

namespace yas {
namespace storage {

enum StorageError : uint32_t {
  kSuccess = 0,
  kDeviceGeneralError = 1,
  kDeviceReadError = 2,
  kDeviceWriteError = 3,

  kFileNotFound = 1,
  kKeyNotFound = 2,

  kUnknownExceptionType
  // TODO : add list
};

struct StorageErrorDescriptor {
  StorageErrorDescriptor(std::string message, StorageError error_code)
      : error_code_(error_code),
        message_(std::move(message))
  {}
  StorageError error_code_;
  std::string message_;           // can be empty
};

} // namespace storage
} // namespace yas
