#pragma once
#include "storage/lib/physical_volume/FreelistHelper.hpp"
#include <iostream>

using namespace yas::freelist_helper;

namespace {

using FreelistHelperType = FreelistHelper<yas::DOffsetType>;

TEST(FreelistHelper, BasicPushPopTest) {
  FreelistHelperType helper;

  for (uint32_t bin_id = 0; bin_id < kBinCount; ++bin_id) {
    helper.PushFreeEntry(bin_id * 0x10101010, FreelistHelperType::kFreelistLimits[bin_id] - 5);
    EXPECT_EQ(helper.PopFreeEntryOffset(FreelistHelperType::kFreelistLimits[bin_id] - 5), bin_id * 0x10101010);
  }
}

}
