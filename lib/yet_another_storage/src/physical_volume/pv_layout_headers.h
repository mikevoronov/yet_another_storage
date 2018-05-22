#pragma once
#include "../common/macros.h"
#include "../common/settings.hpp"
#include "../utils/Version.hpp"
#include <cstdint>

using namespace yas::macros;

namespace yas {
namespace pv_layout_headers {

constexpr uint32_t kBinCount = 11;

// PV layout contains of
// PVHeader
// Freelists
// Inverted index
// Data

STRUCT_PACK(
struct PVHeader {
  uint8_t signature[6] = { 'Y', 'A', 'S', '_', 'P', 'V' };                        //  + 6 bytes
  utils::Version version;                                                         //  + 2 bytes
  uint64_t pv_size_;                                                              //  + 8 bytes
  uint32_t cluster_size_;                                                         //  + 4 bytes
  uint32_t priority_;                                                             //  + 4 bytes
  uint32_t freelist_bins_count_;                                                  //  + 4 bytes
  uint32_t priority_;                                                             //  + 4 bytes
});

STRUCT_PACK(
template<typename OffsetType>
struct FreelistHeader {
  OffsetType free_bins_[kBinCount];
});

enum ValueType : uint16_t {
  kInt8 = 0,
  kUint8 = 1,
  kInt16 = 2,
  kUint16 = 3,
  kInt32 = 4,
  kUint32 = 5,
  kFloat = 6,
  kDouble = 7,
  kInt64 = 8,
  kUint64 = 9,
  kString = 10,
  kBlob = 11,

  kEmpty = 0x7FFF,        // high bit to determine is this value expired or not (tradeoff (types count)/performance
                          // because of expired_time_high - 2 bytes aligment)
};

// I assume that the most common types would be types with 4 and 8 bytes size. So there
// are specially size - optimized header for them.Each header could be in 2 states: allocated and freed.
// Allocated headers contain expired_time and data.Freed headers contain link instead of data. 
// This link points to the next freed header with the same size. This freelists then would used for
// allocating new entry in file (to decrease fragmentation and expensive extension of the physical volume on hdd).
// The idea stolen from (dl)ptmalloc fastbin realization.

STRUCT_PACK(
template <typename T>
struct CommonTypeHeader {
  ValueType value_type_;
  uint16_t expired_time_high;    // goodbye 2038 problem :)
  union {
    struct {
      uint32_t expired_time_low;
      T value;
    };
    OffsetType next_free_entry_offset_;
  };
});

STRUCT_PACK(
struct Simple4TypeHeader {
  CommonTypeHeader<uint32_t> body_;
});

STRUCT_PACK(
struct Simple8TypeHeader {
  CommonTypeHeader<uint64_t> body_;
});

STRUCT_PACK(
struct ComplexTypeHeader {
  ValueType value_type_;
  uint16_t expired_time_high;
  uint32_t expired_time_low;
  union {
    OffsetType next_free_entry_offset_;
    uint8_t data[1];
  };
});

constexpr uint32_t kTimeSize = sizeof(ComplexTypeHeader::expired_time_high) + sizeof(ComplexTypeHeader::expired_time_low);

static_assert(32 == sizeof(PVHeader), "PVHeader should be 12 bytes long - please check aligments and type size on your setup");
static_assert(12 == sizeof(Simple4TypeHeader), "Simple4TypeHeader should be 12 bytes long - please check aligments and type size on your setup");
static_assert(16 == sizeof(Simple8TypeHeader), "Simple8TypeHeader should be 16 bytes long - please check aligments and type size on your setup");
static_assert(16 == sizeof(ComplexTypeHeader), "ComplexTypeHeader should be 16 bytes long - please check aligments and type size on your setup");

} // namespace pv_layout_headers
} // namespace yas
