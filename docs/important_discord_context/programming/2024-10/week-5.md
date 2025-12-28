# October 2024 - Week 5
# Channel: #programming
# Messages: 35

[2024-10-28 01:47] IliDili: [replying to 0x208D9: "https://github.com/angr/angr-management"]
Very good, I'm testing it on Windows 11, I noticed that when it's analyzing it only uses one Thread, maybe if it used more than one thread it would be faster, I thought about the idea of ​​using the GPU in some way to help with the analysis of the binaries, I don't know if that's possible.

[2024-10-28 08:47] 0x208D9: [replying to IliDili: "Very good, I'm testing it on Windows 11, I noticed..."]
well i do think u can utilize gpu to make analysis faster (partiularly in places where MBA is used heavily and CFG is obfuscated beyond recovery) <--- hypothetically but practical results havent shown much improvements over the traditional methods

[2024-10-28 08:49] 0x208D9: pretty similar to yk "using AI for fuzzing" lmaoooo

[2024-10-28 08:49] 0x208D9: ^^ technically corpus improvement and for input mutation purposes so it gets better code coverage but whatever kek

[2024-10-28 15:18] James: Throwing more compute at mbas won’t really help you

[2024-10-28 15:22] Brit: one day people will invent egraphs

[2024-10-28 16:06] 0x208D9: https://cdecl.org

[2024-10-28 16:07] 0x208D9: [replying to James: "Throwing more compute at mbas won’t really help yo..."]
https://users.dimi.uniud.it/~agostino.dovier/PAPERS/CUDAatSAT_JETAI_DRAFT.pdf

[2024-10-28 16:18] moe: wheres GCHQ intern sobriety test the0

[2024-10-28 17:06] James: [replying to Brit: "one day people will invent egraphs"]
Also mostly useless

[2024-10-28 17:06] James: [replying to 0x208D9: "https://users.dimi.uniud.it/~agostino.dovier/PAPER..."]
What part of mba solving is this helping?

[2024-10-28 17:07] 0x208D9: [replying to James: "What part of mba solving is this helping?"]
its not practically, none of those would yeild any useful result with GPU which we havent achieved normally

[2024-10-28 17:07] James: Actually before we even start the argument

[2024-10-28 17:07] 0x208D9: [replying to 0x208D9: "its not practically, none of those would yeild any..."]
im just saying there are papers based on it

[2024-10-28 17:07] James: Please would you both read zhous paper again

[2024-10-28 17:08] 0x208D9: [replying to James: "Please would you both read zhous paper again"]
i did tho

[2024-10-28 17:08] James: What was the key takeaway for you?

[2024-10-28 17:09] 0x208D9: that it will make SAT solving faster with GPU?

[2024-10-28 17:09] 0x208D9: one order of magnitude specifically lmao

[2024-10-28 17:09] James: Oh wait that paper is also written by a zhou?

[2024-10-28 17:09] James: Oh

[2024-10-28 17:09] James: No no sorry

[2024-10-28 17:09] James: I meant the original mba for obfuscation paper

[2024-10-28 17:10] James: It was written by a Mr Zhou

[2024-10-28 17:10] 0x208D9: [replying to James: "I meant the original mba for obfuscation paper"]
yeah ik that, thats why i said practically GPU has no points to speedup the deobfuscations of MBA

[2024-10-28 17:10] 0x208D9: [replying to 0x208D9: "pretty similar to yk "using AI for fuzzing" lmaooo..."]
.

[2024-10-28 17:10] James: OH

[2024-10-28 17:10] James: Lol I did not get that it was sarcasm

[2024-10-28 17:11] James: Damn mb

[2024-10-28 17:12] James: Maybe the sat solving on the gpu could help when proving equivalence

[2024-10-28 17:12] James: So existing solvers could maybe benefit

[2024-10-28 17:12] James: But really the only reason they have to even do that is because they are flawed in their construction from ground up

[2024-10-28 17:14] 0x208D9: [replying to James: "But really the only reason they have to even do th..."]
below 100k assertions there wont be much of a optimization tbf, and if its 100k assertions im pretty sure some of em might be just LHS = RHS

[2024-10-28 17:15] 0x208D9: oh here we go : https://github.com/Z3Prover/z3/issues/1795
[Embed: Has anyone tried building z3 with cuda? · Issue #1795 · Z3Prover/z3]
Hello Everyone, This is more of a curiosity question. NVIDIA Cuda provides GPU-accelerated Libraries for Computing. Is there any way we can build z3 with Cuda? I would like to experiment if CPU int...

[2024-10-28 17:16] 0x208D9: idk why some people just try to shove gpu for everything tbh