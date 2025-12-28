# March 2025 - Week 5
# Channel: #reverse-engineering
# Messages: 248

[2025-03-24 09:04] nu11sec: I  want to keep track of all mem access to ntdll and kernel32 what tool or library do you go for ?

[2025-03-24 12:10] laughtersec: [replying to nu11sec: "I  want to keep track of all mem access to ntdll a..."]
https://www.twingate.com/blog/glossary/function%20hooking

This might help
[Embed: What Is Function Hooking? How It Works & Examples | Twingate]
Discover the essentials of function hooking, from how it works and real-world examples to its risks and protective measures against unauthorized use.

[2025-03-24 12:12] laughtersec: Here's an example 
https://cocomelonc.github.io/tutorial/2021/11/30/basic-hooking-1.html
[Embed: Windows API hooking. Simple C++ example.]
﷽

[2025-03-24 14:12] pinefin: [replying to nu11sec: "I  want to keep track of all mem access to ntdll a..."]
if you set PAGE_GUARD on all pages then you should be able to handle them in an exception handler

[2025-03-24 14:13] pinefin: and then also if you want to go the hypervisor route, EPT hooking would be your friend

[2025-03-24 14:14] pinefin: hyperplatform is good if you just want to set that up quickly. may be a little overkill for what you need though.

[2025-03-24 14:15] pinefin: other than that, i'd say hardware breakpoints, but you can't set a region that you want to listen for, so... thats the downside

[2025-03-24 14:16] pinefin: you would just do 
```VirtualProtect(region, size, current_protection  | PAGE_GUARD, &old_protection);```

[2025-03-24 14:17] pinefin: [replying to laughtersec: "Here's an example 
https://cocomelonc.github.io/tu..."]
hes not asking for windows api. rather he wants all memory access

[2025-03-24 14:18] pinefin: i guess you could use soft breakpoints with windbg too

[2025-03-24 14:19] pinefin: `ba r <ADDRESS> L1000`  or whatever

[2025-03-24 14:46] nu11sec: Thanks for the insight

[2025-03-24 14:46] nu11sec: I think I would prefer using ept hooks

[2025-03-24 14:46] nu11sec: Want to learn something new

[2025-03-24 14:48] nu11sec: [replying to pinefin: "if you set PAGE_GUARD on all pages then you should..."]
Would this be hit with performance though

[2025-03-24 14:48] nu11sec: accesses to ntdll are so frequent

[2025-03-24 14:48] pinefin: any route would be a performance hit

[2025-03-24 14:48] pinefin: hypervisor would be your best bet

[2025-03-24 14:49] nu11sec: Yes but would have to deal with anti vm no ?

[2025-03-24 14:49] pinefin: whats your end goal?

[2025-03-24 14:51] nu11sec: ntdll modifications and integrity checks

[2025-03-24 14:51] pinefin: you want to bypass integrity checks? or u want to place integrity checks

[2025-03-24 14:52] pinefin: whos anti vm are you trying to get past?

[2025-03-24 14:52] nu11sec: [replying to pinefin: "you want to bypass integrity checks? or u want to ..."]
I want to monitor

[2025-03-24 14:52] pinefin: right

[2025-03-24 14:52] pinefin: but where does anti vm come into place

[2025-03-24 14:52] nu11sec: [replying to pinefin: "whos anti vm are you trying to get past?"]
Themida

[2025-03-24 14:53] nu11sec: Setting ept hooks wouldn’t you require a VM ?

[2025-03-24 15:03] pinefin: look into hyperplatform

[2025-03-24 15:04] nu11sec: Yeah thanks

[2025-03-25 22:15] Mintia: Is there a way to receive hardware breakpoint exceptions without having KiUserExceptionDispatcher modify the stack, other than having another process debug mine?

[2025-03-25 22:18] Matti: kernel debugger hw breakpoints should work exactly like this

[2025-03-25 22:19] Matti: but of course you now require another machine, instead of another process

[2025-03-25 22:19] Matti: whether that works for you idk

[2025-03-25 22:19] Matti: kd hw bps can also be set to filter per EPROCESS btw (just like regular ones), a lot of people don't seem to know this

[2025-03-25 22:47] Mintia: It's simpler to just get another process to do the receiving instead of another machine in this case

[2025-03-25 22:56] Matti: well of course that's simpler <:lillullmoa:475778601141403648>

[2025-03-25 22:58] Matti: maybe I misunderstood; is the requirement only not having to *debug* the process with the second process? if so I'm wondering how you would do that

[2025-03-25 22:58] Matti: and still receive the hw bp notifications, I mean

[2025-03-25 23:00] Mintia: Misread your message sorry

[2025-03-25 23:00] Mintia: [replying to Matti: "maybe I misunderstood; is the requirement only not..."]
Yeah and also not modifying the stack

[2025-03-25 23:00] Mintia: Because I could just attach a veh handler

[2025-03-25 23:00] Mintia: But the program stashes stuff in parts of the stack that are modified by the exception handler

[2025-03-25 23:00] Matti: ah I think I misunderstood your original question then

[2025-03-25 23:01] Matti: you already figured out a solution, but you were asking if there isn't a simpler way

[2025-03-25 23:01] Mintia: Yes

[2025-03-25 23:01] Matti: not that I know of

[2025-03-25 23:01] Mintia: I could just attach a second process and do the redirecting there

[2025-03-25 23:01] Mintia: But I wanted to see if there was a simpler way

[2025-03-25 23:01] Mintia: But I guess not

[2025-03-25 23:02] Mintia: Because ntoskrnl pushes the context to the stack

[2025-03-25 23:02] Matti: yeah, I interpreted it as a requirement, i.e. cannot be done using a second process

[2025-03-25 23:03] Matti: [replying to Mintia: "Because ntoskrnl pushes the context to the stack"]
of the debuggee?

[2025-03-25 23:03] Mintia: Yes

[2025-03-25 23:03] Mintia: I think so

[2025-03-25 23:03] Matti: hmm that's interesting, was not aware of this

[2025-03-25 23:04] Matti: actually given that it's a hw bp I'm not even sure it's the kernel that's doing this

[2025-03-25 23:04] Mintia: Because when you have an exception KiUserExceptionDispatcher is called from ntoskrnl with the CONTEXT pushed on the stack

[2025-03-25 23:04] Mintia: If I'm not wrong exception handling works like this

[2025-03-25 23:04] Matti: oh

[2025-03-25 23:04] Mintia: Don't know if it's different with hw bps

[2025-03-25 23:05] Mintia: But it shouldn't be

[2025-03-25 23:05] Matti: ok then yeah it is the user mode part of the exception dispatch

[2025-03-25 23:06] Matti: [replying to Mintia: "Don't know if it's different with hw bps"]
I don't know, but hw bps in general work very differently than sw bps

[2025-03-25 23:06] Mintia: Yes but it's still an exception, it just doesn't modify running code

[2025-03-25 23:08] JustMagic: [replying to Mintia: "Yes"]
Debug port or exception port is the only way from usermode

[2025-03-25 23:10] Mintia: [replying to JustMagic: "Debug port or exception port is the only way from ..."]
Google doesn't bring anything meaningful up, could you please give me the rough picture of what's that

[2025-03-25 23:10] Mintia: Or are you taking about having another process do the exception recieving

[2025-03-25 23:10] Matti: can't the exception port only be set once?

[2025-03-25 23:11] Matti: system wide

[2025-03-25 23:11] Mintia: That guy really wants me to talk to him

[2025-03-25 23:11] Matti: ah no nvm

[2025-03-25 23:11] JustMagic: Debug port is the standard debugger mechanism

[2025-03-25 23:11] Matti: that's the default exception port I'm talking about

[2025-03-25 23:12] JustMagic: Exception port is an obscure alternative that requires TCB privilege

[2025-03-25 23:12] Mintia: [replying to JustMagic: "Debug port is the standard debugger mechanism"]
oh ok, yeah figured that

[2025-03-25 23:12] Matti: yeah you can set the exception port per process

[2025-03-25 23:12] Matti: as an alternative to the debug port, for Dbgk (= debugger subsystem) communication

[2025-03-25 23:13] JustMagic: [replying to Mintia: "Or are you taking about having another process do ..."]
It's not entirely necessary for it to be another process with enough fuckery

[2025-03-25 23:14] Matti: lol

[2025-03-25 23:14] Mintia: What fuckery lol

[2025-03-25 23:14] Matti: a lot of fuckery

[2025-03-25 23:14] Matti: but it's a funny thought yeah <:kekw:904522300257345566>

[2025-03-25 23:14] Matti: the port is an LPC port

[2025-03-25 23:15] Matti: they communicate with TIDs as sender/receiver identifiers

[2025-03-25 23:15] Matti: not PIDs

[2025-03-25 23:15] JustMagic: [replying to Mintia: "What fuckery lol"]
You can technically run the debug loop in your own process as long as you create all threads (or just the one that does the debugging) with a special flag that doesn't suspend them on exception

[2025-03-25 23:16] Mintia: But DebugActiveProcess errors out when I try to debug my own process

[2025-03-25 23:16] Mintia: Maybe because I haven't tried with this special flag?

[2025-03-25 23:17] JustMagic: [replying to Mintia: "But DebugActiveProcess errors out when I try to de..."]
You have to create the debug port in another process and then clone the handle into your own. Basically you just need to pass the initial check that you're not trying to debug your own process, but beyond that it doesn't matter

[2025-03-25 23:17] JustMagic: But I don't think exception port even has this check at all

[2025-03-25 23:18] Matti: it does not

[2025-03-25 23:20] Mintia: Ok so I just write a simple program that debugs my process and I clone the handle into the other process or I use this exception port thing

[2025-03-25 23:20] JustMagic: [replying to Mintia: "Ok so I just write a simple program that debugs my..."]
Sure. Or you can just clone/fork your process and not have another executable

[2025-03-25 23:21] Matti: debugging using the regular dbg API is about a 100x simpler than the exception port idea unless you know how LPC works

[2025-03-25 23:21] Matti: it's just a funny thought

[2025-03-25 23:22] Mintia: I don't really know how lpc works so I'll just do semi-normal debugging stuff

[2025-03-25 23:22] Matti: it's not rocket science or anything

[2025-03-25 23:22] JustMagic: I also wouldn't recommend taking my idea seriously either unless you actually have a reason why second process is bad. Self debugging is going to be the most painful to yourself first and foremost even if it's intended as anti-debug

[2025-03-25 23:23] Matti: but it is quite a lot more to learn to accomplish the same thing in the end

[2025-03-25 23:26] Mintia: [replying to JustMagic: "I also wouldn't recommend taking my idea seriously..."]
It's not really an issue if it doesn't have to run continuously and requires using WriteProcessMemory and similar functions (I already need Get/SetThreadContext, so those are fine).

[2025-03-25 23:26] Mintia: I'll just clone the debug port across processes

[2025-03-25 23:26] Mintia: It's fine (It'll probably come back to bite me in the rear)

[2025-03-25 23:33] Mintia: The debug port is the handle in TEB->DbgSsReserved[1], right?

[2025-03-25 23:35] Matti: erm

[2025-03-25 23:35] Matti: yes and no

[2025-03-25 23:35] Matti: but mostly no

[2025-03-25 23:36] Matti: the debug port is a part of the EPROCESS in kernel mode

[2025-03-25 23:36] Matti: but, the NtCreateProcess APIs temporarily store it there when you create a debugged process for convenient access

[2025-03-25 23:36] Matti: it's kind of a hack

[2025-03-25 23:36] Matti: if you just want the handle though, then yes that works

[2025-03-25 23:38] Matti: the exception port is the same; it's part of a kernel mode struct but there is a user mode API to set it

[2025-03-25 23:39] Mintia: Ok so I just clone it, put it in the special thread's TEB and now it can debug it's process?

[2025-03-25 23:40] Matti: mm, I mean you can but that's not really any different from using an exception port manually for debugging

[2025-03-25 23:40] Matti: why the need for the clone?

[2025-03-25 23:41] Matti: you can clone your own *process* to debug itself though

[2025-03-25 23:41] Mintia: Yeah that's true

[2025-03-25 23:41] Mintia: It isn't any different if I just clone my process

[2025-03-25 23:41] Mintia: Yeah I should just do that

[2025-03-25 23:42] Mintia: Thanks for the help anyways

[2025-03-25 23:42] Matti: you don't need to do this either by the way... all it does is save you needing a second executable

[2025-03-25 23:42] Matti: but depending on what you are doing there is something to be said for having a debuggee.exe and a debugger.exe

[2025-03-25 23:42] Matti: otherwise one exe will need to contain the logic for both

[2025-03-25 23:44] Mintia: The logic is not inside of a process but instead inside of a dll, it was for this reason that I was trying to find a way to not have the second process

[2025-03-25 23:44] Matti: a

[2025-03-25 23:44] Mintia: Because I hook some instructions and when the exception handler runs it modifies the stack

[2025-03-25 23:44] Matti: here is another fun fact then: you can create DLL processes!

[2025-03-25 23:45] Mintia: Huh, Windows has really obscure stuff lol

[2025-03-25 23:45] Matti: not sure it would help with this though

[2025-03-25 23:45] Matti: all it does is run a DLL instead of an EXE

[2025-03-25 23:46] Mintia: It would, because I don't need to run a 700mb exe two times

[2025-03-25 23:46] Mintia: And I can just run the process a single time and then the dll

[2025-03-25 23:46] Mintia: As a child

[2025-03-25 23:46] Matti: well... this is a 700 MB exe with a DLL, and the DLL itself has no further dependencies?

[2025-03-25 23:46] Matti: not significant ones anyway

[2025-03-25 23:47] Mintia: [replying to Matti: "well... this is a 700 MB exe with a DLL, and the D..."]
Just a console to print logs to

[2025-03-25 23:47] Mintia: So kernel32 and user32 I think

[2025-03-25 23:47] Mintia: Apart from that no

[2025-03-25 23:47] Matti: hmm yeah ok I see the benefit then I suppose

[2025-03-25 23:47] Mintia: I don't even run any code in the dll entrypoint

[2025-03-25 23:48] Matti: the bad news is that creating a DLL process is not super trivial <:harold:704245193016344596>

[2025-03-25 23:48] Matti: because you'll need to call the NT API instead of CreateProcess

[2025-03-25 23:48] Mintia: [replying to Matti: "the bad news is that creating a DLL process is not..."]
The stuff I was trying to do before wasn't also trivial so...

[2025-03-25 23:48] Matti: either that or (probably simpler) patch kernel32.dll to remove the DLL restriction

[2025-03-25 23:49] Mintia: I need to look at it in Ida but it would be simpler

[2025-03-25 23:49] Matti: it explicitly disallows DLL files when it makes the syscall to create the process

[2025-03-25 23:49] Mintia: It makes sense lol

[2025-03-25 23:50] Mintia: I think I saw some code in titan engine that I think did something similar, I'll go look

[2025-03-25 23:51] Matti: hm oh yeah

[2025-03-25 23:52] Matti: I think I wrote that? but I don't remember committing it publicly....

[2025-03-25 23:52] Mintia: Could be mis remembering but I saw a function called InitDllDebug

[2025-03-25 23:52] Mintia: Don't know if that's the same thing

[2025-03-25 23:53] Mintia: And GitHub mobile isn't really helping

[2025-03-25 23:53] Matti: oh yeah I did write that

[2025-03-25 23:53] Matti: fuck my memory

[2025-03-25 23:54] Matti: `CreateInfo.InitState.u1.s1.ProhibitedImageCharacteristics = 0; // Normally: IMAGE_FILE_DLL (disallow executing DLLs)`

[2025-03-25 23:54] Matti: this is the relevant line

[2025-03-25 23:55] Mintia: Ok if this is the relevant line I'm looking at the wrong function lol

[2025-03-25 23:55] Matti: kernel32.dll will contain veeery roughly similar logic in CreateProcessInternalW, and then the same line except setting it to IMAGE_FILE_DLL

[2025-03-25 23:55] Matti: https://bitbucket.org/titanengineupdate/titanengine-update/pull-requests/12/diff#LTitanEngine/TitanEngine.Debugger.cppT286
[Embed: Bitbucket]

[2025-03-25 23:56] Matti: process hacker likely also has good example code for this API

[2025-03-25 23:56] Matti: if you want to call it manually

[2025-03-25 23:57] Matti: but, personally I'd just patch the flag in kernel32 lol

[2025-03-25 23:57] Mintia: [replying to Matti: "but, personally I'd just patch the flag in kernel3..."]
Yeah it's simpler lol

[2025-03-25 23:57] Mintia: Wth how did I get the regular role

[2025-03-25 23:58] Mintia: It's the first time that I talk in here

[2025-03-25 23:58] Mintia: Lol whatever

[2025-03-25 23:58] Matti: fuck knows

[2025-03-25 23:58] Matti: I'm only a fake moderator... I was made a mod to harm my credibility

[2025-03-25 23:59] Matti: by one of the other mods (who are all fascists btw)

[2025-03-25 23:59] Matti: idk where the roles come from

[2025-03-25 23:59] Mintia: [replying to Matti: "by one of the other mods (who are all fascists btw..."]
oh cool 😂

[2025-03-25 23:59] Mintia: [replying to Matti: "idk where the roles come from"]
Maybe Mee6? It's pinging me every like five minutes

[2025-03-26 00:00] Matti: oh well yeah, that's the bot

[2025-03-26 00:00] Matti: but I mean someone has set up rules for it to do this

[2025-03-26 00:00] Mintia: yeah true

[2025-03-26 00:00] Mintia: whatever

[2025-03-26 00:00] Mintia: thanks for the help, really appreciate it

[2025-03-26 00:00] Matti: nps!

[2025-03-26 03:38] CyNickal: Hey everyone, I am learning Zydis and running into an issue I have not been able to find the solution to.

I am trying to encode `mov rax, gs:[0x0000000000000060]`, but no matter what combination of Encoder Request I make I cant seem to do it correctly. Furthermore I couldn't find any examples of anything similar so I am pretty stuck. 

Here is the request I am currently sending:
```
ZydisEncoderRequest req;
memset(&req, 0, sizeof(req));

req.mnemonic = ZYDIS_MNEMONIC_MOV;
req.machine_mode = ZYDIS_MACHINE_MODE_LONG_64;
req.operand_count = 2;
req.prefixes = ZYDIS_ATTRIB_HAS_SEGMENT_GS;

req.operands[0].type = ZYDIS_OPERAND_TYPE_REGISTER;
req.operands[0].reg.value = ZYDIS_REGISTER_RAX;    

req.operands[1].type = ZYDIS_OPERAND_TYPE_MEMORY;
req.operands[1].mem.base = ZYDIS_REGISTER_GS;
req.operands[1].mem.displacement = 0x0000000000000060;
req.operands[1].mem.index = ZYDIS_REGISTER_NONE;
req.operands[1].mem.size = 0x8;
req.operands[1].mem.scale = 1;
```
Zyan Error code: 8020000c
Any help is greatly appreciated 🙃

[2025-03-26 04:52] CyNickal: [replying to CyNickal: "Hey everyone, I am learning Zydis and running into..."]
Fixed; scale needs to be 0.
Figured out by using the `ZydisEncoderDecodedInstructionToEncoderRequest` function on the known bytes the compared the requests.

[2025-03-26 15:30] pinefin: [replying to CyNickal: "Hey everyone, I am learning Zydis and running into..."]
another way i get around weird things like this when i run into weird shit like this and stump my head, i assemble the instruction (i like to use defuse.ca for small things like this)

```c++
char operands[] = {0x65, 0x48, 0x8b, 0x04, 0x25, 0x60, 0x00}; //mov rax, gs:[0x0000000000000060]

//decode with zydis here

//print out ALL the results of the decode
```

maybe not the best way to go about it, but it works for me

[2025-03-28 19:38] Matti: [replying to pinefin: "another way i get around weird things like this wh..."]
bit of a late post but this is exactly what I do as well, except using x64dbg as a scratchpad instead

[2025-03-28 19:38] Matti: step 1
[Attachments: image.png]

[2025-03-28 19:38] Matti: step 2 (`zi` is an alias for `ZydisInfo.exe` on my system)
[Attachments: image.png]

[2025-03-28 19:39] snowua: [replying to Matti: "step 1"]
Seems like you might be missing time wasted debugging

[2025-03-28 19:39] Matti: x64dbg is faster, and it also uses zydis for output

[2025-03-28 19:39] Matti: [replying to snowua: "Seems like you might be missing time wasted debugg..."]
no I do not miss that tyvm

[2025-03-28 19:39] Matti: in fact the very first thing I did when I cloned x64dbg for the first time like a decade ago was kill that shit

[2025-03-28 19:40] Matti: it drives me fucking mad

[2025-03-28 19:40] Lyssa: I should invent ignoring things

[2025-03-28 19:41] Matti: I think some people can already just do that

[2025-03-28 19:41] Matti: like by default

[2025-03-28 19:41] Lyssa: damn, someone beat me to it

[2025-03-28 19:41] Lyssa: that's a shame

[2025-03-28 19:41] Lyssa: until my next million dollar idea...

[2025-03-28 19:42] Matti: I can't so I have to resort to doing this kinda stuff

[2025-03-28 19:42] Lyssa: when I saw it I smirked and thought "cool"

[2025-03-28 19:42] Lyssa: then forgot it existed

[2025-03-28 19:42] Matti: *it updates every second*

[2025-03-28 19:43] Lyssa: for me that timer is always broken for some reason

[2025-03-28 19:43] Lyssa: sometimes it stops for an entire minute

[2025-03-28 19:43] Matti: this is distracting me from my job of hacking the NSA!!!! EVERY SECOND

[2025-03-28 19:43] Matti: seriously though for me it literally work as precisely 1 distraction/second

[2025-03-28 19:43] Matti: I see something change

[2025-03-28 19:44] Matti: I need to read it

[2025-03-28 19:44] Lyssa: [replying to Matti: "this is distracting me from my job of hacking the ..."]
can you hack my ex's crypto wallet instead cheers

[2025-03-28 19:44] Matti: no, I'm opposed to storing crypto because that makes it like an investment

[2025-03-28 19:44] Matti: bitcoin was invented for buying drugs

[2025-03-28 19:44] Matti: but

[2025-03-28 19:45] Matti: I do have a solution for you

[2025-03-28 19:45] Matti: get back together with your ex, then ask him for his wallet password

[2025-03-28 19:45] Matti: or hers

[2025-03-28 19:45] Matti: anyway

[2025-03-28 19:45] Lyssa: [replying to Matti: "get back together with your ex, then ask him for h..."]
okay!

[2025-03-28 19:46] Lyssa: I'll blame you in court

[2025-03-28 19:46] Matti: wow

[2025-03-28 19:46] Matti: happy to help, come again

[2025-03-28 19:49] Matti: [replying to Matti: "step 1"]
I just realised screen 1 of step 1 could use a tweak to make this even faster, by borrowing from keypatch's UI

[2025-03-28 19:49] Matti: 
[Attachments: image.png]

[2025-03-28 19:50] Matti: keystone is a horri... not so great assembler in my opinion, and I have lots of fights with it in IDA

[2025-03-28 19:50] Matti: but the part you can copy the bytes right out is genius

[2025-03-28 19:51] Matti: x64dbg is even already showing the bytes.... they just need to go into an edit field

[2025-03-28 19:57] Matti: ugh, I would really like to have this but it would involve touching the GUI

[2025-03-28 19:58] Matti: idk.... it's not exactly a medical emergency is it

[2025-03-28 19:59] Matti: I'd just like to have it

[2025-03-28 20:02] Matti: I feel like I should probably prioritise making a hostile fork that restores XP support first

[2025-03-28 20:02] pinefin: matti lets commit tax evasion

[2025-03-28 20:02] Matti: wow

[2025-03-28 20:03] Matti: no I don't think that's a good idea

[2025-03-28 20:03] Matti: it's illegal

[2025-03-28 20:03] pinefin: oh

[2025-03-28 20:03] pinefin: ok sorry

[2025-03-28 20:04] segmentationfault: Bitcoin is supposed to buy you Ritalin or adderal, not to make millions with it

[2025-03-28 20:05] Matti: ain't that the fucking truth

[2025-03-28 20:07] Matti: bitcoin's literal purpose is facilitating the buying of drugs

[2025-03-28 20:07] Matti: I feel like a lot of people totally misunderstand this

[2025-03-28 20:46] NSA, my beloved<3: [replying to Matti: "in fact the very first thing I did when I cloned x..."]
Is the only way to get rid of it is by compiling the binary for yourself? It drives me crazy, I loathe this feature and could never find an option to turn it off.

[2025-03-28 20:49] diversenok: You can probably patch the string with an early zero terminator

[2025-03-28 20:50] Brit: low tech solutions are often the best

[2025-03-28 20:50] NSA, my beloved<3: That sounds awful, I love it. I'll probably just do that.

[2025-03-28 20:51] Matti: yeah this will almost certainly work

[2025-03-28 20:52] Matti: it's pretty basic code, only the string is probably gonna be a QString and I forget what encoding those use

[2025-03-28 20:53] Matti: use a double null terminator just in case maybe

[2025-03-28 20:53] Matti: but I think it's utf-8

[2025-03-28 20:53] NSA, my beloved<3: Sure thing, thanks!

[2025-03-30 18:24] NSA, my beloved<3: Anyone has any tips on using the SEH tab in x64dbg please? No matter what I do, there is no output in there.

[2025-03-30 20:25] mrexodia: [replying to NSA, my beloved<3: "Anyone has any tips on using the SEH tab in x64dbg..."]
it's only supported for 32 bit