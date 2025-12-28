# October 2024 - Week 2
# Channel: #🔗-research-and-links
# Messages: 123

[2024-10-07 15:51] daax: [replying to brew002: "Just finished my first blog post:
https://brew02.g..."]
> That said, there are still many situations where this concept remains quite viable and useful for creating pseudo-EPT hooks, particularly when coupled with some level of hardware virtualization technology on either Intel or AMD processors.

Where do you see this being  useful over hardware-assisted mechanisms when implemented properly?

[2024-10-07 18:47] donna🤯: [replying to brew002: "Just finished my first blog post:
https://brew02.g..."]
Just thought Id mention that the link in your readme is broken (looks like the title of the post has changed)

[2024-10-07 20:38] brew002: [replying to donna🤯: "Just thought Id mention that the link in your read..."]
Yeah thanks for that I fixed it

[2024-10-07 20:46] brew002: [replying to daax: "> That said, there are still many situations where..."]
To me it just seems like it could be particularly useful on AMD processors where NPT doesn't support execute-only memory. I can't say that I have ever worked on an AMD hypervisor before, but I have heard bemoaning constantly from people that do work with AMD hypervisors about the lack of this feature. The cross-compatibility was something else that I thought could be useful because you could probably make minimal changes to the implementation across AMD and Intel hypervisor projects. I've also looked into some of the detection mechanisms for EPT hooks and it seems that timing attacks are a big giveaway, which this might help with. Edit: Changed the wording to "some", as it's probably more apt than "many".

[2024-10-07 21:15] Can: [replying to brew002: "Just finished my first blog post:
https://brew02.g..."]
selene mentioned <:PES5_Wow:804692198305431572>  more on topic: traversal is sort of expensive too tbh, even if you sandbox the entire OS to sync all r/ws. my recommendation would be to essentially emulate TLB, e.g. you keep an entirely fake page table hierarchy that gets incrementally populated while you're handling the #PF apart from the PML4s reserved for your own code (or if you decide that you really don't care enough about a specific region to copy 4 levels deep, copy but with a special borrow indicator for your use [usermode, etc.]).

You then purge it when you see a INVLPG INVPCID INVD WBINVD MOVCR3 (+ certain MSRs being written to) getting executed or an interrupt arriving >= IPI level which is essentially what the OS also expects to see delivered before a PTE modification is globally visible as well.

[2024-10-07 21:17] Can: cool stuff!

[2024-10-07 21:29] brew002: [replying to Can: "selene mentioned <:PES5_Wow:804692198305431572>  m..."]
That's actually a very good idea. I think my only concern for such a solution would be the difficulty in properly emulating expected system behavior. Granted most virtualization detection mechanisms are very poor and would also need to understand expected system behavior to function properly. 

Also I was so surprised at the timing for the release of selene and this project because I had been asking questions about software virtualization in this server not that long ago, and then this holy grail of project is released with the exact technology that I was looking into.

[2024-10-07 21:38] Can: If you'd like to check the implementation it's at su/plpd + su/mirror where policy == `policy::hostile`. It's about ~600ns of overhead until the guest starts executing at regular speed as it's essentially amortized over runtime and you can speculatively map some pages like the stack or IP. I still went with different levels of isolation because it is still an avoidable overhead to initial vmenter compared to pre-approving the pml4s, so it makes more sense to operate under the "driver does nothing OS related because Microsoft abstracted it" assumption until you catch it doing something, or it is in a viable position to do modifications like that, e.g. executing in their own page tables, or in a raised IRQL IPI (in which scenario overhead is irrelevant anyway since interprocessor latency is more than that).
PCID handling does get a bit tricky though, especially since you definitely want to use it to accelerate page table swaps on vmenter.

[2024-10-07 22:41] Matti: [replying to Torph: "did they start using Clang at some point, or is th..."]
it's just the WRK yeah, compiled with clang

[2024-10-07 22:42] Matti: it compiles but does not run yet atm

[2024-10-08 00:30] szczcur: [replying to brew002: "To me it just seems like it could be particularly ..."]
i think the fundamental issue is a lack of creativity for those that "bemoan it".. there have been discussions in this discord of methods to achieve performant XO implementations when it comes to amd. it just seems like because there are far fewer public resources people don't turn their thinking caps on.. as for hv detections, any detection made can be emulated.. eac has loads of side channel attacks / tlb probes, which can be circumvented by just adjusting parameters or outright modifying thread state and then c3. i don't see any enhanced reliability or usefulness over traditional slat. as can said emulating the tlb as closely as possible will net you best results. you mention "it seems that timing attacks are a big giveaway, which this might help with." but this won't lower your discoverability. there are loads of things you would need to do to improve that metric. the effort required far exceeds the effort to do the same with hardware assisted facilities.

[2024-10-08 05:33] brew002: [replying to szczcur: "i think the fundamental issue is a lack of creativ..."]
You're definitely right that one of the best ways to circumvent detections would be to simply attack the detection itself, rather than relying on hiding from things. I can also sort of agree that this doesn't necessarily make a hypervisor sneakier -- although I'm really taking your word for it -- but I think there is still merit to the claim that this could be useful for AMD hypervisors, and it has some additional cross-compatibility. Final thought: I think that something that I failed to make abundantly clear in the blog post is that using this with hardware virtualization would be preferable **in comparison to** software virtualization, since it would be a lot easier to implement these things. That is to say, if you wished to build this POC out into something that would actually work, it would be much easier to do that with hardware virtualization than software virtualization.

[2024-10-08 05:35] brew002: Thanks to everyone for the comments and thoughts!

[2024-10-08 17:10] daax: [replying to brew002: "You're definitely right that one of the best ways ..."]
I’m having a hard time seeing the merit to the claim of using the method presented as is for anything other than research. This in conjunction with hardware virtualization isn’t going to yield greater performance over most alternatives, *unless* you’re doing similar to what you might with virtualization exceptions (and doing it in a way similar to that). Implementing #VE correctly in conjunction with EPT, would improve over performance over using only EPT in Intel, for instance; but the method presented in the article is slower in comparison — including on AMD with NPT+#PF intercept without a #VE-like mechanism. Like <@1033421942910369823> mentioned, and maybe I’m not understanding the angle you’re looking at this, but I don’t see what enhanced compatibility you’re referring to. Porting #PF hooks, like having one uniform implementation for both amd<->intel? 

I suppose you could potentially use this as a last resort fallback if SLAT is not supported by your processor (if it’s *that* old… unlikely). Though the implementation of #VE or #VE-like handler for AMD is different to this, and there are cleaner ways to do this while minimizing overhead.

[2024-10-09 19:49] brew002: [replying to daax: "I’m having a hard time seeing the merit to the cla..."]
I mean at the end of the day it is only a POC that I don't intend on fleshing out to be used, so I guess you could say that it's just a piece of research. In regards to the rest of your reply, the method that I had envisioned if this were to be used with hardware virtualization would have used a mechanism similar to the #VE handler for EPT. I had actually almost entirely forgotten that you could VMEXIT on #PFs, which is probably why it's something that I didn't mention in the article and consequently lead to confusion about the implementation. In the same vein, though, I deliberately left the details somewhat vague as the focus of the article wasn't about actually using it with hardware virtualization; rather, it was mostly about the feature itself, some of its shortcomings, and ways that those shortcomings could possibly be fixed (which may involve the usage of hardware virtualization). I think that if you were to build this idea out into something that actually worked, you would want to use this to extend the capabilities of #PF hooks.

Maybe the title gave a certain idea that the article would have more of a focus on hardware virtualization (due to the mention of extended page tables), but it was really just a way of illustrating the concept. Sorry if there was any confusion. If you have suggestions for how I could clarify things in the article I would be more than interested!

[2024-10-09 20:03] brew002: Also I believe that I should clarify the ending of the article because it seems like it caused the most confusion. The ending of the article is sort of meant to be two things: the viability of actually implementing a solution (my recommendation was that using software virtualization would require much more effort than hardware virtualization) and the practicality of the solution (this solution has it's applications, but it would probably be the most useful if you were using it **without** hardware virtualization). I definitely think that I should improve the ending so that it is more clear that it would be **easier** to implement with hardware virtualization, but more **practical** to be used without it, and then I could briefly discuss that trade-off. Let me know if this would make things better/more clear.

[2024-10-11 15:07] Timmy: neat! https://youtu.be/D-N4DuW8P0o
[Embed: AMD EPYC Turin Server Benchmarking and Review! Featuring the Zen5 9...]
It craves the power! It needs the power! 500 watts! 

Sooo many benchmarks!: https://forum.level1techs.com/t/amd-epyc-turin-is-out-9575f-benchmarking-w-comsol-phoronix-test-suite/218260

0:00 - Intro


[2024-10-12 01:11] ReaP: https://github.com/Peribunt/CTC
[Embed: GitHub - Peribunt/CTC: Interprocess communication via a covert timi...]
Interprocess communication via a covert timing channel - Peribunt/CTC

[2024-10-12 01:12] ReaP: Due to the ever increasing number of writeups and publications on this subject, I have decided to publish a pretty concise and effective proof of concept

[2024-10-12 01:13] ReaP: I would love to see some creative alterations or implementations of this functionality

[2024-10-12 02:29] szczcur: https://pigweed.dev/docs/blog/05-coroutines.html

[2024-10-12 02:30] szczcur: https://github.com/cbwang505/Win11PoolView
[Embed: GitHub - cbwang505/Win11PoolView: 正确解析 _HEAP_VS_***符号 ,支持在最新win11 2...]
正确解析 _HEAP_VS_***符号 ,支持在最新win11 24h2 运行,替换windbg自带的!pool命令 - cbwang505/Win11PoolView

[2024-10-12 02:30] szczcur: interesting windbg extension for vr types

[2024-10-12 02:35] szczcur: https://github.com/Yukin02/Dwm-Overlay

<@1142623438264082503> lol
[Embed: GitHub - Yukin02/Dwm-Overlay: DWM Overlay without modify .text]
DWM Overlay without modify .text. Contribute to Yukin02/Dwm-Overlay development by creating an account on GitHub.

[2024-10-12 02:37] daax: [replying to szczcur: "https://github.com/cbwang505/Win11PoolView"]
I’m a simple man, I see 吃那 I click the repo

[2024-10-12 02:38] James: never once met a rude chinese person

[2024-10-12 02:39] daax: [replying to James: "never once met a rude chinese person"]
just spend a little time on chinese forums <:Kappa:794707301436358686> if they find out youre using translator you will soon meet one

[2024-10-12 02:41] James: this one is surely bait too...

[2024-10-12 02:42] daax: [replying to James: "this one is surely bait too..."]
eh?

[2024-10-12 02:42] James: the post

[2024-10-12 02:42] James: bait by an ac engineer

[2024-10-12 02:42] daax: the dwm one

[2024-10-12 02:42] daax: ?

[2024-10-12 02:43] James: yes

[2024-10-12 02:43] daax: yeah probably lol

[2024-10-12 02:43] James: 🤣

[2024-10-12 02:43] daax: wouldn’t surprise me

[2024-10-12 02:43] James: however, if it were chained maybe 4 times... might be safe then

[2024-10-12 02:45] szczcur: [replying to James: "however, if it were chained maybe 4 times... might..."]
chained?

[2024-10-12 02:45] James: it is a meme where you chain multiple data pointer calls together

[2024-10-12 02:45] szczcur: [replying to daax: "just spend a little time on chinese forums <:Kappa..."]
yea lol they really dont like westerners lurking

[2024-10-12 02:45] szczcur: [replying to James: "it is a meme where you chain multiple data pointer..."]
oh okay im a boomer ignore me lol

[2024-10-12 02:57] daax: [replying to szczcur: "https://github.com/cbwang505/Win11PoolView"]
I see your 🇨🇳 repo and raise you c compiler for multiple architectures <:Kappa:794707301436358686> https://github.com/tyfkda/xcc
[Embed: GitHub - tyfkda/xcc: Standalone C compiler/assembler/linker/libc fo...]
Standalone C compiler/assembler/linker/libc for x86-64/aarch64/riscv64/wasm - tyfkda/xcc

[2024-10-12 02:57] daax: apparently works for wasm too

[2024-10-12 02:59] szczcur: [replying to daax: "I see your 🇨🇳 repo and raise you c compiler for mu..."]
thats very cool. it’s still active too.. looks like they’re working on the cpp extension now

[2024-10-12 02:59] szczcur: its most recently updated

[2024-10-12 03:45] dinero: get ready to learn chinese buddy

[2024-10-12 03:50] szczcur: [replying to dinero: "get ready to learn chinese buddy"]
i should try. i wonder if there is an app that will translate all text actively without having to select or paste it in, or interact. just type and it works

[2024-10-12 03:50] szczcur: thats what someone should do with llms

[2024-10-12 03:50] dinero: they will know

[2024-10-12 03:50] szczcur: na i mean in general

[2024-10-12 03:50] dinero: oh you mean as you read. use micrsooft edge

[2024-10-12 03:50] szczcur: well as you type and read

[2024-10-12 03:50] dinero: ez

[2024-10-12 03:51] szczcur: like if i type this .. it gets auto translated to whatever language

[2024-10-12 03:51] szczcur: as i type it without any other interaction

[2024-10-12 03:51] dinero: yea like that discord plugin
there was one exactly like that

[2024-10-12 03:51] szczcur: [replying to dinero: "oh you mean as you read. use micrsooft edge"]
this is heresy

[2024-10-12 03:52] szczcur: [replying to dinero: "yea like that discord plugin
there was one exactly..."]
yea but outside of discord

[2024-10-12 03:52] szczcur: anything from texting to discord to writing papers

[2024-10-12 03:52] dinero: [replying to szczcur: "this is heresy"]
how so

[2024-10-12 03:52] szczcur: [replying to dinero: "how so"]
suggesting to use edge

[2024-10-12 03:52] dinero: everything`s chrome unless ur a nerd running icefox or some shit

[2024-10-12 03:53] dinero: <@160202062548697108> can vouch. edge users stay winning

[2024-10-12 03:53] szczcur: [replying to dinero: "everything`s chrome unless ur a nerd running icefo..."]
icefox or firefox are based

[2024-10-12 03:53] szczcur: <@609487237331288074> arent you a fire(ice?)fox user

[2024-10-12 03:53] dinero: womp womp

[2024-10-12 03:54] szczcur: [replying to dinero: "<@160202062548697108> can vouch. edge users stay w..."]
edge users 😂 equivalent to being an android user

[2024-10-12 03:55] szczcur: 🎣

[2024-10-12 03:57] dinero: u know what

[2024-10-12 03:57] dinero: ill give ff a try again. maybe im fooled by msft and their AI skibidi rizz

[2024-10-12 03:59] James: [replying to dinero: "<@160202062548697108> can vouch. edge users stay w..."]
true

[2024-10-12 03:59] James: however you unironically use the vertical tabs.... which is slightly alarming

[2024-10-12 04:01] Azrael: [replying to dinero: "everything`s chrome unless ur a nerd running icefo..."]
Flow engine time.

[2024-10-12 04:03] Azrael: Does anyone even use Brave for the crypto part?

[2024-10-12 04:03] dinero: [replying to James: "however you unironically use the vertical tabs......."]
yeah i wonder why i do this. its not saving me any screen space 
used to it now tho

[2024-10-12 04:03] estrellas: easier to find lost tabs imo

[2024-10-12 04:04] Azrael: Just use tab groups.

[2024-10-12 04:04] James: the problem is that you have to go searching for tabs in the first place.... keep yo shit sorted maybe?

[2024-10-12 04:04] estrellas: i am lazy

[2024-10-12 04:04] Azrael: I have like 17 tab groups active.

[2024-10-12 04:05] Azrael: They’re all colorful and labeled with pretty names (I was bored).

[2024-10-12 04:05] estrellas: i have like 137 unsorted active tabs xD

[2024-10-12 04:05] Azrael: Oh wow, I probably have around 90 tabs open.

[2024-10-12 04:06] estrellas: it does sucks when i have a bunch of PDFs

[2024-10-12 04:07] estrellas: i cant really differ them by the ico

[2024-10-12 04:07] Azrael: I save them and read them in Acrobat 😔

[2024-10-12 04:08] estrellas: great alternative

[2024-10-12 04:08] Azrael: That way I have them on disk which allows me to back them all up.

[2024-10-12 04:08] Azrael: I like the user experience that Acrobat offers. Super important when reading all of those pdf files.

[2024-10-12 04:09] estrellas: its index i assume? yeah its very useful, i use it every time when reading intel's sdm

[2024-10-12 04:10] Azrael: Only thing that can reasonably load it 😆

[2024-10-12 04:28] szczcur: [replying to Azrael: "Does anyone even use Brave for the crypto part?"]
i did for a bit

[2024-10-12 04:29] szczcur: dont have much of an opinion on it. still switched back to chrome

[2024-10-12 04:30] szczcur: [replying to estrellas: "i have like 137 unsorted active tabs xD"]
theres a great plugin called tab session that i use

[2024-10-12 04:30] szczcur: so i can close them and reopen whenever i want. reduces the need to have 60 open even though im only going to go back and read maybe use 10

[2024-10-12 04:35] estrellas: seems interesting

[2024-10-12 04:35] estrellas: ill give it a try

[2024-10-12 04:35] estrellas: slowmode 😑

[2024-10-12 07:18] diversenok: [replying to ReaP: "https://github.com/Peribunt/CTC"]
It would be cool to see your writeup on this topic (i.e., an explanation of the idea and the underlying mechanisms, limitations, maybe even potential detection vectors, etc.)

[2024-10-12 09:01] Timmy: https://www.mozilla.org/en-US/security/advisories/mfsa2024-51/
[Embed: Security Vulnerability fixed in Firefox 131.0.2, Firefox ESR 128.3....]

[2024-10-12 09:15] donna🤯: [replying to ReaP: "Due to the ever increasing number of writeups and ..."]
Would you mind linking some of those writeups? POC seems really cool

[2024-10-12 13:11] szczcur: [replying to diversenok: "It would be cool to see your writeup on this topic..."]
you can find write ups in the git projects that do similar. <@753171127903191102> 

it’s a well practiced channel, some implementations even sending files too.

https://github.com/moehajj/Flush-Reload

https://github.com/jiyongyu/covert_channel_chatbot

https://github.com/SamKG/Flush-Reload-Sidechannel

https://github.com/pavel-kirienko/cpu-load-side-channel

similarly https://github.com/depletionmode/wsIPC

https://github.com/rshnn/covert

https://github.com/casperIITB/Flush-Reload-attack

..and many more
[Embed: GitHub - moehajj/Flush-Reload]
Contribute to moehajj/Flush-Reload development by creating an account on GitHub.
[Embed: GitHub - jiyongyu/covert_channel_chatbot]
Contribute to jiyongyu/covert_channel_chatbot development by creating an account on GitHub.
[Embed: GitHub - SamKG/Flush-Reload-Sidechannel: A demonstration of a sidec...]
A demonstration of a sidechannel vulnerability that exploits cache timings using Flush Reload to communicate information over a covert channel - SamKG/Flush-Reload-Sidechannel
[Embed: GitHub - pavel-kirienko/cpu-load-side-channel: Side-channel file tr...]
Side-channel file transfer between independent VMs or processes executed on the same physical host. - pavel-kirienko/cpu-load-side-channel
[Embed: GitHub - depletionmode/wsIPC: Working Set Page Cache side-channel I...]
Working Set Page Cache side-channel IPC PoC. Contribute to depletionmode/wsIPC development by creating an account on GitHub.

[2024-10-12 13:11] donna🤯: [replying to szczcur: "you can find write ups in the git projects that do..."]
This is wonderful thankyou

[2024-10-12 13:14] szczcur: [replying to diversenok: "It would be cool to see your writeup on this topic..."]
HEXPADS details using hw pmc’s to detect sx usage, there is another i can’t remember that involves introducing environmental noise to decrease the efficacy of these types of communications

[2024-10-12 13:14] szczcur: https://github.com/yshalabi/covert-channel-tutorial
[Embed: GitHub - yshalabi/covert-channel-tutorial: Hands on with side-chann...]
Hands on with side-channels: a tutorial on covert-channels built using shared CPU resources. Three different covert-channel implementations based on Flush+Reload and Prime+Probe (L1, LLC) side-chan...

[2024-10-12 13:15] szczcur: has sources that explain the mechanisms too.

[2024-10-12 13:20] daax: [replying to szczcur: "HEXPADS details using hw pmc’s to detect sx usage,..."]
you might be thinking of cc hunter: https://www2.seas.gwu.edu/~guruv/micro14.pdf

[2024-10-12 13:25] szczcur: [replying to daax: "you might be thinking of cc hunter: https://www2.s..."]
yea. the llc + conflict miss tracking is interesting

[2024-10-12 13:26] szczcur: [replying to donna🤯: "This is wonderful thankyou"]
np.

[2024-10-12 13:38] szczcur: <@753171127903191102> you might check out the post dax sent if you’re interested in some detection possibilities. 

also some neat ideas here for pintools / dtrace for instrumentation: <https://github.com/sabbaghm/SCADET>

<https://github.com/hgn/barnowld> .. should be useful for flush+reload, and evict+time

https://yuval.yarom.org/pdfs/KosasihFCYZ24.pdf

and this has examples of different sx <https://github.com/ECLab-ITU/Cache-Side-Channel-Attacks>

[2024-10-12 14:25] donna🤯: [replying to szczcur: "<@753171127903191102> you might check out the post..."]
Just did some reading this is really cool stuff! thanks for sharing

[2024-10-12 14:30] Brit: [replying to szczcur: "https://github.com/Yukin02/Dwm-Overlay

<@11426234..."]
yes swap a pointer in a read only section

[2024-10-12 14:30] Brit: 🧠

[2024-10-12 14:30] Brit: that's gonna go amazing

[2024-10-13 07:48] idkhidden: fun little project i made while i was learning networking 
https://github.com/idkhidden/beatbeat
[Embed: GitHub - idkhidden/beatbeat: BeatBeat is a straightforward TCP hear...]
BeatBeat is a straightforward TCP heartbeat system implemented in C++ - idkhidden/beatbeat

[2024-10-13 09:00] mrexodia: https://www.ryanliptak.com/blog/every-rc-exe-bug-quirk-probably/
[Embed: Every bug/quirk of the Windows resource compiler (rc.exe), probably...]
Fuzz testing decades-old software can turn up some curious behaviors

[2024-10-13 09:01] mrexodia: [replying to idkhidden: "fun little project i made while i was learning net..."]
But are the numbers actually random, or is the idea to prevent someone from faking your heartbeat?

[2024-10-13 09:20] idkhidden: [replying to mrexodia: "But are the numbers actually random, or is the ide..."]
It was initially designed to prevent someone from faking the heartbeat, but it can still be easily emulated. I might soon add some proper anti-emulation measures.

[2024-10-13 09:20] idkhidden: 
[Attachments: image.png]

[2024-10-13 09:51] mrexodia: [replying to idkhidden: "It was initially designed to prevent someone from ..."]
Yeah I figured, just wasn't too clear from the description. I would use the phrase 'challenge-response' somewhere if you wanted to clarify (that's the term used in the crypto community)

[2024-10-13 10:13] idkhidden: [replying to mrexodia: "Yeah I figured, just wasn't too clear from the des..."]
oh ty didnt knew <:gigachad:904523979249815573>

[2024-10-13 12:18] Can: [replying to mrexodia: "Yeah I figured, just wasn't too clear from the des..."]
ECDH + XOF brr