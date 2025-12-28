# July 2024 - Week 3
# Channel: #reverse-engineering
# Messages: 25

[2024-07-15 10:45] p1ink0: [replying to sariaki: "it unironically goes harder every time i play it"]
True lmao

[2024-07-15 10:45] p1ink0: [replying to unknowntrojan: "HR is gonna have a stroke"]
<:topkek:904522829616263178>

[2024-07-16 04:15] jordan9001: Nyxstone seems helpful. Thanks <@693519715703849061> 
https://www.emproof.com/introducing-nyxstone-an-llvm-based-disassembly-framework/
[Embed: Introducing Nyxstone: An LLVM-based (Dis)assembly Framework - Emproof]
At Emproof, our mission is to enhance the security and integrity of embedded systems through innovative binary rewriting techniques. We are committed to providing advanced […]

[2024-07-16 11:05] jvoisin: [replying to jordan9001: "Nyxstone seems helpful. Thanks <@69351971570384906..."]
based on llvm15 :/

[2024-07-16 11:06] jvoisin: https://github.com/capstone-engine/capstone/issues/2015 <3
[Embed: `auto-sync` progress tracker: Refactor and implement architectures ...]
Note to x86: x86 is not part of this list, because we can not generate all tables in C. Refer to capstone-engine/llvm-capstone#13 for details. Note about changes introduced with auto-sync: For a pr...

[2024-07-16 17:53] mrexodia: [replying to jvoisin: "based on llvm15 :/"]
I don't think there is anything fundamentally blocking an upgrade though

[2024-07-18 06:33] Horsie: I found this on the angr chat

[2024-07-18 06:33] Horsie: https://dogbolt.org/?id=c23b86d0-2b16-42f2-8d80-fdac58bce258
[Embed: Decompiler Explorer]
Decompiler Explorer is an interactive online decompiler which shows equivalent C-like output of decompiled programs from many popular decompilers.

[2024-07-18 06:34] Horsie: Angr's decompiler is pretty good (for human comprehension) compared to all other decomps

[2024-07-18 06:34] Horsie: (For this contrived example)

[2024-07-18 12:58] 0x208D9: [replying to Horsie: "Angr's decompiler is pretty good (for human compre..."]
apart from the fact that its pretty slow, crashes whenever it encounters a binary more than 1MB, and the analysis time is higher than all of the decompiler's avg time combined and the themeing is awful, yes

[2024-07-18 12:58] 0x208D9: also the structs are pretty weirdly formed as well

[2024-07-18 12:59] 0x208D9: and it was only for that specific usecase

[2024-07-18 12:59] 0x208D9: it is also quite bad at parsing symbols from PDB's

[2024-07-18 13:46] ballad of a lone soldier: would i be right to assume this has been done like this
```armasm
ORR W5, W17, W16
AND W16, W17, W16
SUB W16, W5, W16 
``` instead of ```armasm
EOR W16, W16, W17
``` because of obfuscation?

[2024-07-18 13:56] Brit: or just built without optimizations

[2024-07-18 13:56] Brit: but yeah (x | y ) - ( x & y) is an xor

[2024-07-19 22:00] elias: anyone tried rev.ng decompiler? How does it compare to IDA?

[2024-07-20 07:36] jvoisin: [replying to elias: "anyone tried rev.ng decompiler? How does it compar..."]
Isn't it on dogbolt?

[2024-07-20 14:21] repnezz: When is the dirty pte bit of a file backed page pushed out to the pfn (so the modified bit is set) ? 
obviously when the page is unmapped , but what are the other cases ?

[2024-07-20 15:09] daax: [replying to repnezz: "When is the dirty pte bit of a file backed page pu..."]
when the page is different from its copy in paging file or when no copy of it exists in paging file (such as a new page); `MiInitializePfn` if you're looking for where this happens

[2024-07-20 15:11] daax: i.e. if the page can be reused without bothering to store contents, such as an identical copy of it exists in file, then its clean. otherwise, set dirty bit so that the vmm knows the page content hasnt been saved yet

[2024-07-20 15:14] daax: to the other question: release/first probe/as soon as it's mapped (no copy exists in paging file)

[2024-07-20 15:14] daax: which makes sense because the modified bit would also be set which tells the vmm hey if you shrink the working set put this page in the modified list so it gets saved in paging file

[2024-07-20 15:43] repnezz: Thanks !