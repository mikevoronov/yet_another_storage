#pragma once
#include "YASException.h"
#include "../storage/storage_errors.hpp"
#include <exception>
#include <string>

namespace yas {
namespace exception {

// destinguish between our exception and other
storage::StorageErrorDescriptor YASExceptionHandler (std::exception_ptr exception) {
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

} // namespace exception
} // namespace yas
