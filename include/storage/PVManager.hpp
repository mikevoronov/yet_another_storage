#pragma once
#include "lib/physical_volume/PVDeviceDataReaderWriter.hpp"
#include "lib/physical_volume/PVEntriesManager.hpp"
#include "lib/inverted_index/InvertedIndexHelper.hpp"
#include "lib/exceptions/ExceptionHandler.hpp"
#include "IStorage.hpp"
#include <mutex>


namespace yas {
namespace storage {

/**
 *    \brief The class encapsulates all high-level storage-like operations on one Physical Volume (PV)
 *
 *    Objects of this class could be created by Load/Create static methods. Note that for each physically
 *    seprated PV should be created only one instance of this class. If you want to simultaneously work
 *    with several PV please create instances through PVManagerFactory.
 */
template <typename CharType=DCharType, typename OffsetType=DOffsetType, typename Device=DDevice>
class PVManager : public IStorage<CharType> {
  using InvertedIndexType = index_helper::InvertedIndexHelper<CharType, OffsetType>;
  using PVEntriesManagerType = pv::PVEntriesManager<OffsetType, Device>;

 public:
  using pv_manager_type = PVManager<CharType, OffsetType, Device>;
  using key_type = typename IStorage<CharType>::key_type;
  using pv_path_type = typename Device::path_type;

  virtual ~PVManager() {
    close();
  }

  ///  \brief loads the already created PV from specifed path. Can throw YASExceptions if PV has invalid structure
  ///         or device fails.
  ///  \param pv_path - path to exist PV
  ///  \param version - maximum supported version (PVEntriesManager could use it for parsing)
  ///  \return - new PVManager instance
  static std::unique_ptr<pv_manager_type> Load(pv_path_type pv_path, utils::Version version) {
    auto pv_volume_manager = std::unique_ptr<pv_manager_type>(new pv_manager_type(pv_path, version));

    pv_volume_manager->inverted_index_offset_ = pv_volume_manager->entries_manager_.LoadStartEntries();
    const auto serialized_index = pv_volume_manager->entries_manager_.GetEntryContent(
        pv_volume_manager->inverted_index_offset_);
    const auto &vector_serialized_index = std::get<ByteVector>(serialized_index);
    auto indexer = InvertedIndexType::template Deserialize<typename InvertedIndexType::leaf_type>(
        std::cbegin(vector_serialized_index),
        std::cend(vector_serialized_index), 
        version);

    pv_volume_manager->inverted_index_ = std::move(indexer);
    return pv_volume_manager;
  }

  ///  \brief creates new PV in the specified path. Note that if the PV already exist (it means that Device::Exist
  ///         returns success) then it tries to load existing by Load method. Can throw YASExceptions if device fails.
  ///  \param pv_path - path to newly created PV
  ///  \param version - maximum supported version (PVEntriesManager could use it for parsing)
  ///  \return - new PVManager instance
  static std::unique_ptr<pv_manager_type> Create(pv_path_type pv_path, utils::Version version,
      int32_t priority, int32_t cluster_size = kDefaultClusterSize) {
    if (Device::Exists(pv_path)) {
      return Load(pv_path, version);
    }

    Device::CreateEmpty(pv_path);

    // std::make_unique needs access to the class ctor
    auto pv_volume_manager = std::unique_ptr<pv_manager_type>(new pv_manager_type(pv_path, version, priority, 
        cluster_size));
    pv_volume_manager->entries_manager_.SaveStartEntries(offset_traits<OffsetType>::NonExistValue());
    pv_volume_manager->inverted_index_.reset(new InvertedIndexType());
    pv_volume_manager->inverted_index_offset_ = offset_traits<OffsetType>::NonExistValue();
    return pv_volume_manager;
  }

  StorageErrorDescriptor Put(key_type key, const storage_value_type &value) noexcept override {
    std::lock_guard<std::mutex> lock(manager_guard_mutex_);
    try {
      if (value.valueless_by_exception()) {
        return { "Put key: value is valueless", StorageError::kIncorrectStorageValue };
      }
      if (inverted_index_->HasKey(key)) {
        return { "Put key: the storage already has current key, please remove it first", 
            StorageError::kKeyAlreadyCreated };
      }

      const auto offset = entries_manager_.CreateNewEntryValue(value);
      inverted_index_->Insert(key, offset);
      return { std::string(), StorageError::kSuccess };
    }
    catch (...) {
      return exception::ExceptionHandler::Handle(std::current_exception());
    }
  }

  nonstd::expected<storage_value_type, StorageErrorDescriptor> Get(key_type key) noexcept override {
    std::lock_guard<std::mutex> lock(manager_guard_mutex_);
    try {
      const auto entry_offset = inverted_index_->Get(key);
      if (!index_helper::leaf_type_traits<OffsetType>::IsExistValue(entry_offset)) {
        return nonstd::make_unexpected(StorageErrorDescriptor{ "Get key: key hasn't been found",
            StorageError::kKeyNotFound });
      }

      if (!isEntryExpired(entry_offset)) {
        return entries_manager_.GetEntryContent(entry_offset);
      }

      // delete expired values during access
      entries_manager_.DeleteEntry(entry_offset);
      inverted_index_->Delete(key);
      return nonstd::make_unexpected(StorageErrorDescriptor{ "Get key: key hasn't been found",
          StorageError::kKeyNotFound });
    }
    catch (...) {
      return nonstd::make_unexpected(exception::ExceptionHandler::Handle(std::current_exception()));
    }
  }

  StorageErrorDescriptor HasKey(key_type key) noexcept override {
    std::lock_guard<std::mutex> lock(manager_guard_mutex_);
    try {
      const auto entry_offset = inverted_index_->Get(key);
      if (!index_helper::leaf_type_traits<OffsetType>::IsExistValue(entry_offset)) {
        return { std::string(), StorageError::kKeyNotFound };
      }

      if (!isEntryExpired(entry_offset)) {
        return { std::string(), StorageError::kSuccess };
      }

      // delete expired values during access
      entries_manager_.DeleteEntry(entry_offset);
      inverted_index_->Delete(key);
      return { std::string(), StorageError::kKeyNotFound};
    }
    catch (...) {
      return exception::ExceptionHandler::Handle(std::current_exception());
    }
  }

  StorageErrorDescriptor HasCatalog(key_type key) noexcept override {
    std::lock_guard<std::mutex> lock(manager_guard_mutex_);
    try {
      return 0 == inverted_index_->FindMaxSubKey(key) ?
          StorageErrorDescriptor{ std::string(), StorageError::kKeyNotFound } :
          StorageErrorDescriptor{ std::string(), StorageError::kSuccess };
    }
    catch (...) {
      return exception::ExceptionHandler::Handle(std::current_exception());
    }
  }

   StorageErrorDescriptor Delete(key_type key) noexcept override {
    std::lock_guard<std::mutex> lock(manager_guard_mutex_);
    try {
      const auto entry_offset = inverted_index_->Get(key);
      if (!index_helper::leaf_type_traits<OffsetType>::IsExistValue(entry_offset)) {
        return { "Delete key: the key hasn't been found", StorageError::kKeyNotFound };
      }
      entries_manager_.DeleteEntry(entry_offset);
      inverted_index_->Delete(key);
      return { std::string(), StorageError::kSuccess };
    }
    catch (...) {
      return exception::ExceptionHandler::Handle(std::current_exception());
    }
  }

  StorageErrorDescriptor SetExpiredDate(key_type key, time_t expired) noexcept override {
    std::lock_guard<std::mutex> lock(manager_guard_mutex_);
    try {
      const auto entry_offset = inverted_index_->Get(key);
      if (!index_helper::leaf_type_traits<OffsetType>::IsExistValue(entry_offset)) {
        return { "SetExpiredDate key: key hasn't been found", StorageError::kKeyNotFound };
      }
      utils::Time expired_time(expired);
      entries_manager_.SetEntryExpiredDate(entry_offset, expired_time);
      return { std::string(), StorageError::kSuccess};
    }
    catch (...) {
      return exception::ExceptionHandler::Handle(std::current_exception());
    }
  }

  nonstd::expected<time_t, StorageErrorDescriptor> GetExpiredDate(key_type key) noexcept override {
    std::lock_guard<std::mutex> lock(manager_guard_mutex_);
    try {
      const auto entry_offset = inverted_index_->Get(key);
      if (!index_helper::leaf_type_traits<OffsetType>::IsExistValue(entry_offset)) {
        return nonstd::make_unexpected(StorageErrorDescriptor{ "GetExpiredDate key: key hasn't been found",
            StorageError::kKeyNotFound });
      }
      utils::Time expired_date(0, 0);
      if (!entries_manager_.GetEntryExpiredDate(entry_offset, expired_date)) {
        return nonstd::make_unexpected(StorageErrorDescriptor{ "GetExpiredDate key: the key doesn't has expired date",
            StorageError::kKeyDoesntExpired });
      }
      return expired_date.GetTime();
    }
    catch (...) {
      return nonstd::make_unexpected(exception::ExceptionHandler::Handle(std::current_exception()));
    }
  }

  int32_t priority() const { return entries_manager_.priority(); }

#ifdef UNIT_TEST
  PVEntriesManagerType& entries_manager() const { return entries_manager_; }
#endif

  PVManager(const PVManager&) = delete;
  PVManager(PVManager&&) = delete;
  PVManager& operator=(const PVManager&) = delete;
  PVManager& operator=(PVManager&&) = delete;

 private:
  std::unique_ptr<InvertedIndexType> inverted_index_;
  OffsetType inverted_index_offset_;
  PVEntriesManagerType entries_manager_;
  std::mutex manager_guard_mutex_;
  utils::Version version_;

  explicit PVManager(const fs::path &file_path, utils::Version version, uint32_t priority = 0, 
      uint32_t cluster_size = kDefaultClusterSize)
      : entries_manager_(file_path, version, priority, cluster_size),
        version_(version) {
  }

  void close() {
    if (!inverted_index_) {
      return;
    }
    try {
      if (inverted_index_->is_changed()) {
        const auto serialized_index = inverted_index_->template Serialize<OffsetType>(version_);
        if (offset_traits<OffsetType>::IsExistValue(inverted_index_offset_)) {
          entries_manager_.DeleteEntry(inverted_index_offset_);
        }
        inverted_index_offset_ = entries_manager_.CreateNewEntryValue(serialized_index);
      }
      entries_manager_.SaveStartEntries(inverted_index_offset_);
    }
    catch (...) {
      // in future relize there should be some code to save trie while OOM (it could be throwed by Serialize method)
    }
  }

  bool isEntryExpired(OffsetType offset) {
    utils::Time expired_date;
    if (!entries_manager_.GetEntryExpiredDate(offset, expired_date)) {
      return false;
    }

    return expired_date.IsExpired();
  }
};

} // namespace storage
} // namespace yas
