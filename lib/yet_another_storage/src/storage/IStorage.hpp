#pragma once
#include "../../ext/expected/expected.h"
#include "storage_errors.hpp"
#include <string_view>
#include <any>

namespace yas {
namespace storage {

template <typename CharType>
class IStorage {
 public:
  using key_type = std::basic_string_view<CharType>;

  virtual nonstd::expected<bool,     StorageErrorDescriptor> Put(key_type key, std::any value) = 0;
  virtual nonstd::expected<std::any, StorageErrorDescriptor> Get(key_type key) = 0;
  virtual nonstd::expected<bool,     StorageErrorDescriptor> HasKey(key_type key) = 0;
  virtual nonstd::expected<bool,     StorageErrorDescriptor> Delete(key_type key) = 0;
  virtual nonstd::expected<bool,     StorageErrorDescriptor> SetExpiredDate(key_type key, time_t expired) = 0;
  virtual nonstd::expected<time_t,   StorageErrorDescriptor> GetExpiredDate(key_type key) = 0;

  virtual ~IStorage() = default;
};

} // namespace storage
} // namespace yas
