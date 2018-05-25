#pragma once
#include "gtest/gtest.h"
#include "physical_volume/inverted_index_helper/InvertedIndexHelper.hpp"
#include "utils/Version.hpp"

namespace {

using IndexHelper = yas::index_helper::InvertedIndexHelper<char, uint64_t>;

TEST(InvertedIndexHelper, BasicSerializeDeserializeTest) {
  IndexHelper helper;

  IndexHelper::key_type key_1 = "/home/user1/tmp1";
  IndexHelper::leaf_type value_1 = 10;
  IndexHelper::key_type key_2 = "/home/user1/tmp2";
  IndexHelper::leaf_type value_2 = 100;
  IndexHelper::key_type key_3 = "/home/user1/tmp3";
  IndexHelper::leaf_type value_3 = 50;
  IndexHelper::key_type key_4 = "/home/user1/tmp4";
  IndexHelper::leaf_type value_4 = -1;
  IndexHelper::key_type key_5 = "/home/user1/tmp5";
  IndexHelper::leaf_type value_5 = 154;

  helper.Insert(key_1, value_1);
  helper.Insert(key_2, value_2);
  helper.Insert(key_3, value_3);
  helper.Insert(key_4, value_4);
  helper.Insert(key_5, value_5);

  yas::utils::Version version = { 1,1 };
  auto data = helper.Serialize<uint32_t>({ 1,1 });
  auto engine = IndexHelper::Deserialize<uint32_t>(std::cbegin(data), std::cend(data), version);

  EXPECT_EQ(value_1, engine.Get(key_1));
  EXPECT_EQ(value_2, engine.Get(key_2));
  EXPECT_EQ(value_3, engine.Get(key_3));
  EXPECT_EQ(value_4, engine.Get(key_4));
  EXPECT_EQ(value_5, engine.Get(key_5));
}

TEST(InvertedIndexHelper, SerializeDeserializeTest) {
  IndexHelper helper;

  helper.Insert("/root", 1);
  helper.Insert("/home", 1);
  helper.Insert("/tmp", 1);
  helper.Insert("/var", 1);
  helper.Insert("/etc", 1);
  helper.Insert("/bin", 1);
  helper.Insert("/root/aa1", 1);
  helper.Insert("/root/aa1/bb1", 1);
  helper.Insert("/root/aa1/bb2", 1);
  helper.Insert("/root/aa3", 1);
  helper.Insert("/root/aa1/bb3", 1);
  helper.Insert("/root/aa1/bb3/cc1", 1);
  helper.Insert("/root/aa1/bb3/cc2", 1);
  helper.Insert("/root/aa1/bb1/cc1", 1);
  helper.Insert("/home/user1", 1);
  helper.Insert("/home/user2", 1);
  helper.Insert("/home/user3", 1);
  helper.Insert("/home/user4", 1);
  helper.Insert("/home/user5", 1);
  helper.Insert("/home/user6", 1);
  helper.Insert("/home/user1/file1", 1);
  helper.Insert("/home/user2/file2", 1);
  helper.Insert("/home/user3/file3", 1);
  helper.Insert("/home/user4/dir1", 1);
  helper.Insert("/home/user5/dir1", 1);
  helper.Insert("/home/user6/dir1/dir1", 1);
  helper.Insert("/home/user4/dir1/dir2", 1);
  helper.Insert("/home/user5/dir1/dir3", 1);
  helper.Insert("/home/user5/dir1/dir4", 1);
  helper.Insert("/home/user5/dir1/dir5", 1);
  helper.Insert("/home/user5/dir1/dir6", 1);
  helper.Insert("/home/user5/dir1/dir7", 1);
  helper.Insert("/home/user5/dir1/dir8", 1);
  helper.Insert("/home/user5/dir1/dir9", 1);
  helper.Insert("/home/user6/dir1/dir1/dir1", 1);
  helper.Insert("/home/user4/dir1/dir2/dir1", 1);
  helper.Insert("/home/user5/dir1/dir3/dir1", 1);
  helper.Insert("/home/user5/dir1/dir4/dir1", 1);
  helper.Insert("/home/user5/dir1/dir5/dir1", 1);
  helper.Insert("/home/user6/dir1/dir1/dir2", 1);
  helper.Insert("/home/user4/dir1/dir2/dir2", 1);
  helper.Insert("/home/user5/dir1/dir3/dir2", 1);
  helper.Insert("/home/user5/dir1/dir4/dir2", 1);
  helper.Insert("/home/user5/dir1/dir5/dir2", 1);

  yas::utils::Version version = { 1,1 };
  auto data = helper.Serialize<uint32_t>({ 1,1 });
  auto engine = IndexHelper::Deserialize<uint32_t>(std::cbegin(data), std::cend(data), version);

  EXPECT_EQ(1, engine.Get("/root"));
  EXPECT_EQ(1, engine.Get("/home"));
  EXPECT_EQ(1, engine.Get("/tmp"));
  EXPECT_EQ(1, engine.Get("/var"));
  EXPECT_EQ(1, engine.Get("/etc"));
  EXPECT_EQ(1, engine.Get("/bin"));
  EXPECT_EQ(1, engine.Get("/root/aa1"));
  EXPECT_EQ(1, engine.Get("/root/aa1/bb1"));
  EXPECT_EQ(1, engine.Get("/root/aa1/bb2"));
  EXPECT_EQ(1, engine.Get("/root/aa3"));
  EXPECT_EQ(1, engine.Get("/root/aa1/bb3"));
  EXPECT_EQ(1, engine.Get("/root/aa1/bb3/cc1"));
  EXPECT_EQ(1, engine.Get("/root/aa1/bb3/cc2"));
  EXPECT_EQ(1, engine.Get("/root/aa1/bb1/cc1"));
  EXPECT_EQ(1, engine.Get("/home/user1"));
  EXPECT_EQ(1, engine.Get("/home/user2"));
  EXPECT_EQ(1, engine.Get("/home/user3"));
  EXPECT_EQ(1, engine.Get("/home/user4"));
  EXPECT_EQ(1, engine.Get("/home/user5"));
  EXPECT_EQ(1, engine.Get("/home/user6"));
  EXPECT_EQ(1, engine.Get("/home/user1/file1"));
  EXPECT_EQ(1, engine.Get("/home/user2/file2"));
  EXPECT_EQ(1, engine.Get("/home/user3/file3"));
  EXPECT_EQ(1, engine.Get("/home/user4/dir1"));
  EXPECT_EQ(1, engine.Get("/home/user5/dir1"));
  EXPECT_EQ(1, engine.Get("/home/user6/dir1/dir1"));
  EXPECT_EQ(1, engine.Get("/home/user4/dir1/dir2"));
  EXPECT_EQ(1, engine.Get("/home/user5/dir1/dir3"));
  EXPECT_EQ(1, engine.Get("/home/user5/dir1/dir4"));
  EXPECT_EQ(1, engine.Get("/home/user5/dir1/dir5"));
  EXPECT_EQ(1, engine.Get("/home/user5/dir1/dir6"));
  EXPECT_EQ(1, engine.Get("/home/user5/dir1/dir7"));
  EXPECT_EQ(1, engine.Get("/home/user5/dir1/dir8"));
  EXPECT_EQ(1, engine.Get("/home/user5/dir1/dir9"));
  EXPECT_EQ(1, engine.Get("/home/user6/dir1/dir1/dir1"));
  EXPECT_EQ(1, engine.Get("/home/user4/dir1/dir2/dir1"));
  EXPECT_EQ(1, engine.Get("/home/user5/dir1/dir3/dir1"));
  EXPECT_EQ(1, engine.Get("/home/user5/dir1/dir4/dir1"));
  EXPECT_EQ(1, engine.Get("/home/user5/dir1/dir5/dir1"));
  EXPECT_EQ(1, engine.Get("/home/user6/dir1/dir1/dir2"));
  EXPECT_EQ(1, engine.Get("/home/user4/dir1/dir2/dir2"));
  EXPECT_EQ(1, engine.Get("/home/user5/dir1/dir3/dir2"));
  EXPECT_EQ(1, engine.Get("/home/user5/dir1/dir4/dir2"));
  EXPECT_EQ(1, engine.Get("/home/user5/dir1/dir5/dir2"));

  EXPECT_FALSE(engine.HasKey("/home1"));
  EXPECT_FALSE(engine.HasKey("/aaa"));
  EXPECT_FALSE(engine.HasKey("home"));
  EXPECT_FALSE(engine.HasKey("/home/user6/dir1/dir1/dir11"));
  EXPECT_FALSE(engine.HasKey("/home/user2/file1"));
  EXPECT_FALSE(engine.HasKey("/home/user1/file1/"));
  EXPECT_FALSE(engine.HasKey("/home/user1/file1/asd"));
  EXPECT_FALSE(engine.HasKey("/home/user5/dir1/dir4/dir33"));
  EXPECT_FALSE(engine.HasKey("/home/user6/dir1/di"));
  EXPECT_FALSE(engine.HasKey("/home/user5/dir1/dir"));
  EXPECT_FALSE(engine.HasKey("/home/userr"));
  EXPECT_FALSE(engine.HasKey("/root/aa1/bb"));
  EXPECT_FALSE(engine.HasKey("/bin/ls"));
}

TEST(InvertedIndexHelper, IsChangedTest) {
  IndexHelper helper_1;
  EXPECT_FALSE(helper_1.is_changed());

  helper_1.Get("/root");
  EXPECT_FALSE(helper_1.is_changed());

  helper_1.Insert("/root", 5);
  EXPECT_TRUE(helper_1.is_changed());

  utils::Version version(1, 1);
  const auto serialized_helper = helper_1.Serialize<uint32_t>(version);
  auto helper_2 = IndexHelper::Deserialize<uint32_t>(std::cbegin(serialized_helper), std::cend(serialized_helper), version);
  EXPECT_FALSE(helper_2.is_changed());

  helper_2.HasKey("/root");
  EXPECT_FALSE(helper_2.is_changed());

  helper_2.Delete("/root");
  EXPECT_TRUE(helper_2.is_changed());
}

}