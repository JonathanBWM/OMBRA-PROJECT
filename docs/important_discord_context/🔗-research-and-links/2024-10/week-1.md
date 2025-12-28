# October 2024 - Week 1
# Channel: #🔗-research-and-links
# Messages: 169

[2024-10-02 19:49] subgraphisomorphism: 
[Attachments: Figure_1.png]

[2024-10-02 19:54] subgraphisomorphism: 
[Attachments: Figure_2.png]

[2024-10-02 19:56] subgraphisomorphism: seems about right

[2024-10-02 19:59] subgraphisomorphism: 
[Attachments: Figure_3.png]

[2024-10-02 22:31] subgraphisomorphism: 
[Attachments: image.png]

[2024-10-02 22:31] subgraphisomorphism: over 50% of instructions in chrome are mov's, calls, or jccs... 🤨

[2024-10-02 22:34] Brit: [replying to subgraphisomorphism: "over 50% of instructions in chrome are mov's, call..."]
are we surprised?

[2024-10-02 22:34] Brit: I guess about the jccs

[2024-10-02 22:34] Brit: but movs and calls making up the majority of all code isn't that surprising no?

[2024-10-02 23:02] subgraphisomorphism: i thought it would be around 30%, i wasnt including cmov's, movsx, movzx either...

[2024-10-03 00:02] subgraphisomorphism: [replying to subgraphisomorphism: ""]

[Attachments: image.png]

[2024-10-03 05:28] subgraphisomorphism: (ntoskrnl.exe, includes prologs and epilogs)
[Attachments: image.png]

[2024-10-03 06:19] brymko: [replying to Brit: "are we surprised?"]
noving ever happens, all is just moving data around

[2024-10-03 06:58] subgraphisomorphism: 
[Attachments: image.png]

[2024-10-03 07:22] snowua: <:topkek:904522829616263178>

[2024-10-03 11:48] Deleted User: [replying to subgraphisomorphism: ""]
do you have a script for making these or are you doing it manually for now

[2024-10-03 12:44] Brit: I hear he's counting them out

[2024-10-03 12:44] Brit: and making dots on a sheet of paper then summing them up

[2024-10-03 12:44] Brit: by hand

[2024-10-03 12:44] Brit: looking at x64dbg

[2024-10-03 12:47] brymko: heard the same

[2024-10-03 12:48] brymko: but looking at notepad

[2024-10-03 12:48] Brit: disassemble by hand

[2024-10-03 12:48] Brit: looking at a reference sheet

[2024-10-03 12:48] Brit: for encodings

[2024-10-03 12:48] brymko: like a real man

[2024-10-03 12:51] Rairii: i've assembled sm83 payloads by hand before

[2024-10-03 13:09] Torph: [replying to brymko: "noving ever happens, all is just moving data aroun..."]
true !

[2024-10-03 18:05] subgraphisomorphism: [replying to Deleted User: "do you have a script for making these or are you d..."]
im using vtil to do it

[2024-10-03 18:06] Deleted User: [replying to Brit: "and making dots on a sheet of paper then summing t..."]
yes that's exactly what I meant <:omegalul:888053867869249637>

[2024-10-03 18:49] subgraphisomorphism: [replying to Deleted User: "do you have a script for making these or are you d..."]
i solved the halting problem

[2024-10-03 18:49] Deleted User: congrats

[2024-10-03 18:49] subgraphisomorphism: and achieved perfect disassembly

[2024-10-03 18:50] Brit: please give access to halting oracle as a service

[2024-10-03 18:53] subgraphisomorphism: https://tenor.com/view/dog-nose-butterfly-dog-puppy-peaceful-gif-14700589898488542320

[2024-10-03 21:41] Matti: [replying to subgraphisomorphism: ""]
clang... OK yes

[2024-10-03 21:42] Matti: [replying to subgraphisomorphism: "(ntoskrnl.exe, includes prologs and epilogs)"]
ntoskrnl.... sure

[2024-10-03 21:42] Matti: but what about ntoskrnl compiled by clang
[Attachments: ntoskrnl-cl.exe]

[2024-10-03 21:42] subgraphisomorphism: lemme see

[2024-10-03 21:42] Matti: [replying to Deleted User: "do you have a script for making these or are you d..."]
also this

[2024-10-03 21:42] Matti: why would I trust your disassembler

[2024-10-03 21:43] Matti: if you're not doing it by hand

[2024-10-03 21:43] Matti: and if you are doing it by hand, well then that's not better

[2024-10-03 21:46] subgraphisomorphism: [replying to Matti: "but what about ntoskrnl compiled by clang"]

[Attachments: image.png]

[2024-10-03 21:46] subgraphisomorphism: blare made sense of 98.2% of it

[2024-10-03 21:47] subgraphisomorphism: 
[Attachments: image.png]

[2024-10-03 21:47] Matti: hm yes

[2024-10-03 21:47] Matti: this looks right

[2024-10-03 21:47] subgraphisomorphism: 
[Attachments: inst-data.csv]

[2024-10-03 21:48] subgraphisomorphism: obfuscated it to 100mb

[2024-10-03 21:48] subgraphisomorphism: in 14.3s

[2024-10-03 21:49] subgraphisomorphism: 
[Attachments: image.png]

[2024-10-03 21:49] Matti: well if you want a bigger and slower kernel you can just get the windows 11 one

[2024-10-03 21:49] Matti: it's not 3 MB for nothing

[2024-10-03 21:49] subgraphisomorphism: we can probably make it faster using our jolt technology

[2024-10-03 21:49] subgraphisomorphism: wanna run the kernel with our injected basic block counters and give me profile data?

[2024-10-03 21:49] Matti: will it run? it it supposed to run? serious question

[2024-10-03 21:49] Matti: sure

[2024-10-03 21:50] subgraphisomorphism: so i can place hot basic blocks together and make jccs better by having the fallthrough go to the hot path

[2024-10-03 21:50] subgraphisomorphism: [replying to Matti: "will it run? it it supposed to run? serious questi..."]
it should run, does your winload verify ntoskrnl signature?

[2024-10-03 21:50] Matti: well I'm not gonna put it in my build chain, that's for sure

[2024-10-03 21:50] subgraphisomorphism: ive got ntoskrnl obfuscated running as we speak

[2024-10-03 21:50] subgraphisomorphism: you dont trust code defender in your CI pipeline???

[2024-10-03 21:50] Matti: but if you want me to test it and/or send a profile, np

[2024-10-03 21:50] subgraphisomorphism: github action is one click away !!!!!!

[2024-10-03 21:51] Matti: just waiting to see the source code so I can run the obfuscation part myself

[2024-10-03 21:51] subgraphisomorphism: [replying to Matti: "but if you want me to test it and/or send a profil..."]
ill have to write code to trigger a dump of the recorded information to disk.

[2024-10-03 21:51] Matti: and the graphs part....

[2024-10-03 21:51] subgraphisomorphism: [replying to Matti: "just waiting to see the source code so I can run t..."]
the source code?

[2024-10-03 21:52] Matti: yes

[2024-10-03 21:52] Matti: the code that does the whatever

[2024-10-03 21:52] subgraphisomorphism: its a very expensive saas product ✊

[2024-10-03 21:52] subgraphisomorphism: 😵

[2024-10-03 21:52] Matti: oh well I just need the source code, I don't need a license

[2024-10-03 21:53] subgraphisomorphism: wanna license it for 25mil a year with a clause stating you have to share 50% of the profits derived from any product using our framework?

[2024-10-03 21:53] subgraphisomorphism: 🤝

[2024-10-03 21:54] Matti: I'll have a talk with my accountant about that

[2024-10-03 21:54] Matti: I'm gonna guess the advice will be a no

[2024-10-03 21:54] Matti: something like liquidity issues bla bla

[2024-10-03 21:55] Matti: he might also say why do you want to do this anyway that's retarded, bla bla

[2024-10-03 21:55] Matti: but you know how accountants do

[2024-10-03 21:56] subgraphisomorphism: ight so im injecting a basic block counter into all 98.2% of functions

[2024-10-03 21:56] subgraphisomorphism: i didnt inject code to trigger a dump

[2024-10-03 21:57] subgraphisomorphism: of this profile information to disk, so youll need to read it out of memory yourself. I have hacky dump code for kernel drivers and usermode but not for ntoskrnl itself.

[2024-10-03 21:57] Matti: in the interest of full disclosure

[2024-10-03 21:57] Matti: this kernel will not actually boot yet

[2024-10-03 21:57] Matti: not past P0

[2024-10-03 21:58] Matti: due to codegen issues to be worked out

[2024-10-03 21:58] Matti: so np

[2024-10-03 21:58] subgraphisomorphism: mine boots

[2024-10-03 21:58] Matti: ya

[2024-10-03 21:58] Matti: I'm behind the curve

[2024-10-03 21:59] subgraphisomorphism: what version of ntoskrnl is this btw, do you also have win32k modules?

[2024-10-03 21:59] Matti: it should be in resources

[2024-10-03 21:59] Matti: 5.2

[2024-10-03 22:02] subgraphisomorphism: youll have to excuse my usage of pushfq/popfq. we take proper exception unwinding very seriously when it comes to obfuscation, but for profiling bins it doesnt matter.

[2024-10-03 22:02] Matti: ok first question

[2024-10-03 22:02] Matti: how am I gonna debug it without my PDB
[Attachments: image.png]

[2024-10-03 22:02] subgraphisomorphism: load it in ida

[2024-10-03 22:03] Matti: but IDA isn't a debugger

[2024-10-03 22:03] Matti: I also don't use the default pdb plugin in IDA, is that gonna be an issue

[2024-10-03 22:03] subgraphisomorphism: it means you need to contact back engineering labs for the updated pdb

[2024-10-03 22:04] Matti: ok

[2024-10-03 22:04] subgraphisomorphism: 😉

[2024-10-03 22:04] Matti: well you seem to have contacts there

[2024-10-03 22:04] Matti: please send me the pdb

[2024-10-03 22:04] subgraphisomorphism: you didnt give me a pdb so i generated a map file for u

[2024-10-03 22:05] subgraphisomorphism: updating the pdb (its WIP atm) is a selling point for CodeDefender. Game studios that get crash dumps need to know where its happening.

[2024-10-03 22:06] Matti: well at this point I'm not *that* interested in playing along with the elaborate funny to send a PDB and get some other joke back probably

[2024-10-03 22:06] Matti: but you could've just asked

[2024-10-03 22:06] subgraphisomorphism: look in the map file, it has the ranges mapped back to the original

[2024-10-03 22:07] subgraphisomorphism: look at ntoskrnl in ida lol

[2024-10-03 22:07] Matti: yes, I understood this part

[2024-10-03 22:07] Matti: I'll just go write a windbg plugin that can use this remapping then

[2024-10-03 22:07] Matti: brb

[2024-10-03 22:08] subgraphisomorphism: 🙏

[2024-10-03 22:08] subgraphisomorphism: do you want me to not overwrite the pdb?

[2024-10-03 22:08] Matti: I can un-overwite it back myself I think

[2024-10-03 22:09] Matti: just doubt it would help very much somehow

[2024-10-03 22:09] mrexodia: 
[Attachments: image.png]

[2024-10-03 22:09] mrexodia: <@1290450044864172153> irl

[2024-10-03 22:09] subgraphisomorphism: mrmossadia, dont even start!

[2024-10-03 22:12] subgraphisomorphism: <@162611465130475520> we will sponsor you if you accept our vtil award

[2024-10-03 22:12] mrexodia: how big sponsor?

[2024-10-03 22:13] Matti: alright well it applied a whole 174 types from the  original PDB

[2024-10-03 22:13] Matti: that's more than 0

[2024-10-03 22:13] subgraphisomorphism: not bad

[2024-10-03 22:13] Matti: so points for that

[2024-10-03 22:13] Matti: this needs work though
[Attachments: image.png]

[2024-10-03 22:13] subgraphisomorphism: yeah you need to give us the original

[2024-10-03 22:14] subgraphisomorphism: so we can output a new pdb for the bin we produce. not all compilers can even emit pdb files btw so we have that map file format for a reason.

[2024-10-03 22:14] Matti: more original than this?

[2024-10-03 22:14] Matti: oh you mean the same file but with the pdb

[2024-10-03 22:14] subgraphisomorphism: yeah, the code isnt finished for updating the pdb yet

[2024-10-03 22:14] Matti: see above, I guess

[2024-10-03 22:15] subgraphisomorphism: but the goal will be to produce a *new* pdb file given the original pdb file. the pdb file is a crazzzzzy file format though. Ida has her own parser for the pdb

[2024-10-03 22:15] subgraphisomorphism: mrmossadia uses dbghelp, windbg also uses dbghelp but differently than x64dbg

[2024-10-03 22:16] subgraphisomorphism: its truly an amazing world we live in

[2024-10-03 22:16] Matti: yeah I know about the IDA parser, that's why i have my own pdb plugin

[2024-10-03 22:17] Matti: if ilfak cnn't do it right then it does seem like you're right about this

[2024-10-03 22:18] Matti: ok, but I'm used to waiting an hour for UE to link with full LTO and PGO anyway

[2024-10-03 22:18] Matti: what does this actually get me at runtime

[2024-10-03 22:18] subgraphisomorphism: unreal engine demo files in about the same too. 200mb files

[2024-10-03 22:19] Matti: yeah but my point is that I don't care very much about how fast you link it

[2024-10-03 22:19] Matti: otherwise I could just use a dogshit compiler from MS

[2024-10-03 22:19] Matti: I'm not a game studio

[2024-10-03 22:20] Matti: I'm only asking how it will make my code perform better at runtime

[2024-10-03 22:21] Matti: or different in any way that I might like, aactually want

[2024-10-03 22:21] subgraphisomorphism: so you run the profiled bin, gather profile information. we then group blocks that are hot closer together and rewrite jcc instructions so that the fallthrough becomes the hot path

[2024-10-03 22:21] subgraphisomorphism: facebook did this with bolt and they saw some interesting speedups.

[2024-10-03 22:22] subgraphisomorphism: https://research.facebook.com/file/534990324471927/BOLT-A-Practical-Binary-Optimizer-for-Data-Centers-and-Beyond.pdf

[2024-10-03 22:22] Matti: uhuh, but bolt is already part of LLVM

[2024-10-03 22:22] Matti: as are PGI and PGO

[2024-10-03 22:22] subgraphisomorphism: thats cool, what happens if your using msvc or your not interested in recompiling your entire program

[2024-10-03 22:23] subgraphisomorphism: we arent selling post-compilation optimizations fyi

[2024-10-03 22:23] subgraphisomorphism: its just something we could do, its interesting

[2024-10-03 22:23] Matti: then it's probably not me but an impostor wanting to do those things

[2024-10-03 22:23] mrexodia: [replying to subgraphisomorphism: "mrmossadia uses dbghelp, windbg also uses dbghelp ..."]
i dont

[2024-10-03 22:23] Matti: so I'm not intersted in this use case

[2024-10-03 22:24] subgraphisomorphism: others are

[2024-10-03 22:25] Matti: alright! well I know I'm an unusual target audience, and you've done your market research

[2024-10-03 22:26] Matti: so good luck with it

[2024-10-03 22:26] subgraphisomorphism: [replying to subgraphisomorphism: "we arent selling post-compilation optimizations fy..."]
^

[2024-10-03 22:26] Matti: I read and understood the post the first time, what about it?

[2024-10-03 22:27] Matti: this excludes me from the list of people who might be interested is all, but we already understood this I believe

[2024-10-04 04:17] Torph: [replying to mrexodia: ""]
so based.

[2024-10-04 04:17] Torph: [replying to Matti: "then it's probably not me but an impostor wanting ..."]
<:topkek:904522829616263178>

[2024-10-04 04:20] Torph: [replying to Matti: "but what about ntoskrnl compiled by clang"]
did they start using Clang at some point, or is this WRK or from leaked sources or something?

[2024-10-06 22:31] brew002: Just finished my first blog post:
https://brew02.github.io/posts/2024/budget-ept-hooks.html
GitHub POC too:
https://github.com/brew02/BudgetEPT
[Embed: GitHub - brew02/BudgetEPT: Create stealthy, inline, EPT-like hooks ...]
Create stealthy, inline, EPT-like hooks using SMAP and SMEP - brew02/BudgetEPT