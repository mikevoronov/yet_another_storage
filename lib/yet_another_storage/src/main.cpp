#include "physical_volume/inverted_index_helper/InvertedIndexHelper.hpp"
#include "utils/Version.hpp"
#include <string>
#include <iostream>
#include <cstdint>

struct Leaf {
  Leaf(uint64_t value = 0)
    : value_(value)
  {}
  uint64_t value_;
};

bool operator!=(const Leaf &lhs, const Leaf &rhs) {
  return lhs.value_ != rhs.value_;
}

class A {
public:
  A() {
    helper_.Insert("/home/user1", 1);
  }

  void foo() const {
    const auto aa = helper_.Get("/home/user1");
  }

  yas::index_helper::InvertedIndexHelper<char, Leaf> helper_;
};

// TODO : move to unit test
int main() {
  A a;
  a.foo();

  yas::index_helper::InvertedIndexHelper<char, Leaf> helper_;

  helper_.Insert("/home/user1", 1);
  helper_.Insert("/home/user1/file.txt", 2);
  helper_.Insert("/home/user1/tmp", 3);
  helper_.Insert("/home/user1/tmp/aa1", 4);
  helper_.Insert("/home/user1/tmp/aa2", 5);

  std::cout << helper_.HasKey("/home/user1/tmp/aa2") << std::endl;
  std::cout << helper_.HasKey("/home/user1/tmp/aa3") << std::endl;
  const auto t1 = helper_.Get("/home/user1/tmp/aa2");
  std::cout << t1.value_ << std::endl;
  std::cout << helper_.Get("/home/user1/tmp").value_ << std::endl;
  std::cout << helper_.Get("/home/user1/file.txt").value_ << std::endl;

  auto t2 = helper_.Delete("/home/user1/file.txt");
  std::cout << helper_.Get("/home/user1/file.txt").value_ << std::endl;

  yas::utils::Version version{ 1,1 };
  auto data = helper_.Serialize<uint32_t>(version);
  auto yy = yas::index_helper::InvertedIndexHelper<char, Leaf>::Deserialize<uint32_t>(data, version).Get("/home/user1/tmp").value_;


  std::cout << helper_.Get("/home/user1/tmp").value_ << std::endl;
}
