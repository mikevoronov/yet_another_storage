#include "storage/PVManagerFactory.hpp"
#include "common/filesystem.h"
#include <iostream>

using namespace yas;

int main() {
  fs::path pv1_path = "D:/additional_activity/jetbrains_testtask/result/pv1";
  auto &factory = storage::PVManagerFactory::Instance();
  auto manager = factory.Create(pv1_path, {1,1});
  if (!manager) {
    return -1;
  }

  auto pv_manager = manager.value();
  pv_manager->Put("/root/asd", std::make_any<int8_t>(5));
  std::string test_value("aaaaaaaaaaaaaaaaaaaaaaaaaa");
  pv_manager->Put("/root/asd/asd", test_value);
  auto result = pv_manager->Get("/root/asd/asd");
  if (!result) {
    return -1;
  }

  auto tt = result.value();
  auto tt2 = std::any_cast<std::string>(tt);

  return 0;
}
