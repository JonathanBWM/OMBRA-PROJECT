# September 2024 - Week 2
# Channel: #reverse-engineering
# Messages: 675

[2024-09-03 02:55] Luckyman: I am finding reversing expert

[2024-09-03 06:40] th3: [replying to BWA RBX: "Must be in America"]
american

[2024-09-03 06:41] th3: [replying to rin: "I bet Canadian"]
american

[2024-09-05 02:10] abu: Hello. Has anyone ever encountered extremely long waiting times for dumpulator to load? It seems that I have some enormous parent memory region (which I can't see in windbg) that is taking very long to parse
[Attachments: image.png]

[2024-09-05 15:38] mrexodia: [replying to abu: "Hello. Has anyone ever encountered extremely long ..."]
Can you share the dump?

[2024-09-05 21:24] abu: [replying to mrexodia: "Can you share the dump?"]
Sure, I'll dm it to you

[2024-09-06 18:39] expy: any ideas why poorly written hypervisor may influence debug registers obtained by NtGetContextThread and at the same time not influencing the registers I'm seeing in `r drX` in the debugger?

[2024-09-06 18:49] Matti: kd doesn't use this API to retrieve register values

[2024-09-06 18:51] Matti: KD has its own 'kd internal' versions of many things you'd normally use a system call for

[2024-09-06 18:52] Matti: because they need to be able to run under a lot of constraints (1 cpu only, high irql, interrupts disabled(?) - not 100% sure about the last one)

[2024-09-06 18:53] Matti: check out `KdpSendWaitContinue` for the switch case containing functions to dispatch for servicing the debugger requests

[2024-09-06 18:56] Matti: its calling functions should show you how and in what state this CPU arrives there exactly, but you'll have to do this yourself cause it's been too long since I looked at this

[2024-09-06 18:57] Matti: I *think* KdpTrap is responsible for ensuring the other CPUs are spinning until the "chosen" CPU is done

[2024-09-06 18:59] Matti: oh and all this seems to vary massively between different NT versions as well... of course

[2024-09-06 19:00] Matti: I'm gonna guess `KdpGetContext` (win 11) is what kd actually uses rather than `NtGetContextThread`

[2024-09-06 19:05] expy: thanks Matti! I can't find yet where drX registers are read, NtGetContextThread leads to RtlCaptureContext which omits them

[2024-09-06 19:05] Matti: uhhh there may be others involved lol

[2024-09-06 19:05] Matti: KdpCopyContext
KdpGetContext
KdpGetContextEx
KdpInitializeExtendedContext
KdpSanitizeContextFlags
KdpSetContext
KdpSetContextEx
KdpSetContextState

[2024-09-06 19:05] Matti: for just the Kdp*Context matches

[2024-09-06 19:06] Matti: but really you probably need to check out the full flow from start to finish of what gets serviced via KdpSendWaitContinue when you issue `r drX`

[2024-09-06 19:09] irql: kdp saves them onto the KPRCB iirc

[2024-09-06 19:09] irql: ProcessorState

[2024-09-06 19:09] irql: which is KSPECIAL_REGISTERS

[2024-09-06 19:09] irql: which is then fetched by ermmmmm

[2024-09-06 19:09] irql: KdpReadControlSpace

[2024-09-06 19:09] Matti: yeah this sounds likely now that you say it

[2024-09-06 19:09] Matti: I knew it did something fucky with the KPRCB

[2024-09-06 19:09] irql: been a minute since i've touched kd shit too

[2024-09-06 19:10] irql: yea

[2024-09-06 19:10] irql: its so they can switch between processors

[2024-09-06 19:10] irql: some weird stuff

[2024-09-06 19:10] Matti: but it may also be the debug client that reads this value

[2024-09-06 19:10] Matti: well maybe it needs to issue the readcontrolspace to do that

[2024-09-06 19:10] Matti: not sure how it works exactly

[2024-09-06 19:10] irql: yea, the client could be messing with it too

[2024-09-06 19:11] irql: there is also a CONTEXT record, but I dont remember if DRs are actually saved on this

[2024-09-06 19:11] irql: I think the DRs on the context record are the actual process/thread that was interrupted

[2024-09-06 19:11] irql: versus control space which is the kernel drs used by KD to debug

[2024-09-06 19:11] Matti: [replying to irql: "there is also a CONTEXT record, but I dont remembe..."]
probably not if the Get/Set goes through SANITIZE first

[2024-09-06 19:11] irql: i'd assume the client/windbg/kd actually reads the CONTEXT ones for you, but uses the ControlSpace ones for KD itself

[2024-09-06 19:11] Matti: if it doesn't, then yes they should be

[2024-09-06 19:12] irql: yea, im not even sure

[2024-09-06 19:12] irql: its just the normal exception handling code

[2024-09-06 19:12] irql: which is used to save the context record

[2024-09-06 19:12] irql: might depend on ETHREAD flags and such

[2024-09-06 19:12] Matti: yeah that is true, it can and it does

[2024-09-06 19:13] irql: [replying to irql: "I think the DRs on the context record are the actu..."]
I guess you wouldn't see any kernel DRs

[2024-09-06 19:13] irql: taht could be your issue

[2024-09-06 19:13] Matti: idt this would affect *this* result but yeah it's hard to compare the return value of a system call with what you get back from KD

[2024-09-06 19:14] Matti: [replying to Matti: "probably not if the Get/Set goes through SANITIZE ..."]
note Kd also (of course) has its own Kdp SANITIZE

[2024-09-06 19:14] Matti: normally this is just a macro

[2024-09-06 19:14] irql: lemme just took a little peak

[2024-09-06 19:15] Matti: ```c
//
// Define sanitize debug register macros.
//
// Define control register settable bits and active mask.
//

#define DR7_LEGAL 0xffff0355
#define DR7_ACTIVE 0x0355
#define DR7_TRACE_BRANCH 0x200
#define DR7_LAST_BRANCH 0x100

//
// Define macro to sanitize the debug control register.
//

#define SANITIZE_DR7(Dr7, mode) ((Dr7) & DR7_LEGAL)

//
// Define macro to sanitize debug address registers.
//

#define SANITIZE_DRADDR(DrReg, mode)                                         \
    ((mode) == KernelMode ?                                                 \
        (DrReg) :                                                            \
        (((PVOID)(DrReg) <= MM_HIGHEST_USER_ADDRESS) ? (DrReg) : 0))                                 \
```
normal code path sanitize values for reference (NT 5.2)

[2024-09-06 19:16] Matti: there are other sanitize macros as well for things like mxcsr, fcw, and ofc VAs

[2024-09-06 19:16] Matti: oh and eflags, important

[2024-09-06 19:17] Matti: ```c
//
// Define sanitize EFLAGS macro.
//
// If kernel mode, then
//     caller can specify Carry, Parity, AuxCarry, Zero, Sign, Trap,
//     Interrupt, Direction, Overflow, and identification.
//
// If user mode, then
//     caller can specify Carry, Parity, AuxCarry, Zero, Sign, Trap,
//     Interrupt, Direction, Overflow, identification, but Interrupt
//     will always be forced on.
//

#define EFLAGS_SANITIZE 0x00210fd5L

#define SANITIZE_EFLAGS(eFlags, mode) (                                     \
    ((mode) == KernelMode ?                                                 \
        ((eFlags) & EFLAGS_SANITIZE) :                                     \
        (((eFlags) & EFLAGS_SANITIZE) | EFLAGS_IF_MASK)))
```

[2024-09-06 19:17] Matti: the Kdp version is about context flags OTOH though

[2024-09-06 19:18] Matti: and, I don't feel like reversing it

[2024-09-06 19:19] irql: yea, so it seems that typically KiBreakpointTrap is hit, calls KiSaveDebugRegisterState if the thread has DebugActive (otherwise they will be 0/invalid)

builds a CONTEXT from KTRAP_FRAME & KEXCEPTION_FRAME 

then it goes through to KdpTrap -> KdpReport 

KdpCopyContext to copy the CONTEXT which was built
KiSaveProcessorControlState which catches kernel drs (used by windbg, invisible to you)

[2024-09-06 19:19] irql: i think

[2024-09-06 19:19] irql: there may also be CONTEXT record trolling ye ^^

[2024-09-06 19:19] Matti: this sounds like how I remember it from when I looked at this API

[2024-09-06 19:19] irql: im not sure about that stuff matti said 🤣

[2024-09-06 19:19] irql: I hate that context stuff

[2024-09-06 19:19] expy: vast amount of knowledge. I guess hypervisor doesn't have to deal with anything except dr7 in vmcs. On vmexit there dr regs are just present in the context and unless you touch them you shouldn't even save them

[2024-09-06 19:20] Matti: the sanitize stuff may be completely irrelevant

[2024-09-06 19:20] Matti: idk

[2024-09-06 19:20] irql: they could be doing some funny business with it though, no doubt

[2024-09-06 19:20] irql: yea i dont even know lmfao

[2024-09-06 19:20] irql: [replying to expy: "vast amount of knowledge. I guess hypervisor doesn..."]
yea, I dont think it should be doing anything to them

[2024-09-06 19:20] irql: on a vmexit

[2024-09-06 19:20] irql: typically just dr7, like you say

[2024-09-06 19:21] irql: the DRs will only show up if theyre active on that thread though, so hmm

[2024-09-06 19:21] irql: NtGetContextThread will go to erm PspGetSetContextInternal

[2024-09-06 19:22] expy: yes, it issues APC if requested thread is not a current one

[2024-09-06 19:22] irql: yea

[2024-09-06 19:22] irql: I have it reversed in this idb actually if you want it

[2024-09-06 19:22] irql: mostly reversed / typed

[2024-09-06 19:26] Matti: ```c
typedef struct _DBGKD_MANIPULATE_STATE64 {
    ULONG ApiNumber;
    USHORT ProcessorLevel;
    USHORT Processor;
    NTSTATUS ReturnStatus;
    union {
        DBGKD_READ_MEMORY64 ReadMemory;
        DBGKD_WRITE_MEMORY64 WriteMemory;
        DBGKD_GET_CONTEXT GetContext;
        DBGKD_SET_CONTEXT SetContext;
        DBGKD_WRITE_BREAKPOINT64 WriteBreakPoint;
        DBGKD_RESTORE_BREAKPOINT RestoreBreakPoint;
        DBGKD_CONTINUE Continue;
        DBGKD_CONTINUE2 Continue2;
        DBGKD_READ_WRITE_IO64 ReadWriteIo;
        DBGKD_READ_WRITE_IO_EXTENDED64 ReadWriteIoExtended;
        DBGKD_QUERY_SPECIAL_CALLS QuerySpecialCalls;
        DBGKD_SET_SPECIAL_CALL64 SetSpecialCall;
        DBGKD_SET_INTERNAL_BREAKPOINT64 SetInternalBreakpoint;
        DBGKD_GET_INTERNAL_BREAKPOINT64 GetInternalBreakpoint;
        DBGKD_GET_VERSION64 GetVersion64;
        DBGKD_BREAKPOINTEX BreakPointEx;
        DBGKD_READ_WRITE_MSR ReadWriteMsr;
        DBGKD_SEARCH_MEMORY SearchMemory;
        DBGKD_GET_SET_BUS_DATA GetSetBusData;
        DBGKD_FILL_MEMORY FillMemory;
        DBGKD_QUERY_MEMORY QueryMemory;
        DBGKD_SWITCH_PARTITION SwitchPartition;
    } u;
} DBGKD_MANIPULATE_STATE64, *PDBGKD_MANIPULATE_STATE64;
```
this is the most important type here tbh, it drives the state machine that does the KdpSendWaitContinue requests

[2024-09-06 19:26] irql: yea that CONTEXT from windbg should be the same as what you get from NtGetSet

[2024-09-06 19:26] Matti: very dated type btw

[2024-09-06 19:26] irql: oh I got u

[2024-09-06 19:27] irql: https://github.com/irql/nokd/blob/master/kdpl/inc/dbgkd.h
[Embed: nokd/kdpl/inc/dbgkd.h at master · irql/nokd]
reverse engineering of the windows nt kernel debugger protocol & reimplementation. - irql/nokd

[2024-09-06 19:27] Matti: ya thanks

[2024-09-06 19:27] irql: thats all of them 🤣

[2024-09-06 19:27] irql: theyre good on win10 at least

[2024-09-06 19:27] irql: kd enjoyer

[2024-09-06 19:27] Matti: yeah I've got these, I just couldn't fucking remember where for the life of me

[2024-09-06 19:27] irql: I think i got them from the windbg kdexts header

[2024-09-06 19:27] irql: lmfao

[2024-09-06 19:28] irql: and some from ida

[2024-09-06 19:28] Matti: ah yeah, kdexts counts as reference source IMO

[2024-09-06 19:28] Matti: it's basically as good as a kernel PDB

[2024-09-06 19:28] irql: i think it had the KDDEBUGGER_DATA64 in there

[2024-09-06 19:28] irql: but nowhere else

[2024-09-06 19:29] irql: or something

[2024-09-06 19:29] irql: very nice header lmfao

[2024-09-06 19:29] Matti: I've got a few private win 10 PDBs that contain (some of?) these types as well btw

[2024-09-06 19:30] Matti: but, they're not gonna be as up to date as the most recent debugger extensions

[2024-09-06 19:30] irql: I think a bunch of the DBGKD ones are public symbols actually

[2024-09-06 19:30] irql: yea

[2024-09-06 19:30] irql: eh

[2024-09-06 19:30] irql: its probably not been updated in forever tbh

[2024-09-06 19:30] irql: I think most of these structures are like super untouched

[2024-09-06 19:30] Matti: ya you're right, at least this particular one is in public PDBs

[2024-09-06 19:30] Matti: or

[2024-09-06 19:31] Matti: well, *some* of them

[2024-09-06 19:31] Matti: but not reliably

[2024-09-06 19:31] irql: yea I think some might be missing

[2024-09-06 19:31] irql: I love kd lmfao

[2024-09-06 19:31] irql: they had support for a bunch of architectures

[2024-09-06 19:31] irql: ppc, ia64

[2024-09-06 19:33] Matti: yep

[2024-09-06 19:33] Matti: I remember discussing finding these types with you, or at least I think it was

[2024-09-06 19:33] irql: 
[Attachments: image.png]

[2024-09-06 19:33] irql: oh maybe yea

[2024-09-06 19:33] irql: lmfao

[2024-09-06 19:33] Matti: idt I ever found out they were related to KD though

[2024-09-06 19:33] irql: I reversed it all a few years ago so I could put it in my hv

[2024-09-06 19:34] irql: [replying to irql: ""]
oh this was in some windbg header ^^ iirc

[2024-09-06 19:34] Matti: yeah

[2024-09-06 19:34] Matti: there's this one clusterfuck of a header that contains a lot of this stuff

[2024-09-06 19:34] irql: yea

[2024-09-06 19:35] Matti: wdbgexts.h

[2024-09-06 19:35] irql: yea lmfao

[2024-09-06 19:35] Matti: [replying to irql: "I reversed it all a few years ago so I could put i..."]
curious - does your hv support kdnet?

[2024-09-06 19:36] Matti: if so, how many million $ for the source code

[2024-09-06 19:36] irql: oh nah, im not that much of a monster

[2024-09-06 19:36] irql: lmfao

[2024-09-06 19:36] irql: yea I wish

[2024-09-06 19:36] Matti: fak

[2024-09-06 19:36] irql: I thought about doing xhci drivers or something

[2024-09-06 19:36] irql: some of the realtek eth drivers looked okay

[2024-09-06 19:36] irql: but damn

[2024-09-06 19:36] Matti: yeah I'm procrastinating on it
still

[2024-09-06 19:36] irql: yea, I wanted to do it too

[2024-09-06 19:36] Matti: but it will be done

[2024-09-06 19:36] Matti: it has to be

[2024-09-06 19:36] irql: if you remember talking about serial port pci boards with me

[2024-09-06 19:36] irql: I ended up buying some random chip with a dataset sheet lol

[2024-09-06 19:37] irql: implemented a driver for it & used that

[2024-09-06 19:37] Matti: [replying to irql: "I thought about doing xhci drivers or something"]
well the issue is the KD API IMO

[2024-09-06 19:37] irql: it was just a UART at + 8 with some small extra steps

[2024-09-06 19:37] Matti: I was just discussing this with <@609487237331288074> earlier in another channel in fact

[2024-09-06 19:37] irql: hmmm? I only reversed the serial one

[2024-09-06 19:37] Matti: like, matti WRK supports USB4 and TB with PCIe tunneling now, so that's 40 gbit/s just for kd

[2024-09-06 19:37] irql: damn

[2024-09-06 19:37] Matti: but the TB driver does not support the KD API

[2024-09-06 19:37] Matti: obviously

[2024-09-06 19:38] Matti: and that part is more work than adding TB support in the first place

[2024-09-06 19:38] irql: lmfao

[2024-09-06 19:38] irql: yea, I only know about the serial ones

[2024-09-06 19:39] irql: which have 16 byte packet structures, designed for a baby 16 byte FIFO

[2024-09-06 19:39] irql: but yea, thats a headache ☠️

[2024-09-06 19:39] irql: a lot of it is reversed tbh in that project I linked, you can probably make windbg interpret it as a serial / com device

[2024-09-06 19:40] Matti: nah sorry I can't

[2024-09-06 19:40] irql: so you only need to implement the basic UART style server, but no idea if it'd work very well with 40gbit/s

[2024-09-06 19:40] Matti: it needs to be proper kdnet support exactly as in the MS kernel

[2024-09-06 19:40] irql: lmfao

[2024-09-06 19:40] Matti: I'm pedantic about this shit

[2024-09-06 19:40] Matti: even though this will be slower

[2024-09-06 19:40] daax: [replying to Matti: "I was just discussing this with <@6094872373312880..."]
we live in a simulation

[2024-09-06 19:40] irql: nice bit of torture

[2024-09-06 19:40] irql: tbf, serial port windbg is actually not that slow

[2024-09-06 19:41] irql: on physical hardware anyways

[2024-09-06 19:41] daax: [replying to irql: "tbf, serial port windbg is actually not that slow"]
uh

[2024-09-06 19:41] Matti: not matching MS kernel behaviour everywhere even for insane reasons has always come back to bite me in the ass

[2024-09-06 19:41] Matti: [replying to daax: "uh"]
idd

[2024-09-06 19:41] irql: or maybe this specific pcie uart board is speeding it up *

[2024-09-06 19:41] Matti: uh

[2024-09-06 19:41] irql: [replying to Matti: "not matching MS kernel behaviour everywhere even f..."]
lmfao

[2024-09-06 19:41] irql: ehhh

[2024-09-06 19:42] irql: yea, I probably wouldn't want to either

[2024-09-06 19:42] irql: although its just a few Kd* imports from ntos

[2024-09-06 19:43] Matti: [replying to irql: "or maybe this specific pcie uart board is speeding..."]
but how does this speed up the debuggee side? do you just tell it 'do 99999999 baud' and that somehow works? I kinda doubt that

[2024-09-06 19:43] Matti: 115200 is the hardcoded max in a disturbing number of places

[2024-09-06 19:43] daax: it doesn’t. it’s just placebo

[2024-09-06 19:43] daax: <:kekW:626450502279888906>

[2024-09-06 19:43] irql: its some CH382 chipset

[2024-09-06 19:43] daax: painfully it’s just a lie

[2024-09-06 19:43] irql: it had support for something much faster, but I always just used 115200

[2024-09-06 19:43] irql: it seemed pretty damn quick though

[2024-09-06 19:44] irql: break in was like 2 seconds or something

[2024-09-06 19:44] irql: at least from what I remember

[2024-09-06 19:44] daax: [replying to irql: "its some CH382 chipset"]
mm gotcha

[2024-09-06 19:44] Matti: well sure, I can break in 2 seconds

[2024-09-06 19:44] Matti: what about boot time though?

[2024-09-06 19:44] Matti: I can go do groceries in the time it takes my kernel to boot with kdcom enabled

[2024-09-06 19:44] irql: ah its been a while, maybe im misremembering

[2024-09-06 19:44] irql: but I dont think it was too bad

[2024-09-06 19:44] irql: ah

[2024-09-06 19:44] irql: 🤣

[2024-09-06 19:44] irql: damn

[2024-09-06 19:45] irql: tbf I might've had erm

[2024-09-06 19:45] Matti: stepping is also very slow

[2024-09-06 19:45] irql: I might've been using .reload to load all the kernel modules

[2024-09-06 19:45] irql: instead of breaking on every single module

[2024-09-06 19:45] irql: so that probably would explain it

[2024-09-06 19:45] Matti: well I already don't break on modules

[2024-09-06 19:45] Matti: and yeah I also pre-emptively .reload *, but that is just my OCD

[2024-09-06 19:46] Matti: that actually just makes it worse

[2024-09-06 19:46] irql: I mean like

[2024-09-06 19:46] irql: whenever a module is loaded in the normal kernel, it sends a KdpReport

[2024-09-06 19:46] irql: which just breaks in, tells the debugger, and immediately continues

[2024-09-06 19:46] Matti: mm ok, ic what you mean

[2024-09-06 19:46] irql: I had that turned off ^^

[2024-09-06 19:46] irql: yea

[2024-09-06 19:46] irql: that would probs make sense on why I thought it was fast

[2024-09-06 19:47] irql: KdpReportLoadSymbolsStateChange

[2024-09-06 19:47] irql: eb KdpReportLoadSymbolsStateChange c3 🔥 ?

[2024-09-06 19:47] Matti: you just patched the kernel to not call that?

[2024-09-06 19:47] Matti: and yeah it's the latter

[2024-09-06 19:47] irql: sorry I should explain better -- I have a kd reimplementation

[2024-09-06 19:47] irql: so i could throw it in my HV

[2024-09-06 19:47] Matti: oh yeah right

[2024-09-06 19:47] irql: I just turned off the calls to that function lol

[2024-09-06 19:47] irql: you could probs patch it

[2024-09-06 19:47] irql: to speed up boot times

[2024-09-06 19:47] Matti: in my case I could just remove the calls

[2024-09-06 19:47] irql: and a .reload will then fix the issues of unloaded modules

[2024-09-06 19:48] irql: yea

[2024-09-06 19:48] Matti: to achieve the same

[2024-09-06 19:48] irql: [replying to irql: "eb KdpReportLoadSymbolsStateChange c3 🔥 ?"]
this should actually work?

[2024-09-06 19:48] irql: maybe

[2024-09-06 19:48] daax: [replying to irql: "so i could throw it in my HV"]
to use serial right?

[2024-09-06 19:48] Matti: I'll give it a try in a bit

[2024-09-06 19:48] irql: yea I had a basic UART impl

[2024-09-06 19:49] irql: it worked via motherboard COM header, or a pcie serial board with ch382 chip lmfao

[2024-09-06 19:49] irql: or vmware RPC

[2024-09-06 19:49] daax: yeah nix that and go for better. you deserve better.

[2024-09-06 19:49] Matti: yeah I'll still want kdnet regardless lol <:lillullmoa:475778601141403648>

[2024-09-06 19:49] irql: lmfao

[2024-09-06 19:50] Matti: but I'll take this if it helps the boot tedium

[2024-09-06 19:50] Matti: ```c
VOID
DbgLoadImageSymbols (
    _In_ PSTRING FileName,
    _In_ PVOID ImageBase,
    _In_ ULONG_PTR ProcessId
    )

/*++

Routine Description:

    Tells the debugger about newly loaded symbols.

Arguments:

Return Value:

--*/

{
    PIMAGE_NT_HEADERS NtHeaders;
    KD_SYMBOLS_INFO SymbolInfo;

    SymbolInfo.BaseOfDll = ImageBase;
    SymbolInfo.ProcessId = ProcessId;
    NtHeaders = RtlImageNtHeader( ImageBase );
    if (NtHeaders != NULL) {
        SymbolInfo.CheckSum = (ULONG)NtHeaders->OptionalHeader.CheckSum;
        SymbolInfo.SizeOfImage = (ULONG)NtHeaders->OptionalHeader.SizeOfImage;

    } else {

        SymbolInfo.SizeOfImage = 0;
        SymbolInfo.CheckSum    = 0;
    }

    DebugService2(FileName, &SymbolInfo, BREAKPOINT_LOAD_SYMBOLS);
}
```
this is the WRK equivalent

[2024-09-06 19:50] Matti: DebugService2 is the expensive call

[2024-09-06 19:50] irql: DebugService2 will go to that KdpReportLoadSymbolsStateChange which will break-in

[2024-09-06 19:50] irql: and then windbg will internally read memory & stuff

[2024-09-06 19:50] irql: and then continue

[2024-09-06 19:51] Matti: makes sense

[2024-09-06 19:51] irql: but yea, you can literally patch that call

[2024-09-06 19:51] irql: and it should boot a lot faster

[2024-09-06 19:51] Matti: well let's see if my kernel still compiles today

[2024-09-06 19:52] irql: [replying to daax: "yeah nix that and go for better. you deserve bette..."]
nix that btw 🤨 ?

[2024-09-06 19:52] Matti: hmm - any opinion on what to do with the **un**load version of the same API?

[2024-09-06 19:52] daax: [replying to irql: "nix that btw 🤨 ?"]
huh?

[2024-09-06 19:52] irql: [replying to Matti: "hmm - any opinion on what to do with the **un**loa..."]
KdpReportExceptionStateChange is called for both

[2024-09-06 19:52] Matti: I won't be manually calling it, but implementing one but not the other might be an issue

[2024-09-06 19:52] irql: [replying to daax: "huh?"]
ah nvm

[2024-09-06 19:53] Matti: I'll just kill both

[2024-09-06 19:53] irql: oh sorry ***

[2024-09-06 19:53] irql: KdpReportLoadSymbolsStateChange* is called for both lol

[2024-09-06 19:53] irql: https://github.com/irql/nokd/blob/master/kdpl/kdpl.c#L379
[Embed: nokd/kdpl/kdpl.c at master · irql/nokd]
reverse engineering of the windows nt kernel debugger protocol & reimplementation. - irql/nokd

[2024-09-06 19:53] Matti: yeah I mean, that figures

[2024-09-06 19:53] irql: you might want to just return true

[2024-09-06 19:53] Matti: just wondering if you had an opinion on what to do with the unload version either way

[2024-09-06 19:54] irql: oh yea

[2024-09-06 19:54] irql: it'll do the same as the load

[2024-09-06 19:54] irql: but patching that one function should fix it

[2024-09-06 19:54] irql: mov eax, 1
ret

[2024-09-06 19:54] Matti: yes I get this part <:lillullmoa:475778601141403648> it's more a question of cleanliness I suppose

[2024-09-06 19:54] irql: yea 🤣

[2024-09-06 19:55] Matti: is there a caller for this API that is legit and whose bidding I should do

[2024-09-06 19:55] Matti: in order to not fuck up e.g. `'lm`

[2024-09-06 19:55] Matti: doubt it right

[2024-09-06 19:55] irql: you just have to `.reload` when windbg loads

[2024-09-06 19:55] irql: and it'll read all the modules

[2024-09-06 19:55] irql: but I think its ehhhh

[2024-09-06 19:55] irql: KdpTrap?

[2024-09-06 19:56] irql: reads the debug service parameters from the trap_frame

[2024-09-06 19:56] Matti: yea, I already do this anyway so that's np

[2024-09-06 19:56] irql: and then calls KdpReport..?

[2024-09-06 19:56] irql: ah yea, KdpTrap -> KdpSymbol -> KdpReportLoadSymbolsStateChange

[2024-09-06 19:57] irql: you could even block KdpSymbol tbf

[2024-09-06 19:57] irql: although KdpReportLoadSymbolsStateChange is the only thing that actually communicates with KD

[2024-09-06 19:57] irql: but yea, .reload will not use this -- it'll just read the module links & stuff

[2024-09-06 19:57] Matti: no this isn't what I was asking about, sorry

[2024-09-06 19:57] irql: oh, sorry?

[2024-09-06 19:57] Matti: nevermind it's fine

[2024-09-06 19:57] irql: 🤣 sorry bro

[2024-09-06 19:58] irql: misunderstood

[2024-09-06 19:58] Matti: I was trying to come up with some imaginary scenario in which I would **not** want to also nuke the unload version of the API

[2024-09-06 19:58] irql: ohhh lmfao

[2024-09-06 19:58] Matti: say, the kernel relocates drivers after p0 init, informs the debugger, and then the debugger issues (valid, wanted) unloads

[2024-09-06 19:58] Matti: but

[2024-09-06 19:59] Matti: I doubt this fucking works that way

[2024-09-06 19:59] Matti: and if it does I'll notice soon enough

[2024-09-06 20:00] irql: I think it all goes through here anyways

[2024-09-06 20:00] irql: you could allow unloads to be seen by the debugger if you just patched it with some conditional code

```c
BOOLEAN
KdpReportLoadSymbolsStateChange(
    _In_    PSTRING         PathName,
    _In_    PKD_SYMBOL_INFO Symbol,
    _In_    BOOLEAN         Unload,
    _Inout_ PCONTEXT        Context
    )
```

[2024-09-06 20:00] irql: if (Unload)

[2024-09-06 20:00] irql: probs not that useful tbh

[2024-09-06 20:00] Matti: mm no, I think my approach is fine like this

[2024-09-06 20:01] Matti: I'm not touching Kdp anything

[2024-09-06 20:01] Matti: only DbgLoad/DbgUnload whatevers

[2024-09-06 20:01] Matti: which are internal kernel functions

[2024-09-06 20:01] irql: yea that should do the trick tbh

[2024-09-06 20:01] Matti: then windbg can still issue load/unload via the Kdp API if needed

[2024-09-06 20:02] irql: 🤨

[2024-09-06 20:02] irql: well

[2024-09-06 20:02] irql: its more like the kernel telling windbg that a module was loaded/unloaded

[2024-09-06 20:02] irql: if you just .reload again, it'll refresh the full list

[2024-09-06 20:02] irql: unload / load whatever's necessary

[2024-09-06 20:02] Matti: I'm aware

[2024-09-06 20:02] Matti: but that'll go through the KD API right

[2024-09-06 20:02] Matti: not Dbg(Un)LoadImageSymbols

[2024-09-06 20:02] Matti: which is called by the kernel itself

[2024-09-06 20:02] irql: yea, .reload will just break in and do it itself

[2024-09-06 20:03] Matti: when loading modules

[2024-09-06 20:03] irql: yea, mb im sorta tripping trying to understand 🤣

[2024-09-06 20:03] irql: yea

[2024-09-06 20:23] Matti: sorry for the delay, I had a bit of a terminal case of 'insider preview installed' in a state that required urgent unfucking

[2024-09-06 20:24] Matti: so first observation is that I should probably let at least the first 2 loads through (nt, hal) so that the initial break will keep working
[Attachments: image.png]

[2024-09-06 20:24] Matti: ^ nt blatantly printing shit about this and that after I requested a fucking break in

[2024-09-06 20:24] Matti: second observation is: it's still slow

[2024-09-06 20:25] Matti: but it may be a tiny bit faster than before?

[2024-09-06 20:25] Matti: it's hard to say because there is another issue compounding this

[2024-09-06 20:25] Matti: when booting in UEFI mode (i.e. always), the nvidia driver does nothing whatsoever for about 20-60 seconds

[2024-09-06 20:26] Matti: and nothing is displayed on the screen

[2024-09-06 20:26] Matti: this is always the case, debugger or no

[2024-09-06 20:27] Matti: and presumably related to the driver being in no way compatible with UEFI and simply working through sheer luck

[2024-09-06 20:27] Matti: cause it's so old

[2024-09-06 20:27] Matti: so

[2024-09-06 20:27] Matti: I need a stopwatch I think

[2024-09-06 20:27] irql: ohhh yea, sorry -- i do load ntoskrnl manually

[2024-09-06 20:27] irql: [replying to Matti: "second observation is: it's still slow"]
oh huh? 🤨

[2024-09-06 20:27] irql: you sure you managed to patch all the dbg load/unloads?

[2024-09-06 20:27] irql: there's a few of them iirc

[2024-09-06 20:27] irql: [replying to Matti: "when booting in UEFI mode (i.e. always), the nvidi..."]
oh lmfao

[2024-09-06 20:28] Matti: well I patched the functions themselves

[2024-09-06 20:28] Matti: they're gone

[2024-09-06 20:28] Matti: or empty I should say

[2024-09-06 20:28] irql: oh that's pretty strange

[2024-09-06 20:28] irql: I was booting pretty quickly

[2024-09-06 20:28] irql: how long are your boot times typically with kd?

[2024-09-06 20:28] Matti: but ya, it's uncertain which of the two is the worse contributing factor to the boot times here without at least properly timing the two versions

[2024-09-06 20:29] Matti: [replying to irql: "how long are your boot times typically with kd?"]
too long

[2024-09-06 20:29] Matti: honestly no fucking clue

[2024-09-06 20:29] irql: [replying to Matti: "well I patched the functions themselves"]
there's also a user mode variant iirc -- and a few others

[2024-09-06 20:29] Matti: so, I need to time that

[2024-09-06 20:29] irql: [replying to Matti: "but ya, it's uncertain which of the two is the wor..."]
lmfao

[2024-09-06 20:29] irql: yea, idk, mine was definitely not long

[2024-09-06 20:29] irql: I could boot it up to see tbf

[2024-09-06 20:30] irql: [replying to irql: "there's also a user mode variant iirc -- and a few..."]
oh yea, were modules loaded?

[2024-09-06 20:30] irql: when it finally booted?

[2024-09-06 20:30] Matti: [replying to irql: "there's also a user mode variant iirc -- and a few..."]
true there is, but for the purposes of this experiment I'll count 'seeing anything on the screen' as being fully booted

[2024-09-06 20:30] Matti: so that means winlogon.exe is started and the nvidia driver finally started doing its fucking job

[2024-09-06 20:30] Matti: after that it's basically done right away anyway

[2024-09-06 20:30] Matti: so idt user mode symbol loads are an issue

[2024-09-06 20:31] Matti: [replying to irql: "oh yea, were modules loaded?"]
ya

[2024-09-06 20:31] irql: oh so the patch didnt seem to work?

[2024-09-06 20:31] irql: hm

[2024-09-06 20:31] Matti: 
[Attachments: image.png]

[2024-09-06 20:31] irql: [replying to Matti: "after that it's basically done right away anyway"]
lmfao

[2024-09-06 20:31] irql: oh yea, it mustn't be patched

[2024-09-06 20:31] irql: after I load in, I have only `nt` loaded

[2024-09-06 20:31] irql: (I manually load nt)

[2024-09-06 20:32] Matti: oh no but I did the same

[2024-09-06 20:32] Matti: just with * instead of nt

[2024-09-06 20:32] Matti: force of habit

[2024-09-06 20:32] Matti: the patch is working for sure

[2024-09-06 20:32] irql: oh nvm, i dont even load NT

[2024-09-06 20:32] irql: I just give it the kddebugger_data64

[2024-09-06 20:32] irql: oh right

[2024-09-06 20:32] irql: ah

[2024-09-06 20:32] irql: hm

[2024-09-06 20:33] irql: I mean thats pretty weird, I had pretty quick boot times without this

[2024-09-06 20:33] irql: or at least in my reimpl, which doesnt break on module loads

[2024-09-06 20:33] Matti: so either it's just not as effective as hoped, or the nvidia driver issue is waaaay worse than I thought

[2024-09-06 20:33] Matti: [replying to irql: "I mean thats pretty weird, I had pretty quick boot..."]
well I mean
consider stepping right

[2024-09-06 20:34] Matti: stepping doesn't involve the load API

[2024-09-06 20:34] Matti: it's still slow as fuck

[2024-09-06 20:34] irql: yea, but load API will be getting called like idk

[2024-09-06 20:34] irql: 100x during boot

[2024-09-06 20:34] Matti: I think having a serial debug connection enabled just make the world run at 5% of regular speed

[2024-09-06 20:34] Matti: sure that's true

[2024-09-06 20:34] irql: lmfoa

[2024-09-06 20:34] irql: that might be the case

[2024-09-06 20:34] irql: a trip back in time

[2024-09-06 20:35] Matti: but what about other APIs? what is the breakdown

[2024-09-06 20:35] irql: well, shit im sorry it didnt work 🤣

[2024-09-06 20:35] irql: there's only 3 iirc

[2024-09-06 20:35] Matti: and do they affect times evenly

[2024-09-06 20:35] irql: `KdpReportExceptionStateChange` -- an actual exception in kernel space (or user space if KdIgnoreUmExceptions = false)

[2024-09-06 20:35] irql: yea, thats the only other one ^^

[2024-09-06 20:35] irql: thats used for regular break in / exception

[2024-09-06 20:36] irql: yea, thats sorta confusing

[2024-09-06 20:36] irql: my boot times were pretty solid

[2024-09-06 20:36] irql: I assumed it was because of `KdpReportLoadSymbolsStateChange` being disabled

[2024-09-06 20:36] Matti: [replying to irql: "well, shit im sorry it didnt work 🤣"]
nah nps, besides I haven't excluded the nvidia driver being the cause of extreme slowness (rather than just plain regular slowness)

[2024-09-06 20:36] irql: maybe there's something else going on ? but i've never seen it

[2024-09-06 20:36] irql: [replying to Matti: "nah nps, besides I haven't excluded the nvidia dri..."]
hm, yea I guess it could've been that

[2024-09-06 20:37] Matti: I will time it np

[2024-09-06 20:37] Matti: just gonna, well

[2024-09-06 20:37] Matti: take a while

[2024-09-06 20:37] Matti: obviously

[2024-09-06 20:37] irql: 🤣 its actually that long?

[2024-09-06 20:37] Matti: yes

[2024-09-06 20:37] irql: my boot times were not even noticeable -- i never really thought about it

[2024-09-06 20:37] irql: until daax & you mentioned it

[2024-09-06 20:38] Matti: MRK boots in < 3 seconds without kernel debugger, and in BIOS mode (not CSM - real legacy BIOS mode)

[2024-09-06 20:38] Matti: the latter is due to the nvidia bug

[2024-09-06 20:38] Matti: not UEFI itself

[2024-09-06 20:39] Matti: adding a kernel debugger slows everything down on real hardware because that implies real serial

[2024-09-06 20:39] Matti: at that point it's slow in both BIOS and UEFI mode

[2024-09-06 20:39] irql: hmm

[2024-09-06 20:40] irql: even with a motherboard COM header it was pretty reasonable boot times

[2024-09-06 20:40] irql: maybe im going insane

[2024-09-06 20:40] Matti: not like I have a fucking PC without UEFI firmware lying around to test BIOS mode btw

[2024-09-06 20:40] irql: lmfao

[2024-09-06 20:40] Matti: but in vbox it does work like this

[2024-09-06 20:40] Matti: but in vbox it's not so easy to debug the nvidia driver for obvious reasons

[2024-09-06 20:41] irql: lmfao

[2024-09-06 20:41] irql: makes sense

[2024-09-06 20:41] Matti: [replying to irql: "even with a motherboard COM header it was pretty r..."]
it's slow for me even debugging other windows versions too FWIW

[2024-09-06 20:41] Matti: not sure if it's *as* slow

[2024-09-06 20:42] Matti: but slow enough that I'm gonna be doing something else

[2024-09-06 20:42] irql: icl i've never actually tried COM with regular KD, only my little meme

[2024-09-06 20:42] irql: oh yeahhhh

[2024-09-06 20:42] irql: mine was never that bad

[2024-09-06 20:42] Matti: kdnet only adds about 5-10 seconds to win 10/11 boot times

[2024-09-06 20:43] Matti: and the debugger itself is actually usable/functional

[2024-09-06 20:43] Matti: which is more important really

[2024-09-06 20:44] Matti: stepping over, using 115200 baud serial, is so slow that debugging anything becomes too painful to deal with

[2024-09-06 20:44] Matti: it makes kd only useful for catching bugchecks

[2024-09-06 20:45] Matti: which is something but... it's pretty fucking poor compared to kdnet

[2024-09-06 20:45] irql: yea, stepping was pretty rough for me too

[2024-09-06 20:45] irql: breakpoints were ok

[2024-09-06 20:45] irql: I mean I was reversing software, so just breaking and inspecting would suffice

[2024-09-06 20:45] irql: [replying to Matti: "which is something but... it's pretty fucking poor..."]
yea

[2024-09-06 20:45] Matti: yeah
if I want to make it to the end of a function, I need to set bps and use `g`

[2024-09-06 20:45] irql: I wanted to whip up realtek drivers, but its so much work

[2024-09-06 20:45] irql: [replying to Matti: "yeah
if I want to make it to the end of a function..."]
yea lmfao

[2024-09-06 20:46] Matti: [replying to irql: "I wanted to whip up realtek drivers, but its so mu..."]
mm realtek drivers for what?

[2024-09-06 20:46] irql: for KD net

[2024-09-06 20:46] irql: or at least iirc it was realtek

[2024-09-06 20:46] irql: they only support a handful of NICs

[2024-09-06 20:47] Matti: hmmm 🤔 I have 3 realtek NICs which all work with kdnet

[2024-09-06 20:47] irql: its basically the same driver for them all, yea

[2024-09-06 20:47] irql: its only like 5 - 6 drivers in kdnet iirc

[2024-09-06 20:47] irql: https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/supported-ethernet-nics-for-network-kernel-debugging-in-windows-10-1703
[Embed: Supported Ethernet NICs for Network Kernel Debugging in Windows 10 ...]
Learn about kernel debugging over an Ethernet network cable when the target computer is running Windows 10 , version 1703.

[2024-09-06 20:48] Matti: onboard ones though, are you talking about a discrete NIC?

[2024-09-06 20:48] irql: im pretty sure my onboard one was a realtek chipset

[2024-09-06 20:48] irql: got an intel one now anyways

[2024-09-06 20:48] irql: the realtek one had an open source driver too

[2024-09-06 20:48] irql: from linux

[2024-09-06 20:48] irql: but never really bothered to implement it

[2024-09-06 20:48] irql: looked not that fun

[2024-09-06 20:49] irql: [replying to irql: "https://learn.microsoft.com/en-us/windows-hardware..."]
10EC on there is Realtek

[2024-09-06 20:49] Matti: that list is for win 10 1703 though

[2024-09-06 20:49] Matti: <NIC>
    <manufacturer>10EC</manufacturer>
      <deviceid build="9200">8136</deviceid>
      <deviceid build="9200">8137</deviceid>
      <deviceid build="9200">8168</deviceid>
      <deviceid build="9200">8167</deviceid>
      <deviceid build="9200">8169</deviceid>
      <deviceid build="15063">8166</deviceid>
      <deviceid build="18362">8161</deviceid>
      <deviceid build="18362">8125</deviceid>
      <deviceid build="18362">8225</deviceid>
      <deviceid build="18362">2502</deviceid>
      <deviceid build="18362">2600</deviceid>
      <deviceid build="18362">3000</deviceid>
  </NIC>

[2024-09-06 20:49] Matti: is for 22621

[2024-09-06 20:49] irql: hmmm

[2024-09-06 20:49] irql: im sure there's like a generalised realtek driver

[2024-09-06 20:49] irql: which can actually work for all the device ids there

[2024-09-06 20:50] Matti: 8168, 8125, 8167
I think are mine

[2024-09-06 20:50] irql: the linux one I was looking at supported a lot of device ids, without much changes

[2024-09-06 20:50] irql: hm, yea im on some intel one now tbf

[2024-09-06 20:50] Matti: yeah they're probably all gonna be like that

[2024-09-06 20:51] irql: lot of work though, for little reward lmfao

[2024-09-06 20:51] Matti: same, I always put an intel dual port PCIe x1 NIC in debug victim machines

[2024-09-06 20:51] irql: maybe one day

[2024-09-06 20:51] irql: [replying to Matti: "same, I always put an intel dual port PCIe x1 NIC ..."]
lmfao

[2024-09-06 20:52] Matti: btw, are you aware of/familiar with kdnet USB EEM at all?

[2024-09-06 20:52] Matti: I've had to use it once on a board that had no serial, and an ethernet controller that used USB(!!!!) internally

[2024-09-06 20:52] Matti: not PCI

[2024-09-06 20:53] irql: lmfao i've heard about it

[2024-09-06 20:53] irql: but don't know much about it

[2024-09-06 20:53] irql: i think it might've been you that told me a bunch about it lmfao

[2024-09-06 20:53] Matti: it actually worked amazingly well

[2024-09-06 20:53] Matti: but - not so easy to set up

[2024-09-06 20:53] irql: yea, i'm sure it was you that told me its better than the USB one lmfao

[2024-09-06 20:53] Matti: big surprise

[2024-09-06 20:53] irql: ah hm

[2024-09-06 20:54] irql: hm, yea I wonder how hard those drivers are to impl

[2024-09-06 20:54] irql: probably not that fun tbf

[2024-09-06 20:55] Matti: I found a batch file I used for the debuggee side setup

[2024-09-06 20:55] Matti: trigger warning
```cmd
:: EEM = Ethernet Emulation Mode
:: Extended refs:
:: (x86) https://tandasat.github.io/blog/windows/2023/03/21/setting-up-kdnet-over-usb-eem-for-bootloader-and-hyper-v-debugging.html
:: (ARM) https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/setting-up-kernel-mode-debugging-over-usb-eem-arm-kdnet

:: TLDR:
:: 1. run this, after configuring busparams such that it points to a USB controller listed as supported for network debugging by kdnet.exe
::        note 1: the port number can optionally be changed. MSDN gives a recommended range of 50000-50039.
::        note 2: but do *not* change the host IP during this step! 169.254.255.255 is used for all USB EEM connections.
:: 2. on the host, disable firewall (TODO: which FW exceptions are needed?)
::        update: successfully debugged using my X570 Windows 10 install as host, even though I forgot to disable the firewall. so ignore this, it's bullshit
:: 3. on the host, start WinDbg as usual for KDNET: windbg -d -k net:port=52000,key=1.3.3.7
:: 4. reboot target.

bcdedit /dbgsettings net key:1.3.3.7 hostip:169.254.255.255 port:52000 busparams:1.0.5
bcdedit /set {dbgsettings} dhcp no

:: (change {default} below to a desired "debug" boot option GUID if needed)
::
:: from Satoshi's blog. USB EEM seems to work for me either with or without this:
bcdedit /set {default} loadoptions EEM
bcdedit /set {default} debug on

:: expected output of "bcdedit /dbgsettings":
::busparams               <b.d.f>
::key                     1.3.3.7
::debugtype               NET
::hostip                  169.254.255.255
::port                    52000
::dhcp                    No
echo.
bcdedit /dbgsettings
echo.
pause
```

[2024-09-06 20:57] Matti: by far the weirdest thing about this was that satoshi was using a specialised USB3 debug cable for this

[2024-09-06 20:57] Matti: you can see it in the photo on his blog

[2024-09-06 20:58] Matti: I have this same cable (it's not cheap) and I was already expecting to have to use it before I even saw the blog post

[2024-09-06 20:58] Matti: but

[2024-09-06 20:58] Matti: nothing worked in any way whatsoever until I replaced that cable with a shitty USB-A to C one from the supermarket

[2024-09-06 20:59] Matti: neither of us knows why this is

[2024-09-06 20:59] irql: [replying to Matti: "I have this same cable (it's not cheap) and I was ..."]
yea it is not cheap lmfao

[2024-09-06 20:59] irql: hm yea, thats pretty weird

[2024-09-06 21:00] irql: tbf, the ARM documentation mentions a lot about type A vs C

[2024-09-06 21:00] irql: seems weird though

[2024-09-06 21:00] Matti: yeah, that's actually where I got the (vaguely worded) clue/hint about the cable to use

[2024-09-06 21:01] Matti: and that it should NOT be a debug one

[2024-09-06 21:01] Matti: but regardless as you can tell from the blog, for satoshi it clearly did work

[2024-09-06 21:01] Matti: not sure if he ever did a retry with a regular A to C cable

[2024-09-06 21:02] Matti: I'm going to go ahead and guess that would've also worked, but idk

[2024-09-06 21:02] Matti: different machine altogether

[2024-09-06 21:02] Matti: and equally weird

[2024-09-06 21:03] Matti: for me the total number of times that USB3 debug cable has been useful is still 0

[2024-09-06 21:04] Matti: which impressively makes it more useless than even intel DCI cables

[2024-09-06 21:04] irql: lmfao

[2024-09-06 21:04] irql: they're super expensive too

[2024-09-06 21:04] irql: the actual debug capability of XHCI seemed really cool though

[2024-09-06 21:05] irql: I mean, i've kinda forgot how it worked now

[2024-09-06 21:05] Matti: I never believed in it <:hmm:475800667177484308>

[2024-09-06 21:05] irql: but im sure it had some cool weird properties to stop the guest using it whilst it was "debugging"

[2024-09-06 21:05] irql: yea, i remember you hating xhci lmfao

[2024-09-06 21:05] Matti: USB never works, so why would USB debugging work

[2024-09-06 21:05] Matti: and I was proven right

[2024-09-06 21:05] Matti: and it felt good

[2024-09-06 21:05] irql: lmfao

[2024-09-06 21:05] irql: ethernet 🦾

[2024-09-06 21:08] Matti: yeah USB4 has actually fixed this issue

[2024-09-06 21:08] Matti: you can now use USB to connect an intel NIC

[2024-09-06 21:08] Matti: and then use kdnet

[2024-09-06 21:08] Matti: thanks to the magic of thunderbolt PCIe tunneling

[2024-09-06 21:10] Matti: it'd basically just be this but with an E1000 NIC instead of the drive
[Attachments: image.png]

[2024-09-06 21:11] Matti: thank you based usb4

[2024-09-06 21:11] irql: showing off the optane 🙏

[2024-09-06 21:11] irql: yea that is pretty nice icl

[2024-09-06 21:12] Matti: <:wow:762710812904914945>

[2024-09-06 21:12] Matti: that's not an optane sir

[2024-09-06 21:12] irql: fortunately never had the joys of trying to debug via this stuff icl

[2024-09-06 21:12] irql: oh

[2024-09-06 21:12] irql: oh well

[2024-09-06 21:12] Matti: this is an intel(R) 750(TM)

[2024-09-06 21:12] Matti: know your classics!

[2024-09-06 21:12] Matti: this is like the ferrari F40 of nvme drives

[2024-09-06 21:12] irql: should've known lmfao

[2024-09-06 21:12] Matti: it's ancient but classy

[2024-09-06 21:12] Matti: and will forever be a classic

[2024-09-06 21:13] irql: rough ride for intel now

[2024-09-06 21:13] Matti: though, it seems I've got some scratches on mine

[2024-09-06 21:13] irql: ever since they dropped optane...

[2024-09-06 21:13] irql: [replying to Matti: "though, it seems I've got some scratches on mine"]
lmfao

[2024-09-06 21:13] irql: very unfortunate

[2024-09-06 21:13] Matti: yeah oh well it's a 9 year old drive now so

[2024-09-06 21:13] Matti: to be expected ig

[2024-09-06 21:14] Matti: [replying to irql: "ever since they dropped optane..."]
well I'm personally very fond of optane as you may know

[2024-09-06 21:14] Matti: but all of their SSDs were excellent

[2024-09-06 21:14] Matti: and they killed off their entire storage division

[2024-09-06 21:14] Matti: their core business

[2024-09-06 21:15] Matti: (some people think this is CPUs lol)

[2024-09-06 21:15] Matti: intel was the best SSD manufacturer by far

[2024-09-06 21:15] irql: storage ?

[2024-09-06 21:15] irql: damn

[2024-09-06 21:15] irql: yea, I thought it was their CPUs too

[2024-09-06 21:15] Matti: they aren't even an acceptable CPU manufacturer

[2024-09-06 21:15] irql: they're going through a real rough time rn

[2024-09-06 21:15] irql: yea

[2024-09-06 21:16] irql: I liked optane too

[2024-09-06 21:16] Matti: well I mean, revenue wise it is of course their CPU division

[2024-09-06 21:16] Matti: it's a little bit of a le joke

[2024-09-06 21:16] irql: aha

[2024-09-06 21:16] irql: i see

[2024-09-06 21:16] Matti: but it's fucked up that it's like that

[2024-09-06 21:24] Matti: oh fuck fuck fuck I totally forgot that there's a third potential cause of this unsatisfactory boot time on this particular machine

[2024-09-06 21:25] Matti: (etc)
[Attachments: image.png]

[2024-09-06 21:25] Matti: acpi.sys

[2024-09-06 21:25] Matti: idk how many cores this CPU really has, but it's less than this

[2024-09-06 21:25] Matti: this being 64

[2024-09-06 21:26] Matti: these are all using the same AMD CPU driver
[Attachments: image.png]

[2024-09-06 21:27] Matti: because that one works with the CPU, unlike the original intel one from XP/2003

[2024-09-06 21:27] Matti: but I doubt it's working **well**

[2024-09-06 21:27] Matti: and/or as intended

[2024-09-06 21:28] Matti: acpi.sys is 100% to blame for this, it's fucking up parsing whatever table contains the CPUs and their ACPI names/addresses and so on

[2024-09-06 21:28] Matti: 64 is simply the maximum possible for this motherboard

[2024-09-06 21:29] Matti: even though no such cpu exists for the socket, but that's not really important

[2024-09-06 21:30] Matti: the acpi.sys I'm using is a terminal disease

[2024-09-06 21:30] Matti: from a vista beta, that never had PDBs

[2024-09-06 21:31] Matti: the number in the name isn't really a version in the normal sense
it's literally a number I increment when I need to add a new patch to work around some BSOD it causes
[Attachments: image.png]

[2024-09-06 21:35] Matti: problem is:

1. using an older acpi.sys: same issues, maybe more - the only gain would be having the PDB for it
since I'm at v7 now my IDA DB is actually a decent substitute at this point

2. using a newer acpi.sys: these depend on MSI(-X) support, which depends on same being added to the kernel, HAL, and pci.sys all at the same time too

[2024-09-06 21:36] Matti: (2) is the bane of my existence

[2024-09-06 21:36] Matti: idk if it's a solvable problem

[2024-09-06 21:37] Matti: also it's highly embarrassing that I support NVMe but not MSI-X

[2024-09-06 21:37] Matti: so this crushes my ego, every day

[2024-09-06 21:37] Matti: and every night too

[2024-09-06 21:39] irql: lmfao

[2024-09-06 21:39] irql: yea I remember you telling me about the acpi.sys stuff

[2024-09-06 21:40] irql: yea idk what you're gonna do about that lmfao

[2024-09-06 21:41] irql: isnt it some vista driver?

[2024-09-06 21:41] Matti: in the end I guess I'll just put an end to things

[2024-09-06 21:41] Matti: forever

[2024-09-06 21:41] Matti: delete the repo

[2024-09-06 21:42] Matti: it's like keeping someone on life support 20 years past their expiration date just cause you like having them around

[2024-09-06 21:43] Matti: that's not humane and similarly this is not fair to properly written code either that needs to live in the same unsanitary repo

[2024-09-06 21:44] Matti: [replying to irql: "isnt it some vista driver?"]
yes

[2024-09-06 21:44] irql: nah it's a cool project icl

[2024-09-06 21:44] Matti: specifically- vista beta

[2024-09-06 21:44] irql: but I guess you can't open source it or anything

[2024-09-06 21:44] Matti: leaked... vista beta...

[2024-09-06 21:44] irql: lmfao

[2024-09-06 21:44] Matti: nope

[2024-09-06 21:45] Matti: [replying to irql: "nah it's a cool project icl"]
it's cool but is it really cool if you're an optane being used by some barbarian who purposely disabled MSI support in order to make it work?

[2024-09-06 21:46] Matti: if you've ever read crime and punishment, you know that crimes weigh on the conscience

[2024-09-06 21:47] Matti: and the weight gets heavier, not lighter

[2024-09-06 21:47] Matti: until in the end you must come clean with yourself and your actions

[2024-09-06 21:47] Matti: and repent

[2024-09-06 21:48] Matti: if I delete the repository, this acpi.sys can never be used for evil again

[2024-09-06 21:49] Matti: that's such a simple fix that I really have no excuse

[2024-09-06 21:49] irql: lmfao

[2024-09-06 21:49] irql: definitely a crime to abuse an optane like that

[2024-09-06 21:49] irql: no doubt

[2024-09-06 21:50] irql: yea, idk how you'd fix that tbh

[2024-09-06 21:50] irql: Intel really did shine with optanes though

[2024-09-06 21:50] irql: i just literally found out my CPU is smoked by that oxidation bug <a:d3dnig_none:992510303566839978>

[2024-09-06 21:51] Matti: oh well

[2024-09-06 21:51] Matti: see it as a blessing in disguise

[2024-09-06 21:51] Matti: now you can get an AMD instead

[2024-09-06 21:51] irql: lmfao

[2024-09-06 21:51] irql: I should've

[2024-09-06 21:52] Matti: [replying to irql: "yea, idk how you'd fix that tbh"]
I have ideas but they border on the insane (rewriting acpi.sys myself from scratch and using the publicly available acpica library to handle the actual ACPI related parts)

[2024-09-06 21:52] irql: yea thats what I was thinking

[2024-09-06 21:52] irql: acpica doesnt look too bad

[2024-09-06 21:52] irql: you have pretty much no other option than that

[2024-09-06 21:52] irql: it seems

[2024-09-06 21:53] Matti: but most of what acpi.sys does is not related to things acpica does, tbh

[2024-09-06 21:53] Matti: weird as it might sound

[2024-09-06 21:54] Matti: the issue is that these 3 drivers right (I'll call hal a driver here)
hal, acpi. pci
are essentially part of the kernel from a design perspective, even though they are loaded dynamically

[2024-09-06 21:54] Matti: why do I say this - because they include internal kernel headers

[2024-09-06 21:54] Matti: no other MS drivers do this or are allowed to do this

[2024-09-06 21:55] Matti: so that means changing KINTERRUPT is impossible - unless you do it in 4 images all at once

[2024-09-06 21:55] Matti: and correctly ofc

[2024-09-06 21:55] Matti: I mean even just adding an unused member field

[2024-09-06 21:57] Matti: and so most of what acpi REALLY does is: talking to its stupid fuckbuddies pci.sys and hal.dll, and exchanging secret messages about things no one else is allowed to know about

[2024-09-06 21:57] irql: lmfao

[2024-09-06 21:57] irql: <:gdb:992509370908811284>

[2024-09-06 21:58] irql: im not even too surprised icl, those are like core kernel components

[2024-09-06 21:58] irql: that's pretty awful though lmfao

[2024-09-06 21:59] irql: hmmmmm, yea maybe you can find another underground never-seen-before beta

[2024-09-06 21:59] irql: doesn't sound like fun to rewrite any of that stuff

[2024-09-06 21:59] Matti: [replying to irql: "hmmmmm, yea maybe you can find another underground..."]
I know this sounds fucking sad
and the reason is that it is fucking sad

[2024-09-06 22:00] Matti: but I have every vista beta that was ever leaked or released already

[2024-09-06 22:00] Matti: this is already the best of the bunch

[2024-09-06 22:01] irql: ahahahah lmfao

[2024-09-06 22:01] irql: damn, I should've known

[2024-09-06 22:01] Matti: just one of many torrents
[Attachments: image.png]

[2024-09-06 22:01] irql: lmfao

[2024-09-06 22:02] irql: hm, yea maybe not a lot of hope

[2024-09-06 22:02] irql: or well

[2024-09-06 22:02] irql: binary patching could go a long way

[2024-09-06 22:02] irql: .matti section in the acpi.sys driver

[2024-09-06 22:02] Matti: yeah for fixing bsods

[2024-09-06 22:03] Matti: it can and it has

[2024-09-06 22:03] Matti: but not for adding MSI support

[2024-09-06 22:03] irql: hm yea

[2024-09-06 22:03] Matti: that's like half of the driver in terms of code size prob

[2024-09-06 22:03] irql: yea probably tbf

[2024-09-06 22:03] irql: hmm

[2024-09-08 02:12] Matti: <@991360481493262411> bit of a late update before I go to bed, but you were right about suppressing the module load notifications to kd making a big difference after all

[2024-09-08 02:13] Matti: it was just being masked by an even worse perf issue

[2024-09-08 02:15] Matti: it was not the ~20s nvidia driver blackout I mentioned before, in fact it turns out I had already fixed that for the quadro I'm not using. (I must've done that in my sleep cause I couldn't remember having done this fix at all yet)

[2024-09-08 02:16] Matti: the culprit was this piece of shit asus USB4/TB card I've been usinng to add support for USB4 + PCIe tunneling to MRK

[2024-09-08 02:17] Matti: it was slowing down win 11 boots as well, from ~10s to 36s

[2024-09-08 02:18] Matti: but MRK it was slowing down a bit more than that, namely from 21s to 2:14s

[2024-09-08 02:20] Matti: after disabling the card it became quite a bit easier to test the KD change in isolation lol

[2024-09-08 02:21] Matti: so the 21s above was for MRK with no debugger
MRK with serial debugger takes 1:50s to boot using the same settings

[2024-09-08 02:21] Matti: after suppressing symbol loads, it is now 24s

[2024-09-08 02:22] Matti: so breddy gud after all right

[2024-09-08 02:22] Matti: now to figure out what the fuck to do with this piece of shit USB4 host router card

[2024-09-08 02:23] Matti: thank fuck you can at least disable it from the BIOS, if I ever have to physically remove or install that thing again I'll just jump in front of a train instead

[2024-09-08 02:27] Matti: I'm gonna go to bed now because I could easily rant about that card for another hour or so, but here's one of the funnier ways in which it is total dogshit
[Attachments: image.png]

[2024-09-08 02:28] Matti: so the manual tells you this card **has** to be installed in the bottom PCIe x4 slot on my motherboard as you can see

[2024-09-08 02:29] Matti: which I found such a ridiculous and insane restriction that I couldn't even come up with an explanation for it at first
like not even a bad or incorrect one

[2024-09-08 02:31] Matti: but actually it's really easy to make it suck this way, because the USB4 connectors on this card are handled by the motherboard BIOS, not the card itself

[2024-09-08 02:32] Matti: via what else than.... ACPI

[2024-09-08 02:32] Matti: using a table in which the root port number is hardcoded to 9

[2024-09-08 02:32] Matti: which matches my bottom PCIe slot

[2024-09-08 02:33] Matti: <:acpica:1066830025128681512>

[2024-09-08 05:46] brymko: gigachad design

[2024-09-08 05:48] Torph: [replying to Matti: "using a table in which the root port number is har..."]
?????
completely deranged

[2024-09-08 09:01] irql: ahahah what 🤣

[2024-09-08 09:02] irql: yea that's pretty insane lmfao

[2024-09-08 09:02] irql: premium card

[2024-09-08 09:02] irql: [replying to Matti: "after suppressing symbol loads, it is now 24s"]
aha, I had a feeling something else might've been causing it

[2024-09-08 09:03] irql: [replying to Matti: "thank fuck you can at least disable it from the BI..."]
lmfao

[2024-09-08 09:04] irql: damn, 2 minutes boot time is pretty awful

[2024-09-08 09:04] irql: I had no idea it was that bad before lmfao, i've always had symbol loads suppressed

[2024-09-08 09:06] twopic: Has anyone here done reverse engineering within dram?

[2024-09-08 11:35] Azrael: As in what?