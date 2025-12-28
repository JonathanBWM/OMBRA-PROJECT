# February 2024 - Week 2
# Channel: #reverse-engineering
# Messages: 75

[2024-02-05 15:51] diversenok: Does anybody know where Windows stores installers/files for all the optional components from this dialog?
[Attachments: image.png]

[2024-02-05 16:44] dullard: Procmon

[2024-02-06 00:32] ehm: off topic from usual os stuff but does anyone know how to monitor a chrome extension? there's a paid chrome extension that auto shares your listings on a website and I'm trying to see what network requests it's making/api calls

[2024-02-06 02:44] emulatingbe: [replying to ehm: "off topic from usual os stuff but does anyone know..."]
Google to chromewebstore (extension store) search for: "Chrome extension source viewer" and look at the code of the extension

[2024-02-07 12:54] qwerty1423: [replying to ehm: "off topic from usual os stuff but does anyone know..."]
on devtools try to filter the data by entering the desired extension's name and refresh the page afterwards

[2024-02-07 12:56] ehm: [replying to qwerty1423: "on devtools try to filter the data by entering the..."]
you can actually just go to extension in dev mode and click inspect something, it’s  web packed/obfuscated and not worth the time

[2024-02-07 12:57] ehm: plus it puts all the 3rd party libraries they use and what not into the same js file so it’s a 3mb js file with ~80k lines

[2024-02-07 18:10] Xed0sS: [replying to Ignotus: "I saw Recaf has scripting but I cant get it to run"]
Recaf best tool for patching

[2024-02-07 18:11] Ignotus: [replying to Xed0sS: "Recaf best tool for patching"]
Yeah i've used it before but cant get the cli to work so I went with asm

[2024-02-07 18:12] Ignotus: it also worked, though it was mainly luck that the method I had to patch was simple so I didnt need to check the instructions too much

[2024-02-07 18:13] Xed0sS: [replying to Ignotus: "Yeah i've used it before but cant get the cli to w..."]
Why u need cli? You can just run jar with all deps and it works

[2024-02-07 18:14] Ignotus: [replying to Xed0sS: "Why u need cli? You can just run jar with all deps..."]
I needed the script feature but cli is basically the same and needed for testing, neither worked for me

[2024-02-07 18:17] Xed0sS: Works
[Attachments: image.png]

[2024-02-07 23:25] bowen: Complete RE newbie but I'm working through some basics!
[Attachments: image.png]

[2024-02-09 11:08] Deleted User: [replying to bowen: "Complete RE newbie but I'm working through some ba..."]
nihao

[2024-02-11 12:32] .mydev: Rebase & ASLR must be `False` in order to use a DLL's `jmp esp` instruction (jmp esp trampoline) - part of Stack overflow shellcode injection?

[2024-02-11 13:41] brymko: jmp esp is the most useless gadget ever unless u have access to a rwx section

[2024-02-11 14:21] Mateoo: hey! can I use debugger to get to get address of function that is called when I click smthing in program?

[2024-02-11 14:53] .mydev: [replying to brymko: "jmp esp is the most useless gadget ever unless u h..."]
Yeah, but I want to understand stuff conceptually...

I ask it because it seems ASLR&REBASE have overlapping effects.

If ASLR is True, as I see - in particular, it's still rebased.

But, if it's only Rebased, and ASLR not enabled, do we care about it at all?  Our opcode address (JMP esp) is relative to the DLL's base pointer, no? So isn't it irrlevenat if it's rebased for our exploitation?

Also, what does it mean by Immunity Debugger, that a DLL is a OS DLL?

[2024-02-11 17:14] unknowntrojan: [replying to .mydev: "Yeah, but I want to understand stuff conceptually...."]
unless immunity debugger has some very weird nomenclature a OS DLL should refer to a DLL provided by windows, probably determined by the microsoft signature

[2024-02-11 17:14] unknowntrojan: [replying to Mateoo: "hey! can I use debugger to get to get address of f..."]
yes

[2024-02-11 17:23] .mydev: [replying to unknowntrojan: "unless immunity debugger has some very weird nomen..."]
What's the effective security implication of this detail?

[2024-02-11 17:29] jvoisin: Why use immunity debugger in 2024 instead of x64dbg ?

[2024-02-11 17:29] unknowntrojan: [replying to jvoisin: "Why use immunity debugger in 2024 instead of x64db..."]
that wouldve been my next thing to say

[2024-02-11 17:31] unknowntrojan: [replying to .mydev: "What's the effective security implication of this ..."]
have to jump through hoops to modify on disk
other than that any loaded DLL is CoW in your own address space so no problem if you're patching it

[2024-02-11 18:01] Mateoo: [replying to unknowntrojan: "yes"]
how can I do it? I dont understand it.

[2024-02-11 18:01] Mateoo: there are like 10 threads

[2024-02-11 18:02] unknowntrojan: research what happens when you click on a window and go from there

[2024-02-11 18:30] mrexodia: [replying to jvoisin: "Why use immunity debugger in 2024 instead of x64db..."]
OSCP still uses it

[2024-02-11 18:30] mrexodia: and I don't know who to contact to get it updated

[2024-02-11 18:30] mrexodia: (or what features they are actually using so I can match them)

[2024-02-11 18:32] jvoisin: Hey, I didn't ~~harass~~ nudge you to add mona.py support in x64dbg for people to ignore this and use unmaintained stuff :/

[2024-02-11 18:37] dullard: [replying to mrexodia: "OSCP still uses it"]
afaik it doesn't since the x86 bof was removed 🤔

[2024-02-11 18:39] mrexodia: [replying to jvoisin: "Hey, I didn't ~~harass~~ nudge you to add mona.py ..."]
https://github.com/Andy53/ERC.net supposedly this is a good replacement
[Embed: GitHub - Andy53/ERC.net: A collection of tools for debugging Window...]
A collection of tools for debugging Windows application crashes. - GitHub - Andy53/ERC.net: A collection of tools for debugging Windows application crashes.

[2024-02-11 20:14] .mydev: [replying to unknowntrojan: "have to jump through hoops to modify on disk
other..."]
Can you elaborate a bit?
To modify what and why on disk? The DLL's code? Isn't it loaded in memory (on-write at least)?

[2024-02-11 20:30] unknowntrojan: [replying to .mydev: "Can you elaborate a bit?
To modify what and why on..."]
DLLs are shared libraries, as in shared between applications. this was necessary back in the day to save on memory, when a process loads a DLL it is mapped to every other process that loads said DLL using copy-on-write, so that any modifications are always local to the process they were modified on without affecting other processes

[2024-02-11 20:31] unknowntrojan: [replying to .mydev: "Can you elaborate a bit?
To modify what and why on..."]
when DLLs are signed with a digital signature you'd have to do some shenanigans with windows to make them load anyway. if you want to patch a dll for every process you'd want to do it on-disk, which signatures are there to prevent, you can still modify them inside your own address space but that will not be written back to disk

[2024-02-11 20:33] unknowntrojan: [replying to unknowntrojan: "research what happens when you click on a window a..."]
hint: WndProc most likely

[2024-02-11 20:35] .mydev: [replying to unknowntrojan: "DLLs are shared libraries, as in shared between ap..."]
Ah got it I think.
So once a DLL is required by any program, it's loaded in some shared space in memory for all subsequent programs that will come later and use it too?

[2024-02-11 20:35] unknowntrojan: yes

[2024-02-11 20:36] unknowntrojan: windows loads it once and every process loading it subsequently will get a copy on write mapping of it in their address space

[2024-02-11 20:36] .mydev: Interesting... Where in memory these DLLs are loaded such that all programs can access it?

[2024-02-11 20:36] unknowntrojan: thats why ASLR is cheap by the way, it just gets mapped somewhere else for every process

[2024-02-11 20:37] unknowntrojan: [replying to .mydev: "Interesting... Where in memory these DLLs are load..."]
no userspace program has RW access to the original, that is reserved to the OS. the OS simply maps the DLL's pages into process address spaces with the CoW flag

[2024-02-11 20:38] .mydev: [replying to unknowntrojan: "thats why ASLR is cheap by the way, it just gets m..."]
Yeah, it does allocate on memory only on-demand, so it's cheap

[2024-02-11 20:38] unknowntrojan: this mapping is not physical, everytime the CPU accesses virtual memory it goes through a page table lookup. any addresses you have in a program arent "real" physical addresses in RAM, they are a mapping handled by the operating system that is done everytime the CPU accesses something

[2024-02-11 20:39] unknowntrojan: [replying to .mydev: "Yeah, it does allocate on memory only on-demand, s..."]
nono, that wasn't my point, it is loaded once and *mapped* into other processes, *not* copied

[2024-02-11 20:41] .mydev: [replying to unknowntrojan: "nono, that wasn't my point, it is loaded once and ..."]
Okay, so there is one-to-many mapping of original DLL's pages to each corresponding process's pages

[2024-02-11 20:41] unknowntrojan: basically yes

[2024-02-11 20:42] .mydev: And copy ocurrs if and only if the process tries changing one of the DLL's contents

[2024-02-11 20:42] unknowntrojan: https://empyreal96.github.io/nt-info-depot/Windows-Internals-PDFs/Windows%20System%20Internals%207e%20Part%201.pdf

[2024-02-11 20:42] unknowntrojan: page 39

[2024-02-11 20:42] unknowntrojan: [replying to .mydev: "And copy ocurrs if and only if the process tries c..."]
yes

[2024-02-11 20:42] unknowntrojan: copy on write is a really cool concept actually, its even used in file systems a lot nowadays

[2024-02-11 20:42] .mydev: Yeah I know

[2024-02-11 20:43] .mydev: I think it was an issue also with my RDMA project

[2024-02-11 20:43] .mydev: cool stuff

[2024-02-11 20:44] .mydev: [replying to unknowntrojan: "yes"]
Are you engaged in Vulnerability Research?

[2024-02-11 20:44] .mydev: Or just reversing?..

[2024-02-11 20:45] unknowntrojan: [replying to .mydev: "Are you engaged in Vulnerability Research?"]
not professionally

[2024-02-11 20:46] unknowntrojan: mostly for fun

[2024-02-11 20:46] .mydev: [replying to unknowntrojan: "not professionally"]
Gotcha. Very hard to find an organized strategy for learning it...

[2024-02-11 20:47] unknowntrojan: like programming I learned reversing and vuln research using the "fuck around and find out" strategy, I recommend finding something you are motivated to do and just doing it, doesn't have to serve an ulterior purpose, learning is a goal in itself

[2024-02-11 20:47] .mydev: I'm currently building up a learning plan with:
OST2
PwnCollege
ZDE (Universal course)

[2024-02-11 20:48] unknowntrojan: I'm not a fan of courses but I have never done any so I can't judge their effectiveness as a learning tool

[2024-02-11 20:48] .mydev: [replying to unknowntrojan: "like programming I learned reversing and vuln rese..."]
Yea... But this kind of learning can take very long.. I'm a fresh CS graduate haha... I'm too sick being unemployed

[2024-02-11 20:48] unknowntrojan: lots of people got started in gamehacking, it has a very short work-reward loop and can be fun and addicting while having you learn

[2024-02-11 20:50] .mydev: [replying to unknowntrojan: "lots of people got started in gamehacking, it has ..."]
My gaming addiction was stopped like 10 years ago when Maple Story was already underestimated lol

[2024-02-11 20:51] unknowntrojan: morality is a different subject entirely, it's on you to decide whether you want to abuse it.
personally, I try to avoid affecting other players in a competitive environment and focus on having fun with friends in coop games or adding QoL fixes to games that annoy me

[2024-02-11 20:51] unknowntrojan: [replying to .mydev: "My gaming addiction was stopped like 10 years ago ..."]
never really played the game but the artstyle and soundtrack slaps

[2024-02-11 20:51] .mydev: [replying to unknowntrojan: "morality is a different subject entirely, it's on ..."]
ofc agree

[2024-02-11 20:52] .mydev: [replying to unknowntrojan: "never really played the game but the artstyle and ..."]
Haha yeah... there were times..

[2024-02-11 20:53] unknowntrojan: I wonder if gamehacking will remain a good entry to stuff like that with anticheats becoming more effective at raising the barrier to entry

[2024-02-11 20:53] qwerty1423: read writeups of the previous vulns found from your target

[2024-02-11 20:53] unknowntrojan: they refuse to fix the underlying issues people exploited and just raise the barrier of entry to the point new people just cannot get in