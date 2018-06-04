#include "aho_corasick_tests.h"
#include "inverted_index_tests.h"
#include "test_device_tests.h"
#include "version_tests.h"
#include "time_tests.h"
#include "storage_tests.h"
#include "pv_manager_tests.h"
#include "freelist_helper_tests.h"

// (!!!!) Some of these tests write files to tmp directory

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
