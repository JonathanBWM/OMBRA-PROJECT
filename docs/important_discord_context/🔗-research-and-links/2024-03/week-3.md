# March 2024 - Week 3
# Channel: #🔗-research-and-links
# Messages: 16

[2024-03-14 18:05] [Janna]: wanted to share this 
- https://www.codeproject.com/Articles/5304605/Creating-Shellcode-from-any-Code-Using-Visual-Stud
- https://www.arsouyes.org/blog/2019/54_Shellcode/
- https://reversingbinaries.in/
- https://www.tenouk.com/

(Optional)
- `https://github.com/cirosantilli/x86-assembly-cheat/`
[Embed: Creating Shellcode from any Code Using Visual Studio and C++]
Learn how to convert any code to a stable shellcode using Visual Studio 2019 and VC++ in easy steps!
[Embed: Reverse engineering for fun and profit]
The site is created to clear various certifications in the field of cyber security
[Embed: A practical programming tutorials on C++, C language, Windows and L...]
These tutorials cover a wide range of C and C++ programming for both opensource and commercial. The topics include C and C++ basic to advanced programming, C and C++ secure coding, Windows forms, MFC 

[2024-03-17 13:00] naci: Mergen converts Assembly code into LLVM IR, a process known as lifting. It leverages the LLVM optimization pipeline for code optimization and constructs control flow through pseudo-emulation of instructions. Unlike typical emulation, Mergen can handle unknown values, easing the detection of opaque branches and theoretically enabling exploration of multiple code branches.

These capabilities facilitate the deobfuscation and devirtualization of obfuscated or virtualized functions. Currently in early development, Mergen already shows promise in devirtualizing older versions of VMProtect, with ambitions to support most x86_64 instructions.

https://github.com/NaC-L/Mergen
[Embed: GitHub - NaC-L/Mergen: Deobfuscation via optimization with usage of...]
Deobfuscation via optimization with usage of LLVM IR and parsing assembly. - NaC-L/Mergen

[2024-03-17 14:22] daax: https://github.com/sodareverse/TDE
[Embed: GitHub - sodareverse/TDE: A devirtualization engine for Themida.]
A devirtualization engine for Themida. Contribute to sodareverse/TDE development by creating an account on GitHub.

[2024-03-17 14:23] daax: <@839216728008687666>

[2024-03-17 15:15] mrexodia: [replying to naci: "Mergen converts Assembly code into LLVM IR, a proc..."]
Looks very interesting, do you know how it compares to things like remill?

[2024-03-17 15:27] snowua: [replying to daax: "<@839216728008687666>"]
Haha I was actually looking at this the other day pretty interesting for sure. Can’t wait to see the progress of this project as time goes on. It seems like a lot of the ground work is already there but the more complex VMs aren’t supported just yet

[2024-03-17 15:38] naci: [replying to mrexodia: "Looks very interesting, do you know how it compare..."]
I couldnt compare against remill (I dont know if its me or its too painful to compile and get it to work). I compared it against retdec and the results are Mergen outputs more readable and optimized code, **and** since we can compile the output unlike retdec, it benefits from optimization pipeline more.

[2024-03-17 15:39] mrexodia: [replying to naci: "I couldnt compare against remill (I dont know if i..."]
Yeah it looks nice, will give it a try soon!

[2024-03-17 15:40] mrexodia: Remill is painful to compile, it requires a lot of work on Windows especially…

[2024-03-17 16:32] vendor: I couldnt compare against remill (I dont

[2024-03-17 16:52] daax: [replying to snowua: "Haha I was actually looking at this the other day ..."]
the problem is that when the authors @ oreans see the deobf for higher complexity VMs the architecture will likely change just enough to render the entire thing useless, and then it will become a cat-and-mouse game. imo for projects like these it's probably best to keep them private and do as many of the VMs as you can and then release, but it's probs just for "fun" so doesn't really matter

[2024-03-17 16:53] daax: of course still interesting to see the current methods for the versions listed

[2024-03-17 16:54] daax: and you can diff against future versions and tweak, just from a development perspective once it's spotted and gains traction the devs @ oreans would be on this pretty quickly (i'd assume)

[2024-03-17 16:55] naci: [replying to daax: "the problem is that when the authors @ oreans see ..."]
That's why I created mergen, it doesnt target VM's, it targets the assembly code

[2024-03-17 18:19] snowua: [replying to daax: "the problem is that when the authors @ oreans see ..."]
sad reality of the situation but completely fair 😔

[2024-03-17 19:18] daax: [replying to snowua: "sad reality of the situation but completely fair 😔"]
yeah, wasn't meant to take away from the coolness of the project, its very neat to see, just a rambling anecdote