# January 2024 - Week 1
# Channel: #🔗-research-and-links
# Messages: 145

[2024-01-01 20:07] donna🤯: I have been working on an anticheat for a few months now (for fun) and im slowly but surely getting happy with the state that its in. Recently improved system module integrity check performance significantly (1% lows improved by 25% in cs2) and cleaned up a few of the detection mechanisms. This discord has been a goldmine and I thought I'd share what I've got so far. Thanks 🙂

https://github.com/donnaskiez/ac
[Embed: GitHub - donnaskiez/ac: kernel mode anti cheat]
kernel mode anti cheat. Contribute to donnaskiez/ac development by creating an account on GitHub.

[2024-01-01 20:40] mrexodia: [replying to donna🤯: "I have been working on an anticheat for a few mont..."]
You should ask <@221286210851569664> to roast your code for a bit

[2024-01-01 20:46] jvoisin: [replying to donna🤯: "I have been working on an anticheat for a few mont..."]
Isn't it funnier to write a cheat instead ?

[2024-01-01 20:50] Deleted User: thats soo meta, anti cheats are way cooler :3

[2024-01-01 20:52] donna🤯: [replying to mrexodia: "You should ask <@221286210851569664> to roast your..."]
if he has the time that would be greatly appreciated

[2024-01-01 20:57] BrightShard: [replying to jvoisin: "Isn't it funnier to write a cheat instead ?"]
write a cheat for your own anticheat and then update the anticheat to block it and then make another cheat for the anticheat and then update the anticheat and then update the cheat and then...

[2024-01-01 21:30] jvoisin: [replying to BrightShard: "write a cheat for your own anticheat and then upda..."]
and write blogposts about it <3

[2024-01-01 21:33] BrightShard: monetize the first as an anticheat to games and the second as a bypass to cheat developers
then keep selling updates

[2024-01-01 21:33] BrightShard: infinite money glitch

[2024-01-02 03:55] afk: [replying to BrightShard: "monetize the first as an anticheat to games and th..."]
sounds like how antivirus companies doing 😂

[2024-01-02 04:37] BrightShard: [replying to afk: "sounds like how antivirus companies doing 😂"]
"viruses were invented by blue-teamers to sell more antivirus products"

[2024-01-02 04:49] Deleted User: [replying to donna🤯: "I have been working on an anticheat for a few mont..."]
why not integrity check .rdata and other RO and RX sections? Find relocs and check if they point inside the module, also check the IAT. This should be enough

[2024-01-02 04:51] Deleted User: or you can compute the relocation yourself and check against the one in mem as well 😛

[2024-01-02 05:18] donna🤯: [replying to Deleted User: "why not integrity check .rdata and other RO and RX..."]
Yea I think initially I just wanted to get a mvp working - but those are good ideas which I definitely should implement. Thx for the feedback 😄

[2024-01-02 05:20] Deleted User: npnp

[2024-01-02 05:21] Deleted User: 
[Attachments: image.png]

[2024-01-02 05:21] Deleted User: consider doing proper integrity checks on the headers

[2024-01-02 05:22] donna🤯: ahh

[2024-01-02 05:22] donna🤯: definitely needed

[2024-01-02 05:22] Deleted User: you can do the same on the ntheader, you need to fix imagebase

[2024-01-02 05:24] donna🤯: looks like it shouldnt be too hard to implement

[2024-01-02 05:25] Deleted User: it isnt, took me about 20ish minutes

[2024-01-02 05:25] Deleted User: you need to do a tad bit more work on kernel modules tho, especially ntoskrnl

[2024-01-02 05:26] Deleted User: other modules including user ones are piss easy

[2024-01-02 05:27] donna🤯: i dont actually have any ntoskrnl integrity checks atm, the small time i spent trying i just got a bunch of errors  - need to spend some time on figuring out why

[2024-01-02 05:27] Deleted User: [replying to donna🤯: "i dont actually have any ntoskrnl integrity checks..."]
I suggest dump the module to disk and write it in usermode, then port it to kernel. This way you don't waste time dealing with getting the driver running every time you wanna test lol

[2024-01-02 05:28] donna🤯: [replying to Deleted User: "I suggest dump the module to disk and write it in ..."]
thats not a bad idea

[2024-01-02 05:28] donna🤯: yea

[2024-01-02 05:29] donna🤯: worst thing about making drivers 🥴

[2024-01-02 05:29] Deleted User: I thought this was obvious lmao, I always write it in usermode and port to kernel. Unless its something I can only access by kernel

[2024-01-02 05:31] donna🤯: ill be honest i just write it in the kernel purely because i find it more fun.. whether its more efficient that way... thats another story 😹

[2024-01-02 05:45] Torph: [replying to Deleted User: "I suggest dump the module to disk and write it in ..."]
that's a good idea, I get so stuck in what I'm working on that it doesn't even occur to me to do stuff like this

[2024-01-02 05:58] Deleted User: also use vmware snapshots when testing drivers

[2024-01-02 05:59] Torph: i was testing on my real host for a while at first... switching to Hyper-V and having a kernel debugger was the best thing I ever did

[2024-01-02 05:59] Deleted User: 
[Attachments: image.png]

[2024-01-02 06:00] Torph: also. Hyper-V goes kinda crazy, most painless VM software I've ever used

[2024-01-02 06:00] Deleted User: haha

[2024-01-02 06:00] Deleted User: I dont use Hyper-V on my main machine, because I dont want to have anything running

[2024-01-02 06:00] Deleted User: its only on my vms to mess with vbs etc..

[2024-01-02 12:28] rad: https://adnanthekhan.com/2023/12/20/one-supply-chain-attack-to-rule-them-all/
[Embed: One Supply Chain Attack to Rule Them All]
I successfully exploited a critical misconfiguration vulnerability in GitHub’s actions/runner images repository. Posing a risk if improperly configured, self-hosted runners acted as the entry…

[2024-01-02 14:15] JustMagic: [replying to mrexodia: "You should ask <@221286210851569664> to roast your..."]
High effort

[2024-01-02 14:16] mrexodia: [replying to JustMagic: "High effort"]
But it's the best driver since sliced bread

[2024-01-02 14:16] mrexodia: 🧠

[2024-01-02 14:21] JustMagic: [replying to Deleted User: "consider doing proper integrity checks on the head..."]
It's easier to just walk all the executable pages (based on VAD/PTE) and compare with on-disk. Checking whether it's supposed to be executable according to headers doesn't actually give you much.

[2024-01-02 14:23] JustMagic: [replying to mrexodia: "But it's the best driver since sliced bread"]
All I can say is that it's definitely better than that #redteam monstrosity

[2024-01-02 14:23] mrexodia: xD

[2024-01-02 15:37] Deleted User: [replying to JustMagic: "It's easier to just walk all the executable pages ..."]
Yah and also do vad -> pte page protection comparision

[2024-01-02 17:46] Torph: [replying to donna🤯: "I have been working on an anticheat for a few mont..."]
doesn't this allow anything through as long as it renames itself to one of these executable names? is there another mechanism to protect against this?
<https://github.com/donnaskiez/ac/blob/master/driver/callbacks.c#L400>

[2024-01-02 17:47] mono: [replying to donna🤯: "I have been working on an anticheat for a few mont..."]
I have a question to your `TestINVDEmulation` HV detection.
It seems to be that executing the `invd` instruction on windows results in a bsod (likely a #GP(0)) on my and my friends PC.
Not sure why it's behaving like this since cpuid(eax=7) returns intel mpx as disabled on my system
Can you confirm this behavior? The detection does work in my VM since the vmexit only advances rip.

[2024-01-02 17:54] daax: [replying to mono: "I have a question to your `TestINVDEmulation` HV d..."]
that's funky, it shouldn't cause a #GP unless you're not in cpl 0 or you're doing it to some PRMRR region

[2024-01-02 17:54] daax: If it only happens when your vm is running that's one thing, but out of the box it shouldn't just bugcheck you

[2024-01-02 17:57] mono: [replying to daax: "that's funky, it shouldn't cause a #GP unless you'..."]
i tried it on bare metal linux and bare metal windows both in kernelmode.

[2024-01-02 17:57] mono: not sure if it's a #gp, just assuming it after reading the docs

[2024-01-02 17:58] mono: [replying to daax: "If it only happens when your vm is running that's ..."]
no vm was running when i did the test on bare metal windows

[2024-01-02 17:58] daax: [replying to Torph: "doesn't this allow anything through as long as it ..."]
yes, there's also a ton of other processes that will open handles like wininit, conhost, winlogon, audiodg, nvcontainer, nvidia share, system settings broker, etc... some it doesn't matter if they are denied a handle with sufficient access, but others it'll throw a fit or will break functionality like trying to use overlays.

[2024-01-02 18:01] daax: [replying to mono: "no vm was running when i did the test on bare meta..."]
interesting, if you're doing it @ the proper priv level then I'd assume you're doing it while some PRMRR region is active or -- have you checked the SR_BIOS_DONE msr (151h)? If bit 0 is 1 and cpuid[eax=07,ecx1].30 = 1 then it'll cause a problem

[2024-01-02 18:02] daax: you can check this out if you wanna quickly grab cpuid details / msr enums for your machine https://github.com/daaximus/arch_enum

[2024-01-02 18:03] donna🤯: [replying to Torph: "doesn't this allow anything through as long as it ..."]
Technically yes, that is a good point you bring up. Tbh i guess this is one of the problems with an open source anti cheat (lol) is that you arent meant to know thois

[2024-01-02 18:03] donna🤯: but you are right in the fact that I should add some further checks

[2024-01-02 18:04] daax: [replying to donna🤯: "Technically yes, that is a good point you bring up..."]
well, it wouldn't be difficult to find out even if not open source -- it's the whole "security through obscurity is not security"

[2024-01-02 18:04] donna🤯: [replying to daax: "well, it wouldn't be difficult to find out even if..."]
yea definitely true

[2024-01-02 18:18] mono: [replying to daax: "interesting, if you're doing it @ the proper priv ..."]
Testing this on my linux host since the tooling is easier:
`SR_BIOS_DONE (151h)` returns 3 thus bit 0 is set to 1
`cpuid[eax=07,ecx=1].30 ` i assume this is `invd_disable_­post_bios_done`  (https://en.wikipedia.org/wiki/CPUID) is 0

Can also test it on a bare metal windows install if that's better.
[Embed: CPUID]
In the x86 architecture, the CPUID instruction (identified by a CPUID opcode) is a processor supplementary instruction (its name derived from CPU Identification) allowing software to discover details 

[2024-01-02 18:22] daax: [replying to mono: "Testing this on my linux host since the tooling is..."]
it's eax[30] sorry

[2024-01-02 18:23] daax: if it's 1 in that cpuid leaf, and then the msr is 1 then invd after bios execution will cause an exception

[2024-01-02 18:23] jvoisin: narrator: what started as a fun casual anti-cheat project morphed into a feature-complete nation-state-level rootkit

[2024-01-02 18:24] Timmy: > nation-state-level
You mean purchased off a 25 year old

[2024-01-02 18:26] mono: [replying to daax: "it's eax[30] sorry"]
thought so, `invd_disable_­post_bios_done` is eax[30]

[2024-01-02 18:26] daax: [replying to Timmy: "> nation-state-level
You mean purchased off a 25 y..."]
i mean <@285957040688463873>  wrote some stuff that most nation states wish they had... when he was 13 <:lmao3d:611917482105765918>

[2024-01-02 18:27] daax: nothin beats the motivation and ambition of some true nerds <:prayge:827970344865890324>

[2024-01-02 18:27] Timmy: i know <:KEKW:798912872289009664>
just being realistic here

[2024-01-02 18:27] daax: [replying to mono: "thought so, `invd_disable_­post_bios_done` is eax[..."]
ye, so is it enabled on your machine?

[2024-01-02 18:28] daax: if not then the only other option is the PRMRRs are active

[2024-01-02 18:28] daax: [replying to Timmy: "i know <:KEKW:798912872289009664>
just being reali..."]
stoop- we only deal in fantasies <:Kappa:794707301436358686>

[2024-01-02 18:29] qwerty1423: chill

[2024-01-02 18:29] Timmy: oh right sorry forgot about that for a sec

[2024-01-02 18:30] mono: [replying to daax: "ye, so is it enabled on your machine?"]
no, bit 30 is set to zero

[2024-01-02 18:33] mono: [replying to daax: "if not then the only other option is the PRMRRs ar..."]
shouldn't then cpuid(eax=7) return eax bit 2 as 1?

[2024-01-02 22:30] Torph: [replying to donna🤯: "Technically yes, that is a good point you bring up..."]
even if it was closed src you could take the binary apart and see the exceptions being made in the callback code with IDA

[2024-01-02 22:35] diversenok: Yeah, how do you think we know things about Windows

[2024-01-02 22:35] donna🤯: yea of course

[2024-01-02 22:36] donna🤯: I guess i was more thinking the binary would be virtualized in some way, but then again at the end of the day that isnt really enough and the exceptions will still be found

[2024-01-02 22:36] donna🤯: might have to have a look to see if the exceptions have any constant identifiers

[2024-01-02 22:36] donna🤯: that are unique per process

[2024-01-02 22:38] Torph: oh like a way to identify the process other than filename? if those programs are signed or protected, you could check for the signature or process protection status

[2024-01-02 22:39] donna🤯: ah not a bad idea, there would definitely be some kinda of certificate that is used for lsass, csrss etc.

[2024-01-02 22:40] Satoshi: [replying to mono: "shouldn't then cpuid(eax=7) return eax bit 2 as 1?"]
Invd is a funky instruction that can easily mess up software state, eg, data written to memory is not reflected. Analyze dump and see how it dies. I bet it is confused software

[2024-01-02 22:43] Torph: [replying to Torph: "oh like a way to identify the process other than f..."]
(signature might be hard to get right because they could fuck with it in-memory to appear signed, or make it look like it came from the legit file)

[2024-01-02 22:43] Torph: but maybe Windows has stuff to handle that already, idk much about software signing

[2024-01-02 22:44] donna🤯: yea ill have a look into it see what i can find

[2024-01-02 22:55] mono: [replying to Satoshi: "Invd is a funky instruction that can easily mess u..."]
extremely unlikely, there is one instruction between wbinvd and invd and i doubt that there was an interrupt or preemption on every time i tested this. From dmesg on linux i found out that there was a `[  278.890366] general protection fault, maybe for address 0x0: 0000` at the `invd` instruction.

[2024-01-02 23:02] mono: <@260503970349318155> i also checked out how you handle the invd vmexit in your HyperPlatform project and you seem to just execute `invd` .  (https://github.com/tandasat/HyperPlatform/blob/d8bbb21db3eff54c47a0e81ce74c86e9968802bb/HyperPlatform/vmm.cpp#L1242)
Isn't that very dangerous? You don't seem to be doing any writeback for the cache modifications done by the hv before executing `invd`
[Embed: HyperPlatform/HyperPlatform/vmm.cpp at d8bbb21db3eff54c47a0e81ce74c...]
Intel VT-x based hypervisor aiming to provide a thin VM-exit filtering platform on Windows. - tandasat/HyperPlatform

[2024-01-03 03:27] Satoshi: fair enough. that's interesting behaviour and not sure what's causing #GP

[2024-01-03 03:29] Satoshi: and i agree. i would say it is logically broken (even though the project does not care about a guest breaking (into) the hv

[2024-01-03 03:55] drew: ive noticed a #GP(0) on recent processors as well. looking at volume ~~2~~ 3 it may be some interaction with SGX

[2024-01-03 03:55] drew: (unless they changed vol 2 to clarify a different reason)

[2024-01-03 03:57] drew: actually im misremembering, it was a section in volume 3, and it seems this is enabled by the bios
[Attachments: image.png]

[2024-01-03 04:43] daax: [replying to drew: "ive noticed a #GP(0) on recent processors as well...."]
This is what I was saying with the PRMRRs <@265882665952083968>

[2024-01-03 04:43] daax: I thought you had checked this though, if you haven’t I would double check.

[2024-01-03 11:12] mrexodia: https://daniel.haxx.se/blog/2024/01/02/the-i-in-llm-stands-for-intelligence/
[Embed: The I in LLM stands for intelligence]

[2024-01-03 12:38] mono: [replying to drew: "actually im misremembering, it was a section in vo..."]
That's exactly what i have read.
From what i have also read intel sgx is deprecated since 11th gen and i'm using a 12th gen intel cpu. (my friend uses an 11th gen)
<@609487237331288074>  In the UEFI i can't find any option to enable sgx or option to change the PRMMR value. Maybe it's hidden?
I would be surprised to have a feature enabled that was deprecated before the cpu was even produced

[2024-01-04 19:37] mrexodia: https://virtuallyfun.com/2024/01/04/win323mu-diy-wow/
[Embed: Win323mu / DIY WOW]

[2024-01-04 20:57] dullard: [replying to mrexodia: "https://virtuallyfun.com/2024/01/04/win323mu-diy-w..."]
I feel like <@943099229126144030> would find this fun

[2024-01-05 00:31] x86matthew: yeah i was speaking to this guy earlier today actually, his emulator is very similar to mine except he targets 32-bit NT executables whereas mine runs 16-bit dos-based NE exes. this means he can use native handle values and pointers in his emulator which is nice, i needed to write a custom 16bit<->32bit handle-mapping translator and custom heap manager etc for mine which was quite tedious. nice project

[2024-01-06 08:27] Timmy: https://www.speedscope.app/

[2024-01-06 18:17] Matti: https://github.com/vitoplantamura/BugChecker
[Embed: GitHub - vitoplantamura/BugChecker: SoftICE-like kernel debugger fo...]
SoftICE-like kernel debugger for Windows 11. Contribute to vitoplantamura/BugChecker development by creating an account on GitHub.

[2024-01-06 18:18] Matti: softice is finally back
https://www.youtube.com/watch?v=mzEBUHmknrA
[Embed: BugChecker on Bare Metal]
Running BugChecker directly on bare metal, on an HP Pavilion Dv2000, which is an old PC with a PS/2 keyboard. The OS is Windows 7 Home 32bit.

[2024-01-06 19:04] Timmy: need to get myself a keyboard with old interface <:KEKW:798912872289009664>

[2024-01-06 20:36] Torph: bugchecker is a good name 😂

[2024-01-06 20:59] Deleted User: [replying to Matti: "softice is finally back
https://www.youtube.com/wa..."]
dude I had the exact same laptop back in like 2009 or 10

[2024-01-06 21:01] Matti: that's not me, FYI

[2024-01-06 21:02] Matti: I would never use a laptop if I could use a PC

[2024-01-06 21:03] Matti: https://www.youtube.com/watch?v=-EQ5Imy7zZo if you want a more readable version by the way <:thinknow:475800595110821888> 
I posted the bare metal video because it's far more impressive than getting this to work in a VM
[Embed: BugChecker on Windows 11 22H2]
Demonstration of BugChecker on Windows 11 22H2, inside VirtualBox 7.0.4. A JavaScript breakpoint condition is written that changes the flow of execution in an user mode thread.

[2024-01-06 21:19] Deleted User: yea this is mad impressive lol

[2024-01-06 23:03] Torph: [replying to Matti: "https://www.youtube.com/watch?v=-EQ5Imy7zZo if you..."]
why JS scripting instead of something like Lua that's intended for this kind of stuff? JS is a strange choice

[2024-01-07 00:05] Matti: what makes lua "more intended" for this kind of stuff?

[2024-01-07 00:05] Matti: windbg uses JS

[2024-01-07 00:05] Matti: note: I hate both languages, I don't have a favourite here

[2024-01-07 00:35] 25d6cfba-b039-4274-8472-2d2527cb: last I checked lua was meant for Garry's Mod addons

[2024-01-07 03:45] Torph: [replying to Matti: "what makes lua "more intended" for this kind of st..."]
wasn't it created to run portably on embedded systems / places with little resources? whereas JS was created for website scripting
I know JS can be pretty fast so I wouldn't be surprised if they were similar speeds. but I don't know why you would write a JS engine or use a JS engine library, when Lua is already so quick and portable with great built-in support for exposing C APIs

[2024-01-07 04:11] Timmy: [replying to Matti: "what makes lua "more intended" for this kind of st..."]
it's a much smaller and simpler language on all fronts it's also equaly fast if you use luajit

[2024-01-07 04:19] Matti: true, it being smaller is actually worth a point for lua

[2024-01-07 04:20] Matti: as in, there's less of it

[2024-01-07 04:20] Matti: both are horrible languages to use so I don't really have an opinion there either way

[2024-01-07 04:21] Matti: re: performance: I'm pretty sure V8 will crush luajit
before you say there's no way this guy will implement V8 in kernel mode.... well he's already recreated softice

[2024-01-07 04:22] Matti: not that performance is high on my list of requirements for a debugger programming language <:lillullmoa:475778601141403648>

[2024-01-07 04:24] Matti: templeOS was unironically superior to both of these, since its scripting language was compiled and statically typed

[2024-01-07 04:26] Matti: C#, F#, typescript, or any FP language would be better still

[2024-01-07 04:26] contificate: who gives a fuck about speed for the use case this guy is using it for

[2024-01-07 04:27] Matti: +1, the reason kernel debugging is slow isn't the scripting language

[2024-01-07 04:28] contificate: from what I can see, if you even tried to bench any of these examples, you'd really be benching how fast his FFI to his own code works

[2024-01-07 04:29] Matti: it depends on the implementation (I haven't read the code yet), it's possible you'd also be benchmarking some NT peripheral hardware drivers

[2024-01-07 04:30] Matti: 10% that and 90% the FFI to self

[2024-01-07 04:31] Matti: for most regular NT kernel debugging sessions the KD transport time dominates everything else by orders of magnitude

[2024-01-07 04:55] Timmy: [replying to Matti: "both are horrible languages to use so I don't real..."]
I really like lua 🙂

[2024-01-07 05:02] Matti: oh well, don't let me spoil your fun then

[2024-01-07 05:03] Matti: I hate dynamic typing in general, what name someone then gave to some particular syntax that allows this dynamic typing is of like secondary importance

[2024-01-07 05:05] Matti: I will say python is very high on my hate list regardless

[2024-01-07 05:05] Matti: there's just something about the way it never ever fucking works

[2024-01-07 05:08] Matti: though speaking of lua, this is what I remember seeing literally every wow update
[Attachments: th1r1sv08ht21.png]

[2024-01-07 05:08] contificate: my opinion of Lua in particular is that people seem to like it when some game engine has given them a world where it's useful, but wouldn't otherwise implement normal programs with it

[2024-01-07 05:09] contificate: so it happens to reason that Lua could be replaced by any old shit in the game case and satisfy them all the same

[2024-01-07 05:11] Matti: yeah everywhere I've seen lua used, the reason for choosing lua was always simply that's what this game engine gives us

[2024-01-07 05:12] Matti: so that's what we use

[2024-01-07 05:13] Matti: not sure why it's popular among game (engine) devs, I assume it's just easy to implement

[2024-01-07 05:30] brymko: [replying to Matti: "not sure why it's popular among game (engine) devs..."]
yeah