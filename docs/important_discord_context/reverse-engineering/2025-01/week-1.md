# January 2025 - Week 1
# Channel: #reverse-engineering
# Messages: 40

[2025-01-01 10:36] MonTy: Guys, can someone tell me if there is ntapi for VMware detect?

[2025-01-01 10:42] MonTy: read registry or file?

[2025-01-01 10:45] .: https://github.com/kernelwernel/VMAware/blob/main/src/vmaware.hpp and search "vmware"

[2025-01-01 10:50] MonTy: [replying to .: "https://github.com/kernelwernel/VMAware/blob/main/..."]
Great

[2025-01-03 02:29] expy: hey guys, what do you think about me littering on the internet?

https://www.youtube.com/watch?v=1DGFt-M6Pj4
[Embed: Unlock the Secrets of Binary Code Obfuscation in Minutes!]
Ever wondered how binary code obfuscation works without diving into dense technical articles? Here’s an exciting hands-on guide to using an online LLVM obfuscator that lets you obfuscate your C/C++ co

[2025-01-03 02:47] varaa: [replying to expy: "hey guys, what do you think about me littering on ..."]
this is produced like something id watch in school

[2025-01-03 02:57] Nicola: Hello, I have one puzzle but it is hard to solve for me. It is kind of finding rule in some chaos.
Who reverse Engineer can help me?
-------------------
P9847011A9 
P8757892B0 
P5693138A4

[2025-01-03 11:10] nu11sec: anyone used python Triton before . am getting this issue I have compiled everything correctly and dowloaded the compiled version from the git repository any help thanks
[Attachments: image.png]

[2025-01-03 13:26] samflow: Never used it before, but just did a `pip install triton-library` and can access/create a `BasicBlock` with no issues. Sure you don't have the other triton package installed as well or something? There's a `triton` python package to do with neural networks, so if you have any packages to do with ML/AI already installed, that might be installed and messing things up 🤷‍♂️  they both use `import triton` or `from triton import *` etc.

[2025-01-04 02:13] p1ink0: [replying to expy: "hey guys, what do you think about me littering on ..."]
That’s dope

[2025-01-04 12:13] Brit: why are you assuming there's a driver involved?

[2025-01-04 12:26] sync: [replying to Brit: "why are you assuming there's a driver involved?"]
cause game hacking related

[2025-01-04 12:38] Brit: that still doesn't explain it

[2025-01-04 13:14] sync: ye ur right

[2025-01-04 15:28] avx: [replying to Brit: "that still doesn't explain it"]
the gnomes in my cpu told me

[2025-01-05 03:22] SYAZ: yes

[2025-01-05 10:50] Nobody: Hey guys and happy new year to everyone!
Does anyone know any open source projects (or just those that can be integrated into my own project) where Flow-Oriented Disassembly is fully implemented (preferably on Zydis) with bypasses of standard anti-disassembly techniques?

[2025-01-05 12:52] mrexodia: [replying to Nobody: "Hey guys and happy new year to everyone!
Does anyo..."]
Of course!

[2025-01-05 12:52] Nobody: [replying to mrexodia: "Of course!"]
can u provide a link on github please?

[2025-01-05 12:53] mrexodia: [replying to Nobody: "can u provide a link on github please?"]
It's not real

[2025-01-05 12:53] mrexodia: There are a few GPL projects, but I'm guessing you cannot GPL your application?

[2025-01-05 12:54] Nobody: [replying to mrexodia: "There are a few GPL projects, but I'm guessing you..."]
I can

[2025-01-05 12:55] mrexodia: Actually I think rizin isn't even GPL!

[2025-01-05 12:56] mrexodia: What kind of anti-disassembly techniques do you mean though?

[2025-01-05 12:59] Nobody: [replying to mrexodia: "What kind of anti-disassembly techniques do you me..."]
in general this with different patterns on rip pushing
https://unprotect.it/technique/impossible-disassembly/

also basic block unalignment like
```
sub dword ptr ds:[rax-73],ecx
and al,F8
mov qword ptr ss:[rsp],rbx
mov rbx,FFFF
and rdx,rbx
mov rbx,qword ptr ss:[rsp]
mov qword ptr ss:[rsp],r12
push r12
push rax
lea rax,qword ptr ds:[1522DE279]
not r12
jmp rax
```
Correct
```
nop 
lea rsp,qword ptr ss:[rsp-8]
mov qword ptr ss:[rsp],rbx
mov rbx,FFFF
and rdx,rbx
mov rbx,qword ptr ss:[rsp]
mov qword ptr ss:[rsp],r12
push r12
push rax
lea rax,qword ptr ds:[1522DE279]
not r12
jmp rax
```

[2025-01-05 13:00] mrexodia: 🤔

[2025-01-05 13:00] mrexodia: Isn't that trivial if you just discover basic blocks from the start of the function though?

[2025-01-05 13:01] mrexodia: Although obviously the `jmp rax` is going to be a challenge

[2025-01-05 13:03] Nobody: [replying to mrexodia: "Although obviously the `jmp rax` is going to be a ..."]
it's just obuscation. Correct will be
```
jmp 1522DE279
```
> Isn't that trivial if you just discover basic blocks from the start of the function though?
static way needed. All basic block under obfuscation based on conditional jumps and unconditional

[2025-01-05 13:03] mrexodia: Discovery can be static?

[2025-01-05 13:04] mrexodia: But there is nothing out there that will automatically deobfuscate disassembly for you, you're probably better off writing something yourself

[2025-01-05 13:05] Nobody: [replying to mrexodia: "But there is nothing out there that will automatic..."]
Yeah, I think so, too. I was just looking for something off-the-shelf  🙃

[2025-01-05 13:06] mrexodia: It doesn't exist, nobody would give that away for free. Especially as an out of the box library

[2025-01-05 13:07] Nobody: [replying to mrexodia: "It doesn't exist, nobody would give that away for ..."]
Roger that

[2025-01-05 14:59] expy: Hey <@419928555430871041> are you just learning or have some particular sample to solve? If the latter can you share any examples of that sort?

[2025-01-05 15:02] Nobody: [replying to expy: "Hey <@419928555430871041> are you just learning or..."]
Do you want me to send you the task I'm researching?

[2025-01-05 15:02] Glatcher: Can't you build your project on top of ida?)

[2025-01-05 15:03] Nobody: [replying to Glatcher: "Can't you build your project on top of ida?)"]
Ida takes a very long time to analyze my file because of the large size. And binary ninja has a lot of bugs in ir

[2025-01-05 15:14] Brit: <@419928555430871041> if this is a specific obfuscator you're targeting your best bet is to implement your algo so it specifically deals with those obfuscation passes

[2025-01-05 15:27] expy: [replying to Nobody: "Do you want me to send you the task I'm researchin..."]
Well, yes, if it can be shared/isolated ofc