#include "tests/aho_corasick_tests.h"
#include "tests/inverted_index_tests.h"
#include "tests/test_device_tests.h"

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
