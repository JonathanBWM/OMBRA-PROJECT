# July 2025 - Week 2
# Channel: #🔗-research-and-links
# Messages: 19

[2025-07-08 18:03] Lyssa: https://massgrave.dev/blog/pesky-ampersands
[Embed: The &s that (temporarily) killed TSforge | MAS]
Introduction

[2025-07-10 15:30] daax: https://skemman.is/bitstream/1946/50456/1/Challenges_and_Pitfalls_while_Emulating_Six_Current_Icelandic_Household_Routers.pdf

[2025-07-10 15:31] daax: > Evaluation of different tools for routers firmware emulation
> (FACT, QEMU, EMUX, Qiling, Firmadyne, FAT, FirmAE, Pandawan, and EMBA)
> 
> "Challenges and Pitfalls while Emulating Six Current Icelandic Household Routers"

[2025-07-11 18:17] Humza: https://github.com/MicroOperations/PredecodeRE
[Embed: GitHub - MicroOperations/PredecodeRE: Analysis of goldmont plus pre...]
Analysis of goldmont plus predecode cache logic. Contribute to MicroOperations/PredecodeRE development by creating an account on GitHub.

[2025-07-11 18:50] daax: [replying to Humza: "https://github.com/MicroOperations/PredecodeRE"]
> The absence of a micro op cache is due to goldmont and goldmont plus being power optimised microarchitectures. A micro op cache incurs overhead when there is a switch from fetching directly from the micro op cache to the legacy decoder
uop caches are typically added to save power since it lets the large decode pipeline power-gate while hot loops are served from the cache; and the miss-re-enable penalty is only one cycle. the cost is die area and potential leakage power constraints, not dynamic power from switching.

[2025-07-11 18:51] daax: it was left out probably because it isn't justifiable in a smaller low pwr design

[2025-07-11 18:52] Humza: [replying to daax: "> The absence of a micro op cache is due to goldmo..."]
I see, thanks for this

[2025-07-11 18:53] Humza: The thing is however the predecode cache itself is 64kb, which is quite large, so wouldn’t a micro op cache be a better option in terms of die area too? They tend to be much smaller in size

[2025-07-11 19:03] daax: [replying to Humza: "The thing is however the predecode cache itself is..."]
simplicity and area efficiency would make sense for those processors designs. predecode cache can handle required operations without the complexity and area overhead that a uop cache has. the uop cache is going to need lots more logic for tag arrays, replacement, coherency, etc. area efficiency depends heavily on the target use too, perf constraints | requirements, core design, overall whatever their idea is for keeping things efficient "enough" and cost (not necessarily $) low. could be they were willing to accept a larger raw cache size in exchange for reduced complexity overall. point wasn't that one is better than the other (it always depends on goals/design; and one is a complement to the other generally), but that the statement of it incurring overhead and being less power efficient isn't accurate.

[2025-07-11 19:04] Humza: [replying to daax: "simplicity and area efficiency would make sense fo..."]
I see

[2025-07-11 19:04] Humza: Thanks for letting me know this, I’ll update my writeup accordingly

[2025-07-11 19:10] daax: [replying to Humza: "Thanks for letting me know this, I’ll update my wr..."]
a paper that details uop caches complexity and power savings etc and the different designs and their requirements
[Attachments: MICRO_2020.pdf]

[2025-07-11 19:21] daax: <@547086439138066432> also worth comparing:

<https://patents.google.com/patent/US20200285466A1/en> compact uop cache implementation - AMD
<https://patents.google.com/patent/US8103831B2/en> uop cache implementation - Intel (the complexity is sort of implied given all the queues and matching, replacement, etc)
<https://patents.google.com/patent/US5689672A/en> predecode icache (complexity is shown in the images 10-12) - AMD

[2025-07-11 19:22] daax: might better explain/visualize what i meant by complexity and area efficiency

[2025-07-11 19:25] Humza: [replying to daax: "<@547086439138066432> also worth comparing:

<http..."]
Thanks for these

[2025-07-11 19:25] Humza: But yeah, I understand what you mean

[2025-07-11 19:27] daax: [replying to Humza: "Thanks for these"]
np. good work though! uarch research is always a rabbit hole

[2025-07-11 19:27] Humza: [replying to daax: "np. good work though! uarch research is always a r..."]
Thanks for ur help, and for the resources also, will definitely be giving them a read

[2025-07-12 11:26] koyz: did some thinking and some typing. out came a new blog post:
https://invlpg.dev/post/debug_message_hooks/
[Embed: 0x40010006 Ways to Hook Your APIs – invlpg | technical blog]
a blog about exploring reverse engineering, hypervisor development and software programming.