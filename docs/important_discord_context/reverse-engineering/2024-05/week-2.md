# May 2024 - Week 2
# Channel: #reverse-engineering
# Messages: 115

[2024-05-06 21:16] zeropio: heyo guys, dumb question
I'm dumping a packed malware with x64dbg (32 bit version), I tried using the savedata command but can't get it to work, I'm doing something wrong here?
[Attachments: image.png]

[2024-05-06 21:16] zeropio: I'm just following the wiki https://help.x64dbg.com/en/latest/commands/memory-operations/savedata.html
I tried with full path and decimal numbers

[2024-05-06 21:16] zeropio: if this doesn't work, any way to easily dump a bunch of memory using x64dbg? (without python)

[2024-05-06 21:34] Redhpm: [replying to zeropio: "heyo guys, dumb question
I'm dumping a packed malw..."]

[Attachments: Screenshot_20240506-233354.jpg]

[2024-05-06 21:35] zeropio: yeah I was dumb

[2024-05-06 21:35] zeropio: thanks <a:pixel_toro_lovehearts:1236110530721222677>

[2024-05-06 21:36] Redhpm: No problem

[2024-05-08 02:55] abu: Hey has anyone encountered performance issues when using miasm/triton to deobfuscate traces? I've been trying to use it for a trace of 230k instructions but it's taking a pretty long time. Normal behavior?

[2024-05-08 03:54] Deleted User: [replying to abu: "Hey has anyone encountered performance issues when..."]
triton is quite slow yes

[2024-05-08 04:25] Horsie: [replying to abu: "Hey has anyone encountered performance issues when..."]
I've been planning to do this (use triton on very long traces) too

[2024-05-08 04:26] Horsie: What kind of times are you seeing and on what cpu, if you don't mind?

[2024-05-08 06:59] abu: [replying to Horsie: "I've been planning to do this (use triton on very ..."]
1 hour +

[2024-05-08 06:59] abu: [replying to Horsie: "What kind of times are you seeing and on what cpu,..."]
Ryzen 5 3600x

[2024-05-08 06:59] Horsie: [replying to abu: "1 hour +"]
Is that when it finished or are you still waiting? <:topkek:904522829616263178>

[2024-05-08 06:59] abu: When it finished

[2024-05-08 07:00] Horsie: I see...

[2024-05-08 07:00] abu: I went out to see a movie tbh

[2024-05-08 07:00] abu: and left it simplifying

[2024-05-08 07:00] abu: but it was going for a long ass time

[2024-05-08 07:00] abu: Python version btw

[2024-05-08 07:00] abu: maybe different on c++

[2024-05-08 07:00] Horsie: I'm also going to be running it on 100k+ of traces but multiple of them

[2024-05-08 07:00] Horsie: [replying to abu: "Python version btw"]
Oh lol

[2024-05-08 07:00] abu: Need to write analyses using souffle+haskell

[2024-05-08 07:00] Horsie: Yeah, I'm a bit more optimistic then

[2024-05-08 07:00] abu: Yeah let me know how it goes lol

[2024-05-08 07:01] abu: ill use c++ tomorrow and see

[2024-05-08 07:01] Horsie: Yeah. I cant use triton without referencing its source 10x per minute

[2024-05-08 07:01] abu: same thing with miasm lol

[2024-05-08 14:58] Azertinv: are offset from HMODULE akin to offset in the dll file ?

[2024-05-08 14:59] Azertinv: I have a malware which uses offset from its own hmodule to find a payload, in IDA I tried jump to file offset but it seems to give me garbo

[2024-05-08 15:04] Brit: the handle you get from getmodulehandle is usually also that module's base addr

[2024-05-08 15:12] Azertinv: What is a module's base addr then ? is it just the first mapped segment ?

[2024-05-08 15:14] donna🤯: [replying to Azertinv: "What is a module's base addr then ? is it just the..."]
base of the image

[2024-05-08 15:15] donna🤯: i.e beginning of the pe header

[2024-05-08 15:15] Azertinv: So it should be a dumb offset in the file right ?

[2024-05-08 15:16] Azertinv: I'll try to dump the payload again maybe I made a mistake

[2024-05-08 15:53] Henke37: [replying to Azertinv: "I have a malware which uses offset from its own hm..."]
that's a relative virtual address

[2024-05-08 15:53] Henke37: read the PE specification, it explains what they are

[2024-05-08 16:04] diversenok: Yeah, PE files have two different layouts: on-disk and in-memory; HMODULE is just a base address in-memory, so having offsets relative to it means dealing with relative virtual addresses

[2024-05-08 16:05] Torph: [replying to Azertinv: "I have a malware which uses offset from its own hm..."]
the sections are not necessarily loaded into virtual memory at the same offsets from each other that they'd be in the file. I don't remember the exact rules, but for example they might move a section forward a few KB to align it to the next MiB boundary or something like that

[2024-05-08 16:54] Azertinv: All good I found the BaseAddress of my module it was just 0x400000

[2024-05-08 16:55] Azertinv: Weird that IDA doesn't create a segment for the base of the image and only starts at  .text 0x401000

[2024-05-08 16:55] Azertinv: and the payload make much more sense

[2024-05-09 09:17] not nezu: are there any good debuggers on linux similar to x64dbg. and if your answer is GDB, are there any good GUIs for it. Everything I see either seems unfinished, focused on source debugging (this is <#835635446838067210> LOL), or stuck 30 years in the past. Am i just blind or what?

[2024-05-09 09:19] not nezu: or should I just larn to raw dog the cli...

[2024-05-09 09:19] Deleted User: <@918578766647808010> pwndbg or gef even tho they are TUI's

[2024-05-09 09:20] not nezu: I don't mind a good TUI, thx, will try

[2024-05-09 09:43] sodi: [replying to abu: "Hey has anyone encountered performance issues when..."]
ive ran ~6 billion instructions through triton in about 17 hours give or take, make sure to properly concretize any memory/registers you are not interested in and its speed increases drastically (albeit nowhere near something like unicorn)

[2024-05-09 09:44] abu: [replying to sodi: "ive ran ~6 billion instructions through triton in ..."]
python?

[2024-05-09 09:45] sodi: c++ unfortunately, not sure how bad it would be on python

[2024-05-09 09:45] sodi: regardless, the only time ive seen triton start to struggle is when i forget to concretize stuff im not interested in anymore

[2024-05-09 09:46] abu: alright, thank you

[2024-05-09 14:00] sariaki: binja has a gui debugger as well

[2024-05-09 18:30] yegor: [replying to not nezu: "are there any good debuggers on linux similar to x..."]
https://github.com/eteran/edb-debugger -- the last time I used it was quite a while ago, but it worked fine
If you've got an ida, you can use remote GDB debugger (with `gdbserever`) or remote Linux debugger (with linux_server/linux_server64 from `{IDADIR}/dbgsrv`)

[2024-05-09 22:46] dullard: Trolling ?

[2024-05-09 22:46] dullard: x64dbg

[2024-05-09 23:10] abu: LMAO

[2024-05-10 00:39] yoshixi: are there any useless certifications i can get for reversing?

[2024-05-10 13:23] daax: [replying to yoshixi: "are there any useless certifications i can get for..."]
GREM, wouldn’t classify it as useless though lol

[2024-05-10 13:33] luci4: Heard good things about GREM

[2024-05-10 13:34] luci4: Not so many good things about the price though. I don't know anyone who paid out of pocket for it

[2024-05-10 13:36] zeropio: [replying to yoshixi: "are there any useless certifications i can get for..."]
zero2auto (cheap version of grem)

[2024-05-10 16:20] vendor: [replying to luci4: "Heard good things about GREM"]
i did GREM like 5/6 years ago, it’s expired now but found it pretty basic

[2024-05-10 16:21] vendor: i don’t think there was much actual reversing in it. just stuff about looking at imports, strings, packer signatures etc.

[2024-05-10 16:21] vendor: and ofc the exam is stupid easy as loads of multiple choice questions

[2024-05-10 16:22] vendor: i think there might have been a practical aspect with a web vm but maybe i’m mixing up certs it’s been so long

[2024-05-10 16:24] vendor: i had to take the exam in the sketchiest place ever tho lol. recommend doing a google maps street view check before booking the place you do it <:kekw:904522300257345566>

[2024-05-10 19:54] repnezz: How do I get x64dbg or windbg to work on XP?

[2024-05-10 19:55] repnezz: tried to install the older sdk, says setup failed (for windbg), then x64dbg says some api wasn’t found in kernel32

[2024-05-10 20:09] brymko: u heard wrong

[2024-05-10 20:09] 5pider: have u tried ?

[2024-05-10 20:10] 5pider: u can also debug x64 applications

[2024-05-10 20:10] 5pider: lmfao

[2024-05-10 20:11] 5pider: 
[Attachments: image.png]

[2024-05-10 20:12] 5pider: see works with x64 applications

[2024-05-10 20:12] 5pider: i am not sure about wine

[2024-05-10 20:12] diversenok: No, 64-bit debuggers do not exist, lol

[2024-05-10 20:12] repnezz: Shouldnt it be backwards compatible

[2024-05-10 20:12] 5pider: [replying to diversenok: "No, 64-bit debuggers do not exist, lol"]
wdym ? maybe i am missunderstanding something

[2024-05-10 20:13] diversenok: I'm joking

[2024-05-10 20:13] 5pider: ah lmfao

[2024-05-10 20:13] 5pider: 😔

[2024-05-10 20:16] repnezz: [replying to repnezz: "How do I get x64dbg or windbg to work on XP?"]
Ok I’ll try to tackle it from another direction and kernel debug the box , there are these 2  commands I always run in the box to set the environment up - 

bcdedit /debug on
bcdedit /dbgsettings serial debugport:2 baudrate:115200

Now bcdedit is supported only since vista … so what now ?

[2024-05-10 20:17] diversenok: WinDbg from the SDK comes in both 32- and 64-bit flavours; WinDbg Preview is also 64-bit

[2024-05-10 20:18] 5pider: damn

[2024-05-10 20:18] diversenok: Yeah, that's a while ago

[2024-05-10 20:18] 5pider: you think

[2024-05-10 20:19] diversenok: I mean, I do still actively use several tools that are older than that

[2024-05-10 20:19] diversenok: As long as it works ¯\_(ツ)_/¯

[2024-05-10 20:21] diversenok: But 32-bit debuggers are definitely not part of them; WoW64 process have limited options when it comes to inspecting 64-bit processes

[2024-05-10 20:25] Windy Bug: [replying to repnezz: "Ok I’ll try to tackle it from another direction an..."]
1. open c:\boot.ini in notepad 
2. copy the last line 
3. add the following switches :  /debug /debugport=com1 /baudrate=115200

[2024-05-10 20:29] diversenok: I believe your info was outdated even back then; here is a 64-bit WinDbg from the Vista SDK
[Attachments: image.png]

[2024-05-10 20:35] diversenok: And here is an even older one from Windows server 2003 SDK
[Attachments: image.png]

[2024-05-10 20:36] diversenok: What year are you leaving in? 😂

[2024-05-10 20:50] repnezz: anyone knows why a breakpoint in the context of explorer.exe on user32!ExitWindowsEx is NOT being triggered when initiating a shutdown ? docs suggest this function is responsible for starting the shutdown process , and surely it starts in explorer ?

[2024-05-10 21:10] diversenok: Maybe try a breakpoint inside `ShellExperienceHost.exe`

[2024-05-10 21:11] diversenok: Although, no, it doesn't have the shutdown privilege 🤔

[2024-05-10 21:20] repnezz: 
[Attachments: image.png]

[2024-05-10 21:20] repnezz: Guess it’s winlogon?

[2024-05-10 21:21] diversenok: Possible

[2024-05-10 21:22] diversenok: I was about to suggest to set up a breakpoint on `NtUserSetInformationThread` with a kernel debugger since `ExitWindowsEx` eventually calls it

[2024-05-10 21:22] repnezz: well as the snippet shows I set a bp on nt!ZwShutdownSystem

[2024-05-10 21:23] repnezz: it’s also being called I guess ?

[2024-05-10 21:23] diversenok: Maybe; not entirely sure what win32k does

[2024-05-10 21:25] diversenok: It might not go via the syscall and call something internal instead

[2024-05-10 21:42] Windy Bug: there’s a section about shutdown in the windows internals book

[2024-05-10 23:32] the eternal bleb: [replying to repnezz: ""]
How are u debugging it like that

[2024-05-10 23:32] the eternal bleb: I still need to learn proper kernel debugging

[2024-05-10 23:32] szczcur: [replying to the eternal bleb: "How are u debugging it like that"]
windbg

[2024-05-11 00:05] the eternal bleb: Yeah ik

[2024-05-11 00:05] the eternal bleb: But he can see so much

[2024-05-11 00:06] the eternal bleb: Like the hex and stuff

[2024-05-11 02:41] froj: look at the commands chief

[2024-05-12 21:01] raax: [replying to the eternal bleb: "But he can see so much"]
Windbg is good