#pragma once
#include <numeric>

namespace yas {
namespace index_helper {

template<typename IdType>
struct id_type_traits {
  static constexpr IdType [[nodiscard]] NonExistValue() noexcept {
    return static_cast<IdType>(std::numeric_limits<IdType>::max());
  }

  static constexpr bool IsExistValue(IdType value) {
    return value != NonExistValue();
  }
};

} // namespace index_helper
} // namespace yas
