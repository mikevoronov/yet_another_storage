#pragma once
#include "storage_errors.hpp"
#include "lib/common/common.h"
#include "lib/ext/expected/expected.h"
#include <string_view>

namespace yas {
namespace storage {

/**
 *    \brief Interface for storage-like classes of YAS: Storage and PVManager
 *
 *    Provides basic operations over key-value storage
 */
template <typename CharType>
class IStorage {
 public:
  using key_type = std::basic_string_view<CharType>;

  virtual StorageErrorDescriptor Put(key_type key, const storage_value_type &value) noexcept = 0;
  virtual nonstd::expected<storage_value_type, StorageErrorDescriptor> Get(key_type key) noexcept = 0;
  virtual StorageErrorDescriptor HasKey(key_type key) noexcept = 0;
  virtual StorageErrorDescriptor HasCatalog(key_type key) noexcept = 0;
  virtual StorageErrorDescriptor Delete(key_type key) noexcept = 0;
  virtual StorageErrorDescriptor SetExpiredDate(key_type key, time_t expired) noexcept = 0;
  virtual nonstd::expected<time_t, StorageErrorDescriptor> GetExpiredDate(key_type key) noexcept = 0;

  virtual ~IStorage() = default;
};

} // namespace storage
} // namespace yas
