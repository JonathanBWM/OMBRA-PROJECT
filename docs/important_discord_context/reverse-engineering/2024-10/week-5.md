# October 2024 - Week 5
# Channel: #reverse-engineering
# Messages: 80

[2024-10-29 11:47] 0x208D9: anyone in here have the ntoskrnl of version KB5041585? winbindex gives a 404

[2024-10-29 11:47] 0x208D9: [replying to 0x208D9: "anyone in here have the ntoskrnl of version KB5041..."]
respective hash: e40b74571521d1f63d387ae441b3296e505a29550be75c770851832f064d2f40

[2024-10-29 12:16] Deleted User: Hey does anyone have any good starter resources around malware reverse engineering, I have done some malware analysis stuff but it’s mainly been .NET based.

Really interested in learning but getting lost in the huge amount of pre existing stuff kinda need a roadmap type thing.

[2024-10-29 12:23] zeropio: [replying to 0x208D9: "anyone in here have the ntoskrnl of version KB5041..."]
you can apply the patch and download it yourself https://www.catalog.update.microsoft.com/Search.aspx?q=KB5041585

[2024-10-29 12:23] 0x208D9: [replying to zeropio: "you can apply the patch and download it yourself h..."]
thats what i did apparently

[2024-10-29 13:39] diversenok: Oh, you figured out how to trigger it, nice

[2024-10-29 14:58] Timmy: link to the vid is dead for me, but cool!

[2024-10-30 20:46] EfraimDays: Guys, noob question.
An old game that I used to play now uses a driver to protect against cheats.
But I think it’s very easy to bypass.
For example, if I use a compiled version of Cheat Engine and DBVM, I can edit, write memory, and change the ASM. But if I want to bypass it completely, what should I do? Maybe check the threads using Cheat Engine to see if any of them are from the driver and edit the return value? Should I try to track the Windows error when it detects me? Any suggestions?

[2024-10-30 21:32] Matti: if this such an old game, why did it switch to using a driver for anti cheat?

[2024-10-30 21:33] Matti: is it maybe not quite that old after all, or old in the way LoL is old

[2024-10-30 21:33] Brit: please stop applying critical thinking to this prompt

[2024-10-30 21:33] Brit: it might unmask the user's actual motives

[2024-10-30 21:33] Matti: I apologise

[2024-10-30 21:34] Matti: unfortunately I don't use cheat engine so I can't help \:(

[2024-10-30 22:32] EfraimDays: [replying to Matti: "is it maybe not quite that old after all, or old i..."]
Yeah, it's old, but it has a large nostalgic community. Maybe it's the way they found to maintain fair play, but I was shocked when I discovered that I had a driver running on my PC that I didn't recognize

[2024-10-30 22:33] EfraimDays: [replying to EfraimDays: "Guys, noob question.
An old game that I used to pl..."]
Is it a problem to ask this type of thing on this server? If so, I won't ask anymore.

[2024-10-30 23:20] Torph: [replying to EfraimDays: "Guys, noob question.
An old game that I used to pl..."]
\> "driver anticheat"
\> usermode mem read/write still works
isn't that like first order of business for driver-level protection

[2024-10-31 03:04] EfraimDays: [replying to Torph: "\> "driver anticheat"
\> usermode mem read/write s..."]
I can read and write because I'm using DBVM.

[2024-10-31 03:09] Domimmo314: Hi, I'm starting using ghidra for the first time to rev-eng a small mobile game. I'm bumping into what it looks like decompilation over-complications. I know from external observations that this snippet reads x y coords of a character and draws a cloud of smoke around them (saving its x y elsewhere). I also know that x y, both the character read and smoke written ones are adjacent in memory.
Indeed looking at the listing you can see that both pairs are 4 bytes apart. Why does the decompilation get so messy treating x and y differently? Is there something I can tweak to yield a more plain interpretation?
[Attachments: image.png]

[2024-10-31 03:10] Domimmo314: Or is this just something I have to live with? 😅

[2024-10-31 12:18] Deleted User: Not sure if I should be asking here or in <#835656787154960384> so sorry if wrong category.

Before I start this is not for anything AC or game related.

I am looking into things malware research related and got interested in windows stuff for detecting anti debug stuff such as isdebuggerpresent.

The PEB on a process has the containing bytes for beingdebugged and was just wondering if there was a way to system wide spoof the bytes always to 0.

Or would I just be better looking at hooking query functions

[2024-10-31 12:56] Brit: https://github.com/x64dbg/ScyllaHide
[Embed: GitHub - x64dbg/ScyllaHide: Advanced usermode anti-anti-debugger. F...]
Advanced usermode anti-anti-debugger. Forked from https://bitbucket.org/NtQuery/scyllahide - x64dbg/ScyllaHide

[2024-10-31 12:57] Deleted User: Okii I will have a read around the source just wanted to do it without any extra dependencies

[2024-10-31 12:57] Brit: this has a list of anti debugging and anti anti debugging tricks for you to see

[2024-10-31 12:57] Brit: but specifically what you're asking abt is this

[2024-10-31 12:57] Brit: https://github.com/x64dbg/ScyllaHide/blob/baa5c8e853ace2bee752631f27fdfe5b271d92f6/PluginGeneric/Injector.cpp#L126
[Embed: ScyllaHide/PluginGeneric/Injector.cpp at baa5c8e853ace2bee752631f27...]
Advanced usermode anti-anti-debugger. Forked from https://bitbucket.org/NtQuery/scyllahide - x64dbg/ScyllaHide

[2024-10-31 12:58] Deleted User: Ah thats kinda like how I was doing it was just worried about the executable detecting the debugger before I could force it to remain 0, kinda seemed like I would be racing against it

[2024-10-31 12:59] Brit: why is the debugee not suspended

[2024-10-31 13:00] Deleted User: Because I am dumb and overcomplicated it <:facepalm:910170071257727048> I guess my thing I already made would work

[2024-10-31 13:00] Deleted User: I inject to the process read the PEB and set the isbeingdebugged to 0 but only after... oh my god

[2024-10-31 13:01] Deleted User: I do it after its detected being debugged... not my proudest moment

[2024-10-31 13:02] Brit: I do recc reading scylla, it's a pretty good set of strategies for this type of stuff

[2024-10-31 13:03] Deleted User: Will do thank you so much for helping me see the light lol.

[2024-10-31 13:18] Brit: https://github.com/x64dbg/ScyllaHide/blob/master/ConfigCollection/ScyllaHide.pdf
[Embed: ScyllaHide/ConfigCollection/ScyllaHide.pdf at master · x64dbg/Scyll...]
Advanced usermode anti-anti-debugger. Forked from https://bitbucket.org/NtQuery/scyllahide - x64dbg/ScyllaHide

[2024-10-31 13:18] Brit: read this instead of the code first

[2024-10-31 14:13] 5pider: [replying to Brit: "https://github.com/x64dbg/ScyllaHide"]
does this work with ollydbg

[2024-10-31 14:14] 5pider: or Immunity Debugger

[2024-10-31 14:14] 5pider: maybe gdb

[2024-10-31 14:19] Deleted User: [replying to 5pider: "does this work with ollydbg"]
Works with olly

[2024-10-31 14:20] Deleted User: ```
OllyDbg v1 and v2
x64dbg
Hex-Rays IDA v6 (not supported)
TitanEngine v2 (original and updated versions)
```

[2024-10-31 14:20] 5pider: i dont use olly

[2024-10-31 14:20] Deleted User: but you asked if it worked with olly

[2024-10-31 14:20] 5pider: i was just being silly

[2024-10-31 14:21] 5pider: but honestly i didnt expected it to actually work with olly whihc is cool lmao

[2024-10-31 15:28] szczcur: [replying to EfraimDays: "Is it a problem to ask this type of thing on this ..."]
why haven't you mentioned the name of the game if it's old and not some modern mmo/mp game?

[2024-10-31 15:28] szczcur: nobody can assist if we dont know what game / driver it is

[2024-10-31 15:31] Deleted User: [replying to szczcur: "why haven't you mentioned the name of the game if ..."]
idk what that is but yeah
[Attachments: image.png]

[2024-10-31 15:36] szczcur: [replying to Deleted User: "idk what that is but yeah"]
got it. muonline uses proguard.

[2024-10-31 15:36] Deleted User: sorry cleared my message incase it was an issue

[2024-10-31 15:38] szczcur: dont think it is to identify them, just not to assist with cheat development really (at least for newer mp games.. i recall someone was helped with sp game and that was ok)

[2024-10-31 15:41] szczcur: although cheating in muonline affects their microtransaction base so .. that would be problematic

[2024-10-31 15:41] Deleted User: I heavily assume he is on a private server and not retail as that game has 9999999 emulator threads

[2024-10-31 15:41] Deleted User: Still no better

[2024-10-31 15:42] szczcur: [replying to Deleted User: "I heavily assume he is on a private server and not..."]
not sure wym with emulator threads

[2024-10-31 15:43] szczcur: outside of vmp/themida i dont recall proguard doing anything crazy. could be wrong

[2024-10-31 15:43] Deleted User: certain emulation based forums that post source codes to reversed game servers with packet emulation (private servers)

[2024-10-31 15:43] szczcur: oh

[2024-10-31 15:43] szczcur: i see

[2024-10-31 15:43] Deleted User: [replying to szczcur: "outside of vmp/themida i dont recall proguard doin..."]
other than vmp xD

[2024-10-31 15:44] Deleted User: I see VMP I just run

[2024-10-31 17:55] EfraimDays: [replying to Deleted User: "I heavily assume he is on a private server and not..."]
Yep, it's a private server and it uses a proprietary anticheat driver. So, maybe it's not difficult to bypass it because it's developed by the owners of the server. I think that besides being an anticheat driver, it can easily be bypassed using simple methods, because I've already seen some threads on Cheat Engine in a loop that don't look related to the game. However, I've only been studying reverse engineering for two months, so I still don't have enough knowledge to perform a bypass. Are there any methods you can suggest for me to study and try to apply?

[2024-10-31 18:34] Ignotus: [replying to Deleted User: "other than vmp xD"]
what’s that tag u got

[2024-10-31 18:34] Deleted User: Some guild

[2024-10-31 19:47] Ignotus: how do u get one

[2024-10-31 19:48] Deleted User: Join a guild

[2024-10-31 20:33] Matti: [replying to Brit: "https://github.com/x64dbg/ScyllaHide/blob/master/C..."]
damn that is some quality writing and top notch LateX if I do say so myself

[2024-10-31 20:33] Matti: too bad we can't do version numbers

[2024-10-31 20:34] Matti: > Copyright 2014 by Aguila / cypher
this classic is also still there

[2024-10-31 20:36] Matti: oh I see I did credit myself and duncan in the PDF at least

[2024-10-31 20:36] Matti: I guess I just hate GUI programming too much to do that part too

[2024-10-31 21:11] Torph: latex my beloved

[2024-10-31 21:15] Deleted User: im confused

[2024-10-31 21:15] Deleted User: but whoever wrote it thanks 😄

[2024-10-31 21:46] Matti: I more re-wrote it than wrote it IIRC

[2024-10-31 21:47] Matti: well and the latex, I wrote that probably

[2024-10-31 22:55] dullard: [replying to Matti: "damn that is some quality writing and top notch La..."]
<:yea:904521533727342632>
[Attachments: image.png]

[2024-10-31 22:56] dullard: the page margins are massive and images / text overflowing into the right margin makes my eye twitch

[2024-10-31 22:59] Matti: well it's a page with a screenshot of ollydbg

[2024-10-31 22:59] Matti: of course I was gonna make it look like shit on purpose

[2024-10-31 23:04] Deleted User: **purpose**