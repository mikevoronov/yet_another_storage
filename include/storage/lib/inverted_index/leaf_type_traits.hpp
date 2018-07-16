#pragma once
#include <limits>

namespace yas {
namespace index_helper {

template<typename LeafType>
struct leaf_type_traits {
  static constexpr LeafType [[nodiscard]] NonExistValue() noexcept {
    return static_cast<LeafType>(std::numeric_limits<LeafType>::max());
  }

  static constexpr bool IsExistValue(LeafType value) {
    return value != NonExistValue();
  }
};

} // namespace index_helper
} // namespace yas
