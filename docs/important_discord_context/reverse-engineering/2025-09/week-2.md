# September 2025 - Week 2
# Channel: #reverse-engineering
# Messages: 267

[2025-09-08 23:39] Ghoall: A function has the variable retaddr wich is used in encryption, it will always be at [rsp + offset] (90h fe), is there anyway to extract the offset at runtime that i dont know of? Ida seems to distinguish it from other rsp based variables.

[2025-09-09 02:44] qfrtt: [replying to Ghoall: "A function has the variable retaddr wich is used i..."]

[Attachments: image.png]

[2025-09-09 15:58] Ghoall: [replying to qfrtt: ""]
how does this help me exactly? I need to get the offset at runtime from function bytes fe, not in ida.

[2025-09-09 16:01] Brit: it will always be at [rsp + offset] (90h fe)

[2025-09-09 16:01] Brit: if it's always at that offset, don't you just need rsp?

[2025-09-09 16:01] Ghoall: [replying to Brit: "if it's always at that offset, don't you just need..."]
it wont always be 90h, also to emulate it i use a fake stackframe so i have my own rsp

[2025-09-09 16:01] Ghoall: i was saying it will always be relative to rsp

[2025-09-09 16:02] Brit: ah

[2025-09-09 16:02] Brit: but if you are emulating it anyway

[2025-09-09 16:02] Brit: just disass the function

[2025-09-09 16:02] Brit: and grab the offset no?

[2025-09-09 16:03] Ghoall: yes im asking how i do that lol

[2025-09-09 16:03] Brit: what are you emulating with?

[2025-09-09 16:03] Ghoall: unicorn

[2025-09-09 16:04] Brit: is the only thing that changes the offset?

[2025-09-09 16:04] Brit: or is the entire func you care about rewritten?

[2025-09-09 16:04] Ghoall: the entire function is rewritten but i already handle all of that, i just want to write a custom value to the place where retaddr is accessed, in all cases rsp + offset

[2025-09-09 16:08] Brit: so then, import capstone, get the bytes you care about by using the uc.mem_read() func

[2025-09-09 16:08] Brit: and then using capstone disassemble, get the operands for from the instr

[2025-09-09 16:08] Brit: from there you have the offset

[2025-09-09 16:10] Ghoall: hmm i use zydis instead of capstone, but the thing is there is not a clear instruction wich indicates the retaddr being accessed, there are many [rsp + offset] accesses, so im wondering how i distinguish them?

[2025-09-09 16:11] Ghoall: i understand i can get the offset via the operand, but then i need to know what instruction accesses the retaddr

[2025-09-09 16:12] Brit: are you getting this whole rsp+offset business from ida decomp?

[2025-09-09 16:12] Brit: it is time to sit down and look at the disass to actually understand the function

[2025-09-09 16:12] Ghoall: im looking at the assembly.

[2025-09-09 16:13] Ghoall: ```
.text:0000000004E419EF 090                 lea     rcx, [rsp+10h] -> random variable
.text:0000000004E419F4 090                 mov     r15d, ecx
.text:0000000004E419F7 090                 mov     r14d, 237F2A9Eh
.text:0000000004E419FD 090                 sub     r14d, ecx
.text:0000000004E41A00 090                 and     ecx, 0E782D880h
.text:0000000004E41A06 090                 or      r15d, 0E782D887h
.text:0000000004E41A0D 090                 add     r15d, ecx
.text:0000000004E41A10 090                 mov     ecx, r15d
.text:0000000004E41A13 090                 xor     ecx, r14d
.text:0000000004E41A16 090                 and     r14d, r15d
.text:0000000004E41A19 090                 mov     r15, [rsp+90h] -> retaddr
```

[2025-09-09 16:13] Ghoall: this is what i mean

[2025-09-09 16:19] Brit: can you show the entire func?

[2025-09-09 16:19] Brit: cause as such r15 is completely independent from all the other garbage above it

[2025-09-09 16:20] Brit: since it just gets stomped by whatever's on stack at [rsp+90h]

[2025-09-09 16:38] Ghoall: [replying to Brit: "can you show the entire func?"]

[Attachments: message.txt]

[2025-09-09 16:39] Ghoall: ```
retaddr; // [rsp+90h]
```

[2025-09-09 17:06] Brit: [replying to Ghoall: "i understand i can get the offset via the operand,..."]
I don't really understand this requirement, if you are emulating you know where you return to at retn since you can just read it off the stack, but it is manipulating stuff on the stack with 1 byte movs such as 
4E41628 or 4E41666 etc

[2025-09-09 17:10] Brit: but also, the pointer this thing composes just ends up in eax before return, so you can also grab it from there

[2025-09-09 17:11] Ghoall: [replying to Brit: "I don't really understand this requirement, if you..."]
well, in this case, rsp + 90h is used as a key in the decryption, and its not set in this function, since its the retaddr value. Now when the original process calls it it will have the right retaddr set, but when i emulate it i need to change the retaddr manually. But for that i need to know explicitly where its stored on the stack, and i saw ida is able to recognize it as retaddr, so i was wondering if i can do the same.

[2025-09-09 17:11] Ghoall: [replying to Brit: "but also, the pointer this thing composes just end..."]
therefore the function needs to execute first tho???

[2025-09-09 17:12] Ghoall: i am grabbing eax at the end yes

[2025-09-09 17:13] Brit: that should contain whatever they construct with the 4 1byte movs

[2025-09-09 17:14] Brit: since its [rsp + 0x90 - 0x84] which is just rsp + 0x0C

[2025-09-09 17:34] qfrtt: [replying to Ghoall: "```
retaddr; // [rsp+90h]
```"]
bruh if you put stack pointer like i showed you it will give u exactly the offset you need

[2025-09-09 17:35] qfrtt: its just a matter of calculating pushes and pops or vice versa

[2025-09-09 17:38] qfrtt: like ida will slightly color the offset in red it's because it does the calc

[2025-09-09 17:38] qfrtt: at the moment of the call it's always [rsp] but then further in the function push and pop instructions will change it

[2025-09-09 17:39] qfrtt: so at X instruction in the function it would be the sum of the pushes / pops

[2025-09-09 17:40] qfrtt: idk the fuck u using emulation for in this case

[2025-09-09 17:45] qfrtt: [replying to Ghoall: ""]
just look here u have 8 pushes which equals to 0x8 * 0x8

[2025-09-09 17:45] qfrtt: then u have sub rsp 0x50

[2025-09-09 17:45] qfrtt: 0x40 + 0x50 = 0x90

[2025-09-09 17:46] qfrtt: ```.text:0000000004E41A19 090                 mov     r15, [rsp+90h] -> retaddr```

[2025-09-09 17:46] qfrtt: quik mafs

[2025-09-09 18:09] Ghoall: Yeah that makes sense ty

[2025-09-09 18:17] Ghoall: [replying to qfrtt: "idk the fuck u using emulation for in this case"]
Its because im making it for different binaries, it was not useful info

[2025-09-09 23:14] Deleted User: Hey there 👋 14 year old here trying to learn more about Reverse engineering, and overall using IDA. I am experienced in proper coding involving C & Rust.

[2025-09-09 23:16] Addison: Top tip, stop announcing your age

[2025-09-09 23:16] Addison: It will only attract the wrong sort and make people doubt your skill level (yay, ageism)

[2025-09-09 23:46] UJ: Also saying you are under 18 puts the discord server at risk in addition to your account.

[2025-09-09 23:49] the horse: he's ragebaiting
[Attachments: image.png]

[2025-09-10 00:31] Addison: [replying to the horse: "he's ragebaiting"]
Or spent a nonzero amount of time in RE servers lmao

[2025-09-10 01:14] TRUCK: https://youtu.be/by53T03Eeds
[Embed: This C code should be ILLEGAL.  It's also fantastic.]
This C code is disgusting.  But also really, really clever.

The International Obfuscated C Code Contest (IOCCC) has been consistently breaking human minds since 1984.  This year's winners are out of 

[2025-09-10 01:22] TRUCK: can we just make clones of her so we have one for each of us?

[2025-09-10 02:23] Xits: why would you use ghidra instead of running the preprocessor and some formatter?

[2025-09-10 03:54] TRUCK: [replying to Xits: "why would you use ghidra instead of running the pr..."]
pls go easy on her >_<

[2025-09-10 03:54] the horse: [replying to TRUCK: "can we just make clones of her so we have one for ..."]
true

[2025-09-10 03:55] the horse: btw she's like 5"

[2025-09-10 03:57] rin: fair

[2025-09-10 06:49] Addison: y'all seriously need to touch some grass goddamn

[2025-09-10 12:59] dullard: [replying to TRUCK: "can we just make clones of her so we have one for ..."]


[2025-09-10 12:59] dullard: What a weird thing to say 😂

[2025-09-10 15:08] luci4: [replying to dullard: "What a weird thing to say 😂"]
This is what happens when you're terminally online

[2025-09-10 15:29] avx: <a:Joe_shock:1223822861370921051>

[2025-09-10 15:40] c0z: Being terminally online isn't all that bad

[2025-09-10 16:41] TRUCK: my joke has successfully triggered a large response, mission accomplished

[2025-09-10 18:35] Horsie: Her videos make me visibly cringe most of the time. Cant believe people simp for her.

[2025-09-10 18:36] Horsie: I always thought she appeals to the red team crowd

[2025-09-10 18:37] Horsie: Some real 'low level' tier takes.

[2025-09-10 18:37] Horsie: inb4 sexist/incel

[2025-09-10 19:24] iris: [replying to Horsie: "Some real 'low level' tier takes."]
such as? dont rlly watch her videos just kinda curious about the 'low level' type takes 😭

[2025-09-10 22:26] Addison: [replying to Horsie: "Her videos make me visibly cringe most of the time..."]
this is me @ most security influencers tbh

[2025-09-10 22:26] Addison: I just don't think it is winnable

[2025-09-10 22:27] Addison: People doing cool projects: neat, cool, learning things

People having opinions about security: doomed to be cringe

[2025-09-11 16:52] Oliver: Did windows just remove blue screen in latests update?

[2025-09-11 16:52] Oliver: 
[Attachments: image.png]

[2025-09-11 16:52] Oliver: 
[Attachments: image.png]

[2025-09-11 17:00] Addison: blue was too recognisable

[2025-09-11 17:25] Matti: [replying to Oliver: "Did windows just remove blue screen in latests upd..."]
it depends

[2025-09-11 17:25] Matti: they seem to be A/B testing it
[Attachments: image.png]

[2025-09-11 17:26] Matti: this 'FeatureEnabledBsodRejuvenation' comes from a feature ID

[2025-09-11 17:27] Matti: I *think* you can toggle it with ViveTool (<https://github.com/thebookisclosed/ViVe>)
`ViVeTool.exe /enable /id:50070238`, or /disable obviously

[2025-09-11 17:29] Matti: you can also query the current value... it said not present for me initially, but /enable still worked

[2025-09-11 17:29] Matti: guess I'll need to reboot now and then force a bugcheck <:kekw:904522300257345566>

[2025-09-11 17:29] Oliver: [replying to Matti: "guess I'll need to reboot now and then force a bug..."]
for science

[2025-09-11 17:38] Matti: yeah it works
[Attachments: image.png]

[2025-09-11 17:39] Matti: I actually had to do it twice because it helpfully also enabled auto-reboot for me

[2025-09-11 17:40] Matti: and since I haven't got a 128G pagefile making a crash dump takes like 0.5 seconds

[2025-09-11 17:42] Matti: well this matches sure enough
[Attachments: image.png]

[2025-09-11 17:42] Matti: great feature <:harold:704245193016344596>

[2025-09-11 17:42] Matti: time to kill

[2025-09-11 17:47] koyz: [replying to Oliver: "Did windows just remove blue screen in latests upd..."]
No no, they just turned the BSOD into a BSOD <:kappa:716953351622885418>

[2025-09-11 17:48] Oliver: Black screen of death? I see what they did there

[2025-09-11 17:49] koyz: [replying to Oliver: "Black screen of death? I see what they did there"]
Exactly <:kekw:904522300257345566>

[2025-09-11 17:50] Matti: [replying to Oliver: "for science"]
for completion achievement: same system after running `/disable /id:50070238`
[Attachments: image.png]

[2025-09-11 17:52] Matti: it's not strictly speaking any more informative, both screens are already shit

[2025-09-11 17:52] Matti: I just dislike the colour change because black is already in use for fatal bootloader errors

[2025-09-11 17:53] Oliver: Ye what about some pc, image, some registers maybe

[2025-09-11 17:53] Matti: yeah, best you can do is get the args back

[2025-09-11 17:53] Matti: they're just all zeroes for this particular bugcheck

[2025-09-11 17:55] Matti: my settings for bsods
[Attachments: image.png]

[2025-09-11 17:56] Oliver: Just getting rip and current image base is enough for most fast debugging and all registers

[2025-09-11 17:56] Matti: bring back NT4
[Attachments: bluescreen_e_02.gif]

[2025-09-11 17:57] Oliver: Ye thats good

[2025-09-11 17:57] Oliver: Fast and easy diff check on rip and image and u have where u crashed

[2025-09-11 17:58] Matti: what's insane to me is that MS kernel devs also fucking hate this

[2025-09-11 17:58] snowua: [replying to Matti: "yeah it works"]
clean your monitor vro 💔

[2025-09-11 17:58] Matti: but they don't get to decide?!

[2025-09-11 17:58] Matti: [replying to snowua: "clean your monitor vro 💔"]
no I refuse

[2025-09-11 17:59] Matti: I actually attempt-cleaned it just yesterday, I think that's why it looks even worse than usual

[2025-09-11 17:59] Oliver: [replying to snowua: "clean your monitor vro 💔"]
That wasnt even bad?

[2025-09-11 18:00] Matti: I should really get one of those monitor cleaning shits some day probably, nothing meant for household cleaning seems to improve the situation

[2025-09-11 18:00] Matti: [replying to Oliver: "That wasnt even bad?"]
but also this, IME monitors always look dirty af when you take a close up photo

[2025-09-11 18:01] Oliver: [replying to Matti: "but also this, IME monitors always look dirty af w..."]
For me that didnt even look dirty at all

[2025-09-11 18:01] Oliver: Its just pixels

[2025-09-11 18:01] Oliver: Showing?

[2025-09-11 18:01] Matti: first one does

[2025-09-11 18:01] Matti: cause it's black <:kekw:904522300257345566>

[2025-09-11 18:01] Oliver: [replying to Matti: "yeah it works"]
.

[2025-09-11 18:01] Matti: another reason to hate the change

[2025-09-11 18:01] Oliver: Didnt see that one xd

[2025-09-11 18:01] Oliver: Ye

[2025-09-11 18:01] Matti: [replying to Matti: "cause it's black <:kekw:904522300257345566>"]
wait... that came out kinda wrong

[2025-09-11 18:02] Matti: but well it is black though

[2025-09-11 18:03] Matti: I guess I'm a blue smurf supremacist deep down

[2025-09-11 18:03] Oliver: [replying to Matti: "cause it's black <:kekw:904522300257345566>"]
Pun intended?

[2025-09-11 18:03] Matti: no I only realised right after typing it

[2025-09-11 18:03] Matti: ha

[2025-09-11 18:03] Oliver: Haha

[2025-09-11 18:03] segmentationfault: there are no coincidences <:Kapp:1114333831965716571>

[2025-09-11 18:04] Matti: I agree

[2025-09-11 18:04] Matti: sucks to find out you're racist 😔

[2025-09-11 18:06] Matti: [replying to Matti: "I *think* you can toggle it with ViveTool (<https:..."]
between this shit and 'DisplayPreReleaseColor', I really wonder
would it have been so hard to just make it a hex RGB parameter instead

[2025-09-11 18:07] Matti: we all know they're gonna change it to magenta or something in 3 years anyway

[2025-09-11 18:08] qfrtt: was it green in insider builds ?

[2025-09-11 18:09] segmentationfault: [replying to Matti: "sucks to find out you're racist 😔"]
hitler money gang

[2025-09-11 18:09] Matti: [replying to qfrtt: "was it green in insider builds ?"]
I think so

[2025-09-11 18:42] Brit: [replying to qfrtt: "was it green in insider builds ?"]
can confirm

[2025-09-11 18:42] Brit: I have here a leanMeanGreenScreenMachine.sys file

[2025-09-11 18:42] Brit: which can only refer to that

[2025-09-11 19:34] daax: [replying to Matti: "bring back NT4"]
fr

[2025-09-11 19:34] daax: more helpful than there “minimalist” one

[2025-09-11 19:50] Addison: [replying to Addison: "Man I miss frostiest"]
another year, not forgotten 🫡

[2025-09-11 20:11] L1ney: did he die

[2025-09-11 20:12] L1ney: geniue question

[2025-09-11 20:12] Addison: Idk what happened

[2025-09-11 20:12] L1ney: just disappeared ?

[2025-09-11 20:12] Addison: Yeah p much

[2025-09-11 20:13] L1ney: prolly drugs

[2025-09-11 20:15] the horse: i think he got deported

[2025-09-11 20:16] the horse: i don't think he was using drugs

[2025-09-11 20:16] L1ney: deported for drugs

[2025-09-11 20:16] L1ney: rip

[2025-09-12 00:03] Xits: What gui debugger do you guys use for Linux?

[2025-09-12 00:03] Xits: Is there anything as good as x64dbg?

[2025-09-12 00:04] Xits: that also supports arm preferably

[2025-09-12 00:07] truckdad: gdb tui might scratch the itch

[2025-09-12 00:07] truckdad: it's not very known

[2025-09-12 00:16] lom: binja

[2025-09-12 00:38] Xits: [replying to truckdad: "gdb tui might scratch the itch"]
From my brief looking there’re still a lot of keybinds 😭

[2025-09-12 00:39] Xits: [replying to lom: "binja"]
Isn’t that paid?

[2025-09-12 00:55] lom: free version also supports ARM

[2025-09-12 00:55] lom: https://binary.ninja/purchase/#non-commercial
[Embed: Binary Ninja - Purchase]
Compare different editions and purchase

[2025-09-12 00:55] lom: for non-commercial use

[2025-09-12 01:28] UJ: [replying to Xits: "Isn’t that paid?"]
if you are a student, its only 75 bucks.

[2025-09-12 02:27] Xits: I’m poor

[2025-09-12 13:31] GG: do you guys know how to get the structures window in IDA Pro, because its not appearing I have IDA Pro 9.2, and its not there in the subviews, I know the there is local types but I always see people using the actual Structures window, and I can't figure out where it is

[2025-09-12 13:33] Pepsi: [replying to GG: "do you guys know how to get the structures window ..."]
unified types is a thing since 8.4

[2025-09-12 13:35] GG: yes I know its old, but where is it

[2025-09-12 13:37] GG: looking at the subviews menu its not avaliable

[2025-09-12 13:37] GG: should it be enabled somewhere ?

[2025-09-12 13:40] Pepsi: https://docs.hex-rays.com/release-notes/8_4

> New databases will only have Local Types by default and Structures and Enums are deprecated.
[Embed: IDA 8.4 | Hex-Rays Docs]

[2025-09-12 13:41] GG: oh lol

[2025-09-12 19:51] segmentationfault: I don't want to be a hater, but my aversion to drm might have had contribution to this question. i bought the newest borderlands and the game runs slower than my crippled aunt with arthirtis even though i have pretty strong rig , the question mainly stems from how denuvo was implemented in ac origins, If i recall correctly it was the only time the drm was fully removed from the game binary and some consider it the best crack ever from the scene, though I lack that much re knowledge to give opinions on the matter just what i heard

plz tell me how much truth is there in the  fps impact of denuvo in games? the game is probably poorly optimized on the UE engine, but does the anti-tamper (possibly with VMP on top) contribute a significant performance hit?

[2025-09-12 19:53] the horse: little impact.

[2025-09-12 19:53] the horse: used to be much worse

[2025-09-12 19:53] Brit: practically nothing compared to the issues arising from game devs getting their programming degrees in lootboxes

[2025-09-12 19:54] the horse: if the game itself has VMP on top, it might be a performance hit, depending on how much is virtualized, their mutation does not have a significant performance penalty unless the entire game (or a lot of hot areas) is mutated

[2025-09-12 19:54] the horse: denuvo runs selectively and not very often

[2025-09-12 19:55] Brit: they do a lot of things but theyre all fairly low impact performance wise as far as I looked into it

[2025-09-12 19:55] segmentationfault: [replying to the horse: "denuvo runs selectively and not very often"]
ah, so its not like before where there were hundreds or thousands of checks ?

[2025-09-12 19:56] the horse: there's a lot of things, but they don't run that often to my knowledge; but i'll brit speak to it

[2025-09-12 19:56] Brit: there still are many thousands of checks

[2025-09-12 19:56] the horse: momo did a look into hogwarts recently

[2025-09-12 19:56] the horse: https://momo5502.com/posts/2024-03-31-bypassing-denuvo-in-hogwarts-legacy/
[Embed: Bypassing Denuvo in Hogwarts Legacy]
When I announced my Black Ops 3 integrity bypass, someone commented that my research was not impressive and I should try analyzing Denuvo instead.
That kinda stuck with me, so I did what everyone woul

[2025-09-12 19:57] the horse: if the game overall runs like shit, it's not denuvo
if you experience stutters every few seconds, it might be denuvo

[2025-09-12 19:58] Brit: look at how often the hooks are hit

[2025-09-12 19:58] segmentationfault: will give it a read

[2025-09-12 19:58] Brit: its less than once a seccond

[2025-09-12 19:58] Brit: basically never from the pov of a cpu

[2025-09-12 19:59] Brit: nah the game runs like ass because it was written by the intelectual equivalents of pirate software

[2025-09-12 20:00] segmentationfault: gotcha

[2025-09-12 20:07] Matti: UE5 games being horribly optimized is more the norm than the exception

[2025-09-12 20:08] Matti: and much of it is the fault of the engine itself

[2025-09-12 20:08] Matti: <https://www.youtube.com/watch?v=Ls4QS3F8rJU> this guy is obnoxious as fuck but it's hard to disagree with his views on UE

[2025-09-12 20:19] Brit: [replying to Matti: "<https://www.youtube.com/watch?v=Ls4QS3F8rJU> this..."]
I was on board until he started begging for funding. I do not think yet another UE fork is the solution

[2025-09-12 20:21] the horse: did he?

[2025-09-12 20:21] the horse: 
[Attachments: image.png]

[2025-09-12 20:21] the horse: on his website he says the opposite

[2025-09-12 20:22] the horse: (they want to seek 900k after prototype)

[2025-09-12 20:22] Brit: 15 -> 16:30 in that video

[2025-09-12 20:22] Matti: [replying to Brit: "I was on board until he started begging for fundin..."]
lmao I didn't even make it that far into the video before I closed the tab

[2025-09-12 20:22] Brit: <:kekw:904522300257345566>

[2025-09-12 20:23] the horse: i mean that's just typical patreon

[2025-09-12 20:23] the horse: we try to educate and if you like us you can donate, we will give back to the communtity as well

[2025-09-12 20:23] the horse: what's wrong with that

[2025-09-12 20:24] Brit: I suppose Im not used to seeing these so it's jarring as I have sponsor block set to the most aggressive settings I could

[2025-09-12 20:24] the horse: it's very common

[2025-09-12 20:24] Matti: 100% the same for me

[2025-09-12 20:24] Matti: I purposely disabled embed because of this shit

[2025-09-12 20:24] Brit: I dislike interaction and donation calls

[2025-09-12 20:24] Matti: I just can't survive the average youtube video

[2025-09-12 20:25] Matti: with the average youtube personality

[2025-09-12 20:25] Brit: Nothing wrong with having a pateron btw

[2025-09-12 20:25] Brit: and its cool that he delves (LMAO AI WORD) into rendering technology

[2025-09-12 20:25] Brit: I just cringe whenever I hear people ask for patreon subs

[2025-09-12 20:26] the horse: tip your landlord

[2025-09-12 20:27] Matti: [replying to Brit: "and its cool that he delves (LMAO AI WORD) into re..."]
yeah, I feel like I'd read articles if he wrote them

[2025-09-12 20:27] Matti: he is definitely on point on a lot of stuff

[2025-09-12 20:27] the horse: imo if you just took all the subtitles you'd have an article

[2025-09-12 20:27] Brit: [replying to the horse: "tip your landlord"]
I own the place

[2025-09-12 20:27] Matti: no

[2025-09-12 20:27] the horse: video is structured like a read article

[2025-09-12 20:28] Matti: I just can't agree that it works like that

[2025-09-12 20:28] Matti: I mean a well written article

[2025-09-12 20:28] the horse: [replying to Brit: "I own the place"]
tip your energy provider then

[2025-09-12 20:28] Brit: I do

[2025-09-12 20:28] Brit: 35p/kW/h

[2025-09-13 01:16] KR: [replying to Brit: "I own the place"]
The government owns the place, you just bought the right to live there.

You will own nothing and you will *still* not be happy.

[2025-09-13 01:19] KR: [replying to the horse: "if the game overall runs like shit, it's not denuv..."]
From experience Denuvo has a tell-tale "spike" in profiling that's pretty easy to identify.

Usually when one of its checks is hooked to something dumb like player movement *cough* AC Odyssey *cough*.

With that said there's always going to be a degree of performance overhead in any VM-style obfuscation approach.

[2025-09-13 01:21] KR: It *does* have an affect on baseline performance as well but having seen historical tests between protected and unprotected games it's nowhere near as bad as people make it out to be

[2025-09-13 12:13] Deus Vult: [replying to Brit: "and its cool that he delves (LMAO AI WORD) into re..."]
Is this going to be a thing now when we are talking and people think its just AI over certain words because this has happened to me a few times lol

[2025-09-13 12:54] Oliver: Any one have experience with amd SME? Im tryig to encrypt memory in place. Just one page but windows just black screens me

[2025-09-13 12:55] Oliver: Ive been follwing the procedure of the programming manual

[2025-09-13 15:41] ᲼᲼᲼᲼᲼᲼: Hello  I'm looking for how, or whether it's possible, to make the kernel or usermode believe that a real kernel driver is active when in reality it is not; in other words, to falsify its kernel or usermode status. I don't know the details  I'm not a coder or a reverser I'm just looking for an answer to this question. I don't speak English well, so I used ChatGPT to translate the sentence you just read.

[2025-09-13 16:30] daax: <@1235381238915797003> why delete?

[2025-09-13 16:31] GG: [replying to daax: "<@1235381238915797003> why delete?"]
idk have a feeling that probably no one will reply

[2025-09-13 16:31] daax: [replying to GG: "idk have a feeling that probably no one will reply"]
well I don't think many people got the chance to read it?

[2025-09-13 16:31] daax: you posted it an hour or so ago

[2025-09-13 16:32] daax: it's saturday, gotta give people a chance

[2025-09-13 16:33] Brit: [replying to GG: "idk have a feeling that probably no one will reply"]
I actually just put the bin into ida to check literally secconds before you delted it

[2025-09-13 16:34] GG: lmao okay I will going to put it again

[2025-09-13 16:41] GG: So I am reversing a Go sample (Sunshuttle malware used in Solarwinds attack) and trying to emulate the C2, I am currently able to exchange sessions keys and now trying to handle beaconing, how it works is we have `main_beaconing` and `main_resolve_command`, `main_beaconing` gets the command from the server, and `main_resolve_command` executes it, the first thing that happens at `main_resolve_command`, is that it calls `genSplit`, on the recieved command

looking at the Go source code it has the following definition

```go
func genSplit(s, sep string, sepSave, n int) []string
```

```x86asm
UPX0:0000000000647AFA                 mov     [rsp], rax      ; __int64
UPX0:0000000000647AFE                 mov     rax, [rsp+370h]
UPX0:0000000000647B06                 mov     [rsp+8], rax    ; __int64
UPX0:0000000000647B0B                 lea     rax, unk_6CEADF
UPX0:0000000000647B12                 mov     [rsp+10h], rax  ; __int64
UPX0:0000000000647B17                 mov     qword ptr [rsp+18h], 1 ; __int64
UPX0:0000000000647B20                 mov     qword ptr [rsp+20h], 0 ; __int64
UPX0:0000000000647B29                 mov     qword ptr [rsp+28h], 0FFFFFFFFFFFFFFFFh ; __int64
UPX0:0000000000647B32                 call    strings_genSplit ; genSplit(str, str_len, sep, n)
```

the problem is that this function seems to be called incorrectly, in this sample the arguments are passed on the stack, by looking at the stack and comparing it with the function definition it doesn't look right, 

I tried debugging the function it self, to say that if maybe its using the correct things inside, but it wasn't for example in genSplit we have this check 

```go
    if sep == "" {
        return explode(s, n)
    }
```

which should check aganist the seperator string, but it doesn't and checks aganist this `1` that you can see in the stack image I provided.

I though that maybe this internal function has changed over time, but its the same api since Go 1.0

I also tried comparing the disassembly of strings_genSplit with Go source code, and found out that it matches correctly

Sample: https://malshare.com/sample.php?action=detail&hash=8ef68f5ad0f6849d752ea3acd481df2a0d15ae9b027f49fff983256568f840b8

Sample is already patched so it connects to my C2 instead (downgraded to http and connects to local server) , isn't the right way to do things but it worked here.

My Emulator (in Go): https://pastebin.com/uLBaAq52
[Attachments: image.png]

[2025-09-13 16:49] truckdad: https://tip.golang.org/src/cmd/compile/abi-internal
this is the regabi documentation, but the principles i'm going to quote are the same for the older stack-based one. namely:
> Register-assignment of a value V of underlying type T works as follows:
> […]
> 6. If T is a string type, interface type, or slice type, recursively register-assign V’s components (2 for strings and interfaces, 3 for slices).

further up, we see:
> The string type is a sequence of a *[len]byte pointer to the string backing store, and an int giving the len of the string.

meaning: string parameters really decay into two parameters at the ABI level, and the six parameters you see passed to the function are s_ptr, s_len, sep_ptr, sep_len, sepSave, n
[Embed: - The Go Programming Language]

[2025-09-13 16:51] GG: lmao really, I was confused by why its not passing the go string object

[2025-09-13 16:52] GG: I actually saw that this second argument is the string length, but thought its not correct

[2025-09-13 16:56] DirtySecreT: [replying to ᲼᲼᲼᲼᲼᲼: "Hello  I'm looking for how, or whether it's possib..."]
ya. obcreateobject with iodriverobjecttype. obreferenceobjectbypointer or use OBJ_PERMANENT in obj attributes, then call obinsertobject. set fastiodispatch to null so kernel doesnt direct anything at it and fixup your major irp functions and should be fine. you can just set all the major irp functions to point to a stub that does iofcompleterequest with IO_NO_INCREMENT. ofc set up the device object flags as well with like DO_DEVICE_HAS_NAME and call iocreatedevice.

[2025-09-13 16:58] GG: [replying to truckdad: "https://tip.golang.org/src/cmd/compile/abi-interna..."]
okay so now I know thank you, now its clear because I was confused about the separator exactly, but looking at it now its just `*`, since the length here 1. so something like `*Y21kLmV4ZQ==` will work, because it then takes the second element in the array and base64 decodes it

[2025-09-13 16:58] GG: that gopher was about to give me nightmares

[2025-09-13 17:00] GG: and that also makes sense now 

```go
    if sep == "" {
        return explode(s, n)
    }
```

since they can just check for the sep string length here, since this is an empty string

[2025-09-13 17:06] ᲼᲼᲼᲼᲼᲼: [replying to DirtySecreT: "ya. obcreateobject with iodriverobjecttype. obrefe..."]
Ty appreciate

[2025-09-13 17:07] ᲼᲼᲼᲼᲼᲼: [replying to DirtySecreT: "ya. obcreateobject with iodriverobjecttype. obrefe..."]
there is something like this in github?

[2025-09-13 17:08] DirtySecreT: [replying to ᲼᲼᲼᲼᲼᲼: "there is something like this in github?"]
likely

[2025-09-13 17:09] DirtySecreT: its a prtty well documented method

[2025-09-13 17:12] ᲼᲼᲼᲼᲼᲼: ok thx 👍 but u have like a word

[2025-09-13 17:12] ᲼᲼᲼᲼᲼᲼: word to find thing like that

[2025-09-13 17:12] ᲼᲼᲼᲼᲼᲼: srry english bad

[2025-09-13 17:16] DirtySecreT: [replying to ᲼᲼᲼᲼᲼᲼: "srry english bad"]
uh ig just use grep.app and search up some terms i used or the function names like obinsertobject

[2025-09-13 17:18] ᲼᲼᲼᲼᲼᲼: [replying to DirtySecreT: "uh ig just use grep.app and search up some terms i..."]
alr thx

[2025-09-13 17:31] ᲼᲼᲼᲼᲼᲼: [replying to DirtySecreT: "ya. obcreateobject with iodriverobjecttype. obrefe..."]
Its how a vgk emulator work?

[2025-09-13 17:43] Brit: no