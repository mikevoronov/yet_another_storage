#pragma once
#include <cstdint>

namespace yas {
namespace storage {

enum StorageErrors : uint32_t {
  kSuccess = 0,
  kFileNotFound = 1,
  kKeyNotFound = 2
  // TODO : add list
};

struct StorageErrorDescriptor {
  StorageErrors error_code_;
  std::string message_;           // can be empty
};

} // namespace storage
} // namespace yas
