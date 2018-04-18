#pragma once
#include "../external/expected.h"
#include "storage_errors.hpp"
#include <string_view>

namespace yas {
namespace storage {

// should be class with basic exception safety garantee
// all methods should catch inner class exception and transform it to StorageError class wrapper to nonstd::expected
class Storage {
 public:
  Storage() = default;
  ~Storage() = default;

  // TODO : add expired_time
  template <typename ValueType>
  nonstd::expected<bool, StorageErrorDescriptor> Put(std::string_view key, ValueType value);

  template <typename ValueType>
  nonstd::expected<ValueType, StorageErrorDescriptor> Get(std::string_view key) const;

  bool HasKey(std::string_view key) const noexcept;

  template <typename ValueType>
  nonstd::expected<bool, StorageErrorDescriptor> Delete(std::string_view key);

  // get expiration date
  // set expiration date

  Storage(const Storage&) = delete;
  Storage(Storage &&) = delete;
  Storage& operator=(const Storage&) = delete;
  Storage& operator=(Storage&&) = delete;

 private:
};

} // namespace storage
} // namespace yas
