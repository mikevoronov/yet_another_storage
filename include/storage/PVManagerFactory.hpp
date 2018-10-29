#pragma once
#include "PVManager.hpp"
#include <mutex>
#include <memory>
#include <string>
#include <unordered_map>

namespace yas {
namespace storage {

/**
*    \brief This class is used for creating PVManager on specified path.
*
*    The general rule of YAS is one PVManager for one PV file because of thread-safety. So this class creates new
*    or loads an existing PV from the device and keeps shared_ptr on it. If a user specifies the path for already
*    created PVManager - class returns the already created one.
*/
class PVManagerFactory {
 public:
  using manager_type = PVManager<DCharType, DOffsetType, DDevice>;
  using shared_manager_type = std::shared_ptr<manager_type>;
  using pv_path_type = typename manager_type::pv_path_type;

  static PVManagerFactory& Instance() {
    static PVManagerFactory factory(kMaximumSupportedVersion);
    return factory;
  }

  ///  \brief creates new or returns existing PVManager
  ///
  ///  \param path - the path to PVManager
  ///  \param requested_version - the maximum supported version of PV structure
  ///  \param priority - a priority for newly created PVManager
  ///  \param cluster_size - a cluster size for newly created PVManager
  ///  \return - the PVManager for specified path or error
  nonstd::expected<shared_manager_type, StorageErrorDescriptor> Create(const pv_path_type pv_path,
      utils::Version requested_version, int32_t priority = 0, int32_t cluster_size = kDefaultClusterSize) noexcept {

    if (max_supported_version_ < requested_version) {
      return nonstd::make_unexpected(StorageErrorDescriptor{ "requested PV version is unsupported",
          StorageError::kPVVersionUnsupported });
    }

    try {
      std::lock_guard<std::mutex> lock(factory_mutex_);

      if(DDevice::Exists(pv_path)) {
        const auto canonical_path = DDevice::Canonical(pv_path);
        const auto canonical_path_str = canonical_path.wstring();
        if (std::cend(managers_) != managers_.find(canonical_path_str)) {
          return managers_[canonical_path_str];
        }

        auto loaded_manager = manager_type::Load(std::move(canonical_path), max_supported_version_);
        managers_[canonical_path_str] = std::move(loaded_manager);
        return managers_[canonical_path_str];
      }

      auto new_manager = manager_type::Create(pv_path, requested_version, priority, cluster_size);
      const auto canonical_path = DDevice::Canonical(pv_path);
      const auto canonical_path_str = canonical_path.wstring();

      managers_[canonical_path_str] = std::move(new_manager);
      return managers_[canonical_path_str];
    }
    catch (...) {
      return nonstd::make_unexpected(exception::ExceptionHandler::Handle(std::current_exception()));
    }
  }

  ///  \brief returns existing PVManager
  ///
  ///  \param path - the path to PVManager
  ///  \return - the already created PVManager for specified path
  nonstd::expected<shared_manager_type, StorageErrorDescriptor> GetPVManager(const pv_path_type path) noexcept {
    try {
      if (!DDevice::Exists(path)) {
        return nonstd::make_unexpected(StorageErrorDescriptor{ "requested PV path isn't found",
            StorageError::kPathNotFound });
      }

      const auto canonical_path_str = DDevice::Canonical(path).wstring();
      std::lock_guard<std::mutex> lock(factory_mutex_);
      if (std::cend(managers_) == managers_.find(canonical_path_str)) {
        return nonstd::make_unexpected(StorageErrorDescriptor{ "requested PV on the given path isn't found",
            StorageError::kPathNotFound });
      }

      return managers_[canonical_path_str];
    }
    catch (...) {
      return nonstd::make_unexpected(exception::ExceptionHandler::Handle(std::current_exception()));
    }
  }

  ~PVManagerFactory() = default;

  PVManagerFactory(const PVManagerFactory&) = delete;
  PVManagerFactory(PVManagerFactory&&) = delete;
  PVManagerFactory& operator=(const PVManagerFactory&) = delete;
  PVManagerFactory& operator=(PVManagerFactory&&) = delete;

 private:
  std::unordered_map<std::wstring, shared_manager_type> managers_;
  std::mutex factory_mutex_;
  utils::Version max_supported_version_;

  PVManagerFactory(utils::Version max_supported_version) 
      : max_supported_version_(max_supported_version)
  {}
};

} // namespace storage
} // namespace yas
