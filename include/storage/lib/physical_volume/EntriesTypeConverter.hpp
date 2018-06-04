#pragma once
#include "../physical_volume/pv_layout_headers.h"
#include "../common/common.h"
#include <unordered_map>
#include <variant>

using namespace yas::pv_layout_headers;

namespace yas {
namespace pv {
namespace entries_types {

// the idea from http://en.cppreference.com/w/cpp/utility/variant/visit

template<typename... Ts> struct make_overload : Ts... { using Ts::operator()...; };
template<typename... Ts> make_overload(Ts...)->make_overload<Ts...>;

template<typename VariantType, typename... Alternatives>
decltype(auto) VisitEntryTypes(VariantType&& variant_type, Alternatives&&... alternatives) {
  return std::visit(
    make_overload{ std::forward<Alternatives>(alternatives)... },
    std::forward<VariantType>(variant_type)
  );
}

template<typename T, typename U, PVType pv_type>
struct EntryTypeHolder {
  using HeaderType = U;
  using ValueType = T;

  explicit EntryTypeHolder(ValueType value)
      : value_(std::move(value))
  {}
  EntryTypeHolder() = default;

  ValueType value_;
  PVType pv_type_ = pv_type;
};

using Int8_EntryType = EntryTypeHolder<int8_t, Simple4TypeHeader, PVType::kInt8>;
using UInt8_EntryType = EntryTypeHolder<uint8_t, Simple4TypeHeader, PVType::kUint8>;
using Int16_EntryType = EntryTypeHolder<int16_t, Simple4TypeHeader, PVType::kInt16>;
using UInt16_EntryType = EntryTypeHolder<uint16_t, Simple4TypeHeader, PVType::kUint16>;
using Int32_EntryType = EntryTypeHolder<int32_t, Simple4TypeHeader, PVType::kInt32>;
using UInt32_EntryType = EntryTypeHolder<uint32_t, Simple4TypeHeader, PVType::kUint32>;
using Float_EntryType = EntryTypeHolder<float, Simple4TypeHeader, PVType::kFloat>;
using Int64_EntryType = EntryTypeHolder<int64_t, Simple8TypeHeader, PVType::kInt64>;
using UInt64_EntryType = EntryTypeHolder<uint64_t, Simple8TypeHeader, PVType::kUint64>;
using Double_EntryType = EntryTypeHolder<double, Simple8TypeHeader, PVType::kDouble>;
using String_EntryType = EntryTypeHolder<std::string, ComplexTypeHeader, PVType::kString>;
using Blob_EntryType = EntryTypeHolder<ByteVector, ComplexTypeHeader, PVType::kBlob>;

using EntryType = std::variant<Int8_EntryType,
                               UInt8_EntryType,
                               Int16_EntryType,
                               UInt16_EntryType,
                               Int32_EntryType,
                               UInt32_EntryType,
                               Float_EntryType,
                               Int64_EntryType,
                               UInt64_EntryType,
                               Double_EntryType,
                               String_EntryType,
                               Blob_EntryType>;

static_assert(std::variant_size_v<EntryType> == PVType::kBlob+1, "To add new storage types please change PVType and EntryType simultaneously");
static_assert(std::variant_size_v<EntryType> == std::variant_size_v<storage_value_type>, "To add new storage types please change storage_value_type and EntryType simultaneously");

class EntriesTypeConverter {
 public:
  static EntryType ConvertToEntryType(PVType pv_type) {
    switch (pv_type) {
    case PVType::kInt8:
      return Int8_EntryType();
    case PVType::kUint8:
      return UInt8_EntryType();
    case PVType::kInt16:
      return Int16_EntryType();
    case PVType::kUint16:
      return UInt16_EntryType();
    case PVType::kInt32:
      return Int32_EntryType();
    case PVType::kUint32:
      return UInt32_EntryType();
    case PVType::kFloat:
      return Float_EntryType();
    case PVType::kInt64:
      return Int64_EntryType();
    case PVType::kUint64:
      return UInt64_EntryType();
    case PVType::kDouble:
      return Double_EntryType();
    case PVType::kString:
      return String_EntryType();
    case PVType::kBlob:
      return Blob_EntryType();
    }

    throw (exception::YASException("Corrupted storage header type: the unsupported entry type",
        StorageError::kCorruptedHeaderError));
  }

  template<typename Type>
  static EntryType ConvertToEntryType(PVType pv_type, Type &&value) {
    switch (pv_type) {
    case PVType::kInt8:
      return Int8_EntryType(static_cast<Int8_EntryType::ValueType>(value));
    case PVType::kUint8:
      return UInt8_EntryType(static_cast<UInt8_EntryType::ValueType>(value));
    case PVType::kInt16:
      return Int16_EntryType(static_cast<Int16_EntryType::ValueType>(value));
    case PVType::kUint16:
      return UInt16_EntryType(static_cast<UInt16_EntryType::ValueType>(value));
    case PVType::kInt32:
      return Int32_EntryType(static_cast<Int32_EntryType::ValueType>(value));
    case PVType::kUint32:
      return UInt32_EntryType(static_cast<UInt32_EntryType::ValueType>(value));
    case PVType::kFloat:
      static_assert(std::numeric_limits<float>::is_iec559, "The YAS requires using of IEEE 754 floating point format for binary serialization of floats");
      return Float_EntryType(*(reinterpret_cast<const Float_EntryType::ValueType *>(&value)) );
    case PVType::kInt64:
      return Int64_EntryType(static_cast<Int64_EntryType::ValueType>(value));
    case PVType::kUint64:
      return UInt64_EntryType(static_cast<UInt64_EntryType::ValueType>(value));
    case PVType::kDouble:
      static_assert(std::numeric_limits<double>::is_iec559, "The YAS requires using of IEEE 754 floating point format for binary serialization of doubles");
      return Double_EntryType(*(reinterpret_cast<const Double_EntryType::ValueType *>(&value)));
    }

    throw (exception::YASException("Corrupted storage header type: the unsupported entry type",
        StorageError::kCorruptedHeaderError));
  }

  static storage_value_type ConvertToUserType(EntryType &&storage_value) {
    return std::visit([](auto&& storage_value_element) -> storage_value_type { return storage_value_element.value_; },
        storage_value);
  }
};

template<>
EntryType EntriesTypeConverter::ConvertToEntryType<ByteVector>(PVType pv_type, ByteVector &&value) {
  switch (pv_type) {
  case PVType::kString: {
    String_EntryType result_value;
    result_value.value_ = { std::cbegin(value), std::cend(value) };
    return result_value;
  }
  case PVType::kBlob:
    return Blob_EntryType(std::forward<ByteVector>(value));
  }

  throw (exception::YASException("Corrupted storage header type: the unsupported entry type",
    StorageError::kCorruptedHeaderError));
}

} // namespace storage_types
} // namespace pv
} // namespace yas
