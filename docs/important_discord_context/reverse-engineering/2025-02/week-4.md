# February 2025 - Week 4
# Channel: #reverse-engineering
# Messages: 82

[2025-02-17 17:27] NSA, my beloved<3: So another question came into my mind. If the loader lock, which is supposed to prevent threads from executing any code (are they in a suspended state?) is active during initialization and only the Main thread is running, how can the thread pool related threads be running during parallel loading while the libraries are being loaded & executed?

[2025-02-17 17:38] BWA RBX: [replying to x86matthew: "for example:"]
For some odd reason I still fucking love the design of this application, idkw it's really aesthetically pleasing

[2025-02-17 21:34] UJ: [replying to BWA RBX: "For some odd reason I still fucking love the desig..."]
native win32 forever.

[2025-02-18 13:40] mrexodia: [replying to NSA, my beloved<3: "Is this a x64dbg specific thing? The program was w..."]
You might enjoy https://lecture.mrexodia.re
[Embed: Windows Internals Crash Course]
Guest lecture about Windows Internals (aimed at total beginners), given at the Ruhr-Universität Bochum. Slides: https://mrexodia.github.io/files/wicc-2023-sl...

[2025-02-18 15:26] deja: [replying to mrexodia: "You might enjoy https://lecture.mrexodia.re"]
This is so awesome for someone who is starting to learn Windows internals, thank you so much!

[2025-02-18 18:58] NSA, my beloved<3: [replying to mrexodia: "You might enjoy https://lecture.mrexodia.re"]
I did enjoy it, at least 5 times already! Great video.

[2025-02-18 18:58] NSA, my beloved<3: This sparked the question that I asked. 🙂

[2025-02-18 19:04] NSA, my beloved<3: Matthew's answer completed the big picture. It helped me understand the part where you talk about (and show) the thread context.

[2025-02-18 19:18] mrexodia: I always shill it

[2025-02-18 19:26] NSA, my beloved<3: [replying to NSA, my beloved<3: "So another question came into my mind. If the load..."]
Regardless, I still could not find an answer for this. If anyone could drop some knowledge regarding the topic, or even any resources I could read, I would highly appreciate it!

[2025-02-18 19:26] Matti: [replying to NSA, my beloved<3: "So another question came into my mind. If the load..."]
first of all, the loader lock doesn't and isn't supposed to prevent other code from executing. only to protect certain data structures commonly accessed by the loader

so then next the short and oversimplified answer: because the worker threads do different things (more precisely, those things that can be done outside the loader lock, which is more than you might expect! a large amount of module 'loading' is spent on exhausting all of the recursive dependencies, which may involve loading no new modules at all)

recommended reading re: parallel loading in general: https://infocon.org/mirrors/vx%20underground%20-%202024%20June/Papers/Windows/Analysis%20and%20Internals/2017-10-03%20-%20Windows%2010%20Parallel%20Loading%20Breakdown.pdf
and the loader lock: https://elliotonsecurity.com/what-is-loader-lock/ (the last bit touches on how the parallel loader makes use of the new mechanisms introduced in the post)

[2025-02-18 19:27] NSA, my beloved<3: The timing, holy.

[2025-02-18 19:32] NSA, my beloved<3: [replying to Matti: "first of all, the loader lock doesn't and isn't su..."]
Thank you so much! I would have guessed there are certain tasks that are permitted by the loader lock and what these threads do fall within that category. Furthermore, I have read this MSDN doc: https://learn.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-best-practices which gives the impression that the loader lock prevents some functionality, rather than it being the opposite where it would preventing everything and allowing some functionality. This resulting in a broader category regarding what threads are able to do (not only user created threads) during the loader lock. I'll read what you linked, thanks again!
[Embed: Dynamic-Link Library Best Practices - Win32 apps]
Creating DLLs presents a number of challenges for developers.

[2025-02-18 19:36] Matti: one other key point mentioned in the second link re: critical sections in windows in general (I don't actually think this is used for the parallel loader, but it's a very big difference from unix)
is that windows critical sections can be held **recursively** (but only by the same thread, i.e. the owning thread)

[2025-02-18 19:37] Matti: it's something that is rarely needed in practice, but it's useful to know at least

[2025-02-18 19:37] Matti: this only applies to critical sections, not SRW locks, which are too lightweight to store any owner info

[2025-02-18 19:48] NSA, my beloved<3: Thank you so much! I'm sure it will all make sense once I've read through the links.

[2025-02-18 23:40] daax: 2pcs, kdnet

[2025-02-18 23:41] daax: whether you want to to do it via eem or not, doesn't really matter

[2025-02-18 23:41] daax: yeah, but host and them VM works too, doesn't matter really

[2025-02-19 01:33] donna🤯: https://github.com/xalicex/kernel-debug-lab-for-virtual-box this is a really good guide for setting up vm debugging of network - works for vbox and workstation
[Embed: GitHub - xalicex/kernel-debug-lab-for-virtual-box: How to set up 2 ...]
How to set up 2 VirtualBox VM to debug kernel driver using windbg - xalicex/kernel-debug-lab-for-virtual-box

[2025-02-19 01:34] donna🤯: and as Daax said it doesnt really matter, testing on real hardware though is good and required if you want to distribute anything stable to many users

[2025-02-19 01:35] donna🤯: 2 vms?

[2025-02-19 01:36] donna🤯: id definitely just use windbg on the host whats the reason for running it in a vm?

[2025-02-19 01:39] donna🤯: sorry but why are you running windbg in a vm?

[2025-02-19 01:39] donna🤯: thats a massive bottleneck

[2025-02-19 01:40] daax: more ram, more cores, kdnet

[2025-02-19 01:40] donna🤯: id highly recommend do it on the host but if you cant/dont want to its gonna be tough regardless with 2 vms

[2025-02-19 01:44] daax: would say that’s a good idea long term. are you just debugging a driver youre writing?

[2025-02-19 09:19] Matti: second the 'debug on host' advice, debugging vm to vm is only going to double the number of issues you're going to have with kd and the time spent troubleshooting them, nevermind even the speed of the debugging itself... which alone would be enough reason for me to never consider this setup

[2025-02-19 09:22] Matti: sure, and in fact this is a good idea if you're resource constrained

[2025-02-19 09:23] Matti: any environment that's sufficient to debug your driver is sufficient full stop (for that purpose obviously...), and starting, stopping or resuming a live snapshot are all going to take significantly longer the more RAM your guest has, as well as (to a lesser extent) vCPUs

[2025-02-19 09:23] Matti: that's the advice for the target

[2025-02-19 09:25] Matti: the **host** should not be a VM, and should have as much memory available to it as possible, i.e. the natural state when debugging from your host system

[2025-02-19 09:27] zeropio: do you recommend lowering the specs for the debuggee? more RAM/vCPU should speed up windbg actions right?

[2025-02-19 09:27] zeropio: I remember taking so long when looking for pools

[2025-02-19 09:28] Matti: yeah, basically

[2025-02-19 09:32] Matti: the lower the specs of the debuggee VM, the more headroom your CPU has available to run both it (meaning an entire VM) and the host debugger itself, which isn't enormously resource hungry, but it can still either become resource constrained (due to the VM), and certain actions like enumerating CPU PRCBs, pool memory, and so on are just going to take longer by nature due to having more of them/it

[2025-02-19 09:34] Matti: that said you can still get very acceptable speeds even with specs like the 8 GB/8 vCPUs I'm using here
[Attachments: 2025-01-22_10-46-58.mp4]

[2025-02-19 09:34] Matti: it just depends a lot on settings, and usually isn't necessary

[2025-02-19 09:35] zeropio: I'm using similar specs rn

[2025-02-19 09:36] zeropio: but yeah you are right, taking the pool example, more memory means more pools, so it will take longer to look through them

[2025-02-19 10:01] Matti: oh yeah <@456226577798135808> I haven't seen you confirm yet that you are in fact using kdnet as suggested by <@609487237331288074> above

[2025-02-19 10:02] Matti: if you're using serial then that is always going to be massively slow, and even worse bordering on unusable on real hardware

[2025-02-19 11:05] Matti: virtual network* <:thinknow:475800595110821888> but yeah alright

[2025-02-19 19:30] Glatcher: Guys, if anybody knows, why doesn't frida show the full list of modules the process has? I've inspected today that several modules Cheat Engine shows are missed in frida. Is there an opportunity to hide modules from frida?

[2025-02-19 19:35] Glatcher: I've tried to show memory regions and they are also different: frida lacks regions that should present those "hidden" modules

[2025-02-22 10:15] ----: had anyone encountered a `cos` binary in /sbin/ of an IoT device?

[2025-02-22 10:22] KulaGGin: I wrote the pintool as you suggested <@162611465130475520>  to try to log syscalls and to see when the syscall with 0xD in rax and 0x11 in rdx(NtSetInformationThread(Thread, 0x11, 0, 0)) get executed:
```C++
void BeforeInstruction(INS Instruction, CONTEXT* Context, THREADID ThreadID) {
    ADDRINT RAXValue = PIN_GetContextReg(Context, REG_RAX);
    ADDRINT RDXValue = PIN_GetContextReg(Context, REG_RDX);

    if(RAXValue == 0xD && RDXValue == 0x11) {
        std::cerr << "NtSetInformationThread with ThreadHideFromDebugger instruction detected at address: " << std::hex << "0x" << RIPValue << std::dec << std::endl;
        *out << std::flush;
    }
}

// Callback function for instrumenting each instruction
void Instruction(INS Instruction, void* v) {
    // Check if the instruction is a system call
    if(INS_IsSyscall(Instruction)) {
        INS_InsertCall(Instruction, IPOINT_BEFORE, (AFUNPTR)BeforeInstruction,IARG_INST_PTR, IARG_CONTEXT, IARG_THREAD_ID, IARG_END);

        std::cerr << "System call instruction detected at address: " << std::hex << "0x" << INS_Address(Instruction) << std::dec << std::endl;
        *out << std::flush;
    }
}

int main(int argc, char* argv[])
{
    // Initialize Pin
    if(PIN_Init(argc, argv)) {
        std::cerr << "This tool requires Pin to be initialized!" << std::endl;
        std::cerr << std::flush;
        return -1;
    }

    // Register the instruction analysis function
    INS_AddInstrumentFunction(Instruction, 0);

    // Start the program and begin instrumentation
    PIN_StartProgram();
    return 0;
}
```

[2025-02-22 10:22] KulaGGin: The problem is that there is no syscalls with rax 0xD and rdx 0x11 at all. But `NtQueryInformationThread` returns that it's hidden at one point from another injected DLL:
```C++
bool IsThreadHidden(HANDLE Thread) {
    ULONG HideThread = 0;
    ULONG OutputSize = sizeof(ULONG);
    auto NTStatus = (NtQueryInformationThread)(Thread, ThreadHideFromDebugger, &HideThread, sizeof(BOOLEAN), &OutputSize);
    if(NTStatus != 0)
        std::cout << "NtQueryInformationThread wasn't successful\n";
    return HideThread != 0;
}
```

[2025-02-22 10:24] KulaGGin: When I attach this pintool to my own program that hides the main thread:
```C++
void main(int argc, char* argv[]) {
    HMODULE ntDll = LoadLibrary("ntdll.dll");
    NtSetInformationThread = (_NtSetInformationThread)GetProcAddress(ntDll, "NtSetInformationThread");
    NtQueryInformationThread = (_NtQueryInformationThread)GetProcAddress(ntDll, "NtQueryInformationThread");

    std::cout << std::dec;
    std::cout << "Before SetThreadNotDebuggable called:\n";
    auto ThreadIsHidden = IsThreadHidden(GetCurrentThread());
    std::cout << "Thread #" << GetCurrentThreadId() << " is hidden: " << ThreadIsHidden << "\n";
    std::cout << std::flush;

    SetThreadNotDebuggable(GetCurrentThread());

    std::cout << "\nAfter SetThreadNotDebuggable called:\n";
    ThreadIsHidden = IsThreadHidden(GetCurrentThread());
    std::cout << "Thread #" << GetCurrentThreadId() << " is hidden: " << ThreadIsHidden << "\n";
    std::cout << std::flush;
}

void SetThreadNotDebuggable(HANDLE Thread) {
    (NtSetInformationThread)(Thread, 0x11, 0, 0);
}
```
The pintool catches it and logs that syscall is called with those arguments, so the pintool works.

[2025-02-22 10:26] KulaGGin: Is there any other way a thread can make itself hidden from user mode, so that `NtQueryInformationThread` returns that it is hidden?

[2025-02-22 13:55] deja: [replying to ----: "had anyone encountered a `cos` binary in /sbin/ of..."]
I have looked at a fair amount and `cos` does not stand out to me

[2025-02-22 13:56] ----: yeah well probably a custom binary, when I will extract the "true" firmware from the device I guess it would be there

[2025-02-22 14:20] Horgm: I have some dma software development that I don't understand. Can we discuss it? I can pay you a fee

[2025-02-22 17:13] drifter987: does anyone have the FunctionStringAssociate plugin for IDA 9.0.240807?

[2025-02-22 19:32] NoWayDev: [replying to drifter987: "does anyone have the FunctionStringAssociate plugi..."]
this one works for me: https://github.com/cafeed28/IDA_FunctionStringAssociate_PlugIn_IDA90/releases/tag/ida90
[Embed: Release ida 9.0 support · cafeed28/IDA_FunctionStringAssociate_Plug...]
Full Changelog: https://github.com/cafeed28/IDA_FunctionStringAssociate_PlugIn_IDA90/commits/ida90

[2025-02-22 19:32] NoWayDev: using IDA 9.0.241217

[2025-02-22 19:52] mrexodia: [replying to KulaGGin: "Is there any other way a thread can make itself hi..."]
I don’t think so, unless there is a kernel component involved

[2025-02-22 19:53] mrexodia: Are you using -smc_strict?

[2025-02-22 19:57] KulaGGin: No, but I managed to fix it and you were right, it was just `NtSetInformationThread`. I just added my dll to import table and hooked it in DllMain, and in the hook I just return 0 if it's to hide the thread:
```
NTSTATUS NtSetInformationThread_Detour(HANDLE ThreadHandle,
                                       ULONG ThreadInformationClass,
                                       PULONG ThreadInformation,
                                       ULONG ThreadInformationLength) {
  if(ThreadInformationClass == ThreadHideFromDebugger)
    return 0;

  return OriginalNtSetInformationThread(ThreadHandle, ThreadInformationClass, ThreadInformation, ThreadInformationLength);
}
```

[2025-02-22 19:59] KulaGGin: I'm not sure why I couldn't catch it in debuggers with conditional breakpoints: I did loop the executable at start with `EB FE` to jump 2 instructions back infinitely, then attach te debugger on this entry point, then change the bytes to the original `E8 00`, and then I definitely did breakpoint on `NtSetInformationThread` and put a conditional breakpoint on when edx is 0x11 but it didn't catch that.
And attaching with ScyllaHide also wouldn't work.

[2025-02-22 20:00] KulaGGin: Thank you for all the help and pointing me in the right direction

[2025-02-22 20:00] KulaGGin: https://cdn.discordapp.com/emojis/1014169296672411799.png?size=48

[2025-02-23 12:17] Matti: [replying to KulaGGin: "Is there any other way a thread can make itself hi..."]
there actually is

[2025-02-23 12:18] Matti: well, a thread can't make **itself** hidden any other way

[2025-02-23 12:18] Matti: but you can create a thread with this flag set at creation time using `NtCreateThreadEx`

[2025-02-23 12:19] Matti: `#define THREAD_CREATE_FLAGS_HIDE_FROM_DEBUGGER 0x00000004`

[2025-02-23 12:37] KulaGGin: Very interesting

[2025-02-23 12:43] KulaGGin: [replying to Matti: "but you can create a thread with this flag set at ..."]
And it does the exact same thing: sets the flag on the ETHREAD struct?

[2025-02-23 12:44] KulaGGin: So, just out of curiosity, can another process start the process with the main thread already hidden, so hooking NtSetInformationThread and then looking for syscall also wouldn't work, so one would have to use a driver to unset it?

[2025-02-23 12:45] Matti: [replying to KulaGGin: "And it does the exact same thing: sets the flag on..."]
yes

[2025-02-23 12:47] Matti: [replying to KulaGGin: "So, just out of curiosity, can another process sta..."]
yes, NtCreateUserProcess takes both process and thread flags

[2025-02-23 12:47] Matti: alternatively you could create a 0-thread process using NtCreateProcess[Ex] and then create the thread manually, but that is a lot more work

[2025-02-23 14:04] drifter987: [replying to NoWayDev: "this one works for me: https://github.com/cafeed28..."]
thanks!

[2025-02-23 21:21] diversenok: [replying to Matti: "yes, NtCreateUserProcess takes both process and th..."]
The only supported thread flag in `NtCreateUserProcess` is for suspension

[2025-02-23 21:21] diversenok: Other flags are for `NtCreateThreadEx`-only

[2025-02-23 23:37] inconcel: 
[Attachments: image.png]

[2025-02-23 23:39] inconcel: Wait that screenshot is actually wrong

[2025-02-23 23:39] inconcel: Thats actually PDRIVER_UNLOAD

[2025-02-23 23:40] inconcel: cause the offsets are decimal

[2025-02-23 23:41] inconcel: But yeah how can i make it the correc type here