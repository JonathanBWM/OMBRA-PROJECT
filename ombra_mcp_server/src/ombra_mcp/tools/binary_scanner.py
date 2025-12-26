#!/usr/bin/env python3
"""
Binary Signature Scanner — Detect known hypervisor patterns in compiled binaries

This module scans compiled binaries for:
- String literals from known hypervisors
- Instruction sequences (code patterns)
- Pool tags
- Import patterns
- Known function signatures
"""

import re
import struct
from pathlib import Path
from typing import Optional
import json

# =============================================================================
# SIGNATURE DATABASE
# =============================================================================

STRING_SIGNATURES = [
    # HyperPlatform
    {"pattern": b"HyperPlatform", "source": "HyperPlatform", "risk": "critical"},
    {"pattern": b"Virtualized processor", "source": "HyperPlatform", "risk": "critical"},
    {"pattern": b"EPT violation", "source": "HyperPlatform", "risk": "high"},
    {"pattern": b"VMCS corruption", "source": "HyperPlatform", "risk": "high"},
    {"pattern": b"VMX enabled", "source": "HyperPlatform", "risk": "medium"},
    
    # SimpleSvm
    {"pattern": b"SimpleSvm", "source": "SimpleSvm", "risk": "critical"},
    {"pattern": b"SvmVmrun", "source": "SimpleSvm", "risk": "high"},
    {"pattern": b"Nested page fault", "source": "SimpleSvm", "risk": "medium"},
    
    # Voyager
    {"pattern": b"voyager", "source": "Voyager", "risk": "critical"},
    {"pattern": b"Voyager", "source": "Voyager", "risk": "critical"},
    {"pattern": b"vmexit_handler", "source": "Voyager", "risk": "high"},
    
    # gbhv / Blue Pill variants
    {"pattern": b"GoodBye", "source": "gbhv", "risk": "critical"},
    {"pattern": b"BluePill", "source": "BluePill", "risk": "critical"},
    {"pattern": b"YOURH YPERvisor", "source": "Generic", "risk": "critical"},
    
    # Generic hypervisor strings
    {"pattern": b"Hypervisor", "source": "Generic", "risk": "high"},
    {"pattern": b"hypervisor", "source": "Generic", "risk": "high"},
    {"pattern": b"VMXON", "source": "Generic", "risk": "medium"},
    {"pattern": b"VMLAUNCH", "source": "Generic", "risk": "medium"},
    {"pattern": b"VM-exit", "source": "Generic", "risk": "medium"},
    {"pattern": b"vmexit", "source": "Generic", "risk": "medium"},
    
    # Debug strings that shouldn't be in release
    {"pattern": b"DbgPrint", "source": "Debug", "risk": "medium"},
    {"pattern": b"KdPrint", "source": "Debug", "risk": "medium"},
    {"pattern": b"ASSERT", "source": "Debug", "risk": "low"},
    {"pattern": b"TODO", "source": "Debug", "risk": "low"},
    {"pattern": b"FIXME", "source": "Debug", "risk": "low"},
    
    # CPUID vendor strings (should never appear in binary)
    {"pattern": b"KVMKVMKVM", "source": "KVM", "risk": "critical"},
    {"pattern": b"Microsoft Hv", "source": "Hyper-V", "risk": "critical"},
    {"pattern": b"VMwareVMware", "source": "VMware", "risk": "critical"},
    {"pattern": b"XenVMMXenVMM", "source": "Xen", "risk": "critical"},
    {"pattern": b"VBoxVBoxVBox", "source": "VirtualBox", "risk": "critical"},
]

POOL_TAG_SIGNATURES = [
    {"tag": b"HvPl", "source": "HyperPlatform", "risk": "critical"},
    {"tag": b"SSvm", "source": "SimpleSvm", "risk": "critical"},
    {"tag": b"Vgr ", "source": "Voyager", "risk": "critical"},
    {"tag": b"VMXr", "source": "Generic VMX", "risk": "high"},
    {"tag": b"VMCS", "source": "Generic", "risk": "high"},
    {"tag": b"Vmcs", "source": "Generic", "risk": "high"},
    {"tag": b"EPT ", "source": "Generic EPT", "risk": "high"},
    {"tag": b"Ept ", "source": "Generic EPT", "risk": "high"},
    {"tag": b"HyPv", "source": "Generic Hypervisor", "risk": "high"},
    {"tag": b"VmxP", "source": "Generic VMX", "risk": "high"},
    {"tag": b"Hvlp", "source": "Hyper-V like", "risk": "medium"},
]

# Instruction sequences that are suspicious
# Format: (bytes_pattern, mask, description, source, risk)
CODE_SIGNATURES = [
    # Typical VMXON sequence
    {
        "pattern": bytes([0x0F, 0xC7, 0x30]),  # VMXON [rax]
        "mask": bytes([0xFF, 0xFF, 0xF8]),
        "description": "VMXON instruction",
        "source": "VMX",
        "risk": "info"  # Expected, but check context
    },
    # VMLAUNCH
    {
        "pattern": bytes([0x0F, 0x01, 0xC2]),
        "mask": bytes([0xFF, 0xFF, 0xFF]),
        "description": "VMLAUNCH instruction",
        "source": "VMX",
        "risk": "info"
    },
    # VMRESUME
    {
        "pattern": bytes([0x0F, 0x01, 0xC3]),
        "mask": bytes([0xFF, 0xFF, 0xFF]),
        "description": "VMRESUME instruction",
        "source": "VMX",
        "risk": "info"
    },
    # Sequential VMWRITE pattern (20+ in a row is suspicious)
    {
        "pattern": bytes([0x0F, 0x79]),  # VMWRITE
        "mask": bytes([0xFF, 0xFF]),
        "description": "VMWRITE instruction",
        "source": "VMX",
        "risk": "info",
        "check_density": True,
        "density_threshold": 15,
        "density_window": 200
    },
    # HyperPlatform-specific patterns
    {
        "pattern": bytes([
            0x48, 0x8B, 0x01,  # mov rax, [rcx]
            0x48, 0x8B, 0x49, 0x08,  # mov rcx, [rcx+8]
            0xFF, 0xE0  # jmp rax
        ]),
        "mask": bytes([0xFF] * 9),
        "description": "HyperPlatform dispatch pattern",
        "source": "HyperPlatform",
        "risk": "high"
    },
]

# Import combinations that indicate hypervisor
SUSPICIOUS_IMPORTS = [
    {
        "imports": ["ExAllocatePoolWithTag", "MmGetPhysicalAddress", "KeIpiGenericCall"],
        "description": "Typical hypervisor allocation pattern",
        "risk": "medium"
    },
    {
        "imports": ["__vmx_vmread", "__vmx_vmwrite", "__vmx_on"],
        "description": "MSVC VMX intrinsics",
        "risk": "info"
    },
]

# =============================================================================
# SCANNER IMPLEMENTATION
# =============================================================================

class BinarySignatureScanner:
    def __init__(self, binary_path: str):
        self.path = Path(binary_path)
        self.data = b""
        self.findings = []
        
    def load(self) -> bool:
        """Load binary file."""
        try:
            self.data = self.path.read_bytes()
            return True
        except Exception as e:
            self.findings.append({
                "type": "error",
                "message": f"Failed to load binary: {e}"
            })
            return False
    
    def scan_strings(self) -> list:
        """Scan for known string signatures."""
        findings = []
        
        for sig in STRING_SIGNATURES:
            pattern = sig["pattern"]
            offset = 0
            
            while True:
                idx = self.data.find(pattern, offset)
                if idx == -1:
                    break
                    
                # Get context around the match
                start = max(0, idx - 16)
                end = min(len(self.data), idx + len(pattern) + 16)
                context = self.data[start:end]
                
                findings.append({
                    "type": "string",
                    "offset": f"0x{idx:X}",
                    "pattern": pattern.decode('utf-8', errors='replace'),
                    "source": sig["source"],
                    "risk": sig["risk"],
                    "context_hex": context.hex(),
                    "recommendation": f"Remove string literal '{pattern.decode('utf-8', errors='replace')}'"
                })
                
                offset = idx + 1
        
        return findings
    
    def scan_pool_tags(self) -> list:
        """Scan for known pool tags."""
        findings = []
        
        for sig in POOL_TAG_SIGNATURES:
            tag = sig["tag"]
            offset = 0
            
            while True:
                idx = self.data.find(tag, offset)
                if idx == -1:
                    break
                
                # Check if this looks like a pool tag context
                # Pool tags often appear near ExAllocatePoolWithTag calls
                findings.append({
                    "type": "pool_tag",
                    "offset": f"0x{idx:X}",
                    "tag": tag.decode('utf-8', errors='replace'),
                    "source": sig["source"],
                    "risk": sig["risk"],
                    "recommendation": f"Replace pool tag '{tag.decode()}' with random 4-byte tag"
                })
                
                offset = idx + 1
        
        return findings
    
    def scan_code_patterns(self) -> list:
        """Scan for known instruction sequences."""
        findings = []
        
        for sig in CODE_SIGNATURES:
            pattern = sig["pattern"]
            mask = sig.get("mask", bytes([0xFF] * len(pattern)))
            
            matches = []
            for i in range(len(self.data) - len(pattern)):
                match = True
                for j in range(len(pattern)):
                    if (self.data[i + j] & mask[j]) != (pattern[j] & mask[j]):
                        match = False
                        break
                if match:
                    matches.append(i)
            
            # Check density if required
            if sig.get("check_density") and len(matches) >= sig.get("density_threshold", 10):
                # Check if matches are clustered
                for i, offset in enumerate(matches):
                    window = sig.get("density_window", 200)
                    nearby = sum(1 for m in matches if abs(m - offset) < window)
                    
                    if nearby >= sig.get("density_threshold", 10):
                        findings.append({
                            "type": "code_pattern",
                            "offset": f"0x{offset:X}",
                            "description": f"High density of {sig['description']} ({nearby} in {window} bytes)",
                            "source": sig["source"],
                            "risk": "high",
                            "recommendation": "Interleave instructions to break pattern"
                        })
                        break
            elif matches and not sig.get("check_density"):
                if sig["risk"] != "info":
                    for offset in matches[:5]:  # Report first 5
                        findings.append({
                            "type": "code_pattern",
                            "offset": f"0x{offset:X}",
                            "description": sig["description"],
                            "source": sig["source"],
                            "risk": sig["risk"],
                            "recommendation": f"Review code at offset 0x{offset:X}"
                        })
        
        return findings
    
    def scan_pe_imports(self) -> list:
        """Scan PE imports for suspicious combinations."""
        findings = []
        
        # Simple PE import extraction
        # Look for IMAGE_IMPORT_DESCRIPTOR
        try:
            # Find import names in binary
            found_imports = set()
            
            # Common import function names to look for
            import_names = [
                b"ExAllocatePoolWithTag",
                b"ExAllocatePool2",
                b"MmGetPhysicalAddress",
                b"MmMapIoSpace",
                b"KeIpiGenericCall",
                b"KeGenericCallDpc",
                b"__vmx_vmread",
                b"__vmx_vmwrite",
                b"__vmx_on",
                b"ZwQuerySystemInformation",
                b"MmAllocateContiguousMemory",
            ]
            
            for name in import_names:
                if name in self.data:
                    found_imports.add(name.decode())
            
            # Check for suspicious combinations
            for combo in SUSPICIOUS_IMPORTS:
                matches = [imp for imp in combo["imports"] if imp in found_imports]
                if len(matches) >= 2:
                    findings.append({
                        "type": "import_pattern",
                        "imports": matches,
                        "description": combo["description"],
                        "risk": combo["risk"],
                        "recommendation": "Consider if all imports are necessary"
                    })
                    
        except Exception as e:
            pass  # PE parsing failed, skip import analysis
        
        return findings
    
    def calculate_entropy(self, window_size: int = 256) -> list:
        """Find high-entropy sections (possible encryption/packing)."""
        import math
        findings = []
        
        for i in range(0, len(self.data) - window_size, window_size):
            chunk = self.data[i:i + window_size]
            
            # Calculate entropy
            freq = {}
            for byte in chunk:
                freq[byte] = freq.get(byte, 0) + 1
            
            entropy = 0
            for count in freq.values():
                p = count / window_size
                entropy -= p * math.log2(p)
            
            # High entropy (> 7.5) might indicate encryption
            if entropy > 7.5:
                findings.append({
                    "type": "entropy",
                    "offset": f"0x{i:X}",
                    "entropy": round(entropy, 2),
                    "risk": "info",
                    "description": "High entropy section (possible encrypted/compressed data)"
                })
        
        return findings
    
    def scan_all(self) -> dict:
        """Run all scans and return consolidated results."""
        if not self.load():
            return {"success": False, "error": "Failed to load binary"}
        
        all_findings = []
        all_findings.extend(self.scan_strings())
        all_findings.extend(self.scan_pool_tags())
        all_findings.extend(self.scan_code_patterns())
        all_findings.extend(self.scan_pe_imports())
        
        # Sort by risk
        risk_order = {"critical": 0, "high": 1, "medium": 2, "low": 3, "info": 4}
        all_findings.sort(key=lambda x: risk_order.get(x.get("risk", "info"), 5))
        
        # Determine if binary is safe
        critical_count = sum(1 for f in all_findings if f.get("risk") == "critical")
        high_count = sum(1 for f in all_findings if f.get("risk") == "high")
        
        return {
            "success": True,
            "binary_path": str(self.path),
            "binary_size": len(self.data),
            "total_findings": len(all_findings),
            "risk_summary": {
                "critical": critical_count,
                "high": high_count,
                "medium": sum(1 for f in all_findings if f.get("risk") == "medium"),
                "low": sum(1 for f in all_findings if f.get("risk") == "low"),
                "info": sum(1 for f in all_findings if f.get("risk") == "info")
            },
            "safe": critical_count == 0 and high_count == 0,
            "findings": all_findings
        }


# =============================================================================
# MCP TOOL INTERFACE
# =============================================================================

async def scan_binary_signatures(binary_path: str) -> dict:
    """
    Scan compiled binary for known hypervisor signatures.
    
    Checks:
    - String literals from known hypervisors
    - Pool tags
    - Code patterns (instruction sequences)
    - Import patterns
    
    Args:
        binary_path: Path to compiled .sys, .exe, or .dll
    
    Returns:
        Scan results with findings and recommendations
    """
    scanner = BinarySignatureScanner(binary_path)
    return scanner.scan_all()


async def scan_source_for_signatures(source_code: str) -> dict:
    """
    Scan source code for patterns that will become signatures.
    
    Args:
        source_code: C/C++ source code as string
    
    Returns:
        Potential signature issues
    """
    findings = []
    
    # Check for problematic strings
    string_patterns = [
        (r'"[^"]*[Hh]ypervisor[^"]*"', "Hypervisor string literal"),
        (r'"[^"]*[Vv]irtualiz[^"]*"', "Virtualization string literal"),
        (r'"[^"]*VMCS[^"]*"', "VMCS string literal"),
        (r'"[^"]*VMX[^"]*"', "VMX string literal"),
        (r'"[^"]*EPT[^"]*"', "EPT string literal"),
        (r'DbgPrint\s*\(', "Debug print statement"),
        (r'KdPrint\s*\(', "Kernel debug print"),
        (r'#define\s+POOL_TAG\s+[\'"]?(HvPl|SSvm|Vgr|VMX|VMCS)', "Known pool tag"),
    ]
    
    for pattern, description in string_patterns:
        matches = re.finditer(pattern, source_code)
        for match in matches:
            line_num = source_code[:match.start()].count('\n') + 1
            findings.append({
                "type": "source_pattern",
                "line": line_num,
                "match": match.group()[:50],
                "description": description,
                "risk": "high" if "pool tag" in description.lower() else "medium",
                "recommendation": f"Remove or obfuscate: {description}"
            })
    
    # Check for sequential VMWRITE calls
    vmwrite_pattern = r'__vmx_vmwrite\s*\([^)]+\)\s*;'
    vmwrites = list(re.finditer(vmwrite_pattern, source_code))
    
    if len(vmwrites) > 15:
        # Check if they're clustered
        for i in range(len(vmwrites) - 10):
            span = vmwrites[i + 10].start() - vmwrites[i].start()
            if span < 2000:  # 10 VMWRITEs in ~2000 chars
                line_num = source_code[:vmwrites[i].start()].count('\n') + 1
                findings.append({
                    "type": "code_pattern",
                    "line": line_num,
                    "description": f"Dense VMWRITE sequence ({10} calls in {span} chars)",
                    "risk": "medium",
                    "recommendation": "Interleave VMWRITE calls with other operations"
                })
                break
    
    return {
        "findings": findings,
        "safe": not any(f["risk"] in ["critical", "high"] for f in findings)
    }


# Test if run directly
if __name__ == "__main__":
    import sys
    if len(sys.argv) > 1:
        scanner = BinarySignatureScanner(sys.argv[1])
        result = scanner.scan_all()
        print(json.dumps(result, indent=2))
