#include "gtest/gtest.h"
#include "pv_manager_tests.h"

// (!!!!) Some of these tests write files to tmp directory

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
