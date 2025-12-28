# April 2024 - Week 1
# Channel: #reverse-engineering
# Messages: 229

[2024-04-01 17:53] mibho: whats the logic/theory behind using exception handling w/ atexit calls for control flow obfuscation

[2024-04-01 17:54] mibho: theres stuff on exception handling for control flow but not really for atexit

[2024-04-01 22:12] hxm: <@162611465130475520> hello man, do you know if it is possible to tell the linker to never use or keep some addresses zeroed or 0xCC ??

[2024-04-01 22:13] hxm: i mean in a scenario where i want to reserve some slots after linking

[2024-04-01 22:13] hxm: I cant risk just overriding the binary

[2024-04-02 02:37] sync: [replying to vendor: "<https://github.com/lief-project/LIEF>"]
Awesome resource, thank you

[2024-04-02 06:36] repnezz: What happens if I register a filter(wfp) that points to a callout and the callout driver unloads ? will the filter get removed ?

[2024-04-02 07:24] Windy Bug: honestly just read the remarks section… “This function succeeds even if there are filters in the filter engine that specify the callout for the filter's action. In this situation, filters with an action type of FWP_ACTION_CALLOUT_TERMINATING or FWP_ACTION_CALLOUT_UNKNOWN are treated as FWP_ACTION_BLOCK, and filters with an action type of FWP_ACTION_CALLOUT_INSPECTION are ignored after the callout has been deregistered from the filter engine.”

[2024-04-02 07:40] vendor: [replying to hxm: "<@162611465130475520> hello man, do you know if it..."]
spam `__debugbreak() __nop()` in a unique pattern

[2024-04-02 07:42] vendor: or define some empty byte array in a new section and have it ordered to be first

[2024-04-02 10:10] hxm: [replying to vendor: "or define some empty byte array in a new section a..."]
Thats a very ugly solution

[2024-04-02 10:10] hxm: So you telling me there is no better way to do so

[2024-04-02 10:11] hxm: ??

[2024-04-02 10:11] vendor: [replying to hxm: "So you telling me there is no better way to do so"]
well i don't quite understand what you want to do?

[2024-04-02 10:11] vendor: what section of the pe do you want space in?

[2024-04-02 10:12] vendor: and what point in time do you need to know the location of the space?

[2024-04-02 10:14] hxm: Here is the deal : i want the final executable to reserve x and y PDWORD64 empty , i dont want to do it at runtime, but on linking because of other reasons

[2024-04-02 10:14] hxm: « Tell the linker to never touch those address assuming that u know base of x section »

[2024-04-02 10:15] hxm: Thats what i want to do, not manual calculating the diff padding or wtv

[2024-04-02 10:20] vendor: [replying to hxm: "Here is the deal : i want the final executable to ..."]
i still don't understand what is meant by "x and y PDWORD64". you want 2 pointers to a 64 bit int somewhere in the data section?

[2024-04-02 10:22] hxm: i want to make 0x120020 64bit reserved i.eg

[2024-04-02 10:22] hxm: not data

[2024-04-02 10:23] vendor: so x and y are dimensions, you want a 2d array of 64 bit integers?

[2024-04-02 10:27] hxm: x and y are just examples (two addresses) that i dont know obv but since i ll disbale dynamic base and relocs normaly i ll be able to do something like *(uint64\*)0x0200000 = 0 , problem is i want to achieve same behavior without having an instruction to do it, more like asking the linker to keep this address reserved

[2024-04-02 11:10] vendor: [replying to hxm: "x and y are just examples (two addresses) that i d..."]
just declare a normal global variable? why does it need a fixed address

[2024-04-02 18:04] emulatingbe: Anyone got latest linux binary ninja binaries without the license ofc just the binaries

[2024-04-02 21:27] toasts: [replying to emulatingbe: "Anyone got latest linux binary ninja binaries with..."]
binja 4.0 literally has a free version now 😭

[2024-04-02 21:27] emulatingbe: I know, but I have binja on my windows and the uhum

[2024-04-02 21:28] emulatingbe: but I want it on linux

[2024-04-02 21:28] emulatingbe: since I'm switching os

[2024-04-02 21:28] emulatingbe: and my friend will send it prolly tomorrow ig

[2024-04-02 21:29] emulatingbe: 
[Attachments: image.png]

[2024-04-02 21:29] emulatingbe: nothing special just need the linux binaries

[2024-04-02 21:29] emulatingbe: so I can use that one

[2024-04-02 21:30] toasts: oh ur links expired ?

[2024-04-02 21:57] emulatingbe: I mean something like that

[2024-04-02 21:57] emulatingbe: Yes

[2024-04-02 21:57] emulatingbe: Lets just say that

[2024-04-02 21:57] emulatingbe: Just want the linux binaries rq haha

[2024-04-02 21:58] emulatingbe: The latest I also got latest windows version which isn't this one

[2024-04-02 21:58] froj: You can just login and get a new link iirc

[2024-04-02 21:58] emulatingbe: I can't

[2024-04-02 21:58] emulatingbe: Not at company

[2024-04-02 21:58] emulatingbe: But if no one got the linux bins here ill just wait till tomorrow

[2024-04-02 21:59] froj: https://binary.ninja/recover/
[Embed: Binary Ninja - License/Installer Recovery]
Binary Ninja is a modern reverse engineering platform with a scriptable and extensible decompiler.

[2024-04-02 21:59] froj: not going to be distributing my own version

[2024-04-02 21:59] froj: but you can get a new link here

[2024-04-02 22:00] emulatingbe: You're not disturbing your version if you download clean version of linux and make sure you're not licensed it

[2024-04-02 22:00] emulatingbe: but is fine guys

[2024-04-02 22:00] emulatingbe: ill have it tomorrow

[2024-04-02 22:00] emulatingbe: ty either way

[2024-04-02 22:00] froj: Homie if you paid for it you'd just grab a new link there and not trust some random from here to send you a binary lol

[2024-04-02 22:01] emulatingbe: [replying to froj: "Homie if you paid for it you'd just grab a new lin..."]
I do trust but forget about my question guys

[2024-04-02 22:01] emulatingbe: is fine

[2024-04-02 22:01] emulatingbe: xd

[2024-04-02 23:55] future_wizard: <@1019736768507027566> Wait, can you only install binja on  one computer?

[2024-04-03 00:10] emulatingbe: wym

[2024-04-03 00:37] froj: [replying to future_wizard: "<@1019736768507027566> Wait, can you only install ..."]
I have used it across multiple personal machines and havent had issues

[2024-04-03 04:04] emulatingbe: I can use it on any pc as well

[2024-04-03 04:04] emulatingbe: but ye

[2024-04-03 18:23] emulatingbe: 🥳

[2024-04-03 23:27] Bloombit: What is yk?

[2024-04-04 15:51] Horsie: `mr=14fee0:3:4,pc=7ff6b113102b,rax=3,rbx=442aa0,rcx=1,rdx=442aa0,rsi=0,rdi=4476b0,rsp=14fee0,rbp=0,cr3=10d41a000`

[2024-04-04 15:51] Horsie: This is the first time I've seen the stack using such a low address on windows

[2024-04-04 15:51] Horsie: > 14fee0

[2024-04-04 15:52] Horsie: Its this an anomaly or are things normally this low?

[2024-04-04 17:33] Matti: it's uncommon for sure, but I think if you try to allocate at this address yourself it will succeed without issue

[2024-04-04 17:33] Matti: only the the first 64KB is reserved

[2024-04-04 17:33] Matti: due to containing the null page

[2024-04-04 17:45] x86matthew: yeah it's normal, 0x10000 is also the default base address for exes in windows

[2024-04-04 17:45] x86matthew: if you ignore aslr and the PE-defined image base

[2024-04-04 18:23] luci4: [replying to Matti: "due to containing the null page"]
TIL

[2024-04-04 19:31] naci: I always see it around that address

[2024-04-04 20:18] Matti: yeah I missed the fact that it was the stack pointer, I thought this was a heap allocation

[2024-04-04 20:18] Matti: very normal value for rsp

[2024-04-04 20:19] Matti: though it should be possible to make a heap allocation at this VA too

[2024-04-04 20:21] Matti: [replying to luci4: "TIL"]
this was a bit too convenient of a method to turn any kernel null pointer dereference into code execution <:lillullmoa:475778601141403648> so they disabled VA allocation for the lowest range

[2024-04-04 20:23] luci4: [replying to Matti: "this was a bit too convenient of a method to turn ..."]
Oh and I was just thinking about it 😦

[2024-04-06 20:09] Horsie: Did anyone here work on Tetrane or know anyone who did?

[2024-04-06 20:56] jvoisin: [replying to Horsie: "Did anyone here work on Tetrane or know anyone who..."]
Yes

[2024-04-06 20:58] Horsie: [replying to jvoisin: "Yes"]
You did?

[2024-04-06 20:58] Horsie: I wanted to ask about how they captured their stack traces, among other questions

[2024-04-06 20:58] Horsie: But I got most of my answers

[2024-04-06 20:58] jvoisin: A good friend of mine worked there :)

[2024-04-06 20:58] Horsie: [replying to jvoisin: "A good friend of mine worked there :)"]
Ooh I see

[2024-04-06 20:58] Horsie: Very lucky!

[2024-04-06 20:59] jvoisin: Well

[2024-04-06 20:59] jvoisin: It's a tough product to sell

[2024-04-06 20:59] Horsie: Yeah.. I got that feeling when I read the news about the acquisition and the rebranding

[2024-04-06 20:59] Horsie: Hope people see the value now

[2024-04-06 20:59] jvoisin: In the end, terrane got acquired, and ~some people left, my friend included. It wasn't great

[2024-04-06 21:00] Horsie: I see. Sorry to hear that

[2024-04-06 21:00] Horsie: It looked like many people poured their hearts into it. Its a shame that such things are deemed commercially unviable

[2024-04-06 21:01] Horsie: I'm trying to make a similar tool, on a smaller/more personal scale however and pretty much completely inspired by tetrane!

[2024-04-06 21:01] Horsie: Just a a pet project though

[2024-04-06 21:01] jvoisin: Trace all the things !

[2024-04-06 21:01] Horsie: I've got a few parts working, many more to be implemented!

[2024-04-06 21:01] jvoisin: It uses sqlite internally <3

[2024-04-06 21:01] Horsie: Oooh that makes a lot of sense

[2024-04-06 21:02] Horsie: I'm tracing using pure qemu + some of my code instead of using PANDA like them. The source looked really old to me

[2024-04-06 21:02] Horsie: Tracing to ASCII however :P

[2024-04-06 21:02] Horsie: 1GB/5sec of trace

[2024-04-06 21:02] jvoisin: Yeah, they're using an old version of panda iirc

[2024-04-06 21:03] Horsie: TENET views them pretty well now. I'm working on implementing a shadow stack so I can get the call stacks at every call instr

[2024-04-06 21:03] Horsie: A lot of optimizations yet to be made

[2024-04-06 21:04] Horsie: Tenet fortunately does most of what their GUI did. Except for the callstack/strings/taint

[2024-04-06 21:04] jvoisin: Call your tool VENER, it means "angry" in French slang :D

[2024-04-06 21:04] Horsie: [replying to jvoisin: "Call your tool VENER, it means "angry" in French s..."]
Hahaha thats definitely going to be a contender now..

[2024-04-06 21:05] Horsie: I've been calling it (Horsie)ETRANE

[2024-04-06 21:05] Horsie: Or HETRANE for short

[2024-04-06 21:05] Horsie: 
[Attachments: image.png]

[2024-04-06 21:05] Horsie: Tracing a timer-triggered interrupt in nt :P

[2024-04-06 21:05] jvoisin: Yeah, their GUI was atrocious. First time I saw their product, I told my friend even r2's GUI looked better. The guy next to me added "I know, sorry about that, I'm the one who wrote it :D". Hilarity and beers ensued

[2024-04-06 21:06] Horsie: Lol!

[2024-04-06 21:06] Horsie: You can smell terrible qt from a mile away 😹

[2024-04-06 21:06] jvoisin: Yuuuuuuuuuuuuup

[2024-04-06 21:06] jvoisin: The UX was so bad.

[2024-04-06 21:06] jvoisin: (For those wondering wtf we're talking about: https://dustri.org/b/reven-workshop.html)

[2024-04-06 21:07] Horsie: I hope your friend is doing better now.

[2024-04-06 21:08] Horsie: I for one am in love with the concept of such a tool, regardless of never having used it :)

[2024-04-06 21:09] jvoisin: [replying to Horsie: "I hope your friend is doing better now."]
He's having a ton of fum at Canonical now, working remote, pretty happy :)

[2024-04-06 21:10] Horsie: [replying to jvoisin: "(For those wondering wtf we're talking about: http..."]
I love the quotes on your blog btw

[2024-04-06 21:10] Horsie: I first came across it in the doare discord

[2024-04-06 21:10] jvoisin: [replying to Horsie: "I love the quotes on your blog btw"]
I think I have around 1200 of them now :D

[2024-04-06 21:10] Horsie: Haha yeah I remember trying to cycle through them till they repeated

[2024-04-06 21:10] Horsie: I gave up

[2024-04-06 22:06] Torph: [replying to jvoisin: "(For those wondering wtf we're talking about: http..."]
woah, this is pretty cool

[2024-04-07 00:27] beerflu: the xz backdoor is even deeper

[2024-04-07 00:27] beerflu: https://twitter.com/bl4sty/status/1776691497506623562
[Embed: blasty (@bl4sty) on X]
the xz sshd backdoor rabbithole goes quite a bit deeper. I was just able to trigger some harder to reach functionality of the backdoor. there's still more to explore.. 1/n

[2024-04-07 07:24] Horsie: [replying to beerflu: "the xz backdoor is even deeper"]
How do you guys read twitter without having an account?

[2024-04-07 08:25] brymko: have an account

[2024-04-07 11:33] aaaaahhh: [replying to Horsie: "How do you guys read twitter without having an acc..."]
https://twiiit.com/
[Embed: Redirecting proxy for Nitter (alternative Twitter frontend)]
Twiiit picks a random Nitter instance so you can browse Twitter privately.

[2024-04-07 13:07] Horsie: [replying to jvoisin: "It uses sqlite internally <3"]
Do you know if they hooked up to qemu and directly logged the traces into it?

[2024-04-07 13:10] Horsie: Right now I need to replace the the horrendous `fprintf` call I'm using for every instruction callback with something a bit more sane

[2024-04-07 13:10] Horsie: 
[Attachments: image.png]

[2024-04-07 13:11] Horsie: I wonder if logging to sqlite would be (approximately) just as performant while also allowing me to do some query-based analysis on it

[2024-04-07 13:31] mrexodia: [replying to Horsie: "I wonder if logging to sqlite would be (approximat..."]
no

[2024-04-07 13:31] mrexodia: I mean, in-memory it might be alright but overall sqlite isn't performant if you're doing this type of inserts

[2024-04-07 13:32] jvoisin: You can likely batch them

[2024-04-07 13:32] jvoisin: Buffer 1M instructions, insert them in one go

[2024-04-07 13:32] jvoisin: Heck you can even do one db per 1M

[2024-04-07 13:32] jvoisin: Then merge the db afterwards

[2024-04-07 13:33] jvoisin: Keep in mind the tetrane machineries used stupidely powerful hardware

[2024-04-07 13:33] mrexodia: Yeah true, but you end up writing some kind of buffering anyway 🤷‍♂️

[2024-04-07 13:33] jvoisin: Like several TB of high-speed ssd :D

[2024-04-07 13:33] mrexodia: So might as well insert into a file and then sqlite later

[2024-04-07 13:34] jvoisin: Inserting into sqlite isn't significantly slower than writing into a file. Sure there is an overhead, but nothing crazy like 100x

[2024-04-07 13:57] Horsie: https://stackoverflow.com/questions/1711631/improve-insert-per-second-performance-of-sqlite
<@471429575919009802> <@162611465130475520> this is a good read if it interests you guys
[Embed: Improve INSERT-per-second performance of SQLite]
Optimizing SQLite is tricky. Bulk-insert performance of a C application can vary from 85 inserts per second to over 96,000 inserts per second!
Background: We are using SQLite as part of a desktop

[2024-04-07 14:00] Horsie: I'll at least give it a try to see how well on-disk ends up. working with custom formats or fprintf is just not viable in the long term for me

[2024-04-07 14:02] Horsie: I'm a bit desperate for an alternative right now

[2024-04-07 14:03] jvoisin: You can micro-optmize by writing your own printf implementation :D

[2024-04-07 14:05] Horsie: [replying to jvoisin: "You can micro-optmize by writing your own printf i..."]
Possibly but being able to run crazy fast queries on the captured traces might be nice too :P

[2024-04-07 14:05] Horsie: I'll be back to rolling my own printf if sqlite fails regardless

[2024-04-07 14:06] jvoisin: I think that they had some magic sauce to not record everything, since some instructions can be emulated after the fact, so you don't need to record them to replay the trace

[2024-04-07 14:07] Horsie: This might confirm one of my suspicions

[2024-04-07 14:08] Horsie: I was talking with a friend the other day and he suggested that they might be capturing traces/states only at translation block starts

[2024-04-07 14:09] Horsie: Though I really don't want to pull out unicorn for this right now.

[2024-04-07 14:09] mrexodia: unicorn isn't gonna work anyway <:lmao3d:611917482105765918>

[2024-04-07 14:10] Horsie: They did essentially get a lot of the analysis code for 'free' with panda

[2024-04-07 14:10] Horsie: [replying to mrexodia: "unicorn isn't gonna work anyway <:lmao3d:611917482..."]
I mean, in either case you'll have to start emulation from the starting of the trace

[2024-04-07 14:11] Horsie: Why do you say that? <@162611465130475520>

[2024-04-07 14:11] Horsie: Or is it just because you hate it lol

[2024-04-07 14:13] jvoisin: Everyone hates it

[2024-04-07 14:13] jvoisin: See https://dustri.org/b/aquynhs-conferencewares.html
[Embed: Aquynh's conferencewares]
Personal blog of Julien (jvoisin) Voisin

[2024-04-07 14:20] Horsie: Oh.. I see

[2024-04-07 14:25] mrexodia: [replying to Horsie: "Why do you say that? <@162611465130475520>"]
Because it’s ultra inaccurate

[2024-04-07 17:06] JustMagic: [replying to Horsie: "I'll be back to rolling my own printf if sqlite fa..."]
Bruh why are you even serializing it to a human readable format

[2024-04-07 17:06] JustMagic: Just dump it as raw binary

[2024-04-07 17:06] Horsie: [replying to JustMagic: "Bruh why are you even serializing it to a human re..."]
It was easiest at the time when I was prototyping shit

[2024-04-07 17:07] Horsie: Right now I have a poc that will hopefully log at ~3333333 inserts/sec to sqlite

[2024-04-07 17:08] Horsie: But I need to post-process them to human-readable anyway if I want to load them into tenet

[2024-04-07 17:08] Horsie: I'll cross that bridge when I get to it

[2024-04-07 17:09] JustMagic: [replying to Horsie: "Right now I have a poc that will hopefully log at ..."]
That still doesn't sound that great because ultimately you're not inserting that much data with each insert

[2024-04-07 17:10] JustMagic: If you're happy with the perf, it's probably good enough for the short term, but you've still got like 10x remaining space for optimizations just to saturate your SSD

[2024-04-07 17:10] Horsie: [replying to JustMagic: "If you're happy with the perf, it's probably good ..."]
Very true

[2024-04-07 17:10] Horsie: I just want something that works first

[2024-04-07 17:10] Horsie: Can opt on top of that later

[2024-04-07 17:19] Horsie: [replying to JustMagic: "That still doesn't sound that great because ultima..."]
I just realized what you meant by this..

[2024-04-07 17:20] Horsie: fprintf takes 0.06 seconds, as opposed to 0.30 seconds of sqlite

[2024-04-07 17:20] Horsie: Fuck me..

[2024-04-07 17:20] Horsie: I think its better if I just start logging binary directly

[2024-04-07 17:27] JustMagic: [replying to Horsie: "I think its better if I just start logging binary ..."]
Just get a proper async logging lib from google

[2024-04-07 17:30] JustMagic: https://github.com/odygrd/quill?tab=readme-ov-file#performance
[Embed: GitHub - odygrd/quill: Asynchronous Low Latency C++ Logging Library]
Asynchronous Low Latency C++ Logging Library. Contribute to odygrd/quill development by creating an account on GitHub.

[2024-04-07 17:42] Torph: [replying to mrexodia: "Because it’s ultra inaccurate"]
good to know, I haven't heard any opinions either way on unicorn

[2024-04-07 17:43] mrexodia: [replying to Torph: "good to know, I haven't heard any opinions either ..."]
you should use it, it's nice to mess around with

[2024-04-07 17:43] mrexodia: and it's good enough™️

[2024-04-07 17:43] mrexodia: but you shouldn't try to build anything big on top of it, because it will fall apart

[2024-04-07 17:43] Torph: good enough for what use case?

[2024-04-07 17:43] mrexodia: (I use unicorn all the time personally)

[2024-04-07 17:44] mrexodia: [replying to Torph: "good enough for what use case?"]
Quick emulation of whatever you want, write-only code projects essentially

[2024-04-07 17:44] mrexodia: but you shouldn't try to allocate 1000 memory ranges, this will take 3+ _minutes_

[2024-04-07 17:44] Torph: [replying to mrexodia: "but you shouldn't try to build anything big on top..."]
oh ok well that might be a problem... I've been researching JIT emulators past month ish and hoping to write an ARMv8 emulator

[2024-04-07 17:45] Torph: [replying to mrexodia: "but you shouldn't try to allocate 1000 memory rang..."]
???
on what hardware we talking? and ranges for the emulated system?

[2024-04-07 17:46] mrexodia: whatever desktop amd ryzen 7 I have

[2024-04-07 17:46] mrexodia: [replying to Torph: "oh ok well that might be a problem... I've been re..."]
you should just start and implement instructions as-needed for whatever you want to actually emulate

[2024-04-07 17:47] Torph: [replying to Torph: "oh ok well that might be a problem... I've been re..."]
so far I've gotten some basic decoding & most of the C interfaces needed to do an output stream of native code, iirc I got stuck trying to avoid writing my own allocator

[2024-04-07 17:47] mrexodia: just do the same as unicorn and loop over an std::vector<Region>

[2024-04-07 17:48] Torph: [replying to mrexodia: "you should just start and implement instructions a..."]
yeah, that's the plan, I implemented a couple instructions and then got stuck figuring out how I should allocate all the output buffers and mark them executable and stuff.

[2024-04-07 17:48] Torph: do you think I should bother reserving the guest address space as a continuous virtual region, or just wrap mmap/VirtualAlloc?

[2024-04-07 17:49] mrexodia: it's best to just implement the dumbest thing that works, because you will rewrite all the code 10 times anyway

[2024-04-07 17:49] Torph: that's fair

[2024-04-07 17:50] Torph: I will just wrap them then. saves me the research needed to write my own memory allocator for the reserved region 😂

[2024-04-07 17:51] mrexodia: bump allocator is fire

[2024-04-07 17:52] mrexodia: those people writing trading bots in Java knew what's up

[2024-04-07 17:52] mrexodia: just bump allocate at the start and then never allocate/deallocate again 😎

[2024-04-07 17:53] Torph: <:kekw:904522300257345566>

[2024-04-07 17:54] Torph: i had to google that but now I agree

[2024-04-07 18:11] Horsie: [replying to JustMagic: "https://github.com/odygrd/quill?tab=readme-ov-file..."]
This definitely seems like the way to go.

[2024-04-07 18:12] Horsie: As you predicted, SQLite is basically unusable for this purpose

[2024-04-07 18:12] mrexodia: not just him

[2024-04-07 18:12] mrexodia: (just saying)

[2024-04-07 18:12] Horsie: Now I just need to wrestly with the qemu build system a little bit to make it accept c++ code and libraries

[2024-04-07 18:12] Horsie: [replying to mrexodia: "not just him"]
yah... <:yea:904521533727342632>

[2024-04-07 18:16] Horsie: Though I'll have to see what I want to do about the storage issue when logging to ASCII since neither quill nor fmtlog seem to support just dumping raw bytes asynchronously

[2024-04-07 18:17] Horsie: For ~2B instructions SQLite took aroung 6.5gb which is really impressive

[2024-04-07 18:17] Horsie: However the most popular sqlite database viewer crashed when trying to open those many rows

[2024-04-07 18:17] Horsie: So.. yeah.....

[2024-04-07 18:23] jvoisin: Tetrane was dealing with sqlite files of multiples hundreds of G :D

[2024-04-07 18:24] Horsie: I got that feeling when the demo video said it needs ~120gb free <:topkek:904522829616263178>

[2024-04-07 18:25] Horsie: maybe converted them into SQL from PANDA intermediate format

[2024-04-07 18:25] Horsie: rr2, I believe

[2024-04-07 18:25] JustMagic: [replying to Horsie: "Though I'll have to see what I want to do about th..."]
`std::string_view{(char*)&struct, sizeof(struct)}` 🤡

[2024-04-07 18:26] Horsie: [replying to JustMagic: "`std::string_view{(char*)&struct, sizeof(struct)}`..."]
Yeah but I'll have to see how to make that async, if that even helps

[2024-04-07 18:26] Horsie: I dont do a lot of programming, especially with cpp

[2024-04-07 18:26] JustMagic: quill is async by default AFAIK

[2024-04-07 18:27] Horsie: [replying to JustMagic: "quill is async by default AFAIK"]
Ohh, you meant to feed that into Quill

[2024-04-07 18:27] Horsie: Yeah, that works I guess, considering converting that into string_view does not incur too much overhead

[2024-04-07 18:27] Horsie: Thank you!

[2024-04-07 18:31] JustMagic: you can ofc make a better system for dumping binary data, but this should be good enough as an interim solution