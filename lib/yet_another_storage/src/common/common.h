#pragma once
#include <cstdint>
#include <vector>
#include <variant>

namespace yas {

using ByteVector = std::vector<uint8_t>;

using storage_value_type = std::variant<int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, float, int64_t, uint64_t, \
    double, ByteVector, std::string>;

} // namespace yas
