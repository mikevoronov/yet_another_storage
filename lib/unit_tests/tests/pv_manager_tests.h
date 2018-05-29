#pragma once
#include "gtest/gtest.h"
#include "storage/PVManagerFactory.hpp"

using namespace yas;

// (!!!!) Some of these tests write files to tmp directory

namespace {

#define UNIT_TEST

TEST(PVManager, PutGetInt8) {
  using TestType = int8_t;
  const TestType test_value = 1;
  const std::basic_string<CharType> key = "/root/asd1";
  const auto pv_path = fs::temp_directory_path() / "yas_pv_fd60f6e1ae21d37aa1e10007636431ab_1";

  auto &factory = PVManagerFactory::Instance();
  auto manager = factory.Create(pv_path, kMaximumSupportedVersion);
  EXPECT_TRUE(manager);

  auto pv_manager = manager.value();
  pv_manager->Put(key, std::make_any<TestType>(test_value));
  auto result = pv_manager->Get(key);
  EXPECT_TRUE(result);

  auto result_value = result.value();
  EXPECT_TRUE(result_value.has_value());
  EXPECT_EQ(std::type_index(result_value.type()), std::type_index(typeid(TestType)));
  EXPECT_EQ(test_value, std::any_cast<TestType>(result_value));
}

TEST(PVManager, PutGetUInt8) {
  using TestType = uint8_t;
  const TestType test_value = 1;
  const std::basic_string<CharType> key = "/root/asd2";
  const auto pv_path = fs::temp_directory_path() / "yas_pv_fd60f6e1ae21d37aa1e10007636431ab_2";

  auto &factory = PVManagerFactory::Instance();
  auto manager = factory.Create(pv_path, kMaximumSupportedVersion);
  EXPECT_TRUE(manager);

  auto pv_manager = manager.value();
  pv_manager->Put(key, std::make_any<TestType>(test_value));
  auto result = pv_manager->Get(key);
  EXPECT_TRUE(result);

  auto result_value = result.value();
  EXPECT_TRUE(result_value.has_value());
  EXPECT_EQ(std::type_index(result_value.type()), std::type_index(typeid(TestType)));
  EXPECT_EQ(test_value, std::any_cast<TestType>(result_value));
}

TEST(PVManager, PutGetInt16) {
  using TestType = int16_t;
  const TestType test_value = 1;
  const std::basic_string<CharType> key = "/root/asd3";
  const auto pv_path = fs::temp_directory_path() / "yas_pv_fd60f6e1ae21d37aa1e10007636431ab_3";

  auto &factory = PVManagerFactory::Instance();
  auto manager = factory.Create(pv_path, kMaximumSupportedVersion);
  EXPECT_TRUE(manager);

  auto pv_manager = manager.value();
  pv_manager->Put(key, std::make_any<TestType>(test_value));
  auto result = pv_manager->Get(key);
  EXPECT_TRUE(result);

  auto result_value = result.value();
  EXPECT_TRUE(result_value.has_value());
  EXPECT_EQ(std::type_index(result_value.type()), std::type_index(typeid(TestType)));
  EXPECT_EQ(test_value, std::any_cast<TestType>(result_value));
}

TEST(PVManager, PutGetUInt16) {
  using TestType = uint16_t;
  const TestType test_value = 1;
  const std::basic_string<CharType> key = "/root/asd4";
  const auto pv_path = fs::temp_directory_path() / "yas_pv_fd60f6e1ae21d37aa1e10007636431ab_4";

  auto &factory = PVManagerFactory::Instance();
  auto manager = factory.Create(pv_path, kMaximumSupportedVersion);
  EXPECT_TRUE(manager);

  auto pv_manager = manager.value();
  pv_manager->Put(key, std::make_any<TestType>(test_value));
  auto result = pv_manager->Get(key);
  EXPECT_TRUE(result);

  auto result_value = result.value();
  EXPECT_TRUE(result_value.has_value());
  EXPECT_EQ(std::type_index(result_value.type()), std::type_index(typeid(TestType)));
  EXPECT_EQ(test_value, std::any_cast<TestType>(result_value));
}

TEST(PVManager, PutGetInt32) {
  using TestType = int32_t;
  const TestType test_value = 1;
  const std::basic_string<CharType> key = "/root/asd5";
  const auto pv_path = fs::temp_directory_path() / "yas_pv_fd60f6e1ae21d37aa1e10007636431ab_5";

  auto &factory = PVManagerFactory::Instance();
  auto manager = factory.Create(pv_path, kMaximumSupportedVersion);
  EXPECT_TRUE(manager);

  auto pv_manager = manager.value();
  pv_manager->Put(key, std::make_any<TestType>(test_value));
  auto result = pv_manager->Get(key);
  EXPECT_TRUE(result);

  auto result_value = result.value();
  EXPECT_TRUE(result_value.has_value());
  EXPECT_EQ(std::type_index(result_value.type()), std::type_index(typeid(TestType)));
  EXPECT_EQ(test_value, std::any_cast<TestType>(result_value));
}

TEST(PVManager, PutGetUInt32) {
  using TestType = uint32_t;
  const TestType test_value = 1;
  const std::basic_string<CharType> key = "/root/asd6";
  const auto pv_path = fs::temp_directory_path() / "yas_pv_fd60f6e1ae21d37aa1e10007636431ab_6";

  auto &factory = PVManagerFactory::Instance();
  auto manager = factory.Create(pv_path, kMaximumSupportedVersion);
  EXPECT_TRUE(manager);

  auto pv_manager = manager.value();
  pv_manager->Put(key, std::make_any<TestType>(test_value));
  auto result = pv_manager->Get(key);
  EXPECT_TRUE(result);

  auto result_value = result.value();
  EXPECT_TRUE(result_value.has_value());
  EXPECT_EQ(std::type_index(result_value.type()), std::type_index(typeid(TestType)));
  EXPECT_EQ(test_value, std::any_cast<TestType>(result_value));
}

TEST(PVManager, PutGetFloat1) {
  using TestType = float;
  const TestType test_value = 1.101010101f;
  const std::basic_string<CharType> key = "/root/asd7";
  const auto pv_path = fs::temp_directory_path() / "yas_pv_fd60f6e1ae21d37aa1e10007636431ab_7";

  auto &factory = PVManagerFactory::Instance();
  auto manager = factory.Create(pv_path, kMaximumSupportedVersion);
  EXPECT_TRUE(manager);

  auto pv_manager = manager.value();
  pv_manager->Put(key, std::make_any<TestType>(test_value));
  auto result = pv_manager->Get(key);
  EXPECT_TRUE(result);

  auto result_value = result.value();
  EXPECT_TRUE(result_value.has_value());
  EXPECT_EQ(std::type_index(result_value.type()), std::type_index(typeid(TestType)));
  EXPECT_EQ(test_value, std::any_cast<TestType>(result_value));
}

TEST(PVManager, PutGetFloat2) {
  using TestType = float;
  const TestType test_value = std::numeric_limits<TestType>::max();
  const std::basic_string<CharType> key = "/root/asd8";
  const auto pv_path = fs::temp_directory_path() / "yas_pv_fd60f6e1ae21d37aa1e10007636431ab_8";

  auto &factory = PVManagerFactory::Instance();
  auto manager = factory.Create(pv_path, kMaximumSupportedVersion);
  EXPECT_TRUE(manager);

  auto pv_manager = manager.value();
  pv_manager->Put(key, std::make_any<TestType>(test_value));
  auto result = pv_manager->Get(key);
  EXPECT_TRUE(result);

  auto result_value = result.value();
  EXPECT_TRUE(result_value.has_value());
  EXPECT_EQ(std::type_index(result_value.type()), std::type_index(typeid(TestType)));
  EXPECT_EQ(test_value, std::any_cast<TestType>(result_value));
}

TEST(PVManager, PutGetDouble1) {
  using TestType = double;
  const TestType test_value = 1.101010101;
  const std::basic_string<CharType> key = "/root/asd9";
  const auto pv_path = fs::temp_directory_path() / "yas_pv_fd60f6e1ae21d37aa1e10007636431ab_9";

  auto &factory = PVManagerFactory::Instance();
  auto manager = factory.Create(pv_path, kMaximumSupportedVersion);
  EXPECT_TRUE(manager);

  auto pv_manager = manager.value();
  pv_manager->Put(key, std::make_any<TestType>(test_value));
  auto result = pv_manager->Get(key);
  EXPECT_TRUE(result);

  auto result_value = result.value();
  EXPECT_TRUE(result_value.has_value());
  EXPECT_EQ(std::type_index(result_value.type()), std::type_index(typeid(TestType)));
  EXPECT_EQ(test_value, std::any_cast<TestType>(result_value));
}

TEST(PVManager, PutGetDouble2) {
  using TestType = double;
  const TestType test_value = std::numeric_limits<TestType>::max();
  const std::basic_string<CharType> key = "/root/asd10";
  const auto pv_path = fs::temp_directory_path() / "yas_pv_fd60f6e1ae21d37aa1e10007636431ab_10";

  auto &factory = PVManagerFactory::Instance();
  auto manager = factory.Create(pv_path, kMaximumSupportedVersion);
  EXPECT_TRUE(manager);

  auto pv_manager = manager.value();
  pv_manager->Put(key, std::make_any<TestType>(test_value));
  auto result = pv_manager->Get(key);
  EXPECT_TRUE(result);

  auto result_value = result.value();
  EXPECT_TRUE(result_value.has_value());
  EXPECT_EQ(std::type_index(result_value.type()), std::type_index(typeid(TestType)));
  EXPECT_EQ(test_value, std::any_cast<TestType>(result_value));
}

TEST(PVManager, PutGetInt64) {
  using TestType = int64_t;
  const TestType test_value = 1;
  const std::basic_string<CharType> key = "/root/asd11";
  const auto pv_path = fs::temp_directory_path() / "yas_pv_fd60f6e1ae21d37aa1e10007636431ab_11";

  auto &factory = PVManagerFactory::Instance();
  auto manager = factory.Create(pv_path, kMaximumSupportedVersion);
  EXPECT_TRUE(manager);

  auto pv_manager = manager.value();
  pv_manager->Put(key, std::make_any<TestType>(test_value));
  auto result = pv_manager->Get(key);
  EXPECT_TRUE(result);

  auto result_value = result.value();
  EXPECT_TRUE(result_value.has_value());
  EXPECT_EQ(std::type_index(result_value.type()), std::type_index(typeid(TestType)));
  EXPECT_EQ(test_value, std::any_cast<TestType>(result_value));
}

TEST(PVManager, PutGetUInt64) {
  using TestType = uint64_t;
  const TestType test_value = 1;
  const std::basic_string<CharType> key = "/root/asd12";
  const auto pv_path = fs::temp_directory_path() / "yas_pv_fd60f6e1ae21d37aa1e10007636431ab_12";

  auto &factory = PVManagerFactory::Instance();
  auto manager = factory.Create(pv_path, kMaximumSupportedVersion);
  EXPECT_TRUE(manager);

  auto pv_manager = manager.value();
  pv_manager->Put(key, std::make_any<TestType>(test_value));
  auto result = pv_manager->Get(key);
  EXPECT_TRUE(result);

  auto result_value = result.value();
  EXPECT_TRUE(result_value.has_value());
  EXPECT_EQ(std::type_index(result_value.type()), std::type_index(typeid(TestType)));
  EXPECT_EQ(test_value, std::any_cast<TestType>(result_value));
}

TEST(PVManager, PutGetString) {
  using TestType = std::string;
  const TestType test_value("aaaaaaaaaaaaaaaaaaa");
  const std::basic_string<CharType> key = "/root/asd13";
  const auto pv_path = fs::temp_directory_path() / "yas_pv_fd60f6e1ae21d37aa1e10007636431ab_13";

  auto &factory = PVManagerFactory::Instance();
  auto manager = factory.Create(pv_path, kMaximumSupportedVersion);
  EXPECT_TRUE(manager);

  auto pv_manager = manager.value();
  pv_manager->Put(key, std::make_any<TestType>(test_value));
  auto result = pv_manager->Get(key);
  EXPECT_TRUE(result);

  auto result_value = result.value();
  EXPECT_TRUE(result_value.has_value());
  EXPECT_EQ(std::type_index(result_value.type()), std::type_index(typeid(TestType)));
  EXPECT_EQ(test_value, std::any_cast<TestType>(result_value));
}

TEST(PVManager, PutGetBlob) {
  using TestType = ByteVector;
  const TestType test_value = {'\x00', '\x01', '\x02' , '\x03' , '\x04' , '\x05' };
  const std::basic_string<CharType> key = "/root/asd15";
  const auto pv_path = fs::temp_directory_path() / "yas_pv_fd60f6e1ae21d37aa1e10007636431ab_14";

  auto &factory = PVManagerFactory::Instance();
  auto manager = factory.Create(pv_path, kMaximumSupportedVersion);
  EXPECT_TRUE(manager);

  auto pv_manager = manager.value();
  pv_manager->Put(key, std::make_any<TestType>(test_value));
  auto result = pv_manager->Get(key);
  EXPECT_TRUE(result);

  auto result_value = result.value();
  EXPECT_TRUE(result_value.has_value());
  EXPECT_EQ(std::type_index(result_value.type()), std::type_index(typeid(TestType)));
  EXPECT_EQ(test_value, std::any_cast<TestType>(result_value));
}

TEST(PVManager, PutDeleteGetBlob) {
  using TestType = ByteVector;
  const TestType test_value = { '\x00', '\x01', '\x02' , '\x03' , '\x04' , '\x05' };
  const std::basic_string<CharType> key = "/root/asd16";
  const auto pv_path = fs::temp_directory_path() / "yas_pv_fd60f6e1ae21d37aa1e10007636431ab_15";

  auto &factory = PVManagerFactory::Instance();
  auto manager = factory.Create(pv_path, kMaximumSupportedVersion);
  EXPECT_TRUE(manager);

  auto pv_manager = manager.value();
  pv_manager->Put(key, std::make_any<TestType>(test_value));
  pv_manager->Delete(key);
  auto result = pv_manager->Get(key);
  EXPECT_FALSE(result);
}

TEST(PVManager, PutDeleteGetBigBlob) {
  using TestType = ByteVector;
  const TestType test_value(kMaximumTypeSize - 100, '\x41');
  const std::basic_string<CharType> key = "/root/asd17";
  const auto pv_path = fs::temp_directory_path() / "yas_pv_fd60f6e1ae21d37aa1e10007636431ab_16";

  auto &factory = PVManagerFactory::Instance();
  auto manager = factory.Create(pv_path, kMaximumSupportedVersion);
  EXPECT_TRUE(manager);

  auto pv_manager = manager.value();
  pv_manager->Put(key, std::make_any<TestType>(test_value));
  pv_manager->Delete(key);
  auto result = pv_manager->Get(key);
  EXPECT_FALSE(result);
}

TEST(PVManager, PutGetBigBlob) {
  using TestType = ByteVector;
  const TestType test_value(0x1000*2, '\x41');
  const std::basic_string<CharType> key = "/root/asd17";
  const auto pv_path = fs::temp_directory_path() / "yas_pv_fd60f6e1ae21d37aa1e10007636431ab_17";

  auto &factory = PVManagerFactory::Instance();
  auto manager = factory.Create(pv_path, kMaximumSupportedVersion);
  EXPECT_TRUE(manager);

  auto pv_manager = manager.value();
  pv_manager->Put(key, std::make_any<TestType>(test_value));
  auto result = pv_manager->Get(key);
  EXPECT_TRUE(result);

  auto result_value = result.value();
  EXPECT_TRUE(result_value.has_value());
  EXPECT_EQ(std::type_index(result_value.type()), std::type_index(typeid(TestType)));
  EXPECT_EQ(test_value, std::any_cast<TestType>(result_value));
}

TEST(PVManager, HasKeyTest) {
  using TestType = ByteVector;
  const TestType test_value(0x100, '\x41');
  const std::basic_string<CharType> key = "/root/asd18";
  const auto pv_path = fs::temp_directory_path() / "yas_pv_fd60f6e1ae21d37aa1e10007636431ab_18";

  auto &factory = PVManagerFactory::Instance();
  auto manager = factory.Create(pv_path, kMaximumSupportedVersion);
  EXPECT_TRUE(manager);

  auto pv_manager = manager.value();
  pv_manager->Put(key, std::make_any<TestType>(test_value));
  EXPECT_EQ(StorageError::kSuccess, pv_manager->HasKey(key).error_code_);
}

TEST(PVManager, ExpiredTest) {
  using TestType = ByteVector;
  const TestType test_value(0x100, '\x41');
  const std::basic_string<CharType> key = "/root/asd19";
  const auto pv_path = fs::temp_directory_path() / "yas_pv_fd60f6e1ae21d37aa1e10007636431ab_19";

  auto &factory = PVManagerFactory::Instance();
  auto manager = factory.Create(pv_path, kMaximumSupportedVersion);
  EXPECT_TRUE(manager);

  const auto saved_time = time(nullptr) - 100;
  utils::Time expired_time(saved_time);

  auto pv_manager = manager.value();
  pv_manager->Put(key, std::make_any<TestType>(test_value));
  pv_manager->SetExpiredDate(key, saved_time);
  const auto retreived_time = pv_manager->GetExpiredDate(key);
  EXPECT_EQ(0, std::difftime(saved_time, retreived_time.value()));
}

TEST(PVManager, ExpiredDeleteTest) {
  using TestType = ByteVector;
  const TestType test_value(0x100, '\x41');
  const std::basic_string<CharType> key = "/root/asd20";
  const auto pv_path = fs::temp_directory_path() / "yas_pv_fd60f6e1ae21d37aa1e10007636431ab_20";

  auto &factory = PVManagerFactory::Instance();
  auto manager = factory.Create(pv_path, kMaximumSupportedVersion);
  EXPECT_TRUE(manager);

  const auto saved_time = time(nullptr) - 100;
  utils::Time expired_time(saved_time);

  auto pv_manager = manager.value();
  pv_manager->Put(key, std::make_any<TestType>(test_value));
  pv_manager->SetExpiredDate(key, saved_time);
  const auto result = pv_manager->Get(key);
  EXPECT_FALSE(result.has_value()); // value already expired
}

TEST(PVManager, GetNonExistKey) {
  using TestType = ByteVector;
  const TestType test_value(0x100, '\x41');
  const std::basic_string<CharType> key = "/root/asd21";
  const auto pv_path = fs::temp_directory_path() / "yas_pv_fd60f6e1ae21d37aa1e10007636431ab_21";

  auto &factory = PVManagerFactory::Instance();
  auto manager = factory.Create(pv_path, kMaximumSupportedVersion);
  EXPECT_TRUE(manager);

  auto pv_manager = manager.value();
  pv_manager->Put(key, std::make_any<TestType>(test_value));
  auto result = pv_manager->Get("/////");
  EXPECT_FALSE(result.has_value());
}

TEST(PVManager, HasCatalogTest) {
  using TestType = ByteVector;
  const TestType test_value(0x100, '\x41');
  const std::basic_string<CharType> key = "/root/asd22";
  const auto pv_path = fs::temp_directory_path() / "yas_pv_fd60f6e1ae21d37aa1e10007636431ab_22";

  auto &factory = PVManagerFactory::Instance();
  auto manager = factory.Create(pv_path, kMaximumSupportedVersion);
  EXPECT_TRUE(manager);

  auto pv_manager = manager.value();
  pv_manager->Put(key, std::make_any<TestType>(test_value));
  auto result = pv_manager->HasCatalog("/");
  EXPECT_EQ(StorageError::kSuccess, result.error_code_);
}

TEST(PVManager, ReqsFuzzyTest_1) {
  using TestType = uint32_t;

  const uint32_t keys_count = 100000;
  const TestType test_value = 5;
  const std::basic_string<CharType> key = "/root/asd22";
  const auto pv_path = fs::temp_directory_path() / "yas_pv_fd60f6e1ae21d37aa1e10007636431ab_23";

  auto &factory = PVManagerFactory::Instance();
  std::vector<std::shared_ptr<PVManagerFactory::manager_type>> managers;
  auto manager = factory.Create(pv_path, kMaximumSupportedVersion);
  EXPECT_TRUE(manager);

  for (uint32_t key_id = 0; key_id < keys_count; ++key_id) {
    manager.value()->Put("/root/" + std::to_string(key_id), test_value);
  }
}

//TEST(PVManager, ReqsFuzzyTest_2) {
//  using TestType = uint32_t;
//
//  // it seems that on Windows 10 Windows Defender blocks many simultenous file creating - it is important to add 
//  // project to exclusion
//  const uint32_t managers_count = 400;
//  const uint32_t keys_count = 100*1000000;
//  const TestType test_value = 5;
//  auto pv_path = fs::temp_directory_path();
//
//  auto &factory = PVManagerFactory::Instance();
//  std::vector<std::shared_ptr<PVManagerFactory::manager_type>> managers;
//  for (uint32_t manager_id = 0; manager_id < managers_count; ++manager_id) {
//    auto manager = factory.Create(pv_path / ("yas_pv_fd60f6e1ae21d37aa1e10007636431ab_fuzz_" + std::to_string(manager_id)), kMaximumSupportedVersion);
//    if (!manager.has_value()) {
//      continue;
//    }
//    managers.push_back(std::move(manager.value()));
//  }
//
//  for (auto &manager : managers) {
//    for (uint32_t key_id = 0; key_id < keys_count; ++key_id) {
//      manager->Put("/root/" + std::to_string(key_id), test_value);
//    }
//  }
//
//  for (auto &manager : managers) {
//    for (uint32_t key_id = 0; key_id < keys_count; ++key_id) {
//      const auto result = manager->Get("/root/" + std::to_string(key_id));
//      EXPECT_TRUE(result);
//      EXPECT_EQ(test_value, std::any_cast<TestType>(result.value()));
//    }
//  }
//}

}
