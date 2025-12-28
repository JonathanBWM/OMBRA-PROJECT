# August 2025 - Week 5
# Channel: #🔗-research-and-links
# Messages: 105

[2025-08-25 00:20] !cyberpunk: https://github.com/MatheuZSecurity/Rootkit
[Embed: GitHub - MatheuZSecurity/Rootkit: Collection of codes focused on Li...]
Collection of codes focused on Linux rootkits. Contribute to MatheuZSecurity/Rootkit development by creating an account on GitHub.

[2025-08-25 00:25] the horse: what's the real threat surface of malware in linux besides supply chain attacks?

[2025-08-25 13:27] mtu: What exactly are you asking? Introduction vectors are usually insufficient/missing access control, followed by default/stolen creds

[2025-08-25 16:31] Horsie: Cant remember the last time I installed something that wasnt from my package manager/aur

[2025-08-25 16:33] Horsie: But tbh I'm super paranoid about supply chain stuff on windows. Its so easy to just sneak in code to a random ass package's github and it pwns half of all linux users.

[2025-08-27 02:51] mrexodia: https://github.com/poppopjmp/VMDragonSlayer
[Embed: GitHub - poppopjmp/VMDragonSlayer: Automated multi-engine framework...]
Automated multi-engine framework for unpacking, analyzing, and devirtualizing binaries protected by commercial and custom Virtual Machine based protectors. Combines Dynamic Taint Tracking, Symbolic...

[2025-08-27 02:52] mrexodia: <:claude:1410094586579259492>

[2025-08-27 03:05] dinero: lmfao i didnt know "real implementation" was gonna appear 19 times

[2025-08-27 03:06] dinero: this is just code vomit from claude code huh

[2025-08-27 03:07] dinero: `   # In real implementation, would iterate through memory blocks
                binary_data = b'\x90' * 1000  # Simulated binary data
                return binary_data
`

[2025-08-27 03:07] dinero: highly robust test case

[2025-08-27 03:15] iris: 
[Attachments: image.png]

[2025-08-27 03:28] UJ: [replying to dinero: "lmfao i didnt know "real implementation" was gonna..."]
apparently there is a whole "reverse engineer larping community"

[2025-08-27 03:28] UJ: this could be one of those

[2025-08-27 03:29] mrexodia: [replying to UJ: "this could be one of those"]
the guy did a defcon talk

[2025-08-27 03:32] UJ: [replying to mrexodia: "the guy did a defcon talk"]
hmm yeah he looks legit, project looks like ai slop tho.

[2025-08-27 04:20] dinero: [replying to UJ: "apparently there is a whole "reverse engineer larp..."]
Many such cases

[2025-08-27 04:23] dinero: I just wish heavily vibe coded projects had a disclaimer somewhere

[2025-08-27 04:24] UJ: i just look for emojis

[2025-08-27 04:24] UJ: or the em dash

[2025-08-27 04:24] dinero: like for example. This is a nice thing to note
[Attachments: IMG_8883.png]

[2025-08-27 06:47] the horse: 🙏 just wait for the fully vibe coded IR leading to XYZ vibe devirt; if only HayLift could be vibe coded as easily.. 😔

[2025-08-27 09:34] avx: [replying to dinero: "lmfao i didnt know "real implementation" was gonna..."]
uhoh

[2025-08-27 10:11] Brit: [replying to dinero: "lmfao i didnt know "real implementation" was gonna..."]
look at the little sat solver that could

[2025-08-27 10:12] Brit: https://github.com/poppopjmp/VMDragonSlayer/blob/main/dragonslayer/analysis/symbolic_execution/solver.py#L186
[Embed: VMDragonSlayer/dragonslayer/analysis/symbolic_execution/solver.py a...]
Automated multi-engine framework for unpacking, analyzing, and devirtualizing binaries protected by commercial and custom Virtual Machine based protectors. Combines Dynamic Taint Tracking, Symbolic...

[2025-08-27 10:14] Brit: x+1 = x
satisfiable in ℤ

[2025-08-27 10:16] avx: https://tenor.com/view/they-might-be-giants-the-silly-boys-are-back-in-town-watch-out-they-will-fuck-you-up-fuck-your-shit-up-gif-2881580458387500246

[2025-08-27 11:13] safareto: [replying to UJ: "apparently there is a whole "reverse engineer larp..."]
wtf is a reverse engineering larping community

[2025-08-27 11:14] Brit: most of you nerds :^)

[2025-08-27 11:14] safareto: touche

[2025-08-27 11:54] mtu: [replying to mrexodia: "the guy did a defcon talk"]
> Blockchain Node Manteiner

grifter confirmed

[2025-08-27 12:12] Ciarán: [replying to mrexodia: "https://github.com/poppopjmp/VMDragonSlayer"]
Been stuck with a claim he made that he has seen samples from "nation states with VMs that would make commercial VMs jealous" since I watched the talk on this

[2025-08-27 12:16] mtu: “Let me just make this implant even easier to detect as malicious by adding an entire VM”

[2025-08-27 12:17] Brit: you say this but it is possible to make small ellegant vms

[2025-08-27 12:17] Brit: see [here](https://github.com/thesecretclub/riscy-business) :^)
[Embed: GitHub - thesecretclub/riscy-business: RISC-V Virtual Machine]
RISC-V Virtual Machine. Contribute to thesecretclub/riscy-business development by creating an account on GitHub.

[2025-08-27 12:21] mtu: I feel like the things VMs are good for is in direct conflict with the goals of government backed malware though. It’s super fingerprintable and high cost-to-develop, with the only plus side being making static analysis difficult.

[2025-08-27 12:22] Brit: [replying to mtu: "I feel like the things VMs are good for is in dire..."]
it eliminates a very big marker that gets people caught out in post exp situations

[2025-08-27 12:22] Brit: no need to allocate executable memory

[2025-08-27 12:22] mtu: Like, as a SOC monkey I don’t really care what the 200MB exe that’s only running on one laptop and is in autoruns is doing, it’s malicious

[2025-08-27 12:22] Brit: [replying to mtu: "Like, as a SOC monkey I don’t really care what the..."]
that vm I linked you is less than a page

[2025-08-27 12:22] Brit: <= 4kb

[2025-08-27 12:23] Brit: [replying to Brit: "no need to allocate executable memory"]
so you can just keep updating your program tape whenever, and it wont light up the edr like a christmas tree

[2025-08-27 12:23] mtu: I guess my point is the size doesn’t really matter unless you’re shipping the VM as part of your in-memory exploit kit

[2025-08-27 12:24] mtu: Once you’re persisting, you want cheap to develop and low-fingerprint tools

[2025-08-27 12:24] Brit: no as in you get one page of executable memory and you never need to alloc more, which is one of the things that all these redteam types are so concerned about to begin with

[2025-08-27 12:25] Brit: anyway, VMs are so broad a subject, not everything is made with the intent of making static analysis hard

[2025-08-27 12:25] mtu: Yes because the red team types also hate to admit that a properly configured AV/EDR is sufficient to stop or detect 99% of malware

[2025-08-27 12:25] Ciarán: [replying to Brit: "you say this but it is possible to make small elle..."]
why would small and elegant make commercial VMs jealous though?

[2025-08-27 12:26] Brit: [replying to Ciarán: "why would small and elegant make commercial VMs je..."]
oh no it would not, I have no clue what the chatgpt larper was on about

[2025-08-27 12:26] Brit: Im just saying that shipping a vm has benefits

[2025-08-27 12:26] Brit: especially if its built to be low footprint

[2025-08-27 12:28] Ciarán: [replying to Brit: "especially if its built to be low footprint"]
ah okay

[2025-08-27 12:29] Ciarán: [replying to Brit: "oh no it would not, I have no clue what the chatgp..."]
lmao

[2025-08-28 04:23] bishop: [replying to mtu: "Yes because the red team types also hate to admit ..."]
<:topkek:904522829616263178>

[2025-08-28 20:15] Timmy: [replying to Brit: "https://github.com/poppopjmp/VMDragonSlayer/blob/m..."]
https://github.com/poppopjmp/VMDragonSlayer/commit/b16f559ff5e8125887df9dedb5a6362815c62b72#diff-a4b5153152e952f515dd97aa12073bb228b13e36e03f056f24e1bc79477b9b6b
[Embed: Code RC · poppopjmp/VMDragonSlayer@b16f559]

[2025-08-28 20:15] Timmy: he removed all the issues

[2025-08-28 20:31] UJ: [replying to Timmy: "he removed all the issues"]
gemini didn't just [delete ](https://news.ycombinator.com/item?id=44651485) it?

[2025-08-28 23:04] Zophiel: [replying to Timmy: "https://github.com/poppopjmp/VMDragonSlayer/commit..."]
Someone else put a post on the github issue and the author repulsed back they still have to do more testing

[2025-08-28 23:05] Zophiel: Now the repo is been updated stating the release date has been moved to the end of August

[2025-08-29 00:56] iris: cant wait

[2025-08-29 04:15] mtu: I assume the vx-underground tweet was dunking on this, though maybe there was two ai slop talks

[2025-08-29 06:11] Ciarán: [replying to mtu: "I assume the vx-underground tweet was dunking on t..."]
two talks that made heavy use of AI this year and a talk last year which I'm incredibly suspicious of called "Hacking the Skies - Satellite Red Teaming"

[2025-08-29 06:12] Ciarán: cant find anything past the synopsis but that was nearly entirely AI generated
[Attachments: l00DtCy.png]

[2025-08-29 06:16] Ciarán: he agreed to send over the slides, so gonna take a look at them if he actually does

[2025-08-29 06:16] the horse: [replying to Ciarán: "cant find anything past the synopsis but that was ..."]
I wouldn't trust these AI detectors at all; they're extremely unreliable

[2025-08-29 06:17] Ciarán: [replying to the horse: "I wouldn't trust these AI detectors at all; they'r..."]
they are, but given the obvious use of AI everywhere else its just another thing to add to the list

[2025-08-29 06:18] Ciarán: even if he did use AI there it wouldnt be a major issue, its only a summary, but its still something worth looking at further

[2025-08-29 16:12] sariaki: [replying to Ciarán: "cant find anything past the synopsis but that was ..."]
weird how it switches between the pronouns "I" and "we"

[2025-08-29 20:22] Zophiel: [replying to mtu: "I assume the vx-underground tweet was dunking on t..."]
Yup it’s not looking good for the author they have to actually show their results

[2025-08-29 20:28] mtu: I get using ai to assist you in writing things like abstracts

[2025-08-29 20:28] mtu: But I felt icky about that, and still felt like I had to edit it for tone and such so I didn’t feel scummy

[2025-08-29 20:57] Ciarán: [replying to mtu: "I get using ai to assist you in writing things lik..."]
honestly, if he actually had something to back up his claims the heavy use of AI wouldn't bother me, sure the projects codebase might be slop but if it works it works

[2025-08-29 20:58] Ciarán: it just doesnt work tho, none of his claims can be backed up, shit the taint tracking which was in the **title** of the talk doesnt even exist and clearly never has

[2025-08-29 21:00] Ciarán: people spend a lot of money to go to DEFCON, $500 for the ticket, plus flights, plus accommodation, plus food and everything else, and not only is their time being wasted with AI slop which clearly never worked. He also stole the main stage position from another researcher who could have actually provided a valuable and interesting talk

[2025-08-29 21:44] contificate: the future is bright

[2025-08-30 12:48] brymko: [replying to Ciarán: "people spend a lot of money to go to DEFCON, $500 ..."]
crazy to blame him for stealing the main stage and not the conference committee for accepting this in the first place

[2025-08-30 12:56] mtu: How would the CFP review catch “person lied about what they accomplished”? You don’t submit the whole talk before getting accepted, just an abstract

[2025-08-30 12:58] mtu: I suspect a lot of DEF CON presenters would balk at a ACM/IEEE style review, where over the course of like 3 months you have to submit a basically complete paper, then respond to >= 3 reviews and address all comments before final acceptance

[2025-08-30 13:11] dullard: [replying to mtu: "I suspect a lot of DEF CON presenters would balk a..."]
Would significantly raise the barrier to entry which is a good and bad thing tbh

[2025-08-30 13:13] dullard: I’ve presented at a couple of conferences and I doubt I would have done so if the CFP submission and acceptance process was like that 😂

[2025-08-30 13:15] dullard: [replying to mtu: "How would the CFP review catch “person lied about ..."]
Even before the LLM hype train there were still some sub par talks that get accepted at larger cons. One of the problems with the “promise me the world” type CFP processes

[2025-08-30 13:24] mtu: Rip time ai 

https://www.schneier.com/blog/archives/2019/09/the_doghouse_cr_1.html
[Embed: The Doghouse: Crown Sterling - Schneier on Security]
A decade ago, the Doghouse was a regular feature in both my email newsletter Crypto-Gram and my blog. In it, I would call out particularly egregious—and amusing—examples of cryptographic “snake oil.” 

[2025-08-30 13:37] lom: "The founder and CEO, Robert Grant is a successful healthcare CEO and amateur mathematician that has discovered a method for cracking asymmetric encryption methods that are based on the difficulty of finding the prime factors of a large quasi-prime numbers."

[2025-08-30 13:37] lom: Luigi, get the strap

[2025-08-30 15:25] Brit: [replying to lom: ""The founder and CEO, Robert Grant is a successful..."]
Larp statement

[2025-08-30 15:25] Brit: Yet another lattice approach to factorization

[2025-08-30 15:25] Brit: That will not work

[2025-08-30 15:25] Brit: [replying to mtu: "How would the CFP review catch “person lied about ..."]
You do have to submit the slides

[2025-08-30 15:26] Brit: The slides were obviously ai-d to heck

[2025-08-30 15:27] lom: [replying to Brit: "Larp statement"]
Oh I don't doubt it for a second, I just think that the order of achievements is a strange order. Amateur mathematician? Cite some published research, even at an amateur level.

[2025-08-30 15:28] Brit: Alas even that is not a good barometer for this, bunch of publishing houses will happily publish your garbage

[2025-08-30 15:28] Brit: Even in maths

[2025-08-30 15:31] lom: Sure, but even then you would be able to weed that out from the reputability of the publisher

[2025-08-30 15:32] Brit: Meh, arg from authority etc, I'd rather he publish his current research and be clowned on because its barely postgrad level

[2025-08-30 15:43] mtu: [replying to Brit: "You do have to submit the slides"]
After being accepted and 2 weeks before the con

[2025-08-30 15:43] mtu: I don’t think they even look at them, that’s just to get it on the media server

[2025-08-30 18:23] Zophiel: [replying to mtu: "I suspect a lot of DEF CON presenters would balk a..."]
I think we need something like that also with the requirement of reproducible code/demos

[2025-08-30 20:37] mtu: Maybe for demo labs/workshops (and it’s insane that dudes demo was more of just “look at these slides”) but “reproducible code/demos” for a lot of talks doesn’t even make sense

[2025-08-31 11:28] Horsie: [replying to mrexodia: "https://github.com/poppopjmp/VMDragonSlayer"]
<https://x.com/_revng/status/1961450865711009933>

good thread but this ones my fav

https://x.com/_revng/status/1961450903694708957
Some opcodes were classified as "hook browser" or "download URL"\.
Quite weird for low\-level opcode of a VM\. 🤷‍♂️

[2025-08-31 12:01] avx: vm_do_malware

[2025-08-31 12:03] 5pider: ah yes, i wrote similar opcodes for my VM once as well

[2025-08-31 12:06] Brit: with superoperators + direct threading this could happen :^^)

[2025-08-31 14:41] daax: [replying to Horsie: "<https://x.com/_revng/status/1961450865711009933>
..."]
an LLM can just discern VM opcodes without any kind of instrumentation… what a pipe dream — <:smiel:813600304301735947>

[2025-08-31 15:04] avx: ok claude, lift vmp, make no mistakes <a:typing_gif:1123028192056332288>

[2025-08-31 18:47] 001: [replying to Horsie: "<https://x.com/_revng/status/1961450865711009933>
..."]
gg they leaked the opcodes