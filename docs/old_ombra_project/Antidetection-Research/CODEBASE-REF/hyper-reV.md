# hyper-reV Pattern Extraction
**Focus**: Reverse Engineering & Debugging Features

---

## 1. REVERSE ENGINEERING HELPERS

### Memory Inspection Utilities

**Guest Physical Memory Operations**
- `hyperv-attachment/src/hypercall/hypercall.cpp:16-64` - `operate_on_guest_physical_memory()` with page boundary handling
- `hyperv-attachment/src/memory_manager/memory_manager.cpp:18-30` - `map_guest_physical()` translates GPA→HPA via SLAT
- `hyperv-attachment/src/memory_manager/memory_manager.cpp:6` - Identity map at `255ull << 39` for host physical access
- Handles read/write across page boundaries with size tracking (`size_left_of_page`)

**Guest Virtual Memory Operations**
- `hyperv-attachment/src/hypercall/hypercall.cpp:66-128` - `operate_on_guest_virtual_memory()` with CR3 parameter
- `hyperv-attachment/src/memory_manager/memory_manager.cpp:32-100` - Full page walk (PML4→PDPT→PD→PT) with large page support
- Returns size remaining in page for multi-page operations

**Memory Translation**
- `hyperv-attachment/src/memory_manager/memory_manager.cpp:32` - `translate_guest_virtual_address()` handles 1GB/2MB/4KB pages
- Lines 50-60: 1GB page detection via `pdpte.large_page`
- Lines 71-81: 2MB page detection via `pde.large_page`
- Returns 0 on non-present pages for safety

**Host Physical Access**
- `hyperv-attachment/src/memory_manager/memory_manager.cpp:8-16` - `map_host_physical()` / `unmap_host_physical()` via identity map base

### Structure Dumping

**Kernel Module Dumping**
- `usermode/src/system/system.cpp:20-55` - `dump_kernel_module()` reads headers + sections
- Line 42: Uses `portable_executable::image_t` for PE parsing
- Lines 46-52: Iterates sections, reads virtual memory per section
- `usermode/src/commands/commands.cpp:481-504` - `dkm` command writes module to disk

**Module Export Parsing**
- `usermode/src/system/system.cpp:72-86` - `parse_module_exports()` builds symbol map
- Line 76-82: Converts RVA to full kernel address (`module_base + delta`)
- Creates `module_name!export_name` aliases for commands

**PsLoadedModuleList Parsing**
- `usermode/src/system/system.cpp:182-200+` - `parse_modules()` walks kernel linked list
- Line 197: Reads flink via `read_kernel_virtual_memory<uint64_t>()`
- Uses hypercalls to traverse `_LDR_DATA_TABLE_ENTRY` structures
- Maintains module list with base addresses and sizes

---

## 2. DEBUGGING FEATURES

### Breakpoint Support (SLAT-Based Hooks)

**EPT/NPT Code Hook Interface**
- `usermode/src/commands/commands.cpp:259-310` - `akh` (add kernel hook) command
- Lines 263-266: Accepts `--asmbytes`, `--post_original_asmbytes`, `--monitor` flags
- `usermode/src/hook/hook.cpp:184-200+` - `add_kernel_hook()` implementation
- Line 42: `hypercall::add_slat_code_hook(physical_page, shadow_page)`

**Shadow Page Setup**
- `usermode/src/hook/hook.cpp:71-78` - `load_original_bytes_into_shadow_page()`
- Reads 0x1000 or 0x2000 bytes (for overflow hooks crossing page boundary)
- Uses `hook_disasm::get_routine_aligned_bytes()` for instruction alignment

**Inline Hook Construction**
- `usermode/src/hook/hook.cpp:80-142` - `set_up_inline_hook()` builds detour
- Lines 82-86: 14-byte long jump pattern (push low32, mov [rsp+4] high32, ret)
- Lines 108-134: Handles hooks that overflow into next page
- Writes to shadow page mapped in usermode

**Detour Holder**
- `usermode/src/hook/hook.cpp:14-52` - `set_up()` allocates shadow page for ntoskrnl.exe padding section
- Line 49: `kernel_detour_holder::set_up()` manages detour memory pool
- `usermode/src/system/system.cpp:57-70` - `find_kernel_detour_holder_base_address()` finds executable padding section
- Uses SLAT hook to hide detour page from guest

**RIP-Relative Fixup**
- `usermode/src/hook/hook_disassembly.cpp:1-150+` - Full instruction rewriting
- Uses Zydis disassembler/encoder library
- Lines 37-50: `do_push()` - 64-bit value push helper
- Lines 52-62: `do_rip_relative_push()` - RIP-relative addressing fixup
- Lines 75-91: `do_jmp()` / `do_relative_jmp()` - absolute/relative jump generation
- Lines 108-130: `jcc_instructions` array - conditional jump handling
- Resolves RIP-relative memory accesses and branches to absolute addresses

### Trace Mechanisms

**Processor State Logging**
- `shared/structures/trap_frame.h:26-32` - `trap_frame_log_t` captures all GPRs + RIP + CR3 + stack
- `hyperv-attachment/src/logs/logs.cpp:11-26` - `set_up()` allocates 4 pages (contiguous) for log storage
- Line 25: Calculates max logs as `(4*0x1000) / sizeof(trap_frame_log_t)`

**Log Capture**
- `hyperv-attachment/src/logs/logs.cpp:28-42` - `add_log()` appends to ring buffer with mutex
- `hyperv-attachment/src/hypercall/hypercall.cpp:189-199` - `log_current_state()` called on monitored hooks
- Lines 175-187: `do_stack_data_copy()` reads 5 QWORDS from guest stack
- `hyperv-attachment/src/hypercall/hypercall.cpp:130-173` - `copy_stack_data_from_log_exit()` handles stack across page boundaries

**Log Flushing**
- `hyperv-attachment/src/logs/logs.cpp:44-60` - `flush()` copies logs to guest usermode buffer
- Line 48: Returns last N logs (user-specified count)
- `usermode/src/commands/commands.cpp:369-412` - `fl` command formats and prints logs
- Lines 399-407: Prints all GPRs, RIP, CR3, and stack data

**Monitor Injection**
- `usermode/src/commands/commands.cpp:280-298` - `--monitor` flag injects CPUID hypercall
- Lines 282-287: Inline ASM: `push rcx; mov ecx, <hypercall_id>; cpuid; pop rcx`
- Called every time hooked function executes before original bytes

### Command-Line Interface

**Memory Commands**
- `usermode/src/commands/commands.cpp:47-74` - `rgpm` (read guest physical memory)
- `usermode/src/commands/commands.cpp:76-104` - `wgpm` (write guest physical memory)
- `usermode/src/commands/commands.cpp:106-136` - `cgpm` (copy guest physical memory)
- `usermode/src/commands/commands.cpp:138-156` - `gvat` (translate guest virtual address)
- `usermode/src/commands/commands.cpp:158-187` - `rgvm` (read guest virtual memory)
- `usermode/src/commands/commands.cpp:189-219` - `wgvm` (write guest virtual memory)
- `usermode/src/commands/commands.cpp:221-257` - `cgvm` (copy guest virtual memory)

**Hook Commands**
- `usermode/src/commands/commands.cpp:259-310` - `akh` (add kernel hook)
- `usermode/src/commands/commands.cpp:312-335` - `rkh` (remove kernel hook)
- `usermode/src/commands/commands.cpp:337-360` - `hgpp` (hide guest physical page)

**Module Commands**
- `usermode/src/commands/commands.cpp:428-441` - `lkm` (list kernel modules)
- `usermode/src/commands/commands.cpp:443-469` - `kme` (kernel module exports)
- `usermode/src/commands/commands.cpp:471-504` - `dkm` (dump kernel module)

**Alias System**
- `usermode/src/commands/commands.cpp:522-533` - `form_aliases()` creates symbol lookup map
- Line 524: Adds `current_cr3` alias for process context
- Lines 526-530: Adds module names and all exports as command aliases
- Example: Can use `ntoskrnl.exe!PsLookupProcessByProcessId` instead of raw address

---

## 3. ANALYSIS PATTERNS

### How It Facilitates RE Work

**Hypercall Architecture**
- `hyperv-attachment/src/hypercall/hypercall.cpp:219-311` - `process()` dispatcher
- Uses CPUID instruction for VM exits (guest → hypervisor communication)
- `shared/hypercall/hypercall_def.h` - Defines hypercall types via primary/secondary keys
- No kernel driver needed - all hypervisor interaction via hypercalls

**Hypercall Types** (from README.md lines 210-219):
- `guest_physical_memory_operation` - Read/write GPA
- `guest_virtual_memory_operation` - Read/write GVA with CR3
- `translate_guest_virtual_address` - VA→PA translation
- `read_guest_cr3` - Get current process context
- `add_slat_code_hook` - Install EPT/NPT hook
- `remove_slat_code_hook` - Uninstall hook
- `hide_guest_physical_page` - SLAT-hide memory from guest
- `log_current_state` - Capture processor state
- `flush_logs` - Retrieve logs to usermode
- `get_heap_free_page_count` - Hypervisor heap status

**CLI11 Command Parsing**
- `usermode/src/commands/commands.cpp:535-595` - `process()` main dispatcher
- Lines 543-568: Initializes all subcommands with alias transformers
- Lines 570-594: Try/catch parsing with error handling
- Supports case-insensitive commands and hex/decimal input

**PE Format Analysis**
- `usermode/ext/portable_executable/` - Complete PE parser library (external)
- `usermode/src/system/system.cpp:40-54` - Uses `portable_executable::image_t` for headers/sections
- Supports export directory parsing, section iteration
- Used for module dumping and symbol resolution

**Live Kernel Debugging Without KD**
- No Windows Debugging API dependencies
- Direct hypervisor memory access bypasses kernel protections
- SLAT hooks invisible to kernel (execute shadow pages)
- Works under HVCI/VBS (leverages Hyper-V itself)

**Stack Unwinding**
- `hyperv-attachment/src/hypercall/hypercall.cpp:175-187` - `do_stack_data_copy()` captures stack context
- Handles RSP across page boundaries
- Captures 5 QWORDs of stack data per log entry
- Useful for call chain analysis

**NMI-Based Synchronization**
- `hyperv-attachment/src/interrupts/interrupts.cpp` - NMI handler for SLAT cache flushes
- `hyperv-attachment/src/apic/` - APIC library for IPI delivery
- README.md lines 185-198: NMI fired to all logical processors when hooks added/removed
- Ensures TLB coherency across all cores

---

## KEY TAKEAWAYS FOR OMBRA

### Pattern Adoption Recommendations

1. **Trap Frame Logging System** - Implement similar structure for capturing execution context
   - Store logs in hypervisor heap (not guest-accessible)
   - Flush to guest usermode on demand via hypercall
   - Include stack snapshot (5+ QWORDs)

2. **RIP-Relative Fixup** - Critical for reliable hooking
   - Use Zydis or similar disassembler
   - Rewrite JCC, JMP, CALL with RIP-relative operands to absolute
   - Handle memory operands like `cmp [rip+x], 0`

3. **Detour Holder in Existing Code** - Avoid allocating new executable pages
   - Find padding sections in ntoskrnl.exe (.Pad sections)
   - SLAT-hide the detour holder page
   - Store original bytes + return jump in this hidden page

4. **Page Boundary Awareness** - All memory operations check `size_left_of_page`
   - Split operations across pages when needed
   - Handle hooks that span page boundaries (overflow hooks)

5. **Symbol Alias System** - User-friendly debugging
   - Parse PsLoadedModuleList for loaded modules
   - Build map of module exports
   - Allow commands like `hook ntoskrnl!MmMapIoSpace` instead of raw addresses

6. **Multi-Page Log Buffer** - Allocate contiguous pages during init
   - hyper-reV uses 4 pages = ~256 log entries
   - Mutex-protected circular buffer
   - Return count of logs flushed to usermode

7. **Hypercall via CPUID** - Portable, no special instructions
   - Uses unique values in RCX (primary/secondary key) + RAX (call type)
   - Checks keys in VM exit handler to distinguish from legitimate CPUID

### Risks & Mitigations

| Risk | Mitigation from hyper-reV |
|------|---------------------------|
| Hook instability from RIP-relative | Full instruction decoding/rewriting (Zydis) |
| Log buffer overflow | Fixed-size buffer with mutex, returns max count |
| Multi-core cache incoherency | NMI-based INVEPT/INVVPID synchronization |
| Detection via detour page | Use existing ntoskrnl padding, SLAT-hide it |
| Stack read across pages | `copy_stack_data_from_log_exit()` handles boundaries |
| PE parsing failures | Validate MZ signature, check section bounds |

---

**Files Referenced**:
- `hyperv-attachment/src/hypercall/hypercall.cpp` - Main hypercall dispatcher + memory ops
- `hyperv-attachment/src/memory_manager/memory_manager.cpp` - GPA/GVA translation + mapping
- `hyperv-attachment/src/logs/logs.cpp` - Trap frame log storage/flushing
- `usermode/src/commands/commands.cpp` - CLI command implementations
- `usermode/src/hook/hook.cpp` - Kernel hooking via shadow pages
- `usermode/src/hook/hook_disassembly.cpp` - Instruction rewriting with Zydis
- `usermode/src/system/system.cpp` - Module parsing and dumping
- `shared/structures/trap_frame.h` - Register/stack capture structure
- `shared/hypercall/hypercall_def.h` - Hypercall interface definitions
- `README.md` - Architecture documentation (lines 44-365)

**Generated**: 2025-12-20
