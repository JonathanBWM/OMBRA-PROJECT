# May 2024 - Week 1
# Channel: #programming
# Messages: 276

[2024-05-02 00:05] x86matthew: [replying to Brit: "to be fair I think there was only one solve when m..."]
i'm supposed to be enjoying my holiday, don't tempt me with windows challenges 😢

[2024-05-02 00:42] x86matthew: done
[Attachments: image.png]

[2024-05-02 09:13] Brit: GZGZ, that is an actual solve too, no monkeypatching

[2024-05-02 09:16] luci4: [replying to x86matthew: "done"]
Could I get a hint? 🥹

[2024-05-02 10:13] Brit: [replying to luci4: "Could I get a hint? 🥹"]
the hint you get is in the name of the binary sir

[2024-05-02 10:15] luci4: [replying to Brit: "the hint you get is in the name of the binary sir"]
Yeah I got that... Seems like I need to get a better understanding of VEH to solve it

[2024-05-02 10:53] Azalea: aight lets take a crack at this tmr

[2024-05-02 11:37] Azalea: anyone has a list of the exception codes?

[2024-05-02 11:39] Azalea: im not finding 0xc00000096

[2024-05-02 11:45] Brit: priv instr

[2024-05-02 11:46] Brit: https://github.com/wine-mirror/wine/blob/master/include/winnt.h#L670
[Embed: wine/include/winnt.h at master · wine-mirror/wine]
Contribute to wine-mirror/wine development by creating an account on GitHub.

[2024-05-02 11:46] Azalea: aight thanks

[2024-05-02 11:47] diversenok: [replying to Azalea: "im not finding 0xc00000096"]
0xc00000096 does not fit into a 32-bit int

[2024-05-02 11:47] Azalea: does int3 trigger STATUS_BREAKPOINT or STATUS_SINGLE_STEP

[2024-05-02 11:47] Azalea: [replying to diversenok: "0xc00000096 does not fit into a 32-bit int"]
there may be a different number of zeroes

[2024-05-02 11:48] Brit: breakpoint

[2024-05-02 11:48] Azalea: ah thanks

[2024-05-02 11:49] diversenok: Yep, single step requires the TF flag in the thread context

[2024-05-02 11:52] Azalea: if a VEH triggers on an instruction is the RIP at the current instruction or the next instruction

[2024-05-02 11:52] Azalea: i think its at current

[2024-05-02 11:52] Azalea: just making sure

[2024-05-02 11:53] Brit: the code in the challenge should tell you that xdd

[2024-05-02 15:27] Brit: it does not

[2024-05-02 15:28] Brit: there's a fairly obvious way to win that way

[2024-05-02 16:19] luci4: Why is it called "monkey" patching btw?

[2024-05-02 16:20] luci4: I thought of patching it, but cheesing the challenge kinda defeats the purpose

[2024-05-02 16:29] Brit: it's a bit of a misnomer anyways, monkeypatch implies a patch at runtime which would also defeat the purpose, but here the goal is to extract that flag because that's what makes you face the most hardship and is also the most "fun".

[2024-05-02 18:17] helplesness: Is there any public obfuscator that is considered good?

[2024-05-02 19:18] sariaki: [replying to helplesness: "Is there any public obfuscator that is considered ..."]
https://github.com/es3n1n/obfuscator <:1000:888260191961899040>
[Embed: GitHub - es3n1n/obfuscator: PE bin2bin obfuscator]
PE bin2bin obfuscator. Contribute to es3n1n/obfuscator development by creating an account on GitHub.

[2024-05-02 20:07] contificate: still feel like better solutions would just do it over a compiler's IR

[2024-05-02 20:08] contificate: major copes against this take, "we don't typically use clang on Windows, so no LLVM IR", "Tigress only accepts C", "doesn't work well with C++ exceptions", "nobody wants to distribute LLVM IR, let alone source of their programs"

[2024-05-02 20:48] Brit: [replying to contificate: "still feel like better solutions would just do it ..."]
I write Malbolge (unshackled) that's executed in V8 in a cef browser, can't use llvm...

[2024-05-03 01:13] Deleted User: [replying to contificate: "still feel like better solutions would just do it ..."]
the blog mentions that

[2024-05-03 01:13] Deleted User: idk i like the struggle :3

[2024-05-03 17:10] flower: [replying to contificate: "still feel like better solutions would just do it ..."]
been doing this for personal projects for a while... LLVM is a blast to work with i don't get why anyone puts in the effort to implement bin2bin

[2024-05-03 17:10] contificate: <:gigachad:904523979249815573>

[2024-05-03 17:14] Deleted User: [replying to flower: "been doing this for personal projects for a while...."]
because challenges are fun! and for me its easier to integrate it into my project, i just have to provide the binary nothing else

[2024-05-03 17:17] Brit: [replying to flower: "been doing this for personal projects for a while...."]
can't well ask your customers to send you the source to their very cool totally not malware/gamehacks

[2024-05-03 17:17] Brit: so you end up either doing bin2bin

[2024-05-03 17:18] flower: I mean, yeah of course bin2bin has its uses but for my personal projects i control the build environment so the easiest choice ended up being LLVM

[2024-05-03 17:19] Deleted User: yea thats fair, i tried around with llvm plugins and rust a little bit but its weird on windows

[2024-05-03 17:20] flower: How so?

[2024-05-03 17:20] Deleted User: its not supported by default, i had to use a fork

[2024-05-03 17:21] Deleted User: maybe they will merge it with the upstream rust llvm fork at some point, its not alot of changes tbh.

[2024-05-03 17:22] Deleted User: <https://github.com/jamesmth/llvm-project/tree/llvm-17-rust-1.75>

[2024-05-03 18:59] helplesness: [replying to contificate: "major copes against this take, "we don't typically..."]
Tigress actually looks nice. Tnx

[2024-05-03 18:59] helplesness: Do you have any resources to learn more about obfuscation?

[2024-05-03 19:00] contificate: learn compilers

[2024-05-03 19:00] contificate: or read surreptitious software for basics

[2024-05-03 19:00] helplesness: Tigress book?

[2024-05-03 19:00] contificate: is by same authors, yes

[2024-05-03 19:00] helplesness: ok

[2024-05-03 19:18] flower: [replying to helplesness: "Do you have any resources to learn more about obfu..."]
thousands of papers on obfuscation techniques, go wild

[2024-05-03 19:34] lavclash: hi, i'm trying to learn c++ for exploit dev , and trying to recreate a vulnerability that I found it and the POC was basically powershell and so noisy its symlink abuse arbitrary file delete arbitrary move  and arbitrary create file

[2024-05-03 19:35] lavclash: I'm trying to understand the code for this vuln to recreate to my specific vuln :
https://github.com/Wh04m1001/MSIExecEoP
[Embed: GitHub - Wh04m1001/MSIExecEoP: Arbitrary File Delete in Windows Ins...]
Arbitrary File Delete in Windows Installer before 10.0.19045.2193 - Wh04m1001/MSIExecEoP

[2024-05-04 21:06] Deleted User: [replying to contificate: "learn compilers"]
what's the best way to start with this?

[2024-05-04 21:07] contificate: understanding how to represent and manipulate tree structured data in programs

[2024-05-04 21:08] contificate: which sounds a bit like I'm saying to flail around with binary trees or something

[2024-05-04 21:08] contificate: but I really mean how you'd represent ASTs, IRs, etc.

[2024-05-04 21:08] Deleted User: [replying to contificate: "which sounds a bit like I'm saying to flail around..."]
ah, any books or like resources u wld recommend?

[2024-05-04 21:08] contificate: Modern Compiler Implementation in ML is a good start

[2024-05-04 21:08] contificate: but if your interests are more

[2024-05-04 21:09] contificate: related to obfuscation and virtualisation etc.

[2024-05-04 21:09] Deleted User: [replying to contificate: "related to obfuscation and virtualisation etc."]
yeah they are

[2024-05-04 21:09] contificate: can probably speedrun a few important concepts, algorithmic ideas, etc.

[2024-05-04 21:09] contificate: to avoid random front-end concerns you won't care about (but are interesting anyway)

[2024-05-04 21:10] contificate: you should maybe selectively read

[2024-05-04 21:10] contificate: Engineering a Compiler by Cooper and Torczon

[2024-05-04 21:10] Deleted User: [replying to contificate: "Modern Compiler Implementation in ML is a good sta..."]
what about Modern Compiler Implement in C?

[2024-05-04 21:10] contificate: well

[2024-05-04 21:10] contificate: we come back around to my first point

[2024-05-04 21:10] contificate: you can read any of the MCI books (not the 2nd edition of the Java version though)

[2024-05-04 21:11] contificate: if you understand

[2024-05-04 21:11] contificate: representing and working with tree shaped datatypes

[2024-05-04 21:11] contificate: i.e. in C

[2024-05-04 21:11] contificate: a bunch of tagged unions

[2024-05-04 21:11] contificate: and manual pattern matching code

[2024-05-04 21:11] Deleted User: alright, thank you very much

[2024-05-04 21:12] contificate: if you wanna be very practical then

[2024-05-04 21:12] contificate: gotta get into LLVM asap

[2024-05-04 21:12] contificate: assuming you want to work with LLVM

[2024-05-04 21:12] Deleted User: [replying to contificate: "assuming you want to work with LLVM"]
yeah ofc

[2024-05-04 21:12] contificate: there are many people who work with LLVM all day

[2024-05-04 21:12] contificate: who lack fundamental compiler background

[2024-05-04 21:12] Deleted User: [replying to contificate: "gotta get into LLVM asap"]
any resources u would recommend for learning ab llvm infrastructure?

[2024-05-04 21:12] Deleted User: i wanna learn the theory first before i do anything practical

[2024-05-04 21:12] contificate: can you write C

[2024-05-04 21:13] Deleted User: yeah

[2024-05-04 21:13] contificate: I recommend learning LLVM IR concepts by doing

[2024-05-04 21:13] contificate: writing programs in the IR

[2024-05-04 21:13] contificate: similar kinds of algorithmic problems you may do in C

[2024-05-04 21:13] contificate: but you also need to understand some LLVM specific things

[2024-05-04 21:13] contificate: and also SSA form

[2024-05-04 21:14] contificate: and all that entails

[2024-05-04 21:14] contificate: classic data flow analysis too

[2024-05-04 21:14] contificate: properties of CFGs like dominators and (iterated) dominance frontiers

[2024-05-04 21:14] contificate: seems like there's a lot, but there's not really

[2024-05-04 21:14] contificate: in actual reality

[2024-05-04 21:15] contificate: you could write a simple arithmetic obfuscator in very few lines

[2024-05-04 21:15] Deleted User: how long do you think it would take to write a lifter from having 0 background knowledgde

[2024-05-04 21:15] contificate: not very long

[2024-05-04 21:15] Deleted User: a year?

[2024-05-04 21:15] contificate: less than that

[2024-05-04 21:15] contificate: but to be clear

[2024-05-04 21:16] contificate: I mean a subset of some well known architecture

[2024-05-04 21:16] contificate: shit is basically pattern matching all the way down

[2024-05-04 21:16] contificate: if you've used Ghidra, you've implicitly used matchers generated from Ramsey et al's work on SLED (turned into Sleigh in Ghidra)

[2024-05-04 21:17] contificate: so their specifications basically match and produce (lift) to PCODE to model semantics

[2024-05-04 21:17] contificate: of the underlying instructions

[2024-05-04 21:17] Deleted User: Ahhh

[2024-05-04 21:17] contificate: explicit status and flag regs

[2024-05-04 21:17] contificate: yadda yadda

[2024-05-04 21:17] Deleted User: well, where would i start, if i wanted to write a lifter

[2024-05-04 21:18] Deleted User: to llvm ir from x8664 asm

[2024-05-04 21:18] contificate: you need a really good understanding of x86_64

[2024-05-04 21:19] contificate: as you have to model implicit semantics explicitly

[2024-05-04 21:19] contificate: but I mean, honestly

[2024-05-04 21:19] Deleted User: [replying to contificate: "you need a really good understanding of x86_64"]
as well as llvm ir?

[2024-05-04 21:19] contificate: if you imagine that most programs are just the same 10 instructions

[2024-05-04 21:19] contificate: over and over again

[2024-05-04 21:19] contificate: you could do a demo that lifts a tiny subset of x86_64 to LLVM IR

[2024-05-04 21:19] contificate: in a very short period of time

[2024-05-04 21:20] Deleted User: Interesting

[2024-05-04 21:20] contificate: why do you want to lift, anyway

[2024-05-04 21:20] contificate: people often have this idea that they should

[2024-05-04 21:20] contificate: work _from_ machine code

[2024-05-04 21:20] Deleted User: well, i heard i could lift and optimise to deobfuscate

[2024-05-04 21:20] contificate: upwards and then back downwards

[2024-05-04 21:20] contificate: ah

[2024-05-04 21:20] contificate: yeah you can

[2024-05-04 21:20] contificate: but I mean

[2024-05-04 21:20] Deleted User: yeah

[2024-05-04 21:20] contificate: I am sceptical

[2024-05-04 21:21] Deleted User: howcome?

[2024-05-04 21:21] Deleted User: not only that, i wanna write a dissasembler and decompiler so i wanna know how this shit works in depth

[2024-05-04 21:21] contificate: because it's not like there can exist a automated procedure that achieves some relevant program metric reduction

[2024-05-04 21:21] contificate: via transformation

[2024-05-04 21:21] contificate: which is to say that

[2024-05-04 21:21] contificate: your transformations and reliance on opts from LLVM or whatever

[2024-05-04 21:22] contificate: may capture basic cases

[2024-05-04 21:22] contificate: but the same intractability that the compilers face

[2024-05-04 21:22] contificate: faces you as well

[2024-05-04 21:22] contificate: you can disrupt many modern compilers with a bunch of bullshit

[2024-05-04 21:22] contificate: and it's not hard to create novel obfuscators, conceptually

[2024-05-04 21:22] contificate: actually implementing them is tedious

[2024-05-04 21:22] contificate: as you maybe realise that certain things aren't safe to perform in general

[2024-05-04 21:23] contificate: and deciding if you can perform the transformation can also be undecidable

[2024-05-04 21:23] contificate: as with many things in program analysis

[2024-05-04 21:23] contificate: see rice's theorem for the cope that all undergrads studying theory of comp memorise

[2024-05-04 21:23] Deleted User: [replying to contificate: "faces you as well"]
Howcome?

[2024-05-04 21:23] contificate: if you accept exorbitant runtime cost

[2024-05-04 21:23] contificate: the ceiling of how complicated and annoying your obfuscated program can be

[2024-05-04 21:24] contificate: can get higher and higher

[2024-05-04 21:24] contificate: the usual, mainstream, transformations

[2024-05-04 21:24] contificate: in products

[2024-05-04 21:24] contificate: are not that novel in the first place

[2024-05-04 21:24] contificate: but are also ranked by performance decrease

[2024-05-04 21:24] contificate: nobody wants to have huge slowdowns

[2024-05-04 21:24] contificate: (or they will accept huge slowdowns in some places, but not others)

[2024-05-04 21:25] contificate: the point is

[2024-05-04 21:25] contificate: it's a game where

[2024-05-04 21:25] contificate: a deobfuscator can recognise a bunch of patterns as best it can

[2024-05-04 21:25] contificate: and maybe make some good progress

[2024-05-04 21:25] contificate: but a few weeks later

[2024-05-04 21:25] Deleted User: well ye, i'm talking about minor obfuscation in this case that optimisation will remove like

[2024-05-04 21:25] contificate: a newer, different, transformation is performed

[2024-05-04 21:26] Deleted User: opaque predicates etc

[2024-05-04 21:26] contificate: opaque predicates can be tricky and probably require smt solving

[2024-05-04 21:26] contificate: opaque predicates may also not be trivial to work with

[2024-05-04 21:26] contificate: all kinds of aliasing fuckery can be put in

[2024-05-04 21:26] Deleted User: True

[2024-05-04 21:26] contificate: I believe many identification strategies for opaque preds are literally

[2024-05-04 21:27] contificate: "this random bitwise test looks out of place 🥴 "

[2024-05-04 21:27] Deleted User: But for me it's worth learning this stuff bc i think it's related to

[2024-05-04 21:27] Deleted User: [replying to Deleted User: "not only that, i wanna write a dissasembler and de..."]
.

[2024-05-04 21:27] contificate: on paper, disassemblers ought to be easy

[2024-05-04 21:27] contificate: (so easy in fact that, as mentioned, automated tools can do most of it - see LLVM, Ghidra, etc.)

[2024-05-04 21:27] Deleted User: [replying to contificate: "on paper, disassemblers ought to be easy"]
r there good books u would recommend?

[2024-05-04 21:28] Deleted User: like i don't know where to learn this shit

[2024-05-04 21:28] contificate: but you get hit with target concerns

[2024-05-04 21:28] contificate: everywhere

[2024-05-04 21:28] contificate: so much lore

[2024-05-04 21:28] contificate: good decompilers are not so straightforward

[2024-05-04 21:28] contificate: for obvious, well known, reasons

[2024-05-04 21:28] Deleted User: yeah, i can imagine they're much harder

[2024-05-04 21:29] contificate: if you understand how instructions are encoded

[2024-05-04 21:29] Brit: the good thing is that you don't need to write a lifter, remill exists

[2024-05-04 21:29] Brit: the hard problem is optimizing away obfuscations anyway

[2024-05-04 21:29] contificate: and how to represent instructions themselves

[2024-05-04 21:29] contificate: in your program

[2024-05-04 21:29] contificate: in a convenient way

[2024-05-04 21:29] Brit: oh and enjoy all the MBA garbage

[2024-05-04 21:29] Brit: <:yea:904521533727342632>

[2024-05-04 21:29] contificate: you can write a disassembler

[2024-05-04 21:29] contificate: but it's tedious work

[2024-05-04 21:29] Deleted User: [replying to Brit: "the good thing is that you don't need to write a l..."]
i want to learn about them

[2024-05-04 21:29] Deleted User: thats why

[2024-05-04 21:29] contificate: hence automated mostly in other tools

[2024-05-04 21:30] contificate: a good compilers education is a large effort

[2024-05-04 21:30] contificate: but I feel that

[2024-05-04 21:30] contificate: the background you get from it

[2024-05-04 21:30] contificate: largely captures everything you'd want to know

[2024-05-04 21:30] Deleted User: yeah

[2024-05-04 21:30] contificate: to apply the ideas to security

[2024-05-04 21:30] Deleted User: how long you been learning ab this stuff?

[2024-05-04 21:31] contificate: 6 days

[2024-05-04 21:31] Deleted User: fr?

[2024-05-04 21:31] Deleted User: idk u seem experienced

[2024-05-04 21:31] Deleted User: but anyways thankjs

[2024-05-04 21:32] contificate: but yeah

[2024-05-04 21:32] contificate: start writing programs that work with representations of programs

[2024-05-04 21:32] contificate: this kind of shit is a huge thnigs missing from conventional programming education

[2024-05-04 21:32] contificate: get a lot of people who can iterate over vectors and call functions

[2024-05-04 21:33] contificate: but not a lot of actually representing inductive data types and doing structural recursion

[2024-05-04 21:33] contificate: especially if background is mostly languages like C or whatever

[2024-05-04 21:33] Deleted User: c/c++

[2024-05-04 21:33] Deleted User: should i just master the lang before i move on?

[2024-05-04 21:33] contificate: yeah so those are tedious languages for compiler related work

[2024-05-04 21:34] Deleted User: fr?

[2024-05-04 21:34] Deleted User: i thought they'd be the best

[2024-05-04 21:34] Deleted User: what's better than them

[2024-05-04 21:38] contificate: for moving mountains, you really want languages where ideas core to compilers (or certain domains of programming in general) are first class concepts in the language

[2024-05-04 21:38] contificate: but many of those aren't practical for other reasons

[2024-05-04 21:38] contificate: but basically

[2024-05-04 21:39] contificate: if you knew how much of LLVM is powered by a custom back-end for a DSL they maintain

[2024-05-04 21:39] Deleted User: DSL?

[2024-05-04 21:39] contificate: you'd understand why it's not like they just implement all of it purely in handwritten C++ code

[2024-05-04 21:39] contificate: domain specific language

[2024-05-04 21:39] Deleted User: og

[2024-05-04 21:39] contificate: point is

[2024-05-04 21:39] Deleted User: oh

[2024-05-04 21:39] contificate: if you look at clang, gcc, go's compiler, cranelift, etc.

[2024-05-04 21:40] contificate: they all use a DSL they maintain

[2024-05-04 21:40] contificate: to describe how to do things that are pretty core to compilers

[2024-05-04 21:40] contificate: primarily pattern matching

[2024-05-04 21:40] contificate: over trees and DAGs

[2024-05-04 21:40] contificate: along with some other shit

[2024-05-04 21:41] contificate: I'd say that for doing many experimental things:
- garbage collection
- pattern matching
- algebraic datatypes

[2024-05-04 21:41] contificate: really work well for many parts of compilers

[2024-05-04 21:41] contificate: but in the real world, there's things imposed by history, the environment you work in, other concerns outwith the scope of compilers, etc.

[2024-05-04 21:42] contificate: but certainly for pedagogy in compilers

[2024-05-04 21:42] contificate: C and C++ are far from ideal

[2024-05-04 21:42] contificate: so much tedium

[2024-05-04 21:42] contificate: extreme amounts of cruff to do basic things

[2024-05-04 21:42] contificate: when you're learning things, you don't want to be troubled by the burden of implementation

[2024-05-04 21:42] contificate: but C and C++

[2024-05-04 21:43] contificate: are only burden of implementation

[2024-05-04 21:43] Deleted User: I see

[2024-05-04 21:43] Deleted User: Thank you

[2024-05-04 21:44] contificate: I used to come on VC and prove this claim by writing toy compilers in OCaml

[2024-05-04 21:44] contificate: and then contrasting them with the C++ versions

[2024-05-04 21:44] contificate: but no time for such antics these days

[2024-05-04 21:45] Torph: [replying to contificate: "are only burden of implementation"]
trying to do some transpiler stuff, just had to implement my own queue & memory stream in C. agreed.

[2024-05-04 21:54] asz: i like the challenge of doing it with limited toolbox

[2024-05-04 21:56] asz: 
[Attachments: image.png]

[2024-05-04 21:57] asz: pushing preprocessor to new limits

[2024-05-05 00:38] Torph: oh what the hell
that's worse than the preprocessor bullshit I did today

[2024-05-05 00:38] Torph: 1st photo replaces all the functions in the second
[Attachments: 2024-05-04_17-40.png, 2024-05-04_17-42.png]

[2024-05-05 00:38] Torph: [replying to asz: ""]
do the dollar signs have some special meaning? I've never seen anyone use them in C

[2024-05-05 00:40] Torph: and what's with the whitespace between `#` and `define`?

[2024-05-05 00:51] naci: [replying to Deleted User: "well, i heard i could lift and optimise to deobfus..."]
you can, but you will run to a problem where llvm doesnt/wont optimize a certain type of expression and you will have to write custom passes. And you probably will have to modify the backend if you want to achieve a more realistic asm output. It makes life easy but certainly not a cheat code

[2024-05-05 00:51] avx: [replying to Torph: "1st photo replaces all the functions in the second"]
the font be fonting

[2024-05-05 00:54] naci: [replying to naci: "you can, but you will run to a problem where llvm ..."]
this is my own lifter, worked on it for a long time, still developing it. https://github.com/NaC-L/Mergen

[2024-05-05 01:34] JustMagic: [replying to Torph: "do the dollar signs have some special meaning? I'v..."]
nope

[2024-05-05 01:34] JustMagic: [replying to Torph: "and what's with the whitespace between `#` and `de..."]
just a way of formatting. One of the common ways to do indentation for macros

[2024-05-05 01:36] JustMagic: [replying to Torph: "1st photo replaces all the functions in the second"]
where's the do { } while(false) to not get warning about trailing semicolon reeee

[2024-05-05 03:18] Torph: [replying to JustMagic: "where's the do { } while(false) to not get warning..."]
oh I was actually looking for a way to fix that issue, thanks. I saw the in the linux kernel style guide they want you to use that for function-like macros, but they didn't mention why

[2024-05-05 06:18] 0xatul: [replying to asz: ""]
I sometimes wonder if you've made a Linq counterpart in your tooling using just macros

[2024-05-05 06:27] donna🤯: [replying to Torph: "1st photo replaces all the functions in the second"]
oo what theme is that?

[2024-05-05 06:27] donna🤯: + font

[2024-05-05 07:13] Timmy: ^ would love to know the font

[2024-05-05 07:14] Timmy: been using monoisome myself iirc, not 100% sure what to think of it

[2024-05-05 07:18] Torph: [replying to Timmy: "^ would love to know the font"]
ProFont2, the filename is `ProFontIIxNerdFontMono-Regular.ttf`

[2024-05-05 07:19] Torph: [replying to donna🤯: "oo what theme is that?"]
it's whatever Kickstart NeoVim was shipping like 9 months ago, they changed the theme since then & I don't know how to check what I have installed

[2024-05-05 10:26] Deleted User: [replying to naci: "this is my own lifter, worked on it for a long tim..."]
thank u so much, where and how did you learn to write one

[2024-05-05 12:28] naci: alot of trial and error and sometimes getting "inspired" by other lifters/compilers