# November 2024 - Week 3
# Channel: #programming
# Messages: 535

[2024-11-11 06:03] 0x208D9: <@162611465130475520> is there any way for me to specify custom linker in cmkr? if i try it via cmake-before option using the set(CMAKE_C_LINK_EXECUTABLE, "/usr/bin/lld-link") , the linker says no input file found (sorry for the formatting im on phone)

[2024-11-11 21:28] mrexodia: [replying to 0x208D9: "<@162611465130475520> is there any way for me to s..."]
There is nothing specific to cmkr there. You customize the linker the same way as with regular CMake (eg not, or `-fuse-ld=lld` flags in your toolchain)

[2024-11-11 21:28] 0x208D9: [replying to mrexodia: "There is nothing specific to cmkr there. You custo..."]
i did exactly that, works with cmake but not cmkr

[2024-11-11 21:28] 0x208D9: :/

[2024-11-11 21:28] 0x208D9: i can send my project with cmkr and cmake

[2024-11-11 21:29] mrexodia: Sure

[2024-11-11 21:29] mrexodia: But cmkr isn't different from CMake 🤷‍♂️

[2024-11-11 21:29] mrexodia: It just generates a regular `CMakeLists.txt`

[2024-11-11 21:29] mrexodia: but feel free to drop it

[2024-11-11 21:39] 0x208D9: [replying to mrexodia: "but feel free to drop it"]
the cmkr version
[Attachments: helloworld.zip]

[2024-11-11 21:40] 0x208D9: the cmake version : https://github.com/w1redch4d/efichess
[Embed: GitHub - w1redch4d/efichess: Chess inside UEFI]
Chess inside UEFI . Contribute to w1redch4d/efichess development by creating an account on GitHub.

[2024-11-11 21:42] mrexodia: [replying to 0x208D9: "the cmake version : https://github.com/w1redch4d/e..."]
```cmake
set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_C_LINKER "/usr/bin/lld-link")
```

Not 100% sure, but calling this after `project()` is just a noop?

[2024-11-11 21:42] mrexodia: so that's why it 'works'?

[2024-11-11 21:43] 0x208D9: [replying to mrexodia: "```cmake
set(CMAKE_C_COMPILER "/usr/bin/clang")
se..."]
oh yeah lmao, i forgot to comment the LINKER out, but without that it compiles as well

[2024-11-11 21:45] 0x208D9: [replying to mrexodia: "```cmake
set(CMAKE_C_COMPILER "/usr/bin/clang")
se..."]
still works :/
[Attachments: image.png]

[2024-11-11 21:45] 0x208D9: and its running clang as well, i checked

[2024-11-11 21:46] mrexodia: maybe clang is the default compiler?

[2024-11-11 21:46] mrexodia: anyway, I don't think `CMAKE_C_LINKER` is actually a variable

[2024-11-11 21:47] 0x208D9: [replying to mrexodia: "anyway, I don't think `CMAKE_C_LINKER` is actually..."]
it is tho

[2024-11-11 21:48] mrexodia: [replying to 0x208D9: "it is tho"]
https://cmake.org/cmake/help/latest/manual/cmake-variables.7.html

[2024-11-11 21:48] mrexodia: Look for `CMAKE_<LANG>_LINKER` you'll see

[2024-11-11 21:48] mrexodia: The compiler-specific detector determines the linker, very often the linker is actually `clang` itself

[2024-11-11 21:49] 0x208D9: [replying to mrexodia: "The compiler-specific detector determines the link..."]
right but since im specifying the executable as x86_64-unknown-windows its looking for link.exe in linux instead of lld

[2024-11-11 21:49] 0x208D9: where it fails with the error in cmake and cmkr without the -fuse-ld

[2024-11-11 21:50] mrexodia: Yeahh this would require you to look at the compiler detection logic t osee if you could override it

[2024-11-11 21:51] 0x208D9: i see, interesting

[2024-11-11 21:53] 0x208D9: 
[Attachments: image.png]

[2024-11-11 21:56] mrexodia: [replying to 0x208D9: ""]
Yeah generally-speaking you're doing this wrong unfortunately 🥲

[2024-11-11 21:56] mrexodia: You should create a toolchain file and pass that in while configuring

[2024-11-11 21:56] 0x208D9: [replying to mrexodia: "You should create a toolchain file and pass that i..."]
any references on that?

[2024-11-11 21:56] mrexodia: It's scarce 🥲

[2024-11-11 21:57] 0x208D9: ig will stick with cmake then 😭

[2024-11-11 21:57] mrexodia: There is no difference though, you can do `[variables] CMAKE_C_LINKER = xxx`, but it's just not the right approach to cmake

[2024-11-11 21:58] mrexodia: let me see if I can get my zig toolchain to work

[2024-11-11 21:59] 0x208D9: [replying to mrexodia: "There is no difference though, you can do `[variab..."]
^^ specifying the C_LINKER is technically not needed if i can use the -fuse-ld while linking the target directly

[2024-11-11 21:59] mrexodia: Yeah I just mean you can generate 100% the same cmake with cmkr, just that it's not the 'correct' approach of doing things (a common problem with cmake)

[2024-11-11 21:59] 0x208D9: ^^ +1

[2024-11-11 22:00] 0x208D9: im thinking of switching to bazel or meson after i start working on it seriously

[2024-11-11 22:35] mrexodia: [replying to 0x208D9: "im thinking of switching to bazel or meson after i..."]
No worries, I think I got it working ^^

[2024-11-11 22:35] 0x208D9: damnn, so what was the issue?

[2024-11-11 22:36] mrexodia: nothing?

[2024-11-11 22:36] mrexodia: you just have to use a toolchain file

[2024-11-11 22:36] 0x208D9: nice!

[2024-11-11 22:36] 0x208D9: wait lemme try and let ya know

[2024-11-11 22:36] mrexodia: yeah hold on, I need to push it

[2024-11-11 22:36] 0x208D9: also thanks for spending some time on the help 😄

[2024-11-11 22:36] 0x208D9: really appreciate

[2024-11-12 14:03] Xits: [replying to 0x208D9: "any references on that?"]
https://github.com/llvm/llvm-project/blob/main/llvm/cmake/platforms/WinMsvc.cmake
[Embed: llvm-project/llvm/cmake/platforms/WinMsvc.cmake at main · llvm/llvm...]
The LLVM Project is a collection of modular and reusable compiler and toolchain technologies. - llvm/llvm-project

[2024-11-13 13:19] Horsie: I'm trying to write a LLVM pass where im passing the arguments of one function into another function (which takes arguments of the same type).
I've tried initializing a vector of existing args from the `CB->getCalledFunction()->args()` to pass into the new function but I'm getting an error- `Referring to an argument in another function!`

[2024-11-13 13:20] Horsie: I assume I need to 'copy' the arguments somehow but I cant seem to be able to figure out how

[2024-11-13 18:23] contificate: Which part are you talking about here

[2024-11-13 18:24] contificate: the part where you have `f(a1, a2, ..., aN) { ... }` and with to create `g(a1, a2, ..., aN) { ... }` or are you doing it for actual calls

[2024-11-13 22:39] mrexodia: for the call you should be gucci

[2024-11-13 23:45] contificate: you're not gucci

[2024-11-13 23:45] contificate: I think I have deciphered what Horsie means

[2024-11-13 23:46] contificate: ->args will iterate llvm::Argument which is a concept of a formal argument, which is tied to its parent and actually represents the incoming argument of the function, e.g. the value used by the called function

[2024-11-13 23:46] contificate: what you want instead is to actually get the argument operand at the relevant indices

[2024-11-13 23:46] contificate: getArgOperand(i) on the CallInst

[2024-11-13 23:46] contificate: or whatever other method there is

[2024-11-13 23:46] contificate: to reuse the operands somewhere else, usual constraints apply

[2024-11-13 23:48] contificate: you can note that in the case where there's no direct recursion, if you query the `Function` associated with the arguments you're iterating, Horsie, they'll be associated with the getCalledFunction function

[2024-11-13 23:48] contificate: not values belonging to the function containing the call instruction

[2024-11-14 05:29] Horsie: Just woke up folks.

[2024-11-14 05:29] Horsie: [replying to contificate: "getArgOperand(i) on the CallInst"]
Yep. This was the answer

[2024-11-14 05:30] Horsie: Apparently getting the args from getCalledFunction returns the SSA stuf of that's used only for that specific call

[2024-11-14 05:30] Horsie: Getting it directly from the callbase was what worked

[2024-11-14 05:31] Horsie: Thanks Colin :)

[2024-11-14 05:40] Horsie: Another thing that I'm trying to figure out over here-
I cant seem to figure out if I can import some .bc/.ll files into my llvm pass so I can insert calls to functions in said imported code directly without having to type it out in the IR builder (or having to provide a library at link time)

[2024-11-14 06:35] dinero: <@162611465130475520> didnt you give me paste for something very similar to this a few months ago? lol

[2024-11-14 06:36] dinero: (you might have to donate to  x64dbg first though)

[2024-11-14 08:30] Horsie: 👉 👈 😳

[2024-11-14 10:19] contificate: [replying to Horsie: "Apparently getting the args from getCalledFunction..."]
It's a value that's only valid in the function being called - not calls. Argument is a "formal" parameter, as opposed to actual parameter, so what you were doing before was trying to supply operands that represent incoming arguments to another function (and are only valid there)

[2024-11-14 18:39] Horsie: [replying to dinero: "<@162611465130475520> didnt you give me paste for ..."]
Figured it out 👍

[2024-11-14 18:40] Horsie: 1. parseIRFile (https://llvm.org/doxygen/llvm_2IRReader_2IRReader_8h.html) to get an independent Module with the function you want to use
2. Linker::linkInModule (https://llvm.org/doxygen/classllvm_1_1Linker.html#a5a864cc4d71e0234e9f8d786f8716804) to merge it with the other one you're dealing with.

[2024-11-14 23:53] mrexodia: yeahh this stuff is hella annoying

[2024-11-14 23:53] mrexodia: remill has a custom implementation for copying a function to another module

[2024-11-14 23:54] mrexodia: personally I just have a python script 'pipeline' and call the `llvm-link` utility to merge the bitcode files first <:KEKW:912974817295212585>

[2024-11-15 20:35] Max: Does anyone know how to work with those instructions im getting confused but did some of it
[Attachments: message.txt]

[2024-11-15 22:23] BWA RBX: [replying to contificate: "It's a value that's only valid in the function bei..."]
I don't get the part where you said "only valid in the function being called" and when you talk about "formal" parameters rather than "actual"

[2024-11-15 22:35] contificate: The "formal" part is terminology from PL theory (although pretty mainstream) that specifies that it's talking about the parameters as defined at entry of the function being called.

[2024-11-15 22:36] contificate: Whereas _actual_ parameters (what people mean 99% of the time when they refer to "arguments") are the actual call site values.

[2024-11-15 22:37] BWA RBX: I see thanks for explaining that part

[2024-11-15 22:37] contificate: > where you said "only valid in the function being called
They are only valid there because of LLVM's representation. The values he was using before are not the actual call site arguments, but the values you would use - within the called function code - to refer to the incoming arguments.

[2024-11-15 22:37] contificate: so it's like

[2024-11-15 22:37] contificate: he wanted to do

[2024-11-15 22:37] contificate: `f(1, 2, 3)` => `g(1, 2, 3)`

[2024-11-15 22:37] contificate: but instead he was trying to do

[2024-11-15 22:37] contificate: `g(x, y, z)` for some `f(int x, int y, int z)`

[2024-11-15 22:37] contificate: if that makes sense

[2024-11-15 22:38] BWA RBX: Yeah I see thanks for expanding on it

[2024-11-15 22:38] contificate: the args iterator on the called function will be over x, y, z LLVM values

[2024-11-15 22:39] contificate: LLVM is kind of awkward in this way

[2024-11-15 22:39] contificate: well, not awkward as it's quite convenient

[2024-11-15 22:39] contificate: but it's vital LLVM knowledge to know that LLVM's notion of names of temporaries is purely metadata

[2024-11-15 22:39] BWA RBX: I really don't like LLVM but a lot of the projects I see use it

[2024-11-15 22:40] contificate: in the sense that `add i32 %x, %y`, the `x` and `y` are just names a user would provide, but the actual pointer stored by that `add` instruction is actually just pointers to the defining instructions

[2024-11-15 22:40] contificate: this is why LLVM will complain when you use integer names and don't do it in the proper order, its parser has a deterministic numbering system to maintain

[2024-11-15 22:40] contificate: hence it's never good practice when generating code to explicitly name values as integers

[2024-11-15 22:40] contificate: either be nameless (in which case dumping it will provide names at the IR level for you to read it)

[2024-11-15 22:40] contificate: or use actual names

[2024-11-15 22:42] BWA RBX: Is this related to only SSA stuff

[2024-11-15 22:42] BWA RBX: Or in general

[2024-11-15 22:42] contificate: this is just how LLVM represents its IR

[2024-11-15 22:43] contificate: you can do SSA with a different concrete representation

[2024-11-15 22:43] contificate: but then it's somewhat more awkward for certain things

[2024-11-15 22:43] contificate: as you have to maintain names and push them around

[2024-11-15 22:43] contificate: whereas LLVM is like.. you can find an operand, immediately find its defining instruction, rewrite it, and all usage sites will have been rewritten

[2024-11-15 22:44] contificate: the def-use and use-def relation maintenance is less hassle

[2024-11-15 22:44] contificate: and you don't need to maintain much of a notion of globally unique names

[2024-11-15 22:44] contificate: but it does make following certain algorithms from textbooks kind of awkward

[2024-11-15 22:44] contificate: like LLVM is always in SSA

[2024-11-15 22:45] contificate: people target it by avoiding explicit SSA construction themselves but relying on mem2reg to optimise `alloca`s used in specific ways to be normal temporaries

[2024-11-15 22:45] BWA RBX: I've only recently been researching LLVM and I hate it cause I'm finding it hard to understand it lol

[2024-11-15 22:45] contificate: whereas the actual classical algorithm for constructing SSA

[2024-11-15 22:45] contificate: involves splitting live ranges by introducing phis

[2024-11-15 22:46] contificate: and then renaming the variables

[2024-11-15 22:46] contificate: but there are no meaningful names in LLVM so it's hard to represent such intermediary states without breaking how it's designed to work

[2024-11-15 22:46] contificate: the names like `%foo = ..` is purely metadata

[2024-11-15 22:47] contificate: you need a decent understanding of SSA form to understand LLVM

[2024-11-15 22:47] contificate: and fairly basic compiler topics

[2024-11-15 22:47] contificate: anyone can use bindings in an afternoon to compile some trivial language

[2024-11-15 22:49] BWA RBX: I actually got into it because of this 

https://github.com/emproof-com/nyxstone
[Embed: GitHub - emproof-com/nyxstone: Nyxstone: assembly / disassembly lib...]
Nyxstone: assembly / disassembly library based on LLVM, implemented in C++ with Rust and Python bindings, maintained by emproof.com - emproof-com/nyxstone

[2024-11-15 22:57] Respecter: LLVM is hard

[2024-11-15 23:06] contificate: what about it is hard

[2024-11-15 23:07] Respecter: The amount of content

[2024-11-15 23:10] 5pider: [replying to contificate: "what about it is hard"]
the LL but the VM is easy <:denjismirk:1299379832391471114>

[2024-11-15 23:11] contificate: there's not too much content

[2024-11-15 23:11] contificate: I know people who've written toy langs in a few evenings of effort

[2024-11-16 09:44] dinero: it’s not hard to get started at all

[2024-11-16 15:14] Torph: that's good to know, I've always been a little intimidated by it

[2024-11-16 18:10] contificate: use the bindings for a non-shit lang

[2024-11-16 18:10] contificate: and sorted for beginning

[2024-11-16 18:56] Deleted User: [replying to contificate: "and sorted for beginning"]
Don't tell me this is you
[Attachments: image.png]

[2024-11-16 18:56] 5pider: na its not

[2024-11-16 18:56] contificate: hahahha

[2024-11-16 18:56] contificate: what the actual fuck

[2024-11-16 18:57] Deleted User: Haha

[2024-11-16 18:59] contificate: crazy bro

[2024-11-16 18:59] contificate: I'm changing my name now

[2024-11-16 19:00] Deleted User: Good choice

[2024-11-16 19:10] Humza: [replying to Deleted User: "Don't tell me this is you"]
🤣🤣🤣🤣🤣🤣

[2024-11-16 19:10] Humza: Nahhh

[2024-11-16 19:15] contificate: https://tenor.com/view/die-dies-died-funny-fun-gif-27096031

[2024-11-16 19:23] Torph: [replying to contificate: "and sorted for beginning"]
wdym

[2024-11-16 20:11] contificate: you can get going with LLVM

[2024-11-16 20:11] contificate: and be very productive

[2024-11-16 20:11] contificate: at simple things like emitting it

[2024-11-16 20:11] contificate: if you use bindings

[2024-11-16 20:11] contificate: so that you are not using C++ directly

[2024-11-16 20:12] dinero: why did you search in tiktok

[2024-11-16 20:12] dinero: for his username

[2024-11-16 20:13] dinero: [replying to contificate: "so that you are not using C++ directly"]
what bindings in particular

[2024-11-16 20:14] contificate: OCaml, Python, whatever mayn

[2024-11-16 20:14] dinero: just not cpp i see your messaging

[2024-11-16 20:14] dinero: i guess that is a good idea since you’ll never encounter problems with bindings doing 101

[2024-11-16 20:14] dinero: How close to c++ are they in parity

[2024-11-16 20:14] dinero: just curious

[2024-11-16 20:15] contificate: they're intended for emitting IR

[2024-11-16 20:15] contificate: so identical in concepts

[2024-11-16 20:15] contificate: there's this handsome man on youtube

[2024-11-16 20:15] contificate: https://youtu.be/Brcs3GW5-hM
[Embed: Arithmetic Compiler in OCaml with LLVM Bindings]
This video intends to be a short demonstration of using LLVM bindings from OCaml. The hope is that more people will see OCaml's value for rapidly prototyping projects related to compilers (even those 

[2024-11-16 20:15] contificate: who accidentally used the wrong mic settings

[2024-11-16 20:15] contificate: and produced a video that's too quiet

[2024-11-16 20:18] Humza: [replying to contificate: "https://youtu.be/Brcs3GW5-hM"]
Fire

[2024-11-16 20:19] Humza: But Pls switch to dark theme bro damn soon as I opened up that video my eyes just started burning

[2024-11-16 20:20] Humza: Idk if eMacs has that tho I never used eMacs before

[2024-11-16 20:20] Humza: I just use vscode and sometimes vim if need to cook something up fast or test something out

[2024-11-16 20:22] contificate: no

[2024-11-16 20:23] Timmy: ofc emacs has themes

[2024-11-16 20:23] Humza: [replying to contificate: "no"]
Y

[2024-11-16 20:23] contificate: 
[Attachments: image.png]

[2024-11-16 20:23] contificate: ugly as shit

[2024-11-16 20:23] contificate: I'm on Windows rn

[2024-11-16 20:24] Humza: [replying to contificate: ""]
Doesn’t burn my eyes at least but yeh it’s ugly asf lmao

[2024-11-16 20:25] contificate: can't believe people sit and program in this OS

[2024-11-16 20:26] Timmy: gotta do what you gotta do

[2024-11-16 20:34] dinero: [replying to Humza: "But Pls switch to dark theme bro damn soon as I op..."]
reddit moment

[2024-11-16 20:35] dinero: [replying to contificate: "can't believe people sit and program in this OS"]
You program?

[2024-11-16 20:37] contificate: im learning verilog!

[2024-11-16 20:39] dinero: emacs plugin for quartus prime

[2024-11-16 20:45] dinero: i wish the rust LLVM bindings were this mature. there’s inkwell or the raw C bindings LLVM-sys

[2024-11-16 20:45] dinero: i guess there’s this too which claims to be more rusty 

https://github.com/cdisselkoen/llvm-ir
[Embed: GitHub - cdisselkoen/llvm-ir: LLVM IR in natural Rust data structures]
LLVM IR in natural Rust data structures. Contribute to cdisselkoen/llvm-ir development by creating an account on GitHub.

[2024-11-16 20:45] dinero: sad you just have to write c++ in the real world like a big boy

[2024-11-16 20:53] contificate: beginners just need shit to be productive

[2024-11-16 20:53] contificate: I'd actually say that beginners should just

[2024-11-16 20:53] contificate: write programs in LLVM IR

[2024-11-16 20:53] contificate: which is really straightforward

[2024-11-16 20:53] Humza: Beginners for what

[2024-11-16 20:54] contificate: LLVM

[2024-11-16 20:54] Humza: Fair enough

[2024-11-16 21:06] Torph: [replying to contificate: "so that you are not using C++ directly"]
is the C++ API that bad or like just if you don't know C++?

[2024-11-16 21:09] contificate: C++ is a huge waste of time for writing toy compiler-related things in

[2024-11-16 21:41] Timmy: > C++ is a huge waste of time
Coulda stopped there

[2024-11-16 21:42] contificate: it seriously depresses me that people will get really into such shit and forget to learn how to program along the way

[2024-11-16 21:42] Humza: [replying to contificate: "it seriously depresses me that people will get rea..."]
Frfr

[2024-11-16 21:42] Humza: My code still sucks ass after all this time

[2024-11-16 21:43] contificate: you would fail the interview challenges I'd give out!

[2024-11-16 21:43] Humza: I know bro lmao

[2024-11-16 21:43] Humza: Leetcode I haven’t done in ages

[2024-11-16 21:43] contificate: https://discord.com/channels/835610998102425650/835646666858168320/1237866434474807366

[2024-11-16 21:44] Humza: I wouldn’t be able to do that ngl

[2024-11-16 21:44] Humza: I suck ass with algorithms etc

[2024-11-16 21:45] Humza: Idk if that’s a bad thing or not

[2024-11-16 21:45] Humza: I guess I still have lots of time to learn right? Or am I cooked

[2024-11-16 21:46] contificate: you get better at algorithms the more you do algorithm stuff

[2024-11-16 21:46] contificate: the neat thing about learning algorithms is that

[2024-11-16 21:46] contificate: any sufficiently good education in algorithms is basically permanent

[2024-11-16 21:46] contificate: it's a very clean thing to study in that way - most of the time, anyway

[2024-11-16 21:46] contificate: it's like very rich to dig your teeth into

[2024-11-16 21:46] Humza: I can work with certain data structures quite well though like linked lists and queue’s

[2024-11-16 21:47] contificate: have you implemented a treiber stack

[2024-11-16 21:47] Humza: Because of writing kernel drivers linked lists r everywhere so I got decent with them

[2024-11-16 21:47] Humza: [replying to contificate: "have you implemented a treiber stack"]
Nah

[2024-11-16 21:47] contificate: that's a really easy lock-free stack you can implement with linked lists

[2024-11-16 21:47] contificate: compare-and-swap of the head to which every thread atomically refers

[2024-11-16 21:47] Humza: I usually use linked lists like a queue

[2024-11-16 21:48] Humza: Rather than a stack

[2024-11-16 21:48] contificate: I use them as a tree fairly often

[2024-11-16 21:48] Humza: I remember few months ago aswell I wrote a crappy ransomware without using recursion and I came up with just using a linked list queue and it worked quite well for it, made it multithreaded asw

[2024-11-16 21:49] contificate: hope you know that linked lists are really a building block

[2024-11-16 21:49] contificate: in the same way that hash tables are

[2024-11-16 21:49] Humza: Yeh

[2024-11-16 21:49] contificate: you have linked list structure in all of programming

[2024-11-16 21:49] contificate: can induce any indirection to model a list of some kind

[2024-11-16 21:49] Humza: True ye

[2024-11-16 21:49] contificate: you should implement one of my favourite data structures

[2024-11-16 21:49] contificate: the prefix tree ("trie")

[2024-11-16 21:49] Humza: What’s that

[2024-11-16 21:50] contificate: it's a tree that stores strings

[2024-11-16 21:50] safareto: [replying to contificate: "https://discord.com/channels/835610998102425650/83..."]
im guessing no parsing or anything of the sort?

[2024-11-16 21:50] contificate: but it will ensure sharing of prefixes of those strings

[2024-11-16 21:50] contificate: no parsing at all

[2024-11-16 21:50] contificate: but that's easy too

[2024-11-16 21:50] dinero: this is pointless though because he can cheat

[2024-11-16 21:50] dinero: let’s be real

[2024-11-16 21:50] contificate: what do you mean

[2024-11-16 21:50] contificate: cheat

[2024-11-16 21:50] dinero: what was the question

[2024-11-16 21:50] dinero: the cps conversioh one

[2024-11-16 21:50] dinero: ?

[2024-11-16 21:50] contificate: what

[2024-11-16 21:51] dinero: oh nvm i read 3 messages

[2024-11-16 21:51] contificate: I've never given CPS conversion out as a programming exercise

[2024-11-16 21:51] safareto: [replying to contificate: "no parsing at all"]
okay yeah i have no idea how i'd do this in c/cpp

[2024-11-16 21:51] safareto: im guessing im not supposed to?

[2024-11-16 21:51] dinero: anf conversion i think

[2024-11-16 21:51] contificate: it's not really ANF conversion but yeah it's the same idea

[2024-11-16 21:51] contificate: you can do it in C and C++

[2024-11-16 21:52] contificate: the issue is

[2024-11-16 21:52] contificate: most people can't do it off the bat

[2024-11-16 21:52] contificate: or even know where to start with representing the expressions they're meant to manipulate

[2024-11-16 21:52] contificate: because they don't deal with that kind of symbolic programming very often

[2024-11-16 21:52] contificate: or

[2024-11-16 21:52] contificate: have just never dealt with that

[2024-11-16 21:52] safareto: [replying to contificate: "because they don't deal with that kind of symbolic..."]
yeah

[2024-11-16 21:52] contificate: their day-to-day of programming

[2024-11-16 21:52] contificate: is like

[2024-11-16 21:52] contificate: calling functions, storing return values in hash tables, iterating tables, and calling more functions

[2024-11-16 21:52] Humza: My cat is shaped like a croissant when he sleeps

[2024-11-16 21:53] contificate: which is a very limited view of the world

[2024-11-16 21:53] safareto: i remember a lecturer once mentioning a similar problem in one of my structured programming lectures

[2024-11-16 21:53] safareto: i think he was taught something like this in a compilers subject

[2024-11-16 21:53] contificate: yeah

[2024-11-16 21:53] contificate: but you can equally dream up areas a lot of people are unfamiliar with

[2024-11-16 21:53] contificate: that's actually rather mainstream

[2024-11-16 21:53] contificate: for example

[2024-11-16 21:53] contificate: asynchronous programming and all of the ideas in that

[2024-11-16 21:54] safareto: [replying to contificate: "asynchronous programming and all of the ideas in t..."]
oh yeah

[2024-11-16 21:54] contificate: most people can understand something like `await ..` at some level

[2024-11-16 21:54] contificate: but it's not like they're overly familiar with explicitly programming around an event loop

[2024-11-16 21:54] contificate: as you would if you wrote C and used libuv

[2024-11-16 21:54] contificate: or whatever

[2024-11-16 21:54] contificate: or even just epoll where you'd invent that yourself

[2024-11-16 21:54] contificate: lots of interesting topics arise from that kind of programming

[2024-11-16 21:55] contificate: stackful coroutines, continuation passing style, monadic style, delimited continuations, etc.

[2024-11-16 21:55] Humza: Monads

[2024-11-16 21:55] safareto: [replying to contificate: "lots of interesting topics arise from that kind of..."]
im very curious about this sort of thing

[2024-11-16 21:55] contificate: if you wanna get really knowledgeable about programming, you have to have a huge coverage of different ideas in different domains

[2024-11-16 21:55] safareto: not the kind of programming ideas i see on a daily basis

[2024-11-16 21:56] contificate: "what language features would make it convenient to do asynchronous programming"

[2024-11-16 21:56] contificate: it would be ones that hide the gory details of green or coop multithreading from you

[2024-11-16 21:56] contificate: so what are the details of that

[2024-11-16 21:56] safareto: lol

[2024-11-16 21:56] contificate: "what language features would make it convenient to do various compiler transformations"

[2024-11-16 21:56] contificate: it's not even that C++ is bad for writing compilers in the first place

[2024-11-16 21:57] contificate: the real fucking killer to me

[2024-11-16 21:57] contificate: is the fact that it's far far worse in its role as being really bad for _learning_ to do anything in

[2024-11-16 21:57] luci4: [replying to contificate: "https://discord.com/channels/835610998102425650/83..."]
Really curious as to how a solution would look like in C++

[2024-11-16 21:57] safareto: [replying to contificate: "is the fact that it's far far worse in its role as..."]
wdym?

[2024-11-16 21:57] safareto: and what would be "good" for learning here?

[2024-11-16 21:57] contificate: you can't build a mental model effectively if you are burdened by your tools

[2024-11-16 21:57] contificate: if you're learning something

[2024-11-16 21:57] contificate: you want a quick workflow

[2024-11-16 21:58] contificate: to adapt and retry

[2024-11-16 21:58] contificate: you don't wanna flail around like

[2024-11-16 21:58] contificate: "oh, I need to add another kind of expression to this, guess I'll write an entire new class for it and adjust my visitors"

[2024-11-16 21:58] contificate: or

[2024-11-16 21:58] contificate: "I need to give a shit about memory management and ownership"

[2024-11-16 21:58] contificate: you don't if you're learning to do a transformation

[2024-11-16 21:59] contificate: you want to be building a mental model

[2024-11-16 21:59] contificate: I know several people who learned compiler concepts and initially got interest by doing some course that was OCaml or SML or whatever

[2024-11-16 21:59] contificate: and now they write compilers in C

[2024-11-16 21:59] contificate: the point is

[2024-11-16 21:59] contificate: they would've had a real hard time finding it interesting

[2024-11-16 21:59] contificate: or making good progress

[2024-11-16 21:59] contificate: if they were stuck banging their heads with C all day

[2024-11-16 22:00] contificate: I can do compiler stuff in C because I know the ideas and have built a mental model in OCaml

[2024-11-16 22:00] contificate: so if I feel things are getting nasty

[2024-11-16 22:00] contificate: I can take a step back

[2024-11-16 22:00] contificate: appeal to that mental model

[2024-11-16 22:00] contificate: and compile it to C in my head

[2024-11-16 22:00] mrexodia: Yeah I do concur. That’s why it’s helpful to try out higher level languages, because you get an idea what kind of APIs are possible

[2024-11-16 22:00] safareto: correct me if im wrong, but is this the kind of thing where people look at python or something "high level" that should/would be implemented in C instead?

[2024-11-16 22:00] contificate: good programmers are eclectic

[2024-11-16 22:01] contificate: see if you ever seen any bullshit language elitism

[2024-11-16 22:01] safareto: [replying to safareto: "correct me if im wrong, but is this the kind of th..."]
as in, same mentality?

[2024-11-16 22:01] contificate: like "haha, you don't only write C? noob xddd"

[2024-11-16 22:01] contificate: you can sleep well at night

[2024-11-16 22:01] contificate: knowing that the person saying that

[2024-11-16 22:01] contificate: is literally gatekeeping themselves

[2024-11-16 22:01] contificate: from a world of fun and interesting ideas

[2024-11-16 22:01] contificate: genuinely

[2024-11-16 22:01] contificate: eventually if you get good coverage over PLs

[2024-11-16 22:01] Humza: Tcsh is the superior lang

[2024-11-16 22:01] contificate: perhaps by even explicitly studying programming languages

[2024-11-16 22:02] contificate: you can start to just know how things are implemented and what they're actually called

[2024-11-16 22:02] contificate: for example, nominal subtyping, coercive subtyping, ad-hoc polymorphism, parametric polymorphism, row polymorphism, etc.

[2024-11-16 22:02] contificate: these are all general PL ideas that are implemented in various ways in programming languages

[2024-11-16 22:03] contificate: [replying to safareto: "as in, same mentality?"]
yeah

[2024-11-16 22:03] contificate: you don't get brownie points for being close-minded

[2024-11-16 22:03] contificate: high level languages benefit from a level of abstraction that can make expressing ideas more clear

[2024-11-16 22:03] contificate: and new idioms emerge in them

[2024-11-16 22:03] contificate: new language features

[2024-11-16 22:03] contificate: programmers should be eclectic

[2024-11-16 22:03] contificate: for example

[2024-11-16 22:03] contificate: I know a guy who absolutely despises JS

[2024-11-16 22:04] contificate: and he happens to be the guy that knows the absolute most about the language and its internals (V8 impl)

[2024-11-16 22:04] contificate: than anyone I've ever met

[2024-11-16 22:04] contificate: right

[2024-11-16 22:04] contificate: you can dislike things

[2024-11-16 22:04] contificate: but you should be able to back it up

[2024-11-16 22:04] contificate: so you don't appear to be an uneducated hater

[2024-11-16 22:04] contificate: simply scared to admit that learning the basics of C was not sufficient background to make them a great programmer

[2024-11-16 22:06] contificate: [replying to luci4: "Really curious as to how a solution would look lik..."]
do you know of a solution

[2024-11-16 22:06] safareto: [replying to contificate: "and he happens to be the guy that knows the absolu..."]
damn

[2024-11-16 22:06] contificate: forget the concrete programming language

[2024-11-16 22:06] contificate: the approach is everything

[2024-11-16 22:06] contificate: the details are the implementation of those ideas

[2024-11-16 22:07] safareto: [replying to contificate: "you don't get brownie points for being close-minde..."]
i don't think i ever understood *why* people do this

[2024-11-16 22:07] dinero: tsk tsk

[2024-11-16 22:07] contificate: they think there's some kind of elitism in wasting their own time

[2024-11-16 22:07] safareto: it's like console wars but way worse because there's a bazillion languages

[2024-11-16 22:07] Humza: Got to pick the right tool for the job fr

[2024-11-16 22:08] dinero: cursor + claude

[2024-11-16 22:08] dinero: don’t learn to code

[2024-11-16 22:08] safareto: [replying to contificate: "and he happens to be the guy that knows the absolu..."]
i don't think you need this to hate a language, but i agree that you need to at least back up your opinions on something you're gonna shit on relentlessly

[2024-11-16 22:08] Humza: Ngl I stopped writing code and just became homeless

[2024-11-16 22:08] Humza: Fuckit

[2024-11-16 22:08] luci4: [replying to contificate: "do you know of a solution"]
Off the top of my head, no, honestly.

[2024-11-16 22:08] contificate: the really fucking depressing part about programming language close-mindedness is that seasoned programmers do it - which is most sad because they could spend like a weekend becoming familiar with a language and then have something intelligible to say about it - or even have their ideas challenged

[2024-11-16 22:09] contificate: people always ask "should I learn X"

[2024-11-16 22:09] contificate: as if it's not a weekend task

[2024-11-16 22:09] contificate: to dig into it for a bit

[2024-11-16 22:09] contificate: and report back

[2024-11-16 22:09] Humza: I don’t rlly think the lang matters that much just choose the right tool for what ur tryna do , trying to use one specific tool for everything is retarded

[2024-11-16 22:09] luci4: Mostly because I'm not sure how I could "read" an in-program representation of that.

[2024-11-16 22:10] contificate: I don't like JS at all but I can dream up good things to say about it

[2024-11-16 22:10] Humza: Js required for front end dev anyway

[2024-11-16 22:10] contificate: I think it's a good place for beginners to get an idea of how event-driven/async programming can work

[2024-11-16 22:10] Humza: Can hate it ye but still got to use it if u wanna do front end and a lot of the time backend asw

[2024-11-16 22:10] contificate: setInterval and setTimeout etc.

[2024-11-16 22:10] contificate: and it's accessible and in their browser already

[2024-11-16 22:10] contificate: that's about the gist of the good things I have to say about JS

[2024-11-16 22:10] contificate: the rest is all bad

[2024-11-16 22:10] Humza: What about

[2024-11-16 22:10] Humza: Tcsh

[2024-11-16 22:11] contificate: can't comment

[2024-11-16 22:11] Humza: The goat lang

[2024-11-16 22:11] Humza: The best scripting lang out there

[2024-11-16 22:11] contificate: [replying to luci4: "Mostly because I'm not sure how I could "read" an ..."]
what do you mean

[2024-11-16 22:11] Humza: Ima put experienced tcsh/csh dev on my cv

[2024-11-16 22:11] contificate: you mean you aren't sure how to concretely represent such expressions in your programming language of choice

[2024-11-16 22:11] luci4: Yep!

[2024-11-16 22:11] Humza: [replying to Humza: "Ima put experienced tcsh/csh dev on my cv"]
And they won’t know wtf it is

[2024-11-16 22:11] Humza: And ima be sad

[2024-11-16 22:11] contificate: yeah, and this is unsurprising

[2024-11-16 22:11] contificate: the reason being that

[2024-11-16 22:12] contificate: many mainstream languages make the encoding of inductive types/variants very tedious

[2024-11-16 22:12] contificate: by not being a first class concept in the language

[2024-11-16 22:13] Humza: Syrup makes u say a function is inductive when defining it

[2024-11-16 22:13] Humza: Syrup the goat fr

[2024-11-16 22:13] Humza: Gonna bootstrap syrup

[2024-11-16 22:13] contificate: does it support sequential circuits

[2024-11-16 22:13] contificate: or is it purely combinational

[2024-11-16 22:13] Humza: [replying to contificate: "does it support sequential circuits"]
Idk bro

[2024-11-16 22:13] luci4: Although I only write C++ and C# (.NET Core) regularly, so I haven't faced such a problem before.

[2024-11-16 22:13] contificate: C++ is really tedious for this

[2024-11-16 22:13] Humza: I am clueless with syrup so far man ngl I’m cooked outere

[2024-11-16 22:13] contificate: C# has something you can do that's pretty alright

[2024-11-16 22:14] luci4: Where I need to represent a symbolic expression in my code

[2024-11-16 22:14] Humza: Functional programming got me in the mud it’s hard to wrap my head around bc it’s so damn different to anytning I’ve used before I have to change like every way I think

[2024-11-16 22:14] Humza: When it comes to solving a problem in a programming language

[2024-11-16 22:14] contificate: yeah, but that's a great thing

[2024-11-16 22:14] contificate: you know

[2024-11-16 22:14] contificate: I spend so much of my time

[2024-11-16 22:14] contificate: in waiting

[2024-11-16 22:15] contificate: waiting for the next big idea that's gonna change how I think about everything

[2024-11-16 22:15] contificate: there's lots of things that have been cute diversions on topics I already knew about but didn't bother using

[2024-11-16 22:15] contificate: like Prolog

[2024-11-16 22:15] contificate: but I haven't yet found something that's like "wow, this is great"

[2024-11-16 22:15] contificate: maybe it'll be some kind of software transactional memory

[2024-11-16 22:15] safareto: [replying to contificate: "waiting for the next big idea that's gonna change ..."]
sounds like analysis paralysis lmao

[2024-11-16 22:15] contificate: or something

[2024-11-16 22:16] contificate: it's more of a background thread

[2024-11-16 22:16] contificate: I download textbooks for random topics every other day

[2024-11-16 22:16] contificate: and scan through them

[2024-11-16 22:16] contificate: for example, I recently got a tiny bit interested in designing microprocessors in VHDL

[2024-11-16 22:16] contificate: it's neat to learn all the important things there

[2024-11-16 22:16] contificate: and explicitly cover things I kinda knew about vaguely

[2024-11-16 22:16] contificate: but not the details

[2024-11-16 22:16] dinero: colin dude i just don’t thin most people care as much about computer programming as you do and that’s all there is to it. sad but true

[2024-11-16 22:17] contificate: which is very similar to various other things in async programming

[2024-11-16 22:17] safareto: [replying to dinero: "colin dude i just don’t thin most people care as m..."]
i respect the passion tbh

[2024-11-16 22:17] safareto: i love info dumps like this

[2024-11-16 22:19] dinero: nah it’s a blackpill

[2024-11-16 23:43] BWA RBX: [replying to contificate: "https://youtu.be/Brcs3GW5-hM"]
Nice video, the guy sounds like an introvert but was interesting nonetheless

[2024-11-16 23:44] contificate: he sounds hot as well

[2024-11-16 23:44] BWA RBX: [replying to contificate: "he sounds hot as well"]
Questionable 😼

[2024-11-17 01:13] Humza: Idrk that much about cryptography so my bad if this question sounds kinda stupid but

[2024-11-17 01:14] Humza: Basically I’m implementing a key bruteforcing mechanism in something where I’ll have keys encrypted with something ‘light’ that I can brute force to then get the real key to decrypt large data

[2024-11-17 01:15] Humza: So far I’ve managed to get it so I’m xor’ing the key I’m using to decrypt the data and then at runtime bruteforcing the xor keys but I feel like it’s anyone’s first guess and it’s easy to see that I’m xor’ing it and how im bruteforcing it so they can easily write up a script to brute force the data themselves

[2024-11-17 01:15] Humza: Is there any alternative approaches here that I’m missing

[2024-11-17 01:41] dullard: Why

[2024-11-17 02:21] Humza: [replying to Humza: "So far I’ve managed to get it so I’m xor’ing the k..."]
^

[2024-11-17 02:23] Humza: this is being applied within a polymorphic engine

[2024-11-17 03:08] qw3rty01: that's what, not why

[2024-11-17 08:31] Torph: [replying to contificate: "https://discord.com/channels/835610998102425650/83..."]
break down into a tree where each operator has 2 children, ending in leaves that have a value, then recurse back up the tree turning each evaluated operation node into a variable like `t0`? that'd be my first go-to idea. maybe swap out recursion for a queue of things to operate on

[2024-11-17 08:36] Torph: [replying to contificate: "the really fucking depressing part about programmi..."]
yeah I went and picked up Go at a hackathon and it was really eye-opening after being in C++ for so long

[2024-11-17 12:35] Humza: [replying to Torph: "yeah I went and picked up Go at a hackathon and it..."]
Go is a great lang I love go

[2024-11-17 15:17] contificate: [replying to Torph: "break down into a tree where each operator has 2 c..."]
pretty much

[2024-11-17 15:17] contificate: this becomes problematic in more complex scenarios

[2024-11-17 15:18] contificate: but for simple arithmetic, this will suffice

[2024-11-17 15:19] contificate: I don't follow the "break down" into a tree part

[2024-11-17 15:19] contificate: unless you are describing some part of parsing

[2024-11-17 15:20] contificate: your in-program representation of arithmetic will already be a tree shape

[2024-11-17 15:41] naci: [replying to contificate: "https://discord.com/channels/835610998102425650/83..."]
```x * y + z * 7 + 3```
parse for higher order operations
extract `x * y `, `t0` = `op:mul, rhs:x, lhs:y`
replace `x * y` with `t0` in new string
```t0 + z * 7 + 3```
repeat until cant

[2024-11-17 15:45] contificate: that's interesting but it's not a string transformation problem really

[2024-11-17 15:45] contificate: although doing that until a fixpoint is pretty much the same transformation

[2024-11-17 15:45] contificate: there's nothing higher order in there

[2024-11-17 15:46] contificate: I think you mean nested subexpressions or non-atomic/compound operations

[2024-11-17 15:46] naci: i mean we can transform the expressions to llvm ir from there, but its not too interesting for me after you get the expressions

[2024-11-17 15:46] contificate: you can emit the LLVM IR directly

[2024-11-17 15:46] naci: [replying to contificate: "there's nothing higher order in there"]
i meant order of parsing is first search for brackets, then mul/div, then +/-

[2024-11-17 15:47] naci: add function calls somewhere there

[2024-11-17 15:47] contificate: you wouldn't "search" in that way in a parser

[2024-11-17 15:47] contificate: but yeah

[2024-11-17 15:47] contificate: as it happens, I actually wrote a python impl of this for someone on reddit to emit LLVM IR for it

[2024-11-17 15:48] contificate: 
[Attachments: YG50sQc.png]

[2024-11-17 15:48] contificate: I thread names around using continuations though, as I find that more flexible in general

[2024-11-17 15:50] naci: yeah you dont need to parse if you hardcode the expressions

[2024-11-17 15:50] contificate: do you know a good approach for parsing it

[2024-11-17 15:51] naci: [replying to naci: "```x * y + z * 7 + 3```
parse for higher order ope..."]
no but i would do it like this probably

[2024-11-17 15:51] naci: never needed to parse it as a string tbh

[2024-11-17 15:51] contificate: you'd wanna use pratt parsing

[2024-11-17 15:52] contificate: tada

[2024-11-17 15:52] contificate: 
[Attachments: 2024-10-25-195402_438x694_scrot.png]

[2024-11-17 15:53] contificate: now you can rewrite mrexodia's expression parser in x64dbg 😏

[2024-11-17 16:11] naci: ill look it up!

[2024-11-17 16:12] naci: [replying to contificate: ""]
whats the point of nud though?

[2024-11-17 16:12] contificate: if you had `E -> '(' E ')'`

[2024-11-17 16:12] contificate: you'd want `nud` to handle that rule

[2024-11-17 16:12] naci: 🤔

[2024-11-17 16:12] contificate: `led` and `nud` are terminology from pratt parsers

[2024-11-17 16:13] contificate: this was written to teach someone pratt parsing

[2024-11-17 16:13] contificate: it's redundant here but in general, you'd extend `nud` for rules will null denotations

[2024-11-17 16:13] naci: ic ic

[2024-11-17 16:13] contificate: which `( E )` has

[2024-11-17 16:13] contificate: e.g.

[2024-11-17 16:14] contificate: ```py
def nud(t):
  if t == '(':
    e = expr(0)
    shift() # check that it's ), otherwise raise error
    return e
  ...
```

[2024-11-17 18:14] Torph: [replying to contificate: "your in-program representation of arithmetic will ..."]
if it's already a tree then yeah skip that
I just did an assignment almost exactly like this so the parsing is still fresh in mind

[2024-11-17 18:15] contificate: are you gonna write a compiler

[2024-11-17 18:16] Torph: [replying to contificate: "but for simple arithmetic, this will suffice"]
what kinds of scenarios does it break down in?

[2024-11-17 18:17] Torph: [replying to contificate: "are you gonna write a compiler"]
nah they just had us write an postfix expression interpreter that would act like a calculator printing the final result and infix form

[2024-11-17 18:17] Torph: plus variable and ternary support

[2024-11-17 18:40] contificate: [replying to Torph: "what kinds of scenarios does it break down in?"]
name management becomes really tedious

[2024-11-17 18:40] contificate: like if you add support for anonymous functions

[2024-11-17 18:40] contificate: they would need to normalise to a local function definition and have their own expression body that's linearised

[2024-11-17 18:40] contificate: can be arbitrarily nested

[2024-11-17 18:40] contificate: this is often done with stacks of names etc.

[2024-11-17 18:40] contificate: but continuations model the problem nicely

[2024-11-17 18:41] contificate: postfix is weak

[2024-11-17 18:41] contificate: they should be teaching pratt parsing

[2024-11-17 18:54] Torph: [replying to contificate: "they would need to normalise to a local function d..."]
oh like you need to insert the function inline? does linearise mean turning it into a linear sequence of operations as opposed to loops and such?

[2024-11-17 18:54] Torph: [replying to contificate: "they should be teaching pratt parsing"]
I'll look into that
also not sure what continuations are

[2024-11-17 19:11] contificate: yeah, the nested function becomes its own thing you represent and then you push its name down to its usage site

[2024-11-17 19:12] contificate: would focus on pratt parsing first, it's really straightforward

[2024-11-17 19:12] contificate: linearising expressions into various IRs can be annoying

[2024-11-17 19:12] contificate: it's fairly common in imperative languages to do it in two stages

[2024-11-17 19:12] contificate: actually, well.. it's kinda 2 stages everywhere, even if not explicit

[2024-11-17 19:13] contificate: one day I'd like to make some youtube videos about how you can begin writing assembly programs

[2024-11-17 19:13] contificate: a lot of uni students I deal with online seem to believe there's some kind of divine inspiration you need to write assembly

[2024-11-17 19:13] contificate: unaware that there's actually simple strategies based on program transformation from C

[2024-11-17 19:13] contificate: that make it trivial to write bad assembly

[2024-11-17 19:14] Deleted User: [replying to contificate: "a lot of uni students I deal with online seem to b..."]
I would say assembly is simpler than C++.

[2024-11-17 19:14] contificate: I would agree

[2024-11-17 19:14] contificate: but, as always

[2024-11-17 19:14] contificate: the complexity is in scale

[2024-11-17 19:14] Deleted User: [replying to contificate: "the complexity is in scale"]
What do you mean

[2024-11-17 19:14] contificate: maintaining a large assembly program

[2024-11-17 19:14] Deleted User: Ah, true.

[2024-11-17 19:14] contificate: is complexity in the from of extreme tedium

[2024-11-17 19:14] contificate: but yeah

[2024-11-17 19:14] contificate: at its core, assembly is far simpler than C++

[2024-11-17 19:15] contificate: it's arithmetic, loads, stores, comparisons, and jumps

[2024-11-17 19:15] Deleted User: That's true

[2024-11-17 19:15] contificate: but the point is

[2024-11-17 19:15] contificate: universities teach it poorly

[2024-11-17 19:15] contificate: by not showing how compilers work

[2024-11-17 19:15] contificate: compilers have intermediary representations and transformations

[2024-11-17 19:15] contificate: that are helpful to writing assembly by hand

[2024-11-17 19:15] Deleted User: [replying to contificate: "universities teach it poorly"]
The education system teaches everything comp-sci related horribly in my opinion

[2024-11-17 19:16] Deleted User: So not surprised assembly is taught poorly

[2024-11-17 19:18] Humza: [replying to contificate: "universities teach it poorly"]
Become lecturer at strath pls mate thanks

[2024-11-17 19:18] Humza: You’d be a fire lecturer

[2024-11-17 19:24] contificate: I would if I could

[2024-11-17 19:24] contificate: dream job is just teaching CS topics

[2024-11-17 19:24] contificate: all day

[2024-11-17 19:26] luci4: Professor Colin

[2024-11-17 19:56] Torph: [replying to contificate: "a lot of uni students I deal with online seem to b..."]
so many peope seem obsessed & elitist about writing assembly when it's like objectively the worse option in most cases. idk maybe you could say that about a lot of non-performance-critical C/C++ programs too

[2024-11-17 20:56] Torph: [replying to contificate: "they should be teaching pratt parsing"]
just read up on this, I really like the idea of having parser behaviour defined by a precedence lookup table. cool stuff! the use of Rust in the article I read threw me off but I picked up the unfamiliar functional syntax after a little bit

[2024-11-17 23:06] contificate: yeah it's a cute skeleton algorithm to parse all kinds of shit

[2024-11-17 23:06] contificate: if you think it'll be tricky

[2024-11-17 23:06] contificate: can just start with a pratt parser

[2024-11-17 23:06] contificate: and focus on each part piece by piece

[2024-11-17 23:25] Humza: [replying to contificate: "yeah it's a cute skeleton algorithm to parse all k..."]
It’s a cutie patootie?