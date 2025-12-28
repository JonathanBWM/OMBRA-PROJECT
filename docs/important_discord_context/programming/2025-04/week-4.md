# April 2025 - Week 4
# Channel: #programming
# Messages: 80

[2025-04-22 00:18] diversenok: [replying to Matti: "https://omnicognate.wordpress.com/2009/10/05/winsx..."]
It's a good overview. I find it curious how the author approaches the underlying research from a completely different angle than I would (and you too probably). It's based on carefully reading documentation and combining its pieces into a single picture, all verified by some observations. "According to page A there is B, but the MSDN page on assembly search sequence claims C, and there is no mention of the detail D at all. I haven’t tested this, but if E is correct, F is not", etc. My only thought at this point would be to just open a decompiler and see for myself, instead of trying to interpret all this gossip and contradictions 😅

[2025-04-22 00:24] Matti: yeah I definitely felt the same at points

[2025-04-22 00:24] Matti: but he's probably more like most (sane) devs than we are

[2025-04-22 00:24] Matti: in his approach

[2025-04-22 00:25] Matti: the docs just suck and confuse the hell out of anyone who tries to read them, that is a fucking fact I've experienced first hand

[2025-04-22 00:26] Matti: the fact that RE *actually may be the easier alternative* is insane and not obvious to most I would imagine

[2025-04-22 00:29] diversenok: Haha, yeah

[2025-04-23 18:45] Ruben Mike Litros: am i tripping or did they change NyQuerySystemInformation(SystemModuleInformation)?

[2025-04-23 18:45] Ruben Mike Litros: im pretty sure it used to return full base address of the modules

[2025-04-23 18:45] Ruben Mike Litros: but now it only returns like partial address

[2025-04-23 18:46] Ruben Mike Litros: and OffsetToFileName is also always 0

[2025-04-23 18:46] truckdad: https://windows-internals.com/kaslr-leaks-restriction/
[Embed: KASLR Leaks Restriction]

[2025-04-23 18:58] Ruben Mike Litros: [replying to truckdad: "https://windows-internals.com/kaslr-leaks-restrict..."]
so its partial address in kernel mode and nothing in user mode?

[2025-04-23 19:03] truckdad: you're seeing that take effect in the kernel? based on the `if ( !PreviousMode ) return 0;` it seems like it shouldn't have any effects

[2025-04-23 19:05] Ruben Mike Litros: yea the address i get in kernel mode have the sign extension bits and the pml4 index (0x1f) removed

[2025-04-23 19:05] Ruben Mike Litros: and i get 0 in a normal user mode process

[2025-04-23 19:09] Ruben Mike Litros: im running windows 11 22h2 build 22621.4317

[2025-04-23 19:26] valium: is there a way to hook a function in a shared library thats loaded by dlopen()?
the program loads it by dlopen and heres how the /proc/pid/maps look like

```c4d80000-c6fb4000 r-xp 00000000 103:11 520118             /lib/lib.so```

[2025-04-23 19:27] valium: suppose the target function is at an offset 0xAABBCC from the starting of the shared library

[2025-04-23 20:37] Matti: [replying to Ruben Mike Litros: "yea the address i get in kernel mode have the sign..."]
can't say I've seen this - if this is happening in kernel I'd almost suspect there is something else (additionally) at play rather than this

[2025-04-23 20:38] Matti: as <@130836811214880769> pointed out the call is already a no-op in kernel due to the early return, and furthermore it would still allow the access regardless because SeSinglePrivilegeCheck(anything, KernelMode) i is always true

[2025-04-23 20:39] Matti: sorry, "with previousmode is kernelmode" to be precise, not just executing in kernel mode

[2025-04-23 20:39] truckdad: i'm also not 100% sure but i'm pretty sure it didn't get backported before 24H2, either

[2025-04-23 20:39] Matti: if in user mode, yeah you need SeDebug to get addresses back now

[2025-04-23 20:40] truckdad: when i saw "used to" and "now" i was sort of assuming you were on latest 😁

[2025-04-23 20:43] Matti: I just checked (in user mode) on 22621 by explicitly disabling sedebug and yeah, it did not get backported

[2025-04-23 20:43] Matti: I still get the full addresses

[2025-04-23 20:44] Ruben Mike Litros: hmm

[2025-04-23 20:45] Ruben Mike Litros: any ideas why it could be happening for me then?

[2025-04-23 20:49] Matti: no, I have to say I'm officially clueless

[2025-04-23 20:49] Matti: especially about the fact that you are getting incorrect results in kernel mode (I'm assuming PM = kernel like in driverentry for example)

[2025-04-23 20:51] Matti: are you running inside a hypervisor maybe? or a rootkit?

[2025-04-23 20:54] Ruben Mike Litros: im running under vmware

[2025-04-23 20:54] Ruben Mike Litros: no rootkits or additional hypervisors that im aware of

[2025-04-23 20:54] Ruben Mike Litros: maybe i got ratted and dont know it yet

[2025-04-23 21:03] Matti: well, I know vmware isn't malware, but still try running it on bare metal or something saner

[2025-04-23 21:03] Matti: it's unlikely tbh

[2025-04-23 21:03] Matti: but I've guessed worse things about vmware and been right before

[2025-04-23 21:06] Ruben Mike Litros: i will try in a few hours and inform if i find out what the issue is thanks for helping 👍

[2025-04-24 05:55] Ruben Mike Litros: found the issue

[2025-04-24 05:55] Ruben Mike Litros: they changed the module count to a 64 bit integer

[2025-04-24 05:55] Ruben Mike Litros: and i still had it as a 32 bit integer in my code

[2025-04-24 05:56] Ruben Mike Litros: 🥲

[2025-04-24 10:05] diversenok: [replying to Ruben Mike Litros: "they changed the module count to a 64 bit integer"]
No. The module count has always been 32 bit

[2025-04-24 10:06] Ruben Mike Litros: [replying to diversenok: "No. The module count has always been 32 bit"]
its 64 bit for me

[2025-04-24 10:06] diversenok: Stop using definitions from ntinternals, they are the worst

[2025-04-24 10:06] diversenok: They assume 32-bit pointers

[2025-04-24 10:07] Ruben Mike Litros: idk man it seems to be 64 bit for me

[2025-04-24 10:07] diversenok: https://ntdoc.m417z.com/rtl_process_modules
[Embed: RTL_PROCESS_MODULES - NtDoc]
RTL_PROCESS_MODULES - NtDoc, the native NT API online documentation

[2025-04-24 10:07] Ruben Mike Litros: [replying to diversenok: "https://ntdoc.m417z.com/rtl_process_modules"]
this is what i was using

[2025-04-24 10:07] Ruben Mike Litros: looking at expquerymoduleinformation in ida

[2025-04-24 10:08] Ruben Mike Litros: it adds + 2 to the 2nd arg which is dword ptr by default

[2025-04-24 10:08] Ruben Mike Litros: i also asked a friend to run it and he also got the same results as me

[2025-04-24 10:09] Ruben Mike Litros: but i dont know it could just be the two of us

[2025-04-24 10:10] diversenok: Ah, I see what you mean, we are both right

[2025-04-24 10:10] diversenok: The structure comes directly from the debug symbols, and there it's defined with a 32-bit counter

[2025-04-24 10:11] diversenok: But nested structures are pointer-aligned, so there is padding

[2025-04-24 10:11] Ruben Mike Litros: ohhh so its padding

[2025-04-24 10:12] Ruben Mike Litros: idk why i assumed it was packed structure

[2025-04-24 10:12] Ruben Mike Litros: my bad

[2025-04-24 10:12] Ruben Mike Litros: i casted the buffer pointer as a 32 big integer and increment it by one

[2025-04-24 10:14] diversenok: Makes sense. I thought you were another victim of a popular website that defines all NT types using a 32-bit layout 😅

[2025-04-24 10:15] Ruben Mike Litros: i think i know what ur talking about

[2025-04-24 10:16] Ruben Mike Litros: is that the one with no dark mode and the website looks like it was made in early 2000's

[2025-04-24 10:18] Ruben Mike Litros: http://undocumented.ntinternals.net/index.html?page=UserMode%2FUndocumented%20Functions%2FSystem%20Information%2FNtQuerySystemInformation.html

[2025-04-24 10:19] diversenok: ^ NEVER USE THAT WEBSITE 😱

[2025-04-24 10:19] diversenok: Sorry, needed to add a disclaimer 😂

[2025-04-24 10:20] Ruben Mike Litros: 😱

[2025-04-24 10:22] diversenok: Yeah, look at their SYSTEM_MODULE definition; half of the fields marked unknown, another half is at wrong offsets

[2025-04-24 10:26] Ruben Mike Litros: [replying to diversenok: "Yeah, look at their SYSTEM_MODULE definition; half..."]
http://undocumented.ntinternals.net/UserMode/Undocumented%20Functions/System%20Information/SYSTEM_MODULE_INFORMATION.html

[2025-04-24 10:26] Ruben Mike Litros: it says "NoSuchKeyThe specified key does not exist"

[2025-04-24 10:26] Ruben Mike Litros: btw

[2025-04-24 10:27] Ruben Mike Litros: [replying to diversenok: "https://ntdoc.m417z.com/rtl_process_modules"]
do you know what this is called in the debug symbols?

[2025-04-24 10:27] Ruben Mike Litros: i did dt nt!_RTL_PROCESS_MODULE_INFORMATION in windbg but it doesnt seem to exist

[2025-04-24 10:30] diversenok: That's the official name, but looks it's not defined in ntdll/ntoskrnl symbols

[2025-04-24 10:33] diversenok: Symbols for combase.dll have it

[2025-04-24 10:34] Matti: can you believe this is not the first time I've seen this exact same issue for the exact same reason (use of ntinternals types to query driver modules leading to '64 bit'  counts)

[2025-04-24 10:35] Ruben Mike Litros: [replying to Matti: "can you believe this is not the first time I've se..."]
i can ✋

[2025-04-24 10:35] Matti: good man

[2025-04-24 10:36] Matti: types are the only thing standing between us and total lawlessness and anarchism