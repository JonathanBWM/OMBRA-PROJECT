# September 2024 - Week 4
# Channel: #programming
# Messages: 66

[2024-09-16 18:38] xAlex | PING = MUTE: yo

[2024-09-16 21:41] Sapphire: [replying to xAlex | PING = MUTE: "yo"]
Yo

[2024-09-17 01:49] daax: [replying to xAlex | PING = MUTE: "yo"]
yo

[2024-09-17 01:54] Bloombit: [replying to daax: "yo"]
Yo

[2024-09-17 02:04] emma: [replying to Bloombit: "Yo"]
7

[2024-09-17 13:37] 0x208D9: procastinating with rust has became my hobby, someone gimme some cool projects so i can finally get over it and brush up the concepts

[2024-09-17 13:48] expy: [replying to 0x208D9: "procastinating with rust has became my hobby, some..."]
LibAFL?

[2024-09-17 13:56] 0x208D9: [replying to expy: "LibAFL?"]
i aint that gigachad to see the wrapper lib yet, suggest me some simple intermediate projects which are interesting

[2024-09-17 14:01] Brit: what constitutes intermediate for you?

[2024-09-17 16:01] Torph: [replying to 0x208D9: "i aint that gigachad to see the wrapper lib yet, s..."]
can't you just point AFL at a dataset and let it start fuzzing your code? I haven't tried it yet, but I was reading through some of the docs the other day and it didnt seem too bad

[2024-09-17 21:43] Delirium Mode: Asa: Okay super annoying, just found out that if your visual studio project exists on a network drive there are some limitations.
I am unable to run in the debugger any code that needs to read/write to a config file that might exist on the network drive with the .exe.

[2024-09-18 05:46] Timmy: I have to shill it

[2024-09-18 05:46] Timmy: https://cmkr.build/ <@184440253648470017>
[Embed: Documentation]
Modern build system based on CMake and TOML.

[2024-09-18 05:50] prick: i am too much of a cmake and msbuild crackhead atp

[2024-09-18 05:50] prick: bootlegged a macos sdk on a windows host

[2024-09-18 05:50] prick: mastered toolchain files

[2024-09-18 08:34] Delirium Mode: Asa: [replying to Timmy: "I have to shill it"]
What do you mean by shill it?

[2024-09-18 08:41] 0x208D9: [replying to Brit: "what constitutes intermediate for you?"]
i mean i can build something like an gameboy emulator which requires reading through the docs and understanding the entire system which is pretty much already reverse engineered, but wanna do something that is much more geared towards rust concepts and data structures in general

[2024-09-18 08:42] 0x208D9: [replying to Torph: "can't you just point AFL at a dataset and let it s..."]
dataset? u mean corpus? i mean thats how fuzzing works, u provide a corpus and it mutates the inputs based on that

[2024-09-18 08:58] Timmy: [replying to Delirium Mode: Asa: "What do you mean by shill it?"]
I love it, it's amazing, <@162611465130475520> built it.

[2024-09-18 08:59] 0x208D9: cmkr more like readable cmake lmao

[2024-09-18 08:59] Timmy: fuck everything about msbuild, cmkr is awesome.

[2024-09-18 09:00] Timmy: cmkr is cmake, without cmake the language

[2024-09-18 09:01] 0x208D9: thats what makes it readable

[2024-09-18 13:57] Torph: [replying to 0x208D9: "dataset? u mean corpus? i mean thats how fuzzing w..."]
yes corpus

[2024-09-19 09:25] Matti: [replying to Delirium Mode: Asa: "Okay super annoying, just found out that if your v..."]
so you're saying this code works fine when invoked from the same location outside of the debugger?

[2024-09-19 09:26] Matti: if so, try changing the default """trust""" settings in VS to be less fascist
[Attachments: image.png]

[2024-09-19 09:26] Matti: but it may also just be an issue with writing to network shares not necessarily related to VS

[2024-09-19 10:00] Windows2000Warrior: <@148095953742725120> hello , please did you know how to compile driver for windows 2000 in my server the community know for 7 and above , and me i have only experience in reactos compiling driver , i want some helpful informations please if you know

[2024-09-19 10:03] Matti: 1. install the relevant SDK and WDK for whatever your target version is (this may require using a different windows version too)
2. write the code
3. compile

[2024-09-19 10:04] Matti: https://my.visualstudio.com/ and https://files.dog/MSDN/ are both good sources for older DDKs

[2024-09-19 10:05] Matti: no idea if 5.0 is among them but chances are pretty good

[2024-09-19 10:09] Windows2000Warrior: [replying to Matti: "1. install the relevant SDK and WDK for whatever y..."]
ok thanks

[2024-09-19 10:13] x86matthew: windows 2000 DDK and visual c++ 6.0

[2024-09-19 10:13] x86matthew: was the setup i used

[2024-09-19 10:14] x86matthew: you can also use the win server 2003 ddk, it includes a predefined win2k-specific build env

[2024-09-19 10:14] x86matthew: and is probably easier to find

[2024-09-19 10:14] Matti: yeah they always cover a range of versions

[2024-09-19 10:14] Matti: going back a few

[2024-09-19 10:15] Matti: I don't think you technically need VS with the old style DDKs as they included their own compiler toolchains

[2024-09-19 10:15] Matti: it's just more... convenient

[2024-09-19 10:16] Matti: personally I just modify whatever the latest SDK is to add support for older targets back in, but this is horrible work

[2024-09-19 10:17] x86matthew: yeah, win2k ddk needed msvc++ 6.0, winxp/2k3 ddk didn't require it iirc

[2024-09-19 10:17] Matti: ah interesting

[2024-09-19 10:17] Matti: yeah I've never used the former, but the latter for sure didn't need it

[2024-09-19 10:17] Matti: idem dito vista and win 7 DDKs

[2024-09-19 10:17] Rairii: i've personally just shoved libs and headers in from older wdk lol

[2024-09-19 10:17] Rairii: and a small batch file for buildscript

[2024-09-19 10:17] Rairii: of course, for nt ppc thingss are different

[2024-09-19 10:18] x86matthew: `The Microsoft® Windows® DDK ships a complete set of tools for building drivers. These tools have been upgraded from those released with the Windows XP DDK. As in the Windows XP DDK, Microsoft Visual C++® is no longer required to be installed.`

[2024-09-19 10:19] x86matthew: yeah looks like it changed with xp ddk

[2024-09-19 10:19] Matti: and they changed it back for windows 8 <:lillullmoa:475778601141403648>

[2024-09-19 10:19] Matti: what a failure

[2024-09-19 10:19] Windows2000Warrior: ok thanks all for informations

[2024-09-19 10:20] Matti: at least now there's the EWDK

[2024-09-19 17:02] acheron: Any tips on writing a licensing system for a 2-3 out of 10 difficulty crackme?
Currently the plan is to take some info like username, email, license type, expiry and start date, and some unique string that uses the credit card # algorithm for verification, then separate all that with some delimiter and base64-encode the whole thing

Is that too much, or not enough for an ~hour long intro to reverse engineering walkthrough? The goal is to walk the people through patching out the number verification, and get them to use cyberchef to generate the rest of the license themselves
I'm not planning on stripping symbols or doing any sort of obfuscation

[2024-09-19 17:06] Redhpm: those people who you will teach, what level do they have ?

[2024-09-19 17:07] Redhpm: i'd say it's ok for people who have a normal understanding of C and their system

[2024-09-19 17:07] Redhpm: and maybe have already touched a bit some RE software

[2024-09-19 17:08] acheron: I'm assuming that they know a bit of python and java

[2024-09-19 17:08] acheron: I'll probably get them using binja, since it's way less intimidating than ida or ghidra

[2024-09-19 17:09] acheron: [replying to Redhpm: "i'd say it's ok for people who have a normal under..."]
I was hoping that leaving the symbols would be enough to get people who don't know C going at least

[2024-09-19 17:12] Redhpm: [replying to acheron: "I was hoping that leaving the symbols would be eno..."]
I mean, from my experience teaching, it takes a bit of time to explain the stack, some terminology, why you want a decompiler in the first place (also what can the tools do and what they do not).

[2024-09-19 17:13] Redhpm: sure they will be able to read, but I think if you want the knowledge to stick, they have to grasp some little understanding of how the program runs

[2024-09-19 17:13] Redhpm: not much

[2024-09-19 17:20] acheron: Eh, it's mostly to give people a taste of RE and a sense of accomplishment, get them the dopamine hit of breaking the program