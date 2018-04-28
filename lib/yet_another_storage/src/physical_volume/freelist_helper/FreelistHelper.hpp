#pragma once
#include "../../device_worker/DeviceManager.hpp"
#include "FreelistStrategy.hpp"
#include <array>

namespace yas {
namespace freelist_helper {

template <typename Strategy, typename OffsetType>
class FreelistHelper {
 public:
  static constexpr uint32_t kBinCount = 11;
  FreelistHelper() = default;
  ~FreelistHelper() = default;

  OffsetType GetFreeEntry(const uint32_t entry_size, device_worker::DeviceManager &device_manager) {
  }

  FreelistHelper(FreelistHelper&) = delete;
  FreelistHelper(FreelistHelper&&) = delete;
  FreelistHelper& operator=(const FreelistHelper&) = delete;
  FreelistHelper& opeartor = (FreelistHelper&&) = delete;

 private:
  std::array<OffsetType, kBinCount> bins_;
};

} // namespace freelist_helper
} // namespace yas
