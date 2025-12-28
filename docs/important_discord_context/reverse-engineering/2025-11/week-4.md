# November 2025 - Week 4
# Channel: #reverse-engineering
# Messages: 171

[2025-11-18 08:33] guar: can it parse headers like ida does tho?

[2025-11-19 01:58] impl: what is the way to do dynamic taint analysis? do you guys have your own tools?

[2025-11-19 04:54] onyx: Guys, are there any tools to automatically restore what the stack virtual machine handler does? It can be based on symbolic execution…

[2025-11-19 04:56] the horse: have you tried mergen?

[2025-11-19 05:09] onyx: [replying to the horse: "have you tried mergen?"]
?

[2025-11-19 05:10] the horse: https://github.com/NaC-L/Mergen
[Embed: GitHub - NaC-L/Mergen: Deobfuscation via optimization with usage of...]
Deobfuscation via optimization with usage of LLVM IR and parsing assembly. - NaC-L/Mergen

[2025-11-19 05:11] the horse: you can put the vmentry rva as an argument, it will spill out LLVM IR which you can then optimize and compile back to x86 with clang

[2025-11-19 05:11] the horse: usually reducing the handler code pretty good

[2025-11-19 05:27] onyx: I'll try it now

[2025-11-19 05:29] onyx: [replying to the horse: "usually reducing the handler code pretty good"]
Does it remove dead code in the handler or restore semantics?

[2025-11-19 05:30] the horse: it can do both, success varies based on how strong the obfuscation is -- generally if you're trying to recover handler semantics, it should work

[2025-11-19 05:30] the horse: although I'm not entirely sure if there's much success for targeting a specific handler, compared to starting from vmentry through all the handlers of a function?

[2025-11-19 05:31] the horse: can you send me the bin and rva?

[2025-11-19 05:43] onyx: [replying to the horse: "can you send me the bin and rva?"]
You can take a handler in vmprotect ultimate🧐

[2025-11-19 05:44] onyx: As far as I know, it's a powerful enough obfuscator

[2025-11-19 09:14] rin: does anyone know if there is any caveats when searching for `system.load` / `system.libraryload` in android studio. the apk im analyzing has 4 native libraries but the docomplied classes don't seem to have any library loads.

[2025-11-19 09:19] Woah.: Im experimenting with old versions of siege and playing with the engine. Im trying to hook a function that gets called every frame. I have found multiple functions that do this via checking what accesses game manager. But after I hook it will work for a few seconds or a few hundred calls then siege will crash. Im using minhook, any ideas?

[2025-11-19 09:20] the horse: <#1378136917501284443> <@1067372659815481354>

[2025-11-19 09:20] Woah.: Thanks

[2025-11-19 12:39] 0xboga: Say you have a vulnerable bootloader / ability to load a UEFI application, is there a way  to patch / hook somewhere to disable HVCI before the OS starts?

[2025-11-19 13:04] archie_uwu: I think there's an EFI variable that stores whether HVCI should be enabled

[2025-11-19 14:49] BloodBerry: [replying to 0xboga: "Say you have a vulnerable bootloader / ability to ..."]
Check out EfiGuard project.
There is a method in windows bootmgft/efi (windows main loader) that creates config then deletes it

[2025-11-19 14:50] BloodBerry: U can try to patch it via hook of windows boot loader functions

[2025-11-19 14:51] BloodBerry: but be ware cuz it could be easily be detected for ex. AC’s which in kernel mode

[2025-11-19 17:05] elias: [replying to 0xboga: "Say you have a vulnerable bootloader / ability to ..."]
There are many ways, but depending on your goal you might also consider patching skci so you don‘t have to entirely disable hvci

[2025-11-19 17:43] 0xboga: [replying to elias: "There are many ways, but depending on your goal yo..."]
Can you elaborate? Do you mean patching something like the equivalent of g_CiOptions there?

[2025-11-19 17:44] 0xboga: [replying to BloodBerry: "Check out EfiGuard project.
There is a method in w..."]
I did, says it can’t handle HVCI

[2025-11-19 17:54] elias: [replying to 0xboga: "Can you elaborate? Do you mean patching something ..."]
yep

[2025-11-19 17:54] elias: you can just patch the function that verifies signatures to always return success

[2025-11-19 18:33] BloodBerry: [replying to elias: "you can just patch the function that verifies sign..."]
U mean g_CiOptions itself or another function? (If second one — what is name of it?)

Patching first then load sys and revert back I think will cause BSOD, isn’t it?

[2025-11-19 18:34] elias: [replying to BloodBerry: "U mean g_CiOptions itself or another function? (If..."]
another function, I dont remember the exact name rn

[2025-11-19 18:34] elias: and yes

[2025-11-19 18:35] BloodBerry: [replying to elias: "and yes"]
The ez way is to use vulnerable driver

[2025-11-19 18:35] BloodBerry: And map own to memory space

[2025-11-19 18:35] elias: yeah but you cant do that with hvci on

[2025-11-19 18:36] BloodBerry: [replying to elias: "yeah but you cant do that with hvci on"]
I tried and it works lol

[2025-11-19 18:36] BloodBerry: I mean SecureBoot and HVCI on

[2025-11-19 18:36] BloodBerry: [replying to BloodBerry: "I mean SecureBoot and HVCI on"]
Or maybe something was bugged but I mapped driver easily with gdrv.sys (gigabyte driver)

[2025-11-19 18:36] elias: yeah

[2025-11-19 18:37] elias: if HVCI is properly enabled you can't execute your unsigned code in kernel

[2025-11-19 18:37] elias: through vulnerable drivers

[2025-11-19 18:37] BloodBerry: [replying to elias: "if HVCI is properly enabled you can't execute your..."]
I loaded driver somehow and was able to communicate with it

[2025-11-19 18:38] elias: pretty sure that would be a misunderstanding on your side

[2025-11-19 18:38] BloodBerry: Haven’t tested all functions like R/W memory but… it created thread and hello world success

[2025-11-19 18:38] BloodBerry: [replying to elias: "pretty sure that would be a misunderstanding on yo..."]
Maybe…

[2025-11-19 18:39] BloodBerry: I’m new at this stuff, know something round the EfiGuard and a bit of HVCI…

[2025-11-19 18:39] BloodBerry: Will look into it more… thx anyway

[2025-11-19 18:40] elias: the point of hvci is that kernel memory is non executable (and executable memory is non writeable) and executable memory can only be allocated by the secure kernel through hyper v

[2025-11-19 20:11] 0xboga: [replying to elias: "you can just patch the function that verifies sign..."]
I mean I assume I must load before hyper V? When do you exactly patch it in the boot chain?

[2025-11-19 22:10] elias: [replying to 0xboga: "I mean I assume I must load before hyper V? When d..."]
hyper v is loaded by winload so that shouldn't be a problem

[2025-11-19 23:20] Windy Bug: https://tulach.cc/from-firmware-to-vbs-enclave-bootkitting-hyper-v/
[Embed: From firmware to VBS enclave: bootkitting Hyper-V | Samuel Tulach]
Writing a bootkit to manipulate VBS enclave's memory

[2025-11-19 23:21] Windy Bug: I think that’s what you are looking for

[2025-11-20 19:15] Mizzy: Anyone have an idea on how to deobfuscate this, looking to learn and can’t figure this one out
[Attachments: image.png]

[2025-11-20 20:58] Gestalt: [replying to Mizzy: "Anyone have an idea on how to deobfuscate this, lo..."]
Pray to god

[2025-11-20 21:00] ImagineHaxing: Lol

[2025-11-20 21:00] Mizzy: [replying to Gestalt: "Pray to god"]
I know I’ve been praying but nothing seems to be deobfuscating

[2025-11-20 21:00] Gestalt: [replying to Mizzy: "I know I’ve been praying but nothing seems to be d..."]
You need to summon the ByteGuard simplifying horse

[2025-11-20 21:00] Gestalt: well anyways what is your goal?

[2025-11-20 21:03] Mizzy: [replying to Gestalt: "well anyways what is your goal?"]
I honestly just want readable code from this c# app, looking to learn how it does what it does. Thought I could reverse engineer it as a side quest and then learn the code after. Put it in DIE and saw how much protection it has, and in dnSpy and started trying to summon the ByteGuard Simplifying Horse

[2025-11-20 21:04] Gestalt: [replying to Mizzy: "I honestly just want readable code from this c# ap..."]
My naive approach due to a lack of experience reverse engineering c# would just be to use de4dot

[2025-11-20 21:05] Mizzy: [replying to Gestalt: "My naive approach due to a lack of experience reve..."]
I tried using an updated fork of de4dot but just produces errors when trying to deobfuscate it.

[2025-11-20 21:05] Mizzy: That’s what brought me here

[2025-11-20 21:06] Gestalt: ah understandable I wish you good luck

[2025-11-21 15:42] Gerbald: Hi, I'm trying to write a deobfuscator for a shared object I'm trying to reverse engineer. Looking online for how to do it the best I could find was writing rules for d810 (an ida plugin) but d810 uses ida microcode which requires the ida decompiler which takes an unacceptable amount of time to decompile the function because there is a whole lot of dummy code was added by the obufscator. I could patch it manually but it would be extremely annoying and I'd prefer writing some automation because the software I'm trying to reverse engineer gets updated every other day. All of this to ask: does anyone know a good resource that can guide me to writing a deobfuscator? I cannot find anything 🙁

[2025-11-21 16:13] jordan9001: [replying to Gerbald: "Hi, I'm trying to write a deobfuscator for a share..."]
Tim's blog has some good insights on getting started. (or I hear good things about his training)
https://synthesis.to/2021/03/15/control_flow_analysis.html
Start by lifting control flow, either statically or by emulating through the path you care about. (Or just use IDA's, binja's, whatever)
If there is a bunch of dummy code, you can make sure to only follow known good entry points. (Or better yet, follow a concrete execution trace, if you can get one)
Your deobfuscations really depend on the obfuscations in the binary.
If it is a VM you need a way to identify opcode handlers (again, easier with a trace), then you can manually figure out which handlers are what and build tooling as you get used to doing it by hand.
Things like dead code and opaque predicates are defeated by you already taking a trace, or using solvers to prove the branch is dead.
MBA, lift and synthesize. Etc.
If you don't already have a good grasp of what kinds of obfuscations might be in your binary, I think tigress's docs (https://tigress.wtf/control-transforms.html) are great for understanding some common obfuscations and playing with them.
Anyways, GL, sorry for the long reply. Hope you have fun!

[2025-11-21 16:17] Gerbald: [replying to jordan9001: "Tim's blog has some good insights on getting start..."]
thanks! that's really helpful

[2025-11-21 18:31] estrellas: you could also try revng

[2025-11-21 18:31] estrellas: https://www.youtube.com/watch?v=oBfxa9xv24A
[Embed: Deobfuscation with rev.ng]
This video introduces why rev.ng is a great place to perform binary deobfuscation.
In this video we tackle three obfuscation techniques: 1) code mutation + bogus stack traffic, 2) control-flow flatten

[2025-11-21 20:32] Gerbald: [replying to estrellas: "you could also try revng"]
this is *exactly* what I was looking for. Sadly I haven't poked llvm before, I guess a couple of weeks of fun await me. Thanks!

[2025-11-22 13:52] Horsie: Is triton the fastest framework for concolic execution on x86 right now?

[2025-11-22 14:02] Siva: How bypass signature check in android game

[2025-11-22 14:34] the horse: [replying to Horsie: "Is triton the fastest framework for concolic execu..."]
whats the throughput?

[2025-11-22 14:34] Horsie: [replying to the horse: "whats the throughput?"]
as high as possible. Ideally sifting through and emulating interesting parts of a full system recording

[2025-11-22 14:34] Horsie: ~several TBs

[2025-11-22 14:35] Horsie: [replying to the horse: "whats the throughput?"]
throughput right now? I'll measure and let you know. Gimme some time 😔

[2025-11-22 14:35] the horse: do you need full path exploration or initial context can be provided?

[2025-11-22 14:35] Horsie: [replying to the horse: "do you need full path exploration or initial conte..."]
Right now only a single path.

[2025-11-22 14:35] Horsie: But I'm hoping to add exploration

[2025-11-22 14:35] the horse: does it need avx etc?

[2025-11-22 14:36] the horse: and what arch

[2025-11-22 14:36] Horsie: x64

[2025-11-22 14:36] Horsie: AVX not necessary.

[2025-11-22 14:37] the horse: https://github.com/binsnake/KUBERA

my emu should be fastest iirc, but it lacks support for a lot of sse and restricted instructions
[Embed: GitHub - binsnake/KUBERA: A x86_64 software emulator]
A x86_64 software emulator. Contribute to binsnake/KUBERA development by creating an account on GitHub.

[2025-11-22 14:38] the horse: for bigger support i'd do unicorn but its slow

[2025-11-22 14:38] the horse: triton would probably be even slower, but allows concolic exploration

[2025-11-22 14:38] Horsie: I love kubera. been following its progress for a bit

[2025-11-22 14:38] the horse: ❤️

[2025-11-22 14:39] the horse: had to put it on hold for a while

[2025-11-22 14:39] Horsie: But I need symex explicitly :P

[2025-11-22 14:39] the horse: then triton

[2025-11-22 14:39] the horse: but if it's single path then you don't need concolic

[2025-11-22 14:41] Horsie: [replying to the horse: "but if it's single path then you don't need concol..."]
Solving for sat on some funky stuff along the path

[2025-11-22 14:41] the horse: ah im working on something like that

[2025-11-22 14:42] Horsie: Thats the idea for the whole project :)

[2025-11-22 14:42] the horse: for incremental deobf

[2025-11-22 14:42] the horse: run bunch of opt passes, and then z3 for remaining obf

[2025-11-22 14:42] the horse: just need better expr isolation and dependency chains

[2025-11-22 14:43] Horsie: I'd love to pick your brain about how youre doing it whenever youre free (in vc perhaps)

[2025-11-22 14:43] the horse: sure, i'm home in a couple hrs

[2025-11-22 14:43] the horse: but in summary  i lift to my own ir, and solve during jcc/indirect cf

[2025-11-22 14:44] the horse: i run const folding, strength reduction, dce, dbe, algebraic simplification, mem2reg, ...

[2025-11-22 14:44] the horse: atm usually 40% is wiped from a hungarian routine

[2025-11-22 14:44] the horse: rest is semi linear mba

[2025-11-22 14:44] the horse: but z3 takes hours

[2025-11-22 14:45] the horse: need to isolate more efficiently

[2025-11-22 14:45] Horsie: Thats cool as hell dude.

[2025-11-22 14:45] Horsie: Would love to chat. But this is just out of curiosity. I'm pretty out of the loop when it comes to deobf

[2025-11-22 14:46] the horse: i am making a new vmpdragonslayer

[2025-11-22 14:46] the horse: claude maxxing

[2025-11-22 14:46] the horse: but i am reconsidering this IR

[2025-11-22 14:46] the horse: i feel like lifting to LLVM would be miles more efficient

[2025-11-22 14:46] the horse: but for small funcs its probably not worth it

[2025-11-22 14:47] the horse: my ir is very similar to llvm so it shouldnt be that hard to translate

[2025-11-22 14:48] the horse: especially replacing, normalization is just head hurting

[2025-11-22 14:48] avx: horse ir

[2025-11-22 14:48] the horse: haylift

[2025-11-22 14:48] the horse: -> KIRA

[2025-11-22 14:49] Brit: the benefit of llvm is that you have a compiler to rely on

[2025-11-22 14:49] Brit: so it's easy to check if your lifting is correct

[2025-11-22 14:49] Brit: by compiling and comparing the semantics post compile

[2025-11-22 14:49] the horse: yea

[2025-11-22 14:50] Brit: on the other hand

[2025-11-22 14:50] Brit: pita to build llvm

[2025-11-22 14:50] the horse: rn i'm mostly guessing

[2025-11-22 14:50] the horse: yeah my pc

[2025-11-22 14:50] the horse: would take hours

[2025-11-22 14:50] Horsie: bad CPU?

[2025-11-22 14:50] the horse: yup

[2025-11-22 14:50] the horse: i5 9600kf

[2025-11-22 14:50] Brit: even good cpu takes hours

[2025-11-22 14:50] Brit: and fucktons of ram

[2025-11-22 14:50] Brit: ofc

[2025-11-22 14:50] Horsie: I have a couple of threadripper pros. If you want to run something urgent anytime hmu <@901468996229025873>

[2025-11-22 14:50] the horse: i gave my workstation to grandpa

[2025-11-22 14:50] the horse: a long time ago

[2025-11-22 14:50] Brit: which is currently priced very fairly

[2025-11-22 14:51] Horsie: If its just a standalone solver + input

[2025-11-22 14:51] the horse: 5950x to watch news and talk to chat gpt

[2025-11-22 14:51] the horse: 😍

[2025-11-22 14:51] the horse: 2070s for 1080p videos

[2025-11-22 14:51] the horse: 64gb ram for a lot of chrome tabs

[2025-11-22 14:51] the horse: i am on a poor mode atm with shit specs, so my stuff runs fast (not because im broke)

[2025-11-22 14:52] Horsie: Thats a based take man

[2025-11-22 14:52] Horsie: Respect

[2025-11-22 14:52] the horse: i can make educated complaints

[2025-11-22 14:52] the horse: ida takes 5 hrs to analyze 250mb binary

[2025-11-22 14:53] the horse: lifting, obfuscating and rewriting the same bin takes 1,5minutes

[2025-11-22 14:53] the horse: in debug mode..

[2025-11-22 14:54] Horsie: Not that bad

[2025-11-22 15:01] Brit: [replying to the horse: "lifting, obfuscating and rewriting the same bin ta..."]
yeah but what's expensive is drawing all the panels with all the info

[2025-11-22 17:13] the horse: yeah cool so don't redraw it every time when i'm not even in IDA

[2025-11-22 17:14] the horse: search is also atrocious **after analysis is done**

[2025-11-22 17:14] the horse: i think doing headless and making some custom ui for it would be better

[2025-11-22 17:14] the horse: wouldn't help the analysis speed but yea

[2025-11-22 17:27] inflearner: Hey guys, I want to patch ntoskrnl on disk.

Do you know how I can manage to load Windows with a patchednt. right now I am getting  automatic repair screen ?

[2025-11-22 17:29] the horse: disable DSE and integrity checks

[2025-11-23 05:32] rin: [replying to the horse: "i5 9600kf"]
based and poverty pilled

[2025-11-23 10:29] xatat: How can we compare validity of a float value in windows kernel mode ? 
If i want to read a float value  in km by directly reading data in physical memory,
 and try to compare if it is greater than 100.0f or lesser than 0.0f, which method should I employ?

[2025-11-23 12:17] plpg: i dont see why you could not just cast it to float and compare like a float

[2025-11-23 12:17] plpg: is x87 not available in kernel mode for some reason?

[2025-11-23 12:17] plpg: (genuine question, I never touched NT kernel)

[2025-11-23 12:27] xatat: registers need to be explicitly saved and restored before doing floating point calculations in kernel.
I was trying to avoid those calls. I thought there might be some easier way.

[2025-11-23 14:44] plpg: you can implement some softfloat routine if you just need a comparison

[2025-11-23 14:44] plpg: ieee754 is pretty easy

[2025-11-23 14:48] plpg: for greater than/lesser than you can just implement a `float_sub(uintXX_t float1, uintXX_t float2)` and check the sign of the result, it's the top bit

[2025-11-23 14:51] JustMagic: [replying to xatat: "registers need to be explicitly saved and restored..."]
SSE doesn't require saving as it's backed up by default on context switch. You don't need to do anything special.

[2025-11-23 14:53] xatat: So i just read memory as float and i can compare that. correct?

[2025-11-23 14:53] JustMagic: Yes.

[2025-11-23 14:54] 001: if u compile with clang u can do -msoft-float or something like that

[2025-11-23 14:56] JustMagic: Unless you explicitly specify processor arch, clang is also going to just generate SSE which is safe

[2025-11-23 15:00] xatat: Thanks for replies. I am going to try reading it directly .
I was currently using some horrible way of reading it as `FloatIntUnion` and then comparing the int part .