#pragma once
#include "../external/expected.hpp"
#include <cstdint>
#include <vector>

namespace yas {
namespace devices {

class IDevice {
 public:
  enum class DeviceErrorCode : uint8_t {
    DeviceReadError = 0,
    DevicePositionMismatch = 1,
    DeviceLengthMismatch = 2,
    DeviceWriteError = 3,
    DeviceOpenError = 4,
    Last
  };

  virtual nonstd::expected<std::vector<uint8_t>, DeviceErrorCode> Read(uint64_t position, uint64_t data_size) = 0;
  virtual nonstd::expected<uint64_t, DeviceErrorCode> Write(uint64_t position, std::vector<uint8_t> &data) = 0;
  virtual bool IsOpen() const = 0;
  virtual bool Close() = 0;         // Close is public method because of in some cases (f.e. in std::fstream case) 
                                    // it needs to check is this device has been really closed to have a possibility
                                    // to do some magic otherwise. Signature with bool is general and implied that
                                    // in general case exceptions could be used to return details about failture 
  virtual ~IDevice() {}

 protected:
  virtual void Open() = 0;
};

} // namespace devices
} // namespace yas
