#pragma once

namespace yas {
namespace index_helper {

template<typename Type>
struct leaf_traits {
  static Type NonExistValue() {
    return static_cast<Type>(0);
  }
};

} // namespace index_helper
} // namespace yas
