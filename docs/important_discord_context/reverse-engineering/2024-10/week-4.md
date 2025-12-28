# October 2024 - Week 4
# Channel: #reverse-engineering
# Messages: 217

[2024-10-21 10:22] 0x208D9: nvm got it, thanks to <@148095953742725120> 😄

[2024-10-21 10:23] 0x208D9: .

[2024-10-21 11:07] Matti: 😎

[2024-10-21 19:45] ellacrity: Thank you Matti! 🙂 I will make sure to remove any hardcoded values for alignment (`FileAlignment`/`SectionAlignment`)!

[2024-10-23 19:35] 0xboga: Is PG automatically disabled when enabling HVCI? I don’t see how would they coexist

[2024-10-23 19:42] elias: [replying to 0xboga: "Is PG automatically disabled when enabling HVCI? I..."]
there‘s hyperguard

[2024-10-23 19:44] dinero: https://windows-internals.com/hyperguard-secure-kernel-patch-guard-part-1-skpg-initialization/
[Embed: HyperGuard – Secure Kernel Patch Guard: Part 1 – SKPG Initializatio...]

[2024-10-23 19:47] dinero: https://windows-internals.com/wp-content/uploads/2022/01/SKPG_activation_diagram.png if you would refer to the diagram my sirs

[2024-10-23 19:50] dinero: i think the second article will answer your specific question better though:

[2024-10-23 19:50] dinero: > When normal memory extents are being added to the SKPG Context, all normal kernel address ranges get validated to ensure that the pages have a valid mapping for SKPG protection. For a normal kernel page to be valid for SKPG protection, the page can’t be writable. SKPG will monitor all requested pages for changes, so a writable page, whose contents can change at any time, is not a valid “candidate” for this kind of protection. Therefore, SKPG can only monitor pages whose protection is either “read” or “execute”. Obviously, only valid pages (as indicated by the Valid bit in the PTE) can be protected. There are slight differences to some of the memory extents when HVCI is enabled as SKPG can’t handle certain page types in those conditions.

[2024-10-24 02:32] Jason: [replying to 0xboga: "Is PG automatically disabled when enabling HVCI? I..."]
you may think that because HVCI provides advanced kernel protection, there is no need for PatchGuard to run. But in reality, both serve different purposes, and HVCI builds on top of the security provided by PatchGuard.

[2024-10-24 02:32] Jason: HVCI does not automatically disable PatchGuard. They are designed to coexist and provide layered protection for the Windows kernel.

[2024-10-24 02:36] Jason: I'm looking for crackers with experience in reversing electron (node.js).
bytearrays (R8) and reversing compiled JSC files.

[2024-10-24 02:36] Jason: Who can help me?

[2024-10-24 03:22] Matti: [replying to Jason: "HVCI does not automatically disable PatchGuard. Th..."]
yep, this is right, and as <@1236060016973316206>'s quote above says the reasoning for this design isn't something like 'more layers is better', but because there are actual limitations to what hyperguard can protect simply because it works completely differently from PG

[2024-10-24 03:24] Matti: fun fact: efiguard for example can disable patchguard at boot time even on a system that is booted with HVCI enabled

[2024-10-24 03:26] Matti: I've never really thought of trying to find a use for this - if there is one at all, it's probably gonna be more clever than useful

[2024-10-24 06:14] 0xboga: [replying to Jason: "you may think that because HVCI provides advanced ..."]
I was thinking that because PG executes from RWX pages(?)  and HVCI is supposed to prevent that from being possible in kernel?

[2024-10-24 10:00] Jason: [replying to 0xboga: "I was thinking that because PG executes from RWX p..."]
PatchGuard doesn’t need RWX pages to function. It primarily performs integrity checks and monitors for modifications to the kernel’s code and data structures. It can work alongside HVCI, which adds an additional layer of security by enforcing code integrity through the hypervisor.

[2024-10-24 10:01] Jason: HVCI enhances kernel protection by enforcing code signing and integrity at the hardware level, while PatchGuard provides ongoing monitoring and integrity checks. They serve different purposes and don’t conflict, so HVCI doesn’t disable PatchGuard despite the additional restrictions on executable code like blocking RWX pages.

[2024-10-24 10:08] 0xboga: [replying to Jason: "PatchGuard doesn’t need RWX pages to function. It ..."]
http://blog.can.ac/2024/06/28/pgc-garbage-collecting-patchguard/ well?
[Embed: PgC: Garbage collecting Patchguard away]
I have released another article about Patchguard almost 5 years ago, ByePg, which was about exception hooking in the kernel, but let’s be frank, it didn’t entirely get rid of Patchguard

[2024-10-24 13:13] 0x208D9: can anyone point me to some resource which details on how ntfs.sys is loaded from the user mode in windows? the end goal is to get its GUID struct but i also wanna know it details how it loads

[2024-10-24 13:16] 0x208D9: also is there any techniques to figure out the instrumentation of a kernel driver? like i want to reach to a certain function and i wanna see what are the previous functions which are called to reach that certain function

[2024-10-24 13:19] Brit: bp + callstack is the low iq cheap way

[2024-10-24 13:23] 0x208D9: [replying to Brit: "bp + callstack is the low iq cheap way"]
well if i were to do that it wont be possible since there are too many xrefs happening before that call and it would be very tedious to go through all of em

[2024-10-24 13:24] daax: [replying to 0x208D9: "well if i were to do that it wont be possible sinc..."]
conditional bp, or write a little stub with windbg with eb that int 3s on whatever condition youre looking for so its not choking the system.

[2024-10-24 13:25] 0x208D9: seems like a nice idea, lemme try the stubs approach

[2024-10-24 21:13] ByteWhite1x1: Hi. I just posted a job offer related to https://msrc.microsoft.com/update-guide/en-US/advisory/CVE-2024-37977 Does anyone have any idea?
[Embed: Security Update Guide - Microsoft Security Response Center]

[2024-10-24 21:14] ByteWhite1x1: I am an anti-malware driver developer but new to UEFI...

[2024-10-24 22:43] Torph: [replying to Jason: "PatchGuard doesn’t need RWX pages to function. It ..."]
does it use the page dirty bit to watch for changes? I wonder if drivers have the ability to reset dirty bits for kernel memory... I feel like they would've thought of that though

[2024-10-24 22:49] Brit: you're wasting your time replying to chatgpt garb

[2024-10-24 22:52] Torph: <:kekw:904522300257345566>

[2024-10-25 00:57] handle: [replying to Brit: "you're wasting your time replying to chatgpt garb"]
fucking insane to me that someone would let a fucking bot talk for them

[2024-10-25 00:57] handle: what a crazy way to devalue yourself as a person

[2024-10-25 01:48] dinero: unironically true

[2024-10-25 14:12] ByteWhite1x1: Hi SC. I posted last night a job offer. My boss is after this: "Main goal bypass enabled Secure Boot in UEFI to load custom bootloader". AFAIK. An EV signed EFI is the only way but I am listening. Thx.

[2024-10-25 14:14] dinero: wut

[2024-10-25 14:14] dinero: are you recruiting?

[2024-10-25 14:15] ByteWhite1x1: My "boss" is recruiting

[2024-10-25 14:25] ByteWhite1x1: FAILURE_BUCKET_ID:  0x109_19_ANALYSIS_INCONCLUSIVE!unknown_function (PG does not like me) LOL

[2024-10-25 14:40] dinero: analysis_conclusive!p2c_detected

[2024-10-25 15:04] Timmy: <:OMEGALUL:662670462215782440>

[2024-10-25 15:05] Timmy: 
[Attachments: image.png]

[2024-10-25 15:27] brymko: where u see this

[2024-10-25 15:36] dullard: [replying to brymko: "where u see this"]
ByteWhite1x1’s profile

[2024-10-25 15:42] JustMagic: [replying to Timmy: ""]
Pretty sure because he already got banned from here on his main

[2024-10-25 17:53] hecker: [replying to ByteWhite1x1: "Hi SC. I posted last night a job offer. My boss is..."]
The magic word is merci beaucoup 😂

[2024-10-26 13:00] diversenok: It's an internal function called when NTFS needs to back up the original file content via a non-cahed read when modifying files in a transaction

[2024-10-26 13:01] diversenok: The only way to reach it I see is `TxfLogPriorToWrite` -> `TxfLogOldStreamData` -> `TxfReadFileNonCached`

[2024-10-26 13:01] diversenok: While the non-cached flag is set

[2024-10-26 13:03] diversenok: So from `NtfsCommonWrite`, `NtfsSetAllocationInfo`, or `NtfsSetEndOfFileInfo`

[2024-10-26 13:04] diversenok: i.e, when writing or resizing transacted files

[2024-10-26 13:04] 0x208D9: [replying to diversenok: "The only way to reach it I see is `TxfLogPriorToWr..."]
i figured that out while static analysis and tracking the xrefs but couldnt figure out how to set the non-cached flag

[2024-10-26 13:04] 0x208D9: also none of those Txf functions hit on windbg other than NtfsCommonWrite

[2024-10-26 13:05] 0x208D9: but i couldnt analyze the CommonWrite function cuz of lack of debug symbols so dunno how it reaches to that from there

[2024-10-26 13:07] diversenok: You might need to open the file for non-cached writes, so something like `FILE_NO_INTERMEDIATE_BUFFERING` or `FILE_WRITE_THROUGH`

[2024-10-26 13:07] diversenok: How do you trigger the write?

[2024-10-26 13:07] 0x208D9: CreateTransaction then Writefile and then CommitTransaction

[2024-10-26 13:08] 0x208D9: wait im on phone rn will share the snippet when on pc

[2024-10-26 13:08] 0x208D9: [replying to diversenok: "You might need to open the file for non-cached wri..."]
need to recheck if i used those flags as the parameter

[2024-10-26 13:09] diversenok: Not sure how they translate into Win32 equivalents

[2024-10-26 13:10] 0x208D9: [replying to diversenok: "Not sure how they translate into Win32 equivalents"]
TRANSACTION_DO_NOT_PROMOTE?

[2024-10-26 13:11] diversenok: Caching is controlled on a file basis, when opening files, not when creating transactions

[2024-10-26 13:12] diversenok: Transactions only become filesystem transactions when used in combination with file operations

[2024-10-26 13:12] diversenok: It's the wrong level of abstraction to look for special transaction creation flags

[2024-10-26 13:13] diversenok: I can recommend to try doing it via FileTest

[2024-10-26 13:13] diversenok: http://www.zezula.net/en/tools/filetest.html

[2024-10-26 13:14] 0x208D9: ^^ thanks, ima try asap when i reach to my pc

[2024-10-26 13:17] diversenok: Something like `CreateTransaction`, then `NtCreateFile` using this transaction (and some non-caching flags passed in creation options), and then try to write to it or set `FileEndOfFileInformation` or `FileAllocationInformation` or something similar

[2024-10-26 13:17] diversenok: I might try it myself a bit later

[2024-10-26 13:35] MonTy: I want to learn vtil and use the demo version of vmprotect for this. Can you tell me what the vtil code should look like for such a handler?

[2024-10-26 13:36] MonTy: 
[Attachments: IMG_0468.jpg]

[2024-10-26 13:41] koyz: [replying to MonTy: ""]
https://getsharex.com/

[2024-10-26 13:45] MonTy: [replying to koyz: "https://getsharex.com/"]
?

[2024-10-26 13:52] MonTy: Can't see the image?

[2024-10-26 15:01] Deleted User: this got me f'd up

[2024-10-26 15:16] Timmy: https://cdn.discordapp.com/attachments/1160039232388202556/1294827156463157339/brave_saimU3vC2e.gif

[2024-10-26 15:29] 0x208D9: [replying to diversenok: "Something like `CreateTransaction`, then `NtCreate..."]
this seems more appropriate than NtCreateFile in this usecase, isnt it? : https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-createfiletransacteda
[Embed: CreateFileTransactedA function (winbase.h) - Win32 apps]
Creates or opens a file, file stream, or directory as a transacted operation. (ANSI)

[2024-10-26 15:31] 0x208D9: nvm it doesnt support the transaction options, and idk how its implemented internally

[2024-10-26 15:32] 0x208D9: only generic file attributes are supported

[2024-10-26 15:46] Matti: I made a toy project based on https://github.com/3gstudent/Inject-dll-by-Process-Doppelganging some years ago that uses a bunch of these APIs (and also a bunch that are totally unrelated)
[Embed: GitHub - 3gstudent/Inject-dll-by-Process-Doppelganging: Process Dop...]
Process Doppelgänging. Contribute to 3gstudent/Inject-dll-by-Process-Doppelganging development by creating an account on GitHub.

[2024-10-26 15:47] Matti: here's the (GPLv3 of course) main.cpp
[Attachments: main.cpp]

[2024-10-26 15:47] Matti: I can post the entire project if you really want me to but this should be more than enough to answer your questions about how to open a file transacted

[2024-10-26 15:48] Matti: note that the no-caching flag, whichever it is precisely, isn't involved here,, so you do still need to add that to NtCreateFile

[2024-10-26 15:50] Matti: also read https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/creating-a-transactional-client first maybe, especially the fact that committing or rolling back transactions can be done sync or async just like any other I/O API
[Embed: Creating a Transactional Client - Windows drivers]
Creating a Transactional Client

[2024-10-26 15:53] 0x208D9: [replying to Matti: "I made a toy project based on https://github.com/3..."]
greatly appreciate it , i thought u were busy cuz u didnt reply to the dm back then lmao, thanks alot

[2024-10-26 15:54] Matti: oh
well yeah I probably was busy, but also I just get quite a lot of DMs sometimes and then some get tragically lost

[2024-10-26 15:55] Matti: just send me a reminder after a few days if I don't respond

[2024-10-26 15:55] Matti: not the next hour

[2024-10-26 15:55] 0x208D9: sure lol

[2024-10-26 15:55] 0x208D9: thats fine

[2024-10-26 15:57] Matti: interestingly enough the code above still seems to work
[Attachments: image.png]

[2024-10-26 15:57] Matti: with some interesting quirks like the image filename being gone

[2024-10-26 16:40] 0x208D9: [replying to diversenok: "http://www.zezula.net/en/tools/filetest.html"]
tried this utility and my code , doesnt hit the breakpoint, either ways
[Attachments: transact.c]

[2024-10-26 16:40] 0x208D9: [replying to diversenok: "I might try it myself a bit later"]
aight lemme know later on

[2024-10-26 16:41] 0x208D9: oops forgot turn it off, sorry for multiple pings
[Attachments: image.png]

[2024-10-26 17:03] MonTy: Am I wrong?

[2024-10-26 17:03] Deleted User: about what i see nothing

[2024-10-26 17:09] MonTy: Yes, I'm new to the topic of deobfuscation.
I ask questions, politely and carefully
If you can't help, why engage in trolling? Don't you have enough sense for more?

[2024-10-26 17:19] MonTy: [replying to Timmy: "https://cdn.discordapp.com/attachments/11600392323..."]
Sorry;(
[Attachments: image0.jpg]

[2024-10-26 17:29] diversenok: [replying to 0x208D9: "this seems more appropriate than NtCreateFile in t..."]
Were you not using it already? `CreateFileTransacted` is kind of the only Win32 API to open files in a transaction

[2024-10-26 17:30] diversenok: Either way, it calls `NtCreateFile` under the hood

[2024-10-26 17:33] diversenok: [replying to 0x208D9: "tried this utility and my code , doesnt hit the br..."]
You need to tell `NtCreateFile` which transaction to use via `RtlSetCurrentTransaction`, otherwise it won't use any

[2024-10-26 19:02] 0x208D9: oh lmao i sent the wrong code wait, here it is
[Attachments: TxF.c]

[2024-10-26 19:02] 0x208D9: 
[Attachments: ntddk.h]

[2024-10-26 19:26] Matti: ```c
#include <windows.h>
#include <iostream>
#include "ntddk.h"

#pragma comment(lib, "ntdll.lib")
```

[2024-10-26 19:26] Matti: please don't do this

[2024-10-26 19:26] 0x208D9: [replying to 0x208D9: "oh lmao i sent the wrong code wait, here it is"]
wait i set the wrong flags as well

[2024-10-26 19:26] 0x208D9: in hurry

[2024-10-26 19:27] Matti: you're mixing the windows SDK, the STL and the WDK, and then putting a linker option in source code

[2024-10-26 19:27] 0x208D9: [replying to Matti: "you're mixing the windows SDK, the STL and the WDK..."]
any better suggestions?

[2024-10-26 19:27] 0x208D9: im just hacking my way around

[2024-10-26 19:27] Matti: yeah just don't do that

[2024-10-26 19:27] Matti: you can use phnt headers for example

[2024-10-26 19:27] 0x208D9: alright

[2024-10-26 19:27] Matti: and why use iostream ever, printf is superior

[2024-10-26 19:28] Deleted User: but arrows

[2024-10-26 19:30] 0x208D9: [replying to Matti: "and why use iostream ever, printf is superior"]
i just used the template so didnt bother changing the header lol

[2024-10-26 19:30] 0x208D9: i generally use stdio in an empty template

[2024-10-26 19:33] Matti: ok, well only mentioning it because mixing these three up (especially SDK + WDK and STL + WDK) can definitely fuck you in not fun ways

[2024-10-26 19:33] Matti: other than that, why bother with all the getprocaddress stuff - if you use phnt headers you can just call functions and the linker will make them static imports

[2024-10-26 19:33] Matti: that's like half the code gone

[2024-10-26 19:34] Matti: and the other half is gone due to being redefinitions of common types that would also be in phnt

[2024-10-26 19:34] Matti: then we can read the maybe 10-20 lines of code that's in there

[2024-10-26 19:36] 0x208D9: ^^ thanks, ima try reimplementing and fixing it like that

[2024-10-26 19:37] Matti: what is the actual problem atm

[2024-10-26 19:37] Matti: you want to call the function but the bp is not being hit?

[2024-10-26 19:38] diversenok: The bp in an internal TxF function

[2024-10-26 19:39] diversenok: Figuring out how to trigger it

[2024-10-26 19:39] 0x208D9: ^^ this

[2024-10-26 19:40] Matti: [replying to diversenok: "The only way to reach it I see is `TxfLogPriorToWr..."]
well I would just set a bp at the start of this chain

[2024-10-26 19:40] Matti: which is apparently the only path to reach the function

[2024-10-26 19:40] Matti: and at each step, verify that the next function is being called as expected

[2024-10-26 19:40] Matti: if not - figure out why

[2024-10-26 19:42] 0x208D9: [replying to Matti: "which is apparently the only path to reach the fun..."]
thats actually the first thing i did , bp at the top of the function chain on TxfLogPrior*, problem being it aint hitting cuz , i figured maybe finding out which function calls that? i xref'ed and its NtfsCommonWrite, if i set a bp it hits every 30sec and creates an undebuggable environment

[2024-10-26 19:43] Matti: but surely there is a condition that is being checked in order to call this function

[2024-10-26 19:43] Matti: and the check is failing

[2024-10-26 19:43] Matti: you only need to step through that function once

[2024-10-26 19:44] 0x208D9: [replying to Matti: "you only need to step through that function once"]
ig i do need to, its actually quite a big function to step through so lol, figured would be shortcut to repro with a c code, but seems like need to go through

[2024-10-26 19:45] Matti: well you could also find the check first, and then set the bp there

[2024-10-26 19:45] Matti: that will probably save you time

[2024-10-26 19:46] Matti: not a lot of point in stepping through 1000 instructions if you're only interested in 1 or 2 of them

[2024-10-26 19:56] diversenok: Ah, damn, I cannot test it in Windows Sandbox: `STATUS_TRANSACTIONAL_OPEN_NOT_ALLOWED`

[2024-10-26 19:57] diversenok: First time seeing this error

[2024-10-26 21:05] Matti: I think it's possible to disable TxF support now depending on the version

[2024-10-26 21:06] Matti: I saw this in server 2025 preview and immediately hit yes

[2024-10-26 21:06] Matti: maybe sandbox has a similar config?

[2024-10-26 21:44] diversenok: My guess is it's not really supported given all the magic that happens there with filesystem redirection/layering
[Attachments: image.png]

[2024-10-26 21:49] Matti: hmmm maybe

[2024-10-26 21:50] Matti: I wanted to test my own exe on server 2025, turns out windows update has fucked it up
[Attachments: image.png]

[2024-10-26 21:50] Matti: 😩

[2024-10-26 21:52] Matti: > the windows servicing stack uses TxF
oh wow, after 20 years I finally discover the reason it's so fucking slow
[Attachments: image.png]

[2024-10-26 21:53] Matti: my exe also changed its behaviour:
[Attachments: image.png]

[2024-10-26 21:53] Matti: different NTSTATUS than yours though

[2024-10-26 21:54] diversenok: Interesting, didn't know you can disable TxF like this

[2024-10-26 21:54] Matti: I hadn't seen it before either, so I think it must be new

[2024-10-26 21:54] diversenok: `FileFsAttributeInformation` says no transactions inside the sandbox
[Attachments: image.png]

[2024-10-26 21:55] Matti: do you know which FS it is?

[2024-10-26 21:55] Matti: refs? something else?

[2024-10-26 21:56] diversenok: NTFS, but it's complicated because even though it looks like one volume, it's composed of multiple parts, clearly stitched together via a minifilter

[2024-10-26 21:57] Matti: yeah I've heard jonas go on about it

[2024-10-26 21:57] Matti: it sounds insane

[2024-10-26 21:58] Matti: only 2 culprits for your ntstatus
[Attachments: image.png]

[2024-10-26 21:59] diversenok: I see multiple files reporting the same ID if they belong to mounted directories, for instance

[2024-10-26 22:00] diversenok: It's reparse points all the way down, carefully hidden by WCI

[2024-10-26 22:09] Matti: can you send me the exe, or a zip with the vcxproj?

[2024-10-26 22:09] Matti: I'll try it out on server 2025 with TxF on

[2024-10-26 22:12] Matti: oh and likewise here's mine
it should open notepad.exe while actually running netplwiz.exe
[Attachments: HollowingTransacted.exe]

[2024-10-26 22:12] Matti: it does seem to lose some information in the process....
[Attachments: image.png]

[2024-10-26 22:18] Matti: interesting
[Attachments: image.png]

[2024-10-26 22:24] Matti: I guess I need to install windows sandbox now huh

[2024-10-26 22:24] Matti: fuck

[2024-10-26 22:28] Matti: hmmm sandbox may not actually exist as a feature on win server

[2024-10-26 22:28] Matti: it's already got container support after all right

[2024-10-26 22:28] Matti: I can't find it at least

[2024-10-26 22:36] Matti: does it ever start doing anything? or is this just the natural hyper-v slowness I'm experiencing
[Attachments: image.png]

[2024-10-26 22:39] Matti: hmm finally
[Attachments: image.png]

[2024-10-26 22:40] Matti: so failure is due to a failing dependency

[2024-10-26 22:40] Matti: which is "hyper-v host compute service"

[2024-10-26 22:40] Matti: kek
[Attachments: image.png]

[2024-10-26 22:53] Matti: can't make much more of this than whoever wrote vmcompute.exe being too incompetent to use MS RPC correctly
[Attachments: image.png]

[2024-10-26 22:53] Matti: I give up on hyper-v once again

[2024-10-26 22:53] Matti: see ya in a year or so

[2024-10-26 23:45] diversenok: [replying to Matti: "can you send me the exe, or a zip with the vcxproj..."]
There is no exe, I'm just playing in FileTest

[2024-10-26 23:47] diversenok: [replying to Matti: "oh and likewise here's mine
it should open notepad..."]
The same error as before (STATUS_TRANSACTIONAL_OPEN_NOT_ALLOWED) in the sandbox
[Attachments: image.png]

[2024-10-26 23:49] diversenok: And Win 11 has `NtCreateProcessEx` blocked by default (STATUS_NOT_SUPPORTED), sadly

[2024-10-26 23:57] diversenok: Otherwise, yeah, I know this idea, it fails to query the filename at process creation and then caches an empty string

[2024-10-26 23:58] diversenok: Probably failed to query the name with `STATUS_TRANSACTION_NOT_ACTIVE`

[2024-10-26 23:59] diversenok: I guess I can add a new category to this picture
[Attachments: 06.png]

[2024-10-27 00:03] diversenok: [replying to Matti: "so failure is due to a failing dependency"]
Yeah, I have no idea how to fix it

[2024-10-27 00:04] diversenok: I assume if, say, DISM, reports the component as installed correctly it should work fine

[2024-10-27 00:06] diversenok: `dism /Get-FeatureInfo /FeatureName:Containers-DisposableClientVM /Online`

[2024-10-27 00:08] diversenok: It does have a dependency other features like containers and hyper-v

[2024-10-27 00:09] diversenok: ¯\_(ツ)_/¯

[2024-10-27 03:48] Matti: ah, no I wa actually talking about installing it on my own windows 11 machine there

[2024-10-27 03:48] Matti: but hvcompute.exe fails to start meaning neither hyper-v nor the sandbox work

[2024-10-27 03:49] Matti: I'l try dism on hte server install, who knows it might work

[2024-10-27 03:49] Matti: and/or it could be a feature on demand thing there for whatever reason

[2024-10-27 03:50] Matti: but don't worry about it, I'm pretty damn sure I don't actually *want* windows sandbox lol

[2024-10-27 03:51] Matti: hyper-v not working on a fresh win 11 install of less than a week old is disappointing though

[2024-10-27 03:55] Matti: [replying to diversenok: "`dism /Get-FeatureInfo /FeatureName:Containers-Dis..."]
nah like I said this simply doesn't even seem to exist on server

[2024-10-27 03:56] Matti: hyper-v et al do and also work

[2024-10-27 03:57] Matti: on my win 11 box I can install both successfully (dism or control panel), only they don't work

[2024-10-27 03:58] Matti: this all suits me fine though, it's not like I'm going to leave hyper-v running on my workstation

[2024-10-27 03:58] Matti: it's slow enough as is

[2024-10-27 03:59] Matti: then I can use the other system for testing out things like hyper-v

[2024-10-27 07:51] 0x208D9: windows sandbox also lacks many other functionalities which i realised when i tried to run a malware which uses some functionalities from afd (need to check , cant remember on top of my head). and when i opened the sandbox afd driver for static analysis it wasnt simply there. ig its just a toned down version of windows used for dynamic analysis by mpengine under the hood than a viable : 

https://github.com/microsoft/Windows-Sandbox
[Embed: GitHub - microsoft/Windows-Sandbox: Disposable, secure and lightwei...]
Disposable, secure and lightweight Windows Desktop Environment - microsoft/Windows-Sandbox

[2024-10-27 08:09] 0x208D9: https://learn.microsoft.com/en-us/windows/security/application-security/application-isolation/windows-sandbox/windows-sandbox-architecture

also integrated kernel scheduler is something im very susy about tbh
[Embed: Windows Sandbox architecture]

[2024-10-27 16:19] 🐘: ```cpp
__int64 __fastcall ntdll_RtlPcToFileHeader(unsigned __int64 FileHeaderPointer, _QWORD *ImageBaseAddress)
{
  __int64 result; // rax
  __int128 WhatIsThis; // [rsp+20h] [rbp-28h] BYREF

  if ( FileHeaderPointer < (unsigned __int64)off_7FFC4A0D1518
    || FileHeaderPointer >= (unsigned __int64)off_7FFC4A0D1518 + (unsigned int)qword_7FFC4A0D1520 )
  {
    ntdll_RtlpxLookupFunctionTable(FileHeaderPointer, &WhatIsThis);
  }
  else
  {
    WhatIsThis = *(_OWORD *)&off_7FFC4A0D1510;
  }
  result = *((_QWORD *)&WhatIsThis + 1);
  *ImageBaseAddress = *((_QWORD *)&WhatIsThis + 1);
  return result;
}
```

[2024-10-27 16:20] 🐘: 
[Attachments: image.png]

[2024-10-27 16:20] 🐘: what is this

[2024-10-27 16:20] 🐘: ```
ntdll:00007FFC4A0C2000 qword_7FFC4A0C2000 dq 10E200001010h     ; DATA XREF: ntdll:off_7FFC4A0D1510↓o
```

[2024-10-27 16:22] 🐘: something for cfg

[2024-10-27 16:34] diversenok: As you can guess from the name, `RtlpxLookupFunctionTable` takes a pointer to a module and returns the function table for it

[2024-10-27 16:34] diversenok: So `WhatIsThis` is a function table entry

[2024-10-27 16:36] diversenok: Something like this
```c
PVOID __stdcall RtlPcToFileHeader(PVOID PcValue, PVOID *BaseOfImage)
{
  _INVERTED_FUNCTION_TABLE_ENTRY TableEntry;

  if ( PcValue < LdrpInvertedFunctionTable.TableEntry[0].ImageBase
    || PcValue >= (char *)LdrpInvertedFunctionTable.TableEntry[0].ImageBase
                + LdrpInvertedFunctionTable.TableEntry[0].SizeOfImage )
  {
    RtlpxLookupFunctionTable(PcValue, &TableEntry);
  }
  else
  {
    TableEntry = LdrpInvertedFunctionTable.TableEntry[0];
  }
  *BaseOfImage = TableEntry.ImageBase;
  return TableEntry.ImageBase;
}
```

[2024-10-27 16:39] diversenok: ```c
typedef struct _INVERTED_FUNCTION_TABLE_ENTRY
{
    union
    {
        IMAGE_RUNTIME_FUNCTION_ENTRY* FunctionTable;
        DYNAMIC_FUNCTION_TABLE* DynamicTable;
    };
    PVOID ImageBase;
    ULONG SizeOfImage;
    ULONG SizeOfTable;
} INVERTED_FUNCTION_TABLE_ENTRY, *PINVERTED_FUNCTION_TABLE_ENTRY;
```