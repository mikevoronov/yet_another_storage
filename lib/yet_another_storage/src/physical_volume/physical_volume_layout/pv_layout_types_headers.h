#pragma once
#include "../../common/macros.h"
#include "../../common/settings.hpp"
#include <cstdint>

using namespace yas::macros;

namespace yas {
namespace pv_layout_types_headers {

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
struct Simple4TypeHeader {           // 12 bytes (follow the requirements of the lowest physical volume size)
  CommonTypeHeader<uint32_t> body_;
});

STRUCT_PACK(
struct Simple8TypeHeader {           // 16 bytes (follow the requirements of the lowest physical volume size)
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
static_assert(12 == sizeof(Simple4TypeHeader), "Simple4TypeHeader should be 12 bytes long - please check aligments and type size on your setup");
static_assert(16 == sizeof(Simple8TypeHeader), "Simple8TypeHeader should be 16 bytes long - please check aligments and type size on your setup");
static_assert(16 == sizeof(ComplexTypeHeader), "ComplexTypeHeader should be 16 bytes long - please check aligments and type size on your setup");

} // namespace pv_layout_types_headers
} // namespace yas
