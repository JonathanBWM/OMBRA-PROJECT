# February 2024 - Week 4
# Channel: #programming
# Messages: 456

[2024-02-19 14:41] Horsie: How to catch all kernel guard page exceptions in the kernel using a driver?

[2024-02-19 14:43] Horsie: I want to set mem prot on a couple pages such that I can catch violations, get and manipulate context and then resume execution transparently.

[2024-02-19 14:44] Horsie: Does windows allow doing that before the system bugchecks

[2024-02-19 15:54] Deleted User: maybe use mdl with seh 

```c++

    __try {
   MmProtectMdlSystemAddress(mdl, PAGE_READWRITE | PAGE_GUARD);

//trigger exception

    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        if (status == STATUS_GUARD_PAGE_VIOLATION || status == STATUS_ACCESS_VIOLATION) {
         
        }
    }

```

[2024-02-19 15:54] Deleted User: dunno if you mean this tho

[2024-02-19 16:04] Horsie: [replying to Deleted User: "maybe use mdl with seh 

```c++

    __try {
   Mm..."]
Thanks but I probably didnt phrase the question correctly. I want to catch exceptions that happen globally

[2024-02-19 16:04] Horsie: is that a thing that is doable?

[2024-02-19 16:05] Deleted User: You could maybe hook into KdTrap

[2024-02-19 16:08] Deleted User: Won't work with KdTrap, kdtrap will give u a huge amount of the kernel exceptions, but for his specific use case of catching exceptions due to memory protections those don't go to KdTrap

[2024-02-19 16:09] Deleted User: Can catch those via a KiPageFault hook, PG would care though

[2024-02-19 16:10] Deleted User: interesting

[2024-02-19 16:22] Horsie: [replying to Deleted User: "Can catch those via a KiPageFault hook, PG would c..."]
I was hoping there would be a cleaner way than this but I'll just do thing before I find something cleaner

[2024-02-19 16:22] Horsie: Ty

[2024-02-19 18:39] donna🤯: https://twitter.com/yarden_shafir/status/1759625857138565341
[Embed: Yarden Shafir (@yarden_shafir) on X]
Attention EDR developers:
In 24H2 MS will allow you to receive notifications for drivers blocked by HVCI through SeRegisterImageVerificationCallback through a new CallbackType.
You'll need to register

[2024-02-19 18:51] Deleted User: Sus

[2024-02-19 18:52] Deleted User: Good for anticheats and antivirus ig

[2024-02-20 15:24] birdy: spicy

[2024-02-20 16:49] daax: [replying to Horsie: "How to catch all kernel guard page exceptions in t..."]
> VGK RE go brrrr

[2024-02-20 18:22] Deleted User: what do they do?

[2024-02-20 23:40] okokmasta: so... any of you guys have any experience at patching a PE to add a TLS callback ? , ive already extended text section, and injected the code

[2024-02-21 01:10] nezu: So I decided to re-write the riscy-business in rust (don't ask why) and I'm getting slightly different output on return values of inport stubs that needs to be truncated

[2024-02-21 01:10] nezu: 
[Attachments: image.png]

[2024-02-21 01:11] nezu: left is the original implementation, right is mine.
The question is: Are they functionally equivalent or did I fuck something up? (I'm relatively new to LLVM)

[2024-02-21 01:17] nezu: I have verified that I am indeed calling `LLVMBuildTrunc` (the C api for `CreateTrunc`) but it somehow seems to disappear and the `ptrtoint` changes to `i32`

[2024-02-21 01:22] JustMagic: probably fine

[2024-02-21 01:22] JustMagic: if it's valid IR and compiles down to the same thing, I wouldn't worry

[2024-02-21 01:57] birdy: magic man

[2024-02-21 09:23] mrexodia: [replying to nezu: ""]
Why?

[2024-02-21 09:23] mrexodia: But nice though, hope you will publish it!

[2024-02-21 16:15] nezu: [replying to mrexodia: "Why?"]
Why what? Why rust? Well because I wanted something that could handle everything at once, in memory, without a bunch of python scripts and distinct tools glued together with another script/cmake. Having it in rust makes it easy to integrate with other things like a web server and generate code on the fly 😉

[2024-02-21 16:22] nezu: [replying to mrexodia: "But nice though, hope you will publish it!"]
I'll try to publish everything that is just a re-implementation, so essentially `add_riscvm_executable` post build compressed into a single command.

As you probably know, deobfuscting something becomes 10x easier when you have the source for how it was made

[2024-02-21 18:23] mrexodia: [replying to nezu: "I'll try to publish everything that is just a re-i..."]
It actually doesn't become much easier at all ^^ But fair enough

[2024-02-21 18:24] mrexodia: I considered writing it in rust, but the LLVM bindings seemed annoying so I used C++ instead

[2024-02-21 18:24] Brit: [replying to mrexodia: "It actually doesn't become much easier at all ^^ B..."]
especially for vms, gotta make tools anyway imo

[2024-02-21 18:26] nezu: [replying to mrexodia: "I considered writing it in rust, but the LLVM bind..."]
They are annoying indeed, some are even missing. Not gonna be super clean, but hey at least I learned a bunch. The llvm part is harder, but the rest is easier

[2024-02-21 18:26] mrexodia: We understand the p2c/malware vm payload generator won’t be open source 😂

[2024-02-21 18:27] dullard: <@863750832318840833> you a back engineering enjoyer ?

[2024-02-21 18:27] dullard: I haven’t seen that emoji in a while 🥲

[2024-02-21 18:28] Brit: Just an unfamiliar game modifications guy I think anyway

[2024-02-21 18:35] Brit: yeah no

[2024-02-21 18:35] Horsie: I miss the Low Level Engineering discord

[2024-02-21 18:35] Brit: that's not <#835664858526646313> material

[2024-02-21 18:35] Horsie: There were some good discussions there

[2024-02-21 18:35] dullard: [replying to Brit: "that's not <#835664858526646313> material"]
This ⁉️

[2024-02-21 18:35] dullard: We must travel to <#835646666858168320> with our ltsc thug shaking shenanigans

[2024-02-21 19:16] Deleted User: [replying to dullard: "<@863750832318840833> you a back engineering enjoy..."]
is it still alive? The server got deleted a long time ago iirc

[2024-02-21 20:12] dullard: [replying to Deleted User: "is it still alive? The server got deleted a long t..."]
Yes, deleted when they decided to sell out

[2024-02-21 20:35] flower: [replying to mrexodia: "I considered writing it in rust, but the LLVM bind..."]
inkwell isn't that bad
sadly the llvm-c api (which its based on) is quite limited

[2024-02-21 20:48] snowua: [replying to Horsie: "I miss the Low Level Engineering discord"]
what a throwback

[2024-02-21 20:49] flower: good share ++

[2024-02-23 12:04] birdy: This club is public

[2024-02-23 12:05] birdy: ```lua
print("public club")
```

[2024-02-23 12:05] birdy: Rate the code

[2024-02-23 13:41] Timmy: thats great lua right there

[2024-02-23 14:32] Azalea: python ❤️

[2024-02-23 15:18] [Janna]: [replying to birdy: "Rate the code"]
it's too good! the rating is off the scale! `:D`

[2024-02-23 15:18] [Janna]: 1000/100 xD (integer overflow)

[2024-02-23 15:23] naci: [replying to birdy: "```lua
print("public club")
```"]
very nice javascript code

[2024-02-23 15:26] naci: I give a ((unsigned long long)-1) / 10

[2024-02-23 15:26] birdy: 😡

[2024-02-23 15:26] birdy: LUA. NOT PYTHON. NOT JAVASCRIPT.

[2024-02-23 15:26] birdy: [replying to Timmy: "thats great lua right there"]
How can I improve my script?

[2024-02-23 15:29] naci: [replying to birdy: "LUA. NOT PYTHON. NOT JAVASCRIPT."]
lies, its godot

[2024-02-23 16:46] unknowntrojan: [replying to birdy: "LUA. NOT PYTHON. NOT JAVASCRIPT."]
which flavor of lua

[2024-02-23 16:47] birdy: You 😉

[2024-02-23 18:47] BrightShard: I think you need -Ss

[2024-02-23 18:47] BrightShard: one only enables gdb and the other waits iirc? I use them both for bs: https://github.com/Bright-Shard/bs/blob/e63b0e392110f7265119241ad52a36ab8809708c/qemu/src/main.rs#L11C2-L12C27
[Embed: bs/qemu/src/main.rs at e63b0e392110f7265119241ad52a36ab8809708c · B...]
A dependency-less WIP x86-64 operating system written entirely from scratch. - Bright-Shard/bs

[2024-02-23 18:54] BrightShard: odd

[2024-02-23 18:55] BrightShard: what's the full command you're running?

[2024-02-23 19:00] BrightShard: oh this is quite long lmao

[2024-02-23 19:00] BrightShard: you are specifying -S twice

[2024-02-23 19:01] Matti: oh ok, you found it

[2024-02-23 19:01] Matti: just for reference `-gdb tcp:localhost:1234 -S` works for me

[2024-02-23 19:02] Matti: no lowercase s

[2024-02-23 19:02] Matti: unsure what that one is for

[2024-02-23 19:02] Matti: `-no-shutdown -no-reboot` are also often handy

[2024-02-23 19:03] BrightShard: https://www.qemu.org/docs/master/system/gdb.html
-s enables gdb, I guess the -gdb does the same

[2024-02-23 19:03] Matti: aah yes

[2024-02-23 19:04] BrightShard: yeah I'm not sure unless specifying -S twice breaks it for some reason

[2024-02-23 19:04] Matti: honestly it might

[2024-02-23 19:04] Matti: I"ve seen stranger things happen due to qemu args not being perfect

[2024-02-23 19:04] BrightShard: if it just flips a boolean I could see it happening lmao

[2024-02-23 19:05] Matti: other than this, I know that on windows with whpx accel, GDB does not actually work at all
but that's N/A here, it should work with KVM

[2024-02-23 19:05] Matti: on windows you need tcg for GDB

[2024-02-23 19:09] Matti: could `-no-user-config -nodefaults` be making it so that the gdb args actually need to be specified in full?

[2024-02-23 19:09] Matti: mind you I still haven't finished reading the entire command lol

[2024-02-23 19:11] Matti: > -no-user-config
>                 do not load default user-provided config files at startup
ig it depends on whether gdb uses these
but I doubt this is the issue since you say gdb works, it just doesn't pause

[2024-02-23 19:11] Matti: `-nodefaults     don't create default devices` doubt this is related too

[2024-02-23 19:11] BrightShard: try running the same command you sent with one of the -S removed; since it's just not pausing I really suspect that's it

[2024-02-23 19:11] Matti: yeah I agree

[2024-02-23 19:15] Matti: haha

[2024-02-23 19:15] Matti: I got tired of waiting to confirm this

[2024-02-23 19:15] Matti: yes, `-S -S` undoes itself

[2024-02-23 19:15] BrightShard: LMFAO

[2024-02-23 19:15] BrightShard: god i love qemu

[2024-02-23 19:30] Matti: you know, this is the kind of bug that's so stupid and so easy to fix

[2024-02-23 19:30] Matti: that it almost made me consider writing a patch for it

[2024-02-23 19:30] Matti: or well I did that part, that was the code

[2024-02-23 19:30] Matti: but then I opened https://www.qemu.org/docs/master/devel/submitting-a-patch.html

[2024-02-23 19:31] Matti: nevermind

[2024-02-23 19:31] Matti: another time maybe....

[2024-02-23 19:31] BrightShard: *oh*

[2024-02-23 20:13] Deleted User: Do you guys like oop or any other paradigm more?

[2024-02-23 20:20] birdy: I like my own style where I try to make the code unreadable as possible and then try to revise it later, but instead I end up quitting half way through the revision and make another project to do it all over again, each incrementing version number

[2024-02-23 20:21] Torph: [replying to Deleted User: "Do you guys like oop or any other paradigm more?"]
I default to procedural(?) just because that's what I'm used to. there are some scenarios where OOP style interfaces are really nice, since they enforce things more than a function pointer struct or whatever. I didn't like Rust very much, but their trait system looked like a pretty intuitive way of doing inheritance

[2024-02-23 20:22] Torph: I've been doing java lately for school, frankly I think getters & setters are pointless in a lot of cases for simple data types. but it's nice not to have header files

[2024-02-23 20:23] Deleted User: good take, I agree

[2024-02-23 20:24] contificate: can be quite good when you consider maintainability and possibility of using the getter/setter by reference method in various functional-esque APIs

[2024-02-23 20:24] contificate: but yeah it's because people think "encapsulation" means "information hiding", but encapsulation is short for "encapsulation of policy"

[2024-02-23 20:25] Torph: [replying to contificate: "but yeah it's because people think "encapsulation"..."]
I hate asking my professor why a constant value of a linked list would be private and they go "uhhhh it's for abstraction and readability" which means *literally* nothing

[2024-02-23 20:26] BrightShard: [replying to Deleted User: "Do you guys like oop or any other paradigm more?"]
I don't like OOP because inheritance can be really annoying and it can also have performance impacts. Personally I use a bit of functional... and then a bit of whatever isn't functional and isn't OOP lmao

[2024-02-23 20:26] BrightShard: imperative maybe? idk

[2024-02-23 20:26] Torph: lol I still don't know what imperative is, I just hear it thrown in with a bunch of other adjectives sometimes

[2024-02-23 20:26] BrightShard: [replying to Torph: "I hate asking my professor why a constant value of..."]
yeah encapsulation is complete bs imo

[2024-02-23 20:26] contificate: professors usually aren't good programmers

[2024-02-23 20:26] BrightShard: [replying to Torph: "lol I still don't know what imperative is, I just ..."]
ME NEITHER LOL

[2024-02-23 20:26] BrightShard: I'm just assuming it'd be the non-oop non-functional one.

[2024-02-23 20:26] BrightShard: idk

[2024-02-23 20:26] contificate: do you understand

[2024-02-23 20:26] contificate: "imperative tense"

[2024-02-23 20:27] contificate: i.e. the tense you use when doing a github commit title

[2024-02-23 20:27] contificate: "Revoke permissions for foo"

[2024-02-23 20:27] contificate: it's about a style of programming that's about doing stuff in some defined order

[2024-02-23 20:27] contificate: usually statement orientated

[2024-02-23 20:27] BrightShard: yeah so I think I use a bit of functional and a bit of imperative.

[2024-02-23 20:28] BrightShard: functional can be *really* nice sometimes

[2024-02-23 20:28] contificate: yes yes permit me to groom you into OCaml

[2024-02-23 20:28] Deleted User: Yes that is true, sometimes its good to use function programming to not overcomplicate things

[2024-02-23 20:28] BrightShard: ive never actually learned a functional lang LOL

[2024-02-23 20:29] BrightShard: but I really like the aspects of rust that (I hear) it takes from functional

[2024-02-23 20:29] BrightShard: iterators for example 🤌

[2024-02-23 20:29] BrightShard: and traits

[2024-02-23 20:29] Deleted User: true

[2024-02-23 20:29] contificate: ADTs and pattern matching are not inherently functional but so great

[2024-02-23 20:29] BrightShard: [replying to Deleted User: "Yes that is true, sometimes its good to use functi..."]
better performance too in many cases

[2024-02-23 20:29] Torph: [replying to BrightShard: "yeah encapsulation is complete bs imo"]
encapsulation is fine, I really like having a defined interface that can abstract away from something (for example a file read interface which lets you handle zip files transparently). but everyone seems to think classes are required for that, and when I ask why they just blindly go "uhhhh abstraction is good"

[2024-02-23 20:30] Torph: like. I'm sure there's *some* good reason for using OOP. I just can't figure it out

[2024-02-23 20:30] BrightShard: yeah that's nice; what's annoying is the encapsulation often forces you to use that abstracted interface. so if I need to go in and handle something with zip files specifically I can't because it's behind the file read interface and that doesn't have zip file handling.

[2024-02-23 20:30] BrightShard: I don't disagree with having a unified API

[2024-02-23 20:30] Deleted User: [replying to BrightShard: "better performance too in many cases"]
Yeah, well then you have productivity and maintainability standing next to performance, but I agree, but its not for me xD

[2024-02-23 20:31] BrightShard: but I think traits/interfaces/whatever you want to call them are a MUCH better way of going about it

[2024-02-23 20:31] BrightShard: because you can have that abstract interface but still get into the nitty gritty if you need

[2024-02-23 20:31] BrightShard: [replying to Deleted User: "Yeah, well then you have productivity and maintain..."]
wdym?

[2024-02-23 20:31] Torph: [replying to BrightShard: "yeah that's nice; what's annoying is the encapsula..."]
in the filesystem example it never came up when I used a library/interface like that, but I can see what you mean when applied to other problems

[2024-02-23 20:31] BrightShard: yeah

[2024-02-23 20:32] 25d6cfba-b039-4274-8472-2d2527cb: [replying to BrightShard: "because you can have that abstract interface but s..."]
the point often is to ensure that nobody does the "nitty gritty" when using the abstraction

[2024-02-23 20:32] BrightShard: I feel like I've run into it several times with oop style programs

[2024-02-23 20:32] 25d6cfba-b039-4274-8472-2d2527cb: to provide guarantees about sane operation

[2024-02-23 20:32] Torph: [replying to BrightShard: "but I think traits/interfaces/whatever you want to..."]
for me just the *word* "trait" instantly made inheritance and interfaces make like 10x more sense

[2024-02-23 20:32] BrightShard: really? that's interesting

[2024-02-23 20:32] 25d6cfba-b039-4274-8472-2d2527cb: no different from const variables. it's to enforce things on the programmer, not for any technical reason but for guarantees

[2024-02-23 20:33] Torph: ^

[2024-02-23 20:33] BrightShard: I get that

[2024-02-23 20:33] BrightShard: that's why we have private things at all

[2024-02-23 20:33] Deleted User: [replying to BrightShard: "wdym?"]
most of the times when writing large projects in mostly functional parts I find myself not being productive because im focusing too much on performance more than on code productivity (readability/flexibility/ reusability)

[2024-02-23 20:33] BrightShard: oh yeah. performance comes *after* writing the initial code

[2024-02-23 20:34] BrightShard: trying to focus on it early will just kill you

[2024-02-23 20:34] BrightShard: but I find functional is faster for me to use because I can manipulate types and data so quickly

[2024-02-23 20:34] Torph: [replying to BrightShard: "really? that's interesting"]
it's much more intuitive to go "this type has the trait of being streamable and iterable" rather than "this type implements the stream interface and inherits from the iterable parent class"

[2024-02-23 20:36] BrightShard: [replying to BrightShard: "that's why we have private things at all"]
the problem though is if I need a feature that's not in the abstracted API. If everything's private I just *can't* and that seems to happen quite a bit with encapsulation.
on the other hand with traits, a type can implement a trait to comply with that API, but then also offer its own stuff.

[2024-02-23 20:36] BrightShard: it's hard to describe without an example I think

[2024-02-23 20:36] Deleted User: [replying to Torph: "it's much more intuitive to go "this type has the ..."]
programming language language lol

[2024-02-23 20:36] BrightShard: it doesn't even need to be nitty gritty, really just something that's not in the abstraction I guess

[2024-02-23 20:36] Torph: [replying to Deleted User: "most of the times when writing large projects in m..."]
same, all my rendering projects immediately died because I spent like a full week trying to invent a way to handle every possible feature of 3D file formats in the perfect optimal way with no overhead

[2024-02-23 20:37] Deleted User: xD

[2024-02-23 20:37] BrightShard: [replying to Torph: "it's much more intuitive to go "this type has the ..."]
I guess, though IMO they're both just annoying because OOP is annoying. I'd prefer "type A implements trait B" y'know

[2024-02-23 20:37] BrightShard: simpler to talk about imo

[2024-02-23 20:37] Torph: I could go either way on that

[2024-02-23 20:37] BrightShard: [replying to Torph: "same, all my rendering projects immediately died b..."]
yeah and i feel like this is an example where you can't easily have one abstracted api

[2024-02-23 20:38] BrightShard: so encapsulation doesn't work as well

[2024-02-23 20:39] Torph: that was more of a "trying to abstract everything when not bothering would be way more efficient" problem

[2024-02-23 20:40] Torph: frankly I was (am still am) just scared of doing matrix multiplication

[2024-02-23 20:40] Deleted User: [replying to Torph: "that was more of a "trying to abstract everything ..."]
I have that all the time

[2024-02-23 20:41] Deleted User: I think its common for most programmers

[2024-02-23 20:41] BrightShard: [replying to Torph: "that was more of a "trying to abstract everything ..."]
yeah ik; i just mean i think it could exemplify both

[2024-02-23 20:41] BrightShard: [replying to Deleted User: "I think its common for most programmers"]
I think abstractions are just hard

[2024-02-23 20:41] BrightShard: especially if you try to support every format under the sun with it

[2024-02-23 20:44] Deleted User: Well as a programmer you shouldnt care about abstraction until you need to. But my problem is, that I almost immediately start with abstraction because of just not knowing how to actually do this or that and "code aesthetic"

[2024-02-23 20:44] Deleted User: I hate it but it’s true

[2024-02-23 20:44] Deleted User: I would write c all day if it wasn’t that

[2024-02-23 20:45] contificate: implement a compiler

[2024-02-23 20:45] BrightShard: think it also depends on the language you use

[2024-02-23 20:45] BrightShard: it'd be harder to implement an abstraction in C than any other modern languages I think

[2024-02-23 20:46] BrightShard: cause in other langs you have generics and better type systems and a lot of other things that let you move fast

[2024-02-23 20:46] BrightShard: whereas in C you have less features to work with to make that abstraction

[2024-02-23 20:47] Deleted User: [replying to contificate: "implement a compiler"]
does that help?

[2024-02-23 20:47] contificate: yeah

[2024-02-23 20:47] contificate: I'd say so

[2024-02-23 20:47] Deleted User: How

[2024-02-23 20:47] BrightShard: yeah how does that help lmao

[2024-02-23 20:48] BrightShard: just for experience?

[2024-02-23 20:48] contificate: it will make you change what you look for in a programming language

[2024-02-23 20:48] contificate: truth be told, it doesn't really matter what language you use

[2024-02-23 20:48] contificate: if your main ambition is to just

[2024-02-23 20:48] contificate: glue shit together

[2024-02-23 20:48] contificate: and call APIs from libraries you never implemented

[2024-02-23 20:48] contificate: iterate a few vectors

[2024-02-23 20:48] contificate: and store some shit in hash maps

[2024-02-23 20:48] contificate: that is not what programming is really about

[2024-02-23 20:48] Deleted User: damn, those are real words

[2024-02-23 20:49] contificate: compilers are full of little sub-problems that can have interesting solutions

[2024-02-23 20:49] contificate: also makes you appreciate what goes into lowering various languages you may already use

[2024-02-23 20:49] contificate: and, in the process, you may pick up relevant lingo

[2024-02-23 20:50] contificate: you mentioned traits, which are like typeclasses from Haskell, but actually fall under "ad-hoc polymorphism"

[2024-02-23 20:51] Torph: [replying to BrightShard: "it'd be harder to implement an abstraction in C th..."]
oh yeah that rendering project was in C which made the matrix multiplication way more annoying

[2024-02-23 20:52] Deleted User: <@687117677512360003> can I ask you, what language you like writing in the most?

[2024-02-23 20:52] Torph: [replying to contificate: "that is not what programming is really about"]
that's funny because that's been my entire year of CS classes at college so far

[2024-02-23 20:52] contificate: [replying to Deleted User: "<@687117677512360003> can I ask you, what language..."]
OCaml

[2024-02-23 20:52] contificate: brb going store

[2024-02-23 20:52] Torph: I've heard of OCaml from Primeagen but never looked into it. he never really went into detail about what exactly it is

[2024-02-23 20:53] Deleted User: [replying to Torph: "I've heard of OCaml from Primeagen but never looke..."]
xD

[2024-02-23 20:53] BrightShard: idk about that
I think it *does* matter which language you use; different languages will have different restrictions and challenges you'll have to work around. Like what, you gonna write an operating system in JS? Languages can also give you more features to work with, so I think it's quite important to learn about a lot and pick one that works best for you. It's what you spend all of your time in, after all.
as for "what programming is really about", it depends on what you're working on. the project will define what the programming is about.

[2024-02-23 20:53] BrightShard: [replying to Torph: "I've heard of OCaml from Primeagen but never looke..."]
SAME LOL

[2024-02-23 20:53] BrightShard: and then i hear about it in passing sometimes

[2024-02-23 20:53] BrightShard: it's a funcitonal lang, that's all i know

[2024-02-23 20:54] Deleted User: [replying to contificate: "OCaml"]
what reasons are there for that being your fav language?

[2024-02-23 20:54] contificate: I'll reply soon

[2024-02-23 20:54] Deleted User: kk

[2024-02-23 20:59] BrightShard: <a:typing:987092017689526415> **Torph** is typing...

[2024-02-23 20:59] Torph: [replying to BrightShard: "idk about that
I think it *does* matter which lang..."]
that's true. like I love Zig but it enforces certain restrictions on memory that makes some sketchy pointer casts illegal. if I was writing something where I create all the data structures, it'd be totally fine. but if I need to read binary data from disk that was originally a C structure, it can be a little annoying

[2024-02-23 21:00] BrightShard: I've not used zig, but I love it's comptime stuff

[2024-02-23 21:01] BrightShard: in rust I'm usually able to read binary without much issue so I'm surprised it's an issue in zig <:rooThink:596576798351949847>

[2024-02-23 21:01] Torph: haha I haven't tried out the comptime yet
I really liked the native optional types and the super simple control flow

[2024-02-23 21:01] Torph: [replying to BrightShard: "in rust I'm usually able to read binary without mu..."]
that's funny, I originally tried to do it in Rust and gave up after 5 days of being completely unable to read binary from disk without an unsafe block

[2024-02-23 21:01] luci4: I recently read a book on Go, it was really nice

[2024-02-23 21:01] BrightShard: [replying to Torph: "that's funny, I originally tried to do it in Rust ..."]
damn

[2024-02-23 21:01] Torph: I've heard good things about go

[2024-02-23 21:02] BrightShard: I don't like the idea of a built in gc

[2024-02-23 21:02] BrightShard: but otherwise it does look neat

[2024-02-23 21:02] luci4: [replying to Torph: "I've heard good things about go"]
It's very interesting

[2024-02-23 21:02] Torph: [replying to BrightShard: "I don't like the idea of a built in gc"]
isn't it optional

[2024-02-23 21:02] BrightShard: no

[2024-02-23 21:02] Torph: oh was that ocaml

[2024-02-23 21:02] BrightShard: unless it's changed since discord made that blog post

[2024-02-23 21:02] luci4: I personally decided to learn a bit about it for my web server needs

[2024-02-23 21:02] BrightShard: https://discord.com/blog/why-discord-is-switching-from-go-to-rust
[Embed: Why Discord is switching from Go to Rust]
This post explains why Rust made sense for Discord to reimplement: how it was done, and the resulting performance improvements.

[2024-02-23 21:02] Deleted User: Im not too much a fan of golangs syntax

[2024-02-23 21:03] luci4: [replying to Deleted User: "Im not too much a fan of golangs syntax"]
I don't think it's that bad, honestly

[2024-02-23 21:03] Torph: I really like that it has nice high level features but ends up in native code

[2024-02-23 21:03] BrightShard: [replying to Torph: "that's funny, I originally tried to do it in Rust ..."]
my job has a binary format, and it works just fine in rust.
you do have to adjust the way you program from other languages for it to work well in Rust.

[2024-02-23 21:03] Deleted User: [replying to luci4: "I don't think it's that bad, honestly"]
Its not bad, I'm just not used to it. I'd learn it, but for my purpose right now, I don't need it

[2024-02-23 21:03] BrightShard: I think that's something people coming from C(++) struggle with a lot

[2024-02-23 21:03] BrightShard: [replying to Torph: "I really like that it has nice high level features..."]
THIS

[2024-02-23 21:03] BrightShard: I love that concept

[2024-02-23 21:03] BrightShard: cost free abstractions

[2024-02-23 21:04] Deleted User: True

[2024-02-23 21:04] Torph: [replying to BrightShard: "my job has a binary format, and it works just fine..."]
is the solution to just use a library to read into a struct? I saw a lot of people suggest this, but coming from C I use libraries as a last resort. 
I feel like there ought to be a way to do this fairly simple task with the base language & stdlib features

[2024-02-23 21:05] BrightShard: libraries can make it easier; i personally dislike using dependencies

[2024-02-23 21:05] Torph: same

[2024-02-23 21:05] BrightShard: we don't use dependencies, we iterate over the raw bytes and just construct the data from there

[2024-02-23 21:06] Torph: that seems incredibly annoying

[2024-02-23 21:06] BrightShard: not really

[2024-02-23 21:06] BrightShard: I mean it's how binary formats work

[2024-02-23 21:06] Torph: compared to just `fread` with the file, struct ptr, & size?

[2024-02-23 21:06] Torph: no iteration required, one fn call to stdlib

[2024-02-23 21:06] luci4: [replying to Deleted User: "Its not bad, I'm just not used to it. I'd learn it..."]
Fair enough. Goroutines are also pretty interesting

[2024-02-23 21:06] BrightShard: [replying to Torph: "compared to just `fread` with the file, struct ptr..."]
oh so you just cast it to the struct?

[2024-02-23 21:07] BrightShard: this doesn't verify the data though

[2024-02-23 21:07] BrightShard: if you really want to, you can do that with mem::transmute

[2024-02-23 21:07] BrightShard: or if you have a pointer ptr.cast()

[2024-02-23 21:07] BrightShard: though pointers are generally discouraged ofc

[2024-02-23 21:07] Torph: [replying to BrightShard: "oh so you just cast it to the struct?"]
I just pass in `&struct` to its `void*` arg if thats what you mean

[2024-02-23 21:08] BrightShard: yeah but then you don't get any benefits of types...

[2024-02-23 21:08] Torph: [replying to BrightShard: "this doesn't verify the data though"]
I'm not sure there's really any good way to verify the data except making sure enough bytes were read and the stuff you read in looks roughly correct

[2024-02-23 21:08] BrightShard: [replying to BrightShard: "we don't use dependencies, we iterate over the raw..."]
we verify it here

[2024-02-23 21:09] Torph: [replying to BrightShard: "yeah but then you don't get any benefits of types...."]
wdym? it just copies the bytes from the file into the struct, and I access them with all the type info from the struct

[2024-02-23 21:09] luci4: Has anyone here read this, by chance?

https://www.amazon.com/Windows-Kernel-Programming-Pavel-Yosifovich/dp/1977593372
[Embed: Windows Kernel Programming]
There is nothing like the power of the kernel in Windows - but how do you write kernel drivers to take advantage of that power? This book will show you how.The book describes software kernel drivers p

[2024-02-23 21:09] Torph: I have not

[2024-02-23 21:09] BrightShard: nope

[2024-02-23 21:09] luci4: I'm gonna try it

[2024-02-23 21:09] BrightShard: ive not done much windows internals

[2024-02-23 21:09] BrightShard: i started but then jumped into osdev lol

[2024-02-23 21:09] luci4: [replying to BrightShard: "ive not done much windows internals"]
I want to learn more on rootkits, and all of that

[2024-02-23 21:09] Deleted User: I do recommend Windows Internals Books, there's so much information in them

[2024-02-23 21:10] Torph: [replying to BrightShard: "we verify it here"]
so are you just reading in one `u32` at a time or whatever then copying it to the struct field? that's my interpretation

[2024-02-23 21:10] BrightShard: [replying to Torph: "wdym? it just copies the bytes from the file into ..."]
in Rust, when you take an argument as a struct, you know the following:
1) the data is valid
2) if it's a reference, it's not null

you don't get that with void pointers. You also lose the opportunity for things like `Option` and other results in Rust, and none of the cool things that come with it's ownership model

[2024-02-23 21:11] BrightShard: the type system can assure a lot of things ahead of time

[2024-02-23 21:11] BrightShard: [replying to Torph: "so are you just reading in one `u32` at a time or ..."]
u8, u32 is 4 bytes
it depends on the binary format. but yeah you could create the values for the struct and then create the struct with those values.

[2024-02-23 21:12] BrightShard: some people will also do 0-copy, where the struct borrows the data from the file

[2024-02-23 21:12] Torph: [replying to BrightShard: "in Rust, when you take an argument as a struct, yo..."]
oh I was talking about C fread & `void*`. I'm passing in `&struct` to `fread` which doesn't care about the input type and only cares about the size

[2024-02-23 21:12] BrightShard: so it stores references instead of the raw data

[2024-02-23 21:12] Torph: [replying to BrightShard: "some people will also do 0-copy, where the struct ..."]
is that via OS APIs for memory-mapped files?

[2024-02-23 21:12] BrightShard: just the file that's loaded in memory

[2024-02-23 21:12] BrightShard: via fs::read usually

[2024-02-23 21:12] BrightShard: or fs::read_to_string

[2024-02-23 21:13] BrightShard: so it'll take like an `&[u8]` and just use that.

[2024-02-23 21:13] Torph: oh yeah I did that in C for a while. was kinda a waste of time and made my code worse, but that was mostly my fault because of how I implemented it

[2024-02-23 21:13] BrightShard: I think there's a library that does this automatically

[2024-02-23 21:13] BrightShard: a variant of serde

[2024-02-23 21:13] BrightShard: can't remember the name now

[2024-02-23 21:16] Torph: [replying to BrightShard: "u8, u32 is 4 bytes
it depends on the binary format..."]
I don't see why I would want to manually copy the data in 1 field at a time instead of just having something like `fread` in stdlib that just copies the data into the struct
but from what I remember this isn't done because there's no consistent memory layout for structs across architectures

[2024-02-23 21:16] BrightShard: no, you definitely can also do that

[2024-02-23 21:16] Torph: I was not able to find a way in several days of googling

[2024-02-23 21:16] BrightShard: if you #[repr(c)] it'll use the c abi so the struct has a constant field layout

[2024-02-23 21:16] BrightShard: then you'd just transmute the bytes into the struct

[2024-02-23 21:16] BrightShard: but this really is not ideal

[2024-02-23 21:17] BrightShard: becuase you haven't verified the data at all

[2024-02-23 21:17] Torph: so? as long as it's the right size it'll fit, annd then I can check each field of the struct afterwards to see if the data looks ok. and since it's in the struct I can just have a function that verifies it taking a struct as input

[2024-02-23 21:17] BrightShard: rust doesn't have a stable abi at the moment, so you're right that there's no memory layout that's guaranteed to be consistent

[2024-02-23 21:17] BrightShard: hence the repr(c)

[2024-02-23 21:19] BrightShard: [replying to Torph: "so? as long as it's the right size it'll fit, annd..."]
there's a few things I think, one would be a code debate about the type system (a variable should never be uninitialized), but the main thing is variable-sized types

[2024-02-23 21:20] BrightShard: wait

[2024-02-23 21:20] BrightShard: I think I'm debating a different thing here

[2024-02-23 21:20] BrightShard: you have the literal, in-memory representation of a struct?

[2024-02-23 21:21] BrightShard: I was really talking about binary formats, not what's essentially a memory dump.

[2024-02-23 21:21] contificate: [replying to Deleted User: "what reasons are there for that being your fav lan..."]
1) It's really convenient for middle-end compiler transformations because it supports ADTs, pattern matching, etc.
2) It's statically typed and garbage collected, which removes much of the burden of implementation from quickly prototyping>
3) Its primary paradigm is functional which is convenient for implementing many kinds of programs.
4) It's used in PL academia quite a bit and exposes you to many interesting ideas and related literature (example: there's extensive literature about continuation passing style, delimited control, defunctionalisation, etc. which are all very interesting).

[2024-02-23 21:21] BrightShard: pl academia?

[2024-02-23 21:21] BrightShard: gc 💀

[2024-02-23 21:21] contificate: programming language

[2024-02-23 21:21] contificate: garbage collection is a really good thing

[2024-02-23 21:21] BrightShard: nahhhh

[2024-02-23 21:22] contificate: no, it is, objectively

[2024-02-23 21:22] BrightShard: [replying to BrightShard: "https://discord.com/blog/why-discord-is-switching-..."]
they had to rewrite their code because of the gc's performance

[2024-02-23 21:22] BrightShard: well ok I guess it depends on the type of gc; if it's reference counted it's not as bad.

[2024-02-23 21:22] BrightShard: but it's unnecessary overhead

[2024-02-23 21:22] contificate: GC is an acceptable cost to pay for most programs

[2024-02-23 21:23] BrightShard: it's not a cost I want to pay

[2024-02-23 21:23] contificate: the issue with not using GC is that it makes programs harder to write and sometimes even slower

[2024-02-23 21:23] BrightShard: how the hell is it going to be slower with no gc

[2024-02-23 21:23] BrightShard: less memory usage i could see

[2024-02-23 21:23] Torph: [replying to BrightShard: "there's a few things I think, one would be a code ..."]
well it should be initialized to zero and then immediately overwritten by the contents of the file I'm reading

[2024-02-23 21:23] BrightShard: more performant makes no sense

[2024-02-23 21:23] contificate: because generational collectors are built for many allocations

[2024-02-23 21:24] contificate: and collect less frequently than handwritten or generated calls to `free`

[2024-02-23 21:24] BrightShard: oh

[2024-02-23 21:24] contificate: so `malloc` and `free` usage in many algorithms slows the fuck out of the thing

[2024-02-23 21:24] BrightShard: yeah mass allocations/deallocations

[2024-02-23 21:24] contificate: which is why game engines, compilers, etc. all write custom allocators that capture what minor heaps in generational collectors already do

[2024-02-23 21:24] BrightShard: that makes sense; but won't necessarily pay for the overhead of the gc, it depends on the use case.

[2024-02-23 21:24] Torph: [replying to BrightShard: "how the hell is it going to be slower with no gc"]
the GC can give similar benefits to a well-placed arena allocator

[2024-02-23 21:24] Torph: [replying to contificate: "which is why game engines, compilers, etc. all wri..."]
yeah this

[2024-02-23 21:25] contificate: I consider it to be a total waste of time and mental bandwidth to manage memory manually for most software projects

[2024-02-23 21:25] luci4: TIL that you can tweak Go's garbage collection, which means you can also disable it

[2024-02-23 21:25] contificate: it's not so bad if you're using Rust, C++, etc.

[2024-02-23 21:25] BrightShard: also why are you calling it generational collectors?

[2024-02-23 21:25] BrightShard: isn't the word garbage collectors? is this different?

[2024-02-23 21:25] Torph: [replying to BrightShard: "I was really talking about binary formats, not wha..."]
what's the difference? a struct filled with data in memory is the same as the binary data format on disk

[2024-02-23 21:25] contificate: because I use languages with generational garbage collectors

[2024-02-23 21:25] contificate: generational implies it's relocating

[2024-02-23 21:26] contificate: you have a "minor" heap that permits very fast allocation, by bumping a pointer

[2024-02-23 21:26] contificate: then a "major" heap where objects alive during a sweep of the minor heap go to live later

[2024-02-23 21:26] Torph: [replying to contificate: "generational implies it's relocating"]
oh is that a system where it gives you double pointers so it can re-arrange the memory as needed transparently

[2024-02-23 21:26] Deleted User: In c# I really like the unsafe feature, although I don't need to use it often

[2024-02-23 21:27] Deleted User: lets you use pointers and all that good stuff

[2024-02-23 21:27] 25d6cfba-b039-4274-8472-2d2527cb: [replying to contificate: "it's not so bad if you're using Rust, C++, etc."]
with default allocators still gotta be careful with which thread makes big allocations etc. because of thread based arenas etc.

[2024-02-23 21:27] Torph: [replying to Deleted User: "lets you use pointers and all that good stuff"]
oh really? i didn't know that existed

[2024-02-23 21:27] contificate: yeah, but even just mental bandwidth wise

[2024-02-23 21:27] contificate: ownership remains a huge concept in Rust and C++

[2024-02-23 21:27] contificate: when like

[2024-02-23 21:27] contificate: who gives a shit man

[2024-02-23 21:27] contificate: I'm trying to transform a program using another program such that certain properties hold

[2024-02-23 21:27] contificate: not waste my time

[2024-02-23 21:28] contificate: there's a tendency for people in these kinds of communities to think there's something noble in wasting one's time

[2024-02-23 21:28] contificate: C++ is horrible for many kinds of programs

[2024-02-23 21:28] BrightShard: [replying to Torph: "what's the difference? a struct filled with data i..."]
no - a binary format will support multiple types and variable-length types usually

[2024-02-23 21:28] BrightShard: not just 1 type

[2024-02-23 21:28] contificate: [replying to Torph: "what's the difference? a struct filled with data i..."]
depends if packed or not and endian differences 😳

[2024-02-23 21:29] contificate: major error if you just cast a file's contents to a struct pointer

[2024-02-23 21:29] luci4: [replying to contificate: "C++ is horrible for many kinds of programs"]
Why the hate for C++?

[2024-02-23 21:29] Torph: [replying to BrightShard: "no - a binary format will support multiple types a..."]
all binary formats I've worked with are representable as normal C structs

[2024-02-23 21:29] Torph: [replying to contificate: "depends if packed or not and endian differences 😳"]
well yeah obv

[2024-02-23 21:29] contificate: usually they use the cute "zero length array member" shit to have variable length structs anyway

[2024-02-23 21:29] BrightShard: [replying to contificate: "who gives a shit man"]
depends on the person, I think; ownership makes enough sense to me that it never gets in the way. I don't have to fight with the borrow checker or anything, stuff just appears and disappears without me even thinking about it

[2024-02-23 21:29] contificate: kinda need to follow that to avoid having to write a full parser for random shit you don't care about

[2024-02-23 21:29] Torph: [replying to contificate: "usually they use the cute "zero length array membe..."]
i forgot those existed 😭

[2024-02-23 21:29] contificate: which is key for "extensible" file formats

[2024-02-23 21:30] contificate: like Mach-O

[2024-02-23 21:30] contificate: which is trivial to locate sections etc.

[2024-02-23 21:30] contificate: primarily because you don't need to give a shit about the rest of the file

[2024-02-23 21:30] BrightShard: [replying to Torph: "all binary formats I've worked with are representa..."]
not the one for my job. not for apple's bplist format either. just depends

[2024-02-23 21:30] Deleted User: [replying to luci4: "Why the hate for C++?"]
no hate just truth actually, theres no need to make life hard, but if you want you can do it the hard way

[2024-02-23 21:30] contificate: I spent ages dealing with C++

[2024-02-23 21:30] BrightShard: I will also share some hate for c++ lmao

[2024-02-23 21:30] BrightShard: absolutely insane language

[2024-02-23 21:30] BrightShard: I'd write C before C++

[2024-02-23 21:30] luci4: [replying to Deleted User: "no hate just truth actually, theres no need to mak..."]
How does C++ make life hard?

[2024-02-23 21:30] contificate: I consider it to be an esolang really

[2024-02-23 21:30] Torph: I think we're all hating on C++ in completely different directions

[2024-02-23 21:31] Deleted User: [replying to luci4: "How does C++ make life hard?"]
Imagine writing a simple calculator, you could choose python or c++. One of them is harder

[2024-02-23 21:31] contificate: I primarily hate how tedious C++ makes representing any kind of inductive data type

[2024-02-23 21:32] Torph: [replying to Deleted User: "Imagine writing a simple calculator, you could cho..."]
I mean for a calculator memory management probably wouldn't even come up so it'd be fine

[2024-02-23 21:32] luci4: [replying to Deleted User: "Imagine writing a simple calculator, you could cho..."]
I mean...they have pretty different use cases, right?

[2024-02-23 21:32] Deleted User: Yeah, thats the reason you shouldn't use c++ for everything that can be made in easier ways

[2024-02-23 21:32] contificate: I advocate the opinion that C++ should have less perceived use cases

[2024-02-23 21:32] contificate: being general purpose doesn't mean it should be applied generally

[2024-02-23 21:32] Torph: that's fair

[2024-02-23 21:33] contificate: it's like writing technical debt to just misuse it for random shite

[2024-02-23 21:34] luci4: [replying to Deleted User: "Yeah, thats the reason you shouldn't use c++ for e..."]
I never said I do? Not sure what you are referring to

[2024-02-23 21:34] BrightShard: [replying to luci4: "How does C++ make life hard?"]
in my opinion - 
the language is very convoluted. I'm sure you can learn C++ well enough that it feels native-ish, but the syntax is all over the place. Ever new feature has new syntax with it, and most people I talk to don't even know the difference between some syntaxes (like initializing stuff in classes - there's () and {} and something else ider). Plus you still have manual memory management in some cases and the awful build tools

[2024-02-23 21:34] Torph: ok but every language has kinda shit build tools in one way or another

[2024-02-23 21:35] BrightShard: gtg, be back later

[2024-02-23 21:35] BrightShard: [replying to Torph: "ok but every language has kinda shit build tools i..."]
rust just builds idk man 💀

[2024-02-23 21:35] Torph: [replying to BrightShard: "rust just builds idk man 💀"]
yeah in like 4 hours after using 4GB disk space

[2024-02-23 21:37] Deleted User: [replying to luci4: "I never said I do? Not sure what you are referring..."]
I was answering to you saying they have pretty different use cases. C++ can make life harder if it is applied to some cases where it shouldn't

[2024-02-23 21:38] Deleted User: It is good if you want to write codespaces from scratch for example

[2024-02-23 21:39] luci4: [replying to Deleted User: "I was answering to you saying they have pretty dif..."]
Well...yeah. Who would argue that C++ is better than Python for a simple calculator?

[2024-02-23 21:40] Deleted User: Some people might misunderstand the use case of c++ and make their life harder that way

[2024-02-23 21:40] emi: [replying to BrightShard: "in my opinion - 
the language is very convoluted. ..."]
so real

[2024-02-23 21:40] emi: c++ is unbeatable for comp programming tho

[2024-02-23 21:40] Deleted User: true

[2024-02-23 21:41] Deleted User: I primarily use it when solving leetcode problems

[2024-02-23 21:43] contificate: I just use python for leetcode

[2024-02-23 21:44] Deleted User: I always try to get lowest runtime possible hence why I use c++

[2024-02-23 21:44] contificate: but you are compared against people using the same language

[2024-02-23 21:45] contificate: python was just designed for leetcode shit honestly

[2024-02-23 21:45] Deleted User: [replying to contificate: "but you are compared against people using the same..."]
Ah didnt kno that

[2024-02-23 21:45] contificate: did all these leetcodes

[2024-02-23 21:45] contificate: just to never be asked one in an interview, ever

[2024-02-23 21:46] Deleted User: lmao

[2024-02-23 21:46] Deleted User: like the one guy that solved 1500 problems

[2024-02-23 21:46] Deleted User: I like leetcode because it helps me use the language where I usually don't

[2024-02-23 21:46] Torph: I need to try advent of code

[2024-02-23 21:47] contificate: I hate advent of code

[2024-02-23 21:47] contificate: it is amusing seeing people getting filtered by parsing of two kinds though

[2024-02-23 21:47] contificate: 1) Parsing the input format, lol.
2) Parsing the problem statement mentally.

[2024-02-23 21:47] contificate: just great

[2024-02-23 21:49] BrightShard: [replying to Torph: "yeah in like 4 hours after using 4GB disk space"]
Ah yeah if you get in dependency hell it can suck. Most gui apps have fucking 1k+ deps 💀

[2024-02-23 21:49] BrightShard: [replying to emi: "c++ is unbeatable for comp programming tho"]
Comp?

[2024-02-23 21:49] Brit: [replying to contificate: "I hate advent of code"]
good fun if you use toylangs

[2024-02-23 21:49] Brit: t. done it on a machine of my own design

[2024-02-23 21:50] contificate: hate stupid problems, hate esolangs

[2024-02-23 21:50] emi: [replying to BrightShard: "Comp?"]
competitive

[2024-02-23 21:50] Brit: [replying to contificate: "hate stupid problems, hate esolangs"]
mental onanism is always fun

[2024-02-23 21:50] contificate: just like

[2024-02-23 21:50] contificate: can't justify that

[2024-02-23 21:51] contificate: when I have a huge backlog

[2024-02-23 21:51] contificate: of things I actually find interesting

[2024-02-23 21:54] Torph: [replying to BrightShard: "Ah yeah if you get in dependency hell it can suck...."]
even just installing rust toolchain is several GB
1k dependencies is completely deranged... the most dependencies I've ever had was like 5, and a full build from scratch took maybe 30 seconds

[2024-02-23 22:03] BrightShard: It shouldn't be multiple gb <:rooThink:596576798351949847>

[2024-02-23 22:03] BrightShard: I have 3 or 4 and it's only a couple gb with all the extensions I have for osdev too.

[2024-02-23 22:03] BrightShard: I don't think that's right

[2024-02-23 22:04] BrightShard: Also I mean the whole dep tree

[2024-02-23 22:04] BrightShard: Like when those deps pull in deps

[2024-02-23 22:04] BrightShard: Then it adds up to 1k 💀

[2024-02-23 22:05] BrightShard: I started working with some other people to make a new library that only has like 5

[2024-02-24 02:02] birdy: [replying to BrightShard: "It shouldn't be multiple gb <:rooThink:59657679835..."]
goat the gb

[2024-02-24 02:45] birdy: Try changing the batteries

[2024-02-24 02:45] birdy: Or switch their places

[2024-02-24 02:46] birdy: Sometimes it works for me

[2024-02-25 06:18] irfan_eternal: is there any good resources which i can use to learn more about this Delphi VMT Structs and what they mean
[Attachments: image.png]

[2024-02-25 20:09] Summit: '

[2024-02-25 23:04] Deleted User: Is it worth learning languages like delphi  or pascal nowadays?

[2024-02-25 23:13] Bloombit: I’ve heard Brazilian malware is commonly written in delphi

[2024-02-25 23:13] Bloombit: I probably wouldn’t these myself

[2024-02-25 23:13] Brit: if it's delphi the odds it's malware is 80%

[2024-02-25 23:16] Torph: isn't HxD written in Delphi

[2024-02-25 23:25] contificate: Pascal is full of mental people

[2024-02-25 23:25] contificate: in the sense that like

[2024-02-25 23:26] contificate: the forums are filled with complete chads

[2024-02-25 23:26] contificate: happily writing thousands upon thousands of lines of Pascal

[2024-02-25 23:26] contificate: for huge software projects

[2024-02-25 23:26] contificate: and a small tightknit community

[2024-02-25 23:27] contificate: I wrote a bit of Pascal once upon a time, found it kind of painful but it has some nice features

[2024-02-25 23:30] Brit: plain pascal is actually brain damaged

[2024-02-25 23:31] Brit: begin end. looking ass

[2024-02-25 23:39] contificate: it does feel verbose when you write it