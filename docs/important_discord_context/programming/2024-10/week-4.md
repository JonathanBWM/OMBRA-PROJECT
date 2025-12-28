# October 2024 - Week 4
# Channel: #programming
# Messages: 82

[2024-10-22 00:21] psykana: What's a not-brain-tumor-inducing way to make a proper array/iterator from a pointer in c++? I have `nextrel` elements of type `relocation_info` at address `span.data()`.
```
relocation_info(*relocations) = (relocation_info(*))span.data();
// relocations++
```

[2024-10-22 00:32] JustMagic: [replying to psykana: "What's a not-brain-tumor-inducing way to make a pr..."]
wha

[2024-10-22 03:48] North: [replying to psykana: "What's a not-brain-tumor-inducing way to make a pr..."]
can you cast it into an std::array?

[2024-10-22 03:48] North: idk if you can but if you can i think that would technically work

[2024-10-22 03:49] North: wait

[2024-10-22 03:49] North: std::span

[2024-10-22 03:50] North: not sure if span has an iterator though

[2024-10-22 04:02] North: you cannot use std::span

[2024-10-22 08:41] psykana: that span is coming from elsewhere as <const uint8_t>, I need to cast it into something managable

[2024-10-22 09:02] prick: C++ user trips on C++ abstractions with something like relocations

[2024-10-22 09:02] prick: truly a beautiful sight

[2024-10-22 09:02] Brit: nah this is just a skill issue

[2024-10-22 09:03] prick: i don't think C++ belongs in a codebase that touches microsoft stuff

[2024-10-22 09:03] prick: case on point: ATL, MFC, COM

[2024-10-22 09:04] prick: any time i've seen a low level thing that tries to shoehorn C++ everywhere into it, it's just a disaster to read

[2024-10-22 09:10] Brit: it's okay to have wrong opinions

[2024-10-22 09:16] psykana: [replying to prick: "C++ user trips on C++ abstractions with something ..."]
I am new to C++ 👉 👈

[2024-10-22 09:16] psykana: it absolutely is a skill issue

[2024-10-22 09:22] psykana: it has been treating me well so far, with the exception of io streams, that shit is devil's work

[2024-10-22 09:35] 5pider: [replying to Brit: "it's okay to have wrong opinions"]
LMFAO

[2024-10-22 13:06] Torph: [replying to prick: "C++ user trips on C++ abstractions with something ..."]
haha I tripped on relocations pretty hard when I was first learning about PEs

[2024-10-22 13:17] prick: this is not quite the world's dumbest bugfix but it's pretty close
[Attachments: image.png]

[2024-10-22 13:25] dinero: wtf is this

[2024-10-22 13:25] dinero: oh

[2024-10-22 13:25] dinero: zig

[2024-10-22 13:25] dinero: it’s over

[2024-10-22 15:19] 0x208D9: does anyone have any documentations related to using VEX and VEX IR with libvex in cpp?

[2024-10-22 15:20] 0x208D9: i couldnt found one in google

[2024-10-22 16:00] prick: <@1014596930392821820> try /none/tests/libvex_test.c in valgrind source tree

[2024-10-22 16:00] prick: it has *some* stuff

[2024-10-22 16:00] prick: or you could see what is available that uses the pyvex package and read the binding source if you want to translate it that badly

[2024-10-22 16:01] 0x208D9: [replying to prick: "or you could see what is available that uses the p..."]
read pyvex but i cant read ffi of python cuz im half asleep

[2024-10-22 16:01] 0x208D9: so idk how the lifting happens

[2024-10-22 16:01] 0x208D9: in pyvex

[2024-10-22 16:02] prick: what makes you want to use it over other tooling

[2024-10-22 16:15] 0x208D9: [replying to prick: "what makes you want to use it over other tooling"]
other failed decompilers that used LLVM IR

[2024-10-22 16:16] 0x208D9: plus vex ir seems easier to lift than llvm ir

[2024-10-22 16:16] 0x208D9: by looking at pyvex

[2024-10-22 16:16] prick: good luck trailblazer

[2024-10-22 16:16] prick: lol

[2024-10-22 16:31] 0x208D9: [replying to prick: "good luck trailblazer"]
i mean angr has a decompiler and it uses pyvex with ailment

[2024-10-22 16:31] 0x208D9: so yeah

[2024-10-22 16:31] 0x208D9: its the closest to hexrays decompiler output as far as i can see

[2024-10-22 16:31] 0x208D9: check out angr-management

[2024-10-22 16:31] prick: i meant to use angr for something

[2024-10-22 16:32] prick: the entire field of static analysis is cursed and ive had no real use of doing it until recently

[2024-10-22 16:33] 0x208D9: https://github.com/angr/angr-management
[Embed: GitHub - angr/angr-management: The official angr GUI.]
The official angr GUI. Contribute to angr/angr-management development by creating an account on GitHub.

[2024-10-22 16:34] prick: brother is up to a v9

[2024-10-22 16:34] prick: of a ui

[2024-10-22 16:36] 0x208D9: not me , credit goes to mahaloz

[2024-10-22 18:28] Torph: [replying to 0x208D9: "https://github.com/angr/angr-management"]
top tier project name

[2024-10-23 09:03] Timmy: 💯

[2024-10-23 10:07] luci4: [replying to rin: "this is the one I originally learned on but its no..."]
Why not? Sorry for bringing up the dead, but I only got time to look into it now 😦

[2024-10-23 12:02] idkhidden: pass i want 100k

[2024-10-23 12:52] 0x208D9: [replying to idkhidden: "pass i want 100k"]
what was in there?

[2024-10-23 12:56] idkhidden: Some scam

[2024-10-23 12:57] 0x208D9: [replying to idkhidden: "Some scam"]
archive?

[2024-10-23 15:53] Windows2000Warrior: just a question, please  is it possible to create or update a tool , specifically designed for analyzing driver files (.sys). The tool should have the following features:

Function Detection: Automatically identify and list all functions in .sys or other binary files, providing details about their signatures.

Parameter and Struct Validation: Check if the driver function parameters match system-defined structures, validating both the number of parameters and types against the system structs (e.g., for compatibility with Windows NT4, Windows 2000, XP..).

Compatibility Reports: Generate reports about function and parameter compatibility with target operating systems. 

Interactive UI: Allow users to click on specific function calls and view detailed information, including:

Parameter types
Struct compatibility
Mismatches in types, fields, or number of parameters
Version Comparison: Enable comparisons of driver compatibility across different Windows versions (e.g., NT4 vs. 2000/XP ...), helping developers ensure that drivers work consistently in different environments. <@148095953742725120> <@503274729894051901> is this tool https://github.com/wbenny/pdbex can perform this work?
[Embed: GitHub - wbenny/pdbex: pdbex is a utility for reconstructing struct...]
pdbex is a utility for reconstructing structures and unions from the PDB into compilable C headers - wbenny/pdbex

[2024-10-23 15:54] Brit: <:mmmm:904523247205351454>

[2024-10-23 18:41] dinero: i can see exactly where you pressed enter in the query/prompt

[2024-10-23 18:41] dinero: That’s crazy

[2024-10-23 18:42] dinero: actually

[2024-10-23 18:42] dinero: can you ask this again without using chatgpt

[2024-10-23 18:50] Deleted User: [replying to Windows2000Warrior: "just a question, please  is it possible to create ..."]
least obvious gpt

[2024-10-23 20:05] Windows2000Warrior: [replying to dinero: "can you ask this again without using chatgpt"]
gpt because my english is very bad

[2024-10-23 20:08] Windows2000Warrior: [replying to dinero: "can you ask this again without using chatgpt"]
is it possible to create program that check the structs compatibility between driver and system? there are some structs updated from version to version of windows , leads to driver crash , even in some case maybe there are a diffrence in number of parameters between calls and defined system functions

[2024-10-23 21:58] dinero: symbols..?

[2024-10-23 21:58] dinero: oh nvm

[2024-10-23 22:00] dinero: i mean

[2024-10-23 22:00] dinero: why do you need a program for this

[2024-10-23 22:00] dinero: Frankly for your p2c driver just ifdef it, that  shit

[2024-10-23 22:03] dinero: auto update i guess is not solved by that

[2024-10-23 22:05] dinero: Or do you mean a literal tool like winbindiff

[2024-10-23 22:06] Brit: pdb parsing too hard

[2024-10-23 22:08] dinero: https://github.com/ergrelet/windiff
[Embed: GitHub - ergrelet/windiff: Web-based tool that allows comparing sym...]
Web-based tool that allows comparing symbol, type and syscall information of Microsoft Windows binaries across different versions of the OS. - ergrelet/windiff

[2024-10-23 22:08] dinero: nah fuck it man

[2024-10-23 22:09] dinero: [replying to Brit: "pdb parsing too hard"]
idk man if he could use symbols why hasn’t he used symbols

[2024-10-23 22:13] Windows2000Warrior: [replying to dinero: "https://github.com/ergrelet/windiff"]
Oh yes I didn't know about winbindiff , it looks like almost what I need (But the concern is that if some Windows files are modified, there may be an incompatibility with the symbols) .I'll give it a try.

[2024-10-24 03:14] Matti: [replying to Windows2000Warrior: "just a question, please  is it possible to create ..."]
repost this in <#902892977284841503> where it belongs, because no one sane would do this for you for free
mention what you'll be paying in the same post to save other people time as well please

and never @ me again

[2024-10-24 04:50] Torph: [replying to Windows2000Warrior: "Oh yes I didn't know about winbindiff , it looks l..."]
if some unknown files are modified on some installs aren't all bets kinda off

[2024-10-24 12:33] Windows2000Warrior: sorry for that, yesterday i notice that your name you and diversenok are in github pdbex project  ,that's why i mention you , and for the program i just ask for the possibility means i want to know if this idea can be applied in the real or not , i don't want for now to create it ,maybe in the future..

[2024-10-24 14:07] 0x208D9: bro abandoned idea in one sec damn