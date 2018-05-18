#pragma once
#include "../physical_volume_layout/pv_layout_headers.h"
#include "../utils/serialization_utils.h"
#include "FreelistStrategy.hpp"
#include <array>

using namespace yas::pv_layout_headers;

namespace yas {
namespace freelist_helper {

template <typename Device, typename OffsetType>
class PVDeviceWorker;

template <typename Strategy, typename OffsetType>
class FreelistHelper {
 public:
  ~FreelistHelper() = default;

  static FreelistHelper Load(const FreelistHeader &header) {
    FreelistHelper helper;
    for (int bin_id = 0; bin_id < kBinCount; ++bin_id) {
      bins_[bin_id] = header.free_bins_[bin_id];
    }
  }

  template <typename Iterator>
  static FreelistHelper Load(const Iterator cbegin, const Iterator cend) {
    FreelistHelper helper;
    serialization_utils::LoadFromBytes(cbegin, cend, helper);

    return helper;
  }

  OffsetType GetFreeEntry(const uint32_t entry_size, PVDeviceWorker &device_worker) {
  }

  FreelistHelper(FreelistHelper&) = delete;
  FreelistHelper(FreelistHelper&&) = delete;
  FreelistHelper& operator=(const FreelistHelper&) = delete;
  FreelistHelper& opeartor = (FreelistHelper&&) = delete;

 private:
  std::array<OffsetType, kBinCount> bins_;

  FreelistHelper() = default;
};

} // namespace freelist_helper
} // namespace yas
