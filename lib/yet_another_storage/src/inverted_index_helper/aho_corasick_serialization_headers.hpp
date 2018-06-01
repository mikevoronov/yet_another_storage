#pragma once
#include "../utils/Version.hpp"
#include "../common/macros.h"
#include "leaf_type_traits.hpp"
#include <cstdint>
#include <type_traits>    // for underlying_type_t

using namespace yas::macros;

namespace yas {
namespace index_helper {
namespace aho_corasick_serialization_headers {

STRUCT_PACK(
template<typename IdType, typename CharType>
struct NodeSerializationDescriptorT {
  constexpr NodeSerializationDescriptorT(IdType node_id = 0,
      IdType parent_node_id = 0,
      IdType depth_level = 0,
      IdType leaf_id = 0,
      CharType parent_node_ch = std::char_traits<CharType>::to_char_type('/'))
      : node_id_(node_id),
        parent_node_id_(parent_node_id),
        depth_level_(depth_level),
        leaf_id_(leaf_id),
        parent_node_ch_(parent_node_ch)
  {}

  IdType node_id_;
  IdType parent_node_id_;
  IdType depth_level_;
  IdType leaf_id_;
  CharType parent_node_ch_;     // should be the last one - because of alignment
});

STRUCT_PACK(
template<typename IdType, typename LeafType>
struct LeafSerializationDescriptorT {
  constexpr LeafSerializationDescriptorT(IdType node_id = 0,
      LeafType leaf = leaf_traits<LeafType>::NonExistValue())
      : node_id_(node_id),
        leaf_(leaf)
  {}

  IdType node_id_;
  LeafType leaf_;
});

STRUCT_PACK(
template<typename IdType, typename LeafType, typename CharNode, typename CharType>
struct NodeDescriptorT {
  constexpr NodeDescriptorT(IdType node_id,
      CharNode *node,
      IdType parent_node_id,
      CharType parent_node_ch)
      : node_id_(node_id),
        node_(node),
        parent_node_id_(parent_node_id),
        parent_node_ch_(parent_node_ch)
  {}

  IdType node_id_;
  CharNode *node_;
  IdType parent_node_id_;
  CharType parent_node_ch_;
});

enum IdTypeSize : uint8_t { k4Byte = 4, k8Byte = 8 };
// from Effective and modern C++
template <typename ValueType>
constexpr auto to_underlying(ValueType value) noexcept {
  return static_cast<std::underlying_type_t<ValueType>>(value);
}

constexpr auto ConvertIdType(IdTypeSize id_type_size) {
  return to_underlying(id_type_size);
}
constexpr auto ConvertIdType(uint32_t size) {
  return static_cast<IdTypeSize>(size);
}

STRUCT_PACK(
template <typename IdType>
struct SerializationDataHeaderT {
  SerializationDataHeaderT(utils::Version version = {1,1},
      IdType leafs_count = 0,
      IdType nodes_count = 0,
      IdTypeSize id_type_size = IdTypeSize::k4Byte)
      : version_(version),
        leafs_count_(leafs_count),
        nodes_count_(nodes_count),
        id_type_size_(id_type_size)
  {}

  utils::Version version_;
  IdType leafs_count_;
  IdType nodes_count_;
  IdTypeSize id_type_size_;          // also can determine by version but its better to explicitly check it
});

} // namespace aho_corasick_serialization_headers
} // namespace index_helper
} // namespace yas
