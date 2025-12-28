# May 2025 - Week 2
# Channel: #programming
# Messages: 232

[2025-05-06 17:29] david: Hello, is there a alternative for hotexamples.com, I noticed It went down and currently I'm in a stuation where I could really use it.

I know this is probably not the right place to ask, but I dont know any better

[2025-05-06 19:14] dullard: [replying to david: "Hello, is there a alternative for hotexamples.com,..."]
What’s the site for ( haven’t heard of it before ) but does it give code examples in different languages ? 

If so, does Rosetta code work ?

[2025-05-06 19:30] david: [replying to dullard: "What’s the site for ( haven’t heard of it before )..."]
Rosetta code, even if it looks like a cool webside, does not work for me.

 Hotexample gave you the ability to search for "Api" Function names (for example NtWriteFile) and gave back crawled code-examples from github.

I'm currently exploring dbgeng api in c++, and the documentation is very poor. So hotexample could have been a valuable source.

[2025-05-06 19:50] mrexodia: [replying to david: "Rosetta code, even if it looks like a cool webside..."]
https://cs.github.com
[Embed: Build software better, together]
GitHub is where people build software. More than 150 million people use GitHub to discover, fork, and contribute to over 420 million projects.

[2025-05-06 19:50] mrexodia: https://grep.app/
[Embed: Code Search | Grep by Vercel]
Effortlessly search for code, files, and paths across a million GitHub repositories.

[2025-05-06 20:03] david: this is so perfect I want to kiss you

[2025-05-06 20:08] mrexodia: let's avoid that

[2025-05-06 20:15] david: 😦

[2025-05-06 20:15] david: XD

[2025-05-06 20:44] 5pider: He is unfortunately married. He has a husband already

[2025-05-06 21:30] dullard: Yes, 👰‍♂️

[2025-05-07 04:12] david: congratulations ☺️

[2025-05-07 06:37] James: [replying to Ruben Mike Litros: "what is the point of `UWOP_SET_FPREG` operation co..."]
First...

[2025-05-07 06:37] James: It obviously cannot be a volatile register, because those are not restored when unwinding.

[2025-05-07 06:37] James: So if you have a frame pointer in A, and A calls B, and B raises, A's frame pointer in a volatile register will not be restored.

[2025-05-07 06:38] James: Second, it's purpose is to allow for non-constant stack allocations. Think VLAs and alloca.

[2025-05-07 06:38] James: LLVM also uses it in an (failed)attempt to support Windows SEH constructs.

[2025-05-07 06:39] James: SET_FPREG allows you to specify the offset from the FP to where RSP should be adjusted back to before continuing the remaining unwind codes.

[2025-05-07 06:41] James: It's very likely you will see a serquence like this

```
PUSH other_nonvols
PUSH RBP
SUB RSP,20h
MOV RBP,RSP or LEA RBP,[RSP+Offset]
```

[2025-05-07 06:41] James: Additionally when a framepointer is present, the epilog starts with a `LEA RSP,[RBP+(-Offset)]`

[2025-05-07 06:42] James: Your best references are the .net unwinder, reactos, ntoskrnl's impl of RtlUnwindEpilog(or whatever it's proper name is)

[2025-05-07 06:43] James: Additionally be aware that the docs are outdated and the unwind info is now on version 2, at least that is what modern msvc is emitting. This brings new unwind codes that specify epilog locations so decoding becomes less needed.

[2025-05-07 07:55] Ruben Mike Litros: i think i get it now, i thought the unwinder goes through unwind code list sequentially which would mean that besides the `SAVE_*` entries, `SET_FPREG` had to be the first entry (since the list is inverted) because if there were say push operations before it then the unwinder wouldn't be able to restore them because the stack's size is unknown but now i realize it probably works more like this -> check if fp used, find `SET_FPREG` entry, calculate stack base, then check if theres any push entries before `SET_FPREG` and if there are restore them by doing new_stack_base - 8 then do this for all entries that come before `SET_FPREG` and then restore entries that come after `SET_FPREG`. i think i didn't explain it very well but is this kind of how the unwinder works?

[2025-05-08 02:55] valium: ```cpp
typedef union _table {
  u64 value;
  struct {
    u64 flag_1: 1;
    u64 flag_2: 1;
    u64 remaining: 62;
  } bits;
} table, *ptable;
```
how can i do this in rust?

[2025-05-08 03:15] qw3rty01: rust doesn't have bitfields, so you'll need to use an external library for it: https://crates.io/crates/bitfield

[2025-05-08 05:40] valium: [replying to qw3rty01: "rust doesn't have bitfields, so you'll need to use..."]
kinda sucks to not have inbuilt bitfields but yeah this looks good

[2025-05-08 05:53] James: [replying to valium: "kinda sucks to not have inbuilt bitfields but yeah..."]
No it doesn't.

[2025-05-08 05:54] James: `table.bits.flag_1 << 15` has vastly different meanings depending on the compiler you're using.

[2025-05-08 05:54] valium: [replying to James: "`table.bits.flag_1 << 15` has vastly different mea..."]
can confirm

[2025-05-08 05:54] James: Although that isn't inherently an issue with the bitfield itself, still sucks.

[2025-05-08 05:55] valium: [replying to James: "Although that isn't inherently an issue with the b..."]
what would you use then? a crate?

[2025-05-08 05:55] valium: i kinda dont want to do some hacky stuff implementing my own bitfields so i think a crate would be fine

[2025-05-08 05:56] James: [replying to valium: "what would you use then? a crate?"]
shifting and masking.

[2025-05-08 05:57] James: that crate will work fine though.

[2025-05-08 05:58] James: there are also crates to give you portable functionality of bit extracts. and they use bextr when evaliable on x86 and whatever arm's counterpart is.

[2025-05-08 05:58] valium: [replying to James: "there are also crates to give you portable functio..."]
okay this sounds great which crates are those?

[2025-05-08 05:59] James: google "rust bit extract"

[2025-05-08 06:00] valium: "bextr" is this some bmi instruction

[2025-05-08 06:01] Lyssa: [replying to valium: ""bextr" is this some bmi instruction"]
yes

[2025-05-08 21:22] Tommyrexx: https://github.com/EpsilonNought117/libapac
[Embed: GitHub - EpsilonNought117/libapac: Library for arbitrary precision ...]
Library for arbitrary precision arithmetic in C and Assembly - EpsilonNought117/libapac

[2025-05-08 21:23] Tommyrexx: My arbitrary precision arithmetic library (WIP) being written for x64 on windows in C and asm

[2025-05-09 09:37] mrexodia: [replying to Tommyrexx: "https://github.com/EpsilonNought117/libapac"]
Thanks! We need an easy-to-compile alternative to GMP with a liberal license 🙏

[2025-05-09 09:38] Tommyrexx: I use the MIT License. I don't like LGPL v3.0 personally.

[2025-05-09 09:38] Tommyrexx: [replying to mrexodia: "Thanks! We need an easy-to-compile alternative to ..."]
The library is very much WIP right now though as I am a student so juggling this with university work 😅

[2025-05-09 09:39] mrexodia: Still, nice to see people just do things!

[2025-05-09 09:39] Tommyrexx: 😄

[2025-05-09 20:07] pinefin: `static_cast<void*>(const_cast<char*>("Hello World"))` i hate my life

[2025-05-09 20:31] Tommyrexx: [replying to pinefin: "`static_cast<void*>(const_cast<char*>("Hello World..."]
Another day in C++ land

[2025-05-09 20:31] pinefin: [replying to Tommyrexx: "Another day in C++ land"]
<:yea:904521533727342632>

[2025-05-09 20:39] the horse: [replying to pinefin: "`static_cast<void*>(const_cast<char*>("Hello World..."]
why not just have a const void* <a:trollcat:873064659190702140>

[2025-05-09 20:41] pinefin: [replying to the horse: "why not just have a const void* <a:trollcat:873064..."]
cause function takes void*

[2025-05-09 20:41] pinefin: and if i pass in const it get mad at me because it think it changer the void*

[2025-05-09 20:43] pinefin: its a blackbox function in silabs sdk

[2025-05-09 20:43] the horse: makes sense 😔

[2025-05-09 20:43] the horse: i thought you can just pass a char* to it though

[2025-05-09 20:44] the horse: an implicit cast should happen

[2025-05-09 20:44] pinefin: i probably could yeah

[2025-05-09 20:44] pinefin: yeah i could

[2025-05-09 20:44] pinefin: rofl

[2025-05-09 20:44] pinefin: cast abuse though 😦

[2025-05-09 20:44] the horse: my_fn(const_cast<char*>("hello my brother in christ"));

[2025-05-09 20:45] the horse: [replying to pinefin: "cast abuse though 😦"]
sometimes it's just too much.

[2025-05-09 20:45] pinefin: [replying to the horse: "sometimes it's just too much."]
well sometimes i want something with substance in my life

[2025-05-09 20:46] pinefin: cast abuse is substance

[2025-05-09 20:46] pinefin: just like `3[array]`

[2025-05-09 20:46] the horse: 3[array] WHAT

[2025-05-09 20:46] pinefin: ```
const char* array = "hello world!";
printf("%c\n", 3[array]);
```

[2025-05-09 20:46] pinefin: thats valid syntax

[2025-05-09 20:46] the horse: no it cant be

[2025-05-09 20:47] the horse: you can't have a variable starting with a number

[2025-05-09 20:47] pinefin: try it

[2025-05-09 20:47] the horse: dude what

[2025-05-09 20:47] pinefin: yep

[2025-05-09 20:47] pinefin: think about it as

[2025-05-09 20:47] pinefin: *(3 + array)

[2025-05-09 20:47] Redhpm: [replying to the horse: "you can't have a variable starting with a number"]
3 is not a variable here, thats the raw pointer 3, with an offset of `array`

[2025-05-09 20:48] the horse: oh yeah so an implicit cast happens

[2025-05-09 20:48] the horse: day ruined

[2025-05-09 20:48] pinefin: its insane

[2025-05-09 20:48] pinefin: 🤣

[2025-05-09 20:48] the horse: thank you so fucking much pinefin

[2025-05-09 20:48] the horse: i hate you

[2025-05-09 20:48] pinefin: love u too buddy

[2025-05-09 20:48] the horse: <@148095953742725120> get your ass over here and help me hate

[2025-05-09 20:48] pinefin: 🤣

[2025-05-09 20:49] the horse: honestly still doesn't overcome daax's code golf

[2025-05-09 20:49] the horse: for peb

[2025-05-09 20:49] the horse: although

[2025-05-09 20:49] the horse: its very close.

[2025-05-09 20:51] pinefin: i havent seen that yet

[2025-05-09 20:51] the horse: 
[Attachments: image.png]

[2025-05-09 20:51] the horse: i'm off to avx grave

[2025-05-09 20:52] the horse: bil gats, i hate you.

[2025-05-09 20:53] pinefin: did you know about the comma operator

[2025-05-09 20:54] the horse: PINEFIN PLEASE

[2025-05-09 20:54] Matti: [replying to the horse: "<@148095953742725120> get your ass over here and h..."]
have to say I'm kinda surprised you didn't know this

[2025-05-09 20:54] Matti: it's a famous example of how terrible C is

[2025-05-09 20:54] the horse: you win pinefin

[2025-05-09 20:55] the horse: [replying to Matti: "it's a famous example of how terrible C is"]
i try to not type terrible code 😔

[2025-05-09 20:55] pinefin: ```c++
struct Weird {
  int operator,(int rhs) { return rhs * 2; }
} w;
int y = (w , 5);     // calls w.operator,(5) → 10
```

[2025-05-09 20:55] pinefin: 🙂

[2025-05-09 20:55] the horse: honestly

[2025-05-09 20:55] the horse: that can be quite useful

[2025-05-09 20:55] pinefin: yeah i agree

[2025-05-09 20:55] pinefin: i use it on some stuff in embedded

[2025-05-09 20:56] the horse: I could make probably use that in my static codec stuff

[2025-05-09 20:56] the horse: , decrypt
. encrypt

[2025-05-09 20:57] pinefin: oh hey bobby

[2025-05-09 20:57] pinefin: in some compilers

[2025-05-09 20:57] pinefin: i forgot about this

[2025-05-09 20:57] pinefin: but a coworker tried to use it recently

[2025-05-09 20:57] pinefin: and i got super pissed at him for it

[2025-05-09 20:57] pinefin: you ever used python?

[2025-05-09 20:57] the horse: unfortunately yes

[2025-05-09 20:57] pinefin: alright

[2025-05-09 20:57] pinefin: so

[2025-05-09 20:57] pinefin: you know how instead of

[2025-05-09 20:57] pinefin: && and ||

[2025-05-09 20:57] pinefin: they use and & or

[2025-05-09 20:57] the horse: ye

[2025-05-09 20:57] pinefin: you can do that in c++

[2025-05-09 20:57] pinefin: in some compilers

[2025-05-09 20:57] pinefin: its a preprocessor macro

[2025-05-09 20:58] pinefin: `#define and &&`

[2025-05-09 20:58] the horse: and is reserved in other right though?

[2025-05-09 20:58] the horse: and or xor

[2025-05-09 20:58] the horse: you can do int a = 7 xor 5

[2025-05-09 20:59] the horse: MSVC has it, I think clang might as well?

[2025-05-09 20:59] pinefin: [replying to the horse: "MSVC has it, I think clang might as well?"]
yep im using clang and it has support

[2025-05-09 20:59] the horse: i kind of understand your coworker's motivation

[2025-05-09 20:59] the horse: as someone who fucked with lua quite a bit

[2025-05-09 21:00] pinefin: 
[Attachments: Screenshot_2025-05-09_at_4.00.08_PM.png]

[2025-05-09 21:00] pinefin: [replying to the horse: "i kind of understand your coworker's motivation"]
god fuck no i hate it

[2025-05-09 21:00] the horse: i didn't say i like it

[2025-05-09 21:00] pinefin: no he did it to be different

[2025-05-09 21:00] pinefin: our whole code base is && and ||

[2025-05-09 21:00] pinefin: its hard to read when you're used to the originals

[2025-05-09 21:01] pinefin: anyways thats all the weird quirks i have for now

[2025-05-09 21:01] the horse: (a & b & c) != 0 !!!!

[2025-05-09 21:01] pinefin: he also liked to do mass inheritance

[2025-05-09 21:01] pinefin: which i got mad at as well

[2025-05-09 21:02] pinefin: [replying to the horse: "(a & b & c) != 0 !!!!"]
<:flush:705499004560998530>

[2025-05-09 21:02] the horse: [replying to pinefin: "he also liked to do mass inheritance"]
virtual?

[2025-05-09 21:02] pinefin: [replying to the horse: "virtual?"]
nope

[2025-05-09 21:02] the horse: or just class on class

[2025-05-09 21:02] pinefin: just mass inheritance

[2025-05-09 21:02] pinefin: class on class

[2025-05-09 21:02] the horse: eh fair, I have a lot of traits

[2025-05-09 21:02] pinefin: no its just annoying seeing

[2025-05-09 21:02] the horse: for RAII, constructor overrides, ...

[2025-05-09 21:02] pinefin: Thread::start()

[2025-05-09 21:03] pinefin: CANBUS::listen();

[2025-05-09 21:03] pinefin: god

[2025-05-09 21:03] pinefin: no

[2025-05-09 21:03] the horse: [replying to pinefin: "Thread::start()"]
okay now this is just too much

[2025-05-09 21:03] the horse: std::jthread has fine syntax

[2025-05-09 21:03] pinefin: [replying to the horse: "okay now this is just too much"]
yeah

[2025-05-09 21:03] pinefin: [replying to the horse: "std::jthread has fine syntax"]
we dont use std::thread since its freertos

[2025-05-09 21:03] pinefin: afaik its not implemented

[2025-05-09 21:03] the horse: jthread (new)

[2025-05-09 21:03] pinefin: oh

[2025-05-09 21:03] pinefin: jthread

[2025-05-09 21:03] pinefin: what is this

[2025-05-09 21:03] the horse: which is what you want in 99% of cases

[2025-05-09 21:04] the horse: it's a std::thread reimpl to fix many outstanding bugs

[2025-05-09 21:04] pinefin: oh thats neat

[2025-05-09 21:04] the horse: joining causing problems etc

[2025-05-09 21:04] the horse: they can't fix the original up due to backwards compatibility

[2025-05-09 21:04] the horse: honestly i'm hyped for C++26>

[2025-05-09 21:05] pinefin: is reflection making it in?

[2025-05-09 21:05] pinefin: or is that just

[2025-05-09 21:05] pinefin: speculated

[2025-05-09 21:05] the horse: probably not

[2025-05-09 21:05] the horse: there's openmp though

[2025-05-09 21:05] the horse: wait no

[2025-05-09 21:05] the horse: uhh dunno who made it

[2025-05-09 21:05] the horse: bunch of open source libs, very nice

[2025-05-09 21:05] pinefin: i can use it on godbolt

[2025-05-09 21:05] pinefin: under clang experimental

[2025-05-09 21:05] pinefin: with a pr attached

[2025-05-09 21:06] pinefin: 
[Attachments: Screenshot_2025-05-09_at_4.06.15_PM.png]

[2025-05-09 21:06] pinefin: https://godbolt.org/z/4PMEPKTYE
[Embed: Compiler Explorer - C++ (x86-64 clang (experimental P2996))]
// start 'expand' definition
namespace __impl {
  template&lt;auto... vals&gt;
  struct replicator_type {
    template&lt;typename F&gt;
      constexpr void operator&gt;&gt;(F body) const {
        (

[2025-05-09 21:08] the horse: https://github.com/qlibs
[Embed: Qlibs++]
C++ libraries. Qlibs++ has 14 repositories available. Follow their code on GitHub.

[2025-05-09 21:08] the horse: this guy

[2025-05-09 21:08] the horse: https://github.com/qlibs/jmp
https://github.com/qlibs/reflect
[Embed: GitHub - qlibs/jmp: C++20 Static Branch library]
C++20 Static Branch library. Contribute to qlibs/jmp development by creating an account on GitHub.
[Embed: GitHub - qlibs/reflect: C++20 Static Reflection library]
C++20 Static Reflection library. Contribute to qlibs/reflect development by creating an account on GitHub.

[2025-05-09 21:09] pinefin: ah yes

[2025-05-09 21:10] pinefin: `if (not jmp::init())`

[2025-05-09 21:10] pinefin: oh no.

[2025-05-09 21:10] pinefin: oh no.

[2025-05-09 21:10] pinefin: he unironically using it.

[2025-05-09 21:11] pinefin: oh no.

[2025-05-09 21:11] the horse: LMAOI

[2025-05-09 21:25] the horse: https://learn.microsoft.com/en-us/previous-versions/visualstudio/visual-studio-2013/hh388953(v=vs.120)?redirectedfrom=MSDN
[Embed: restrict (C++ AMP)]

[2025-05-09 21:25] the horse: cpu is a reserved keyword????

[2025-05-09 22:24] vendor: [replying to pinefin: "`if (not jmp::init())`"]
??????????????

[2025-05-09 22:25] vendor: please

[2025-05-09 22:25] vendor: tell me this is a macro

[2025-05-09 22:25] vendor: and not a fucking c++80 “feature”

[2025-05-09 22:27] pinefin: [replying to vendor: "and not a fucking c++80 “feature”"]
its a preprocessor

[2025-05-09 22:27] pinefin: afaik

[2025-05-09 22:27] pinefin: its not a new feature tho, its been here since the dawn of days

[2025-05-09 22:27] pinefin: c++98 is when it was added

[2025-05-10 06:27] Timmy: ima have to look at how mans getting the aggregate types member variable names.

[2025-05-10 06:27] Timmy: thatd be super useful

[2025-05-11 14:53] Yoran: <@1085173695850487900> Thanks for telling me to move here.
First of all, AVX512 is nice but different CPUs have different subsets of it. So that's a thing to take in mind (assuming you want to do something that is specific to one or more subsets).
Also, In the zen4 they are using two 256-bit exec units simultaneously. Not "*actually*" avx512. And there are more differences down the ladder. So perf can vary between different chips with avx512 and zen4s with avx512.

[2025-05-11 14:53] Yoran: I don't know if that is going to be crucial or not (prob not). But its something to keep in mind

[2025-05-11 14:54] Tommyrexx: It will be crucial imo. Because microarchitectural differences impact performance.

[2025-05-11 14:54] Yoran: [replying to Tommyrexx: "It will be crucial imo. Because microarchitectural..."]
So thats why i said "kinda"

[2025-05-11 14:55] Yoran: In any case, for zen4- checkout the optimization manual. For intel checkout the SDM

[2025-05-11 14:55] Tommyrexx: IIRC the AVX512 in Zen 4 has all the extensions except one

[2025-05-11 14:56] Yoran: [replying to Tommyrexx: "IIRC the AVX512 in Zen 4 has all the extensions ex..."]
zen4 avx512 is very based

[2025-05-11 14:56] Tommyrexx: Yeh

[2025-05-11 14:58] Tommyrexx: I'm going to buy a Thinkpad

[2025-05-11 15:00] Yoran: [replying to Tommyrexx: "I'm going to buy a Thinkpad"]
What is the library you are building?

[2025-05-11 15:01] Tommyrexx: It's a library similar to libgmp

[2025-05-11 15:01] Yoran: [replying to Tommyrexx: "It's a library similar to libgmp"]
So codec ig?

[2025-05-11 15:01] Tommyrexx: for fast multi-precision arithmetic

[2025-05-11 15:01] Tommyrexx: so you can work on numbers greater than word size

[2025-05-11 15:01] Tommyrexx: Right now I can multiply 9600 hexadecimal digit numbers

[2025-05-11 15:01] Yoran: [replying to Tommyrexx: "so you can work on numbers greater than word size"]
How it is going so far? any data on avx2 or omething?

[2025-05-11 15:02] Yoran: compared to like other libs

[2025-05-11 15:02] Yoran: [replying to Tommyrexx: "Right now I can multiply 9600 hexadecimal digit nu..."]
nice

[2025-05-11 15:04] Tommyrexx: [replying to Yoran: "How it is going so far? any data on avx2 or omethi..."]
a lot of the algorithms don't use sse, avx or avx512

[2025-05-11 15:04] Tommyrexx: Some do, like the function that performs two's complement of a big int, or one that copies it to another or sets all digits to one value

[2025-05-11 15:05] Tommyrexx: I didn't really have to write assembly for the SIMD based functions yet, that's because the compiler output is good

[2025-05-11 15:08] Yoran: [replying to Tommyrexx: "I didn't really have to write assembly for the SIM..."]
I guess in this library perf is very very crucial. So you really DO want to benchmark this. I know in the codec worlds the different between **intrinsics** and linked assembly is somewhere around 5-15% precent. So i can only assume youd get a bunch of perf for writing the assembly by hand

[2025-05-11 15:08] Tommyrexx: But I did write assembly for the addition, subtraction and multiplication routines

[2025-05-11 15:09] Tommyrexx: [replying to Yoran: "I guess in this library perf is very very crucial...."]
Yep performance is king

[2025-05-11 15:10] Tommyrexx: These routines aren't really the bottlenecks in most computation

[2025-05-11 15:11] Tommyrexx: By far, the most critical routine is multiplication

[2025-05-11 15:12] Tommyrexx: Low level multiplication which does little apart from the actual computation

[2025-05-11 15:15] Tommyrexx: Because almost every other performance critical routine depends on it