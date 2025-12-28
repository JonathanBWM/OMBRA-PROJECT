# April 2025 - Week 4
# Channel: #reverse-engineering
# Messages: 85

[2025-04-21 03:45] Deleted User: Anyone know a good way to find signed drivers without going thru tons of hoops like installers?

[2025-04-21 09:12] Matti: ya

[2025-04-21 09:12] Matti: `dir C:\Windows\System32\drivers`

[2025-04-21 09:13] Matti: I feel like there is some condition in your question that you forgot to include

[2025-04-21 09:49] the horse: i guess he's looking for more of a centralized list of signed drivers, instead of installing applications

[2025-04-21 09:56] sariaki: depending on what you're looking for this might be good:
https://www.loldrivers.io/
[Embed: LOLDrivers]

[2025-04-21 09:57] sariaki: also scanning virus total / hybrid analysis

[2025-04-21 09:57] sariaki: also also there's some websites that are meant to fix like broken files or whatever and they usually also host some drivers

[2025-04-21 12:20] 0xatul: ^ <@609487237331288074>

[2025-04-21 13:36] daax: [replying to 0xatul: "^ <@609487237331288074>"]
?

[2025-04-21 13:37] 0xatul: [replying to daax: "?"]
there was spam bro

[2025-04-21 13:37] 0xatul: should've pinged at Moderator but muscle memory 😂

[2025-04-21 13:38] daax: [replying to 0xatul: "there was spam bro"]
ah, thanks for pointing it out haha

[2025-04-21 13:38] daax: It looked liked I was tagged for the loldrivers thing and I was like ?? What’d I miss

[2025-04-21 13:38] 0xatul: just a make money at home through this telegram channel scam

[2025-04-21 13:38] 0xatul: nothing else

[2025-04-21 14:38] Deleted User: [replying to Matti: "`dir C:\Windows\System32\drivers`"]
yeah i kinda didnt't specifiy but loldrivers has a few ones + and they are public, but i want to find my own. i've looked thru all my drivers on my machine include the drivers folder and none were any good for what im looking for or nothing that were ready for exploitation.

[2025-04-21 16:51] DirtySecreT: [replying to Deleted User: "yeah i kinda didnt't specifiy but loldrivers has a..."]
download diagnostic software for your vendor

[2025-04-21 16:51] DirtySecreT: look at oem software shipped with mobile platforms

[2025-04-21 16:51] DirtySecreT: third-party applications that get system info using a driver (most will wind up using winring0 sadly)

[2025-04-21 16:53] DirtySecreT: i dont really know what you were expecting unless your board manufacturer shipped a bunch of crap (ex. asus)

[2025-04-21 16:54] DirtySecreT: ur gonna have to do some leg work to find your own, and if you mean via io pathways those are a dime a dozen

[2025-04-21 16:55] DirtySecreT: if you want something that's more subtle youll actually have to do some re and fuzzing of operations like create/read/write/delete/set security info/etc

[2025-04-21 22:39] Deleted User: 👍

[2025-04-22 08:42] NSA, my beloved<3: Hey. I am reverse engineering a binary using Ghidra. There is a call to a function with the only parameter being in ecx. This behavior matches the one when you declare a class in C++, and you call one of its functions. This function initializes a bunch of variables on the stack. These should also be part of the class that the function is in. However, Ghidra interprets these as standalone variables which are related to the ebp. Is there a way for me to make it treat the variables as if they were part of the class that the parent function resides in? Basically declare ebp+offset as a base for the class?

[2025-04-22 08:52] Flo: [replying to NSA, my beloved<3: "Hey. I am reverse engineering a binary using Ghidr..."]
Is the __thiscall convention properly applied?

[2025-04-22 08:53] NSA, my beloved<3: [replying to Flo: "Is the __thiscall convention properly applied?"]
It is set to `__thiscall` yes.

[2025-04-22 08:56] NSA, my beloved<3: I was going to provide a screenshot, however the zoom out functionality of Ghidra doesn't work... Only the zoom in, no matter if I redefine the keybinding...

[2025-04-22 09:02] Flo: I don’t use Ghidra myself, but when you see the decompiler treating what should be member accesses as stack locals—i.e. showing every field as [EBP+0xXX] instead of [ECX+0xXX] —that usually means it thinks your object lives on the stack, so depending on the decompilation, you may need to manually set the base class type.

[2025-04-22 09:02] NSA, my beloved<3: Okay, so apparently deleting the function from the Symbol Tree and re-defining it somehow fixed the issue.

[2025-04-22 09:04] NSA, my beloved<3: Still pissed over the lack of zoom out functionality, and I can not find anyone complaining about it. 😦

[2025-04-22 16:04] daax: <@1333898168949280812> It's great to be proud of your accomplishments, but this is not the channel to put an ad in. We don't know what you are working on or who the people mentioned are. It would be better to give more detail before posting something a bit out of context, or having a discussion around it.

[2025-04-23 18:23] the horse: context: same process
When I load a module with LoadLibraryExA and DONT_RESOLVE_DLL_REFERENCES
then resolve stuff like imports, exceptions, tls, relocations manually and try to run from crt dll main, the crt fails around module_handles_0 or parts of try_get_function

does anyone know why this happens? if it's loaded from file then there's no problem

[2025-04-24 01:47] Deleted User: [replying to the horse: "context: same process
When I load a module with Lo..."]
https://github.com/TheCruZ/Simple-Manual-Map-Injector/blob/ae4bf482920e8f26ff6fdc99544b27c20b9c5312/Manual%20Map%20Injector/injector.cpp#L312
[Embed: Simple-Manual-Map-Injector/Manual Map Injector/injector.cpp at ae4b...]
Simple C++ DLL Manual Map Injector For x86 and x64 - TheCruZ/Simple-Manual-Map-Injector

[2025-04-24 01:48] Deleted User: maybe SEH is not being setup/initialized correctly

[2025-04-24 10:59] Matti: [replying to the horse: "context: same process
When I load a module with Lo..."]
this has got to be SEH related like <@456226577798135808> says
just look at the CRT functions acessing `module_handles[]` and `try_get_function()` -  all are marked as `throw()` which makes them immediately suspect

[2025-04-24 11:00] the horse: probably, makes sense

[2025-04-24 11:00] the horse: thank you

[2025-04-24 11:00] the horse: i just didnt think crt funcs would throw by defaut lol

[2025-04-24 11:00] Matti: best solution: don't use a CRT and also don't use C++ exceptions, not even implicitly
alternative solution: call RtlAddFunctionTable like inthe example above

[2025-04-24 11:01] Matti: this will make your unwind data known to the loader so that SEH will work as intended

[2025-04-24 11:03] Matti: note that SEH != C++ exceptions, that is a whole other minefield

[2025-04-24 11:03] Matti: C++ exceptions may or may not work after doing this, I really don't know

[2025-04-24 11:06] Matti: [replying to the horse: "i just didnt think crt funcs would throw by defaut..."]
I don't think they throw by default btw, that's why they are marked `throw()` after all
but the MS CRT code is generally riddled with C++ exception usage anywhere you look

[2025-04-24 11:07] the horse: yeah I think I'm just gonna rip CRT away; the module I'm trying to map is small anyway; and I can make due without vector & map

[2025-04-24 11:07] the horse: and I'll have to figure out how to handle exceptions in my emulator

[2025-04-24 11:07] the horse: luckily momo supports it in his stuff, so there's some decent material 😄

[2025-04-24 11:08] the horse: still not sure why it only seems to happen with load lib though

[2025-04-24 11:10] Matti: are you sure that is the only place <:thinknow:475800595110821888>

[2025-04-24 11:11] Matti: I count 907 functions marked as `throw()` in the UCRT source code, unless you've tried all of these I wouldn't be so certain

[2025-04-24 11:12] the horse: might not be.. i don't get past that point so 😄

[2025-04-24 11:13] the horse: just the same place that throws in both my emulator and while manual mapping

[2025-04-24 11:14] Matti: yeah, cause loading libraries is generally one of the first things you tend to do right

[2025-04-24 11:14] Matti: and it's something that can fail for a fair amount of reasons

[2025-04-24 11:15] the horse: it's either it tries to init an invalid critical section, or it fails resolving the function
[Attachments: image.png]

[2025-04-24 11:15] the horse: depending on whether it's MSVC or LLVM lol

[2025-04-24 11:18] Matti: yeah that last part sounds familiar... especially if you're linking against the static CRT but also for the DLL case

[2025-04-24 11:18] the horse: yeah this is using the static libs

[2025-04-24 11:19] the horse: and this is msvc
[Attachments: image.png]

[2025-04-24 11:19] the horse: corrupted function pointer from try_get_function

[2025-04-24 11:20] Matti: LLVM does (or can) use the MS UCRT but clang and MSVC generally tend to disagree a fair amount on what constitutes valid C++
nevermind even UB which is well... UB

[2025-04-24 11:20] the horse: i've verified that my semantics are correct for all instructions that are ran

[2025-04-24 11:20] the horse: ```
adc: 24 failed 127236 passed
add: ALL PASSING
sub: 256 failed 117672 passed
and: 906 failed 93194 passed
bextr: 1120 failed 646488 passed
bswap: 144 failed 2080 passed
bzhi: 120 failed 766208 passed

btc: 224 failed 78760 passed
btr: 120 failed 76776 passed
bts: 0 failed 77696 passed

cbw: 0 failed 27 passed
cdq: 0 failed 63 passed
cdqe: 0 failed 87 passed
cqo: 0 failed 127 passed
cwd: 0 failed 32 passed
cwde: 0 failed 42 passed

dec: 0 failed 3288 passed
div: 53 failed 7239 passed

or: 1352 failed 103924 passed
not: 32 failed 2600 passed
neg: 16 failed 3224 passed

rcl: 0 failed 32090 passed
rcr: 22 failed 31468 passed (parser errors)
rol: 32 failed 28986 passed
ror: 0 failed 28157 passed
rol: 32 failed 28986 passed
ror: 0 failed 28157 passed

sahf: 0 failed 8 passed

sal:
sar: 16 failed 28258 passed

sbb: 140 failed 126168 passed
shl: 16 failed 24002 passed
shld: 7286 failed 577850 passed
shr:  0 failed 24110 passed
shrd:  6013 failed 574786 passed
``` -- all of the fails are parser errors

[2025-04-24 11:22] the horse: [replying to Matti: "LLVM does (or can) use the MS UCRT but clang and M..."]
i'll try to add SEH support

[2025-04-24 13:03] Windows2000Warrior**: if i want to compare the throttling CPU in kernel between 2k and XP , what things should i look on and using windbg , any idea ?

[2025-04-26 13:35] diversenok: How do I convert data (marked as `db`/`dw`/`dd`/`dq`) with a string into an actual string (i.e., into `text "UTF-16LE", 'Something',0`) in IDA?

[2025-04-26 13:40] iPower: 
[Attachments: image.png]

[2025-04-26 13:40] f00d: [replying to diversenok: "How do I convert data (marked as `db`/`dw`/`dd`/`d..."]
alt+a select
[Attachments: image.png]

[2025-04-26 13:42] iPower: I usually just set everything to `db` and then apply the change

[2025-04-26 13:43] diversenok: Thanks, I don't think I would've found either of the methods in the next hour =)

[2025-04-26 13:45] iPower: you can also go to `Options` -> `General` -> `Strings` and set the default type

[2025-04-26 13:46] iPower: 
[Attachments: image.png]

[2025-04-26 13:46] iPower: then you can just press A to define a string using whatever you set as default

[2025-04-26 17:33] ShekelMerchant: whats yall goto method for Kernel debugging?? Currently im thinking of spinning up a second vm to run my windbg. What are yall thoughts?

```
┌────────────┐        TCP / COM Pipe        ┌──────────────┐
│  Host A    │            <———————————————► │  Host B      │
│  Ubuntu    │                              │  Windows /   │
│  QEMU‑KVM  │                              │  WinDbg x64  │
│  VM: Win10 │                              └──────────────┘
└────────────┘
```

[2025-04-26 17:57] Horsie: [replying to ShekelMerchant: "whats yall goto method for Kernel debugging?? Curr..."]
I usually prefer to have it native

[2025-04-26 17:58] Horsie: its either windbg on wine if im on a linux host or just plain windbg if im on windows

[2025-04-26 17:59] daax: [replying to ShekelMerchant: "whats yall goto method for Kernel debugging?? Curr..."]
Check the pins in the channel (I believe some others) for discussions in the same topic with Matti and I’s opinions on what we use and what makes life easier

[2025-04-26 18:00] daax: https://discord.com/channels/835610998102425650/835667591393574919/1313901538762162351

[2025-04-26 18:01] daax: Is another discussion, hopefully this gives you some helpful info.

[2025-04-26 18:21] ShekelMerchant: ok thank you for the information 💯

[2025-04-26 21:55] toro: Anyone here have experience with kace and speakeasy? Trying to determine which is the better starter pack to emulate kernel drivers. Speakeasy seems to be the most extendable and usable but willing to hear out anyone who has experience in both.

[2025-04-27 10:13] the horse: if you want to emulate drivers that specifically try to prevent this behavior and obfuscate their functionality, traditional emulators will not suffice

KACE lacks control

it runs majority of the driver natively, only entering their vm on exceptions to support instructions that aren't available in the current cpl (privileged movs like mov cr3, reg, ...)
for more finer control you'd need to single step through the native instructions, introduce your own memory mapping, fix timing discrepancies
^ this is a lot of work that's generally just not viable nowadays

Speakeasy will fail due to unicorn engine's shortcomings (which may or may not be fixed with the development branch using updated qemu), which are usually well abused to either crash the system or divert control flow into a path to throw you off the trail during tracing, it might be a decent starting point towards environment emulation

if anything; i'd use both of them to write your own thing, really depends on what you're trying to accomplish and what types of drivers you're targeting
when it comes to modern anti-cheats, this tooling is obsolete and requires a major overhaul

I don't know of a modern x86 software emulator that's easy to use and wouldn't be targeted for anti-emulation, perhaps [bosch](https://github.com/bochs-emu/Bochs)?
It might be more worthwhile looking into virtualization-assisted solutions, but I might release my x86 emulator in the coming months which aims at supporting kernel drivers with such measures.

[2025-04-27 15:44] UJ: [replying to the horse: "if you want to emulate drivers that specifically t..."]
> virtualization-assisted solutions
> 
Do you have any good examples? I've been looking into DRAKVUF but i need Intel CPU for that.

[2025-04-27 15:46] the horse: qemu for example, you run it in a virtual machine or you use a hypervisor in an already running system (could be a nested environment or metal)

[2025-04-27 15:46] UJ: ah