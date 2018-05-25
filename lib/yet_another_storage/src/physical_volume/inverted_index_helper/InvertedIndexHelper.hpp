#pragma once
#include "AhoCorasickEngine.hpp"
#include "AhoCorasickSerializationHelper.hpp"
#include "../../utils/Version.hpp"
#include "../../common/common.h"
#include "../../common/settings.hpp"
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

    is_changed = true;
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

    is_changed = true;
    return engine_.Delete(key);
  }

  bool HasKey(key_type key) const noexcept {
    if (key.empty()) {
      return false;
    }
    return engine_.HasKey(key);
  }

  constexpr bool is_changed() const { return is_changed_; }

  template<typename IdType>
  ByteVector Serialize(utils::Version version) const {
    AhoCorasickSerializationHelper<CharType, LeafType, IdType> serializer(version);
    return serializer.Serialize(engine_);
  }

  template<typename IdType, typename Iterator>
  static InvertedIndexHelper Deserialize(Iterator begin, Iterator end, utils::Version version) {
    AhoCorasickSerializationHelper<CharType, LeafType, IdType> serializer(version);
    InvertedIndexHelper<CharType, LeafType> index_helper;
    serializer.Deserialize(begin, end, index_helper.engine_);

    return index_helper;
  }

  InvertedIndexHelper(const InvertedIndexHelper&) = delete;
  InvertedIndexHelper& operator=(const InvertedIndexHelper&) = delete;
  InvertedIndexHelper& operator=(InvertedIndexHelper&&) = delete;

 private:
  // TODO: engine probably should be template or inner class (pimpl?) - need to decide later
  AhoCorasickEngine<CharType, LeafType> engine_;
  // true if index has been changed after creating
  bool is_changed_ = true;
};

} // namespace index_helper
} // namespace yas
