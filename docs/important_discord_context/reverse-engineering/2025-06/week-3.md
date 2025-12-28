# June 2025 - Week 3
# Channel: #reverse-engineering
# Messages: 114

[2025-06-09 10:22] mrexodia: [replying to ruan: "I've checked on idapyswitch its using 3.13.3150.10..."]
Whatever was the latest 3.12 at the time I worked on it

[2025-06-09 10:23] mrexodia: [replying to ruan: "I'm not using the release package, I have installe..."]
Yeah, I was referring to the IDA plugin. The VSCode extension does pretty much nothing and doesn't need any updates

[2025-06-10 01:04] ruan: I'm getting this message on ida
```
[IDACode] Detected python executable: C:\Users\RUAN\AppData\Local\Programs\Python\Python312\python.exe
0.00s - Debugger warning: It seems that frozen modules are being used, which may
0.00s - make the debugger miss breakpoints. Please pass -Xfrozen_modules=off
0.00s - to python to disable frozen modules.
0.00s - Note: Debugging will proceed. Set PYDEVD_DISABLE_FILE_VALIDATION=1 to disable this validation.
```
What about this "-Xfrozen-modules=off"?
I've found this answer about how to set it: https://stackoverflow.com/a/75347466
You had to do this thing?

[2025-06-10 03:13] ruan: I have updated all things used by the extension on its package.json and recompiled it, now its working properly with py 3.13, it was using some very old packages

[2025-06-10 08:29] Horsie: Do many EDRs hook syscalls in the kernel?

[2025-06-10 08:30] Horsie: Wonder if there are any patchguard safe interfaces that Microsoft provides them

[2025-06-10 08:45] Windy Bug: Nah

[2025-06-10 09:21] mrexodia: [replying to ruan: "I'm getting this message on ida
```
[IDACode] Dete..."]
I didn't have to do anything with `-Xfrozen-modules=off`, but got the same warning

[2025-06-10 09:22] mrexodia: [replying to ruan: "I have updated all things used by the extension on..."]
Could you do a PR? I can get the extensions updated in the store

[2025-06-10 11:53] mtu: [replying to Horsie: "Wonder if there are any patchguard safe interfaces..."]
ETW provides hook-like data to EDRs, if that’s relevant to you

[2025-06-10 12:06] diversenok: [replying to mtu: "ETW provides hook-like data to EDRs, if that’s rel..."]
You mean ETW can functionally replace hooks? Not really - hooks are synchronous (and allow modifying the resutl), ETW is asynchronous

[2025-06-10 12:24] mtu: I agree, but they provide hook-like data in the context of EDR

[2025-06-10 12:25] mtu: Call stacks, calling processes

[2025-06-10 12:28] diversenok: They just provide it when it's a bit too late 😅

[2025-06-10 12:30] mtu: As is the case of most EDR functions

[2025-06-10 12:31] mtu: AV blocks you, EDR stops you after you’ve been running for 5 minutes 😬

[2025-06-10 12:33] mtu: I think the default flush time for ETW is seconds, and I’d imagine you can somehow get more real-time consumption especially of the threat intelligence events

[2025-06-10 13:07] Brit: [replying to Horsie: "Wonder if there are any patchguard safe interfaces..."]
write to the lstar nothing bad will happen

[2025-06-10 13:07] Brit: promise

[2025-06-10 13:20] 0xatul: **  R  I  Y  A  L  L  **

[2025-06-10 13:20] 0xatul: https://tenor.com/view/meme-man-mememan-ah-meme-man-gif-16183367341737945519

[2025-06-10 13:35] daax: [replying to Horsie: "Wonder if there are any patchguard safe interfaces..."]
Some of the methods for doing this are suspiciously placed, but no— there aren’t any designed/given to EDR/AV that can be leveraged in a documented way.

[2025-06-10 16:05] x86matthew: [replying to Horsie: "Do many EDRs hook syscalls in the kernel?"]
nah, as above, just etw(ti) and synchronous callbacks

[2025-06-10 16:05] x86matthew: it was common practice for antivirus software to patch the ssdt 20 years ago

[2025-06-10 16:05] x86matthew: but this was one of the main reasons why microsoft introduced patchguard in vista

[2025-06-10 16:05] x86matthew: dodgy kernel hooks by AVs (and other software) caused system instability and made microsoft look bad

[2025-06-10 16:07] Horsie: [replying to mtu: "AV blocks you, EDR stops you after you’ve been run..."]
<@943099229126144030> So does this stand?

[2025-06-10 16:08] Horsie: I asked this because I got curious when I attended the conference.
It was a chinese AV team where they were tracking 0days used by threat actors

[2025-06-10 16:10] Horsie: They found a particular IOCTL code for some device that was exploited. After having discovered the vuln they had their inhouse AV hooking engine that hooked (Nt)DeviceIoControl(File?) and tracked that IOCTL and thats how they found a variant of that exploit too, after it was patched.

[2025-06-10 16:11] Horsie: Well that doesnt really make much sense wrt my original question because it couldve just been a usermode hook. Just something that piqued my interest in syscall hooks for monitoring calls with certain arguments.

[2025-06-10 16:12] Horsie: >synchronous callbacks
I guess the AVs do indeed hook some syscalls with some goofy patching I assume?

[2025-06-10 16:16] Horsie: Because I also read this blog some time ago which mentions that they also do indeed hook stuff

[2025-06-10 16:16] Horsie: https://www.paloaltonetworks.com/blog/security-operations/a-deep-dive-into-malicious-direct-syscall-detection/
[Embed: A Deep Dive Into Malicious Direct Syscall Detection - Palo Alto Net...]
This blog explains how attackers use direct syscalls to overcome most EDR solutions, by first discussing the conventional Windows syscall flow and how most EDR solutions monitor those calls.

[2025-06-10 16:25] diversenok: [replying to Horsie: "They found a particular IOCTL code for some device..."]
Attaching a filter/minifilter to any device to get notified about I/O is within the documented territory

[2025-06-10 16:28] diversenok: [replying to x86matthew: "it was common practice for antivirus software to p..."]
Some still do kernel hooks; though not via ssdt patching
[Attachments: image.png]

[2025-06-10 16:38] x86matthew: [replying to diversenok: "Some still do kernel hooks; though not via ssdt pa..."]
oh interesting, i didn't realise any of the big AVs still did this

[2025-06-10 16:38] x86matthew: i would've thought MVI membership would prevent them from straying too far

[2025-06-10 16:39] x86matthew: i'm a bit out of touch with anything other than elastic these days though

[2025-06-10 16:39] Horsie: [replying to diversenok: "Attaching a filter/minifilter to any device to get..."]
Ah fair. That does answer how one could possibly do it. But I was looking for a broader answer for hooking all kinds of syscalls. Thanks for the reminder though. Had forgotten about minifilters

[2025-06-10 21:20] daax: [replying to diversenok: "Some still do kernel hooks; though not via ssdt pa..."]
avast moment

[2025-06-10 21:21] daax: them and their goofy hypervisor -- that thing is vulnerable af.

[2025-06-10 21:23] daax: [replying to x86matthew: "oh interesting, i didn't realise any of the big AV..."]
ipower and i were going to release a post about some of these. notably kasp+avast because they're so cooked, but a few others do as well still.

[2025-06-10 23:04] iPower: yep. avast uses infinityhook variant whenever its possible. if they cant use infinityhook they will fallback to their hypervisor

[2025-06-11 00:30] toasts: is that exclusive to their enterprise or paid products? or does the free version ship with and utilize the hypervisor?

[2025-06-11 00:32] iPower: havent downloaded avast for a long time but im pretty sure you can configure free version to use hardware-assisted virtualization

[2025-06-11 00:34] toasts: very interesting, gonna have to give it a shot to tinker with it, thanks

[2025-06-11 01:29] UJ: [replying to toasts: "very interesting, gonna have to give it a shot to ..."]
lemme know if their free version has the hv in it (and if its obfuscated), i wanna take a look at it myself .

[2025-06-11 04:53] Windy Bug: [replying to daax: "ipower and i were going to release a post about so..."]
Kaspersky hook syscalls? Are you sure?

[2025-06-11 04:55] abu: [replying to diversenok: "Some still do kernel hooks; though not via ssdt pa..."]
How do you get a nice stack trace like that? Do you just dump the stack and check against modules from SystemInformation?

[2025-06-11 04:56] Windy Bug: [replying to Windy Bug: "Kaspersky hook syscalls? Are you sure?"]
Iirc their anti rootkit arkmon.sys hooks a limited set of APIs often leveraged in read / write primitives but that’s about it

[2025-06-11 04:58] Windy Bug: And even then they apply the hooks only on already detected drivers

[2025-06-11 05:39] iPower: [replying to Windy Bug: "Kaspersky hook syscalls? Are you sure?"]
man I even have a project on github that leverages it lol

[2025-06-11 05:40] iPower: avast/avg and others also do it. and they have been documented for years

[2025-06-11 05:47] iPower: [replying to UJ: "lemme know if their free version has the hv in it ..."]
answering your question, yes it has

[2025-06-11 05:47] iPower: all the relevant binaries come with the free version

[2025-06-11 05:47] iPower: its aswVmm.sys the name

[2025-06-11 05:48] iPower: and no, its not obfuscated. pretty easy to reverse engineer

[2025-06-11 05:49] iPower: same applies to kaspersky. klhk.sys is the hypervisor responsible for hooking system calls

[2025-06-11 10:11] Windy Bug: [replying to iPower: "man I even have a project on github that leverages..."]
The hypervisor? Which product actually uses it?

[2025-06-11 10:11] Windy Bug: KES doesn’t does it

[2025-06-11 10:12] Windy Bug: [replying to iPower: "same applies to kaspersky. klhk.sys is the hypervi..."]
Never seen it in use in prod

[2025-06-11 10:17] Windy Bug: Just checked, it loads only for their “SafeBrowser” feature. Not loaded by default <@789295938753396817>

[2025-06-11 12:53] diversenok: [replying to abu: "How do you get a nice stack trace like that? Do yo..."]
Enabled `ProcessHandleTracing` and passed a bogus handle to the API. `ObReferenceObjectByHandle` collected a stack trace on bad ref, which I formatted into text with DbgHelp using modules from PEB and `SystemModuleInformation`, yeah

[2025-06-11 19:55] iPower: [replying to Windy Bug: "Just checked, it loads only for their “SafeBrowser..."]
the driver is always loaded regardless of whether virtualization is enabled in the settings or not

[2025-06-11 19:56] iPower: [replying to Windy Bug: "The hypervisor? Which product actually uses it?"]
standard product

[2025-06-11 20:48] Windy Bug: [replying to iPower: "the driver is always loaded regardless of whether ..."]
I’m pretty sure that’s incorrect, will have to double check tomorrow

[2025-06-11 20:52] iPower: [replying to Windy Bug: "I’m pretty sure that’s incorrect, will have to dou..."]
man some of you really cant give up eh

[2025-06-11 20:52] iPower: 
[Attachments: image.png]

[2025-06-11 20:52] iPower: literally installed in fresh machine

[2025-06-11 20:52] iPower: free version

[2025-06-11 21:16] abu: [replying to diversenok: "Enabled `ProcessHandleTracing` and passed a bogus ..."]
Thx 🙏

[2025-06-11 21:41] mellownightt: [replying to Windy Bug: "Kaspersky hook syscalls? Are you sure?"]
Etw hook

[2025-06-11 21:43] iPower: [replying to mellownightt: "Etw hook"]
no, kaspersky uses their hypervisor

[2025-06-11 21:43] iPower: avast is the one using etw

[2025-06-11 21:46] mellownightt: Oh yeah i got mixed up

[2025-06-11 21:48] iPower: iirc klgse is the entity that communicates with klhk to apply the syscall hooks. you will find all functions that kaspersky hooks there

[2025-06-12 01:45] daax: [replying to Windy Bug: "I’m pretty sure that’s incorrect, will have to dou..."]
It’s not incorrect. Please do verify.

[2025-06-12 01:45] daax: Wild to suggest incorrect without doing that first.

[2025-06-12 04:13] Windy Bug: [replying to daax: "Wild to suggest incorrect without doing that first..."]
I actually did earlier that morning, klhk wasn’t loaded in my labs. That’s why I said double check, and I was referring to my lab’s configuration to make sure settings were set to default

[2025-06-12 07:27] Windy Bug: Just checked the documentation, says it loads only for their home products - KAV, KIS, KTS, KPLUS, KASP, KPREM

I do see it loaded on KES tho, but doesn’t hook anything. Bails on vmxon? Because the lab is set on VMs <@789295938753396817>

[2025-06-12 08:07] Xyrem: 😐

[2025-06-12 15:31] iPower: [replying to Windy Bug: "Just checked the documentation, says it loads only..."]
I literally googled kaspersky free, downloaded it and showed the screenshot for you lol

[2025-06-12 15:32] iPower: and yes you require nested virtualization if you're loading in VMs

[2025-06-12 15:44] Windy Bug: [replying to iPower: "I literally googled kaspersky free, downloaded it ..."]
What’s Kaspersky free

[2025-06-12 15:44] Windy Bug: Is it KIS?

[2025-06-12 15:44] Windy Bug: The Internet security?

[2025-06-12 15:45] Windy Bug: How do they call it

[2025-06-12 15:45] iPower: it's literally called kaspersky free lol

[2025-06-12 15:48] Windy Bug: Can you show the name they give arkmon?

[2025-06-12 15:49] iPower: what's arkmon? you're just throwing words with no context

[2025-06-12 15:50] the horse: arkmon is a chinese tool

[2025-06-12 15:50] iPower: I don't know why I'm doing the research you're supposed to be doing

[2025-06-12 15:50] the horse: as far as i know

[2025-06-12 15:50] mtu: Ok unrelated AV because you mentioned Chinese AV

[2025-06-12 15:51] mtu: Has anyone ever seriously researched a product named OneAV? By threatbook[.]cn

[2025-06-12 15:52] pinefin: [replying to mtu: "Has anyone ever seriously researched a product nam..."]
-# you're fine to post links without the https://, dont need to [.]

[2025-06-12 15:52] mtu: I has copies if anyone wants them, it’s pretty standard LUA/Yara with some obfuscation, I got the modules decrypted but never managed to dump their yara stuff

[2025-06-12 15:55] JustMagic: [replying to mtu: "I has copies if anyone wants them, it’s pretty sta..."]
Driver? 👀

[2025-06-12 15:55] mtu: Nope, all user space lincox

[2025-06-12 15:55] mtu: Otherwise I’d care much more lmao

[2025-06-12 15:55] mtu: I mainly want it because it’s shipped as a component of btpanel

[2025-06-12 15:56] JustMagic: [replying to mtu: "Nope, all user space lincox"]
<:old_man_yells_at_microsoft:1352528846720729108>

[2025-06-12 15:56] mtu: So a RCE/privesc in the file parsing would impact a bunch of chicom stuff, which I find amusing

[2025-06-12 16:13] Windy Bug: [replying to iPower: "what's arkmon? you're just throwing words with no ..."]
Their anti rootkit module

[2025-06-12 16:14] Windy Bug: They usually append the version name im  looking for after the name of the driver

[2025-06-12 16:14] iPower: ah no idea. I already uninstalled kaspersky

[2025-06-12 19:30] sync: [replying to the horse: "arkmon is a chinese tool"]
think u mean pyark

[2025-06-12 19:30] the horse: ah yes

[2025-06-13 16:13] TenuousMeal: I know this must have been asked a lot what would you recommend a a starting point to learn to devirtualize modern packers ? I have done a ton of research but it's still unclear to me. Should I learn to lift to LLVM, manually analyse the VM myself ? I guess I am just asking about a roadmap of the skills I need to acquire. 
I have found some pretty decent articles from Thalium but they were way above my paygrade.
I have tried to reverse a simple xor with every protection disabled besides virtualization and I struggle to even makes sense of is happening after. 
I don't understand for example why with every protection disabled besides virtualization of a simple function it  renders  the binary so different. It seems like themida is inserting code everywhere.

[2025-06-13 17:37] the horse: https://github.com/NaC-L/Mergen
https://back.engineering/blog/2021/05/18
https://back.engineering/blog/2021/06/21
https://cfp.recon.cx/recon2024/talk/YQK3FK/
[Embed: GitHub - NaC-L/Mergen: Deobfuscation via optimization with usage of...]
Deobfuscation via optimization with usage of LLVM IR and parsing assembly. - NaC-L/Mergen
[Embed: VMProtect 2 - Detailed Analysis of the Virtual Machine Architecture]
VMProtect 2 is a virtual machine based x86 obfuscator which converts x86 instructions to a RISC, stack machine, instruction set.
[Embed: VMProtect 2 - Part Two, Complete Static Analysis]
The purpose of this article is to expound upon the prior work disclosed in the last article titled, VMProtect 2 - Detailed Analysis of the Virtual Machine Architecture, as well as correct a few mistak
[Embed: Architecture Analysis of VMProtect 3.8: Demystifying the Complexity...]
VMProtect stands as one of the most sophisticated software protection systems employed in obfuscating malware. Increasingly utilized by malware authors, it is crucial for reverse engineers to understa

[2025-06-13 17:39] the horse: SC also has good articles

[2025-06-13 17:46] TenuousMeal: I will read all of this thanks

[2025-06-13 19:02] UJ: [replying to TenuousMeal: "I know this must have been asked a lot what would ..."]
> It seems like themida is inserting code everywhere

bro spawned into the game and went straight for the final boss. 

Anyways, yeah +1 to mergen as a good starting point, i played with it and easy to start with (simple docker cmd to run it). They even have an article published where you can try it out on a vmprotect func and see the original func - https://nac-l.github.io/2025/01/25/lifting_0.html
[Embed: Lifting Binaries, Part 0: Devirtualizing VMProtect and Themida: It...]
Table Of Contents

[2025-06-14 19:50] TenuousMeal: [replying to UJ: "> It seems like themida is inserting code everywhe..."]
I am open to suggestions for easier challenges. I just have a valid Themida licence that's why I used Themida.