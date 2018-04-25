#pragma once
#include <cstdint>

namespace yas {
namespace storage {

enum StorageError : uint32_t {
  kSuccess = 0,
  kFileNotFound = 1,
  kKeyNotFound = 2
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
