#pragma once
#include <cstdint>

namespace yas {
namespace storage {

enum class StorageError : uint32_t {
  kSuccess = 0,
  kDeviceGeneralError = 1,
  kDeviceReadError = 2,
  kDeviceWriteError = 3,
  kDeviceExpandError = 4,
  kInvertedIndexDeserializationError = 5,
  kInvertedIndexDeserializationVersionUnsupportedError = 6,   // different type from kInvertedIndexDeserializationError
                                                              // because of it could be recover from it with different
                                                              // version header
  kCorruptedHeaderError = 7,
  kInvalidPVSignatureError = 8,
  kKeyNotFound = 9,
  kKeyExpired = 10,
  kKeyDoesntExpired = 11,
  kPVMountPointManagerAdapterNotReady = 12,
  kIncorrectStorageValue = 13,                                // used for valueless std::variant
  kKeyAlreadyCreated = 14,
  kPVVersionUnsupported = 15,
  kUnknownError = 16,
  kCatalogNotFoundError = 17,
  kMemoryNotEnough = 18,
  kPathNotFound = 19,
  kPVNotFound = 20,

  kUnknownExceptionType
};

struct StorageErrorDescriptor {
  // message should be optimized by compiler through copy elision
  StorageErrorDescriptor(std::string message, StorageError error_code)
      : message_(std::move(message)),
        error_code_(error_code)
  {}

  StorageErrorDescriptor(const StorageErrorDescriptor &other) noexcept {
    try {
      error_code_ = other.error_code_;
      message_.assign(other.message_);
    }
    catch (const std::bad_alloc &) {
      error_code_ = StorageError::kMemoryNotEnough;
      message_.clear();
    }
    catch (...) {
      error_code_ = StorageError::kUnknownError;
      message_.clear();
    }
  }

  StorageError error_code_;
  std::string message_;           // can be empty

  operator bool() const {
    return StorageError::kSuccess == error_code_ ;
  }
};

} // namespace storage
} // namespace yas
