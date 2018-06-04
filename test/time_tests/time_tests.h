#pragma once
#include "storage/lib/utils/Time.hpp"
#include <limits>

using namespace yas;

namespace {

TEST(Time, BasicTest) {
  utils::Time current_time(time(nullptr));
  utils::Time previous_time(time(nullptr) - 100);

  EXPECT_TRUE(previous_time < current_time);
}

TEST(Time, InfiniteTimeTest) {
  utils::Time current_time(time(nullptr));
  utils::Time infinite_time(std::numeric_limits<time_t>::max());

  EXPECT_TRUE(current_time < infinite_time);
}

TEST(Time, ExpiredTest) {
  utils::Time current_time(time(nullptr)-100);

  EXPECT_TRUE(current_time.IsExpired());
}

}
