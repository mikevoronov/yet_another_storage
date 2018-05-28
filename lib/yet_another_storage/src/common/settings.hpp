#pragma once
#include "../utils/Version.hpp"
#include <cstdint>

namespace yas {

// pointer type to navigate through PV (compeltely determines the maximum size of volume)
using OffsetType = uintptr_t;

// type of key chars in storage
using CharType = char;

// there must be some maximum type size
constexpr uint64_t kMaximumTypeSize = 400 * 1024 * 1024; // 400 Mb

// 3840 - to guaranteed fit in page size on x86/amd64
constexpr uint32_t kDefaultClusterSize = 3840;

constexpr utils::Version kMaximumSupportedVersion(1, 1);

} // namespace yas
