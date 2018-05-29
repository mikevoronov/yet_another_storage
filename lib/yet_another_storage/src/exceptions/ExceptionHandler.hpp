#pragma once
#include "YASException.hpp"
#include <exception>
#include <string>

namespace yas {
namespace exception {

class ExceptionHandler {
 public:
// destinguish between our exception and other
  static storage::StorageErrorDescriptor Handle (std::exception_ptr exception) {
    try {
      std::rethrow_exception(exception);
    }
    catch(const YASException &yas_exception) {
      return yas_exception.getError();
    }
    catch (...) {
      return { "unknown exception", storage::StorageError::kUnknownExceptionType };
    }
  }
};

} // namespace exception
} // namespace yas
