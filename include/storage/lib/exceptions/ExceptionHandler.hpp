#pragma once
#include "YASException.hpp"
#include <exception>
#include <string>

namespace yas {
namespace exception {

class ExceptionHandler {
 public:
   /// \brief convert the exception to StorageErrorDescriptor
   /// \param exception - the pointer to exception to convert
   static storage::StorageErrorDescriptor Handle (std::exception_ptr exception) {
    try {
      std::rethrow_exception(exception);
    }
    catch(const YASException &yas_exception) {
      return yas_exception.getError();
    }
    catch (const std::runtime_error &runtime_exception) {
      return { runtime_exception.what(), storage::StorageError::kUnknownExceptionType };
    }
    catch (const std::exception &exception) {
      return { exception.what(), storage::StorageError::kUnknownExceptionType };
    }
    catch (...) {
      return { "unknown exception", storage::StorageError::kUnknownExceptionType };
    }
  }
};

} // namespace exception
} // namespace yas
