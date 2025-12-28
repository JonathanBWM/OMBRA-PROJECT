# August 2025 - Week 3
# Channel: #programming
# Messages: 146

[2025-08-11 01:12] VMMX: hi anyone have build remill before ?

[2025-08-11 01:28] UJ: [replying to VMMX: "hi anyone have build remill before ?"]
They have a docker image so you dont have to build it

[2025-08-11 01:28] UJ: but i have also built it, what issue are you having?

[2025-08-11 01:35] VMMX: [replying to UJ: "but i have also built it, what issue are you havin..."]
yeah i have issue with xed (:

[2025-08-11 01:36] VMMX: first z3 but i mange to fix it them some other tools

[2025-08-11 01:36] VMMX: now im with xed```re
CMake Error at CMakeLists.txt:89 (find_package):
  Could not find a package configuration file provided by "XED" with any of
  the following names:

    XEDConfig.cmake
    xed-config.cmake

  Add the installation prefix of "XED" to CMAKE_PREFIX_PATH or set "XED_DIR"
  to a directory containing one of the above files.  If "XED" provides a
  separate development package or SDK, be sure it has been installed.```

[2025-08-11 01:38] VMMX: ```git clone https://github.com/intelxed/xed.git xed
git clone https://github.com/intelxed/mbuild.git mbuild
cd xed
./mfile.py install --shared```

[2025-08-11 01:39] VMMX: those are the commands that i tried to run to fix the problem

[2025-08-11 01:39] VMMX: but i still can't find the XEDConfig.cmake or xed-config.cmake

[2025-08-11 03:48] VMMX: as u can see i need one of those files XEDConfig.cmake or xed-config.cmake but my xen doesn't generate any of those and doesn't have a CMakeLists.txt anywhere in its original repo

[2025-08-11 05:16] VMMX: well to solve it i had to create the files and configure them manually it work but im not sure what is the normal solution i got other errors ,im still looking for a guide to setup remill sadly couldn't find something that work for me

[2025-08-11 23:22] ml: Hi, does anyone know how VEH/SEH execution works during TLS callback execution?

[2025-08-11 23:50] JustMagic: [replying to ml: "Hi, does anyone know how VEH/SEH execution works d..."]
Do you have something specific in mind? Because there should be no real difference.

[2025-08-12 10:45] ml: [replying to JustMagic: "Do you have something specific in mind? Because th..."]
Actually, I don’t know how to apply it to the execution of the TLS callbacks.

[2025-08-12 11:42] Brit: [replying to ml: "Actually, I don’t know how to apply it to the exec..."]
What do you mean by this, the tls callback runs, if there is an exception, the usual flow happens, call any veh , then look up the ExceptionDirectory for entries matching

[2025-08-12 11:42] Brit: Etc etc etc

[2025-08-12 11:42] Brit: Go take a peek at KiUserExceptionDispatcher

[2025-08-12 11:52] Brit: In fact this "tech" was used by overwatch to prevent people from just create remote threading after manual mapping some code into the game. By having a tls callback that would trigger a ud2 iirc.

[2025-08-12 19:29] ml: [replying to Brit: "Go take a peek at KiUserExceptionDispatcher"]
I did some research, but I didn’t really understand. If you could help me, I’m in a usermode context and I need to implement exception handling for the execution of my TLS callbacks and my DllMain (my DLL is protected with VMP). The problem is, I have no idea how to implement exception handling. If you have any resources (no skid stuff btw) to share or explanations, I’d be interested

[2025-08-12 19:40] Brit: I don't really understand what this could possibly mean, when a thread is created the flow eventually ends up in LdrpCallTlsInitializers which as its name implies calls the tls callback, at this point you are in a regular process and exception handling happens normally, you can go look at kiuserexcpetiondispatcher

[2025-08-12 19:46] ml: [replying to Brit: "I don't really understand what this could possibly..."]
I do everything myself, i don't register the dll into the PEB

[2025-08-12 19:47] ml: So, there is no LdrpCallTlsInitializers

[2025-08-12 19:59] Brit: So you call the tls callback manually like everyone else manual mapping.

[2025-08-12 20:03] Brit: And then if you need seh you have to do the whole rtladdfunctable rigmarole.

[2025-08-12 20:08] ml: [replying to Brit: "And then if you need seh you have to do the whole ..."]
Okay

[2025-08-13 12:31] Eriktion: [replying to Brit: "In fact this "tech" was used by overwatch to preve..."]
Yes correct

[2025-08-13 12:36] dwordxyz: [replying to Brit: "In fact this "tech" was used by overwatch to preve..."]
1+

[2025-08-14 16:33] ml: Does anyone know why my VMP-protected DLL crashes when manually mapped if I don’t execute it at its preferred image base? I’ve done all the relocations, import fixes, exception handling, etc., but the problem seems to be that VMP has some hardcoded values pointing to addresses that are expected to work in the DLL only when it’s executed at its preferred image base.

[2025-08-14 16:53] Brit: either you dont do relocs, or vmp has some funny features that you do not account for

[2025-08-14 16:56] ml: [replying to Brit: "either you dont do relocs, or vmp has some funny f..."]
I do the relocations, but I think it's the second one i'll go check

[2025-08-14 16:57] ml: [replying to Brit: "either you dont do relocs, or vmp has some funny f..."]
Oh, I completely forgot, but I think I need to do the relocations in VMP’s sections

[2025-08-14 16:58] Brit: I feel like I should be charging a consultation fee for how long this conversation has gone on <:topkek:904522829616263178>

[2025-08-14 16:59] ml: [replying to Brit: "I feel like I should be charging a consultation fe..."]
lol at this point I might need to start taking donations just to keep up 😂

[2025-08-14 18:16] Torph: <@148095953742725120> just went to install OpenShell and was surprised to see you on the release page lol 
thanks for the ARM64 support :D

[2025-08-14 18:51] Matti: <:lillullmoa:475778601141403648> no problem at all

[2025-08-14 18:51] Matti: it was a tiny change that took 4 years to get merged

[2025-08-14 18:51] Matti: but they managed eventually

[2025-08-14 18:53] Matti: I was kinda surprised when I got a notification of the PR being merged, cause I unsubscribed from updates about it ages ago

[2025-08-15 13:49] froj: For some prior context I'm writing a debugger using the DbgEng & DbgHelp APIs.

When creating and attaching to a live usermode 64-bit process, the architecture is grabbed correctly and all is good. However when specifying a 32-bit target, after attaching (on initial break) both `GetEffectiveProcessorType` & `GetExecutingProcessorType` return AMD64 still. Once I continue, on the _next_ debug event they return I386 as expected.

Anyone know why this would be the case/how I could work around it? The debugger is compiled as 64-bit so my initial assumption is that this could be syswow related.

Screenshots for further info below also.
[Attachments: image.png, image.png, image.png]

[2025-08-15 13:50] froj: One potential workaround could be to grab the actual file and get it's architecture from there, then use `SetEffectiveProcessorType`, but I'm unsure if that's feasible for remote debugging (not implemented yet) and would like to try and implement a generic solution

[2025-08-15 14:07] froj: https://x.com/bbrachaczek/status/1517042840404119553 stumbled across this, will report back
@lowleveldesign @timmisiak Of the 3, only the effective can possibly be x86\. But you have to SetEffectiveProcessorType first\. Windbg does it for you, except when you inspect a full memory dump, wher

[2025-08-15 14:08] Brit: grab peb, read fields you know are pointers, by the size of the ptr you know if you are in a 64bit or 32 bit process

[2025-08-15 14:09] 0xatul: 
[Attachments: Screenshot_2025-08-15-19-54-10-49_0b2fce7a16bf2b728d6ffa28c8d60efb.jpg]

[2025-08-15 14:12] froj: Legend, will give that a go

[2025-08-15 14:35] froj: Can confirm works, thanks! :)

[2025-08-15 21:39] Matti: [replying to froj: "For some prior context I'm writing a debugger usin..."]
> Anyone know why this would be the case
I'm pretty sure you're right and that this is wow64 related

from memory, in a 64 bit debugger you should receive 2 load notification events for ntdll, because there are two ntdlls loaded in a wow64 process (one 32 bit below 4GB, and one 64 bit)

[2025-08-15 21:40] Matti: the 64 bit one comes first by necessity because some of the wow64 usermode logic is native 64 bit code

[2025-08-16 07:58] abu: [replying to VMMX: "well to solve it i had to create the files and con..."]
Hey, I had the same issues. I think <@162611465130475520> has a GitHub repository that has cmake files for XED and some others. Try looking there

[2025-08-16 12:38] virpy: Whats some peoples favourite techniques/tips for learning assembly?

[2025-08-16 12:43] Brit: https://discord.com/channels/835610998102425650/835656385421115403/1397495491297546342

[2025-08-16 20:15] cinder: So I am developing my bootloader and I am stuck in a bit of a pickle: it seems like I cant get breakpoints to trigger when I try to debug it using QEMU + GDB. Given that I am developing it using EDK2 I can call `CpuBreakpoint()`, which is a wrapper for `__debugbreak()` . I see the QEMU VM stopping but on GDB nothing happens. For reference, here is how I launch QEMU (it's a VM which has a Windows 10 install on it, my end goal is to chainload Windows eventually):
```
qemu-system-x86_64 \
  -S -s \
  -machine q35,accel=tcg \
  -smp 1 \
  -m 4096 \
  -drive if=pflash,format=raw,readonly=on,file=OVMF_CODE.fd \
  -drive if=pflash,format=raw,file=OVMF_VARS.fd \
  -drive file=win10.qcow2,format=qcow2,if=virtio \
  -cdrom inferno-bootable.iso \
  -drive file=virtio.iso,media=cdrom \
  -boot d \
  -net nic,model=virtio \
  -vga qxl \
  -usb -device usb-tablet \
  -debugcon file:debug.log -global isa-debugcon.iobase=0x402 \
  -d int -D qemu-int.log
```
`inferno-bootable.iso` is my EFI application in a .ISO format. The OVMF binaries were compiled targeting DEBUG directly from source, the reason I am using a debug build is to have a debug trace over serial (in this case it is dumped to `debug.log`), but the debug build hits an assertion at some point within the code and I would like to investigate it further. Moreover, it should trigger a breakpoint which should be caught by GDB,  but that does not happen.

The `-d int -D qemu-int.log` is my last ditch attempt at checking out which interrupts is QEMU registering and, by grepping the `qemu-int.log` trace, I see no breakpoints being triggered, even if the machine halts when I call `CpuBreakpoint()`.  For reference, here is what I see (omitted for brevity):
```
➜  win10 cat qemu-int.log | grep INT
Servicing hardware INT=0x20
Servicing hardware INT=0x20
Servicing hardware INT=0x20
Servicing hardware INT=0x20
[... list goes on with more 0x20s, but no #BP!]
Servicing hardware INT=0x20
Servicing hardware INT=0x20
```
I connect to the GDB stub by using `target remote :1234` and, if I ctrl+c, I can break within the stub, read registers, disassemble... but breakpoints will not trigger, even when hitting a `__debugbreak()` GDB does not react at all, unless I ctrl+c of course. 
Alas, for reference, I leave the assertion that is logged to the serial port by OVMF:
```
==> debug.log <== 
FSOpen: Open '\EFI\BOOT\BOOTX64.EFI' Success 
[Bds] Expand PciRoot(0x0)/Pci(0x1,0x1)/Ata(Secondary,Master,0x0) -> PciRoot(0x0)/Pci(0x1,0x1)/Ata(Secondary,Master,0x0)/CDROM(0x0,0x22,0x5000)/\EFI\BOOT\BOOTX64.EFI [Security] 3rd party image[0] can be loaded after EndOfDxe: PciRoot(0x0)/Pci(0x1,0x1)/Ata(Secondary,Master,0x0)/CDROM(0x0,0x22,0x5000)/\EFI\BOOT\BOOTX64.EFI. None of Tcg2Protocol/CcMeasurementProtocol is installed. InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B BEA96040 
Loading driver at 0x000BDEAA000 EntryPoint=0x000BDEB2160 InfernoBootloader.efi InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF BF015698 
ProtectUefiImageCommon - 0xBEA96040 - 0x00000000BDEAA000 - 0x000000000000E140 
ASSERT ZeroMemWrapper.c(47): Buffer != ((void *) 0)
```
Completely lost on what to do, clearly I am missing something. Any ideas?

[2025-08-16 20:16] cinder: Also I don't know if this is the correct channel for asking QEMU related questions, let me know if I should move the message to another channel and I'll do it ASAP

[2025-08-16 20:20] cinder: P.S: I'm developing it on Arch, using EDK2's `edk2-stable202505` branch. For building I am using the Ubuntu 22 Docker container Tianocore's provides, using CLANGDWARF toolchain.

[2025-08-16 21:21] Matti: [replying to cinder: "Also I don't know if this is the correct channel f..."]
nah this is fine

[2025-08-16 21:22] Matti: [replying to cinder: "So I am developing my bootloader and I am stuck in..."]
I don't know the answer to your question (what's causing this), but FWIW I just tried this on my own machine which has a completely different setup, and gdb behaves the same for me
which is to say it doesn't do the thing

[2025-08-16 21:24] Matti: I do get an absolutely stopped machine though unlike you, I'm *guessing* this might be because I'm also redirecting serial to stdout

[2025-08-16 21:24] Matti: `-chardev stdio,mux=on,id=char0 -serial chardev:char0` <-- this is for windows, but IIRC this should be the same on linux

[2025-08-16 21:24] Matti: 
[Attachments: image.png]

[2025-08-16 21:26] Matti: as you can see this trashes the entire console output, but having it this way on windows is still more useful to me than nothing at all

[2025-08-16 21:27] Matti: hm actually on second thought the muxing is just to make DEBUG() statements readable in theory

[2025-08-16 21:28] Matti: without it I'd probably have a dead VM with no clue as to the reason unless I was also storing the serial output in a log file like you

[2025-08-16 21:30] Matti: the 'nice' (by OVMF/edk2 standards) debug print with stacktrace is due to the PDB, I think you should get the same if you use CLANGPDB instead but that isn't going to improve anything in gdb

[2025-08-16 21:48] cinder: [replying to Matti: "I do get an absolutely stopped machine though unli..."]
In my case the machine goes unresponsive as well, I guess it either enters a dead loop or it actually triggers a #BP, but it should be visible from the interrupt logs. I can't quite piece everything together because one thing contradicts the other.

I think I'll go for a workaround, since the assertion is caused by a call to AllocateZeroPool() I'll work around it by implementing it myself with AllocatePool() and ZeroMem() instead and see if I manage to dodge the assertion.

For debugging yeah I really have to check out why GDB or QEMU are misbehaving like that. I may ask on the EDK2 mailing list if I am missing some important step, but for now I'll see if vmware can expose some gdb stub like QEMU and see if GDB misbehaves with vmware too.

[2025-08-16 21:49] cinder: Many thanks for your help and for spending time recreating the issue, I really appreciate the support 🙏

[2025-08-16 21:49] Matti: FWIW it doesn't seem to be gdb, I tested this in lldb and it behaves identically for me

[2025-08-16 21:50] Matti: [replying to cinder: "Many thanks for your help and for spending time re..."]
nws, I have a bunch of qemu debug related traumas so I try to help people if I can <:lillullmoa:475778601141403648>

[2025-08-16 21:57] cinder: [replying to Matti: "FWIW it doesn't seem to be gdb, I tested this in l..."]
guess it could be something with QEMU, tomorrow I'll try setting up VMWare for debugging (if at all possible, but I do remember GDB functionality being present) and if I can successfully break into GDB. I'll report back with the results, but I am seriously weirded out because I remember successfully debugging an earlier version of the bootloader which wasn't relying on EDK2 but gnu-efi

[2025-08-16 21:58] cinder: and I think that QEMU is indeed breaking given that the machine freezes, so I suspect it is not communicating properly to the debugger instance for some unknown reason. maybe I am speculating too much without understanding how QEMU works at all

[2025-08-16 21:59] Matti: yeah I want to say I know who is to blame between OVMF/edk2 and qemu but I don't
and yeah interested to hear the results

[2025-08-16 22:00] Matti: [replying to cinder: "and I think that QEMU is indeed breaking given tha..."]
no one knows how qemu works

[2025-08-16 22:00] Matti: it's fine, this is commonly understood

[2025-08-16 22:02] Matti: I would try this on vbox if I could, but I haven't used its gdb stub before (it was added in v7) so that'd take a bit too much time I think

[2025-08-16 22:02] Matti: vmware I don't even have installed, I hate it with a passion

[2025-08-16 22:04] Yoran: [replying to cinder: "So I am developing my bootloader and I am stuck in..."]
The weird thing to me is that it works fine besides breakpoints

[2025-08-16 22:06] Yoran: [replying to cinder: "So I am developing my bootloader and I am stuck in..."]
Can you DEBUG the entry point of your code?

[2025-08-16 22:07] Yoran: Not debug, I mean DEBUG like logging efi_d_info

[2025-08-16 22:10] Yoran: [replying to cinder: "In my case the machine goes unresponsive as well, ..."]
If I recall correctly from the qemu&edk2 example you need to remap sections, I'll find it rq

[2025-08-16 22:11] Yoran: https://github.com/tianocore/tianocore.github.io/wiki/How-to-debug-OVMF-with-QEMU-using-GDB?utm_source=chatgpt.com
Yeah idk about that
[Embed: How to debug OVMF with QEMU using GDB]
Tianocore website. Contribute to tianocore/tianocore.github.io development by creating an account on GitHub.

[2025-08-16 22:11] cinder: [replying to Yoran: "Not debug, I mean DEBUG like logging efi_d_info"]
let me see if I can manage, I still consider myself a rookie when it comes to UEFI so I need to research a bit first

[2025-08-16 22:12] Yoran: [replying to cinder: "let me see if I can manage, I still consider mysel..."]
I would suggest btw not using EDK2

[2025-08-16 22:12] Yoran: All you need is the symbols

[2025-08-16 22:13] cinder: [replying to Yoran: "https://github.com/tianocore/tianocore.github.io/w..."]
oh it's there! i'll try it out in a second

[2025-08-16 22:13] Yoran: Build a small efi.h and call it a day

[2025-08-16 22:13] Yoran: I have horror stories from using EDK2

[2025-08-16 22:13] Yoran: It's a real bitch

[2025-08-16 22:13] cinder: yeah I am starting to notice, but at the same time I am getting the hang of it more and more

[2025-08-16 22:13] Yoran: [replying to Yoran: "Build a small efi.h and call it a day"]
It's also nice cause you can rename shit and all that

[2025-08-16 22:14] Yoran: SO NO NEED TO HAVE CODE THAT LOOKS LIKE THIS ALL OVER THE PLACE

[2025-08-16 22:14] Yoran: [replying to cinder: "yeah I am starting to notice, but at the same time..."]
Yeah but there's no need to

[2025-08-16 22:14] Yoran: The spec is dead easy to understand

[2025-08-16 22:14] Yoran: You just implement the symbols from there

[2025-08-16 22:14] Yoran: Have a nice macro for the GUIDs

[2025-08-16 22:15] Yoran: And you're good to go

[2025-08-16 22:15] Yoran: No fucking need to traverse trough EDK2 which is hugeee

[2025-08-16 22:16] cinder: EDK2 makes my day job of mantaining EDR drivers feel like a breeze

[2025-08-16 22:16] cinder: seriously it feels like it's the easiest thing ever after trying EDK2

[2025-08-16 22:16] cinder: (well not always)

[2025-08-16 22:17] Yoran: Until you need to remap sections if you want to debug something<:mmmm:904523247205351454>

[2025-08-16 22:18] Matti: edk2 does suck I agree, but I don't even have gnu-efi installed because I fear the GCC linker (and also lld to a lesser degree) a lot more than I do edk2

[2025-08-16 22:18] Matti: when it comes to PE binaries which is what the spec mandates

[2025-08-16 22:19] Matti: if you can recreate a simple test app like mine in gnu-efi quicker that might be faster to rule out one of the two

[2025-08-16 22:21] Matti: this does still leave the possibility of OVMF being at fault somehow though, I don't see a way around that because it's the entire FW for qemu

[2025-08-16 22:22] Yoran: [replying to Matti: "edk2 does suck I agree, but I don't even have gnu-..."]
I really really push for having a small efi.h file

[2025-08-16 22:22] Yoran: And you just implement on the go what you need

[2025-08-16 22:23] Yoran: It takes 3 mins per sub-section in the spec max

[2025-08-16 22:23] Yoran: Essentially copy paste and rename

[2025-08-16 22:38] cinder: [replying to Yoran: "Not debug, I mean DEBUG like logging efi_d_info"]
well this is interesting, unless I am doing something wrong on my end (which is highly probable) I don't see any debug traces in the serial log file.
I do this:
```
DEBUG((EFI_D_INFO, "EP=0x%x\r\n", UefiMain));
```
I've set `gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000040` under `[PcdsFixedAtBuild]` in my DSC file as per multiple mailing list entries, but I don't see it in my serial output.
```
[Bds]Booting UEFI QEMU DVD-ROM QM00005 
 BlockSize : 2048 
 LastBlock : 14D4 
PartitionDxe: El Torito standard found on handle 0x7E15F098.
FatDiskIo: Cache Page OutBound occurred! 
FSOpen: Open '\EFI\BOOT\BOOTX64.EFI' Success
[Bds] Expand PciRoot(0x0)/Pci(0x1F,0x2)/Sata(0x2,0xFFFF,0x0) -> PciRoot(0x0)/Pci(0x1F,0x2)/Sata(0x2,0xFFFF,0x0)/CDROM(0x0,0x22,0x5000)/\EFI\BOOT\BOOTX64.EFI
[Security] 3rd party image[0] can be loaded after EndOfDxe: PciRoot(0x0)/Pci(0x1F,0x2)/Sata(0x2,0xFFFF,0x0)/CDROM(0x0,0x22,0x5000)/\EFI\BOOT\BOOTX64.EFI.
None of Tcg2Protocol/CcMeasurementProtocol is installed.
InstallProtocolInterface: 5B1B31A1-9562-11D2-8E3F-00A0C969723B 7E71A040
Loading driver at 0x0007DB48000 EntryPoint=0x0007DB50160 InfernoBootloader.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 7E71AE98
ProtectUefiImageCommon - 0x7E71A040
  - 0x000000007DB48000 - 0x000000000000E180
ASSERT ZeroMemWrapper.c(47): Buffer != ((void *) 0)
```

[2025-08-16 22:38] Matti: try `-D DEBUG_ON_SERIAL_PORT` in your build invocation

[2025-08-16 22:40] Matti: I'm not sure off hand whether this is required for OVMF but 99% sure it's not if all you're interested in seeing is your own debug print statements

[2025-08-16 22:40] Matti: evidently OVMF is already successfully logging *some* stuff to serial so yeah

[2025-08-16 22:44] cinder: sadly no dice even by defining `DEBUG_ON_SERIAL_PORT`. I do log some stuff to `ConOut` (not the way I should be doing stuff as I should be using serial!), it is not great but I could send a screenshot there of the first few lines that display where UefiMain is in memory

[2025-08-16 22:45] cinder: not a great approach but I see UefiMain at 0x7DB504C0
[Attachments: image.png]

[2025-08-16 22:46] Matti: hmm one thing I missed, sorry - I used EFI_ERROR, not INFO

[2025-08-16 22:47] Matti: INFO statements might not be getting through depending on the PCD mask value

[2025-08-16 22:48] Matti: last possible difference I can think of: in my .dsc I've got this
```
[PcdsFixedAtBuild]
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x07
```

[2025-08-16 22:48] Matti: without comment

[2025-08-16 22:48] Matti: thanks past me

[2025-08-16 22:48] cinder: Same exact value on my hand, for reference I found the flags for PCD in a mailing list:
```
// Declare bits for PcdDebugPrintErrorLevel and the ErrorLevel parameter of 
DebugPrint()
//
#define DEBUG_INIT      0x00000001  // Initialization
#define DEBUG_WARN      0x00000002  // Warnings
#define DEBUG_LOAD      0x00000004  // Load events
#define DEBUG_FS        0x00000008  // EFI File system
#define DEBUG_POOL      0x00000010  // Alloc & Free's
#define DEBUG_PAGE      0x00000020  // Alloc & Free's
#define DEBUG_INFO      0x00000040  // Informational debug messages
#define DEBUG_DISPATCH  0x00000080  // PEI/DXE/SMM Dispatchers
#define DEBUG_VARIABLE  0x00000100  // Variable
#define DEBUG_BM        0x00000400  // Boot Manager
#define DEBUG_BLKIO     0x00001000  // BlkIo Driver
#define DEBUG_NET       0x00004000  // SNI Driver
#define DEBUG_UNDI      0x00010000  // UNDI Driver
#define DEBUG_LOADFILE  0x00020000  // UNDI Driver
#define DEBUG_EVENT     0x00080000  // Event messages
#define DEBUG_GCD       0x00100000  // Global Coherency Database changes
#define DEBUG_CACHE     0x00200000  // Memory range cachability changes
#define DEBUG_VERBOSE   0x00400000  // Detailed debug messages that may 
significantly impact boot performance
#define DEBUG_ERROR     0x80000000  // Error
```

[2025-08-16 22:49] Matti: yeah that looks about right, it should be in a comment in whatever .dec file in the MdePkg clusterfuck defines the PCD

[2025-08-16 22:51] Matti: ah wait, I'm confusing the literal defines used as the first arg to DEBUG() with the PCD mask values

[2025-08-16 22:51] Matti: [replying to Matti: "hmm one thing I missed, sorry - I used EFI_ERROR, ..."]
so these

[2025-08-16 22:52] Matti: correspond

[2025-08-16 22:52] Matti: I doubt I would have changed the debug property mask to anything that wouldn't at the very least also include error statements

[2025-08-16 22:53] Matti: let me look this up though

[2025-08-16 22:55] Matti: <https://github.com/tianocore/edk2/blob/master/MdePkg/MdePkg.dec#L2375>

[2025-08-16 22:56] Matti: you might wanna just set all of these to 1 if it's just for triaging this tbh

[2025-08-16 22:58] cinder: [replying to Matti: "<https://github.com/tianocore/edk2/blob/master/Mde..."]
I completely missed these, very interesting. I'll see if by editing some values I can get at least my own serial messages to go through

[2025-08-16 23:00] Matti: yeah so just to be clear because this is probably confusing (because it's edk2)
you want to set this mask value in your own application's .dsc file, not necessarily OVMF, that shouldn't be needed especially since you're already compiling it in debug mode

[2025-08-16 23:01] cinder: which I guess "overshadows" what is defined in MdePkg.dec

[2025-08-16 23:02] Matti: kinda, but it's worse

[2025-08-16 23:02] Matti: the file I linked is a **.dec** file which I guess stands for declare(???)

[2025-08-16 23:02] Matti: who the fuck knows

[2025-08-16 23:03] cinder: [replying to Matti: "who the fuck knows"]
how to describe EDK2 in a single sentence

[2025-08-16 23:03] Matti: but think of this as the header declaring the PCD as extern, in C terms

[2025-08-16 23:03] Matti: and your **.dsc** (did someone call? idk) provides the value, so a TU again in C terms

[2025-08-16 23:05] Matti: the reason for this system is that PCDs can be set to 'fixed at build', like my line above, but you can also use other sections that will make them runtime modifiable

[2025-08-16 23:06] Matti: fixed at build pretty much turns them into defines basically, and then in turn some `if ` branch might be optimized out to save a few bytes

[2025-08-16 23:07] Matti: this matters a lot for firmware so it makes sense, but it's also part of the many reasons why edk2 is such a massive clusterfuck if you just want to compile an app

[2025-08-16 23:18] cinder: well I guess that I'll need to revisit this issue with a fresh pair of eyes, i'll probably put a workaround in place to avoid asserting and I will test with vmware and virtualbox to see if I can get GDB to break on a CpuBreakpoint() call, it's getting very very late and tomorrow I have a bit of a drive to a touristy place, so I have to wake up early

[2025-08-16 23:20] cinder: I did notice multiple times that EDK2 is indeed a mess, but I think that eventually I'll understand how to move within it and configure it to my liking, as with all things it will take patience and a lot of practice but I feel like it is rewarding to try "taming the beast"

[2025-08-16 23:20] cinder: i am already beyond proud that I can compile things with EDK2, that alone took a week to get running

[2025-08-16 23:22] cinder: many many and even more many thanks <@148095953742725120> and <@317912264256520192> for your kind help

[2025-08-16 23:23] Yoran: [replying to cinder: "many many and even more many thanks <@148095953742..."]
For sure dude, anytime

[2025-08-16 23:23] Yoran: Lmk  how your project goes!

[2025-08-16 23:24] Matti: cool, and yeah feel free to ping if/when you find out more, would be interested to see

[2025-08-16 23:25] cinder: absolutely, I will update as I experiment more