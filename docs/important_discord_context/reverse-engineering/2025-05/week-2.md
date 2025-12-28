# May 2025 - Week 2
# Channel: #reverse-engineering
# Messages: 18

[2025-05-07 11:31] heh: EnumSystemFirmwareTables and GetSystemFirmwareTable don't seem to return ACPI's DSDT table on the buffers

[2025-05-07 11:32] heh: 
[Attachments: Screenshot_2025-05-07_133146.png]

[2025-05-07 11:32] heh: after more research, i found that intel's acpidump utility can retrieve it
https://github.com/acpica/acpica/blob/e374230f5fea5b2c92d9221ae3d5cad4d788569b/source/os_specific/service_layers/oswintbl.c#L421
[Embed: acpica/source/os_specific/service_layers/oswintbl.c at e374230f5fea...]
The ACPI Component Architecture (ACPICA) project provides an open-source operating system-independent implementation of the Advanced Configuration and Power Interface specification (ACPI).  For det...

[2025-05-07 11:33] heh: apparently, it retrieves this table from registry

[2025-05-07 11:34] heh: [replying to heh: "EnumSystemFirmwareTables and GetSystemFirmwareTabl..."]
does anyone know why this happens? tables like ssdt and others are fetched correctly by this winapi tho, its maybe just because the OEM didnt choose to publish DSDT through this interface?

[2025-05-07 11:39] heh: The only way i found to parse this table from cpl3 is to locate the Root System Description Pointer in the EBDGA and the BIOS upper-memory window, then map and parse the RSDT/XSDT given by the RSDP and locate the FADT which contains two fields for the DSDT's physical location

[2025-05-07 14:01] daax: [replying to heh: "does anyone know why this happens? tables like ssd..."]
GetSystemFirmwareTable with the ACPI provider would return the tables data buffer just fine as long as the table id from the previous enum/NtQuerySystemInformation call is correct. You'd need to share your code for anyone to tell you what you're doing incorrectly.

[2025-05-07 14:57] heh: you're right, after using _byteswap_ulong on the tableid now it correctly returns the table

[2025-05-07 14:58] heh: 
[Attachments: image.png]

[2025-05-07 14:58] heh: thank you

[2025-05-08 16:06] Windows2000Warrior**: anyone here know the differences between PROCESSOR_STATE_HANDLER2 in XP and PROCESSOR_STATE_HANDLER in Win2k ? when force the driver `processr.sys` to use PROCESSOR_STATE_HANDLER instead of PROCESSOR_STATE_HANDLER2 it works but no diffrences in results of cpu throttling means just the driver up but not work correctly

[2025-05-09 21:16] 5pider: https://github.com/N0fix/IDA_rust_metadata_finder
[Embed: GitHub - N0fix/IDA_rust_metadata_finder: IDA plugin to recover sour...]
IDA plugin to recover source code from panic information on rust - N0fix/IDA_rust_metadata_finder

[2025-05-10 00:41] the horse: Does anyone know what TEB->SchedulerSharedDataSlot is initialized with? (24h2+)

[2025-05-10 00:41] the horse: I don't want to dig much everywhere trying to see how to reconstruct it, it's used as part of RtlEnterCriticalSection

[2025-05-10 00:41] the horse: pretty much undocumented, no google search leads me anywhere

[2025-05-10 03:43] JustMagic: [replying to the horse: "Does anyone know what TEB->SchedulerSharedDataSlot..."]
Kernel returns 64 bytes of zeroed memory. Didn't really look deeply into what it's filled with in usermode.

[2025-05-10 03:53] JustMagic: Looks like it's just used for optimization of `NtWaitForAlertByThreadId` and its related functions + the auto thread priority boost stuff

[2025-05-10 23:42] the horse: thanks