# November 2025 - Week 2
# Channel: #reverse-engineering
# Messages: 203

[2025-11-03 04:40] grb: im trying to do stack walking for my syscall hook and i found 2 different method, one using RtlVirtualUnwind and one using RtlCaptureStackBackTrace, which one is better? From what i see, the RtlCaptureStackBackTrace is easier and simpler to use but when i consult with GPT, it said that it usually produce incomplete stack traces

[2025-11-03 04:56] ImagineHaxing: [replying to grb: "im trying to do stack walking for my syscall hook ..."]
Id say gpt is full of crap usually even though idk if it's true in this case, but Google would probably be better (watch out for ai written blogs)

[2025-11-03 04:58] grb: I saw this blogpost that uses RtlCaptureStackBackTrace, i dont know if this is just a "pseudocode" or the ACs in this blog post really use it
https://research.meekolab.com/understanding-kernel-level-anticheats-in-online-games#heading-dpcapc-stackwalking
[Embed: Understanding Kernel-Level Anticheats in Online Games]
Seeing Behind the Grey-ish Tactics of Anticheats

[2025-11-03 05:00] koyz: Offtopic but cool to see that my blog post about ace was referenced in there lmao

[2025-11-03 07:24] archie_uwu: According to http://www.nynaeve.net/?p=101, RtlVirtualUnwind is the lower-level routine, and RtlCaptureStackBackTrace builds on it
[Embed: Programming against the x64 exception handling support, part 2: A d...]

[2025-11-03 09:20] grb: [replying to archie_uwu: "According to http://www.nynaeve.net/?p=101, RtlVir..."]
ahh i see i see

[2025-11-03 09:20] grb: nice info

[2025-11-03 09:20] grb: tygoodshare

[2025-11-03 09:21] grb: will use RtlCaptureStackBackTrace then

[2025-11-03 09:22] grb: yup, saw the code under IDA, it does use the internal RtlVirtualUnwind

[2025-11-03 10:02] xatat: Is it possible to queue an USER mode Apc(From within kernel driver) to a specific thread of Target-process, without stack attaching to it?
To unconditionally deliver the apc, I tried setting Alerted offset(0x72) and UserApcPending offset(0x2a from KAPC_STATE) of _KTHREAD after KeInsertQueueApc.
I set both offsets by writing via physical memory. But the user mode apc function never seem to get executed ?

[2025-11-03 10:03] xatat: Both the KTHREAD offsets were found to be same in all windows version from win 10 21h2 to till now.

[2025-11-03 22:01] zeropio: any of you guys synchronise ida with windbg? I used ret-sync for 7.7 but it has not been updated for 4 years and looks like a pain in the ass to re-do

[2025-11-04 05:54] Novoline: https://likeagod.revers.engineering/nt/ consult the bible

[2025-11-04 23:56] Mitch: [replying to grb: "I saw this blogpost that uses RtlCaptureStackBackT..."]
How does a blog post have so much technical information and in depth discussion but still talk about "kernel anticheats can steal your data" at the end

Dude no one stores credit card info and private, personal stuff in the kernel. You can do all of that in usermode.

[2025-11-04 23:57] Mitch: That's such a bad argument. The rest is fine I guess, but no clue how that claim gets repeated over and over

[2025-11-04 23:57] Mitch: To the point that someone who should know better, that its just not a genuine argument.

[2025-11-05 00:42] grb: hahah

[2025-11-05 07:24] grb: [replying to grb: "im trying to do stack walking for my syscall hook ..."]
does RtlCaptureStackBackTrace captures usermode stack too?

[2025-11-05 09:15] grb: switched to RtlWalkFrameChain to capture usermode stack

[2025-11-05 09:49] selfprxvoked: [replying to Mitch: "How does a blog post have so much technical inform..."]
"To be fully honest, even though i do not trust Riot Games (partly owned by Tencent, a Chinese entity) or MiHoYo (owned by MiHoYo, a Chinese entity) if they were to do something such as stealing personal user information it would've probably already been found out by now due to the amount of people trying to crack apart mhyprot and Vanguard. The same applies to other cheat systems like BattlEye, EAC, VAC, etc."

It literally states that at the conclusions so I think the author is trying to say that kernel ACs could be hijacked for EDR bypasses just like mhyprot was in the past.

[2025-11-05 12:24] mtu: You _really_ don’t need to run in the kernel/driver subsystem to avoid being blocked by EDR

[2025-11-05 12:25] mtu: If it’s really important to you that you block EDR telemetry you do, but most malware considers “FUD”/“EDR bypass” to stop at successful execution without being blocked

[2025-11-05 12:26] Addison: I think the point is "avoiding making that easier"

[2025-11-05 12:27] Addison: (but of course, people running EDRs in an enterprise context presumably won't be running games with anti-cheat... right?)

[2025-11-05 12:27] Addison: :clueless:

[2025-11-05 13:11] koyz: [replying to Addison: "(but of course, people running EDRs in an enterpri..."]
I know of several IT admins that have blocked VGK/EAC/BE nowadays because people HAVE tried with company confidential files on their system <:mmmm:904523247205351454>

[2025-11-05 13:57] mtu: Yes

[2025-11-05 13:57] mtu: People also install non-kernel AC video games on corporate assets, which is equally concerning since obviously the sensitive data is available in the user's security context

[2025-11-05 14:35] 0xboga: Is there a way to leverage a kernel R/W primitive for UM shellcode injection/ DLL injection to a remote process?

[2025-11-05 14:56] the horse: yes

[2025-11-05 14:57] the horse: the approach depends on security features enabled

[2025-11-05 14:59] the horse: for shellcode injection; you can do two things

[2025-11-05 14:59] the horse: a) find a code cave, overwrite it and swap some import / data ptr to call into it within the process

[2025-11-05 15:00] the horse: b) swap a function pointer in kernel (often win32k is abused since you can syscall into the functions) and do it that way

[2025-11-05 15:01] the horse: and the other factors depend on the process itself, whether it's protected by an anti-tamper/anti-cheat, etc..

[2025-11-05 15:45] Mitch: [replying to selfprxvoked: ""To be fully honest, even though i do not trust Ri..."]
Sure, it does, but why would they list it as a reason in conclusion. Why give it *any* validity

[2025-11-05 19:38] 0xboga: [replying to the horse: "a) find a code cave, overwrite it and swap some im..."]
I mean that I have a virtual read write through a vulnerable driver, how do I write to an arbitrary (not current) process memory in this manner?

[2025-11-06 04:02] selfprxvoked: [replying to Mitch: "Sure, it does, but why would they list it as a rea..."]
It is valid tho and it did happened in the past like the ESEA crypto mining stuff iirc

[2025-11-06 04:06] selfprxvoked: but I agree that this a silly common-sense argument that kernel anti-cheats will steal your most sensitive data or open vulnerabilities is quite stupid when all of that is easy to do in usermode and wouldn't require an extra component to do it (not even an usermode anti-cheat)

[2025-11-06 10:16] the horse: Is there a way to disable IDA from lifting __readgsword into NtCurrentTeb()?

[2025-11-06 10:18] the horse: nvm, got it -> unload type libraries

[2025-11-06 11:34] Matti: [replying to the horse: "nvm, got it -> unload type libraries"]
well this definitely works, it just also kills all of your type libraries which seems a bit extreme...
pretty sure the offending TIL for this is any/all of the `ntddk*`/`ntddk64` ones in `til/pc`

[2025-11-06 11:35] Matti: those are also responsible for auto-loading a bunch of common internal NT types that change frequently between versions, so they are always wrong

[2025-11-06 11:35] the horse: well the bin i'm looking at doesn't need type libs at all

[2025-11-06 11:36] the horse: it's just an assumption by IDA because it's a PE

[2025-11-06 11:37] Matti: [replying to the horse: "well the bin i'm looking at doesn't need type libs..."]
wdym, type libs are useful on any binary

[2025-11-06 11:37] Matti: just the ones I mentioned are shit

[2025-11-06 11:37] the horse: i'm looking at hvix, i don't believe it has any use for type libraries as there isn't windows-specific code that they cover

[2025-11-06 11:37] the horse: **maybe** hypercalls

[2025-11-06 11:37] the horse: but I think type libs don't contain the structures or map indeces anyway for those

[2025-11-06 11:38] Matti: you can generate tils from pdbs
[Attachments: hvix64.pdb, hvix64.pdb]

[2025-11-06 11:38] the horse: YOU HAVE IDBS

[2025-11-06 11:38] Matti: note that these pdbs are gonna be shit for your specific use case... I'm just saying

[2025-11-06 11:38] Matti: type libs are good

[2025-11-06 11:38] Matti: use them

[2025-11-06 11:39] Matti: https://docs.hex-rays.com/user-guide/types/type-libraries/idaclang_tutorial
[Embed: IDAClang | Hex-Rays Docs]

[2025-11-06 11:39] the horse: you don't happen to have matching bins for those pdbs?

[2025-11-06 11:39] Matti: well probably

[2025-11-06 11:39] Matti: they're not going to be of much use

[2025-11-06 11:40] the horse: any extra info is good use

[2025-11-06 11:40] the horse: even if it's 5 years old

[2025-11-06 11:40] Matti: 5? try 15-20

[2025-11-06 11:40] the horse: oh

[2025-11-06 11:40] Matti: I do have an ARM64 pdb (I think...) for that hv
so hvaa

[2025-11-06 11:40] the horse: did hvix even exist back then lol

[2025-11-06 11:41] Matti: sure
windows server 2008 R2, maybe even 2008

[2025-11-06 11:41] Matti: 
[Attachments: hvix64.exe]

[2025-11-06 11:41] the horse: crazy

[2025-11-06 11:41] the horse: thanks!!

[2025-11-06 11:41] Matti: this probably matches one of those

[2025-11-06 11:41] Matti: don't get too excited... did I mention it's not going to be helpful

[2025-11-06 11:42] Matti: this might be another one
[Attachments: hvix64.exe]

[2025-11-06 11:42] Matti: even older

[2025-11-06 11:43] the horse: actually- quite useful

[2025-11-06 11:43] the horse: thank you very much

[2025-11-06 11:43] the horse: i immediately recognize some funcs, albeit some stuff is not there

[2025-11-06 11:43] Matti: these are arm64 as the filenames indicate, but they are from ~2016, not ~2006
[Attachments: hvaa64.pdb, hvaa64.exe]

[2025-11-06 11:43] the horse: some code never changes!

[2025-11-06 11:44] Matti: make type libs from the pdbs

[2025-11-06 11:45] the horse: will do!

[2025-11-06 11:45] Matti: you can make FLIRT sigs from the binaries (x64 only), then make the sig `autoload.cfg` apply the TILs

[2025-11-06 11:45] Matti: they are more effective when combined

[2025-11-06 11:50] the horse: tilib download locked down now 😔

[2025-11-06 11:50] Matti: thank god

[2025-11-06 11:50] Matti: idaclang >>>> tilib

[2025-11-06 11:50] Matti: I used to use tilib, it was nightmare fuel

[2025-11-06 11:51] the horse: idaclang locked down too

[2025-11-06 11:51] the horse: sad day for poorons

[2025-11-06 11:52] Matti: it should be on fckilfk

[2025-11-06 11:52] Matti: allegedly

[2025-11-06 11:52] the horse: isn't that taken down?

[2025-11-06 11:52] Matti: I actually thought idaclang was free to redistribute, but maybe they changed their minds

[2025-11-06 11:53] Matti: [replying to the horse: "isn't that taken down?"]
unsure, unsure

[2025-11-06 11:54] Matti: I pay for IDA like everyone else

[2025-11-06 11:55] Matti: 
[Attachments: image.png]

[2025-11-06 11:56] the horse: basically a honorary degree

[2025-11-06 11:57] Kano: can anyone bypass this?
[Attachments: support.jpg]

[2025-11-06 11:58] the horse: this looks like a nightmare /s

[2025-11-06 11:58] Kano: no this is tool

[2025-11-06 11:58] the horse: oh is the background not part of it ?

[2025-11-06 11:59] Kano: this coding in delphi language

[2025-11-06 11:59] the horse: I thought you're getting locked out of a beach house 🥀

[2025-11-06 11:59] safareto: i love throwing out random exes in the wild

[2025-11-06 12:00] Kano: [replying to the horse: "oh is the background not part of it ?"]
no thats my screen

[2025-11-06 12:01] Matti: [replying to safareto: "i love throwing out random exes in the wild"]
yeah guy, let's not do this
thanks

[2025-11-06 12:02] Matti: some more context at least would be nice, or else zip and password protect it

[2025-11-06 12:02] safareto: [replying to Matti: "yeah guy, let's not do this
thanks"]
was just about to ping a mod but was wondering if this was worth it

[2025-11-06 12:06] Kano: look at the TLoginCredentialService struct after decompiling with ghidra or ida.
[Attachments: image.png]

[2025-11-06 18:43] noahsx: [replying to Matti: "you can generate tils from pdbs"]
this is absolutely life saving, by any chance do you have some hvax64 pdbs and their corresponding executables?

[2025-11-06 19:07] Matti: [replying to noahsx: "this is absolutely life saving, by any chance do y..."]
certainly
[Attachments: hvax64.exe, hvax64.pdb]

[2025-11-06 19:07] Matti: from ~2009, mind you...

[2025-11-06 19:09] noahsx: [replying to Matti: "certainly"]
thank you very much

[2025-11-06 19:09] noahsx: [replying to Matti: "from ~2009, mind you..."]
itll do just fine! have a great day

[2025-11-06 19:09] Matti: <:AMD:752946304472055850>

[2025-11-06 19:09] Matti: you too!

[2025-11-06 19:25] Matti: <@612231915759468544> I seem to have some other hyperv related PDBs, not sure which ones are interesting other than obviously the HV exes, but hvloader might be of interest
[Attachments: image.png]

[2025-11-06 19:25] Matti: note that these are all public PDBs, so no types

[2025-11-06 19:25] Matti: but as you can see I do seem to have the binaries for them, with version numbers even for some reason

[2025-11-06 19:27] Matti: kdhvcom is another one I'm pretty sure I've got, probably with types too in ARM64 form

[2025-11-06 19:28] noahsx: could you send the hvloader one and the hvboot one too?

[2025-11-06 19:28] noahsx: would be great

[2025-11-06 19:29] noahsx: im on the brink of making hyper-v not cry when it doesnt handle a vmexit itself for a while

[2025-11-06 19:29] noahsx: the images you sent helped loads

[2025-11-06 19:29] noahsx: was able to find routines and cross match them over to latest versions

[2025-11-06 19:29] Matti: hmm well, with regards to hvboot the news is not so good <:harold:704245193016344596> 
technically I've got a pdb for it, but it is public, so no types, and furthermore I haven't got a matching binary either

[2025-11-06 19:30] Matti: so you might have to do some bisecting on win 7 RTM binaries... that is the only version related info I can see so far

[2025-11-06 19:30] Matti: 
[Attachments: hvboot.pdb]

[2025-11-06 19:31] Matti: `C:\Symbols\hvboot.pdb\31AE61E537BD4E2BAF14AAA478C728401`

[2025-11-06 19:31] noahsx: [replying to Matti: "hmm well, with regards to hvboot the news is not s..."]
ah well thank you anyways

[2025-11-06 19:31] Matti: I'll zip up the other two dirs in a bit

[2025-11-06 19:32] noahsx: much appreciated

[2025-11-06 19:32] noahsx: its gonna be great to have a little more help with hyper-v

[2025-11-06 19:53] Matti: here you go, took me a bit longer than I expected
the other dir (10.0.10240.0) turned out to be in delta compressed form, from a WinSxs dir most likely
[Attachments: amd64_microsoft-hyper-v-drivers-hypervisor_31bf3856ad364e35_10.0.15063.608_none_c59617bdcbb15133.7z]

[2025-11-06 19:54] Matti: so those are no good

[2025-11-06 19:55] Matti: I might have the PDBs for them if you could get the original/uncompressed PEs, but that would still be pretty pointless since this dir has got the same PDBs but newer

[2025-11-06 19:56] Matti: the ARM64 and x86 32 bit dirs I threw in are the only ones with private PDBs - but then for the ARM64 kdhvcom I've got the opposite problem where I have the PDB but not the binary 😎

[2025-11-06 20:58] the horse: [replying to noahsx: "im on the brink of making hyper-v not cry when it ..."]
ah, so you larped having the solution for cred

[2025-11-06 20:58] the horse: understandable

[2025-11-06 21:02] Matti: as opposed to your solution, which is...

[2025-11-06 21:02] noahsx: [replying to the horse: "ah, so you larped having the solution for cred"]
no?

[2025-11-06 21:02] noahsx: i just want to find a better way

[2025-11-06 21:02] Matti: I missed whenever the whatever 'cred' was claimed, but try for some constructive posting please

[2025-11-06 21:03] noahsx: a friend of mine foudn the solution

[2025-11-06 21:03] noahsx: and hence why im not releasing it

[2025-11-06 21:03] noahsx: because its his fix

[2025-11-06 21:03] noahsx: where exactly did i claim credit for it too?

[2025-11-06 21:03] noahsx: [replying to noahsx: "a friend of mine foudn the solution"]
theres a simple fix for it which is just a tad bit inefficient

[2025-11-06 21:04] the horse: [replying to Matti: "I missed whenever the whatever 'cred' was claimed,..."]
out-of-sc

[2025-11-06 21:04] the horse: fair enough

[2025-11-06 21:04] noahsx: [replying to Matti: "here you go, took me a bit longer than I expected
..."]
thanks by the way

[2025-11-06 21:06] noahsx: i really appreciate the files youve posted

[2025-11-06 21:06] noahsx: have a good one

[2025-11-06 22:09] toro: Anyone here knows of a database that has versions of different core Windows drivers? The goal is to ascertain how often a particular driver is changed across Windows versions.

[2025-11-07 00:29] the horse: Is there any way to have C++ anonymous unions play nicely with IDA?

[2025-11-07 00:29] the horse: and if something is setting the value/flags directly, is there a way I could tell IDA to lift this into some C++-like constructor?

[2025-11-07 00:31] iPower: [replying to Matti: "<@612231915759468544> I seem to have some other hy..."]
can you share the hvix one?

[2025-11-07 00:31] the horse: [replying to Matti: "you can generate tils from pdbs"]
. <@789295938753396817>

[2025-11-07 00:31] iPower: aaaaah didnt see it

[2025-11-07 00:31] iPower: thanks

[2025-11-07 03:02] expy: [replying to Matti: "the ARM64 and x86 32 bit dirs I threw in are the o..."]
lmk if you need this
[Attachments: image.png]

[2025-11-07 06:56] Kano: [replying to Kano: "look at the TLoginCredentialService struct after d..."]
anyone can do?

[2025-11-07 15:21] Ghoall: Does anyone know a way to reliably check pointer validity (like if it can be safely dereferenced) from usermode without a syscall or crt?

[2025-11-07 15:27] diversenok: https://devblogs.microsoft.com/oldnewthing/20060927-07/?p=29563
[Embed: IsBadXxxPtr should really be called CrashProgramRandomly - The Old ...]
Masking a bug just creates a different, harder-to-find, bug.

[2025-11-07 15:28] Brit: is adress cannonical if so, we ball <:topkek:904522829616263178>

[2025-11-07 15:32] Timmy: there's no way to do this that doesn't introduce race conditions afaik

[2025-11-07 15:33] Brandon: [replying to Ghoall: "Does anyone know a way to reliably check pointer v..."]
I don't think that's possible. 
Although maybe one day someone will find a cool trick using speculative execution. 
Otherwise you'll need to use something like mmap.

[2025-11-07 15:34] Ghoall: hmm alright thanks for the response

[2025-11-07 15:35] noahsx: [replying to Ghoall: "Does anyone know a way to reliably check pointer v..."]
what use case is this where syscalls arent able to be used

[2025-11-07 15:38] Timmy: he said no crt either, without a crt running in the process or a syscall its not possible at all

[2025-11-07 16:09] Matti: ignoring the glaring problems with the whole idea to begin with (see <@503274729894051901>'s link, also I would add race conditions to what raymond chen already listed)...
...you don't need a CRT nor syscalls for SEH to work

[2025-11-07 16:11] Matti: though SEH does go through the kernel which has to raise the user mode exception

[2025-11-07 16:12] Matti: IsBadRead/WritePtr are basically functions that do
```c
__try { *(volatile uint8_t*)p = *(volatile uint8_t*)p } __except(1) { return TRUE; } return FALSE;
```

[2025-11-08 20:11] diversenok: Question: `winnt.h` defines quite a few types of relocations. What is the difference between "based relocations" and all other architecture specific types? And when are they used? Some of them share values, so I assume they apply in different contexts

[2025-11-08 20:11] diversenok: From what I can tell, based relocations appear in .reloc sections of PE files
```c
#define IMAGE_REL_BASED_ABSOLUTE              0
#define IMAGE_REL_BASED_HIGH                  1
#define IMAGE_REL_BASED_LOW                   2
#define IMAGE_REL_BASED_HIGHLOW               3
#define IMAGE_REL_BASED_HIGHADJ               4
#define IMAGE_REL_BASED_MACHINE_SPECIFIC_5    5
#define IMAGE_REL_BASED_RESERVED              6
#define IMAGE_REL_BASED_MACHINE_SPECIFIC_7    7
#define IMAGE_REL_BASED_MACHINE_SPECIFIC_8    8
#define IMAGE_REL_BASED_MACHINE_SPECIFIC_9    9
#define IMAGE_REL_BASED_DIR64                 10
```

[2025-11-08 20:12] diversenok: But then what about i386/AMD64/ARM/ARM64 ones? Where can I see these?

[2025-11-08 20:14] diversenok: So all the `IMAGE_REL_I386_*`, `IMAGE_REL_AMD64_*`, `IMAGE_REL_ARM_*`,  `IMAGE_REL_ARM64_*`, etc.

[2025-11-08 20:26] diversenok: I guess the answer is these are COFF relocations and they do not appear in the resulting PE file

[2025-11-08 20:26] diversenok: https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#coff-relocations-object-only
[Embed: PE Format - Win32 apps]
This specification describes the structure of executable (image) files and object files under the Windows family of operating systems. These files are referred to as Portable Executable (PE) and Commo

[2025-11-08 22:28] x86matthew: [replying to diversenok: "I guess the answer is these are COFF relocations a..."]
yeah these are just for object files IIRC

[2025-11-08 22:28] x86matthew: i have a vague memory of dealing with this when i wrote an emulator for the MIPS version of windows NT

[2025-11-08 22:28] x86matthew: i incorrectly assumed that PE files for MIPS used the `IMAGE_REL_MIPS_*` values and couldn't figure out why the results didn't make any sense

[2025-11-08 22:29] x86matthew: until i realised they just used the same `IMAGE_REL_BASED_*` values as x86 PEs

[2025-11-08 22:35] dullard: [replying to diversenok: "From what I can tell, based relocations appear in ..."]
Based

[2025-11-09 02:56] Siva: mscorlib.dll-resources.dat
Hii guys I found the file in unity related game, how I decompress

[2025-11-09 13:38] ImagineHaxing: im "reversing" a website's bot check and this function is supposed to report data to the server
```js
function saveData(data){
    return;
}``` lmao

[2025-11-09 13:39] Brit: Could also be monkey patched by something else

[2025-11-09 13:39] Brit: Since in js you can just overwrite a func with something else at any point

[2025-11-09 13:40] ImagineHaxing: i didnt think about that

[2025-11-09 13:48] ImagineHaxing: [replying to Brit: "Could also be monkey patched by something else"]
thing is i dont see any network requests made sending over the data

[2025-11-09 13:53] ImagineHaxing: Theres also no websockets open

[2025-11-09 14:10] Brit: Just open the site and dump out the function at runtime

[2025-11-09 14:19] ImagineHaxing: [replying to Brit: "Just open the site and dump out the function at ru..."]
Ill try that and see

[2025-11-09 15:25] Windy Bug: Anyone on here got the chance to look at Windows’s system guard runtime monitor?

[2025-11-09 18:57] ImagineHaxing: [replying to Brit: "Just open the site and dump out the function at ru..."]
i opened the javascript file and looked for the function

[2025-11-09 18:57] ImagineHaxing: still return;

[2025-11-09 19:00] Brit: not the file, you load  the website on a browser like chrome say, and then using the console dump out the contents of whateverscope.saveData

[2025-11-09 19:01] ImagineHaxing: [replying to Brit: "not the file, you load  the website on a browser l..."]
```window.saveData.toString()

"function saveData(data){
    return;
}" ```

[2025-11-09 19:01] ImagineHaxing: this?

[2025-11-09 19:01] Brit: sure

[2025-11-09 19:01] Brit: then it is indeed useless

[2025-11-09 19:02] ImagineHaxing: i told them theres someone macroing and they said they got cutting edge anti botting technologies and they will be banned shortly

[2025-11-09 19:02] ImagineHaxing: lmao