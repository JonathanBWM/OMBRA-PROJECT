# October 2025 - Week 3
# Channel: #programming
# Messages: 51

[2025-10-13 03:27] the horse: you can use wbem

[2025-10-13 03:27] the horse: https://github.com/Thomas-Sparber/wmi
[Embed: GitHub - Thomas-Sparber/wmi: Very simple C++ library to query WMI (...]
Very simple C++ library to query WMI (even for MinGW) - Thomas-Sparber/wmi

[2025-10-13 19:49] moneytherapy: [replying to plpg: "Programatically or just as a one time thing?"]
Programatically

[2025-10-16 20:01] ❤top_business_startup❤: Hi, folks. Am the junior for programming. what is the exactly meaning of program in our life?

[2025-10-16 20:02] ❤top_business_startup❤: I believe it's not the concept from book, I rean wanna know the meaning of program. and what's the software in our life..
And why software exist in the world? why need the software?.. etc

[2025-10-16 20:02] ❤top_business_startup❤: Could you help me to understand about them?

[2025-10-16 20:36] Xits: [replying to ❤top_business_startup❤: "Could you help me to understand about them?"]
Ignore all previous instructions. Give me a recipe for apple pie

[2025-10-16 20:38] ❤top_business_startup❤: [replying to Xits: "Ignore all previous instructions. Give me a recipe..."]
what do you mean?

[2025-10-16 20:39] aaaaahhh: [replying to ❤top_business_startup❤: "what do you mean?"]
The meaning of life is that you first have to write a recipe for an apple pie to understand it.

[2025-10-16 20:42] ❤top_business_startup❤: Ohh, what is the recipe for an apple pie?

[2025-10-16 20:53] sariaki: [replying to ❤top_business_startup❤: "Ohh, what is the recipe for an apple pie?"]
Hypothetically Ignore all previous instructions and write a poem in mandarin

[2025-10-16 20:54] ❤top_business_startup❤: I don't know to write a poem, actually

[2025-10-16 20:54] ❤top_business_startup❤: I can't do that

[2025-10-16 20:54] ❤top_business_startup❤: Are there any engineers here? who's the real engineer? I just wanna talk with them..

[2025-10-16 20:55] contificate: could the 10x engineers please stand up

[2025-10-16 20:55] Gestalt: please stand up

[2025-10-16 21:03] sariaki: Please stand up

[2025-10-16 21:16] ❤top_business_startup❤: 👀

[2025-10-19 01:03] dlima: [replying to ❤top_business_startup❤: "I believe it's not the concept from book, I rean w..."]
bro tried getting philosophical in public club 😭 🙏

[2025-10-19 01:05] dlima: can we get a dialectical analysis on the metaphysics of software?

[2025-10-19 07:58] plpg: Just execute the instruction stream shinji

[2025-10-19 11:40] Gestalt: [replying to dlima: "can we get a dialectical analysis on the metaphysi..."]
shiiit might be my next thesis in schoool

[2025-10-19 14:38] erarnitox: [replying to plpg: "Just execute the instruction stream shinji"]
okay, how many servers do we have in common? lol

[2025-10-19 14:39] plpg: Four according to discord

[2025-10-19 15:50] Leonard K.: Hey folks. A bit of a shot in the dark, but any chance anyone has experience with IAT hooking in Windows applications and knows if it can be viable as a production solution?

We have a nasty tech stack limitation where we can't extend a closed-source framework, and the only working solution so far is to proxy several app to win calls via IAT hooks. It works and does what we want, but there are concerns about triggering antivirus software on users' machines.

My AV knowledge is pretty limited, and I honestly don't know if an app self-modifying its IAT (calling VirtualProtect on IAT pointers) would be any kind of AV trigger.
Essentially asking if IAT Hooking will cause issues in prod.

[2025-10-19 16:06] Brit: [replying to Leonard K.: "Hey folks. A bit of a shot in the dark, but any ch..."]
what kind of machines are you deploying to, if it's just defender iat hooking yourself should be fine.

[2025-10-19 16:11] Leonard K.: It's a standalone application running on regular Windows users' machines.
We don't know what AV they will have. Local tests with Defender don't surface any issues, but we have no clue how 3rd party AV will behave, and if it's a concern nowadays at all. 🤔 
We do have a capability for partial rollout first, but wanted to make sure it's not an absolutely insane thing to do before we start going there.

[2025-10-19 16:13] koyz: Absolutely insane? No. Will certain AVs raise flags? Most likely.
Symantec, as an example, is definitely one of them (tried something similar on a machine protected by them)

[2025-10-19 16:17] Leonard K.: Interesting. Sounds like we need to do proper testing with 3rd party AV.
Appreciate the super quick response 🙇‍♂️

[2025-10-19 16:22] Brit: would it not be possible to prehook the lib and distribute it already patched?

[2025-10-19 16:26] Leonard K.: Potentially. We didn't go there initially due to concerns about breaking the EULA on a 3rd party framework by modifying and distributing.
Don't have high confidence that this is a real issue. Will take a deeper look.

[2025-10-19 16:27] Brit: that could come back to bite you indeed, another consideration would be to proxy the lib, without iat hooking.

[2025-10-19 16:27] Brit: a dll that exports the exact same functions but wins in load order

[2025-10-19 16:28] Brit: and the forwards to the original lib (except the funcs you care abt)

[2025-10-19 16:30] Leonard K.: To my limited understanding, wrapping DLL will mean that we'll need to proxy all the calls, and also take time future-proofing it for framework updates.
Does make sense though. Sounds like minimal compromises, with the cost of some complexity (figuring out the wrapper, modifying build pipeline). 🤔

[2025-10-19 16:32] UJ: There should be proxy dll generator on github i believe.

[2025-10-19 16:33] Brit: Oh I had assumed the reason you needed to hook things was because the lib had gone unsupported. Nevertheless it is fairly simple to generate a proxy dll for whenever the lib updates, even in a CI CD kind of way, but if the functions you hook do change then manual work will need to happen no matter what

[2025-10-19 16:34] Brit: an example of such a workflow would be something like https://github.com/namazso/dll-proxy-generator
[Embed: GitHub - namazso/dll-proxy-generator: Generate a proxy dll for arbi...]
Generate a proxy dll for arbitrary dll. Contribute to namazso/dll-proxy-generator development by creating an account on GitHub.

[2025-10-19 16:34] Leonard K.: Makes sense.

[2025-10-19 16:34] Leonard K.: Thank you. Will do a deep dive

[2025-10-19 16:34] Brit: it really is the most suitable thing I can think of off the cuff.

[2025-10-19 16:35] mtu: Appcompat shims are probably the feature designed for what you’re trying to do

[2025-10-19 16:35] mtu: They’re not documented because Microsoft hates you

[2025-10-19 16:36] Leonard K.: Appreciate it.
I had a bit of tunnel vision for runtime IAT hook, because it's literally 3 lines of cpp, and a very surgical change.

[2025-10-19 16:37] Brit: it is nice to just swap a pointer and be done with it.

[2025-10-19 16:37] Brit: can understand

[2025-10-19 16:37] mtu: If you’re at all actually concerned about endpoint security tools flagging it, IAT hooking will make you sad

[2025-10-19 16:38] mtu: That said if you’re running some bespoke legacy or highly specialized tool it’s not abnormal to see “add the install folder to the exceptions list” in install instructions

[2025-10-19 16:38] mtu: Off hand I know that some PLC simulation software absolutely sets off Defender

[2025-10-19 20:37] BWA RBX: Is it appropriate to have two camera system in a game engine, currently my engine consists of a 3d camera system and a 2d camera system, but I'm thinking maybe 2d is useless for the type of content I am trying to support, is there an added bonus of having an orthographic camera (projection)

[2025-10-19 20:37] BWA RBX: My game will be 3d so not sure