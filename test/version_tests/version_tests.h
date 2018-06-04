#pragma once
#include "storage/lib/utils/Version.hpp"

using namespace yas;

namespace {

TEST(Version, BasicTest1) {
  utils::Version version_1(1, 2);
  utils::Version version_2(1, 2);

  EXPECT_TRUE(version_1 == version_2);
}

TEST(Version, BasicTest2) {
  utils::Version version_1(1, 2);
  utils::Version version_2(2, 1);

  EXPECT_TRUE(version_1 < version_2);
}

TEST(Version, BasicTest3) {
  utils::Version version_1(1, 2);
  utils::Version version_2(1, 3);

  EXPECT_TRUE(version_1 < version_2);
}

}
