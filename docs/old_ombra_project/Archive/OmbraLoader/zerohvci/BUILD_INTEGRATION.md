# Build Integration Guide: Hypercall Verification

## Adding to OmbraLoader.vcxproj

### Step 1: Add Assembly File

**Option A: Visual Studio GUI**

1. Right-click on OmbraLoader project → Add → Existing Item
2. Navigate to `zerohvci/` folder
3. Select `hypercall.asm`
4. Right-click `hypercall.asm` in Solution Explorer → Properties
5. Configuration: All Configurations
6. Item Type: Microsoft Macro Assembler
7. Click OK

**Option B: Edit .vcxproj Directly**

Add to OmbraLoader.vcxproj:

```xml
<ItemGroup>
  <MASM Include="zerohvci\hypercall.asm" />
</ItemGroup>
```

### Step 2: Include Header

In your source file (e.g., `main.cpp`):

```cpp
#include "zerohvci/hypercall_verify.h"
```

### Step 3: Use the API

```cpp
// After RuntimeHijacker.HijackHyperV()
uint64_t session_key = zerohvci::hypercall::VerifyHypervisorActive();
if (session_key == 0) {
    printf("Hypercall verification failed\n");
    return -1;
}

// session_key is now valid for all hypercalls
```

## Build Configuration

**Platform:** x64 only (MASM x64)

**Configuration:** Release or Debug (both work)

**Required Settings:**
- C++ Standard: C++17 or later (`/std:c++17`)
- Exception Handling: Yes (`/EHsc`)
- Platform Toolset: v143 (Visual Studio 2022) or v142 (VS 2019)

## Dependencies

**Headers:**
- `Windows.h` (Windows SDK)
- `intrin.h` (MSVC intrinsics)
- `OmbraShared/communication.hpp` (already in project)

**Libraries:**
- None additional (uses only Windows.lib)

**Other Projects:**
- OmbraShared (for VMCALL_TYPE and COMMAND_DATA definitions)

## Compiler Output

When building, you should see:

```
1>------ Build started: Project: OmbraLoader, Configuration: Release x64 ------
1>  hypercall.asm
1>  Assembling: hypercall.asm
1>  main.cpp
1>  OmbraLoader.vcxproj -> C:\...\Release\OmbraLoader.exe
========== Build: 1 succeeded, 0 failed, 0 up-to-date, 0 skipped ==========
```

## Troubleshooting

### "hypercall.asm is not recognized"

**Problem:** MASM not enabled for the file

**Solution:**
- Right-click hypercall.asm → Properties
- Item Type: Microsoft Macro Assembler
- Or add `<MASM Include="...">` to .vcxproj

### "error LNK2019: unresolved external symbol hypercall_asm"

**Problem:** Assembly file not being compiled

**Solutions:**
1. Check hypercall.asm is in project (visible in Solution Explorer)
2. Verify Item Type is set to "Microsoft Macro Assembler"
3. Clean solution and rebuild
4. Check Build Output for "Assembling: hypercall.asm" message

### "error A2008: syntax error"

**Problem:** MASM syntax error in hypercall.asm

**Solution:**
- Verify hypercall.asm matches the provided implementation exactly
- Check for stray characters or encoding issues
- Ensure file uses Windows line endings (CRLF)

### "cannot open include file 'communication.hpp'"

**Problem:** OmbraShared not in include path

**Solution:**
- Add OmbraShared to Additional Include Directories:
  - Project Properties → C/C++ → General → Additional Include Directories
  - Add: `$(ProjectDir)\..\OmbraShared`

## Example .vcxproj Snippet

Complete example of how files should appear in .vcxproj:

```xml
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">

  <!-- ... other settings ... -->

  <ItemGroup>
    <ClInclude Include="zerohvci\hypercall_verify.h" />
    <ClInclude Include="zerohvci\hyperv_hijack.h" />
    <ClInclude Include="zerohvci\zerohvci.h" />
    <!-- ... other headers ... -->
  </ItemGroup>

  <ItemGroup>
    <ClCompile Include="main.cpp" />
    <ClCompile Include="zerohvci\zerohvci.cpp" />
    <!-- ... other source files ... -->
  </ItemGroup>

  <ItemGroup>
    <MASM Include="zerohvci\hypercall.asm" />
  </ItemGroup>

  <ItemGroup>
    <ProjectReference Include="..\OmbraShared\OmbraShared.vcxproj">
      <Project>{GUID-HERE}</Project>
    </ProjectReference>
  </ItemGroup>

  <!-- ... rest of project ... -->

</Project>
```

## Verification After Build

**Check the output directory contains:**
- `OmbraLoader.exe`
- No hypercall.obj (gets linked into .exe)

**Run the program:**
```
OmbraLoader.exe
```

**Expected output (after successful hijack):**
```
[*] Generated session key: 0x...
[*] Setting communication key with hypervisor...
[+] Communication key set successfully
[*] Testing hypercall with VMCALL_GET_CR3...
[+] VMCALL_GET_CR3 succeeded
[+] Current CR3 (page directory base): 0x...
[+] Hypercall verification complete - communication channel active
```

## IDE-Specific Notes

### Visual Studio 2022

- MASM fully supported
- May need to install "C++ Build Tools" workload
- Check "MSVC v143 - VS 2022 C++ x64/x86 build tools"

### Visual Studio 2019

- MASM fully supported
- Platform toolset v142
- Ensure "Desktop development with C++" workload installed

### Visual Studio Code

- Use MSBuild command line:
  ```
  msbuild OmbraLoader.vcxproj /p:Configuration=Release /p:Platform=x64
  ```
- Ensure MSVC and MASM are in PATH

### CLion

- Import .vcxproj as Visual Studio project
- MASM files may need manual configuration
- Recommended: Use Visual Studio for MASM projects

## CMake Integration (Alternative)

If using CMake instead of .vcxproj:

```cmake
project(OmbraLoader)

# Enable MASM
enable_language(ASM_MASM)

add_executable(OmbraLoader
    main.cpp
    zerohvci/zerohvci.cpp
    zerohvci/hypercall.asm
)

target_include_directories(OmbraLoader PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../OmbraShared
)

target_link_libraries(OmbraLoader
    OmbraShared
)
```

## Cross-Compilation Notes

**This module is Windows x64 specific and cannot be cross-compiled:**

- MASM x64 assembly (Windows-only assembler)
- CPUID-based hypercall (x86-64 instruction)
- Windows API dependencies (GetCurrentProcessId)
- MSVC intrinsics (__rdtsc)

For Linux builds: Not applicable (Hyper-V is Windows-only)

## Production Build Settings

**For maximum performance and minimal size:**

```xml
<PropertyGroup Condition="'$(Configuration)'=='Release'">
  <Optimization>MaxSpeed</Optimization>
  <InlineFunctionExpansion>AnySuitable</InlineFunctionExpansion>
  <FavorSizeOrSpeed>Speed</FavorSizeOrSpeed>
  <WholeProgramOptimization>true</WholeProgramOptimization>
  <LinkTimeCodeGeneration>UseLinkTimeCodeGeneration</LinkTimeCodeGeneration>
</PropertyGroup>
```

**Note:** Inline functions in hypercall_verify.h will be optimized away at call sites.

## Debug Build Settings

**For easier debugging:**

```xml
<PropertyGroup Condition="'$(Configuration)'=='Debug'">
  <Optimization>Disabled</Optimization>
  <InlineFunctionExpansion>Disabled</InlineFunctionExpansion>
  <DebugInformationFormat>ProgramDatabase</DebugInformationFormat>
  <GenerateDebugInformation>true</GenerateDebugInformation>
</PropertyGroup>
```

**Debugging tips:**
- Set breakpoint in VerifyHypervisorActive()
- Watch session_key value
- Check RAX after hypercall_asm returns
- Monitor printf output for error messages

## Final Checklist

Before committing/shipping:

- [ ] hypercall.asm in project and set to MASM
- [ ] hypercall_verify.h included in source file
- [ ] OmbraShared dependency configured
- [ ] Builds successfully on Release x64
- [ ] Tested on actual hardware (Intel and AMD if possible)
- [ ] Verified hypercall returns valid session key
- [ ] Checked output for verification success messages

## Next Integration Steps

After hypercall verification is working:

1. **Phase 3.2**: Implement driver mapping using verified hypercalls
2. **Phase 3.3**: Register driver callback via VMCALL_STORAGE_QUERY
3. **Phase 3.4**: Set up EPT hiding for driver protection

All of these will use the session_key returned by VerifyHypervisorActive().
