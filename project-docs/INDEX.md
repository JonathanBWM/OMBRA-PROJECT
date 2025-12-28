# PROJECT-OMBRA Documentation Index

## GENERATION STATUS
```
Generated: 2025-12-27
Git hash: 73853be
Verified: Spot-checked against source code
```

---

## Documentation Hierarchy

### Tier 1: Project Level

| Document | Purpose | Status |
|----------|---------|--------|
| [00_PROJECT_OVERVIEW.md](./00_PROJECT_OVERVIEW.md) | What the project does, components, tech stack | COMPLETE |
| [01_ARCHITECTURE.md](./01_ARCHITECTURE.md) | How components connect, data flows, lifecycle | COMPLETE |

### Tier 2: Component Deep Dives

| Document | Component | Status |
|----------|-----------|--------|
| [HYPERVISOR_CORE.md](./components/HYPERVISOR_CORE.md) | VMX, VMCS, EPT, handlers | COMPLETE |
| [BYOVD_SUPDRV.md](./components/BYOVD_SUPDRV.md) | Ld9BoxSup.sys exploitation | COMPLETE |
| [USERMODE_LOADER.md](./components/USERMODE_LOADER.md) | PE mapping, hypervisor loading | COMPLETE |
| [EXIT_HANDLERS.md](./components/EXIT_HANDLERS.md) | VM-exit dispatch, stealth handlers | COMPLETE |
| [MCP_SERVERS.md](./components/MCP_SERVERS.md) | 152 ombra-mcp + 59 driver-re-mcp tools | COMPLETE |

### Tier 3: Critical File Implementation

| Document | File | Status |
|----------|------|--------|
| [vmx_c.md](./implementation/vmx_c.md) | hypervisor/hypervisor/vmx.c | COMPLETE |
| [ept_c.md](./implementation/ept_c.md) | hypervisor/hypervisor/ept.c | COMPLETE |

---

## Verification Results

**Spot-checks performed:**

| Claim | Location | Result |
|-------|----------|--------|
| MAX_CPUS = 256 | vmx.h:212 | VERIFIED |
| VMCALL key hardcoded | hv_loader.c:241 | VERIFIED |
| EPT_ENTRIES_PER_TABLE = 512 | ept_defs.h:62 | VERIFIED |
| 21 exit reasons handled | exit_dispatch.c | VERIFIED |
| 152 MCP tools | server.py | VERIFIED |

---

## Confidence Assessment

| Component | Structure | Runtime | Stealth |
|-----------|-----------|---------|---------|
| Hypervisor Core | HIGH | MEDIUM | MEDIUM |
| BYOVD/Supdrv | HIGH | HIGH | LOW |
| Usermode Loader | HIGH | MEDIUM | LOW |
| Exit Handlers | HIGH | MEDIUM | MEDIUM |
| MCP Servers | HIGH | HIGH | N/A |
| EPT Management | HIGH | MEDIUM | MEDIUM |
| VMX Operations | HIGH | MEDIUM | N/A |

---

## Known Gaps

### Hypervisor
- [ ] Host IDT configuration for #PF during VM-exit
- [ ] VMRESUME failure handling
- [ ] Host stack overflow protection
- [ ] Maximum concurrent hooks limit

### BYOVD
- [ ] ThrottleStop bypass not documented
- [ ] -618 error root cause unclear
- [ ] ETW cleanup specifics

### Loader
- [ ] Dynamic CPU count >256 handling
- [ ] Import resolution for different ntoskrnl versions
- [ ] Rollback on partial load failure

### EPT
- [ ] MMIO memory type handling
- [ ] Concurrent split protection
- [ ] PT table tracking for granular permissions

---

## File Count Summary

| Category | Files | Lines (approx) |
|----------|-------|----------------|
| Hypervisor Core | 15 | 4,500 |
| Exit Handlers | 12 | 2,400 |
| Usermode Loader | 10 | 1,200 |
| BYOVD Interface | 4 | 800 |
| Shared Headers | 8 | 1,500 |
| MCP Servers | 25+ | 8,000+ |

---

## Usage

### For New Developers
1. Start with `00_PROJECT_OVERVIEW.md` to understand what the project does
2. Read `01_ARCHITECTURE.md` for component relationships
3. Dive into specific component docs as needed

### For Implementation Work
1. Check component doc for the area you're modifying
2. Reference implementation docs for critical file details
3. Use MCP tools for Intel SDM lookups and code generation

### For Debugging
1. Check component doc's CONCERNS section
2. Look at GAPS AND UNKNOWNS for known limitations
3. Cross-reference with verification claims

---

*Index generated 2025-12-27 as part of codebase documentation effort*
