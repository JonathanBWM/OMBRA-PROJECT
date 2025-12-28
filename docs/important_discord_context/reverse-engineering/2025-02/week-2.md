# February 2025 - Week 2
# Channel: #reverse-engineering
# Messages: 59

[2025-02-04 17:21] elias: Can someone recommend books for getting into vulnerability research (for people that already have experience with RE)?

[2025-02-05 15:09] qwerty1423: [replying to elias: "Can someone recommend books for getting into vulne..."]
not a good idea

[2025-02-05 15:09] qwerty1423: its one of the worst ways of learning it

[2025-02-05 15:16] elias: [replying to qwerty1423: "not a good idea"]
The thing I like about books is that I can get away from the PC with them

[2025-02-05 16:32] Saturnalia: [replying to elias: "Can someone recommend books for getting into vulne..."]
TAOSSA is the classic

[2025-02-07 12:25] davi: ppl that have written kernel drivers - what kind of things are you disallowed from doing from a driver?
like, can you read a sqlite db from a minifilter or is reading files in itself verbotten?

[2025-02-07 12:48] varaa: anything is possible

[2025-02-07 13:08] koyz: [replying to davi: "ppl that have written kernel drivers - what kind o..."]
CrowdStrike moment

[2025-02-07 13:54] idalen: [replying to elias: "Can someone recommend books for getting into vulne..."]
And for people with no experience in RE? Any suggestion?

[2025-02-07 15:26] Torph: [replying to davi: "ppl that have written kernel drivers - what kind o..."]
my understanding is that you can do literally anything except modify the code of the core kernel

[2025-02-07 15:37] Brit: that's a bit of an oversimplification

[2025-02-07 15:40] pinefin: [replying to Torph: "my understanding is that you can do literally anyt..."]
well this isnt true!

[2025-02-07 15:40] pinefin: patchguard? more like no match for me

[2025-02-07 15:40] pinefin: (satire)

[2025-02-07 15:40] avx: https://tenor.com/view/hammer-gif-3284201032904258120

[2025-02-07 15:49] Torph: [replying to Brit: "that's a bit of an oversimplification"]
it probably is, I've only written 1 small kernel driver

[2025-02-08 04:41] Humza: [replying to davi: "ppl that have written kernel drivers - what kind o..."]
can do practically anything, however some things are not recommended and pretty messy, for example on linux some things are better done in userland like parsing configuration files, querying databases etc

[2025-02-08 12:10] donna🤯: You should only be doing things in kernel mode when you absolutely need to. Just because its possible in kernel mode (which, essentially everything is) doesn't mean its the best option - in fact most of the time whatever it is you need to be done should be done in user mode

[2025-02-08 15:34] .: not everything is possible just because you're running in kernel mode

[2025-02-08 15:39] .: because ring 0 is not the greatest privilege level, for example, you cannot disable SMM or alter its behavior, you cannot access Intel ME or AMD PSP directly, unless there's a vulnerability

[2025-02-08 15:40] contificate: In many cases, it's just more flexible to use the kernel module to bridge the gap, rather than to incorporate tons of shit into kernel space. For a pragmatic example, consider [blktap](https://wiki.xenproject.org/wiki/Blktap) this lets you back block devices, exposed to Xen guests, with userspace software running in the control domain. So, you can use things like `nbdserver` etc. to serve guests virtual disks that are backed by making `curl` requests to a website, for example (HTTP network block device backend).
[Embed: Blktap]

[2025-02-08 17:12] daax: [replying to .: "because ring 0 is not the greatest privilege level..."]
who cares. he asked something simple, and for most people it enables everything they’ll ever have a need or desire to interact with, no need to be a pedantic dick about it.

[2025-02-08 17:20] daax: [replying to .: "because ring 0 is not the greatest privilege level..."]
Also, no shit hardware layers can override OS/FW layers or avoid modification / generic interaction — they’re separate components. You mention AMD PSP/Intel ME which are coprocessors. Nuh uh! The iDRAC isn’t modifiable from kernel! For all general intents and purposes that are relevant to his question: yes, everything that it appears he is interested in is possible in the kernel.

[2025-02-08 17:21] .: ok man..

[2025-02-08 17:21] .: its always good to be accurate and tell him the truth

[2025-02-08 17:21] .: instead of deleting my messages

[2025-02-08 17:21] daax: [replying to .: "ok man.."]
Don’t act like you’re the victim. Play pedantic dickhead get addressed like one.

[2025-02-08 17:21] daax: [replying to .: "instead of deleting my messages"]
Huh?

[2025-02-08 17:22] .: you did delete my msg before

[2025-02-08 17:22] contificate: I feel this is an overreaction, I got no sense of pedantry from it 😳

[2025-02-08 17:23] daax: [replying to contificate: "I feel this is an overreaction, I got no sense of ..."]
That’s because you’re king of pedants.

[2025-02-08 17:23] daax: [replying to .: "you did delete my msg before"]
At what point? I responded to them here

[2025-02-08 17:24] contificate: a lot of people just think they have neat little bits of information to share, it doesn't make them pedantic

[2025-02-08 17:25] .: I sent the same message a while ago, it was no longer here, so i sent it again, and you're the one complaining about it at that level (its clear that you deleted the message)

[2025-02-08 17:25] daax: [replying to contificate: "a lot of people just think they have neat little b..."]
It certainly does when there is a pattern.

[2025-02-08 17:25] contificate: I'm unaware of a pattern

[2025-02-08 17:25] contificate: so I can't speak to that

[2025-02-08 17:25] contificate: from an outsider's perspective, you are the one looking bad here

[2025-02-08 17:26] contificate: I'd prefer pedantry to whatever this is https://discord.com/channels/835610998102425650/835635446838067210/1337835405122338856

[2025-02-08 17:28] daax: [replying to contificate: "I'd prefer pedantry to whatever this is https://di..."]
You can prefer what you’d like, I’d prefer simple answers to simple questions to not be met with unnecessary *ackshaully’s*. I know many others comment on it as well when discussing the conversations that take place here.

[2025-02-08 17:29] .: and could you specify that you want simple answers before calling me a pedantic dick ?

[2025-02-08 17:29] contificate: perhaps, but I can't see it being an intentional "gotcha" or "ackshually" - sometimes people have little facts they like to embellish commentary with

[2025-02-08 17:29] contificate: I do the same, until I realise I'm going off on a tangent - a little fact here and there is benign

[2025-02-08 17:29] daax: [replying to contificate: "I do the same, until I realise I'm going off on a ..."]
Sure is

[2025-02-08 17:29] daax: Dose makes the poison.

[2025-02-08 17:30] BWA RBX: <@1009300022988374046> are you okay?

[2025-02-08 17:30] contificate: alright, well I don't interact here often enough to know, I'll stop

[2025-02-08 17:30] BWA RBX: I'm always here if you want to talk

[2025-02-08 17:31] .: no i dont need your help but ty for asking

[2025-02-08 17:33] BWA RBX: https://nicolo.dev/en/blog/disassembling-binary-linear-recursive/ interesting read
[Embed: Disassembling a binary: linear sweep and recursive traversal]
Building your own set of analysis tools is a great exercise for those who already have some basics and allows you to later move on to implement more targeted analyses in reverse engineering. Even just

[2025-02-08 18:12] szczcur: [replying to daax: "You can prefer what you’d like, I’d prefer simple ..."]
no dont u realize.. its vital that everyone know who the real men are that know hw security boundaries. just like everyone must know if youre a vegan and esp when someone suggests the kernel is the greatest.. i think the real problem is we aren’t trying to make the kernel great again. ‘reflash spi’, ring-69, arc coprocessors.. these are the real villains. i want full control from the host cpu!

[2025-02-08 18:19] sync: [replying to daax: "It certainly does when there is a pattern."]
fr

[2025-02-08 18:20] sync: [replying to .: "and could you specify that you want simple answers..."]
i need to lookup my I9 15900k in your db sir

[2025-02-08 18:20] sync: can i do that

[2025-02-08 19:17] .: [replying to sync: "i need to lookup my I9 15900k in your db sir"]
8C/16T

[2025-02-09 17:39] Ignotus: I have a .jar file with some Kotlin code. I can import it as a library to IDEA and view decompiled code but I can't add breakpoints to it. I need to be able to debug it. What can I do?

[2025-02-09 17:44] Ignotus: also I don't have the sources for it

[2025-02-09 19:09] dullard: [replying to Ignotus: "I have a .jar file with some Kotlin code. I can im..."]
You can set up the Java command line to accept debug connections

[2025-02-09 19:11] Ignotus: [replying to dullard: "You can set up the Java command line to accept deb..."]
I did but the problem is I can't set breakpoints because it's kotlin