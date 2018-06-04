#pragma once
#include "PVDeviceDataReaderWriter.hpp"

namespace yas {
namespace pv {

template <typename OffsetType>
class PVEntriesAllocator {
 public:
  explicit PVEntriesAllocator(int32_t cluster_size)
      : cluster_size_(cluster_size) {
    simultaneously_allocated_clusters_ = std::max<int32_t>(maximum_simultaneously_extend_pv_size_/ cluster_size_, 1);
    last_allocated_clusters_count_ = simultaneously_allocated_clusters_;
  }

  ~PVEntriesAllocator() = default;

  template<typename PVDeviceDataReaderWriterType>
  OffsetType ExpandPV(PVDeviceDataReaderWriterType &data_reader_writer, OffsetType free_entry_offset) {
    last_allocated_clusters_count_ *= extend_factor_;

    const int64_t allocate_clusters_count = static_cast<int64_t>(last_allocated_clusters_count_);
    ByteVector new_clusters(simultaneously_allocated_clusters_* cluster_size_, debug_filler);

    ComplexTypeHeader header;
    header.overall_size_ = cluster_size_ - offsetof(ComplexTypeHeader, data_);
    header.chunk_size_ = cluster_size_ - offsetof(ComplexTypeHeader, data_);
    header.value_type_ = PVType::kEmptyComplex;

    for (int64_t written_clusters = 0; written_clusters < allocate_clusters_count; 
        written_clusters += simultaneously_allocated_clusters_) {
      auto current_cursor = std::begin(new_clusters);
      const auto data_end = std::end(new_clusters);
      for (int32_t new_cluster_id = 0; new_cluster_id < simultaneously_allocated_clusters_; ++new_cluster_id) {
        header.next_free_entry_offset_ = free_entry_offset;
        serialization_utils::SaveAsBytes(current_cursor, std::end(new_clusters), &header);
        std::advance(current_cursor, cluster_size_);
        free_entry_offset = offset_traits<OffsetType>::IsExistValue(free_entry_offset) ? 
            free_entry_offset + cluster_size_ : device_end_;
      }
      device_end_ += data_reader_writer.RawWrite(device_end_, std::cbegin(new_clusters), std::cend(new_clusters));
    }

    return free_entry_offset - cluster_size_;
  }

  void device_end(OffsetType device_end) { device_end_ = device_end;}
  OffsetType device_end() const { return device_end_; }

 private:
  OffsetType device_end_;
  int32_t cluster_size_;
  int32_t simultaneously_allocated_clusters_;
  double last_allocated_clusters_count_;

  // TODO : for better heterogeneous devices support maximum_simultaneously_extend_pv_size_ should be
  // taken from some static Device method
  const int32_t maximum_simultaneously_extend_pv_size_ = 5 * 0x1000;
  const ByteVector::value_type debug_filler = 0xAA;
  const double extend_factor_ = 1.1;
};

} // namespace pv
} // namespace yas
