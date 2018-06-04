#pragma once
#include "storage/lib/devices/TestDevice.hpp"
#include "storage/lib/common/common.h"

using namespace yas;

namespace {

void CompareByteVectors(const ByteVector &first, const ByteVector &second) {
  EXPECT_EQ(first.size(), second.size());
  auto first_vector_iterator = std::cbegin(first);
  for (auto second_vector_iterator = std::cbegin(second), end = std::cend(second);
      second_vector_iterator != end; ++second_vector_iterator) {
    EXPECT_EQ(*first_vector_iterator, *second_vector_iterator);
    ++first_vector_iterator;
  }
}

TEST(TestDevice, ReadTest) {
  devices::TestDevice<uint64_t> test_device("/root");
  
  ByteVector write_vector = {'\x00', '\x01', '\x02', '\x04', '\x05'};
  test_device.SetStorageContent(std::cbegin(write_vector), std::cend(write_vector));

  ByteVector read_vector(write_vector.size());
  test_device.Read(0, std::begin(read_vector), std::end(read_vector));

  CompareByteVectors(read_vector, write_vector);
}

TEST(TestDevice, WriteToEndEmptyFileTest) {
  devices::TestDevice<uint64_t> test_device("/root");

  ByteVector write_vector = { '\x00', '\x01', '\x02', '\x04', '\x05' };
  test_device.Write(0, std::cbegin(write_vector), std::cend(write_vector));

  ByteVector read_vector(write_vector.size());
  test_device.Read(0, std::begin(read_vector), std::end(read_vector));
  CompareByteVectors(read_vector, write_vector);
}

TEST(TestDevice, WriteToEndNonEmptyFileTest) {
  yas::devices::TestDevice<uint64_t> test_device("/root");

  yas::ByteVector write_vector = { '\x00', '\x01', '\x02', '\x04', '\x05' };
  test_device.SetStorageContent(std::cbegin(write_vector), std::cend(write_vector));
  test_device.Write(5, std::cbegin(write_vector), std::cend(write_vector));

  ByteVector read_vector(write_vector.size());
  test_device.Read(0, std::begin(read_vector), std::end(read_vector));

  CompareByteVectors(read_vector, write_vector);

  test_device.Read(5, std::begin(read_vector), std::end(read_vector));
  CompareByteVectors(read_vector, write_vector);
}

TEST(TestDevice, WriteToMiddleFileTest) {
  yas::devices::TestDevice<uint64_t> test_device("/root");

  yas::ByteVector write_vector = { '\x00', '\x01', '\x02', '\x04', '\x05' };
  test_device.SetStorageContent(std::cbegin(write_vector), std::cend(write_vector));
  test_device.Write(3, std::cbegin(write_vector), std::cend(write_vector));

  ByteVector read_vector(write_vector.size());
  test_device.Read(3, std::begin(read_vector), std::end(read_vector));

  CompareByteVectors(read_vector, write_vector);
}

}
