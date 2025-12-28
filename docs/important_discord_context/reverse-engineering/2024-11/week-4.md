# November 2024 - Week 4
# Channel: #reverse-engineering
# Messages: 17

[2024-11-18 13:49] Domimmo314: Guys I'm rev-eng a game app with Ghidra and I've found that a pivotal function (the session of a game battle) has no calling functions...
Then I try IDA and it can resolve the calling function of this one
The difficulty I think is that this function is called dynamically via a pointer and Ghidra can't resolve it, is there something I can do to fix it?

[2024-11-18 13:50] Domimmo314: I mean IDA finds the caller by default, I had to do nothing... I'm wondering if there's a simple setup option to make Ghidra find it too

[2024-11-20 16:05] hxm: is there anyone arround who knows shet with ghidra ? 

https://github.com/NationalSecurityAgency/ghidra/issues/7209
[Embed: How to get __imp_NAME from ORDINAL ? · Issue #7209 · NationalSecuri...]
i got this piece of code : public List<ExternalCallInfo> extractExternalCalls() { List<ExternalCallInfo> externalCalls = new ArrayList<>(); InstructionIterator instructions = list...

[2024-11-21 13:46] elias: why are so many research papers on reverse engineering written by the chinese?

[2024-11-21 13:46] elias: almost every paper I come across is written by chinese researches

[2024-11-21 14:06] 0x208D9: [replying to elias: "why are so many research papers on reverse enginee..."]
wait till u see the nguyen's of emulation

[2024-11-21 14:08] 0x208D9: and also ig, they dont have anything other than giving CCP's every exploit possible or running of to america to publicly disclose the methodologies their people are using to cook those up [ for greater good ofc 😉 ]

[2024-11-21 16:47] root: They have been practicing it in one form of another for a very long time

[2024-11-21 20:29] rin: [replying to elias: "why are so many research papers on reverse enginee..."]
Something Something bell curve

[2024-11-21 20:31] rin: (Joke)

[2024-11-21 20:40] luci4: Well well

[2024-11-21 20:41] rin: To racist ik

[2024-11-21 21:20] dullard: real

[2024-11-22 02:36] Deleted User: has anyone here ever used MiGetLargePage or knows how to initialize a pfn for a large page pde. or should I just use MiGetPage with MiInitializePfn?

[2024-11-23 19:37] Deleted User: [replying to Deleted User: "has anyone here ever used MiGetLargePage or knows ..."]
pages or some

[2024-11-24 16:25] r0asty: Recently I came across a research paper from the Chinese about compilers https://ieeexplore.ieee.org/abstract/document/10580120 machine learning-based compiler auto-tuning methods

[2024-11-24 16:27] r0asty: [replying to r0asty: "Recently I came across a research paper from the C..."]
I didn't read it, but the synopsis explains a lot about what it's about