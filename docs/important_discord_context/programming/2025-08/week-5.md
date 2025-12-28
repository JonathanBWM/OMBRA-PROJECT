# August 2025 - Week 5
# Channel: #programming
# Messages: 62

[2025-08-25 17:56] Humza: Chief sanitation engineer (cleans toilets)

[2025-08-25 17:59] 0xatul: dont underestimate that position esp if there's a tacobell in the neighborhood

[2025-08-25 19:41] twopic: [replying to Humza: "Chief sanitation engineer (cleans toilets)"]
Yo I remember you from atl

[2025-08-26 04:12] Deus Vult: [replying to Humza: "Chief sanitation engineer (cleans toilets)"]
Reminds me of GTA SA with OG LOC saying he's a hygiene technician

[2025-08-26 16:19] Eriktion: Quick question:
Is manipulating the unwind info and language specific handler stuff in a runtime function considered unsafe behavior in bin2bin obfuscation?

Cause there are many different language specific handlers which you will have to cover and different versions of unwind info, sometimes you will also not even know what type of handler you are dealing with if it’s sth unknown/custom that someone has implemented for their own language

[2025-08-26 16:20] Eriktion: In the context of windows pe64 binaries

[2025-08-26 16:21] Eriktion: Cause if you don’t properly parse unwind info and consequently also language specific handler stuff, instruction transforms applied will maybe break exception handling due to the changed offsets within the functions

[2025-08-26 16:21] Eriktion: Ty for your takes on that

[2025-08-26 19:10] umseeker: They don't have business engineers I hope....


Nope... I had to go and jinx it. [Goddamn it](https://www.indeed.com/career-advice/finding-a-job/what-is-business-engineer)

[2025-08-26 19:12] umseeker: [replying to koyz: "everyone is an engineer nowadays! look at the job ..."]
<:mochi:1124839160742690916> I meant to reply to this. <:yea:904521533727342632>

[2025-08-26 19:13] koyz: [replying to umseeker: "<:mochi:1124839160742690916> I meant to reply to t..."]
you just did, great job! <:lillullmoa:475778601141403648>

[2025-08-26 19:14] umseeker: [replying to koyz: "you just did, great job! <:lillullmoa:475778601141..."]
I wrote a message above <:mmmm:904523247205351454>

[2025-08-26 20:29] Sussibaki: Did someone compile eRPC to work with windows kernel maybe?

[2025-08-26 21:05] UJ: [replying to Sussibaki: "Did someone compile eRPC to work with windows kern..."]
You need to post this in at least 5 more channels for a response /s

[2025-08-26 21:23] ml: Hi, after successfully injecting my DLL via manual mapping, I wanted to protect it against dumping. To achieve this, I thought about fragmenting it inside the remote process, meaning splitting the DLL into multiple sections (by sections I don’t mean .reloc, but for example 8KB chunks), placed out of order with fake sections and empty sections, and then executing it.

The problem is that since the DLL is broken apart, I have no idea how to actually execute it. That’s why I’m here asking for help or if you have any resources on this.

[2025-08-26 21:26] rico: [replying to ml: "Hi, after successfully injecting my DLL via manual..."]
https://github.com/btbd/smap
[Embed: GitHub - btbd/smap: DLL scatter manual mapper]
DLL scatter manual mapper. Contribute to btbd/smap development by creating an account on GitHub.

[2025-08-26 21:45] ml: [replying to rico: "https://github.com/btbd/smap"]
thx will take look at that

[2025-08-26 21:57] rico: [replying to ml: "thx will take look at that"]
obviously though, the binary can still be easily "dumped", since you will have a bunch of executable pages outside of the range of any loaded module, and thus pages pertaining to your binary can be "found" easily

[2025-08-26 22:21] ml: [replying to rico: "obviously though, the binary can still be easily "..."]
yeah ofc but it will be a better

[2025-08-27 02:48] Brit: [replying to ml: "yeah ofc but it will be a better"]
Map into the padding between existing page boundaries instead

[2025-08-27 10:45] ml: ok

[2025-08-27 14:39] 0xatul: [replying to rico: "https://github.com/btbd/smap"]
interesting project name

[2025-08-28 06:36] KR: [replying to contificate: "inb4 works at intel as a janitor, draws infeasible..."]
reverse goodwill hunting

[2025-08-28 15:53] Ciarán: [replying to x0a: "i think its the official "intel" discord server , ..."]
nah its just using ligatures that look like multiple chars but are actually just a single char

[2025-08-28 15:54] Ciarán: thats how I made my tag, the `ffi` is a single character

[2025-08-29 01:13] KR: "I have made a severe and continuous lapse in my judgement"
[Attachments: image.png]

[2025-08-29 01:22] Matti: was this post necessary

[2025-08-29 02:02] KR: [replying to Matti: "was this post necessary"]
No? It's just me lamenting a design choice I made early on in a project.

[2025-08-29 02:03] KR: I'm happy to give context but it's more of a joke than anything

[2025-08-29 02:10] Matti: yeah I got the second part, so I guess I either don't get the joke or maybe it just isn't all that funny without context

[2025-08-29 02:10] Matti: possibly even with context

[2025-08-29 02:10] Matti: but it wouldn't hurt...

[2025-08-29 02:11] KR: [replying to Matti: "yeah I got the second part, so I guess I either do..."]
That feels like more of a critique of my shitty humour than anything else... which, fair point.

[2025-08-29 02:15] Matti: I mean, it is

[2025-08-29 02:15] Matti: but humour is subjective, so maybe I really am just not getting it

[2025-08-29 02:15] the horse: i don't get it either

[2025-08-29 02:15] the horse: it's not funny

[2025-08-29 02:16] the horse: maybe it's a placeholder for the real joke

[2025-08-29 02:18] KR: [replying to the horse: "i don't get it either"]
https://tenor.com/view/you-wouldnt-get-it-joker-smoking-gif-15952801

[2025-08-29 02:19] KR: Anyway, my jokes shit and I should feel bad. Got it. I'm just going to drop this and move on, yeah?

[2025-08-29 02:23] Matti: dw about it, I'm pretty sure we can adapt and overcome to move on from the trauma caused

[2025-08-29 02:24] Matti: I was honestly just confused about the post cause there was no context to it

[2025-08-29 13:43] koyz: The AI did a good job <:kappa:716953351622885418>
[Attachments: image.png]

[2025-08-29 15:39] Deus Vult: [replying to koyz: "The AI did a good job <:kappa:716953351622885418>"]
I'd love to know why it chose to use static in this context, was it just a hallucination or was there a valid reason for it?  also why a full int for such a small constant value

[2025-08-29 15:47] koyz: [replying to Deus Vult: "I'd love to know why it chose to use static in thi..."]
Read about compile time constants, in this case the size of the datatype does not really matter as the storage cost will be the same

[2025-08-29 15:48] Deus Vult: [replying to koyz: "Read about compile time constants, in this case th..."]
So the compiler would just optimize that anyways?

[2025-08-29 15:48] koyz: yeah it at least should inline the values directly

[2025-08-29 15:49] koyz: if inlining/optimization is enabled of course (although I seriously doubt a compiler would never not inline them)

[2025-08-29 15:50] Deus Vult: I'm trying to dig deeper into how optimization works with C++, so thanks for pointing that out I'll definitely have a look at it

[2025-08-29 15:50] Deus Vult: <@229977096078753793> Nice blog btw!

[2025-08-29 15:51] koyz: Thank you :)

[2025-08-29 15:55] elias: How would you guys approach debugging a system freeze with no crash and no deadlocks detected in windbg?

[2025-08-29 16:03] diversenok: You have a manually triggered crash dump, right?

[2025-08-29 16:03] diversenok: I guess start with inspecting stack traces from all processors

[2025-08-29 16:03] elias: [replying to diversenok: "You have a manually triggered crash dump, right?"]
No Im attached to the VM as the freeze happens

[2025-08-29 18:00] cinder: [replying to elias: "How would you guys approach debugging a system fre..."]
never had this happen to me but I guess that as diverse said i'd go processor from processor and see if there is anything hung or waiting for something that is never happening

[2025-08-29 18:01] elias: yeah I did that and noticed it was my shitty self made spinlock mechanism that had a flaw lol

[2025-08-29 18:01] cinder: and see if I am at fault or if something else is at fault, usually the first

[2025-08-29 18:02] cinder: [replying to elias: "yeah I did that and noticed it was my shitty self ..."]
yep that's usually the case, I noticed that windbg does not catch deadlocks in a reliable manner

[2025-08-30 19:54] Deus Vult: Is it possible to create a custom application for my Game Engine like a launcher, I was working off of [Walnut](https://github.com/StudioCherno/Walnut/) however they modify GLFW to accomplish a lot of their features, I managed to actually get everything working without modifying GLFW, however my resizing of my application does not work at all, does this mean I would have to edit GLFW to add support for this I don't believe I have to but I've been trying for a while
[Embed: GitHub - StudioCherno/Walnut: Walnut is a simple application framew...]
Walnut is a simple application framework for Vulkan and Dear ImGui apps - StudioCherno/Walnut

[2025-08-30 20:47] Deus Vult: [replying to Deus Vult: "Is it possible to create a custom application for ..."]
Got it to work my code is so shit because I am so new to C++ but I am just glad I got it to work without touching GLFW

[2025-08-30 20:48] Deus Vult: It just uses Vulkan and ImGui now