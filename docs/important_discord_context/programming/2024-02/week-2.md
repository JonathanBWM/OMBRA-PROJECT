# February 2024 - Week 2
# Channel: #programming
# Messages: 53

[2024-02-05 10:26] donna🤯: Does anyone have experiencing implementing WMI in a wdm driver as a producer? Ive spent the entirety of today reading the docs / examples and I think I am even more confused now then when I initially started. Anyone know of any simple examples on github that arent using the wdf framework?

[2024-02-05 14:39] szczcur: [replying to donna🤯: "Does anyone have experiencing implementing WMI in ..."]
https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/registering-as-a-wmi-data-provider
[Embed: Registering as a WMI Data Provider - Windows drivers]
Registering as a WMI Data Provider

[2024-02-05 14:42] szczcur: the wmi provider details for drivers is somewhat undocumented. that’s about all you’ll find. the alternative is registering as an etw provider to provide information to anyone requesting it

[2024-02-05 16:15] donna🤯: [replying to szczcur: "the wmi provider details for drivers is somewhat u..."]
Yea I have read the documentation, but as you said yourself it is still quite hard to understand. There is a WDF provider example but I am wondering if there is a WDM example

[2024-02-05 16:15] donna🤯: or if anyone has experience with it since it seems quite overly complicated for what it seems to do

[2024-02-05 17:29] szczcur: [replying to donna🤯: "or if anyone has experience with it since it seems..."]
yeah i have done wmi provision, you mainly just have to register with the wmi subsystem using IoWMIRegistrationControl. which enables it to produce or respond to data requests. then you have a usermode “consumer” requesting or querying this data. actually you can do either a user module or km. the primary thing to know to make it work properly is that communication between a provider and a consumer is structured around “data blocks” which are schemas that describe and encapsulate the data and methods the provider supports. all of those are defined in a MOF (mgmt object format) file and compiled using the MOF compiler. 

then you have event blocks that drivers send to wmi subsystem when a consumer requests “notification”… an ex would be querying status of a storage device via wmi. the event blocks are basically notifications. you can find examples of them in the various .mof files in the system directory. 

you have to do some manual reversing of the wmi subsystem to figure out how it works. that’s why so few products enable it. the wrk has and winxp leak has references you can use such as the registration structure and the structures that reference event and data blocks for a given wmi instance/guid

IoWMIRegistrationControl does is tell the wmi subsystem hey this device object supports wmi. you can reverse a few of asus’ wdm drivers and see how they do it or use wdf.

you can find the relevant structures in combase or mbaeapicpublic pdbs

[2024-02-05 18:15] donna🤯: [replying to szczcur: "yeah i have done wmi provision, you mainly just ha..."]
Perfect - thankyou 😄

[2024-02-06 00:47] es3n1n: hey, <@162611465130475520>. Not sure whether this is the right place to ask questions about cmkr, but is there a way how can i define a template that i later want to use with different types? Like for example if i want to create a template that could be used both with a library and executable(for sharing compile definitions and other stuff mostly).

[2024-02-06 01:41] Timmy: [replying to es3n1n: "hey, <@162611465130475520>. Not sure whether this ..."]
https://github.com/build-cpp/wdk_template
[Embed: GitHub - build-cpp/wdk_template: Windows kernel driver template for...]
Windows kernel driver template for cmkr (with testsigning). - GitHub - build-cpp/wdk_template: Windows kernel driver template for cmkr (with testsigning).

[2024-02-06 01:42] es3n1n: hm? i don't see anywhere where it explicitly changes the type

[2024-02-06 01:42] es3n1n: how is this relevant

[2024-02-06 08:22] mrexodia: [replying to es3n1n: "hey, <@162611465130475520>. Not sure whether this ..."]
No, you would have to define the template twice.

[2024-02-06 08:23] mrexodia: But most likely you don’t need a template. You can use an interface target and “link” to that

[2024-02-06 08:31] 25d6cfba-b039-4274-8472-2d2527cb: So what you're saying is that they need a template template using jinja2 or something 👀 Sorry I'll go away.

[2024-02-06 20:08] mibho: https://i.gyazo.com/d0c04c5182fc6855f1a4df27876d938c.png
does anyone know how fix this? <:nickyoung:783940642945368104>

[2024-02-06 20:08] mibho: last resort is reinstalling vs lol

[2024-02-07 09:50] GregoryTheGrimmoth: are there any problems with using a 5 byte hook in the windows kernel to patch a function? (how made will patch guard get 😬)

[2024-02-07 10:17] Timmy: it'll be mad if its in ntoskrnl or other protected modules or pages.

[2024-02-07 11:27] Matti: depends on which 5 bytes precisely in ntoskrnl, but since it's 5 bytes (and not 4 or even 8), I can guess.... yes there will be problems with patchguard

[2024-02-07 11:28] Matti: (I'm assuming you're patching `.text` or some other code section due to the hook size)

[2024-02-07 11:37] Brit: in general patching text is a bonk

[2024-02-07 11:46] Matti: 💯

[2024-02-07 12:46] Deleted User: patch the patchguard

[2024-02-07 12:46] Deleted User: https://media.discordapp.net/attachments/1165753940588113980/1165968856616022096/shocked-black-guy-shocked.gif

[2024-02-07 12:49] Brit: simple whenever you would kebugcheck just don't

[2024-02-07 13:12] brymko: pro tip

[2024-02-07 13:12] brymko: replace the whole kernel with linux

[2024-02-07 13:12] brymko: and then shim wine into it

[2024-02-07 15:08] Timmy: [replying to brymko: "and then shim wine into it"]
everyone would if that worked for everything

[2024-02-07 15:09] Timmy: I certainly would

[2024-02-07 23:35] GregoryTheGrimmoth: [replying to Timmy: "it'll be mad if its in ntoskrnl or other protected..."]
so everything useful

[2024-02-07 23:35] GregoryTheGrimmoth: 😭

[2024-02-08 00:26] Matti: if it makes you feel better, patchguard also protects things that are completely useless, such as win32k

[2024-02-08 00:30] GregoryTheGrimmoth: i love win32k u can use it to change the global cursor value 👍

[2024-02-08 00:34] Matti: that's what I'm saying

[2024-02-08 00:42] GregoryTheGrimmoth: do u know any unprotected functions that get called by user mode functions?

[2024-02-08 00:49] Matti: sure

[2024-02-08 00:49] Matti: every function that gets called by a user mode function is unprotected by patchguard

[2024-02-08 00:50] Matti: that's not counting system calls, which are protected by patchguard.... because they're part of the kernel

[2024-02-08 02:01] GregoryTheGrimmoth: wait so i can hook something like NtWriteFile and patchguard wont care

[2024-02-08 02:03] diversenok: In user mode, yes

[2024-02-08 02:03] diversenok: You can do whatever you want there

[2024-02-08 07:06] Torph: [replying to GregoryTheGrimmoth: "wait so i can hook something like NtWriteFile and ..."]
I mean yeah, isn't that just ntdll? I hooked `CreateFile2()` for a project, it was super easy and worked fine

[2024-02-08 16:37] Kynes: Is there any Windows API to allocate page aligned memory in Usermode?

[2024-02-08 16:41] Timmy: p sure VirtualAlloc qualifies

[2024-02-08 16:42] Kynes: I can only see support for `MEM_LARGE_PAGES` on MSDN unless I'm blind

[2024-02-08 16:43] Kynes: nvm

[2024-02-08 17:08] Matti: if page aligned is all you need, virtualalloc will always give you that

[2024-02-08 17:08] Matti: if you need more you can also use the ZeroBits parameter to NtAllocateVirtualMemory

[2024-02-10 21:07] prick: anyone know the specifics of why this doesn't work? allocating on top of a region that just got freed
[Attachments: image.png]

[2024-02-10 21:21] x86matthew: MEM_RESERVE | MEM_COMMIT

[2024-02-10 21:22] x86matthew: you can omit the reserve flag when an address is not explicitly specified (but you shouldn't)

[2024-02-10 21:23] x86matthew: but this won't work if a fixed address is specified