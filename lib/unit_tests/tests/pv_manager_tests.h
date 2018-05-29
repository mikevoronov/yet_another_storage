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
  const auto pv_path = fs::temp_directory_path() / "pv1";

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
  const auto pv_path = fs::temp_directory_path() / "pv2";

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
  const auto pv_path = fs::temp_directory_path() / "pv3";

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
  const auto pv_path = fs::temp_directory_path() / "pv4";

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
  const auto pv_path = fs::temp_directory_path() / "pv5";

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
  const auto pv_path = fs::temp_directory_path() / "pv6";

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
  const auto pv_path = fs::temp_directory_path() / "pv7";

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
  const auto pv_path = fs::temp_directory_path() / "pv8";

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
  const auto pv_path = fs::temp_directory_path() / "pv9";

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
  const auto pv_path = fs::temp_directory_path() / "pv10";

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
  const auto pv_path = fs::temp_directory_path() / "pv11";

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
  const auto pv_path = fs::temp_directory_path() / "pv12";

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
  const auto pv_path = fs::temp_directory_path() / "pv13";

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
  const auto pv_path = fs::temp_directory_path() / "pv14";

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
  const auto pv_path = fs::temp_directory_path() / "pv15";

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
  const auto pv_path = fs::temp_directory_path() / "pv16";

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
  const auto pv_path = fs::temp_directory_path() / "pv17";

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
  const auto pv_path = fs::temp_directory_path() / "pv18";

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
  const auto pv_path = fs::temp_directory_path() / "pv19";

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
  const auto pv_path = fs::temp_directory_path() / "pv20";

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

}
