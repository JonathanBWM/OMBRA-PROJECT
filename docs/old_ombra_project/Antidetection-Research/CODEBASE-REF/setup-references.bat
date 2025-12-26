@echo off
REM Clone all reference codebases for Ombra Hypervisor development
REM Run this script after cloning the main repository

cd /d "%~dp0"

echo Cloning reference codebases (~1.3GB total)...
echo.

call :clone "https://github.com/cutecatsandvirtualmachines/CheatDriver.git" "CheatDriver"
call :clone "https://github.com/tandasat/DdiMon.git" "DdiMon"
call :clone "https://github.com/TheCruZ/EFI_Driver_Access.git" "EFI_Driver_Access"
call :clone "https://github.com/Mattiwatti/EfiGuard.git" "EfiGuard"
call :clone "https://github.com/Gbps/gbhv.git" "gbhv"
call :clone "https://github.com/wbenny/hvpp.git" "hvpp"
call :clone "https://github.com/DarthTon/HyperBone.git" "HyperBone"
call :clone "https://github.com/tandasat/HyperPlatform.git" "HyperPlatform"
call :clone "https://github.com/noahware/hyper-reV.git" "hyper-reV"
call :clone "https://github.com/Bareflank/hypervisor.git" "hypervisor"
call :clone "https://github.com/everdox/InfinityHook.git" "InfinityHook"
call :clone "https://github.com/Zer0Mem0ry/KernelBhop.git" "KernelBhop"
call :clone "https://github.com/HoShiMin/Kernel-Bridge.git" "Kernel-Bridge"
call :clone "https://github.com/asamy/ksm.git" "ksm"
call :clone "https://github.com/Zero-Tang/NoirVisor.git" "NoirVisor"
call :clone "https://github.com/SafeBreach-Labs/PoolParty.git" "PoolParty"
call :clone "https://github.com/SamuelTulach/rainbow.git" "rainbow"
call :clone "https://github.com/Cr4sh/s6_pcie_microblaze.git" "s6_pcie_microblaze"
call :clone "https://github.com/tandasat/SimpleSvm.git" "SimpleSvm"
call :clone "https://github.com/tandasat/SimpleSvmHook.git" "SimpleSvmHook"
call :clone "https://github.com/ionescu007/SimpleVisor.git" "SimpleVisor"
call :clone "https://github.com/cutecatsandvirtualmachines/SKLib.git" "SKLib"
call :clone "https://github.com/cutecatsandvirtualmachines/Sputnik.git" "Sputnik"
call :clone "https://github.com/ajkhoury/UEFI-Bootkit.git" "UEFI-Bootkit"
call :clone "https://github.com/lmcinnes/umap.git" "umap"
call :clone "https://github.com/hfiref0x/VBoxHardenedLoader.git" "VBoxHardenedLoader"
call :clone "https://github.com/hzqst/VmwareHardenedLoader.git" "VmwareHardenedLoader"
call :clone "https://github.com/backengineering/Voyager.git" "Voyager"

echo.
echo Done! All reference codebases cloned.
goto :eof

:clone
if exist "%~2" (
    echo Skipping %~2 (already exists)
) else (
    echo Cloning %~2...
    git clone --depth 1 "%~1" "%~2"
)
goto :eof
