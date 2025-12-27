#pragma once

#include <string>
#include <cstdint>

namespace Ombra {

enum class LicenseType {
    None,
    Trial,
    Monthly,
    Lifetime
};

struct AuthResult {
    bool authorized;
    LicenseType licenseType;
    std::string username;
    std::string message;
    int64_t expiresAt;
    int daysRemaining;

    AuthResult()
        : authorized(false)
        , licenseType(LicenseType::None)
        , expiresAt(0)
        , daysRemaining(0)
    {}
};

struct HardwareFingerprint {
    std::string diskSerial;
    std::string macAddress;
    std::string volumeSerial;
    std::string cpuId;
    std::string combined;

    HardwareFingerprint()
        : diskSerial("")
        , macAddress("")
        , volumeSerial("")
        , cpuId("")
        , combined("")
    {}
};

class AuthManager {
public:
    static HardwareFingerprint CollectFingerprint();
    static AuthResult Validate(const HardwareFingerprint& fingerprint);
    static bool IsSessionValid();
    static const char* LicenseTypeString(LicenseType type);

private:
    static bool s_validated;
    static int64_t s_sessionExpiry;
};

} // namespace Ombra
