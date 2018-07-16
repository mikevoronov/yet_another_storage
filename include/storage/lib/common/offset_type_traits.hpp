#pragma once

namespace yas {

template<typename OffsetType>
struct offset_traits {
  static constexpr [[nodiscard]] OffsetType NonExistValue() noexcept {
    return static_cast<OffsetType>(0);
  }

  static constexpr bool IsExistValue(OffsetType value) {
    return value != NonExistValue();
  }
};

} // namespace yas
