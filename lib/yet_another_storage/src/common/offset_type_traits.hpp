#pragma once

namespace yas {

template<typename OffsetType>
struct offset_traits {
  static OffsetType NonExistValue() {
    return static_cast<OffsetType>(0);
  }

  static bool IsExistValue(OffsetType value) {
    return value != NonExistValue();
  }
};

} // namespace yas
