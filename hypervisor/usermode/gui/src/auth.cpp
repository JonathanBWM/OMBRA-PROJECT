#include "auth.h"
#include <Windows.h>
#include <intrin.h>
#include <iphlpapi.h>
#include <sstream>
#include <iomanip>
#include <chrono>

#pragma comment(lib, "iphlpapi.lib")

namespace Ombra {

bool AuthManager::s_validated = false;
int64_t AuthManager::s_sessionExpiry = 0;

HardwareFingerprint AuthManager::CollectFingerprint() {
    HardwareFingerprint fp;

    // Get volume serial number
    char volumeName[MAX_PATH + 1] = { 0 };
    char fileSystemName[MAX_PATH + 1] = { 0 };
    DWORD serialNumber = 0;
    DWORD maxComponentLen = 0;
    DWORD fileSystemFlags = 0;

    if (GetVolumeInformationA(
        "C:\\",
        volumeName,
        sizeof(volumeName),
        &serialNumber,
        &maxComponentLen,
        &fileSystemFlags,
        fileSystemName,
        sizeof(fileSystemName))) {

        std::ostringstream oss;
        oss << std::hex << std::setw(8) << std::setfill('0') << serialNumber;
        fp.volumeSerial = oss.str();
    }

    // Get MAC address
    ULONG bufferSize = 15000;
    IP_ADAPTER_INFO* adapterInfo = (IP_ADAPTER_INFO*)malloc(bufferSize);

    if (adapterInfo) {
        if (GetAdaptersInfo(adapterInfo, &bufferSize) == ERROR_SUCCESS) {
            IP_ADAPTER_INFO* adapter = adapterInfo;
            if (adapter) {
                std::ostringstream oss;
                for (UINT i = 0; i < adapter->AddressLength; i++) {
                    if (i > 0) oss << ":";
                    oss << std::hex << std::setw(2) << std::setfill('0')
                        << (int)adapter->Address[i];
                }
                fp.macAddress = oss.str();
            }
        }
        free(adapterInfo);
    }

    // Get CPUID
    int cpuInfo[4] = { 0 };
    __cpuid(cpuInfo, 1);

    std::ostringstream oss;
    oss << std::hex << std::setw(8) << std::setfill('0') << cpuInfo[0]
        << std::setw(8) << std::setfill('0') << cpuInfo[3];
    fp.cpuId = oss.str();

    // Combine all fingerprints
    fp.combined = fp.volumeSerial + "_" + fp.macAddress + "_" + fp.cpuId;

    return fp;
}

AuthResult AuthManager::Validate(const HardwareFingerprint& fingerprint) {
    // STUB: Development build - always authorize
    AuthResult result;
    result.authorized = true;
    result.licenseType = LicenseType::Lifetime;
    result.username = "Developer";
    result.message = "Development build - auth bypassed";
    result.expiresAt = 0; // Never expires
    result.daysRemaining = -1; // Lifetime

    // Set session as validated
    s_validated = true;

    // Session never expires for dev builds
    s_sessionExpiry = LLONG_MAX;

    return result;
}

bool AuthManager::IsSessionValid() {
    if (!s_validated) {
        return false;
    }

    auto now = std::chrono::system_clock::now();
    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();

    return nowMs < s_sessionExpiry;
}

const char* AuthManager::LicenseTypeString(LicenseType type) {
    switch (type) {
        case LicenseType::None:     return "None";
        case LicenseType::Trial:    return "Trial";
        case LicenseType::Monthly:  return "Monthly";
        case LicenseType::Lifetime: return "Lifetime";
        default:                    return "Unknown";
    }
}

} // namespace Ombra
