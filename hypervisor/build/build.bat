@echo off
setlocal enabledelayedexpansion

REM =============================================================================
REM OmbraHypervisor Build Script
REM Auto-detects Visual Studio installation using vswhere
REM =============================================================================

echo.
echo ============================================
echo   OmbraHypervisor Build Script
echo ============================================
echo.

REM Try to find vswhere.exe
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo ERROR: vswhere.exe not found. Visual Studio may not be installed.
    echo Expected location: %VSWHERE%
    exit /b 1
)

REM Find Visual Studio installation path
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set "VS_PATH=%%i"
)

if not defined VS_PATH (
    echo ERROR: Visual Studio with C++ tools not found.
    echo Please install Visual Studio with "Desktop development with C++" workload.
    exit /b 1
)

echo Found Visual Studio at: %VS_PATH%

REM Setup Visual Studio environment
set "VCVARS=%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat"
if not exist "%VCVARS%" (
    echo ERROR: vcvars64.bat not found at %VCVARS%
    exit /b 1
)

echo Initializing x64 build environment...
call "%VCVARS%" >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo ERROR: Failed to initialize Visual Studio environment.
    exit /b 1
)

REM Verify required tools
where cl.exe >nul 2>&1 || (echo ERROR: cl.exe not found && exit /b 1)
where ml64.exe >nul 2>&1 || (echo ERROR: ml64.exe (MASM) not found && exit /b 1)
where lib.exe >nul 2>&1 || (echo ERROR: lib.exe not found && exit /b 1)

echo Toolchain verified: cl.exe, ml64.exe, lib.exe
echo.

REM Get script directory and project root
set "SCRIPT_DIR=%~dp0"
set "PROJECT_ROOT=%SCRIPT_DIR%.."

REM =============================================================================
REM Build Usermode Loader
REM =============================================================================
echo [1/3] Building usermode loader...
cd /d "%PROJECT_ROOT%\usermode"

REM NOTE: payload_loader.c and loader_api.c removed - legacy code using broken structures
REM       main.c now uses hv_loader.c which uses correct supdrv.c implementation
REM NOTE: drv_loader.c removed - Phase 2 feature not yet integrated, uses old ALLOC_INFO
cl.exe /nologo /O2 /W4 /WX- /Fe:loader.exe ^
    main.c driver_interface.c ^
    loader\hv_loader.c loader\pe_parser.c loader\pe_mapper.c ^
    loader\pe_relocs.c loader\pe_iat.c loader\pe_imports.c ^
    loader\pe_utils.c loader\pe_wipe.c ^
    byovd\supdrv.c byovd\deployer.c byovd\crypto.c byovd\nt_helpers.c byovd\throttlestop.c ^
    /I..\shared /I. ^
    /link kernel32.lib advapi32.lib

if %ERRORLEVEL% neq 0 (
    echo ERROR: Usermode loader build failed!
    exit /b 1
)
echo    loader.exe built successfully.
echo.

REM =============================================================================
REM Build Hypervisor Assembly
REM =============================================================================
echo [2/3] Building hypervisor assembly...
cd /d "%PROJECT_ROOT%\hypervisor"

if not exist asm\obj mkdir asm\obj

ml64.exe /nologo /c /Fo:asm\obj\vmexit.obj asm\vmexit.asm
if %ERRORLEVEL% neq 0 (echo ERROR: vmexit.asm failed && exit /b 1)

ml64.exe /nologo /c /Fo:asm\obj\intrinsics.obj asm\intrinsics.asm
if %ERRORLEVEL% neq 0 (echo ERROR: intrinsics.asm failed && exit /b 1)

ml64.exe /nologo /c /Fo:asm\obj\segment.obj asm\segment.asm
if %ERRORLEVEL% neq 0 (echo ERROR: segment.asm failed && exit /b 1)

echo    Assembly modules built.
echo.

REM =============================================================================
REM Build Hypervisor C Code
REM =============================================================================
echo [3/3] Building hypervisor C code...

if not exist obj mkdir obj

REM Core modules
cl.exe /nologo /c /O2 /W4 /WX- /I..\shared /Foobj\ ^
    entry.c vmx.c vmcs.c ept.c exit_dispatch.c timing.c ^
    hooks.c debug.c nested.c spoof.c

if %ERRORLEVEL% neq 0 (
    echo ERROR: Core module compilation failed!
    exit /b 1
)

REM Exit handlers
cl.exe /nologo /c /O2 /W4 /WX- /I..\shared /Foobj\ ^
    handlers\cpuid.c handlers\msr.c handlers\rdtsc.c ^
    handlers\cr_access.c handlers\exception.c handlers\io.c ^
    handlers\vmcall.c handlers\vmcall_phase2.c ^
    handlers\ept_violation.c handlers\mtf.c ^
    handlers\ept_misconfig.c handlers\power_mgmt.c

if %ERRORLEVEL% neq 0 (
    echo ERROR: Handler compilation failed!
    exit /b 1
)

REM Create static library
lib.exe /nologo /OUT:hypervisor.lib obj\*.obj asm\obj\*.obj
if %ERRORLEVEL% neq 0 (
    echo ERROR: Library creation failed!
    exit /b 1
)

echo    hypervisor.lib created successfully.
echo.

REM =============================================================================
REM Summary
REM =============================================================================
echo ============================================
echo   Build Complete!
echo ============================================
echo.
echo Artifacts:
echo   usermode\loader.exe
echo   hypervisor\hypervisor.lib
echo.

cd /d "%SCRIPT_DIR%"
endlocal
