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
   /// TODO (!) - this class is only ad-hoc solution now (yes, there are some ugly work with std::string in try-catch)
   /// and should be refactored in version 1.4
   static storage::StorageErrorDescriptor Handle(std::exception_ptr exception) {
    try {
      std::rethrow_exception(exception);
    }
    catch(const YASException &yas_exception) {
      return yas_exception.getError();
    }
    catch (const std::runtime_error &runtime_exception) {
      try {
        // additional try-catch is needed because of ctor of std::string can throw exception
        return { runtime_exception.what(), storage::StorageError::kUnknownExceptionType };
      }
      catch (...) {
        // note that in C++17 std::string() is noexcept
        return { std::string(), storage::StorageError::kMemoryNotEnough };
      }
    }
    catch (const std::bad_alloc &exception) {
      try {
        return { exception.what(), storage::StorageError::kMemoryNotEnough };
      }
      catch (...) {
        return { std::string(), storage::StorageError::kMemoryNotEnough };
      }
   }
    catch (const std::exception &exception) {
      try {
        return { exception.what(), storage::StorageError::kUnknownExceptionType };
      }
      catch (...) {
        return { std::string(), storage::StorageError::kMemoryNotEnough };
      }
   }
    catch (...) {
      // since in C++17 std::string() is noexcept - there shouldn't additional try-catch block
      return { std::string(), storage::StorageError::kUnknownExceptionType };
    }
  }
};

} // namespace exception
} // namespace yas
