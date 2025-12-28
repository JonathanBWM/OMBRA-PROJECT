# November 2024 - Week 2
# Channel: #reverse-engineering
# Messages: 36

[2024-11-04 16:29] iPower: [replying to cmpzx: "Hello guys, how can i unpack themida c++ app?"]
try this: https://github.com/ergrelet/unlicense
[Embed: GitHub - ergrelet/unlicense: Dynamic unpacker and import fixer for ...]
Dynamic unpacker and import fixer for Themida/WinLicense 2.x and 3.x. - ergrelet/unlicense

[2024-11-04 16:29] iPower: usually works fine but it won't produce a runnable dump

[2024-11-04 16:30] iPower: and if it virtualizes OEP it might not be able to get it

[2024-11-04 16:31] iPower: <@1163917765967613952> <@175375853595787264> <@1292860544356712490> if you don't have anything useful to answer a question please just don't post.

[2024-11-04 16:32] iPower: this is my first and only warning

[2024-11-04 18:49] BWA RBX: [replying to iPower: "try this: https://github.com/ergrelet/unlicense"]
I always thought tinkering with Themida would result in a lawsuit but idkw lmfao

[2024-11-04 23:56] 0x208D9: https://lise.pnfsoftware.com/winpdb/

dunno if contains undocumented symbols never looked, but might be useful
[Embed: Index of Microsoft Windows PDB Files]

[2024-11-05 02:29] vykt: I wrote a pointer scanner for Linux. Recently tested it on huge processes ( :p ) and it works very well. https://github.com/vykt/ptrscan
[Embed: GitHub - vykt/ptrscan: Pointer chain scanner for Linux.]
Pointer chain scanner for Linux. Contribute to vykt/ptrscan development by creating an account on GitHub.

[2024-11-05 02:29] vykt: Spoke to the devs of PINCE, will be integrating it into it soon

[2024-11-05 09:20] mrexodia: [replying to vykt: "Spoke to the devs of PINCE, will be integrating it..."]
Awesome!

[2024-11-05 19:41] Torph: [replying to ByteWhite1x1: "As per as of MS policies: A user-mode driver shoul..."]
huh? there is no reason a kernel driver *wouldn't* have access to all of Firefox's memory, that's kinda the point

[2024-11-05 19:43] Torph: [replying to ByteWhite1x1: "URLs I visited in FF browser... How it's possible ..."]
i would be more surprised if the kernel *couldn't* see those URLs

[2024-11-05 23:09] 0x208D9: [replying to vykt: "Spoke to the devs of PINCE, will be integrating it..."]
doesnt pince already support that?

[2024-11-06 08:13] Deleted User: Has anyone experience using LIEF to rebuild PE files? I'm just reading through docs atm and it doesn't appear possible to extend an existing section and rebuild the PE with fixes.. or is it?

[2024-11-06 09:03] vykt: [replying to 0x208D9: "doesnt pince already support that?"]
Their current version doesn't really work properly, and it doesn't work with proton at all

[2024-11-06 11:32] roddux: [replying to vykt: "I wrote a pointer scanner for Linux. Recently test..."]
what's the use case for a tool like this?

[2024-11-06 11:33] roddux: from what I can deduce, it's primarily designed to help reconstruct data structures for binaries (?)

[2024-11-06 12:10] vykt: [replying to roddux: "from what I can deduce, it's primarily designed to..."]
Yeah its really nice for identifying structures. If you use something like scanmem and find 2 seemingly unrelated values in a program, then ptrscan for both, you can see where in the pointer chain they start to deviate

[2024-11-06 12:10] vykt: Also if you want to navigate the memory of a process from an external process it makes it super easy

[2024-11-06 19:29] BWA RBX: [replying to vykt: "I wrote a pointer scanner for Linux. Recently test..."]
Really great work dude 🙂

[2024-11-06 21:10] elias: Hello, can someone recommend scientific papers regarding VMprotect?

[2024-11-06 21:46] BWA RBX: [replying to elias: "Hello, can someone recommend scientific papers reg..."]
I usually search here https://scholar.google.com/ but some of the best articles I've seen about VMProtect are 

https://www.msreverseengineering.com/blog/2014/6/23/vmprotect-part-0-basics
https://secret.club/2021/09/08/vmprotect-llvm-lifting-1.html
https://www.usenix.org/legacy/event/woot09/tech/full_papers/rolles.pdf
[Embed: VMProtect, Part 0:  Basics — Möbius Strip Reverse Engineering]
Originally published on August 6th, 2008 on OpenRCE   This is part #0 of a four-part series on VMProtect. The other parts can be found here:    Part 0: Basics    Part 1: Bytecode and IR    Part 2: Pri
[Embed: Tickling VMProtect with LLVM: Part 1]
This series of posts delves into a collection of experiments I did in the past while playing around with LLVM and VMProtect. I recently decided to dust off the code, organize it a bit better and attem

[2024-11-06 21:48] BWA RBX: Although if you are looking for scientific articles that theorized the code transformations VMProtect implemented I'm sure you can find on Google Scholar

[2024-11-06 21:58] elias: thank you very much

[2024-11-06 21:58] elias: I already looked on google scholar but didnt find anything yet… gonna look again

[2024-11-06 22:19] BWA RBX: So I would say that you should look at the code transformations VMProtect use instead of VMProtect itself, then when you understand the obfuscations through reading the scientific papers I'd go with reading generalized articles of reverse engineering VMProtect.

I think it's also worth mentioning, some stuff you'll see in VMProtect binaries might not make sense unless you have background knowledge of some of the techniques VM-based virtualizers use, so getting to know those and how they work is also important in my opinion.

[2024-11-06 22:24] BWA RBX: The best way to look at those are to look at old VM-Based virtualizers, some questions I had was when I seen the `push 0` in a VMProtect sample from version 1.53, along with pushing all registers and EFLAGS onto the stack, whilst reading Rolf's article it just says "imagebase fix" and stuff like that are generally techniques they've seen and already analyzed and know, but with people like us coming at something complex we might not have known that and might get confused with these techniques that these virtualizers use

[2024-11-06 22:50] vykt: [replying to BWA RBX: "Really great work dude 🙂"]
Thank you :)

[2024-11-07 15:01] Mitch: Had a thought process of something to make as a personal project, and I actually do not know if its been done before or why it hasn't, at least, I have not come across it, so I wanted to ask if my goal has a technical limitation of some sort


One thing I think that would be nice for reverse engineering would be a .dll or .exe "differ", in the sense it can make a database of functions, hash them in some form, and then find the same functions and attempt to hash those, and basically have the .dll list you what has and hasnt changed, and if something has changed, possibly list out what has (for example, maybe one thing just added a cmp check, so I can say "this function from the original DLL has been found in the new dll, but it has 1 new assembly line", etc)

I do realize the pitfalls of this, as well as the issues that might occur, (There are many), but I do think it would be a fun project to work on and publish. 

Does anyone know of tools like this, or why this doesnt seem to be a tool that is popular in the first place?

[2024-11-07 15:01] Mitch: Thanks in advance for any commentary or criticism!

-# Its also possible this is in existing feature in IDA or some other common tool and I just never saw it in my life. Sorry if so for my ignorance!

[2024-11-07 15:30] tifkin: Bindiff or Ghidra's Version Tracking

[2024-11-07 15:40] tifkin: BSim and flirt/FunctionIDatabases are also tangentially related depending on what you're doing

For binary comparison, bindiff is the standard. Relyze isn't widely used but it's actually the easiest out-of-the-box bindiff'er I've seen. 
I've really enjoyed ghidriff  when I've used it as well (uses Ghidra's version tracking and other function matching techniques)

[2024-11-07 15:59] 冰: [replying to Mitch: "Had a thought process of something to make as a pe..."]
https://theses.hal.science/tel-03667920/
[Embed: Binary Diffing as a Network Alignment Problem]
In this thesis, we address the problem of binary diffing, i.e. the problem of finding the best possible one-to-one correspondence between the functions of two programs in binary form. This problem is 

[2024-11-07 16:11] Mitch: Thanks! I appreciate the information, didnt realize these existed, and I find it super interesting and neat!

[2024-11-07 17:07] 0x208D9: [replying to Mitch: "Had a thought process of something to make as a pe..."]
diaphora

[2024-11-07 19:26] tifkin: [replying to 冰: "https://theses.hal.science/tel-03667920/"]
Have you had much luck with qbindiff? Investigated it the past summer but had reliability issues with it on the targets I was testing (phantom crashes, but maybe it's been addressed now)