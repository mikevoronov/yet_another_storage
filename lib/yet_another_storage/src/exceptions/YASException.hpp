#pragma once
#include "../storage/storage_errors.hpp"
#include <exception>
#include <string>

namespace yas {
namespace exception {

class YASException : public std::exception {
 public:
  YASException(std::string message, storage::StorageError error_code)
      : message_(std::move(message)),
        error_code_(error_code)
  {}

  storage::StorageErrorDescriptor getError() const {
    return { message_, error_code_ };
  }

 private:
  std::string message_;
  storage::StorageError error_code_;
};

} // namespace exception
} // namespace yas
