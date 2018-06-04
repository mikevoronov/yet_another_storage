#pragma once
#include "pv_layout_headers.h"
#include "../../settings.h"
#include "../common/offset_type_traits.hpp"
#include <array>
#include <algorithm>

using namespace yas::pv_layout_headers;

namespace yas {
namespace freelist_helper {

template <typename OffsetType>
class FreelistHelper {
  using FreelistHeaderType = FreelistHeader<OffsetType>;
 public:
  // the last one must be equals to default cluster size 
  static constexpr std::array<OffsetType, kBinCount> kFreelistLimits = { sizeof(Simple4TypeHeader), sizeof(Simple8TypeHeader), 64, 100, 128, 256, 512, 1024, 1520, 2048, kDefaultClusterSize };
   
  FreelistHelper() {
    for (int32_t bin_id = 0; bin_id < kBinCount; ++bin_id) {
      bin_descriptors_[bin_id].offset_ = offset_traits<OffsetType>::NonExistValue();
      bin_descriptors_[bin_id].limit_ = kFreelistLimits[bin_id];
    }
  }

  explicit FreelistHelper(const FreelistHeaderType &header) {
    for (int32_t bin_id = 0; bin_id < kBinCount; ++bin_id) {
      bin_descriptors_[bin_id].offset_ = header.free_bins_[bin_id];
      bin_descriptors_[bin_id].limit_ = kFreelistLimits[bin_id];
    }
  }

  void SetBins(const FreelistHeaderType &header) noexcept {
    for (int32_t bin_id = 0; bin_id < kBinCount; ++bin_id) {
      bin_descriptors_[bin_id].offset_ = header.free_bins_[bin_id];
      bin_descriptors_[bin_id].limit_ = kFreelistLimits[bin_id];
    }
  }

  FreelistHeaderType GetBins() const {
    FreelistHeaderType header;
    for (int32_t bin_id = 0; bin_id < kBinCount; ++bin_id) {
      header.free_bins_[bin_id] = bin_descriptors_[bin_id].offset_;
    }

    return header;
  }

  ~FreelistHelper() = default;

  OffsetType PopFreeEntryOffset(OffsetType entry_size) {
    // very simple strategy
    bool is_less_cluster_size = entry_size < kDefaultClusterSize;

    int32_t last_viewed_freed_bin_id = -1;
    for (int32_t bin_id = 0; bin_id < kBinCount; ++bin_id) {
      if (entry_size <= bin_descriptors_[bin_id].limit_ 
          && offset_traits<OffsetType>::IsExistValue(bin_descriptors_[bin_id].offset_)) {
        const auto offset = bin_descriptors_[bin_id].offset_;
        bin_descriptors_[bin_id].offset_ = offset_traits<OffsetType>::NonExistValue();
        return offset;
      }
      if (!is_less_cluster_size && offset_traits<OffsetType>::IsExistValue(bin_descriptors_[bin_id].offset_)) {
        last_viewed_freed_bin_id = bin_id;
      }
    }

    if (!is_less_cluster_size && last_viewed_freed_bin_id > 5) {
      const auto offset = bin_descriptors_[last_viewed_freed_bin_id].offset_;
      bin_descriptors_[last_viewed_freed_bin_id].offset_ = offset_traits<OffsetType>::NonExistValue();
      return offset;
    }

    return offset_traits<OffsetType>::NonExistValue();
  }

  OffsetType PushFreeEntry(OffsetType new_offset, OffsetType entry_size) {
    const auto bin_id = getBinIdForSize(entry_size);
    const auto offset = bin_descriptors_[bin_id].offset_;
    bin_descriptors_[bin_id].offset_ = new_offset;
    return offset;
  }

  OffsetType GetFreeEntry(OffsetType entry_size) const {
    if (entry_size > kDefaultClusterSize) {
      entry_size = kDefaultClusterSize;
    }

    const auto bin_id = getBinIdForSize(entry_size);
    return bin_descriptors_[bin_id].offset_;
  }

  FreelistHelper(FreelistHelper&) = delete;
  FreelistHelper(FreelistHelper&&) = delete;
  FreelistHelper& operator=(const FreelistHelper&) = delete;
  FreelistHelper& operator= (FreelistHelper&&) = delete;

 private:
  struct BinDescriptor {
    OffsetType offset_;
    OffsetType limit_;
  };
  std::array<BinDescriptor, kBinCount> bin_descriptors_;

  uint32_t getBinIdForSize(OffsetType entry_size) const {
    auto found_value_it = std::lower_bound(std::cbegin(bin_descriptors_), std::cend(bin_descriptors_), entry_size, [](
        const BinDescriptor &bin_descriptor,
        const OffsetType entry_size) {
      return entry_size > bin_descriptor.limit_;
    });
    if (std::cend(bin_descriptors_) == found_value_it) {
      return kBinCount-1;
    }

    return static_cast<uint32_t>(std::distance(std::cbegin(bin_descriptors_), found_value_it));
  }
};

} // namespace freelist_helper
} // namespace yas
