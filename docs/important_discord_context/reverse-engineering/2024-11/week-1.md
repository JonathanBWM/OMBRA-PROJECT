# November 2024 - Week 1
# Channel: #reverse-engineering
# Messages: 198

[2024-11-01 13:33] hxm: <@162611465130475520> https://pastebin.com/JFTKed5c
[Embed: #ifndef ICICLE_VM_H#define ICICLE_VM_H#include #include]
Pastebin.com is the number one paste tool since 2002. Pastebin is a website where you can store text online for a set period of time.

[2024-11-01 13:33] hxm: incompatibilities between Rust types and the requirements of cbindgen is annoying asf

[2024-11-01 13:34] Brit: could just learn rust

[2024-11-01 13:34] Brit: or use the py bindings

[2024-11-01 13:35] hxm: your comment is helpless <:waitstare:1295829770990387361>

[2024-11-01 13:35] Brit: ah right I should commiserate because making rust bindings for c++ is pain

[2024-11-01 13:36] Brit: that sucks, I hope you figure it out 👍🏻

[2024-11-01 13:54] mrexodia: [replying to hxm: "<@162611465130475520> https://pastebin.com/JFTKed5..."]
This exposed API seems to be a bit much?

[2024-11-01 13:54] mrexodia: See the python bindings, there I exposed a much simpler API. Should be easy to mirror in C

[2024-11-01 15:19] James: [replying to hxm: "your comment is helpless <:waitstare:1295829770990..."]
Asking people for help online is like using a llm. Make sure u fully specify what you want or you will be given bullshit.

[2024-11-01 16:22] Deleted User: Inb4 links how to ask a question

[2024-11-01 16:24] James: Most people are cynical and intentionally look for holes in people’s questions to focus on and bully them over.

[2024-11-01 16:25] James: Mrexodia is not one of those people though, he’s pretty good at figuring out what people are asking cuz he’s been doing it a while

[2024-11-01 16:46] dinero: wonder who he’s talking about

[2024-11-01 16:46] dinero: truly a mystery

[2024-11-01 17:15] ByteWhite1x1: Hi. Lets start off with this https://www.gigabyte.com/Motherboard/X470-AORUS-ULTRA-GAMING-rev-10/support#support-dl-bios
[Embed: X470 AORUS ULTRA GAMING (rev. 1.0) Support | Motherboard - GIGABYTE...]
Lasting Quality from GIGABYTE.GIGABYTE Ultra Durable™ motherboards bring together a unique blend of features and technologies that offer users the absolute ...

[2024-11-01 17:15] ByteWhite1x1: This is related to "my" job-listings

[2024-11-01 17:16] ByteWhite1x1: And the MB is I've in my gaming PC (provided I got time to play, I currently do not)

[2024-11-01 17:20] ByteWhite1x1: As of Mar 22, 2024. (LogoFAIL) is still a go or no? If you haven't updated your MB BIOS in the past few years..

[2024-11-01 17:21] ByteWhite1x1: I am an anti-malware driver developer but UEFI (I simply have no time for everything..)

[2024-11-01 17:22] Sapphire: Like EDR?

[2024-11-01 17:23] snowua: [replying to ByteWhite1x1: "I am an anti-malware driver developer but UEFI (I ..."]
https://tenor.com/view/funny-gif-11026644638307910936

[2024-11-01 17:23] Sapphire: so are you trying to detect UEFI exploits? or just if a bios is outdated?

[2024-11-01 17:24] Brit: he wants to bypass secureboot

[2024-11-01 17:24] ByteWhite1x1: Yes

[2024-11-01 17:24] Sapphire: but for what purpose

[2024-11-01 17:25] Brit: to load his p2c

[2024-11-01 17:25] dinero: cheats

[2024-11-01 17:25] ByteWhite1x1: For https://zerodium.com/
[Embed: ZERODIUM - The Premium Exploit Acquisition Platform]
ZERODIUM is the leading exploit acquisition platform for premium zero-days and advanced cybersecurity research. Our platform allows security researchers to sell their 0day (zero-day) exploits for the 

[2024-11-01 17:25] dinero: they don’t buy paster bugs

[2024-11-01 17:25] Sapphire: so are you asking someone to find a zeroday for you? or submitting old known vulns?

[2024-11-01 17:25] ByteWhite1x1: I try to be honest over here on this server...

[2024-11-01 17:26] dinero: that’s why you’re probably not banned

[2024-11-01 17:26] snowua: please make exploit for me so i can sell your exploit

[2024-11-01 17:26] dinero: honesty is a virtue!

[2024-11-01 17:26] dinero: anyway

[2024-11-01 17:26] dinero: [replying to ByteWhite1x1: "As of Mar 22, 2024. (LogoFAIL) is still a go or no..."]
is logofail a exploit

[2024-11-01 17:26] ByteWhite1x1: I hear you guys. So I finally figured out what was my "boss" intentions...

[2024-11-01 17:26] ByteWhite1x1: Thank you so much for telling me that.

[2024-11-01 17:27] Sapphire: No problem! Your "boss" is a dickhead

[2024-11-01 17:27] dinero: https://www.kaspersky.com/blog/logofail-uefi-vulnerabilities/50160/
[Embed: LogoFAIL attack via image substitution in UEFI]
Researchers have found a way to attack computers through the logo image replacement mechanism in UEFI.

[2024-11-01 17:27] ByteWhite1x1: I am personally working on an OSINT -principles on an anti-malware driver if any of you wanted to know that

[2024-11-01 17:27] dinero: ah

[2024-11-01 17:28] ByteWhite1x1: I don't want to pretend anything that I am not

[2024-11-01 17:29] ByteWhite1x1: But the "LogoFAIL" is probably still a good to provided you haven't updated your BIOS in the past two or more years. That's what I figured.

[2024-11-01 17:29] ByteWhite1x1: You all know that. Two or more brains are usually better than one..

[2024-11-01 17:31] ByteWhite1x1: And what comes to can1357 PG bypass. Yes I do know about that too...

[2024-11-01 17:32] ByteWhite1x1: The PG payload is in the OS itself. "Injected" in a legitimate payloads...

[2024-11-01 17:32] ByteWhite1x1: Err. In a legitimate routines...

[2024-11-01 17:32] ByteWhite1x1: Let me paste something to get you started

[2024-11-01 17:35] ByteWhite1x1: To be honest. Sometimes I wonder that shoud I just release my bypass publicly to everone.

[2024-11-01 17:38] ByteWhite1x1: Satoshi Tanda is one of the main researches. But I don't see many giving him credits?

[2024-11-01 17:39] ByteWhite1x1: Show let me share something. Whenever the PatchGuard initialization method decied to pick up the thread method (you'll also find find possible malware threads with it)

[2024-11-01 17:39] ByteWhite1x1: I am an malware analyst as I develop my own...

[2024-11-01 17:41] ByteWhite1x1: In terms of reversal. Once you know the terms what to search for (OSINT)

[2024-11-01 17:41] ByteWhite1x1: An outdated EDRSandblast-master...

[2024-11-01 17:42] ByteWhite1x1: They overlooked my research and deemed me as an idiot

[2024-11-01 17:42] ByteWhite1x1: This time

[2024-11-01 17:42] ByteWhite1x1: I'll return the favoer

[2024-11-01 17:42] ByteWhite1x1: I'll post this on off-topic

[2024-11-01 17:55] ByteWhite1x1: Positive Research, 2014. It's almost still the same

[2024-11-01 17:56] ByteWhite1x1: A kernel routine. No this time a DPC? No a custom in the CPU0 block. Or how about all the tree at once?

[2024-11-01 17:56] ByteWhite1x1: LOL

[2024-11-01 17:57] ByteWhite1x1: The PG routine itself is basically a malware in the kernel that tries to distinguis a legitimate routine

[2024-11-01 17:57] ByteWhite1x1: Of course obfuscated DPC's

[2024-11-01 18:00] ByteWhite1x1: So I though of sharing before someone else will take a credit of my research

[2024-11-01 18:02] dinero: tbh i don’t see a link just a walk of text

[2024-11-01 18:03] dinero: yapping about it isn’t the same as posting it

[2024-11-01 18:03] ByteWhite1x1: KiWaitNever and KiWaitAlways. I am almost sure that daax understands it

[2024-11-01 18:04] ByteWhite1x1: In the CPU0 block

[2024-11-01 18:04] ByteWhite1x1: A Chinise work?

[2024-11-01 18:04] ByteWhite1x1: // dt _KDPC <pDpc>
            // dt _KTIMER <pTimer>

            // db nt!KiSetTimerEx
            // 
            // v10 = KiWaitNever ^ __ROR8__(a1 ^ _byteswap_uint64(a5 ^ KiWaitAlways), KiWaitNever);

            ptrDpc ^= (*(ULONG_PTR*)KiWaitNever); // v13 = KiWaitNever ^ v12;
            ptrDpc = _rotl64(ptrDpc, (*(ULONG_PTR*)KiWaitNever)); // v12 = __ROR8__(a1 ^ _RSI, KiWaitNever);
            ptrDpc ^= (ULONG_PTR)pTimer;
            ptrDpc = _byteswap_uint64(ptrDpc); // __asm { bswap rbx }
            ptrDpc ^= *((ULONG_PTR*)KiWaitAlways); // _RSI = a5 ^ KiWaitAlways;

[2024-11-01 18:06] ByteWhite1x1: Custom routines in the kernel are "backed" up by the PatchGuard routines in a useful payloads

[2024-11-01 18:06] ByteWhite1x1: So basically a malware in the kernel

[2024-11-01 18:08] dinero: 🤯

[2024-11-01 18:08] dinero: big if true

[2024-11-01 18:08] ByteWhite1x1: It is. Took me a long time to figure it out

[2024-11-01 18:09] Brit: is your take that pg is malware

[2024-11-01 18:09] Brit: that's a new one

[2024-11-01 18:09] ByteWhite1x1: The way the routine initializes...

[2024-11-01 18:09] dinero: someone got the tetrane link

[2024-11-01 18:10] dinero: didn’t they reverse engineer pg and it’s init routines extensively

[2024-11-01 18:10] ByteWhite1x1: As of today. You can't get nothing working out if it

[2024-11-01 18:10] ByteWhite1x1: The funny thing is

[2024-11-01 18:11] dinero: is this malware lads
[Attachments: unknown.png]

[2024-11-01 18:11] ByteWhite1x1: An U.S company bough their solution off and when I tried to offer mine

[2024-11-01 18:11] ByteWhite1x1: None of them was interested

[2024-11-01 18:11] ByteWhite1x1: So I started to suspect them as well

[2024-11-01 18:12] ByteWhite1x1: They all overlooked the facts I provided. As a "big" chip delevering company whatever that BS was bou

[2024-11-01 18:12] ByteWhite1x1: was about

[2024-11-01 18:12] dinero: have you considered lithium

[2024-11-01 18:12] dinero: i’m not calling you crazy but this is clearly manic

[2024-11-01 18:12] dinero: no offense

[2024-11-01 18:13] ByteWhite1x1: Maybe I am but. The facts:

[2024-11-01 18:13] ByteWhite1x1: 1) System thread

[2024-11-01 18:13] ByteWhite1x1: 2) WorkerItem

[2024-11-01 18:13] ByteWhite1x1: 3) DPC that initiases itself in an encrypted context

[2024-11-01 18:13] ByteWhite1x1: In a "legitimate" kernel routine

[2024-11-01 18:14] ByteWhite1x1: And sometimes just a direct kernel routine

[2024-11-01 18:15] ByteWhite1x1: Or all the aforementioned (3) at once...

[2024-11-01 18:15] ByteWhite1x1: And that's why it's been difficult for all of us "security researches" reach our goals because we all trusted in the kernel

[2024-11-01 18:16] ByteWhite1x1: Do you have any better fac to provide with the community then? Yes I have a mental problem but this is my hobby!

[2024-11-01 18:17] Brit: what's the alleged issue with the fact that pg is tricky?

[2024-11-01 18:18] ByteWhite1x1: Tha'ts a basics Microsoft*ism

[2024-11-01 18:18] ByteWhite1x1: Or how that's called in a native U.S language?

[2024-11-01 18:18] Brit: I don't see the problem

[2024-11-01 18:19] ByteWhite1x1: A PG payload distinguis in a legitimate kernel routine

[2024-11-01 18:20] Brit: it is legitimate

[2024-11-01 18:20] ByteWhite1x1: Is it? So how abou then the fact

[2024-11-01 18:20] ByteWhite1x1: Wh is my browsing history in the kernel memory area

[2024-11-01 18:20] ByteWhite1x1: As per of MS's good practises

[2024-11-01 18:20] Brit: why wouldn't it be?

[2024-11-01 18:21] ByteWhite1x1: A user-mode should leak the data to the kernel memory

[2024-11-01 18:21] ByteWhite1x1: Why I do see URL's I visited also in the kernel memory while using FireFox browser=

[2024-11-01 18:21] ByteWhite1x1: should not leak

[2024-11-01 18:22] ByteWhite1x1: Why I do see URL's I visited in FF in the kernel memory as well?

[2024-11-01 18:22] ByteWhite1x1: As per as of MS policies: A user-mode driver should not leak the DATA from UM->KM

[2024-11-01 18:22] ByteWhite1x1: So WTF?

[2024-11-01 18:23] Sapphire: does FF load any drivers?

[2024-11-01 18:24] ByteWhite1x1: I hope that I am not breaking any rules

[2024-11-01 18:24] ByteWhite1x1: I'll PM to u

[2024-11-01 18:24] Brit: there's no barrier in that direction

[2024-11-01 18:25] Brit: why couldn't any random driver copy um memory

[2024-11-01 18:26] ByteWhite1x1: I found the URL's I visited in the kernel memory address space. And I know that as an anti-malware kernel driver developer that all the major AV/EPP's refused to admit as far as I contacted them...

[2024-11-01 18:26] ByteWhite1x1: And there is an article on MS site that a user-mode should not never leak the DATA from UM-KM...

[2024-11-01 18:28] Brit: you've got the relationship reversed

[2024-11-01 18:28] ByteWhite1x1: URLs I visited in FF browser... How it's possible those appreared in the kernel memory?

[2024-11-01 18:29] ByteWhite1x1: How about your Windows 10/11 CD-KEY in plain text in the kernel memory?

[2024-11-01 18:29] ByteWhite1x1: And all those "major" bullshit AV/EDR/name-our-shit-over-here

[2024-11-01 18:30] ByteWhite1x1: This is what I wanted to share with the community.

[2024-11-01 18:30] ByteWhite1x1: Take a look at my job offer once again...

[2024-11-01 18:31] ByteWhite1x1: Thank you. I got hired by for the job...

[2024-11-01 18:31] ByteWhite1x1: And as a honest one. I try to do the job as good as I can. My intentions are good but I am not sure about the one who hired me...

[2024-11-01 18:32] ByteWhite1x1: This?

[2024-11-01 18:32] ByteWhite1x1: https://zerodium.com/
[Embed: ZERODIUM - The Premium Exploit Acquisition Platform]
ZERODIUM is the leading exploit acquisition platform for premium zero-days and advanced cybersecurity research. Our platform allows security researchers to sell their 0day (zero-day) exploits for the 

[2024-11-01 18:32] ByteWhite1x1: Hire me for the job. Offer a $100k USD and receive 1M USD if that works?

[2024-11-01 18:32] ByteWhite1x1: Yeah.

[2024-11-01 18:33] ByteWhite1x1: Thanks for understanding.

[2024-11-01 18:34] ByteWhite1x1: Regardless of all that. This is my hobby as I got nothing else to do. So I continue then. Thank you!

[2024-11-01 18:35] ByteWhite1x1: Any of you suspecting me. I can provide a video PoC of my anti-malware that it's true...

[2024-11-01 18:38] brymko: sure

[2024-11-01 18:38] ByteWhite1x1: Who is the fhishy one?

[2024-11-01 18:40] ByteWhite1x1: I'll tell you all this. I've personally no need to bypass SB in the BIOS just to bypass kernel nati-cheat. There are still working ones...

[2024-11-01 18:40] ByteWhite1x1: I don't run any P2C

[2024-11-01 18:40] ByteWhite1x1: or provide one

[2024-11-01 18:41] ByteWhite1x1: In the past I used to but at time I was even more retarded

[2024-11-01 18:42] ByteWhite1x1: All of a sudden. At my corver someone I nevet met appear with a 100x more cash I never earned in my lifetime...

[2024-11-01 18:42] ByteWhite1x1: DEVIL

[2024-11-01 18:42] brymko: sure

[2024-11-01 18:44] James: <@1299060349827289169> sadly what you seek is actually impossible

[2024-11-01 18:44] James: it's been formally proven.

[2024-11-01 18:44] James: that is very unfortunate for you as you will not be receiving that money from your employer.

[2024-11-01 18:45] elias: [replying to ByteWhite1x1: "And as a honest one. I try to do the job as good a..."]
your "boss" is almost certainly a threat actor

[2024-11-01 18:45] ByteWhite1x1: I think I hear you. And thank you for providing me the information.

[2024-11-01 18:46] James: No problem.

[2024-11-01 18:46] James: By the way are you the persohn that made that custom REClass like thing?

[2024-11-01 18:46] elias: [replying to James: "<@1299060349827289169> sadly what you seek is actu..."]
huh?

[2024-11-01 18:46] elias: bypassing secure boot?

[2024-11-01 18:46] James: [replying to elias: "bypassing secure boot?"]
Yep, entirely impossible.

[2024-11-01 18:46] James: An article came out from the NSA last month proving it.

[2024-11-01 18:47] Brit: zhou et al proved it I hear

[2024-11-01 18:47] brymko: indeed

[2024-11-01 18:47] James: Yeah I was surprised to see his name on the paper...

[2024-11-01 18:47] James: Guy really gets around.

[2024-11-01 18:47] elias: oh damn thanks for enlightening me

[2024-11-01 18:47] Brit: that "et al" guy especially

[2024-11-01 18:47] Brit: where'd you reckon he's from

[2024-11-01 18:47] ByteWhite1x1: A lost month?

[2024-11-01 18:48] James: [replying to Brit: "where'd you reckon he's from"]
Some eastern european country for sure.

[2024-11-01 18:48] brymko: Ho Chi Minh, Nan Jing, Xia men and Chong qing did an amazing job

[2024-11-01 18:48] James: Maybe he's from a previously uncontacted african tribe?

[2024-11-01 18:50] ByteWhite1x1: Should have known this. The offer was "too good to be real". For all of as a &ref in the future do this:

[2024-11-01 18:50] James: Glad I could help set you straight 👍

[2024-11-01 18:50] ByteWhite1x1: I mean all of you in the channel

[2024-11-01 18:50] ByteWhite1x1: A moment

[2024-11-01 19:36] dinero: [replying to Brit: "zhou et al proved it I hear"]
zhou*

[2024-11-01 19:37] dinero: I can't tell you about "et al". All i can tell you is that Zhou has eyes and ears everywhere

[2024-11-01 19:37] dinero: Watch your step

[2024-11-01 19:37] dinero: booleans might get mixed into your breakfast one day

[2024-11-03 00:34] emma: yes

[2024-11-03 13:46] Deleted User: anyone know if there is a PID specific wireshark plugin or anything

[2024-11-03 15:07] daax: [replying to Deleted User: "anyone know if there is a PID specific wireshark p..."]
https://github.com/airbus-cert/Winshark
[Embed: GitHub - airbus-cert/Winshark: A wireshark plugin to instrument ETW]
A wireshark plugin to instrument ETW. Contribute to airbus-cert/Winshark development by creating an account on GitHub.

[2024-11-03 15:39] 0x208D9: [replying to ByteWhite1x1: "Custom routines in the kernel are "backed" up by t..."]
omgggggg lmao, can u name the function ? so i can take a look at it myself, i can post a similar ida snippet and claim the kernel has backdoors and modifies the certstore, aint no way to verify if true until the function name and the driver/binary in question is provided or named

[2024-11-03 15:40] 0x208D9: [replying to ByteWhite1x1: "// dt _KDPC <pDpc>
            // dt _KTIMER <pTim..."]
and that doesnt seem like a backdoor my friend i have seen worse which will actually make u skeptical about the way kernel does things

[2024-11-03 15:44] 0x208D9: [replying to ByteWhite1x1: "URLs I visited in FF browser... How it's possible ..."]
show proof of how and where u saw it in km memory or dont yap, as simple as that

[2024-11-03 15:44] 0x208D9: also ig the way he is reacting, he doesnt get the point of pg at all

[2024-11-03 15:45] 0x208D9: nvm he left, guess i would never find the backdoor

[2024-11-03 15:49] 0x208D9: dunno what kinda bypass he was talking about but u could bypass some security features exploiting windows : 
CVE 2024 29062

[2024-11-03 16:16] daax: [replying to Deleted User: "anyone know if there is a PID specific wireshark p..."]
btw if that plugin doesn't work, you can always resort to `netsh trace start capture=yes` <do whatever> `netsh trace stop` and then use this modified etl2pcapng (associates pid to process name in comments) and then open the pcap in wireshark and look around.

`etl2pcapng path\to\NetTraces\NetTrace.etl path\to\output\what.pcapng`
[Attachments: image.png, modified-etl2pcapng.cpp]

[2024-11-03 16:23] daax: worth noting that the pid given doesn't always mean it came from that pid... i.e. it can be misleading. the in/out pkts are traced in dpcs so they may run in some arbitrary process context. the pid association as well is flimsy because the process may not exist anymore when you run etl2pcapng

[2024-11-03 16:38] diversenok: Also I remember you can steal socket handles from other processes to mess with attribution

[2024-11-03 16:39] daax: [replying to diversenok: "Also I remember you can steal socket handles from ..."]
shadowmove or sthg along those lines iirc

[2024-11-03 16:48] Deleted User: [replying to daax: "worth noting that the pid given doesn't always mea..."]
ahh thanks again man ill look into this

[2024-11-03 21:12] 0x208D9: anyone knows any bypass around KCET?

[2024-11-03 22:25] cmpzx: Hello guys, how can i unpack themida c++ app?

[2024-11-03 22:41] toasts: use 7zip

[2024-11-03 22:46] idkhidden: [replying to cmpzx: "Hello guys, how can i unpack themida c++ app?"]
<:mmmm:904523247205351454>

[2024-11-03 22:50] Chucky: [replying to toasts: "use 7zip"]
and hxd