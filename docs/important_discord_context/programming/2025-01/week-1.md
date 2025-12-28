# January 2025 - Week 1
# Channel: #programming
# Messages: 225

[2025-01-01 00:03] .: because to detect detours you can compare the module on disk vs the module in memory byte by byte and detect discrepancies in sections that shouldnt change in memory like .text

[2025-01-01 00:07] .: but in the case of apc injection the queues are stored at kernel level in the KTHREAD struct

[2025-01-01 00:08] .: and can be queued from external processes, they need your threads to be in alertable states but normally you will always find a thread waiting for something

[2025-01-01 00:10] .: in terms of being stealthy i would prefer apcs, you would only have to bypass a few things, i guess a usermode EDR would stack-trace the threads of the process

[2025-01-01 00:11] diversenok: Normal APCs require alertable state, yes, but you can also queue special user-mode APCs which don't

[2025-01-01 00:12] Atom: requiem sounds like a bit of giipiitii

[2025-01-01 00:12] diversenok: Modern EDRs are supposed to use EtwTi to detect cross-process APCs, there is a special event for them

[2025-01-01 00:12] diversenok: Older ones can hook KiUserApcDispatcher

[2025-01-01 00:13] .: bypassing ETW is fairly easy, and for kernel EDRs theres a lot of things you can to detect them yes, i didnt know you can queue user-mode apcs without requiring alertable states

[2025-01-01 00:13] .: that makes APC Injection even better

[2025-01-01 00:14] diversenok: EtwTi is not like user-mode ETW which you can patch out

[2025-01-01 00:14] diversenok: It's a kernel provider

[2025-01-01 00:15] .: didnt even find documentation about it

[2025-01-01 00:15] Atom: gotta look into ntosk

[2025-01-01 00:15] .: i remember stack tracing my threads because i know they should point to specific memory pages in my program

[2025-01-01 00:15] .: would also prevent normal thread hijacking without needing to check if they enter in suspended mode

[2025-01-01 00:16] .: while APC injection will redirect the RIP to their shellcode (as with any other injection method)

[2025-01-01 00:16] Atom: [replying to .: "i remember stack tracing my threads because i know..."]
what

[2025-01-01 00:16] .: for example game variables

[2025-01-01 00:16] .: each process heap segments where they were stored were located at specific offsets

[2025-01-01 00:17] Atom: you cant reliably detect stuff like that just by walking the stack by interrupting it, unless you hook something deep

[2025-01-01 00:17] .: APC injection is a variant of thread hijacking, indeed you can detect if the threads were hijacked by stack tracing where they are

[2025-01-01 00:18] .: you dont need "something deep"

[2025-01-01 00:18] Atom: know what, you're right im wrong

[2025-01-01 00:18] Atom: my bad

[2025-01-01 00:18] Atom: gonna dip

[2025-01-01 00:20] diversenok: [replying to .: "didnt even find documentation about it"]
Here is the definition, search for `QUEUEUSERAPC` in it
[Attachments: Microsoft-Windows-Threat-Intelligence.xml]

[2025-01-01 00:21] .: did you get that info from github?

[2025-01-01 00:21] .: i didnt find anything like that in MSDN

[2025-01-01 00:21] diversenok: EtwExplorer can show these

[2025-01-01 00:22] diversenok: https://github.com/zodiacon/EtwExplorer

[2025-01-01 00:22] .: oh i remember vaguely about etwexplorer

[2025-01-01 00:22] .: yeah it was a good tool

[2025-01-01 00:22] diversenok: Provider name is `Microsoft-Windows-Threat-Intelligence`

[2025-01-01 00:22] elias: is there any official documentation for etwti?

[2025-01-01 00:23] elias: I can only find non-microsoft articles

[2025-01-01 00:23] diversenok: Might be only for antivirus vendors

[2025-01-01 00:24] elias: <:peepoDetective:570300270089732096>

[2025-01-01 00:24] elias: probably

[2025-01-01 00:25] .: and apc injection doesnt need to open process handles

[2025-01-01 00:25] diversenok: Yeah, that's actually a really good point

[2025-01-01 00:25] .: whats the way to queue an APC without needing alertable state

[2025-01-01 00:26] diversenok: https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-queueuserapc2
[Embed: QueueUserAPC2 function (processthreadsapi.h) - Win32 apps]
Adds a user-mode asynchronous procedure call (APC) object to the APC queue of the specified thread. (QueueUserAPC2)

[2025-01-01 00:26] diversenok: https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/types-of-apcs

[2025-01-01 00:26] Atom: you need to open a handle for the thread

[2025-01-01 00:26] .: [replying to Atom: "you need to open a handle for the thread"]
thats why i mentioned "process" handles

[2025-01-01 00:26] .: not thread handles

[2025-01-01 00:27] diversenok: [replying to Atom: "you need to open a handle for the thread"]
Which is, to be fair, is a much less monitored operation

[2025-01-01 00:27] Atom: ye

[2025-01-01 00:28] diversenok: Like, Sysmon allows monitoring process handles but not thread handles

[2025-01-01 00:28] diversenok: Even though it's literally the same callback

[2025-01-01 00:28] Atom: but most kernel edrs/acs would strip it

[2025-01-01 00:28] .: imagine you want to detect process handles opened to your process with risky accesses like VM_OPERATION from userspace, so you start looping the system handle table to detect them, but then they hijack a handle lets say from PcaSvc (because your program is opened at administrator and its not a PPL process)

[2025-01-01 00:28] Atom: [replying to diversenok: "Even though it's literally the same callback"]
laziness

[2025-01-01 00:29] .: so its ok to detect unsigned programs with an open process handle to signed programs that have an opened handle to your program?

[2025-01-01 00:29] .: lol recursive loop

[2025-01-01 00:29] diversenok: What

[2025-01-01 00:30] diversenok: Ahh, okay, I get it

[2025-01-01 00:30] diversenok: Why looping the handle table though

[2025-01-01 00:30] diversenok: EDRs have drivers that can subscribe to Ob- callbacks

[2025-01-01 00:31] .: imagine you're a usermode edr

[2025-01-01 00:31] .: like bitdefender

[2025-01-01 00:31] diversenok: Huh?

[2025-01-01 00:31] Atom: Bro what

[2025-01-01 00:31] .: or a usermode anticheat

[2025-01-01 00:32] Atom: are there any commercial usermode edrs which actually exist? i dont think so

[2025-01-01 00:32] diversenok: By user-mode you mean heavily relying on user-mode hooks or not having a kernel component?

[2025-01-01 00:32] Atom: not having a kernel comp

[2025-01-01 00:32] .: not having a kernel comp

[2025-01-01 00:32] .: [replying to diversenok: "Why looping the handle table though"]
i made a poc of this a while ago in one of my git accs https://github.com/diegcrane/PcaSvc-Proxy
[Embed: GitHub - diegcrane/PcaSvc-Proxy: Handle hijacking]
Handle hijacking. Contribute to diegcrane/PcaSvc-Proxy development by creating an account on GitHub.

[2025-01-01 00:33] diversenok: And bitdefender is one of them? Interesting

[2025-01-01 00:33] Atom: bitdefender legit has a hv too

[2025-01-01 00:33] .: so im making another poc to detect them

[2025-01-01 00:33] Atom: its kernel

[2025-01-01 00:33] .: [replying to Atom: "its kernel"]
some1 told me it placed usermode hooks

[2025-01-01 00:33] .: and i confirmed it

[2025-01-01 00:33] Atom: just because it places usermode hooks doesnt mean it doesnt have a driver

[2025-01-01 00:33] .: so why would it place usermode hooks when you can place kernel hooks

[2025-01-01 00:34] .: setting usermode hooks in every process

[2025-01-01 00:34] diversenok: Oh, who knows, honestly

[2025-01-01 00:34] Atom: in every process?

[2025-01-01 00:34] Atom: wtf

[2025-01-01 00:34] .: yes

[2025-01-01 00:34] diversenok: Laziness

[2025-01-01 00:34] diversenok: Legacy code

[2025-01-01 00:34] .: lemme find the disassembly i did

[2025-01-01 00:34] .: wait

[2025-01-01 00:34] diversenok: Yeah, I can totally believe about um hooks

[2025-01-01 00:35] .: 
[Attachments: image.png, image2.png]

[2025-01-01 00:35] .: and not landing without a valid (signed) module

[2025-01-01 00:35] .: and I've never seen 36 36 36 36 before too

[2025-01-01 00:35] Atom: the dll is on disk (its not manual mapped)

[2025-01-01 00:35] diversenok: VMware Carbon Black does that as well, and it's an EDR, not just an AV (and definitely with a driver)

[2025-01-01 00:36] .: Bitdefender Premium, Carbon Black, Sophos, SentinelOne, McAfee, Cylance, and CrowdStrike Falcon

[2025-01-01 00:36] .: to be exact

[2025-01-01 00:37] .: [replying to Atom: "the dll is on disk (its not manual mapped)"]
and what

[2025-01-01 00:37] .: it fucks up hook detection

[2025-01-01 00:37] .: from usermode

[2025-01-01 00:37] .: we're supposed to recursive following the jmp chain and checking if it lands in a valid module

[2025-01-01 00:38] .: we had to whitelist a lot of stuff like that

[2025-01-01 00:38] Atom: https://tenor.com/view/sad-emoji-sad-emoji-emoji-stare-disgust-gif-405076546600269050

[2025-01-01 00:38] .: sigscanning for the hook procedures of every EDR vendor that did that before flagging a malicious hook

[2025-01-01 00:39] .: one pattern i noticed is that in the case of bitdefender all ntdll hooks landed in the main module

[2025-01-01 00:39] .: but i would prefer them to land in the bitdefender module

[2025-01-01 00:40] Atom: unhook them 🔥

[2025-01-01 00:41] diversenok: Or just issue syscalls yourself

[2025-01-01 00:41] .: the thing is not bypassing the um hook with direct/indirect syscalls

[2025-01-01 00:42] Atom: wat

[2025-01-01 00:42] .: the thing is preventing programs from placing um hooks in your process

[2025-01-01 00:42] .: in this case

[2025-01-01 00:42] .: detecting and flagging those hooks

[2025-01-01 00:42] .: we false flagged some um hooks of edrs because of that issue

[2025-01-01 00:42] Atom: block page prot changes

[2025-01-01 00:42] diversenok: Ahh, so prevents shellcode from hooking things you mean?

[2025-01-01 00:43] Atom: which is what vgk does with its syscall hook

[2025-01-01 00:43] .: [replying to diversenok: "Ahh, so prevents shellcode from hooking things you..."]
not exactly because they can hook before we load

[2025-01-01 00:43] .: so after the function is patched

[2025-01-01 00:43] .: detect that its a valid hook

[2025-01-01 00:43] .: and detect hooks not made by EDRs or by us

[2025-01-01 00:43] Atom: [replying to .: "not exactly because they can hook before we load"]
how do you load

[2025-01-01 00:44] .: in my case I load at the start of the program but they can hook before your hook check runs

[2025-01-01 00:44] .: anyways this issue is fixed

[2025-01-01 00:45] diversenok: What's the end goal anyway?

[2025-01-01 00:45] .: huh?

[2025-01-01 00:45] .: prevent injection

[2025-01-01 00:45] Atom: for what

[2025-01-01 00:45] .: for my program

[2025-01-01 00:45] Atom: ac?

[2025-01-01 00:45] .: nah

[2025-01-01 00:45] .: it can be any program i made

[2025-01-01 00:45] .: just making a guard module

[2025-01-01 00:46] .: that runs without km comps

[2025-01-01 00:47] .: the final answer is APC injection is fairly better than um hooking

[2025-01-01 00:47] .: in terms of evasion

[2025-01-01 00:48] .: if your program is made in C++ and you do use classes you will have to prevent other kind of detouring like patching your vtables with VMT hooking

[2025-01-01 00:48] .: but the typical ones IAT/EAT/inline in .text can be detected with the module on disk vs in memory comparison

[2025-01-01 00:49] .: and about VEH hooking

[2025-01-01 00:49] .: https://github.com/NotRequiem/veh-disasm the VEH chain is undocumented but it can be easily figured out where it is

[2025-01-01 00:49] .: it kinda depends on the detouring method but in general APC is better

[2025-01-01 00:50] diversenok: APCs have a good baseline difficulty of detection, but I think you can go a bit further with modifying remote memory if you employ some tricks

[2025-01-01 00:51] diversenok: Not via a remote RWX allocation, of course

[2025-01-01 00:53] diversenok: [replying to .: "but the typical ones IAT/EAT/inline in .text can b..."]
How do you deal with relocations?

[2025-01-01 00:55] .: [replying to diversenok: "How do you deal with relocations?"]
precisely because you're comparing the module on disk vs the module in memory byte by byte you don't need to deal with that, thats why I do prefer to not traverse the import/export table and checking if their pointers land within a valid memory module range

[2025-01-01 00:55] .: [replying to diversenok: "Not via a remote RWX allocation, of course"]
i remember doing a hook in NtAllocateVirtualMemory so that if an executable page was not allocated by the process itself and was allocated externally, it should be part of a digitally signed image, they cant bypass the hook with syscalls because you need to trigger this hook if you want to allocate externallyt

[2025-01-01 00:55] .: meaning you force them to write in pages that you allocate, which are hashed of course

[2025-01-01 00:56] diversenok: [replying to .: "precisely because you're comparing the module on d..."]
What do you mean? Some compilers generate relocations inside .text, which means that under mandatory ASLR the data in memory will be different from the data on disk

[2025-01-01 00:57] .: its not relocated for any of the programs i made, they always matched

[2025-01-01 00:58] .: note that i usually make programs run under the JVM

[2025-01-01 00:58] .: to ensure compatibility across different os

[2025-01-01 00:58] .: might be a reason

[2025-01-01 00:59] .: thats how i made the comparison
[Attachments: image.png]

[2025-01-01 00:59] .: ```cpp
const auto compare_bytes = [](const std::uint8_t* __restrict a, const std::uint8_t* __restrict b, const size_t size, const int offset = 0) {
    constexpr int no_difference = -1;
    __assume(size > 0);
    for (int i = offset; i < size; ++i) {
        if (a[i] != b[i]) {
            return i;
        }
    }
    return no_difference;
};
````

[2025-01-01 01:00] diversenok: [replying to .: "i remember doing a hook in NtAllocateVirtualMemory..."]
I still don't get it. You have a target process that you protect. And there is an attacker process which tries to allocate RWX in your target. Which of them has NtAllocateVirtualMemory hooked?

[2025-01-01 01:01] .: there is a target process that you protect with a NtAllocateVirtualMemory hook that you place

[2025-01-01 01:01] .: there are some hooks, tho, that you dont place

[2025-01-01 01:01] .: which are placed by the attacker

[2025-01-01 01:03] .: the hook check ensures that hooks placed by EDR or by the guard module doesnt get flagged, and hooks placed by an attacker in any other part of important modules are detected

[2025-01-01 01:03] .: to whitelist your own hooks is simple
[Attachments: image.png]

[2025-01-01 01:05] diversenok: So it's pretty much a self integrity check, right?

[2025-01-01 01:05] .: yea

[2025-01-01 01:06] diversenok: That assume that the integrity checking logic itself is not yet compromised, right?

[2025-01-01 01:06] diversenok: Well, I guess it's going to work against automated attackers

[2025-01-01 01:06] .: the integrity checking logic itself is checked too with a CRC32 checksum

[2025-01-01 01:07] .: but well

[2025-01-01 01:07] .: they can patch the checksum check too and so on

[2025-01-01 01:09] .: that's why I kinda try to obfuscate/virtualize and confusing with signatures that look exactly like the self-integrity code placed at random locations

[2025-01-01 01:09] .: they act as honey pots

[2025-01-01 01:09] diversenok: Fair

[2025-01-01 01:09] diversenok: You're making a CTF at this point 😂

[2025-01-01 01:10] .: for UM programs it's kinda hard to achieve good protection against specialized attacks

[2025-01-01 01:11] diversenok: Ohh, so the hook on NtAllocateVirtualMemory is not actually to prevent malicious allocations but to know which ones are yours?

[2025-01-01 01:11] .: [replying to diversenok: "Ohh, so the hook on NtAllocateVirtualMemory is not..."]
yes

[2025-01-01 01:11] diversenok: Got it

[2025-01-01 01:11] .: but well it can be used to detect external allocations indirectly

[2025-01-01 01:11] .: If a page was not caught by the hook

[2025-01-01 01:11] .: It means it was allocated by an external process

[2025-01-01 01:12] .: As external processes won't trigger the hook

[2025-01-01 01:12] diversenok: Yeah, interesting approach

[2025-01-01 01:12] diversenok: There are more syscalls that can allocate memory though

[2025-01-01 01:12] .: If you're talking about something like map views they all end up in NtAllocateVirtualMemory

[2025-01-01 01:13] .: what API functions allocate memory without ending in NtAllocateVirtualMemory?

[2025-01-01 01:13] .: all the functions I disassembled in IDA eventually called it

[2025-01-01 01:14] .: but it doesn't matter because all the functions that I use call it

[2025-01-01 01:14] .: so I don't care

[2025-01-01 01:15] .: <@946449225154195466> please let's stop reacting each other messages

[2025-01-01 01:15] .: 😂

[2025-01-01 01:19] diversenok: `NtAllocateVirtualMemoryEx`, `NtMapViewOfSection`, `NtMapViewOfSectionEx`, `NtCreateThreadEx`, `NtSetInformationProcess`, `NtMapUserPhysicalPages`, `NtMapUserPhysicalPagesScatter`

[2025-01-01 01:19] diversenok: And probably much more

[2025-01-01 01:20] diversenok: Some might require changing protection after

[2025-01-01 01:21] diversenok: There are also legitimate scenarios when other processes can allocate memory in your process remotely. CSRSS likes doing that, for instance

[2025-01-01 01:21] diversenok: Oh, and ALPC, of course

[2025-01-01 01:23] .: [replying to .: "i remember doing a hook in NtAllocateVirtualMemory..."]
none of those allocate executable pages
[Attachments: image.png]

[2025-01-01 01:24] .: i think

[2025-01-01 01:24] .: [replying to diversenok: "`NtAllocateVirtualMemoryEx`, `NtMapViewOfSection`,..."]
didnt knew this

[2025-01-01 01:24] .: interesting

[2025-01-01 01:24] diversenok: `NtMapViewOfSection[Ex]` can

[2025-01-01 01:24] diversenok: Others probably don't, unless you count changing page protection later

[2025-01-01 01:26] .: and CSRSS/ALPC are not allocated as executable memory too right?

[2025-01-01 01:27] .: i remember them being allocated as mapped, readable memory only?

[2025-01-01 01:28] diversenok: Yeah, activation contexts, for instance, are mapped read-only regions from CSRSS

[2025-01-01 01:28] .: [replying to .: "interesting"]
probably because just reversing kernel32.dll functions but you mentioned "syscalls"

[2025-01-01 01:28] .: so yh mb

[2025-01-01 01:28] .: [replying to diversenok: "Yeah, activation contexts, for instance, are mappe..."]
yeah true

[2025-01-01 01:29] diversenok: And looks like you cannot change their protection, so no executable memory here, I guess

[2025-01-01 01:30] .: i was about to check it but
[Attachments: image.png]

[2025-01-01 01:31] .: latest ver of system informer canary is bugged

[2025-01-01 01:31] .: it seems

[2025-01-01 01:31] diversenok: Haha, yeah

[2025-01-01 01:31] diversenok: It's an oopsie

[2025-01-01 01:32] .: btw does the traditional method of leaking the system EPROCESS kernel address still work

[2025-01-01 01:33] .: because i was about to test if the ZeroHVCI project that uses two CVEs for arbitrary kernel r/w still works

[2025-01-01 01:33] diversenok: You mean via system handle snapshotting?

[2025-01-01 01:33] .: yes

[2025-01-01 01:34] diversenok: I'm not sure which version enforces requiring the debug privilege for kernel addresses

[2025-01-01 01:34] diversenok: It might still only be on insider

[2025-01-01 01:34] .: i think insider only?

[2025-01-01 01:34] .: yes

[2025-01-01 01:34] .: ok ill check if after enabling that token it still works

[2025-01-01 01:36] .: no they dont
[Attachments: image.png]

[2025-01-01 07:43] Atom: [replying to diversenok: "I'm not sure which version enforces requiring the ..."]
24h2

[2025-01-03 09:02] Nicola: Hi, Programmers.
I have one puzzle but it is hard to solve for me. It is kind of finding rule in some chaos.
Who reverse Engineer can help me?
-------------------
P9847011A9 
P8757892B0 
P5693138A4

[2025-01-03 14:15] Timmy: English.

[2025-01-03 15:07] pinefin: [replying to Timmy: "English."]
he wants us to dereference those pointers

[2025-01-03 18:56] Torph: <:kekw:849303666925371479>

[2025-01-03 23:04] roddux: [replying to Nicola: "Hi, Programmers.
I have one puzzle but it is hard ..."]
`P[0-9]{7}[AB][0-9]`    ?