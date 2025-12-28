# July 2024 - Week 1
# Channel: #reverse-engineering
# Messages: 112

[2024-07-02 07:50] abu: Hello! When accessing MmPfnDatabase with my page frame number from my pte, the pte address field seems to point to a different pte. Does anyone know why this is? Is it because the database isn't synced all the time?

[2024-07-02 08:31] abu: Nevermind, goddamn MMPFN is somehow 0x38 instead of 0x30

[2024-07-02 14:16] expy: hello, <@162611465130475520> I might have a questing regarding LLVM lifting for deobfuscation purposes.
I'm currently playing around with very basic string stack obfuscation, I have a function which decodes values on the stack at runtime and passes that to a MessageBox. I'm able to lift it to LLVM and unoptimized lifted code runs correctly, displaying a valid message. However, when I turn on optimizations, that string decryption is optimized away, there is a call to MessageBox function but the string itself is invalid. I guess this happens because the parameter passed to MessageBox (stack memory reference for a lifted code) is obviously not annotated as a variable and LLVM thinks it doesn't matter. So, the generic question is, how would deobfuscator distinguish between junk writes on stack in obfuscated code and something like string decryption, when "the junk" can be referred as a parameter to other function.

[2024-07-02 15:01] naci: [replying to expy: "hello, <@162611465130475520> I might have a questi..."]
interesting, how do you handle stack reads? it would be useful if you can show the unoptimized ir

[2024-07-02 15:39] expy: [replying to naci: "interesting, how do you handle stack reads? it wou..."]
every memory access is passed through `__remill_read_memory_xxx`. When you compile .ll back to bin, you could just replace it with a direct memory access. I could also setup the memory for stack at runtime and redirect everything which references stack to that buffer, this will make output binary runnable
[Attachments: test_messagebox.ll, stub.ll]

[2024-07-02 16:14] naci: [replying to expy: "every memory access is passed through `__remill_re..."]
so you say, compiling them with -O0 should print `MessageBox 'Hello, World!'` and compiling with -O3 would print `MessageBox '<random_stuff>'` ? both prints `MessageBox 'Hello, World!'` in my case

[2024-07-02 16:41] expy: `-flto`?

[2024-07-02 16:43] expy: Just noticed something weird. Printf in a memory write function is gone after `-flto`. This should not be  happening

[2024-07-02 17:01] .daniel.w: With IDA, how do you go about updating a database to use an updated executable? It would be nice if there was something like BinDiff that could write to the database and rename everything that's the same

[2024-07-02 17:06] irql: oh man wait till you find out about BinDiff extension for ida

[2024-07-02 17:06] irql: or well i think bindiff natively works with ida pro anyways?

[2024-07-02 17:07] irql: you usually just bindiff the new binary idb against the old idb

[2024-07-02 17:35] Saturnalia: [replying to .daniel.w: "With IDA, how do you go about updating a database ..."]
diaphora

[2024-07-02 17:38] .daniel.w: [replying to irql: "or well i think bindiff natively works with ida pr..."]
I have it, but I don't remember seeing an option for such thing? 🤔

[2024-07-02 17:38] .daniel.w: [replying to Saturnalia: "diaphora"]
Couldn't figure out how to get it working tbh, was really tired

[2024-07-02 17:38] irql: [replying to .daniel.w: "I have it, but I don't remember seeing an option f..."]
I just have the option here
[Attachments: image.png]

[2024-07-02 17:39] irql: I just select any IDB to bindiff against, and it'll try match up symbols

[2024-07-02 17:39] irql: you can then imports names & comments iirc

[2024-07-02 17:39] irql: the decomp will need to be retyped though, but names wise, it works okay

[2024-07-02 17:39] irql: im not sure if you need to somehow integrate it properly

[2024-07-02 17:40] irql: i think I literally just installed bindiff and it integrated into ida

[2024-07-02 17:49] mrexodia: [replying to expy: "every memory access is passed through `__remill_re..."]
So recently I learned from Matteo that this is an issue with the implementation of the read memory function. To resolve this you first need to do an inlining pass to inline all the memory accesses and only after that run the regular `-O3` pipeline

[2024-07-02 18:21] .daniel.w: [replying to irql: "I just have the option here"]
I'll have a look when I'm home from work, thanks a lot 🙏

[2024-07-03 12:51] abu: HELLO! Does anyone know what bit controls the state (Active/Transition/StandBy/Modified/Free/Zeroed) flag in MmPfnDatabase?

[2024-07-03 12:51] abu: https://www.vergiliusproject.com/kernels/x64/windows-10/22h2/_MMPFN
[Embed: Vergilius Project]
Take a look into the depths of Windows kernels and reveal more than 60000 undocumented structures.

[2024-07-03 14:24] expy: I'd like to kindly ask for a help with remill lifting and optimizations. 
What am I trying to accomplish is to lift this simplest possible printf invocation with a stack variable:
```
.text:0000000140001000                 sub     rsp, 38h
.text:0000000140001004                 mov     [rsp+38h+var_18], 539h
.text:000000014000100D                 mov     rdx, [rsp+38h+var_18]
.text:0000000140001012                 lea     rcx, Format     ; "v1: %zu\n"
.text:0000000140001019                 call    printf
.text:000000014000101E                 xor     eax, eax
.text:0000000140001020                 add     rsp, 38h
.text:0000000140001024                 retn
```
I lift it with remill and writing the missed functions (memory writes and printf stub), now I have combined.ll which contains the working code if I run it without optimizations (`-O0` flag), I can see 539h variable passed to printf stub, as well as stack modifications in a stack dump.
However, as soon as I try `-O1` clang clears up the call to stack variable initialization `%call.i.i = call ptr @__remill_write_memory_64(ptr noundef %15, i64 noundef %14, i64 noundef 1337)` which is super weird because it has a `printf` inside ⚠️  I don't understand how clang can simply remove a call to a `printf`.
I'm attaching two version of .ll files, one is `-O0` another is `-O1`, diffing it against each other shows the removed bit code (func `sub_140001000`)
Thanks in advance!
cc <@1242437753455513610>  <@162611465130475520>
[Attachments: printf2_o1.ll, printf2_o0.ll]

[2024-07-03 14:30] mrexodia: [replying to expy: "I'd like to kindly ask for a help with remill lift..."]
I already answered this above

[2024-07-03 14:31] mrexodia: You need to first run an inline pass and only then you can use the regular optimization pipelines like O1 and O3 etc

[2024-07-03 14:31] mrexodia: The reason is the attributes on the write memory intrinsic which says there are no side effects, so it can be removed

[2024-07-03 14:35] expy: is this change tells clang to remove printf? `declare i32 @printf(ptr noundef, ...) #1` -> `declare noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #1`. I'm not getting this part, printf causes a write to console, how can this be treated as something which is a subject to optmize away?

[2024-07-03 14:39] mrexodia: It has nothing to do with printf, it's the attributes on `__remill_write_memory_64`

[2024-07-03 14:39] mrexodia: LLVM doesn't look inside the function in that case

[2024-07-03 14:40] mrexodia: Hm actually you have a different issue that the implementation I used...

[2024-07-03 15:37] .daniel.w: [replying to irql: "you can then imports names & comments iirc"]
Is there a way to quickly import/rename? Currently right clicking everything that's green 😄

[2024-07-03 15:37] .daniel.w: 
[Attachments: 1720021081516.png]

[2024-07-03 15:39] .daniel.w: Ah, good old shift click to the rescue. Apologies for the ping

[2024-07-03 15:40] naci: [replying to expy: "is this change tells clang to remove printf? `decl..."]
I see three `printf`s getting removed and the basic block for them looks like this
```llvm
land.lhs.true2:                                   ; preds = %if.end
  br i1 false, label %if.then3, label %if.end5

if.then3:                                         ; preds = %land.lhs.true2
  %call4 = call i32 (ptr, ...) @printf(ptr noundef @.str.2)
  store i32 4660, ptr %retval, align 4
  br label %return
```

in this case they are removed because the branch condition is always false

[2024-07-03 15:43] irql: [replying to .daniel.w: "Ah, good old shift click to the rescue. Apologies ..."]
lmfao all good

[2024-07-03 15:43] irql: yea, confirm mathces, or import symbols/comments

[2024-07-03 15:48] expy: > in this case they are removed because the branch condition is always false
thanks for looking! 
that one is intended to be optimized, you're looking at read memory redirection for an address which is not used in this program. Concerning function is `sub_140001000`, for some reason `__remill_write_memory_64` is removed (which contained printf inside). Im trying to write inline pass at the moment to try 0xmrexotic's idea

[2024-07-03 15:56] expy: omg, it looks like it's working now! At least with `-O1` inlining of `__remill_write_memory_64` helped. I'm still wondering which specific attribute could cause this behavior
```
attributes #0 = { mustprogress noinline optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
```

[2024-07-03 16:26] naci: yea well i realized now

[2024-07-03 16:33] naci: `__remill_write_memory_64` is removed because its returning a null value `ret ptr null`, though it still shouldnt remove printf

[2024-07-03 16:46] naci: replacing that `ret ptr null` with `ret ptr %call` stops it from getting removed but still doesnt explain why printf is getting removed

[2024-07-03 17:49] expy: [replying to naci: "replacing that `ret ptr null` with `ret ptr %call`..."]
good catch, so it's not about attributes of the function, but about returning a value

[2024-07-04 00:34] .daniel.w: Any idea why these two functions are tied together in IDA?
```
void __fastcall td_registry::setBool(_QWORD *gameMemory, __int64 name, char gfxApi)
game_t *__fastcall td_game::ctor(__int64 gameMemory, _DWORD **name, __int64 gfxApi)
```
If I change the argument name on any of them, it changes on the other 🤔

[2024-07-04 08:08] 冰: [replying to .daniel.w: "Any idea why these two functions are tied together..."]
hmm

[2024-07-04 13:01] .daniel.w: [replying to 冰: "hmm"]
That's the first thing I checked and they don't

[2024-07-04 20:47] expy: hello guys, any ideas why this write & modification of a variable on the stack is not optimized away? The code is writing 'A', 'B', 'C' to stack, then reads it and increments every variable +3 and stores it back. The bitcode is emitted with -O3
[Attachments: image.png]

[2024-07-04 21:07] naci: alias analysis doesnt like inttoptr

[2024-07-04 21:13] naci: you can maybe get away with just using GEP with %stack, but I like to concretize the stack pointer in my lifter so I dont have keep track where sp is moved and convert to alloca

[2024-07-04 21:21] naci: a trick that works fine for me is, everything is just a GEP to a %memory arg (could be a global var aswell), I let cse and simplifyinst run, then i just remove convert those GEP's to inttoptr's https://github.com/NaC-L/Mergen/blob/main/lifter/CustomPasses.hpp#L184 (the code isnt the prettiest but you should get the idea)

[2024-07-04 21:24] contificate: code like this that makes clear that 4 spaces is a mistake

[2024-07-04 21:24] naci: 😭

[2024-07-04 21:24] naci: clang-format trolled me and i was too lazy to fix it

[2024-07-04 21:26] contificate: come write some passes and we can b2b saas our way into early retirement on the back of llvm
[Attachments: 2024-06-08-144622_1477x820_scrot_2.png]

[2024-07-04 21:27] naci: 🫡 🫡 🫡 🫡

[2024-07-04 21:27] Brit: first you have to leverage a lifter, say idk, remill

[2024-07-04 21:28] contificate: clownage

[2024-07-04 21:28] contificate: need to find a nicer language to use for LLVM rewriting

[2024-07-04 21:29] contificate: should be fully possible even if a ton of work

[2024-07-04 21:29] contificate: so many great bindings for just shitting IR via IRBuilder

[2024-07-04 21:29] contificate: but not so much for doing new pass manager-esque worklist algorithms

[2024-07-04 21:29] Brit: there was that one project

[2024-07-04 21:29] Brit: called rellume

[2024-07-04 21:30] contificate: I remember this idea that I'd just embed Python or something

[2024-07-04 21:30] contificate: and do the algorithmic parts in Python or Scheme or whatever

[2024-07-04 21:30] contificate: maybe even OCaml

[2024-07-04 21:30] Brit: <:topkek:904522829616263178>

[2024-07-04 21:30] contificate: and then just reflect results into C++ garbage

[2024-07-04 21:30] contificate: honestly convinced that

[2024-07-04 21:30] contificate: C++ is a major reason why you don't see the above

[2024-07-04 21:31] contificate: obviously the situation is more complex because LLVM versions have breaking file format changes

[2024-07-04 21:31] contificate: but really think there's some value in doing LLVM injection/rewrites as part of CI or something

[2024-07-04 21:31] contificate: maybe for some people somewhere

[2024-07-04 21:32] naci: [replying to contificate: "come write some passes and we can b2b saas our way..."]
no MIR passes?

[2024-07-04 21:33] contificate: maybe, but that's harder

[2024-07-04 21:33] contificate: to do in a plugin way

[2024-07-04 21:33] contificate: the most accessible is the default LLVM IR pipeline

[2024-07-04 21:33] contificate: changing much else requires in-tree work

[2024-07-05 13:29] expy: [replying to naci: "alias analysis doesnt like inttoptr"]
it looks like `inttoptr` is not a problem itself. What I tried now is the array with only one element, there is `inttoptr`  as well, however it's optimized and I can see only one memory write. As long as I create two elements in array, the optimization stop working for both elements 🤔

[2024-07-05 17:13] contificate: You could dump some information if you have a debug build of LLVM

[2024-07-05 23:34] expy: > You could dump some information if you have a debug build of LLVM
for example which one?

[2024-07-05 23:40] expy: btw found weird example, reordering of unrelated blocks facilitates optimizations
https://godbolt.org/z/e4fGv1Kns -- store two variables, then modify them, two resulting values are not optimized, its 20 and 20
https://godbolt.org/z/rWcsMxYKT -- store variable, modify it, store another one, modify new one, the result got optimized

[2024-07-06 12:08] naci: [replying to expy: "it looks like `inttoptr` is not a problem itself. ..."]
https://godbolt.org/z/K3dETWWbv
[Embed: Compiler Explorer - LLVM IR (clang (trunk))]
define dso_local i32 @fun() local_unnamed_addr #5 {                                                                       
                                                                             

[2024-07-06 12:15] naci: i dont remember the exact details but if u run basic-aa the alias analysis will have problems figuring out with pointer aliases which with inttoptr

[2024-07-07 13:27] repnezz: how do I filter debug messages in windbg (like you can do with dbgview) ?

[2024-07-07 13:42] koyz: [replying to repnezz: "how do I filter debug messages in windbg (like you..."]
In case u have a consistent sub-string in your debug messages you could use .ofilter -> https://learn.microsoft.com/en-us/windows-hardware/drivers/debuggercmds/-ofilter--filter-target-output-
[Embed: .ofilter (Filter Target Output) - Windows drivers]
The .ofilter command filters the output from the target application or target computer.

[2024-07-07 13:59] repnezz: [replying to koyz: "In case u have a consistent sub-string in your deb..."]
Thanks

[2024-07-07 14:13] repnezz: .ofilter “[*]” doesn’t work

[2024-07-07 14:31] koyz: [replying to repnezz: ".ofilter “[*]” doesn’t work"]
You probably would want something like:
```
.ofilter \[*\]*
```

[2024-07-07 14:31] koyz: https://learn.microsoft.com/en-us/windows-hardware/drivers/debuggercmds/string-wildcard-syntax
[Embed: String Wildcard Syntax - Windows drivers]
This topic covers string wildcard syntax. Some debugger commands have string parameters that accept a variety of wildcard characters.

[2024-07-07 17:20] naci: <@1078478533908779078> nvm, it wasnt basic-aa, it was aa-eval https://godbolt.org/z/13Y69feh5
[Embed: Compiler Explorer]
Compiler Explorer is an interactive online compiler which shows the assembly output of compiled C++, Rust, Go (and many more) code.

[2024-07-07 17:23] expy: [replying to naci: "<@1078478533908779078> nvm, it wasnt basic-aa, it ..."]
didn't get that, both windows on the right side are having add instruction

[2024-07-07 17:25] naci: if you use `inttoptr` alias analysis will mark it as `may alias` no matter what, meaning it will think the second inttoptr might change/modify the first inttoptr

[2024-07-07 17:26] naci: so it cant safely load the first pointer

[2024-07-07 17:27] naci: below case with getelementptr, it will mark as `no alias` as it supposed to be, because two pointers are seperate pointers and dont modify each other in any means, so its safe to load and optimize

[2024-07-07 17:27] naci: look at the compiler output window

[2024-07-07 17:28] naci: 
[Attachments: image.png]

[2024-07-07 17:33] expy: thanks! but why "may alias" produce `inttoptr` 🤔 I was thinking about it as an opaque pointer, and I was thinking GEP is better for optimizations, no?

[2024-07-07 17:36] naci: that is a great question and i dont know why

[2024-07-07 17:36] naci: gep is better for optimizations, thats right though

[2024-07-07 18:02] expy: yeah, another thing which I don't understand it's why everything in this case is not optimized away? it's only stack allocation and modification, it's not used anywhere else. If I do int in C++ it's gone https://godbolt.org/z/aeWKWqYbj

[2024-07-07 18:15] naci: which case are you referring by 
>  why everything in this case is not optimized away

[2024-07-07 18:23] expy: nvm, I figured out, I meant the original one (top left on your link https://godbolt.org/z/13Y69feh5). So, if I just move away from inttoptr and use only gep it's gone, here is an example https://godbolt.org/z/bx9nTd8qh

[2024-07-07 18:43] expy: so the solution for me would be: after remill lifted code and I inlined memory reads/writes (to use a variable on the actual functions' stack), then I must write a pass which replaces all `inttoptr`s to GEPs. 
Is this the right way of deobfuscating runtime stack variable ofuscation? cc <@162611465130475520>

[2024-07-07 18:53] naci: yeah I would convert inttoptrs to GEPs then run passes like early-cse and simplifyinst then convert it back to inttoptr

[2024-07-07 18:53] naci: idk if thats the right way but its the most convenient way for me

[2024-07-07 18:56] expy: [replying to naci: "yeah I would convert inttoptrs to GEPs then run pa..."]
why do you need to have `inttoptr` in the end though?

[2024-07-07 19:01] naci: lets say I have something like
```asm
lea rax, off_14000F3B8
mov rax, [rax]
ret
```
I lift this to 
```llvm
define i64 @main(i64 %rax, i64 %rcx, i64 %rdx, i64 %rbx, i64 %0, i64 %rbp, i64 %rsi, i64 %rdi, i64 %r8, i64 %r9, i64 %r10, i64 %r11, i64 %r12, i64 %r13, i64 %r14, i64 %r15, ptr %memory) {
entry:
%GEPLoadxd-5369456437- = getelementptr i8, ptr %memory, i64 0x14000F3B8
%loadvalue = load i64, ptr %GEPLoadxd-5369456437-, align 4
ret i64 %loadvalue
}
```
since this will use %memory as arg it will mess up the compilation so i convert it back to inttoptr

[2024-07-07 19:01] naci: if you use it for stack stuff, no need to do this ig

[2024-07-07 19:04] naci: basically what i do: if not stack, convert it back to inttoptr, else dont do anything