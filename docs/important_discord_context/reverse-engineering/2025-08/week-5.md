# August 2025 - Week 5
# Channel: #reverse-engineering
# Messages: 283

[2025-08-25 05:25] Ignotus: [replying to toasts: "you could try kaspersky’s hrtng plugin"]
Not exactly what im looking for though can still be used for it and is useful. Basically looking for plugin that checks for known function calls eg. GetModuleHandleA and renames the parameters, since they are known (lpModuleName)

[2025-08-26 07:23] cd: was reading this article: https://www.cyberark.com/resources/threat-research-blog/c4-bomb-blowing-up-chromes-appbound-cookie-encryption. In padding oracle attack section, it says:
> However, Google added one more step for their Chrome browser called post processing (which is not part of the general Chromium project). We won’t go into full detail here, but in short, there’s some encrypting of the content returned by the CryptUnprotectData with a hardcoded key, followed by some XORs. The result is the cookie key, which we used to decrypt the cookies:
they didn't include much information there about it, so I decided to dig a little bit and found this: https://github.com/runassu/chrome_v20_decryption - it has the hardcoded key in there but it doesn't mention any steps or procedure to find the key). i wonder if anyone knows on how to locate this key?
[Embed: GitHub - runassu/chrome_v20_decryption: Chrome COOKIE v20 decryptio...]
Chrome COOKIE v20 decryption PoC. Contribute to runassu/chrome_v20_decryption development by creating an account on GitHub.

[2025-08-26 09:12] dullard: [replying to cd: "was reading this article: https://www.cyberark.com..."]
Knowing chrome it’s probably stored in DPAPI

[2025-08-26 12:35] abu: Hello! Does anyone have any articles on internal workings of MSVC?

[2025-08-26 13:22] dinero: Like c1.dll, c2.dll etc?

[2025-08-26 13:23] dinero: https://lectem.github.io/msvc/reverse-engineering/build/2019/01/21/MSVC-hidden-flags.html#:~:text=c1.,4).

https://www.geoffchappell.com/studies/msvc/cl/c2/options/index.htm
[Embed: MSVC hidden flags]
I had seen a while ago the “hidden” MSVC flags that output interesting information such as /d2cgsummary or d1reportAllClassLayout.

[2025-08-26 13:24] dinero: might be somewhat relevant if you just want to know flags

[2025-08-26 13:27] dinero: Is there anything specific you’re looking for

[2025-08-26 13:40] Brit: no. I need you to explain how the compiler works, from lexing to producing the final bin

[2025-08-26 13:57] dinero: enjoy sir
[Attachments: Modern_Compiler_Implementation_in_C.pdf]

[2025-08-26 15:47] abu: [replying to dinero: "https://lectem.github.io/msvc/reverse-engineering/..."]
Thanks 🙂

[2025-08-26 15:48] abu: [replying to dinero: "Is there anything specific you’re looking for"]
Looking for something on their SSA IR and passes they implement

[2025-08-26 19:26] XE: [replying to abu: "Looking for something on their SSA IR and passes t..."]
Here's a good book on SSA formation: https://pfalcon.github.io/ssabook/latest/book-full.pdf

If you're working with a Register allocation (specifically linear scan) this also a good resource: https://bernsteinbear.com/blog/linear-scan/

I really recommend reading chapter 9 of "Engineer A Compiler", which mostly talks about SSA formations, along with dominance, dominance frontiers, immediate doms, dom trees, Phi insertion, rename passes for the SSA, etc. It's a really good book over tho
[Embed: Linear scan register allocation on SSA]
A linear scan through the history of… linear scan register allocation.

[2025-08-26 19:29] XE: also if you do plan on learning reg allocation, such as linear scan or graph coloring, there's some really nice chapters in "Engineering a Compiler"

[2025-08-26 19:32] XE: Also for pass implementation, they should also discuss that in the book too

[2025-08-26 19:50] ra.mn: am i cheating if im patching the binary

[2025-08-26 19:50] ra.mn: like i dont see rules saying not to

[2025-08-26 19:51] ra.mn: context is ctfs

[2025-08-26 19:53] valium: [replying to ra.mn: "context is ctfs"]
if u get the flag then it doesnt matter imo

[2025-08-26 19:58] dinero: ask the organizers if you’re unsure

[2025-08-26 19:58] dinero: And if it’s not live who cares

[2025-08-26 20:20] ra.mn: Fair

[2025-08-26 23:29] felps: help me
[Attachments: image.png]

[2025-08-27 02:59] cd: [replying to dullard: "Knowing chrome it’s probably stored in DPAPI"]
No?

[2025-08-27 03:00] cd: Read anything tht i've sent you?

[2025-08-27 03:00] cd: I'm looking to find the hardcoded key

[2025-08-27 03:04] cd: 1. get the APPB encrypted key
2. CryptUnprotectData from SYSTEM context
3. CryptUnprotectData from user context
4. Remove the validation data from output 
5. Decrypt the output with hardcoded key to get the master key (aes256-gcm)

[2025-08-27 10:29] ra.mn: ```x86asm
00401118  mov     edx, 0x17
0040111d  lea     rsi, [rel data_401101]  {"your encouraging word:"}
00401124  mov     eax, 0x1
00401129  mov     edi, 0x1
0040112e  syscall 
00401130  mov     eax, 0x0
00401135  mov     edi, 0x0
0040113a  mov     rsi, data_402000
00401144  mov     edx, 0x9
00401149  syscall 
0040114b  retn    
```

[2025-08-27 10:30] ra.mn: the program asks me for the word and then if i input anything i get this message: 
```
your encouraging word: l
[1]    4027 segmentation fault (core dumped)  ./jump
```

[2025-08-27 10:30] ra.mn: how can i find informations about `data_402000`

[2025-08-27 11:00] valium: [replying to ra.mn: "```x86asm
00401118  mov     edx, 0x17
0040111d  le..."]
the second syscall is read, and rsi holds the address of the buffer where the read data will be stored

[2025-08-27 11:01] valium: you also dont setup the stack

[2025-08-27 11:02] valium: and you just return

[2025-08-27 11:02] valium: is this a hand written asm program?

[2025-08-27 11:03] ra.mn: [replying to valium: "is this a hand written asm program?"]
It's a crackme I just have the binary

[2025-08-27 11:03] valium: pretty its crashing cause u straight up return, you should do an exit syscall

[2025-08-27 11:04] ra.mn: You cant return when you don't setup the stack?

[2025-08-27 11:04] ra.mn: I'll try to patch it that way and see

[2025-08-27 11:04] valium: [replying to ra.mn: "You cant return when you don't setup the stack?"]
u can but what does this do?

[2025-08-27 11:04] valium: is this a function?

[2025-08-27 11:04] valium: can u send the crackme?

[2025-08-27 11:04] ra.mn: Sure

[2025-08-27 11:06] ra.mn: [replying to valium: "can u send the crackme?"]
https://crackmes.one/crackme/6869287daadb6eeafb398fec
[Embed: toasterbirb's jump]
That illegal instruction minefield looks a bit sketch. Can you jump into the middle of it without having anything blow up?

Figure out a working password

[2025-08-27 11:07] ra.mn: [replying to valium: "is this a function?"]
It's just a bunch of syscalls

[2025-08-27 11:31] Brit: You need to read the actual code that happens before, it sets up a very funny stack for itself

[2025-08-27 11:31] Brit: which means the return after the sys read takes it on a bit of a journey

[2025-08-27 11:32] Brit: I have given you enough clues to solve this

[2025-08-27 11:46] Brit: https://cyber.autis.moe/MUQIgaHI12.png

[2025-08-27 11:46] Brit: took 10 ish mins to solve

[2025-08-27 13:26] x86matthew: [replying to Brit: "took 10 ish mins to solve"]
am i missing something or ||does this only check one character||?

[2025-08-27 13:26] x86matthew: looks too easy but i'm on windows so i can't test it

[2025-08-27 13:32] Brit: yee

[2025-08-27 13:32] Brit: idk how it got a difficulty of 2.2

[2025-08-27 13:34] x86matthew: yeah weird lol

[2025-08-27 13:36] x86matthew: funnily enough if i could've run it i probably would've solved it by accident immediately

[2025-08-27 13:36] x86matthew: before looking at it

[2025-08-27 13:36] x86matthew: using a common "test word" that one might use for something like this

[2025-08-27 13:36] Brit: <:topkek:904522829616263178>

[2025-08-27 13:41] ra.mn: i still have no clue how the binary works <:waitstare:1295829770990387361>

[2025-08-27 13:42] x86matthew: [replying to Brit: "<:topkek:904522829616263178>"]
on second thoughts it's even worse than i thought

[2025-08-27 13:42] x86matthew: as there are 2 nops before the dest lol

[2025-08-27 13:42] Brit: yes

[2025-08-27 13:42] Brit: actually

[2025-08-27 13:42] Brit: <:kekw:904522300257345566>

[2025-08-27 13:43] Brit: [replying to ra.mn: "i still have no clue how the binary works <:waitst..."]
okay so you have disassembly of this right?

[2025-08-27 13:43] ra.mn: yep

[2025-08-27 13:43] ra.mn: the line that makes no sens is this: 
```x86asm
  mov     rsp, 0x401365
```

[2025-08-27 13:44] Brit: we set rsp to that value

[2025-08-27 13:44] ra.mn: i thought you need to push rbp,rsp to the stack first

[2025-08-27 13:44] Brit: what is rsp on x86?

[2025-08-27 13:44] ra.mn: stack pointer

[2025-08-27 13:44] Brit: okay so, this tells you something very important right?

[2025-08-27 13:45] ra.mn: im not familiar to moving stuff to it i.e usually just `sub` it for local variables

[2025-08-27 13:45] Brit: okay forget all that, what is the stack used for

[2025-08-27 13:45] Brit: specifically for control flow

[2025-08-27 13:46] ra.mn: to store function variables , basically manage the function

[2025-08-27 13:46] Brit: no

[2025-08-27 13:46] ra.mn: or the running code

[2025-08-27 13:46] Brit: what gets pushed to the stack

[2025-08-27 13:46] Brit: when you do a call instr

[2025-08-27 13:46] Brit: and why is that important

[2025-08-27 13:46] ra.mn: [replying to Brit: "when you do a call instr"]
instr?

[2025-08-27 13:46] Brit: yeah

[2025-08-27 13:46] valium: instruction

[2025-08-27 13:46] Brit: https://www.felixcloutier.com/x86/call

[2025-08-27 13:46] Brit: this

[2025-08-27 13:46] Brit: what happens to the stack

[2025-08-27 13:46] Brit: when this instruction happens

[2025-08-27 13:48] Brit: <@1243243131458162748> ill walk you through it but you actually have to put in the effort of understanding otherwise im just wasting time typing it out

[2025-08-27 13:49] ra.mn: i'd read the article and if i have further questions i'll ask here , thx

[2025-08-27 13:50] Brit: nah, I'm trying to get you to understand

[2025-08-27 13:50] Brit: what the stack is used for

[2025-08-27 13:50] Brit: this should be simple

[2025-08-27 13:50] Brit: how does the cpu know

[2025-08-27 13:50] Brit: where to go

[2025-08-27 13:50] Brit: after a procedure that has been called

[2025-08-27 13:50] Brit: returns

[2025-08-27 13:50] Brit: Think about this please

[2025-08-27 13:51] Brit: I'm not calling you out

[2025-08-27 13:51] Brit: Im trying to help

[2025-08-27 13:53] valium: ooh nice

[2025-08-27 13:53] Brit: I doubt it makes too too much difference here, Im just trying to get him to think about how calls and returns pair up on an x86 machine

[2025-08-27 13:54] Brit: if he reads the instr description it should activate the neuron

[2025-08-27 13:54] avx: ohhhh neat

[2025-08-27 13:55] Brit: although I agree chunking up the intel manual was a great idea

[2025-08-27 14:01] daax: [replying to Brit: "https://www.felixcloutier.com/x86/call"]
https://revers.engineering/x86/call.pdf

[2025-08-27 14:01] daax: better <:Kappa:794707301436358686>

[2025-08-27 14:01] Brit: yes yes

[2025-08-27 14:01] Brit: I get it

[2025-08-27 14:01] daax: oh

[2025-08-27 14:01] Brit: <:kekw:904522300257345566>

[2025-08-27 14:01] daax: I see

[2025-08-27 14:01] daax: didn’t see that message

[2025-08-27 14:01] Brit: https://tenor.com/view/hades-i-know-i-get-the-concept-gif-13406522

[2025-08-27 14:02] daax: [replying to Brit: "if he reads the instr description it should activa..."]

[Attachments: IMG_1738.jpg]

[2025-08-27 14:02] avx: mmm if only

[2025-08-27 14:02] avx: i could return

[2025-08-27 14:02] Brit: impossible

[2025-08-27 14:10] ra.mn: [replying to Brit: "Im trying to help"]
sorry i wasnt around , so from what i understand `rsp` now is having that address at the top of the stack

[2025-08-27 14:11] Brit: first

[2025-08-27 14:11] Brit: before we think about that

[2025-08-27 14:11] Brit: we must understand what both call and return do

[2025-08-27 14:11] Brit: on x86

[2025-08-27 14:11] Brit: then we shall think about what moving that addr into rsp does

[2025-08-27 14:12] Brit: so I ask again, how does the cpu know where to return if it hits a return instr

[2025-08-27 14:12] Brit: [replying to ra.mn: "sorry i wasnt around , so from what i understand `..."]
?

[2025-08-27 14:13] ra.mn: [replying to Brit: "so I ask again, how does the cpu know where to ret..."]
it just stores the caller address on the stack ?

[2025-08-27 14:13] Brit: yes

[2025-08-27 14:13] Brit: great

[2025-08-27 14:14] Brit: so call pushes a value to the stack and ret pops one off and sets the ip to that yes?

[2025-08-27 14:14] ra.mn: [replying to Brit: "so call pushes a value to the stack and ret pops o..."]
yep

[2025-08-27 14:15] ra.mn: i thought ret just pushes too

[2025-08-27 14:15] Brit: no

[2025-08-27 14:16] Brit: ret pops

[2025-08-27 14:16] Brit: call pushes

[2025-08-27 14:16] ra.mn: i see

[2025-08-27 14:16] Brit: obviously

[2025-08-27 14:16] Brit: how else would that work

[2025-08-27 14:16] Brit: what would ret push?

[2025-08-27 14:16] ra.mn: the ip

[2025-08-27 14:16] ra.mn: the address

[2025-08-27 14:17] Brit: but why?

[2025-08-27 14:17] Brit: what would that accomplish?

[2025-08-27 14:18] Brit: anyway, the cpu knows where to return on a ret because the addr is on stack and it gets popped off

[2025-08-27 14:18] Brit: so that the next ret can return to the next function on the callstack

[2025-08-27 14:18] Brit: etc

[2025-08-27 14:18] Brit: anyway as you pointed out earlier rsp is the stack pointer

[2025-08-27 14:18] ra.mn: aha

[2025-08-27 14:19] Brit: this means that mov rsp, 0x12345
does not mean that the top of the stack will be 0x12345, but rather that's where the stack is

[2025-08-27 14:19] Brit: a pointer to the stack

[2025-08-27 14:19] Brit: so now armed with this knowledge we can go back to our diassembly

[2025-08-27 14:19] Brit: and take a peek at what's at 0x401365

[2025-08-27 14:20] ra.mn: [replying to Brit: "and take a peek at what's at 0x401365"]
it just an address

[2025-08-27 14:20] Brit: yes

[2025-08-27 14:20] Brit: but what's there

[2025-08-27 14:20] Brit: in the binary

[2025-08-27 14:21] Brit: (assuming you use a tool that maps the binary as if it had been loaded at its preffered addr, but all the tools you are likely to use do this)

[2025-08-27 14:21] ra.mn: [replying to Brit: "but what's there"]
``\x0b\x0f\x0b\x0f\x0b<\x13@\x00\x00\x00\x00\x00\x07\x10@\x00\x00\x00\x00\x007\x13@\x00\x00\x00\x00\x00\x00\x10@``

[2025-08-27 14:22] Brit: oh jesus please use a hex view

[2025-08-27 14:22] Brit: and not this ascii mapping

[2025-08-27 14:23] ra.mn: ok

[2025-08-27 14:23] ra.mn: 
[Attachments: image.png]

[2025-08-27 14:23] Brit: does this not look suspiciously like something to you?

[2025-08-27 14:24] Brit: I'll help

[2025-08-27 14:24] Brit: its an array of pointers

[2025-08-27 14:24] Brit: const uint64_t data[5] = 
{
    0x000000000040133c, 0x0000000000401007,
    0x0000000000401337, 0x0000000000401000,
    0x000000000040114c
};

[2025-08-27 14:24] Brit: its 5 little endian pointers

[2025-08-27 14:25] Brit: we obviously can infer this because the crackme sets up it's own stack

[2025-08-27 14:25] Brit: which is sus

[2025-08-27 14:25] Brit: so we can assume its going to be pointers

[2025-08-27 14:25] Brit: because nothing else touches the stack

[2025-08-27 14:25] Brit: so the only reason they'd do this

[2025-08-27 14:25] Brit: is in order to ROP themselves

[2025-08-27 14:26] ra.mn: interesting

[2025-08-27 14:26] Brit: so what have we learned

[2025-08-27 14:26] Brit: what will happen when we hit our first ret

[2025-08-27 14:27] Brit: the one at 0x40114b ?

[2025-08-27 14:28] Brit: where will we land given our specially crafted stack

[2025-08-28 20:23] W4ZM: i have a question

[2025-08-28 20:35] W4ZM: why when we define an array in a function for example like this :

```
const UCHAR bytes[] = {
      0x48, 0x89, 0x74, 0x24, 0x00, 0x57, 0x48, 0x81,
      0xEC, 0x00, 0x00, 0x00, 0x00, 0x49, 0x8B, 0xF0,
      0x00,
   };
```
when debugging it appears like this in the text section : 

```
00007FF68E635C03 | C745 D8 48897424       | mov dword ptr ss:[rbp-28],24748948             |
00007FF68E635C0A | C745 DC 00574881       | mov dword ptr ss:[rbp-24],81485700             |
00007FF68E635C11 | C745 E0 EC000000       | mov dword ptr ss:[rbp-20],EC                   |
00007FF68E635C18 | C745 E4 00498BF0       | mov dword ptr ss:[rbp-1C],F08B4900             |
00007FF68E635C1F | C645 E8 00             | mov byte ptr ss:[rbp-18],0                     |
```

why it doesnt get stored in the .rdata section like this one (also inside a function): 

```
const char bytes[] = "H\x89t$\0WH\x81\xEC\0\0\0\0I\x8B\xF0";
```
this will be stored in .rdata section like this : 

```
48 89 74 24 00 57 48 81 EC 00 00 00 00 49 8B F0 00 ...
```

[2025-08-28 20:39] W4ZM: in both debug and release version , the case is the same

[2025-08-28 20:47] truckdad: string literals have static storage, while a local variable does not, even if it is declared `const`

[2025-08-28 21:02] Xits: gcc puts this in registers for me
```
const char bytes[] = "H\x89t$\0WH\x81\xEC\0\0\0\0I\x8B\xF0";
```

[2025-08-28 21:04] Xits: and the other one.

[2025-08-28 21:05] Xits: I wonder why they don't do it for local const string literals though

[2025-08-28 21:06] Xits: I guess if you're passing the pointer around its impossible

[2025-08-28 21:07] Xits: but why not use registers here? https://godbolt.org/z/oq9xza5fz
[Embed: Compiler Explorer - C++ (x86-64 gcc 15.2)]
int main2(){
    const char bytes[] = "H\x89t$\0WH\x81\xEC\0\0\0\0I\x8B\xF0";
    const char* test = "hello world";
    auto test2 = 0u;
    for(auto i = 0; i &lt; sizeof(bytes); i++){
        test2
 

[2025-08-28 21:09] Xits: I guess because the code is wrong lol nvm

[2025-08-28 21:23] Xits: can anyone explain this?
https://godbolt.org/z/shbbb8Pbn
[Embed: Compiler Explorer - C++ (x86-64 gcc 15.2)]
int main2(int arg){
    const char test[] = "hello world";
    return test[arg % sizeof(test)];
}

int main3(int arg){
    const char* test = "hello world";
    return test[arg % sizeof(test)];
}

[2025-08-28 21:23] Xits: it inlines
```
const char test[] = "hello world";
```
but not
```
 const char* test = "hello world";
```

[2025-08-28 21:31] W4ZM: [replying to truckdad: "string literals have static storage, while a local..."]
true,  when i declare the array as static or outside the function (global) it will be stored at .rdata

[2025-08-28 23:22] lom: [replying to Xits: "it inlines
```
const char test[] = "hello world";
..."]
`const char test[] = "hello world";` -> char array 
` const char* test = "hello world";` -> pointer to array of char 
placed into rodata because attempts  to modify it  vias the `test` pointer are undefined behavior according to c99

[2025-08-28 23:49] Matti: [replying to Xits: "it inlines
```
const char test[] = "hello world";
..."]
1. see above, pointer decay prevents this, ~~unless you force the linker to use readonly segments via compiler extensions maybe (but then you cannot use a local variable)~~
2. this isn't really a reason, but a bug in your code: `main2()` does not do the same thing as `main3()`. why? consider pointer decay again, and what `sizeof(test)` evaluates to in both functions. `sizeof(*test)` also does not work because this gives you the size of one element
<https://godbolt.org/z/xfEevdj3r> gives the correct result (but still does not inline the string)
3. I'm not sure whether this counts as 'inlined' or not, but here's my best effort to confuse godbolt: <https://godbolt.org/z/jYT6Yqa4E>
(if you combine this with a pragma/linker hack to force this crap back into .text it might count I guess?)

[2025-08-28 23:53] Matti: scratch the 'unless' in (1), I already kind of forgot that your goal here is inlining strings, not placing them in a RO segment

[2025-08-28 23:54] Matti: I prefer to put data with data, and if it should be readonly then the data goes in a readonly segment <:thinknow:475800595110821888>

[2025-08-28 23:55] Matti: not that this objectively superior, especially for short strings inlining can make sense for performance reasons

[2025-08-28 23:59] diversenok: Matti, do you know anything about `TEB->IsImpersonating`? I thought you are supposed to set it when assigning an impersonation token, but then it causes crashes for me

[2025-08-29 00:00] diversenok: It crashes inside `NlsValidateLocale`, `GetLocaleInfoHelper`, etc., so NLS clearly doesn't like it

[2025-08-29 00:01] Matti: hum, no... can't say that I do off hand

[2025-08-29 00:02] Matti: I've certainly never used it, whether for impersonating or anything else

[2025-08-29 00:03] Matti: let me consult the holy ripgrep to see if I can find more

[2025-08-29 00:08] Matti: ah I think I've found it: user mode definitely can and does check this bit (as a quick way to save a syscall I guess? not sure I like this as an optimization but anyway)
but it is only ever set and cleared by the kernel

[2025-08-29 00:08] Matti: in the WRK this is in `PspWriteTebImpersonationInfo`

[2025-08-29 00:08] diversenok: `PsAssignImpersonationToken` used to set it but I don't think it does it anymore?

[2025-08-29 00:09] Matti: it's probably not called the same thing anymore for sure, that happens all the time

[2025-08-29 00:10] Matti: but I'm skeptical the assignment is not still done by the kernel somewhere

[2025-08-29 00:10] diversenok: Yeah, but purely from the observation perspective I don't see the value in TEB changing when I do a syscall that sets the token

[2025-08-29 00:11] Matti: ok well that does rule it out then lol

[2025-08-29 00:11] diversenok: And if I do it myself if crashes, lol

[2025-08-29 00:12] Matti: this is odd in two ways then, because like I said I already didn't think this looked like the best thing to save a syscall on by storing it in a user writeable place

[2025-08-29 00:12] diversenok: Although, wait... There is `PspWriteTebImpersonationInfo`

[2025-08-29 00:14] diversenok: Huh, I am so confused now. It does change it

[2025-08-29 00:15] Matti: [replying to diversenok: "Yeah, but purely from the observation perspective ..."]
which syscall precisely? can you show some context?
I'm not sure if this bit is *always* supposed to be set in the TEB or if this is intentionally not done/needed for your specific code path

[2025-08-29 00:15] Matti: that also seems weird and I would be inclined to say it'd be a bug, but then again I don't really know what the purpose of the bit is

[2025-08-29 00:16] Matti: the windows sources do have `#define RtlIsImpersonating() (NtCurrentTeb()->IsImpersonating ? TRUE : FALSE)` which is used somewhat often

[2025-08-29 00:17] Matti: but not as often as you'd expect given that this is all of windows 2003

[2025-08-29 00:17] Matti: it's only ntdll and kernel32 in specific places

[2025-08-29 00:17] diversenok: Okay, I need to reevaluate everything from the start, I cannot reproduce what I thought I saw

[2025-08-29 00:18] diversenok: It was a simple `NtSetInformationThread` with `ThreadImpersonationToken`

[2025-08-29 00:20] Matti: OK, I think that should set it, from a quick look at the WRK at least
ThreadImpersonationToken -> PsAssignImpersonationToken ->  PsImpersonateClient -> PspWriteTebImpersonationInfo

[2025-08-29 00:20] diversenok: It does, I just checked

[2025-08-29 00:21] diversenok: But what the heck did I do the last time

[2025-08-29 00:22] diversenok: It was working fine. Then I added code that manually sets `NtCurrentTeb()->IsImpersonating = !!hToken;` right after a call to  `NtSetInformationThread` and subsequent calls to task dialogs started crashing with exceptions in NLS functions that read `IsImpersonating`

[2025-08-29 00:22] diversenok: I reverted the change and it went back to normal

[2025-08-29 00:23] diversenok: But if I was setting `NtCurrentTeb()->IsImpersonating`  to 1 when it was already 1, how...

[2025-08-29 00:24] Matti: indeed <:wow:762710812904914945>

[2025-08-29 00:26] Matti: oho

[2025-08-29 00:27] Matti: [replying to diversenok: "It was working fine. Then I added code that manual..."]
```c
try {
    if (Impersonating) {
        Teb->ImpersonationLocale = (LCID)-1;
        Teb->IsImpersonating = 1;
    } else {
        Teb->ImpersonationLocale = (LCID) 0;
        Teb->IsImpersonating = 0;
    }
} except (EXCEPTION_EXECUTE_HANDLER) {
}
```
I feel like the other assignment may be responsible

[2025-08-29 00:27] Matti: the name does make it suspect here <:lillullmoa:475778601141403648>

[2025-08-29 00:28] diversenok: Also NLS uses value 2 for something in addition to 1

[2025-08-29 00:29] Matti: { 0, 0 } or { -1, 1 }

[2025-08-29 00:29] Matti: perfectly logical

[2025-08-29 00:30] diversenok: ```c
//
//  Possible NtCurrentTeb()->IsImpersonating values :
//
//  0  : Thread isn't impersonating any user.
//
//  1  : Thread has just started to do impersonation.
//       Per thread cache needs to be allocated now.
//
//  2  : Thread is calling the NLS apis while its
//       a context other than the interactive logged on user.
//
```

[2025-08-29 00:30] Matti: [replying to diversenok: "Also NLS uses value 2 for something in addition to..."]
yeah no doubt the true number of magic constants abused as 'locales' is unknown

[2025-08-29 00:30] Matti: [replying to diversenok: "```c
//
//  Possible NtCurrentTeb()->IsImpersonati..."]
I mean that is the list *now*

[2025-08-29 00:31] Matti: oh but wait, this is about IsImpersonating, not the locale one?

[2025-08-29 00:32] Matti: then 2 must be an addition from vista

[2025-08-29 00:38] diversenok: That's a commen from `NlsGetCurrentUserNlsInfo` in XP though ^

[2025-08-29 00:41] Matti: oh yeah, I see it

[2025-08-29 00:41] Matti: and the comment is even correct because the assignment is in this same file

[2025-08-29 00:42] Matti: the only place outside of the kernel

[2025-08-29 00:42] Matti: I dunno, this doesn't count to me

[2025-08-29 00:43] Matti: or alternatively it simply proves that the TEB bit is ??meaningless??

[2025-08-29 00:44] Matti: its interpretation is up for debate

[2025-08-29 00:45] Matti: I'm still just stuck wondering when you would ever want to know if a thread is impersonating or not, but not so much that it's a potential security risk to read this hack value from the TEB instead

[2025-08-29 00:47] diversenok: Yeah

[2025-08-29 00:51] diversenok: Okay, I'll have to revisit this later because I just can't seem to reproduce the issue anymore

[2025-08-29 00:51] diversenok: I have no idea what that was

[2025-08-29 00:52] diversenok: I'll have to restore all dependencies to the exact state they were when I reproduced it

[2025-08-29 00:52] diversenok: And if it still won't work, I guess I'm going insane, lol

[2025-08-29 00:53] Matti: oh well

[2025-08-29 00:53] Matti: insanity really isn't as bad as they make it out to be anyway

[2025-08-29 01:16] diversenok: Okay, insanity postponed. It's just a skill issue

[2025-08-29 01:16] diversenok: Turns out I was assigning  a `-1` due to some things I didn't know about how Delphi compiler extends booleans

[2025-08-29 01:18] diversenok: And NLS clearly didn't like an out-of-bound value

[2025-08-29 01:24] Matti: [replying to diversenok: "Turns out I was assigning  a `-1` due to some thin..."]
I was going to type something here, but I'm just in horror I think
I'll take your word for it

[2025-08-29 01:26] diversenok: Delphi has analogs for `BOOLEAN` and `BOOL`, and for some reason assigning `True` (a built-in constant) to the first sets it to `1` but the second to `0xFFFFFFFF`

[2025-08-29 01:27] diversenok: I guess it's not an issue for true booleans (where you are supposed to check for inequality to 0 instead of equality to 1 anyway) plus maybe helps compatibility with variant booleans

[2025-08-29 01:29] diversenok: But it failed me because I declared `IsImpersonating` as `BOOL`-analog and not `ULONG`-analog

[2025-08-29 02:06] Matti: [replying to diversenok: "Delphi has analogs for `BOOLEAN` and `BOOL`, and f..."]
ok this is strange to be sure, but yeah I think using BOOL is pretty much always a mistake

[2025-08-29 02:07] Matti: so I blame MS at least as much for making the typedef

[2025-08-29 02:07] diversenok: I don't know why I expected `IsImpersonating` to accept only two values

[2025-08-29 02:07] Matti: ...and using it... though that is only in win32 code

[2025-08-29 02:09] diversenok: What about `LOGICAL`?

[2025-08-29 02:11] diversenok: It's technically a native API type since declared in `ntdef.h`

[2025-08-29 02:11] diversenok: Not widely used though

[2025-08-29 02:11] Matti: ugh what is that again? also int32_t right

[2025-08-29 02:11] Matti: yea

[2025-08-29 02:11] diversenok: Yeah, just an `ULONG`

[2025-08-29 02:12] Matti: well ULONG is unsigned

[2025-08-29 02:12] Matti: BOOL is both signed and 32 bits

[2025-08-29 02:12] Matti: wait no, I'm wrong... I think

[2025-08-29 02:12] the horse: i think BOOL is int32_t

[2025-08-29 02:12] the horse: no?

[2025-08-29 02:13] diversenok: Oh, yeah, it is signed

[2025-08-29 02:13] Matti: ok not wrong then <:lillullmoa:475778601141403648>

[2025-08-29 02:14] Matti: yeah so LOGICAL is not great, but you can argue that UCHAR isn't really that much better since it still leaves 254 values other than 0 or 1

[2025-08-29 02:14] Matti: LOGICAL just adds a few more

[2025-08-29 02:14] Matti: but being signed is baaaaad

[2025-08-29 12:48] diversenok: [replying to Matti: "I'm still just stuck wondering when you would ever..."]
I thought about it for a bit, and I suppose it makes sense as a rudimentary way of notifying user-mode NLS code about impersonation changes from the kernel without breaking too many abstraction levels. It's actually a pretty smart hack:
- If NLS sees `IsImpersonating` of `0`, it can use the process locale and free any previously used per-thread locale data
- If NLS sees `IsImpersonating` of `1`, it knows somebody changed the impersonation token since the last NLS call (only the kernel writes the value of `1`) so it can query the token, allocate new locale state, and set `IsImpersonating` to `2`
- If NLS sees `IsImpersonating` of `2`, it knows the thread is still impersonating the same token as before

[2025-08-29 12:52] diversenok: NLS code wants to cache locale data, so it wants to know about impersonation changes. But it's not critical enough to justify constantly querying the token and comparing its ID to see if impersonation changed

[2025-08-29 12:53] diversenok: Modern Windows would probably solve a similar challenge by using something like WNF, but I don't think it would necessarily be a better approach

[2025-08-31 20:03] Sussibaki: Anybody knows how can I get more information about a specific EFI GUID from an EFI application I am currently researching?