# April 2025 - Week 5
# Channel: #reverse-engineering
# Messages: 347

[2025-04-28 00:54] nukinton: currently reversing winload.efi and trying to find a sig for BlImgAllocateImageBuffer, i tried binary search but cant find the usual sequence, any ideas?

[2025-04-28 02:32] UJ: [replying to nukinton: "currently reversing winload.efi and trying to find..."]
I just opened winload.efi in ida, found BlImgAllocateImageBuffer export and generated the sig `48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC ? 48 83 65 ? ? 41 8B F0` for it. Is that what you are referring to or something else? i havent messed with efi in depth (yet).

[2025-04-28 02:33] pinefin: [replying to the horse: "qemu for example, you run it in a virtual machine ..."]
bochs is good, i haven’t played around with it and i do t know if theres a sdk that you can include and play with…i’ve also not set it up with win10/11 yet, dont know if theres anything specific you have to do

[2025-04-28 02:33] pinefin: i’ve tried to use it for debugging but only barely got it to boot win98

[2025-04-28 02:37] UJ: [replying to pinefin: "bochs is good, i haven’t played around with it and..."]
all i want for christmas is something with an intel pin tool like api/feature set but running at the hypervisor level and doesn't mess with the asm in the process/rewrite it at all.

[2025-04-28 02:56] nukinton: [replying to UJ: "I just opened winload.efi in ida, found BlImgAlloc..."]
holy shit thank you

[2025-04-28 02:57] UJ: [replying to nukinton: "holy shit thank you"]
verify this is what you are actually looking for since this felt too easy, the pdb is available for winload.efi no?

[2025-04-28 02:58] KIZZY: Yeah the pdb should be available

[2025-04-28 02:58] nukinton: [replying to UJ: "verify this is what you are actually looking for s..."]
didnt check that far, just opened it, tried binary search, no luck, so decided to ask for another work around

[2025-04-28 02:58] nukinton: [replying to KIZZY: "Yeah the pdb should be available"]
oh good your online

[2025-04-28 02:59] nukinton: I get side tracked very easily so once something doesnt work i skip it and move onto something else, im a lazy pos ngl but still try to do my fair share

[2025-04-28 03:24] daax: [replying to nukinton: "I get side tracked very easily so once something d..."]
you barely did any share

[2025-04-28 03:25] nukinton: [replying to daax: "you barely did any share"]
your right, i didnt in here

[2025-04-28 04:40] abu: [replying to toro: "Anyone here have experience with kace and speakeas..."]
Focus on individual functions instead of the entire driver.

[2025-04-28 05:18] KIZZY: so when it comes to messing with packets how should I go about actually decrypting the data being sent. I am messing with a couple cames rn one of the eso. I have tried looking everywhere for a place I can hook before or after its decrypted and not finding anything. I am new to this realm. I dont need spoon feeding just like some things to look for.

[2025-04-28 10:03] sync: [replying to nukinton: "didnt check that far, just opened it, tried binary..."]
https://media0.giphy.com/media/l55ShMh3JSV7chmf9T/giphy.gif

[2025-04-28 12:05] nukinton: [replying to sync: "https://media0.giphy.com/media/l55ShMh3JSV7chmf9T/..."]
used your spoofer didnt work

[2025-04-28 12:05] nukinton: still banned on jerkmate

[2025-04-28 12:07] sync: get scammed bro <:evilplan:1247897662062071909>

[2025-04-28 12:10] iPower: can you stop the shitposting this isn't your average UC post

[2025-04-28 12:10] sync: my initial gif wasnt a shitpost

[2025-04-28 12:10] iPower: and if you post that image again I'm timing you out <@1079485771154735174>

[2025-04-28 12:10] sync: just facts

[2025-04-28 12:11] nukinton: [replying to iPower: "and if you post that image again I'm timing you ou..."]
WHAT 👉👈 nervous asf rn

[2025-04-28 12:11] iPower: stop. the. shitposting. last warning

[2025-04-28 14:31] BlitzByte: so how difficult is using Ghidra? I got an application that's in Chinese that I want to try and pull apart and reimplement. It's using Qt6 for the UI and what looks like some common dlls (winpthread, stdc++)

[2025-04-28 15:53] daax: [replying to BlitzByte: "so how difficult is using Ghidra? I got an applica..."]
All RE tools have a learning curve, and getting familiar with their pseudo and quirks. I don’t use ghidra other than for embedded architectures, so can’t say for large complex projects, but it should be able to do what every other tool does — you may have to write some plugins if you want specific functionality / bindings though.

Someone who uses it as a daily driver for all RE might offer better insight. For embedded RE, it’s perfectly suitable; but no idea for what you’re doing.

[2025-04-28 16:50] mrexodia: [replying to the horse: "```
adc: 24 failed 127236 passed
add: ALL PASSING
..."]
what did you use to verify it?

[2025-04-28 16:59] the horse: https://github.com/LLVMParty/x86Tester-Results
[Embed: GitHub - LLVMParty/x86Tester-Results: Contains pre-generated result...]
Contains pre-generated results for ZehMatt's x86Tester - LLVMParty/x86Tester-Results

[2025-04-28 17:01] the horse: 
[Attachments: image.png]

[2025-04-28 17:01] the horse: <@162611465130475520>

[2025-04-28 17:03] the horse: it's a horrendous format though

[2025-04-28 17:03] the horse: the generator needs rewriting

[2025-04-28 17:51] mrexodia: Ah nice!

[2025-04-28 21:08] DirtySecreT: [replying to the horse: ""]
what instructions / extensions are you supporting?

[2025-04-28 21:09] the horse: no extensions except basic SSE currently

[2025-04-28 21:09] the horse: slowly building it up

[2025-04-28 21:10] the horse: but most common x86 instructions are supported, and I think I haven't seen one that's missing when executing VMP/Themida binaries

[2025-04-28 21:10] the horse: where they substitute for a bunch of random stuff

[2025-04-28 21:10] the horse: I plan to extend support for AVX -> AVX2 -> ...

[2025-04-28 21:10] the horse: oh and BMI is supported

[2025-04-28 21:10] the horse: partially at least

[2025-04-28 21:11] the horse: bextr, popcnt, ..

[2025-04-28 21:11] the horse: SHLx SHRx SARx RORx BZHI

[2025-04-28 21:11] the horse: and all those

[2025-04-28 21:12] the horse: nothing runs natively and it has pretty good throughput even in debug (1.3mil instructions/s)

[2025-04-28 21:12] the horse: on a i5-9600kf

[2025-04-28 21:13] the horse: currently winapi has an abstraction layer that just vmexits and vmenters post-return; but I plan to do something similar to momo's emulator and just emulate at syscall level; so vmexit almost never happens unless we meet an unsupported syscall

[2025-04-28 21:13] DirtySecreT: [replying to the horse: "nothing runs natively and it has pretty good throu..."]
our definitions of good throughput might be different haha but nice.

[2025-04-28 21:13] the horse: but other OS stuff is more of a PITA to implement

[2025-04-28 21:14] the horse: just using better optimized structures would speed that up a lot 🙂

[2025-04-28 21:15] the horse: some things will have to be a bit slower; memory mappings especially

[2025-04-28 21:16] DirtySecreT: are you planning to support the use of bugs in specific generations?

[2025-04-28 21:16] contificate: [replying to the horse: ""]
structured bindings eta

[2025-04-28 21:16] the horse: if someone would wish to PR them after the open-source release

[2025-04-28 21:16] the horse: i'd be open to that

[2025-04-28 21:17] DirtySecreT: ah it's open source

[2025-04-28 21:17] the horse: currently not

[2025-04-28 21:17] DirtySecreT: i would be interested in

[2025-04-28 21:17] DirtySecreT: oh alr

[2025-04-28 21:17] DirtySecreT: why's that?

[2025-04-28 21:17] the horse: I like to keep things closed until they're not messy 😄

[2025-04-28 21:18] the horse: also need to isolate some parts from other projects its embedded in

[2025-04-28 21:18] the horse: I'm kind of cut on open-sourcing my stuff; not sure to what extent it might be bad for me as a lot of the stuff is planned to be used commercially as part of something bigger

[2025-04-28 21:19] DirtySecreT: mmm

[2025-04-28 21:19] DirtySecreT: sounds familiar

[2025-04-28 21:19] DirtySecreT: lol

[2025-04-28 21:20] the horse: regarding the generation-thingies;

might be pretty nice to do, currently cpuid is passed through host, but we could capture it for the specific cpu (also make sure it would match it everywhere else, cores, threads, ...) and overload handlers based on it

[2025-04-28 21:20] the horse: i'm sure there's a lot of quirks between amd & intel as well

[2025-04-28 21:20] the horse: i'm currently testing against a threadripper since that's the only public data i've got

[2025-04-28 21:20] the horse: and after 3 days trying to generate it on a pentium, it kind of went nowhere

[2025-04-28 21:21] DirtySecreT: well im not referring to cpuid usage to determine. u can use other means to determine what generation / microarchitecture ur operating on that don't involve cpuid

[2025-04-28 21:21] the horse: even with most avx stuff stripped it just takes days because of the slow debugger they use to capture data

[2025-04-28 21:22] the horse: [replying to contificate: "structured bindings eta"]
meh that little shit is just for tests

[2025-04-28 21:22] the horse: don't care if it's ugly

[2025-04-28 21:22] DirtySecreT: but it would be nice to support. if you put it open source, i'll be happy to contribute where necessary

[2025-04-28 21:23] the horse: sounds good man

[2025-04-28 21:23] DirtySecreT: [replying to the horse: "I'm kind of cut on open-sourcing my stuff; not sur..."]
also i get what you mean here but we open sourced our pcie core and it has been a blessing even though we dont get paid directly from it

[2025-04-28 21:23] DirtySecreT: lots of other opportunities come from it being open sourced and having worked on it

[2025-04-28 21:24] DirtySecreT: people use it for all sorts of things but we get paid in different ways from our contributions

[2025-04-28 21:24] the horse: i guess in most cases you can do the same with unicorn what you'd do with my stuff, even though it''s supposed to have a friendlier set up for direct binary emulation

[2025-04-28 21:24] the horse: but i find unicorn extremely ugly

[2025-04-28 21:24] DirtySecreT: u dont need unicorn though

[2025-04-28 21:24] DirtySecreT: unicorn is crap

[2025-04-28 21:25] the horse: + all of the public emus have some weakpoints that get abused to prevent emulation and change program behaviour because of it to hide true control flow

[2025-04-28 21:25] DirtySecreT: what i was saying was just food for thought. not everything has to be gate kept. an emulator is a dime a dozen, no offense. if its well designed then community contribution is much easier to handle.

[2025-04-28 21:26] the horse: i've gotten some really good help from open source community; showing me the github CIs and all, might be pretty good to get some help

[2025-04-28 21:28] DirtySecreT: yeah. project recognition goes a long way too. anyway itll be cool to see where it goes. you might also look into simics and see how they are handling things too.

[2025-04-29 00:46] VMMX: hello im trying to build a disassembler im currently using lief and capstone facing some issues with disassembling obfuscated exe any advice (better way to disassemble other then linear sweep disassembler and 
 recursive traversal disassembler) ?

[2025-04-29 05:19] the horse: for anti-disassembly tricks you'd have to pattern match probably

[2025-04-29 05:50] UJ: bro

[2025-04-29 05:51] UJ: https://github.com/Microsoft/llvm-mctoll#:~:text=SIMD%20instructions%20such%20as%20SSE%2C%20AVX%2C%20and%20Neon%20cannot%20be%20raised%20at%20this%20time.%20For%20X86%2D64%20you%20can%20sometimes%20work%20around%20this%20issue%20by%20compiling%20the%20binary%20to%20raise%20with%20SSE%20disabled%20(clang%20%2Dmno%2Dsse).

> SIMD instructions such as SSE, AVX, and Neon cannot be raised at this time. For X86-64 you can sometimes work around this issue by compiling the binary to raise with SSE disabled (clang -mno-sse).

compiling the binary to raise

[2025-04-29 14:19] BlitzByte: [replying to daax: "All RE tools have a learning curve, and getting fa..."]
I'm trying to decompile an application that only has chinese characters, and most likely written in chinese, so I can RE it and reimplement the things it does in a more unified manner (like proper translation support/Linux support/other niceties)

[2025-04-29 18:58] DirtySecreT: [replying to BlitzByte: "I'm trying to decompile an application that only h..."]
u know... u might be okay to use one of those ai reverse engineering sets like what <@162611465130475520> released with the mcp server + ida and have it translate those strings for u. maybe u could build a little python thing to query the google translate api and have it convert it then u can write a patcher to replace them

[2025-04-29 18:58] DirtySecreT: thats kind of fun. im going to try this

[2025-04-29 19:00] pinefin: [replying to DirtySecreT: "u know... u might be okay to use one of those ai r..."]
did u see the stream where it literally reversed a whole binary and named \*most* functions in a correct manner

[2025-04-29 19:00] pinefin: lowkey will become a vibe reverser some day

[2025-04-29 19:00] DirtySecreT: [replying to pinefin: "did u see the stream where it literally reversed a..."]
i didnt

[2025-04-29 19:00] DirtySecreT: i dont really watch yt live or anything

[2025-04-29 19:00] pinefin: i watched the replay of it

[2025-04-29 19:00] pinefin: let me try and find it

[2025-04-29 19:00] pinefin: it was in oa

[2025-04-29 19:00] DirtySecreT: yeah share it here would be cool to watch

[2025-04-29 19:01] pinefin: https://discord.com/channels/885624530071085097/913166771916271706/1355577425622208544

[2025-04-29 19:01] pinefin: he gives the statistics here

[2025-04-29 19:01] DirtySecreT: thks

[2025-04-29 19:01] pinefin: 335kb costed $90 to annotate 80% of it

[2025-04-29 19:01] DirtySecreT: oh

[2025-04-29 19:01] pinefin: insane

[2025-04-29 19:02] pinefin: its super expensive for the time being

[2025-04-29 19:02] DirtySecreT: im gonna i need to find a tool thats mostly chinese text and not malware

[2025-04-29 19:02] DirtySecreT: i want to try this little experiment

[2025-04-29 19:02] DirtySecreT: any thoughts?

[2025-04-29 19:08] mrexodia: [replying to pinefin: "its super expensive for the time being"]
I mean this is pretty cheap actually

[2025-04-29 19:11] daax: [replying to DirtySecreT: "any thoughts?"]
you can always go the other direction. english to alt language if it supports utf8

[2025-04-29 19:11] mrexodia: Perhaps a bit more expensive than a junior analyst, but that's 187k/year if that person works full time for 90/h

[2025-04-29 19:11] pinefin: [replying to mrexodia: "Perhaps a bit more expensive than a junior analyst..."]
ok when u think about it this way i guess you're right

[2025-04-29 19:12] DirtySecreT: [replying to daax: "you can always go the other direction. english to ..."]
was thinking this to be the case. have you done this before?

[2025-04-29 19:12] pinefin: how long did that whole binary take?

[2025-04-29 19:12] mrexodia: 20min?

[2025-04-29 19:12] mrexodia: I don't remember honestly, we were just messing around

[2025-04-29 19:12] pinefin: oh lord

[2025-04-29 19:12] pinefin: thats how long ntoskrnl takes to load

[2025-04-29 19:13] mrexodia: This MCP approach was also mega regarded (as pointed out many times by my most loyal fan on X)

[2025-04-29 19:13] pinefin: maybe not ntoskrnl

[2025-04-29 19:13] pinefin: another binary

[2025-04-29 19:13] pinefin: but still, thats insane

[2025-04-29 19:13] daax: [replying to DirtySecreT: "was thinking this to be the case. have you done th..."]
no but you could grab any old tool, see if it would support utf8 and reallocate to a new section/or if it fits you just patch the translated text in place and truncate the remainder. easy to get all this info with idas api and then replace with translation after calling out to what you said -- some translation api or praying the llm can do it properly (i'd go for the former).

[2025-04-29 19:13] mrexodia: Likely you can do this much cheaper and with better quality results/have it work like an autocomplete (binary ninja sidekick style)

[2025-04-29 19:13] pinefin: [replying to mrexodia: "This MCP approach was also mega regarded (as point..."]
is there a better way to go about it?

[2025-04-29 19:14] daax: this would be pretty cool, ive had some tools where i went through and manually translated them because i couldnt read jack shit

[2025-04-29 19:14] pinefin: [replying to mrexodia: "Likely you can do this much cheaper and with bette..."]
whats sidekick? havent heard of it

[2025-04-29 19:14] DirtySecreT: [replying to daax: "this would be pretty cool, ive had some tools wher..."]
dms

[2025-04-29 19:14] pinefin: copilot for binja?

[2025-04-29 19:14] mrexodia: [replying to pinefin: "is there a better way to go about it?"]
This was done with pure brute force of scaling models, there was almost 0 thought that went into it

[2025-04-29 22:37] BlitzByte: forgive me for being an idiot, but what does MCP mean in RE terms? My smooth brain instantly thinks of Minecraft Coder Pack and that's obviously wrong

[2025-04-29 22:39] sunbather: [replying to BlitzByte: "forgive me for being an idiot, but what does MCP m..."]
It's Model Context Protocol (https://modelcontextprotocol.io/introduction)

[2025-04-29 22:40] BlitzByte: huh, neat

[2025-04-29 22:45] BlitzByte: I don't think I have anything currently powerful enough (that I can re-purpose) to use this with

[2025-04-29 23:47] pinefin: [replying to mrexodia: "This was done with pure brute force of scaling mod..."]
would it be possible with running with a local deepseek server?

[2025-04-29 23:47] pinefin: bringing costs down?

[2025-04-29 23:48] pinefin: im not fluent in LLM's or whatever key term that you could say...but wouldnt that make it free besides electricity?

[2025-04-30 08:10] mrexodia: [replying to pinefin: "would it be possible with running with a local dee..."]
Everything is possible

[2025-04-30 08:10] mrexodia: [replying to pinefin: "im not fluent in LLM's or whatever key term that y..."]
And no, the GPU cost up front is pretty high to run a capable model.

[2025-04-30 12:04] the horse: [replying to BlitzByte: "I'm trying to decompile an application that only h..."]
FLOSS

[2025-04-30 12:04] the horse: https://github.com/mandiant/flare-floss
[Embed: GitHub - mandiant/flare-floss: FLARE Obfuscated String Solver - Aut...]
FLARE Obfuscated String Solver - Automatically extract obfuscated strings from malware. - mandiant/flare-floss

[2025-04-30 12:04] the horse: i'm not sure if there's a plugin that will add comments there

[2025-04-30 12:05] the horse: you can launch it with -v flag

[2025-04-30 12:05] the horse: that will print function start, and address where string is decrypted

[2025-04-30 12:05] the horse: along with the full decrypted string

[2025-04-30 12:05] the horse: very useful, works on large bins too with extra flags

[2025-04-30 12:05] the horse: pretty slow because it's compiled python but a good tool

[2025-04-30 12:06] the horse: hrtng has more advanced support for decrypting stuff and updating decompilation based on it 
https://github.com/KasperskyLab/hrtng

requires a lot more manual input though
[Embed: GitHub - KasperskyLab/hrtng: IDA Pro plugin with a rich set of feat...]
IDA Pro plugin with a rich set of features: decryption, deobfuscation, patching, lib code recognition and various pseudocode transformations - KasperskyLab/hrtng

[2025-04-30 12:06] the horse: more useful for reversing encryption passes (xor, rol, ...)

[2025-04-30 14:10] Matti: +1 for floss, the only gripe I have with it is that it only supports x86 32/64 (IIRC), and it sort of supports shellcode but basically assumes files to be PEs

[2025-04-30 14:11] Matti: oh and it's slow as you said, but yeah python

[2025-04-30 14:13] diversenok: I had trouble trying to make it work due to its old dependencies

[2025-04-30 14:14] Matti: yeah, if you wanna modify it in any way that's probably gonna be a PITA too

[2025-04-30 14:14] diversenok: I think it brings some old version of numpy that refuses to work on newer python, or something along those lines

[2025-04-30 14:14] Matti: because of pip being pip

[2025-04-30 14:14] Matti: I just download the binary release nowadays, I used to build the standalone exe but it became too painful

[2025-04-30 14:20] Matti: legitimately a good target for a rust rewrite actually

[2025-04-30 14:20] the horse: [replying to Matti: "legitimately a good target for a rust rewrite actu..."]
+

[2025-04-30 14:20] Matti: almost everything about it that is painful is due to it being written in python

[2025-04-30 14:23] the horse: might make something similar once i'm done with my emulator, would be a fun project

[2025-04-30 14:23] Brit: similar tech anyway

[2025-04-30 14:23] the horse: + would be useful in my disassembler

[2025-04-30 14:23] the horse: i hate python with all my heart

[2025-04-30 14:24] Matti: the main problem would be that you're forking a very popular project that's actively being maintained

[2025-04-30 14:24] Brit: you can just send the state to your server and emu the chunked out bit there

[2025-04-30 14:24] Matti: if mandiant could be convinced that would be even better imo

[2025-04-30 14:24] Brit: the folk over at xenuine tried to do this, albeit accross two processes and not over network iirc

[2025-04-30 14:25] Brit: oh shit I thought this was related to the drm talk from techgen

[2025-04-30 14:25] the horse: oh no it is not

[2025-04-30 14:25] Brit: my bad disregard what I was saying

[2025-04-30 14:25] the horse: i was a bit confused

[2025-04-30 14:25] Matti: [replying to the horse: "i hate python with all my heart"]
who doesn't

[2025-04-30 14:26] the horse: also flare is apache 2.0 👍

[2025-04-30 14:26] Matti: well I know some, I won't name and shame but like

[2025-04-30 14:26] Matti: *really*

[2025-04-30 14:26] Brit: [replying to Matti: "who doesn't"]
https://www.runpyxl.com/
[Embed: PyXL - Run Python directly in hardware]
PyXL runs Python directly in hardware. Come see how.

[2025-04-30 14:26] Brit: but you can run it on a chip now

[2025-04-30 14:26] Brit: <:nomore:927764940276772925>

[2025-04-30 14:27] Matti: oh that's good

[2025-04-30 14:27] the horse: people will do everything but write code in a compiled language; but ig useful for tensor stuff?

[2025-04-30 14:27] Matti: that fixes all of my problems with python basically

[2025-04-30 14:27] Matti: or wait no

[2025-04-30 14:27] Matti: I think it's 0

[2025-04-30 14:28] Matti: 0 it fixes

[2025-04-30 14:28] Matti: damn, another missed opportunity

[2025-04-30 14:28] the horse: yeah now you have an additional problem of getting a board to run it on <:blacktroll:873063821344907325>

[2025-04-30 14:28] Matti: and it runs faster probably

[2025-04-30 14:28] Matti: that's also bad

[2025-04-30 14:28] the horse: 😄

[2025-04-30 14:28] Matti: python being slow is actually a feature

[2025-04-30 14:28] pinefin: a feature?

[2025-04-30 14:29] the horse: i originally thought cpython would be somewhat along the lines of il2cpp from unity

[2025-04-30 14:29] pinefin: or even

[2025-04-30 14:29] pinefin: luaJIT (kek)

[2025-04-30 14:29] 25d6cfba-b039-4274-8472-2d2527cb: what we really need is V8 native processors. I've been saying this for years.

[2025-04-30 14:29] the horse: but what is it? just python compiled into bytecode and an interpreter that's compiled?

[2025-04-30 14:29] Matti: [replying to the horse: "but what is it? just python compiled into bytecode..."]
yes

[2025-04-30 14:30] pinefin: [replying to 25d6cfba-b039-4274-8472-2d2527cb: "what we really need is V8 native processors. I've ..."]
https://tenor.com/view/top-fuel-dragster-idling-exhaust-flame-fire-gif-5521018327572092457

[2025-04-30 14:30] Matti: and it requires windows 8 to run

[2025-04-30 14:30] the horse: <:kappa:583797200249815053>

[2025-04-30 14:30] pinefin: yeah bro i have a top fuel processor

[2025-04-30 14:30] the horse: [replying to 25d6cfba-b039-4274-8472-2d2527cb: "what we really need is V8 native processors. I've ..."]
why when wasm exists

[2025-04-30 14:31] 25d6cfba-b039-4274-8472-2d2527cb: [replying to the horse: "why when wasm exists"]
V8 runs wasm does it not

[2025-04-30 14:31] the horse: (which is still frankly limited to a large extent from what i've seen but much better than hardware-accelerated javascript nodes)

[2025-04-30 14:31] the horse: [replying to 25d6cfba-b039-4274-8472-2d2527cb: "V8 runs wasm does it not"]
i think it just governs isolation?

[2025-04-30 14:31] the horse: and interop with other site components?

[2025-04-30 14:31] the horse: i might be mistaken though

[2025-04-30 14:32] the horse: but wasm should run native, but heavily isolated?

[2025-04-30 14:32] 25d6cfba-b039-4274-8472-2d2527cb: > V8 is Google’s open source high-performance JavaScript and WebAssembly engine, written in C++. It is used in Chrome and in Node.js, among others. It implements ECMAScript and WebAssembly, and runs on Windows, macOS, and Linux systems that use x64, IA-32, or ARM processors. V8 can be embedded into any C++ application.
says the internets. wasm is bytecode

[2025-04-30 14:32] 25d6cfba-b039-4274-8472-2d2527cb: so in essence we need wasm cpus! browsers are the future

[2025-04-30 14:33] the horse: ah you're right, it is a cross platform interpreted thingy

[2025-04-30 14:33] the horse: makes sense

[2025-04-30 14:34] Matti: [replying to Matti: "and it requires windows 8 to run"]
this reminds me I've been meaning to open source my un-ass fork that restores compat with older OS versions for something as simple as what cpython does
but they just released v3.13 😩
[Attachments: image.webp]

[2025-04-30 14:34] the horse: i thought it eventually compiles down to the browser arch, but i guess that would have weird security implications

[2025-04-30 14:35] Matti: not that this would be useful to anyone.... I just dislike python quite a lot

[2025-04-30 14:36] Matti: so making a point of how lazy their devs really are is a goal in itself for me

[2025-04-30 14:38] the horse: does anyone know if i can forcefully create a session for another user (like guest) in windows without manually logging in to that account?
there's some token magic but it looks like it's just impersonation of the user in the same session?

(need to spawn a process as a user without password and without administrative privileges)

[2025-04-30 14:38] Matti: I won't lie though the code for this is a horror shitshow, even compared to the rest of cpython

[2025-04-30 14:38] Matti: and last time I checked it still failed like 5 out of 200 unit tests

[2025-04-30 14:38] Matti: probably broken tests but whatever

[2025-04-30 14:39] Matti: ah <@503274729894051901> is typing

[2025-04-30 14:39] Matti: he'll know the answer better than me

[2025-04-30 14:40] pinefin: get colin in here and he'll tell you oCAml is the future /s

[2025-04-30 14:40] diversenok: The last discussion about session creation was quite productive, I definitely need your input <@148095953742725120>

[2025-04-30 14:41] Matti: yeah but I'm supposed to be working

[2025-04-30 14:41] pinefin: boss makes a dollar i make a dime

[2025-04-30 14:41] pinefin: thats why i talk in secret.club discord on company time

[2025-04-30 14:41] diversenok: [replying to the horse: "does anyone know if i can forcefully create a sess..."]
The first question is do you really need a session? As in, with its own CSRSS and everything

[2025-04-30 14:41] the horse: yes

[2025-04-30 14:42] the horse: i'm trying to study the actual separation and it would be easier for me to elevate, raise tokens, create another process and see what differences arise from this

[2025-04-30 14:42] Matti: that session at the very least needs to belong to an account, I think

[2025-04-30 14:42] the horse: very interested in how the session processes and drivers are mapped in those secnarios

[2025-04-30 14:43] pinefin: are those sessions given their own kernelmodes? or are they shared

[2025-04-30 14:43] the horse: I can create a user account; would be best if I could re-use one (Administrator, Guest)

[2025-04-30 14:43] Matti: whether you can cheat at the login requirement idk

[2025-04-30 14:43] the horse: [replying to pinefin: "are those sessions given their own kernelmodes? or..."]
there's session drivers which seem to have a different context

[2025-04-30 14:43] the horse: + system processes

[2025-04-30 14:43] pinefin: ah

[2025-04-30 14:43] pinefin: neat

[2025-04-30 14:43] the horse: ```
User: Administrator
User: DefaultAccount
User: Guest
User: ricoc
User: WDAGUtilityAccount
```

[2025-04-30 14:43] Matti: [replying to the horse: "there's session drivers which seem to have a diffe..."]
not since 24H2 actually!

[2025-04-30 14:44] Matti: session spaces were removed

[2025-04-30 14:44] pinefin: microsoft removing core components like theres no tomorrow

[2025-04-30 14:44] the horse: oh yes I saw that; there's an array for some of the syscall resolving that takes session id as a parameter

[2025-04-30 14:44] the horse: or well rather; to get the endpoint in win32kfull

[2025-04-30 14:44] Matti: [replying to pinefin: "microsoft removing core components like theres no ..."]
session spaces weren't a core component

[2025-04-30 14:45] pinefin: [replying to Matti: "session spaces weren't a core component"]
yeah nah im joking, id expect the kernelmode to stay the same and compartmentalize the usermode

[2025-04-30 14:45] the horse: GetW32State -> gets index of current session id and then indexes that with Global/LowSlots

[2025-04-30 14:45] Matti: they were a huge mistake that took them years of work to rectify

[2025-04-30 14:45] Matti: I'm amazed they succeeded actually

[2025-04-30 14:45] the horse: didn't know that they rid it of the space though

[2025-04-30 14:45] pinefin: [replying to Matti: "they were a huge mistake that took them years of w..."]
when were they initially added?

[2025-04-30 14:46] Matti: [replying to the horse: "GetW32State -> gets index of current session id an..."]
yeah sorry, they are still isolated in as far as the user can tell, to be clear

[2025-04-30 14:46] Matti: I mean session address spaces in the Mm sense

[2025-04-30 14:46] the horse: how were the spaces implemented exactly though? was there just a different physical allocation for the whole driver, or only its data?

[2025-04-30 14:46] Matti: [replying to pinefin: "when were they initially added?"]
2000 I think, maaaybe NT4?

[2025-04-30 14:46] the horse: i assume csrss is another process spawn

[2025-04-30 14:46] the horse: isolated from the other user

[2025-04-30 14:47] Matti: [replying to the horse: "how were the spaces implemented exactly though? wa..."]
this is definitely too much for me to type right now sorry <:kekw:904522300257345566>

[2025-04-30 14:47] the horse: np

[2025-04-30 14:47] Matti: windows internals should cover this

[2025-04-30 14:48] diversenok: I think the easiest option is to just log in under multiple users and use fast user switching. Programmatically creating sessions is pain, and I'm not sure how to make it work correctly anyway

[2025-04-30 14:48] Matti: same

[2025-04-30 14:48] Matti: re: the second part

[2025-04-30 14:48] Matti: I hate the first part but I can't think of a better solution either

[2025-04-30 14:49] Matti: well you could make RDP profiles with autologin to make the sessions *really* quickly I guess

[2025-04-30 14:49] the horse: with the impersonation mechanics, does it all run under the current user in reality?

[2025-04-30 14:49] Matti: but it's still logging on

[2025-04-30 14:50] Matti: you can even quit the RDP client after (don't log out) and with the correct settings the session should stay around

[2025-04-30 14:50] Matti: not sure if it's indefinitely but I think it could be made indefinitely

[2025-04-30 14:51] the horse: would that work purely on localhost?

[2025-04-30 14:51] the horse: not that informed on RDP

[2025-04-30 14:51] Matti: with a lot of pain probably yeah

[2025-04-30 14:51] diversenok: [replying to Matti: "well you could make RDP profiles with autologin to..."]
You don't have to type credentials every time; `WinStationConnectW` doesn't require a password when invoked by SYSTEM (at least on non-terminal server OS) for switching between existing sessions

[2025-04-30 14:52] Matti: ah no I only meant the initial creation of them

[2025-04-30 14:52] the horse: think I could create sessions without password as SYSTEM/NT_AUTHORITY?

[2025-04-30 14:52] Matti: depending on how many you even need.... maybe this is a non-issue

[2025-04-30 14:52] the horse: 1-2

[2025-04-30 14:52] the horse: I can elevate to SYSTEM

[2025-04-30 14:52] Matti: and also maybe SYSTEM can do the same thing regardless, heh

[2025-04-30 14:53] the horse: hmm.. SYSTEM might have its own session?

[2025-04-30 14:53] Matti: not really, but there's something close to this which is session 0

[2025-04-30 14:53] Matti: it's non-interactive, meant for services

[2025-04-30 14:53] Matti: including any running as system

[2025-04-30 14:54] the horse: yeah but it's also incompatible with a lot of stuff

[2025-04-30 14:54] the horse: 24h2 basically enforces this as well

[2025-04-30 14:54] the horse: the arrays are index like idx - 1

[2025-04-30 14:55] the horse: and you'd bugcheck if you use any of it essentially

[2025-04-30 14:55] diversenok: What?

[2025-04-30 14:55] Matti: the company I write the driver for to make s0 semi-usable again hasn't OK'd the hours I put up for making it 24h2 compatible lol

[2025-04-30 14:55] Matti: it was a big number

[2025-04-30 14:55] Matti: but I think it should be possible fine

[2025-04-30 14:56] diversenok: [replying to the horse: "think I could create sessions without password as ..."]
Kind of... https://discord.com/channels/835610998102425650/835667591393574919/1290487930779074620

[2025-04-30 14:56] the horse: [replying to diversenok: "What?"]
some features, like gui-related stuff have strict checks on the session

if the process has a session, its ID is >= 1
if you call a function related to this from a session id 0 process; it wouldn't get a valid index into the internal structures, they have a prerequisite check that will bsod you to prevent UB

[2025-04-30 14:56] the horse: 
[Attachments: image.png]

[2025-04-30 14:57] the horse: well this is for -1

[2025-04-30 14:57] Matti: yeah but this has always been like this

[2025-04-30 14:57] the horse: 
[Attachments: image.png]

[2025-04-30 14:57] Matti: in plenty of places

[2025-04-30 14:57] diversenok: Session -1 means no session at all; session 0 can use GUI functions fine. Or at least used to

[2025-04-30 14:57] Matti: well not always

[2025-04-30 14:57] Matti: starting since around win 10

[2025-04-30 14:57] the horse: maybe PsGetProcessSessionId handles this internally

[2025-04-30 14:57] the horse: otherwise you'd be dereferencing [-1]

[2025-04-30 14:58] the horse: oh

[2025-04-30 14:58] the horse: im blind

[2025-04-30 14:58] the horse: my bad 🙂

[2025-04-30 14:58] the horse: you're right 👍

[2025-04-30 14:58] Matti: [replying to diversenok: "Session -1 means no session at all; session 0 can ..."]
yeah, all they've done is added explicit checks agains the service session ID(s) (remember there can be multiple now because of containers) to disallow this

[2025-04-30 14:59] Matti: but they're selective half-arsed shitty checks like usual

[2025-04-30 14:59] the horse: i'm not sure why some of this stuff even exists

[2025-04-30 14:59] Matti: [replying to the horse: "you're right 👍"]
you're not entirely wrong though

[2025-04-30 14:59] Matti: [replying to the horse: "the arrays are index like idx - 1"]
this is annoying

[2025-04-30 15:00] the horse: yeah it will switch to lowSlots

[2025-04-30 15:00] Matti: not the same thing but still

[2025-04-30 15:00] the horse: which I guess could be use for restrictions on certain api

[2025-04-30 15:00] the horse: since those arrays map function pointers to the actual api in win32kfull

[2025-04-30 15:01] the horse: 24h2 is a very weird version

[2025-04-30 15:02] Matti: just wait for 25h2 <:thinknow:475800595110821888>

[2025-04-30 15:02] Matti: they've already changed how it works again

[2025-04-30 15:02] Matti: not by much though

[2025-04-30 15:02] the horse: they seem to be doing a lot more restrictions vs exploit development

[2025-04-30 15:03] the horse: especially limits on the interactions you can do with system processes

[2025-04-30 15:03] the horse: security overrides on them don't seem to work properly

[2025-04-30 15:03] the horse: i.e. disabling ACG (whether globally or process-specific)

[2025-04-30 15:04] Matti: IDK the reason for the 25h2 changes (maybe they just came up with a better method too late), but the removal of session spaces is only vaguely exploit related at best

[2025-04-30 15:04] Matti: sec

[2025-04-30 15:04] diversenok: [replying to the horse: "i.e. disabling ACG (whether globally or process-sp..."]
Arbitrary Code Guard?

[2025-04-30 15:04] the horse: yes

[2025-04-30 15:05] Matti: well, it is and isn't related... but I'd say it's more of a side effect than anything else

[2025-04-30 15:05] Matti: 👋 <@260503970349318155>
[Attachments: Hypervisor-enforced_Paging_Translation.pdf]

[2025-04-30 15:06] diversenok: Either way, if your goal is to test things in multiple interactive sessions at once, there is a way to switch between them in a single click

[2025-04-30 15:07] diversenok: Log into multiple accounts so there are multiple sessions side by side. Run System Informer as SYSTEM in every session, and then go to the main menu -> User -> (another user) -> Connect

[2025-04-30 15:08] diversenok: That's it, not password required

[2025-04-30 15:15] diversenok: If, for whatever reason, you want to programmatically create sessions, there is high-level (undocumented) API offered by the LSM service and low-level SMSS API

[2025-04-30 15:16] diversenok: Oh, and even lower-level flag in `NtCreateUserProcess`, but it requires the calling process to not have a session, so not that useful

[2025-04-30 15:17] the horse: appreciate it 🙂

[2025-04-30 15:18] diversenok: Here is the start of the previous discussion: https://discord.com/channels/835610998102425650/835667591393574919/1290351318644559914

[2025-04-30 15:19] diversenok: (it's pretty long)

[2025-04-30 15:20] diversenok: There is even a half-working example for programmatically creating new sessions there

[2025-04-30 18:32] daax: [replying to diversenok: "If, for whatever reason, you want to programmatica..."]
the rpc method? or is there some other way from the lsm service?

[2025-04-30 22:24] diversenok: I haven't looked much into it, just saw that LSM implements lots of methods for `ITSSession`, `ISessionManager`, `ICsrMgr` and similar interfaces, so I assume it's for RPC