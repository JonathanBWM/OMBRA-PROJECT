# October 2024 - Week 1
# Channel: #reverse-engineering
# Messages: 370

[2024-10-01 00:17] expy: [replying to MonTy: "NtContinue - is it used to continue the flow, in p..."]
not really, it continues the normal flow after exception was handled. You can think of these two functions as entry and exit for exception handler

[2024-10-02 00:08] Matti: that's what it reads like he said to me

[2024-10-02 00:09] Matti: does  it not

[2024-10-02 00:10] Matti: > it continues the normal flow **after exception was handled**

[2024-10-02 00:21] Termolyx: [replying to Matti: "> it continues the normal flow **after exception w..."]
Hm, my bad

[2024-10-02 00:21] Termolyx: Shouldve paid more attention lmao

[2024-10-02 00:27] Matti: you are forgiven my child

[2024-10-02 04:29] James: [replying to Matti: "found a new record holder: the PDB for matti-llvm"]
https://i.imgur.com/0W9sy62.png

[2024-10-02 05:16] MonTy: guys, I downloaded the compiled triton libraries.
[Attachments: image.png]

[2024-10-02 05:17] MonTy: how to include them to python in windows?

[2024-10-02 11:55] kite: [replying to MonTy: "guys, I downloaded the compiled triton libraries."]
wdym? like add them to your environment variables? or call functions within them with python?

[2024-10-02 12:48] MonTy: [replying to kite: "wdym? like add them to your environment variables?..."]
the python script contains import triton *

[2024-10-02 12:49] MonTy: I mainly programmed in C++, I have little experience in python(

[2024-10-02 13:29] Matti: [replying to James: "https://i.imgur.com/0W9sy62.png"]
impressive dude

[2024-10-02 13:29] Matti: I concede defeat

[2024-10-02 13:30] Matti: I did find some larger (~4GB) PDBs but they are from UE so that's noe xactly fair

[2024-10-02 13:30] Matti: any flags you use to get maximum bloat in the PDBs?

[2024-10-02 13:31] Matti: I suppose `/pdbpgagesize:8192` alone might help but I haven't tried this

[2024-10-02 13:31] James: Biggest factor I’ve found is weather it’s a clean rebuild or not

[2024-10-02 13:31] James: When editing the pdb instead of creating new one from scratch msvc makes larger ones

[2024-10-02 13:31] Matti: ahhhh are you using /INCREMENTAL?

[2024-10-02 13:32] James: No idea that was compiled a while ago

[2024-10-02 13:32] Matti: yeah I have removed that flag from my LLVM fork so that it cannot accidentally be used

[2024-10-02 13:32] Matti: what you describe requires incremental linking

[2024-10-02 13:32] Matti: so yeah

[2024-10-02 13:33] Matti: there's also `/Gm[-]`, which on MSVC is horrible to enable (so the plus version), but I don't think it does very much on clang

[2024-10-02 18:08] prick: are you guys trying to crash ida's pdb parser or something? stuff older than 8.3 crashes on newer windows 11 PDBs

[2024-10-02 18:28] James: No I was just posting a big pdb file

[2024-10-02 19:26] mrexodia: [replying to James: "https://i.imgur.com/0W9sy62.png"]
get rekt nerd
[Attachments: image.png]

[2024-10-02 21:17] Matti: nah using Qt or UE is cheating IMO

[2024-10-02 21:18] Matti: he clearly has a much bigger clang.exe PDB that mine

[2024-10-02 21:18] Matti: even though mine isn't exactly small eiter

[2024-10-02 21:19] Matti: actually that's probably more chromium than Qt but yeah

[2024-10-02 21:21] Matti: I swear I used to get > 4 GB PDBs for monolithic UE executables, but the biggest I can find now is a mere 2 GB

[2024-10-02 21:22] Matti: this only with full LTO though, no PGO yet

[2024-10-02 21:22] Matti: that might work

[2024-10-02 21:28] James: [replying to mrexodia: "get rekt nerd"]
:o

[2024-10-02 21:51] Matti: [replying to prick: "are you guys trying to crash ida's pdb parser or s..."]
if you use IDAPDB probably yeah

[2024-10-02 21:51] Matti: but this is a mistake to begin with

[2024-10-02 21:52] prick: what's the alternative?

[2024-10-02 21:52] Matti: MS DIA

[2024-10-02 21:52] Matti: the latest

[2024-10-02 21:52] Matti: the source code for this is in the SDK

[2024-10-02 21:52] prick: can this be used within IDA to not crash?

[2024-10-02 21:52] Matti: you just need to add the GUID for msdia140

[2024-10-02 21:53] Matti: [replying to prick: "can this be used within IDA to not crash?"]
well it can't be used outside of IDA

[2024-10-02 21:53] Matti: so idg the question

[2024-10-02 21:53] Matti: but yes it fixes all PDB crashes I've seen

[2024-10-02 21:53] prick: i dont know what a "MS DIA" is or where to find it

[2024-10-02 21:54] Matti: its the MS DIA (= ?? idk,, gooogle it) SDK

[2024-10-02 21:54] Matti: it comes with visual studio

[2024-10-02 21:54] prick: hmmmm

[2024-10-02 21:56] Matti: you can use MS DIA even witout recompiling the plugin, but tat is just as bad because IDA uses like the VS 2005 version at best by default

[2024-10-02 21:56] Matti: which you can actually use to craft PDB files that will crash IDA specifically

[2024-10-02 21:57] Matti: most IDAPDB crashes sseem to be kind of random generic "it just sucks" related

[2024-10-02 21:57] prick: [replying to Matti: "you just need to add the GUID for msdia140"]
now i need to figure out what this means

[2024-10-02 21:57] Matti: though one thing it will always crash on for example is xbox 360 PBs

[2024-10-02 21:58] prick: amazing

[2024-10-02 21:58] prick: what about nt 4.0 ppc

[2024-10-02 21:58] Matti: it's just COM shit

[2024-10-02 21:58] Deleted User: COM 😭

[2024-10-02 21:58] Matti: yeah well it's still an MS library

[2024-10-02 21:58] Azrael: [replying to prick: "i dont know what a "MS DIA" is or where to find it"]
> <https://learn.microsoft.com/en-us/visualstudio/debugger/debug-interface-access/debug-interface-access-sdk?view=vs-2022>

<:ThumbsUpBlob:585620701453746179>

[2024-10-02 21:59] Matti: I'm only saying it's less shit than IDA's

[2024-10-02 21:59] prick: [replying to Azrael: "> <https://learn.microsoft.com/en-us/visualstudio/..."]
already found it on disk

[2024-10-02 21:59] prick: [replying to Matti: "I'm only saying it's less shit than IDA's"]
i'm still not sure on how to integrate it with ida

[2024-10-02 22:00] Matti: you don't need to understand COM or MS DIA to make this improvement

[2024-10-02 22:00] Matti: there's an array of DLL names

[2024-10-02 22:00] Matti: and an array of GUIDs

[2024-10-02 22:00] prick: what do i do with the dll in order to get ida to use it

[2024-10-02 22:00] Matti: compile it from source to not suck

[2024-10-02 22:01] Matti: I was in the profcess of describing this

[2024-10-02 22:01] prick: okay

[2024-10-02 22:01] prick: and then?

[2024-10-02 22:02] Matti: when IDA loads the plugin, the plugin will try to load each msdiaXX.dll in the arrrays (the name and GUID for each version should match)
starting with the "best" (= highest version), and breaking after a sucessful load

[2024-10-02 22:03] Matti: the GUID is for one of the interfaces in MS DIA, I can find it for you but you can also just do it yourself I'm pretty sure

[2024-10-02 22:04] prick: okay now, what ida plugin are we talking about

[2024-10-02 22:04] prick: you are leaving many things unexplained

[2024-10-02 22:04] Matti: pdb.dll

[2024-10-02 22:04] prick: ok

[2024-10-02 22:04] Matti: am I really

[2024-10-02 22:04] prick: yea

[2024-10-02 22:04] Matti: continue

[2024-10-02 22:05] prick: still not sure what arrays/guids means in order to get pdb plugin to use msdia140.dll

[2024-10-02 22:05] prick: what commands, what files do i move, etc

[2024-10-02 22:05] Matti: read the source code

[2024-10-02 22:06] prick: can't find it

[2024-10-02 22:06] Matti: I'm not even gonna appologize for that one

[2024-10-02 22:06] Matti: it's utterly trivial code and you'll get it when you see it

[2024-10-02 22:06] Matti: which you do by grepping for msdia

[2024-10-02 22:06] Matti: or msdia80.dll or whatever

[2024-10-02 22:07] Matti: [replying to prick: "can't find it"]
that's strange, I feel like I mentioned where it is

[2024-10-02 22:07] Matti: maybe you have a different  SDK

[2024-10-02 22:07] prick: this is the only thing on ms github
[Attachments: image.png]

[2024-10-02 22:07] Matti: *the IDA SDK*

[2024-10-02 22:08] Matti: what other SDK would contain the source code for the IDA PDB plugin

[2024-10-02 22:08] prick: i thought you meant ms dia source

[2024-10-02 22:09] Matti: MS DIA isn't open source, ut you don't need its source forthis

[2024-10-02 22:09] Matti: just the filename and GUILD combination

[2024-10-02 22:09] Matti: and due to COM being COM, mabe regsvr32 the DLL

[2024-10-02 22:10] Matti: this is needed if you installed VS 2022 first and then some older version

[2024-10-02 22:11] prick: never got the ida sdk for the version i primarily use so let me find some shit

[2024-10-02 22:13] Matti: all of the SDKs (and most releases...) are on fckilfk

[2024-10-02 22:13] Matti: I can't llink it due to <#835634425995853834>

[2024-10-02 22:13] Matti: but it's not a super difficult site to find

[2024-10-02 22:13] Brit: piracy bad mkay

[2024-10-02 22:14] Brit: :^)

[2024-10-02 22:14] Matti: yeah but fuck ilfak more

[2024-10-02 22:15] Matti: I endorse IDA piracy only I can't actively help people with it in public due to discord's fascist TOS

[2024-10-02 22:18] prick: good thing a direct messaging feature exists

[2024-10-02 22:20] Matti: and google too

[2024-10-02 22:20] Matti: what would we do without em

[2024-10-02 22:23] prick: the difficulty of trying to extract information out of someone in a passive aggressive mood ....

[2024-10-02 22:26] Mikewind22: I don't have link to IDA RC1 torrent link - DM for confirmation... ;)

[2024-10-02 22:27] prick: i have that i am just looking for the sdk for 7.6 sp1

[2024-10-02 22:27] Termolyx: 
[Attachments: nQFVNtG.mp4]

[2024-10-02 22:29] prick: found it

[2024-10-02 22:40] Matti: [replying to prick: "the difficulty of trying to extract information ou..."]
> why would anyone ever get annoyed at being contradicted at every turn by someone who reads half of his attempts to hel,p, at best

[2024-10-02 22:41] Matti: also not phrasing everything in the absolute most assholeish way would go a long way

[2024-10-02 22:41] Matti: [replying to prick: "you are leaving many things unexplained"]
you could try posting questions instead of things like this

[2024-10-02 22:43] Deleted User: theres a rc1 now?

[2024-10-02 22:45] Deleted User: could someone yknow in private please

[2024-10-02 22:45] Deleted User: 🙂

[2024-10-02 22:50] Deleted User: think i got it

[2024-10-02 22:56] prick: [replying to Matti: "also not phrasing everything in the absolute most ..."]
please refer to the nametag

[2024-10-02 22:57] Deleted User: FYI i bought a license from hex-rays.com 🙂

[2024-10-02 22:58] Termolyx: [replying to Deleted User: "FYI i bought a license from hex-rays.com 🙂"]
Imagine actually buying a license

[2024-10-02 22:58] Termolyx: !!!!

[2024-10-02 22:58] Matti: [replying to prick: "good thing a direct messaging feature exists"]
oh yeah I forgot about this tip:
maybe don't call others "passive-aggressive" if you yourself are posting to complain that **someone else** di not message **you** for your IDA SDK for a version you did not specify

[2024-10-02 22:58] Matti: like, you could be more poactive in this I feel

[2024-10-02 22:59] prick: [replying to Matti: "like, you could be more poactive in this I feel"]

[Attachments: image.png]

[2024-10-02 22:59] prick: sorry for any miscommunications

[2024-10-02 22:59] Matti: yeah the whole "I'm just an asshole, sorry but not really that is justmy personality" thing

[2024-10-02 22:59] Matti: I get it

[2024-10-02 23:00] Matti: it's very .. uh well it's a personality for sure

[2024-10-02 23:00] prick: be careful i'll start posting patrick bateman

[2024-10-02 23:00] Matti: but you could consider a more agreeable one

[2024-10-02 23:01] Matti: [replying to prick: "be careful i'll start posting patrick bateman"]
well if I were really petty enough then you're just giving me ammunition to get you banned

[2024-10-02 23:01] Matti: <#835635446838067210>

[2024-10-02 23:01] Matti: not #shitposting

[2024-10-02 23:03] prick: well, just who is the asshole?

[2024-10-02 23:04] Matti: there can be multiple

[2024-10-02 23:04] Brit: me

[2024-10-02 23:04] Matti: but you are one

[2024-10-02 23:04] Brit: I am the asshole

[2024-10-02 23:04] Matti: see

[2024-10-02 23:04] Matti: multiple

[2024-10-02 23:04] Brit: in fact, we could capitalize it

[2024-10-02 23:09] Matti: privatize, then capitalize

[2024-10-02 23:09] Matti: the brymko strategy

[2024-10-02 23:09] Brit: I meant more like "I am the Asshole"

[2024-10-02 23:09] Brit: but that also works

[2024-10-02 23:10] Matti: yeah ik I just thought of te combination and thought it sounded pretty good

[2024-10-02 23:11] Matti: like, tories would've done better if I had done their slogan work for them

[2024-10-02 23:11] Matti: though thatcher probably already said this daily and I'm not even inventing anything new here

[2024-10-02 23:16] Brit: it is empowering to know you're the antagonist in the story

[2024-10-02 23:17] Matti: ikr

[2024-10-02 23:17] Matti: it's just so *nice*

[2024-10-02 23:18] Matti: one of the most satisfying feelings

[2024-10-02 23:19] Matti: some people are mistaken for protagonists or even just random side characters for years.... I know I have been

[2024-10-02 23:19] James: what was the whole ida9 accidental leak thing?

[2024-10-02 23:19] James: was it intentional?

[2024-10-02 23:19] James: or really an accident?

[2024-10-02 23:19] Matti: but when recognition finally comes it is so overhwelmingly good

[2024-10-02 23:19] James: or russian spyware release?

[2024-10-02 23:19] Matti: hex-rays are morons

[2024-10-02 23:19] Matti: is the explanation AFAIK

[2024-10-02 23:20] Matti: bbut now the binaries are also no longer watermarked

[2024-10-02 23:20] Brit: well for rc1 at least

[2024-10-02 23:20] Matti: so they will be leaked no matter what

[2024-10-02 23:20] Brit: we don't really know going forwards

[2024-10-02 23:20] Matti: yeah true

[2024-10-02 23:20] Brit: probably they realize the fuckup and starts wmarking again

[2024-10-02 23:20] Deleted User: if i start spamming steam gifts bro in 2 days know its not me

[2024-10-02 23:22] James: the cfg layout watermarking is gone?

[2024-10-02 23:22] Matti: [replying to Brit: "probably they realize the fuckup and starts wmarki..."]
imagine if some day they changed the keys so that the modulus patch would need updating

[2024-10-02 23:22] Brit: that'd be sad

[2024-10-02 23:22] Matti: utter chaos

[2024-10-02 23:22] Brit: for about a day

[2024-10-02 23:22] Matti: world in panic

[2024-10-02 23:22] Matti: <:lillullmoa:475778601141403648> c'mon

[2024-10-02 23:22] Deleted User: the 7 hour war

[2024-10-02 23:22] Matti: it's super difficult!

[2024-10-02 23:23] Brit: they could change the whole format

[2024-10-02 23:23] Brit: too

[2024-10-02 23:23] Brit: imagine

[2024-10-02 23:23] Matti: the byte to patch might chanbee both in position and value! not to mention the number of them!

[2024-10-02 23:23] Brit: another extra 20 mins

[2024-10-02 23:23] Deleted User: i can imagine it would go something like the gtav battleeye addition

[2024-10-02 23:23] Matti: [replying to Brit: "another extra 20 mins"]
yeah, just the end of the world basically

[2024-10-02 23:24] Matti: cause how could anyone possibly crack IDA

[2024-10-02 23:24] Matti: without having IDA

[2024-10-02 23:24] Deleted User: yeah i wonder

[2024-10-02 23:24] Matti: a program that is notorious for being almost impossible to crack

[2024-10-02 23:25] Brit: ehh, no one was stripping the watermark

[2024-10-02 23:25] Brit: afaik

[2024-10-02 23:25] Brit: tantamount to uncrackable since post leak the person can't buy it again

[2024-10-02 23:25] James: [replying to James: "the cfg layout watermarking is gone?"]
??? how long has this not existed anymore?

[2024-10-02 23:25] Matti: that's true, but that's only for the nazi edition right

[2024-10-02 23:25] Matti: that code isn't in the other two license form, or so nama told me once, I think.....

[2024-10-02 23:26] Matti: oh nvm

[2024-10-02 23:26] Matti: I'm thinking of the floating license DRM

[2024-10-02 23:27] Brit: probably

[2024-10-02 23:27] Matti: which was actual cancer code right

[2024-10-02 23:27] Matti: but yeah no one ever removed the watermark

[2024-10-02 23:27] Matti: that I know of

[2024-10-02 23:27] Matti: dunno if this is a difficullty or time/boredom related issue honestly

[2024-10-02 23:28] Brit: sourcing multiple ida copies

[2024-10-02 23:28] Matti: yea

[2024-10-02 23:28] Brit: and coming up with a strategy to recompile the affected binaries

[2024-10-02 23:28] Brit: I mean it should be doable

[2024-10-02 23:28] Matti: it's nto difficult, but it is shit work that takes a lot of time

[2024-10-02 23:28] Brit: throw remill at it :^)

[2024-10-02 23:28] Brit: lift to llvm, recompile

[2024-10-02 23:28] Brit: done

[2024-10-02 23:30] Matti: hmmm would recompilaton be an absolute requirement?

[2024-10-02 23:30] Matti: I mean the real goal is to remove association with the owner/buyer right

[2024-10-02 23:30] Matti: what if the watermark was corruptible

[2024-10-02 23:30] Matti: but in a way that kept the binary working

[2024-10-02 23:31] Brit: I expect that they have multiple ways of recovering the original owner

[2024-10-02 23:31] Matti: yeah.... unknowns everywhere

[2024-10-02 23:32] James: cfg and encoded immediates are whats known but probably more

[2024-10-02 23:32] James: so lifting might help with both of those

[2024-10-02 23:32] mrexodia: yeah bro 'just' lift the binary perfectly

[2024-10-02 23:32] James: if the optimizer removed removed operations that use opaque immeduates that are used to identify

[2024-10-02 23:32] mrexodia: and reorder the functions and data by size

[2024-10-02 23:33] James: data reordering can't be done so that would be unfortunate

[2024-10-02 23:33] Brit: [replying to mrexodia: "yeah bro 'just' lift the binary perfectly"]
this

[2024-10-02 23:33] Brit: this trivial problem

[2024-10-02 23:33] Brit: here

[2024-10-02 23:34] Matti: sheesh, bunch of pessimists here

[2024-10-02 23:34] Matti: it's not the fucking halting problem ok

[2024-10-02 23:34] Matti: just lift the binary

[2024-10-02 23:34] James: it is literally undecidable

[2024-10-02 23:34] Matti: perfectly

[2024-10-02 23:34] Brit: bet y'all cowards don't even lift

[2024-10-02 23:35] Matti: [replying to James: "it is literally undecidable"]
but is it eqivalent to the halting problem?

[2024-10-02 23:35] James: [replying to Matti: "but is it eqivalent to the halting problem?"]
mmmmmmmmmmmm

[2024-10-02 23:36] James: i think the reasons we don't know if a program halts are similar to the reasons we don't know if we've properly lifted a binary

[2024-10-02 23:37] James: a program that takes user input to decide to enter an infinite loop

vs

a program that takes user input to decide a jump destination

[2024-10-02 23:37] Matti: I would agree

[2024-10-02 23:37] Matti: but still write out the proof in full pls

[2024-10-02 23:37] James: but beyond that i couldn't possibly reason about it

[2024-10-02 23:38] mrexodia: https://codedefender.io/blog/2024/07/02
[Embed: Technical Challenges of Indirect Control Flow]
This article discusses the challenges any binary analysis framework will face with indirect control flow. It covers indirect calls, jump tables (indirect jumps), and details our approach.

[2024-10-02 23:38] mrexodia: you can read this poast

[2024-10-02 23:38] Matti: otherwise I will continue to revert to
> it's not exactly the fucking halting problem is it

[2024-10-02 23:38] Matti: god dammit duncan

[2024-10-02 23:38] Matti: ok fine you win

[2024-10-02 23:38] Matti: the pessimists win

[2024-10-02 23:38] mrexodia: im actually optimistic here!

[2024-10-02 23:38] Matti: it turns out that it's actually kinda hard maybe

[2024-10-02 23:38] mrexodia: the main problem isn't lifting the code, it would be recovering the data ranges

[2024-10-02 23:38] James: [replying to mrexodia: "https://codedefender.io/blog/2024/07/02"]
great post 🤣

[2024-10-02 23:39] subgraphisomorphism: [replying to mrexodia: "and reorder the functions and data by size"]
consider it done!

[2024-10-02 23:39] subgraphisomorphism: excluding the data part

[2024-10-02 23:39] James: data...

[2024-10-02 23:39] James: could be interesting actually

[2024-10-02 23:39] James: if u did it dynamically

[2024-10-02 23:39] James: traced what accesses what

[2024-10-02 23:40] James: as long as ur corpus is large enough, it will probably work

[2024-10-02 23:40] mrexodia: 100% coverage

[2024-10-02 23:40] mrexodia: then you can sell that to hex-rays as proper tests

[2024-10-02 23:40] mrexodia: <:KEKW:912974817295212585>

[2024-10-02 23:40] Brit: <:kekw:904522300257345566>

[2024-10-02 23:41] James: all i need is to open ida, load a binary to x64, scroll in the disassembly aimlessly, press f5, and close ida

[2024-10-02 23:41] Brit: real

[2024-10-02 23:41] James: so i'll trace that execution path, and call it a day

[2024-10-02 23:41] Brit: turing complete computers were a mistake

[2024-10-02 23:42] mrexodia: [replying to James: "so i'll trace that execution path, and call it a d..."]
hope you don't approach your bin2bin in such an unsound way

[2024-10-02 23:42] mrexodia: <:kappa:697728545631371294>

[2024-10-02 23:42] Matti: ok fuck I gotta go
something actually urgent just came up
[Attachments: image.png]

[2024-10-02 23:43] James: [replying to mrexodia: "hope you don't approach your bin2bin in such an un..."]
i would never!

[2024-10-02 23:43] subgraphisomorphism: we just use llvm

[2024-10-02 23:43] Matti: I even fucking did this verification bullshit

[2024-10-02 23:43] subgraphisomorphism: and we also use x64dbg tracer plugin

[2024-10-02 23:43] subgraphisomorphism: to uncover control flow

[2024-10-02 23:43] subgraphisomorphism: 👍

[2024-10-02 23:43] Matti: how can they just forget about this at random

[2024-10-02 23:43] James: remember how I PM'd you a while ago about tracing in x64dbg <@162611465130475520>

[2024-10-02 23:43] James: basicallyh i take that text output

[2024-10-02 23:43] James: throw that into llvm

[2024-10-02 23:44] James: and call it a day

[2024-10-02 23:44] James: works great 👍

[2024-10-02 23:44] Brit: soundest code recovery ever

[2024-10-02 23:44] subgraphisomorphism: we also use ghidra's pcode as our ir

[2024-10-02 23:44] subgraphisomorphism: llvm ir to pcode

[2024-10-02 23:44] subgraphisomorphism: and back

[2024-10-02 23:44] James: actually first though i go from text disassembly to bytes again using keystone, then using llvms decoder i go to machine instructions, then to llvm, then to pcode.

[2024-10-02 23:45] James: ok ok back to work now <@1290450044864172153> slave

[2024-10-02 23:45] subgraphisomorphism: yes we output the disassembly as text and then put it into asmjit

[2024-10-02 23:45] subgraphisomorphism: https://tenor.com/view/i-have-no-enemies-dog-with-a-butterfly-gif-6941508655603565844

[2024-10-02 23:45] Brit: that's cool and all, but when's all this releasing :^)

[2024-10-02 23:46] James: why am I a fed whisperer?

[2024-10-02 23:47] Brit: our heuristics tell us you talk to the fed

[2024-10-02 23:48] James: I don't own a pair of aviators...

[2024-10-03 03:27] Torph: [replying to Matti: "how can they just forget about this at random"]
that's really funny

[2024-10-03 12:19] Matti: well it's less funny if you've already had to do it once

[2024-10-03 12:20] Matti: it's kinda cancerous

[2024-10-03 12:21] Matti: plus I obviously am not even a game dev or  publisher at all, I just have a bullshit company registered in the epic games store so I can keep up with what they are up to better

[2024-10-03 12:21] Matti: steam idem dito

[2024-10-03 12:21] Matti: though I may* publish my legendary MattiSnake there some day for $1.00

[2024-10-03 12:26] Matti: damn, shocked to see I've never posted it here yet
[Attachments: Snake.Release.zip]

[2024-10-03 12:29] Matti: only thing is, when I made this 9 years ago there were working explosion graphics

[2024-10-03 12:29] Matti: this is a recompile and they seem to have disappeared somehow

[2024-10-03 12:30] Matti: other than that it is fully functional and the best game in the world

[2024-10-03 15:12] x86matthew: [replying to Matti: "damn, shocked to see I've never posted it here yet"]
10/10, most deranged snake

[2024-10-03 15:16] Matti: I still need to add some more essentials like a 'compiling shaders...' screen

[2024-10-03 15:16] Matti: but it's mostly feature complete

[2024-10-03 15:27] Brit: [replying to Matti: "damn, shocked to see I've never posted it here yet"]
<:kekw:904522300257345566>

[2024-10-03 15:27] Brit: I love it

[2024-10-03 15:27] Brit: you need more intro movies

[2024-10-03 15:29] Matti: I'm working on it

[2024-10-03 15:30] Matti: apparently this one tomb raider game from 2013 has more than this alone

[2024-10-03 15:30] Matti: so still need to get that

[2024-10-03 15:31] Matti: the dramatic quotes about war and  its victims are my favourite feature

[2024-10-03 15:32] Matti: they were entirely copied from a call of duty game that had them in a convenient text file

[2024-10-03 15:47] MDC: I'm not *entirely* sure if this fits within "permitted discussion" so feel free to tell me to piss off if it doesn't, but since SecretClub is the only resource I was able to find (the usual places just focus on bypassing etc) that has gone into the same sort of territory as what I'm trying to do, I figured it'd be worth asking here. 

I'm attempting to disable BattlEye on my own *server* instance, specifically while retaining RCON functionality (which is the tricky bit), and without touching the client in any way shape or form. Has anyone either a) already done this, or b) have any annotations for BEServer?

[2024-10-03 16:37] James: [replying to MDC: "I'm not *entirely* sure if this fits within "permi..."]
I did this on DayZ, you can launch the game without BE, and there is a setting in the server to not require BE from clients.

[2024-10-03 16:38] MDC: [replying to James: "I did this on DayZ, you can launch the game withou..."]
There *used* to be, unfortunately, back in 2019; you can still do it via Diag build, and you can also just JMP over the initialization of BEServer, but then you lose RCON as it's provided by BEServer_x64.dll

[2024-10-03 16:39] MDC: And yeah, this is also DayZ-related <:kekw:1256036418615250965>

[2024-10-03 16:39] James: funny coincidence. mmmmmmm are you sure? maybe I'm just losing my mind but I could have sworn I did this more recently than 2019

[2024-10-03 16:39] MDC: This article basically outlines the *problems*, but it handles them from a client perspective, and there's no real analysis of BEServer itself: https://secret.club/2020/07/06/bottleye.html
[Embed: BattlEye client emulation]
The popular anti-cheat BattlEye is widely used by modern online games such as Escape from Tarkov and is considered an industry standard anti-cheat by many. In this article I will demonstrate a method 

[2024-10-03 20:03] Dudcom: anyone doing flare-on?

[2024-10-03 20:40] daax: [replying to Dudcom: "anyone doing flare-on?"]
<@943099229126144030> did, probably some others

[2024-10-03 20:56] Brit: <@318786725700960257>

[2024-10-03 21:11] Dudcom: [replying to daax: "<@943099229126144030> did, probably some others"]
oh shi did they max ?

[2024-10-03 21:13] Dudcom: <@943099229126144030> how far ru

[2024-10-03 21:13] Brit: both have finished

[2024-10-03 21:13] Brit: I was also doing it but didn't have much time this week

[2024-10-03 21:14] Dudcom: oh damn

[2024-10-03 21:14] Dudcom: nice

[2024-10-03 21:15] Dudcom: I am on 5 preaty close but been busy hopefuly tonight tho

[2024-10-03 22:16] f0rk: https://github.com/arphanetx/Monocle
[Embed: GitHub - arphanetx/Monocle: Tooling backed by an LLM for performing...]
Tooling backed by an LLM for performing natural language searches against compiled target binaries. Search for encryption code, password strings, vulnerabilities, etc.  - GitHub - arphanetx/Monocle...

[2024-10-03 22:16] f0rk: how am i just hearing about this

[2024-10-03 22:17] f0rk: fkn <:yumshot:1289734272579665950>

[2024-10-04 05:47] idkhidden: [replying to Dudcom: "anyone doing flare-on?"]
Me i just got to 5

[2024-10-04 05:59] Dudcom: [replying to idkhidden: "Me i just got to 5"]
W just got to 6

[2024-10-04 05:59] Dudcom: Well earlier today

[2024-10-04 06:00] Dudcom: Need to lock in and get it tonight hopefully

[2024-10-04 06:02] idkhidden: [replying to Dudcom: "W just got to 6"]
nice

[2024-10-04 16:26] expy: congratz to the early flare-on finishers! was it harder than last year?

[2024-10-04 16:42] x86matthew: the earlier challenges were easier than last year

[2024-10-04 16:42] x86matthew: the later ones (1 or 2 of them in particular) were a similar difficulty to last year imo

[2024-10-04 16:49] x86matthew: a couple of the middle challenges had silly solutions this year (obviously can't go into any more details though)

[2024-10-04 17:00] Brit: [replying to x86matthew: "a couple of the middle challenges had silly soluti..."]
hdl chal go brr?

[2024-10-04 17:11] x86matthew: [replying to Brit: "hdl chal go brr?"]
yep that's one of them lol

[2024-10-04 17:13] Brit: I haven't finished yet, I hope I get some time this weekend

[2024-10-04 17:25] x86matthew: 7 and 9 were probably the hardest if i remember correctly

[2024-10-04 17:26] x86matthew: 10 (uefi) was my favourite though

[2024-10-04 17:38] Brit: I wanna know how they justify some of these once it's over

[2024-10-04 17:38] Brit: I wasted so much time

[2024-10-04 17:42] x86matthew: which one you up to?

[2024-10-04 18:30] Brit: stopped midway through 7

[2024-10-04 19:49] Leeky: this year was def easier than last, but also silly challenges that might as well have not existed inbetween

[2024-10-04 19:49] Leeky: both 9 and 10 were pretty good

[2024-10-04 19:50] Leeky: 7 as well but I didn't like the "last" part of it, that is just personal preference though, and honestly because of lack of knowledge kinda skill issue on my part

[2024-10-04 19:51] Leeky: I'll say that up to 9 I was pretty disappointed with the actual challenges, but 9 and 10 were fun enough to redeem it overall

[2024-10-04 23:02] ShekelMerchant: yo, not sure if im spamming just by asking this question: but how do I get started in reverse engineering?

[2024-10-05 06:16] Deleted User: learn how to C

[2024-10-05 06:16] Deleted User: install ida

[2024-10-05 06:17] Deleted User: see a button that you don't know what it does? google

[2024-10-05 09:17] anubian: [replying to ShekelMerchant: "yo, not sure if im spamming just by asking this qu..."]
write some code in C, compile it then open it in a disassembler.

[2024-10-05 09:17] anubian: understand how it works

[2024-10-05 09:17] anubian: play some ctfs

[2024-10-05 09:20] Deleted User: yeah

[2024-10-05 13:33] f0rk: Eventually you could play with other weird machines too. Play with EVM or BEAM or JVM byte code at some point for sure

[2024-10-06 01:28] ChloeOS: [replying to ShekelMerchant: "yo, not sure if im spamming just by asking this qu..."]
There's no other way bro
[Attachments: IMG_0330-1.jpg]

[2024-10-06 01:56] ShekelMerchant: https://tenor.com/view/wilk-z-wall-street-the-wolf-of-wall-street-laugh-lmao-ha-ha-ha-gif-22086762

[2024-10-06 19:12] Leeky: no I agree, I didn't expect to sit on 2 for an hour, though given the next "classical" reversing chals are at 5 and 7 respectively, I guess it is fine to skill check a bit early

[2024-10-06 21:26] eversinc33: [replying to idkhidden: "Me i just got to 5"]
same also at 5 now