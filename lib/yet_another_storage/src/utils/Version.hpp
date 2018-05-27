#pragma once
#include "../common/macros.h"
#include <cstdint>
#include <vector>

using namespace yas::macros;

namespace yas {
namespace utils {

STRUCT_PACK(
class Version {
 public:
  constexpr Version(uint8_t major = 0, uint8_t minor = 0)
      : major_(major), minor_(minor)
  {}

  friend constexpr bool operator<(const Version &lhs, const Version &rhs) {
    return (lhs.major() < rhs.major()) ||
      (lhs.major() == rhs.major() && lhs.minor() < rhs.minor());
  }

  friend constexpr bool operator>(const Version &lhs, const Version &rhs) {
    return rhs < lhs;
  }

  friend constexpr bool operator==(const Version &lhs, const Version &rhs) {
    return !(lhs < rhs) && !(rhs < lhs);
  }

  constexpr uint16_t major() const { return major_; }
  constexpr uint32_t minor() const { return minor_; }

 private:
  uint8_t major_;
  uint8_t minor_;
});

} // namespace utils
} // namespace yas
