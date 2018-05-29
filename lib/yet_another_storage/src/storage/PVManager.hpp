#pragma once
#include "../physical_volume/PVDeviceDataReaderWriter.hpp"
#include "../physical_volume/PVEntriesManager.hpp"
#include "../inverted_index_helper/InvertedIndexHelper.hpp"
#include "../storage/IStorage.hpp"
#include "../exceptions/ExceptionHandler.hpp"
#include <mutex>

using namespace yas::pv_layout_headers;
using namespace yas::index_helper;

namespace yas {
namespace storage {

template<typename OffsetType>
using DefaultDevice = devices::FileDevice<OffsetType>;

/** 
 *    \brief Class encapsulate all high-level storage-like operations on one Physical Volume (PV)
 *
 *    Objects of this class could be created by Load/Create static methods. Note that for each physically
 *    seprated PV should be created only one instance of this class. If you want to simultaneously work
 *    with several PV please create instances through PVManagerFactory
 */
template <typename CharType=CharType, typename OffsetType=OffsetType, typename Device=DefaultDevice<OffsetType>>
class PVManager : public IStorage<CharType> {
  using InvertedIndexType = InvertedIndexHelper<CharType, OffsetType>;
  using PVEntriesManagerType = pv::PVEntriesManager<OffsetType, Device>;

 public:
  using pv_manager_type = PVManager<CharType, OffsetType, Device>;
  using key_type = IStorage<CharType>::key_type;
  using pv_path_type = typename Device::path_type;

  virtual ~PVManager() {
    close();
  }

  ///  \brief loads an already created PV from specifed path. Can throw YASExceptions if PV has invalid structure
  ///         or device fails
  ///  \param pv_path - path to exist PV
  ///  \param version - maximum supported version (PVEntriesManager could use it for parsing)
  ///  \return - new PVManager instance
  static std::unique_ptr<pv_manager_type> Load(const pv_path_type &pv_path, utils::Version version) {
    auto pv_volume_manager = std::unique_ptr<pv_manager_type>(new pv_manager_type(pv_path, version));

    pv_volume_manager->inverted_index_offset_ = pv_volume_manager->entries_manager_.LoadStartSections();
    const auto serialized_index = pv_volume_manager->entries_manager_.GetEntryContent(
        pv_volume_manager->inverted_index_offset_);
    const auto &vector_serialized_index = std::any_cast<const ByteVector &>(serialized_index);
    auto indexer = InvertedIndexType::Deserialize<InvertedIndexType::leaf_type>(std::cbegin(vector_serialized_index),
        std::cend(vector_serialized_index), version);

    pv_volume_manager->inverted_index_ = std::move(indexer);
    return pv_volume_manager;
  }

  ///  \brief create new PV in the specified path. Note that if PV already exist (it means that Device::Exist returns
  ///         success) then it try to Load existing by Load method. Can throw YASExceptions if device fails. 
  ///  \param pv_path - path to newly created PV
  ///  \param version - maximum supported version (PVEntriesManager could use it for parsing)
  ///  \return - new PVManager instance
  static std::unique_ptr<pv_manager_type> Create(const pv_path_type &pv_path, utils::Version version,
      uint32_t priority, uint32_t cluster_size = kDefaultClusterSize) {
    if (Device::Exist(pv_path)) {
      return Load(pv_path, version);
    }

    Device::CreateEmpty(pv_path);

    // std::make_unique needs access to the class ctor
    auto pv_volume_manager = std::unique_ptr<pv_manager_type>(new pv_manager_type(pv_path, version, cluster_size));
    pv_volume_manager->entries_manager_.CreateStartSections(offset_traits<OffsetType>::NonExistValue());
    pv_volume_manager->inverted_index_.reset(new InvertedIndexType());
    return pv_volume_manager;
  }

  virtual StorageErrorDescriptor Put(key_type key, std::any value) override {
    std::lock_guard<std::mutex> lock(manager_guard_mutex_);
    try {
      if (!value.has_value()) {
        return { "Put key: std::any doesn't contain any value", StorageError::kValueNotFound };
      }
      if (inverted_index_->HasKey(key)) {
        return { "Put key: the storage already has current key, please remove it first", 
            StorageError::kKeyAlreadyCreated };
      }

      const auto offset = entries_manager_.CreateNewEntryValue(value);
      inverted_index_->Insert(key, offset);
      return { "", StorageError::kSuccess };
    }
    catch (...) {
      return exception::ExceptionHandler::Handle(std::current_exception());
    }
  }

  virtual nonstd::expected<std::any, StorageErrorDescriptor> Get(key_type key) override {
    std::lock_guard<std::mutex> lock(manager_guard_mutex_);
    try {
      const auto entry_offset = inverted_index_->Get(key);
      if (!leaf_traits<OffsetType>::IsExistValue(entry_offset)) {
        return nonstd::make_unexpected(StorageErrorDescriptor("Get key: key hasn't been found", 
            StorageError::kKeyNotFound));
      }

      if (!isEntryExpired(entry_offset)) {
        return entries_manager_.GetEntryContent(entry_offset);
      }

      // delete expired values during access
      entries_manager_.DeleteEntry(entry_offset);
      inverted_index_->Delete(key);
      return nonstd::make_unexpected(StorageErrorDescriptor("Get key: key hasn't been found", 
          StorageError::kKeyNotFound));
    }
    catch (...) {
      return nonstd::make_unexpected(exception::ExceptionHandler::Handle(std::current_exception()));
    }
  }

  virtual StorageErrorDescriptor HasKey(key_type key) override {
    std::lock_guard<std::mutex> lock(manager_guard_mutex_);
    try {
      const auto entry_offset = inverted_index_->Get(key);
      if (!leaf_traits<OffsetType>::IsExistValue(entry_offset)) {
        return { "", StorageError::kKeyNotFound };
      }

      if (!isEntryExpired(entry_offset)) {
        return { "", StorageError::kSuccess };
      }

      // delete expired values during access
      entries_manager_.DeleteEntry(entry_offset);
      inverted_index_->Delete(key);
      return { "", StorageError::kKeyNotFound};
    }
    catch (...) {
      return exception::ExceptionHandler::Handle(std::current_exception());
    }
  }
  
  StorageErrorDescriptor HasCatalog(key_type key) override {
    std::lock_guard<std::mutex> lock(manager_guard_mutex_);
    try {
      return inverted_index_->HasPath(key) ? 
          StorageErrorDescriptor( "", StorageError::kSuccess ) :
          StorageErrorDescriptor( "", StorageError::kKeyNotFound );
    }
    catch (...) {
      return exception::ExceptionHandler::Handle(std::current_exception());
    }
  }
    
  virtual StorageErrorDescriptor Delete(key_type key) override {
    std::lock_guard<std::mutex> lock(manager_guard_mutex_);
    try {
      const auto entry_offset = inverted_index_->Get(key);
      if (!leaf_traits<OffsetType>::IsExistValue(entry_offset)) {
        return { "Delete key: key hasn't been found", StorageError::kKeyNotFound };
      }
      entries_manager_.DeleteEntry(entry_offset);
      inverted_index_->Delete(key);
      return { "", StorageError::kSuccess };
    }
    catch (...) {
      return exception::ExceptionHandler::Handle(std::current_exception());
    }
  }

  virtual StorageErrorDescriptor SetExpiredDate(key_type key, time_t expired) override {
    std::lock_guard<std::mutex> lock(manager_guard_mutex_);
    try {
      const auto entry_offset = inverted_index_->Get(key);
      if (!leaf_traits<OffsetType>::IsExistValue(entry_offset)) {
        return { "SetExpiredDate key: key hasn't been found", StorageError::kKeyNotFound };
      }
      utils::Time expired_time(expired);
      entries_manager_.SetEntryExpiredDate(entry_offset, expired_time);
      return {"", StorageError::kSuccess};
    }
    catch (...) {
      return exception::ExceptionHandler::Handle(std::current_exception());
    }
  }

  virtual nonstd::expected<time_t, StorageErrorDescriptor> GetExpiredDate(key_type key) override {
    std::lock_guard<std::mutex> lock(manager_guard_mutex_);
    try {
      const auto entry_offset = inverted_index_->Get(key);
      if (!leaf_traits<OffsetType>::IsExistValue(entry_offset)) {
        return nonstd::make_unexpected(StorageErrorDescriptor("GetExpiredDate key: key hasn't been found", 
            StorageError::kKeyNotFound));
      }
      utils::Time expired_date(0,1);
      if (!entries_manager_.GetEntryExpiredDate(entry_offset, expired_date)) {
        return nonstd::make_unexpected(StorageErrorDescriptor("GetExpiredDate key: the key doesn't has expired date",
            StorageError::kKeyDoesntExpired));
      }
      return expired_date.GetTime();
    }
    catch (...) {
      return nonstd::make_unexpected(exception::ExceptionHandler::Handle(std::current_exception()));
    }
  }

  uint32_t priority() const { return entries_manager_.priority(); }

#ifdef UNIT_TEST
  Device& GetDevice() const { return device_; }
#endif

  PVManager(const PVManager&) = delete;
  PVManager(PVManager&&) = default;
  PVManager& operator=(const PVManager&) = delete;
  PVManager& operator=(PVManager&&) = delete;

 private:
  std::unique_ptr<InvertedIndexType> inverted_index_;
  OffsetType inverted_index_offset_;
  PVEntriesManagerType entries_manager_;
  std::mutex manager_guard_mutex_;
  utils::Version version_;

  explicit PVManager(const fs::path &file_path, utils::Version version, uint32_t cluster_size = kDefaultClusterSize)
      : entries_manager_(file_path, version, cluster_size),
        version_(version) {
  }

  void close() {
    // TODO : exception during serailization 
    const auto serialized_index = inverted_index_->Serialize<OffsetType>(version_);
    if (leaf_traits<OffsetType>::IsExistValue(inverted_index_offset_)) {
      entries_manager_.DeleteEntry(inverted_index_offset_);
    }
    const auto new_index_offset = entries_manager_.CreateNewEntryValue(serialized_index);
    entries_manager_.CreateStartSections(new_index_offset);
  }

  bool isEntryExpired(OffsetType offset) {
    utils::Time expired_date(0, 0);
    if (!entries_manager_.GetEntryExpiredDate(offset, expired_date)) {
      return false;
    }

    return expired_date.IsExpired();
  }
};

} // namespace storage
} // namespace yas
