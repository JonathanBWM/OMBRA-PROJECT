# October 2025 - Week 1
# Channel: #🔗-research-and-links
# Messages: 13

[2025-10-01 10:48] 0xdeluks: not sure if this fits in here but I reverse engineered my first VM a few days ago :))

https://deluks2006.github.io/Reverse-Engineering/Breaking-TadpoleVM
[Embed: Breaking TadpoleVM]
Reverse engineering a virtual machine with a custom instruction set With the rising popularity of virtual machines as an anti-analysis technique such as VMProtect & Themida, more and more malware samp

[2025-10-01 16:34] daax: [replying to 0xdeluks: "not sure if this fits in here but I reverse engine..."]
nice work on first vm. have you given any others a look since? finfisher samples might be fun for you to look at if you’re starting out with these

[2025-10-01 16:35] 0xdeluks: [replying to daax: "nice work on first vm. have you given any others a..."]
havent taken a look at any others yet, just starting out. thanks for the recommendation :)

[2025-10-01 21:04] Sirmabus: Updated my main IDA plugins to 9.2: https://github.com/kweatherman
[Embed: kweatherman - Overview]
kweatherman has 14 repositories available. Follow their code on GitHub.

[2025-10-02 10:17] Xits: Is there a comprehensive list of all the instructions somewhere?

[2025-10-02 10:19] Xits: I guess they might be in that 1997 patent

[2025-10-02 10:24] Xits: Also I would’ve thought that sandsifter would find those instructions 🤔

[2025-10-02 10:24] Brit: https://haruspex.can.ac/

[2025-10-02 10:25] Brit: but also, xed handles these just fine

[2025-10-02 10:26] Brit: they all fall under XED_CATEGORY_WIDENOP

[2025-10-02 10:28] Brit: Seems like a bit of an oversight to not try the actual intel disassembler

[2025-10-02 10:31] Xits: [replying to Brit: "https://haruspex.can.ac/"]
Cool project. Thanks for sharing

[2025-10-02 23:15] daax: [replying to Xits: "Is there a comprehensive list of all the instructi..."]
Yes, and the paper is wrong hence why it was deleted. Plenty of disassemblers handle it. IDA hasn’t, Binja hasn’t either but it’s not real software, same with Ghidra. There are numerous ways to desync them in similar ways and none of them warrant a paper being written about it. It’s already in a patent.