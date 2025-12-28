# December 2025 - Week 4
# Channel: #programming
# Messages: 12

[2025-12-22 01:12] plpg: I dont even know where to start

[2025-12-22 01:12] plpg: I uhh

[2025-12-22 01:12] plpg: Fill the shellcode buffer with 0xcc and see if the debugger triggers

[2025-12-22 01:13] plpg: This shit is beyond debuggable

[2025-12-22 01:13] plpg: Does Rustc even support PIC?

[2025-12-22 04:04] ImagineHaxing: [replying to plpg: "Does Rustc even support PIC?"]
I mean no std is pic if u make ot

[2025-12-22 04:07] ImagineHaxing: Ill try to attach x64dbg while this is in memory and do it step by step

[2025-12-22 06:24] ImagineHaxing: Yea I think the output shellcode is fucked...

[2025-12-22 10:49] ImagineHaxing: i managed to debug and saw that there was certainly somethig wrong with the binary

[2025-12-22 10:49] ImagineHaxing: rebuilt with different flags and now it works

[2025-12-22 14:28] plpg: Nice

[2025-12-24 08:31] grb: is there any NT API that triggers an invlpg instruction internally? i want to flush the TLB across all cores from usermode. previously my idea is to do a context switch (by simply using NtReadVirtualMemory/NtWriteVirtualMemory), but Gemini said that it doesnt flush the TLB across all cores, idk if its correct or not tbh