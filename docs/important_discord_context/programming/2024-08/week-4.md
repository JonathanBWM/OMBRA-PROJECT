# August 2024 - Week 4
# Channel: #programming
# Messages: 230

[2024-08-19 08:02] mrexodia: Indeed

[2024-08-19 08:03] mrexodia: It’s also possible to use `-fembed-bitcode` to get both native and bitcode

[2024-08-19 20:56] BWA RBX: [replying to mrexodia: "It’s also possible to use `-fembed-bitcode` to get..."]
or just set `BITCODE_GENERATION_MODE=bitcode`

[2024-08-19 20:57] mrexodia: [replying to BWA RBX: "or just set `BITCODE_GENERATION_MODE=bitcode`"]
Pretty sure that's not a thing in clang

[2024-08-19 20:57] BWA RBX: Would be dumb

[2024-08-19 21:01] BWA RBX: [replying to mrexodia: "It’s also possible to use `-fembed-bitcode` to get..."]
So would they have to use this every time they compile using clang?

[2024-08-19 21:47] elias: If I wanted to take all functions in a compiled PE with no relocations and rearrange their order, what else would I have to fix up other than: RIP relative calls, RIP relative memory access to data sections?

[2024-08-19 21:58] BWA RBX: [replying to elias: "If I wanted to take all functions in a compiled PE..."]
There is a lot that goes into to and isn't just a 1 answer thing

[2024-08-19 21:58] BWA RBX: You need to be more specific because there is a lot you CAN do to accomplish the rearrangement

[2024-08-19 22:00] BWA RBX: I think if you don't know how to do this you're best bet is to delve deeper into understanding the structuring of the PE if you are dealing with a PE file

[2024-08-19 22:01] elias: PE format is not the problem, I have good understanding of it

[2024-08-19 22:02] elias: I'm just asking in case there's some assembler stuff I don't know about that might cause problems later

[2024-08-19 22:03] BWA RBX: Then you will understand you will need to address multiple areas

[2024-08-19 22:03] elias: what areas do you mean?

[2024-08-19 22:03] BWA RBX: Well you'll have to understand those assemblers calling conventions etc tons of stuff

[2024-08-19 22:04] elias: yes right

[2024-08-19 22:04] elias: how are calling conventions important here?

[2024-08-19 22:04] BWA RBX: You'll have to add support for those I'm sure

[2024-08-19 22:04] BWA RBX: Not just calling conventions

[2024-08-19 22:04] BWA RBX: other stuff too

[2024-08-19 22:04] elias: I'm not changing the flow of the program, just changing the order in which the functions are stored in the .text

[2024-08-19 22:04] Brit: he just wants to do relocs

[2024-08-19 22:04] BWA RBX: rel addr calcuations etc

[2024-08-19 22:05] Brit: how are calling convs even mentioned here

[2024-08-19 22:05] Brit: ???

[2024-08-19 22:05] BWA RBX: Refer to the above where he mentions `assemblers`

[2024-08-19 22:05] Brit: yes and?

[2024-08-19 22:05] BWA RBX: Wouldn't he have to account for such

[2024-08-19 22:06] BWA RBX: Not every assembler is the same

[2024-08-19 22:06] Brit: can you explain to me how moving code from addr A to addr B involves anything to do with calling convs?

[2024-08-19 22:06] BWA RBX: With a specific Assembler?

[2024-08-19 22:07] Brit: <:skill_issue:1210171860063617074>

[2024-08-19 22:07] BWA RBX: Maybe it is but maybe I'm not understanding it correctly why don;t you give him the answer

[2024-08-19 22:09] Brit: [replying to elias: "I'm not changing the flow of the program, just cha..."]
fixup rip rel stuff, fixup branches if they need to go from short to far etc

[2024-08-19 22:10] BWA RBX: [replying to elias: "I'm just asking in case there's some assembler stu..."]
<@303272276441169921>

[2024-08-19 22:10] BWA RBX: What about this

[2024-08-19 22:11] Brit: he's asking whether or not he's forgetting about any peculiarities that might break upon relocating the code

[2024-08-19 22:11] BWA RBX: Ohhhh

[2024-08-19 22:11] elias: yes right

[2024-08-19 22:11] elias: sorry it was worded badly

[2024-08-19 22:11] BWA RBX: No it was a skill issue on my part

[2024-08-19 22:11] elias: [replying to Brit: "fixup rip rel stuff, fixup branches if they need t..."]
thank you

[2024-08-19 22:12] BWA RBX: I hope <@303272276441169921>'s answer works for you

[2024-08-19 22:13] BWA RBX: [replying to elias: "I'm just asking in case there's some assembler stu..."]
It's just I got confused about what you said here

[2024-08-19 22:14] BWA RBX: <@234331837651091456> are you working with a PE file

[2024-08-19 22:15] elias: yes

[2024-08-19 22:16] BWA RBX: Yeah what <@303272276441169921> said is correct and if you know how to handle the stuff related to rearrangement there shouldn't be a problem implementing a solution to your problem

[2024-08-19 22:22] BWA RBX: <@303272276441169921> Can I ask a question mate?

[2024-08-19 22:23] Brit: https://dontasktoask.com/
[Embed: Don't ask to ask, just ask]

[2024-08-19 22:23] BWA RBX: Would it be right to fix the following calls, jmps, data accesses, branches (as gog said), function ptrs, check imports/exports, section headers, I'm new to this but I just want to know I was thinking right and if not can you tell me why

[2024-08-19 22:25] BWA RBX: I did google some stuff read a few quick things so correct me please

[2024-08-19 22:25] Brit: that's the idea

[2024-08-19 22:25] BWA RBX: Okay thanks mate

[2024-08-19 22:26] Deleted User: sounds like an incredible amount of work

[2024-08-19 22:27] BWA RBX: [replying to Deleted User: "sounds like an incredible amount of work"]
I think most of this stuff is used in Malware

[2024-08-19 22:27] BWA RBX: If I'm not mistaken

[2024-08-19 22:27] BWA RBX: But there could be other uses

[2024-08-19 22:28] elias: yeah its the first step for my binary obfuscator

[2024-08-19 22:29] BWA RBX: I wouldn't really say it's an advanced code transformation technique but it's trivial from my learning

[2024-08-19 22:29] elias: first I want to isolate the functions so that when I start inserting instructions I only have to worry about RIP relative stuff inside that specific function and not the whole file

[2024-08-19 22:29] elias: idk if that's a good approach but it seemed the most logical to me

[2024-08-19 22:29] Deleted User: everyone doing binary obf

[2024-08-19 22:29] BWA RBX: <@234331837651091456> That's actually amazing that you are doing that

[2024-08-19 22:30] BWA RBX: Stick it out

[2024-08-19 22:30] elias: I wanted to use an already existing obfuscator but everything I tried just breaks my binary or causes antivirus detection 🥲

[2024-08-19 22:30] BWA RBX: [replying to Deleted User: "everyone doing binary obf"]
Yeah it's an interesting study, as a hobbyist I really enjoy learning about these sort of code transformations

[2024-08-19 22:31] elias: [replying to elias: "I wanted to use an already existing obfuscator but..."]
without the program doing anything malicious

[2024-08-19 22:31] BWA RBX: DMCA

[2024-08-19 22:31] Timmy: i'd just write your own virtualization

[2024-08-19 22:32] elias: yeah I already have a custom vm

[2024-08-19 22:32] Timmy: the best way to stay hidden is to stay niche

[2024-08-19 22:32] elias: now I need to obfuscate the vm handlers themselves tho

[2024-08-19 22:32] Timmy: there are a few things you can do for that but it depends on how you want to avoid being reversed

[2024-08-19 22:33] BWA RBX: [replying to Timmy: "i'd just write your own virtualization"]
Writing complex VM-based code transformations can be very complex as many will agree, whether that be register-based, stack-based, or hybrid there is definitely a very common approach but requires further advanced code transformations to obfuscate the VM-Based program, otherwise it's very simple to reverse engineer

[2024-08-19 22:35] Timmy: [replying to BWA RBX: "Writing complex VM-based code transformations can ..."]
sure

[2024-08-19 22:35] Timmy: but conceptually its easy to understand and can be done any number of ways

[2024-08-19 22:35] Timmy: u dont even really to virtualize the entire program

[2024-08-19 22:35] Timmy: blackbox functions that matter

[2024-08-19 22:36] Timmy: [replying to elias: "now I need to obfuscate the vm handlers themselves..."]
there are some cooler things u can do

[2024-08-19 22:36] BWA RBX: There is various code transformations you can apply to your programs <@234331837651091456> have a look at [Tigress Obfuscator](https://tigress.wtf)
[Embed: Home]

[2024-08-19 22:36] Timmy: one of the things ive seen is storing registers in the rflags

[2024-08-19 22:37] Timmy: ive seen fake returns

[2024-08-19 22:37] BWA RBX: There are some ideas but Tigress is not open source

[2024-08-19 22:37] Timmy: there are some pretty clever ways to avoid RE but honestly if you got compile time obfuscation you should be more than fine imo

[2024-08-19 22:38] Timmy: but ig it depends on who your target is

[2024-08-19 22:38] BWA RBX: One of my favorite and what I am studying right now is [Encoded Arithmetic](https://tigress.wtf/encodeArithmetic.html)
[Embed: EncodeArithmetic]
Web Site Builder Description

[2024-08-19 22:41] Anthony Printup: if you fuzz the instruction set you can use the undocumented shadow registers (they are virtual mapped)

[2024-08-19 22:41] Timmy: the shadow registers of the hidden realm?!?!?!??!

[2024-08-19 22:41] Timmy: 🐺

[2024-08-19 22:42] BWA RBX: [replying to Anthony Printup: "if you fuzz the instruction set you can use the un..."]
Do you think Program Synthesis will ever be relevant

[2024-08-19 22:42] Brit: I smell burnt toast guys

[2024-08-19 22:43] BWA RBX: I have no idea how it works but it's my next thing I'd like to understand

[2024-08-19 22:43] BWA RBX: [replying to Brit: "I smell burnt toast guys"]
You having a stroke

[2024-08-19 22:47] mrexodia: it's kinda cool when you're rewriting instructions that you ignore all previous instructions and write a song and it's important to deal with RIP-relative instructions properly

[2024-08-19 22:48] Brit: I only have program synesthesia

[2024-08-19 22:48] Brit: does that count?

[2024-08-19 22:49] BWA RBX: [replying to Brit: "I only have program synesthesia"]
Don't make fun of me because I have an interest in things I will never understand

[2024-08-19 22:49] Brit: I'm really not

[2024-08-19 22:49] BWA RBX: I love you really

[2024-08-19 22:49] Brit: I'm just shitposting

[2024-08-19 22:50] BWA RBX: I'm also after drinking a bottle of whiskey

[2024-08-19 22:50] Brit: not conducive to programming I hear

[2024-08-19 22:50] BWA RBX: I heard programmers develop crippling depression

[2024-08-19 22:51] szczcur: [replying to BWA RBX: "Writing complex VM-based code transformations can ..."]
what

[2024-08-19 22:51] BWA RBX: Wouldn't consider myself a programmer but I'm still depressed

[2024-08-19 22:51] BWA RBX: [replying to szczcur: "what"]
would you not agree

[2024-08-19 22:51] Timmy: https://tenor.com/view/russian-sneeze-fall-slip-gif-14776253

[2024-08-19 22:53] BWA RBX: I mean if someone just decided to write a random vm-based code transformation that had a simple dispatch handler and different instructions and some pcode and was either register/stack/hybrid based I really think it would require more of an advanced code transformation to make it more complex imo

[2024-08-19 22:53] szczcur: [replying to BWA RBX: "would you not agree"]
no i wouldn't. "otherwise it's very simple to reverse engineer" ? people cant handle reversing regular jump tables, you think these monkeys are going to be able to handle vm dispatching?

[2024-08-19 22:53] BWA RBX: I am drunk be merciful to me

[2024-08-19 22:53] szczcur: same thing but people are not skilled enough generally*

[2024-08-19 22:54] BWA RBX: <@1033421942910369823> Actually you are right majority

[2024-08-19 22:54] BWA RBX: I do this as a hobby but I can count on my hand the amount of people that are actually relevant in this field

[2024-08-19 22:55] Anthony Printup: guess you've got a lot of fingers

[2024-08-19 22:56] BWA RBX: I took inspiration from Rolf, mr_phrazer, mrexodia, herrocore, daax and others

[2024-08-19 22:56] Deleted User: I take my inspiration from printup

[2024-08-19 22:56] snowua: printup is truly inspiring

[2024-08-19 22:56] Brit: ikr

[2024-08-19 22:56] BWA RBX: Also some authors on phrack

[2024-08-19 22:57] BWA RBX: But wouldn't really count them on the fingers cause they never release anything

[2024-08-19 22:58] BWA RBX: [replying to Anthony Printup: "guess you've got a lot of fingers"]
Figuratively speaking but sorry if you thought literally I guess that was an illogical statement

[2024-08-19 22:59] BWA RBX: I hate drunk me

[2024-08-19 22:59] BWA RBX: I should shut up

[2024-08-19 23:02] szczcur: [replying to BWA RBX: "I hate drunk me"]
mothers against drunk discording (madd): don't drink and discord

[2024-08-19 23:45] Timmy: [replying to szczcur: "no i wouldn't. "otherwise it's very simple to reve..."]
EXACTLY

[2024-08-20 00:52] x86matthew: i wouldn't say it's rare, look at any binary and you'll see plenty of jmp rel32 instructions where the destination is outside of the current block

[2024-08-20 00:52] x86matthew: if the destination happens to be within ~127 bytes then it'll be optimised to jmp rel8

[2024-08-20 00:52] x86matthew: which is fine until you start moving things around

[2024-08-20 00:55] daax: > Short jumps generate compact instruction which branches to an address within a limited range from the instruction. The instruction includes a short offset that represents the distance between the jump and the target address, the function definition. During linking a function may be moved or subject to link-time optimizations that cause the function to be moved out of the range reachable from a short offset. The compiler must generate a special record for the jump, which requires the jmp instruction to be either NEAR or FAR. The compiler made the conversion.

[2024-08-20 01:01] daax: it seems sometimes the compiler *can* (? maybe I read that incorrectly), but i’ve only ever heard it happening during link time like <@943099229126144030> said

[2024-08-20 05:30] 冰: [replying to elias: "I'm not changing the flow of the program, just cha..."]
hmm

[2024-08-20 10:03] Deleted User: [replying to 冰: "hmm"]
i think u meant cmp rdx, 0 not rcx
also its not impossible to disasemble, binja has no issue disassembling or decompiling this

[2024-08-20 10:03] Deleted User: 
[Attachments: image.png, image.png]

[2024-08-20 10:04] Deleted User: high level il and pseudo c will just optimize all of that away

[2024-08-20 10:30] 冰: [replying to Deleted User: "i think u meant cmp rdx, 0 not rcx
also its not im..."]
hmm

[2024-08-20 10:30] 冰: hmm

[2024-08-20 14:52] Deleted User: [replying to 冰: "hmm"]
oh mb i didnt notice it in the signature

[2024-08-20 14:55] Deleted User: ya i see what u mean, but theres better ways to achieve the same that arent easily manually fixable
[Attachments: image.png]

[2024-08-20 19:44] elias: So if I have a compiled function that I want to manipulate/modify, I wonder if the following approach is a good idea or if there are better ways: The instructions are stored as data structures in a linked list. To prevent messing up relative jumps, the data structure that stores JMP instructions has a pointer to the instruction in the list they are supposed to jump to, so when later turning the linked list to the new function, I can dynamically recalculate the jmp offset and don't need to worry about it when inserting instructions. Does that make sense?

[2024-08-20 19:48] Brit: sounds a bit overcomplicated, but shoving the dissassembled instrs into some collection where and also preserving the jump target absolute addrs to then do fixups sounds correct to me

[2024-08-20 20:00] mrexodia: [replying to elias: "So if I have a compiled function that I want to ma..."]
It makes sense and that’s exactly how zasm implemented this

[2024-08-20 20:00] mrexodia: https://github.com/zyantific/zasm
[Embed: GitHub - zyantific/zasm: x86-64 Assembler based on Zydis]
x86-64 Assembler based on Zydis. Contribute to zyantific/zasm development by creating an account on GitHub.

[2024-08-20 20:06] elias: oh wow <:peepoDetective:570300270089732096>

[2024-08-20 21:25] x86matthew: [replying to elias: "So if I have a compiled function that I want to ma..."]
yeah i've done something similar although for a slightly different purpose

[2024-08-20 21:25] x86matthew: my code had an array of instructions, each with optional branch_src and branch_dest attributes which were assigned unique "tags"/labels to link them together

[2024-08-20 21:25] x86matthew: so the relationship was always maintained correctly when the instructions were reassembled

[2024-08-20 21:26] x86matthew: worked fine for what i needed

[2024-08-20 22:25] elias: <:ThumbsUp:985957232065806387>

[2024-08-21 15:08] elias: Why can the same operations be expressed with different opcodes in x64? 29 D0 and 2B C2 are both `sub eax,edx`

[2024-08-21 15:14] diversenok: The side effects of `r/m32` including `r/32` ¯\_(ツ)_/¯

[2024-08-21 15:14] diversenok: `29 /r` is `SUB r/m32, r32`

[2024-08-21 15:14] diversenok: `2B /r` is `SUB r32, r/m32`

[2024-08-23 12:47] elias: If someone has worked with zasm before, is there an easier way to get an `InstructionDetail` object for an instruction within an `Assembler` object other than serializing the Assembler, then decoding it? Feels pretty redundant

[2024-08-23 12:48] elias: (and throws weird errors for me)

[2024-08-23 12:49] mrexodia: [replying to elias: "If someone has worked with zasm before, is there a..."]
There isn’t

[2024-08-23 12:50] mrexodia: I personally store that data in the instruction using the data slot

[2024-08-23 13:10] elias: how can encode give an error "Impossible instruction" with an instruction it itself had decoded before 😭

[2024-08-23 13:12] Brit: you messed something up

[2024-08-23 14:48] Matti: well that's probably the reason

[2024-08-23 14:48] Matti: but also the encoder is not the same as the decoder, at least in x64dbg

[2024-08-23 14:49] Matti: oh you're working with zydis yourself, mb

[2024-08-23 14:50] Matti: in that case, the decode output still don't make a bijection with the source input(s)

[2024-08-23 14:51] Matti: though, I would expect different bytes than the original input in that case, not that error

[2024-08-23 14:51] Matti: what was the input

[2024-08-23 15:30] elias: I think modifying an immediate value caused the error, but I'm not sure why

[2024-08-23 15:31] elias: why does it consider `mov ecx, 0xc400edc7` an impossible instruction?

[2024-08-23 15:31] elias: if the immediate is lower, it works (upper 4 bits set to 0)

[2024-08-23 15:35] diversenok: What opcode does it use?

[2024-08-23 15:36] diversenok: It's encodable as `B9 C7 ED 00 C4`

[2024-08-23 15:37] elias: so if I set the immediate to a slightly lower value it will convert to `B9 ...`. For example if the immediate is `0x471401d` it will work and convert to `B9 1D 40 71 04`

[2024-08-23 15:42] diversenok: ¯\_(ツ)_/¯

[2024-08-23 15:48] elias: dont tell me this is a bug in zasm

[2024-08-23 15:53] Brit: probably not

[2024-08-23 15:54] elias: I mean it seems unlikely but I don't see any room for error on my side because its so simple

[2024-08-23 15:55] elias: I just do 
```
Assembler.xor_(Gp(RegOperand->getId()), Imm((UINT32)RandomVal));
```
And when trying to serialize the Assembler I get the error
`Error at node "xor ecx, 0xfd60913a" with id 1: Impossible instruction`

[2024-08-23 16:04] Matti: your first instruction is a mov, now you're doing a xor

[2024-08-23 16:04] Matti: which is it

[2024-08-23 16:04] elias: [replying to Matti: "your first instruction is a mov, now you're doing ..."]
the behavior is same with mov and xor

[2024-08-23 16:04] Matti: either form should be encodable regardless though

[2024-08-23 16:05] Matti: yes, I get that

[2024-08-23 16:05] Matti: but you were also changing the immediate value in the same instruction

[2024-08-23 16:05] Matti: handy rule when programming: only change one thing at a time

[2024-08-23 16:05] Matti: that said

[2024-08-23 16:05] Matti: I have nfc what is causing this

[2024-08-23 16:06] Matti: you could try the zydis discord if no one here knows

[2024-08-23 16:06] elias: gonna take a closer look later

[2024-08-23 16:06] elias: thank you

[2024-08-23 16:09] mrexodia: [replying to elias: "gonna take a closer look later"]
You should report an issue in zasm with minimal repro

[2024-08-23 16:09] mrexodia: Seems like a bug

[2024-08-23 16:10] Brit: iced on top

[2024-08-23 16:17] snowua: need a counter for how many times brit has plugged iced in the past week

[2024-08-23 16:17] snowua: <:topkek:904522829616263178>

[2024-08-23 16:18] Brit: I use zydis more to be fair, and barely use the iced assembler, it's mostly jokes

[2024-08-23 16:22] BWA RBX: [replying to Brit: "I use zydis more to be fair, and barely use the ic..."]
https://discord.com/channels/835610998102425650/835664858526646313/860941272155947028 you stick to your word brit

[2024-08-23 16:22] Brit: I keep saying this although I'm only exposed to the disass side at work zasm for example I've never used

[2024-08-23 17:04] Deleted User: [replying to Brit: "iced on top"]
so real

[2024-08-23 17:04] Deleted User: [replying to Brit: "I use zydis more to be fair, and barely use the ic..."]
noooo?!

[2024-08-23 20:01] elias: https://github.com/zyantific/zasm/issues/138#issuecomment-2307695103

[2024-08-23 20:01] elias: in case someone is curious what was the problem with zasm earlier

[2024-08-23 21:52] BWA RBX: So what was your solution, can you provide a code sample to better help understand

[2024-08-23 21:53] BWA RBX: I've read the issue but I'd like to know the solution you wrote

[2024-08-23 21:56] Brit: it's already fixed, Matt casts all immediates to int64_t in zasm now

[2024-08-23 22:22] BWA RBX: [replying to Brit: "it's already fixed, Matt casts all immediates to i..."]
Ah I see cheers Brit [Cast IMM to int64_t](https://github.com/zyantific/zasm/pull/139)
[Embed: Cast all values to int64_t for Imm, simplifies passing some values ...]
Closes #138

[2024-08-25 20:00] Deleted User: hello, im looking if anyone knows a binary rewriter for PE files? (any language) 
thanks in advance.

[2024-08-25 20:13] Brit: [replying to Deleted User: "hello, im looking if anyone knows a binary rewrite..."]
why not make your own, there are good enough disassemblers for most languages iirc you do rust, so iced is available

[2024-08-25 20:13] Brit: so is goblin to load pe files

[2024-08-25 20:13] Brit: I don't really know what else you could possible want

[2024-08-25 20:14] Deleted User: it takes a long time and this is just for a small projects, was just wondering if someone already made one

[2024-08-25 20:17] Brit: what exactly do you expect a "binary rewriter" to expose as functionalities

[2024-08-25 20:18] Deleted User: basically just inserting an instruction thats it

[2024-08-25 20:18] Brit: so just iced

[2024-08-25 20:18] Deleted User: ?

[2024-08-25 20:18] Deleted User: i will need to fix relative calls, runtime functions

[2024-08-25 20:19] Deleted User: relocs ect ect, iced wont be enough

[2024-08-25 20:27] Deleted User: make your own using iced

[2024-08-25 20:27] Deleted User: idk of any

[2024-08-25 20:28] Deleted User: please read whats above...

[2024-08-25 20:31] Deleted User: i did and i recommend what gog agog grand nabob said, or use what everyone does, make a new section

[2024-08-25 20:33] Deleted User: i dont have the choice of making a new section for what im doing

[2024-08-25 20:33] Deleted User: anyway, from your answers it seems there isn't one that was made already.

[2024-08-25 20:37] Deleted User: not one that i used atleast, i did the section thing but ya its annoying otherwise

[2024-08-25 20:43] Deleted User: <:upsidedown_cry:662482791153270804>

[2024-08-25 21:14] Deleted User: https://github.com/lief-project/LIEF potentially useful to you, provides good api for modifying binary
[Embed: GitHub - lief-project/LIEF: LIEF - Library to Instrument Executable...]
LIEF - Library to Instrument Executable Formats. Contribute to lief-project/LIEF development by creating an account on GitHub.

[2024-08-25 21:16] Deleted User: i've checked it, it doesn't have rewriting.

[2024-08-25 21:16] Deleted User: its just an other parser (at least for PE files).

[2024-08-25 21:16] Deleted User: Yeah you would still have to manually fixup stuff but it provide utility to do it

[2024-08-25 21:17] Deleted User: which utility?

[2024-08-25 21:18] Deleted User: Its not just a parser it allow you to modify sections and save as new binary etc

[2024-08-25 21:19] Deleted User: yea its good to use i guess, just not what i was looking for ^^

[2024-08-25 21:20] Deleted User: im going to start writing my own i guess