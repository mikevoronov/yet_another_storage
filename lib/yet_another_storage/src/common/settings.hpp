#pragma once
#include "../utils/Version.hpp"
#include <cstdint>

// TODO : should specified in the build script
namespace yas {

// pointer type to navigate through PV
using OffsetType = uintptr_t;

// type of path char
using CharType = char;

// there must be some maximum type size
constexpr uint64_t kMaximumTypeSize = 400 * 1024 * 1024; // 400 Mb

// 3840 - to guaranteed fit in page size on x86/amd64
constexpr uint32_t kDefaultClusterSize = 3840;

constexpr utils::Version max_supported_version(1, 1);

} // namespace yas
