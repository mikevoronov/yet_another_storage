#pragma once
#include "../storage/storage_errors.hpp"
#include <exception>
#include <string>

namespace yas {
namespace exception {

class YASException : public std::exception {
 public:
  YASException(std::string &&message, storage::StorageErrors error_code) 
      : message_(std::forward<std::string>(message)),
        error_code_(error_code)
  {}

  storage::StorageErrors GetErrorCode() const noexcept {
    return error_code_;
  }

  std::string GetMessage() const {
    return message_;
  }

 private:
  std::string message_;
  storage::StorageErrors error_code_;
};

} // namespace exception
} // namespace yas
