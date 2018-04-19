#pragma once
#include "../../common/macros.h"
#include "../../common/settings.hpp"
#include <cstdint>

using namespace yas::macros;

namespace yas {
namespace pv_layout_headers {

STRUCT_PACK(
struct PVHeader {
  uint8_t signature[] = { 'Y', 'A', 'S', '_', 'P', 'V' };                         //  + 6 bytes
  uint8_t major;
  uint8_t minor;                                                                  //  + 2 bytes 
  uint64_t pv_size_;                                                              //  + 8 bytes
  uint32_t cluster_size_;                                                         //  + 4 bytes
  uint32_t priority_;               // should be class (?)                            + 4 bytes
  OffsetType root_entry_offset_;                                                  //  + 8 bytes
  // freelists
  //
  // some index
  //
  // data

});

// TODO : add static assert to size of PVHeader

constexpr uint32_t kVersionSize = sizeof(PVHeader::major) + sizeof(PVHeader::minor);
} // namespace pv_layout_types_headers
} // namespace yas
