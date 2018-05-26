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
namespace types {

struct StorageType {};
struct Simple4Type : public StorageType { };
struct Simple8Type : public StorageType { };
struct ComplexType : public StorageType { };

class TypeConverter {
 public:
  TypeConverter() {
     init();
  }

  PVType ConvertToPVType(std::any &value) {
    return user_to_pv_type_mapping_[std::type_index(value.type())];
  }

  template <typename StorageType, typename UserType>
  static constexpr StorageType ConvertToStorageType(UserType type) {
    if constexpr (sizeof(type) <= 4) {
      return Simple4Type;
    }
    else if constexpr (sizeof(type) <= 8) {
      return Simple8Type;
    }
    return ComplexType;
  }

  template <typename StorageType>
  constexpr StorageType ConvertToStorageType(PVType type) {
    return pv_to_storage_type_mapping_[type];
  }

 private:
   std::unordered_map<std::type_index, PVType> user_to_pv_type_mapping_;
   std::unordered_map<PVType, std::type_index> pv_to_storage_type_mapping_;

   void init() {
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
     user_to_pv_type_mapping_[std::type_index(typeid(std::string))] = PVType::kComplexBegin;
     user_to_pv_type_mapping_[std::type_index(typeid(ByteVector))] = PVType::kComplexBegin;
     user_to_pv_type_mapping_[std::type_index(typeid(void))] = PVType::kEmpty;

     pv_to_storage_type_mapping_.insert(std::make_pair(PVType::kInt8, std::type_index(typeid(Simple4Type))));
     pv_to_storage_type_mapping_.insert(std::make_pair(PVType::kUint8, std::type_index(typeid(Simple4Type))));
     pv_to_storage_type_mapping_.insert(std::make_pair(PVType::kInt16, std::type_index(typeid(Simple4Type))));
     pv_to_storage_type_mapping_.insert(std::make_pair(PVType::kUint16, std::type_index(typeid(Simple4Type))));
     pv_to_storage_type_mapping_.insert(std::make_pair(PVType::kInt32, std::type_index(typeid(Simple4Type))));
     pv_to_storage_type_mapping_.insert(std::make_pair(PVType::kUint32, std::type_index(typeid(Simple4Type))));
     pv_to_storage_type_mapping_.insert(std::make_pair(PVType::kFloat, std::type_index(typeid(Simple4Type))));
     pv_to_storage_type_mapping_.insert(std::make_pair(PVType::kDouble, std::type_index(typeid(Simple8Type))));
     pv_to_storage_type_mapping_.insert(std::make_pair(PVType::kInt64, std::type_index(typeid(Simple8Type))));
     pv_to_storage_type_mapping_.insert(std::make_pair(PVType::kUint64, std::type_index(typeid(Simple8Type))));
     pv_to_storage_type_mapping_.insert(std::make_pair(PVType::kComplexBegin, std::type_index(typeid(ComplexType))));
     pv_to_storage_type_mapping_.insert(std::make_pair(PVType::kComplexBegin, std::type_index(typeid(ComplexType))));
     pv_to_storage_type_mapping_.insert(std::make_pair(PVType::kEmpty, std::type_index(typeid(StorageType))));
   }
};


} // namespace types
} // namespace storage
} // namespace yas
