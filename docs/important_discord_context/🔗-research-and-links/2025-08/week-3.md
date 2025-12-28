# August 2025 - Week 3
# Channel: #🔗-research-and-links
# Messages: 13

[2025-08-11 10:54] mrexodia: [replying to terraphax: "thank you! i was not aware of the term direct thre..."]
You might also enjoy https://tigress.wtf/virtualize.html

[2025-08-11 10:54] mrexodia: I think he coined the term 'superoperators'

[2025-08-11 10:54] mrexodia: ah no he didn't, some boomer did in 1995 https://dl.acm.org/doi/10.1145/199448.199526

[2025-08-11 10:55] mrexodia: there is also this funny meme I discovered while working on a vm: if you set the maximum amount of instruction to be combined into a superoperator to infinite, you basically create control flow flattening

[2025-08-11 13:00] naci: [replying to mrexodia: "there is also this funny meme I discovered while w..."]
I argue that VM's are just a fancy way to implement control flow flattening

[2025-08-11 13:03] mrexodia: Yeah almost. Traditionally control flow flattening has a dispatch variable that is set directly in the basic blocks. With a VM there is another level of indirection and parameterization if the bytecode also contains information about which vm register is accessed etc.

[2025-08-12 22:45] Wane: https://github.com/NeoMaster831/kurasagi
Windows 11 PatchGuard Runtime Bypass.
Huge credit to these works, they gave me a huge insight and approach for me.
+ https://shhoya.github.io/windows_pgintro.html
+ https://blog.can.ac/2024/06/28/pgc-garbage-collecting-patchguard/
+  https://blog.tetrane.com/downloads/Tetrane_PatchGuard_Analysis_RS4_v1.01.pdf
[Embed: GitHub - NeoMaster831/kurasagi: Windows 11 24H2 Runtime PatchGuard ...]
Windows 11 24H2 Runtime PatchGuard Bypass. Contribute to NeoMaster831/kurasagi development by creating an account on GitHub.
[Embed: PatchGuard Introduction | Shh0ya Security Lab]
Windows KPP Introduction
[Embed: PgC: Garbage collecting Patchguard away]
<p>I have released another article about Patchguard almost 5 years ago, ByePg, which was about exception hooking in the kernel, but let’s be frank, it didn’t entirely get rid of Patchguard

[2025-08-13 05:46] LabGuy94: https://github.com/LabGuy94/Diskjacker
https://readcc.net/posts/runtimehypervhijacking/

Hijacking Hyper-V at runtime using disk drives for DMA.
This is my first public RE related project so feedback and PRs are welcome and encouraged!
[Embed: GitHub - LabGuy94/Diskjacker: Runtime Hyper-V Hijacking with DDMA]
Runtime Hyper-V Hijacking with DDMA. Contribute to LabGuy94/Diskjacker development by creating an account on GitHub.
[Embed: Runtime Hyper-V Hijacking]
Using DDMA to hijack Hyper-Vs vmexit handler at runtime

[2025-08-14 10:53] mrexodia: https://zimperium.com/blog/the-rooting-of-all-evil-security-holes-that-could-compromise-your-mobile-device
[Embed: The Root(ing) Of All Evil: Security Holes That Could Compromise You...]
Learn how vulnerabilities in rooting frameworks like KernelSU can expose your Android device to severe security risks, and discover how Zimperium zLabs helps mitigate these threats.

[2025-08-14 16:13] 0xatul: https://media.defcon.org/DEF%20CON%2033/DEF%20CON%2033%20presentations/Sam%20Collins%20Marius%20Muench%20-%20Playing%20Dirty%20Without%20Cheating%20-%20Getting%20Banned%20for%20Fun%20and%20No%20Profit.pdf

<@609487237331288074> the academic thing I mentioned a while back

[2025-08-14 17:18] UJ: [replying to 0xatul: "https://media.defcon.org/DEF%20CON%2033/DEF%20CON%..."]
the slide that mentioned hypervisor based debugging and it was just 3 pics of blue  (black now) screens of death. 🤣

[2025-08-14 17:59] koyz: happens when you use a subpar hypervisor <:mmmm:904523247205351454>

[2025-08-14 19:35] mtu: Crosshair++ made me chuckle