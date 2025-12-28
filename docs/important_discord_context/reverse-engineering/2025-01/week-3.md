# January 2025 - Week 3
# Channel: #reverse-engineering
# Messages: 15

[2025-01-14 09:22] broeder: how is `KeUserExceptionDispatcher` (not ki) set in ntoskrnl? i cant find xrefs to it and the old nt sources do it in `PspLookupKernelUserEntryPoints` but i think it doesnt do that anymore

[2025-01-14 09:41] x86matthew: [replying to broeder: "how is `KeUserExceptionDispatcher` (not ki) set in..."]
they're resolved in PspInitializeSystemDlls

[2025-01-14 09:41] x86matthew: there's a lookup table with export names and pointers

[2025-01-14 09:46] broeder: thanks👍

[2025-01-16 00:13] f00d: can u disable `__tabform`s when generating e.g. arrays in ida? or at least copying struct content without it

[2025-01-17 18:51] 0xc3.ard: I’m pretty sure it’s a dumb question bot how an application can call a winapi function that doesn’t seems to return anywhere to the application. Like the call stack doesn’t says anything who called it if that makes sense. Also when I’m stepping through the api call it jumps back and fourth the api function and the application’s code.

[2025-01-17 18:55] diversenok: Jumping back and fourth can happen when there is a callback parameter in the function which the function invokes before returning. Here is an example winapi function that uses this design: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-enumdesktopsw

[2025-01-17 18:56] diversenok: As for the other issue, stack walking might just be broken

[2025-01-17 20:58] 0xc3.ard: Hmm I don’t think that’s the case these are simple networking winapi functions. I’m confuseeeed.

[2025-01-19 05:04] expy: Hey guys, I've created a little bit of content for you. What do you think? Is it good or bad, if it's good enough what'd you suggest me to create next video about? Thanks 😉

https://youtu.be/LQo4ZWuaCW4
[Embed: Entry Level Keygen Me]
https://crackmes.one/crackme/671162d89b533b4c22bd160d

[2025-01-19 11:45] Glatcher: Guys, is there `dissect address as structure` feature in x64dbg like one presented in Cheat Engine?

[2025-01-19 13:26] mrexodia: [replying to Glatcher: "Guys, is there `dissect address as structure` feat..."]
there is something, recently I worked on https://github.com/x64dbg/DataExplorer
[Embed: GitHub - x64dbg/DataExplorer]
Contribute to x64dbg/DataExplorer development by creating an account on GitHub.

[2025-01-19 16:04] mrexodia: [replying to Glatcher: "Guys, is there `dissect address as structure` feat..."]
Just published a release for convenience https://github.com/x64dbg/DataExplorer/releases/tag/v1.0
[Embed: Release v1.0 · x64dbg/DataExplorer]
The DataExplorer plugin integrates the pattern language from ImHex into x64dbg.
Installation

Download the latest release
Install the plugin so you have:
<x64dbg-dir>/x64/plugins/DataExplorer...

[2025-01-19 17:18] MalcomVX: [replying to expy: "Hey guys, I've created a little bit of content for..."]
It’s not a bad idea, video tutorials for crackmes

[2025-01-19 17:19] MalcomVX: Lenas tutorials was big back in the day