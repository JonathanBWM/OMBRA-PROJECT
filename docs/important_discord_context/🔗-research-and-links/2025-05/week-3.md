# May 2025 - Week 3
# Channel: #🔗-research-and-links
# Messages: 78

[2025-05-12 11:10] Flamingo: gg

[2025-05-12 12:34] Yoran: [replying to Daniel: "currently trying to reverse engineer the source co..."]
The whole thing?

[2025-05-12 14:18] Daniel: [replying to Yoran: "The whole thing?"]
Yeah. But when i started reading the 3k lines of hand-written assembly, i realized i was in over my head. I was curious how it installs itself as a hypervisor just above the firmware and loads Windows into it as a VM. I understood some of the code, but not all of it. I plan to circle back once my x64 asm is better. Frankly it's probably my computer architecture knowledge that also needs improvement.

[2025-05-12 20:25] UJ: [replying to Daniel: "Yeah. But when i started reading the 3k lines of h..."]
take a look at voyager as well as it might be more approachable and it does something similar.

[2025-05-12 20:26] UJ: also has way less assembly

[2025-05-13 00:00] iPower: [replying to UJ: "take a look at voyager as well as it might be more..."]
what does voyager have to do with DBVM m8

[2025-05-13 00:01] iPower: literally different projects with different purposes

[2025-05-13 00:01] iPower: the only thing they have in common is the context of virtualization

[2025-05-13 00:04] UJ: [replying to iPower: "the only thing they have in common is the context ..."]
yeah thats the piece i was referring to `I was curious how it installs itself as a hypervisor just above the firmware and loads Windows into it as a VM`

[2025-05-13 00:04] iPower: [replying to UJ: "yeah thats the piece i was referring to `I was cur..."]
yeah he isn't learning how hyper-v is loaded with... voyager

[2025-05-13 00:04] UJ: ah

[2025-05-13 01:38] Daniel: <@271835180644302849> I'll still take a peek. Thanks. I appreciate the feedback guys

[2025-05-13 07:04] teabound: not sure if this is useful to anyone but:
https://github.com/UnwindSafe/impbind
[Embed: GitHub - UnwindSafe/impbind: Adds fake imports to a 64bit PE file.]
Adds fake imports to a 64bit PE file. Contribute to UnwindSafe/impbind development by creating an account on GitHub.

[2025-05-13 13:37] david: Does anyone remember a post about finding the PEB in a Windows process without using the GS/FS register?

(This request was written without any ulterior motive. 😇)

[2025-05-13 14:03] 5pider: not possible

[2025-05-13 14:10] david: dang

[2025-05-13 14:25] hellohackers: [replying to 5pider: "not possible"]
https://gist.github.com/daaximus/570ba1902c335b02d866d8a312f1aa19
[Embed: find the peb in a novel, pain inducing manner]
find the peb in a novel, pain inducing manner. GitHub Gist: instantly share code, notes, and snippets.

[2025-05-13 14:26] 5pider: its fake

[2025-05-13 14:26] 5pider: this daax guy doesnt know what he is doing

[2025-05-13 14:28] hellohackers: [replying to 5pider: "not possible"]
there are some apis that leave memory addresses in registers that can be used to find peb as well

[2025-05-13 14:29] david: bait

[2025-05-13 14:29] david: I trust spider

[2025-05-13 14:29] 5pider: good

[2025-05-13 14:30] hellohackers: [replying to david: "I trust spider"]
https://x.com/yarden_shafir/status/1920651870202196109
[Embed: Yarden Shafir (@yarden_shafir) on X]
FYI if you’re willing to link with ntdll or dynamically resolve it there’s a ton of APIs that return TEB/PEB or leave them in one of the registers\.
\(Don’t believe official return values\. MSDN is a 

[2025-05-13 14:30] 5pider: i am just trolling

[2025-05-13 14:31] david: But I guess this is not very reliable across different windows versions?

[2025-05-13 14:32] diversenok: [replying to hellohackers: "there are some apis that leave memory addresses in..."]
Like `RtlGetCurrentPeb` that leaves it in `rax`

[2025-05-13 14:32] david: XD

[2025-05-13 14:42] david: I actually meant daax's tweet. I couldn't remember where I saw it. I just knew it was somehow related to this Discord.

[2025-05-13 15:07] david: lmao I'm dumb

[2025-05-13 15:09] david: I should stop talking (edit: sorry I was nervous)

[2025-05-14 01:58] 𝚜𝚊𝚟𝚘𝚒𝚛𝚎-𝚏𝚊𝚒𝚛𝚎: https://www.youtube.com/watch?v=mzlSBJl75cY&t=2
[Embed: Kernel OOB read/write to modprobe_path; pwn/checksumz IrisCTF 2025]
Capture the flag challenge from IrisCTF 2025.

Topics Covered:
* exploiting a vulnerable kernel driver
* OOB relative Read/Write
* overwrite modprobe_path

00:00 Intro
00:29 Challenge Setup
06:27 Code

[2025-05-15 09:48] diversenok: My new blog post about the IOCTLs behind Winsock and the recently added System Informer feature for inspecting `\Device\Afd` socket handles: https://www.huntandhackett.com/blog/improving_afd_socket_visibility
[Embed: Improving AFD Socket Visibility for Windows Forensics & Troubleshoo...]
This blog post explains the basics of Ancillary Function Driver API and how it can help explore networking activity on Windows systems.

[2025-05-15 11:22] mrexodia: https://blog.jetbrains.com/blog/2025/05/14/clion-and-the-open-source-community-growing-together/

<@839216728008687666> Windows types mentioned!
[Embed: CLion and the Open-Source Community: Growing Together | The JetBrai...]
From the beginning, CLion has been shaped by the needs of C and C++ developers around the world. Our cross-platform IDE was built to simplify development, boost productivity, and make working with C++

[2025-05-15 11:34] Saturnalia: > Now with over four million downloads, x64dbg is used globally by a diverse range of individuals, from indie hackers to institutional researchers, and even the Vatican

[2025-05-15 11:34] Saturnalia: Why does the Vatican use Duncandbg

[2025-05-15 11:35] brymko: holy missions

[2025-05-15 12:47] Matti: to know when it's easter

[2025-05-15 13:02] sariaki: Does this mean that x64dbg is blessed?

[2025-05-15 13:09] brymko: wait until you hear that they have like 10+ bloomberg terminals

[2025-05-15 13:28] mrexodia: [replying to Matti: "to know when it's easter"]
LOOOOOL this made me laugh

[2025-05-16 06:08] Sleepy: [replying to david: "Does anyone remember a post about finding the PEB ..."]
Are you talking about my post ? They banned my for posting it

[2025-05-16 06:10] Sleepy: It was my remote debugger I’ve been working on

[2025-05-16 06:12] Sleepy: https://github.com/sleepyG8/Remote-Debugger
[Embed: GitHub - sleepyG8/Remote-Debugger: A debugger I wrote capable of de...]
A debugger I wrote capable of debugging a remote process, this is a work in progress and will be updated frequently for more features - sleepyG8/Remote-Debugger

[2025-05-16 14:07] idkhidden: made a c++ macro for italian brainrot <:wojack:1352531001015930921>
https://github.com/idkhidden/ibmacro
[Embed: GitHub - idkhidden/ibmacro: C++ macro for italian brainrot.]
C++ macro for italian brainrot. Contribute to idkhidden/ibmacro development by creating an account on GitHub.

[2025-05-16 14:41] pinefin: [replying to idkhidden: "made a c++ macro for italian brainrot <:wojack:135..."]
tell me how i knew

[2025-05-16 14:42] pinefin: i saw "italian brainrot" and knew

[2025-05-16 17:04] Ђинђић: Part of reversing micorsoft Zone servers for titels like Internet Backgammon, everything i did and first time ever. Kinda bumpy and lumpy tho...
https://github.com/SljivoTompus/EXPERIMENT-BackgammonZoneServers
[Embed: GitHub - SljivoTompus/EXPERIMENT-BackgammonZoneServers: My little j...]
My little journey on reverse engineering the way Microsofts Servers for Internet Backgammon communicated and made a match. (THIS IS EXPERIMENTAL STATE, THERE COULD BE RIGHT INFO. OR MISINFO.) - Slj...

[2025-05-17 12:03] idkhidden: well
https://github.com/ReverseSec/vmpmylove
[Embed: GitHub - ReverseSec/vmpmylove: vmpmylove is a VMProtect unpacker an...]
vmpmylove is a VMProtect unpacker and iat resolver, all without any user execution - ReverseSec/vmpmylove

[2025-05-17 19:07] iPower: [replying to idkhidden: "well
https://github.com/ReverseSec/vmpmylove"]
uuuh this isn't even specific to VMProtect. many packers do the same thing... and this doesn't really handle IAT protection (which is still used a lot)

[2025-05-17 23:27] bishop: And not a devirt sad

[2025-05-18 06:26] idkhidden: [replying to iPower: "uuuh this isn't even specific to VMProtect. many p..."]
I have used import protection and it seems it resolves imports.

[2025-05-18 08:44] iPower: [replying to idkhidden: "I have used import protection and it seems it reso..."]
what VMP version are you using? import protection should create stubs for resolving imports

[2025-05-18 08:45] iPower: so automatic dumping won't work without a little effort

[2025-05-18 08:48] iPower: I haven't touched VMP in a long time but I'm pretty sure this is wrong

[2025-05-18 09:02] iPower: yeah I'm not crazy. just tried your tool with import protection and it doesn't work. this isn't even the complete IAT

[2025-05-18 09:02] iPower: 
[Attachments: image.png]

[2025-05-18 09:02] iPower: 
[Attachments: image.png]

[2025-05-18 09:03] iPower: or actually... not even close to it.

[2025-05-18 09:04] iPower: this was supposed to be the original IAT
[Attachments: image.png]

[2025-05-18 09:06] idkhidden: [replying to idkhidden: "I have used import protection and it seems it reso..."]
oh i was talking in runtime yes dumps will be broken

[2025-05-18 09:06] iPower: [replying to idkhidden: "oh i was talking in runtime yes dumps will be brok..."]
what do you mean by "in runtime"

[2025-05-18 09:07] iPower: not even "in runtime" this tool defeats VMPs import protection

[2025-05-18 09:07] iPower: it will just point to a bunch of stubs that will resolve the actual virtual addresses

[2025-05-18 09:11] idkhidden: [replying to iPower: "it will just point to a bunch of stubs that will r..."]
yes i know that

[2025-05-18 09:11] iPower: yeah so this doesn't accomplish what you said

[2025-05-18 09:11] iPower: which is what I said at the beginning

[2025-05-18 09:11] iPower: 🤦‍♂️

[2025-05-18 09:13] iPower: I wouldn't even advertise this as a VMP unpacker since it's... just the nature of almost every packer

[2025-05-18 09:44] Xyrem: [replying to idkhidden: "well
https://github.com/ReverseSec/vmpmylove"]
this doesnt unpack anything

[2025-05-18 09:47] Xyrem: I dont even understand what this is doing, you're just waiting for rdata to be writable and mem comparing between hopefully what is packed/encrypted with what it is after

[2025-05-18 09:49] Xyrem: there is 0 unpacking logic, matter of fact I dont even understand how this remotely worked for you. Sleeping for 100ms is so high of a delay that im gonna bet you're going to miss it 9/10 times and none of this code executes

[2025-05-18 09:50] Xyrem: "all without any user execution" this is such a lie

[2025-05-18 11:31] kiilo: Any book readers here? I was wondering if you guys have any recommendations on the general topic of assembly, binaries, dissasemblers and related things. Currently im reading a book on C, but I would love to hear from someone who's more experienced to read/research next.

[2025-05-18 12:55] GG: [replying to kiilo: "Any book readers here? I was wondering if you guys..."]
I found `practical malware analysis` and `reversing: secrets of reverse engineering` to be usefull

[2025-05-18 16:42] Mevas: [replying to idkhidden: "well
https://github.com/ReverseSec/vmpmylove"]
Wtf

[2025-05-18 16:42] Mevas: https://github.com/colby57/VMP-Imports-Deobfuscator?ysclid=matvx0mert902308574
[Embed: GitHub - colby57/VMP-Imports-Deobfuscator: VMProtect 2.x-3.x x64 Im...]
VMProtect 2.x-3.x x64 Import Deobfuscator. Contribute to colby57/VMP-Imports-Deobfuscator development by creating an account on GitHub.

[2025-05-18 16:43] Mevas: Works on all vmp3.*