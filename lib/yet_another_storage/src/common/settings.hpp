#pragma once
#include <cstdint>

namespace yas {

constexpr uint32_t kMaxPath = 1024;
constexpr uint32_t kKeyNameMax = 128;

// this would be using as pointer type in storage
using OffsetType = uint64_t;      

} // namespace yas
