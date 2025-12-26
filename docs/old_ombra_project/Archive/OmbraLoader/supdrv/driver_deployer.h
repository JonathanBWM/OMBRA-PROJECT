/**
 * @file driver_deployer.h
 * @brief Portable driver deployment via SCM or NtLoadDriver
 *
 * Handles extraction, deployment, and cleanup of embedded signed drivers.
 * Supports two loading methods:
 * - SCM: Service Control Manager (CreateService/StartService)
 * - NtLoadDriver: Direct registry + ntdll!NtLoadDriver
 */

#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <cstdint>

namespace supdrv {

/**
 * Deployment method selection
 */
enum class DeployMethod {
    SCM,           // Service Control Manager (CreateService/StartService)
    NtLoadDriver,  // Direct registry manipulation + NtLoadDriver
    Auto           // Try SCM first, fallback to NtLoadDriver
};

/**
 * Resource identifiers for embedded drivers
 */
struct DriverResource {
    const wchar_t* wszResourceName;   // Resource name (e.g., L"LD9BOXSUP_ENCRYPTED")
    const wchar_t* wszResourceType;   // Resource type (e.g., RT_RCDATA or custom)
    const wchar_t* wszDeviceName;     // Expected device name after loading
    const wchar_t* wszServiceName;    // Service name to use
    uint32_t dwEncryptionKey;         // Encryption key (0 = use header key)
};

/**
 * DriverDeployer - Portable driver loading and cleanup
 *
 * Usage:
 *   DriverDeployer deployer;
 *   if (!deployer.Deploy(DeployMethod::Auto)) { handle error }
 *   // Use deployer.GetDeviceHandle() for I/O
 *   // ... application runs ...
 *   deployer.Cleanup(true);  // Remove all traces
 */
class DriverDeployer {
public:
    DriverDeployer();
    ~DriverDeployer();

    // Non-copyable
    DriverDeployer(const DriverDeployer&) = delete;
    DriverDeployer& operator=(const DriverDeployer&) = delete;

    /**
     * Deploy driver from embedded resources
     *
     * @param method Deployment method (SCM, NtLoadDriver, or Auto)
     * @param pResource Optional custom resource info (nullptr = use default)
     * @return true if driver loaded and device opened successfully
     */
    bool Deploy(DeployMethod method = DeployMethod::Auto,
                const DriverResource* pResource = nullptr);

    /**
     * Check if a SUPDrv-compatible driver is already loaded
     * Useful for detecting existing LDPlayer/VBox installations
     *
     * @return true if device can be opened
     */
    static bool IsDriverLoaded();

    /**
     * Check if driver service exists in SCM
     *
     * @param wszServiceName Service name to check
     * @return true if service is registered
     */
    static bool IsServiceRegistered(const wchar_t* wszServiceName);

    /**
     * Get service status
     *
     * @param wszServiceName Service name to query
     * @param pStatus Output service status (optional)
     * @return true if service exists and status was retrieved
     */
    static bool GetServiceStatus(const wchar_t* wszServiceName, DWORD* pStatus);

    /**
     * Known installation paths for LDPlayer driver
     */
    static const wchar_t* KNOWN_DRIVER_PATHS[];
    static const size_t KNOWN_DRIVER_PATHS_COUNT;

    /**
     * Get device handle for driver communication
     * Only valid after successful Deploy()
     */
    HANDLE GetDeviceHandle() const { return m_hDevice; }

    /**
     * Release device handle ownership (transfer to another component)
     * After calling this, GetDeviceHandle() returns INVALID_HANDLE_VALUE
     * and Cleanup() will not close the handle.
     *
     * @return The handle (caller takes ownership)
     */
    HANDLE ReleaseDeviceHandle() {
        HANDLE h = m_hDevice;
        m_hDevice = INVALID_HANDLE_VALUE;
        return h;
    }

    /**
     * Check if deployment succeeded
     */
    bool IsDeployed() const { return m_bDeployed; }

    /**
     * Full cleanup - stop service, delete files, remove registry entries
     *
     * @param bDeleteFiles true to delete extracted driver file
     */
    void Cleanup(bool bDeleteFiles = true);

    /**
     * Get path to extracted driver file
     */
    const std::wstring& GetDriverPath() const { return m_wszDriverPath; }

    /**
     * Get service name used for deployment
     */
    const std::wstring& GetServiceName() const { return m_wszServiceName; }

    /**
     * Get last error message
     */
    const std::string& GetLastError() const { return m_LastError; }

    /**
     * Get deployment method that was used
     */
    DeployMethod GetDeployMethod() const { return m_DeployMethod; }

    /**
     * Default resource info for Ld9BoxSup.sys
     */
    static const DriverResource DEFAULT_RESOURCE;

private:
    HANDLE       m_hDevice;
    SC_HANDLE    m_hService;
    SC_HANDLE    m_hSCM;
    std::wstring m_wszDriverPath;
    std::wstring m_wszServiceName;
    std::string  m_LastError;
    bool         m_bDeployed;
    bool         m_bCreatedService;
    bool         m_bCreatedRegistry;
    DeployMethod m_DeployMethod;

    //-------------------------------------------------------------------------
    // Extraction
    //-------------------------------------------------------------------------

    /**
     * Extract encrypted driver from PE resources
     *
     * @param pResource Resource info
     * @param wszOutputPath Output file path
     * @return true if extraction and decryption succeeded
     */
    bool ExtractDriverFromResource(const DriverResource* pResource,
                                   const wchar_t* wszOutputPath);

    /**
     * Extract from raw encrypted blob
     *
     * @param pEncrypted Pointer to encrypted data
     * @param cbEncrypted Size of encrypted data
     * @param dwKey Decryption key (0 = use header)
     * @param wszOutputPath Output file path
     * @return true if decryption and write succeeded
     */
    bool ExtractFromBlob(const uint8_t* pEncrypted, size_t cbEncrypted,
                         uint32_t dwKey, const wchar_t* wszOutputPath);

    //-------------------------------------------------------------------------
    // SCM-based deployment
    //-------------------------------------------------------------------------

    /**
     * Deploy via Service Control Manager
     */
    bool DeployViaSCM(const DriverResource* pResource);

    //-------------------------------------------------------------------------
    // NtLoadDriver-based deployment
    //-------------------------------------------------------------------------

    /**
     * Deploy via NtLoadDriver (no SCM)
     */
    bool DeployViaNtLoadDriver(const DriverResource* pResource);

    /**
     * Create registry entries required for NtLoadDriver
     */
    bool CreateDriverRegistry(const wchar_t* wszDriverPath,
                              const wchar_t* wszServiceName);

    /**
     * Delete registry entries with retry logic
     * @return true if registry key deleted or doesn't exist, false on persistent failure
     */
    bool DeleteDriverRegistry(const wchar_t* wszServiceName);

    //-------------------------------------------------------------------------
    // Device access
    //-------------------------------------------------------------------------

    /**
     * Open device handle after driver is loaded
     */
    bool OpenDevice(const wchar_t* wszDeviceName);

    //-------------------------------------------------------------------------
    // Existing installation detection
    //-------------------------------------------------------------------------

    /**
     * Try to use an existing driver installation
     * Checks: running device → existing service → known paths
     *
     * @return true if existing driver was found and device opened
     */
    bool TryUseExistingDriver(const DriverResource* pResource);

    /**
     * Try to start an existing service and open device
     */
    bool TryStartExistingService(const wchar_t* wszServiceName,
                                  const wchar_t* wszDeviceName);

    /**
     * Try to register and start driver from a known path
     */
    bool TryUseDriverFromPath(const wchar_t* wszDriverPath,
                               const wchar_t* wszServiceName,
                               const wchar_t* wszDeviceName);

    //-------------------------------------------------------------------------
    // Error handling
    //-------------------------------------------------------------------------

    void SetError(const char* format, ...);
};

} // namespace supdrv
