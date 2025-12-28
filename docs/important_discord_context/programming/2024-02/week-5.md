# February 2024 - Week 5
# Channel: #programming
# Messages: 459

[2024-02-26 07:17] Matti: what do you mean by 'rw breakpoint'? I'm guessing a HW bp, yes?

[2024-02-26 07:17] Matti: think about what a call does

[2024-02-26 07:17] Matti: it executes the code at [rip+0x10]

[2024-02-26 07:18] Matti: so you'll want an execute bp for that, not read or write

[2024-02-26 07:20] Matti: maybe you're expecting some bytes at [rip+0x10] to be read here?

[2024-02-26 07:21] Matti: it does jump to there, but there is no need to read the bytes at this address to do that

[2024-02-26 07:22] Matti: since rip is known at the time the instruction executes, and 0x10 is obviously just added to that to obtain the destination

[2024-02-26 07:24] Matti: hmm, well that depends on the compiler and linker

[2024-02-26 07:25] Matti: there are a few different versions of 'calling an export' that I've seen MSVC use just in the windows kernel

[2024-02-26 07:25] Matti: one sec

[2024-02-26 07:27] Matti: just copying a comment of mine from somewhere....
```c
// Check if this is
// 'call IMM:CiInitialize thunk'        // E8 ?? ?? ?? ??            // Windows Vista/7
// or
// 'jmp qword ptr ds:[CiInitialize IAT RVA]'    // 48 FF 25 ?? ?? ?? ??        // Windows 8 through 10.0.15063.0
// or
// 'call qword ptr ds:[CiInitialize IAT RVA]'    // FF 15 ?? ?? ?? ??        // Windows 10.0.16299.0+
```

[2024-02-26 07:27] Matti: where CiInitialize is an export in CI.dll

[2024-02-26 07:28] Matti: I thiiiink these are all rip relative actually?

[2024-02-26 07:28] Matti: hm lemme check in zydis

[2024-02-26 07:29] Matti: they'd have to be I guess

[2024-02-26 07:29] Matti: ya

[2024-02-26 07:29] Matti: 
[Attachments: image.png]

[2024-02-26 07:30] Matti: so the 7 here is added as the instruction's own length

[2024-02-26 07:30] Matti: to make rip

[2024-02-26 07:31] Matti: hm well, zydis *does* say there's an R from memory at rip+0

[2024-02-26 07:32] Matti: this is your ^

[2024-02-26 07:32] Matti: it's a call

[2024-02-26 07:32] Matti: but yeah

[2024-02-26 07:35] Matti: I just tried in x64dbg to make sure, but no it doesn't trigger for me

[2024-02-26 07:35] Matti: using either HW access, or memory read BPs

[2024-02-26 07:36] Matti: that is what I expected to happen, but I have to say the zydis output is confusing me now <:lillullmoa:475778601141403648>

[2024-02-26 07:36] Matti: based on *that* I'd say it would be hit

[2024-02-26 07:40] Matti: well the cpu doesn't need to 'read' the bytes at some address in order to execute them

[2024-02-26 07:40] Matti: it's not like a human reading a book

[2024-02-26 07:40] Matti: R and X are different operations

[2024-02-26 07:41] Matti: that's why you can have execute-only page protections, for example

[2024-02-26 07:42] Matti: yeah, at some point it will obviously need to know what bytes to decode and then execute

[2024-02-26 07:42] Matti: but at the assembly level this is different from a read such as with mov

[2024-02-26 07:43] Matti: I'm not great on CPU design internals, so someone else can probably give a more in depth explanation of the difference

[2024-02-26 07:44] Matti: but basically the CPU makes it possible for the programmer to distinguish between R and X

[2024-02-26 07:46] Matti: indistinguishable to whom? they definitely are different operations

[2024-02-26 07:47] Matti: again that's why it's possible to make a page that can be executed but not read from

[2024-02-26 07:48] Matti: not wrong, as you can see the zydis output above has me scratching my head a bit

[2024-02-26 07:49] Matti: any example that comes to mind where you really wouldn't know which is which?

[2024-02-26 07:50] Matti: for me it's kinda intuitive that a call is X and a mov is R/W, I guess

[2024-02-26 07:50] Matti: you can have all three as well though, with self modifying code

[2024-02-26 07:51] Matti: not sure if there's a single instruction that would be RWX all in one though

[2024-02-26 07:51] Matti: nah

[2024-02-26 07:51] Matti: the CPU only jumps to this address and continues execution there

[2024-02-26 07:52] Matti: by this logic any instruction followiing the current one would be a read, I'd say

[2024-02-26 07:56] Matti: I'm not sure what restrictions are giving you R/W breakpoints, but not X ones?

[2024-02-26 07:56] Matti: oh, read your edit

[2024-02-26 07:56] Matti: well that's like, unfortunate

[2024-02-26 07:56] Matti: but that's how it works

[2024-02-26 07:59] Matti: do you want to hook all jumps to this export, or one specific jump

[2024-02-26 07:59] Matti: the former right

[2024-02-26 08:00] Matti: hmk, well you could disassemble the entire .text section and find out which jmp/call instructions end up going to the export

[2024-02-26 08:01] Matti: you can do this without needing to execute the code since the calls are all rip relative

[2024-02-26 08:01] Matti: all you need is the IAT address of the export (import for your binary)

[2024-02-26 08:10] Matti: well that can work

[2024-02-26 08:11] Matti: I was kinda assuming you wanted to do this from code

[2024-02-26 08:11] Matti: zydis makes this pretty easy

[2024-02-26 08:12] Matti: no reason you couldn't use IDA to do this, it's just not as reusable I guess

[2024-02-26 08:12] Matti: but depending on what you're doing that might not matter

[2024-02-26 08:36] Matti: sure that's understandable enough

[2024-02-26 08:36] Matti: debugging in IDA + gdb can work fine

[2024-02-26 08:36] Matti: it's the scripting part I'd be afraid of <:harold:704245193016344596>

[2024-02-26 08:37] Matti: but, whatever you're used to right

[2024-02-26 08:37] Matti: it's pretty simple tbh

[2024-02-26 08:38] Matti: like, it's really no different from user mode in any way to use

[2024-02-26 08:39] Matti: UEFI is supported, and so are NT and all popular *nix kernel variants AFAIK

[2024-02-26 08:41] Matti: if anything it might be easier to compile than for user mode, since you don't need zycore other than the headers really

[2024-02-26 08:56] Matti: hm well what are the options again? IDC or python right?

[2024-02-26 08:56] Matti: I've never used IDC, it seems so arcane that only someone like jonas would use it (and he does)

[2024-02-26 08:57] Matti: and python is just a poor programming language that I have a lot of hate for

[2024-02-26 08:57] luci4: [replying to Matti: "and python is just a poor programming language tha..."]
Why?

[2024-02-26 08:58] Timmy: IDC isn't the C api right?  its like weird scripting thing?

[2024-02-26 08:58] Matti: mm, I don't really wanna do the whole programming language wars thing all over again today, it wouldn't be the first time this week and I kinda wanna go to bed

[2024-02-26 08:58] Matti: but basically: it's not statically typed, and nothing works in it

[2024-02-26 08:58] Timmy: cuz the c api seems gud

[2024-02-26 08:58] Matti: [replying to Timmy: "IDC isn't the C api right?  its like weird scripti..."]
correct

[2024-02-26 08:59] luci4: [replying to Matti: "but basically: it's not statically typed, and noth..."]
Fair enough

[2024-02-26 08:59] Matti: [replying to Timmy: "cuz the c api seems gud"]
yeah, but that is only for plugins, right?

[2024-02-26 08:59] Matti: at that point you might as well use zydis

[2024-02-26 08:59] Timmy: I mean thats kinda what I was using Python for already

[2024-02-26 09:00] Timmy: except they keep breaking my pythong code

[2024-02-26 09:00] Timmy: every. single.  update

[2024-02-26 09:00] Matti: well tbf I am saying to use zydis

[2024-02-26 09:00] Matti: but it seems even more the obvious choice if you're also already going to be using C anyway

[2024-02-26 09:01] Timmy: right,  except isnt the same thing to use whatsoever

[2024-02-26 09:01] Matti: true true

[2024-02-26 09:01] Matti: they do different things

[2024-02-26 09:01] Matti: but different in a way that makes it hard to convince me there's any benefit to doing it in IDA <:harold:704245193016344596>

[2024-02-26 09:02] Matti: I don't feel strongly about this though, don't get me wrong

[2024-02-26 09:02] Matti: it'll work fine

[2024-02-26 09:02] Matti: I just don't really see a point

[2024-02-26 09:03] Timmy: well first I built plugins that I want to use in IDA, like copying the current in memory rva of the current line in IDA view

[2024-02-26 09:03] Timmy: irrespective of starting the disassembly from file or out of memory dump

[2024-02-26 09:05] Timmy: and then I added pattern scanning using ida xrefs, like strings for example

[2024-02-26 09:06] Matti: I will say, finding xrefs to anything is going to be far easier in IDA

[2024-02-26 09:06] Matti: [replying to Timmy: "irrespective of starting the disassembly from file..."]
but as far as this part goes, you could literally use zydis in your IDA plugin to do the same if you wanted to

[2024-02-26 09:08] Matti: since xrefs aren't needed here, only an IAT address and resolving call targets to it.... well I'd go for zydis for sure since it removes the dependency on IDA

[2024-02-26 09:12] Timmy: I think we miscommunicated. I'm saying I want the current view RVA to clipboard feature inside the IDA GUI.

[2024-02-26 09:14] Timmy: removing a dependency on IDA in this case feels like ordering coffee without water.

[2024-02-26 09:15] Matti: ok, we definitely did then cause I was talking about OP's problem

[2024-02-26 09:15] Matti: sure, your use case does kind of need IDA lol

[2024-02-26 09:16] Matti: but still - you could implement the actual 'what is the RVA' function using zydis! if you *really* wanted to....

[2024-02-26 09:16] asz: 
[Attachments: message.txt.c]

[2024-02-26 09:16] asz: its fine

[2024-02-26 09:17] Matti: ah jonas

[2024-02-26 09:17] Matti: just in time

[2024-02-26 09:17] Timmy: https://tenor.com/view/april-fools-joke-dog-its-fine-this-is-not-gif-16757454

[2024-02-26 09:18] Timmy: [replying to Matti: "just in time"]
he was waiting for the coffee to get hot...

[2024-02-26 09:18] asz: i use it to apply func sigs and import types

[2024-02-26 09:19] asz: who cares how to wrap a statement

[2024-02-26 09:21] Matti: this does look much easier than idaclang!

[2024-02-26 09:21] Matti: thank you jonas

[2024-02-26 09:21] Matti: well

[2024-02-26 09:22] Matti: the `apply_type("name_of_til")` part is fine

[2024-02-26 09:22] Matti: if that was the only part

[2024-02-26 09:23] Matti: but I think I prefer having the types in TILfiles generated by idaclang <:thinknow:475800595110821888>

[2024-02-26 09:29] asz: yah- it have advantages defintly

[2024-02-26 09:50] 0xatul: [replying to Matti: "but I think I prefer having the types in TILfiles ..."]
have you been able to generate a TIL for phnt single header :Kappa:

[2024-02-26 09:50] 0xatul: it has been more painful than its supposed to be

[2024-02-26 14:33] Matti: uh, well not *that*, I guess

[2024-02-26 14:34] Matti: but I have been able to make a TIL for the windows SDK and DDK + the WRK sources

[2024-02-26 14:34] Matti: just by feeding it a header that includes all of the other ones

[2024-02-26 14:34] Matti: I also made one for the IDA SDK... quite useful for cracking IDA

[2024-02-26 14:35] Matti: https://hex-rays.com/tutorials/idaclang/ here
[Embed: Using the IDAClang plugin for IDA Pro]
A powerful disassembler and a versatile debugger

[2024-02-26 14:35] Matti: it's real easy

[2024-02-26 14:37] Matti: I always used to do this, FYI, but previously it required tilib to do this

[2024-02-26 14:38] Matti: that **was** a fucking nightmare

[2024-02-26 15:21] 0xatul: [replying to Matti: "just by feeding it a header that includes all of t..."]
well that should technically work for phnt single header no?

[2024-02-26 15:21] 0xatul: but it doesnt :pain:

[2024-02-26 15:24] 0xatul: 
[Attachments: image.png]

[2024-02-26 15:24] 0xatul: I take that back my friend

[2024-02-26 15:24] 0xatul: I respectfully take that back

[2024-02-26 15:24] 0xatul: I really really take that back man

[2024-02-26 15:24] 0xatul: thanks

[2024-02-26 15:24] Matti: good thinking

[2024-02-26 15:24] Matti: and well done

[2024-02-26 15:24] 0xatul: holy fucking shit

[2024-02-26 15:24] 0xatul: wow

[2024-02-26 15:24] 0xatul: <@162611465130475520> ^

[2024-02-26 16:25] mrexodia: That’s not a TIL though?

[2024-02-26 16:32] 0xatul: yeah but I can emit a TIL with idaclang

[2024-02-26 16:32] 0xatul: wait

[2024-02-26 16:32] 0xatul: I will figure this shit out soon

[2024-02-26 16:33] 0xatul: <@148095953742725120> can you pass the cmdline flag you used to gen a TIL using idaclang?

[2024-02-26 16:35] Matti: I can but not right now, not at my PC

[2024-02-26 16:35] 0xatul: sure whenever you get time

[2024-02-26 16:47] 0xatul: ```
idaclang --idaclang-log-all --idaclang-tilname phnt.til --idaclang-tildesc "PHNT type library" -ferror-limit=50  -x c++ -target x86_64-pc-win32 phnt.h```

[2024-02-26 16:47] 0xatul: worked

[2024-02-26 19:38] Matti: [replying to 0xatul: "```
idaclang --idaclang-log-all --idaclang-tilname..."]
yep that looks pretty much like what I used for the IDA SDK

[2024-02-26 19:39] Matti: only I set `-ferror-limit=0` because I'm paranoid after having used tilib in the past

[2024-02-26 19:39] Matti: also, some defines are important for certain SDKs/libraries

[2024-02-26 19:40] Matti: e.g. for phnt.h I'd set PHNT_VERSION or whatever, and PHNT_MODE(?) to either user or kernel

[2024-02-26 19:41] Matti: most of my IDA SDK args (the whole thing is actually a batch file, but I'll spare you the rest since it's not interesting)
```batch
set CLANG_ARGV=-target x86_64-pc-win32                                    ^
            -x c++                                                        ^
            -I"C:\Program Files\IDA 8.3\idasdk83\include"                ^
            -I"C:\Program Files\IDA 8.3\plugins\hexrays_sdk\include"    ^
            -D__NT__                                                    ^
            -D__VC__                                                    ^
            -D__EA64__                                                    ^
            -DNDEBUG                                                    ^
            -DNO_TV_STREAMS                                                ^
            -DMAXSTR=1024                                                ^
            -Wno-nullability-completeness                                ^
            -ferror-limit=0
```

[2024-02-26 19:42] Matti: `__NT__`, `__VC__`, `__EA64__` are all 100% IDA specific defines that you do probably want to set

[2024-02-26 19:43] Matti: `--idaclang-log-all` is probably not useful btw.... or at least it wasn't for me

[2024-02-26 19:43] Matti: just way too much spam

[2024-02-26 19:43] Matti: I toned it down to `--idaclang-log-target --idaclang-log-warnings`

[2024-02-27 04:38] 0xatul: alrighty

[2024-02-27 04:38] 0xatul: thanks

[2024-02-27 15:35] luci4: This question might be a bit too general, but how are projects of this size usually planned out? Like, is a high-level overview of the project designed first, then "split" into components?

ex: <https://github.com/ldpreload/BlackLotus/tree/main/src>

[2024-02-27 15:41] contificate: well, you have separate entities here

[2024-02-27 15:42] contificate: the panel and the malware, they should be separate

[2024-02-27 15:42] contificate: sometimes you write different entities that would have copies of some general logic in them

[2024-02-27 15:42] contificate: factor that out to a shared/static library

[2024-02-27 15:43] contificate: if you're asking about something like this specifically

[2024-02-27 15:43] contificate: I imagine they get the base communication logic working

[2024-02-27 15:43] contificate: then build outwards

[2024-02-27 15:43] contificate: or they already have all these capabilities written

[2024-02-27 15:43] contificate: then just hook it up to a means of communication

[2024-02-27 15:45] luci4: [replying to contificate: "sometimes you write different entities that would ..."]
Well I see that it has a "shared" folder, which seems to contain general things used in multiple components

[2024-02-27 15:46] luci4: It's also set as one of the include directories, or so it seems from the .vcxitems file

[2024-02-27 15:47] luci4: [replying to contificate: "if you're asking about something like this specifi..."]
Not particularly, just general software architecture, ig

[2024-02-27 15:47] 25d6cfba-b039-4274-8472-2d2527cb: [replying to luci4: "This question might be a bit too general, but how ..."]
Well you can usually tell from experience the bare minimum of separate components you will need.

[2024-02-27 15:47] contificate: yeah, you generally just factor things out when you believe it to be useful

[2024-02-27 15:47] contificate: it's similar to introducing a template in C++

[2024-02-27 15:47] 25d6cfba-b039-4274-8472-2d2527cb: Or you might start with quick and dirty prototyping to get something that works as a base, then after you get a hang of whatever you are doing you refactor it to logical parts

[2024-02-27 15:47] contificate: you could do it top-down, but most do it bottom-up

[2024-02-27 15:48] Brit: I don't think you do these kinds of things top down

[2024-02-27 15:48] Brit: imo anyway

[2024-02-27 15:48] contificate: depends what you consider top-down and bottom-up for this

[2024-02-27 15:48] contificate: this seems pretty basic

[2024-02-27 15:48] contificate: I'd do it the way I suggested

[2024-02-27 15:48] contificate: get the communication working then just build out features

[2024-02-27 15:49] contificate: I think malware I've seen tends to be some basic shit paired with something juicy

[2024-02-27 15:49] contificate: the something juicy is developed separately

[2024-02-27 15:50] Brit: that's down to the nature of low skill high incentive nature of malware

[2024-02-27 15:50] contificate: this code seems mostly well written

[2024-02-27 15:51] Brit: malware in general though

[2024-02-27 15:51] contificate: some redundancy

[2024-02-27 15:51] Brit: on average is awful

[2024-02-27 15:52] 25d6cfba-b039-4274-8472-2d2527cb: I stopped reading after reading 5 lines of the naming convention

[2024-02-27 15:52] 25d6cfba-b039-4274-8472-2d2527cb: SAD

[2024-02-27 15:52] contificate: tfw most malware seems to be less complex than writing a torrent client

[2024-02-27 15:52] contificate: excluding any involved Windows lore required for some capability

[2024-02-27 15:52] contificate: the actual communication, protocol, panel, etc.

[2024-02-27 15:52] luci4: [replying to 25d6cfba-b039-4274-8472-2d2527cb: "I stopped reading after reading 5 lines of the nam..."]
AFAIK BlackLotus first appeared after it was sold on some shady forum thing

[2024-02-27 15:52] contificate: is all just standard basic nonsense

[2024-02-27 15:53] luci4: But it's been a while since I read the paper on it

[2024-02-27 15:53] 25d6cfba-b039-4274-8472-2d2527cb: I mean the code

[2024-02-27 15:53] 25d6cfba-b039-4274-8472-2d2527cb: not the project name

[2024-02-27 15:56] Brit: idk this doesn't seem all that insane as a project

[2024-02-27 15:56] contificate: dislike the general code style of initialising a bunch of variables at the top of functions in C

[2024-02-27 15:56] contificate: is alright in small doses like here

[2024-02-27 15:56] contificate: but is otherwise an idiotic practice

[2024-02-27 15:56] 25d6cfba-b039-4274-8472-2d2527cb: I looked at Shared/crypto.c to learn top end cryptography.

[2024-02-27 15:57] Brit: [replying to contificate: "dislike the general code style of initialising a b..."]
I feel like it's nearly always cancer

[2024-02-27 15:57] Brit: declare at first use is much more sane

[2024-02-27 15:57] Brit: but that's me

[2024-02-27 15:57] luci4: [replying to contificate: "dislike the general code style of initialising a b..."]
I usually initialize them when I need to use them, is that better?

[2024-02-27 15:57] contificate: yeah, and that's how the compiler cares about it

[2024-02-27 15:57] contificate: I'd say so

[2024-02-27 15:57] contificate: some disagree because they're boomers who wrote Pascal before

[2024-02-27 15:58] contificate: where you declare all variables in `var` sections before the function body

[2024-02-27 15:58] 25d6cfba-b039-4274-8472-2d2527cb: I mean it used to be a part of C standard too

[2024-02-27 15:58] 25d6cfba-b039-4274-8472-2d2527cb: don't remember when they got rid of it tho

[2024-02-27 15:58] contificate: yeah but it's dead now

[2024-02-27 15:58] contificate: as of decades ago

[2024-02-27 15:58] contificate: you want lexical scope to contain that is relevant

[2024-02-27 15:58] 25d6cfba-b039-4274-8472-2d2527cb: well its MSVC so maybe it was relevant still when this malware was written.

[2024-02-27 15:58] 25d6cfba-b039-4274-8472-2d2527cb: <:apu:436527914369024020>

[2024-02-27 15:58] contificate: you also don't want to miss chances to use `const`

[2024-02-27 15:58] contificate: because you decided a default value to init something that need not be already in scope

[2024-02-27 15:58] contificate: was better than just introducing the binding later

[2024-02-27 15:59] luci4: What's wrong with MSVC?

[2024-02-27 15:59] contificate: used to be shit

[2024-02-27 15:59] contificate: think they hired the right people and now it's decent

[2024-02-27 15:59] 冰: hmm

[2024-02-27 16:00] Brit: I'm pretty sure some of that efi code is yoinked from matti anyway

[2024-02-27 16:00] Brit: yet another paste job by the malware lads

[2024-02-27 16:00] Brit: <:mmmm:904523247205351454>

[2024-02-27 16:00] Deleted User: https://tenor.com/view/cejm-cavalcade-rodifa-juhulati-gif-21763328

[2024-02-27 16:01] 25d6cfba-b039-4274-8472-2d2527cb: pretty sure MSVC still doesn't support C99 fully (probably doesn't plan to either)

[2024-02-27 16:01] contificate: would explain why they write C++ as though it's C

[2024-02-27 16:01] contificate: and then go on Discord and tell beginners to cast the result of `malloc`

[2024-02-27 16:03] Brit: how will the people know the type of the buff 🤡

[2024-02-27 16:41] Matti: [replying to contificate: "think they hired the right people and now it's dec..."]
I've heard the first part too

[2024-02-27 16:41] Matti: can confirm the second part is false

[2024-02-27 16:44] Matti: its standards compliance is improving, but only for C++ - C is still a joke and they've confirmed that that is just how they treat C

[2024-02-27 16:44] Matti: but mostly, codegen is shockingly bad compared to llvm

[2024-02-27 16:46] Matti: its LTO is poor and its auto vectorizer might as well not exist

[2024-02-27 16:46] contificate: never began

[2024-02-27 16:46] luci4: Would you say mingw is better?

[2024-02-27 16:46] contificate: I trust we all use Clang on Windows

[2024-02-27 16:48] Matti: [replying to luci4: "Would you say mingw is better?"]
better than what? mingw isn't completely comparable IMO, since it also comes with a minimal unix-like environment

[2024-02-27 16:48] 25d6cfba-b039-4274-8472-2d2527cb: we dont code on windows

[2024-02-27 16:48] Matti: but you can use clang both ways on windows

[2024-02-27 16:48] Matti: [replying to 25d6cfba-b039-4274-8472-2d2527cb: "we dont code on windows"]
> we

[2024-02-27 16:49] luci4: [replying to Matti: "better than what? mingw isn't completely comparabl..."]
Yeah, fair enough, nvm then

[2024-02-27 16:49] Matti: well I mean, it's not unreasonable to compare them, it depends on what you need I guess

[2024-02-27 16:50] Matti: just saying they're not really the same thing

[2024-02-27 16:50] Matti: I use llvm about 50/50

[2024-02-27 16:50] Matti: meaning native or mingw

[2024-02-27 16:50] Matti: I mostly use mingw to compile stuff that's already got a PKGBUILD set up in MSYS2

[2024-02-27 16:50] Matti: it's super convenient

[2024-02-27 16:51] Matti: also, with mingw you can choose to use GCC or clang and it works pretty much transparently

[2024-02-27 16:51] luci4: Do you ever use Visual Studio?

[2024-02-27 16:52] Matti: yea

[2024-02-27 16:52] Matti: lol

[2024-02-27 16:52] Matti: I have 5 instances open right now

[2024-02-27 16:52] Matti: I do my coding in VS

[2024-02-27 16:52] Matti: but its default compiler isn't great

[2024-02-27 16:53] luci4: [replying to Matti: "but its default compiler isn't great"]
Im guilty of using it, will have to switch it out for LLVM

[2024-02-27 16:54] Matti: you can't really avoid MSVC on windows anyway

[2024-02-27 16:54] Matti: especially when compiling other people's projects

[2024-02-27 16:54] Matti: but personally I use llvm for my own projects when possible

[2024-02-27 16:55] Matti: and then I just set up a second build config to verify it isn't broken in MSVC

[2024-02-27 16:56] luci4: I recently got started with driver development, so haven't tweaked the settings much, yet

[2024-02-27 16:56] Matti: oh <:kekw:904522300257345566>

[2024-02-27 16:56] Matti: there's one thing not to use LLVM for on windows

[2024-02-27 16:57] daax: [replying to Matti: "there's one thing not to use LLVM for on windows"]
drivers?

[2024-02-27 16:57] Matti: yeaaa

[2024-02-27 16:57] daax: I gotta strong disagree with that one

[2024-02-27 16:57] Matti: unless you absolutely don't need SEH at all

[2024-02-27 16:58] daax: yeah alright fair enough, just find non faulting gadgets for probing <:Kappa:794707301436358686>

[2024-02-27 16:58] Matti: well I mean, yeah that works

[2024-02-27 16:59] Matti: but SEH is so widely used in NT drivers that even if you don't run into problems with your own code, someone else's code will eventually make sure you will

[2024-02-27 16:59] Matti: the fact that
```c
__try {
  *(int*)0 = 0;
} __except(1) {
}
```
BSODs is just embarrassing

[2024-02-27 17:00] luci4: [replying to Matti: "there's one thing not to use LLVM for on windows"]
I had a feeling that would be the case...

[2024-02-27 17:01] Matti: you can work around this using lambdas in C++, by the way

[2024-02-27 17:01] daax: [replying to Matti: "but SEH is so widely used in NT drivers that even ..."]
Eh, true, but I’ve yet to encounter an issue that can’t be resolved as long as you roll a lot of your own required components- though there’s plenty of problems like the example you just gave that could happen… except, if you’re blindly dereferencing an object without checking if it’s valid… kind of a skill issue? (I know that’s just an example)

[2024-02-27 17:01] Matti: some people do this

[2024-02-27 17:02] Matti: I don't really find it acceptable to have to do it

[2024-02-27 17:02] Matti: [replying to daax: "Eh, true, but I’ve yet to encounter an issue that ..."]
yeah I mean the example code is dumb

[2024-02-27 17:02] Matti: but this is how ProbeForWrite() works under the hood

[2024-02-27 17:03] Matti: and plenty of things can expectedly fault

[2024-02-27 17:03] Matti: in some circumstances

[2024-02-27 17:05] daax: Yeah, I mean for general driver dev using LLVM is unnecessary and won’t work well like you said. Depends what you’re doing for sure, most of the stuff I’ve been doing requires reinventing the wheel to an extent which is time consuming but allows a lot of flexibility since I can use llvm

[2024-02-27 17:06] Matti: don't get me wrong, I'm also working on porting matti WRK to LLVM <:Kappa:794707301436358686>

[2024-02-27 17:06] Matti: but yeah like you say, for general driver dev I would definitely advise against it on NT

[2024-02-27 17:15] Matti: actually, there's a second argument for adding a clang toolchain I'm forgetting

[2024-02-27 17:15] Matti: better diagnostics and warnings

[2024-02-27 17:16] Matti: you can still compile the driver to actually run with MSVC

[2024-02-27 17:16] Matti: but clang's diagnostics are generally just better

[2024-02-27 17:17] Matti: there's the static driver verifier from MS but they don't really do the same thing

[2024-02-27 17:18] Matti: I'd say they complement each other

[2024-02-27 17:38] Timmy: [replying to Matti: "the fact that
```c
__try {
  *(int*)0 = 0;
} __exc..."]
that's documented tho, they say in the documentation it only works for calls inside that scope

[2024-02-27 17:39] Matti: I know that <:thinknow:475800595110821888>

[2024-02-27 17:39] Matti: doesn't make it not terrible

[2024-02-27 17:42] Matti: I understand documenting something as a known issue

[2024-02-27 17:42] Matti: but SEH has been broken in LLVM for so long that I no longer think there is any sort of initiative underway to fix it, ever

[2024-02-27 17:46] Timmy: yeah fair.

[2024-02-27 17:46] Timmy: I think this project here has some patches for seh. but no idea if it makes a tangible difference

[2024-02-27 17:47] Timmy: https://github.com/backengineering/llvm-msvc
[Embed: GitHub - backengineering/llvm-msvc: [WIP] A forked version of LLVM ...]
[WIP] A forked version of LLVM that prioritizes MSVC compatibility. This version is tailored for Windows users. - backengineering/llvm-msvc

[2024-02-27 17:49] Matti: mmm... it basically seems to be maintained by one guy?  <:harold:704245193016344596>

[2024-02-27 17:51] Matti: I'd be willing to give it a try (e.g. to check if my example above works) if it wasn't going to be such a PITA to set this up

[2024-02-27 17:51] Matti: I'm skeptical, but idk

[2024-02-27 19:22] daax: [replying to Timmy: "https://github.com/backengineering/llvm-msvc"]
no thanks

[2024-02-27 19:22] daax: would not recommend for drivers + llvm

[2024-02-27 19:24] Timmy: you wouldnt recommend llvm at all for windows krnl?

[2024-02-27 20:34] [Janna]: [replying to Timmy: "https://github.com/backengineering/llvm-msvc"]
wait- isn't that the Penrose triangle as the icon?

[2024-02-27 20:35] [Janna]: - `https://en.wikipedia.org/wiki/Penrose_triangle` <- it's like a puzzle or something

**here we have this as well** which.. just breaks my vision O.O - "impossible staircase" - 
any direction u go in will lead to infinity
[Attachments: image.png]

[2024-02-27 20:40] [Janna]: this actually goes up
[Attachments: image.png]

[2024-02-27 20:40] [Janna]: 🤦‍♂️ just, impossible alright ill stop spam here xD

[2024-02-27 20:59] dullard: ok

[2024-02-27 21:10] duk: [replying to Matti: "but SEH is so widely used in NT drivers that even ..."]
-fasync-exceptions

[2024-02-27 21:10] duk: they finally fixed this crap

[2024-02-27 21:11] duk: [replying to Timmy: "https://github.com/backengineering/llvm-msvc"]
lol this was probably the worst possible time for this project, MS is starting to go all-in on clang/llvm

[2024-02-27 21:11] Matti: you're joking

[2024-02-27 21:11] duk: [replying to Matti: "you're joking"]
:)

[2024-02-27 21:11] Matti: wait wait ok I'll go try it out

[2024-02-27 21:11] Matti: I don't believe you

[2024-02-27 21:11] duk: look they even have a warning

[2024-02-27 21:12] duk: for your example

[2024-02-27 21:12] duk: 
[Attachments: image.png]

[2024-02-27 21:12] Matti: <:kekw:904522300257345566>

[2024-02-27 21:12] duk: https://godbolt.org/z/d6To4Kf1j
[Embed: Compiler Explorer - C++ (x86-64 clang (assertions trunk))]
int test() {
    __try {
  *(volatile int*)0 = 0;
} __except(1) {
}
return 4;
}

[2024-02-27 21:12] Matti: true it was a shitty example for different reasons too

[2024-02-27 21:12] duk: no but seriously they actually fixed it

[2024-02-27 21:12] Matti: damn

[2024-02-27 21:12] Matti: I can't believe it

[2024-02-27 21:12] duk: I KNOW RIGHT

[2024-02-27 21:12] Matti: a fix 10 years in the making?

[2024-02-27 21:13] Matti: maybe more

[2024-02-27 21:13] duk: yeah except

[2024-02-27 21:13] duk: they fixed it in the funniest way possible

[2024-02-27 21:13] Matti: except

[2024-02-27 21:13] Matti: geddit

[2024-02-27 21:13] duk: wow

[2024-02-27 21:13] duk: ok so basically there's some intrinsics and they turn literally every store in between those intrinsics into volatile stores

[2024-02-27 21:14] Matti: hmmm ic

[2024-02-27 21:14] duk: which.. works

[2024-02-27 21:14] duk: so hey

[2024-02-27 21:14] duk: fuck it

[2024-02-27 21:14] Matti: yeah

[2024-02-27 21:14] Matti: it's not the worst thing in the world

[2024-02-27 21:14] duk: + msvc generates literal garbage in SEH anyway

[2024-02-27 21:14] duk: so

[2024-02-27 21:14] Matti: you generally don't wrap 1000 lines in try/except

[2024-02-27 21:14] duk: yeah

[2024-02-27 21:17] Matti: lmao

[2024-02-27 21:18] Matti: I put your godbolt example in my local clang

[2024-02-27 21:18] Matti: it's still compiling

[2024-02-27 21:18] duk: LOL

[2024-02-27 21:18] duk: what

[2024-02-27 21:18] Matti: using 100% of 1 core

[2024-02-27 21:18] duk: uh what version do you have

[2024-02-27 21:18] duk: if it's <17 you're probably fucked

[2024-02-27 21:18] Matti: ```
Matti clang version 18.0.0 (https://github.com/Mattiwatti/llvm-project.git 386825c6a6475c7306e99090d030156f61080046)
Target: x86_64-pc-windows-msvc
Thread model: posix
InstalledDir: C:\Program Files\LLVM\bin
__DATE__ __TIME__ of compilation: Dec 13 2023 00:47:56
```

[2024-02-27 21:18] duk: ok yeah should be fine

[2024-02-27 21:18] duk: that's wack

[2024-02-27 21:19] Matti: 
[Attachments: image.png]

[2024-02-27 21:19] Matti: this is a debug build FWIW, or it should be I think

[2024-02-27 21:20] Matti: just with the switch added

[2024-02-27 21:21] duk: lmao selectiondag moment

[2024-02-27 21:21] Matti: ok maybe it's secretly doing `-O3` regardless because my msbuild .props file is shit

[2024-02-27 21:21] Matti: either way

[2024-02-27 21:21] Matti: it's just your function

[2024-02-27 21:21] duk: yeah uh

[2024-02-27 21:21] duk: couldn't tell you

[2024-02-27 21:22] duk: <:kekw:904522300257345566>

[2024-02-27 21:22] Matti: I'll just go updoot it

[2024-02-27 21:22] Matti: 50% chance it works

[2024-02-27 21:22] Matti: like always really

[2024-02-27 21:23] duk: true

[2024-02-27 21:23] duk: either works or it doesn't

[2024-02-27 21:23] Matti: that too

[2024-02-27 21:23] Matti: but also, the chance of it working is about 50% in my experience

[2024-02-27 21:24] Matti: it's things like these that kill you
[Attachments: image.png]

[2024-02-27 21:24] Matti: ✅  = may work
[Attachments: image.png]

[2024-02-27 21:25] duk: real

[2024-02-27 21:29] Matti: here we go
[Attachments: image.png]

[2024-02-27 21:31] duk: russian roulette time

[2024-02-27 21:32] Matti: aaaahh fakk

[2024-02-27 21:33] Matti: first I need to fix my `/DESTROY` switch in the lld-link driver

[2024-02-27 21:33] Matti: they fucked with section merging stuff somewhere

[2024-02-27 21:33] Matti: so /MERGE I guess

[2024-02-27 21:34] Matti: /DESTROY is like that but with no destination section

[2024-02-28 01:20] Matti: god fucking dammit

[2024-02-28 01:20] Matti: it failed in the *stage 2 build*??
[Attachments: image.png]

[2024-02-28 01:20] Matti: how does that happen

[2024-02-28 01:22] Matti: not like I'm gonna find the answer to that, thanks to 2000 warnings apparently being more important to print than the 1 error

[2024-02-28 01:25] Matti: time for the good ole `ninja -j1` build I guess

[2024-02-28 01:25] Matti: see ya in a few days

[2024-02-28 01:54] Matti: holy shit <@835638356624801793> it works

[2024-02-28 01:55] Matti: `-fasync-exceptions` works with the sample code I mean

[2024-02-28 01:55] Matti: [replying to Matti: "time for the good ole `ninja -j1` build I guess"]
this ended up just... working on the second try

[2024-02-28 01:56] Matti: thanks a lot ninja

[2024-02-28 02:03] Matti: it's just.... correct
[Attachments: image.png]

[2024-02-28 02:03] Matti: I'm still having trouble processing that they really fixed it

[2024-02-28 02:05] JustMagic: [replying to duk: "they finally fixed this crap"]
wait, llvm fixed seh?

[2024-02-28 02:05] Matti: I think they really did

[2024-02-28 02:10] daax: [replying to duk: "they finally fixed this crap"]
hell yes

[2024-02-28 02:10] daax: [replying to duk: "lol this was probably the worst possible time for ..."]
many years late to the party indeed

[2024-02-28 02:11] daax: [replying to duk: "which.. works"]
amazing

[2024-02-28 02:13] daax: for historical reasons

[2024-02-28 02:13] daax: on this landmark discovery

[2024-02-28 02:13] daax: thank you llvm

[2024-02-28 07:57] luci4: [replying to Matti: "I think they really did"]
So I CAN use it to make drivers w/SEH??

[2024-02-28 07:57] luci4: hell yeah

[2024-02-28 08:01] asz: your mixing up macro and function variadic arguments

[2024-02-28 08:03] asz: i prefer something like this

[2024-02-28 08:03] asz: 
[Attachments: image.png]

[2024-02-28 08:04] asz: 
[Attachments: image.png]

[2024-02-28 08:04] asz: automatic dispatch based on type of what to print

[2024-02-28 17:57] asz: its c

[2024-02-28 17:58] asz: nop

[2024-02-29 06:42] Bored engineer: how do avoid defender flagging your exe program as malware ? to be clear it is for a normal compiled program not for making anything as FUD

[2024-02-29 07:21] Random Visitor: probably the easiest approach is getting an EV certificate and signing your programs

[2024-02-29 07:22] Random Visitor: having an install base of executables using those certificates would be good as well

[2024-02-29 07:53] Bored engineer: how do you obtain one ? i tried if i upload to the virus total defender will later detect it as malware but if i make the exe again it won't

[2024-02-29 07:57] Bored engineer: [replying to Random Visitor: "having an install base of executables using those ..."]
i will check the signing process with sign tool and update you it that one works EV i think will take time

[2024-02-29 08:35] dullard: [replying to Bored engineer: "how do avoid defender flagging your exe program as..."]
Why

[2024-02-29 08:35] dullard: What does your exe contain ?

[2024-02-29 08:35] dullard: What are your goals ?

[2024-02-29 08:36] dullard: Does it get flagged statically or dynamically (at runtime)

[2024-02-29 08:38] Bored engineer: [replying to dullard: "Why"]
it's a simple python program for encryption and decryption  , it was flagged statically after i send it to one of my friend who submitted it to virus total , before that it was not flagged

[2024-02-29 08:42] Bored engineer: now i have re compiled the exe now it is not flagged but will try to sign it to avoid it

[2024-02-29 08:42] dullard: Why do you need it to be an exe ?

[2024-02-29 08:43] Bored engineer: [replying to dullard: "Why do you need it to be an exe ?"]
because it's .py file for easy distribution of it + don't want it's source code disclosure

[2024-02-29 08:43] Azalea: im getting sherlock flashbacks here

[2024-02-29 08:45] Bored engineer: [replying to Azalea: "im getting sherlock flashbacks here"]
?

[2024-02-29 08:48] Azalea: 
[Attachments: images.png]

[2024-02-29 09:50] luci4: [replying to Azalea: "im getting sherlock flashbacks here"]
Lmao, I loved that post

[2024-02-29 12:53] naci: [replying to Bored engineer: "it's a simple python program for encryption and de..."]
py2exe is flagged af, use a better language like ruby, its memory safe and approved by white house

[2024-02-29 13:32] brymko: [replying to naci: "py2exe is flagged af, use a better language like r..."]
<:muskdoubt:1124837924911992892>

[2024-02-29 13:43] naci: <:yara_lover:1148745271577157673>

[2024-02-29 13:53] Bored engineer: [replying to naci: "py2exe is flagged af, use a better language like r..."]
hmm i am using Pyinstaller sure can translate the code in ruby and run it

[2024-02-29 14:00] naci: [replying to Bored engineer: "hmm i am using Pyinstaller sure can translate the ..."]
nah that was the joke part

[2024-02-29 14:00] naci: use whatever

[2024-02-29 14:00] naci: pyinstaller, py2exe and stuff like that are usually flagged as false positive

[2024-02-29 14:01] naci: use c, cpp, rust idc

[2024-02-29 14:01] Bored engineer: but when i wrote a reverse shell in python it was statically flagged by defender even when it was not compiled into .exe the moment i pasted the code in visual code it was flagged

[2024-02-29 14:21] 25d6cfba-b039-4274-8472-2d2527cb: Does this writing of a reverse shell involve copying code from a malicious program verbatim?

[2024-02-29 14:39] naci: [replying to Bored engineer: "but when i wrote a reverse shell in python it was ..."]
first time hearing something like that

[2024-02-29 14:40] naci: just bypass the pattern recognition

[2024-02-29 14:40] naci: u can send the file to antivirus companies so they can remove the false positives

[2024-02-29 14:41] naci: but I dont know how realistic a solution is that

[2024-02-29 14:41] Bored engineer: [replying to 25d6cfba-b039-4274-8472-2d2527cb: "Does this writing of a reverse shell involve copyi..."]
i copied from https://www.revshells.com/
[Embed: Online - Reverse Shell Generator]
Online Reverse Shell generator with Local Storage functionality, URI & Base64 Encoding, MSFVenom Generator, and Raw Mode. Great for CTFs.

[2024-02-29 14:42] Bored engineer: [replying to naci: "but I dont know how realistic a solution is that"]
i can make it FUD but i was like surprised without compiling defender will kick it out , for making it fud i need to disable defender first on windows then need to make it

[2024-02-29 14:43] naci: well it will be detected shortly after you send it to virustotal anyways

[2024-02-29 14:43] Bored engineer: [replying to naci: "well it will be detected shortly after you send it..."]
yeah but why would i do that intentionally

[2024-02-29 14:59] Bored engineer: [replying to naci: "well it will be detected shortly after you send it..."]
writing fud agents and controlling with php backend will be my own C2

[2024-02-29 15:00] Bored engineer: xD

[2024-02-29 17:40] [Janna]: <@456226577798135808>  Hi! `:D`

[2024-02-29 17:40] Deleted User: hey there

[2024-02-29 18:30] dullard: [replying to Bored engineer: "i can make it FUD but i was like surprised without..."]
FUD is a myth

[2024-02-29 18:30] dullard: FUD doesn't exist

[2024-02-29 18:35] Bored engineer: [replying to dullard: "FUD doesn't exist"]
It exist in my heart

[2024-02-29 19:09] root: "for now"