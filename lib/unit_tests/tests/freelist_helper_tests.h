#pragma once
#include "gtest/gtest.h"
#include "physical_volume/freelist_helper/FreelistHelper.hpp"

using namespace yas::freelist_helper;

namespace {

using FreelistHelperType = FreelistHelper<yas::OffsetType>;

TEST(FreelistHelper, BasicPushPopTest) {
  FreelistHelperType helper;

  for (int bin_id = 0; bin_id < kBinCount; ++bin_id) {
    helper.PushFreeEntry(bin_id * 0x10101010, kFreelistLimits[bin_id] - 5);
    EXPECT_EQ(helper.PopFreeEntryOffset(kFreelistLimits[bin_id] - 5), bin_id * 0x10101010);
  }
}

}
