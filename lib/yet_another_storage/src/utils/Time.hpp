#pragma once
#include "../physical_volume/pv_layout_headers.h"    // for sizeof expired time structure
#include "../common/macros.h"
#include <cstdint>
#include <vector>
#include <ctime>
#include <limits>

using namespace yas::macros;

namespace yas {
namespace utils {

STRUCT_PACK(
class Time {
 public:
  constexpr explicit Time(uint32_t expired_time_low, uint16_t expired_time_high = 0)
      : expired_time_high_(expired_time_high), expired_time_low_(expired_time_low)
  {}

  explicit Time(time_t expired_time) {
    uint64_t time = static_cast<uint64_t>(expired_time);      // time_t can be int or floating point type
    if (time & 0xFFFF000000000000) {
      // some kind of infinite future time
      expired_time_high_ = std::numeric_limits<uint16_t>::max();
      expired_time_low_ = std::numeric_limits<uint32_t>::max();
      return;
    }
    expired_time_high_ = (time >> 32) & 0xFFFF;
    expired_time_low_ = time & 0xFFFFFFFF;
  }

  bool IsExpired() const {
    return *this < Time(std::time(nullptr));
  }

  friend constexpr bool operator<(Time lhs, Time rhs) {
    return (lhs.expired_time_high() == rhs.expired_time_high() && lhs.expired_time_low() < rhs.expired_time_low()) ||
      (lhs.expired_time_high() < rhs.expired_time_high());
  }

  friend constexpr bool operator==(const Time &lhs, const Time &rhs) {
    return !(lhs < rhs) && !(rhs < lhs);
  }

  friend constexpr bool operator>(const Time &lhs, const Time &rhs) {
    return rhs < lhs;
  }

  time_t GetTime() const noexcept {
    uint64_t time = expired_time_low_;    
    time |= (static_cast<uint64_t>(expired_time_high_) << 48);

    return static_cast<time_t>(time);
  }
  
  constexpr uint16_t expired_time_high() const { return expired_time_high_; }
  constexpr uint32_t expired_time_low() const { return expired_time_low_; }

 private:
  uint16_t expired_time_high_;
  uint32_t expired_time_low_;
});

// this class then would be used to unpack from raw bytes
static_assert(pv_layout_headers::kTimeSize == sizeof(Time), "size of Time should be the same as the size in physical_volume_layout");

} // namespace utils
} // namespace yas
