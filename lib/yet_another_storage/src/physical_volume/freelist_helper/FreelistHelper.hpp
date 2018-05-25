#pragma once
#include "../pv_layout_headers.h"
#include "../../common/settings.hpp"
#include "../../common/offset_type_traits.hpp"
#include <array>

using namespace yas::pv_layout_headers;

namespace yas {
namespace freelist_helper {

// the last one must be equals to default cluster size 
std::array<OffsetType, kBinCount> kFreelistLimits = { 12, 16, 32, 64, 128, 256, 512, 1024, 2048, kDefaultClusterSize };
struct BinDescriptor {
  OffsetType offset_;
  OffsetType limit_;
};

template <typename Strategy, typename OffsetType>
class FreelistHelper {
 public:
  FreelistHelper(const FreelistHeader &header) {
    for (int32_t bin_id = 0; bin_id < kBinCount; ++bin_id) {
      bin_descriptors_[bin_id].offset_ = header.free_bins_[bin_id];
      bin_descriptors_[bin_id].limit_ = kFreelistLimits[bin_id];
    }
  }

  ~FreelistHelper() = default;

  OffsetType GetFreeEntryOffset(uint32_t entry_size) {
    // very simple strategy
    bool is_less_cluster_size = entry_size < kDefaultClusterSize;

    OffsetType last_viewed_freed_bin_id = offset_traits<OffsetType>::NonExistValue();
    for (int32_t bin_id = 0; bin_id < kBinCount; ++bin_id) {
      if (entry_size <= bin_descriptors_[bin_id].limit_ 
          && bin_descriptors_[bin_id].offset_ != offset_traits<OffsetType>::NonExistValue()) {
        const auto offset = bin_descriptors_[bin_id].offset_;
        bin_descriptors_[bin_id].offset_ = offset_traits<OffsetType>::NonExistValue();
        return offset;
      }
      if (is_less_cluster_size && bin_descriptors_[bin_id].offset_ != offset_traits<OffsetType>::NonExistValue()) {
        last_viewed_freed_bin_id = bin_id;
      }
    }

    if (!is_less_cluster_size && last_viewed_freed_bin_id > 5 
        && last_viewed_freed_bin_id != offset_traits<OffsetType>::NonExistValue()) {
      const auto offset = bin_descriptors_[last_viewed_freed_bin_id].offset_;
      bin_descriptors_[last_viewed_freed_bin_id].offset_ = offset_traits<OffsetType>::NonExistValue();
      return offset;
    }

    return offset_traits<OffsetType>::NonExistValue();
  }

  OffsetType SetFreeEntry(OffsetType new_offset, uint32_t entry_size) {
    assert(entry_size > kDefaultClusterSize);

    for (int32_t bin_id = 0; bin_id < kBinCount; ++bin_id) {
      if (entry_size <= bin_descriptors_[bin_id].limit_) {
        const auto offset = bin_descriptors_[bin_id].offset_;
        bin_descriptors_[bin_id].offset_ = new_offset;
        return offset;
      }
    }
  }

  FreelistHelper(FreelistHelper&) = delete;
  FreelistHelper(FreelistHelper&&) = delete;
  FreelistHelper& operator=(const FreelistHelper&) = delete;
  FreelistHelper& opeartor = (FreelistHelper&&) = delete;

 private:
  std::array<BinDescriptor, kBinCount> bin_descriptors_;
};

} // namespace freelist_helper
} // namespace yas
