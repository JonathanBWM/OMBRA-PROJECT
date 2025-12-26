#!/bin/bash
# Clone all reference codebases for Ombra Hypervisor development
# Run this script after cloning the main repository

set -e
cd "$(dirname "$0")"

echo "Cloning reference codebases (~1.3GB total)..."
echo ""

repos=(
    "https://github.com/cutecatsandvirtualmachines/CheatDriver.git"
    "https://github.com/tandasat/DdiMon.git"
    "https://github.com/TheCruZ/EFI_Driver_Access.git"
    "https://github.com/Mattiwatti/EfiGuard.git"
    "https://github.com/Gbps/gbhv.git"
    "https://github.com/wbenny/hvpp.git"
    "https://github.com/DarthTon/HyperBone.git"
    "https://github.com/tandasat/HyperPlatform.git"
    "https://github.com/noahware/hyper-reV.git"
    "https://github.com/Bareflank/hypervisor.git"
    "https://github.com/everdox/InfinityHook.git"
    "https://github.com/Zer0Mem0ry/KernelBhop.git"
    "https://github.com/HoShiMin/Kernel-Bridge.git"
    "https://github.com/asamy/ksm.git"
    "https://github.com/Zero-Tang/NoirVisor.git"
    "https://github.com/SafeBreach-Labs/PoolParty.git"
    "https://github.com/SamuelTulach/rainbow.git"
    "https://github.com/Cr4sh/s6_pcie_microblaze.git"
    "https://github.com/tandasat/SimpleSvm.git"
    "https://github.com/tandasat/SimpleSvmHook.git"
    "https://github.com/ionescu007/SimpleVisor.git"
    "https://github.com/cutecatsandvirtualmachines/SKLib.git"
    "https://github.com/cutecatsandvirtualmachines/Sputnik.git"
    "https://github.com/ajkhoury/UEFI-Bootkit.git"
    "https://github.com/lmcinnes/umap.git"
    "https://github.com/hfiref0x/VBoxHardenedLoader.git"
    "https://github.com/hzqst/VmwareHardenedLoader.git"
    "https://github.com/backengineering/Voyager.git"
)

for repo in "${repos[@]}"; do
    name=$(basename "$repo" .git)
    if [ -d "$name" ]; then
        echo "Skipping $name (already exists)"
    else
        echo "Cloning $name..."
        git clone --depth 1 "$repo" "$name"
    fi
done

echo ""
echo "Done! All reference codebases cloned."
