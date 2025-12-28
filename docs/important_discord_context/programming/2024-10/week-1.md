# October 2024 - Week 1
# Channel: #programming
# Messages: 50

[2024-10-02 23:25] mrexodia: [replying to brymko: "genuinely abnoxious"]
I would like to draw your attention to rust macros
[Attachments: image.png]

[2024-10-02 23:25] mrexodia: `,* $(,)?`<a:licks_FB:586019327212388383>

[2024-10-02 23:29] prick: jesus christ

[2024-10-02 23:30] Brit: this is also fine

[2024-10-02 23:32] prick: psychopath

[2024-10-02 23:36] Brit: I just don't have this brain disease where I can't conceptualize tokens for anything but their original meaning

[2024-10-03 03:23] Torph: this is confusing mostly just because I don't understand Rust syntax

[2024-10-03 05:16] brymko: [replying to mrexodia: "I would like to draw your attention to rust macros"]
wdym it's like a regex

[2024-10-03 05:17] brymko: with the change that capture groups are a little easier to read the $(xxx) part

[2024-10-03 05:18] brymko: but that's actually a nice trick to not need the last , at the end

[2024-10-03 05:19] brymko: whereass the c++ comitee has too much pride and ego to not invent their own bullshit

[2024-10-03 05:22] snowua: here comes the rust user to explain why the dog shit syntax is not actually all that dog shit 🙄

[2024-10-03 05:29] brymko: nah the syntax is dogshit

[2024-10-03 05:29] brymko: but not in this case atleast

[2024-10-03 05:29] brymko: but my metric of dogshit is probably different from yalls

[2024-10-03 06:09] snowua: fuck hes being reasonable

[2024-10-03 09:00] mrexodia: [replying to brymko: "wdym it's like a regex"]
yeah I'm obviously just clowning you

[2024-10-03 09:00] mrexodia: just saw this abomination yesterday and thought I'd share

[2024-10-03 09:04] brymko: [replying to mrexodia: "yeah I'm obviously just clowning you"]
why

[2024-10-03 09:04] mrexodia: it is fun!

[2024-10-03 12:54] daax: [replying to snowua: "fuck hes being reasonable"]
<:whyy:820544448798392330>

[2024-10-03 13:12] qw3rty01: lol if you think that’s bad, check out proc macros in rust

[2024-10-03 13:12] brymko: why

[2024-10-03 13:14] qw3rty01: with declarative macros it’s at least like generating template code, with proc macros you have to parse the token stream directly

[2024-10-03 13:14] Torph: oh there are multiple types of rust macros?

[2024-10-03 13:14] Torph: [replying to qw3rty01: "with declarative macros it’s at least like generat..."]
no way. 😭<:topkek:904522829616263178>

[2024-10-03 13:15] Torph: so you can just do literally whatever you want?

[2024-10-03 13:16] qw3rty01: well you don’t have type info, so there’s a couple limitations, but yea you can transform one piece of code into something entirely different

[2024-10-03 13:34] brymko: [replying to qw3rty01: "with declarative macros it’s at least like generat..."]
so what's that todo with the syntax

[2024-10-03 13:43] qw3rty01: nothing, just the reading vs comprehension is way worse than with declarative macros, which is the heart of what people have an issue with when talking about syntax

[2024-10-05 11:51] Windows2000Warrior: <@148095953742725120> hello , please did you know is this function can implimented in the driver code or it reference to internal global state of the kernel ? : `InterlockedPushEntrySList`

[2024-10-05 12:30] x86matthew: looking at what this function does on MSDN should make the answer fairly obvious

[2024-10-05 12:30] x86matthew: if still in doubt, open winxp ntoskrnl in disassembler or just check WRK

[2024-10-05 12:30] x86matthew: if still in doubt again, yes it can easily be reimplemented

[2024-10-05 15:23] Termolyx: [replying to Windows2000Warrior: "<@148095953742725120> hello , please did you know ..."]

[Attachments: 1728116252753-png.png]

[2024-10-05 15:35] Windows2000Warrior: [replying to x86matthew: "looking at what this function does on MSDN should ..."]
ok thanks

[2024-10-05 17:08] JustMagic: [replying to x86matthew: "looking at what this function does on MSDN should ..."]
Atomic slist functions are cursed

[2024-10-05 17:09] JustMagic: They are special cased in the kernel exception dispatcher

[2024-10-05 17:09] Brit: are you done being a furry?

[2024-10-05 17:09] Brit: was my prediction accurate

[2024-10-05 17:09] JustMagic: [replying to Brit: "are you done being a furry?"]
What

[2024-10-05 17:09] Brit: wuffs

[2024-10-05 17:10] JustMagic: Oh nah, I didn't really pain myself with that much yesterday

[2024-10-05 19:06] x86matthew: [replying to JustMagic: "They are special cased in the kernel exception dis..."]
he's just trying to implement winXP-32bit level support into win2000 (for some reason) so i think he should be ok

[2024-10-05 19:06] x86matthew: nothing special was happening back then afaik

[2024-10-05 19:06] x86matthew: not sure why you'd spend time retrofitting features to 32-bit NT 5.0 when 64-bit NT 5.2 exists though

[2024-10-05 20:20] Windows2000Warrior: [replying to x86matthew: "not sure why you'd spend time retrofitting feature..."]
Many people like/love NT 5.0 rather than 5.2 , for a many reasons like all things is classic and also for nostalgic reason , like blackwingcat there is no explanation that can convince everyone why this man spent his life creating the extended kernel for win2000 or why wildbill rewrite the full kernel32 of it.

[2024-10-06 00:57] Windows2000Warrior: i succeed to impliment  KeAcquireInterruptSpinLock , KeReleaseInterruptSpinLock , vDbgPrintExWithPrefix (no-op) , keAcquireInStackQueuedSpinLock , keReleaseInStackQueuedSpinLock , only `InterlockedPushEntrySList` should be replaced with an alternative one Existed in 2000 or this should implimented in my custom driver if this possible
[Attachments: storport.sys]

[2024-10-06 01:27] Windows2000Warrior: And also i change in fdo.c this call that not exist in 2000 ntoskrnl
[Attachments: Other_method.txt, with_IoForwardIrpSynchronously.txt]

[2024-10-06 01:35] Windows2000Warrior: I hope the replace of IoForwardIrpSynchronously with other method in the code is correct