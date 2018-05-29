#pragma once
#include "AhoCorasickEngine.hpp"
#include "aho_corasick_serialization_headers.hpp"
#include "../exceptions/YASException.hpp"
#include "../utils/serialization_utils.h"
#include "../common/common.h"
#include "id_type_type_traits.hpp"
#include <cstdint>
#include <vector>
#include <algorithm>

using namespace yas::storage;

namespace yas {
namespace index_helper {

template <typename CharType, typename LeafType, typename IdType>
class AhoCorasickSerializationHelper {
  using Engine = AhoCorasickEngine<CharType, LeafType>;
  using CharNode = typename Engine::CharNode;

 public:
  static_assert(std::is_integral_v<IdType>, "IdType should be an integral type");

  explicit AhoCorasickSerializationHelper(utils::Version version)
      : version_(version)
  {}
  ~AhoCorasickSerializationHelper() = default;

  ByteVector Serialize(const Engine &engine) const {
    NodeSerializationDescriptorStorage serialized_nodes;
    LeafSerializationDescriptorStorage serialized_leafs;
    NodeDescriptorStorage current_level_nodes;
    NodeDescriptorStorage next_level_nodes;
    
    if (!engine.trie_) {
      // nothing to serialize
      return {};
    }

    current_level_nodes.emplace_back(0, engine.trie_.get(), 0, std::char_traits<CharType>::to_char_type('/'));

    IdType current_node_id = 1;     // root node has been already counted 
    IdType current_leaf_id = 0;
    IdType parent_node_id = 0;
    IdType depth_level = 0;

    // breadth-first search
    while (!current_level_nodes.empty()) {
      for (const auto &node_descriptor : current_level_nodes) {
        auto leaf_id = id_type_traits<IdType>::NonExistValue();
        if (leaf_traits<LeafType>::IsExistValue(node_descriptor.node_->leaf_)) {
          leaf_id = current_leaf_id;
          serialized_leafs.push_back(serialize(node_descriptor.node_->leaf_, node_descriptor.node_id_));
          ++current_leaf_id;
        }
        serialized_nodes.push_back(serialize(node_descriptor, depth_level, leaf_id));

        next_level_nodes.reserve(node_descriptor.node_->routes_.size());
        for(auto &route : node_descriptor.node_->routes_) {
          next_level_nodes.emplace_back(current_node_id, route.second.get(), node_descriptor.node_id_, route.first);
          ++current_node_id;
        }

      }
      ++depth_level;
      current_level_nodes = std::move(next_level_nodes);
    }

    return constructResultSerializedBuffer(serialized_leafs, serialized_nodes);
  }

  template <typename Iterator>
  void Deserialize(const Iterator begin, const Iterator end, Engine &engine) const {
    SerializedDataHeader header;
    auto current_cursor = deserializeDataHeader(begin, end, header);
    if (!header.nodes_count_) {
      return;
    }

    LeafSerializationDescriptorStorage leaf_descriptors;
    current_cursor = deserializeLeafDescriptors(current_cursor, end, header.leafs_count_, leaf_descriptors);
    std::sort(std::begin(leaf_descriptors), std::end(leaf_descriptors),
        [](const LeafSerializationDescriptorStorage::value_type &left,
           const LeafSerializationDescriptorStorage::value_type &right) {
      return left.node_id_ < right.node_id_;
    });

    // extract root entry
    NodeSerializationDescriptor node_descriptor;
    current_cursor = deserializeNodeDescriptor(current_cursor, end, node_descriptor);
    if (node_descriptor.node_id_ != node_descriptor.parent_node_id_) {
      throw (exception::YASException("Corrupt data: root must be the first node in the serialized descriptors list",
          StorageError::kInvertedIndexDesirializationError));
    }

    // at first completely construct the trie on function level
    // and only then modify engine for exception safety
    auto root = std::make_unique<CharNode>();

    NodeDescriptorStorage previous_level_nodes;
    NodeDescriptorStorage current_level_nodes;
    previous_level_nodes.push_back(deserialize(node_descriptor, root.get()));

    // root extracted -> depth_level should be 1
    IdType depth_level = 1;
    for (IdType node_id = 1; node_id < header.nodes_count_; ++node_id) {
      current_cursor = deserializeNodeDescriptor(current_cursor, end, node_descriptor);
      if (depth_level < node_descriptor.depth_level_) {
        // gone to the next level
        prepareForNextDeserializationDepth(previous_level_nodes, current_level_nodes);
        ++depth_level;
      }

      const auto parent = findNodeByParentNodeId(std::cbegin(previous_level_nodes), std::cend(previous_level_nodes),
          node_descriptor.parent_node_id_);
      if (std::cend(previous_level_nodes) == parent) {
        throw (exception::YASException("Corrupt data: parent node can't be found",
            StorageError::kInvertedIndexDesirializationError));
      }

      parent->node_->routes_[node_descriptor.parent_node_ch_].reset(new CharNode());
      const auto route = parent->node_->routes_[node_descriptor.parent_node_ch_].get();
      current_level_nodes.push_back(deserialize(node_descriptor, route));

      if (id_type_traits<IdType>::IsExistValue(node_descriptor.leaf_id_)) {
        const auto found_leaf_iterator = findLeafByNodeId(std::cbegin(leaf_descriptors), std::cend(leaf_descriptors),
            node_descriptor.node_id_);
        if (std::cend(leaf_descriptors) == found_leaf_iterator) {
          throw (exception::YASException("Corrupt data: node's leaf can't be found",
              StorageError::kInvertedIndexDesirializationError));
        }
        route->leaf_ = found_leaf_iterator->leaf_;
      }
    }

    engine.trie_.swap(root);
  }

  AhoCorasickSerializationHelper(const AhoCorasickSerializationHelper&) = delete;
  AhoCorasickSerializationHelper(AhoCorasickSerializationHelper&&) = delete;
  AhoCorasickSerializationHelper& operator=(const AhoCorasickSerializationHelper&) = delete;
  AhoCorasickSerializationHelper& operator=(AhoCorasickSerializationHelper&&) = delete;

 private:
  using NodeSerializationDescriptor = aho_corasick_serialization_headers::NodeSerializationDescriptorT<IdType, CharType>;
  using LeafSerializationDescriptor = aho_corasick_serialization_headers::LeafSerializationDescriptorT<IdType, LeafType>;
  using NodeDescriptor              = aho_corasick_serialization_headers::NodeDescriptorT<IdType, CharType, CharNode, CharType>;
  using SerializedDataHeader        = aho_corasick_serialization_headers::SerializedDataHeaderT<IdType>;
  using NodeSerializationDescriptorStorage = std::vector<NodeSerializationDescriptor>;
  using LeafSerializationDescriptorStorage = std::vector<LeafSerializationDescriptor>;
  using NodeDescriptorStorage              = std::vector<NodeDescriptor>;

  utils::Version version_; 

  constexpr NodeSerializationDescriptor serialize(const NodeDescriptor &node, IdType depth_level, IdType leaf_id) const noexcept {
    return { node.node_id_, node.parent_node_id_, depth_level, leaf_id, node.parent_node_ch_ };
  }

  constexpr LeafSerializationDescriptor serialize(LeafType leaf, IdType node_id) const noexcept {
    return { node_id, std::move(leaf) };
  }

  constexpr NodeDescriptor deserialize(const NodeSerializationDescriptor &node_descriptor, CharNode *node) const noexcept {
    return { node_descriptor.node_id_, node, node_descriptor.parent_node_id_, node_descriptor.parent_node_ch_ };
  }

  ByteVector constructResultSerializedBuffer(LeafSerializationDescriptorStorage &serialized_leafs, 
      NodeSerializationDescriptorStorage &serialized_nodes) const {
    ByteVector result(sizeof(SerializedDataHeader) + 
        sizeof(NodeSerializationDescriptorStorage::value_type)*serialized_nodes.size() + 
        sizeof(LeafSerializationDescriptorStorage::value_type)*serialized_leafs.size());

    // TODO : to make SerializedDataHeader ctor explicit it is need to add a list-initialization
    SerializedDataHeader header {
        version_,
        static_cast<IdType>(serialized_leafs.size()),
        static_cast<IdType>(serialized_nodes.size()),
        aho_corasick_serialization_headers::ConvertIdType(sizeof(IdType))
    };

    // in VS checked iterator should be disabled to prevent unnecessary checks
    auto result_end = std::end(result);
    auto current_cursor = serialization_utils::SaveAsBytes(std::begin(result), result_end, &header);
    for (const auto &leaf : serialized_leafs) {
      current_cursor = serialization_utils::SaveAsBytes(current_cursor, result_end, &leaf);
    }
    for (const auto &node : serialized_nodes) {
      current_cursor = serialization_utils::SaveAsBytes(current_cursor, result_end, &node);
    }

    return result;
  }

  template <typename Iterator>
  Iterator deserializeDataHeader(const Iterator begin, Iterator end, SerializedDataHeader &header) const {
    auto current_cursor = serialization_utils::LoadFromBytes(begin, end, &header);
    if (begin == current_cursor) {
      throw (exception::YASException("Invalid data size: data size less than size of SerializedDataHeader",
          StorageError::kInvertedIndexDesirializationError));
    }

    if (version_ < header.version_) {
      throw (exception::YASException("Corrupted header: header version unsupported",
          StorageError::kInvertedIndexDesirializationVersionUnsopportedError));
    }

    if (sizeof(IdType) != header.id_type_size_) {
      throw (exception::YASException("Corrupted header: size of IdType mismath",
          StorageError::kInvertedIndexDesirializationError));
    }

    return current_cursor;
  }

  template <typename Iterator>
  Iterator deserializeLeafDescriptors(const Iterator begin, Iterator end,
      IdType leafs_count, LeafSerializationDescriptorStorage &deserialized_leaf_descriptors) const {
    deserialized_leaf_descriptors.reserve(leafs_count);

    auto current_cursor = begin;
    for (IdType leaf_id = 0; leaf_id < leafs_count; ++leaf_id) {
      LeafSerializationDescriptor leaf_descriptor;
      current_cursor = serialization_utils::LoadFromBytes(current_cursor, end, &leaf_descriptor);
      if (begin == current_cursor) {
        throw (exception::YASException("leafs count don't corresponds to header",
            StorageError::kInvertedIndexDesirializationError));
      }
      deserialized_leaf_descriptors.push_back(leaf_descriptor);
    }

    return current_cursor;
  }

  template <typename Iterator>
  Iterator deserializeNodeDescriptor(const Iterator begin, Iterator end,
      NodeSerializationDescriptor &node_descriptor) const {
    auto current_cursor = serialization_utils::LoadFromBytes(begin, end, &node_descriptor);
    if (begin == current_cursor) {
      throw (exception::YASException("Invalid data size: there is a mismatch with size of data and descriptors count",
        StorageError::kInvertedIndexDesirializationError));
    }

    return current_cursor;
  }

  template<typename Iterator>
  Iterator findNodeByParentNodeId(const Iterator begin, const Iterator end, IdType parent_node_id) const {
    auto found_parent_node = std::lower_bound(begin, end, parent_node_id, [](
        const typename std::iterator_traits<Iterator>::value_type &left,
        const IdType right){
      return left.node_id_ < right;
    });
    if (end == found_parent_node || parent_node_id != found_parent_node->node_id_) {
      return end;
    }
    return found_parent_node;
  }

  template<typename Iterator>
  Iterator findLeafByNodeId(const Iterator begin, const Iterator end, IdType node_id) const {
    auto found_leaf = std::lower_bound(begin, end, node_id, [](
          const typename std::iterator_traits<Iterator>::value_type &left,
          const IdType right) {
        return left.node_id_ < right;
    });
    if (end == found_leaf || node_id != found_leaf->node_id_) {
      return end;
    }
    return found_leaf;
  }

  void prepareForNextDeserializationDepth(NodeDescriptorStorage &previous_level_nodes, 
      NodeDescriptorStorage &current_level_nodes) const {
    previous_level_nodes = std::move(current_level_nodes);
    std::sort(std::begin(previous_level_nodes), std::end(previous_level_nodes), [](
        const NodeDescriptorStorage::value_type &left,
        const NodeDescriptorStorage::value_type &right) {
      return left.node_id_ < right.node_id_;
    });
  }
};

} // namespace index_helper
} // namespace yas
