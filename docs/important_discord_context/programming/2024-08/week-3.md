# August 2024 - Week 3
# Channel: #programming
# Messages: 143

[2024-08-12 09:39] Matti: [replying to mrexodia: "As far as I know that's not possible. Leaf functio..."]
this is true, but in MASM you can force a function to have RUNTIME_FUNCTION metadata by using the `NESTED_ENTRY`/`NESTED_END` macros (as opposed to `LEAF_ENTRY`/`LEAF_END`)
you'll probably also need to add an `END_PROLOGUE` as the first macro invocation in the function before any other instructions (if it really is a leaf function)

[2024-08-12 09:40] Matti: I don't think it's impossible cl.exe/c1.dll has a flag to accomplish this, but if so it's undocumented and I don't know what it is

[2024-08-12 09:51] Matti: one trivial way that should work is to make a .def file that dllexports every single function in your PE

[2024-08-12 09:52] Matti: trying to think of something that would accomplish the same thing without having to dllexport

[2024-08-12 15:10] Matti: <@234331837651091456> if you are really unironically using MSVC: can you try `/HOMEPARAMS` and report if this has any effect (<https://learn.microsoft.com/en-us/cpp/build/reference/homeparams-copy-register-parameters-to-stack>)
otherwise,  I *believe*`-fno-omit-frame-pointer` should work

[2024-08-12 15:10] Matti: in both cases: no guarantees what will happen to inlined functions, you may need to prevent inlining yourself if that is undesirable too

[2024-08-12 15:11] elias: Im gonna try it later <:ThumbsUp:985957232065806387>

[2024-08-12 15:11] elias: thank you very much

[2024-08-12 15:15] Matti: ```c
const void* reference_everything_hack[] = {
  &myfunc1,
  &myfunc2,
  ...
};
#pragma comment(linker, "/include:reference_everything_hack")
```
autogenerating something like this should also work... probably

[2024-08-12 15:20] elias: but seems like more manual work

[2024-08-12 15:20] Matti: well I said autogenerating, not generating

[2024-08-12 15:20] Matti: that would be insane

[2024-08-12 15:20] Matti: but the overhead from `/HOMEPARAMS` may be quite significant

[2024-08-12 15:21] Matti: if you're using gcc/clang, `-fno-omit-frame-pointer` is the best option

[2024-08-12 15:21] elias: maybe a little more context to my question. I want to easily identify the start of every function in a compiled PE. I thought if I can force msvc to add an entry for every function in the exception table that would be useful

[2024-08-12 15:21] elias: otherwise I‘d probably just use the pdb

[2024-08-12 15:21] elias: to find all the functions

[2024-08-12 15:22] Matti: why pdb? a map file would be simpler for this

[2024-08-12 15:22] sariaki: this is such a xy problem

[2024-08-12 15:22] Matti: that too

[2024-08-12 15:23] Timmy: ```cpp
struct force_link {
    explicit force_link(const void* p) {
      const auto v = reinterpret_cast<uint64_t>(p);
      SetLastError(static_cast<DWORD>(v));
    }
};
```

[2024-08-12 15:23] Timmy: it aint stupid, if it works

[2024-08-12 15:23] Matti: what if the function is inlined? where are you expecting to find it then? (pdb, map, pdata - doesn't matter)
why do you need to find the start of these functions?

[2024-08-12 15:24] Matti: generally IMO if something is a leaf function, it's rarely worth finding anyway

[2024-08-12 15:24] Matti: but maybe I'm missing some use case

[2024-08-12 15:26] elias: [replying to Matti: "what if the function is inlined? where are you exp..."]
I want to implement a vm obfuscation mechanism that decrypts single instructions using a self modifying key

[2024-08-12 15:26] elias: that requires me to find all the function entries when encrypting the instructions

[2024-08-12 15:27] elias: I dont think inlined functions would be a problem here

[2024-08-12 17:05] mrexodia: Pdb time?

[2024-08-13 09:28] Windows2000Warrior: <@148095953742725120> hello ,i want to ask you but I'm not sure if this would be feasible, can I integrate code of the missings HAL and ntoskrnl functions from ReactOS into the storport driver of reactos to make it work with 2000?


HAL

KeAcquireInStackQueuedSpinLock
KeReleaseInStackQueuedSpinLock


NTOSKRNL 

IoAttachDeviceToDeviceStackSafe
IoForwardIrpSynchronously
KeAcquireInterruptSpinLock
KeReleaseInterruptSpinLock
vDbgPrintExWithPrefix

[2024-08-13 09:55] Windows2000Warrior: I am concerned about the internal structural variations that might arise.

[2024-08-13 10:03] Matti: spinlock functions: yes, these are standalone and only touch their input/output parameters
vDbgPrintExWithPrefix: only if you implement this as a no-op. a real implementation would need to access internal ntoskrnl data
both Io functions: no, these reference internal global state of the kernel

[2024-08-13 10:08] Windows2000Warrior: [replying to Matti: "spinlock functions: yes, these are standalone and ..."]
Well, 
So both io functions need to be rewritten specifically to be compatible with Windows 2000? Means reactos code is impossible in those

[2024-08-13 10:09] Matti: no, I mean they are impossible to place in storport instead of the kernel

[2024-08-13 10:10] Matti: at least in any sane way

[2024-08-13 10:10] Windows2000Warrior: [replying to Matti: "no, I mean they are impossible to place in storpor..."]
Oh, okay, I understand, but what is the best solution in this case?

[2024-08-13 10:11] Matti: uh, I'm trying to resist saying "don't do it"

[2024-08-13 10:11] Matti: really I am

[2024-08-13 10:11] Matti: but I guess if you really wanted to do this, you could find some padding space in ntoskrnl to implement these manually, and then add them to the export table

[2024-08-13 10:14] Windows2000Warrior: [replying to Matti: "but I guess if you really wanted to do this, you c..."]
Well maybe I can, but I'm wondering if there are older alternatives in Windows 2000 kernel that can do similar things like those both io

[2024-08-13 10:17] Matti: "similar things"?

[2024-08-13 10:18] Matti: what do you expect will happen if storport calls an API that it expects to do something, and it does something *similar* instead

[2024-08-13 10:21] Matti: by the way, my copy of NT 5.0 has `IoAttachDeviceToDeviceStackSafe`...

[2024-08-13 10:22] Windows2000Warrior: [replying to Matti: "what do you expect will happen if storport calls a..."]
Well, maybe a glitch occurs in this case?

[2024-08-13 10:22] Matti: yes, we can't know for sure of course but I would say that is a pretty good fucking guess

[2024-08-13 10:23] Windows2000Warrior: [replying to Matti: "by the way, my copy of NT 5.0 has `IoAttachDeviceT..."]
What do you mean by your version of NT 5.0

[2024-08-13 10:23] Matti: of the kernel

[2024-08-13 10:23] Matti: what else

[2024-08-13 10:27] Windows2000Warrior: Well, maybe English makes me not understand the purposes sometimes, I thought you had a version for NT 5.0 and I was able to write  IoAttachDeviceToDeviceStackSafe

[2024-08-13 10:28] Matti: what I am saying is

[2024-08-13 10:28] Matti: you already have `IoAttachDeviceToDeviceStackSafe`

[2024-08-13 10:28] Matti: unless you are running windows 2000 RTM or something

[2024-08-13 10:29] Matti: if you haven't got it, it's time to update your windows.... 2000

[2024-08-13 10:30] Windows2000Warrior: [replying to Matti: "if you haven't got it, it's time to update your wi..."]
Ok I'll double check that, but I'm using a 2000 sp4 with rollup 1 of 2005.

[2024-08-13 10:31] Matti: 
[Attachments: ntkrnlpa5.0.exe]

[2024-08-13 10:46] Windows2000Warrior: [replying to Matti: ""]
Well really sorry, I have a lot of 2000 systems installed between real and virtual. I did not notice that the system I checked did not contain rollup1. I was really confused.

[2024-08-13 11:04] Matti: my other question is, why not modify the reactos storport sources so that it doesn't need `IoForwardIrpSynchronously`

[2024-08-13 11:05] Matti: the MS version doesn't use it, so clearly it's possible for storport to do without

[2024-08-13 11:10] Windows2000Warrior: [replying to Matti: "the MS version doesn't use it, so clearly it's pos..."]
If so, that's even better

[2024-08-13 11:15] Windows2000Warrior: [replying to Matti: "my other question is, why not modify the reactos s..."]
Or call things existed like that is possible ?
`NTSTATUS StorportForwardIrpSynchronously(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
{
    KEVENT event;
    NTSTATUS status;

    // Initialize an event object to wait on
    KeInitializeEvent(&event, NotificationEvent, FALSE);

    // Copy the current stack location to the next stack location
    IoCopyCurrentIrpStackLocationToNext(Irp);

    // Set up the completion routine
    IoSetCompletionRoutine(
        Irp,
        CompletionRoutine,  // Custom completion routine
        &event,             // Pass the event as context
        TRUE,
        TRUE,
        TRUE
    );

    // Call the next driver in the stack
    status = IoCallDriver(DeviceObject, Irp);

    // Wait for the IRP to complete
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = Irp->IoStatus.Status; // Use the IRP's final status
    }

    return status;
}

// Completion routine
NTSTATUS CompletionRoutine(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    )
{
    PKEVENT event = (PKEVENT)Context;

    // Check if the IRP was successfully completed
    if (Irp->PendingReturned) {
        KeSetEvent(event, IO_NO_INCREMENT, FALSE); // Signal the event to wake up the waiting thread
    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}`

[2024-08-13 11:20] Matti: go ask a compiler

[2024-08-13 11:20] Matti: I think it will compile but you never know

[2024-08-13 11:21] Matti: if so, then yes this is possible

[2024-08-13 11:21] Windows2000Warrior: [replying to Matti: "go ask a compiler"]
Ok I will, but sorry for the many questions

[2024-08-13 11:21] Matti: you are doing it wrong

[2024-08-13 11:22] Matti: I already told you why it is not possible to implement `IoForwardIrpSynchronously` in storport

[2024-08-13 11:22] Matti: **and** that the MS driver does not use this function

[2024-08-13 11:22] Matti: go look at what the MS driver does instead

[2024-08-13 11:27] Windows2000Warrior: [replying to Matti: "**and** that the MS driver does not use this funct..."]
Well, excuse me, you are right, thank you for answering my questions 
Now I got a general idea of ​​what to do.

[2024-08-13 12:53] elias: In what scenario would a ARM64 compiler generate a BR instruction?

[2024-08-13 16:52] mrexodia: Probably if it wants to branch?

[2024-08-13 16:52] mrexodia: <:kappa:697728545631371294>

[2024-08-13 17:03] elias: [replying to mrexodia: "Probably if it wants to branch?"]
yes but normally for branches within a function I only see it generating immediate branches and for function calls it would use BLR or BL

[2024-08-13 17:07] mrexodia: [replying to elias: "yes but normally for branches within a function I ..."]
I think if the function is big enough it will need to load into a register

[2024-08-13 17:08] mrexodia: And tail calls too?

[2024-08-13 17:22] Bloombit: [replying to elias: "In what scenario would a ARM64 compiler generate a..."]
look at the plt section gcc creates on a binary with some imported symbols

[2024-08-13 17:35] contificate: just any time you invoke a function pointer

[2024-08-13 17:36] contificate: [replying to mrexodia: "And tail calls too?"]
why would it need a register in this case

[2024-08-13 17:36] contificate: the simplest case is just a function pointer

[2024-08-13 17:36] contificate: y'all overcomplicating reality

[2024-08-13 17:43] Bloombit: I'm seeing blr while invoking a function pointer under gcc 14.2.0-1 under debian

[2024-08-13 17:48] contificate: oh right yeah I see

[2024-08-13 17:48] contificate: I see duncan's point now

[2024-08-13 17:48] contificate: got these instructions mixed up

[2024-08-13 17:48] contificate: make it a tail

[2024-08-13 17:49] contificate: 
[Attachments: 2024-08-13-184907_597x136_scrot.png]

[2024-08-13 17:49] contificate: apologies, I was confused and forgot the semantics

[2024-08-13 18:10] Eriktion: Do you guys think it is possible to allocate memory at a specifc virtual address in the kernel? Similar to VirtualAlloc but just in Km
I thought about manually mapping the memory via manipulation of the page tables, but I want it to be in every process. Thus I would have to do this for every process cr3? Lmk if you guys have ever done that or how that might work

[2024-08-13 18:12] Eriktion: Nvm, ignore what I said, they point to the same pdpts for most processes (exlcuding sys ones)

[2024-08-13 19:32] expy: how are you going to make it Windows compatible? I mean Windows can keep internal state variables which are used in page tables, once you insert something there, which you've just allocated on your own, and windows tries to release it, it's not going to end well I guess. 
Btw, if you just need a hook, there is a HookLib mentioned in re channel.

[2024-08-13 19:40] JustMagic: [replying to Eriktion: "Do you guys think it is possible to allocate memor..."]
You'll have to manually extend and manipulate bitmaps to get an allocation at an arbitrary address

[2024-08-13 19:41] JustMagic: If PTE is not in memory space bitmap, It'll eventually end up being invalidated

[2024-08-13 20:28] Eriktion: [replying to expy: "how are you going to make it Windows compatible? I..."]
Nah man I’m good, I don’t wanna hook anything rly, well I do want to hook stuff but it’s a sorta unique approach -> I need to allocate memory at a specific address

[2024-08-13 20:29] Eriktion: [replying to JustMagic: "You'll have to manually extend and manipulate bitm..."]
Ehm idk what you mean by bitmaps exactly, but I got things working by just manually inserting a pdpt entry where the paging hierarchy ended for the specific virtual address

[2024-08-13 20:31] Eriktion: From what I have gathered during my research windows doesn’t provide an api to allocate memory at a specific virtual address in the kernel

[2024-08-13 20:31] Eriktion: Ik my method might not be the most stable one due to kpp, but it gets the job done

[2024-08-16 13:37] eura4002: When attempting to hide the hypervisor region using NPT (Nested Page Tables), why does a system error occur? (Please refer to the comments in the HandleHide function.)
[Attachments: image.png, image.png]

[2024-08-16 13:40] eura4002: Even if the PFN of the PDE in the hypervisor is pointed to a different PFN, a system error occurs. (I tried changing the PFN to 0.)

[2024-08-16 13:43] Deleted User: how are you loading the hypervisor? if you do it as a driver via sc start then u will run into issues hiding your own memory

[2024-08-16 13:43] Timmy: current setup also doesn't match the guest pages 1on1 so you might be hiding way too much memory.

[2024-08-16 13:44] eura4002: <@456226577798135808> kdmapper used

[2024-08-16 13:47] Deleted User: okay i dont know how exactly npt works but if you set the valid bit to zero then that causes the crash right? are you perhaps hiding the memory from the driver? in this case you would need to do it from usermode via a hypercall for example

[2024-08-16 13:47] eura4002: <@83203731226628096>  Is the problem related to 2MB 1:1 mapping?

[2024-08-16 13:47] Deleted User: also what timmy said ya

[2024-08-16 13:48] Timmy: if it's not the problem right now, it'll 100% be a problem in the near future

[2024-08-16 13:48] Timmy: XD

[2024-08-16 13:48] Timmy: might be able to pad your driver binary to 2mib in order to try if that's the only issue

[2024-08-16 13:48] eura4002: In user mode, a vmcall is requested and handled by the hypervisor.

[2024-08-16 13:49] kian: why pml4 0? cause you're also mapping address 0x0. from my knowledge the first 64kb musn't be mapped for nullptr reasons

[2024-08-16 13:49] Deleted User: oki ya then is probably a good idea to implement splitting for npt hiding, so you split pdes into its seperate pt entries to not overhide memory

[2024-08-16 13:50] eura4002: Hmmm... This is complicated...

[2024-08-16 13:51] Timmy: imo should first actually create npt structure that matches the guest

[2024-08-16 13:51] Timmy: (host at the time of creation)

[2024-08-16 13:55] eura4002: Understood. Thank you all for your responses.

[2024-08-16 15:51] Hunter: why should u even mark it as nonpresent

[2024-08-16 15:51] Hunter: can always point it to a zeroed out page

[2024-08-16 17:17] elias: Did someone ever successfully compile an executable with warbird?

[2024-08-16 18:08] dullard: cc <@651054861533839370>

[2024-08-16 18:08] dullard: Actually wait

[2024-08-16 18:08] dullard: I think I saw a project which used warbird for payload protection

[2024-08-16 18:09] 5pider: what really

[2024-08-16 18:09] dullard: Ah no, https://github.com/KiFilterFiberContext/warbird-hook
[Embed: GitHub - KiFilterFiberContext/warbird-hook: Using Microsoft Warbird...]
Using Microsoft Warbird to automatically unpack and execute encrypted shellcode in ClipSp.sys without triggering PatchGuard - KiFilterFiberContext/warbird-hook

[2024-08-16 18:09] 5pider: or you mean it used warbird to execute their payload

[2024-08-16 18:09] dullard: this is what I saw

[2024-08-16 18:09] dullard: execute

[2024-08-16 18:09] dullard: correction, unpack / decrypt

[2024-08-16 18:09] 5pider: oh wait this is sick

[2024-08-16 18:10] 5pider: i have seen something else a while ago

[2024-08-16 18:10] 5pider: that used some feature from warbird from Userland to get their payload/shellcode to execute

[2024-08-16 18:10] 5pider: or i might be mistaken

[2024-08-16 18:46] elias: [replying to dullard: "Ah no, https://github.com/KiFilterFiberContext/war..."]
this is really cool but unfortunately not what I'm looking for

[2024-08-16 18:47] elias: Whenever I try to add anything to the warbird config I get this error
[Attachments: image.png]

[2024-08-16 21:05] JustMagic: [replying to 5pider: "that used some feature from warbird from Userland ..."]
It's pretty easy

[2024-08-16 21:06] JustMagic: All you have to do is encrypt the payload using leaked warbird source and make a syscall

[2024-08-16 21:09] 5pider: yeah i was talking about the syscall

[2024-08-16 21:09] 5pider: i remember a project making a syscall to execute the warbird payload

[2024-08-16 21:09] 5pider: imma search it again cuz was really interesting

[2024-08-16 22:51] dullard: There's this too https://github.com/KiFilterFiberContext/microsoft-warbird
[Embed: GitHub - KiFilterFiberContext/microsoft-warbird: Reimplementation o...]
Reimplementation of Microsoft's Warbird obuscator. Contribute to KiFilterFiberContext/microsoft-warbird development by creating an account on GitHub.

[2024-08-16 23:58] elias: is it possible to protect constants with ollvm?

[2024-08-17 11:50] kian: [replying to elias: "is it possible to protect constants with ollvm?"]
yeah, i think there's actually a modified ollvm with that transformation somewhere on github

[2024-08-17 14:59] 6bd835a1d0095059128d4d8cf6d16171: [replying to elias: "Whenever I try to add anything to the warbird conf..."]
pretty sure this warbird dll can only target arm64

[2024-08-17 14:59] 6bd835a1d0095059128d4d8cf6d16171: another warbird dll leaked before in a ewdk apparently but i don't have it myself

[2024-08-17 15:09] elias: [replying to 6bd835a1d0095059128d4d8cf6d16171: "pretty sure this warbird dll can only target arm64"]
ooh okay good to know