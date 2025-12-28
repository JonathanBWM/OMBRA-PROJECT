# March 2024 - Week 5
# Channel: #reverse-engineering
# Messages: 321

[2024-03-25 02:13] 0xatul: just sse things

[2024-03-25 09:28] unknowntrojan: [replying to 0xatul: "```
 do
      {
        v61 = _mm_loadu_si128(v57 ..."]

[Attachments: A58F36D63077FC960A46C12DFAC8157A.mp4]

[2024-03-25 09:35] 0xatul: [replying to unknowntrojan: ""]
What color scheme is that?

[2024-03-25 09:37] unknowntrojan: catpuccin mocha

[2024-03-25 11:59] mrexodia: [replying to 0xatul: "What color scheme is that?"]
STOP

[2024-03-25 11:59] mrexodia: 
[Attachments: image.png]

[2024-03-25 11:59] 0xatul: [replying to mrexodia: "STOP"]
No u haha

[2024-03-25 11:59] 0xatul: Ok

[2024-03-25 12:00] 0xatul: On topic, microavx is nice

[2024-03-25 14:25] Mysterio: you don't just nope out when you see that much float?

[2024-03-25 21:02] dullard: [replying to unknowntrojan: ""]
https://tenor.com/view/joe-pesci-what-the-fuck-wtf-wtf-is-that-what-the-fuck-is-this-gif-24937450

[2024-03-29 21:47] irql: not done by the UEFI API, as soon as ExitBootServices is called, all the memory is "handed over" to the OS

[2024-03-29 21:47] irql: ExitBootServices is called inside `OslFwpKernelSetupPhase1` if you want to take a look

[2024-03-29 21:48] irql: I can't say how the kernel manages its memory in early boot, but the UEFI OS doesn't manage it as soon as ExitBootServices is called

[2024-03-29 21:49] irql: it's likely ntoskrnl is called with all of this memory marked as some kind of "boot loader reclaimable"

[2024-03-29 21:50] irql: [replying to irql: "it's likely ntoskrnl is called with all of this me..."]
well, as you probably know ExitBootServices takes a memory map key, and that's after UEFI has handed it over anyways

[2024-03-29 21:51] irql: if the kernel uses that memory it'll be marked as BootLoaderCode or something similar

[2024-03-29 21:51] irql: but yea, not sure how the kernel uses that memory map exactly

[2024-03-29 22:03] irql: all good

[2024-03-29 22:03] irql: [replying to irql: "ExitBootServices is called inside `OslFwpKernelSet..."]
I mean, yea, this function pulls the UEFI memory map and exit's boot services

[2024-03-29 22:04] irql: then calls the kernel a little later

[2024-03-29 22:04] irql: but yea, there will probably be another memory map to keep track of what's discardable and stuff

[2024-03-29 22:05] irql: someone probably knows better than me about this stuff tbf

[2024-03-29 22:05] irql: might be worth looking at the LOADER_PARAMETER_BLOCK or something

[2024-03-29 22:15] irql: you can install a callback on ExitBootServices tbf, but not sure how that'll go, because of the memory map key

[2024-03-29 22:15] irql: if you're trying to hide memory for funny purposes, you can vtable swap the GetMemoryMap function, or just allocate the memory as a reserved type :^)

[2024-03-29 22:15] irql: <:Troll:881936864314023967>

[2024-03-29 22:16] irql: theres probably some kind of callback you can install if its for legit purposes

[2024-03-29 22:16] irql: UEFI has a bunch of them

[2024-03-29 22:16] irql: "event callbacks" or something

[2024-03-29 22:18] irql: there's also memory types that will be freed by the kernel

[2024-03-29 22:18] irql: once UEFI is loaded

[2024-03-29 22:18] irql: if you mean like that ?

[2024-03-29 22:18] irql: BootServicesData for example

[2024-03-29 22:19] irql: you dont need to free it, the kernel will discard it after loading

[2024-03-29 22:21] irql: UEFI spec is pretty rough, yea

[2024-03-29 22:21] irql: BootServicesData should work though yea, the kernel will reuse that after ExitBootServices, anyways

[2024-03-29 23:20] asz: ```c
__int64 __fastcall HvlLaunchHypervisor(void (*a1)(void), __int64 a2)
{
  __int64 v2; // r8
  __int64 result; // rax
  unsigned __int64 v6; // rax
  unsigned __int64 v7; // rdx
  unsigned __int64 v8; // rax
  char v9; // di
  _QWORD *v10; // r8
  unsigned __int64 v11; // rax
  unsigned __int64 v12; // rax
  unsigned __int64 v13; // rax
  __int64 v14; // rbp
  unsigned __int64 v15; // r14
  unsigned __int64 v16; // rax
  char v17; // r8
  __int16 v18; // [rsp+30h] [rbp-8h]

  v2 = HvlpLoaderBlock;
  if ( (*(_DWORD *)(HvlpLoaderBlock + 280) & 0x20000000) != 0 )
  {
    LODWORD(result) = dword_0_180231558;
    if ( dword_0_180231558 == -1073741267 )
    {
      if ( qword_0_180231560 == -1 )
      {
        qword_0_180231560 = *(_QWORD *)(HvlpLoaderBlock + 24);
        BtUpdateHypervisorPageTableRoot(HvlpLoaderBlock);
        v2 = HvlpLoaderBlock;
      }
      LODWORD(result) = BalDisableHypervisor(a1, a2, *(_QWORD *)(v2 + 16));
      dword_0_180231558 = result;
      if ( !(_DWORD)result )
      {
        HvlpDestroySubsumedContext();
        LODWORD(result) = dword_0_180231558;
      }
    }
    result = (int)result;
    *(_QWORD *)(a2 + 16) = (int)result;
  }
  else
  {
    HvlpSavedContext = HvlpReadCs();
    __sgdt(byte_0_18028580E);
    __sidt(byte_0_18028581E);
    qword_0_180285828 = __readmsr(0xC0000100);
    qword_0_180285830 = __readmsr(0xC0000101);
    v6 = __readmsr(0xC0000102);
    v7 = (unsigned __int64)HIDWORD(v6) << 32;
    qword_0_180285838 = v6;
    v8 = __readcr4();
    if ( (v8 & 0x40000) != 0 )
    {
      __asm { xgetbv }
      qword_0_180285840 = v8 | (v7 << 32);
    }
    *(_QWORD *)(a2 + 8) = *(_QWORD *)(HvlpLoaderBlock + 312);
    *(_DWORD *)a2 = *(_DWORD *)(HvlpLoaderBlock + 304);
    *(_DWORD *)(HvlpLoaderBlock + 300) = 48;
    HvlpBspContext = HvlpLoaderBlock + *(unsigned int *)(HvlpLoaderBlock + 292);
    *(_DWORD *)(HvlpLoaderBlock + 328) = (unsigned int)HvlpLowMemoryStub - (unsigned int)&HvlpLowMemoryStubEnd + 4096;
    memmove(
      HvlpBelow1MbPage + (unsigned int)HvlpLowMemoryStub - (unsigned int)&HvlpLowMemoryStubEnd + 4096,
      HvlpLowMemoryStub,
      &HvlpLowMemoryStubEnd - (char *)HvlpLowMemoryStub);
    v9 = 0;
    if ( (v18 & 0x200) != 0 )
    {
      v9 = 1;
      _disable();
    }
    *(_QWORD *)HvlpBspContext = &HvlpReturnFromHypervisor;
    v10 = (_QWORD *)HvlpBspContext;
    v11 = __readcr0();
    v10[1] = v11;
    v12 = __readcr3();
    v10[2] = v12;
    v13 = __readcr4();
    v10[3] = v13;
    v10[4] = __readmsr(0xC0000080);
    v10[5] = __readmsr(0x277u);
    BlBdStop(631i64);
    v14 = HvlpBelow1MbPage + *(unsigned int *)(HvlpLoaderBlock + 328);
    v15 = *(_QWORD *)(HvlpBspContext + 16);
    if ( a1 )
      a1();
    v16 = __readcr4();
    __writecr4(v16 & 0xFFFFFFFFFFFFFF7Fui64);
    HvlpTransferToHypervisorViaTransitionSpace(
      *(_QWORD *)(HvlpLoaderBlock + 24),
      *(_QWORD *)(HvlpLoaderBlock + 16),
      v14,
      v15);
    result = HvlpRestoreProcessorState();
    v17 = HvlpHypervisorLaunchSucceeded;
    if ( HvlpHypervisorLaunchSucceeded )
      BlpEnvironmentState |= 4u;
    if ( (qword_0_18026C878 & 0x4000) == 0
      || BdDebugDevice
      && HvDebugDevice
      && (result = *(unsigned __int16 *)(BdDebugDevice + 8), *(_WORD *)(HvDebugDevice + 8) == (_WORD)result)
      && (result = *(unsigned int *)(BdDebugDevice + 4), *(_DWORD *)(HvDebugDevice + 4) == (_DWORD)result) )
    {
      result = BlBdStart(HvDebugDevice, BdDebugDevice);
      v17 = HvlpHypervisorLaunchSucceeded;
    }
    if ( v17 )
    {
      *(_QWORD *)(a2 + 16) = 0i64;
    }
    else
    {
      *(_QWORD *)(a2 + 16) = 35i64;
      result = HvlpHypervisorLaunchPhase;
      *(_QWORD *)(a2 + 24) = HvlpHypervisorLaunchPhase;
    }
    if ( v9 )
      _enable();
  }
  return result;
}```

[2024-03-29 23:27] asz: 
[Attachments: image.png]

[2024-03-29 23:27] asz: from linux hyperv loader

[2024-03-29 23:28] asz: had it open so took a look

[2024-03-29 23:29] asz: seems to use

[2024-03-29 23:29] asz: 
[Attachments: image.png]

[2024-03-30 00:05] dullard: гипервизора

[2024-03-30 02:41] Matti: [replying to irql: "but yea, there will probably be another memory map..."]
yeah, the kernel takes the loader-assigned memory types (`MEMORY_TYPE` / `TYPE_OF_MEMORY`) into account when deciding which pages can be reclaimed

[2024-03-30 02:41] Matti: but it is much more conservative than you probably think

[2024-03-30 02:42] Matti: LoaderData for example is not always safe to reuse at all, because boot loaded drivers may have references to these addresses (and these drivers themselves are also part of boot loaded memory that can only be reclaimed later after the boot drivers have been relocated)

[2024-03-30 02:44] Matti: another factor to take into account is simply buggy UEFI BIOS implementations that break when you do something like this, because they allocated the wrong memory type

[2024-03-30 02:45] Matti: the only exception the kernel makes for certain is for PAs < 1M (the low mem stub), because this is needed for the AP startup code

[2024-03-30 02:51] Matti: I ran into this same issue when porting matti WRK to boot in UEFI mode, after a lot of trial and error I came up with the following check (abbreviated for clarity)
```c
VOID
ExpReclaimFirmwareBootPages(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
{
    PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor;
    PLIST_ENTRY PrevMd = LoaderBlock->MemoryDescriptorListHead.Blink;

    while (PrevMd != &LoaderBlock->MemoryDescriptorListHead) {
        MemoryDescriptor = CONTAINING_RECORD(PrevMd,
                              MEMORY_ALLOCATION_DESCRIPTOR,
                              ListEntry);

        if ((MemoryDescriptor->BasePage <= 255 &&
            (MemoryDescriptor->MemoryType != LoaderFree &&
            MemoryDescriptor->MemoryType != LoaderFirmwareTemporary &&
            MemoryDescriptor->MemoryType != LoaderFirmwarePermanent &&
            MemoryDescriptor->MemoryType != LoaderMemoryData))) {

            // Mark PTE as invalid so the HAL can use it for the AP startup asm stub
        }

        if (NumPagesReclaimed >= NumPagesToReclaim) {
            break;
        }

        PrevMd = MemoryDescriptor->ListEntry.Blink;
    }
}
```

[2024-03-30 02:52] Matti: [replying to Matti: "LoaderData for example is not always safe to reuse..."]
so looking at this code I must've meant LoaderMemoryData - LoaderData seems to be OK to reclaim in the above (so long as it is < 1M)

[2024-03-30 02:55] Matti: I thought this was a gross hack, but when describing it to someone from MS who works on this stuff she told me, 'yeah that sounds about what I would've done'

[2024-03-30 02:55] Matti: so 🤷

[2024-03-30 02:57] Matti: win 11 definitely does a lot more than this nowadays though, see `MiRemoveLargeFreeLoaderDescriptors` which is later followed by `MxInitializeFreeNodeDescriptors` -> `MiInitializeBootMemoryDescriptor`

[2024-03-30 02:58] Matti: but win 11 also needs even more low memory because of potential pages needed for hypervisor AP startup

[2024-03-30 03:06] Matti: oh yeah, speaking of workarounds for bugs... sometimes it's not even a buggy BIOS that MS need to work around, but their own broken releases

[2024-03-30 03:08] Matti: see e.g. this comment in `BmFwMemoryInitialize` (in the boot manager, not kernel)
```c
//
// The HAL requires 1 page below 1MB to setup the APs. This is allocated
// from kernel phase 0 reclaimable memory - regions the OS loader marks as
// free or firmware temporary. On machines where there are no EFI boot
// services code or data pages (i.e. firmware temporary pages) in the first
// 1MB, the Vista and Windows 7 HAL was relying on the fact that the
// Vista/Windows 7 boot manager had happened to allocate its heap in the
// first 1MB of memory. The boot manager's heap is marked as firmware
// temporary memory by the OS loader and reclaimed by the HAL. This class of
// EFI machines were successfully booting by a stroke of good luck.
//
// Post Vista/Windows 7, boot library allocation policy changed for EFI
// systems, disallowing normal allocations to use memory below 1MB. The
// purpose of this change was to make it easier for the hypervisor loader
// to find a page below 1MB to reserve for the hypervisor, which requires
// said page to bring APs online.
//
// As this change caused the boot manager to make no reclaimable heap
// allocations in the first 1MB, it broke the previously discussed class of
// EFI machines when attempting to boot Vista and Windows 7. The Vista and
// Windows 7 OS loaders, without the new allocation policy, fill the first
// 1MB with un-reclaimable allocations, leaving no memory for the HAL to
// usein kernel phase 0. This causes the Vista and Windows 7 HALs to
// bugcheck.
//
// The fix is to allocate a single page below 1MB that a Vista or Windows 7
// OS loader will not reclaim, but that a Vista or Windows 7 HAL can
// reclaim. The memory type for this allocation should have a subtype of
// MEMORY_APPLICATION_TYPE_UNAVAILABLE.
//
// This is a hack. It can only be removed once Vista and Windows 7 have been
// deprecated.
//

Attributes = MEMORY_ATTRIBUTE_ALLOCATION_REALMODE_ACCESSIBLE;
INITIALIZE_MEMORY_ADDRESS_RANGE(&AddressRange, 0, _1MB);
RangeFlags = MEMORY_RANGE_PREFER_LOW_ADDRESSES;
Status = BlMmAllocatePhysicalPagesInRange(&PhysicalAddress,
                    1,
                    MEMORY_TYPE_APPLICATION_UNAVAILABLE,
                    Attributes,
                    0,
                    &AddressRange,
                    RangeFlags);
```

[2024-03-30 07:42] RAJ: Hii  I have an app Android app

[2024-03-30 07:42] RAJ: They said it can't be hacked

[2024-03-30 07:43] RAJ: I want some some one who can hack it

[2024-03-30 07:44] RAJ: Here is apk
[Attachments: 91CLUB.apk]

[2024-03-30 07:46] froj: <:KEK:661789881223348224>

[2024-03-30 14:44] dullard: [replying to RAJ: "I want some some one who can hack it"]
done

[2024-03-30 16:16] irql: [replying to Matti: "the only exception the kernel makes for certain is..."]
yea this is all kinda fucked -- i was looking at these stubs the other day

[2024-03-30 16:16] irql: seems to always use 0x1000

[2024-03-30 16:16] irql: I always just overwrite whatever's at 0x1000, and hope for the best

[2024-03-30 16:17] irql: usually anything below 1MB is bios data area stuff anyways

[2024-03-30 16:20] Matti: yeah it needs 1 page to be precise

[2024-03-30 16:20] Matti: not exactly sure for the hyper-v case, it might be 2 pages, might be more

[2024-03-30 16:22] Matti: without this hack I could only boot in UEFI mode successfully with `/ONECPU` lol

[2024-03-30 16:25] Matti: [replying to RAJ: "They said it can't be hacked"]
no this is a common misconception about android

[2024-03-30 16:25] Matti: the tool you need to hack this is 7zip

[2024-03-30 16:25] Matti: it will allow you to extract and then hack the files inside

[2024-03-30 16:25] Matti: I trust that part won't be an issue

[2024-03-30 19:38] RAJ: <@148095953742725120> okay

[2024-03-30 20:02] irql: [replying to Matti: "yeah it needs 1 page to be precise"]
shit is massive for a stub ???

[2024-03-30 20:03] Matti: well it's about the PTE for this region

[2024-03-30 20:03] Matti: so it doesn't come in smaller sizes

[2024-03-30 20:03] irql: lmfao

[2024-03-30 20:03] irql: yea i guess so

[2024-03-30 20:04] irql: meant the code itself like

[2024-03-30 20:04] irql: and the huge buffer

[2024-03-30 20:04] irql: guess its not that crazy

[2024-03-30 20:04] irql: it is position independent though <:nigsmile:873355139354886144>

[2024-03-30 20:06] Matti: I think the craziest part about the stub is that it builds and links

[2024-03-30 20:07] Matti: given that it starts in real mode, goes to 32 bit protected (but with paging disabled IIRC?), and then long mode

[2024-03-30 20:07] Matti: the build system they have in place to make this a single binary is gruesome

[2024-03-30 20:08] irql: lmfao

[2024-03-30 20:08] irql: I mean they do split it up

[2024-03-30 20:08] Matti: or well I'm talking about what they used for NT5 to be fair, but from what I've heard this is still pretty much the same

[2024-03-30 20:08] irql: although NASM can actually do it ?

[2024-03-30 20:08] irql: [replying to Matti: "or well I'm talking about what they used for NT5 t..."]
yea probs

[2024-03-30 20:08] Matti: yeah but do you think they use nasm lol

[2024-03-30 20:09] irql: yea of course not 🤣

[2024-03-30 20:09] irql: can't imagine what they did for it

[2024-03-30 20:09] Matti: sec, let me see if I can find it

[2024-03-30 20:09] Matti: though I don't wanna

[2024-03-30 20:09] irql: https://media.discordapp.net/attachments/967469505313910886/1223402797463240734/image.png?ex=6619b9a4&is=660744a4&hm=df253cd5acc2fc2730accd5916a8bd918961255aae1203ba0fb83f6ed3e04a8c&=&format=webp&quality=lossless&width=836&height=676

[2024-03-30 20:09] irql: PMStub without the erm

[2024-03-30 20:09] irql: jmp

[2024-03-30 20:10] irql: over the data region

[2024-03-30 20:10] irql: forgot the base of it, but it is pos independent

[2024-03-30 20:10] irql: https://cdn.discordapp.com/attachments/967469505313910886/1223402855227064402/image.png?ex=6619b9b2&is=660744b2&hm=d5fb8edb3df9441a2341eb2ce57d53a6900248addc1d73787507552a2463f1dc&

[2024-03-30 20:10] irql: RMStub

[2024-03-30 20:10] Matti: yep yep

[2024-03-30 20:10] Matti: those are the ones

[2024-03-30 20:10] irql: sorry, RMStub is missing a jump with around 0x600 bytes of data

[2024-03-30 20:10] irql: PMstub based at 0x67C

[2024-03-30 20:10] irql: yea

[2024-03-30 20:10] irql: was taking a look yesterday

[2024-03-30 20:11] irql: I had a typo in my HV, and had some mess up with the EFER

[2024-03-30 20:11] irql: fun debugging

[2024-03-30 20:13] Matti: ```asm
;++
;
; Module Name:
;
;    xmstub.asm
;
; Abstract:
;
;    This module implements the code that starts secondary processors. This
;    module is unique in that it is assembled by the i386 32-bit assembler,
;    because the Amd64 assembler does not assemble 16- or 32-bit x86 code.
;
;    The .obj file that is the result of assembling this module is fed
;    through a tool, DMPOBJ.EXE, that stores the contents of the relevant
;    section and generates a c file (startup.c) that can be included in the
;    64-bit compilation process.
;
; Author:
;
;    Forrest Foltz (forrestf) March 6, 2001
;
; Environment:
;
;    Kernel mode only.
;--
```

[2024-03-30 20:13] Matti: dmpobj.exe is itself 4KB lmao

[2024-03-30 20:13] Matti: what a joker

[2024-03-30 20:14] Matti: anyway I wrote a batch file to invoke the different tools involved in this mess

[2024-03-30 20:15] Matti: when I added the HAL to matti WRK

[2024-03-30 20:15] Matti: cause I use msbuild, not MS' insane razzle build system

[2024-03-30 20:15] Matti: msbuild is positively sane in comparison

[2024-03-30 20:16] irql: lmfao

[2024-03-30 20:22] Matti: ok this is the layout of all the different stubs for NT 5.2 amd64
[Attachments: image.png]

[2024-03-30 20:23] Matti: in the end these all end up in the same .obj file via some route or other

[2024-03-30 20:24] Matti: and because it's PIC like you said, you can then copy the bytes to pretty much anywhere

[2024-03-30 20:25] Matti: ...so long as it is < 1M

[2024-03-30 20:27] Matti: some bytes do need to be modified I see btw, like the reset vector location and the linear address of the block itself (cause the AP needs to know where it is)

[2024-03-30 20:28] Matti: but other than a few things like this HalStartNextProcessor is basically just a really convoluted memcpy

[2024-03-30 20:28] Matti: or it was, I'm sure it's very different now

[2024-03-30 20:28] Matti: but in essence the same

[2024-03-30 21:32] irql: yea lmfao

[2024-03-30 21:32] irql: most convoluted memcpy

[2024-03-30 21:32] irql: the code itself seems to be position independent actually

[2024-03-30 21:33] irql: I guess there is a giant structure on there too

[2024-03-30 21:33] irql: but the start of the RM stub, i thought it was kinda cool

[2024-03-30 21:33] irql: ```x86
mov ax, cs
shl ax, 4
```

[2024-03-30 21:33] irql: <:nerd2:1085056953170014218>

[2024-03-30 21:33] irql: completely unnecessary but fair play tbh

[2024-03-30 21:45] Matti: yeah the code is position independent, as in it doesn't require any relocations

[2024-03-30 21:46] Matti: but HalStartNextProcessor does fill in some bits and bobs the code needs to reference at certain places, before starting the AP

[2024-03-30 21:47] Matti: I think it's a pretty well thought out design, given that you've got 4KB of code (relatively speaking quite a lot), only you have no guarantees about where it will be, other than below 1M which isn't much to go on

[2024-03-30 21:49] Matti: here's the start of HalpRMStub after conversion to C btw <:kekw:904522300257345566>
[Attachments: image.png]

[2024-03-30 21:51] Matti: so after the jmp it's all zeroes until this bit at the end
[Attachments: image.png]

[2024-03-30 21:52] Matti: it says
> source: xmstub.obj
but that is of course a totally useless comment

[2024-03-30 21:52] Matti: that obj file is itself generated by an assembler

[2024-03-30 21:53] Matti: in my case I managed to get it to work with current masm, but this must still be 32 bit masm

[2024-03-30 21:56] Matti: the reason for the gap is because they put the processor start block after the jmp
[Attachments: image.png]

[2024-03-30 21:57] Matti: and this is a huge struct as you can see

[2024-03-30 21:57] irql: [replying to Matti: "so after the jmp it's all zeroes until this bit at..."]
yea lmfao that's the bit I cut out

[2024-03-30 21:58] irql: [replying to Matti: "the reason for the gap is because they put the pro..."]
yea, very weird tbh lmfao

[2024-03-30 21:58] irql: absolutely massive, considering all it needs to do is load a few registers

[2024-03-30 21:59] irql: [replying to Matti: "the reason for the gap is because they put the pro..."]
lmfao

[2024-03-30 21:59] Matti: yeah not sure where all of that is going, I haven't looked at this code in quite some time

[2024-03-30 21:59] irql: db 66h 🦾 🦾

[2024-03-30 21:59] irql: yea, probably never being touched again

[2024-03-30 21:59] irql: serves its purpose

[2024-03-30 22:01] Matti: yeah so basically the bulk seems to come from a single field which is the processor state

[2024-03-30 22:01] Matti: type
```c
typedef struct _KPROCESSOR_STATE
{
    /* 0x0000 */ struct _KSPECIAL_REGISTERS SpecialRegisters;
    /* 0x00f0 */ struct _CONTEXT ContextFrame;
} KPROCESSOR_STATE, *PKPROCESSOR_STATE; /* size: 0x05c0 */
```

[2024-03-30 22:01] irql: yea

[2024-03-30 22:01] Matti: so yeah, kind of overkill I guess you could say

[2024-03-30 22:01] irql: KSPECIAL_REGISTERS <:nerd2:1085056953170014218>

[2024-03-30 22:02] irql: pretty overkill yea defo

[2024-03-30 22:02] Matti: but, there might be a good reason they're doing it this way that I'm not aware of

[2024-03-30 22:02] Matti: I hardly ever look at HAL code

[2024-03-30 22:02] Matti: it makes me nauseous

[2024-03-30 22:03] irql: I also start vomiting when I read it

[2024-03-30 22:03] Matti: especially ACPI HAL which is the only kind of HAL

[2024-03-30 22:03] Matti: <:acpica:1066830025128681512>

[2024-03-30 22:03] irql: yea lmfao

[2024-03-30 22:03] irql: very gross

[2024-03-30 22:05] Matti: KSPECIAL_REGISTERS seems to be 100% made for aiding debugging

[2024-03-30 22:06] Matti: ```c
//
// Processor State frame: Before a processor freezes itself, it
// dumps the processor state to the processor state frame for
// debugger to examine.
//

typedef struct _KPROCESSOR_STATE {
    struct _CONTEXT ContextFrame;
    struct _KSPECIAL_REGISTERS SpecialRegisters;
} KPROCESSOR_STATE, *PKPROCESSOR_STATE;
```

[2024-03-30 22:06] Matti: fair enough IMO

[2024-03-30 22:06] irql: oh yea fair play

[2024-03-30 22:07] Matti: totally unrelated to this, but seeing the word CONTEXT.... I just remembered something

[2024-03-30 22:08] Matti: did you know there is an EBC (as in the UEFI target independent bytecode 'cpu') CONTEXT in NT?

[2024-03-30 22:08] Matti: so EBC_CONTEXT

[2024-03-30 22:08] Matti: and quite a few others which surprised me since I found them in a windows 11 PDB

[2024-03-30 22:08] irql: lmfao

[2024-03-30 22:09] irql: really wonder what they're using that for

[2024-03-30 22:09] irql: kinda strange for UEFI to even have it

[2024-03-30 22:09] irql: i think it was designed for like, what, architecture independent code?

[2024-03-30 22:09] Matti: yep

[2024-03-30 22:09] irql: but you're writing firmware... ?

[2024-03-30 22:09] Matti: no it was meant for OPROMs I think

[2024-03-30 22:09] irql: ahhhhh

[2024-03-30 22:09] irql: that actually makes sense

[2024-03-30 22:09] Matti: so the idea was that you'd have a PCIe card that could work in say, both x86 and ARM PCs

[2024-03-30 22:09] irql: I guess ACPI is somewhat like that

[2024-03-30 22:10] irql: yeahhh

[2024-03-30 22:10] Matti: <:harold:704245193016344596> yeah

[2024-03-30 22:10] irql: fair play

[2024-03-30 22:10] Matti: ok so here's all the contexts they keep around for ??reasons??

```c
typedef struct _CROSS_PLATFORM_CONTEXT
{
    union
    {
        /* 0x0000 */ struct _X86_CONTEXT X86Context;
        /* 0x0000 */ struct _X86_NT5_CONTEXT X86Nt5Context;
        /* 0x0000 */ struct _ALPHA_CONTEXT AlphaContext;
        /* 0x0000 */ struct _ALPHA_NT5_CONTEXT AlphaNt5Context;
        /* 0x0000 */ struct _IA64_CONTEXT IA64Context;
        /* 0x0000 */ struct _AMD64_CONTEXT Amd64Context;
        /* 0x0000 */ struct _AMD64_OBSOLETE_CONTEXT_1 Amd64ObsContext1;
        /* 0x0000 */ struct _AMD64_OBSOLETE_CONTEXT_2 Amd64ObsContext2;
        /* 0x0000 */ struct _ARM_CONTEXT ArmContext;
        /* 0x0000 */ struct _ARM64_CONTEXT Arm64Context;
        /* 0x0000 */ struct _ARMCE_CONTEXT ArmCeContext;
        /* 0x0000 */ struct _EBC_CONTEXT EbcContext;
        /* 0x0000 */ struct _PPC_CONTEXT PpcContext;
        /* 0x0000 */ struct _MIPS32_CONTEXT Mips32Context;
        /* 0x0000 */ struct _MIPS64_CONTEXT Mips64Context;
        struct
        {
            /* 0x0000 */ struct _SH_CONTEXT ShContext;
            /* 0x00e4 */ long __PADDING__[611];
        }; /* size: 0x0a70 */
    }; /* size: 0x0a70 */
} CROSS_PLATFORM_CONTEXT, *PCROSS_PLATFORM_CONTEXT; /* size: 0x0a70 */
```

[2024-03-30 22:10] irql: ohhh yea, i remember seeing a lot of this in windbg code

[2024-03-30 22:11] irql: NT really could run on anything

[2024-03-30 22:11] Matti: SH_CONTEXT is for SuperH by the way

[2024-03-30 22:11] Matti: a CPU I'd never heard of before

[2024-03-30 22:11] irql: oh wtf

[2024-03-30 22:12] irql: yea, neither

[2024-03-30 22:12] Matti: it has by far the smallest context

[2024-03-30 22:12] Matti: ```c
typedef struct _SH_CONTEXT
{
    /* 0x0000 */ unsigned long ContextFlags;
    /* 0x0004 */ unsigned long PR;
    /* 0x0008 */ unsigned long MACH;
    /* 0x000c */ unsigned long MACL;
    /* 0x0010 */ unsigned long GBR;
    /* 0x0014 */ unsigned long R0;
// snip..
    /* 0x0050 */ unsigned long R15;
    /* 0x0054 */ unsigned long Fir;
    /* 0x0058 */ unsigned long Psr;
    /* 0x005c */ unsigned long Fpscr;
    /* 0x0060 */ unsigned long Fpul;
    /* 0x0064 */ unsigned long FRegs[16];
    /* 0x00a4 */ unsigned long xFRegs[16];
} SH_CONTEXT, *PSH_CONTEXT; /* size: 0x00e4 */
```

[2024-03-30 22:13] irql: 🤨

[2024-03-30 22:13] irql: more impressed it runs on all this stuff

[2024-03-30 22:13] irql: somewhat

[2024-03-30 22:13] Matti: well that's not proven tbh

[2024-03-30 22:14] Matti: I mean I know NT is portable but I highly doubt there is an EBC version of NT in a lab somewhere

[2024-03-30 22:14] irql: 🤣

[2024-03-30 22:14] irql: well, excluding EBC lmfao

[2024-03-30 22:14] Matti: mm yeah but the superH one I also doubt

[2024-03-30 22:14] irql: yea idk about that one

[2024-03-30 22:14] Matti: that's actually the one I have no explanation whatsoever for

[2024-03-30 22:15] irql: I remember seeing a big list in some DBGKD headers

[2024-03-30 22:15] irql: was pretty surprised

[2024-03-30 22:15] irql: yea idk what that is either lmfao

[2024-03-30 22:15] Matti: EBC I could imagine needing somehow during the boot stages

[2024-03-30 22:15] Matti: maaaybe

[2024-03-30 22:15] Matti: but superH? what

[2024-03-30 22:16] Matti: hmm apparently it did exist as part of the dreamcast

[2024-03-30 22:17] Matti: so it's not a literal garbage CPU

[2024-03-30 22:17] Matti: the SH4 that is

[2024-03-30 22:18] irql: oh hmm

[2024-03-30 22:18] Matti: still, I feel like if there was an SH4 port of NT, I would have heard of it before

[2024-03-30 22:18] irql: oh yea hm

[2024-03-30 22:18] irql: yea wow

[2024-03-30 22:18] Matti: or it would have simply been in any leak ever

[2024-03-30 22:18] Matti: MS have had plenty of leaks

[2024-03-30 22:19] irql: apparently the dreamcast used to run Windows CE?

[2024-03-30 22:19] Matti: oh wow really

[2024-03-30 22:19] irql: apparently so lmfao

[2024-03-30 22:19] irql: wow

[2024-03-30 22:20] Matti: yeah

[2024-03-30 22:20] Matti: > Developers were able to include a custom version of the Windows CE operating system on game discs to make porting PC games easy, [...]

[2024-03-30 22:20] Matti: crazy

[2024-03-30 22:20] irql: lmfao

[2024-03-30 22:20] irql: wow

[2024-03-30 22:20] Matti: and it does list
> Hitachi SH-4 @ 200 MHz
as the CPU

[2024-03-30 22:21] Matti: well kept secret that

[2024-03-30 22:21] Matti: though, idk if CE counts as NT

[2024-03-30 22:21] irql: yea, idk enough about CE tbh

[2024-03-30 22:21] Matti: same

[2024-03-31 01:14] irql: anyone know what parameter 1's "4" is? https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/bug-check-0x139--kernel-security-check-failure
[Embed: Bug Check 0x139 KERNEL_SECURITY_CHECK_FAILURE - Windows drivers]
The KERNEL_SECURITY_CHECK_FAILURE bug check has a value of 0x00000139. This bug check indicates that the kernel has detected the corruption of a critical data structure.

[2024-03-31 01:14] irql: had it a few times, but its documented as "reserved"

[2024-03-31 01:14] irql: patchguard (?)

[2024-03-31 01:37] daax: [replying to irql: "had it a few times, but its documented as "reserve..."]
pretty sure it’s always 0

[2024-03-31 01:38] daax: and is probably reserved for something later, ive not seen it used for these types, even when it complained of critical structure corruption with 139

[2024-03-31 01:38] daax: if it isn’t 0 for yours that would be interesting

[2024-03-31 01:39] irql: always 0 for patchguard? hm

[2024-03-31 01:39] irql: yea, idk why its flagging 4

[2024-03-31 01:40] irql: im assuming patchguard

[2024-03-31 01:41] daax: [replying to irql: "always 0 for patchguard? hm"]
no, its just always 0

[2024-03-31 01:41] daax: because it’s reserved

[2024-03-31 01:41] daax: [replying to irql: "yea, idk why its flagging 4"]
maybe something messed up registers

[2024-03-31 01:42] daax: i’ve not seen 4 there before <:Thonk:614926448700162084>

[2024-03-31 01:42] daax: what were you breaking?

[2024-03-31 01:42] irql: ohhh sorry

[2024-03-31 01:42] irql: I mean

[2024-03-31 01:42] irql: parameter 1

[2024-03-31 01:42] irql: the value was 4

[2024-03-31 01:42] irql: `KeBugCheckEx 00000139 0000000000000004 FFFFA7014A3E7010 FFFFA7014A3E6F68 0000000000000000`

[2024-03-31 01:43] irql: I guess it could've been messed up registers, but not sure

[2024-03-31 01:43] irql: doubtful

[2024-03-31 01:43] irql: I was messing around, set the DPL for all of usermode to 0

[2024-03-31 01:43] irql: with a HV using the access rights field

[2024-03-31 01:43] irql: dont ask why lmfao

[2024-03-31 01:44] daax: param 1 being 4 is a threads stack ptr being fkd

[2024-03-31 01:44] irql: ohh ?

[2024-03-31 01:44] irql: weird they dont document that

[2024-03-31 01:44] irql: really annoying

[2024-03-31 01:45] daax: Yeah that’s annoying. I thought you were saying parameter 4 like arg 4 for a second I was gonna be like wat

[2024-03-31 01:46] daax: but ye check threads

[2024-03-31 01:46] daax: [replying to irql: "I was messing around, set the DPL for all of userm..."]
oh

[2024-03-31 01:46] daax: yeah that’ll do it

[2024-03-31 01:46] irql: oh yea, it would

[2024-03-31 01:46] irql: i've had it a few times, and my friend too

[2024-03-31 01:46] irql: idk why they wouldn't just document it

[2024-03-31 01:47] irql: [replying to daax: "Yeah that’s annoying. I thought you were saying pa..."]
😞

[2024-03-31 01:47] daax: <:PES2_Shrug:513352546341879808> microsoft tings

[2024-03-31 01:47] irql: thank you

[2024-03-31 02:16] Matti: [replying to irql: "weird they dont document that"]
they do, just not always publicly as per usual with MS....

[2024-03-31 02:16] Matti: this has param 4's meaning for me
[Attachments: bugcodes.txt]

[2024-03-31 02:16] Matti: > 4  : The thread's stack pointer was outside the legal stack
>      extents for the thread.

[2024-03-31 02:17] Matti: not sure why this one alone is 'reserved' on MSDN

[2024-03-31 02:18] Matti: you can see the txt file is a few years old by now though

[2024-03-31 02:18] Matti: it only goes up to P1 = 29

[2024-03-31 02:19] Matti: whereas MSDN goes up to P1 = 39

[2024-03-31 02:19] Matti: oh they did also skip from 29 to 37 >.<

[2024-03-31 02:20] Matti: I'm guessing those are more 'reserved's

[2024-03-31 03:51] Matti: oh yeah.... totally forgot that these codes are also in `wdm.h`! just grep for `RtlFailFast` and scroll up a bit

[2024-03-31 03:51] Matti: [replying to Matti: "oh they did also skip from 29 to 37 >.<"]
so these are actually
```c
#define FAST_FAIL_INVALID_NEXT_THREAD               30
#define FAST_FAIL_GUARD_ICALL_CHECK_SUPPRESSED      31         // Telemetry, nonfatal
#define FAST_FAIL_APCS_DISABLED                     32
#define FAST_FAIL_INVALID_IDLE_STATE                33
#define FAST_FAIL_MRDATA_PROTECTION_FAILURE         34
#define FAST_FAIL_UNEXPECTED_HEAP_EXCEPTION         35
#define FAST_FAIL_INVALID_LOCK_STATE                36
```

[2024-03-31 03:52] Matti: as well as your
```c
#define FAST_FAIL_INCORRECT_STACK                   4
```

[2024-03-31 03:53] Matti: not a lot of comments as you can see (unlike bugcodes.txt), but the names are usually enough for these anyway

[2024-03-31 03:55] Matti: this is only for failfast codes btw, regular bugcheck codes with reserved parameters you'll have to either find in a bugcodes.txt MS leaked by accident like the one above, or find all calls to KeBugCheckEx that match your bugcheck code and then reverse the path that produces your """reserved""" value

[2024-03-31 03:58] Matti: "reserved" in MS documentation or headers never means "reserved for future use" or something like that

[2024-03-31 03:58] Matti: it means "fuck you we're not telling you what this is"

[2024-03-31 04:02] future_wizard: Not sure where else to post this.... So this is a binary I got from an embedded device, and this is supposed to be the libc_start_main function. Ghidra however, does not actually see the main function as a function. It sees it as a label. Not sure what to do about this. Taking a look at the next screenshot Ghidra will decompile it as well. Not sure what about this. Any suggestions are appreciated thanks.
[Attachments: continued_gl_ghidra_moment.JPG, gl_ghidra_moment.JPG]

[2024-03-31 04:02] future_wizard: FYI, the company has a security reporting policy.

[2024-03-31 04:58] daax: [replying to future_wizard: "Not sure where else to post this.... So this is a ..."]
Have you tried reanalyzing the bin? Or using the aggressive instruction finder option when analyzing? Sometimes those have fixed it, otherwise I’ve had to go and manually define a function

[2024-03-31 05:03] daax: I’m assuming it’s also been rebased to the proper loading address?

[2024-03-31 05:19] future_wizard: [replying to daax: "Have you tried reanalyzing the bin? Or using the a..."]
I will give those a shot. Thanks.

[2024-03-31 12:52] irql: [replying to Matti: "this has param 4's meaning for me"]
oh shit, thanks

[2024-03-31 12:52] irql: 🙏🏾

[2024-03-31 12:53] irql: [replying to Matti: "oh they did also skip from 29 to 37 >.<"]
lmfao

[2024-03-31 12:53] irql: love the documentation

[2024-03-31 18:04] hxm: is there any opensource thing that can pack a PE sections like merging them

[2024-03-31 18:04] hxm: ??

[2024-03-31 18:09] sariaki: wdym?

[2024-03-31 18:09] sariaki: like of an already existing binary?

[2024-03-31 18:09] sariaki: cause if not, just tell the linker to merge sections

[2024-03-31 18:11] hxm: how to tell the linker to merge them

[2024-03-31 18:11] hxm: ld

[2024-03-31 18:12] hxm: actualy the point here is to be able to write an obj into x address, assuming there is no relocation and base addr is fixed, was thinking about abusing of the sections thing.

[2024-03-31 18:16] sariaki: i don't actually know exactly how to do it with ld, but it's explained here https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_mono/ld.html#SEC16

[2024-03-31 18:16] sariaki: there's also a stackoverflow thread

[2024-03-31 18:16] hxm: fuck ld, is there any other linker i can do it with

[2024-03-31 18:17] sariaki: i know msvc's linker  has this https://learn.microsoft.com/en-us/cpp/build/reference/merge-combine-sections?view=msvc-170
[Embed: /MERGE (Combine Sections)]
Learn more about: /MERGE (Combine Sections)

[2024-03-31 18:17] hxm: [replying to hxm: "actualy the point here is to be able to write an o..."]
was asking about this

[2024-03-31 18:26] sariaki: https://stackoverflow.com/questions/20962775/how-do-i-specify-the-memory-address-for-a-specific-section-with-a-linker-script
[Embed: How do I specify the memory address for a specific section with a l...]
I'm working on a big project that involves writing a Perl script to transform assembly files and I'm trying to determine what the assembly files will look like after the transformation, and what the

[2024-03-31 18:26] sariaki: <@692740168196685914> could this maby work?

[2024-03-31 18:45] hxm: no, it's not working

[2024-03-31 18:45] hxm: at least for x64pe it produce a broken one

[2024-03-31 20:07] future_wizard: [replying to daax: "Have you tried reanalyzing the bin? Or using the a..."]
Alright, I tried reanalyzing with aggressive instruction finder, and that did not work. It's time to manually define a function.

[2024-03-31 20:13] future_wizard: <@609487237331288074> I created the function. Much easier than I thought it would be ngl. Thanks for the suggestions.

[2024-03-31 23:18] vendor: [replying to hxm: "is there any opensource thing that can pack a PE s..."]
<https://github.com/lief-project/LIEF>