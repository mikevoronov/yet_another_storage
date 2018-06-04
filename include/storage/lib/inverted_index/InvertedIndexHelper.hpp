#pragma once
#include "AhoCorasickEngine.hpp"
#include "AhoCorasickSerializationHelper.hpp"
#include <memory>
#include <string_view>
#include <type_traits>

namespace yas {
namespace index_helper {

template <typename CharType, typename LeafType>
class InvertedIndexHelper {
public:
  using char_type = CharType;
  using leaf_type = LeafType;
  using key_type = std::basic_string_view<CharType>;

  InvertedIndexHelper() = default;
  ~InvertedIndexHelper() = default;

  InvertedIndexHelper(InvertedIndexHelper &&other) noexcept
      : engine_(std::move(other.engine_))
  {}

  bool Insert(key_type key, LeafType leaf) {
    if (key.empty()) {
      return false;
    }

    is_changed_ = true;
    return engine_.Insert(key, leaf);
  }

  LeafType Get(key_type key) noexcept {
    if (key.empty()) {
      return leaf_traits<LeafType>::NonExistValue();
    }
    return engine_.Get(key);
  }

  const LeafType Get(key_type key) const noexcept {
    if (key.empty()) {
      return leaf_traits<LeafType>::NonExistValue();
    }
    return engine_.Get(key);
  }

  bool Delete(key_type key) {
    if (key.empty()) {
      return false;
    }

    is_changed_ = true;
    return engine_.Delete(key);
  }

  bool HasKey(key_type key) const noexcept {
    if (key.empty()) {
      return false;
    }
    return engine_.HasKey(key);
  }

  int64_t FindMaxSubKey(key_type key) const noexcept {
    if (key.empty()) {
      return 0;
    }
    return engine_.FindMaxSubKey(key);
  }

  constexpr bool is_changed() const { return is_changed_; }

  template<typename IdType>
  ByteVector Serialize(utils::Version version) const {
    AhoCorasickSerializationHelper<CharType, LeafType, IdType> serializer(version);
    return serializer.Serialize(engine_);
  }

  template<typename IdType, typename Iterator>
  static std::unique_ptr<InvertedIndexHelper> Deserialize(Iterator begin, Iterator end, utils::Version version) {
    AhoCorasickSerializationHelper<CharType, LeafType, IdType> serializer(version);
    auto inverted_index = std::make_unique<InvertedIndexHelper<CharType, LeafType>>();
    serializer.Deserialize(begin, end, inverted_index->engine_);
    inverted_index->is_changed_ = false;

    return inverted_index;
  }

  InvertedIndexHelper(const InvertedIndexHelper&) = delete;
  InvertedIndexHelper& operator=(const InvertedIndexHelper&) = delete;
  InvertedIndexHelper& operator=(InvertedIndexHelper&&) = delete;

 private:
  AhoCorasickEngine<CharType, LeafType> engine_;
  // true if index has been changed after creating
  bool is_changed_ = false;
};

} // namespace index_helper
} // namespace yas
