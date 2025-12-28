# July 2025 - Week 4
# Channel: #programming
# Messages: 536

[2025-07-21 00:24] Matti: unresolved symbol errors can happen to the best of us when compiling a codebase that it itself big and complex enough to requiree a dedicated drive for a singlle repository

[2025-07-21 00:24] Matti: meaning UE of course

[2025-07-21 00:25] Matti: in general I've found that as usual, using MSVC is a mistake and its linker errors in this case are a lot less useful than lld's

[2025-07-21 00:26] Matti: lld will tell you (with or without LTO enabled) the referencing obj file, what module it is a part of, and even the referencing location(s) down to the source line sometimes

[2025-07-21 00:28] Matti: [replying to elias: "The most useful error message I've seen in a while"]
whoever wrote both the thing you're building as well as the library you're failing to link against, foresaw this issue and even went as far as to make a function so explicitly named that it mostly compensates for MSVC's shit-ness

[2025-07-21 00:29] Matti: it's kind of amusing, and not the worst idea I've seen tbh

[2025-07-21 00:29] Matti: it's not like lld will magically know what lib you need for some function

[2025-07-21 00:30] Matti: you're supposed to know that

[2025-07-21 00:30] Matti: but if you don't, it still points you to the referencing location(s) so that it is trivial to find out

[2025-07-21 00:32] elias: the error came from having address sanitizer enabled for the build and also using /nodefaultlib linker option

[2025-07-21 00:33] Matti: yeah

[2025-07-21 00:33] elias: without it I would have to google what libs are required or worse do trial and error so yeah it did safe some time

[2025-07-21 00:33] elias: [replying to contificate: "this situation only really arises if you're buildi..."]
thats how I encounter most of those errors btw

[2025-07-21 00:34] Matti: tbf, had you read the llvm documentation I'm pretty sure the asan docs would have pointed out the runtime library requirement

[2025-07-21 00:35] Matti: I also compile with /NODEFAULTLIB always so I've seen my share of these

[2025-07-21 00:37] Matti: generally this is either caused by a full blown external dependency, in which case yeah... what did you expect would happen
or a compiler flag that secretly requires a lot more help than you might expect (MSVC /GS is one example)

[2025-07-21 00:38] Matti: I doubt /GS requiring a static lib, nevermind which one, is documented since MS assume you are aalways using libc

[2025-07-21 00:38] Matti: but for all of llvm's cases I've run into so far (asan, PGI) this was in the documentation

[2025-07-21 00:41] Matti: I think the most egregious case I've seen is trying to compile something like
`float x = 3.14;`
in MSVC, targeting 32 bit x86

[2025-07-21 00:41] Matti: and then failing because it requires a compiler library for FP maths

[2025-07-21 00:42] Matti: https://stackoverflow.com/questions/1583196/building-visual-c-app-that-doesnt-use-crt-functions-still-references-some

[2025-07-21 00:55] LabGuy94: Does anyone know if it's possible to allocate memory within a specific virtual address range in Windows kernel?

[2025-07-21 01:11] Matti: sure

[2025-07-21 01:11] Matti: just set the base address to the VA you want the allocation at

[2025-07-21 01:11] Matti: before calling VirtualAlloc/NtAllocateeVM

[2025-07-21 01:12] Matti: if the VA range (base + size) is available, you'll get it

[2025-07-21 01:21] diversenok: `NtAllocateVirtualMemoryEx` also allows specifying `MemExtendedParameterAddressRequirements` using the following structure:
```c
typedef struct _MEM_ADDRESS_REQUIREMENTS {
    PVOID LowestStartingAddress;
    PVOID HighestEndingAddress;
    SIZE_T Alignment;
} MEM_ADDRESS_REQUIREMENTS, *PMEM_ADDRESS_REQUIREMENTS;
```

[2025-07-21 01:22] diversenok: Available since RS5 though

[2025-07-21 01:24] diversenok: While explicitly choosing an address as Matti suggested is available since at least Win 2000

[2025-07-21 01:41] Matti: ugh, that reminds me that API exists

[2025-07-21 01:42] Matti: I still need to implement it but I don't want to

[2025-07-21 01:43] Matti: it seems like half of the parameters you can pass are related to initializing enclave memory

[2025-07-21 01:43] Matti: which is an explicit non-goal of MRK

[2025-07-21 01:44] Matti: I could of course just make those return failure and implement the rest, but I don't like half-implementing APIs either cause it always leads to issues down the road

[2025-07-21 01:49] Matti: am I crazy or does VirtualAlloc2 let you request both (but not at the same time)
\- `MEM_EXTENDED_PARAMETER_NONPAGED` via `MEM_EXTENDED_PARAMETER `
and
\- `MEM_PHYSICAL` via `MemExtendedParameterAttributeFlags`

[2025-07-21 01:49] Matti: and AWE memory

[2025-07-21 01:49] Matti: and none of the docs mention anything about privileges required for this

[2025-07-21 01:50] Matti: surely all of those would require SeLockMemory

[2025-07-21 01:50] Matti: sorry, MEM_PHYSICAL *means* AWE memory

[2025-07-21 01:52] Matti: but stil, two typics of physical allocations, plus `MEM_64K_PAGES` which can be combined with nonpaged and possibly AWE

[2025-07-21 01:52] Matti: no mention of any access rights required for doing this

[2025-07-21 01:54] Matti: wait but then why the fuck is there **also** a new `AllocateUserPhysicalPages2`?

[2025-07-21 01:54] Matti: also taking MEM_EXTENDED_PARAMETER again

[2025-07-21 01:56] Matti: I should just close these tabs, reading win32 docs is no good for my health

[2025-07-21 01:57] diversenok: lol

[2025-07-21 01:57] diversenok: There is also `NtMapViewOfSectionEx`, btw

[2025-07-21 01:57] Matti: yeah

[2025-07-21 01:57] Matti: let me guess, it takes MEM_EXTENDED_PARAMETER

[2025-07-21 01:58] diversenok: Yep

[2025-07-21 01:58] Matti: shocker

[2025-07-21 01:58] Matti: oh well at least this one has kernel mode docs

[2025-07-21 01:59] diversenok: > For more information about extended parameters, see the description of the MapViewOfFile3 routine.

[2025-07-21 01:59] diversenok: Too bad

[2025-07-21 01:59] Matti: yeah oh well

[2025-07-21 01:59] Matti: those by themselves are self-explanatory enough

[2025-07-21 02:00] Matti: what I don't get is why they decided to make a generic one-struct-fits all type for completely different syscalls

[2025-07-21 02:01] Matti: which leads to confusing situations because the number of parameters you can now pass suffers from combinatorial explosion

[2025-07-21 02:01] Matti: and for each 'weird' but possible combination of flags and attributes you have to determine whether it is supported or not, and if so, for which API(s)

[2025-07-21 02:03] diversenok: They also liked this design pattern so much that there is another API using a very similar approach: `NtLoadKey3`

[2025-07-21 02:03] Matti: haha

[2025-07-21 02:03] Matti: yeah but Cm is insane so I expect stuff like that

[2025-07-21 02:03] Matti: did you know that

[2025-07-21 02:04] Matti: omg how to even word this

[2025-07-21 02:04] Matti: if you copy a a registry (sub)tree (which I didn't even know was possible)

[2025-07-21 02:05] Matti: there is a kernel function that is used for this that works on a *virtual* stack

[2025-07-21 02:05] Matti: with about 30 goto labels in this function alone

[2025-07-21 02:06] Matti: so as to prevent recursion in the registry to too far a level from causing a kernel stack overflow

[2025-07-21 02:06] diversenok: How do you copy a registry tree?

[2025-07-21 02:06] diversenok: I only know save/restore

[2025-07-21 02:06] Matti: that is a good question

[2025-07-21 02:07] Matti: I hope it's simply unreachable code

[2025-07-21 02:07] Matti: but I fear it's not

[2025-07-21 02:07] diversenok: It could be related to layered hives

[2025-07-21 02:08] diversenok: Layering a differencing hive on top of another one

[2025-07-21 02:08] Matti: nah, well I mean that too

[2025-07-21 02:08] Matti: but I found a simpler example

[2025-07-21 02:10] Matti: `CmpCloneControlSet` -> `CmpCopySyncTree` -> `CmpCopySyncTree2`  (trigger warning)
```c
BOOLEAN
CmpCopySyncTree2(
    PCMP_COPY_STACK_ENTRY CmpCopyStack,
    ULONG                 CmpCopyStackSize,
    ULONG                 CmpCopyStackTop,
    PHHIVE                 CmpSourceHive,
    PHHIVE                 CmpTargetHive,
    BOOLEAN                 CopyVolatile,
    CMP_COPY_TYPE         CopyType
    );
```

[2025-07-21 02:10] LabGuy94: [replying to Matti: "before calling VirtualAlloc/NtAllocateeVM"]
I was talking about allocating kernel virtual memory not user mode memory from a kernel driver (my mistake)

[2025-07-21 02:11] Matti: there is no such thing

[2025-07-21 02:11] LabGuy94: Yeah what I expected

[2025-07-21 02:11] Matti: if you want to allocate VM, you can call ZwAllocateVM

[2025-07-21 02:11] Matti: but it will still return a user mode address

[2025-07-21 02:11] Matti: this can be useful for calling into win32k sometimes

[2025-07-21 02:11] Matti: that's about it

[2025-07-21 02:12] LabGuy94: Yeah I wanted to allocate in the kernel cr3 so probably not what I’m looking for

[2025-07-21 02:12] Matti: (win32k assumes every call comes from user mode by default without ever checking this, so using kernel stack memory for parameters is usually a bsod)

[2025-07-21 02:14] Matti: [replying to LabGuy94: "Yeah I wanted to allocate in the kernel cr3 so pro..."]
'in the kernel cr3'?
the system process will give you that if it's any help, but not VM above the user limit

[2025-07-21 02:14] Matti: why VM?

[2025-07-21 02:16] diversenok: [replying to Matti: "`CmpCloneControlSet` -> `CmpCopySyncTree` -> `CmpC..."]
I see it's also called from `CmSaveKey`, `CmSaveMergedKeys`, and `CmRestoreKey`

[2025-07-21 02:16] Matti: yeah... there are quite a few paths

[2025-07-21 02:17] Matti: not sure if any of them are reachable via syscalls or exports

[2025-07-21 02:17] Matti: but more likely yes than no

[2025-07-21 02:18] Matti: ah yeah the obvious... NtSaveKey -> CmSaveKey

[2025-07-21 02:19] Matti: but I don't think there's a full fledged NtSynchronizeKeys or whatever that gives you control over this insane function

[2025-07-21 02:21] Matti: helper
```c
//
// Helper
//

BOOLEAN
CmpCopySyncTree2(
    PCMP_COPY_STACK_ENTRY CmpCopyStack,
    ULONG                 CmpCopyStackSize,
    ULONG                 CmpCopyStackTop,
    PHHIVE                 CmpSourceHive,
    PHHIVE                 CmpTargetHive,
    BOOLEAN                 CopyVolatile,
    CMP_COPY_TYPE         CopyType
    )
/*++

Routine Description:

 This is a helper routine for CmpCopySyncTree. It accomplishes
 the functionality described by that routine in a "virtually"
 recursive manner which frees this routine from the limitations
 of the Kernel stack.
 
 This routine should not be called directly. Use CmpCopySyncTree!.
     
Arguments:

    (All of these are "virtual globals")

    CmpCopyStack - "global" pointer to stack for frames

    CmpCopyStackSize - allocateed size of stack

    CmpCopyStackTop - current top

    CmpSourceHive, CmpTargetHive - source and target hives
    
    CopyVolatile, CopyType - same as CmpCopySyncTree.


Return Value:

    BOOLEAN - Result code from call, among the following:
        TRUE - it worked
        FALSE - the tree copy/sync was not completed (though more than 0
                keys may have been copied/synced)

--*/
```

[2025-07-21 02:21] Matti: <:yea:904521533727342632>

[2025-07-21 02:22] Matti: also the definition of a 'copy' is quite broad

[2025-07-21 02:22] Matti: ```c
typedef enum _CMP_COPY_TYPE {
    Copy,
    Sync,
    Merge
} CMP_COPY_TYPE;
```

[2025-07-21 02:23] diversenok: Merge is probably for `NtSaveMergedKeys`

[2025-07-21 02:23] Matti: yeah

[2025-07-21 02:23] Matti: has to be

[2025-07-21 02:25] Matti: the comment on the function you should!!! call (CmpCopySyncTree)  is even longer

[2025-07-21 02:25] Matti: 
[Attachments: image.png]

[2025-07-21 02:25] Matti: but no mention of merging

[2025-07-21 02:27] diversenok: Damn, now I want to create a tool like FileTest, but for registry so I can play with these things

[2025-07-21 02:28] Matti: you'd find so many vulnerabilities

[2025-07-21 02:28] Matti: Cm is completely insane code

[2025-07-21 02:28] Matti: much like NTFS

[2025-07-21 02:28] Matti: you could become the jonas of registry bugs

[2025-07-21 02:31] Matti: CmSaveMergedKeys is a pretty good example of a (relatively!) well-written Cm function actually

[2025-07-21 02:32] Matti: it's only well-written when compared to the rest of Cm... 🥲

[2025-07-21 02:34] Matti: ```c
#define CmRetryExAllocatePoolWithTag(a,b,c,Result) \
    {                                             \
        ULONG RetryCount = 10;                    \
        do {                                        \
            Result = ExAllocatePoolWithTag(a,b,c); \
        } while ((!Result) && (RetryCount--));     \
    }
```
😂

[2025-07-21 02:35] Matti: not even an mm_pause

[2025-07-21 02:35] Matti: just fail 10 times in a row as quickly as possible

[2025-07-21 02:46] JustMagic: [replying to Matti: "not even an mm_pause"]
I mean it's calling an expensive allocation function, I don't think there is any point in pause

[2025-07-21 02:47] Matti: yeah you're right, it does depend on the failure reason how expensive it will be but it's not a spinlock

[2025-07-21 02:48] Matti: I'm just used to adding a pause statement when I'm already doing something questionable in a loop, like retry an allocation for the 10th time after it failed the last 9 calls

[2025-07-21 02:48] Matti: but this is only useful when you're raping the CPU without giving other (SMT) cores a chance

[2025-07-21 02:50] Matti: what Mm code tends to do fairly often instead in similar not-spinlock situations is literally wait a short amount of time

[2025-07-21 02:50] Matti: when IRQL permits, which is definitely the case here cause all callers are at passive

[2025-07-21 02:51] Matti: seems a lot more likely to give a succesful result back in the end to me

[2025-07-21 02:54] Matti: I see RtlBackoff is now also used (very) sparingly in some kernel code

[2025-07-21 08:12] dullard: [replying to Matti: "you could become the jonas of registry bugs"]
j00ru is the Jonas of registry bugs

[2025-07-21 15:37] 0xatul: dude I tried reproducing some of his bugs

[2025-07-21 15:37] 0xatul: the code is just not very pleasant to read

[2025-07-21 15:38] 0xatul: even with types from many symbol sources

[2025-07-21 15:47] Deleted User: <@775688014176452608> check dms.

[2025-07-23 01:29] UJ: [replying to elias: "The most useful error message I've seen in a while"]
how about a LTE for telling you, you used floats?
[Attachments: Screenshot_2025-07-22_182555.png]

[2025-07-23 03:22] Matti: [replying to Matti: "I think the most egregious case I've seen is tryin..."]
yeah _fltused is from this one, it's in the SO I linked

[2025-07-23 03:24] Matti: [replying to 0xatul: "the code is just not very pleasant to read"]
I have the same experience with his code
it wasn't a POC of his I was trying to reproduce exactly, but rather get some ALPC code in a blogpost of his (forget whether it was related to a vulnerability or not) to work

[2025-07-23 03:25] Matti: in the end I simply failed and gave up on that and rewrote the servers (plural now, the goal was to learn about ALPC after all...) and client from scratch

[2025-07-23 03:26] Matti: it's one of those projects I should have put on github years ago but I've been slacking on the last 5%

[2025-07-23 03:27] Matti: I think there still isn't any actual working example of a basic ALPC server and client talking to each other that will actually work after all these years

[2025-07-23 04:04] UJ: [replying to Matti: "yeah _fltused is from this one, it's in the SO I l..."]
Do you know if there is any similar way to error out if any sse/sse2/avx instruction is used/ (will end up in the compiled binary) (directly or indirectly)? even disabling it in msvc doesn't guarantee they won't be used.

[2025-07-23 04:05] Matti: `/kernel` should do the trick if it's 'just code'

[2025-07-23 04:06] Matti: but really the answer is no because you can always sabotage yourself by linking in any assembly or precompiled .obj file that contains these instructions

[2025-07-23 04:07] Matti: actually `/kernel` will still give a linker error if other object files were not also compiled with `/kernel` - I forgot about this

[2025-07-23 04:08] UJ: [replying to Matti: "`/kernel` should do the trick if it's 'just code'"]
unfortunately my question was exactly due to this not being sufficient. i don't want to have to save and restore avx type registers on vmexits so ive just been looking at my bin in ida after compiling to spot check.

[2025-07-23 04:08] Matti: but like, you can find a way to make a bad .obj file with that flag set too I'm sure

[2025-07-23 04:08] UJ: yea

[2025-07-23 04:09] Matti: [replying to UJ: "unfortunately my question was exactly due to this ..."]
well `/kernel` will disallow AVX, not SSE/2

[2025-07-23 04:09] Matti: what if you combine with `/arch:IA32`

[2025-07-23 04:10] Matti: `/arch:SSE2` is the implicit default on amd64 because it doesn't have the same limitations AVX does in NT kernel mode - but that does make me wonder why you would care about SSE usage

[2025-07-23 04:11] UJ: [replying to Matti: "what if you combine with `/arch:IA32`"]
i could try that. it hasn't been an issue *so far*, i just want to avoid one of those nasty bugs where i potentially clobber one of these registers on accident and have to spend a month finding out what broke.

[2025-07-23 04:12] Matti: well even memset and memcpy will use SSE by default on windows 10/11, if using the standard kernel libs

[2025-07-23 04:12] Matti: getting rid of all SSE usage including in code not written by you is a pretty uphill battle

[2025-07-23 04:13] UJ: [replying to Matti: "well even memset and memcpy will use SSE by defaul..."]
yep. im avoiding all of these. thankfully its just in the vmexit code i need to make sure they aren't being used so the surface area is pretty small.

[2025-07-23 05:32] the horse: [replying to Matti: "well even memset and memcpy will use SSE by defaul..."]
win11 ucrt is soooo good

[2025-07-23 05:32] the horse: the memset/memcpy implementations actually slap

[2025-07-23 05:32] the horse: supa fucking fast

[2025-07-23 05:32] the horse: optimized for host cpu 🙏

[2025-07-23 05:33] Matti: yeah but that one is not so good in kernel mode

[2025-07-23 05:33] the horse: true..

[2025-07-23 05:33] Matti: that's why there is a separate crt for kernel

[2025-07-23 05:33] the horse: I just use Musa for kernel CRT

[2025-07-23 05:33] the horse: ¯\_(ツ)_/¯

[2025-07-23 05:33] learncpp.com: [replying to the horse: "true.."]
Actix or axum?

[2025-07-23 05:34] the horse: actix

[2025-07-23 05:34] learncpp.com: yesyes

[2025-07-23 05:34] the horse: if you are talking about da zed rich presence

[2025-07-23 05:34] the horse: the amount of vibe coding that has gone into this
[Attachments: image.png]

[2025-07-23 05:34] learncpp.com: All the homies love actix, so much better than axum and rocket

[2025-07-23 05:34] Matti: [replying to the horse: "I just use Musa for kernel CRT"]
googled
> The Universal C++ RunTime library, supporting kernel-mode C++ exception-handler and STL. 
this one? no thanks...

[2025-07-23 05:35] Matti: did you know the WDK comes with a CRT

[2025-07-23 05:35] the horse: mf will do anything but use typescript 🙏

[2025-07-23 05:35] the horse: [replying to Matti: "did you know the WDK comes with a CRT"]
yeah but from experience it has awful support for std::

[2025-07-23 05:35] the horse: and I write cpp not C

[2025-07-23 05:35] the horse: I like my containers and fancy types 💔

[2025-07-23 05:36] the horse: [replying to the horse: "the amount of vibe coding that has gone into this"]
also kinda clean,
[Attachments: image.png]

[2025-07-23 05:36] learncpp.com: [replying to the horse: "mf will do anything but use typescript 🙏"]
Facts, what are you using for frontend? I just started trying svelte out and it isnt to bad but still prefer vue with ts

[2025-07-23 05:36] the horse: [replying to learncpp.com: "Facts, what are you using for frontend? I just sta..."]
rust

[2025-07-23 05:36] the horse: with dioxide

[2025-07-23 05:36] the horse: it looks like react

[2025-07-23 05:37] the horse: this whole thing is in rust

[2025-07-23 05:37] learncpp.com: Oh nice, I havnt tried that yet

[2025-07-23 05:37] learncpp.com: Last frontend thing I tried with rust was yew

[2025-07-23 05:37] the horse: api, backend, frontend, middleware

(of course with css elements)

[2025-07-23 05:37] learncpp.com: Can't you inline css with it?

[2025-07-23 05:37] the horse: looks like this
[Attachments: image.png]

[2025-07-23 05:37] learncpp.com: With yew your able to

[2025-07-23 05:37] learncpp.com: Yes you can

[2025-07-23 05:38] the horse: 
[Attachments: image.png]

[2025-07-23 05:38] the horse: actually very clean

[2025-07-23 05:38] learncpp.com: I might try it out, I love to do templating so it would be really cool to do

[2025-07-23 05:39] the horse: this has been my most hated rust excercise so far

[2025-07-23 05:39] the horse: honestly; fuck web stuff

[2025-07-23 05:39] learncpp.com: My current small saas uses tera for templating

[2025-07-23 05:39] the horse: i'd rather pay $500 to a vibe coding indian

[2025-07-23 05:39] learncpp.com: [replying to the horse: "honestly; fuck web stuff"]
I like backend development but not frontend

[2025-07-23 05:39] the horse: thank you gemini
[Attachments: image.png]

[2025-07-23 05:40] learncpp.com: Yikes ai

[2025-07-23 05:40] learncpp.com: jkjk, I think ai is cool for finding answers but I dont fw copy pasting whole bits of code

[2025-07-23 05:40] learncpp.com: Except redundant stuff or frontend

[2025-07-23 05:41] the horse: sometimes it's incredibly stupid but I ain't doing frontend 🤣

[2025-07-23 05:41] Deleted User: frontend is like the worst part

[2025-07-23 05:41] learncpp.com: My friend has done some crazy frontend using claude, idk how cuz i tried and it gave me some nasty asf basic landing page and I gave up

[2025-07-23 05:41] Deleted User: so bad im still using shitty ass scuffed python scripts to manage my shit

[2025-07-23 05:41] the horse: claude has a visualizer but so far it didn't produce anything worthwhile for me

[2025-07-23 05:42] the horse: I think chatgpt does too now

[2025-07-23 05:42] the horse: and grok4?

[2025-07-23 05:42] learncpp.com: Ye, I'll stick with my vue components for now

[2025-07-23 05:42] learncpp.com: [replying to the horse: "and grok4?"]
If it does it sucks

[2025-07-23 05:42] the horse: my prompt engineering skills guarantee me a job forever.

[2025-07-23 05:42] the horse: until someone makes a prompt engineering llm

[2025-07-23 05:42] the horse: 💔

[2025-07-23 05:42] learncpp.com: I got grok 4 for math and its good for thay but I tried frontend with it and it was pretty ass

[2025-07-23 05:44] learncpp.com: I wanna try grok out on one of my code bases and see if it can do it better but im going to wait for the coding model thats supposed to come out next month

[2025-07-23 08:55] Yoran: [replying to the horse: "this whole thing is in rust"]
Vibes
[Attachments: The_Irritating_Gentleman_23072025115435.jpg]

[2025-07-23 10:49] ImagineHaxing: Rust best

[2025-07-23 12:28] selfprxvoked: [replying to Yoran: "Vibes"]
C++ adding Memory Safe to the Standard will be enough to kill Rust, the awful language <:mmmm:904523247205351454>

[2025-07-23 12:34] Brit: [replying to selfprxvoked: "C++ adding Memory Safe to the Standard will be eno..."]
given how good recent c++ developments have been I expect that it will push even more people to stick to whatever c++ ver they settled on

[2025-07-23 12:34] Yoran: [replying to selfprxvoked: "C++ adding Memory Safe to the Standard will be eno..."]
Ok

[2025-07-23 12:36] selfprxvoked: [replying to Brit: "given how good recent c++ developments have been I..."]
Wdym? People are loving C++26

[2025-07-23 12:36] ImagineHaxing: [replying to selfprxvoked: "C++ adding Memory Safe to the Standard will be eno..."]
rust is awesome

[2025-07-23 12:42] selfprxvoked: [replying to ImagineHaxing: "rust is awesome"]
Rust is awful and it syntax just make it worse

[2025-07-23 12:44] Yoran: American English is awful and it's syntax just makes it worse ngl

[2025-07-23 12:45] selfprxvoked: It is because I'm typing in my phone without US and it keeps auto correcting it <:mmmm:904523247205351454>

[2025-07-23 12:46] Brit: [replying to selfprxvoked: "Wdym? People are loving C++26"]
are the people in the room w you rn?

[2025-07-23 12:46] Yoran: [replying to selfprxvoked: "Rust is awful and it syntax just make it worse"]
Maybe share your experience or something

[2025-07-23 12:46] Yoran: Like this is an empty statement

[2025-07-23 12:46] Yoran: What was awful? What in the design/impl was so bad?

[2025-07-23 12:46] Brit: Im not even a rust evangelist the language has plenty issues but I've used c++ long enough to say that its awful

[2025-07-23 12:47] Brit: and made more awful by the comitee by the minute

[2025-07-23 12:47] Brit: just think back to the embed debacle

[2025-07-23 12:47] Yoran: My keyboard tweaking

[2025-07-23 12:47] selfprxvoked: [replying to Brit: "and made more awful by the comitee by the minute"]
Thats true tho

[2025-07-23 12:49] selfprxvoked: Does Rust support something similar to `constexpr` and `consteval`?

[2025-07-23 12:51] sariaki: yes lol

[2025-07-23 12:51] Brit: ofc

[2025-07-23 12:52] sariaki: its macros are also waaay more powerful in general

[2025-07-23 12:53] sariaki: [replying to selfprxvoked: "Rust is awful and it syntax just make it worse"]
i used to think this.
how much you like the syntax of a language depends a lot on the languages you use

[2025-07-23 12:53] sariaki: you ain't seen nothing yet if you think rust is bad

[2025-07-23 12:53] sariaki: look at functional languages

[2025-07-23 12:54] sariaki: they will look super wacky the first time you see them

[2025-07-23 12:54] sariaki: lisp, clojure, haskell
[Attachments: 1_iJioRCmg3f-uur2da3Zl7Q.png, Screen-Shot-2019-03-25-at-9.21.39-AM_huacf19c8c9f63351ccde435e7ae55c35a_175668_0x600_resize_q75_h2_box_2.webp, Screenshot_2024-02-09_at_03.42.53.png]

[2025-07-23 12:55] sariaki: idk i thought they looked pretty weird when i first saw them ¯\_(ツ)_/¯

[2025-07-23 12:55] sariaki: the syntax really doesn't matter much though

[2025-07-23 12:57] selfprxvoked: [replying to sariaki: "i used to think this.
how much you like the syntax..."]
I'll try out in small projects then 👍

[2025-07-23 13:02] elias: Is there a better way to find the VadRoot of a process other than hardcoding the offset? Appears to be rather volatile

[2025-07-23 13:26] Yoran: [replying to selfprxvoked: "Does Rust support something similar to `constexpr`..."]
First of all

[2025-07-23 13:26] Yoran: What cpp version are we talking

[2025-07-23 13:26] Yoran: Second of all

[2025-07-23 13:26] Yoran: Yes

[2025-07-23 13:26] Yoran: Third of all

[2025-07-23 13:26] Yoran: Nah I won't actually

[2025-07-23 13:27] Yoran: [replying to sariaki: "i used to think this.
how much you like the syntax..."]
IMO unless the syntax is absolute dog shit to the point of unbearable I don't really care

[2025-07-23 13:28] Yoran: The throughout of my fingers on a keyboard is pretty high. I don't mind typing more. Same goes for reading

[2025-07-23 13:29] contificate: You should mind

[2025-07-23 13:29] contificate: actual carpal tunnel syndrome to use some langs in various domains

[2025-07-23 13:31] contificate: > look at functional languages
> they will look super wacky the first time you see them
you've chosen the wacky languages for your example as well

[2025-07-23 13:32] contificate: Lisp and Haskell, really?

[2025-07-23 13:37] Edel: I like the odin language, would also like to try the jai language but they are pretty similar

[2025-07-23 13:38] Edel: zig seems interesting as well, but I didn't really enjoy using it

[2025-07-23 13:44] contificate: low key, I just love garbage collection

[2025-07-23 13:44] contificate: so nice

[2025-07-23 13:44] contificate: when I was young and bigoted, I thought it was a bad idea

[2025-07-23 13:44] Edel: using arena allocators/linear allocators has made memory management so easy

[2025-07-23 13:44] contificate: yeah but like

[2025-07-23 13:45] contificate: they effectively emulate generational garbage collectors

[2025-07-23 13:45] contificate: and region based lifetimes

[2025-07-23 13:45] contificate: in that every good generational collector's minor heap is bump allocated

[2025-07-23 13:45] contificate: but yeah I agree, I use arena allocators in C constantly

[2025-07-23 13:45] contificate: for almost everything, actually

[2025-07-23 14:35] sariaki: [replying to contificate: "Lisp and Haskell, really?"]
Those and Isabelle are the ones I’ve mainly come into contact with. What lang would you have preferred I show?

[2025-07-23 14:41] contificate: well, I'm just noting

[2025-07-23 14:41] contificate: you seem to have been talking about syntax

[2025-07-23 14:41] contificate: and then show S-expr languages and a language where codegolfing with point-free style is the norm

[2025-07-23 14:42] contificate: there are a whole bunch of FP languages whose syntaxes are not that different, despite being expression orientated

[2025-07-23 14:42] contificate: e.g. OCaml, SML, etc.

[2025-07-23 14:43] contificate: > you ain't seen nothing yet if you think rust is bad
> look at functional languages
interested in what you meant by this

[2025-07-23 14:50] sariaki: I sent those cause i think that people will have heard of Haskell etc. + i'd say that haskell is a fairly typical functional language, no?

[2025-07-23 14:50] sariaki: i mean i've seen it used academically quite a bit - not sure how far its actual use has gone beyond meta spam filtering
edit: damn https://wiki.haskell.org/Haskell_in_industry
Bank of America, AT&T, Barklays Credit Suisse
[Embed: Haskell in industry]

[2025-07-23 14:51] sariaki: maybe i shouldn't have written "functional languages". It just happened to be that the wacky languages were all functional lol

[2025-07-23 14:55] sariaki: [replying to sariaki: "i mean i've seen it used academically quite a bit ..."]
who lotta finance

[2025-07-23 14:55] sariaki: <@687117677512360003> I'm actually interested in this - why are functional languages used in finance so much?

[2025-07-23 14:56] sariaki: i know a lot is also implemented in c++ there but ocaml and haskell seem to be disproportionately represented in finance

[2025-07-23 15:01] contificate: Haskell is actually not typical at all

[2025-07-23 15:02] contificate: it's an exception to the rule, largely because it's lazy

[2025-07-23 15:02] contificate: if you wanted typical FP languages, you're talking like Scala and F#

[2025-07-23 15:03] contificate: they're also not overly represented at all

[2025-07-23 15:03] contificate: some company using an FP language is not telling you anything about the usage of it

[2025-07-23 15:03] contificate: like, at my last job, I used Prolog and GNU Guile

[2025-07-23 15:03] contificate: I could say my company used those languages and that I've written them professionally, lol

[2025-07-23 15:03] contificate: but it's meaningless, it was an off-hand 20% project

[2025-07-23 15:03] sariaki: never have i ever heard of guile

[2025-07-23 15:04] sariaki: yea okay gotcha

[2025-07-23 15:04] contificate: just a Scheme implementation

[2025-07-23 15:04] contificate: anyway, a lot of people like functional programming without actually writing languages that advertise their paradigm as such

[2025-07-23 15:04] Edel: used in the guix package manager/operating system

[2025-07-23 15:04] contificate: modern C#, Java, etc. all have functional-esque ideas and features

[2025-07-23 15:04] sariaki: yea i mean c++ has some stuff in the stdlib

[2025-07-23 15:05] sariaki: and rust also has some features from what i know

[2025-07-23 15:05] contificate: the story with Java is actually depressing

[2025-07-23 15:05] contificate: modern Java has converged upon the creator of Scala's vision from the late 90s/early 2000s, lol

[2025-07-23 15:05] sariaki: i'm all ears!

[2025-07-23 15:05] contificate: https://en.wikipedia.org/wiki/Pizza_(programming_language)
[Embed: Pizza (programming language)]
Pizza is an open-source superset of Java 1.4, prior to the introduction of generics for the Java programming language.  In addition to its own solution for adding generics to the language, Pizza also 

[2025-07-23 15:05] contificate: lol

[2025-07-23 15:05] contificate: ```java
new Lines(new DataInputStream(System.in))
        .takeWhile(nonEmpty)
        .map(fun(String s) -> int { return Integer.parseInt(s); })
        .reduceLeft(0, fun(int x, int y) -> int { return x + y; }));
        while(x == 0) { map.create.newInstance() }
```

[2025-07-23 15:05] contificate: literally what modern Java looks like

[2025-07-23 15:06] contificate: near enough

[2025-07-23 15:06] contificate: this is from like 2001

[2025-07-23 15:06] contificate: and they added streams to actual Java years later

[2025-07-23 15:06] sariaki: damn

[2025-07-23 15:07] sariaki: cool fact

[2025-07-23 21:51] the horse: [replying to Brit: "are the people in the room w you rn?"]
🙏

[2025-07-23 21:51] the horse: C++ needs to deprecate old stuff.

[2025-07-23 21:51] the horse: C++23, 26 is great!

[2025-07-23 21:51] Yoran: [replying to the horse: "C++23, 26 is great!"]
Yeah... Great...

[2025-07-23 21:52] the horse: standardized fmt is a win!

[2025-07-23 21:52] Yoran: [replying to the horse: "standardized fmt is a win!"]
Indeed

[2025-07-23 21:53] the horse: there's a lot of things still unimplemented but the list of proposed features/changes is great

[2025-07-23 21:53] Yoran: But "standardized fmt is a win!" => "C++23, 26 is great!" is a huge strech

[2025-07-23 21:53] the horse: 😢
[Attachments: image.png]

[2025-07-23 21:54] Brit: still no constexpr map

[2025-07-23 21:54] snowua: [replying to the horse: "😢"]
>in standard
>unsupported by every compiler

[2025-07-23 21:54] Brit: in 2025

[2025-07-23 21:54] snowua: ohhhh yeah we are c++ing

[2025-07-23 21:54] Brit: youre a c++ stan snow

[2025-07-23 21:54] the horse: takes time 🙏

[2025-07-23 21:54] Brit: why you piping up

[2025-07-23 21:54] the horse: C++23 is slowly filled up

[2025-07-23 21:54] snowua: am i a stan?

[2025-07-23 21:55] Brit: ye

[2025-07-23 21:55] Brit: one of the stans

[2025-07-23 21:55] Brit: like tajik

[2025-07-23 21:55] Brit: or afghan

[2025-07-23 21:55] Brit: etc

[2025-07-23 21:55] contificate: in 2030, a hello world will take 5 seconds to compile

[2025-07-23 21:55] contificate: thanks cpp

[2025-07-23 21:55] contificate: thanks for introducing more layers of template slop to monomorphise inefficiently every time I compile

[2025-07-23 21:55] contificate: thanks man

[2025-07-23 21:55] the horse: anything in cpp still compiles miles faster than rust

[2025-07-23 21:56] contificate: compiles slow as fuck anyway

[2025-07-23 21:56] the horse: if you have build time issues in 2025 it's your fault

[2025-07-23 21:56] contificate: haha

[2025-07-23 21:56] contificate: no it isn't

[2025-07-23 21:56] Deleted User: i like #embed

[2025-07-23 21:56] the horse: the language offers so many things to solve this

[2025-07-23 21:56] contificate: no it doesn't

[2025-07-23 21:56] the horse: yes, precompiled headers, header/source file actual separation

[2025-07-23 21:56] contificate: you don't know what fast compile times are if you truly believe this

[2025-07-23 21:56] contificate: lmao

[2025-07-23 21:56] contificate: precompiled headers are an MSVC hack

[2025-07-23 21:56] the horse: none of my projects take more than 10s to compile changes

[2025-07-23 21:56] Brit: [replying to Deleted User: "i like #embed"]
it only took the most massive crashout ever

[2025-07-23 21:56] the horse: and im on a i5 9600kf

[2025-07-23 21:56] Brit: for #embed to happen

[2025-07-23 21:56] Brit: <:mmmm:904523247205351454>

[2025-07-23 21:56] contificate: how large are your projects

[2025-07-23 21:57] the horse: if pch isn't available, modules should be a standardized way to handle this

[2025-07-23 21:57] the horse: [replying to contificate: "how large are your projects"]
with or without libs?

[2025-07-23 21:57] contificate: what does this mean

[2025-07-23 21:57] contificate: are they your libs

[2025-07-23 21:57] the horse: external libs

[2025-07-23 21:57] Deleted User: [replying to Brit: "it only took the most massive crashout ever"]
i had to switch to deb to even use it

[2025-07-23 21:57] contificate: oh are you a Windows clown

[2025-07-23 21:57] Deleted User: apple clang wont even support it

[2025-07-23 21:57] contificate: are you building fuckme.lib

[2025-07-23 21:57] contificate: haha

[2025-07-23 21:57] the horse: LMAO

[2025-07-23 21:57] contificate: fuck sake man so embarrassing

[2025-07-23 21:58] contificate: y'know

[2025-07-23 21:58] contificate: when I used to program for the Xbox 360

[2025-07-23 21:58] contificate: people used to like

[2025-07-23 21:58] contificate: upload code to git services

[2025-07-23 21:58] contificate: and they'd include the static libs

[2025-07-23 21:58] contificate: like in the repo

[2025-07-23 21:58] the horse: us windows folks have this nice thing called vcpkg

[2025-07-23 21:58] contificate: this was before that existed

[2025-07-23 21:58] the horse: [replying to contificate: "and they'd include the static libs"]
💔

[2025-07-23 21:59] the horse: I do have a static lib in one of my github repos as well

[2025-07-23 22:00] contificate: clownage

[2025-07-23 22:00] the horse: https://github.com/binsnake/KUBERA this was made radically smaller <30s full recompilation
[Embed: GitHub - binsnake/KUBERA: A x86 CPU & Environment emulator for Wind...]
A x86 CPU & Environment emulator for Windows user and kernel binaries. - binsnake/KUBERA

[2025-07-23 22:00] contificate: I save so much time programming by using good languages that compile fast as fuck and have GC etc.

[2025-07-23 22:00] contificate: lmao 30s

[2025-07-23 22:00] contificate: bro that's crazy long

[2025-07-23 22:01] contificate: I can bootstrap and build the OCaml compiler faster

[2025-07-23 22:01] the horse: relative to rust it's pretty fast

[2025-07-23 22:01] contificate: I assume it's not suicidal

[2025-07-23 22:01] contificate: because of incremental builds

[2025-07-23 22:01] the horse: changes take much less

[2025-07-23 22:01] the horse: yeah

[2025-07-23 22:01] contificate: weak symbols my beloved

[2025-07-23 22:01] contificate: y'know

[2025-07-23 22:01] contificate: the only reason I write C from time to time

[2025-07-23 22:01] the horse: either way any VS project I upon that compiles in >4m I effectively cut down to <45s regularly

[2025-07-23 22:02] contificate: is the sense of accomplishment when I have tons of code

[2025-07-23 22:02] contificate: that compiles extremely quickly

[2025-07-23 22:02] the horse: because it's usually purely poor code structure

[2025-07-23 22:02] the horse: oh to mention, that compiles 4 projects

[2025-07-23 22:02] the horse: not just one

[2025-07-23 22:02] the horse: the 30s

[2025-07-23 22:02] the horse: so linker overhead etc

[2025-07-23 22:02] contificate: see if you want a fun open source contribution

[2025-07-23 22:02] mrexodia: CONFITICATE

[2025-07-23 22:02] contificate: should implement build parallleism opportunities to LLVM

[2025-07-23 22:02] mrexodia: debating

[2025-07-23 22:02] the horse: does LLVM still not support /MP

[2025-07-23 22:03] contificate: nah it's just

[2025-07-23 22:03] contificate: I'm amazed at how large the auto-generated files are

[2025-07-23 22:03] contificate: they `#include` 28MB of generated C++ code into the middle of a class definition

[2025-07-23 22:03] contificate: and then send that off

[2025-07-23 22:03] contificate: to be compiled

[2025-07-23 22:03] the horse: I use a ton of stl dependencies and it does compile pretty fast

[2025-07-23 22:04] the horse: (surprisingly)

[2025-07-23 22:04] contificate: I love how fast OCaml compiles

[2025-07-23 22:04] contificate: so productive

[2025-07-23 22:04] contificate: implemented this entire paper in an evening
[Attachments: 2025-07-21-213746_1453x920_scrot.webp]

[2025-07-23 22:04] contificate: would've taken me quite a bit longer in sloplangs

[2025-07-23 22:05] the horse: it's always good to consider if you're using the right language for the job

[2025-07-23 22:05] the horse: most of things you can write in a small amount of time in higher level languages like go

[2025-07-23 22:05] the horse: vs reimplementing half the libs and functionality in cpp

[2025-07-23 22:06] contificate: I'd wager a lot of people around here aren't using a suitable language

[2025-07-23 22:06] contificate: for random projects they do

[2025-07-23 22:06] contificate: don't get me wrong

[2025-07-23 22:06] contificate: I would never say they should not use C, C++, Rust, for systems level stuff

[2025-07-23 22:06] the horse: I mean hell, I shouldn't speak, I'm working on a website in rust

[2025-07-23 22:06] contificate: but random other shit

[2025-07-23 22:06] contificate: tokio?

[2025-07-23 22:06] the horse: actix(rest) + dioxus(frontend)

[2025-07-23 22:06] contificate: aight

[2025-07-23 22:06] contificate: I'm not familiar so I can't comment

[2025-07-23 22:07] the horse: my service uses tokio

[2025-07-23 22:07] contificate: but I have implemented websockets in C

[2025-07-23 22:07] contificate: surprisingly easy

[2025-07-23 22:07] the horse: websockets by themselves shouldn't be that hard

[2025-07-23 22:08] contificate: well yeah but

[2025-07-23 22:08] contificate: I did the parser by defunctionalising some combinator slop from another language

[2025-07-23 22:08] contificate: long story short

[2025-07-23 22:09] contificate: monadic parser combinators, by virtue of being in continuation passing style, work really well when you have to re-enter/parse incrementally (like in async callbacks when you get random `n` bytes every time the callback is hit)

[2025-07-23 22:09] contificate: whereas in C

[2025-07-23 22:09] the horse: either way; our work is probably entirely different

[2025-07-23 22:09] contificate: you manally implement a state machine

[2025-07-23 22:09] contificate: I don't know what you work on

[2025-07-23 22:09] the horse: emulators, anti-tamper stuff and so on

[2025-07-23 22:09] the horse: uefi

[2025-07-23 22:09] contificate: seems suitable for C++

[2025-07-23 22:09] contificate: what kind of anti-tamper

[2025-07-23 22:10] the horse: for DRM purposes mainly; user attestation, client-side code stripping for remote emulation, custom loaders for NT-related crap

[2025-07-23 22:10] the horse: preventing modifications (and accordingly reporting these anomalies)

[2025-07-23 22:11] the horse: also looking into binary rewriting a bit because the Windows solutions are absolutely dogshit

[2025-07-23 22:11] the horse: any decent project has barely ELF support

[2025-07-23 22:11] the horse: and if it's for windows, exceptions are off the table and so on

[2025-07-23 22:11] contificate: fuck binary rewriting

[2025-07-23 22:11] contificate: I dunno why everyone gets a hard-on for it

[2025-07-23 22:11] the horse: For SaaS/EV solutions it's probably the only reasonable thing

[2025-07-23 22:12] the horse: without forcing developers to make changes in their codebase

[2025-07-23 22:12] contificate: well, only because they're unlikely to move to a more useful compiler

[2025-07-23 22:12] the horse: or using patches for their compilers

[2025-07-23 22:12] contificate: but y'know

[2025-07-23 22:12] contificate: clang is like first class in VS I've heard

[2025-07-23 22:12] the horse: ollvm by itself is an unstable mess

[2025-07-23 22:12] contificate: nobody is saying ollvm

[2025-07-23 22:12] contificate: that's amateur stuff

[2025-07-23 22:12] the horse: just threw an example

[2025-07-23 22:12] the horse: in the ideal world I'd learn LLVM and work on bitcode directly

[2025-07-23 22:12] the horse: but that's some time away

[2025-07-23 22:13] contificate: should learn LLVM

[2025-07-23 22:13] contificate: it's not a huge undertaking

[2025-07-23 22:13] the horse: yeah; atm just learning the internals of x86 instructions; lifting them and emulating to get the grounds up

[2025-07-23 22:13] the horse: ultimate goal is a versatile framework for both obfuscation and deobfuscation

[2025-07-23 22:13] the horse: but the prerequisites to make this happen and not be completely dogshit are pretty high usually

[2025-07-23 22:14] the horse: got like 3-4 more years to make bank before EU collapses 💪

[2025-07-23 22:34] contificate: there's something been on my mind for many years in that domain

[2025-07-23 22:34] contificate: but it's so much work for a PoC

[2025-07-23 22:34] contificate: that it'll never happen

[2025-07-23 22:34] Yoran: [replying to Brit: "still no constexpr map"]
Hit hard

[2025-07-23 23:22] selfprxvoked: [replying to the horse: "I use a ton of stl dependencies and it does compil..."]
STL includes are probably optimized to be compiled fast tbh

[2025-07-23 23:34] Brit: boy do I have some news for you

[2025-07-23 23:44] the horse: [replying to selfprxvoked: "STL includes are probably optimized to be compiled..."]
well, in effect they're only as expensive as the amount of variations you use of them

[2025-07-23 23:45] the horse: just including them essentially has no effect besides intellisense

[2025-07-23 23:45] the horse: then there's additional overhead on the templated instances

[2025-07-23 23:45] the horse: because the memory layout changes, therefore it puts out more versions of it leading to higher compilation times

[2025-07-23 23:46] the horse: once you move away from primitive types that's when it gets a bit worse

[2025-07-23 23:51] selfprxvoked: [replying to Brit: "boy do I have some news for you"]
I'm just separating things out

[2025-07-23 23:51] selfprxvoked: I know that they are optimized for a lot of reasons

[2025-07-23 23:51] selfprxvoked: but it is way beyond that

[2025-07-23 23:52] selfprxvoked: just like ricochet explained

[2025-07-24 01:23] UJ: [replying to UJ: "i could try that. it hasn't been an issue *so far*..."]
speak of the devil.
[Attachments: Screenshot_2025-07-23_182218.png]

[2025-07-24 02:06] Matti: I cannae read this shit

[2025-07-24 02:07] Matti: is it possible discord is raping the resolution? this happened to me just today as well https://discord.com/channels/835610998102425650/835646666858168320/1397454866909106176

[2025-07-24 02:07] Matti: though I don't see width or height parameters in the url

[2025-07-24 02:09] daax: [replying to UJ: "speak of the devil."]
Can you link the post or copy it here?

[2025-07-24 02:15] UJ: [replying to Matti: "is it possible discord is raping the resolution? t..."]
yeah this is weird, discord is doing something wierd with it. in your post its blurry when i open it and right click and open in new tab but then when i remove width and height from the url i can read it properly (2500x10000 resolution)

[2025-07-24 02:17] Matti: I'm pretty sure yours is just too low resolution for the asm to be readable honestly

[2025-07-24 02:18] Matti: I aaved the original at 2232x487 and that isn't enough

[2025-07-24 02:18] Matti: [replying to daax: "Can you link the post or copy it here?"]
second this

[2025-07-24 02:18] UJ: test

[2025-07-24 02:18] Matti: or copy the two images in that post separately

[2025-07-24 02:19] Matti: yeah that's better

[2025-07-24 02:20] Matti: ruh-roh

[2025-07-24 02:20] Matti: I see hyperdbg code

[2025-07-24 02:20] UJ: even tho this ended up not being a bug in msvc, do you know what this bug is referring to? see the tweet about the ps3 emulator - https://x.com/0Xiphorus/status/1948062027156426947

[2025-07-24 02:21] UJ: or know of any recent bugs in msvc?

[2025-07-24 02:21] Matti: uhhhh

[2025-07-24 02:21] daax: > Doesn't look like its a bug in msvc. In the optimized build, rbx is getting clobbered out of band with the AsmVmxSupportDetection  call it looks like but msvc still thinks its 0 at the point of comparison so it uses that to cmp device with 0x0. In the unoptimized build, its comparing the memory location to 0x0 directly. 
MSVC has done this for ages and it's usually the result of UB in the binary; it's also a big reason I stopped using MSVC -- it's a piece of shit.

[2025-07-24 02:21] Matti: ++++++

[2025-07-24 02:22] Matti: what bug ISN'T in MSVC

[2025-07-24 02:22] Matti: they have all the bugs

[2025-07-24 02:22] Matti: ones you couldn't even think of

[2025-07-24 02:23] Matti: though I mean, generating fucked up code on UB isn't a *bug*

[2025-07-24 02:23] Matti: it's just shit

[2025-07-24 02:23] daax: [replying to Matti: "though I mean, generating fucked up code on UB isn..."]
right

[2025-07-24 02:24] UJ: yeah the hyperdbg issue is just user error. im just wondering what bug in msvc [they ](https://x.com/rpcs3/status/1946069204282565100) are referring to now.

[2025-07-24 02:24] Matti: a godbolt link in the tweet would've been nice...

[2025-07-24 02:25] UJ: at least they have some good news. `We plan to switch to using the Clang compiler on Windows`

[2025-07-24 02:26] Matti: MSVC is shit but this sort of post definitely makes me want to play 'spot the UB' when someone makes claims like this

[2025-07-24 02:26] daax: [replying to UJ: "at least they have some good news. `We plan to swi..."]
it's an improvement but Clang also... has it's own set of rage inducing bugs

[2025-07-24 02:27] daax: here is a good and entertaining post by <@835638356624801793> about compiler issues btw <https://secret.club/2024/10/21/unnecessarily-exhaustice-rca.html>

[2025-07-24 02:28] Matti: [replying to Matti: "MSVC is shit but this sort of post definitely make..."]
source of my infinite wisdom: having worked with unreal engine

[2025-07-24 02:29] Matti: every new MSVC release blows up somewhere with a new ICE

[2025-07-24 02:29] Matti: it's rarely a compiler bug

[2025-07-24 02:30] Matti: more like a regression in how UB is being handled

[2025-07-24 02:30] Matti: and clang does the same

[2025-07-24 02:33] Matti: [replying to Matti: "more like a regression in how UB is being handled"]
which to be pedantic is sort of a compiler bug too... but really only in the most trivial 'akshually' meaning of the word

[2025-07-24 02:35] Matti: when working with a language that features insane things such as UB, it's just more practical to try to avoid UB at all costs rather than depending on the compiler to 'get it right' for you (whatever that means - could be generating the code you actually meant to write, could be throwing an error)

[2025-07-24 03:39] the horse: crazy workaround
[Attachments: image.png]

[2025-07-24 04:22] UJ: yea, they did this before i told them the root cause. (when they thought it was a msvc bug). it should be fixed soon.

[2025-07-24 07:44] UJ: [replying to Matti: "a godbolt link in the tweet would've been nice..."]
this is it, it seems and has a repro as well - https://developercommunity.visualstudio.com/t/Code-optimization-bug-SIMD-std::transf/10912292

[2025-07-27 02:29] UJ: its fixed in dev - https://github.com/HyperDbg/HyperDbg/tree/dev

[2025-07-27 02:30] UJ: too bad the usermode debugger didnt make this cut as well. it compiles with it disabled unless you build it yourself.

[2025-07-27 03:51] the horse: anyone has an idea on how I could make the buttons here look better on mobile?
[Attachments: image.png]

[2025-07-27 03:51] the horse: left/right/center doesn't look good

[2025-07-27 03:52] the horse: <@1340811848022360096> queen you must know

[2025-07-27 03:52] the horse: 💔

[2025-07-27 03:56] the horse: thought you might have an eye for design

[2025-07-27 03:57] the horse: 
[Attachments: image.png]

[2025-07-27 03:57] the horse: this will have to do

[2025-07-27 04:02] the horse: hmmm ye

[2025-07-27 04:02] the horse: ty

[2025-07-27 04:03] the horse: epic
[Attachments: image.png]

[2025-07-27 04:05] the horse: hmm is 5 pixels away

[2025-07-27 04:05] the horse: like

[2025-07-27 04:05] the horse: 5px more than left status icon

[2025-07-27 16:09] avx: looks neat

[2025-07-27 19:24] Timmy: why not use something like uptime kuma instead of rolling your own?