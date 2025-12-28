# September 2025 - Week 3
# Channel: #programming
# Messages: 99

[2025-09-16 22:31] TRUCK: any ways to setup themida so it doesnt cause crashes or at least crashes less often?  i've seen other people complain, but not much people talking about any ways to improve or fix.

[2025-09-17 01:35] daax: [replying to TRUCK: "any ways to setup themida so it doesnt cause crash..."]
I know these types of answers are annoying but either: report it to Oreans, and get them to resolve the bug that way (they are very good in response, but ofc requires you have a license) or, don’t use Themida. It’s known for all kinds of bugs with threaded applications / potentially unsupported asynchronous operations.

[2025-09-17 01:55] the horse: [replying to TRUCK: "any ways to setup themida so it doesnt cause crash..."]
/Zc:threadSafeInit-
Don't use exceptions, avoid static variables.
Prefer C++14 over newer standards.
Avoid virtualization in multi-threading -- mutate instead, keep virtualization for the primary thread with most sensitive functionality.

[2025-09-17 02:20] daax: [replying to the horse: "/Zc:threadSafeInit-
Don't use exceptions, avoid st..."]
Yeah so, basically don’t use Themida for anything complex.

[2025-09-17 02:30] the horse: yeah.. you'll have to cater to themida's architecture and restrictions

[2025-09-17 02:30] the horse: i'd use VMP

[2025-09-17 02:41] TRUCK: [replying to the horse: "/Zc:threadSafeInit-
Don't use exceptions, avoid st..."]
thank you!  have not seen some of this advice yet

[2025-09-17 10:19] Brit: it is time to write your own binary obfuscator, surely not a waste of time

[2025-09-17 10:48] Brit: I just need another analysis pass trust :^)

[2025-09-17 14:06] pygrum: lmao reminds me of the VSA plans I had

[2025-09-17 14:06] pygrum: Cba

[2025-09-17 14:08] Brit: more dataflow analyses will set me free

[2025-09-17 14:08] Brit: [replying to pygrum: "lmao reminds me of the VSA plans I had"]
I recall suggesting it

[2025-09-17 14:08] Brit: <:mmmm:904523247205351454>

[2025-09-17 14:09] pygrum: yep you did, first step of the plan was the tail call detection to clean stuff up

[2025-09-17 14:09] pygrum: haven’t added any analyses since then

[2025-09-17 14:11] pygrum: deserves a much nicer design and ir

[2025-09-17 14:11] Brit: real

[2025-09-17 14:35] pygrum: something vex-like would be good but that’s a lot of effort for oss

[2025-09-17 15:45] avx: too true

[2025-09-17 15:47] contificate: need less shit languages for working with the IRs in

[2025-09-17 16:01] avx: <:Susge:1270806898869928048>

[2025-09-18 06:07] abu: [replying to contificate: "need less shit languages for working with the IRs ..."]
what would you recommend

[2025-09-18 06:14] Brit: You will write ocaml, you will bald, you will apply and be rejected from jane

[2025-09-18 06:44] abu: [replying to Brit: "You will write ocaml, you will bald, you will appl..."]
https://cdn.discordapp.com/attachments/1279939366684332042/1326991857972019211/attachment.gif

[2025-09-18 14:20] avx: [replying to Brit: "You will write ocaml, you will bald, you will appl..."]
LOL

[2025-09-18 14:21] Brit: Its not too late for c++ and dutasteride.

[2025-09-19 13:46] elias: Did anyone find a better way to find EPROCESS->VadRoot other than hardcoding offsets?

[2025-09-19 13:47] dinero: lmao

[2025-09-19 13:47] dinero: + 0x684

[2025-09-19 13:47] dinero: <:real:1393276555312103444>

[2025-09-19 13:58] elias: <:yea:904521533727342632>

[2025-09-19 17:19] Brit: [replying to elias: "Did anyone find a better way to find EPROCESS->Vad..."]
you do something like this https://hastebin.skyra.pw/agidowomul.rust

[2025-09-19 17:19] Brit: in your language of choice ofc

[2025-09-19 19:36] daax: <@820654436279124029> wrong channel.

[2025-09-19 19:37] daax: go to <#1378136917501284443> or <#835656385421115403>

[2025-09-19 20:34] Eriktion: [replying to elias: "Did anyone find a better way to find EPROCESS->Vad..."]
Pdb parse

[2025-09-19 20:36] diversenok: That is worse, not better

[2025-09-19 20:38] Brit: [replying to diversenok: "That is worse, not better"]
you say this but when you have to support many winvers

[2025-09-19 20:39] elias: I noticed the ExitStatus member is always the one before VadRoot (since at least Win10) which should have value STATUS_PENDING for a running process, you can try to search for this value then combine with some more memory checks to maximize probability that this is the right offset

[2025-09-20 06:25] Eriktion: [replying to elias: "Did anyone find a better way to find EPROCESS->Vad..."]
Another way would be to disassemble your way to success. There are exported functions within ntoskrnl which access EPROCESS->VadRoot. You can disassemble these functions and check at which offsets they access it

[2025-09-20 06:26] Eriktion: It’s a hacky solution, at least i would call it that, but still

[2025-09-20 06:27] Eriktion: In my opinion pdbparse, but if your‘re living in the kernel and want to pdbparse there that will be somewhat painful

[2025-09-20 06:27] Eriktion: Especially if you then need to download the pdb first

[2025-09-20 06:27] Eriktion: Gl using winsock (fuck those slow download speeds)

[2025-09-20 06:28] Eriktion: Also you have to then reroute https traffic via a proxy or sth to be converted to http cause guess what? Winsock doesn’t support that out of the box

[2025-09-20 09:01] Brit: [replying to Eriktion: "In my opinion pdbparse, but if your‘re living in t..."]
You do it pre build, or if need be at linking

[2025-09-20 11:47] daax: [replying to Brit: "you say this but when you have to support many win..."]
I’m with diversnok on this one. You can do an internal config that doesn’t require hardcoding or pdb parsing. More time writing it and determining the different things that change so your checks will execute correctly, but km zydis + wincfg to run your search routine will be better. no reliance on pdbs, you just won’t be able to handle future updates without pushing an update; trade off I suppose, but much prefer it over having to have symbols available.

similar to what is mentioned here: https://discord.com/channels/835610998102425650/835635446838067210/1092090274211168426

[2025-09-20 11:53] daax: And yes, this specific example is not generic, it is now I’m just not at PC yet to paste an updated one with the generic configuration based on winver + qfe.

[2025-09-20 11:54] Eriktion: [replying to daax: "I’m with diversnok on this one. You can do an inte..."]
Probably is better and faster then parsing pdbs but you sacrifice the ability to be completely independent from disassemblers etc. as well as being able to easily run on any winver

[2025-09-20 11:54] daax: [replying to Eriktion: "Probably is better and faster then parsing pdbs bu..."]
Disassemblers can be made very small.

[2025-09-20 11:54] Brit: the tradeoff is having to bring HDE || Zydis

[2025-09-20 11:54] Brit: yeah yeah

[2025-09-20 11:54] Brit: no I agree

[2025-09-20 11:54] Brit: I dont think its the solution for everyone

[2025-09-20 11:55] Brit: really depends on what you're making

[2025-09-20 11:55] Brit: but pdb parsing isnt all that cancerous (TM)

[2025-09-20 11:56] Eriktion: [replying to Brit: "but pdb parsing isnt all that cancerous (TM)"]
I found it to be okay-ish, but the truly horrible thing about pdb parsing in the kernel ain’t the parsing itself but the fetching of the pdb

[2025-09-20 11:57] daax: [replying to Brit: "I dont think its the solution for everyone"]
Of course not, if it doesn’t matter if you have to wait for PDBs, choose the easier path. If you want to be completely independent, it’s the best solution I’ve implemented so far to support everything from 1507 up to 25h2.

[2025-09-20 11:57] Brit: [replying to Eriktion: "I found it to be okay-ish, but the truly horrible ..."]
I would never advocate to do the pdb parsing in kernel itself

[2025-09-20 11:58] Brit: I like the prebuild way

[2025-09-20 11:58] Brit: personally

[2025-09-20 11:58] Brit: and for a lot of things pdb parsing is just way overkill

[2025-09-20 11:59] daax: [replying to Eriktion: "I found it to be okay-ish, but the truly horrible ..."]
You predownload them prior to loading your driver… wtf? I feel like if you have to do it all in your driver you’re doing something wrong, and your environment is more restrictive than purported.

[2025-09-20 12:00] Brit: [replying to daax: "Of course not, if it doesn’t matter if you have to..."]
if you're just disassing from an export sure, but when youre 15 calls deep in a chain for some offset then it starts being a bit cancer

[2025-09-20 12:00] Brit: and brittle

[2025-09-20 12:00] Brit: but yeah ofc bounded by pdb availability and how you set all that shit  up

[2025-09-20 12:01] daax: [replying to Brit: "if you're just disassing from an export sure, but ..."]
What item requires that deep of a chain? Furthest I’ve needed out of hundreds is 9. It’s not that terrible when you have helpers to “disasm into” the next part and allow arbitrary stages to execute for each of your configuration templates.

[2025-09-20 12:02] Brit: hypothetical, really, I have one thing I was working on where I decided to just pdb parse because it was cancerous

[2025-09-20 12:02] Brit: ill go find it

[2025-09-20 12:23] Eriktion: [replying to daax: "You predownload them prior to loading your driver…..."]
Take the example of an uefi driver that needs some windows specific offsets or sth

[2025-09-20 12:41] diversenok: I have several concerns regarding on-demand PDB parsing for anything unrelated to displaying debug information to the user:

[2025-09-20 12:41] diversenok: First, requiring internet connection to download several megabytes just to get some offsets (to read memory from a driver or do any other low-level operation) is a huge violation of abstraction levels and minimal dependencies

[2025-09-20 12:42] diversenok: Second, the whole point of using PDBs for dynamically retrieving offsets is to avoid hardcoding assumptions. Yet, often the only thing it achieves is shifting these assumptions from knowing specific values to knowing that the underlying logic itself didn't change

[2025-09-20 12:42] diversenok: Finally, being able to download PDBs for Windows binaries is a privilege, not something to take for granted. If people start abusing it too much outside of intended purposes, MS might restrict it. And I really don't want that.

[2025-09-20 12:42] Brit: I agree with all of these concerns, which is why I advocate for it as a build step and not at runtime

[2025-09-20 13:04] daax: [replying to Eriktion: "Take the example of an uefi driver that needs some..."]
Doing any form of on-demand PDB parsing in UEFI is ridiculous imo lol. I guess I've seen goofier things... I haven't needed more than heuristic-based discovery to get what I need in UEFI, and then acquire the rest once everything is available. It's what <@503274729894051901> said: violating abstraction levels and minimal dependencies.

[2025-09-20 13:10] diversenok: UEFI shouldn't know my WiFi password, right? 🤔

[2025-09-20 13:11] Brit: how else is it gonna get the pdbs sir

[2025-09-20 13:16] diversenok: Let's hope it honors my metered connection settings

[2025-09-20 13:24] Timmy: isn't wifi in uefi an absolute nightmare?

[2025-09-20 14:38] Eriktion: [replying to Timmy: "isn't wifi in uefi an absolute nightmare?"]
network connections in general are not too bad, however implementing the TLS protocol and thus https is a nightmore of its own due to the cryptogrophy envolved

[2025-09-20 18:39] mtu: [replying to diversenok: "UEFI shouldn't know my WiFi password, right? 🤔"]
Oh god are we doing this again

[2025-09-20 18:44] mtu: I’d rather implement TLS in EFI than any version of WPA from the last decade

[2025-09-21 03:13] daax: [replying to Eriktion: "network connections in general are not too bad, ho..."]
you’re suggesting they roll their own?

[2025-09-21 03:47] Addison: self-rolled is best my friend 🚬

[2025-09-21 11:54] donna🤯: [replying to Eriktion: "network connections in general are not too bad, ho..."]
Could you elaborate on this? Lol

[2025-09-21 11:57] donna🤯: [replying to mtu: "I’d rather implement TLS in EFI than any version o..."]
supplicant and hostapd give me nightmares 🙈

[2025-09-21 13:44] Eriktion: [replying to donna🤯: "Could you elaborate on this? Lol"]
Okay this is the way I understand it:
https basically is http  over TLS. 
Implementing it in a kernel driver would need  a TLS implementation: you will need to perform a TLS handshake, negotiate cipher suites, verify the server certificate chain, derive traffic keys, and then send/receive HTTP bytes within TLS records.

[2025-09-21 13:44] Eriktion: Correct me if I misunderstood that though ❤️

[2025-09-21 14:47] daax: [replying to Eriktion: "Okay this is the way I understand it:
https basica..."]
there exist libraries for this to run baremetal | in uefi. modified lwip / mbedtls.

[2025-09-21 17:44] Addison: rust crypto good

[2025-09-21 19:35] JustMagic: `SeAuditProcessCreationInfo`

[2025-09-21 20:48] pygrum: count on Geoff https://www.geoffchappell.com/studies/windows/km/ntoskrnl/inc/ntos/ps/eprocess/index.htm

[2025-09-21 20:52] koyz: Sadly we can’t count on him anymore :(( rip

[2025-09-21 20:59] the horse: rip

[2025-09-21 20:59] pygrum: No way

[2025-09-21 21:17] koyz: [replying to pygrum: "No way"]
Yeah he passed away about 2 years ago :/

[2025-09-21 23:35] Timmy: What kind of unlucky monstrocity did I run into here?

https://godbolt.org/z/MjazhExaG
[Embed: Compiler Explorer - C++]
template &lt;typename T&gt;
concept is_int_yes = requires { requires std::is_same_v&lt;T, int&gt;; };

template &lt;typename T&gt;
concept is_int_yesyes = requires(T) { requires std::is_same_v&lt;T, i