# May 2024 - Week 4
# Channel: #reverse-engineering
# Messages: 67

[2024-05-21 10:16] uname: Anyone know a good x64 asm debugger for quickly running some snippets to see how registers are manipulated ?

[2024-05-21 10:18] Oliver: [replying to uname: "Anyone know a good x64 asm debugger for quickly ru..."]
just attach the program to x64dbg?

[2024-05-21 10:19] zeropio: [replying to uname: "Anyone know a good x64 asm debugger for quickly ru..."]
try this https://github.com/netspooky/scare

[2024-05-21 10:19] zeropio: or https://github.com/zodiacon/QuickAsm

[2024-05-21 10:21] uname: [replying to zeropio: "or https://github.com/zodiacon/QuickAsm"]
perfect

[2024-05-21 10:21] uname: [replying to Oliver: "just attach the program to x64dbg?"]
I'm just running bits of shellcode

[2024-05-21 10:22] uname: Not going to compile every time then attach with x64dbg

[2024-05-21 10:22] Oliver: [replying to uname: "I'm just running bits of shellcode"]
okok, glad you found a solution at least 🙂

[2024-05-21 10:22] uname: Sorry didn't mean to sound rude

[2024-05-21 10:23] zeropio: [replying to uname: "Not going to compile every time then attach with x..."]
you can open any process, patch some bytes and use that section as your playground

[2024-05-21 10:23] Oliver: [replying to uname: "Sorry didn't mean to sound rude"]
nono, i didnt take it as that. you are good

[2024-05-21 10:27] Brit: [replying to uname: "I'm just running bits of shellcode"]
you do know you can asmjit the sequence you care about straight in x64dbg right?

[2024-05-21 10:28] uname: mrexodia has too much free time smh

[2024-05-21 10:29] Brit: just load any bin, and press space at rip to start assembling

[2024-05-21 10:30] uname: ok cheers

[2024-05-21 12:40] projizdivlak: is there an easy way to deal with std::string's in ida

[2024-05-21 13:23] mrexodia: [replying to uname: "Not going to compile every time then attach with x..."]
You can automate it pretty easily with JITCall: `"JITLoadDll.exe" C:\Users\steve\Desktop\shellcode.bin -scb 0 "void(int i)" 0x10 -bp`

[2024-05-21 13:23] mrexodia: https://github.com/stevemk14ebr/RETools/tree/master/JITCall
[Embed: RETools/JITCall at master · stevemk14ebr/RETools]
My reversing tools. Some custom, some not. Contribute to stevemk14ebr/RETools development by creating an account on GitHub.

[2024-05-21 18:20] Ensar Gök: Do you guys pay for IDA pro or using some cracked version (or did you cracked it with  idafree)? I want to start writing idapython and that means ida free is done for me

[2024-05-21 18:26] contificate: everybody here using IDA Pro uses a licensed version of the product

[2024-05-21 18:26] contificate: 💯

[2024-05-21 18:26] contificate: 💎

[2024-05-21 18:27] Ensar Gök: it is too expensive for me

[2024-05-21 18:27] Ensar Gök: 😦

[2024-05-21 18:39] contificate: looks like it's back to the GNU objdump mines with you

[2024-05-21 18:40] Ensar Gök: .d

[2024-05-21 18:42] Ensar Gök: who is he

[2024-05-21 18:44] Ensar Gök: what did he do

[2024-05-21 19:04] jvoisin: [replying to Ensar Gök: "it is too expensive for me"]
Use ghidra ?

[2024-05-21 19:05] Ensar Gök: [replying to jvoisin: "Use ghidra ?"]
well, i am comparing and thinking about it. Do you think is it as good as ida? or is it just enugh for the purpose

[2024-05-21 19:06] jvoisin: [replying to Ensar Gök: "well, i am comparing and thinking about it. Do you..."]
Try it and form your own opinion about it. But if its good enough for the NSA, odds are it's good enough for everyone.

[2024-05-21 19:07] 5pider: [replying to contificate: "looks like it's back to the GNU objdump mines with..."]
LMFAO bro pulled the "back into the shadow realm with you" card

[2024-05-21 19:07] luci4: [replying to jvoisin: "Try it and form your own opinion about it. But if ..."]
I kinda doubt they are using Ghidra for their own work

[2024-05-21 19:07] Ensar Gök: [replying to jvoisin: "Try it and form your own opinion about it. But if ..."]
yeah, you're right. I was also watching a video about ghidra python scripting rn. probably will go with ghidra, Ida is like 3k dollars

[2024-05-21 19:08] jvoisin: [replying to luci4: "I kinda doubt they are using Ghidra for their own ..."]
Why do you think they wrote it and are maintaining it for around 30y?

[2024-05-21 19:09] diversenok: They are payed for that, at the very least

[2024-05-21 19:09] 5pider: [replying to Ensar Gök: "well, i am comparing and thinking about it. Do you..."]
honestly i would even suggest binary ninja. used it a couple of times and it was very pleasing. the ui is clean, the disassembler looks good and the decompiler is not bad.

[2024-05-21 19:10] luci4: [replying to 5pider: "honestly i would even suggest binary ninja. used i..."]
good price too

[2024-05-21 19:12] Ensar Gök: [replying to 5pider: "honestly i would even suggest binary ninja. used i..."]
I'll check it. thx for the advice

[2024-05-21 19:13] Ensar Gök: [replying to luci4: "good price too"]
honestly, I think I am not going to pay for any of these, at least for now. I am a student and our economy is f'ed up rn

[2024-05-21 19:13] luci4: [replying to Ensar Gök: "honestly, I think I am not going to pay for any of..."]
Fair enough

[2024-05-21 19:13] zeropio: binja has students discount for ~70$ btw

[2024-05-21 19:15] Ensar Gök: [replying to luci4: "Fair enough"]
I really dont want to be the greedy one. but the current situation is really bad for us 😦 I really want to give people they desire some time

[2024-05-21 19:17] Ensar Gök: [replying to zeropio: "binja has students discount for ~70$ btw"]
yeah, its ofc better than 300. I'll think about it.

[2024-05-21 19:19] zeropio: but don't buy anything if you can't afford it

[2024-05-21 19:20] Ensar Gök: [replying to zeropio: "but don't buy anything if you can't afford it"]
yes, i wont. my idea is like to buy the things in the future that i cant afford now

[2024-05-21 19:26] Saturnalia: [replying to Ensar Gök: "I really dont want to be the greedy one. but the c..."]
binary ninja has a pretty full featured free version, I'd play with it and ghidra and see what you like more

[2024-05-21 19:28] Ensar Gök: [replying to Saturnalia: "binary ninja has a pretty full featured free versi..."]
ninja's looking really good. I want to script in both, will do that

[2024-05-21 19:36] Brit: [replying to jvoisin: "Why do you think they wrote it and are maintaining..."]
this, ghidra is good enough for most everything you'd need an interactive disassembler for, beyond that it's all about making tools anyway

[2024-05-21 19:37] Saturnalia: [replying to Ensar Gök: "ninja's looking really good. I want to script in b..."]
the free version of binja doesn't do scripting, but if you get the student version binja scripting >>>> ghidra scripting

[2024-05-21 19:40] Ensar Gök: [replying to Saturnalia: "the free version of binja doesn't do scripting, bu..."]
this made me feel sad. thx for the advice

[2024-05-21 19:45] Saturnalia: ghidra is still very good! just not as 'modern'

[2024-05-21 19:48] jvoisin: [replying to Saturnalia: "ghidra is still very good! just not as 'modern'"]
Being written in java doesn't really help to convey a "modern" vibe. But I've found ghidra more pleasant for reversing big stuff, while ninja's pretty cool for CTF and the likes

[2024-05-21 19:49] Ensar Gök: [replying to jvoisin: "Being written in java doesn't really help to conve..."]
being written in java made me feel wtf tbh.

[2024-05-21 19:50] Ensar Gök: isn't it a lil. weird ? or is it just for to suppert multiple platforms

[2024-05-21 19:51] Saturnalia: [replying to jvoisin: "Being written in java doesn't really help to conve..."]
for me it's less the ui than stuff like bnil vs pcode, and proper python bindings instead of jython making it nicer to use

[2024-05-21 19:57] jvoisin: [replying to Ensar Gök: "being written in java made me feel wtf tbh."]
Back in the days™ it was the right call

[2024-05-21 19:57] jvoisin: [replying to Saturnalia: "for me it's less the ui than stuff like bnil vs pc..."]
Fair

[2024-05-21 20:00] Ensar Gök: [replying to jvoisin: "Back in the days™ it was the right call"]
do you think java is great ?

[2024-05-21 20:00] jvoisin: [replying to Ensar Gök: "do you think java is great ?"]
In the 90s, it was

[2024-05-21 20:00] jvoisin: They tried C++, it didn't work

[2024-05-21 20:01] Torph: you can definitely support multiple platforms w/ C++, but I wouldn't be surprised if that was way harder in the 90s bc of less standardized & lower quality compilers

[2024-05-21 20:02] Ensar Gök: I understand now, ty both

[2024-05-21 20:04] jvoisin: [replying to Torph: "you can definitely support multiple platforms w/ C..."]
Check the Ghidra release talk, iirc it was blackhat. Or maybe defcon ?

[2024-05-21 20:04] jvoisin: "But they could have used LISP" :<

[2024-05-21 20:04] Torph: [replying to jvoisin: "Check the Ghidra release talk, iirc it was blackha..."]
lol didnt know there was a release talk

[2024-05-21 20:06] jvoisin: [replying to Torph: "lol didnt know there was a release talk"]
https://m.youtube.com/watch?v=kx2xp7IQNSc&pp=ygUPZ2hpZHJhIGJsYWNraGF0
[Embed: Ghidra - Journey from Classified NSA Tool to Open Source]
This year was a momentous one for the National Security Agency (NSA) as we released our game-changing software reverse engineering (SRE) framework to the open source community: Ghidra. This was a long