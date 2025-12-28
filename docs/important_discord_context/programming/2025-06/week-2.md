# June 2025 - Week 2
# Channel: #programming
# Messages: 147

[2025-06-02 01:35] grb: anybody had trouble initializing GLFW from a manual mapped DLL? what should i handle in my manual mapper to make GLFW work?

[2025-06-02 13:55] Dis: Hello, how do you guys get active timers in Windows 11? I couldn't find any resources 😦

```cpp
void* TimerTableEntry = (void*)((uintptr_t)KeGetCurrentPrcb() + 0x4100 + 0x200);
for (size_t i = 0; i < 512; i++) {
    void* EntryFlink = *(void**)((uintptr_t)TimerTableEntry + (i * 0x20) + 0x8);
    void* EntryBlink = *(void**)((uintptr_t)TimerTableEntry + (i * 0x20) + 0x10);
    if (EntryFlink != EntryBlink) {
        DbgPrintEx(0, 0, "Entry Found (%llu): %p, %p", i, EntryFlink, EntryBlink);
    }
}
```

This is my original code, every entry was same for 512 entries. it might be completely wrong so anyone can teach me?

[2025-06-03 06:00] James: [replying to Matti: "I think.... my ARM is a little rusty, not sure why..."]
Because ARM atomic instructions can fail for unknown reasons.

[2025-06-03 06:00] James: And so when they do, it just tries again.

[2025-06-03 06:01] James: And thats not that the comparand != the value in memory. it's literally that the instruction has failed in an unpredictable way and you have to try again.

[2025-06-03 06:01] James: pretty annoying little thing isn't it....

[2025-06-03 06:10] James: this is because they TRY to get exclusive access to memory, relying on a software loop to retry if they cannot.

[2025-06-03 06:10] James: sometimes^, but they are also just allowed to fail in some cases

[2025-06-03 10:40] Ruben Mike Litros: what does `Total private commit` and `Total shared commit` values represent in windbg's !vad command? i thought they were the total commit charge of private vads and total commit charge of shared vads respectively but the values are way over the commit charges summed up like i ran the command on a notepad process and commit charge of all vads summed up was 9558 pages but the values for `Total private commit` and `Total shared commit` were 0x2556 pages (38232 KB) and 0x48cb pages (74540 KB)

[2025-06-03 13:50] the horse: https://github.com/binsnake/icedpp
[Embed: GitHub - binsnake/icedpp: A C++ wrapper for icedx86 decoder]
A C++ wrapper for icedx86 decoder. Contribute to binsnake/icedpp development by creating an account on GitHub.

[2025-06-03 14:10] pinefin: [replying to the horse: "https://github.com/binsnake/icedpp"]
😍

[2025-06-03 17:38] sariaki: [replying to the horse: "https://github.com/binsnake/icedpp"]
what exactly is the upside of this?

[2025-06-03 17:38] sariaki: i know iced is nice to use and all

[2025-06-03 17:38] sariaki: and has a neat label feature

[2025-06-03 17:38] sariaki: but is there like a functional argument for using this

[2025-06-03 17:38] the horse: its faster than other decoders

[2025-06-03 17:38] the horse: and you can use cpp

[2025-06-03 17:38] pinefin: yeah it is the fastest out of all

[2025-06-03 17:38] sariaki: oh yea okay

[2025-06-03 17:38] pinefin: besides industry grade

[2025-06-03 17:38] the horse: by a decent margin

[2025-06-03 17:38] sariaki: makes sense in that case

[2025-06-03 17:38] the horse: 2-10x

[2025-06-03 17:38] sariaki: DAMN

[2025-06-03 17:38] pinefin: yeep

[2025-06-03 17:38] the horse: [replying to pinefin: "besides industry grade"]
industry grade is usually 4-6xslower

[2025-06-03 17:38] the horse: like xed

[2025-06-03 17:39] pinefin: ive been wanting to learn rust just to use iced 😭

[2025-06-03 17:39] pinefin: which honestly i probably should learn it regardless

[2025-06-03 17:39] pinefin: but

[2025-06-03 17:39] pinefin: mehhhhhh

[2025-06-03 17:40] the horse: same

[2025-06-03 17:40] sariaki: sick project in that case

[2025-06-03 17:51] James: [replying to pinefin: "besides industry grade"]
What makes iced not industry grade?

[2025-06-03 17:51] James: They test against other supposed "industry grade" decoder/encoders.

[2025-06-03 17:52] James: And found bugs in them in the process of doing so.

[2025-06-03 17:53] James: 
[Attachments: Screenshot_2025-06-03_at_10.52.48_AM.png]

[2025-06-03 17:53] pinefin: [replying to James: "What makes iced not industry grade?"]
sorry, im talking from stuff that you don't see on github. im sure iced definitely competes

[2025-06-03 17:53] pinefin: i dont mean to say iced is bad, i love iced

[2025-06-03 17:53] James: [replying to pinefin: "sorry, im talking from stuff that you don't see on..."]
Like?

[2025-06-03 17:53] pinefin: i say industry grade because theres probably some shit nasa has in their cabinet

[2025-06-03 17:53] pinefin: and you're right, iced definitely is industry grade

[2025-06-03 17:54] pinefin: im talking super level of emu/decoders

[2025-06-03 17:54] James: [replying to pinefin: "i say industry grade because theres probably some ..."]
It's definitely more dangerous to try and develop such a complex system in house, when open source, tested at large, public systems like iced are available.

[2025-06-03 17:54] James: but given that nasa is all PPC(don't quote me)? maybe they had to write their own for that.

[2025-06-03 17:55] pinefin: thats what im referring to, im not saying iced is bad at all

[2025-06-03 17:55] pinefin: and you're right, i just kind of meant like on a million dollar level scale of development

[2025-06-03 17:55] pinefin: (idk how much it'd cost to develop something better, or in house)

[2025-06-03 17:56] James: Well probably a lot less, but the value in xed/iced/zydis is that lots of people use it, and so all the edge cases are tickled regularly by people doing weird things.

[2025-06-03 17:56] pinefin: i like the use of the word tickled 😭 but yeah for sure. i just kind of worded that wrong

[2025-06-03 17:56] James: xD

[2025-06-03 17:56] James: Wrote my own ARM decoder recently...

[2025-06-03 17:57] James: Need to open source and hopefully get others using to help me find bugs

[2025-06-03 17:57] James: because there are ~5k instructions(haven't supported SVE yet)

[2025-06-03 17:57] James: and i just KNOW there are weird bugs in places that won't show up until lots of people test

[2025-06-03 17:57] pinefin: ARM also feels like it'd be a lot easier since it's (mostly) padded by 2-4 bytes. correct me if im wrong ive only grazed the surface of arm asm with armv7

[2025-06-03 17:58] James: I did ARM64, so v8+ with only 64bit.

[2025-06-03 17:58] James: its easier that x86 for sure

[2025-06-03 17:58] James: hold on.

[2025-06-03 17:58] James: ill show u

[2025-06-03 17:59] James: ```
Tbx_Vd8B_1Vn16B_Vm8B,0|0|001110000|VmR|0|00|100|VnR|VdH,,,
Tbx_Vd16B_1Vn16B_Vm16B,0|1|001110000|VmR|0|00|100|VnR|VdH,,,
Tbx_Vd8B_2Vn16B_Vm8B,0|0|001110000|VmR|0|01|100|VnR+1|VdH,,,
Tbx_Vd16B_2Vn16B_Vm16B,0|1|001110000|VmR|0|01|100|VnR+1|VdH,,,
Tbx_Vd8B_3Vn16B_Vm8B,0|0|001110000|VmR|0|10|100|VnR+2|VdH,,,
Tbx_Vd16B_3Vn16B_Vm16B,0|1|001110000|VmR|0|10|100|VnR+2|VdH,,,
Tbx_Vd8B_4Vn16B_Vm8B,0|0|001110000|VmR|0|11|100|VnR+3|VdH,,,
Tbx_Vd16B_4Vn16B_Vm16B,0|1|001110000|VmR|0|11|100|VnR+3|VdH,,,
```

[2025-06-03 17:59] James: I have a grammer somewhere... hold up

[2025-06-03 17:59] pinefin: erm what is going on here

[2025-06-03 17:59] James: Opcode,Encoding/Fields,Implicits,FailureConditions,

[2025-06-03 18:00] James: Most decoders/encoders have tables behind them, then use those to generate code,

[2025-06-03 18:00] James: I took that approach as well

[2025-06-03 18:00] James: So this is for the SIMD&FP Tbx Instruction

[2025-06-03 18:00] James: gtg

[2025-06-03 18:05] pinefin: [replying to James: "So this is for the SIMD&FP Tbx Instruction"]
oh i see now

[2025-06-03 18:05] pinefin: this better helps me understand

[2025-06-03 18:05] pinefin: yeah i dont know much about arm asm like i said, and still dont know much about how to read the table...but nice work brother

[2025-06-03 18:06] pinefin: only time i ever have to look at arm assembly is whenever an ios app crashes or an embedded device crashes in siliconlabs black box code

[2025-06-05 20:23] pbgr: does anybody know any reliable & fast methods to asynchronously call user-mode code from ring0 at an IRQL above the thread scheduler's? i don't mind if they're hacky

[2025-06-05 20:23] pbgr: i tried scheduling a threaded DPC to a core with low enough IRQL, the DPC would then register an APC to a pre-specified user-mode handler/thread, worked but slow

[2025-06-05 23:37] Matti: [replying to pbgr: "i tried scheduling a threaded DPC to a core with l..."]
this is generally how all interrupt handlers tackle this problem too, if there was a faster way they would use that instead

[2025-06-05 23:37] Matti: though, your choice of a threaded DPC may be what's causing the slowness

[2025-06-05 23:39] Matti: threaded DPCs are not necessarily faster than 'old school' DPCs

[2025-06-05 23:40] Matti: also, are you setting the DPC to highest importance and doing the equivalent (I forget the name) for the APC?

[2025-06-05 23:41] Matti: the type of APC matters too if you can influence that without breaking the behaviour

[2025-06-05 23:41] Matti: special kernel > kernel > special user > user

[2025-06-05 23:43] Matti: the last thing that comes to mind that you could try out is raising the priority of the APC target thread... but that's basically what the `Increment` parameter to `KeInsertQueueApc` is already for

[2025-06-05 23:47] Matti: thinking hackier... if there was a safe way to queue an APC directly at your IRQL (what is it?) that would of course mean you skip the DPC part

[2025-06-05 23:48] Matti: I don't remember off the top of my head how this is synchronized, but it would be pretty gruesome if it's possible to do this at all

[2025-06-05 23:49] Matti: I guess you would need to take the APC queue spinlock exclusively

[2025-06-05 23:51] Matti: the specifics of how the APC queues are managed all vary somewhat between different versions of NT (as well as the APC queue struct itself) - I'd say enough to make this one of those hacks that only work on one specific version of NT

[2025-06-05 23:52] Matti: other than that it's probably the method that would shave off the most time here

[2025-06-06 10:04] eval00: you could probably just use a high-importance regular dpc with KeInsertQueueDpc and KeSetImportanceDpc at IRQL 2 / dispatch level when the IRQL drops to queue an apc in the dpc routine using KeInsertQueueApc with a priority boost (setting the increment param to some value (e.g. 1)) and make sure it's alertable with SleepEx or something so the apc runs when the thread is scheduled when it's passive level

[2025-06-06 10:05] eval00: <@1353833913293864994>

[2025-06-06 11:57] Matti: isn't that what I just posted, mr. joined-yesterday?
although granted I didn't assume OP would be capable of queueing an APC but somehow have a target thread incapable of actually executing it... so maybe SleepEx is the answer

[2025-06-06 11:59] Matti: but yes, regular (high importance) DPCs will normally be much lower latency than threaded DPCs

[2025-06-06 14:32] pbgr: [replying to Matti: "also, are you setting the DPC to highest importanc..."]
yeah, i am. the DPC->APC method works but they're just too slow for what i'm doing ( the UM thread is always alertable and ready to service the APC ). it's apart of an instrumentation/ introspection framework, and e.g if i wanted to log every `call` instr executed on a given core that method would just be too slow ( log as in, notify user-mode ). the current IRQL is arbitrary, it's whatever the IRQL was at the time of the interrupted code for which i wish to log, so i want a generic method that'll work with even high irqls.

[2025-06-06 14:33] pbgr: [replying to Matti: "I guess you would need to take the APC queue spinl..."]
do you mean, acquire the main apc environment spinlock exclusively and manually insert?

[2025-06-06 14:36] pbgr: i just think APCs are slow in this case given that the kernel doesn't drain the APC queue as soon as they're inserted

[2025-06-06 14:45] Matti: [replying to pbgr: "do you mean, acquire the main apc environment spin..."]
yes

[2025-06-06 14:46] Matti: [replying to pbgr: "i just think APCs are slow in this case given that..."]
well no, of course not, that would kind of defeat the point of having a queue

[2025-06-06 14:47] pbgr: [replying to Matti: "well no, of course not, that would kind of defeat ..."]
yeah, i use APCs in a different scenario when speed isn't so paramount, just in this case i'm wondering just if there's anything superfast

[2025-06-06 14:47] Matti: but a special kernel APC will absolutely be the first thing a thread going from ready to running will start executing (assuming no other even higher IRQL switches happen unexpectedly)

[2025-06-06 14:49] Matti: there's nothing faster than this if the IRQL being <= APC is a requirement (which it is of course, or you wouldn't be asking)

[2025-06-06 14:50] Matti: what you seem to want is a way to force a certain thread to change from whatever its current state is, to running, *now*

[2025-06-06 14:50] Matti: i.e. you want to be the kernel thread scheduler

[2025-06-06 14:51] Matti: I'm not saying that's impossible with even more hackery... but it sure is going very far

[2025-06-06 14:51] Matti: I wouldn't be able to write the code that does this safely and correctly

[2025-06-06 14:52] pbgr: [replying to Matti: "i.e. you want to be the kernel thread scheduler"]
yeah i thought about messing with the scheduler as the last resort, if it's the only option it's something i'm willing to implement

[2025-06-06 14:52] pbgr: [replying to Matti: "I'm not saying that's impossible with even more ha..."]
Yh, it's just doing so in a manner that won't mess anything up

[2025-06-06 14:53] pbgr: i've tried a lot of different methods, all with their issues

[2025-06-06 14:54] Matti: can you give some rough (order of magnitude) definition of what 'fast' or 'slow' execution would be for you here

[2025-06-06 14:55] Matti: what is it that requires this extremely low latency that normal hardware ISRs don't seem to be bothered by much

[2025-06-06 15:02] pbgr: [replying to Matti: "can you give some rough (order of magnitude) defin..."]
this code is part of a hv. the fastest method i've implemented is: upon the execution of an instruction that's been registered to be logged, i'll save architectural state and vmenter to a pre-specified user-mode guest handler. the guest handler will perform it's operation and vmexit back to the vmm which will restore the architectural state back to the interrupted code that had the instruction i wanted to log. the vmenter/exit mechanism is instantaneous & probably faster than i need. you can imagine its speed because the transitions are handled by a single CPU instruction rather than lengthy computations & a long call chain, e.g., the scheduler ( which is ok in most cases ) the thing with APCs is, once the scheduler has dispatched the target thread to execute, upon transition to ring3, it has to check the main apc env and service scheduled APCs. while this is happening, another event which i wish to log could've occurred, and i want no execution on the virtualised core until the user-mode handler is done. so, i have to stall the virtualised core until the APC is serviced, which i've tested to be pretty slow. imagine i want to log all far jmps that happen millions of times a second

[2025-06-06 15:06] Matti: it sounds to me like you're trying to write a hardware instruction tracer in software

[2025-06-06 15:06] pbgr: [replying to Matti: "it sounds to me like you're trying to write a hard..."]
essentially

[2025-06-06 15:07] Timmy: is all you're intending to go back into um for just logging?

[2025-06-06 15:08] pbgr: [replying to pbgr: "essentially"]
not just for instrs, that'd probably be the thing that i log that occurs most frequently, but i log any system event, e.g system pte space allocation, process creation, thread hard affinity change etc

[2025-06-06 15:08] pbgr: [replying to Timmy: "is all you're intending to go back into um for jus..."]
nah, i could log entirely from KM

[2025-06-06 15:08] Timmy: ah

[2025-06-06 15:08] Timmy: ye was about to suggest tracelogging

[2025-06-06 15:08] pbgr: yeah, thanks for the suggestion though

[2025-06-06 15:08] Timmy: should allow for that kind of throughput

[2025-06-06 15:11] pbgr: the vmenter/exit mechanism worked fast enough, the problem there is just saving architectural state extensively enough. currently i use xsave, save gprs and crs, but not all systems will support that instruction

[2025-06-06 15:11] Timmy: ah ye it's latency you care about

[2025-06-06 15:11] pbgr: [replying to Timmy: "ah ye it's latency you care about"]
yeah

[2025-06-06 15:18] Matti: [replying to Matti: "it sounds to me like you're trying to write a hard..."]
not that there's anything wrong with this necessarily by the way
I'm just thinking that, ultimately your issue is that any logging related code that's going to need to be executed (that includes your code as well as any kernel code required to get it to execute)
is probably going to require a number of CPU cycles that's going to have a lot more zeroes in it than your actual tracing code will

[2025-06-06 15:19] pbgr: [replying to Matti: "not that there's anything wrong with this necessar..."]
>  probably going to require a number of CPU cycles that's going to have a lot more zeroes in it than your actual tracing code will
yeah, not sure if it's actually feasible

[2025-06-06 15:20] Matti: that is just the nature of doing hardware things in software (yes technically your tracing is also fully in software, but your description above makes the speed difference pretty clear I think)

[2025-06-06 15:20] Matti: it's HW-assisted, at the very least

[2025-06-06 15:23] Matti: question: is it your actual *goal* (or at least the idea) to write the tracer in this way, or do you really want a tracer for some other purpose and this is what you came up with

[2025-06-06 15:23] Matti: I suspect it's the former but just checking

[2025-06-06 15:23] Matti: I assume you're aware HW tracing does just exist, it's a thing

[2025-06-06 15:24] pbgr: nah there's stuff like IPT but my case is the former

[2025-06-06 15:24] pbgr: i just choose to implement this tracer with software, in a hw assisted manner, because i trace much more than instrs

[2025-06-06 15:28] Matti: one thing that comes to mind is intel trace hub - it's hardware tracing that can be consumed via NT ETW events

[2025-06-06 15:29] Matti: so it could be possible to have a hybrid approach where you do the heaviest tracing (instructions) via ITH and use ETW to also produce a user mode usable tracer for your other events like thread creation or whatnot

[2025-06-06 15:30] Matti: this is just me typing out thoughts, I've never used ITH myself

[2025-06-06 15:30] Matti: 
[Attachments: Non-NPKETW-Userguide.pdf]

[2025-06-06 15:32] Matti: (I'm assuming your HV is already VT-x only - there's no AMD equivalent for ETW that I know of)

[2025-06-06 15:33] Matti: ITH is normally used with a host and target system for debugging, but it can also target itself (the host system) for tracing

[2025-06-06 15:35] pbgr: [replying to Matti: "so it could be possible to have a hybrid approach ..."]
yeah it's vt-x. ith/etw sounds good for instructions since they occur so frequently & ill probably use DPC/APC for less common events like a memory allocation. thanks a lot for the help & the reference.

[2025-06-06 15:36] Matti: np - have some drivers for ITH as well since they're not so easy to find
[Attachments: Intel_Trace_Hub-10.0.17763.306-RS5_2021.7z, Intel_Trace_Hub_10.0.18339.84_2019_NDA.7z]

[2025-06-06 15:36] koyz: honest question, why not use an intermediate buffer to cache these events (with timestamps) and then clear them out periodically, I don't really understand the need for DPCs/ APCs

[2025-06-06 15:36] Matti: the PDF is from the 2021*zip

[2025-06-06 15:37] pbgr: [replying to koyz: "honest question, why not use an intermediate buffe..."]
i want guest ring3 execution

[2025-06-06 15:37] pbgr: not just shared data

[2025-06-06 15:38] pbgr: [replying to Matti: "np - have some drivers for ITH as well since they'..."]
thanks

[2025-06-06 15:44] Matti: I should note that ITH isn't normally enabled or enable-able on consumer systems by default... it's pretty much exactly like DCI when it comes to the restrictions on it and having to unlock access, probably using a hardware flasher or otherwise buying a dev board instead

[2025-06-06 15:46] Matti: but unlike DCI ITH does not depend on JTAG over USB, which is the main part that IMO makes DCI so horrible (impossible really) to use for debugging

[2025-06-06 15:47] Matti: DCI is a lot of work to enable on a regular non-devkit board, and then on top of that it also doesn't work

[2025-06-06 15:47] Matti: I *expect* ITH will just be the former

[2025-06-06 15:47] Matti: but as I said I have no experience using it myself