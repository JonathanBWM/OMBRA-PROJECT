# July 2025 - Week 2
# Channel: #reverse-engineering
# Messages: 103

[2025-07-10 15:39] AndyM: how to patch in memory an msdos 16-bit exe which is PACKED? If I understand well, TSRCRACK http://www.textfiles.com/hacking/tsrcrack.txt expects an interrupt to be called in the segment where you want to change the bytes, but there is no interrupt called in the segment I need to patch... What to do? 
I used Softice 2.8 to analyze the exe

[2025-07-10 16:06] x86matthew: [replying to AndyM: "how to patch in memory an msdos 16-bit exe which i..."]
if it really doesn't use any interrupts then that tool won't help you
unless you're already sure, try hooking these common ones: int 0x21 (dos system calls), int 0x10 (video), int 0x13 (disk IO), int 0x16 (keyboard), int 0x1A (RTC)

[2025-07-10 16:07] x86matthew: might be easier to just unpack the exe though, most used primitive tools such as pklite or lzexe

[2025-07-10 16:07] AndyM: [replying to x86matthew: "if it really doesn't use any interrupts then that ..."]
the app uses interrupts but not in the same segment where the call which I want to NOP resides. I am not sure whether this matters or not

[2025-07-10 16:08] AndyM: [replying to x86matthew: "might be easier to just unpack the exe though, mos..."]
tried to unpack it but failed

I tried a lot of tools, the best tools, and they almost go through it , they support the algorithm but the exe is quite big so I get a "Program is too big to fit in memory" before the unpacking is finished

[2025-07-10 16:11] x86matthew: [replying to AndyM: "the app uses interrupts but not in the same segmen..."]
yeah it wouldn't get very far without any interrupts at all 🥲  i'm sure an interrupt will occur somewhere while your segment is already decoded/decompressed in memory though

[2025-07-10 16:11] AndyM: yes there are interrupts called after that segment is already decoded

[2025-07-10 16:11] x86matthew: but yeah unpacking would be better

[2025-07-10 16:12] AndyM: [replying to AndyM: "yes there are interrupts called after that segment..."]
so I am thinking how to make that tool know that segment, I think it changes due to how msdos loads things so I can't just hardcode it

[2025-07-10 16:15] AndyM: https://github.com/BenjaminSoelberg/dos-DumpExe
Also tried manual unpacking with this tool

Almost succeeded, using Turbo Debugger as described in the tutorial (after a lot of fiddling to get around "not enough memory to load the exe" at the beginning when I try to load it into TD). But at some point it would crash because of lack of memory (That happened on an MSDOS virtual machine on VirtualBox.)

On that msdos virtual machine for some reason no mouse driver would work, then I figured out that using win 3.1 instead , which is also a full msdos, has no mouse driver problems on VirtualBox. So MAYBE repeating the same process using Turbo Debugger would work. But now I fail even to open the target exe in TD, it says "not enough memory".

So another option is to use Softice instead of Turbo Debugger because TD uses too much memory. But the executables I unpack with Softice  and DumpExe as described in the DumpExe tutorial, are broken... When I unpack the same executable TESTEXE.EXE using Turbo Debugger, the resulting unpacked executable works...
[Embed: GitHub - BenjaminSoelberg/dos-DumpExe: The OBSESSiON EXE-Dumper, ve...]
The OBSESSiON EXE-Dumper, version 2.5. Contribute to BenjaminSoelberg/dos-DumpExe development by creating an account on GitHub.

[2025-07-10 16:22] AndyM: [replying to x86matthew: "but yeah unpacking would be better"]
a tutorial about manual generic unpacking using Softice would help, but I have not been able to find any...

[2025-07-11 18:31] NSA, my beloved<3: Hello! How do you guys move over your work in IDA (renamed variables, functions, structs, changed types, etc...) from one project to another one if the binary you were working on got updated?

[2025-07-11 18:48] guixsm: Does anyone know about the cracking? To help me with a question?

[2025-07-11 18:55] iPower: [replying to guixsm: "Does anyone know about the cracking? To help me wi..."]
<#835634425995853834>

[2025-07-11 19:02] guixsm: [replying to iPower: "<#835634425995853834>"]
ahhh okay, sorry  <"(º_º)">

[2025-07-11 19:02] guixsm: Do you know any place that can talk about it?

[2025-07-11 19:03] brymko: yeah i do crack how can i help

[2025-07-11 19:04] guixsm: [replying to brymko: "yeah i do crack how can i help"]
can I send you a message in pv?

[2025-07-11 19:05] brymko: no

[2025-07-11 19:05] guixsm: <:mmmm:904523247205351454>

[2025-07-11 19:06] pinefin: <:facepalm:555389217513930776>

[2025-07-11 19:52] absceptual: [replying to guixsm: "Does anyone know about the cracking? To help me wi..."]
https://dontasktoask.com
[Embed: Don't ask to ask, just ask]

[2025-07-11 19:53] Brit: in this case dont ask because its against the rules

[2025-07-11 19:54] absceptual: wont lie i dont think ive opened that channel yet

[2025-07-11 19:56] guixsm: [replying to absceptual: "https://dontasktoask.com"]
xd <:delet_this:838736832002261043>

[2025-07-11 19:57] iPower: [replying to absceptual: "wont lie i dont think ive opened that channel yet"]
then you should to avoid having me banning you in a near future

[2025-07-11 19:58] iPower: the usual flowchart is:
someone doesn't read rules -> I tell them to stop -> they ignore -> I ban them

[2025-07-11 19:58] absceptual: maybe

[2025-07-11 19:58] guixsm: [replying to iPower: "then you should to avoid having me banning you in ..."]
People look for groups like this to develop, they usually don't have rules and are made to help each other.

[2025-07-11 19:58] guixsm: and when there is a rule, usually no one reads it.

[2025-07-11 19:58] Brit: tough luck, this community has rules

[2025-07-11 19:58] iPower: you should stop replying

[2025-07-11 19:59] iPower: you're starting to piss me off

[2025-07-11 19:59] guixsm: '-'

[2025-07-11 20:00] iPower: [replying to guixsm: "People look for groups like this to develop, they ..."]
and just to answer this statement: any serious reverse engineering community has rules.

[2025-07-11 20:00] guixsm: [replying to iPower: "and just to answer this statement: any serious rev..."]
yes bro

[2025-07-11 20:00] daax: [replying to guixsm: "People look for groups like this to develop, they ..."]
There is plenty of help going on in this channel. Assisting with cracking an application to get something that is paid for free is 1) illegal and 2) against Discord ToS and could get the channel deleted.

[2025-07-11 20:01] guixsm: [replying to daax: "There is plenty of help going on in this channel. ..."]
for that I asked if I could ask in pv

[2025-07-11 20:01] daax: Continuing to antagonize the moderators will be lead to the problem getting solved rather quickly. You're welcome to participate in accordance with the rules.

[2025-07-11 20:01] iPower: [replying to guixsm: "for that I asked if I could ask in pv"]
don't do that. just go somewhere else.

[2025-07-11 20:03] guixsm: I just asked for help, I didn't understand so much ignorance, you guys are acting like it was Twitter, but ok

[2025-07-11 20:03] daax: [replying to guixsm: "for that I asked if I could ask in pv"]
If you're not willing to ask publicly, the people you're soliciting probably don't want it in their DMs. You can DM whoever you want but don't post in here asking if you can.

[2025-07-11 20:04] daax: [replying to guixsm: "I just asked for help, I didn't understand so much..."]
Nobody is acting like anything. You're being obnoxious and trying to justify it, and openly trying to break the rules here (that are there for a reason, we don't necessarily like all the rules either but we like having a server).

[2025-07-11 20:05] UJ: As discord is trying to go public on the stock market, they are going to (already is) clamp down on servers they consider stock price affecting as well (reddit did the same thing right before they IPOd). Best to read the rules.

[2025-07-11 20:26] pinefin: [replying to guixsm: "People look for groups like this to develop, they ..."]
theres a ton of discords you can go right ahead and ask. like the RE discord, and a lot more. oh wait but theres a catch, they have rules too!

[2025-07-11 20:27] pinefin: oh hes gone

[2025-07-12 03:27] varaa: poor guy

[2025-07-13 17:03] expy: hey guys, is there any particular feature you were always missing in x64dbg/windbg/cheatengine?
I'm building something new and may consider implementing it
[Attachments: output000.mp4]

[2025-07-13 17:06] Deleted User: [replying to expy: "hey guys, is there any particular feature you were..."]
dude that looks really good, what did you use to make the interface?

[2025-07-13 17:07] Lyssa: electron

[2025-07-13 17:09] koyz: [replying to Lyssa: "electron"]
kill it with fire.

[2025-07-13 17:14] the horse: [replying to expy: "hey guys, is there any particular feature you were..."]
looks great!

[2025-07-13 17:15] the horse: most I missed was emulator-based debugging but https://www.zathura.dev/ takes care of it
[Embed: ZathuraDbg: Open-Source GUI Debugger for Assembly | Learn Assembly ...]
ZathuraDbg is an open-source GUI debugger for assembly. The primary goal of ZathuraDbg is to make the process of learning assembly easier with a clean UI.

[2025-07-13 17:16] the horse: and primary address view as a module rva

[2025-07-13 17:20] Deleted User: [replying to the horse: "most I missed was emulator-based debugging but htt..."]
is that with imgui?

[2025-07-13 17:20] the horse: +

[2025-07-13 17:22] expy: [replying to Deleted User: "dude that looks really good, what did you use to m..."]
Tauri

[2025-07-13 17:25] expy: [replying to the horse: "most I missed was emulator-based debugging but htt..."]
OMG, I never heard of it, but was thinking about something similar, when was it released?

[2025-07-13 17:25] the horse: a few months back

[2025-07-13 17:25] the horse: it's open source

[2025-07-13 17:25] the horse: https://github.com/ZathuraDbg/ZathuraDbg
[Embed: GitHub - ZathuraDbg/ZathuraDbg: An emulation based tool for learnin...]
An emulation based tool for learning and debugging assembly. - ZathuraDbg/ZathuraDbg

[2025-07-13 17:25] expy: [replying to the horse: "it's open source"]
Thanks! Will have a good look

[2025-07-13 17:26] the horse: if your preferred aim is emulation-assisted analysis you might also like https://github.com/momo5502/sogen
[Embed: GitHub - momo5502/sogen: 🪅 Windows User Space Emulator]
🪅 Windows User Space Emulator . Contribute to momo5502/sogen development by creating an account on GitHub.

[2025-07-13 17:26] the horse: it's always very nice to have access to see what syscalls a program makes and where no matter if it's inlined or not

[2025-07-13 17:26] the horse: + can detect polymorphic code/self-rewriting code

[2025-07-13 17:27] the horse: point towards integrity checks etc

[2025-07-13 17:27] the horse: very good project by momo

[2025-07-13 17:29] expy: Yeah, that one I saw 😉

[2025-07-13 17:31] expy: I was thinking about running an emulator based on the current process state in the debugger, so the user could basically see the following API/syscalls, transition between modules, etc. and navigate through it with a single click

[2025-07-13 17:46] the horse: proper sandboxing of the application is kind of difficult to do

[2025-07-13 17:46] the horse: especially in a multi-threaded environment

[2025-07-13 17:47] the horse: you need to be careful to prevent vmescapes

[2025-07-13 17:49] the horse: and depending on whether the target program implements anti-emulation methods this might be a bit harder; qemu has issues, icicle has even more issues

[2025-07-13 17:50] the horse: i've been trying to make my own software-level x86_64 emulator but it's an incredible amount of work

[2025-07-13 17:50] the horse: rust rewrite soon hopefully though 💔 ✊
https://github.com/binsnake/KUBERA
[Embed: GitHub - binsnake/KUBERA: A x86 CPU & Environment emulator for Wind...]
A x86 CPU & Environment emulator for Windows user and kernel binaries. - binsnake/KUBERA

[2025-07-13 17:51] Brit: Ill believe this when I see it <:topkek:904522829616263178>

[2025-07-13 17:52] the horse: mb wrong link

[2025-07-13 17:53] the horse: it's a piece of shit though currently 💔 unfortunately the hw results I was testing on were bad

[2025-07-13 17:53] the horse: it does have some basic exception support though just no dispatcher

[2025-07-13 17:54] the horse: 
[Attachments: image.png]

[2025-07-13 17:54] the horse: + 200k instructions a second is terrible

[2025-07-13 17:54] the horse: extremely slow

[2025-07-13 17:55] the horse: it wouldn't survive any protected bin (unless the timing checks are very poor, i just increment tsc by 1 every instruction) or larger packed bins

[2025-07-13 17:56] the horse: however 80-bit fpu seems to be properly emulating compared to icicle (so the mantissa horse doesn't cause issues)

[2025-07-13 18:46] magicbyte: [replying to expy: "hey guys, is there any particular feature you were..."]
the ability to sync it with static analysis tools

[2025-07-13 21:05] daax: [replying to the horse: "rust rewrite soon hopefully though 💔 ✊
https://git..."]
Please no

[2025-07-13 21:06] the horse: yeah i gave it a quick look

[2025-07-13 21:06] the horse: i dont have some libs i need that would be easily usable

[2025-07-13 21:06] the horse: so rewrite will be in C++ 🙏 😔

[2025-07-13 21:07] the horse: and the main lib I want -- iced, I have ported to be usable from cpp with naci

[2025-07-13 21:07] daax: [replying to the horse: "so rewrite will be in C++ 🙏 😔"]
let someone else bare that burden

[2025-07-13 21:07] daax: and them rewrite it in rust

[2025-07-13 21:07] daax: [replying to the horse: "and the main lib I want -- iced, I have ported to ..."]
eh not really

[2025-07-13 21:07] the horse: yeah I don't think I'm ready for a large-scale multi-component project in rust

[2025-07-13 21:08] daax: yours doesn’t use most of the available info

[2025-07-13 21:08] the horse: I only did small things without lib usage

[2025-07-13 21:08] the horse: [replying to daax: "yours doesn’t use most of the available info"]
yeah it's very basic

[2025-07-13 21:08] the horse: i'm missing a couple things I was using from capstone still, working on it atm

[2025-07-13 21:09] the horse: it would probably be a better strategy to edit the iced source itself; I think all the transformations to the C structure are meh

[2025-07-13 21:10] the horse: and it's also not really set up for batch decoding (not that I need it; would hurt my stuff rather than help it as disassembly desynchronization in obfuscated binaries would break it up)

[2025-07-13 22:42] Deleted User: hey, Im a programmer and try to become a game reverse engineer.

[2025-07-13 22:44] daax: [replying to Deleted User: "hey, Im a programmer and try to become a game reve..."]
hello. check out <#835648484035002378>

[2025-07-13 22:45] Deleted User: [replying to daax: "hello. check out <#835648484035002378>"]
okay, thanks for guiding me.