# June 2024 - Week 3
# Channel: #reverse-engineering
# Messages: 195

[2024-06-10 22:24] iPower: <#835634425995853834>

[2024-06-10 22:25] iPower: we're not trying to get ourselves banned by discord so please avoid discussions about specific anti-cheats.

[2024-06-10 22:38] crash: Anyone know what I'm doing wrong in my code here? I'm trying to clear WP-bit in Cr0 to write but to do that I also need to clear CR4.CET. When I finish doing those two things, i end up faulting (General protection) inside of write_protection_disable() @ __writecr0. ```

void write_protection_disable() {
    UINT64 cr4 = readcr4();
    writecr4(cr4 & 0x7FFFFF); // clear 23rd bit in cr4 (cr4.cet). 
    


    UINT64 cr0 = readcr0(); //clear wp bit
    cr0 &= 0xfffffffffffeffff;
    writecr0(cr0);
    disable();
}
```

[2024-06-10 22:45] iPower: > writecr4(cr4 & 0x7FFFFF); // clear 23rd bit in cr4 (cr4.cet). 
this mask is wrong

[2024-06-10 22:48] iPower: also you should disable() before doing changes to CRs

[2024-06-10 22:49] iPower: your issue might be that you're context switching to another processor and then CR4.CET is enabled there

[2024-06-10 22:49] iPower: moving disable() to the beginning should fix the issue. you should also fix the mask even though for this specific case it doesn't really matter

[2024-06-10 22:51] iPower: another improvement I'd do is checking if CET is really supported and enabled so you don't end up setting unsupported bits (or changing the system to a different state)

[2024-06-10 23:00] Matti: idg what the disable() is doing in the original code at all tbh

[2024-06-10 23:01] Matti: disabling interrupts is not part of disabling WP, so why does a function with this name do that - especially as the very last thing

[2024-06-10 23:01] iPower: got triggered by that as well but decided to not comment lol

[2024-06-10 23:01] Matti: it could be useful to prevent context switches like <@789295938753396817> said, but in that case put it at the start and finish with enable()

[2024-06-10 23:02] Matti: but tbh, why not just test in a VM with 1 CPU

[2024-06-10 23:02] Matti: another issue I see is that idt you can't* just clear CR4.CET even if it's on, if kernel shadow stacks have been enabled

[2024-06-10 23:04] Matti: see <https://github.com/Mattiwatti/EfiGuard/blob/master/EfiGuardDxe/X64/Cet.nasm> for a version that worked for me when I tested it with windows KCET on at least

[2024-06-10 23:05] Matti: not sure if user shadow stacks affect anything here, this code only ever runs in kernel mode before there's even an OS, let alone user mode processes

[2024-06-10 23:05] Matti: also the reason for the lack of cli...

[2024-06-10 23:06] Matti: [replying to Matti: "see <https://github.com/Mattiwatti/EfiGuard/blob/m..."]
<https://github.com/Mattiwatti/EfiGuard/blob/master/EfiGuardDxe/util.c#L124> calling function for this btw

[2024-06-10 23:07] iPower: [replying to Matti: "not sure if user shadow stacks affect anything her..."]
> Control-flow Enforcement Technology (bit 23 of CR4) — Enables control-flow enforcement technology
> when set. See Chapter 17, “Control-flow Enforcement Technology (CET)‚” of the IA-32 Intel® Architecture
> Software Developer’s Manual, Volume 1. This flag can be set only if CR0.WP is set, and it must be
> clear before CR0.WP can be cleared (see below).
ig only CR0.WP makes any difference

[2024-06-10 23:07] Matti: I should actually add a cli here now since this code can now also run aftter boot

[2024-06-10 23:07] crash: [replying to iPower: "> writecr4(cr4 & 0x7FFFFF); // clear 23rd bit in c..."]
Hi thanks for the responses. Why is this mask wrong? I'm looking at in binary and it seems correct unless I'm misunderstanding how to mask certain bits (b0111 1111 1111 1111 1111 1111).

[2024-06-10 23:08] iPower: [replying to crash: "Hi thanks for the responses. Why is this mask wron..."]
your mask ignores the upper bits

[2024-06-10 23:08] crash: oh so should it be 0xFF7FFFFF?

[2024-06-10 23:09] Matti: [replying to iPower: "> Control-flow Enforcement Technology (bit 23 of C..."]
yeah, I mean in the sense that you must set CR0.WP=1 before CR4.CET=1, and CR4.CET=0 before CR0.WP=0

[2024-06-10 23:10] Matti: but it's not necessarily safe to just write 0 to CR4.CET depending on shadow stack usage being enabled when you do this

[2024-06-10 23:10] Matti: though, idk when exactly this will go wrong and in what way

[2024-06-10 23:10] Matti: it's not gonna #GP on writing to CR4 I'm pretty sure

[2024-06-10 23:11] Matti: but when you re-enable CET I'm pretty sure you're gonna run into issues if you don't do* the manual accounting here

[2024-06-10 23:12] iPower: [replying to crash: "oh so should it be 0xFF7FFFFF?"]
for correctness: 0xFFFFFFFFFF7FFFFF

[2024-06-10 23:12] iPower: but i prefer using (1 << bit_pos) instead of these masks

[2024-06-10 23:12] iPower: then you can just add a `~`

[2024-06-10 23:13] Matti: yeah left shifting is much more readable

[2024-06-10 23:18] crash: thanks for the responses, ill test when i get home

[2024-06-10 23:26] crash: [replying to iPower: "but i prefer using (1 << bit_pos) instead of these..."]
i would cast the 1 like this correct: 1ULL to make sure thr compiler doesnt try to optimize it

[2024-06-10 23:35] iPower: [replying to Matti: "but when you re-enable CET I'm pretty sure you're ..."]
oh yeah true. there's a chance the shadow stack and current stack might mismatch when doing a RET

[2024-06-10 23:37] iPower: [replying to crash: "i would cast the 1 like this correct: 1ULL to make..."]
mmmm I wouldn't say it's related to optimization. more like ensuring you're working with the correct data types so you don't get undesired behavior because of wrong code generation

[2024-06-11 02:07] crash: Hey! So I did some cleaning up and I have this now. For some reason, it throws a #GP (on baremetal and inside vmware) inside write_protection_disable when writing the cr0.  ```
#define _BITULL(x)    ((1ULL) << (x))
#define X86_CR4_CET_BIT        23 /* enable Control-flow Enforcement Technology */
#define X86_CR4_CET        _BITULL(X86_CR4_CET_BIT)
#define X86_CR0_WP_BIT        16 /* Write Protect */
#define X86_CR0_WP        _BITULL(X86_CR0_WP_BIT)



void write_protection_enable() {
    

    UINT64 cr0 = readcr0();
    writecr0(cr0 | X86_CR0_WP);

    UINT64 cr4 = readcr4();
    writecr4(cr4 | X86_CR4_CET);

    _enable();
}


void write_protection_disable() {
    _disable();

    UINT64 cr4 = readcr4();
    writecr4(cr4 & ~X86_CR4_CET);
    


    UINT64 cr0 = readcr0(); //clear wp bit
    writecr0(cr0 & ~X86_CR0_WP);
   
}
```

[2024-06-11 02:09] crash: I'm assuming that the cr4 isn't being set correctly but I don't understand why

[2024-06-11 02:20] crash: 
[Attachments: image.png]

[2024-06-11 02:21] crash: Apparently the instruction isn't even being modified since the bit is already cleared. So, writing to cr0 is the problem i guess

[2024-06-11 02:36] crash: Alright I fixed it my apologies. I was writing to CET again even when it wasn't turned on

[2024-06-11 02:53] donna🤯: <@834275342046068749> depending on what compiler you use you may find this repo helpful - it just saves having to do bitwise operations and instead you can just use bitwise structs https://github.com/ia32-doc/ia32-doc/tree/main
[Embed: GitHub - ia32-doc/ia32-doc: IA32-doc is a project which aims to put...]
IA32-doc is a project which aims to put as many definitions from the Intel Manual into machine-processable format as possible - ia32-doc/ia32-doc

[2024-06-11 03:33] crash: [replying to donna🤯: "<@834275342046068749> depending on what compiler y..."]
Thank you

[2024-06-11 04:12] crash: Ok so the code works on the VM but faults when running on bare metal. jesus fucking christ. Anyone have an explanation: ```cpp #define _BITULL(x)    ((1ULL) << (x))
#define X86_CR4_CET_BIT        23 /* enable Control-flow Enforcement Technology */
#define X86_CR4_CET        _BITULL(X86_CR4_CET_BIT)
#define X86_CR0_WP_BIT        16 /* Write Protect */
#define X86_CR0_WP        _BITULL(X86_CR0_WP_BIT)
#define CHECK_BIT(var,pos) ((var) & (1ULL<<(pos)))



void write_protection_enable() {
    

    UINT64 cr0 = __readcr0();
        __writecr0(cr0 | X86_CR0_WP);

       
    _enable();
}

void write_protection_disable() {
    _disable();

    UINT64 cr4 = __readcr4(); // if cet bit is set, then unset it
    __writecr4(cr4 & ~X86_CR4_CET);
    

    UINT64 cr0 = __readcr0();  
        __writecr0(cr0 & ~X86_CR0_WP);
   
}
 ```

[2024-06-11 04:54] crash: alright fuck this, time to just use mdl

[2024-06-11 05:43] Matti: [replying to crash: "Ok so the code works on the VM but faults when run..."]
well you're still not checking if kernel shadow stacks are enabled and dealing with that scenario if they are

[2024-06-11 05:44] Matti: don't know if that is the cause though

[2024-06-11 05:45] Matti: where is it faulting on bare metal

[2024-06-11 05:46] Matti: I'd really suggest getting rid of the `_disable()`/`_enable()` calls as well for now and just set your BIOS to limit to 1 CPU with no SMT

[2024-06-11 05:49] Matti: context switches are definitely a possible issue, but disabling interrupts to work around this is just as likely to cause some unexpected issue

[2024-06-11 05:50] Matti: especially if this code is supposed to be running at low IRQL

[2024-06-11 05:50] Matti: or else boot with `/NUMPROC=1 /ONECPU` or whatever the flags are again, I forget

[2024-06-11 05:50] Matti: but preferably just do it in BIOS

[2024-06-11 05:52] Matti: then run the code again (but with the cli/sti removed obviously) so you can at least rule that out as a cause

[2024-06-11 05:53] Matti: the most straightforward explanation I can think of would be that your VM has kernel shadow stacks disabled and your real machine has them enabled for some reason

[2024-06-11 05:54] Matti: but need to know the faulting instruction at the very least

[2024-06-11 05:55] Matti: just make a crash dump in any case, you'll  need to set up kd anyway if you  haven't done so already

[2024-06-11 05:55] crash: Hi sorry i forgot to add that. It faults on writing cr0 after writing x86_cr4_CET

[2024-06-11 05:56] crash: [replying to Matti: "well you're still not checking if kernel shadow st..."]
Is that the same as your efiguarddxe code? I'll try that rn

[2024-06-11 05:56] Matti: yeah that is what that bit of asm code is for

[2024-06-11 05:57] Matti: but if the fault is when writing to cr0 I doubt it will fix your issue

[2024-06-11 05:58] Matti: but, CR4's value did change right? from `whatever` to `whatever & ~CR4.CET`

[2024-06-11 05:58] Matti: and `whatever` had `CR4.CET=1`

[2024-06-11 05:58] crash: Yeah it did

[2024-06-11 05:58] crash: Actually, the CET flag was already cleared

[2024-06-11 05:59] Matti: hmmm

[2024-06-11 06:00] Matti: I don't really have time to dig this up in the SDM now because work soon, but can you do me a favour and just try out the same thing with 1 CPU and SMT off

[2024-06-11 06:00] crash: [replying to Matti: "I don't really have time to dig this up in the SDM..."]
sure

[2024-06-11 06:21] Matti: btw, just checking but you do restore CR4.CET to 1 somewhere in your code right...

[2024-06-11 06:21] Matti: and also obviously only if it was previously 1

[2024-06-11 06:25] Matti: this is why when I added this to efiguard I made the `DisableWriteProtect` function return the previous enabled states of both

[2024-06-11 06:26] Matti: otherwise it becomes very difficult to write an Enable version that always does the right thing

[2024-06-11 09:29] crash: Double checked everything and wrote values to static memory before crashing so I could view them. CR4 value=0x350ef8,  CR0=0x80040031.

[2024-06-11 09:29] crash: No reason for a general protection fault

[2024-06-11 09:38] crash: <@148095953742725120> As a last ditch attempt, I turned off virtualization in my bios and ran a test. It worked successfully... ffs 😭

[2024-06-11 09:38] crash: What sort of shenanigans is microsoft doing? Or am I being raped by a hypervisor rootkit. But for some reason it's fixed now

[2024-06-11 09:39] crash: I mean it makes sense because CET requires virtualization to be enabled, does microsoft have these registers protected with HVCI or something like that?

[2024-06-11 09:48] crash: 
[Attachments: image.png]

[2024-06-11 10:45] averageavx512enjoyer: [replying to crash: "<@148095953742725120> As a last ditch attempt, I t..."]
you had core isolation on while trying to disable write protection?

[2024-06-11 10:45] averageavx512enjoyer: <:pepes:904523905576865832>

[2024-06-11 12:04] Matti: [replying to crash: "I mean it makes sense because CET requires virtual..."]
> CET requires virtualization to be enabled
well yes but also no
more no than yes

[2024-06-11 12:05] Matti: it's only windows that requires VBS to be on to enable KCET for reasons unknown to me

[2024-06-11 12:05] Matti: CET is not dependent on virtualization by itself

[2024-06-11 12:05] Matti: but yeah I do get what you mean

[2024-06-11 12:06] Matti: can you imagine how fun it was to test whether this worked correctly in efiguard
which disables VBS by design

[2024-06-11 23:35] crash: [replying to averageavx512enjoyer: "you had core isolation on while trying to disable ..."]
No I have windows defender turned completely off. Core isolation and memory integrity is also off

[2024-06-11 23:35] crash: But I think for some reason they were still on

[2024-06-11 23:36] crash: I used "sordum window defender disabler" but idk if it worked correctly

[2024-06-12 01:33] Matti: [replying to crash: "No I have windows defender turned completely off. ..."]
never rely on what the settings app reports re: VBS

[2024-06-12 01:33] Matti: "core isolation" and "memory integrity" are meaningless terms anyway

[2024-06-12 01:35] Matti: what you would need for this to have been the cause is the MS hypervisor loaded and probably VBS enabled with no additonal setup needed for the MSR filtering

[2024-06-12 01:36] Matti: this is the default configuration in windows 11 if your PC supports it

[2024-06-12 01:37] Matti: `msinfo32` (run as administrator) is already a huge improvement over whatever the settings app tells you

[2024-06-12 01:38] Matti: you say that disabling VT-x made the issue go away right
did you also confirm afterwards that, before this change, a hypervisor was in fact loaded?

[2024-06-12 01:39] Matti: the answer pretty much has to be yes honestly

[2024-06-12 01:40] Matti: and msinfo32 will at least tell you that correctly

[2024-06-12 01:42] Matti: if you happen to have efiguard installed, `efidsefix.exe -i` will also give a bunch of useful output, quite a bit of it related to VBS

[2024-06-12 01:44] Matti: both still work by querying NT APIs though, the best way is always going to be to read the relevant cpuid/msr/CR bits manually

[2024-06-12 01:45] Matti: this just tends to be faster if you just want to know if VBS is on or not

[2024-06-13 17:56] Deleted User: can someone crack programms for me

[2024-06-13 17:56] Redhpm: no

[2024-06-13 17:56] mrexodia: Banned

[2024-06-13 17:57] Redhpm: good

[2024-06-13 17:57] root: lol

[2024-06-13 18:33] Azrael: [replying to Deleted User: "can someone crack programms for me"]
🤑

[2024-06-13 18:52] dullard: [replying to Deleted User: "can someone crack programms for me"]
Ok, 423 btc

[2024-06-13 18:53] dullard: 🫃🏻 🫃🏼 🫃🏽 🫃🏾 🫃🏿

[2024-06-13 18:55] 0xatul: [replying to dullard: "Ok, 423 btc"]
Nah bro it's 69420 BTC

[2024-06-13 19:41] rin: [replying to mrexodia: "Banned"]
Big mistake, I saw real potential from him

[2024-06-13 19:44] Azrael: So real.

[2024-06-14 07:42] crash: Hi. Am I passing parameters to ObReferenceObjectByHandle correctly here? I have a process in usermode which I want to wait on until it terminates. I perform this by trying to get an object from ObReferenceObjectByHandle to wait on using KeWaitForSingleObject, but ObReferenceObjectByHandle keeps returning INVALID_HANDLE_VALUE ntstatus error. I've double checked that the pid is a real process. Anyone encountered this issue?```cpp
  
NTIFS::PEPROCESS proc2;
HANDLE our_handle = ULongToHandle((UINT32)globals::global_info.process_id); // usermode pid
ObReferenceObjectByHandle(our_handle, SYNCHRONIZE, *PsProcessType, UserMode, (PVOID*)&proc2, nullptr);
```

[2024-06-14 09:41] Matti: [replying to crash: "Hi. Am I passing parameters to ObReferenceObjectBy..."]
short answer: no

[2024-06-14 09:41] Matti: long answer: a PID is not the same as a HANDLE

[2024-06-14 09:41] Matti: to do what you're trying to do, you can do one of two things

[2024-06-14 09:43] Matti: 1. obtain a handle from the PID via ZwOpenProcess, and then your code will work (although pass KernelMode as access mode here)
2. use a different API that will do the same thing for you in one call

[2024-06-14 09:43] Matti: `PsLookupProcessByProcessId` is probably best to use here

[2024-06-14 09:44] abu: Yeah I did use that to get my process using my PID from usermode and it worked

[2024-06-14 09:44] abu: But PsLookupProcessByProcessId returns a PEPROCESS struct, how should I get a HANDLE from that?

[2024-06-14 09:45] Matti: `ObOpenObjectByPointer` can do that if you really need to

[2024-06-14 09:46] Matti: but IME this API is not often useful when you're writing kernel code, because basically, why do you need or want a HANDLE when you've already got a direct pointer

[2024-06-14 09:47] Matti: the answer is usually something like: to give it to a user mode process

[2024-06-14 09:48] Matti: which true, this is a good API for

[2024-06-14 09:48] Matti: it's just another thing that will make me go "...yeah but why though...."

[2024-06-14 09:49] abu: Oh damn I didn't know that this gives a HANDLE back, thanks. I just read the name and passed it off.

[2024-06-14 09:49] Matti: hm? which

[2024-06-14 09:49] abu: [replying to Matti: "hm? which"]
Same problem

[2024-06-14 09:49] Matti: PsLookupProcessByProcessId? it does not

[2024-06-14 09:50] abu: Nah ObOpenObjectByPointer

[2024-06-14 09:50] Matti: yeah but actually it does though

[2024-06-14 09:50] abu: Might as well reimplement this back into my driver as well.

[2024-06-14 09:50] Matti: how does it not

[2024-06-14 09:50] abu: No I'm saying I didn't know it didn't. I'm just thanking u because I had the same problem as <@834275342046068749>

[2024-06-14 09:50] Matti: ah right

[2024-06-14 09:50] abu: Your explanation solved it for me

[2024-06-14 09:51] Matti: sure, np

[2024-06-14 09:51] Matti: I'm just curious as to why you would need this when you've already got an EPROCESS

[2024-06-14 09:51] crash: [replying to Matti: "`ObOpenObjectByPointer` can do that if you really ..."]
Thanks, ill test this out

[2024-06-14 09:51] crash: [replying to Matti: "I'm just curious as to why you would need this whe..."]
Me or him?

[2024-06-14 09:51] Matti: him

[2024-06-14 09:52] Matti: [replying to crash: "Thanks, ill test this out"]
you shouldn't need this, unless there's more to your original question

[2024-06-14 09:52] Matti: use PsLookupProcessByProcessId

[2024-06-14 09:53] crash: Alrighty thanks

[2024-06-14 09:53] Matti: oh yeah, and don't forget to dereference the eprocess once you're done with it

[2024-06-14 09:53] Matti: this will return a referenced pointer

[2024-06-14 10:49] crash: <@148095953742725120> Thanks for all the help these past days. My code's working fine now \:)

[2024-06-14 10:54] Matti: no problem

[2024-06-14 10:54] Matti: there's nothing I can't answer 😎

[2024-06-14 10:55] Matti: only sometimes the answers are wrong

[2024-06-14 10:55] Matti: that's your job to check

[2024-06-14 10:56] snowua: [replying to Matti: "there's nothing I can't answer 😎"]
😎

[2024-06-14 23:50] crash: [replying to Matti: ""]
Is there any reason why KeWaitForSingleObject(&hProc, Executive, KernelMode, FALSE, NULL); would hang a system? Other threads seem to be executing (from DbgPrint output)

[2024-06-14 23:55] Matti: uh well to answer the more general question first: no, even on a single CPU this will work as the thread will simply yield and other threads can continue doing work

[2024-06-14 23:55] abu: [replying to crash: "Is there any reason why KeWaitForSingleObject(&hPr..."]
Check zwopenprocess

[2024-06-14 23:56] abu: U open handle without SYNCHRONIZE access?

[2024-06-14 23:56] Matti: but also: I assume you meant `hProc`, not `&hProc`, here ... though if so, that still looks to me like you're confusing handles and object pointers

[2024-06-14 23:57] Matti: [replying to abu: "U open handle without SYNCHRONIZE access?"]
this is the wrong question to ask

[2024-06-14 23:57] Matti: KeWaitXxx simply doesn't take a handle as input parameter

[2024-06-14 23:57] Matti: Nt/ZwWaitXxx functions do that

[2024-06-15 00:02] Matti: <@834275342046068749>: read
https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/managing-kernel-objects
and
https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/object-handles
[Embed: Managing Kernel Objects - Windows drivers]
Managing Kernel Objects
[Embed: Object Handles - Windows drivers]
Object Handles

[2024-06-15 00:03] Matti: since you've got a process, the object for that would be a (pointer to an) `EPROCESS`

[2024-06-15 00:05] Matti: a handle in NT is a sort of encoding of this pointer, that the kernel can use to find the real object and do things with on your behalf (if the handle has sufficient access for the call)

[2024-06-15 00:06] Matti: oh also important: handles are per-process

[2024-06-15 00:08] Matti: so, all of this makes it so that generally when you're in kernel mode, you'll want to be using object pointers if possible

[2024-06-15 00:08] Matti: they should be referenced before use and dereferenced after

[2024-06-15 00:09] Matti: but they  take up the same amount of storage as a HANDLE without the added lookup overhead on each kernel API call

[2024-06-15 00:10] Matti: that said there are definitely valid uses for handles in KM, it kind of depends on what you're doing

[2024-06-15 00:10] Matti: so: what are you doing?

[2024-06-15 00:19] crash: Hi! Thanks for the resources. I'm just waiting for a process to terminate in kernel. I get a HANDLE using ZwOpenProcess, but I erroneously pass it to KeWaitForSingleObject as a pointer (which isn't correct, from what I"m reading here). Now, I'm going to simply use ZwWaitForSingleObject using that handle but it seems that from your perspective, I should be using ObDerefernceObjectByHandle and then pass the pointer of the object to KeWaitForSingleObject?

[2024-06-15 00:20] Matti: ObRef*, not ObDeref I think you meant

[2024-06-15 00:20] Matti: but yes that is exactly right

[2024-06-15 00:20] crash: Alright

[2024-06-15 00:21] Matti: this is basically the same reason why I told you to use `PsLookupProcessByProcessId` before, instead of `ZwOpenProcess` I mean

[2024-06-15 00:23] Matti: if you do that, you'll receive a referenced pointer to the process directly, rather than creating a handle by calling what is essentially a user mode API, only to then turn it into an object pointer anyway

[2024-06-15 00:23] crash: Hey if I'm ZwOpenProcess on a usermode PID, shold I be passing KernelMode to ObRefObjByHandle?

[2024-06-15 00:23] crash: ```
Specifies the access mode to use for the access check. It must be either UserMode or KernelMode. Drivers should always specify UserMode for handles they receive from user address space.
```

[2024-06-15 00:24] crash: I didn't receive the handle from UserMOde, but I got it from ZwOpenProcess on a UserMode Process...

[2024-06-15 00:24] Matti: well it's basically what it says there

[2024-06-15 00:24] Matti: yeah

[2024-06-15 00:24] Matti: so you received the value from your own logic elsewhere in the driver I assume

[2024-06-15 00:25] crash: Yeah so i assume kernelmode

[2024-06-15 00:25] Matti: what MS are talking about here is if you would have an IRP handler function that accepts user mode input

[2024-06-15 00:25] Matti: and then you taking this parameter and trying to obtain a reference

[2024-06-15 00:26] Matti: that's really the only time you should be passing UserMode - if the input came from user mode

[2024-06-15 00:26] Matti: [replying to crash: "Yeah so i assume kernelmode"]
yep

[2024-06-15 00:43] Azrael: [replying to Matti: "a handle in NT is a sort of encoding of this point..."]
Thanks for the explanation.

[2024-06-15 08:54] Horsie: Any tool that will dump the following disk, for windows:
cr3, base address, size of all processes (including kernel modules)

[2024-06-15 08:59] Horsie: Any format is fine. I don't mind parsing it. Slim chance but I wanted to ask just in case.

[2024-06-16 14:31] kcahres: Do you know any GUI for dissecting a PDF?

[2024-06-16 14:34] contificate: visually or structurally

[2024-06-16 14:45] kcahres: structurally, like inspecting the proprieties and objects by a GUI

[2024-06-16 14:46] kcahres: because if not I have the idea for a good tool that I'll write in Rust and also benefit people

[2024-06-16 14:46] kcahres: (just looking for the weekend project)

[2024-06-16 14:48] mrexodia: I think Cerbero Suite supports PDF

[2024-06-16 14:49] mrexodia: or maybe not? 🤔

[2024-06-16 14:53] kcahres: yeah but it's private source 😦