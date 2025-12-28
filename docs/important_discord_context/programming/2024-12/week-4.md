# December 2024 - Week 4
# Channel: #programming
# Messages: 126

[2024-12-16 14:10] BWA RBX: [replying to stefan: "anyone have an idea of what kinda of data structur..."]
Xor linked list <:Kappa:332115433337520130>

[2024-12-16 15:12] stefan: [replying to BWA RBX: "Xor linked list <:Kappa:332115433337520130>"]
Its a singly linked list haha

[2024-12-16 15:34] BWA RBX: Nah I was just messing 😂

[2024-12-16 16:17] BWA RBX: Executable code isn't always in the .text section header, you'll have to look at the permissions of the PE files sections to know which sections are executable etc

[2024-12-16 16:18] Brit: it could even reprotect at runtime

[2024-12-16 16:19] BWA RBX: Yes then you could use a tool to locate the memory address where the  .text section starts and ends

[2024-12-16 16:20] BWA RBX: What

[2024-12-16 16:21] BWA RBX: Do you have a tool that you can use to analyse the PE File

[2024-12-16 16:21] Brit: if he's talking fseek, and has some section header, presumably he's parsing the PE somwhat

[2024-12-16 16:22] BWA RBX: Okay then you'll want to look into the Windows API to find out how to parse those headers

[2024-12-16 16:22] BWA RBX: Okay so you didn't say that

[2024-12-16 16:23] BWA RBX: You didn't mention anything about patching at all

[2024-12-16 16:24] Brit: if you're parseing the section headers, they should have a pointer to raw data

[2024-12-16 16:24] Brit: and raw size

[2024-12-16 16:24] snowua: https://github.com/can1357/linux-pe

<@456226577798135808> brother, whatever you are doing you’re doing it in the worst way possible. load your pe into memory and use a parser
[Embed: GitHub - can1357/linux-pe: COFF and Portable Executable format desc...]
COFF and Portable Executable format described using standard C++ with no dependencies. - can1357/linux-pe

[2024-12-16 16:24] snowua: problem solved

[2024-12-16 16:26] diversenok: Okay, so you know how PE files have different layout as files and in memory?

[2024-12-16 16:27] Brit: the math goes something like filebase + section->ptrtorawdata for the beginning , and the end of the section is the same + sizeofraw

[2024-12-16 16:27] Brit: if we're talking on disk

[2024-12-16 16:28] x86matthew: BaseOfCode  is irrelevant

[2024-12-16 16:30] Brit: but I mean this is such an X to Y problem moment, if you're trying to statically patch a file on disk, why bother with any of this, just pattern match whatever the fuck you wanna patch and do it at wherever you found it

[2024-12-16 16:30] Brit: there's no reason to do all this work

[2024-12-16 16:30] BWA RBX: Also I would suggest also looking at open sourced projects that do what you want to do, that way you can see how they do it

[2024-12-16 16:33] BWA RBX: Bytes

[2024-12-16 16:33] Brit: yes but you throw the file into memory, then search through it with the pattern you care about, given it is unique (which shouldn't be too hard), you modifier your buffer in memory with your patch and then write it to disk

[2024-12-16 16:34] BWA RBX: I would search for the sequence of bytes and patch it that way statically

[2024-12-16 16:34] BWA RBX: Inb4 HxD moment 😂

[2024-12-16 16:38] snowua: [replying to Brit: "yes but you throw the file into memory, then searc..."]
but if it’s not unique (and likely no way to guarantee that on an arbitrary executable) you go back to the same pe parsing problem. and the only way to guarantee you’re patching actual code is to parse the pe and check that you’re in an executable section

[2024-12-16 16:38] snowua: its really really not that hard

[2024-12-16 16:40] snowua: just load your bin into memory as suggested and use linuxpe. it quite literally 10 lines of code. that is if you don’t care about learning the pe format

[2024-12-16 16:40] Brit: [replying to snowua: "but if it’s not unique (and likely no way to guara..."]
I can't really think of a problem where I need to patch things on disk but also it's a common enough seq that it might collide and also it's an arb binary

[2024-12-16 16:41] Brit: I sort of assumed he re'd something, figured out something to patch, and doesn't want to have to direct other people through hex editing and just wants to ship a patcher program

[2024-12-16 16:41] Brit: but yeah, linuxpe is blessed

[2024-12-16 16:42] snowua: yeah that’s what i was wondering. sounds like he’s talking about patching some address he’s aware of using code, at which point you can do it by hand

[2024-12-16 16:42] snowua: confused

[2024-12-16 16:42] Brit: that, or it's some seq across multiple versions of some binary

[2024-12-16 16:42] Brit: and he doesn't want to manually find it each time

[2024-12-16 16:42] Brit: (we don't pirate software :^) )

[2024-12-16 16:43] BWA RBX: If he wants to patch code within the text section surely he knows what he wants to patch, I just think it's quite literally an import re job and patching it by searching the sequence of bytes and replacing them with something else if there's only 1 match

[2024-12-16 16:43] Brit: [replying to snowua: "but if it’s not unique (and likely no way to guara..."]
we always be stanning Can code

[2024-12-16 16:43] Brit: <:topkek:904522829616263178>

[2024-12-16 16:44] snowua: linuxpe is one of the few libraries i can actually advocate for using

[2024-12-16 16:45] snowua: you’re pretty much going to end up writing the same thing with equal or more overhead. with probably worse code quality

[2024-12-16 16:45] snowua: ¯\_(ツ)_/¯

[2024-12-16 16:55] diversenok: They could use more range checks to make sure they don't read beyond the end of the file

[2024-12-16 17:44] Hunter: convert the rvas that are given in the executable file format to file offsets then seek to that

[2024-12-16 22:01] pinefin: 
[Attachments: linus-doesnt-beat-around-the-bush-v0-lmqn8oc7ztda1.png]

[2024-12-16 22:01] pinefin: linus is king

[2024-12-16 22:02] pinefin: signatured and everything LOL

[2024-12-17 03:06] durandal: [replying to pinefin: ""]
haha wasn't this the one that got linus in a bit of hot water?

[2024-12-17 03:06] durandal: cause i remember one he tore into some contributor and she whined to the kernel.org team about how it was because she was a woman

[2024-12-17 10:33] avx: [replying to Brit: "we always be stanning Can code"]
real af

[2024-12-17 17:59] Torph: [replying to pinefin: ""]
<:ubernut:897469891358097409> <:ubernut:897469891358097409>

[2024-12-17 18:00] Torph: based

[2024-12-17 21:06] Xits: Does anyone know a good way to do thunks in c++ without raw assembly trampolines?

[2024-12-17 21:07] Xits: By thunk I mean converting a capture lambda or class method to be C callback compatible

[2024-12-17 21:08] Xits: It’s easy for single instances with global variables but thats not scalable

[2024-12-18 11:22] ether: <@162611465130475520> 

 I've been looking at your https://github.com/build-cpp/vcpkg_template project and had a question about using static versions of the vcpkg packages so i can compile with /MT. 

Specifically, I'm wondering how I can make it reference my global vcpkg install rather than the manifest so I can reuse the packages in other projects as well. Do you have any tips on how to set that up? Im specifically using Visual Studio 2022 with its built in CMake support to modify and build the project.
[Embed: GitHub - build-cpp/vcpkg_template: Simple cmkr template to get you ...]
Simple cmkr template to get you started with vcpkg right away. - build-cpp/vcpkg_template

[2024-12-18 11:52] mrexodia: [replying to ether: "<@162611465130475520> 

 I've been looking at your..."]
> how I can make it reference my global vcpkg install rather than the manifest so I can reuse the packages in other projects as well

[2024-12-18 11:53] mrexodia: This is incorrect and you should never do it.

[2024-12-18 11:53] mrexodia: Your cache will prevent unnecessary rebuilds already and different projects can point to different vcpkg versions, sharing a vcpkg install is a dumb hack they should have never introduced in the first place

[2024-12-18 11:54] mrexodia: You can modify `vcpkg-overlay/x64-windows.cmake` to contain:
```cmake
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE static)
set(VCPKG_LIBRARY_LINKAGE static)
```
This will compile all the libraries with static CRT linkage (which has terrible support across the board, just FYI)

[2024-12-18 11:56] mrexodia: (also the builtin CMake support from Visual Studio is awful too and you're better off using the solution generator in the majority of cases)

[2024-12-18 14:13] ether: Thanks for the tips

[2024-12-18 14:13] ether: [replying to mrexodia: "(also the builtin CMake support from Visual Studio..."]
Could you go into detail a bit? I find it to be quite good besides the vcpkg stuff

[2024-12-18 14:13] ether: I've been using solutions forever, and I wanted to get into CMake

[2024-12-18 14:13] ether: I just dont know the best way to get what I want done

[2024-12-18 14:14] ether: I was using FetchContent before but I wanted to move to vcpkg

[2024-12-18 14:16] mrexodia: [replying to ether: "I've been using solutions forever, and I wanted to..."]
CMake can generate the solution for you and everything just works

[2024-12-18 14:17] mrexodia: I just never liked the CMake integration VS has, always bugs out and they have their weird non-standard json files etc

[2024-12-18 14:17] mrexodia: But if it works for you 🤷‍♂️

[2024-12-18 14:17] ether: [replying to mrexodia: "CMake can generate the solution for you and everyt..."]
Thats the thing, I find it to be quite bloated and I don't like dealing with slns regardless haha

[2024-12-18 14:18] mrexodia: Might be bloated, but at least it works 😂

[2024-12-18 14:18] BWA RBX: [replying to mrexodia: "Might be bloated, but at least it works 😂"]
Spoken like a true project manager

[2024-12-18 14:40] Torph: [replying to mrexodia: "(also the builtin CMake support from Visual Studio..."]
what's wrong with it? I've been on CLion for a few years now, but when I was on VS2022 it seemed to be mostly fine with CMake

[2024-12-18 14:41] Torph: [replying to mrexodia: "I just never liked the CMake integration VS has, a..."]
oh that's true those are annoying

[2024-12-18 14:54] mrexodia: [replying to Torph: "what's wrong with it? I've been on CLion for a few..."]
CLion is a different IDE

[2024-12-18 15:39] Torph: [replying to mrexodia: "CLion is a different IDE"]
yeah ik, I'm saying when I used to use CMake with VS it seemed fine (not on CLion's level but not awful)

[2024-12-18 15:53] Torph: lol I'm using CMake so I can have 1 build script for Windows & Linux

[2024-12-18 15:57] Yoran: [replying to Torph: "lol I'm using CMake so I can have 1 build script f..."]
Nice.
Cool trick which **I** find useful:
If you have git on your windows machine you can create a build.sh and use the git (guess its using MinGW or something like that) terminal to run it. On Linux and Mac bash runs perfectly fine.

[2024-12-18 15:58] pinefin: vscode with cmake > vs with cmake

[2024-12-18 15:58] pinefin: trust

[2024-12-18 15:59] pinefin: vs is just overkill and its developed around their toolchains

[2024-12-18 15:59] pinefin: i use it for embedded/hardware shit

[2024-12-18 15:59] pinefin: on vscode

[2024-12-18 15:59] pinefin: the cmake extension is actually very nice

[2024-12-18 16:00] pinefin: these extensions

[2024-12-18 16:00] pinefin: yes yes

[2024-12-18 16:00] pinefin: 
[Attachments: Screenshot_2024-12-18_at_10.00.25_AM.png]

[2024-12-18 16:00] pinefin: is all i need

[2024-12-18 16:01] pinefin: though, i dont have to make actual CMakeLists.txt because theyre generated for me from silicon labs

[2024-12-18 16:01] pinefin: i just make the application code CMakeLists which just basically consists of

[2024-12-18 16:02] pinefin: ```cmake
add_library
add_subdirectory
target_sources
target_include_directories
target_link_libraries
```

[2024-12-18 16:02] pinefin: other than that, its pretty simple to use

[2024-12-18 16:03] pinefin: ive used it on all my mac projects

[2024-12-18 16:03] pinefin: anyways

[2024-12-18 16:03] pinefin: back to what i was saying

[2024-12-18 16:04] pinefin: **vs** isnt made to use cmake, __its made to support it__

**vscode** wasnt made to use cmake, __but it was made to be flexible enough to make it seem liek it was made to use cmake__

[2024-12-18 16:04] pinefin: at least how i see it

[2024-12-18 16:12] mrexodia: https://cmkr.build <:yumshot:1289734272579665950>
[Embed: Documentation]
Modern build system based on CMake and TOML.

[2024-12-18 16:13] Timmy: vsc*de <a:pepepuke:834048872753790999>

[2024-12-18 16:20] Torph: [replying to Yoran: "Nice.
Cool trick which **I** find useful:
If you h..."]
i might try that, I kinda forgot git bash was a thing. there are a few projects where I really do need different behaviour on MSVC in the build system, but for everything else that should work fine

[2024-12-18 16:22] Yoran: [replying to Torph: "i might try that, I kinda forgot git bash was a th..."]
What does different behaviour mean?

[2024-12-18 16:23] Yoran: I suspect you can pass a flag...:
`build.sh msvc release`

[2024-12-18 16:33] Torph: sometimes I use a library to embed binary files that uses an `.incbin` assembler macro that MSVC doesn't support, so I have to use a workaround by auto-generating a header full of byte literals

[2024-12-18 16:33] Torph: [replying to Yoran: "I suspect you can pass a flag...:
`build.sh msvc r..."]
oh yeah that'd work

[2024-12-18 16:34] Torph: for literally everything else though the platform macros are enough

[2024-12-19 04:41] ChloeOS: pretty simple example of a SCSI read command, used in part of a project ive been working on
[Attachments: message.cpp]

[2024-12-19 17:56] Horsie: I wonder how that would work.

[2024-12-19 17:57] Horsie: If the scheduler uses the same process list to assign time slices then it will just stop executing after a bit?

[2024-12-19 18:09] Windy Bug: CSRSS handles, PspCidTable

[2024-12-19 20:53] Brit: try it and tell me the funny bsod screen you get

[2024-12-19 21:47] iPower: [replying to Windy Bug: "CSRSS handles, PspCidTable"]
don't forget about handle tables from all processes as well

[2024-12-19 21:53] diversenok: And maybe you can do some kind of pattern matching to find unlinked EPROCESS objects

[2024-12-19 22:11] elias: You could probably iterate global handle table and check if there are open handles that belong to processes that shouldnt exist

[2024-12-19 22:16] .: [replying to Horsie: "If the scheduler uses the same process list to ass..."]
well the windows scheduler does not directly depend on the ActiveProcessLinks list to schedule processes, but certain kernel operations (like process enumeration or termination) yea, so they would have to like patch those functions to prevent dumb behavior

[2024-12-19 22:17] .: i dont remember exactly if the scheduler can still access the KTHREAD structures of a hidden process independently of ActiveProcessLinks?

[2024-12-19 22:19] .: [replying to elias: "You could probably iterate global handle table and..."]
or like enumerate all active kernel timers and analyze their callbacks to see if a timer references code within a hidden EPROCESS

[2024-12-19 22:21] .: the best bet is prob is to enumerate ETHREADs directly and cross-reference each thread’s ETHREAD->EPROCESS pointer with the known process list

[2024-12-19 22:23] .: so you could enumerate all processors in the system, access the KPRCB for each processor and traverse the thread queues (ReadyQueue, DispatcherReadyList) to locate all ETHREAD structures

then you compare enumerated threads against threads linked to known processes (EPROCESS->ThreadListHead) and you would just need to look for threads whose parent processes are unlinked or not visible in ActiveProcessLinks

[2024-12-19 22:26] .: and i'd assume it would be better than getting process handles as they can be closed without any impact

[2024-12-20 04:43] daax: could always iterate the KiDispatcherReadyListHead/KiWaitListHead - or use swapcontext hook and build a list of processes that way

[2024-12-20 04:44] daax: easy enough and no hassle with scanning bs, or do all of them and have enough information to deduce an active process list with high confidence

[2024-12-21 22:29] eversinc33: https://eversinc33.com/posts/anti-anti-rootkit-part-ii.html 
I wrote about my shitty approach here (see 'detection 2' part)
[Embed: (Anti-)Anti-Rootkit Techniques - Part II: Stomped Drivers and Hidde...]
Detecting driver 'stomping' and hiding system threads by manipulating the PspCidTable.

[2024-12-21 22:29] eversinc33: at least detects pspcidtable removal

[2024-12-22 08:23] 冰: https://learn.microsoft.com/en-us/windows/win32/etw/cswitch
[Embed: CSwitch class - Win32 apps]
This class is the event type class for context switch events. The following syntax is simplified from MOF code.