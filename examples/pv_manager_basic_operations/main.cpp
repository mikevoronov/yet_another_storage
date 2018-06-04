#include "storage/PVManagerFactory.hpp"
#include <iostream>
#include <variant>

using namespace yas;

// (!) be careful - this sketch project write file to tmp directory

int main() {
  const auto pv_path = fs::temp_directory_path() / "yas_pv_manager_basic_operations_23bb82e69577a6762d36fc552b2f952e";
  const utils::Version desired_version{ 1,1 };

  auto &factory = storage::PVManagerFactory::Instance();
  auto manager = factory.Create(pv_path, desired_version);
  if (!manager) {
    std::cout << manager.error().message_;
    return -1;
  }

  auto pv_manager = manager.value();

  const int8_t test_value_1 = 5;
  const std::string test_value_2{"Welcome to YAS!"};
  const std::basic_string<DCharType> key{"/root/asd"};

  pv_manager->Delete(key);
  auto error_code = pv_manager->Put(key, test_value_1);
  if (!error_code) {
    std::cout << error_code.message_;
    return -1;
  }

  error_code = pv_manager->Delete(key);
  if (!error_code) {
    std::cout << error_code.message_;
    return -1;
  }

  error_code = pv_manager->Put(key, test_value_2);
  if (!error_code) {
    std::cout << error_code.message_;
    return -1;
  }

  const auto result = pv_manager->Get(key);
  if (!result) {
    std::cout << result.error().message_;
    return -1;
  }

  const auto result_value = std::get<std::string>(result.value());
  std::cout << result_value;

  return 0;
}
