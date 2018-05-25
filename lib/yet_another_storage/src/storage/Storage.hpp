#pragma once
#include "IStorage.hpp"

namespace yas {
namespace storage {

// should be class with basic exception safety garantee
// all methods should catch inner class exception and transform it to StorageError class wrapper to nonstd::expected
  template<typename CharType>
class Storage : public IStorage<CharType> {
 public:
  Storage() = default;
  ~Storage() = default;

  Storage(const Storage&) = delete;
  Storage(Storage &&) = delete;
  Storage& operator=(const Storage&) = delete;
  Storage& operator=(Storage&&) = delete;

 private:
};

} // namespace storage
} // namespace yas
