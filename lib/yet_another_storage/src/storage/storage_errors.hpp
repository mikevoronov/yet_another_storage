#pragma once
#include <cstdint>

namespace yas {
namespace storage {

enum StorageError : uint32_t {
  kSuccess = 0,
  kDeviceGeneralError = 1,
  kDeviceReadError = 2,
  kDeviceWriteError = 3,
  kDeviceExpandError = 4,
  kInvertedIndexDesirializationError = 5,
  kInvertedIndexDesirializationVersionUnsopportedError = 6,   // explicit type from kInvertedIndexDesirializationError
                                                              // because of it could be recover from it with different
                                                              // version header
  kCorruptedHeaderError = 7,
  kInvalidPVSignatureError = 8,
  kPVVersionNotSupported = 9,
  kKeyNotFound = 10,
  kPVMountPointManagerAdapterNotReady = 11,
  kValueNotFound = 12,          // used for std::any
  kKeyAlreadyCreated = 13,
  kPVVersionUnsopported = 14,

  kUnknownExceptionType
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
