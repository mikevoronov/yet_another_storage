#pragma once
#include <cstdint>

namespace yas {
namespace serialization_utils {

// idea from https://stackoverflow.com/questions/47917576/append-structs-to-a-vectoruint8-t

template<typename Iterator>
struct range_t {
  Iterator begin_;
  Iterator end_;
  Iterator begin() noexcept { return begin_; }
  Iterator end() noexcept { return end_; }
  constexpr Iterator cbegin() const noexcept { return begin_; }
  constexpr Iterator cend() const noexcept { return end_; }
  decltype(auto) size() const  { return std::distance(begin_, end_); }
};

template<typename Iterator>
range_t<Iterator> range(Iterator begin, Iterator end) { return { begin, end }; }

template<typename Type>
range_t<uint8_t*> AsBytes(Type* type) {
  static_assert(std::is_trivially_copyable_v<Type>, "Type must be POD");
  auto* ptr = reinterpret_cast<uint8_t*>(type);
  return range(ptr, ptr + sizeof(Type));
}

template<typename Type>
range_t<const uint8_t*> AsBytes(const Type *type) {
  static_assert(std::is_trivially_copyable_v<Type>, "Type must be POD");
  auto *ptr = reinterpret_cast<const uint8_t *>(type);
  return range(ptr, ptr + sizeof(Type));
}

// checks correspondes between sizes
template<typename InsertIterator, typename Type>
InsertIterator SaveAsBytes(InsertIterator begin, InsertIterator end, const Type *data) {
  auto bytes = AsBytes(data);
  if (std::distance(begin, end) < bytes.size()) {
    return end;
  }
  return std::copy(std::begin(bytes), std::end(bytes), begin);
}

// checks correspondes between sizes
template<typename ReadIterator, typename Type>
ReadIterator LoadFromBytes(ReadIterator begin, ReadIterator end, Type* data) {
  auto bytes = AsBytes(data);
  if (std::distance(begin, end) > bytes.size()) {
    return end;
  }

  std::copy(begin, begin + bytes.size(), std::begin(bytes));
  return begin + bytes.size();
}

} // namespace utils
} // namespace yas
