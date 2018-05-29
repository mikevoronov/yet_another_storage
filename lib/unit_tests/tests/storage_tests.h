#pragma once
#include "gtest/gtest.h"
#include "storage/Storage.hpp"

namespace {

TEST(Storage, MountTest) {
  yas::storage::Storage<char> storage;
  using TestType = uint32_t;
  const TestType test_value = 5;
  const auto pv_path = fs::temp_directory_path() / "pv21";

  auto &factory = PVManagerFactory::Instance();
  auto manager = factory.Create(pv_path, kMaximumSupportedVersion);
  EXPECT_TRUE(manager);

  auto pv_manager = manager.value();
  pv_manager->Put("/home/user1/dir1", std::make_any<TestType>(test_value));

  storage.Mount(pv_path, "/home/user2/dir2", "/home/user1");
  storage.Put("/home/user2/dir2/dir3", test_value);
}


}
