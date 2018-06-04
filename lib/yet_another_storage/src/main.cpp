#include "storage/PVManagerFactory.hpp"
#include "common/filesystem.h"
#include <iostream>
#include <variant>

using namespace yas;

int main() {
  const auto ttt = fs::temp_directory_path();
  fs::path pv1_path = "D:/additional_activity/jetbrains_testtask/result/pv1";
  auto &factory = storage::PVManagerFactory::Instance();
  auto manager = factory.Create(pv1_path, {1,1});
  if (!manager) {
    return -1;
  }

  auto pv_manager = manager.value();
  
  int8_t value = 5;
  pv_manager->Put("/root/asd", value);
  std::string test_value("aaaaaaaaaaaaaaaaaaaaaaaaaa");
  pv_manager->Delete("/root/asd/asd");
  pv_manager->Put("/root/asd/asd", test_value);
  auto result = pv_manager->Get("/root/asd/asd");
  if (!result) {
    return -1;
  }

  auto tt = result.value();
  auto tt2 = std::get<std::string>(tt);
  
  pv_manager->SetExpiredDate("/root/asd/asd", 0);
  auto result_2 = pv_manager->Get("/root/asd/asd");
  

  return 0;
}
