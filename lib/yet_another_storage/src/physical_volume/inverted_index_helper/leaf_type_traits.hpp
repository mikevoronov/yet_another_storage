#pragma once

namespace yas {
namespace index_helper {

template<typename LeafType>
struct leaf_traits {
  static LeafType NonExistValue() {
    return static_cast<LeafType>(0);
  }
};

} // namespace index_helper
} // namespace yas
