#pragma once

#include "AhoCorasickEngine.hpp"
#include "../../utils/serialization_utils.h"
#include <cstdint>
#include <vector>
#include <type_traits>
#include <numeric>
#include <algorithm>

namespace yas {
namespace index_helper {

template <typename CharType, typename LeafType, typename IdType>
class AhoCorasickSerializationHelper {
  using Engine = AhoCorasickEngine<CharType, LeafType>;
  template<typename CharType>
  struct Node {
    Node() : leaf_(LeafType::MakeNonExistType()) {}
    std::unordered_map<CharType, std::unique_ptr<Node>> routes_;
    LeafType leaf_;
  };
  using CharNode = Node<CharType>;

 public:
  static_assert(std::is_integral_v<IdType>, "IdType should be an integral type");
  AhoCorasickSerializationHelper() = default;
  ~AhoCorasickSerializationHelper() = default;

  std::vector<uint8_t> Serialize(const Engine &engine) const {
    NodeSerializationDescriptorStorage serialized_nodes;
    LeafSerializationDescriptorStorage serialized_leafs;
    NodeDescriptorStorage current_level_nodes;
    NodeDescriptorStorage next_level_nodes;
    
    // TODO : change type conversion after TODO_1
    // TODO : emplace back
    current_level_nodes.push_back({ 0, (CharNode *)engine.root_.get(), 0, (CharType)'/' });

    IdType current_node_id = 0;
    IdType current_leaf_id = 0;
    IdType parent_node_id = 0;
    IdType depth_level = 0;
    
    // breadth-first search
    while (!current_level_nodes.empty()) {
      for (const auto &node_descriptor : current_level_nodes) {
        auto leaf_id = std::numeric_limits<IdType>::max();     // non exist leaf
        if (node_descriptor.node_->leaf_ != LeafType::MakeNonExistType()) {
          leaf_id = current_leaf_id;
          serialized_leafs.push_back(serialize(node_descriptor.node_->leaf_, current_node_id));
          ++current_leaf_id;
        }
        serialized_nodes.push_back(serialize(node_descriptor, depth_level, leaf_id));

        next_level_nodes.reserve(node_descriptor.node_->routes_.size());
        for(auto &route : node_descriptor.node_->routes_) {
          // TODO : emplace back
          next_level_nodes.push_back({ current_node_id, route.second.get(), node_descriptor.node_id_, route.first });
          ++current_node_id;
        }

      }
      ++depth_level;
      current_level_nodes = std::move(next_level_nodes);
      next_level_nodes.clear();
    }

    return constructResultSerializedBuffer(serialized_nodes, serialized_leafs);
  }

 private:
  struct NodeSerializationDescriptor {
    IdType node_id_;
    IdType parent_node_id_;
    IdType depth_level_;
    IdType leaf_id_;
    CharType parent_node_ch_;     // should be the last one - because of alignment
  };
  
  struct LeafSerializationDescriptor {
    IdType node_id_;
    LeafType leaf_;
  };

  struct NodeDescriptor {
    IdType node_id_;
    CharNode *node_;
    IdType parent_node_id_;
    CharType parent_node_ch_;
  };

  struct SerializedDataHeader {
    Version version_;
    IdType leafs_count_;
    IdType nodes_count_;

    enum IdTypeSize : uint8_t {k4Byte = 4, k8Byte = 8};
    IdTypeSize id_type_size_;          // also can determine by version but its better to explicitly check it

    static uint32_t ConvertIdType(IdTypeSize id_type_size) {
      return id_type_size;
    }
    static IdTypeSize ConvertIdType(uint32_t size) {
      return static_cast<IdTypeSize>(size);
    }
  };
  using NodeSerializationDescriptorStorage = std::vector<NodeSerializationDescriptor>;
  using LeafSerializationDescriptorStorage = std::vector<LeafSerializationDescriptor>;
  using NodeDescriptorStorage = std::vector<NodeDescriptor>;
  Version version_ = {1,1}; // TODO

  NodeSerializationDescriptor serialize(const NodeDescriptor &node, IdType depth_level, IdType leaf_id) const noexcept {
    return { node.node_id_, node.parent_node_id_, depth_level, leaf_id, node.parent_node_ch_ };
  }

  LeafSerializationDescriptor serialize(LeafType leaf, IdType node_id) const noexcept {
    return { node_id, std::move(leaf) };
  }

  NodeDescriptor deserialize(const NodeSerializationDescriptor &node_descriptor, CharNode *node) const noexcept {
    return { node_descriptor.node_id_, node, node_descriptor.parent_node_id_, node_descriptor.parent_node_ch_ };
  }

  std::vector<uint8_t> constructResultSerializedBuffer(NodeSerializationDescriptorStorage &serialized_nodes, 
      LeafSerializationDescriptorStorage &serialized_leafs) const {
    std::vector<uint8_t> result(sizeof(SerializedDataHeader) + serialized_nodes.size() + serialized_leafs.size());

    SerializedDataHeader header = { version_, serialized_leafs.size(), serialized_nodes.size(),
        SerializedDataHeader::ConvertIdType(sizeof(IdType))};

    // for VS checked iterator should be disabled
    auto current_cursor = serialization_utils::SaveAsBytes(std::begin(result), std::end(result), 
        reinterpret_cast<uint8_t *>(&header));
    current_cursor = std::copy(reinterpret_cast<uint8_t*>(serialized_leafs.data()),
        reinterpret_cast<uint8_t*>(serialized_leafs.data() + serialized_leafs.size()), current_cursor);
    std::copy(reinterpret_cast<uint8_t*>(serialized_nodes.data()),
      reinterpret_cast<uint8_t*>(serialized_nodes.data() + serialized_nodes.size()), current_cursor);

    return result;
  }

};

} // namespace index_helper
} // namespace yas
