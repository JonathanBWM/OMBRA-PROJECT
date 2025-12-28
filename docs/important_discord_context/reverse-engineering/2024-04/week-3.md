# April 2024 - Week 3
# Channel: #reverse-engineering
# Messages: 80

[2024-04-16 03:00] hxm: <@651054861533839370>  is it possible to get the VA using dtrace or ur script ?

[2024-04-18 17:03] TheXSVV: what kernel debuggers are there besides WinDbg? I want more gui than an inconvenient bunch of commands, and WinDbg itself is quite slow (my processor is intel i8 9700)

[2024-04-18 17:13] Matti: you want the debugger to have *more GUI*, but also be faster?

[2024-04-18 17:13] Matti: are you using new windbg? if so, switch to classic, it will run more than fine on that CPU

[2024-04-18 17:13] Matti: for windows there just aren't really any serious alternatives to windbg

[2024-04-18 17:14] TheXSVV: [replying to Matti: "you want the debugger to have *more GUI*, but also..."]
faster to fetch information from machine, WinDbg makes it too slow. 15 seconds to fetch exception code and its translation? WTF

Yes, i’m using new WinDbg

[2024-04-18 17:14] Matti: the softice days are long gone

[2024-04-18 17:14] Matti: [replying to TheXSVV: "faster to fetch information from machine, WinDbg m..."]
well this is weird, because things like this (not UI stuff) are exactly the things that are faster in new windbg for me than in old windbg

[2024-04-18 17:15] Matti: UI-wise though - old windbg wins

[2024-04-18 17:15] Matti: which KD transport are you using?

[2024-04-18 17:16] TheXSVV: serial port

[2024-04-18 17:16] Matti: yeah that'll be it

[2024-04-18 17:16] Matti: try kdnet

[2024-04-18 17:17] Matti: if kdnet is not an option, you can also try kdnet-EEM (ethernet emulation mode - uses USB)

[2024-04-18 17:17] Matti: it's very rare to need this though

[2024-04-18 17:18] 5pider: [replying to Matti: "for windows there just aren't really any serious a..."]
stupid question. is there a nt kernel debugger (doesnt need to be gui can also be cli) that allows me to debug from a linux host ? 
only way i know is to spin up two VMs. one is the dbger and the other is the kernel to be debugged.

[2024-04-18 17:19] Matti: EXDI, sort of

[2024-04-18 17:19] Matti: if you're OK with debugging qemu

[2024-04-18 17:19] Matti: other than that, plenty of VMs have gdb stubs too

[2024-04-18 17:19] Matti: that's what EXDI is built on in fact

[2024-04-18 17:20] Matti: if your target is a physical machine.... hmmm

[2024-04-18 17:20] Matti: unsure

[2024-04-18 17:20] 5pider: na vm should be it

[2024-04-18 17:20] Matti: alright

[2024-04-18 17:20] 5pider: thanks a lot

[2024-04-18 17:20] 5pider: i think i got it now

[2024-04-18 17:21] 5pider: found this. whihc is using qemu https://github.com/microsoft/WinDbg-Samples/blob/master/Exdi/exdigdbsrv/doc/ExdiGdbSrv_readme.md
[Embed: WinDbg-Samples/Exdi/exdigdbsrv/doc/ExdiGdbSrv_readme.md at master ·...]
Sample extensions, scripts, and API uses for WinDbg. - microsoft/WinDbg-Samples

[2024-04-18 17:21] 5pider: which i think is what you meant right ?

[2024-04-18 17:21] 5pider: i dont mind using qemu. what ever helps me achieve my goal of debugging windows kernel on a linux host

[2024-04-18 17:21] Matti: yeah

[2024-04-18 17:22] Matti: https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/configuring-the-exdi-debugger-transport
and
https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/setting-up-qemu-kernel-mode-debugging-using-exdi
are the docs I used for setting this up
[Embed: Configuring the EXDI Debugger Transport - Windows drivers]
Debugging Tools for Windows supports debugging using EXDI. This topic describes how to configure the EXDI transport.
[Embed: Setting Up QEMU Kernel-Mode Debugging Using EXDI - Windows drivers]
Debugging Tools for Windows supports debugging QEMU using EXDI. This topic describes how to setup QEMU kernel debugging using EXDI.

[2024-04-18 17:22] Matti: be warned; it's quite a bit of work

[2024-04-18 17:22] 5pider: thanks a lot mate i appreciate this a lot.

[2024-04-18 17:22] 5pider: [replying to Matti: "be warned; it's quite a bit of work"]
already guessed so

[2024-04-18 17:22] 5pider: still thanks a lot

[2024-04-18 18:20] 5pider: thank you kind stranger <:yara_lover:1148745271577157673>

[2024-04-19 08:05] Lewis: A good read: https://synthesis.to/2021/10/21/vm_based_obfuscation.html

[2024-04-20 01:14] abu: Hey all, does anyone have any resources or examples on hooking page fault exceptions on windows in the kernel. KiPageFault  related or anything else is fine.

[2024-04-20 01:24] szczcur: [replying to abu: "Hey all, does anyone have any resources or example..."]
much like any other hook

[2024-04-20 01:24] szczcur: if youre looking for specifics to do it without tripping patchguard

[2024-04-20 01:25] szczcur: i dont imagine people will just willingly give you that since thats a pretty powerful capability

[2024-04-20 01:27] szczcur: if youre using a hypervisor, you can trap #pf and then forward back to guest after you inspect or modify what you want

[2024-04-20 07:46] abu: [replying to szczcur: "if youre using a hypervisor, you can trap #pf and ..."]
might be the best bet at this point lol

[2024-04-20 07:46] abu: Can't find anything related to KiPageFault tbh

[2024-04-20 12:13] vendor: [replying to abu: "Hey all, does anyone have any resources or example..."]
unless they've patched it you can hook literally anything in the kernel with the ETW stuff

[2024-04-20 12:13] vendor: <https://revers.engineering/fun-with-pg-compliant-hook/>

[2024-04-20 12:14] vendor: but it depends on what you want to do with your hook if this will work for you

[2024-04-20 12:15] vendor: pretty sure the ETW event for page faults happens after it's been processed but i could be wrong

[2024-04-20 15:27] repnezz: Is there a windbg command to view parameters (so, register values context ) for a certain call in the middle of a callstack displayed by ‘k’? On x64

[2024-04-20 17:26] daax: [replying to vendor: "pretty sure the ETW event for page faults happens ..."]
yes, the page-fault ETW event occurs in MmAccessFault → EtwTracePageFault which is after a few things of potential interest have occurred. There's still plenty of fun to be had even though it's called after fault resolution.

[2024-04-20 18:23] abu: [replying to vendor: "unless they've patched it you can hook literally a..."]
Yeah I saw that as well but iirc it uses ETW and I want to dynamically resolve and handle the page fault but thanks 🙏

[2024-04-20 21:55] the eternal bleb: anyone got a good read to learn about gadgets and ROP? I read this and want to learn more about them https://secret.club/2021/01/12/callout.html
[Embed: Hiding execution of unsigned code in system threads]
Anti-cheat development is, by nature, reactive; anti-cheats exist to respond to and thwart a videogame’s population of cheaters. For instance, a videogame with an exceedingly low amount of cheaters wo

[2024-04-20 21:56] szczcur: [replying to the eternal bleb: "anyone got a good read to learn about gadgets and ..."]
<https://synthesis.to/papers/gadget_synthesis.pdf>
<https://fastercapital.com/content/ROP-Techniques--Mastering-the-Art-of-Ascending.html>

[2024-04-20 21:58] the eternal bleb: Ty

[2024-04-20 21:58] the eternal bleb: Bro provided me a whole book lol

[2024-04-20 22:02] daax: [replying to the eternal bleb: "Bro provided me a whole book lol"]
honcho always delivers <:PES_God:643089270537584680>

[2024-04-21 16:37] DeChaos: Does anyone know how TickCountMultiplier works on windows? like, on what circumstances does it change itself's value, etc

[2024-04-21 17:31] Azrael: [replying to DeChaos: "Does anyone know how TickCountMultiplier works on ..."]
> https://oxikkk.github.io/articles/re_gettickcount.html
[Embed: GetTickCount() implementation on Windows]
Read about how the GetTickCount() function exposed by the Win32 api is implemented under the hood.

[2024-04-21 17:35] Azrael: It doesn't change.

[2024-04-21 17:36] Azrael: > <https://www.geoffchappell.com/studies/windows/km/ntoskrnl/inc/api/ntexapi_x/kuser_shared_data/index.htm>

[2024-04-21 17:37] Azrael: This one is from NT 3.5. But I doubt that it has changed.

[2024-04-21 17:37] DeChaos: thanks, oh wow. so it must be my debugger's fault for capturing if ever, they spits wrong value for that then. i had some weird case that it's changing on my debugger's watch, so i was confused a lot

[2024-04-21 17:38] DeChaos: thanks for linking them!

[2024-04-21 17:40] Azrael: [replying to Azrael: "> <https://www.geoffchappell.com/studies/windows/k..."]
This site is very nice to casually browse.

[2024-04-21 17:41] Azrael: [replying to DeChaos: "thanks for linking them!"]


[2024-04-21 19:08] froj: [replying to Azrael: "> <https://www.geoffchappell.com/studies/windows/k..."]
Rip Geoff ❤️

[2024-04-21 19:12] Azrael: Truly.

[2024-04-21 19:13] Azrael: I first found him back in 2017 I think.

[2024-04-21 21:47] Azrael: Lmfao.

[2024-04-21 21:54] Azrael: I doubt that this is the place for hiring others, plus **game cracking** goes against rule number 6.

[2024-04-21 21:54] Azrael: But oh well.

[2024-04-21 22:21] Deleted User: [replying to Azrael: "I doubt that this is the place for hiring others, ..."]
for multiplayer games

[2024-04-21 22:21] Azrael: [replying to Deleted User: "for multiplayer games"]
That's an interesting rule, thanks.

[2024-04-21 22:22] Deleted User: its from discord TOS

[2024-04-21 22:22] Azrael: Never read it lol.

[2024-04-21 22:23] Deleted User: it doesnt make sense to forbid talking about reversing single player games 🤷‍♂️

[2024-04-21 22:23] Azrael: Ehh, that depends on the community I guess.

[2024-04-21 22:24] Azrael: Larger communities like TCCPP should definitely forbid it.

[2024-04-21 22:25] Deleted User: why <:kekw:904522300257345566>

[2024-04-21 22:33] Azrael: [replying to Deleted User: "why <:kekw:904522300257345566>"]
Because Discord has a shitty history of not being able to differentiate between single- and multi-player games.