# July 2025 - Week 3
# Channel: #programming
# Messages: 76

[2025-07-14 16:46] BWA RBX: [replying to GG: "hello all, does any one have access to this repo ?..."]
Does the README contain this 

```
# Clang loop pragma example

This repository contains an example for registering a custom clang pragma as loop annotation.
It registers `#pragma loop_info unrollcount(4)`. 
The option `unrollcount` is checked to be available for the pragma `loop_info`.
It is checked that the provided option value `4` is an integer.

## How to use the provided example

In order to make use of this example, a recent clang/llvm version is necessary.
You can find related information in the [LLVM documentation](https://llvm.org/docs/GettingStarted.html).
For the sake of simplicity, we assume you have a recent clang version installed.

### Build the plugin

First, you have to build the provided code as clang plugin.
We provide a `CMakeLists.txt` for this purpose.
sh
mkdir build
cd build
cmake ..
make
```

[2025-07-14 16:51] BWA RBX: [replying to GG: "hello all, does any one have access to this repo ?..."]
If so Google Gemini has it cached

[2025-07-15 23:28] estrellas: Is there any other options besides capstone (Python) that can disassemble multiple architectures? ARM/AArch64 options are also welcomed, then I could use iced for x86 and another lib for the previously said ones

[2025-07-16 00:21] the horse: `then I could use iced for x86 and another lib for the previously said ones`

Just make a unified wrapper for capstone & iced

[2025-07-16 00:23] estrellas: I didn't _really_ wanna deal with capstone, their docs are bad and I've disliked the usage so far

[2025-07-16 00:39] the horse: it's not that bad, but i'd probably want to hang myself if I used python as well

[2025-07-16 01:58] Arckmed: [replying to estrellas: "Is there any other options besides capstone (Pytho..."]
perhaps https://github.com/emproof-com/nyxstone ?
[Embed: GitHub - emproof-com/nyxstone: Nyxstone: assembly / disassembly lib...]
Nyxstone: assembly / disassembly library based on LLVM, implemented in C++ with Rust and Python bindings, maintained by emproof.com - emproof-com/nyxstone

[2025-07-16 12:30] valium: thank you

[2025-07-16 16:08] NSA, my beloved<3: Shouldn't you be seeing your kernel driver's DbgPrint-s in WinDbg (local kernel debug) or DebugViewer? Is there something I am missing?

[2025-07-16 16:24] NSA, my beloved<3: Oh yeah, read something about that. I'll try it thanks.

[2025-07-16 16:24] NSA, my beloved<3: Read something about this as well, is this something other than bdedit debugging on?

[2025-07-16 16:27] NSA, my beloved<3: Oh yeah, tried that one, did not work unofrtunately.

[2025-07-16 16:27] NSA, my beloved<3: I'll try the registry.

[2025-07-16 16:27] NSA, my beloved<3: Thanks, I will!

[2025-07-16 16:34] NSA, my beloved<3: Well, set both to 0xf then 0x8, none of them worked. Could it be because I am starting my driver throughput sc.exe as a service?

[2025-07-16 16:41] UJ: [replying to NSA, my beloved<3: "Well, set both to 0xf then 0x8, none of them worke..."]
Are you sure you are using DbgPrint or KdPrint (which gets compiled away in release builds)?

[2025-07-16 16:42] NSA, my beloved<3: [replying to UJ: "Are you sure you are using DbgPrint or KdPrint (wh..."]
Yes, to make sure I am using both.

[2025-07-16 16:42] NSA, my beloved<3: Using the boilerplate code from https://learn.microsoft.com/en-us/windows-hardware/drivers/gettingstarted/writing-a-very-small-kmdf--driver along with an additional DbgPrint inside DriverEntry.
[Embed: Write a Hello World Windows Driver (Kernel-Mode) - Windows drivers]
How to write a Windows driver using Kernel-Mode Driver Framework (KMDF). You start with a Microsoft Visual Studio template.

[2025-07-16 16:45] NSA, my beloved<3: Shouldn't I be seeing lots of debug messages in DebugViewer anyways?

[2025-07-16 16:45] UJ: what i usually do to enable dbgprint logs in windbg is to run debugview as admin and it just enables it for me.

[2025-07-16 16:45] NSA, my beloved<3: Both WinDbg and DebugViewer are empty.

[2025-07-16 16:45] UJ: under capture in debugview, make sure capture kernel is selected and the cog icon doesnt have a red x on it

[2025-07-16 16:46] NSA, my beloved<3: Yep. That's how it is.

[2025-07-16 16:46] NSA, my beloved<3: How are you loading/unloading your driver? It must be that...

[2025-07-16 16:46] UJ: just sc create/sc start.

[2025-07-16 16:47] NSA, my beloved<3: But again, it's weird that both are emtpy. I've read that I should be seeing lots of junk messages.

[2025-07-16 16:47] UJ: Yeah debugview should at least be showing messages from other drivers

[2025-07-16 16:48] NSA, my beloved<3: What's even funnier is it keeps crashing.

[2025-07-16 16:48] NSA, my beloved<3: Test signing is on, along with no integrity checks. Not sure if that's causing it.

[2025-07-16 16:50] UJ: another dumb question, you are running dbgview64 version as admin right?

[2025-07-16 16:51] NSA, my beloved<3: Yep. 😕

[2025-07-16 16:51] NSA, my beloved<3: I'll try the other versions.

[2025-07-16 17:08] NSA, my beloved<3: Could it be because I've set up network kernel debugging and even though it does not work, it still hijacks all the debug messages?

[2025-07-16 17:52] Matti: it's weird that this article is talking about DEFAULT, unless this is secretly an alias for WIN2000, which I'm pretty sure it's not since they're located at different addresses and can have different values

[2025-07-16 17:53] Matti: for reasons long lost to time, `Kd_WIN2000_Mask` is the one you actually want to set to 0xf or 0x1f to get the behaviour the article is talking about

[2025-07-16 17:54] Matti: whether you use the debugger or the registry for this doesn't matter, but the registry does require a reboot

[2025-07-16 17:55] Matti: the WIN2000 mask is different from all the other named masks, since it is (1) not in `KdComponentTable[]` (rather it gets special treatment for plain DbgPrint calls, hardcoded by this mask), and (2) it is the only mask with a nonzero initial value (= 1)

[2025-07-16 17:56] Matti: really though, if this is your own code, why not use DbgPrintEx instead and print at DPFLTR_ERROR_LEVEL

[2025-07-16 18:41] NSA, my beloved<3: [replying to Matti: "really though, if this is your own code, why not u..."]
Yes, I've stumbled upon this: https://www.unknowncheats.me/forum/c-and-c-/205321-debugview-dbgprint.html and they suggest the same thing. I'll try it now. 🙂
[Embed: DebugView not showing DbgPrint]
Hello UC. I have had a problem trying to output DbgPrint's in DebugView with a simple driver. The driver loads properly but no message is shown in Deb

[2025-07-16 18:42] Matti: great reading material, thanks

[2025-07-16 18:43] Matti: you know this is simply documented on MSDN right

[2025-07-16 18:43] NSA, my beloved<3: No. Searching for the issue brought this up, but not MSDN.

[2025-07-16 18:44] Matti: https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/reading-and-filtering-debugging-messages
[Embed: Reading and Filtering Debugging Messages - Windows drivers]
Reading and Filtering Debugging Messages

[2025-07-16 18:44] Matti: https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-dbgprintex
[Embed: DbgPrintEx function (wdm.h) - Windows drivers]
The DbgPrintEx routine sends a string to the kernel debugger if the conditions you specify are met.

[2025-07-16 18:45] Matti: as well as the DbgPrint page stating
> It is recommended that you use DbgPrintEx instead of DbgPrint, since this allows you to control the conditions under which the message is sent.

[2025-07-16 18:45] NSA, my beloved<3: Cheers! I'll just replace the functions then, rather than messing around with the registry. Seems more logical.

[2025-07-16 18:47] NSA, my beloved<3: I can't believe this... Still nothing in WinDgb...

[2025-07-16 18:57] NSA, my beloved<3: Okay so, even though I recompiled my driver, the service was still loading an old version of it. Which is a bit odd, since looking up the service in the registry shows the ImagePath is set to the one where the newly compiled one is put.

[2025-07-16 18:57] NSA, my beloved<3: Updating my code resulted in no changes.

[2025-07-16 18:58] NSA, my beloved<3: I have to keep deleting the service, restarting, creating it and restarting again.

[2025-07-16 18:58] NSA, my beloved<3: Not to mention that loading/unloading it is not possible without a reboot, which is really annoying.

[2025-07-16 18:58] NSA, my beloved<3: What's also annoying is how DebugView just keeps closing its self.

[2025-07-16 19:01] NSA, my beloved<3: [replying to NSA, my beloved<3: "Okay so, even though I recompiled my driver, the s..."]
Nevermind. Visual studio is not compiling it even though it says it does.

[2025-07-16 19:02] Matti: [replying to NSA, my beloved<3: "Nevermind. Visual studio is not compiling it even ..."]
yeah and that is because

[2025-07-16 19:02] Matti: setting the driver binary path to your literal compiler's output path is a bad idea for multiple reasons, including linker errors because you forgot to unload the driver, or because windbg still has the PDB loaded (that is also a fatal error FYI). just don't do this. make a postbuild step to copy to system32\drivers instead or something if needed

[2025-07-16 19:03] Matti: [replying to NSA, my beloved<3: "Not to mention that loading/unloading it is not po..."]
wdym 'isn't possible'

[2025-07-16 19:03] Matti: `sc stop` unloads a driver

[2025-07-16 19:03] Matti: so what error does it return?

[2025-07-16 19:04] NSA, my beloved<3: Yeah, yields the error: The requested control is not valid for this service.

[2025-07-16 19:04] NSA, my beloved<3: Not going to lie, googled it, but didn't spend much time digging into it.

[2025-07-16 19:04] Matti: OK, so implement a driverunload dispatch function perhaps, and make your driver object passed to DriverEntry point to it

[2025-07-16 19:05] NSA, my beloved<3: [replying to Matti: "setting the driver binary path to your literal com..."]
All right, I'll apply this, thank you.

[2025-07-16 19:05] NSA, my beloved<3: Okay so this can be fixed programatically and is not something service setup-related. I'm happy to hear that.

[2025-07-16 19:06] NSA, my beloved<3: Thank you for the help and the valuable information you shared. 🙂

[2025-07-16 19:07] NSA, my beloved<3: I'll include it, thank you!

[2025-07-19 14:26] elias: The most useful error message I've seen in a while
[Attachments: image.png]

[2025-07-20 03:46] selfprxvoked: <:skill_issue:1210171860063617074>

[2025-07-20 03:46] selfprxvoked: After 5+ years using only MSVC you start understanding errors <:cooding:904523055374676019>

[2025-07-20 10:18] elias: The point is you dont have to google or guess what lib contains some random unresolved symbol

[2025-07-20 16:51] mtu: That’s why I just vibe compile

How often are you pasting code with external libs that linker errors are a large part of your problem 🧐 I agree linker errors don’t provide a lot of info (how would they, they don’t know what the symbol is much less what it’s supposed to be from)

[2025-07-20 17:41] contificate: lots of libs use naming conventions so it's kind of obvious

[2025-07-20 19:48] mtu: To your grey matter with outside context yeah, but ld/msvc doesn’t ship with heuristics mapping function calls to libraries for good reason

[2025-07-20 19:56] contificate: this situation only really arises if you're building other peoples' code

[2025-07-20 19:56] contificate: otherwise you'd know what libs you intend to use (and how you're planning to link them)

[2025-07-20 19:56] contificate: tbh I never have any problems

[2025-07-20 19:56] contificate: I just go "oh yeah gotta link libfoo because clearly foo_bar is in that, as I've intended"