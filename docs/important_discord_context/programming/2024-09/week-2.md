# September 2024 - Week 2
# Channel: #programming
# Messages: 349

[2024-09-02 00:10] diversenok: You can

[2024-09-02 00:10] diversenok: Do you have enough access on the process handle?

[2024-09-02 00:10] Deleted User: ~~the handle gets returned not given~~ thought you we're talking about ThreadHandle <:facepalm:654412877687685130>

[2024-09-02 00:10] projizdivlak: calling it from already injected image, using current process

[2024-09-02 00:12] projizdivlak: also in loader lock, shouldn't be a problem tho

[2024-09-02 00:13] Deleted User: which process are you trying to create a thread

[2024-09-02 00:13] diversenok: I only tried using it to create a thread that shares the stack with the existing one, but it worked

[2024-09-02 00:13] projizdivlak: [replying to Deleted User: "which process are you trying to create a thread"]
from a game process, from dllmain, the game isn't protected

[2024-09-02 00:15] projizdivlak: `RtlCreateUserThread` and `NtCreateThreadEx` with the same parameters works tho

[2024-09-02 00:15] diversenok: They hardly have the same parameters 😅

[2024-09-02 00:16] projizdivlak: i meant like not modified

[2024-09-02 00:20] Deleted User: you're problem is something related to the process

[2024-09-02 00:20] Deleted User: EPROCESS.MitigationFlags2

[2024-09-02 00:20] Deleted User: 
[Attachments: image.png]

[2024-09-02 00:21] Deleted User: 
[Attachments: image.png]

[2024-09-02 00:21] diversenok: Is CFG enabled?

[2024-09-02 00:21] Deleted User: yea it is one of the checked bits aka "ControlFlowGuardEnabled", the first 2 checks

[2024-09-02 00:29] projizdivlak: it's compiled with msvc 6.0, runs on windows 95 and has no cfg, i don't think it even existed back then

[2024-09-02 00:34] diversenok: Fair

[2024-09-02 03:21] Hunter: ```cpp
if (...
    current_process->control_flow_guard_enabled ||
    target_process->control_flow_guard_enabled ||
    current_process->cet_user_shadow_stacks ||
    target_process->cet_user_shadow_stacks) 
  return STATUS_ACCESS_DENIED;
```

[2024-09-02 14:13] Deleted User: hello

[2024-09-02 14:15] Deleted User: what is the best way to emulate winapi functions in unicorn?
I am thinking is to take the address statically and map a fake function at that address.
but I am pretty sure there is a better method

[2024-09-02 14:17] Deleted User: or just do it dynamically, should be better to do it dynamically

[2024-09-02 14:24] Brit: https://github.com/mrexodia/dumpulator
[Embed: GitHub - mrexodia/dumpulator: An easy-to-use library for emulating ...]
An easy-to-use library for emulating memory dumps. Useful for malware analysis (config extraction, unpacking) and dynamic analysis in general (sandboxing). - mrexodia/dumpulator

[2024-09-02 14:25] Brit: https://github.com/mrexodia/dumpulator/blob/main/src/dumpulator/ntsyscalls.py
[Embed: dumpulator/src/dumpulator/ntsyscalls.py at main · mrexodia/dumpulator]
An easy-to-use library for emulating memory dumps. Useful for malware analysis (config extraction, unpacking) and dynamic analysis in general (sandboxing). - mrexodia/dumpulator

[2024-09-02 14:26] Deleted User: Thanks Alot!

[2024-09-03 01:36] hxm: is there any fast way to make all thes into hex

[2024-09-03 01:36] hxm: 
[Attachments: vmware_2inUTlhsGc.png]

[2024-09-03 03:19] Torph: wdym?

[2024-09-03 04:54] sariaki: Oh like at once?

[2024-09-03 05:35] daax: [replying to hxm: ""]
edit > plugins > hexrays decompiler > settings > radix = 16

[2024-09-03 05:35] daax: f5

[2024-09-04 16:17] luci4: So I was experimenting a bit with stack encryption for sleep obfuscation (not the best approach but eh). While it works fine, I keep hitting the guard page and expanding the stack:

```
    PBYTE Start = Tib->StackLimit;
    SIZE_T StackSize = (SIZE_T) Tib->StackBase - (SIZE_T) Tib->StackLimit;

    for(DWORD Index = 0; Index < StackSize; Index++) {
        Start[Index] ^= 0x69;
    }
```

I'm pretty sure this issue is caused by me starting at `StackLimit`, so I've tried increasing the pointer `PBYTE((ULONG_PTR)Tib->StackLimit + 100)`  and substracting from `StackSize`, to not much success

[2024-09-04 16:19] luci4: Is there a specific threshold I am missing?

[2024-09-04 16:43] Hunter: [replying to luci4: "So I was experimenting a bit with stack encryption..."]
try stacklimit + 0x1000

[2024-09-04 16:43] Hunter: and calculate the stack size through stackbase - start instead

[2024-09-04 17:14] projizdivlak: [replying to luci4: "So I was experimenting a bit with stack encryption..."]
```cpp
SIZE_T StackHigh = (SIZE_T)NtCurrentTeb()->NtTib.StackLimit + SystemBasicInfo.PageSize; /* TODO: Check if guard page is present */
SIZE_T StackLow = (SIZE_T)NtCurrentTeb()->NtTib.StackBase;
ULONG StackSize = StackLow - StackHigh;
```
what about this

[2024-09-04 17:26] luci4: [replying to Hunter: "and calculate the stack size through stackbase - s..."]
This leads to stacksize being 0

[2024-09-04 17:26] luci4: So I'm encrypting a stack that starts at the default size

[2024-09-04 17:26] luci4: which is 0x1000

[2024-09-04 18:00] 冰: hmm

[2024-09-04 18:02] contificate: yes

[2024-09-04 18:02] contificate: I don't think the underlying structure matters too much

[2024-09-04 18:03] contificate: my memory of robin hood hashing is just the cute result that all your effort means you can short circuit searches early

[2024-09-04 18:16] 冰: [replying to contificate: "yes"]
hmm

[2024-09-04 18:18] Deleted User: mmh, might be in his book

[2024-09-04 18:18] Deleted User: "art of programming"

[2024-09-04 18:20] Deleted User: nvm, I looked through the TOC and didn't find anything particularly for hashmaps

[2024-09-04 18:22] Deleted User: [replying to 冰: "hmm"]
>  Is it possible to use 32 bits instead of 64 bits per entry
yeah, compile with ``-m32`` /s

[2024-09-04 18:23] Deleted User: [replying to 冰: "hmm"]
What do you mean by the former question?

[2024-09-04 18:23] Deleted User: [replying to 冰: "hmm"]
Where did you find this btw?

[2024-09-04 18:26] 冰: [replying to Deleted User: ">  Is it possible to use 32 bits instead of 64 bit..."]
hmm

[2024-09-04 18:26] 冰: [replying to Deleted User: "Where did you find this btw?"]
hmm

[2024-09-04 18:33] Deleted User: [replying to 冰: "hmm"]
/s means sarcasm, I was just messing around

[2024-09-04 18:35] Deleted User: [replying to 冰: "hmm"]

[Attachments: image.png]

[2024-09-04 18:35] Deleted User: this section here tells you about it

[2024-09-04 18:36] Deleted User: 
[Attachments: image.png]

[2024-09-04 18:36] Deleted User: fwiw, it's 4 bytes man

[2024-09-04 18:36] Deleted User: it doesn't matter

[2024-09-04 18:37] Brit: 4 bytes per entry

[2024-09-04 18:37] Deleted User: robin hood hashing looks cool, but it seems specialized for cases where more times than not you query for something that isn't there and you have no information about the data you're querying so you just have to take a shot to the face and accept most of your queries are for non-existent keys

[2024-09-04 18:37] Brit: it's not just "4 bytes lmao"

[2024-09-04 18:37] Brit: it's *n

[2024-09-04 18:38] Deleted User: [replying to Brit: "4 bytes per entry"]
4 bytes per entry :(

[2024-09-04 18:38] Deleted User: fuck man, we're wasting bytes with all this shit, just store everything in a 64-bit integer and conserve some space! /s

[2024-09-04 18:38] 冰: hmm

[2024-09-04 18:38] Deleted User: yeah I know

[2024-09-04 18:39] Brit: [replying to Deleted User: "fuck man, we're wasting bytes with all this shit, ..."]
true

[2024-09-04 18:39] Deleted User: [replying to 冰: "hmm"]
but read the comments and replies they talk about some cool shit

[2024-09-04 18:39] Deleted User: also read the original article linked

[2024-09-04 18:41] Deleted User: [replying to Brit: "true"]
who's that in your pfp btw

[2024-09-04 18:41] Deleted User: kinda looks like von neumann

[2024-09-04 18:42] Brit: it is the man himself

[2024-09-04 18:42] Brit: I should upgrade to erdös he is less known

[2024-09-04 18:42] Brit: mildly

[2024-09-04 18:42] Brit: by image anyways

[2024-09-04 18:42] Brit: or maybe Villany

[2024-09-04 18:44] Deleted User: [replying to Brit: "I should upgrade to erdös he is less known"]
lesser known by anyone who hasn't taken discrete math

[2024-09-04 18:44] Deleted User: guy has like 1500 fuckin conjectures with his name plastered all over discrete math

[2024-09-04 18:45] Brit: he's the most cited mathematician still I think

[2024-09-04 18:45] Brit: ofc he's known but how many people can put a face on that name

[2024-09-04 18:46] Deleted User: [replying to Brit: "he's the most cited mathematician still I think"]
yeah, bro wrote like over a 1000 papers on math

[2024-09-04 18:48] Deleted User: maybe more, idk

[2024-09-04 18:48] Deleted User: I only heard about the dude recently in a youtube video about ramsey numbers

[2024-09-04 18:48] Deleted User: https://www.youtube.com/watch?v=4HHUGnHcDQw this one
[Embed: Biggest Breakthroughs in Math: 2023]
Quanta Magazine’s mathematics coverage in 2023 included landmark results in Ramsey theory and a remarkably simple aperiodic tile capped a year of mathematical delight and discovery.

Read about more m

[2024-09-04 18:48] Deleted User: https://www.quantamagazine.org/after-nearly-a-century-a-new-limit-for-patterns-in-graphs-20230502/ - and this article too
[Embed: After Nearly a Century, a New Limit for Patterns in Graphs | Quanta...]
Four mathematicians have found a new upper limit to the “Ramsey number,” a crucial property describing unavoidable structure in graphs.

[2024-09-04 18:53] contificate: [replying to 冰: "hmm"]
you ever heard of xor linked lists

[2024-09-04 18:56] contificate: it comes to mind but I dunno how you'd verify the thing

[2024-09-04 19:04] contificate: wondering what this "trick" is

[2024-09-04 19:05] BWA RBX: Must save a bunch of space

[2024-09-04 19:07] BWA RBX: [replying to contificate: "you ever heard of xor linked lists"]
Never used them before, I can see why it would be beneficial but is it practical for use in real world scenarios

[2024-09-04 19:07] contificate: it's not practical at all

[2024-09-04 19:08] contificate: a general issue with them for GC'd languages is that you fuck up root scanning if you fuck with pointers

[2024-09-04 19:08] contificate: and also who gives a shit about space

[2024-09-04 19:08] BWA RBX: I mean yeah but you get those people that love to benchmark

[2024-09-04 19:08] contificate: just remember the granularity at which block devices write (ignore fact they don't sync/commit every write immediately)

[2024-09-04 19:08] BWA RBX: That would suggest shit like that

[2024-09-04 19:08] contificate: and have a pint

[2024-09-04 19:08] contificate: and settle down

[2024-09-04 19:15] Deleted User: [replying to BWA RBX: "Never used them before, I can see why it would be ..."]

[Attachments: image.png]

[2024-09-04 19:15] Deleted User: they have a ton of problems

[2024-09-04 19:34] contificate: imagine if all along

[2024-09-04 19:34] contificate: knuth's "trick"

[2024-09-04 19:34] contificate: was "we store 16 bits for the hash and 16 bits for the distance"

[2024-09-04 19:35] contificate: "thus we have reduced space usage from 8 bytes to 4 bytes"

[2024-09-04 19:35] contificate: this claim seems to come from a hacker news comment

[2024-09-04 19:42] 冰: [replying to contificate: "you ever heard of xor linked lists"]
hmm

[2024-09-04 19:45] contificate: possible

[2024-09-04 19:45] contificate: I forget what context you need to store

[2024-09-04 19:45] contificate: and what can be computed on the fly

[2024-09-04 19:45] contificate: been several years since I implemented robin hood hashing

[2024-09-04 19:59] Deleted User: [replying to contificate: "was "we store 16 bits for the hash and 16 bits for..."]
mmmh, that does seem reasonable

[2024-09-04 19:59] Deleted User: cool idea

[2024-09-04 20:02] contificate: lmao

[2024-09-04 20:02] Deleted User: is something funny, fucker

[2024-09-04 20:02] Deleted User: how do you know rai

[2024-09-04 20:06] contificate: no idea

[2024-09-04 20:06] contificate: I assume you are referring to our mutual friend on Discord

[2024-09-04 20:06] contificate: all I will say on the matter is

[2024-09-04 20:06] contificate: I've never sent a FR on Discord

[2024-09-04 20:06] contificate: but I tend to accept ones I receive

[2024-09-04 20:07] contificate: still waiting for a nigerian prince to give me an offer I can't ignore

[2024-09-04 20:08] Deleted User: omg you're so special and different, I wanna grow up and be just like you one day...

[2024-09-04 20:08] Deleted User: [replying to contificate: "no idea"]
mmh, okay

[2024-09-04 20:10] contificate: [replying to Deleted User: "omg you're so special and different, I wanna grow ..."]
I sure am

[2024-09-04 20:10] contificate: I can give you tips to be just like me when you grow up

[2024-09-04 20:10] Deleted User: sure, let's hear it

[2024-09-04 20:10] contificate: drink tons of pepsi max in your youth and don't brush teeth even nearly enough

[2024-09-04 20:10] contificate: never learn to code

[2024-09-04 20:10] contificate: put on weight that you can't get rid of

[2024-09-04 20:11] contificate: continue to damage eyesight to worsen already bad prescription

[2024-09-04 20:11] Deleted User: give me a second to jot this down

[2024-09-04 20:12] Deleted User: Is that all?

[2024-09-04 20:12] Deleted User: Is the pepsi max optional such that I can replace it with something else? Such as root beer?

[2024-09-04 20:12] Deleted User: Not a big pepsi fan

[2024-09-04 20:13] contificate: anything acidic

[2024-09-04 20:16] 冰: hmm

[2024-09-04 20:20] contificate: gotta be careful though

[2024-09-04 20:21] contificate: as mixing bleach with urine will produce ammonia in your mouth

[2024-09-04 20:22] Brit: don't you know the rule, don't mix any chemical with bleach

[2024-09-04 20:25] contificate: false

[2024-09-04 20:26] contificate: don't dissuade the members here from ingesting this charming elixir

[2024-09-04 20:30] Brit: make nice crystals by mixing bleach and ammonia and a copper coin

[2024-09-04 20:39] Deleted User: [replying to contificate: "anything acidic"]
just making sure

[2024-09-04 20:42] Deleted User: [replying to Brit: "make nice crystals by mixing bleach and ammonia an..."]
you can make something even cooler when mixing bleach with some vinegar

[2024-09-04 20:42] Deleted User: very good for the lungs

[2024-09-04 21:13] projizdivlak: where should a thread return to after it's done? trying out creating thread without ip and parameter, then setting it manually using context

[2024-09-04 21:16] Brit: depends where you're starting the thread?

[2024-09-04 21:16] Brit: and what you want it to do?

[2024-09-04 21:26] projizdivlak: i mean anywhere, user code, and i'm just curious

[2024-09-04 21:28] projizdivlak: i *could* just terminate the thread at end, ig that's what the api does anyways

[2024-09-04 21:31] contificate: thread? return to?

[2024-09-04 21:32] contificate: typically you explicitly _join_ a thread from its parent

[2024-09-04 21:32] contificate: which blocks until the child finishes

[2024-09-04 21:32] contificate: is that what you mean?

[2024-09-04 21:32] contificate: in multithreaded programs that are doing synchronous processing of things

[2024-09-04 21:32] contificate: that'll be whenever you want the thread's output/work

[2024-09-04 21:33] Brit: you're talking about something completely different

[2024-09-04 21:33] contificate: is this some WinAPI shit

[2024-09-04 21:33] Brit: this is windows lore

[2024-09-04 21:33] Brit: yes

[2024-09-04 21:33] contificate: sigh

[2024-09-04 21:33] projizdivlak: lmao

[2024-09-04 21:34] projizdivlak: i meant literally return as `ret` instruction

[2024-09-04 21:35] projizdivlak: this is what i came up with
```cpp
DECLSPEC_NORETURN VOID NTAPI StartUserThread(IN PUSER_THREAD_START_ROUTINE StartAddress, IN PVOID Parameter OPTIONAL)
{
    RtlExitUserThread(StartAddress(Parameter));
}
```
do stuff and set ip here

[2024-09-04 21:36] vendor: wut

[2024-09-04 21:38] vendor: there is a function called something like RtlUserStartThread where regular threads created by kernel32 level APIs are started at. when the thread is done it returns to this code in ntdll which exits the thread. all of this you can debug easily and see the normal flow.

[2024-09-04 21:40] projizdivlak: [replying to vendor: "there is a function called something like RtlUserS..."]
yeah this is what i was looking for, implementing it myself

[2024-09-04 21:40] vendor: it doesn’t make much sense to do it like you say unless you want to set additional parameters but even then using a stub to unpack and pass params would probably make more sense

[2024-09-04 21:41] projizdivlak: [replying to vendor: "it doesn’t make much sense to do it like you say u..."]
it isn't in older versions of windows

[2024-09-04 21:42] vendor: wdym

[2024-09-04 21:42] projizdivlak: not exported

[2024-09-04 21:52] x86matthew: [replying to projizdivlak: "i *could* just terminate the thread at end, ig tha..."]
general flow is: `LdrInitializeThunk -> NtContinue (to RtlUserThreadStart) -> BaseThreadInitThunk -> (thread entry point) -> returns to BaseThreadInitThunk and calls RtlExitUserThread`

[2024-09-04 21:52] x86matthew: you can't skip LdrInitializeThunk but you can override the call to RtlUserThreadStart by changing the context which i'm guessing is what you're talking about

[2024-09-04 21:53] luci4: [replying to x86matthew: "general flow is: `LdrInitializeThunk -> NtContinue..."]
Why is the NtContinue step necessary?

[2024-09-04 21:54] x86matthew: but you can't just return from your stub and expect the thread to exit cleanly if you override the context, there would be no return address on the stack to return to, you need to explicitly exit the thread (as you've probably found)

[2024-09-04 21:56] x86matthew: [replying to luci4: "Why is the NtContinue step necessary?"]
LdrInitializeThunk is the usermode entrypoint for a new thread (from the kernel)

[2024-09-04 21:56] Matti: [replying to luci4: "Why is the NtContinue step necessary?"]
because LdrInitializeThunk is started in a pretty funky manner using a user APC

[2024-09-04 21:57] luci4: [replying to Matti: "because LdrInitializeThunk is started in a pretty ..."]
Afaik when a thread is created Windows queues LdrInitializeThunk as an APC

[2024-09-04 21:57] Matti: yes

[2024-09-04 21:57] luci4: Oh I didnt know you could queue user APCs like that

[2024-09-04 21:58] luci4: That's interesting

[2024-09-04 21:58] luci4: Thank you!

[2024-09-04 21:59] Matti: you can yeah, but this isn't how user threads normally execute

[2024-09-04 22:00] Matti: only the initial LdrInitializeThunk part is (for reasons I don't know/remember) queued as a user APC directly from the kernel

[2024-09-04 22:01] Matti: RtlUserThreadStart (or your alternative entry point of preference) continue as regular UM threads after the NtContinue call

[2024-09-04 22:01] Matti: it's been a while since I looked at this, sorry for the lack of specifics but yeah

[2024-09-04 22:02] luci4: I remember LdrpInitialize doing a LOT of shit, like initializing the security cookie, something for hot patching

[2024-09-04 22:03] luci4: It was so much I kinda got bored of it half-through

[2024-09-04 22:03] Matti: another interesting-ish thing to note is that there is not really a concept of an 'initial thread' as far as NT is concerned*

*yes there is a bit in the PEB for this, but it's only for ntdll and I don't even know what if anything it affects

[2024-09-04 22:04] Matti: so, you can suspend the first thread, create a second one, kill the initial thread, and then continue execution

[2024-09-04 22:04] Matti: (you can't kill the thread first because killing the last thread in a process kills the process)

[2024-09-04 22:07] luci4: [replying to Matti: "so, you can suspend the first thread, create a sec..."]
That's pretty interesting!

[2024-09-04 22:07] luci4: For my hollower I just changed the context of the "main" thread

[2024-09-04 22:08] luci4: <https://github.com/l00sy4/Tiburon/blob/d650a61845ac8f17fcbdc01df9dd4c9425eaa038/Tiburon/Tiburon/HollowingVariations.cpp#L15>

[2024-09-04 22:08] Matti: yep, that works

[2024-09-04 22:09] Matti: and tbh I don't really see a benefit to killing the initial thread

[2024-09-04 22:09] luci4: Although now that I look back I could have used NtContinue to change the context AND resume execution

[2024-09-04 22:09] Matti: it was more of an interesting factoid, no more

[2024-09-04 22:09] luci4: [replying to luci4: "Although now that I look back I could have used Nt..."]
Pe-sieve catches it because the name of the module doesn't correspond to the section 😦

[2024-09-04 22:10] luci4: I tried some COW bs but couldn't get it to work unfortunately

[2024-09-04 22:10] luci4: [replying to Matti: "it was more of an interesting factoid, no more"]
Noted!

[2024-09-04 22:10] luci4: I wonder how you figured that out tbh

[2024-09-04 22:11] diversenok: [replying to Matti: "and tbh I don't really see a benefit to killing th..."]
Maybe an antivirus that queues an APC to the first thread

[2024-09-04 22:11] diversenok: To load its DLL

[2024-09-04 22:11] Matti: well I had to implement the NtCreateUserProcess and NtCreateThreadEx syscalls in my WRK fork <:harold:704245193016344596>

[2024-09-04 22:11] x86matthew: it kinda comes naturally when you realise a process only ends when there are no threads remaining

[2024-09-04 22:11] Matti: so I was forced to

[2024-09-04 22:11] Matti: ^ and that

[2024-09-04 22:12] Deleted User: 🤨

[2024-09-04 22:12] x86matthew: [replying to diversenok: "Maybe an antivirus that queues an APC to the first..."]
i used it for exactly this once, a silly bypass to prevent the DLL loading

[2024-09-04 22:12] Matti: interesting factoid #2 (most people here probably know this...): you *can* create a process with 0 threads

[2024-09-04 22:12] Matti: you just can't make a process have 0 threads after it's had 1

[2024-09-04 22:13] diversenok: Oh, Matti, do you know about two-phase process termination?

[2024-09-04 22:13] Matti: no!

[2024-09-04 22:13] Matti: enlighten me

[2024-09-04 22:13] diversenok: `NtTerminateProcess(NULL, status)` is not the same as `NtTerminateProcess(NtCurrentProcess(), status)`

[2024-09-04 22:13] luci4: [replying to Matti: "well I had to implement the NtCreateUserProcess an..."]
https://github.com/HighSchoolSoftwareClub/Windows-Research-Kernel-WRK-
[Embed: GitHub - HighSchoolSoftwareClub/Windows-Research-Kernel-WRK-: Windo...]
Windows Research Kernel Source Code. Contribute to HighSchoolSoftwareClub/Windows-Research-Kernel-WRK- development by creating an account on GitHub.

[2024-09-04 22:13] luci4: You mean this?

[2024-09-04 22:14] Matti: [replying to luci4: "https://github.com/HighSchoolSoftwareClub/Windows-..."]
yeah, my fork of this

[2024-09-04 22:14] luci4: [replying to Matti: "interesting factoid #2 (most people here probably ..."]
A minimal process, right?

[2024-09-04 22:14] Matti: [replying to diversenok: "`NtTerminateProcess(NULL, status)` is not the same..."]
hmmm I think I see where you're going with this

[2024-09-04 22:14] Matti: vaguely

[2024-09-04 22:14] Matti: because yes I do know that part

[2024-09-04 22:14] diversenok: `NtTerminateProcess(NULL, status)` kills all threads except the calling one and marks the process for self-delete

[2024-09-04 22:15] Matti: yep yep

[2024-09-04 22:15] Matti: I remember now

[2024-09-04 22:15] diversenok: It also sets the process exit status, even though the object is not signalled yet

[2024-09-04 22:15] Matti: oh that's interesting

[2024-09-04 22:15] Matti: [replying to luci4: "A minimal process, right?"]
uhm I think probably yes - but unsure

[2024-09-04 22:15] Matti: the way most people would make one is using `NtCreateProcess[Ex]`

[2024-09-04 22:15] mrexodia: [replying to luci4: "I remember LdrpInitialize doing a LOT of shit, lik..."]
I did a little lecture about it

[2024-09-04 22:17] Matti: [replying to Matti: "the way most people would make one is using `NtCre..."]
this is the pre-vista API for creating a process, just like how `NtCreateThread` (non-Ex) is the pre-vista API for creating a thread

[2024-09-04 22:17] Matti: you really don't want to use these normally if it can be helped

[2024-09-04 22:17] diversenok: [replying to diversenok: "It also sets the process exit status, even though ..."]
Trying to create more threads in such process results in `STATUS_PROCESS_IS_TERMINATING`, except if you pass the `THREAD_CREATE_FLAGS_SKIP_THREAD_ATTACH` flag, then it's fine

[2024-09-04 22:18] diversenok: [replying to Matti: "you really don't want to use these normally if it ..."]
Well, process snapshotting is still a supported operation, it uses `NtCreateProcessEx`

[2024-09-04 22:18] Matti: yeah sure

[2024-09-04 22:18] Matti: I just meant calling them manually because they're so fun and easy to use

[2024-09-04 22:18] Matti: especially NtCreateThread

[2024-09-04 22:19] Matti: [replying to diversenok: "Trying to create more threads in such process resu..."]
ok this is pretty crazy

[2024-09-04 22:19] diversenok: [replying to Matti: "I just meant calling them manually because they're..."]
Ahh, yeah, definitely

[2024-09-04 22:19] luci4: [replying to Matti: "the way most people would make one is using `NtCre..."]
Oh. Task failed succesfully

[2024-09-04 22:19] Matti: so - then you can half-terminate the process, and create a new thread that will execute normally?

[2024-09-04 22:20] diversenok: The calling one also continues fine

[2024-09-04 22:20] Matti: ah yeah

[2024-09-04 22:20] Matti: but regardless, that's pretty crazy

[2024-09-04 22:21] Matti: mm, but if you don't create a new thread, does self-delete actually self-delete? automatically? cause that's the behaviour I would expect

[2024-09-04 22:22] Matti: and eventually of course

[2024-09-04 22:22] Matti: I know it's not instant

[2024-09-04 22:22] diversenok: This part I don't know

[2024-09-04 22:22] diversenok: Maybe on thread exit? But that's normal anyway

[2024-09-04 22:23] diversenok: I can check if there is a timeout, but I'm having hard time imagining how they would implement that

[2024-09-04 22:24] diversenok: The funny thing is a process can change it's exit status twice

[2024-09-04 22:24] diversenok: `NtTerminateProcess(NULL, statusA)` makes it change from `STATUS_PENDING` to statusA

[2024-09-04 22:25] diversenok: And then doing  `NtTerminateProcess(NtCurrentProcess(), statusB)` will overwrite it with statusB

[2024-09-04 22:26] Matti: yep, I was just reading that code and saw the same actually, heh

[2024-09-04 22:26] Matti: it's far more complex than it needs to be

[2024-09-04 22:26] diversenok: There is also a cool special case for `DBG_TERMINATE_PROCESS` status there, btw

[2024-09-04 22:26] Matti: unless there's a reason for having this functionality that I'm missing...

[2024-09-04 22:26] Matti: [replying to diversenok: "There is also a cool special case for `DBG_TERMINA..."]
yep, spotted

[2024-09-04 22:27] diversenok: `RtlExitUserProcess` uses this two-phase approach... maybe to avoid race conditions?

[2024-09-04 22:27] Matti: this here
[Attachments: image.png]

[2024-09-04 22:27] Matti: is what causes all this trouble

[2024-09-04 22:28] Matti: cause that bit is pretty fucking important

[2024-09-04 22:28] Matti: [replying to diversenok: "`RtlExitUserProcess` uses this two-phase approach...."]
hmm, what with? a second thread calling the same?

[2024-09-04 22:29] Matti: surely the kernel could take care of that

[2024-09-04 22:29] diversenok: ¯\_(ツ)_/¯

[2024-09-04 22:29] diversenok: Somebody creating more threads, perhaps?

[2024-09-04 22:29] Matti: yeah but that's like, too bad for them

[2024-09-04 22:30] Matti: someone called RtlExitUserProcess

[2024-09-04 22:30] Matti: we are leaving

[2024-09-04 22:30] Matti: in fact if the bit was set properly above, creating new threads would not be possible

[2024-09-04 22:31] diversenok: [replying to Matti: "yeah but that's like, too bad for them"]
Time to re-initialize ntdll back after its shutdown! 😂

[2024-09-04 22:31] Matti: always have a spare ntdll handy

[2024-09-04 22:31] Matti: themida knows this

[2024-09-04 22:35] Matti: NtTerminateThread also has some gore by the way... god
[Attachments: image.png]

[2024-09-04 22:35] Matti: but not nearly as bad as the double process terminate

[2024-09-04 22:35] Matti: I just don't like it when win32 retardation seeps into the kernel

[2024-09-04 22:36] Matti: first of all this is impossible to do in a thread safe way anyway, and second, if you wanna do this anyway, why not use the query call `AmILastThread` or whatever it's called

[2024-09-04 22:37] diversenok: Yeah, I was about to mention it

[2024-09-04 22:37] diversenok: `ThreadAmILastThread` indeed

[2024-09-04 22:39] diversenok: Do they have something like win32k calling a user-mode callback to notify about window destruction inside thread termination?

[2024-09-04 22:40] diversenok: It sounds like it would be cursed, but I wouldn't be exactly surprised

[2024-09-04 22:40] Matti: hmmm maybe....

[2024-09-04 22:40] Matti: ```c
typedef enum _PSW32THREADCALLOUTTYPE {
    PsW32ThreadCalloutInitialize,
    PsW32ThreadCalloutExit
} PSW32THREADCALLOUTTYPE;

typedef
NTSTATUS
(*PKWIN32_THREAD_CALLOUT) (
    IN PETHREAD Thread,
    IN PSW32THREADCALLOUTTYPE CalloutType
    );
```

[2024-09-04 22:40] Matti: similar one for process

[2024-09-04 22:41] luci4: [replying to Matti: "NtTerminateThread also has some gore by the way......"]
Where is this code from?

[2024-09-04 22:41] Matti: [replying to luci4: "https://github.com/HighSchoolSoftwareClub/Windows-..."]
^ 😄

[2024-09-04 22:41] luci4: Oh

[2024-09-04 22:41] luci4: 🤦‍♂️

[2024-09-04 22:41] luci4: Thanks 😆

[2024-09-04 22:44] Matti: [replying to Matti: "```c
typedef enum _PSW32THREADCALLOUTTYPE {
    Ps..."]
to give an idea of whereabouts these are called
[Attachments: image.png]

[2024-09-04 22:45] Azrael: What's up with the spaces after the function names?

[2024-09-04 22:46] Matti: this is after the exit status has been finally (re)set and both the debug port and termination port were tried (second chance in case of debug port) but failed/passed unhandled

[2024-09-04 22:46] Matti: [replying to Azrael: "What's up with the spaces after the function names..."]
what about them?

[2024-09-04 22:47] Matti: are you asking me why MS uses the code style it does?

[2024-09-04 22:47] Matti: (inconsistently)

[2024-09-04 22:50] Azrael: [replying to Matti: "are you asking me why MS uses the code style it do..."]
Yes.

[2024-09-04 22:51] Matti: well I don't really know how to respond to that

[2024-09-04 22:51] Matti: do you think it looks wrong? or particularly good maybe?

[2024-09-04 22:51] Matti: every codebase has its own code style

[2024-09-04 22:52] Matti: so long as it's consistent and not completely insane (think openssl), I just adapt to whatever is used

[2024-09-04 22:52] Matti: NT in particular tends to use CNF (cutler normal form)

[2024-09-04 22:53] Matti: though idk if your question is answered by CNF

[2024-09-04 22:53] Matti: maybe someone just likes to add spaces, and that person happened to write this code

[2024-09-04 22:53] Matti: there is plenty of code in the WRK that does not do this

[2024-09-04 22:55] Matti: this is a pretty representative piece of CNF code I would say....

```c
BOOLEAN
DbgkpSuspendProcess (
    VOID
    )

/*++

Routine Description:

    This function causes all threads in the calling process except for
    the calling thread to suspend.

Arguments:

    None.

Return Value:

    None.

--*/

{
    PAGED_CODE();

    //
    // Freeze the execution of all threads in the current process, but the
    // calling thread. If we are in the process of being deleted don't do this.
    //
    if ((PsGetCurrentProcess()->Flags&PS_PROCESS_FLAGS_PROCESS_DELETE) == 0) {
        KeFreezeAllThreads();
        return TRUE;
    }

    return FALSE;
}
```

[2024-09-04 22:56] Matti: any more than this is just an invention of whoever is writing the function

[2024-09-04 22:56] Torph: 4 line function declaration is crazy

[2024-09-04 22:57] Matti: ^ see, this I hear a lot more often 😄
as well as complaints about the verbose comments

[2024-09-04 22:59] Azrael: [replying to Matti: "do you think it looks wrong? or particularly good ..."]
I love their style(s), except for the spaces.

[2024-09-04 22:59] Matti: well but like

[2024-09-04 22:59] Azrael: Function declaration is fine.

[2024-09-04 22:59] Matti: the piece I just posted does not do the spaces thing

[2024-09-04 23:00] Azrael: Yeah, I know.

[2024-09-04 23:00] Azrael: Insyde does it too.

[2024-09-04 23:00] Matti: so what I'm saying is, that is not defined anywhere

[2024-09-04 23:00] Matti: and it's relatively uncommon IME

[2024-09-04 23:00] Azrael: True, but when it's used it looks odd.

[2024-09-04 23:00] Matti: it just depends on what you're reading and who wrote it

[2024-09-04 23:00] Matti: [replying to Azrael: "True, but when it's used it looks odd."]
well oh no

[2024-09-04 23:00] diversenok: You won't see spaces if you read Windows code via IDA 😉

[2024-09-04 23:01] Azrael: For sure, but Insyde seems to be pretty consistent across their codebase(s).

[2024-09-04 23:01] Matti: sorry but like... that's how it goes in projects with more than 1 person working on them

[2024-09-04 23:01] Matti: people will disagree on code style preferences

[2024-09-04 23:01] Matti: just settle on one thing and be consistent

[2024-09-04 23:02] Azrael: Of course, I'm just saying that I personally don't like it.

[2024-09-04 23:02] Matti: noted

[2024-09-04 23:02] Matti: I'm just saying that personally I don't really uh,  well care

[2024-09-04 23:02] Matti: code style arguments are so tired

[2024-09-04 23:02] Matti: again unless you're working on openssl or something

[2024-09-04 23:03] Matti: some projects do go too far

[2024-09-04 23:03] Matti: but they are few and far between

[2024-09-04 23:03] luci4: You made me curious about openssl now lol

[2024-09-04 23:06] Matti: it's mainly its insane indentation
that seems purposely designed to mislead

[2024-09-04 23:06] Matti: wish I could find an example but I can't right now

[2024-09-04 23:07] diversenok: Yeah, every line break inside parenthesis leads to a custom number of spaces depending on where the opening `(` is, lol

[2024-09-04 23:07] Matti: eurgh yeah that's exactly it

[2024-09-04 23:08] luci4: Thankfully I don't use that library, lol

[2024-09-04 23:09] diversenok: You probably do though

[2024-09-04 23:09] Matti: for sure

[2024-09-04 23:09] diversenok: Just like SQLite

[2024-09-04 23:10] Matti: using it isn't so bad, apart from the critical security vulnerabilities

[2024-09-04 23:10] Matti: just don't try to read or write openssl code

[2024-09-04 23:15] luci4: [replying to diversenok: "You probably do though"]
Not willingly*

[2024-09-05 00:51] Torph: [replying to Matti: "^ see, this I hear a lot more often 😄
as well as c..."]
yeah that's my other problem with it lol

[2024-09-05 01:03] Bloombit: Just automatically format code in CI

[2024-09-05 03:02] Deleted User: [replying to Torph: "4 line function declaration is crazy"]
```cpp
template
<
typename
T
>
BOOLEAN
DbgkpSuspendProcess
(
T x
)
{
}
```

[2024-09-07 20:43] dtb: [replying to Deleted User: "```cpp
template
<
typename
T
>
BOOLEAN
DbgkpSuspen..."]
```cpp
template
<
typename
T
>
BOOLEAN
D\
b\
g\
k\
p\
S\
u\
s\
p\
e\
n\
d\
P\
r\
o\
c\
e\
s\
s
(
T x
)
{
}
```