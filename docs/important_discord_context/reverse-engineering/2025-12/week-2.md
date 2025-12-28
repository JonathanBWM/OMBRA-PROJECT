# December 2025 - Week 2
# Channel: #reverse-engineering
# Messages: 55

[2025-12-08 15:59] eversinc33: someone familiar with this tool lmao? I found it on VT hunting for hypervisor rootkits but didnt find any references online
[Attachments: image.png]

[2025-12-08 16:00] eversinc33: some hypervisor based debugger

[2025-12-09 00:16] Jacob: [replying to eversinc33: "someone familiar with this tool lmao? I found it o..."]
Better hope you can read Chinese

[2025-12-09 00:16] Jacob: Or Indonesian?

[2025-12-09 21:24] zero: yo how do you guys feel about binary ninja

[2025-12-09 21:37] Mikewind22: [replying to zero: "yo how do you guys feel about binary ninja"]
It is OK for free software...

[2025-12-09 21:39] zero: any words on the paid version

[2025-12-09 21:59] Brit: I like it

[2025-12-09 21:59] Brit: very nice api

[2025-12-10 02:30] The_Toolsmith: [replying to zero: "any words on the paid version"]
I've let my subscription lapse a while ago (but that should in no way be construed as judgement - I just didn't get to do much RE), but have been following their update/functionalities journey, and might become interested again.
I loved the game-engine CTFs V35 did put out, and I quite liked Jordan's presentation on breaking decompilers earlier this year - as far as the company and its people go, they seem a good bunch.
If you want a quick comparison of just the decompilation functionality, you're probably aware of https://dogbolt.org/
[Embed: Decompiler Explorer]
Decompiler Explorer is an interactive online decompiler which shows equivalent C-like output of decompiled programs from many popular decompilers.

[2025-12-10 04:23] Woah.: [replying to grb: "i tried to hook win32k syscalls using InfinityHook..."]
Patchguard?

[2025-12-10 15:31] 0xBera: [replying to eversinc33: "someone familiar with this tool lmao? I found it o..."]
Learn chinese and master reverse engineering

[2025-12-11 16:01] 0xboga: Is it possible to load a driver from a network share? Will an AV still see the driver being “dropped” in a minifilter?

[2025-12-11 18:39] Pwnosaur: [replying to 0xboga: "Is it possible to load a driver from a network sha..."]
Interesting question, consider me naively thinking about this, but are you referring to an AV as in one that only checks signatures ? because if it is using notification routines to monitor for loading/unloading of driver and mapping them to memory, I believe it wouldn't matter where it came from. That's assuming it's a loadable driver in non testing mode and all those new measures out there to prevent loading of unsigned or unsanctioned drivers.

I'm assuming that you are asking this for detection stuff ( don't know why I thought that , your question was specific ), but otherwise that would be interesting question, if it's just about whether the minifilter will see it or not, maybe ? maybe it has to create a local temporary copy and load it to create the memory mapping to it ?

[2025-12-11 20:00] BloodBerry: [replying to 0xboga: "Is it possible to load a driver from a network sha..."]
Due to my knowledge the AV checks all the files itself when u: downloading, manual mapping, copy between 2 folders (ESET Making it 100%) others untested

[2025-12-11 20:01] BloodBerry: Pretty sure it could be detected

[2025-12-11 20:04] Pwnosaur: Well <@380963698694684684> I think we are both turning on the security/defender mindset and thinking about detection (habitual stuff), but I think <@836981274496991252> maybe just curious what would happen in the case you try to load a driver from network share , if it would show up to the minifilter or not ...

[2025-12-11 21:20] 0xboga: [replying to Pwnosaur: "Interesting question, consider me naively thinking..."]
Yea sure it will see the image load but from my experience a file drop event, especially if it’s a PE, triggers different scans than what an image load event would. Honestly it is about detection but I’ll handle the detection details myself, my question here is rather a technical one - will I/O to a network share be visible to most AV minifilters? I’m aware you can attach to \device\mup on instance setup, can this be useful? Or will they have different alternatives to have visibility into such I/O? or none at all

[2025-12-11 21:35] Pwnosaur: Not too sure honestly, but I doubt you can load a driver from network share even without AV interference, it’s unlike regular binaries, it could cause system instability , I’m thinking about this because of demand paging and lazy loading stuff, in case of a normal binary it would just crash no issues, but a driver ….

[2025-12-11 21:36] Pwnosaur: Also a driver isn’t really something you execute or load over network on regular basis, but independent executables maybe 🤔

[2025-12-11 21:38] Pwnosaur: I’m actually curious what would happen if a binary is being lazy loaded over network that is suddenly disconnected, or how to trigger this behavior

[2025-12-11 21:39] the horse: a driver won't load if it's partial

[2025-12-11 21:39] Pwnosaur: That makes sense

[2025-12-11 21:39] the horse: with normal usermode binaries, what do you mean lazy loaded?

[2025-12-11 21:39] the horse: sort of a hit page -> stream contents if not hit yet?

[2025-12-11 21:40] the horse: if so, you'd timeout and crash

[2025-12-11 21:40] the horse: or infinitely hang if there is not a timeout mechanism

[2025-12-11 21:40] Pwnosaur: Yes, it defers loading the entire thing until a page fault happens and it goes to the disk and load the rest of it

[2025-12-11 21:40] the horse: likely throws an access violation

[2025-12-11 21:40] the horse: but i'm pretty sure you'd normally have to stream the full bin to temp

[2025-12-11 21:40] the horse: before it would execute in a normal manner

[2025-12-11 21:41] Pwnosaur: Well I mean that should be true, but who knows 😂😂 there is a lot of weird stuff in the windows kernel

[2025-12-11 21:41] Pwnosaur: They call it features, I call it whatever

[2025-12-11 21:42] the horse: a windows driver will always have to be loaded in full because it won't pass the authenticode checks otherwise, and most of drivers are non-paged

[2025-12-11 21:42] the horse: they might have a paged section though

[2025-12-11 21:43] the horse: like INIT (or discardable) sections

[2025-12-11 21:44] the horse: I'd assume something similar would apply to binaries during streaming for security reasons

entire executable gets streamed, checksum is checked for the transfer, and it's launched

[2025-12-11 21:45] Pwnosaur: Maybe but if so , would it be streamed directly to memory or stored temp on disk somewhere?

[2025-12-11 21:45] Pwnosaur: I guess on disk , especially for large binaries

[2025-12-11 21:45] the horse: definitely on disk after the transfer is complete, perhaps during if it's too large

[2025-12-11 21:46] the horse: realistically enterprise software won't just manual map binaries to get them to load without disk store

[2025-12-11 21:46] the horse: would introduce a lot of compatibiltiy issues

[2025-12-11 21:48] the horse: streamed into memory first -> during transfer you'd watch for ACKs during transfer in case a chunk drops and the server will transfer that chunk again or continue after it -> also checks tcp header checksum for the packet

[2025-12-11 21:49] Pwnosaur: I mean, maybe if we look into the parent path were the binary was started from that would shed some light 🤔

[2025-12-11 21:49] the horse: regarding the original question i am fairly sure you practically need to do a copy to temp for the driver to actually "load" from network share

[2025-12-11 21:50] the horse: and that's probably what happens under the hood

[2025-12-12 01:32] daax: [replying to 0xboga: "Is it possible to load a driver from a network sha..."]
yes and yes

[2025-12-13 17:42] 0xBera: Working on Space Sniffer right now 👀

[2025-12-13 19:59] brymko: [replying to 0xBera: "Working on Space Sniffer right now 👀"]
whats that

[2025-12-14 12:17] 'Snyder: [replying to brymko: "whats that"]
to check which files or folders take up space

[2025-12-14 12:21] the horse: ever since i heard of wiztree i see no reason for another

[2025-12-14 14:47] plpg: Windirstat or something

[2025-12-14 18:00] Titoot: [replying to plpg: "Windirstat or something"]
wiztree is way way better tho

[2025-12-14 18:06] ImagineHaxing: File explorer on top

[2025-12-14 18:07] ImagineHaxing: I love how missguiding the file explorer last changed field is