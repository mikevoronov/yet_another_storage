#pragma once
#include "../physical_volume/physical_volume_layout/pv_layout_types_headers.h"    // for sizeof expired time structure
#include "../common/macros.h"
#include <cstdint>
#include <vector>

using namespace yas::macros;

namespace yas {
namespace utils {

STRUCT_PACK(
class Time {
 public:
  Time(uint32_t expired_time_low, uint16_t expired_time_high = 0)
      : expired_time_high_(expired_time_high), expired_time_low(expired_time_low_)
  {}

  bool IsExpired() const noexcept {
    return !(*this < Now());
  }

  // TODO : return now time in seconds
  Time Now() const noexcept {
    return { 0, 0 };
  }

  uint16_t expired_time_high() const { return expired_time_high_; }
  uint32_t expired_time_low() const { return expired_time_low_; }

  Time(const Time&) = default;
  Time(Time &&) = default;
  Time& operator=(const Time&) = default;
  Time& operator=(Time&&) = default;

 private:
  uint16_t expired_time_high_;
  uint32_t expired_time_low_;
});

constexpr bool operator<(const Time &lhs, const Time &rhs) {
  return (lhs.expired_time_high() < rhs.expired_time_high()) ||
      (lhs.expired_time_high() == rhs.expired_time_high() && lhs.expired_time_low() < rhs.expired_time_low());
}

// this class then would used to unpack raw bytes
static_assert(pv_layout_types_headers::kTimeSize == sizeof(Time), "size of Time should be the same as the size in physical_volume_layout");
} // namespace utils
} // namespace yas
