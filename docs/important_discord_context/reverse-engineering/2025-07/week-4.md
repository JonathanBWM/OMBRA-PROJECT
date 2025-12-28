# July 2025 - Week 4
# Channel: #reverse-engineering
# Messages: 205

[2025-07-21 10:57] sariaki: wow powerful results ! ! !
I really like how they used ChatGPT and Copilot. 🔥 🚀 
<:deadman:1271808245610057730>
[Attachments: image.png]

[2025-07-21 14:12] dullard: [replying to sariaki: "wow powerful results ! ! !
I really like how they ..."]
🔥 🚀 🔥 🚀 🔥 🚀 🔥 🚀

[2025-07-21 14:12] dullard: Vibe research paper writing 🔥 🚀 🔥 🚀

[2025-07-21 14:13] sariaki: maybe 🔥 🔥 🔥
[Attachments: image.png]

[2025-07-21 14:37] dullard: @grok, can you summarise this paper please

[2025-07-21 18:38] Oliver: ive tried to use ghidras decompiler a couple of times but it always leads to me to say IDAs decompiler is much better at least in terms of humable like code. Anyone with a similar experience or have something different to say?

[2025-07-21 18:38] Lyssa: i can heavily concur

[2025-07-21 18:38] Lyssa: i do not like ghidra at all lol

[2025-07-21 18:39] Oliver: [replying to Lyssa: "i do not like ghidra at all lol"]
i watched a youtube vid by an experienced reverser, he said they where pretty similar but i just couldnt grasp that saying haha

[2025-07-21 18:40] Lyssa: i know of some people who have switched from IDA to ghidra and came to that conclusion

[2025-07-21 18:40] Lyssa: (that they're pretty similar)

[2025-07-21 18:42] Oliver: my overall interaction with ghidra has also been akward. The gui is often hard to use, difficult to find the actions you want to do etc while IDAs is more natural in its feel

[2025-07-21 19:13] mtu: Ghidra decompiler is _usually_ easier to read, but I suspect a lot of people here are mainly targeting apps that are deliberately hard to RE. Last I used it, HexRays output prioritizes completeness over ease of understanding, while Ghidra will just go “lol branch unparsable good luck”

[2025-07-21 19:14] x86matthew: i only really used ghidra once

[2025-07-21 19:14] x86matthew: to check if i was losing my mind

[2025-07-21 19:14] x86matthew: IDA seemed to be disassembling an instruction wrong and i was pretty certain it was a bug by this point

[2025-07-21 19:14] x86matthew: ghidra output was correct which confirmed that it was a bug in IDA

[2025-07-21 19:14] x86matthew: but overall i much prefer the IDA experience

[2025-07-21 19:17] UJ: [replying to mtu: "Ghidra decompiler is _usually_ easier to read, but..."]
i like how binary ninja would try to make a psudo complete function out of jmp-fuscated code while IDA would just error out.

[2025-07-21 19:18] mtu: I’ve never used binja, you can take me RE member card

[2025-07-21 19:18] mtu: I also haven’t used IDA pro since, like, 7.x

[2025-07-21 19:20] Oliver: [replying to mtu: "I also haven’t used IDA pro since, like, 7.x"]
why? job? preference?

[2025-07-21 19:20] mtu: Stopped doing professional RE yeah

[2025-07-21 19:21] mtu: My VR/RE is all personal projects now, and because it’s often embedded/other arch’s IDA Free/Home are basically unusable

[2025-07-21 19:21] Oliver: ye IDA free for arm etc is not even possible right?

[2025-07-21 19:22] mtu: Yep

[2025-07-21 23:19] Deleted User: i was reading ghidra output and half of the function was missing

[2025-07-21 23:20] Deleted User: without any obfuscation

[2025-07-21 23:22] struppigel: [replying to Oliver: "ive tried to use ghidras decompiler a couple of ti..."]
I use Ghidra at home and IDA at work. For me the decompiler quality is similar. IDA seems sometimes more precise but at the same time I had two samples lately where IDA refused to decompile while Ghidra just worked. There are definitely things that annoy me about Ghidra, but the decompiler is not one of them.

[2025-07-21 23:35] struppigel: [replying to Oliver: "my overall interaction with ghidra has also been a..."]
Are you sure that's not just from being used to how IDA does things? E.g. if you drag a window out of the GUI, figuring out how to put it back on your own in IDA is barely possible. Even knowing how to do it, it is a hazzle to put the mouse on that invisible bar that feels like it has a height of one pixel.

[2025-07-21 23:37] f00d: [replying to struppigel: "Are you sure that's not just from being used to ho..."]
you can just hover over it and it will expand

[2025-07-21 23:38] struppigel: I know but I do have difficulty hovering over the exact spot that it expands

[2025-07-22 07:03] Oliver: [replying to struppigel: "Are you sure that's not just from being used to ho..."]
This is probably the case. Im used to ida and i am bias

[2025-07-23 05:45] learncpp.com: Does anyone have any suggestions entry level resources for reversing? Preferably not just jumping into crackmes etc

[2025-07-23 05:56] Xits: [replying to learncpp.com: "Does anyone have any suggestions entry level resou..."]
https://www.youtube.com/channel/UC--DwaiMV-jtO-6EvmKOnqg
[Embed: OALabs]
Malware analysis tools, techniques, and tutorials!

[2025-07-23 05:56] Xits: there are so many good reversing videos that oalabs made

[2025-07-23 05:57] Xits: https://www.youtube.com/watch?v=PVnjYgoX_ck
heres a beginner friendly one for example but there're a bunch
[Embed: What is a Breakpoint - Debugging Explained]
What is a breakpoint and how does it work under the hood of a debugger? Learn more about how both hardware and software breakpoints work, expand for more...

See more on https://www.patreon.com/oalabs

[2025-07-23 06:16] learncpp.com: Thank you very much <@824294337584169001> , after I submit my essay I will take a look

[2025-07-23 12:14] expy: hey, <@148095953742725120>! is it possible to run EfiGuard under hyper-v? it doesn't seem to have regular bios menu

[2025-07-23 15:35] Matti: I don't actually know the answer to this question

[2025-07-23 15:35] Matti: that's not great huh

[2025-07-23 15:36] Matti: I literally never use hyper-v

[2025-07-23 15:36] Matti: but

[2025-07-23 15:37] Matti: assuming
(1) it's only the 'BIOS settings' or the 'BIOS menu' that is missing, and you can configure UEFI boot options in a different way (e.g. linux efibootmgr, bootice on windows)
(2) you do not have VBS enabled on the host, only hyper-v

[2025-07-23 15:38] Matti: then it should work

[2025-07-23 17:16] sariaki: could someone please try the following in their ida real quick? the binary does not matter.
try printing out a graph
view -> toolbars -> graph view
and then there's a printer icon in your toolbar

when i click it and press ok nothing happens

[2025-07-23 17:17] sariaki: alternatively you can right click on your toolbar.
[Attachments: image.png]

[2025-07-23 17:20] f00d: [replying to sariaki: "could someone please try the following in their id..."]
after ok
[Attachments: ida_rXPrZppLUZ.png]

[2025-07-23 17:23] sariaki: [replying to f00d: "after ok"]
thank you! what ida version are you on?

[2025-07-23 17:24] sariaki: cause imma probably have to switch - i don't think ida will offer me support if you know what i mean

[2025-07-23 17:25] Lyssa: [replying to sariaki: "could someone please try the following in their id..."]
works for me on IDA 9.1

[2025-07-23 17:25] f00d: [replying to sariaki: "thank you! what ida version are you on?"]
tested on 9.0 & 9.1

[2025-07-23 17:50] Matti: funnily enough I tried this and I have the same issue on 9.2 beta

[2025-07-23 17:50] Matti: so I'm not sure it's IDA related

[2025-07-23 17:50] Matti: I have anything related to printers and printing disabled in windows

[2025-07-23 17:51] Matti: so the print spooler service doesn't normally run (I did start it for this), and I doubt I have any printers installed

[2025-07-23 17:51] Lyssa: yeah that's probably it, I kinda doubted it had anything to do with the version

[2025-07-23 17:51] Lyssa: but I wasn't in the mood to write out suggestions for fixes 🤭

[2025-07-23 17:52] Matti: yeah I don't really have any, I think if I re-enabled the spooler and then also installed e.g. the MS print to PDF driver, it should work

[2025-07-23 17:52] Matti: but fuck printers and fuck the print spooler

[2025-07-23 17:53] Matti: single-point vulnerability collector #2 easily I think, with #1 being win32k

[2025-07-23 17:54] Matti: and I don't even own a fucking printer

[2025-07-23 18:00] sariaki: works with ida 9.1 for me now

[2025-07-23 18:11] Matti: [replying to Matti: "yeah I don't really have any, I think if I re-enab..."]
OK I actually did this and can confirm it's a windows printer setup issue, not IDA
[Attachments: image.png]

[2025-07-23 18:11] Matti: this is with the MS print to PDF printer installed

[2025-07-23 18:11] □Encrypted□: i need a hand im willing to pay for service im very stuck on extracting conents of .narc or nmd file

[2025-07-23 18:12] □Encrypted□: did i post this in correct place?

[2025-07-23 18:16] Matti: correct place: maybe, but then you'll need to provide a lot more details than this before anyone can help you
otherwise post in <#902892977284841503>, in which case you'll need to provide details on what exactly you are going to be paying (meaning the amount) in order to interest anyone

[2025-07-23 18:16] Matti: and then you'll still need to provide *that* person with the technical details of whatever it is you're stuck on

[2025-07-23 18:16] snowua: [replying to Matti: "OK I actually did this and can confirm it's a wind..."]
Big fan of the license

[2025-07-23 18:17] Matti: yeah it's a great art piece tbh

[2025-07-23 18:18] Matti: I do feel a bit dirty now having used a printer

[2025-07-23 18:18] Matti: even if it's only a virtual one

[2025-07-23 18:20] □Encrypted□: basically i need someone to help me a nd my team reverse a few formats for watch dogs legion - we have already removed the version check from the scripthook we have also made a bridge to inject custom code however we need access to whats nside the .nmd file and .narc file

[2025-07-23 18:22] Matti: [replying to sariaki: "works with ida 9.1 for me now"]
hum, so you actually only updated IDA?

[2025-07-23 18:23] Matti: for me it didn't work on 9.2 until I enabled these things (not sure which one(s) are actually required here, but you need some kind of printer I assume) and enabled and started the print spooler

[2025-07-23 18:24] Matti: 
[Attachments: image.png]

[2025-07-23 18:26] UJ: [replying to Matti: "for me it didn't work on 9.2 until I enabled these..."]
is that the beta version with the new ui 👀

[2025-07-23 18:27] Matti: it is

[2025-07-23 18:28] Matti: I haven't really used it, using IDA beta versions would get seriously tiring

[2025-07-23 18:28] Matti: they already release more than enough updates as is for me to keep up with all of my plugins

[2025-07-23 18:28] UJ: is it faster/more responsive? im not sure what changed underneath.

[2025-07-23 18:28] Matti: but I wanted to know if the license was still capable of html injection now that they upgraded to qt 6

[2025-07-23 18:29] Matti: [replying to UJ: "is it faster/more responsive? im not sure what cha..."]
couldn't tell you, sorry <:kekw:904522300257345566>

[2025-07-23 18:29] Matti: feels about the same to me if you mean simply opening it

[2025-07-23 18:29] Matti: but that's also literally all I've done

[2025-07-23 18:30] UJ: [replying to Matti: "feels about the same to me if you mean simply open..."]
yeah nothing to scientific. jw about the general feel.

[2025-07-23 18:32] □Encrypted□: any ideas or pointers? tried changing hash to a newer  yet i think pointers incorrect
[Attachments: image.webp]

[2025-07-23 18:34] sariaki: [replying to Matti: "hum, so you actually only updated IDA?"]
ye

[2025-07-23 18:34] sariaki: i had all the printer stuff enabled before

[2025-07-23 18:36] Matti: weird, what version were you on before?

[2025-07-23 18:36] sariaki: 7.7 sp1 💀

[2025-07-23 18:36] Brit: why

[2025-07-23 18:36] sariaki: never updated after it

[2025-07-23 18:36] sariaki: just never felt the need i guess

[2025-07-23 18:36] sariaki: why the fuck did they remove this
[Attachments: image.png]

[2025-07-23 18:37] sariaki: wrong arrow

[2025-07-23 18:37] sariaki: it should be pointing to "Generate HTML"

[2025-07-23 18:39] sariaki: i'm dumb, you need to have the entire function selected

[2025-07-23 18:46] snowua: [replying to UJ: "is it faster/more responsive? im not sure what cha..."]
No.

[2025-07-23 18:47] snowua: The ribbon bar has more padding and that is all I have noticed

[2025-07-23 18:58] Brit: just like a windows update

[2025-07-23 19:48] Deleted User: [replying to Matti: "OK I actually did this and can confirm it's a wind..."]
would you *cough* installer?

[2025-07-23 19:52] Matti: hell yeah

[2025-07-23 19:52] Matti: who wouldn't right

[2025-07-23 19:59] Deleted User: sorry i have a cold, im talking about screenshots

[2025-07-23 19:59] Deleted User: i would never support piracy against hexrays sa

[2025-07-23 21:34] daax: [replying to □Encrypted□: "any ideas or pointers? tried changing hash to a ne..."]
<#1378136917501284443>

[2025-07-24 19:58] UJ: https://www.youtube.com/watch?v=BeeXHWvCG9M
[Embed: IDA 9.2 beta 1: A first look]
In this first look video into IDA 9.2 beta 1, we demonstrate the new and upcoming features.

Since this is the first beta, expect to find bugs and other glitches that will be fixed in next betas.

Ple

[2025-07-25 10:52] Lyssa: looks fine to me

[2025-07-25 11:51] 0xatul: when are all the rolf goodness coming

[2025-07-25 12:28] the horse: [replying to UJ: "https://www.youtube.com/watch?v=BeeXHWvCG9M"]
honestly this is amazing

[2025-07-25 13:13] the horse: https://binary.ninja/2025/07/24/5.1-helion.html
[Embed: Binary Ninja - 5.1 Helion]
Binary Ninja is a modern reverse engineering platform with a scriptable and extensible decompiler.

[2025-07-25 13:17] daax: [replying to the horse: "https://binary.ninja/2025/07/24/5.1-helion.html"]
Am going to give it a whirl today. Saw the updates and am hoping it’s as much of an improvement as they say

[2025-07-25 13:21] the horse: 5.0 was already pretty good!

[2025-07-25 13:21] the horse: have to renew my license :/

[2025-07-25 13:34] the horse: it does look epic tho

[2025-07-25 13:35] the horse: is it like an official logo change? thought it's just like a summer thing

[2025-07-25 13:48] estwellas: \> $5k license
\> still don't pay real artists

[2025-07-25 13:54] Lyssa: there are ai detectors for images?

[2025-07-25 13:54] Lyssa: that's nice

[2025-07-25 13:55] daax: I was pretty unimpressed given the amount of money spent. An example on the same version a friend had: IDA handled an obfuscated binary with ease, but binja took multiple hours for the analysis passes. Not to mention all the unimplemented instructions/flags/prefixes with instructions that become “??”

[2025-07-25 13:55] Lyssa: expect nothing less from ilfak 🐈

[2025-07-25 13:55] the horse: there are bins which absolutely fail for me (or take absolute ages) with IDA, and don't on binja and vice versa

[2025-07-25 13:55] the horse: was this for x64?

[2025-07-25 13:56] daax: [replying to the horse: "there are bins which absolutely fail for me (or ta..."]
That doesn’t mean anything to me lol. The binary was a simple obfuscation that shouldn’t have taken either more than a few seconds.

[2025-07-25 13:56] daax: [replying to the horse: "was this for x64?"]
Both x86 and x64.

[2025-07-25 13:57] estwellas: I had a very unpleasant experience with binja & obfuscation as well

[2025-07-25 13:57] estwellas: for example:
[Attachments: image.png]

[2025-07-25 13:58] estwellas: what does this even mean lmao

[2025-07-25 13:58] daax: [replying to estwellas: "for example:"]
Binja themed to look like default IDA. very based

[2025-07-25 14:03] Lyssa: sick

[2025-07-25 14:03] Lyssa: mm, I'm not sure if it's fully ai generated

[2025-07-25 14:04] Lyssa: [replying to Lyssa: "mm, I'm not sure if it's fully ai generated"]
because that 9.2 in the background is throwing me off

[2025-07-25 14:04] Lyssa: doesn't seem like an AI-ey thing to do

[2025-07-25 14:05] 0xatul: Have a background

[2025-07-25 14:05] daax: [replying to Lyssa: "because that 9.2 in the background is throwing me ..."]
tbh the logo looks like something my wife has done before in her graphic design stuff

[2025-07-25 14:06] 0xatul: Put typography

[2025-07-25 14:06] daax: i dont think that’s ai

[2025-07-25 14:06] 0xatul: Then have the Ida image

[2025-07-25 14:06] daax: but if it is: noice

[2025-07-25 14:06] 0xatul: The Ida image may or may not be ai who the fuck cares

[2025-07-25 14:06] Lyssa: IDA costs 5k

[2025-07-25 14:06] Lyssa: if they use AI for marketing material then it's extremely embarassing

[2025-07-25 14:08] daax: [replying to Lyssa: "if they use AI for marketing material then it's ex..."]
I don’t think it matters at all personally. they’re meant to make a very technical product, if they don’t have the marketing/design staff… eh, use what you can.

[2025-07-25 14:08] 0xatul: ^ this exactly

[2025-07-25 14:08] daax: none of us can tell one way or another, so whatever they did worked well

[2025-07-25 14:08] daax: huh?

[2025-07-25 14:08] 0xatul: ...

[2025-07-25 14:09] 0xatul: Bye 👋

[2025-07-25 14:09] daax: it’s catchy, looks pretty cool for a fun summer release logo. im all for it

[2025-07-25 14:10] daax: we’re talking about a product release not a blog post. don’t conflate the two.

[2025-07-25 14:10] daax: alright, but this is hardly sloppy.

[2025-07-25 14:11] Lyssa: at least it's not this
[Attachments: jE5rSk6n.png]

[2025-07-25 14:11] daax: most of us are haters of ai slop too. I think you’re the first one to call it borderline slop without knowing if it’s ai or not.

[2025-07-25 14:12] daax: [replying to Lyssa: "at least it's not this"]
now that is 100% ai slop. also checkpoint research. slop, ai or not.

[2025-07-25 14:12] Lyssa: lol
[Attachments: 7jQnWEa6.png]

[2025-07-25 14:12] Lyssa: I only know about them because of their antidebug pages thingies

[2025-07-25 14:12] daax: the ai checkers have said my blog posts are written by ai

[2025-07-25 14:12] Lyssa: and they link to their "research blog"

[2025-07-25 14:13] daax: but they are written before gpt was available

[2025-07-25 14:13] Lyssa: [replying to daax: "the ai checkers have said my blog posts are writte..."]
text based detectors work very differently from images I imagine

[2025-07-25 14:13] daax: theyve also said other items were. checkers are regularly inaccurate

[2025-07-25 14:13] daax: [replying to Lyssa: "text based detectors work very differently from im..."]
And I imagine images are even more difficult to be accurate on. You can’t just flag every art style

[2025-07-25 14:14] Lyssa: [replying to daax: "And I imagine images are even more difficult to be..."]
the research I've seen doesn't take art style directly into account, not sure how it works but it's definitely not that primitive

[2025-07-25 14:14] Lyssa: I'm not sure to what extent it's not primitive so that's all I can say

[2025-07-25 14:14] Lyssa: :P

[2025-07-25 14:17] f00d: it's def ai slop

[2025-07-25 14:17] Lyssa: the research came to me in a dream so unfortunately not

[2025-07-25 14:18] f00d: i mean just look at the hairline and necklace it's easy to tell

[2025-07-25 14:18] daax: [replying to Lyssa: "the research I've seen doesn't take art style dire..."]
The research I’ve not read might not be accurate even though I also am not sure how it works. We both don’t know for certain

[2025-07-25 14:18] Yoran: Does binja even worth the transition? What does it offer give ida doesn't??

[2025-07-25 14:18] diversenok: I think you should dislike misuse of AI and not AI itself

[2025-07-25 14:36] Pepsi: pseudo rust 🤔

[2025-07-25 14:36] Pepsi: lemme put some `unsafe { }` into your pseudo, i have been waiting for this feature

[2025-07-25 14:36] elias: [replying to Yoran: "Does binja even worth the transition? What does it..."]
a better UI with reasonable pricing and licensing model

[2025-07-25 14:37] AstroB: i don't like that firmware ninja is only part of the ultimate edition, other than that i think binja is pretty good

[2025-07-25 14:38] AstroB: even ghidra is sometimes OK for my needs but a native interface runs much better imo

[2025-07-25 14:44] 0xatul: btw <@503274729894051901> thanks for the ntafd header in phnt, saves me a lot of time messing with my hacky, poorly maintained RE'd version of it

[2025-07-25 16:02] hxm: is there any known bugg with latest ghidra  ? it fails at decoding some instructions...
[Attachments: Screenshot_2025-07-25_at_16.59.16.png]

[2025-07-25 16:05] struppigel: [replying to hxm: "is there any known bugg with latest ghidra  ? it f..."]
These bytes are skipped because they cannot be reached. Press D if it bothers you.

[2025-07-25 16:10] hxm: can i auto do it for all funcs

[2025-07-25 16:47] struppigel: Of course you can write a script that disassembles undefined data. But it makes no sense to do that.

[2025-07-25 17:01] diversenok: [replying to 0xatul: "btw <@503274729894051901> thanks for the ntafd hea..."]
There is also a blog post about it, btw: https://discord.com/channels/835610998102425650/835655011115466772/1372511022908178504

[2025-07-25 17:01] 0xatul: I have read that

[2025-07-25 19:56] mrexodia: <:skill_issue:1210171860063617074>

[2025-07-25 19:57] mrexodia: > i cant find anything anymore without having to look at 95%+ ai slop
tooling skill issue no?

[2025-07-25 21:05] daax: send your stopwatch program here and I’ll give it a go in binja ult latest and see

[2025-07-26 19:13] Aj Topper: can anyone suggest some books related to RE, like I was reading  practice malware analysis and I kinda liked it..

[2025-07-26 20:20] mrexodia: [replying to Aj Topper: "can anyone suggest some books related to RE, like ..."]
books are for suckers, practical experience trumps all

[2025-07-26 20:26] Aj Topper: [replying to mrexodia: "books are for suckers, practical experience trumps..."]
true tbh, but I enjoy reading

[2025-07-26 20:28] mrexodia: [replying to Aj Topper: "true tbh, but I enjoy reading"]
you should switch up that attitude to enjoy doing, because you will achieve nothing by reading

[2025-07-27 03:52] struppigel: Nothing wrong with reading as long as it does not replace practical application entirely X)

[2025-07-27 03:56] struppigel: [replying to Aj Topper: "can anyone suggest some books related to RE, like ..."]
If you are interested in the history of AV, viruses and worms, check out The Art of Virus Research and Defense by Szor. It's a book I really enjoyed.
If you want something more recent, check out Evasive Malware by Cucci

[2025-07-27 03:59] daax: The only book you need is the holy Intel SDM / AMD APM, and specifics about calling conventions for your OS; you'll find everything you need in the holy manuals.

[2025-07-27 03:59] daax: <:Prayge:796414754054078464> <:intelmanual:552942314952196096>

[2025-07-27 09:59] Deleted User: [replying to mrexodia: "you should switch up that attitude to enjoy doing,..."]
true

[2025-07-27 10:48] Brit: [replying to mrexodia: "you should switch up that attitude to enjoy doing,..."]
do both for maximum gains

[2025-07-27 11:11] mrexodia: [replying to Brit: "do both for maximum gains"]
my point was practice first, reading second

[2025-07-27 11:11] Brit: sure

[2025-07-27 11:11] mrexodia: albeit formulated a bit harsher 😅

[2025-07-27 11:17] Hyun Lih: Yeah, you can't really understand what you're reading without some base knowledge... and the best way to gain that is to do some practical work.⁣👍

[2025-07-27 15:11] 0xatul: plan to impl something, search material about it

[2025-07-27 15:11] 0xatul: then try doing it, rinse and repeat

[2025-07-27 22:44] kralos: @grok is this real ?