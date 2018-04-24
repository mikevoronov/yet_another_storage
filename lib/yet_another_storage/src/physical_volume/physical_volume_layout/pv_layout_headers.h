#pragma once
#include "../../common/macros.h"
#include "../../common/settings.hpp"
#include "../freelist_helper/FreelistHelper.hpp"
#include <cstdint>

using namespace yas::macros;

namespace yas {
namespace pv_layout_headers {

// PV layout contains of
// PVHeader
// Freelists
// Inverted index
// Data

STRUCT_PACK(
struct PVHeader {
  uint8_t signature[6] /*= { 'Y', 'A', 'S', '_', 'P', 'V' }*/;                    //  + 6 bytes
  uint8_t major;
  uint8_t minor;                                                                  //  + 2 bytes 
  uint64_t pv_size_;                                                              //  + 8 bytes
  uint32_t cluster_size_;                                                         //  + 4 bytes
  uint32_t priority_;                                                             //  + 4 bytes
  uint64_t inverted_index_size_;                                                  //  + 8 bytes
});

STRUCT_PACK(
  template<typename OffsetType>
  struct FreelistHeader {
    OffsetType free_bins_[freelist_helper::FreelistHelper::kBinCount];
});

static_assert(32 == sizeof(PVHeader), "PVHeader should be 12 bytes long - please check aligments and type size on your setup");

constexpr uint32_t kVersionSize = sizeof(PVHeader::major) + sizeof(PVHeader::minor);
} // namespace pv_layout_types_headers
} // namespace yas
