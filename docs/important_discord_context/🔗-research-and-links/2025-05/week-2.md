# May 2025 - Week 2
# Channel: #🔗-research-and-links
# Messages: 38

[2025-05-05 14:54] diversenok: You guys went wild on this PEB searching challenge

[2025-05-05 14:58] diversenok: <@609487237331288074>  here is another idea for you (which I'm too lazy to implement) - after locating something inside ntdll via the stack pointers, pattern-search for the body of `RtlGetCurrentPeb` and call it 🙂

[2025-05-05 15:18] daax: [replying to diversenok: "<@609487237331288074>  here is another idea for yo..."]
heh that’s funny. can’t flag ntdll <a:think:895724772992876594>  new technique: the ~diversenokwalk~ 

<@162611465130475520> and I were talking about the many funnies that could be had by finding ntdll fncs on the stack

[2025-05-05 15:18] daax: personally, im just proud of the shifting arrays being viable so I could stick P, E, B in my code somewhere

[2025-05-05 15:20] diversenok: Hahah, how did I not notice that

[2025-05-05 16:40] mrexodia: Apparently the issue is that the module walking itself gets detected with some guard page or whatever, but 🤷‍♂️

[2025-05-05 17:12] DirtySecreT: [replying to mrexodia: "Apparently the issue is that the module walking it..."]
wym?

[2025-05-05 17:14] mrexodia: [replying to DirtySecreT: "wym?"]
This is what I was told

[2025-05-05 17:14] mrexodia: That some EDRs/AVs put a PAGE_GUARD on the PEB and then detecc

[2025-05-05 17:17] DirtySecreT: [replying to mrexodia: "That some EDRs/AVs put a PAGE_GUARD on the PEB and..."]
i see. id think depending on the application this would get a lot of inoffensif hits

[2025-05-05 17:18] mrexodia: yeah, 100%

[2025-05-05 19:00] mannyfreddy: [replying to mrexodia: "That some EDRs/AVs put a PAGE_GUARD on the PEB and..."]
S1 mods the DllBase in InMemoryOrderModuleList so when some payload or whatever parses the PEB the "normal way" it gets an address pointing towards S1's hooking dll instead, which has guard pages

[2025-05-06 07:44] mrexodia: [replying to mannyfreddy: "S1 mods the DllBase in InMemoryOrderModuleList so ..."]
the base of ntdll or all modules?

[2025-05-06 07:45] mannyfreddy: [replying to mrexodia: "the base of ntdll or all modules?"]
iirc only ntdll and kernel32

[2025-05-06 15:21] DirtySecreT: [replying to mannyfreddy: "S1 mods the DllBase in InMemoryOrderModuleList so ..."]
interesting, but onn the stack rtluserthreadstart or basethreadinitthunk are on the stack pointing to ntdll&kernel32. u could check against whats in the lists to see if theyre the same or different or not use the list since u can get ntdll base without it

[2025-05-06 15:21] DirtySecreT: that seems like such a weird strategy but ig if it works why not

[2025-05-06 15:22] x86matthew: there are also lots of PEB fields that point to stuff within the real ntdll

[2025-05-06 15:22] x86matthew: so you can get those and work backwards

[2025-05-06 15:23] DirtySecreT: [replying to x86matthew: "there are also lots of PEB fields that point to st..."]
true. i actually remember seeing you and the others posting these on twitter too lol

[2025-05-06 15:24] DirtySecreT: ngl it kind of puts a lot of these red teams to shame that theyve been recycling and somehow u guys post some funny stuff and it becomes an addition to the repertoire

[2025-05-06 15:25] DirtySecreT: kept seeing "obfuscated" get peb or teb or ntdll base or whatever for months on end

[2025-05-06 15:26] DirtySecreT: then suddenly new red team tricks that were right in front of them the whole time

[2025-05-06 15:26] DirtySecreT: no shade to them ig but i would think there'd be more creative ppl to find uncommon things

[2025-05-06 17:04] luci4: [replying to x86matthew: "there are also lots of PEB fields that point to st..."]
You can simply iterate through the linked list until you find an entry whose DllName matches "ntdll.dll". The name of S1's dll is something like "ntdl1.dll"

[2025-05-06 17:04] luci4: The only thing S1 catches by doing this is shitty shellcode that just traverses the list twice, which would normally correspond to ntdll

[2025-05-06 17:07] luci4: https://redops.at/en/blog/edr-analysis-leveraging-fake-dlls-guard-pages-and-veh-for-enhanced-detection
[Embed: EDR Analysis: Leveraging Fake DLLs, Guard Pages, and VEH for Enhanc...]

[2025-05-06 17:33] 0xatul: https://keowu.re/posts/Writing-a-Windows-ARM64-Debugger-for-Reverse-Engineering-KoiDbg/
[Embed: Writing a Windows ARM64 Debugger for Reverse Engineering - KoiDbg]
Author: João Vitor (@Keowu) - Security Researcher

[2025-05-06 19:06] mrexodia: [replying to DirtySecreT: "true. i actually remember seeing you and the other..."]
Yeah I poasted a poc stack walking to find ntdll

[2025-05-07 14:43] mrexodia: https://blog.jetbrains.com/clion/2025/05/clion-is-now-free-for-non-commercial-use/
[Embed: CLion Is Now Free for Non-Commercial Use | The CLion Blog]
CLion, a JetBrains IDE, is now free for non-commercial use! Learn more in the blog post.

[2025-05-07 14:43] mrexodia: This is pretty big!

[2025-05-07 18:30] stefan: [replying to DirtySecreT: "whats the hype behind getting the peb?"]
That is a good question lol...

[2025-05-08 01:12] DirtySecreT: [replying to stefan: "That is a good question lol..."]
im guessing <@609487237331288074> is just trolling lol

https://x.com/vxunderground/status/1920208595808821334
[Embed: vx-underground (@vxunderground) on X]
Daax, being the traditional memesteroni he is, shared a cool proof\-of\-concept which demonstrates how to get a pointer to the Process Environment Block without using the GS and/or FS register\.

Look

[2025-05-08 01:12] DirtySecreT: his thing works though which makes it a good troll at least hahaha

[2025-05-09 03:15] stefan: [replying to DirtySecreT: "his thing works though which makes it a good troll..."]
Yeah i imagine its inspired by the actual PEB slop that you encounter on twitter lol

[2025-05-10 17:44] Daniel: currently trying to reverse engineer the source code for Cheat Engine's DBVM. I'm super curious about how it works. I've made some progress, but am just getting started. if anyone wants to collaborate, feel free to let me know 🙂

[2025-05-10 17:48] diversenok: [replying to Daniel: "currently trying to reverse engineer the source co..."]
Isn't Cheat Engine open source? Or by reverse engineering you mean reading it?

[2025-05-10 17:48] Daniel: [replying to diversenok: "Isn't Cheat Engine open source? Or by reverse engi..."]
yeah, it's open source. i'm just trying to understand how it works. i probably shouldn't have used the term reverse engineer.

[2025-05-10 17:50] Daniel: 
[Attachments: Screenshot_2025-05-10_080546.png]