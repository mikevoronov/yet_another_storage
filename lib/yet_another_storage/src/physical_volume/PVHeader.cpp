#pragma once
#include "pv_layout_headers.h"
#include "../utils/serialization_utils.h"
#include "../common/common.h"

namespace yas {
namespace pv_layout_headers {

template<typename OffsetType, typename Device>
PVHeader PVHeader::Read(Device<OffsetType> &device) {
  ByteVector raw_bytes(sizeof(PVHeader));
  device.read(0, std::begin(raw_bytes), std::end(raw_bytes));

  PVHeader header;
  serialization_utils::LoadFromBytes(std::begin(raw_bytes), std::end(raw_bytes), header);
  return header;
}

template<typename OffsetType, typename Device>
void PVHeader::Write(Header &header, Device<OffsetType> &device) {
  ByteVector raw_bytes(sizeof(PVHeader));
  serialization_utils::SaveAsBytes(std::cbegin(raw_bytes), std::cend(raw_bytes), &header);

  device.write(0, std::begin(raw_bytes), std::end(raw_bytes));
}

} // namespace pv_layout_headers
} // namespace yas
