#pragma once
#include "../physical_volume/physical_volume_layout/pv_layout_headers.h"    // for sizeof of version in header
#include "../common/macros.h"
#include <cstdint>
#include <vector>

using namespace yas::macros;

namespace yas {
namespace utils {

STRUCT_PACK(
class Version {
 public:
  Version(uint8_t major, uint8_t minor)
      : major_(major), minor_(minor)
  {}
  ~Version() = default;

  uint16_t major() const { return major_; }
  uint32_t minor() const { return minor_; }

  Version(const Version&) = default;
  Version(Version &&) = default;
  Version& operator=(const Version&) = default;
  Version& operator=(Version&&) = default;

 private:
  uint8_t major_;
  uint8_t minor_;
});

constexpr bool operator<(const Version &lhs, const Version &rhs) {
  return (lhs.major() < rhs.major()) ||
      (lhs.major() == rhs.major() && lhs.minor() < rhs.minor());
}

constexpr bool operator==(const Version &lhs, const Version &rhs) {
  return !(lhs < rhs) && !(rhs < lhs);
}

// this class then would used to unpack raw bytes
static_assert(pv_layout_headers::kVersionSize == sizeof(Version), "size of Time should be the same as the size in physical_volume_layout");
} // namespace utils
} // namespace yas
