# September 2025 - Week 3
# Channel: #🔗-research-and-links
# Messages: 7

[2025-09-19 09:12] pygrum: https://security.apple.com/blog/memory-integrity-enforcement/
[Embed: Memory Integrity Enforcement: A complete vision for memory safety i...]
Memory Integrity Enforcement (MIE) is the culmination of an unprecedented design and engineering effort spanning half a decade that combines the unique strengths of Apple silicon hardware with our adv

[2025-09-19 11:44] suleif: [replying to pygrum: "https://security.apple.com/blog/memory-integrity-e..."]
I saw this the other day, I wonder how much will it actually reduce exploitation though

[2025-09-19 12:17] pygrum: just another hurdle i think

[2025-09-19 12:21] pygrum: https://arxiv.org/pdf/1802.09517

[2025-09-19 12:24] pygrum: They say themselves if the tag size is 4 bits there’s a 1 in 16 chance memory gets the same tag if assignments random

[2025-09-19 12:24] Brit: apple has been doing some real good work with security, with banning reprotection of memory and tagged pointers a la PAC and this new MIE stuff

[2025-09-19 12:48] mtu: MIE/MTE effectively prevents reliable heap overflow exploits, where the object you control isn’t of interest so you try to write to another chunk/chunk metadata. A 1/16 hit rate is noisy, since the 99% case there is “cpu fault crashes target application”.

It’s sort of like how stack canaries prevent you from writing the return pointer, but you can still muck around within the stack frame. There’s still exploits to be had, just harder to do