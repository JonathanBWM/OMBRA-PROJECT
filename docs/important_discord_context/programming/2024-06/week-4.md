# June 2024 - Week 4
# Channel: #programming
# Messages: 266

[2024-06-17 00:01] donna🤯: i.e ensuring we dont queue an apc to a thread thats just terminated. Sometime though we are looking at just data inside the structure but the main reason for the intensive tracking is so that when we, say queue some apc's to a subset of threads, we are doing that safely.

[2024-06-17 05:24] Koreos: [replying to Deleted User: "is there a better way of enumerating current proce..."]
In case you're still working on this, Pavel himself just released this: https://www.youtube.com/watch?v=kEIE91kpsGg
[Embed: Modules Enumeration]

[2024-06-17 05:25] Deleted User: [replying to Koreos: "In case you're still working on this, Pavel himsel..."]
oh that's cool i'll watch in a bit

[2024-06-17 05:25] Deleted User: need to go get something to eat and stuff

[2024-06-17 05:50] Horsie: >have question
>open windows internals
>question answered

[2024-06-17 05:50] Horsie: how can one book be so good?

[2024-06-17 07:21] 冰: [replying to donna🤯: "Hello, recently I have been overhauling some inter..."]
hmm

[2024-06-17 07:22] 冰: hmm

[2024-06-17 07:26] donna🤯: so it turns out it is a safe reference its just the process / thread will be terminated - but the object itself wont be discarded. So it is a potential solution but requires more work to ensure its a live thread / process and not an object waiting to be cleaned up.

[2024-06-17 07:27] 冰: hmm

[2024-06-17 07:28] donna🤯: [replying to 冰: "hmm"]
reference counts are more or less for a garbage collection purpose i.e run in the background and (i assume - implementation dependent) scan for objects with a reference count of 0 and then free them

[2024-06-17 07:28] donna🤯: for example, when you close a process say a game, the reference count is not 0

[2024-06-17 07:28] donna🤯: but the process is terminated and subsequent threads also terminated

[2024-06-17 07:29] 冰: [replying to donna🤯: "for example, when you close a process say a game, ..."]
hmm

[2024-06-17 07:29] donna🤯: [replying to 冰: "hmm"]
it can be anything

[2024-06-17 07:29] donna🤯: probably much higher

[2024-06-17 07:29] 冰: hmm

[2024-06-17 07:29] donna🤯: nope

[2024-06-17 07:32] donna🤯: what could also happen is that objects arent cleanedup until the reference count is 0 - but they are effectively terminated

[2024-06-17 07:33] donna🤯: i.e during cleanup all handles, objects etc. are cleaned up - then if the reference count is still not 0 its not freed, in some cases this would be a leak - in most cases the reference count would be 0 and then the object cleaned up

[2024-06-17 07:34] 冰: [replying to donna🤯: "i.e during cleanup all handles, objects etc. are c..."]
hmm

[2024-06-17 07:35] donna🤯: so in theory your method would work, but even though its a valid object its not a "live" process or thread if that makes sense - also i could be wrong here hence why i asked the question in the first place

[2024-06-17 07:35] donna🤯: [replying to 冰: "hmm"]
something like that is simply not scalable

[2024-06-17 07:35] donna🤯: shouldnt be used in a kernel driver

[2024-06-17 07:36] donna🤯: ah yea so it seems like, using process as an example, the process is set to terminated but the object itself isnt deleted until the reference count is 0

[2024-06-17 07:37] 冰: [replying to donna🤯: "ah yea so it seems like, using process as an examp..."]
hmm

[2024-06-17 07:37] donna🤯: [replying to 冰: "hmm"]
not really a fan of using undocumented locks

[2024-06-17 07:38] donna🤯: shouldnt really use what you dont have access to - this isnt something that just needs to work for me - ideally it should work on any version of windows

[2024-06-17 07:38] 冰: [replying to donna🤯: "not really a fan of using undocumented locks"]
hmm

[2024-06-17 07:39] donna🤯: [replying to 冰: "hmm"]
you are correct and I shouldnt really have that but definitely shouldnt be acquiring a lock using an offset

[2024-06-17 07:40] donna🤯: the key thing here is safety

[2024-06-17 07:40] donna🤯: i care more about ensuring safety and scalability

[2024-06-17 07:40] 冰: [replying to donna🤯: "the key thing here is safety"]
hmm

[2024-06-17 07:41] donna🤯: [replying to 冰: "hmm"]
fair enough

[2024-06-17 07:41] donna🤯: but its still not really recommended

[2024-06-17 07:41] donna🤯: afterall this is why thread and process callbacks were made

[2024-06-17 07:41] donna🤯: to allow safe access

[2024-06-17 07:43] donna🤯: again, i could be wrong here but ideally youd want to limit yourself to what you can do with the windows api to ensure safety as much as possible without having to rely undocumented structs

[2024-06-17 07:43] donna🤯: thats my approach

[2024-06-17 07:44] donna🤯: also here i am kinda wrong as i said since is a safe reference to the object but yea the process will be in a terminated state i.e waiting cleanup by the object manager and not in any of the active lists.

[2024-06-17 07:44] 冰: [replying to donna🤯: "again, i could be wrong here but ideally youd want..."]
hmm

[2024-06-17 07:46] donna🤯: [replying to 冰: "hmm"]
the main solutions would be to detect patchguard bypasses - as for ntoskrnl image integrity checks it is on the 2 do list as its a bit trickier then regular module integrity checks

[2024-06-17 07:47] donna🤯: not very thorogh but havent really looked into it too much yet

[2024-06-17 07:48] donna🤯: if you have any ideas though... let me know xD

[2024-06-17 08:51] elias: Is there way to compile c code to risc v and link it as a normal PE (with IAT)?

[2024-06-17 08:58] @Cypher - Read Bio: [replying to elias: "Is there way to compile c code to risc v and link ..."]
its possible but it requires heavy work

[2024-06-17 09:04] elias: so would it be less work to base my vm interpreter on arm64 instead?

[2024-06-17 09:42] vendor: [replying to elias: "Is there way to compile c code to risc v and link ..."]
you could hack clang into doing this pretty easily but you won't be able to link against anything

[2024-06-17 09:43] vendor: (i'm assuming you've read <https://secret.club/2023/12/24/riscy-business.html>)

[2024-06-17 09:43] vendor: but i don't see why you would want a full RISC-V PE image

[2024-06-17 09:53] elias: I‘ve read the riscy business blog but I‘d prefer to take a different approach. Instead of using the rather complicated build process with clang cl and cmake (and as far as I understand manually defining every import) I want to normally compile the PE and then modify the instructions in the compiled binary and embed it in the vm… if that‘s not possible with riscv I probably have to go with arm64 since vs can directly compile to it

[2024-06-17 11:18] vendor: [replying to elias: "I‘ve read the riscy business blog but I‘d prefer t..."]
unless you include extra metadata you will have difficulty translating the calling convention once your emulator reaches the external call instruction.

[2024-06-17 13:25] elias: [replying to vendor: "unless you include extra metadata you will have di..."]
because I don't know the number of arguments?

[2024-06-17 15:52] JustMagic: [replying to elias: "I‘ve read the riscy business blog but I‘d prefer t..."]
IIRC any sort of a useful arm target is going to be much more complicated to emulate as well

[2024-06-17 15:52] Torph: [replying to Horsie: ">have question
>open windows internals
>question a..."]
<:kekw:904522300257345566>

[2024-06-17 15:54] JustMagic: [replying to donna🤯: "also here i am kinda wrong as i said since is a sa..."]
I don't particularly see what's the problem here

[2024-06-17 16:00] Torph: lol. lmao.
[Attachments: 2024-06-17_12-00.png]

[2024-06-17 16:42] mrexodia: [replying to elias: "I‘ve read the riscy business blog but I‘d prefer t..."]
The blog post mentions why binary rewriting (eg replacing instructions) wasn't picked. The main reason is that you need to write a full disassembler to do this properly. The 'complex' build process with clang-cl (no cmake) isn't anywhere near as complex as implementing a binary rewriter.

[2024-06-17 16:43] mrexodia: That being said, here is a nice resource on it: https://github.com/es3n1n/obfuscator (with the blog post https://blog.es3n1n.eu/posts/obfuscator-pt-1)

[2024-06-17 16:43] mrexodia: > and as far as I understand manually defining every import
You misunderstood, the import stubs are generated fully automatically

[2024-06-17 16:50] elias: Oh okay I apologize

[2024-06-17 16:50] elias: gonna revisit it

[2024-06-17 17:26] contificate: all this clownage when you could just obfuscate LLVM

[2024-06-17 19:36] Brit: [replying to contificate: "all this clownage when you could just obfuscate LL..."]
the albanian nim malware developer and the polish CS GO cheat pasters won't part with their exceedingly valuable source code, therefore bin2bin is what he must make.

[2024-06-17 19:37] contificate: probably legitimately the case

[2024-06-17 19:38] Brit: unfortunately

[2024-06-18 09:55] elias: I personally just really dislike meta programming and hacking together toolchains consisting of tools not written by mega corporations

[2024-06-18 09:55] elias: thats why I personally view bin2bin as the 'cleaner' technology, although requires more effort

[2024-06-18 09:57] elias: but of course the work of mrexotic is still impressive

[2024-06-18 10:20] snowua: why do more work when you can do less 😞

[2024-06-18 10:21] Deleted User: its fun!! and you learn stuff ^-^

[2024-06-18 10:23] elias: [replying to Deleted User: "its fun!! and you learn stuff ^-^"]
right

[2024-06-18 10:24] snowua: either way you will learn stuff. except one will feel far more painful than the other. unless you’re working with closed source binaries i can’t see why you would willingly do this to yourself.

[2024-06-18 10:27] Deleted User: for me because its the best way to learn about reversing while also building something, and because its easier to integrate it with a normal build flow instead of having to setup some weird toolchain/llvm stuff that needs to get updated for newer versions etc blabla, llvm on windows (especially for rust) is annoying since it doesnt have pass support enabled you need to recompile it, and bin2bin is a solution that works on a binary nomatter what compiler/lang :3

[2024-06-18 10:30] Deleted User: ive spent around 200-300 hours (at minimum i think) on this project (including debugging etc) and i learned alot!!

[2024-06-18 12:09] elias: [replying to snowua: "either way you will learn stuff. except one will f..."]
You learn different things tho. With one way you'll learn how to hack together various tools and with the other you can learn in depth about instruction sets which is more interesting imo. And since I'm doing this in my free time I dont need to worry about efficiency or any costs... so why not

[2024-06-18 12:57] Brit: the types of transforms you can do when you control the compiler vs just the lifted out code in the best case or straight disassembly are completely different.

[2024-06-18 12:58] elias: right

[2024-06-18 12:58] elias: but you can achieve satisfying results both ways

[2024-06-18 17:40] mrexodia: [replying to Deleted User: "for me because its the best way to learn about rev..."]
This used to be true, but that was exactly the problem we solved in the blog post

[2024-06-18 17:41] mrexodia: I still have an open PR on LLVM though, because there are some edge cases to be fair...

[2024-06-18 17:41] mrexodia: The linux pass support is a meme anyway and not useful for obfuscation really (of any real product)

[2024-06-18 17:41] contificate: > With one way you'll learn how to hack together various tools
No, _you_ might "hack" together various tools, but there's a great load of interesting CS topics that come in and are more useful laterally than memorising from ISA semantics

[2024-06-18 17:42] contificate: bin2bin is also conceptually much nastier

[2024-06-18 17:42] contificate: it's not a cleaner approach at all

[2024-06-18 17:42] contificate: if anything, it's more limited

[2024-06-18 17:43] mrexodia: [replying to elias: "but you can achieve satisfying results both ways"]
It all depends on what you find satisfying. The fact that you have to write your own disassembler that is on-par with IDA and your own lifter on par with Ghidra to do anything proper is just kind of sad.

[2024-06-18 17:43] mrexodia: Doing a few instruction substitutions and (maybe) some opaque predicates (if you do the proper liveness analysis) is 'easy' enough

[2024-06-18 17:44] Deleted User: i dont write my own disassembler, i use icedx86. but for control flow disassembly so far it works pretty good and is relatively simple while achieving the same results as ida/binja (except for some edge cases)

[2024-06-18 17:44] mrexodia: But the problem space is just extremely large, so you cannot naturally evolve from a basic thing that works on a few binaries to anything production-ready without rewriting everything

[2024-06-18 17:44] mrexodia: [replying to Deleted User: "i dont write my own disassembler, i use icedx86. b..."]
'Edge cases' such as switch tables and tail calls 😆

[2024-06-18 17:44] Deleted User: no those are fine, i mean no_return call analysis for api etc

[2024-06-18 17:45] mrexodia: And you need to recover the calling convention as well if you want to do anything meaningful with the parameters

[2024-06-18 17:45] Deleted User: i need to add import analysis stuff to be able to identify stuff like exit

[2024-06-18 17:45] contificate: I maintain that most public LLVM obfuscations are pretty limited

[2024-06-18 17:45] mrexodia: [replying to mrexodia: "But the problem space is just extremely large, so ..."]
I think this is the key point <@456226577798135808>

[2024-06-18 17:46] Deleted User: yeah def its alot of work

[2024-06-18 17:46] Deleted User: but i like that about it

[2024-06-18 17:46] mrexodia: Also the amount of testing you need to do to prove your VM semantics are correct is insane

[2024-06-18 17:46] Deleted User: if you want something easy then go with llvm stuff

[2024-06-18 17:46] contificate: the problem of general intractability just means it's always a game of cat and mouse, with heuristic and automated means (by way of solvers)

[2024-06-18 17:46] mrexodia: But I guess it's just professional product vs fun personal project

[2024-06-18 17:47] mrexodia: [replying to Deleted User: "if you want something easy then go with llvm stuff"]
I don't think the problem is easy vs hard, it's just extremely tedious work to lift all instruction semantics correctly 🤷‍♂️

[2024-06-18 17:47] Deleted User: [replying to mrexodia: "Also the amount of testing you need to do to prove..."]
true, for now i autogenerate tests from a repository with test data but i wanna expand on the testing and add the binaries that back.engineering uses too

[2024-06-18 17:48] mrexodia: Which repo are you using?

[2024-06-18 17:48] Deleted User: one that snowua linked here, its not the best

[2024-06-18 17:48] Deleted User: https://github.com/ZehMatt/x86TestData/
[Embed: GitHub - ZehMatt/x86TestData: Test data for x86 instructions]
Test data for x86 instructions. Contribute to ZehMatt/x86TestData development by creating an account on GitHub.

[2024-06-18 17:48] mrexodia: Ah yeah that's the one I sent him 😆

[2024-06-18 17:49] mrexodia: Yeah Matt is the legend, he's been doing binary rewriters for 10+ years at this point

[2024-06-18 17:49] Deleted User: some of the mul data was faulty idk how that happened

[2024-06-18 17:49] mrexodia: 🤷‍♂️

[2024-06-18 17:50] Deleted User: but besides that its nice, generated 17k~ tests (excluded immediates for now bcuz thats 100k plus tests and idek how many hundred thousands lines of code for those)

[2024-06-18 17:53] contificate: I have general anxieties that bin2bin shit will be generally worse quality and limited

[2024-06-18 17:53] contificate: also harder to develop for any meaningful use case

[2024-06-18 17:54] contificate: like, for the brief time I was employed by an obfuscation company

[2024-06-18 17:54] mrexodia: There's pros and cons for both, LLVM just doesn't give you much control over the final instructions without doing major hackery

[2024-06-18 17:54] contificate: most customer issues were like "ah fuck we've got a deadlock"

[2024-06-18 17:54] mrexodia: So if you wanna do on-demand encryption of functions etc like Arxan bin2bin is definitely easier

[2024-06-18 17:54] contificate: and it usually transpired that it wasn't our fault really, it'd just never produced the faulty interleaving until we introduce some slowdown as part of the obfuscations

[2024-06-18 17:54] Deleted User: [replying to contificate: "I have general anxieties that bin2bin shit will be..."]
did u see the codedefender.io stuff

[2024-06-18 17:54] contificate: is this backengineering?

[2024-06-18 17:54] Deleted User: they have transformed huge binaries

[2024-06-18 17:54] Deleted User: ya

[2024-06-18 17:55] mrexodia: allegedly <:kappa:697728545631371294>

[2024-06-18 17:55] contificate: I haven't but am generally unsure

[2024-06-18 17:55] Deleted User: fair

[2024-06-18 17:55] Deleted User: [replying to contificate: "most customer issues were like "ah fuck we've got ..."]
did you work at themida..

[2024-06-18 17:55] contificate: I think, morally, you'd actually need write custom LLVM back-end stuff to address duncan's most recent comments

[2024-06-18 17:55] contificate: or just a custom compiler back-end

[2024-06-18 17:56] elias: [replying to contificate: "> With one way you'll learn how to hack together v..."]
I think at the end it comes down to personal preference, but I agree with you that these topics might be interesting too

[2024-06-18 17:56] contificate: more interesting

[2024-06-18 17:56] contificate: you get that like Ghidra's IR lifting/matching language stuff spawns out of Ramsey et al research

[2024-06-18 17:56] contificate: which is a compiler orientated research group

[2024-06-18 17:57] contificate: responsible for work on C-- IR etc.

[2024-06-18 17:57] contificate: the overlap exists if you come at problems from the right angle

[2024-06-18 17:57] contificate: otherwise you just invent "ah fuck we need an IR" again and again

[2024-06-18 17:57] contificate: and recreate basic compiler stuff again and again

[2024-06-18 17:57] mrexodia: To be fair it's not like LLVM is the most easy to reuse technology, considering it's like 80mb of bloatware

[2024-06-18 17:58] contificate: yeah it's unfortunate

[2024-06-18 17:58] Deleted User: and you have to recompile it on windoof for plugin support (afaik)!!!

[2024-06-18 17:58] contificate: I've always wanted something a bit like BAP but easier for generalists to use

[2024-06-18 17:58] Deleted User: atleast thats how it was when i tried it for rust

[2024-06-18 17:58] Timmy: isnt there some way more neat technology written in C? optimizes less too, but yk

[2024-06-18 17:58] Timmy: i can't quite rcall rn

[2024-06-18 17:58] mrexodia: I've just seen the first hand customer support experience for bin2bin obfuscation and the issues are _not_ fun (think 2 months of debugging in the worst cases)

[2024-06-18 17:58] contificate: C and C++ are complete tedium for compilers

[2024-06-18 17:58] contificate: which is the main pain I feel

[2024-06-18 17:59] mrexodia: But if you're just doing a red team meme and you only want to obfusacte your own binaries 🤷‍♂️

[2024-06-18 17:59] Deleted User: [replying to mrexodia: "I've just seen the first hand customer support exp..."]
yeahhh.. true 😭

[2024-06-18 17:59] mrexodia: Can hack a poc vm together in a week and you're totally golden

[2024-06-18 17:59] contificate: it's hard to meet security people who give a shit at the same kind of level

[2024-06-18 17:59] contificate: think there was one guy here before

[2024-06-18 17:59] contificate: and obviously the German researchers

[2024-06-18 17:59] mrexodia: [replying to contificate: "I've always wanted something a bit like BAP but ea..."]
mister ocaml

[2024-06-18 17:59] Deleted User: im german!!

[2024-06-18 18:00] contificate: sehr gut

[2024-06-18 18:00] mrexodia: [replying to Deleted User: "and you have to recompile it on windoof for plugin..."]
check the poast 😉

[2024-06-18 18:00] mrexodia: the trick is to use LTO

[2024-06-18 18:00] Deleted User: also i looked up the zehmatt guy and i believe zeh is a reference to how germans pronounce the

[2024-06-18 18:00] mrexodia: and you will get the .bc file for the whole binary

[2024-06-18 18:00] Deleted User: also his vm stuff seems interesting

[2024-06-18 18:00] Deleted User: [replying to mrexodia: "the trick is to use LTO"]
i am using lto

[2024-06-18 18:01] mrexodia: so no need to recompile llvm then

[2024-06-18 18:01] Deleted User: i dont remember how exactly it works to tell rust to use my passes but it was annoying

[2024-06-18 18:01] contificate: of course, take my views with scepticism as I am not a software engineer or computer scientist, but more of an apprentice joiner

[2024-06-18 18:01] mrexodia: rust has a skill issue though

[2024-06-18 18:01] Deleted User: [replying to mrexodia: "so no need to recompile llvm then"]
mhm idk there was a custom fork which i had to use, ill have to check it again sometime

[2024-06-18 18:01] mrexodia: also everyone knows that no real projects are written in rust

[2024-06-18 18:01] mrexodia: <:kappa:697728545631371294>

[2024-06-18 18:01] Deleted User: https://github.com/jamesmth/llvm-plugin-rs
[Embed: GitHub - jamesmth/llvm-plugin-rs: Out-of-tree LLVM passes in Rust]
Out-of-tree LLVM passes in Rust. Contribute to jamesmth/llvm-plugin-rs development by creating an account on GitHub.

[2024-06-18 18:02] contificate: only vaguely creepy meta projects like cargo mummy are written in Rust

[2024-06-18 18:02] Timmy: helix is really based

[2024-06-18 18:02] mrexodia: [replying to Deleted User: "mhm idk there was a custom fork which i had to use..."]
yeah because the rust people don't know what they re doing

[2024-06-18 18:02] Timmy: ngl

[2024-06-18 18:02] Deleted User: [replying to mrexodia: "yeah because the rust people don't know what they ..."]
i dont think that was rust related since it was a fork of their official llvm fork but who knows

[2024-06-18 18:02] Timmy: helix has fully replaced vscode for me now

[2024-06-18 18:02] Deleted User: [replying to contificate: "only vaguely creepy meta projects like cargo mummy..."]
woow okay..

[2024-06-18 18:02] mrexodia: [replying to Deleted User: "i dont think that was rust related since it was a ..."]
the plugin system of llvm is not what I am using

[2024-06-18 18:03] mrexodia: that thing doesn't work on windows because llvm has more than 65k exports so you cannot compile it as a DLL (minus some msys hackery that doesn't count)

[2024-06-18 18:03] contificate: 😏

[2024-06-18 18:03] contificate: luckily duncan mains Linux

[2024-06-18 18:03] contificate: phew!

[2024-06-18 18:03] contificate: thought we were stuck there!

[2024-06-18 18:03] Deleted User: [replying to mrexodia: "that thing doesn't work on windows because llvm ha..."]
what thing u mean the repo i linked?

[2024-06-18 18:04] Timmy: on linux now as well

[2024-06-18 18:04] Deleted User: [replying to contificate: "luckily duncan mains Linux"]
me soontm hopefully, no more windows pain

[2024-06-18 18:04] mrexodia: [replying to Deleted User: "what thing u mean the repo i linked?"]
the `-load-pass-plugin` flag

[2024-06-18 18:04] Deleted User: until i install a windows vm to support windows

[2024-06-18 18:04] Timmy: gonna dip another toe in ocaml soon

[2024-06-18 18:04] contificate: what, really, Timmy?

[2024-06-18 18:04] Deleted User: [replying to mrexodia: "the `-load-pass-plugin` flag"]
ah

[2024-06-18 18:04] contificate: what a chad

[2024-06-18 18:04] Timmy: ye

[2024-06-18 18:04] Timmy: nixos

[2024-06-18 18:04] contificate: ah

[2024-06-18 18:04] contificate: less based

[2024-06-18 18:04] contificate: but still based

[2024-06-18 18:04] Timmy: nixos unstable with cachyos kernel and hyprland

[2024-06-18 18:05] Timmy: absolutely zoomin

[2024-06-18 18:05] Timmy: both qt and gnome apps installed

[2024-06-18 18:05] contificate: just need to pick up more serious FP

[2024-06-18 18:05] contificate: praise be

[2024-06-18 18:05] Timmy: no compat issues

[2024-06-18 18:05] Timmy: <:gigachad:904523979249815573>

[2024-06-18 18:06] Timmy: try that on your debian distro

[2024-06-18 18:07] contificate: I mean I do that on Arch

[2024-06-18 18:07] contificate: I use Arch because easy

[2024-06-18 18:07] contificate: and bleeding edge packages

[2024-06-18 18:07] Deleted User: i was thinking of switching to arch

[2024-06-18 18:07] Timmy: arch is very hard compared to nixos

[2024-06-18 18:07] Timmy: just gotta learn a little nix

[2024-06-18 18:07] Deleted User: good!!

[2024-06-18 18:07] contificate: I promise it isn't

[2024-06-18 18:08] mrexodia: <#835664858526646313>

[2024-06-18 18:08] Timmy: (functional language)

[2024-06-18 18:08] mrexodia: no #distrofighting

[2024-06-18 18:08] Deleted User: 😔

[2024-06-18 18:08] contificate: it's not fighting

[2024-06-18 18:08] Timmy: https://tenor.com/view/joueur-du-grenier-no-fun-fun-fun-is-not-allowed-stop-it-gif-16558170

[2024-06-18 18:15] contificate: clown

[2024-06-18 18:16] mrexodia: <#835646666858168320> is the place

[2024-06-19 00:48] Torph: [replying to Timmy: "isnt there some way more neat technology written i..."]
if there is that would be cool. I was looking into ARM->x86 JIT, but decoding & handling every possible instruction seems extremely tedious, and using LLVM seems very frustrating

[2024-06-19 05:05] abu: [replying to contificate: "only vaguely creepy meta projects like cargo mummy..."]
true true true preach brother

[2024-06-19 09:25] Timmy: [replying to Torph: "if there is that would be cool. I was looking into..."]
https://c9x.me/compile/
https://github.com/RealNeGate/Cuik
[Embed: GitHub - RealNeGate/Cuik: A Modern C11 compiler (STILL EARLY)]
A Modern C11 compiler (STILL EARLY). Contribute to RealNeGate/Cuik development by creating an account on GitHub.

[2024-06-19 16:15] contificate: QBE is a bit simplistic

[2024-06-19 16:15] contificate: lacks a few notable features and mainly works for Linux

[2024-06-19 16:15] contificate: it lacks a switch instruction, for example

[2024-06-19 16:15] contificate: so all lowering of conditionals are kind of fucked in cproc

[2024-06-19 16:15] contificate: misses out on basic instruction selection stuff in its lowering as well

[2024-06-19 16:15] contificate: has a few branches to do with making automated selectors by tree pattern matching

[2024-06-19 16:15] contificate: author is based though, that's what counts

[2024-06-19 16:45] Torph: [replying to Timmy: "https://c9x.me/compile/
https://github.com/RealNeG..."]
cool, I'll look into this. looks like I'd still need to translate AArch64->QBE IR (is it QBE or Cuik?), but having it handle the x86 output would help a lot

[2024-06-19 16:46] Timmy: It's 2 of em, quic != qbe

[2024-06-19 16:47] Timmy: taking contificates words for granted (he likes this field), neither seem very mature sadly

[2024-06-19 16:48] Torph: [replying to Torph: "cool, I'll look into this. looks like I'd still ne..."]
also since it can output to assembly, I wonder if I could give it the C code for an OS translation layer that replaces the AArch64 system calls and output a native binary with pre-translated assembly code instead of using JIT style

[2024-06-19 16:48] Torph: [replying to Timmy: "It's 2 of em, quic != qbe"]
ohhh ok

[2024-06-19 16:49] Torph: [replying to Timmy: "taking contificates words for granted (he likes th..."]
I wouldn't mind digging into the library codebase and working on it as problems come up, my code isn't exactly mature either 😂

[2024-06-19 16:53] Timmy: yeah that's very fair haha

[2024-06-19 16:53] Timmy: I'd say go for qbe then it looks quite nice and hackable

[2024-06-19 17:00] contificate: I wouldn't

[2024-06-19 17:11] Timmy: <:OMEGALUL:662670462215782440>

[2024-06-19 17:21] Saturnalia: <:kekw:728766271772033046>

[2024-06-19 17:21] Saturnalia: >i dont know any C++ so i just took my python code asked chattgpt to convert it in C++ and tada its working its not this good but its working i tried to add color selection but its not working for me couse if i select other color (rgb code) then Purple the searcherino funktion stops working or gets really slow at it.

[2024-06-19 18:52] Deleted User: this makes me think im a somewhat decent programmer

[2024-06-19 19:20] HelpWare: i knew u guys would me fun of me

[2024-06-19 19:21] HelpWare: but for the start its pretty good?
yea obv gpt helped me but its working on purple outline

[2024-06-19 19:27] iPower: [replying to HelpWare: "but for the start its pretty good?
yea obv gpt hel..."]
read the fucking rules

[2024-06-19 19:28] iPower: if all you're doing here is asking for help for your valorant shit paste gtfo

[2024-06-19 19:28] rin: [replying to iPower: "if all you're doing here is asking for help for yo..."]
he has great potential

[2024-06-19 19:28] HelpWare: mb

[2024-06-19 19:29] HelpWare: i found this discord from a git post with cheats so i thought yall here abt cheating cracking

[2024-06-19 19:29] iPower: we're obviously not a cheating community. like I said, read the rules

[2024-06-19 19:30] HelpWare: [replying to iPower: "we're obviously not a cheating community. like I s..."]
yea i saw it now xd

[2024-06-19 19:31] iPower: [replying to rin: "he has great potential"]
i couldnt care less about potential if someone is breaking the server rules.

[2024-06-19 19:41] elias: I'm trying to compile a unix library for Windows and I'm getting a compile error at the following line
`return (-immr % 16) <= (15 - imms);`
`immr` is of type unsigned int so the compiler complains because of `-immr`: `unary minus operator applied to unsigned type, result still unsigned`.

I wonder, how would gcc compile this? Cast immr to signed?

[2024-06-19 19:46] Torph: I'm not sure... does anything noticable happen to the behaviour if you make `immr` signed on Windows vs. Unix versions? do you have unit tests around it so you can check for changing behaviour?

[2024-06-19 20:01] contificate: GCC will probably just `neg`

[2024-06-19 20:01] contificate: seems that your compiler gets upset because you're asking it to basically do a two's complement operation over a type that has no concept of being in that representation

[2024-06-19 20:01] contificate: could probably insert the right casts around it to get it to shut up

[2024-06-19 20:02] contificate: issue is always that unsigned to signed is always narrowing (with equal widths)

[2024-06-19 20:03] contificate: you could rewrite the negation in terms of equivalent operations

[2024-06-19 20:03] contificate: and just expect the compiler to emit the same shit

[2024-06-19 20:04] contificate: 
[Attachments: 2024-06-19-210435_695x182_scrot.png]

[2024-06-19 20:20] elias: thanks !

[2024-06-20 01:18] daax: [replying to HelpWare: "i found this discord from a git post with cheats s..."]
You read the git post but not the rules for the discord.

[2024-06-20 01:18] daax: Try not to forget to breathe. Bye.