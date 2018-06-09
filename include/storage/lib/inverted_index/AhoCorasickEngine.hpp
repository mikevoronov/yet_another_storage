#pragma once
#include "leaf_type_traits.hpp"
#include <unordered_map>
#include <memory>

namespace yas {
namespace index_helper {

template <typename CharType, typename LeafType>
class AhoCorasickEngine {
 public:
  static_assert(std::is_trivially_copyable_v<LeafType>, "LeafType should be POD");

  using char_type = CharType;
  using leaf_type = LeafType;
  using key_type = std::basic_string_view<CharType>;

  AhoCorasickEngine()
      : trie_(new Node())
  {}
  ~AhoCorasickEngine() = default;
  AhoCorasickEngine(AhoCorasickEngine &&other) noexcept 
      : trie_(std::move(other.trie_))
  {}

  bool Insert(key_type key, LeafType &leaf) {
    auto current = trie_.get();

    for (const auto &ch : key) {
      auto next = getNextNode(ch, current);
      if (nullptr == next) {
        current->routes_[ch].reset(new Node());
        current = current->routes_[ch].get();
        continue;
      }
      current = next;
    }
    current->leaf_= leaf;
    return true;
  }

  LeafType Get(key_type key) noexcept {
    const auto node = getPathNode(key);
    return (nullptr == node) ? leaf_type_traits<LeafType>::NonExistValue() : node->leaf_;
  }

  const LeafType Get(key_type key) const noexcept {
    const auto node = getPathNode(key);
    return (nullptr == node) ? leaf_type_traits<LeafType>::NonExistValue() : node->leaf_;
  }

  bool Delete(key_type key) {
    // TODO : delete node path
    const auto node = getPathNode(key);
    if (nullptr == node) {
      return false;
    }

    node->leaf_ = leaf_type_traits<LeafType>::NonExistValue();
    return true;
  }

  bool HasKey(key_type key) const noexcept {
    const auto node = getPathNode(key);
    return (nullptr == node) ? false : leaf_type_traits<LeafType>::NonExistValue() != node->leaf_;
  }

  int64_t FindMaxSubKey(key_type key) const noexcept {
    // to make class more generic on future this method could be replaced with a method
    // that could apply visitors to nodes
    auto current = trie_.get();
    int64_t max_path = 0;

    for (const auto &ch : key) {
      current = getNextNode(ch, current);
      if (nullptr == current) {
        return max_path;
      }
      ++max_path;
    }
    return max_path;
  }

  AhoCorasickEngine(const AhoCorasickEngine&) = delete;
  AhoCorasickEngine& operator=(const AhoCorasickEngine&) = delete;
  AhoCorasickEngine& operator=(AhoCorasickEngine&&) = delete;

private:
  template <typename T1, typename T2, typename T3> friend class AhoCorasickSerializationHelper;

  // TODO : for better perfomance and cache-friendly nodes should placed next to each other
  // in this case traversal would almost free (could used the same allocator as in std::deque)
  struct Node {
    Node() : leaf_(leaf_type_traits<LeafType>::NonExistValue()) {}
    std::unordered_map<CharType, std::unique_ptr<Node>> routes_;
    LeafType leaf_;
  };

  std::unique_ptr<Node> trie_;

  Node *getNextNode(CharType ch, Node *current) const noexcept {
    if (nullptr == current || !current->routes_.count(ch)) {
      // the next node hasn't been created yet
      return nullptr;
    }
    return current->routes_[ch].get();
  } 

  Node *getPathNode(key_type key) const noexcept {
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
