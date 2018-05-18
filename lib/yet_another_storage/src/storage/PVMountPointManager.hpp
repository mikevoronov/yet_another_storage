#pragma once

// TODO : too many headers...
#include "../physical_volume/physical_volume_layout/pv_layout_headers.h"
#include "../physical_volume/physical_volume_layout/pv_layout_types_headers.h"
#include "../physical_volume/inverted_index_helper/InvertedIndexHelper.hpp"
#include "../physical_volume/freelist_helper/FreelistHelper.hpp"
#include "../physical_volume/freelist_helper/FreelistStrategy.hpp"
#include "../physical_volume/inverted_index_helper/aho_corasick_serialization_headers.hpp"
#include "../device_worker/PVDeviceWorker.hpp"
#include "../device_worker/devices/FileDevice.hpp"
#include "../common/settings.hpp"
#include <string_view>

using namespace yas::pv_layout_headers;
using namespace yas::pv_layout_types_headers;

namespace yas {
namespace storage {

template<typename CharType>
class PVMountPointManager {
  using Path = std::string_view<CharType>;
 public:
  ~PVMountPointManager() = default;

  static PVMountPointManager Load(Path path, utils::Version version) {
    PVMountPointManager<CharType> mount_point_manager(path, version);

    PVHeader pv_header = device_worker_.Read<PVHeader>(0);
    checkPVHeader(pv_header);

    const auto freelist_header = device_worker_.Read<FreelistHeader>(sizeof(PVHeader));
    mount_point_manager.freelist_helper_ = freelist_helper::FreelistHelper::Load(freelist_header);
    ByteVector inverted_index = 
        device_worker_.Read < aho_corasick_serialization_headers::SerializedDataHeaderT<OffsetType>.Read(sizeof(PVHeader) + pv_header.inverted_index_size_);
    mount_point_manager.inverted_index_helper_ = InvertedIndexHelper::Deserialize(inverted_index, version_);

    return mount_point_manager;
  }

  PVMountPointManager(const PVMountPointManager&) = delete;
  PVMountPointManager operator=(const PVMountPointManager&) = delete;

 private:
  device_worker::PVDeviceWorker<devices::FileDevice<OffsetType>, OffsetType> device_worker_;
  freelist_helper::FreelistHelper<freelist_helper::FreelistStrategy, OffsetType> freelist_helper_;
  index_helper::InvertedIndexHelper<CharType, OffsetType> inverted_index_helper_;
  utils::Version version_;

  PVMountPointManager(Path path, const utils::Version &version)
      : device_worker_(path),
        version_(version)
  {}

  PVMountPointManager(PVMountPointManager&&) = default;
  PVMountPointManager operator=(PVMountPointManager&&) = default;

  bool checkPVHeader(pv_layout_headers::PVHeader &header) const {
    // TODO : check signature, size and version
  }
};

} // namespace storage
} // namespace yas
