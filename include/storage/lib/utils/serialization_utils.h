#pragma once
#include <cstdint>
#include <vector>

namespace yas {
namespace serialization_utils {

// the idea from https://stackoverflow.com/questions/47917576/append-structs-to-a-vectoruint8-t

template<typename Iterator>
struct range_t {
  Iterator begin_;
  Iterator end_;
  Iterator begin() noexcept { return begin_; }
  Iterator end() noexcept { return end_; }
  const Iterator cbegin() const noexcept { return begin_; }
  const Iterator cend() const noexcept { return end_; }
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

template<typename InsertIterator, typename Type>
InsertIterator SaveAsBytes(const InsertIterator begin, const InsertIterator end, const Type *data) {
  auto bytes = AsBytes(data);
  if (std::distance(begin, end) < bytes.size()) {
    return end;
  }
  return std::copy(std::begin(bytes), std::end(bytes), begin);
}

template<typename ReadIterator, typename Type>
ReadIterator LoadFromBytes(ReadIterator begin, ReadIterator end, Type *data) {
  auto bytes = AsBytes(data);
  if (std::distance(begin, end) < bytes.size()) {
    // nithing has been readed
    return begin;
  }

  auto new_end = begin;
  std::advance(new_end, bytes.size());
  std::copy(begin, new_end, std::begin(bytes));
  return new_end;
}

// the built-in offsetof macros is compile-specific and could have some important limitations
// that prevent us of using it if constexpr f.e.
template <typename T1, typename T2>
size_t constexpr offset_of(T1 T2::*member) {
  constexpr T2 object{};
  return size_t(&(object.*member)) - size_t(&object);
}

} // namespace serialization_utils
} // namespace yas
