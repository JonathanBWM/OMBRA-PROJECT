# February 2025 - Week 3
# Channel: #reverse-engineering
# Messages: 84

[2025-02-10 23:11] pinefin: though its still running on the JVM

[2025-02-10 23:11] pinefin: so because its kotlin doesn't matter because its still java bytecode

[2025-02-10 23:11] pinefin: im not a huge java nerd but i've built an app in kotlin before

[2025-02-11 00:53] elias: Is the WdFilter minifilter still active even when third party EDR/AV is installed?

[2025-02-11 02:32] toasts: if the third-party product is the primary provider it should be off, typically some prevention mode in the third-party product needs to be enabled otherwise it will sit behind defender

[2025-02-11 13:17] Xits: Is there some public research on how windows manages the paging hierarchy entries? I’m interested in how it decides what pml4 entries to use

[2025-02-11 13:18] Xits: I know the identity map and it needs one for every 512GB of memory. So that’s 2 but I’ve seen that it uses more entires and I’m wondering why

[2025-02-11 13:19] Xits: Isn’t that suboptimal to use different pml4 indexes unless absolutely necessary (in terms of caching)?

[2025-02-11 16:14] Timmy: <:give:835809674480582656>

[2025-02-12 11:51] Eriktion: [replying to Xits: "Isn’t that suboptimal to use different pml4 indexe..."]
No, ehm with that model windows can seperate the user address space from the kernel. Windows can map different permissions to these memory regions. While in theory it could also be done in 1 pml4, I think it is easier to do in multiple pml4 entries

[2025-02-12 11:53] Eriktion: I have not tested this but this is also a theory: When an allocation is requested at a weird base address by the user this will lead to a pml4 entry being filled with at least 1 entry containing the memory the user allocated

[2025-02-12 11:53] Eriktion: E.g. Virtualalloc with a peculiar bade address request

[2025-02-14 10:11] Terry: I'm hooking RtlDispatchException to moniter VEH / PAGE_GUARD hooks, and all works great on windows 10, but for some reason the same logic on windows 10 doesn't work on windows 11. Upon a STATUS_GUARD_PAGE_VIOLATION, I set the trap flag in eflags from the context record then return true (don't call the original RtlDispatchException), then catch the single step exception & set the page permission to current page permission |  PAGE_GUARD. However, on windows 11 I never get the STATUS_SINGLE_STEP exception,  I figure it's probably some security / logic change in windows 11 that I don't know about. Don't know a better place to ask than here ;p


And incase anyone was going to ask, no anti cheat or anti virus is involved in any of this, so no problem coming from that

[2025-02-15 09:34] NSA, my beloved<3: Hey! So I am a little bit confused regarding a fresh Windows process and its Threads.
1. I noticed that the first thread that is created by default for every process (main thread)'s IP is always set to ntdll.dll!LdrInitializeThunk. This function starts a function call chain serving the purpose of loading core system libraries (kernel32.dll, kernelbase.dll) and then calling their entry points (DllMains). After this is done, 'less vital?' libraries are being loaded like ucrtbase.dll, but their entry points are not called just yet.
2. Then comes the "system breakpoint", which is ntdll code checking if the process is being debugged, if so jmp to a int3 software interrupt to stop execution.
3. Then kernel32.dll!BaseThreadInitThunk is called, which is starts a more 'thread focused' function call chain and instructions. This is where the above mentioned not so crucial libraries' TLS callbacks and DllMains are called (like ucrtbase.dll).
**This is where my questions come in place and my confusion starts.** Comes the ZwContinue syscall from ntdll.dll!LdrInitializeThunk, which makes our code execution continue at kernel32.dll!BaseThreadInitThunk, again? Which does the jump to the entry point of our executable file, that returns here so the thread can be gracefully terminated along with the process if there aren't any threads left.
The problem is, as many times I debug the same process using x64dbg, as many times I get different call order results. There are cases where a thread is created out of nowhere, at different locations of execution, which messes up the execution path that I am trying to follow and understand, why is that? I am not creating new threads. Also, when a new thread object is created what I mostly see is that it starts execution at ntdll.dll!RtlUserThreadStart, which then eventually calls ntdll.dll!LdrInitializeThunk. What is the point of doing this? We don't need more libaries. Why not set the IP directly to ntdll.dll!LdrInitializeThunk?

[2025-02-15 10:59] x86matthew: RtlUserThreadStart doesn't call LdrInitializeThunk

[2025-02-15 10:59] x86matthew: LdrInitializeThunk is the first usermode entry point for threads

[2025-02-15 11:00] x86matthew: LdrInitializeThunk -> NtContinue (to RtlUserThreadStart) -> BaseThreadInitThunk -> (thread entry point) -> returns to BaseThreadInitThunk and calls RtlExitUserThread

[2025-02-15 11:00] x86matthew: the "extra threads" you're talking about are from the threadpool

[2025-02-15 11:00] x86matthew: for parallel loader etc

[2025-02-15 11:05] 5pider: [replying to NSA, my beloved<3: "Hey! So I am a little bit confused regarding a fre..."]
You can read more related the initialization of LdrInitializeThunk here: https://www.outflank.nl/blog/2024/10/15/introducing-early-cascade-injection-from-windows-process-creation-to-stealthy-injection/

they also have references to parallel loading and other mechanism how how the process starts and how the thread starts executing
[Embed: Introducting Early Cascade Injection | Outflank Blog]
Get an introdcution to Early Cascade, a novel process injection technique that is effective against top tier EDRs while avoiding detection.

[2025-02-15 11:50] NSA, my beloved<3: [replying to x86matthew: "LdrInitializeThunk -> NtContinue (to RtlUserThread..."]
Yeah this is exactly what I though the execution order was. But every time a new thread appears, an automatic thread entry breakpoint is hit at RtlUserThreadStart, and the next thing is my manual software breakpoint at LdrInitializeThunk. I'd assume this is then called by the main thread? But no, that makes no sense, since the thread has to jump here in order to call the TLS callbacks and entry points of user loaded libraries (LdrInitializeThunk has the function chain which handles this behavior). So the thread has to somehow end up here, to execute library code, which would also be confirmed by x64dbg and by setting a breakpoint to the InitializeThunk function. Or am I lost here?

[2025-02-15 11:50] NSA, my beloved<3: [replying to x86matthew: "the "extra threads" you're talking about are from ..."]
Ah, that explains a lot. Thank you!

[2025-02-15 11:52] NSA, my beloved<3: [replying to 5pider: "You can read more related the initialization of Ld..."]
Thanks, I'll have a look now.

[2025-02-15 11:59] NSA, my beloved<3: [replying to 5pider: "You can read more related the initialization of Ld..."]
Wow, this is a great blog post! Thanks.

[2025-02-15 18:00] NSA, my beloved<3: It's weird. The breakpoint is hit after the thread entry starts executing code at RtlUserThreadStart. The call stack points to zeroes, so this must be a jmp? After running through the LdrInitializeThunk and arriving at BaseThreadInitThunk,the call stack in x64dbg doesn't show LdrInitializeThunk anywhere except with the main thread. So I'd assume this is an anomaly with x64dbg? That the breakpoint is triggered? (checked the threads, and it's not the main thread hitting LdrInitializeThunk, but the fresh thread). But that also makes no sense, because this is from where the custom user dll entry points and TLS callbacks are called once a new thread is created, so we should end up here.

[2025-02-15 18:17] x86matthew: it's normal for LdrInitializeThunk to have no call stack / return address because that's where it enters directly from the kernel

[2025-02-15 18:18] x86matthew: same for the following RtlUserThreadStart call which is executed via NtContinue

[2025-02-15 18:19] x86matthew: if you're seeing breakpoints being hit in the "wrong" order then it'll just be due to a race with other threads (the initial threadpool)

[2025-02-15 18:20] x86matthew: make a note of the thread ID for each breakpoint

[2025-02-15 18:51] NSA, my beloved<3: [replying to x86matthew: "it's normal for LdrInitializeThunk to have no call..."]
But in the case of a new thread, this is not the entry point, so it should have a call stack I'd assume.

[2025-02-15 18:52] x86matthew: all threads start at LdrInitializeThunk

[2025-02-15 18:52] x86matthew: the user-defined entry point is executed later on

[2025-02-15 18:52] x86matthew: by RtlUserThreadStart

[2025-02-15 18:55] NSA, my beloved<3: [replying to x86matthew: "all threads start at LdrInitializeThunk"]
Hmm, well, whenever a new thread gets created it always starts at RtlUserThreadStart whatever I do, according to x64dbg. Then comes a call to LdrInitializeThunk. The only exception is the main thread. The order matches there. You migth want to give it a try and see if you get the same execution order.

[2025-02-15 19:03] x86matthew: i do this a lot, it's always fine

[2025-02-15 19:04] x86matthew: for example:
[Attachments: image.png]

[2025-02-15 19:04] x86matthew: and then the second breakpoint:
[Attachments: image.png]

[2025-02-15 19:05] x86matthew: note thread ID and time

[2025-02-15 19:07] NSA, my beloved<3: That is so weird. I would have thought you'd have the same order. Let me double check the Thread ID but this time by looking at the title of the Window.

[2025-02-15 19:09] x86matthew: it wouldn't make any sense at all for them to execute in the wrong order

[2025-02-15 19:09] x86matthew: so i'm not sure what you're seeing lol

[2025-02-15 19:09] NSA, my beloved<3: Yeah it's the exact opposite for me.

[2025-02-15 19:10] x86matthew: how are you testing this?

[2025-02-15 19:11] NSA, my beloved<3: I may be misinterpreting something. Let me triple check by creating a thread manually.

[2025-02-15 19:12] NSA, my beloved<3: Same results. RtlUserThreadStart -> LdrInitializeThunk -> RtlUserThreadStart  -> BaseThreadInitThunk.

[2025-02-15 19:13] NSA, my beloved<3: [replying to x86matthew: "how are you testing this?"]
By setting a breakpoint at RtlUserThreadStart and LdrInitializeThunk, however if I disable the breakpoint at RtlUserThreadStart, and enable Thread Entry, it breaks there as well without a need for a breakpoint. Then jumps to LdrInitializeThunk and back.

[2025-02-15 19:14] x86matthew: you're not running this via emulation on ARM64 or something weird are you

[2025-02-15 19:14] NSA, my beloved<3: No I am not.

[2025-02-15 19:15] NSA, my beloved<3: Should we ask mrexodia?

[2025-02-15 19:16] NSA, my beloved<3: My last guess would have been, x86 versus x64, but we both are testing in x64, so it should be the same there as well.

[2025-02-15 19:19] x86matthew: when the debugger first breaks, are you sure it's actually breaking on RtlUserThreadStart? check rip value to confirm

[2025-02-15 19:19] NSA, my beloved<3: Yes, I confirmed with the RIP.

[2025-02-15 19:41] Brit: can you show the callstack that you get

[2025-02-15 19:41] Brit: wait nvm

[2025-02-15 19:41] Brit: [replying to NSA, my beloved<3: "Same results. RtlUserThreadStart -> LdrInitializeT..."]
are you positive abt this?

[2025-02-15 19:41] NSA, my beloved<3: [replying to Brit: "are you positive abt this?"]
I am 110% sure. Tested it so many times, in so many ways.

[2025-02-15 19:43] Brit: could you send a screen of the callstack tab of x64dbg if you're using that for the thread in question

[2025-02-15 19:48] NSA, my beloved<3: Sure!

[2025-02-15 19:49] NSA, my beloved<3: At the thread entry break, or after LdrInitializeThunk was ran? Since as I mentioned, for some reason LdrInitializeThunk does not show up in the call stack of any thread other than the main thread.

[2025-02-15 19:51] Brit: on a new thread after it hits the thread init bp

[2025-02-15 19:53] NSA, my beloved<3: 
[Attachments: image.png]

[2025-02-15 19:55] x86matthew: press F7 to single step from that point

[2025-02-15 19:56] x86matthew: my guess is that it just the thread creation debug event, not the bp

[2025-02-15 19:56] x86matthew: in which case single step will jump straight to LdrInitializeThunk

[2025-02-15 19:56] NSA, my beloved<3: That's exactly what happened.

[2025-02-15 19:57] NSA, my beloved<3: Shouldn't this be disabled if I turn everything off in preferences?

[2025-02-15 19:57] x86matthew: to prove this, disable "Thread Create" break in options

[2025-02-15 19:57] x86matthew: and try again

[2025-02-15 19:57] x86matthew: this way you'll only see your breakpoints

[2025-02-15 19:57] x86matthew: and it should run as expected

[2025-02-15 19:59] NSA, my beloved<3: Great. This yields the expected results. You are a genius.

[2025-02-15 19:59] NSA, my beloved<3: My only question left is, why does this happen with Thread Create enabled?

[2025-02-15 20:00] NSA, my beloved<3: Is this a x64dbg specific thing? The program was written in a way where if the option is enabled, it automatically breaks at RtlUserThreadStart?

[2025-02-15 20:00] NSA, my beloved<3: By the way, thank you for your assistance as well, Brit.

[2025-02-15 20:01] Brit: I hardly did anything haha, thank matthew

[2025-02-15 20:03] NSA, my beloved<3: Thanking him won't be enough, I owe him one, for helping me throughout the day. 😄

[2025-02-15 20:59] x86matthew: [replying to NSA, my beloved<3: "Is this a x64dbg specific thing? The program was w..."]
glad you sorted it! i can understand the confusion

[2025-02-15 20:59] x86matthew: i haven't looked at the x64dbg code but the behaviour is technically correct

[2025-02-15 20:59] x86matthew: when it catches the create_thread debug event it'll be displaying the current thread context

[2025-02-15 21:00] x86matthew: which will contain RtlUserThreadStart

[2025-02-15 21:00] x86matthew: but because the thread hasn't yet been initialised, this isn't the true entry-point, LdrInitializeThunk will always be called first

[2025-02-15 21:01] x86matthew: you'd see the same thing if you create a suspended thread and immediately call GetThreadContext, RIP will contain RtlUserThreadStart, but when you resume the thread, execution actually begins at LdrInitializeThunk

[2025-02-15 21:10] x86matthew: to think of it another way, imagine a normal APC call being scheduled against a suspended thread, same idea

[2025-02-15 21:11] NSA, my beloved<3: Oh, that makes complete sense. It's interesting and can cause some confusion. Or at least it did for me. 🙂