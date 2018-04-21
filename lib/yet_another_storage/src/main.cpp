#include "storage/Storage.hpp"
#include "physical_volume/physical_volume_layout/pv_layout_types_headers.h"
#include "physical_volume/inverted_index_helper/AhoCorasickEngine.hpp"
#include <iostream>

using namespace yas;

template <typename OffsetType>
struct Leaf {
  Leaf(OffsetType value = 0)
    : value_(value)
  {}

  static Leaf& MakeNonExistType() noexcept {
    static Leaf leaf{ 0 };
    return leaf;
  }
  OffsetType value_;
};

int main() {
  storage::Storage storage;
  Leaf<OffsetType> leaf(5);

  index_helper::AhoCorasickEngine<char, Leaf<OffsetType>> engine;
  engine.Insert("/home/user", leaf);

  return 0;
}
