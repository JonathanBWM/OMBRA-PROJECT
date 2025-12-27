# s6_pcie_microblaze DMA Attack Patterns

**Source**: Refs/codebases/s6_pcie_microblaze/
**Focus**: PCIe DMA attacks, TLP manipulation, physical memory access patterns

---

## 1. DMA ATTACK PATTERNS

### 1.1 PCIe TLP Construction

#### Memory Read TLPs (MRd32/MRd64)
**File**: `python/pcie_lib.py:1328-1371`

- **MRd64 packet construction** - 64-bit memory read TLP:
  - Header format: `FMT_4_NO_DATA` (4-dword header, no data payload)
  - TLP type: `0x20` (MRd64)
  - Fields: tag, requester ID, address (split into high/low dwords)
  - Byte enable fields: `h_first_dw_be = 0xf`, `h_last_dw_be = 0xf`
- **Address encoding** (`pcie_lib.py:1254-1279`):
  - 32-bit: Single dword at offset 2 (masked to `0xfffffffc`)
  - 64-bit: High dword at offset 2, low dword at offset 3
  - 4-byte alignment enforced
- **Random tag generation**: `random.randrange(0, 0xff)` for request tracking

#### Memory Write TLPs (MWr32/MWr64)
**File**: `python/pcie_lib.py:1374-1429`

- **MWr64 packet construction**:
  - Header format: `FMT_4_DATA` (4-dword header + data payload)
  - TLP type: `0x60` (MWr64)
  - Data appended after header as dword list
- **Data encoding** (`pcie_lib.py:1626-1628`):
  - Unpack bytes to big-endian dwords: `unpack('>' + ('I' * (size / 4)), data)`
  - Chunk data to avoid page boundary crossing

#### Completion TLPs (CplD)
**File**: `python/pcie_lib.py:1477-1493`

- **CplD decoding**:
  - Completer/requester IDs extracted from header
  - Tag matching for request correlation
  - Byte count indicates payload size
  - Data extraction: `get_data = lambda: pack('>' + ('I' * h_length), *data)`

### 1.2 DMA Memory Access Strategies

#### Chunked Memory Operations
**File**: `python/pcie_lib.py:1558-1604`

- **Read chunking** (`_mem_read`):
  - Default chunk: `MEM_RD_TLP_LEN = 0x40` (64 bytes)
  - Page boundary awareness - avoid crossing 4KB boundaries
  - Max chunk calculation: `min(chunk_size, PAGE_SIZE - (addr & 0xfff))`
  - Multiple CplD packets aggregated into single output
- **Write chunking** (`_mem_write`):
  - Default chunk: `MEM_WR_TLP_LEN = 0x04` (4 bytes)
  - Read-modify-write for unaligned access
  - Page-aligned splitting identical to reads

#### Alignment Handling
**File**: `python/pcie_lib.py:1637-1665`

- **4-byte alignment enforcement** (`MEM_ALIGN = 0x4`):
  - Unaligned reads: Round down start, round up size, extract subset
  - Unaligned writes: Read surrounding aligned block, patch bytes, write back
- **Helper functions**:
  - `align_down(x, a) = (x // a) * a`
  - `align_up(x, a) = ((x + a - 1) // a) * a`

#### Fast-Path Direct Access
**File**: `python/pcie_lib.py:1637-1673`

- **Bypass TLP layer** when `ep.fast_flow = True`:
  - Direct call to `ep.mem_read(addr, size)` / `ep.mem_write(addr, data)`
  - Used by `EndpointDriverDirect` class (line 1067)
  - Error handling: `errno.EFAULT` → TLP completion error
  - Zero-copy kernel driver interface via `/dev/zc_dma_mem_0`

---

## 2. PCIE COMMUNICATION

### 2.1 BAR Configuration

#### Configuration Space Registers
**File**: `python/pcie_lib.py:18-71`

- **BAR register definitions**:
  - `CFG_BASE_ADDRESS_0 = 0x10` (BAR0 - primary memory region)
  - `CFG_BASE_ADDRESS_1 = 0x14` (BAR1)
  - `CFG_BASE_ADDRESS_2 = 0x18` through `CFG_BASE_ADDRESS_5 = 0x24`
- **BAR read access** (`pcie_lib.py:356-368`):
  - `cfg_read(cfg_addr, cfg_size)` - generic config space accessor
  - Size validation: `[1, 2, 4]` bytes only
  - Multi-dword reads for spanning registers

#### Memory Region Mapping
**File**: `payloads/DmaBackdoorHv/backdoor_client/backdoor_client/winio.cpp:3-33`

- **Physical memory mapping via IOCTL**:
  - `IOCTL_WINIO_PHYS_MEM_MAP` - map BAR to user space
  - Returns: `section_handle`, `section_address`, `object` handle
  - Wraps `DeviceIoControl` for kernel driver communication
- **Shared memory structure** (`transport.h:44-51`):
  - `MEM_DEVICE` contains `SharedAddr` pointer to communication region
  - BAR0 stores physical address of shared memory

### 2.2 Device Enumeration

#### Bus-Device-Function Addressing
**File**: `python/pcie_lib.py:88-90`

- **BDF encoding/decoding**:
  - Encode: `(bus << 8) | (dev << 3) | (func << 0)`
  - Decode: `((val >> 8) & 0xff, (val >> 3) & 0x1f, (val >> 0) & 0x07)`
  - String format: `%.2x:%.2x.%x`
- **Device discovery** (`pcie_lib.py:327-347`):
  - Query status register for assigned BDF
  - Wait loop for root complex enumeration
  - `ErrorNotReady` if BDF still `0` after timeout

#### TLP Type Encoding
**File**: `python/pcie_lib.py:73-105`

- **Format/type split**:
  - Format (bits 29-30): `FMT_3_NO_DATA=0`, `FMT_4_NO_DATA=1`, `FMT_3_DATA=2`, `FMT_4_DATA=3`
  - Type (bits 24-28): `0x00=MRd32`, `0x20=MRd64`, `0x40=MWr32`, etc.
  - Combined lookup: `tlp_types = {0x00: 'MRd32', 0x20: 'MRd64', ...}`
- **Type extraction** (`pcie_lib.py:93-95`):
  - `tlp_type_name(dw0) = tlp_types[(dw0 >> 24) & 0xff]`

### 2.3 Transport Mechanisms

#### TCP/IP Transport
**File**: `python/pcie_lib.py:702-720`

- **Socket-based TLP forwarding**:
  - MicroBlaze firmware bridges PCIe ↔ TCP
  - Default port: `28472` (configurable)
  - Connection init: `Socket(addr=(host, port))`
  - Blocking reads with timeout via `select.select()`

#### Serial Transport
**File**: `python/pcie_lib.py:723-740`

- **UART-based communication**:
  - Default: `/dev/ttyUSB0` at `115200` baud
  - Protocol: `pack('=BB', command, size)` header
  - Non-blocking reads with manual timeout loop

#### Driver-Based Transport
**File**: `python/pcie_lib.py:901-1064`

- **Linux kernel module** (`zc_dma_mem.ko`):
  - `/dev/zc_dma_mem_0` - memory access device
  - `/dev/zc_dma_mem_1` - raw TLP device
  - IOCTLs: `ZC_DMA_MEM_IOCTL_RESET`, `GET_DEVICE_ID`, `CONFIG_READ`
- **TLP send/recv** (`pcie_lib.py:975-993`):
  - Write TLP: `os.write(fd_1, pack('I', dw) * len(data))`
  - Read TLP: `os.read(fd_1, RECV_BUFF_LEN)` (4KB buffer)
  - Timeout error: `errno.ETIME` exception

---

## 3. HARDWARE ACCESS

### 3.1 Physical Memory Access Patterns

#### Direct Memory Read
**File**: `python/pcie_mem.py:36-49`

- **Dump to file**:
  - Page-aligned reads: `dev.mem_read(addr + ptr, PAGE_SIZE)`
  - Iterative 4KB chunks until size exhausted
  - Progress printing every page
- **Hexdump output** (`pcie_lib.py:139-168`):
  - 16-byte wide format with address prefix
  - ASCII representation with non-printable filtering
  - `quoted(data) = ''.join(b if b.isalnum() else '.')`

#### Shared Memory Communication
**File**: `payloads/DmaBackdoorBoot/backdoor/dma_shell_user/transport.h:8-42`

- **Bidirectional channels**:
  - `MEM_SHARED` structure: 4KB total (`MEM_SHARED_SIZE = 0x1000`)
  - RX channel: `RxHdr` (12 bytes) + `RxData` (2036 bytes)
  - TX channel: `TxHdr` (12 bytes) + `TxData` (2036 bytes)
- **Header format**:
  - `Sign = 0x92de7fc0` - magic signature
  - `Command`: `IDLE=0`, `TXD=1`, `ACK=2`
  - `Size`: payload byte count
- **Poll-based protocol** (`transport.h:3-6`):
  - Poll interval: `DEVICE_IO_POLL = 20ms`
  - Timeout: `DEVICE_IO_TIMEOUT = 3000ms`

#### DMA Backdoor Memory Layout
**File**: `payloads/DmaBackdoorBoot/src/DmaBackdoorBoot.c:93-105`

- **Debug output buffer**:
  - Address advertised via EFI variable `BACKDOOR_VAR_NAME`
  - Size: `DEBUG_OUTPUT_SIZE` pages
  - Allocated as `EfiRuntimeServicesData` for OS persistence
- **Status reporting regions**:
  - `INFECTOR_STATUS_ADDR` - backdoor metadata
  - `HYPER_V_INFO_ADDR` - hypervisor hook status
  - `m_PendingOutputAddr` - pointer to debug log

### 3.2 EPT/NPT Physical Address Translation

#### EPT PTE Address Calculation
**File**: `payloads/DmaBackdoorHv/backdoor_client/backdoor_client/backdoor_client.cpp:103-158`

- **VTL detection via EPT inspection**:
  - `ept_is_host_vtl0()`: Execute bit usually clear at 0x1000
  - `ept_is_host_vtl1()`: Execute bit usually set at 0x1000
  - `ept_is_guest()`: Physical address ≠ guest physical address
- **PTE retrieval** (lines 108-118):
  - `backdoor_pte_ept_addr(0x1000, &pte_addr, &pte_size, ept_addr)`
  - `backdoor_phys_read_64(pte_addr, &pte_val)`
  - Permission check: `EPT_X(pte_val)` macro for execute bit

#### Free PTE Scanning
**File**: `payloads/DmaBackdoorHv/backdoor_client/backdoor_client/backdoor_client.cpp:160-228`

- **Virtual memory hole finding**:
  - Scan range: `addr_start` to `addr_end` (e.g., kernel space)
  - Step: `PAGE_SIZE` (4KB)
  - Filter: `pte_size == HVBD_PTE_SIZE_4K` and `pte == 0`
  - Contiguity tracking: Find `size/PAGE_SIZE` consecutive free PTEs

#### Payload Injection
**File**: `payloads/DmaBackdoorHv/backdoor_client/backdoor_client/backdoor_client.cpp:230-250`

- **VM injection flow**:
  - Read current CR3: `backdoor_vmread(GUEST_CR3, &current_pml4_addr)`
  - Read current EPT: `backdoor_ept_addr(&current_ept_addr)`
  - Find free virtual range via `backdoor_find_free_pte()`
  - Allocate physical pages and map via EPT manipulation

### 3.3 UEFI Pre-Boot Memory Scanning

#### EFI System Table Discovery
**File**: `README.MD:625-629`

- **Memory scan parameters**:
  - Default range: `0xf0000000` down to `0x0`
  - Step: `0x10000` (64KB)
  - Override: `SCAN_FROM`, `SCAN_STEP` environment variables
- **Signature matching**:
  - `EFI_SYSTEM_TABLE` magic: `IBI SYST` at offset 0x40
  - `LocateProtocol()` offset within `EFI_BOOT_SERVICES`

#### Protocol Entry Hijack
**File**: `README.MD:627-629`

- **Alternative injection method**:
  - Scan range: `0x76000000` to `0xa0000000`
  - Step: `0x1000` (4KB)
  - Target: `EFI_CPU_IO2_PROTOCOL` structure
  - Hook: Patch function pointer in protocol vtable

#### ACPI Table Enumeration
**File**: `python/uefi.py` (referenced in `README.MD:631`)

- **Information gathering without attack**:
  - Pause at BIOS menu to freeze DXE environment
  - Scan for UEFI protocols, loaded drivers
  - Extract ACPI tables (RSDT, XSDT, MADT, etc.)
  - Dump descriptor tables (GDT, IDT) physical addresses

---

## SUMMARY

**Key Takeaways for Ombra Hypervisor**:

1. **TLP Layer Abstraction**: Build packet classes similar to `PacketMRd64`/`PacketMWr64` with encode/decode separation
2. **Page Boundary Safety**: Always check `(addr & 0xfff) + size <= PAGE_SIZE` before issuing TLPs
3. **Alignment Discipline**: Enforce 4-byte aligned access; handle unaligned via read-modify-write
4. **Shared Memory Protocol**: Use dual-channel header+data structure with signature validation and command polling
5. **EPT-Based Injection**: Scan for free PTEs, validate page size, allocate contiguous physical memory
6. **BAR0 Indirection**: Store communication region physical address in BAR0, not payload itself
7. **Transport Flexibility**: Abstract TLP send/recv to support TCP, serial, or direct kernel driver backends

**Performance Patterns**:
- Small writes (4 bytes) for control flow hooks
- Large reads (64 bytes) for memory dumps
- Fast-path bypass for high-throughput scenarios

**Security Considerations**:
- No IOMMU assumptions - scan for accessible memory ranges
- Tag randomization to avoid TLP collision
- Error handling for CplD vs Cpl (access denied) differentiation
