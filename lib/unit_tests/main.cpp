#include "tests/aho_corasick_tests.h"
#include "tests/inverted_index_tests.h"
#include "tests/test_device_tests.h"
#include "tests/version_tests.h"
#include "tests/time_tests.h"
#include "tests/storage_tests.h"
#include "tests/pv_manager_tests.h"
#include "tests/freelist_helper_tests.h"

// (!!!!) Some of these tests write files to tmp directory

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
