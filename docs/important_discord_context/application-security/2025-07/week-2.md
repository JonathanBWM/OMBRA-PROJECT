# July 2025 - Week 2
# Channel: #application-security
# Messages: 25

[2025-07-10 19:08] Brit: I think this was pointed out back then too, but its not a reliable datapoint given that there are legitimate reasons for for a cpu to have less threads than it should have ootb.

[2025-07-10 20:48] Matti: hey it's all good, I have to say I did enjoy writing that massive flame post near the end

[2025-07-10 20:48] Matti: I'd say the positive way to look at it is that at least it's a FN and not a FP

[2025-07-10 20:49] Matti: you can leave out the 'theoretically' <:thinknow:475800595110821888> <@303272276441169921> is simply right wrt this

[2025-07-11 17:02] DirtySecreT: went back and read this https://discord.com/channels/835610998102425650/835656787154960384/1336502232740265994 im not seeing what is novel or ultimate about this

[2025-07-11 17:02] DirtySecreT: but the fallout is pretty amusing as your friend doubles down

[2025-07-11 17:02] DirtySecreT: no triples down

[2025-07-12 08:00] ngildea: [replying to sariaki: "ello, could anyone point me to some samples or obf..."]
Just be aware that the simplification of MBA expressions is also pretty advanced nowadays, when I last looked at this denuvo's Simba and gamba scripts could simplify anything I threw at them

[2025-07-12 08:09] sariaki: [replying to ngildea: "Just be aware that the simplification of MBA expre..."]
Funny you say that now, I got around to reading the GAMBA paper and realized the same thing

[2025-07-12 08:10] ngildea: [replying to sariaki: "Funny you say that now, I got around to reading th..."]
Yeah I went from "oh maybe this could be really useful" to "nope it's useless" in about a day 😆

[2025-07-12 08:24] sariaki: [replying to ngildea: "Yeah I went from "oh maybe this could be really us..."]
Yea when reading the zhou et al. paper I originally thought the same

[2025-07-12 08:26] sariaki: I mean the fact that it’s NP-hard pretty much means that even with GAMBA being good there’s still some parts of the problem that it can’t solve

[2025-07-12 08:26] sariaki: I just have no idea what those are

[2025-07-12 14:24] selfprxvoked: [replying to ngildea: "Just be aware that the simplification of MBA expre..."]
But how fast?

[2025-07-13 09:32] ngildea: [replying to selfprxvoked: "But how fast?"]
An expression with hundreds of terms was pretty much instant IIRC. There was a paper that proved it was sufficient to compute them with a single bit so if they can be resolved it's always going to be quickly

[2025-07-13 09:33] ngildea: https://www.usenix.org/conference/usenixsecurity21/presentation/liu-binbin

[2025-07-13 10:20] Brit: [replying to ngildea: "An expression with hundreds of terms was pretty mu..."]
while I have no trouble imagining bit blasting linear mbas away is trivial I have a reservations about non linear exprs

[2025-07-13 10:21] Brit: maybe with synthesis

[2025-07-13 10:21] Brit: but Id need to investigate more

[2025-07-13 10:22] ngildea: [replying to Brit: "while I have no trouble imagining bit blasting lin..."]
Yeah it's definitely possible I was only looking at linear ones. If anyone fancies doing a wee investigation and write-up it'd be interesting to get a handle on the state of the art

[2025-07-13 10:23] ngildea: From a quick Google there seems to be some active research on solving still ongoing

[2025-07-13 11:10] sariaki: [replying to selfprxvoked: "But how fast?"]

[Attachments: image.png]

[2025-07-13 11:10] sariaki: here's one of the benchmarks from GAMBA

[2025-07-13 11:14] sariaki: [replying to Brit: "while I have no trouble imagining bit blasting lin..."]
could you elaborate? GAMBA from my quick skim pretty much just iteratively applies SiMBA which itself just does bitblasting and is time-wise perfect on pretty much all benchmarks

[2025-07-13 11:17] sariaki: I genuinely want to know since I originally planned on working on expanding the NeuReduce/gMBA papers but if GAMBA is already perfect then there probably isn't any value in that idea