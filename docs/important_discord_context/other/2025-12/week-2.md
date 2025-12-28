# December 2025 - Week 2
# Messages: 51

[2025-12-08 21:44] pinefin: ai cheat vs ai anti cheat cat and mouse game soon™

[2025-12-08 21:44] Timmy: https://cdn.discordapp.com/attachments/1311227847507181599/1433300324717826108/togif.gif

[2025-12-08 21:45] pinefin: pov most dma developers

[2025-12-09 01:23] mint: altera pcie leech when

[2025-12-09 09:09] Wane: Hello guys, I have a question. When you are dealing with anti-cheats, what workflow do you guys use?

Recently I'm analyzing EAC, I use hypervisor-based debugging workflow. Placing ept hooks, monitor function calls, walk stack, reverse engineer that return address. However I faced a problem, the driver (program) is virtualized heavily, so my analysis is very slow. I think my approach is somewhat wrong so I'm here to ask of your workflows against anticheats.

[2025-12-09 17:52] phage: Statically lift, optimize, and recompile to remove their obfuscation

[2025-12-09 19:57] BloodBerry: [replying to Wane: "Hello guys, I have a question. When you are dealin..."]
I use vulnerable driver to load own driver to get system control

[2025-12-09 19:57] BloodBerry: U can try smth like that but….
Idk about how to inject DLL “safely”

[2025-12-09 19:58] BloodBerry: I wanna try to load it and hook stuff but don’t know how it can be done

[2025-12-10 02:15] daax: [replying to Wane: "Hello guys, I have a question. When you are dealin..."]
What do you believe would be the optimal next step without going into statically lifting?

[2025-12-10 02:16] phage: let me shill

[2025-12-10 02:23] Wane: [replying to daax: "What do you believe would be the optimal next step..."]
I don't know the exact answer, usually I did hand-ray reverse engineering and figure out the routine, see another routine, repeat. Do you have a clever approach?

[2025-12-10 02:29] daax: [replying to Wane: "I don't know the exact answer, usually I did hand-..."]
what information are you actually missing right now?

[2025-12-10 02:32] daax: and what would “solved” look like to you? like what information would you have?

[2025-12-10 02:48] Wane: [replying to daax: "and what would “solved” look like to you? like wha..."]
I have a 'partial' examined routines which I think I figured out. The problem is the time and uncertainty; the analysis takes a long time and unstable, also I don't know how many detections they have.

Since I took approach examining individual routine without devirtualizing, this problem seems chronic. I thought lifting and deobfuscating is only approach for this, is there any other?

[2025-12-10 02:53] daax: [replying to Wane: "I have a 'partial' examined routines which I think..."]
Yes… and you are choosing to roadblock yourself. Answer the question directly: what information would make your life easier? What parts of your RE are missing that would give you more context? You’re just picking routines randomly and instrumenting/hooking them? There are some very obvious things you would want to get in some automated form / efficient way if you’ve looked at an obfuscated target.

[2025-12-10 03:11] Wane: [replying to daax: "Yes… and you are choosing to roadblock yourself. A..."]
1. I don't know how they call individual detection routines, do sanity check, etc. So if I have a 'blueprint' of AC (which describes internal behavior abstractly), the RE process will be easier for me
2. The 'blueprint' I said. But how do I get it?
3. I was placing API ept hooks on well known functions like `MmGetSystemRoutineAddress`, trace them, walk stack, and there is an AC routine that uses the routine

Until now I believe the one and only way to gather contexts from obfuscated target is deobfuscating. Let me guess with a hint if you have another 'automated form / efficient way.'

[2025-12-10 03:15] daax: [replying to Wane: "1. I don't know how they call individual detection..."]
well the most beneficial thing to get for an obfuscated target, as you sort of just mentioned, is control flow. how do you imagine you could recover control flow without deobfuscation? what approaches have you considered or can you think of? (walk through them, one by one, benefits:tradeoffs). that do not involve hand picking apis to hook and check against*

[2025-12-10 03:24] Wane: Yes the control flow! I didn't deeply think about this, but for now on, I thought of Intel PT, LBR, single step with MTF, etc. I think the most promising approach for this is Intel PT, however I am uncertain of if it is more efficient way than lifting & recompiling method.

[2025-12-10 03:39] daax: [replying to Wane: "Yes the control flow! I didn't deeply think about ..."]
yay! now you’re using your noodle (or an LLM, that’s at your own peril if so). PT is useful, but there is another option that combines well with what the facilities you’re already using. I think you should look into each in depth and you’ll likely see immediately which one integrates easiest into what you already have setup.

as for what’s more efficient, depends on time constraints and your overall objective/what you can work with. if you want 1:1 pseudocode for RE of the target then … the answer is to go deobfuscation route. however, you don’t need to “lift to abcd ir, optimize, choke hold, and lower to x86” to be able to reason about an anti-cheats internals. there are also tools/frameworks you can use to apply specific context information to a segment of code to simplify it and deobfuscate segments as well that aren’t llvm.

[2025-12-10 03:44] Wane: [replying to daax: "yay! now you’re using your noodle (or an LLM, that..."]
Thanks for the detailed explanation. I really appreciate it. I’m curious about the ‘other option that combines well with the facilities I’m already using’ that you mentioned. Also could you clarify what specific tool or framework you were referring to?

[2025-12-10 03:49] snowua: 
[Attachments: image.png]

[2025-12-10 03:49] daax: [replying to Wane: "Thanks for the detailed explanation. I really appr..."]
given your current hypervisor context and EPT hooks, which of those three facilities can you access with the least additional implementation work, and why?

[2025-12-10 03:54] Wane: I see, I was shortsighted. I can place stealth hooks on AC too, and it might be answer

[2025-12-10 03:54] daax: [replying to Wane: "I see, I was shortsighted. I can place stealth hoo..."]
mf what

[2025-12-10 03:54] Wane: what

[2025-12-10 03:54] Wane: Isnt it

[2025-12-10 03:56] Wane: Ahhh I didnt understand english

[2025-12-10 03:57] Wane: I think LBR, as I know it is associated with MSR right?

[2025-12-10 03:57] koyz: Did you by chance hit the ChatGPT daily limit?

[2025-12-10 03:57] Wane: Bruh I dont use any LLM

[2025-12-10 04:01] daax: [replying to Wane: "I think LBR, as I know it is associated with MSR r..."]
Open the SDM. Think on it critically for a bit and you’d be surprised what problems you can solve.

[2025-12-10 04:01] daax: I don’t wanna definitively say you are using an LLM, but it sure felt like it there.

[2025-12-10 04:06] Wane: Maybe I have to think by myself.. All I can say for this, I didn't use LLM for this conversation at all, like why would I? For what?

[2025-12-10 04:07] Wane: Wait, are there people using LLM for like this situation? (I don't know what 'this situation' I wrote but yeah)

[2025-12-10 04:08] selfprxvoked: [replying to snowua: ""]
I'm concerned that you guys may be categorizing people that just struggle with english and may be using translators which outputs everything as formal as possible sometimes or that people that just like to speak formally are being treated as LLM users <:mmmm:904523247205351454>

[2025-12-10 04:09] Wane: Um yeah I did use translators..

[2025-12-10 04:09] Wane: Actually I don't know where 'can' I use LLM for this

[2025-12-10 04:10] daax: [replying to selfprxvoked: "I'm concerned that you guys may be categorizing pe..."]
I’m not concerned at all. Whether he did or not, only he knows for certain, and it will only impact the people who use them. I can’t with certainty say he did but the responses felt strange considering the context.

[2025-12-10 04:12] snowua: [replying to selfprxvoked: "I'm concerned that you guys may be categorizing pe..."]
Says the guy who consistently posts garbage in the server. Most recent chat of you is trolling a guy for 30 minutes about something you don't understand and being completely misleading. I wouldn't be surprised that you would jump in to defend this. Not to mention you have a hammer and sickle in your bio says enough https://discord.com/channels/835610998102425650/835664858526646313/1436750902621900992

[2025-12-10 04:12] selfprxvoked: [replying to Wane: "Actually I don't know where 'can' I use LLM for th..."]
I don't know about you, but I've certainly being categorized by using LLMs in a whole lot of Discord servers just because I tend to be either too formal or because I use rarely used english words because my primary language uses them and I've translated to english

[2025-12-10 04:13] daax: [replying to Wane: "I think LBR, as I know it is associated with MSR r..."]
As for this:

we have narrowed down where to look and what to look for. this is the point where you open the SDM and read about them and make the determination yourself. it all depends on what your platform has implemented and how easily you can integrate one of these

[2025-12-10 04:14] selfprxvoked: and you are stating that I'm wrong most of the times

[2025-12-10 04:15] selfprxvoked: [replying to snowua: "Says the guy who consistently posts garbage in the..."]
I'm not defending him btw

[2025-12-10 04:17] Wane: [replying to daax: "As for this:

we have narrowed down where to look ..."]
Yeah I'll do, I learned lots of things on this conversation..

[2025-12-10 04:23] Wane: Okay, I'll come back with a proof, (or more 'valuable question') I may have lack of many knowledge that people can consider me as LLM user (or something)

[2025-12-10 04:29] aslrnk: i dont really think so

[2025-12-10 04:34] daax: [replying to Wane: "Okay, I'll come back with a proof, (or more 'valua..."]
It’s no big deal, if you aren’t then cool; if you are whatever just use it for learning. Feel free to ask more questions if you get stuck, implementing these things can be hard regardless

[2025-12-10 04:39] selfprxvoked: [replying to aslrnk: "i dont really think so"]
well, tbf it is really hard to say "how much accuracy rate AI does have" because it is a really vague statement

[2025-12-14 04:11] DeChaos: [replying to Wane: "I see, I was shortsighted. I can place stealth hoo..."]
He's not even trying to hide at this part holy shit

[2025-12-14 10:25] 0xdeluks: [replying to DeChaos: "He's not even trying to hide at this part holy shi..."]
broo its “stealth hooks”, stealth means its FUD <:topkek:904522829616263178>