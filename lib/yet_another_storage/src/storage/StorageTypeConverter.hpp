#pragma once
#include "../physical_volume/pv_layout_headers.h"
#include "../common/common.h"
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <any>

using namespace yas::pv_layout_headers;

namespace yas {
namespace storage {

class TypeConverter {
 public:
  TypeConverter() {
     init();
  }

  PVType ConvertToPVType(const std::any &value) {
    return user_to_pv_type_mapping_[std::type_index(value.type())];
  }

  std::any ConvertToUserType(PVType pv_type, uint64_t value) const noexcept {
    // TODO
    switch (pv_type) {
    case PVType::kInt8:
      return static_cast<int8_t>(value);
    case PVType::kUint8:
      return static_cast<uint8_t>(value);
    case PVType::kInt16:
      return static_cast<int16_t>(value);
    case PVType::kUint16:
      return static_cast<uint16_t>(value);
    case PVType::kInt32:
      return static_cast<int32_t>(value);
    case PVType::kUint32:
      return static_cast<uint32_t>(value);
    case PVType::kFloat:
      static_assert(std::numeric_limits<float>::is_iec559, "The YAS requires using of IEEE 754 floating point format for binary serialization of floats and doubles.");
      return *(reinterpret_cast<float *>(&value));
    case PVType::kDouble:
      static_assert(std::numeric_limits<double>::is_iec559, "The YAS requires using of IEEE 754 floating point format for binary serialization of floats and doubles.");
      return *(reinterpret_cast<double *>(&value));
    case PVType::kInt64:
      return static_cast<int64_t>(value);
    case PVType::kUint64:
      return static_cast<uint64_t>(value);
    default:
      return 0;
    }
  }

  template<typename Iterator>
  std::any ConvertToUserType(PVType pv_type, const Iterator begin, const Iterator end) const {
    switch (pv_type) {
    case PVType::kString:
      return std::string{ begin, end };
    case PVType::kBlob:
      return ByteVector{ begin, end };
    default:
      return 0;
    }
  }

 private:
   std::unordered_map<std::type_index, PVType> user_to_pv_type_mapping_;

   void init() {
     // std::type_index hasn't constexpr ctor -> we can't easy make this compile-time
     user_to_pv_type_mapping_[std::type_index(typeid(int8_t))] = PVType::kInt8;
     user_to_pv_type_mapping_[std::type_index(typeid(uint8_t))] = PVType::kUint8;
     user_to_pv_type_mapping_[std::type_index(typeid(int16_t))] = PVType::kInt16;
     user_to_pv_type_mapping_[std::type_index(typeid(uint16_t))] = PVType::kUint16;
     user_to_pv_type_mapping_[std::type_index(typeid(int32_t))] = PVType::kInt32;
     user_to_pv_type_mapping_[std::type_index(typeid(uint32_t))] = PVType::kUint32;
     user_to_pv_type_mapping_[std::type_index(typeid(float))] = PVType::kFloat;
     user_to_pv_type_mapping_[std::type_index(typeid(double))] = PVType::kDouble;
     user_to_pv_type_mapping_[std::type_index(typeid(int64_t))] = PVType::kInt64;
     user_to_pv_type_mapping_[std::type_index(typeid(uint64_t))] = PVType::kUint64;
     user_to_pv_type_mapping_[std::type_index(typeid(std::string))] = PVType::kString;
     user_to_pv_type_mapping_[std::type_index(typeid(ByteVector))] = PVType::kBlob;
   }
};

} // namespace storage
} // namespace yas
