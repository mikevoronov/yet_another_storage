#pragma once
#include "PVManager.hpp"
#include <mutex>
#include <memory>
#include <string>
#include <unordered_map>

namespace yas {
namespace storage {

/**
*    \brief The class is used for creating PVManager on specified path.
*
*    The general rule of YAS is one PVManager for one PV file because of thread-safety. So this class creates new
*    or loads an existing PV from device and keep shared_ptr on them in internal unordered_map. If user specifies 
*    path for already created PVManager - class returns him already created one.
*/
class PVManagerFactory {
 public:
  using manager_type = PVManager<CharType, OffsetType, DefaultDevice<OffsetType>>;
  using pv_path_type = typename manager_type::pv_path_type;

  static PVManagerFactory& Instance() {
    static PVManagerFactory factory(kMaximumSupportedVersion);
    return factory;
  }

  ///  \brief creates new or returns existing PVManager
  ///
  ///  \param path - path to PVManager
  ///  \param requested_version - maximum supported version of PV structure
  ///  \param priority - a priority for newly created PVManager
  ///  \param cluster_size - a cluster size for newly created PVManager
  ///  \return - a PVManager for specified path or error
  nonstd::expected<std::shared_ptr<manager_type>, StorageErrorDescriptor> Create(const pv_path_type path,
      utils::Version requested_version, uint32_t priority = 0, uint32_t cluster_size = kDefaultClusterSize) {

    if (max_supported_version_ < requested_version) {
      return nonstd::make_unexpected(StorageErrorDescriptor("requested PV version is unsupported", 
          StorageError::kPVVersionUnsupported));
    }
    
    auto canonical_path = fs::canonical(path);
    auto canonical_path_str = std::wstring(canonical_path);
    std::lock_guard<std::mutex> lock(factory_mutex_);

    if (managers_.count(canonical_path_str)) {
      return managers_[canonical_path_str];
    }

    try {
      if (fs::exists(canonical_path)) {
        auto loaded_manager = manager_type::Load(canonical_path, max_supported_version_);
        managers_[canonical_path_str] = std::move(loaded_manager);
        return managers_[canonical_path_str];
      }

      auto new_manager = manager_type::Create(canonical_path, requested_version, priority, cluster_size);
      managers_[canonical_path_str] = std::move(new_manager);
      return managers_[canonical_path_str];
    }
    catch (...) {
      return nonstd::make_unexpected(exception::ExceptionHandler::Handle(std::current_exception()));
    }
  }

  ///  \brief returns existing PVManager
  ///
  ///  \param path - path to PVManager
  ///  \return - already created PVManager for specified path or nullptr if it hasn't been created yet
  std::shared_ptr<manager_type> GetPVManager(const pv_path_type path) {
    auto canonical_path = std::wstring(fs::canonical(path));

    std::lock_guard<std::mutex> lock(factory_mutex_);
    return managers_.count(canonical_path) ? managers_[canonical_path] : nullptr;
  }

  ~PVManagerFactory() = default;
  PVManagerFactory(const PVManagerFactory&) = delete;
  PVManagerFactory(PVManagerFactory&&) = delete;
  PVManagerFactory operator=(const PVManagerFactory&) = delete;

 private:
  std::unordered_map<std::wstring, std::shared_ptr<manager_type>> managers_;
  std::mutex factory_mutex_;
  utils::Version max_supported_version_;

  PVManagerFactory(utils::Version max_supported_version) 
      : max_supported_version_(max_supported_version)
  {}
};

} // namespace storage
} // namespace yas
