#pragma once
#include "../physical_volume/pv_layout_headers.h"
#include "../common/common.h"
#include <unordered_map>
#include <typeindex>
#include <any>
#include <variant>

using namespace yas::pv_layout_headers;

namespace yas {
namespace pv {

template<typename T, typename U>
struct StorageTypeHolder {
  using HeaderType = U;
  using ValueType = T;
  ValueType value;
};

using Int8_StorageType = StorageTypeHolder<int8_t, Simple4TypeHeader>;
using UInt8_StorageType = StorageTypeHolder<uint8_t, Simple4TypeHeader>;
using Int16_StorageType = StorageTypeHolder<int16_t, Simple4TypeHeader>;
using UInt16_StorageType = StorageTypeHolder<uint16_t, Simple4TypeHeader>;
using Int32_StorageType = StorageTypeHolder<int32_t, Simple4TypeHeader>;
using UInt32_StorageType = StorageTypeHolder<uint32_t, Simple4TypeHeader>;
using Float_StorageType = StorageTypeHolder<float, Simple4TypeHeader>;
using Int64_StorageType = StorageTypeHolder<int64_t, Simple8TypeHeader>;
using UInt64_StorageType = StorageTypeHolder<uint64_t, Simple8TypeHeader>;
using Double_StorageType = StorageTypeHolder<double, Simple8TypeHeader>;
using String_StorageType = StorageTypeHolder<std::string, ComplexTypeHeader>;
using Blob_StorageType = StorageTypeHolder<ByteVector, ComplexTypeHeader>;

using StorageType = std::variant<Int8_StorageType,
                                 UInt8_StorageType,
                                 Int16_StorageType,
                                 UInt16_StorageType,
                                 Int32_StorageType,
                                 UInt32_StorageType,
                                 Float_StorageType,
                                 Int64_StorageType,
                                 UInt64_StorageType,
                                 Double_StorageType,
                                 String_StorageType,
                                 Blob_StorageType>;


static_assert(std::variant_size_v<StorageType> == PVType::kBlob+1, "To add new storage types please change PVType and StorageType simultaneously");

class EntriesTypeConverter {
 public:
   EntriesTypeConverter() {
     init();
  }

  static StorageType ConvertToStorageType(PVType pv_type) {
    switch (pv_type) {
    case PVType::kInt8:
      return Int8_StorageType();
    case PVType::kUint8:
      return UInt8_StorageType();
    case PVType::kInt16:
      return Int16_StorageType();
    case PVType::kUint16:
      return UInt16_StorageType();
    case PVType::kInt32:
      return Int32_StorageType();
    case PVType::kUint32:
      return UInt32_StorageType();
    case PVType::kFloat:
      return Float_StorageType();
    case PVType::kInt64:
      return Int64_StorageType();
    case PVType::kUint64:
      return UInt64_StorageType();
    case PVType::kDouble:
      return Double_StorageType();
    case PVType::kString:
      return String_StorageType();
    case PVType::kBlob:
      return Blob_StorageType();
    }
    throw (exception::YASException("Corrupted storage header type: unsupported entry type",
        StorageError::kCorruptedHeaderError));
  }

  PVType ConvertToPVType(storage_value_type value) {
    return user_to_pv_type_mapping_[std::type_index(value.type())];
  }

  storage_value_type ConvertToUserType(PVType pv_type, uint64_t value) const noexcept {
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
  storage_value_type ConvertToUserType(PVType pv_type, const Iterator begin, const Iterator end) const {
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

} // namespace pv
} // namespace yas
