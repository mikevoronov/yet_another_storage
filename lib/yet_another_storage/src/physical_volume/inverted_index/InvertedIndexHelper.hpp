#pragma once
#include "AhoCorasickEngine.hpp"
#include <memory>
#include <string_view>
#include <type_traits>

namespace yas {
namespace index_helper {

template <typename CharType, typename LeafType>
class InvertedIndexHelper {
  using StringViewType = std::basic_string_view<CharType>;
public:
  InvertedIndexHelper() = default;
  ~InvertedIndexHelper() = default;

  InvertedIndexHelper(InvertedIndexHelper &&other) 
      : trie_(std::move(other.trie_)) 
  {}

  bool Insert(StringViewType key, LeafType leaf) {
    if (key.empty()) {
      return false;
    }
    return trie_.Insert(key, leaf);
  }

  LeafType& Get(StringViewType key) noexcept {
    if (key.empty()) {
      return LeafType::MakeNonExistType();
    }
    return trie_.Get(key);
  }

  const LeafType& Get(StringViewType key) const noexcept {
    if (key.empty()) {
      return LeafType::MakeNonExistType();
    }
    return trie_.Get(key);
  }


  bool Delete(StringViewType key) {
    if (key.empty()) {
      return false;
    }
    return trie_.Delete(key);
  }

  bool HasKey(StringViewType key) const noexcept {
    if (key.empty()) {
      return false;
    }
    return trie_.HasKey(key);
  }
  
  InvertedIndexHelper(const InvertedIndexHelper&) = delete;
  InvertedIndexHelper& operator=(const InvertedIndexHelper&) = delete;
  InvertedIndexHelper& operator=(InvertedIndexHelper&&) = delete;
private:
  // TODO: engine probably should be template or inner class (pimpl?) - need to decide later
  AhoCorasickEngine<CharType, LeafType> trie_;
};

} // namespace index_helper
} // namespace yas
