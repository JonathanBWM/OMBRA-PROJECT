# May 2024 - Week 2
# Channel: #programming
# Messages: 284

[2024-05-07 09:33] repnezz: Can you create 2 device objects with the same name?

[2024-05-07 10:31] ᲼᲼: [replying to repnezz: "Can you create 2 device objects with the same name..."]
no

[2024-05-07 10:32] ᲼᲼: see STATUS_OBJECT_NAME_COLLISION (0xC0000035)

[2024-05-07 18:00] Horsie: For any SIMD gurus here:
I need to convert an array of uint64 into a bitset depending on the value of the element. (The array holds only 2 values, `0` and `UINT64MAX`). Iterating through the values and doing a `x & 1` would probably be the obvious approach but its a bit boring.

[2024-05-07 18:01] Horsie: For ex: `[0xFFFF, 0xFFFF, 0x0, 0xFFFF]` turns into `0b1101`

[2024-05-07 18:02] Horsie: This seems like too much of a niche simd requirement but I'm putting it out there regardless.

[2024-05-07 18:32] JustMagic: [replying to Horsie: "For ex: `[0xFFFF, 0xFFFF, 0x0, 0xFFFF]` turns into..."]
movemask?

[2024-05-07 18:41] Horsie: [replying to JustMagic: "movemask?"]
Looks good! Ty!

[2024-05-07 18:41] Horsie: https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#ig_expand=4108,809,4605&techs=MMX,SSE_ALL,AVX_ALL,Other&text=movemask
[Embed: Intel® Intrinsics Guide]
Intel® Intrinsics Guide includes C-style functions that provide access to other instructions without writing assembly code.

[2024-05-07 18:42] Horsie: `_mm256_movemask_pd`

[2024-05-07 18:55] Horsie: I'm surprised to see that its interpreting 'floating' doubles also as 'normal' numbers in the pseudo?

[2024-05-07 18:57] daax: [replying to Horsie: "For ex: `[0xFFFF, 0xFFFF, 0x0, 0xFFFF]` turns into..."]
```cpp
__m256i vec = _mm256_loadu_si256((__m256i*)p);  
__m256i cmp= _mm256_cmpeq_epi64(vec, _mm256_set1_epi64x(-1));
uint64_t mask = _mm256_movemask_epi8(cmp);
return mask >> 7;
```

[2024-05-07 18:59] Horsie: [replying to daax: "```cpp
__m256i vec = _mm256_loadu_si256((__m256i*)..."]
I dont think this will work though right? (with the _epi8)

[2024-05-07 19:00] Horsie: Even if you shift for the first 7 bits, youre still going to get the 'extra' bits from the other 3 qwords?

[2024-05-07 19:00] Horsie: I might be wrong. I'm a bit sleepy and will have to try for myself

[2024-05-07 19:00] daax: [replying to Horsie: "I dont think this will work though right? (with th..."]
~~no idea~~ mm yeah probably not

[2024-05-07 19:00] Horsie: Anyway, for now- using _pd works

[2024-05-07 19:00] Horsie: <https://godbolt.org/z/e9WrM39rj>

[2024-05-07 19:00] Horsie: Pardon the really bad code

[2024-05-07 19:04] daax: [replying to Horsie: "Pardon the really bad code"]
hard to view on mobile

[2024-05-07 19:04] Horsie: 
[Attachments: image.png]

[2024-05-07 19:05] Horsie: if `mask` is `ffffffffffffffff ffffffffffffffff 0 ffffffffffffffff `
`mov_mask` is `1 1 0 1`

[2024-05-07 19:06] Horsie: Ty for suggesting `cmpez_epi64`.. I was just using `_mm256_cmp_pd`

[2024-05-07 19:06] daax: [replying to Horsie: "if `mask` is `ffffffffffffffff ffffffffffffffff 0 ..."]
what was the reasoning for only simd?

[2024-05-07 19:07] Horsie: [replying to daax: "what was the reasoning for only simd?"]
This is code in a really hot path so I thought it would be fun to play with simd (to get some perf out of it). I've never really made anything useful with it so far so I'm just trying things out

[2024-05-07 19:07] Horsie: Next step is to benchmark it against the scalar variant of the code

[2024-05-07 19:08] Horsie: I swear, it took me 30 minutes to just look for these 3 intrinsics that would do what I want. Its such a meme <:topkek:904522829616263178>
Also, unfortunately it seems like https://www.felixcloutier.com/x86/vcompresspd is AVX512 only, which my AMD cant do. I'll need to find a replacement for that as well

[2024-05-07 19:13] Windows2000Warrior: Hello , please Is there anyone interested in helping us get this running on Windows 2000? , it should work , but it is not completed yet, https://github.com/reactos/reactos/pull/5570
[Embed: [BOOTDATA][NVME][DDK] Bare Bones NVMe by atharva1910 · Pull Request...]
Overview
A Working NVMe driver. The driver is takes some inspiration from NVMe OF driver (https://github.com/gigaherz/nvmewin).
The OpenFabric driver is written for NVMe over Fabrics and loads over...

[2024-05-08 03:58] daax: [replying to Windows2000Warrior: "Hello , please Is there anyone interested in helpi..."]
<@148095953742725120>

[2024-05-08 04:04] Matti: it would be a lot easier to backport storport.sys from windows 2003 to 2000/XP and simply use the OFA driver with that

[2024-05-08 04:04] Matti: abusing scsiport.sys for this is not going to end well, is my guess

[2024-05-08 04:05] Matti: since it's, well, made for SCSI drivers

[2024-05-08 04:09] Matti: checking storport.sys (from NT 5.2 32 bit) against windows 2000, the only things you'd be missing are
```
hal:
    KeAcquireInStackQueuedSpinLock
    KeReleaseInStackQueuedSpinLock

ntos:
    InterlockedPopEntrySList
    InterlockedPushEntrySList
    KeAcquireInStackQueuedSpinLockAtDpcLevel
    KeAcquireInterruptSpinLock
    KeFlushQueuedDpcs
    KeReleaseInStackQueuedSpinLockFromDpcLevel
    KeReleaseInterruptSpinLock
    MmProtectMdlSystemAddress
    WmiQueryTraceInformation
    WmiTraceMessage
    WmiTraceMessageVa
    _vsnwprintf
    vDbgPrintExWithPrefix
```

[2024-05-08 04:09] Matti: all of these are trivial with the possible exception of KeFlushQueuedDpcs

[2024-05-08 04:11] Matti: so: write a storport2000.sys that takes care of the above APIs, then make the OFA stornvme driver use that instead of the DDK storport.sys

[2024-05-08 04:13] Matti: do not: rewrite the OFA driver to attempt to make it work with win 2000's scsiport.sys instead

[2024-05-08 04:17] Matti: oh and here's an OFA driver fork that works with 2003 out of the box https://sourceforge.net/projects/nvme-for-windows-2003-server/files/
[Embed: NVMe for Windows 2003 Server -  Browse Files at SourceForge.net]
Community OFA NVMe Storport for Windows Server 2003 R2 SP2

[2024-05-08 06:09] Horsie: [replying to daax: "what was the reasoning for only simd?"]
I'm impressed

[2024-05-08 06:09] Horsie: ~~My simd implementation is only 2.5x slower than the default compiler codegen for scalar~~

[2024-05-08 06:18] Horsie: I've managed to bring it to 1x perf of scalar

[2024-05-08 06:19] Horsie: If only I had VCOMPRESS on my AMD, I think I could beat this easily

[2024-05-08 09:24] Windows2000Warrior: [replying to Matti: "checking storport.sys (from NT 5.2 32 bit) against..."]
Great, so you want it to work even with windows 2000 vanilla , because i think these missing are implimented with extended core 16b

[2024-05-08 09:34] Matti: uhm, I don't know what 'extended core 16b' is
I just ran dependency walker vs the latest NT 5.0 kernel (SP4?)

[2024-05-08 09:39] Windows2000Warrior: [replying to Matti: "uhm, I don't know what 'extended core 16b' is
I ju..."]
Extended core is an update from blackwingcat  ,this person make unofficial updates to windows 2000 from 2008 and he steel work until now (extended core make Win2k pro to run in 32gb of ram and make many new hardware that support XP to work) and extended kernel make Windows 2000 run many new apps that run on XP or vista in some case Http://www.win2k.org/wlu/wluen.htm (extended kernel require IE6 sp1 + official rollup1 (SP4 2 Of 2005)

[2024-05-08 11:29] Matti: [replying to Windows2000Warrior: "Extended core is an update from blackwingcat  ,thi..."]
I mean, 32 GB RAM was always the limit of NT 5.0 x86 with PAE
it was only due to licensing restrictions that you couldn't use this much unless on win 2000 datacenter, so that's a 1 byte patch

[2024-05-08 11:31] Matti: can't comment on the rest really, I don't really use anything older than win 7 for user mode stuff

[2024-05-08 11:31] Matti: preferably nothing newer either...

[2024-05-08 11:31] luci4: [replying to Matti: "preferably nothing newer either..."]
why?

[2024-05-08 11:32] Matti: the modern windows UI is an abomination

[2024-05-08 11:32] Matti: unironically inferior to windows 95

[2024-05-08 11:32] luci4: [replying to Matti: "the modern windows UI is an abomination"]
The Win11 UI is terrible, but I can stand Win10

[2024-05-08 11:32] Matti: they are both shit

[2024-05-08 11:32] Matti: in fact this is what I really don't get

[2024-05-08 11:33] Matti: people who see some kind of difference between the two

[2024-05-08 11:33] Matti: no, windows 10 was always shit too

[2024-05-08 11:33] Matti: as were 8 and 8.1

[2024-05-08 11:33] Azalea: as a gnome user, can confirm windows ui sucks

[2024-05-08 11:34] Matti: > gnome

[2024-05-08 11:35] luci4: [replying to Matti: "people who see some kind of difference between the..."]
I mean there is a small difference. Are they both shit? Sure, but they're slightly different shit

[2024-05-08 11:36] Matti: is there

[2024-05-08 11:36] Matti: looks the same to me

[2024-05-08 11:37] Matti: windows 7 was the last usable version

[2024-05-08 11:37] Windows2000Warrior: [replying to Matti: "checking storport.sys (from NT 5.2 32 bit) against..."]
yes , but In terms of the storport driver in the extended core, these missing are added i think  (i will check for that). in user mode in Win2k i can use discord in serpent basilisk browser and chat gpt ...

[2024-05-08 11:38] Matti: but... I really don't mean to judge, but like why

[2024-05-08 11:39] Matti: why actually use this OS

[2024-05-08 11:39] Matti: I understand hacking on it

[2024-05-08 11:39] Windows2000Warrior: [replying to Matti: "why actually use this OS"]
For reasons of nostalgia

[2024-05-08 11:41] Matti: careful with nostalgia
if you always think back on that 1RM deadlift you got that one time
how are you ever going to improve on it

[2024-05-08 11:42] Matti: yeah windows 2000 was cool, and 2003 was even better

[2024-05-08 11:43] Matti: but time has passed them by

[2024-05-08 11:44] Matti: that doesn't necessarily mean that newer == better obviously.... see windows 8 and after

[2024-05-08 11:44] Matti: a tragic tale

[2024-05-08 11:47] Windows2000Warrior: [replying to Matti: "a tragic tale"]
I use Windows 10 for web development (Windows 7 is the most newer system than 2000 that I can accept ). I just want to see win2k live longer. There are a lot of people interested in this

[2024-05-08 11:49] Matti: a lot of nutjobs

[2024-05-08 11:49] Matti: did you know x86 CPUs support 64 bit instructions now?

[2024-05-08 11:49] Matti: since 2003, in fact...

[2024-05-08 11:54] Windows2000Warrior: [replying to Matti: "a lot of nutjobs"]
Yes, but that is not the problem. The important thing is to feel satisfied. Everyone is comfortable with certain things. There are people who see it as a hobby of making old things into new things

[2024-05-08 11:55] Matti: sure

[2024-05-08 11:55] Matti: me included

[2024-05-08 11:55] Matti: I just don't touch user mode

[2024-05-08 11:55] Matti: or 32 bit

[2024-05-08 11:57] Matti: I mean MRK does still compile for and run on 32 bit x86... but this is mostly to prevent introducing any unintended incompatibility issues that would interfere with a future ARM64 port

[2024-05-08 12:09] rin: [replying to luci4: "The Win11 UI is terrible, but I can stand Win10"]
After win11 came out i started to like win10 ui

[2024-05-08 12:34] sn0w: [replying to Matti: "unironically inferior to windows 95"]
To be fair, win95 / 98 UI was pretty much perfect

[2024-05-08 12:34] Matti: I concur

[2024-05-08 12:35] Matti: I used the classic theme until its removal in windows 8

[2024-05-08 12:35] sn0w: Felt a lot more "solid" than the modern, flat UI they're using now

[2024-05-08 12:36] luci4: [replying to Matti: "I used the classic theme until its removal in wind..."]
Why tf did they remove the themes anyway? I loved that feature

[2024-05-08 12:36] Matti: the *themes*? 😂 you mean the ability to disable them right

[2024-05-08 12:37] Matti: i.e. the classic theme

[2024-05-08 12:59] Deleted User: win11 ui is the best, u just gotta get used to it

[2024-05-08 12:59] Deleted User: when i used win10 i thought the win11 ui was ass, but now i'm familiar with it i can say it's way btter

[2024-05-08 12:59] Brit: I guarantee you were born after 2000

[2024-05-08 12:59] Deleted User: yeah

[2024-05-08 12:59] Deleted User: you aren't?

[2024-05-08 13:00] Brit: nope

[2024-05-08 13:00] Deleted User: oh

[2024-05-08 13:00] Deleted User: that's probably why you guys prefer the old stuff

[2024-05-08 13:00] ash: win11 UI would be alright, if it was applied to everything and not just random bits and pieces

[2024-05-08 13:00] ash: like the old control panel

[2024-05-08 13:01] Brit: baked in ads in the start menu kinda beat

[2024-05-08 13:03] ash: [replying to ash: "win11 UI would be alright, if it was applied to ev..."]
I like cohesiveness and if my hacked together linux desktop is more cohesive, responsive and also feels more polished than a product of a billion dollar company there is something seriously wrong

[2024-05-08 13:12] Matti: [replying to Deleted User: "that's probably why you guys prefer the old stuff"]
yeah, it must be because we're old and therefore incapable of seeing the benefits of the much improved windows ~~8~~ ~~8.1~~ ~~10~~ ~~11~~ 12 user interface

[2024-05-08 13:13] Matti: or maybe the classic theme was simply functional and therefore better

[2024-05-08 13:16] Windows2000Warrior: <@148095953742725120> i check storport.sys with extended core installed , only these are missing : `hal :  KeAcquireInterruptSpinLock
KeReleaseInterruptSpinLock ,ntoskrnl
MmProtectMdlSystemAddress
WmiQueryTraceInformation
WmiTraceMessage
WmiTraceMessageVa
vDbgPrintExWithPrefix`

[2024-05-08 13:18] Matti: OK, that should be your todo list then

[2024-05-08 13:18] Matti: pretty easy

[2024-05-08 13:19] Matti: the Wmi* ones you can even omit completely, same for DbgPrint (although I wouldn't recommend doing so cause it's kinda useful)

[2024-05-08 13:19] brymko: [replying to Brit: "baked in ads in the start menu kinda beat"]
$25 dollar horse kinda beat

[2024-05-08 13:19] Matti: the rest are just 10 variations of the same spinlock acquire/releases

[2024-05-08 13:20] Brit: [replying to brymko: "$25 dollar horse kinda beat"]
horse armor*

[2024-05-08 13:21] Brit: it was a reaction to the vault pass meme

[2024-05-08 13:21] Brit: combined to a shit release

[2024-05-08 13:27] brymko: what horse even 😭

[2024-05-08 13:59] DibyaXP: [replying to Matti: "OK, that should be your todo list then"]
their another problem , storport from 2003 call Hal's interupt table to poke APIC for MSI Interrupts. But 2000 doesn't support MSI Interrupt

[2024-05-08 14:04] Matti: neither does 2003

[2024-05-08 14:04] Matti: you can just indicate to storport that you do not support MSIs when initializing the adapter

[2024-05-08 14:04] Matti: as the OFA driver I linked does

[2024-05-08 14:05] Matti: since it's a well written driver, it has a fallback path to deal with this scenario using legacy interrupts

[2024-05-08 14:06] Matti: so it's literally changing a boolean

[2024-05-08 16:12] Torph: [replying to Matti: "windows 7 was the last usable version"]
lol my grandpa had an old thinkpad with win7, last summer I had to copy files off it and I remember being amazed at how responsive file explorer was compared to the win10 one.
the last time I used win7 was when I was like 8 or 9

[2024-05-08 19:37] Windows2000Warrior: <@148095953742725120> thanks for these information , I have people on my server who are interested in programming. I trying to find someone who can help them with programming or even with only providing information. If you or anyone here wants to join, the server link is on my profile here. The challenge is to make storport work on 2000

[2024-05-08 21:33] Matti: mm well, the way I see it this should be more than doable honestly

[2024-05-08 21:34] Matti: but, I have no interest in doing it

[2024-05-08 21:54] Windows2000Warrior: [replying to Matti: "but, I have no interest in doing it"]
Well, no problem. You are welcome at any time. If you become interested in the future, do not hesitate. Also knowing that <@1143231497386594405>  decided some time ago to rewrite the storport for 2000. It will now start in PNP Part after completing the first stage.(If anyone want to help him please don't hesitate) , thanks

[2024-05-08 21:55] Windows2000Warrior: 
[Attachments: Screenshot_2024-05-08-22-45-19-282_com.discord-edit.jpg]

[2024-05-08 22:39] Nobody: Hi guys! Does anyone know how I can handle windows exceptions in unicorn, for example, EXCEPTION_SINGLE_STEP(https://github.com/Nitr0-G/PeVisor/blob/master/PeVisor/Src/ucHooks.cpp#L45) ? That is, eventually I need to have a fully functional _try __except block and the like. I kind of completely rewrote and adapted all the places in rtls(https://github.com/Nitr0-G/PeVisor/blob/master/PeVisor/Src/Rtls.cpp) for reading memory and writing via uc_mem_read/uc_mem_write of unicorn. Maybe there's a ready-made library somewhere or something?

[2024-05-09 00:30] daax: [replying to Nobody: "Hi guys! Does anyone know how I can handle windows..."]
Are you saying you want to be able to wrap things in `__try / __except` and if an exception occurs it does some sort of inherent emulation with Unicorn? or that it does the emulation within the try/except without any additional code necessary? Can you give an example of what you mean?

[2024-05-09 00:40] Nobody: [replying to daax: "Are you saying you want to be able to wrap things ..."]
I mean handling internal exceptions during code emulation, i.e. if, for example, we execute this section of code in malware:
```
            __try {
                // set T flag
                __writeeflags(__readeflags() | 0x100);
                val = __rdtsc();
                __nop();
                //some stuff
            }
            __except (ctx = (GetExceptionInformation())->ContextRecord,
            drx = (ctx->ContextFlags & CONTEXT_DEBUG_REGISTERS) ? ctx->Dr0 | ctx->Dr1 | ctx->Dr2 | ctx->Dr3 : 0), EXCEPTION_EXECUTE_HANDLER) {
                                //some payload
            }
```
That is, I will receive exception 1, but I will not be transferred to the __except section and so it is possible to mistakenly detect execution through unicorn (our code that we emulate) how to execute via the debugger (I took it as an example).

In reality without debugger , after executing "nop();",  "KiUserExceptionDispatch" will be called and execution will be transferred to the "except section"

[2024-05-09 00:51] szczcur: [replying to Nobody: "I mean handling internal exceptions during code em..."]
are you setting up intr hooks?

[2024-05-09 00:53] Nobody: [replying to szczcur: "are you setting up intr hooks?"]
https://github.com/Nitr0-G/PeVisor/blob/master/PeVisor/Src/ucHooks.cpp#L50

I'm trying to do this)

[2024-05-09 00:55] daax: [replying to Nobody: "https://github.com/Nitr0-G/PeVisor/blob/master/PeV..."]
then you need uc_hook_intr events

[2024-05-09 01:00] Nobody: [replying to daax: "then you need uc_hook_intr events"]
```
LocalAlloc 352 bytes, allocated at 10000000
GetModuleHandleA kernel32.dll, return 224cbfb0000
GetModuleHandleA ntdll.dll, return 224cca40000
IsDebuggerPresent, return 0
CheckRemoteDebuggerPresent handle: FFFFFFFFFFFFFFFF DebuggerPresent: 000000000004FA5C, return 1
NtQueryInformationProcess Process Handle: FFFFFFFFFFFFFFFF ProcessInfoClass: ProcessDebugObjectHandle Process Information: 000000000004FC78 Process Information Length: 8 (PVOID)Return length: 0000000000000000
NtSetInformationThread ThreadHandle: FFFFFFFFFFFFFFFE ThreadInformationClass: ThreadHideFromDebugger ThreadInformation: 0000000000000000 ThreadInformationLength: 0
NtQuerySystemInformation  SystemInformationClass: SystemKernelDebuggerInformation SystemInformation: 000000000004FF78 SystemInformationLength: 2 ReturnLength: 0000000000000000 return 0
NtQuerySystemInformation  SystemInformationClass: SystemModuleInformation SystemInformation: 000000000004F9E8 SystemInformationLength: 0 ReturnLength: 000000000004F9B0 return ffffffffc0000004
LocalAlloc 1e800 bytes, allocated at 10000160
NtQuerySystemInformation  SystemInformationClass: SystemModuleInformation SystemInformation: 0000000010000160 SystemInformationLength: 1e800 ReturnLength: 0000000000000000 return 0
LocalFree, free at 0000000010000160
NtProtectVirtualMemory at 224cae54000 (crackme1.vmp.exe+4000), size 251fff bytes, return 0
NtProtectVirtualMemory at 224cae52000 (crackme1.vmp.exe+2000), size 1b6 bytes, return 0
NtProtectVirtualMemory at 224cae85000 (crackme1.vmp.exe+35000), size 5b58 bytes, return 0
CloseHandle 00000000DEADC0DE, return 0
Exception 1
```
I have just run my sample again(https://github.com/Nitr0-G/PeVisor/tree/master/Samples/Protected). I get to the exception, and then I need to somehow make the transition to the  exception handler

[2024-05-09 01:25] daax: [replying to Nobody: "```
LocalAlloc 352 bytes, allocated at 10000000
Ge..."]
okay, so what have you tried then? you have control of the entire context. modify it, redirect to the exception handler manually (by locating exception_registration_record through the exceptionlist field in the TIB), find the corresponding pexc_routine, construct exception_pointers structure that is expected by the __except block, setup stack to mimic the layout for except block and then modify rip to point at the exception handler function that was parsed from the exception_registration_record.subrecord.handler, and so on

[2024-05-09 01:30] Nobody: [replying to daax: "okay, so what have you tried then? you have contro..."]
Here! That's why I came here) 

I tried to rewrite RTLs for exception from windows xp, but apparently it was the wrong way. I'll try to do as you wrote.

[2024-05-09 04:57] five: im starting to develop with winAPI and i have a newbie question. what programming environment do you recommend? would you guys recommend pelles C compiler?

[2024-05-09 04:59] five: not a procrastination issue, just want to know if there's a better (more productive) way instead of my current

[2024-05-09 05:29] donna🤯: [replying to five: "im starting to develop with winAPI and i have a ne..."]
visual studio

[2024-05-09 05:29] donna🤯: it just makes everything easy

[2024-05-09 13:32] the eternal bleb: microsoft wordpad

[2024-05-09 13:38] Brit: [replying to five: "im starting to develop with winAPI and i have a ne..."]
vscode + msvc if you're just learning stuff

[2024-05-09 15:31] Windows2000Warrior: Requested by developper in my server :  anyone here  please can figure out StorPortBusy,  StorPortCompleteRequest,StorPortDeviceBusy and  StorPortDeviceReady doing? Even better if u can kindly write a implementation. Itwill be also helpful if u can analyze PNP portion Stor Initialization routines. Thanks in Advance

[2024-05-09 15:47] daax: [replying to Windows2000Warrior: "Requested by developper in my server :  anyone her..."]
<#902892977284841503>

[2024-05-09 16:02] Windows2000Warrior: [replying to daax: "<#902892977284841503>"]
Ok i will move this to this  job-listings

[2024-05-09 16:06] daax: [replying to Windows2000Warrior: "Ok i will move this to this  job-listings"]
no no, job listings is for job reqs with compensation included. i don't imagine you will find someone who will do this for free unless it's an open source project

[2024-05-09 16:09] Windows2000Warrior: [replying to daax: "no no, job listings is for job reqs with compensat..."]
Ah sorry ok

[2024-05-09 16:09] sn0w: [replying to Windows2000Warrior: "Requested by developper in my server :  anyone her..."]
Is that related to ReactOS?

[2024-05-09 16:10] Windows2000Warrior: [replying to sn0w: "Is that related to ReactOS?"]
No , we want to rewrite the storport from the scratch now for win2000

[2024-05-09 16:10] sn0w: Ah, fair enough.

[2024-05-09 16:12] Windows2000Warrior: Well, we are really afraid that the people of reactos will steal the code, so the code will be open to programmers only for now .

[2024-05-09 16:42] North: Anyone know if there is a simple way to use Windows.h definitions on Linux with GCC? I don't want to use Windows functions and am compiling normal Linux ELF binaries, I just want to be able to use Windows structs without making my own janky headers copy pasting from the actual Windows headers.

[2024-05-09 20:01] Windy Bug: https://x.com/osrdrivers/status/1788658540510326907?s=46&t=KmxCN1W2Ggg2br8H8_VXHw
[Embed: OSR (@OSRDrivers) on X]
Driver Verifier, everyone's favorite tool for driver quality, is effectively broken in Windows 11.  Seriously.
https://t.co/XUf4vSF2su

[2024-05-09 20:01] Windy Bug: :0

[2024-05-09 20:56] Matti: [replying to Windy Bug: "https://x.com/osrdrivers/status/178865854051032690..."]
that is absolutely insane

[2024-05-09 20:57] Matti: I always assumed MS depended on DV at the very least for driver signing submissions

[2024-05-09 20:57] Matti: so checked kernels are gone, and driver verifier has been broken for 3 years without anyone realising

[2024-05-09 20:57] Matti: good going

[2024-05-09 22:57] Torph: [replying to five: "im starting to develop with winAPI and i have a ne..."]
I agree with the first response, just use VS2022. it's free, will do everything you want. CLion also works great and is a little faster in (most) areas

[2024-05-09 22:58] Torph: [replying to Windows2000Warrior: "Well, we are really afraid that the people of reac..."]
is there a reason to specifically keep it out of ReactOS?

[2024-05-09 23:12] Windows2000Warrior: [replying to Torph: "is there a reason to specifically keep it out of R..."]
The developers of reactos are arrogant and do not want anyone to advise them dibyaXP wanted to help them but they were condescending, and instead of encouragement, they destroy the will.

[2024-05-09 23:17] Matti: on the other hand, however arrogant they might be, their source code is freely available online for you to use

[2024-05-09 23:17] Matti: where is your code

[2024-05-09 23:19] diversenok: [replying to Windows2000Warrior: "Well, we are really afraid that the people of reac..."]
Yeah, I also was about to ask, what's the point of keeping it private? Are you planning to sell it or something

[2024-05-09 23:20] Windows2000Warrior: [replying to diversenok: "Yeah, I also was about to ask, what's the point of..."]
No, is for windows 2000 , for what i can sell ?

[2024-05-09 23:21] Matti: he's not belittling you, come on

[2024-05-09 23:21] diversenok: Yeah, and the guys from reactos didn't accept the pull request, so why do you think they want it now

[2024-05-09 23:21] Windows2000Warrior: [replying to Matti: "on the other hand, however arrogant they might be,..."]
I'm not talking about this aspect, which is not appropriate is to belittle a person

[2024-05-09 23:21] Matti: it's a perfectly reasonable question

[2024-05-09 23:21] Matti: [replying to Matti: "he's not belittling you, come on"]
nor am I

[2024-05-09 23:22] Matti: don't know what  the reactos devs said to the guy but I also doubt they were

[2024-05-09 23:22] diversenok: Do you think they want to steal it in spite now? I just don't get where it comes from

[2024-05-09 23:23] Matti: [replying to Windows2000Warrior: "I'm not talking about this aspect, which is not ap..."]
I am talking about this aspect though

[2024-05-09 23:23] Matti: because I think it's important

[2024-05-09 23:23] Matti: reactos is FOSS

[2024-05-09 23:23] Matti: where is your code

[2024-05-09 23:23] Windows2000Warrior: Ok if anyone want to help dibyaXP he will make all things clear for all questions

[2024-05-09 23:24] Matti: are you suggesting I DM him "where is your code" instead

[2024-05-09 23:24] Matti: I don't really feel like doing that

[2024-05-09 23:25] Matti: I feel like I already know the answer
it's what they call one of them rhetorical questions

[2024-05-09 23:26] Windows2000Warrior: [replying to Matti: "are you suggesting I DM him "where is your code" i..."]
Send him you are free

[2024-05-09 23:26] diversenok: Keeping old code alive is cool and all, I just don't understand why you think somebody would want to steal it. What do they even gain by that

[2024-05-09 23:26] diversenok: Your choice of course

[2024-05-09 23:28] Windows2000Warrior: [replying to diversenok: "Keeping old code alive is cool and all, I just don..."]
They have a problem with the storport

[2024-05-09 23:30] diversenok: Okay, but they explicitly said they don't want to support the driver themselves and instead want to use something that also works on Windows

[2024-05-09 23:30] diversenok: Stealing it will bring the same hassle

[2024-05-09 23:35] Windows2000Warrior: [replying to Matti: "are you suggesting I DM him "where is your code" i..."]
And you can join my server you and <@503274729894051901> and ask all hou need dibyaXP ,  
I now feel a kind of attack. I am sacrificing my thinking for this. My only concern is for this to work on 2000.  I don't know why we humans want to find something that can hinder each other instead of seeking to help. This is a challenge to make storport work on 2000. Who is the crazy person who will buy this code for 2000? It is volunteer work par excellence. Well, I'm sorry for saying the word steal, I haven't found a nicer synonym

[2024-05-09 23:38] Matti: look, let's say I'm interested in this

[2024-05-09 23:38] Matti: money is no issue, I'm a millionaire

[2024-05-09 23:38] Matti: but

[2024-05-09 23:39] Matti: reactos' storport code is here: <https://github.com/reactos/reactos/tree/master/drivers/storage/port/storport>
your storport code is: ....in a private discord? maybe?

[2024-05-09 23:39] Matti: easy choice

[2024-05-09 23:46] Windows2000Warrior: Sometimes I can't explain in words

[2024-05-09 23:46] Matti: [replying to Matti: "look, let's say I'm interested in this"]
ok this is what you might call a 'hypothetical'

[2024-05-09 23:46] Matti: just to clarify, I'm not interested in this

[2024-05-09 23:46] Matti: at all

[2024-05-09 23:47] Matti: so do not add me to a group DM, thanks

[2024-05-09 23:51] diversenok: Same for me

[2024-05-09 23:53] diversenok: Just seems strange (at least in my opinion) to think somebody desperately needs your code and will steal it and erase credits after they declined the pull request with a "don't want to support it" reason

[2024-05-09 23:54] Matti: it does

[2024-05-09 23:54] Matti: more than a little paranoid

[2024-05-09 23:58] diversenok: This sounds more like "my code is great and they are dumb to not realize it yet, but when they do they will desparetely want to steal it" which is kind of sad

[2024-05-09 23:59] diversenok: Hopefully I'm wrong and it's not the case

[2024-05-09 23:59] diversenok: But whatever, honestly

[2024-05-10 00:00] Windows2000Warrior: <@148095953742725120> <@503274729894051901>  I have another person who wants to start developping SATA for 2000 driver and he does not mind that the code is open source. In this case, do you agree to help?

[2024-05-10 00:00] diversenok: I don't specialize in drivers, sorry

[2024-05-10 00:01] Matti: I don't care about 32 bit operating systems, sorry

[2024-05-10 00:02] Windows2000Warrior: Well no problem

[2024-05-10 00:05] Windows2000Warrior: [replying to Matti: "I don't care about 32 bit operating systems, sorry"]
No problem ,I thought you were fond of making old things work into the new 😄 , Especially in the good beginning, where you searched Windows 2000 for missings

[2024-05-10 00:07] Matti: well sure, but I do that in my spare time on a codebase that's private (only because redistributing it would be illegal)
which makes it doubly useless

[2024-05-10 00:07] Matti: I don't mind giving pointers to help people with questions though

[2024-05-10 00:07] Matti: which is what I did

[2024-05-10 00:09] Matti: MS storport is working fine here, but this is on a 64 bit NT 5.2 I'm afraid
[Attachments: image.png]

[2024-05-10 00:10] Windows2000Warrior: [replying to Matti: "I don't mind giving pointers to help people with q..."]
Well, can you join my server to answer difficult questions when needed? There are not many questions on it, but sometimes there are somewhat complicated questions.

[2024-05-10 00:11] Matti: no thank you

[2024-05-10 00:11] Matti: I'm in enough discord servers as is

[2024-05-10 00:11] Matti: if you've got a technical question related to storport, feel free to ask it here

[2024-05-10 00:12] Matti: I'm not even the only person with knowledge of storport either

[2024-05-10 00:20] Windows2000Warrior: [replying to Matti: "if you've got a technical question related to stor..."]
Well, do what makes you comfortable. Whoever writes the code will be the one who will ask, and I may not know how to deliver the message like I did yesterday and I fall into embarrassment

[2024-05-10 00:21] Matti: mind if I ask you something

[2024-05-10 00:21] Matti: why do you have to ask their questions for them

[2024-05-10 00:23] Windows2000Warrior: [replying to Matti: "why do you have to ask their questions for them"]
It's all for Windows 2000, I just want to achieve this on it, I'm not the programmer, I'm just trying to gather volunteer people, I'm just a web developer who doesn't understand these complicated things.

[2024-05-10 00:25] Windows2000Warrior: I think that thinking about this is more tiring than writing 😅

[2024-05-10 00:25] Matti: well what I mean is, why can't they simply ask their questions themselves

[2024-05-10 00:26] Matti: that tends to work better than having someone act as message delivery boy

[2024-05-10 00:38] Windows2000Warrior: [replying to Matti: "that tends to work better than having someone act ..."]
I naturally want to help others, and also it's related to something I love.

[2024-05-10 00:43] Windows2000Warrior: But you are right, I feel embarrassed to move the questions from my server to here, but I have no other choice to since my server lacks experienced people like you.So I kindly ask anyone here who has experience for storport. The server link is on my profile .thanks

[2024-05-10 13:20] daax: [replying to Windows2000Warrior: "It's all for Windows 2000, I just want to achieve ..."]
not sure if this was answered before but what’s driving the interest in windows 2000?

[2024-05-10 13:26] Koreos: [replying to daax: "not sure if this was answered before but what’s dr..."]
he left

[2024-05-10 16:50] rin: [replying to daax: "not sure if this was answered before but what’s dr..."]
Psyop

[2024-05-10 16:50] Brit: [replying to Koreos: "he left"]
yeeees he "left"

[2024-05-10 16:50] Brit: lives on a farm now

[2024-05-10 16:56] Sapphire: Then he wrote templeos 2

[2024-05-11 03:33] Deleted User: ~~hi, is there anyone with experience with the old gdi stuff in windows? would it be possible to create a brush from a bitmap (with at least 16 colors)?~~ figured by disassembling another old piece of software and looking up how it does it

[2024-05-11 04:04] Deleted User: god i hate my ideas sometimes
[Attachments: image.png]

[2024-05-11 13:11] hxm: Hell, what makes CE scan so fast, is there any proted c++ proejct based on its scanner ?

[2024-05-11 13:12] hxm: go an answer there https://forum.cheatengine.org/viewtopic.php?p=5754311 but ig its bit old

[2024-05-11 13:17] hxm: https://github.com/cheat-engine/cheat-engine/tree/master/Cheat%20Engine/CUDA%20pointerscan  is that the answer

[2024-05-11 13:29] sariaki: yes

[2024-05-11 13:33] Brit: https://forum.cheatengine.org/viewtopic.php?p=5772505#5772505

[2024-05-11 13:33] Brit: the algo for searching is quite naive too

[2024-05-11 13:34] hxm: [replying to hxm: "https://github.com/cheat-engine/cheat-engine/tree/..."]
will try to implement this on my project

[2024-05-11 13:34] Brit: probably could get a nice speedup doing horspool

[2024-05-11 13:35] hxm: [replying to Brit: "probably could get a nice speedup doing horspool"]
yes with that it will get faster as the pattern being searched for becomes longer.

[2024-05-11 13:36] Brit: it should outcompete the naive algo for any pattern longer than  2 bytes

[2024-05-11 13:37] Brit: although there probably are even better substring search algorithms than horspool, that was good in the 1980s

[2024-05-11 13:41] hxm: i'm more looking for thefastest way to check bytes differences between two dumps

[2024-05-11 13:41] hxm: ig cuda will do well in that case

[2024-05-11 13:44] Brit: there's probably a function in thrust for cuda to do exactly that

[2024-05-11 13:44] Brit: I'd wager

[2024-05-11 14:26] hxm: <@162611465130475520>  how can i set_target_properties(cmake_cuda PROPERTIES CUDA_SEPARABLE_COMPILATION ON) on cmkr ?

[2024-05-11 14:27] mrexodia: [target.cmake_cuda.properties]
CUDA_blahblah = true

[2024-05-11 14:28] mrexodia: https://cmkr.build/cmake-toml/#targets
[Embed: Reference]
Modern build system based on CMake and TOML.

[2024-05-11 14:28] mrexodia: As you can see in the documentation

[2024-05-11 14:41] hxm: [replying to mrexodia: "[target.cmake_cuda.properties]
CUDA_blahblah = tru..."]

[Attachments: image.png]

[2024-05-11 14:41] mrexodia: You need to make a target first

[2024-05-11 14:42] hxm: ah i wrote it after

[2024-05-11 14:42] mrexodia: I will check next week

[2024-05-11 14:42] hxm: no actualy three is a problem : 

```
[target.cude-tests]
type = "executable" 
sources = ["src/*.cu", "src/*.cuh"]
include-directories = ["include"]
compile-features = ["cxx_std_17"]

[target.cude-tests.properties]
CUDA_SEPARABLE_COMPILATION = true
```

[2024-05-11 14:42] hxm: this cause the type bugg

[2024-05-11 14:45] mrexodia: The target has to match

[2024-05-12 04:18] Torph: [replying to hxm: "https://github.com/cheat-engine/cheat-engine/tree/..."]
wtf didn't realize CE was 50% Pascal

[2024-05-12 04:19] Torph: [replying to mrexodia: "https://cmkr.build/cmake-toml/#targets"]
just realized that cmkr is an actual tool and not a common typo

[2024-05-12 22:13] hxm: i have to use Linux for someshet any dist to advice ???

[2024-05-12 22:14] contificate: if you intend to just use it to get something done

[2024-05-12 22:14] contificate: probs want some ez distro with an installer

[2024-05-12 22:15] Brit: debian

[2024-05-12 22:15] contificate: like ubuntu, linux mint, manjaro, debian

[2024-05-12 22:15] contificate: otherwise the correct answer is always arch

[2024-05-12 22:15] Brit: whatever is currently in lts

[2024-05-12 22:15] Brit: at debian

[2024-05-12 22:15] Brit: don't even think about anything else

[2024-05-12 22:15] Brit: everything else is broken

[2024-05-12 22:15] hxm: alright will go for debian

[2024-05-12 22:15] Brit: or gets broken on upgrade

[2024-05-12 22:15] contificate: clown

[2024-05-12 22:15] hxm: thanx

[2024-05-12 22:15] contificate: just doesn't

[2024-05-12 22:16] contificate: but I'll relent because clearly linux beginner over here and he would break shit

[2024-05-12 22:16] Brit: 
[Attachments: pzn4aybdbmd71.png]

[2024-05-12 22:16] hxm: https://www.linuxmint.com/download_lmde.php
[Embed: Download LMDE 6  - Linux Mint]
Linux Mint is an elegant, easy to use, up to date and comfortable desktop operating system.

[2024-05-12 22:16] contificate: never happened to me

[2024-05-12 22:16] hxm: will go with this

[2024-05-12 22:16] contificate: good choice honestly

[2024-05-12 22:54] sn0w: [replying to contificate: "like ubuntu, linux mint, manjaro, debian"]
Waiting for the next Manjaro AUR DDoS