#pragma once
#include "AhoCorasickEngine.hpp"
#include "aho_corasick_serialization_headers.hpp"
#include "../../utils/serialization_utils.h"
#include "../../utils/Version.hpp"
#include "../../common/common.h"
#include "id_type_type_traits.hpp"
#include "../../exceptions/YASException.h"
#include <cstdint>
#include <vector>
#include <type_traits>
#include <numeric>
#include <algorithm>

using namespace yas::storage;

namespace yas {
namespace index_helper {

template <typename CharType, typename LeafType, typename IdType>
class AhoCorasickSerializationHelper {
  using Engine = AhoCorasickEngine<CharType, LeafType>;
  template<typename CharType>
  struct Node {
    Node() : leaf_(leaf_traits<Leaf>::NonExistValue()) {}
    std::unordered_map<CharType, std::unique_ptr<Node>> routes_;
    LeafType leaf_;
  };
  using CharNode = Node<CharType>;

 public:
  static_assert(std::is_integral_v<IdType>, "IdType should be an integral type");

  AhoCorasickSerializationHelper(utils::Version version) 
      : version_(version)
  {}
  ~AhoCorasickSerializationHelper() = default;

  ByteVector Serialize(const Engine &engine) const {
    NodeSerializationDescriptorStorage serialized_nodes;
    LeafSerializationDescriptorStorage serialized_leafs;
    NodeDescriptorStorage current_level_nodes;
    NodeDescriptorStorage next_level_nodes;
    
    // TODO (!!) : bad type conversion (need to investigate how VS can view inner struct from friends class)
    current_level_nodes.emplace_back(0, reinterpret_cast<CharNode*>(engine.trie_.get()), 0,
        std::char_traits<CharType>::to_char_type('/'));

    IdType current_node_id = 0;
    IdType current_leaf_id = 0;
    IdType parent_node_id = 0;
    IdType depth_level = 0;
    
    // breadth-first search
    while (!current_level_nodes.empty()) {
      for (const auto &node_descriptor : current_level_nodes) {
        auto leaf_id = id_type_traits<IdType>::NonExistValue();
        if (node_descriptor.node_->leaf_ != leaf_traits<Leaf>::NonExistValue()) {
          leaf_id = current_leaf_id;
          serialized_leafs.push_back(serialize(node_descriptor.node_->leaf_, current_node_id));
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
      next_level_nodes.clear();
    }

    return constructResultSerializedBuffer(serialized_nodes, serialized_leafs);
  }

  void Deserialize(ByteVector &data, Engine &engine) const {
    if (data.size() < sizeof(SerializedDataHeader)) {
      throw (exception::YASException("Invalid data size: < sizeof(SerializedDataHeader)", 
          kInvertedIndexDesirializationError));
    }

    SerializedDataHeader header;
    auto current_cursor = serialization_utils::LoadFromBytes(std::begin(data), std::end(data), &header);
    // TODO : exception if end
    checkSerializedHeader(header);
    const auto serialized_leafs = deserializeLeafDecriptors(header, data, current_cursor);
    NodeSerializationDescriptor node_descriptor;
    current_cursor = serialization_utils::LoadFromBytes(current_cursor, std::end(data), &node_descriptor);
    if (node_descriptor.node_id_ != node_descriptor.parent_node_id_) {
      throw (exception::YASException("Corrupt data: multiple roots found",
          kInvertedIndexDesirializationError));
    }

    std::unique_ptr<CharNode> root(new CharNode());         // at first completly construct the trie on function level
                                                            // and only then modify engine to exception safety
    NodeDescriptorStorage prev_level_nodes;
    NodeDescriptorStorage current_level_nodes;
    prev_level_nodes.push_back(deserialize(node_descriptor, root.get()));

    IdType depth_level = 1;                                     // root extracted -> depth_level should be 1
    for (IdType node_id = 1; node_id < header.nodes_count_; ++node_id) {
      current_cursor = serialization_utils::LoadFromBytes(current_cursor, std::end(data), &node_descriptor);
      const auto parent = getAppropriateParentNode(prev_level_nodes, node_descriptor.parent_node_id_);
      if (std::cend(prev_level_nodes) == parent) {
        throw (exception::YASException("Corrupt data: parent node can't be found",
            kInvertedIndexDesirializationError));
      }
      parent->node_->routes_[node_descriptor.parent_node_ch_].reset(new CharNode());
      const auto route = parent->node_->routes_[node_descriptor.parent_node_ch_].get();
      current_level_nodes.push_back(deserialize(node_descriptor, route));

      if (node_descriptor.leaf_id_ != id_type_traits<IdType>::NonExistValue()) {
        const auto found_leaf_iterator = getAppropriateLeaf(serialized_leafs, node_descriptor.leaf_id_);
        if (std::cend(serialized_leafs) == found_leaf_iterator) {
          throw (exception::YASException("Corrupt data: node's leaf can't be found",
              kInvertedIndexDesirializationError));
        }
        route->leaf_ = found_leaf_iterator->leaf_;
      }

      if (depth_level < node_descriptor.depth_level_) {
        // TODO : move all if to explicit function to improve readability
        prev_level_nodes = std::move(current_level_nodes);
        std::sort(std::begin(prev_level_nodes), std::end(prev_level_nodes), [](
            const NodeDescriptorStorage::value_type &left,
            const NodeDescriptorStorage::value_type &right) {
          return left.node_id_ < right.node_id_;
        });
        {
          // there was stl realization that doesn't decrease size after move :(
          NodeDescriptorStorage tmp;
          current_level_nodes.swap(tmp);
        }
        ++depth_level;
      }
    }

    // TODO (!!) : bad type conversion (need to investigate how VS can view inner struct from friends class)
    engine.trie_.swap(*(std::unique_ptr<AhoCorasickEngine<CharType, LeafType>::CharNode>*)(&root));
  }

 private:
  using NodeSerializationDescriptor = aho_corasick_serialization_headers::NodeSerializationDescriptorT<IdType, CharType>;
  using LeafSerializationDescriptor = aho_corasick_serialization_headers::LeafSerializationDescriptorT<IdType, LeafType>;
  using NodeDescriptor = aho_corasick_serialization_headers::NodeDescriptorT<IdType, CharType, CharNode, CharType>;
  using SerializedDataHeader = aho_corasick_serialization_headers::SerializedDataHeaderT<IdType>;
  using NodeSerializationDescriptorStorage = std::vector<NodeSerializationDescriptor>;
  using LeafSerializationDescriptorStorage = std::vector<LeafSerializationDescriptor>;
  using NodeDescriptorStorage = std::vector<NodeDescriptor>;

  utils::Version version_; 

  NodeSerializationDescriptor serialize(const NodeDescriptor &node, IdType depth_level, IdType leaf_id) const noexcept {
    return { node.node_id_, node.parent_node_id_, depth_level, leaf_id, node.parent_node_ch_ };
  }

  LeafSerializationDescriptor serialize(LeafType leaf, IdType node_id) const noexcept {
    return { node_id, std::move(leaf) };
  }

  NodeDescriptor deserialize(const NodeSerializationDescriptor &node_descriptor, CharNode *node) const noexcept {
    return { node_descriptor.node_id_, node, node_descriptor.parent_node_id_, node_descriptor.parent_node_ch_ };
  }

  ByteVector constructResultSerializedBuffer(NodeSerializationDescriptorStorage &serialized_nodes,
      LeafSerializationDescriptorStorage &serialized_leafs) const {
    ByteVector result(sizeof(SerializedDataHeader) + serialized_nodes.size() + serialized_leafs.size());

    SerializedDataHeader header = { 
        version_, 
        static_cast<IdType>(serialized_leafs.size()), 
        static_cast<IdType>(serialized_nodes.size()),
        SerializedDataHeader::ConvertIdType(sizeof(IdType))
    };

    // for VS checked iterator should be disabled
    auto current_cursor = serialization_utils::SaveAsBytes(std::begin(result), std::end(result), 
        reinterpret_cast<uint8_t *>(&header));
    current_cursor = std::copy(reinterpret_cast<uint8_t*>(serialized_leafs.data()),
        reinterpret_cast<uint8_t*>(serialized_leafs.data() + serialized_leafs.size()), current_cursor);
    std::copy(reinterpret_cast<uint8_t*>(serialized_nodes.data()),
        reinterpret_cast<uint8_t*>(serialized_nodes.data() + serialized_nodes.size()), current_cursor);

    return result;
  }

  void checkSerializedHeader(SerializedDataHeader &header) const {
    if (version_ < header.version_) {
      throw (exception::YASException("Corrupted header: header version unsupported",
          kInvertedIndexDesirializationVersionUnsopportedError));
    }

    if (header.id_type_size_ != sizeof(IdType)) {
      throw (exception::YASException("Corrupted header: size of IdType mismath",
          kInvertedIndexDesirializationError));
    }
  }

  // this function return sorted leaf decriptors
  template <typename Iterator>
  LeafSerializationDescriptorStorage deserializeLeafDecriptors(SerializedDataHeader header, ByteVector &data,
      Iterator current_cursor) const {
    LeafSerializationDescriptorStorage serialized_leafs;
    serialized_leafs.reserve(header.leafs_count_);

    for (IdType leaf_id = 0; leaf_id <= header.leafs_count_; ++leaf_id) {
      LeafSerializationDescriptor leaf_descriptor;
      current_cursor = serialization_utils::LoadFromBytes(current_cursor, std::end(data), &leaf_descriptor);
      if (std::end(data) == current_cursor) {
        throw (exception::YASException("leafs count don't corresponds to header",
            kInvertedIndexDesirializationError));
      }
      serialized_leafs.push_back(std::move(leaf_descriptor));
    }
    std::sort(std::begin(serialized_leafs), std::end(serialized_leafs),
        [](const LeafSerializationDescriptorStorage::value_type &left,
        const LeafSerializationDescriptorStorage::value_type &right) {
      return left.node_id_ < right.node_id_;
    });

    return serialized_leafs;
  }

  decltype(auto) getAppropriateParentNode(const NodeDescriptorStorage &nodes, IdType parent_node_id) const {
    auto parent_node = std::lower_bound(std::begin(nodes), std::end(nodes), parent_node_id, [](
        const NodeDescriptorStorage::value_type &left,
        const IdType right){
      return left.parent_node_id_ < right;
    });
    if (std::end(nodes) == parent_node || parent_node_id != parent_node->node_id_) {
      return std::end(nodes);
    }
    return parent_node;
  }

  decltype(auto) getAppropriateLeaf(const LeafSerializationDescriptorStorage &leafs, IdType node_id) const {
    auto leaf = std::lower_bound(std::begin(leafs), std::end(leafs), node_id, [](
          const LeafSerializationDescriptorStorage::value_type &left,
          IdType right) {
        return left.node_id_ < right;
    });
    if (std::cend(leafs) == leaf || node_id != leaf->node_id_) {
      return std::end(leafs);
    }
    return leaf;
  }
};

} // namespace index_helper
} // namespace yas
