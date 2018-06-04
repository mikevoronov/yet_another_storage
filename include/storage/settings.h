#pragma once
#include "lib/utils/Version.hpp"
#include "lib/devices/FileDevice.hpp"
#include <cstdint>

namespace yas {

// pointer type to navigate through PV (completely determines the maximum size of volume)
using DOffsetType = uintptr_t;

// type of key chars in storage
using DCharType = char;

// device that would be used to access PV
using DDevice = devices::FileDevice<DOffsetType>;

// there must be some maximum type size
constexpr uint64_t kMaximumTypeSize = 400 * 1024 * 1024; // 400 Mb

// 3840 - to guaranteed fit in page size on x86/amd64
constexpr uint32_t kDefaultClusterSize = 3840;

constexpr utils::Version kMaximumSupportedVersion(1, 2);

} // namespace yas
