# October 2025 - Week 5
# Channel: #reverse-engineering
# Messages: 81

[2025-10-27 16:58] mrexodia: [replying to guar: "<:Madeg:883865452500099102> please help
is there a..."]
You can uncheck “keep size” and then you can replace as much data as you like

[2025-10-27 16:59] mrexodia: Or select a bigger range and edit it

[2025-10-27 21:57] guar: [replying to mrexodia: "Or select a bigger range and edit it"]
<:Shruge:1048387309080416347> maybe im using an outdated version (jul4 2025)
[Attachments: x64dbg_ytvLkBSzq5.mp4]

[2025-10-27 21:59] mrexodia: [replying to guar: "<:Shruge:1048387309080416347> maybe im using an ou..."]
Press `Ctrl+E` (Binary -> Edit)

[2025-10-27 23:01] guar: [replying to mrexodia: "Press `Ctrl+E` (Binary -> Edit)"]
<:Prayge:897681381121982484> <:pepe_love:797439008137150504>

[2025-10-28 20:16] the horse: What does 'f' and 'i' mean in Microsoft's map files? File/Function and input(code/data)?

[2025-10-28 22:21] Federico Fantini: Hi folks 👋

[2025-10-29 10:00] the horse: [replying to the horse: "What does 'f' and 'i' mean in Microsoft's map file..."]
apparently f is for function, but it's not present for all functions

[2025-10-29 10:42] guar: <a:PepegeSitWave:801478289789157436> good morning folks.
anyone has any idea what `ReservedForDebuggerInstrumentation` is used for in _TEB?

[2025-10-29 11:29] lukiuzzz: [replying to guar: "<a:PepegeSitWave:801478289789157436> good morning ..."]
it s a reserved block of memory in the TEB where the kernel debugger can directly inject and execute a small stub of code or a data payload in the context of a user-mode thread.

[2025-10-29 14:21] guar: [replying to lukiuzzz: "it s a reserved block of memory in the TEB where t..."]
so i assume normally (as in, without a kernel debugger) it is basically free ~~real estate~~ RW 128 bytes ?

[2025-10-29 15:01] lukiuzzz: [replying to guar: "so i assume normally (as in, without a kernel debu..."]
Technically, yes, while no debugger is attached those 128 bytes are just writable memory in your TEB

[2025-10-29 21:33] 987839324l992821l329183: <@162611465130475520>hey there, hope you  dont mind a ping- Im a 14 year old learning reverse engineer hoping to  go down paths for red teaming, Malware Analysis, Cyber Sec, and computer science for college... If  you can give me any tips where to start for malware analysis and reversing itll be a great help!

[2025-10-29 21:33] the horse: <#835655011115466772>

[2025-10-29 21:33] 987839324l992821l329183: [replying to the horse: "<#835655011115466772>"]
is tyhis just for  research?

[2025-10-29 21:34] mrexodia: [replying to 987839324l992821l329183: "<@162611465130475520>hey there, hope you  dont min..."]
Learn software engineering (C/C++) and do reversing on the side

[2025-10-29 21:34] mrexodia: A lot of people are making a fool out of themselves these days as a “red teamer” or “security researcher” not knowing any basics

[2025-10-29 21:34] 987839324l992821l329183: [replying to mrexodia: "Learn software engineering (C/C++) and do reversin..."]
yea, ive been begining to learn CPP and Rust atm. then after that ill start public depos then u believe i9 should start reversing on the side?

[2025-10-29 21:35] mrexodia: Depos?

[2025-10-29 21:35] Brit: repos*

[2025-10-29 21:35] Brit: most likely

[2025-10-29 21:35] 987839324l992821l329183: [replying to mrexodia: "Depos?"]
as in public projects

[2025-10-29 21:35] 987839324l992821l329183: mb

[2025-10-29 21:35] 987839324l992821l329183: lol

[2025-10-29 21:35] mrexodia: I guess you can try to write blog posts about what you learned

[2025-10-29 21:35] mrexodia: Just don’t expect people to read them or be nice to you, it’s very likely going to suck for a while

[2025-10-29 21:36] 987839324l992821l329183: [replying to mrexodia: "I guess you can try to write blog posts about what..."]
that and & associate myself with AntiCheat developers and engineers and other who believe i could also try making a P2C just for fun and learning usage.

[2025-10-29 21:36] mrexodia: But putting things you learned in writing is the best way to test if you really understand something (ignoring teaching which you cannot do)

[2025-10-29 21:36] 987839324l992821l329183: [replying to mrexodia: "Just don’t expect people to read them or be nice t..."]
sure yea, expecting that. Ill write posts as i learn and ill do as you said, Learn CPP etc basics first then slowly learn reversing. both seem fun and i already take rust & cpp as a passion

[2025-10-29 21:36] mrexodia: [replying to 987839324l992821l329183: "that and & associate myself with AntiCheat develop..."]
Don’t, P2C is like doing crime. You will hang around the most toxic people and have an awful experience in 99% of cases

[2025-10-29 21:38] mrexodia: Learn fundamentals and create value, much more likely to be productive than breaking other people’s stuff

[2025-10-29 21:38] 987839324l992821l329183: [replying to mrexodia: "Learn fundamentals and create value, much more lik..."]
Ah yea sounds good. Ill be focusing on basics and cpp / rust, slowly explore more and reach more boundaries. Reversing  seems fun to  me just wnated to reach out cuz id love to learn.

[2025-10-29 21:39] Brit: [replying to mrexodia: "Don’t, P2C is like doing crime. You will hang arou..."]
some of the worst people I've ever known are from there, some of the better ones too but mostly retards

[2025-10-29 21:40] the horse: cheating and narcissism are linked

[2025-10-29 21:40] mrexodia: [replying to 987839324l992821l329183: "Ah yea sounds good. Ill be focusing on basics and ..."]
Yeah, but reversing malware for example is something that’s actually good for society. Additionally it’s easy to be a top 1% malware analyst, not the case with a top 1% cheat developer

[2025-10-29 21:41] 987839324l992821l329183: [replying to mrexodia: "Yeah, but reversing malware for example is somethi..."]
okay yea i understand. Funny to  say I dream to be a anticheat engineer eventually, big hopes and dreams, just got pulled out of my private school system to go online school based for my future. as a 14 year old i still have lots to learn. ill be focusing on CPP Fundamentals and learning the lang itself.

[2025-10-29 21:42] Brit: I obviously do not know your circumstances but given you are 14 and no longer in school with your peers I think the most important thing for you is actually to socialize with people your age so you do not become completely disconnected from reality

[2025-10-29 21:44] 987839324l992821l329183: [replying to Brit: "I obviously do not know your circumstances but giv..."]
i still speak with them and we hangout all the time. but at my old school they didn't offer classes i need for big colleges i wish to go to, ex: Cambridge University & Oxford and ETH Zurich. So me going online based itll help me focus for comp sci and the math's i need to  learn.

[2025-10-29 21:44] 987839324l992821l329183: at my old school they did not offer classes for Calculus 2 & up. only Pre Calc

[2025-10-29 21:44] 987839324l992821l329183: was kind of needed to  switch. its rough, but itll be worth it in the long run.

[2025-10-29 21:49] mtu: [replying to mrexodia: "A lot of people are making a fool out of themselve..."]
Counterpoint: a lot are making good money

[2025-10-29 21:49] mtu: Step 1. Be lucky

[2025-10-29 21:52] mtu: [replying to mrexodia: "Yeah, but reversing malware for example is somethi..."]
+1 to this, being a mediocre RE analyst/low level software dev/vuln researcher is more beneficial to society than making the 165925th “fully ud Fortnite aimbot”

[2025-10-29 22:00] guar: [replying to mrexodia: "Just don’t expect people to read them or be nice t..."]
||better stay silent never make anything public and pretend you are mentally challenged when asking questions to not embarrass yourself|| (thats what i do <:5Head:854042364711272488> )

[2025-10-29 22:20] abu: [replying to guar: "||better stay silent never make anything public an..."]
And then once you ask the question,delete it so no one knows how stupid you are when you realize that the answer was right in front of you

[2025-10-29 22:23] mrexodia: [replying to mtu: "Counterpoint: a lot are making good money"]
Yeah sure, you can make a ton of money doing exit scams <:kekW:927728762978701342>

[2025-10-29 22:27] abu: [replying to mrexodia: "Yeah sure, you can make a ton of money doing exit ..."]
$2000 per year! What colours your yacht! 🛥️

[2025-10-30 00:20] daax: [replying to mtu: "Step 1. Be lucky"]
I’d argue Step 1. Be somewhat competent. You can consistently run profitable sites, doesn’t take luck at all imo.

[2025-10-30 00:26] UJ: how much do p2cs actually make? it feels like its only worth doing for 18-22 while in college for some beer money with no other job. anyone that can do that can get even the most basic dev job making a guaranteed 80k+

[2025-10-30 00:27] UJ: us centric salary ofc

[2025-10-30 00:28] Brit: if you are not retarded you are pulling more than that monthly

[2025-10-30 00:28] koyz: [replying to UJ: "how much do p2cs actually make? it feels like its ..."]
It ranges from a few bucks to hundreds of thousands of buckeroos a month, the latter is of course a huge exception but realistically if you don't care about being sued you can make a lot <:pepe_shrug:839310938770636811> (not legal advice <:kappa:716953351622885418>)

[2025-10-30 02:14] 987839324l992821l329183: [replying to UJ: "how much do p2cs actually make? it feels like its ..."]
Dpeends, some well known private devs who just pump out rebrands to 30+ brands at 450-550 a month make hella

[2025-10-30 02:14] 987839324l992821l329183: can make up to like 20k a month

[2025-10-30 02:15] 987839324l992821l329183: sounds insane, but R6 devs & Val devs make the most really

[2025-10-30 02:16] 987839324l992821l329183: some wackjobs charge like a 1.1k fee upfront then 350-750 monthly

[2025-10-30 02:16] 987839324l992821l329183: its absurd

[2025-10-30 02:16] 987839324l992821l329183: if ur pushing 750 a  month and got like 40 - 50 resellers with panels ur making up to 40 bands

[2025-10-30 02:16] 987839324l992821l329183: but you gotta be insane for ts. its rare

[2025-10-30 02:17] 987839324l992821l329183: and risky. heavy risk of getting fedded

[2025-10-30 03:45] daax: Please move the p2c discussion to <#1378136680271319121> or <#1378136917501284443>

[2025-10-30 04:01] rin: [replying to abu: "And then once you ask the question,delete it so no..."]
I feel personally called out

[2025-10-30 04:10] abu: [replying to rin: "I feel personally called out"]
And then make another account to ask questions because you think that it's embarassing and have crippling imposter syndrome

[2025-10-30 12:01] AWAY FOR THE WEEK: [replying to 987839324l992821l329183: "some wackjobs charge like a 1.1k fee upfront then ..."]
Roblox p2cs make the most, one of the top executors had the buy in set at 5k usd and thousands of users. They made close to a million dollars but 50k+ each month

[2025-10-30 12:44] guar: [replying to abu: "And then make another account to ask questions bec..."]
at least i get schizophenia benefits, free money ez

[2025-10-30 14:12] daax: [replying to AWAY FOR THE WEEK: "Roblox p2cs make the most, one of the top executor..."]
https://discord.com/channels/835610998102425650/835635446838067210/1433300702347923487

[2025-10-30 14:12] mtu: [replying to 987839324l992821l329183: "can make up to like 20k a month"]
At least in the U.S, that’s about what a “security engineer” at a largeish company can expect to make, with the advantage of not being at perpetual risk of getting sued or losing revenue when your preferred techniques get detected by ac

[2025-10-30 14:15] AWAY FOR THE WEEK: [replying to daax: "https://discord.com/channels/835610998102425650/83..."]
Mb

[2025-10-30 21:13] expy: quick question about Can's PatchGuard Garbage Collection https://blog.can.ac/2024/06/28/pgc-garbage-collecting-patchguard/: if HVCI is on, would it still catch up? SECURE_KERNEL_ERROR (18b)
UPD: HypervisorEnforcedCodeIntegrity is off in registry
[Attachments: image.png]

[2025-10-30 21:24] expy: is there any other legitimate code running from the RWX pages in kernel? once I mark all RWX pages as NX in kernel I still see some false positives + eventual SECURE_KERNEL_ERROR

[2025-10-30 22:08] elias: I don't think there are any rwx pages in vtl0

[2025-10-30 22:09] elias: there was an exploit a year ago or so iirc but should have been fixed a while ago

[2025-10-30 22:10] elias: Does someone here know what a value of 0 for `SourceHandle` in a call to `NtDuplicateObject` means?
[Attachments: image.png]

[2025-10-30 22:12] the horse: [replying to expy: "is there any other legitimate code running from th..."]
Yes, warbird (clipsp)

[2025-10-30 22:16] elias: [replying to the horse: "Yes, warbird (clipsp)"]
I don't think the pages are actually executable and writable at the same time, but the protections are changed using MmChangeImageProtection which calls to vtl1 if necessary

[2025-10-30 22:17] the horse: <@776638120950235167>

[2025-10-30 22:17] the horse: knows a lot more

[2025-10-30 22:17] elias: yeah

[2025-10-30 22:17] elias: I know this because of him lmao

[2025-10-30 22:21] grb: [replying to expy: "is there any other legitimate code running from th..."]
in VTL0 RWX is not possible if HVCI is running but it still possible to change from writable to executable and vice versa but only for specific cases only like warbird

[2025-10-31 06:56] grb: [replying to expy: "quick question about Can's PatchGuard Garbage Coll..."]
oh and btw, if you have VBS on, i dont see the point of a PG bypass because you cant even modify any code in the first place.