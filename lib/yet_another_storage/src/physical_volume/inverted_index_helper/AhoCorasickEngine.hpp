#pragma once
#include "leaf_type_traits.hpp"
#include <unordered_map>
#include <memory>

namespace yas {
namespace index_helper {

template <typename CharType, typename LeafType>
class AhoCorasickEngine {
  using StringViewType = std::basic_string_view<CharType>;
  template<typename CharType>
  struct Node;
  using CharNode = Node<CharType>;

 public:
  static_assert(std::is_trivially_copyable_v<LeafType>, "LeafType should be POD");

  AhoCorasickEngine()
      : trie_(new CharNode())
  {}
  ~AhoCorasickEngine() = default;
  AhoCorasickEngine(AhoCorasickEngine &&other) noexcept 
      : trie_(std::move(other.trie_))
  {}

  bool Insert(StringViewType key, LeafType &leaf) {
    auto current = trie_.get();

    for (const auto &ch : key) {
      auto next = getNextNode(ch, current);
      if (nullptr == next) {
        current->routes_[ch].reset(new CharNode());
        current = current->routes_[ch].get();
        continue;
      }
      current = next;
    }
    current->leaf_= std::move(leaf);
    return true;
  }

  LeafType Get(StringViewType key) noexcept {
    const auto node = getPathNode(key);
    return (nullptr == node) ? leaf_traits<Leaf>::NonExistValue() : node->leaf_;
  }

  const LeafType Get(StringViewType key) const noexcept {
    const auto node = getPathNode(key);
    return (nullptr == node) ? leaf_traits<Leaf>::NonExistValue() : node->leaf_;
  }

  bool Delete(StringViewType key) {
    const auto node = getPathNode(key);
    if (nullptr == node) {
      return false;
    }

    node->leaf_ = leaf_traits<Leaf>::NonExistValue();
    return true;
  }

  bool HasKey(StringViewType key) const noexcept {
    const auto node = getPathNode(key);
    return (nullptr == node) ? false : leaf_traits<Leaf>::NonExistValue() != node->leaf_;
  }

  AhoCorasickEngine(const AhoCorasickEngine&) = delete;
  AhoCorasickEngine& operator=(const AhoCorasickEngine&) = delete;
  AhoCorasickEngine& operator=(AhoCorasickEngine&&) = delete;

private:
  template <typename T1, typename T2, typename T3> friend class AhoCorasickSerializationHelper;
  template<typename CharType>
  struct Node {
    Node() : leaf_(leaf_traits<Leaf>::NonExistValue()) {}
    std::unordered_map<CharType, std::unique_ptr<Node>> routes_;
    LeafType leaf_;
  };

  std::unique_ptr<CharNode> trie_;

  CharNode *getNextNode(CharType ch, CharNode *current) const noexcept {
    if (nullptr == current || !current->routes_.count(ch)) {
      // the next node hasn't been created yet
      return nullptr;
    }
    return current->routes_[ch].get();
  } 

  CharNode *getPathNode(StringViewType key) const noexcept {
    auto current = trie_.get();

    for (const auto &ch : key) {
      current = getNextNode(ch, current);
      if (nullptr == current) {
        return nullptr;
      }
    }
    return current;
  }
};

} // namespace index_helper
} // namespace yas
