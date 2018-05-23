#pragma once
#include "gtest/gtest.h"
#include "device_worker/devices/TestDevice.hpp"
#include "common/common.h"

namespace {
TEST(TestDevice, ReadTest) {
  yas::devices::TestDevice<uint64_t> test_device("/root");
  
  yas::ByteVector vector = {'\x00', '\x01', '\x02', '\x04', '\x05'};

  test_device.SetStorageContent(std::cbegin(vector), std::cend(vector));
  const auto result = test_device.Read(0, vector.size());

  EXPECT_EQ(vector.size(), result.size());
  auto result_it = std::cbegin(result);
  for (auto it = std::cbegin(vector), end = std::cend(vector); it != end; ++it) {
    EXPECT_EQ(*it, *result_it);
    ++result_it;
  }
}

TEST(TestDevice, WriteToEndEmptyFileTest) {
  yas::devices::TestDevice<uint64_t> test_device("/root");

  yas::ByteVector vector = { '\x00', '\x01', '\x02', '\x04', '\x05' };
  test_device.Write(0, std::cbegin(vector), std::cend(vector));

  const auto result = test_device.Read(0, vector.size());

  EXPECT_EQ(vector.size(), result.size());
  auto result_it = std::cbegin(result);
  for (auto it = std::cbegin(vector), end = std::cend(vector); it != end; ++it) {
    EXPECT_EQ(*it, *result_it);
    ++result_it;
  }
}

TEST(TestDevice, WriteToEndNonEmptyFileTest) {
  yas::devices::TestDevice<uint64_t> test_device("/root");

  yas::ByteVector vector = { '\x00', '\x01', '\x02', '\x04', '\x05' };
  test_device.SetStorageContent(std::cbegin(vector), std::cend(vector));
  test_device.Write(5, std::cbegin(vector), std::cend(vector));

  const auto result = test_device.Read(5, vector.size());

  EXPECT_EQ(vector.size(), result.size());
  auto result_it = std::cbegin(result);
  for (auto it = std::cbegin(vector), end = std::cend(vector); it != end; ++it) {
    EXPECT_EQ(*it, *result_it);
    ++result_it;
  }
}

TEST(TestDevice, WriteToMiddleFileTest) {
  yas::devices::TestDevice<uint64_t> test_device("/root");

  yas::ByteVector vector = { '\x00', '\x01', '\x02', '\x04', '\x05' };
  test_device.SetStorageContent(std::cbegin(vector), std::cend(vector));
  test_device.Write(3, std::cbegin(vector), std::cend(vector));

  const auto result = test_device.Read(3, vector.size());

  EXPECT_EQ(vector.size(), result.size());
  auto result_it = std::cbegin(result);
  for (auto it = std::cbegin(vector), end = std::cend(vector); it != end; ++it) {
    EXPECT_EQ(*it, *result_it);
    ++result_it;
  }
}

}
