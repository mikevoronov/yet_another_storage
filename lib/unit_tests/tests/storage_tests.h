#pragma once
#include "gtest/gtest.h"
#include "storage/Storage.hpp"

// (!!!!) Some of these tests write files to tmp directory

namespace {

TEST(Storage, MountTest_1) {
  yas::storage::Storage<char> storage;
  using TestType = uint32_t;
  const TestType test_value = 5;
  const auto pv_path = fs::temp_directory_path() / "yas_pv_fd60f6e1ae21d37aa1e10007636431aas_1";

  auto &factory = PVManagerFactory::Instance();
  auto manager = factory.Create(pv_path, kMaximumSupportedVersion);
  EXPECT_TRUE(manager);

  auto pv_manager = manager.value();
  pv_manager->Put("/home/user1/dir1", std::make_any<TestType>(test_value));

  storage.Mount(pv_path, "/home/user2/dir2", "/home/user1");
  storage.Put("/home/user2/dir2/dir3", test_value);
  const auto result = storage.Get("/home/user2/dir2/dir3");
  EXPECT_TRUE(result);
  EXPECT_EQ(test_value, std::any_cast<TestType>(result.value()));
}

TEST(Storage, MountTest_2) {
  yas::storage::Storage<char> storage;
  using TestType = uint32_t;
  const TestType test_value_1 = 5;
  const TestType test_value_2 = 7;
  const auto pv_path_1 = fs::temp_directory_path() / "yas_pv_fd60f6e1ae21d37aa1e10007636431aas_2";
  const auto pv_path_2 = fs::temp_directory_path() / "yas_pv_fd60f6e1ae21d37aa1e10007636431aas_3";

  auto &factory = PVManagerFactory::Instance();
  auto manager_1 = factory.Create(pv_path_1, kMaximumSupportedVersion, 10);
  auto manager_2 = factory.Create(pv_path_2, kMaximumSupportedVersion, 20);
  EXPECT_TRUE(manager_1);
  EXPECT_TRUE(manager_2);

  auto pv_manager_1 = manager_1.value();
  auto pv_manager_2 = manager_2.value();
  pv_manager_1->Put("/home/user1/dir1", std::make_any<TestType>(test_value_1));
  pv_manager_2->Put("/root/user1/dir1", std::make_any<TestType>(test_value_2));

  storage.Mount(pv_path_1, "/home/user2/dir2", "/home/user1");
  storage.Mount(pv_path_2, "/home/user2/dir2", "/root/user1");
  const auto result = storage.Get("/home/user2/dir2/dir1");
  EXPECT_TRUE(result);
  // test_value_2 is in storage with a higher priority
  EXPECT_EQ(test_value_2, std::any_cast<TestType>(result.value()));
}

}
