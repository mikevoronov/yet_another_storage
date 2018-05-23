#pragma once
#include "gtest/gtest.h"
#include "physical_volume/inverted_index_helper/AhoCorasickEngine.hpp"
#include "physical_volume/inverted_index_helper/leaf_type_traits.hpp"
#include <string_view>

namespace {
TEST(AhoCorasickEngine, BasicInsertTest) {
  using Engine = yas::index_helper::AhoCorasickEngine<char, uint64_t>;
  Engine engine;

  Engine::key_type key_1 = "/home/user1/tmp1";
  Engine::leaf_type value_1 = 10;
  Engine::key_type key_2 = "/home/user1/tmp2";
  Engine::leaf_type value_2 = 100;
  Engine::key_type key_3 = "/home/user1/tmp3";
  Engine::leaf_type value_3 = 50;
  Engine::key_type key_4 = "/home/user1/tmp4";
  Engine::leaf_type value_4 = -1;
  Engine::key_type key_5 = "/home/user1/tmp5";
  Engine::leaf_type value_5 = 154;

  engine.Insert(key_1, value_1);
  engine.Insert(key_2, value_2);
  engine.Insert(key_3, value_3);
  engine.Insert(key_4, value_4);
  engine.Insert(key_5, value_5);

  EXPECT_EQ(value_1, engine.Get(key_1));
  EXPECT_EQ(value_2, engine.Get(key_2));
  EXPECT_EQ(value_3, engine.Get(key_3));
  EXPECT_EQ(value_4, engine.Get(key_4));
  EXPECT_EQ(value_5, engine.Get(key_5));
}

TEST(AhoCorasickEngine, InsertTestConstGet) {
  class Test {
   public:

    void foo() const {
      using Engine = yas::index_helper::AhoCorasickEngine<char, uint64_t>;
      Engine engine;

      Engine::key_type key_1 = "/home/user1/tmp1";
      Engine::leaf_type value_1 = 10;
      Engine::key_type key_2 = "/home/user1/tmp2";
      Engine::leaf_type value_2 = 100;
      Engine::key_type key_3 = "/home/user1/tmp3";
      Engine::leaf_type value_3 = 50;
      Engine::key_type key_4 = "/home/user1/tmp4";
      Engine::leaf_type value_4 = -1;
      Engine::key_type key_5 = "/home/user1/tmp5";
      Engine::leaf_type value_5 = 154;

      engine.Insert(key_1, value_1);
      engine.Insert(key_2, value_2);
      engine.Insert(key_3, value_3);
      engine.Insert(key_4, value_4);
      engine.Insert(key_5, value_5);

      EXPECT_EQ(value_1, engine.Get(key_1));
      EXPECT_EQ(value_2, engine.Get(key_2));
      EXPECT_EQ(value_3, engine.Get(key_3));
      EXPECT_EQ(value_4, engine.Get(key_4));
      EXPECT_EQ(value_5, engine.Get(key_5));
    }
  };

  Test t;
  t.foo();
}

TEST(AhoCorasickEngine, InsertTestWithDifferentRootPath) {
  using Engine = yas::index_helper::AhoCorasickEngine<char, uint64_t>;
  Engine engine;

  Engine::key_type key_1 = "ahome/user1/tmp1";
  Engine::leaf_type value_1 = 10;
  Engine::key_type key_2 = "bhome/user1/tmp2";
  Engine::leaf_type value_2 = 100;
  Engine::key_type key_3 = "chome/user1/tmp3";
  Engine::leaf_type value_3 = 50;
  Engine::key_type key_4 = "dhome/user1/tmp4";
  Engine::leaf_type value_4 = -1;
  Engine::key_type key_5 = "ehome/user1/tmp5";
  Engine::leaf_type value_5 = 154;

  engine.Insert(key_1, value_1);
  engine.Insert(key_2, value_2);
  engine.Insert(key_3, value_3);
  engine.Insert(key_4, value_4);
  engine.Insert(key_5, value_5);

  EXPECT_EQ(value_1, engine.Get(key_1));
  EXPECT_EQ(value_2, engine.Get(key_2));
  EXPECT_EQ(value_3, engine.Get(key_3));
  EXPECT_EQ(value_4, engine.Get(key_4));
  EXPECT_EQ(value_5, engine.Get(key_5));
}

TEST(AhoCorasickEngine, BasicDeleteTest) {
  using Engine = yas::index_helper::AhoCorasickEngine<char, uint64_t>;
  Engine engine;

  Engine::key_type key_1 = "/home/user1/tmp1";
  Engine::leaf_type value_1 = 10;
  Engine::key_type key_2 = "/home/user1/tmp2";
  Engine::leaf_type value_2 = 100;
  Engine::key_type key_3 = "/home/user1/tmp3";
  Engine::leaf_type value_3 = 50;
  Engine::key_type key_4 = "/home/user1/tmp4";
  Engine::leaf_type value_4 = -1;
  Engine::key_type key_5 = "/home/user1/tmp5";
  Engine::leaf_type value_5 = 154;

  engine.Insert(key_1, value_1);
  engine.Insert(key_2, value_2);
  engine.Insert(key_3, value_3);
  engine.Insert(key_4, value_4);
  engine.Insert(key_5, value_5);

  engine.Delete(key_1);
  engine.Delete(key_3);
  engine.Delete(key_5);

  Engine::leaf_type non_exist_value = yas::index_helper::leaf_traits<Engine::leaf_type>::NonExistValue();
  EXPECT_EQ(non_exist_value, engine.Get(key_1));
  EXPECT_EQ(value_2, engine.Get(key_2));
  EXPECT_EQ(non_exist_value, engine.Get(key_3));
  EXPECT_EQ(value_4, engine.Get(key_4));
  EXPECT_EQ(non_exist_value, engine.Get(key_5));
}

TEST(AhoCorasickEngine, DeleteRootTest) {
  using Engine = yas::index_helper::AhoCorasickEngine<char, uint64_t>;
  Engine engine;

  Engine::key_type key_1 = "/home/user1/tmp1";
  Engine::leaf_type value_1 = 10;

  engine.Insert(key_1, value_1);
  engine.Delete(key_1);

  EXPECT_EQ(yas::index_helper::leaf_traits<Engine::leaf_type>::NonExistValue(), engine.Get(key_1));
  EXPECT_EQ(false, engine.HasKey(key_1));
  EXPECT_EQ(false, engine.HasKey("/home/user1/"));
  EXPECT_EQ(false, engine.HasKey("/home/"));
}

}
