# September 2024 - Week 3
# Channel: #reverse-engineering
# Messages: 252

[2024-09-09 20:17] expy: any ideas where would this function copy DR registers from?
```
00 fffff307`3bdcf238 fffff801`3995edf9     nt!RtlpCopyLegacyContextAmd64+0x1c1
01 fffff307`3bdcf240 fffff801`39c9caa6     nt!RtlpCopyExtendedContext+0xd9
02 fffff307`3bdcf290 fffff801`39d22473     nt!RtlpWriteExtendedContext+0x9a
03 fffff307`3bdcf2e0 fffff801`39cff1aa     nt!PspGetContextThreadInternal+0x287
04 fffff307`3bdcfa10 fffff801`5afdbeb6     nt!NtGetContextThread+0x7a
```
It's not current actual DR register value (CONTEXT is acquired for a current thread) and it's not a trap frame

[2024-09-09 20:24] expy: it's very close to _KTHREAD->KernelStack so it must have be written there by syscall handler

[2024-09-09 20:34] Matti: ```c
typedef struct _AMD64_XMM_SAVE_AREA32
{
    /* 0x0000 */ unsigned short ControlWord;
    /* 0x0002 */ unsigned short StatusWord;
    /* 0x0004 */ unsigned char TagWord;
    /* 0x0005 */ unsigned char Reserved1;
    /* 0x0006 */ unsigned short ErrorOpcode;
    /* 0x0008 */ unsigned long ErrorOffset;
    /* 0x000c */ unsigned short ErrorSelector;
    /* 0x000e */ unsigned short Reserved2;
    /* 0x0010 */ unsigned long DataOffset;
    /* 0x0014 */ unsigned short DataSelector;
    /* 0x0016 */ unsigned short Reserved3;
    /* 0x0018 */ unsigned long MxCsr;
    /* 0x001c */ unsigned long MxCsr_Mask;
    /* 0x0020 */ struct _AMD64_M128 FloatRegisters[8];
    /* 0x00a0 */ struct _AMD64_M128 XmmRegisters[16];
    /* 0x01a0 */ unsigned char Reserved4[96];
} AMD64_XMM_SAVE_AREA32, *PAMD64_XMM_SAVE_AREA32; /* size: 0x0200 */

typedef struct _AMD64_CONTEXT
{
    /* 0x0000 */ unsigned __int64 P1Home;
    /* 0x0008 */ unsigned __int64 P2Home;
    /* 0x0010 */ unsigned __int64 P3Home;
    /* 0x0018 */ unsigned __int64 P4Home;
    /* 0x0020 */ unsigned __int64 P5Home;
    /* 0x0028 */ unsigned __int64 P6Home;
    /* 0x0030 */ unsigned long ContextFlags;
    /* 0x0034 */ unsigned long MxCsr;
    /* 0x0038 */ unsigned short SegCs;
    /* 0x003a */ unsigned short SegDs;
    /* 0x003c */ unsigned short SegEs;
    /* 0x003e */ unsigned short SegFs;
    /* 0x0040 */ unsigned short SegGs;
    /* 0x0042 */ unsigned short SegSs;
    /* 0x0044 */ unsigned long EFlags;
    /* 0x0048 */ unsigned __int64 Dr0;
    /* 0x0050 */ unsigned __int64 Dr1;
    /* 0x0058 */ unsigned __int64 Dr2;
    /* 0x0060 */ unsigned __int64 Dr3;
    /* 0x0068 */ unsigned __int64 Dr6;
    /* 0x0070 */ unsigned __int64 Dr7;
    /* 0x0078 */ unsigned __int64 Rax;
    /* 0x0080 */ unsigned __int64 Rcx;
    /* 0x0088 */ unsigned __int64 Rdx;
    /* 0x0090 */ unsigned __int64 Rbx;
    /* 0x0098 */ unsigned __int64 Rsp;
    /* 0x00a0 */ unsigned __int64 Rbp;
    /* 0x00a8 */ unsigned __int64 Rsi;
    /* 0x00b0 */ unsigned __int64 Rdi;
    /* 0x00b8 */ unsigned __int64 R8;
    /* 0x00c0 */ unsigned __int64 R9;
    /* 0x00c8 */ unsigned __int64 R10;
    /* 0x00d0 */ unsigned __int64 R11;
    /* 0x00d8 */ unsigned __int64 R12;
    /* 0x00e0 */ unsigned __int64 R13;
    /* 0x00e8 */ unsigned __int64 R14;
    /* 0x00f0 */ unsigned __int64 R15;
    /* 0x00f8 */ unsigned __int64 Rip;
    union
    {
        /* 0x0100 */ struct _AMD64_XMM_SAVE_AREA32 FltSave;
        struct
        {
            /* 0x0100 */ struct _AMD64_M128 Header[2];
            /* 0x0120 */ struct _AMD64_M128 Legacy[8];
            /* 0x01a0 */ struct _AMD64_M128 Xmm0;
            /* 0x01b0 */ struct _AMD64_M128 Xmm1;
            /* 0x01c0 */ struct _AMD64_M128 Xmm2;
            /* 0x01d0 */ struct _AMD64_M128 Xmm3;
            /* 0x01e0 */ struct _AMD64_M128 Xmm4;
            /* 0x01f0 */ struct _AMD64_M128 Xmm5;
            /* 0x0200 */ struct _AMD64_M128 Xmm6;
            /* 0x0210 */ struct _AMD64_M128 Xmm7;
            /* 0x0220 */ struct _AMD64_M128 Xmm8;
            /* 0x0230 */ struct _AMD64_M128 Xmm9;
            /* 0x0240 */ struct _AMD64_M128 Xmm10;
            /* 0x0250 */ struct _AMD64_M128 Xmm11;
            /* 0x0260 */ struct _AMD64_M128 Xmm12;
            /* 0x0270 */ struct _AMD64_M128 Xmm13;
            /* 0x0280 */ struct _AMD64_M128 Xmm14;
            /* 0x0290 */ struct _AMD64_M128 Xmm15;
        }; /* size: 0x01a0 */
    }; /* size: 0x0200 */
    /* 0x0300 */ struct _AMD64_M128 VectorRegister[26];
    /* 0x04a0 */ unsigned __int64 VectorControl;
    /* 0x04a8 */ unsigned __int64 DebugControl;
    /* 0x04b0 */ unsigned __int64 LastBranchToRip;
    /* 0x04b8 */ unsigned __int64 LastBranchFromRip;
    /* 0x04c0 */ unsigned __int64 LastExceptionToRip;
    /* 0x04c8 */ unsigned __int64 LastExceptionFromRip;
} AMD64_CONTEXT, *PAMD64_CONTEXT; /* size: 0x04d0 */
```

[2024-09-09 20:34] Matti: pretty sure it's this

[2024-09-09 20:35] Matti: oh and
```c
typedef struct _AMD64_M128
{
    /* 0x0000 */ unsigned __int64 Low;
    /* 0x0008 */ __int64 High;
} AMD64_M128, *PAMD64_M128; /* size: 0x0010 */
```

[2024-09-09 20:39] expy: the question is where is that coming from? In doesn't match to usermode DRs according to the debugger (just before the syscall)

[2024-09-09 20:41] Matti: well you've got quite the stack there

[2024-09-09 20:42] Matti: so any number of things may be filtering or replacing these values after the call, and I have no idea where the inputs come from that go into the call

[2024-09-09 20:42] Matti: I merely answered the question of what type it is

[2024-09-09 20:42] Matti: I'm pretty sure it's this

[2024-09-09 20:43] Matti: as for your usermode debugger - doesn't it go through the same NtGetContextThread? i.e. why would one be more trustworthy than the other?

[2024-09-09 20:44] expy: I'm using kernel debugger

[2024-09-09 20:45] Matti: right, in which case same question because the DRs go through the Kd API
but at least it makes more sense that they don't match up that way, after all calling the same syscall should give the same result

[2024-09-09 20:46] Matti: actually on an X64 kernel (I'm assuming this is one) you can just substitute `PCONTEXT` and it'll refer to the same DR registers

[2024-09-09 20:47] Matti: the function only copies a subset of them (importantly excluding Dr7 for example), for reasons unknown

[2024-09-09 20:48] Matti: I'm only saying what it does

[2024-09-09 20:48] Matti: afraid I don't know why it does what it does

[2024-09-09 20:48] irql: its from the oldest KTRAP_FRAME / KEXCEPTION_FRAME on the stack

[2024-09-09 20:48] irql: iirc

[2024-09-09 20:49] irql: which is the last interrupted program?

[2024-09-09 20:50] Matti: I checked that just to be sure but that doesn't seem to match at all, on 22621 anyway

[2024-09-09 20:50] Matti: it's true PspGetContext (WRK name...) and whatever the win 10/11 equivalent is do do what you say

[2024-09-09 20:51] Matti: but they also copy all of the DR registers

[2024-09-09 20:51] Matti: this function seems oddly handicapped for reasons I can't explain

[2024-09-09 20:51] Matti: the source and destination offsets are the same, FWIW

[2024-09-09 20:52] Matti: so it's very likely the same type going in and out

[2024-09-09 20:52] Matti: it is called copy after all

[2024-09-09 20:52] Matti: best I can make of it
[Attachments: image.png]

[2024-09-09 20:53] irql: 
[Attachments: image.png]

[2024-09-09 20:54] irql: mhm

[2024-09-09 20:54] irql: maybe im on a different ntoskrnl build

[2024-09-09 20:54] irql: 
[Attachments: message.txt]

[2024-09-09 20:54] irql: i think most things are typed in there

[2024-09-09 20:54] irql: 
[Attachments: image.png]

[2024-09-09 20:55] irql: maybe im on the wrong build

[2024-09-09 20:55] irql: I have the rest of these functions reversed if they're any use

[2024-09-09 20:55] irql: looks to me, like your debugger caught a bp inside RtlCopyContext

[2024-09-09 20:55] Matti: [replying to irql: "maybe im on the wrong build"]
well this could be true for me as well

[2024-09-09 20:55] Matti: but

[2024-09-09 20:56] Matti: the OP's stack contains some functions I don't see in this pseudocode

[2024-09-09 20:56] Matti: I only looked at the topmost function in the stack

[2024-09-09 20:56] irql: yea

[2024-09-09 20:56] Matti: the code you posted is the one I'm familiar with too

[2024-09-09 20:56] irql: thats why I think it may be different

[2024-09-09 20:56] irql: yea

[2024-09-09 20:57] irql: this bin is 19041

[2024-09-09 20:58] irql: ohhh wait yea I do

[2024-09-09 20:58] irql: ohhhh

[2024-09-09 20:58] irql: you're calling PspGetContextThreadInternal

[2024-09-09 20:59] irql: which is what gets called when you get your own context

[2024-09-09 20:59] irql: 
[Attachments: message.txt]

[2024-09-09 20:59] Matti: ah there we go

[2024-09-09 20:59] irql: ehh

[2024-09-09 20:59] irql: not as well reversed

[2024-09-09 21:00] Matti: `RtlpWriteExtendedContext` at the end is the call

[2024-09-09 21:00] irql: i sorta left that one 🤣

[2024-09-09 21:00] irql: might be useful

[2024-09-09 21:00] irql: yea

[2024-09-09 21:00] irql: oh right yea

[2024-09-09 21:00] irql: sorry

[2024-09-09 21:00] irql: this gets called

[2024-09-09 21:00] irql: and then debug registers get copied inside of these

[2024-09-09 21:00] irql: 
[Attachments: image.png]

[2024-09-09 21:00] irql: depending on whether you're in the thread or not

[2024-09-09 21:00] irql: [replying to irql: ""]
so it would've been this ^^^

[2024-09-09 21:00] irql: from the top of the KTRAP_FRAME on the interrupted thread / current thread

[2024-09-09 21:01] Matti: ok, I think that's mystery solved then right?

[2024-09-09 21:01] Matti: that explains why the DR registers get overwritten?

[2024-09-09 21:01] irql: I guess so lmfao

[2024-09-09 21:01] irql: not exactly sure what's going on with his DRs tbh

[2024-09-09 21:01] irql: but they're always copied from KTRAP_FRAME

[2024-09-09 21:01] irql: from the thread itself

[2024-09-09 21:02] Matti: yea, this is also the process I'm familiar with

[2024-09-09 21:02] irql: which would've been saved if being debugged

[2024-09-09 21:02] irql: ie KiSaveDebugRegisterState

[2024-09-09 21:02] Matti: though in the WRK PspGetContext is still borderline readable

[2024-09-09 21:02] irql: lmfao yea its pretty horrible here

[2024-09-09 21:02] irql: they have like a custom struct with KAPC + some data

[2024-09-09 21:02] Matti: yeaahhh this exists in the WRK too

[2024-09-09 21:02] Matti: but not the 20 different types of contexts

[2024-09-09 21:03] irql: this thing
[Attachments: image.png]

[2024-09-09 21:03] irql: yea

[2024-09-09 21:03] Matti: especially XSAVE accounting

[2024-09-09 21:03] irql: yea that xsave stuff is horrible lmfao

[2024-09-09 21:03] irql: especially once you get some good optimizations on it

[2024-09-09 21:05] daax: [replying to irql: "yea that xsave stuff is horrible lmfao"]
xsave shit is the biggest pain  and it gets all fucked in ida

[2024-09-09 21:05] irql: yea

[2024-09-09 21:05] irql: soon as those optimizations hit

[2024-09-09 21:05] irql: <:gdb:992509370908811284>

[2024-09-09 21:06] Matti: C
O
N
T
E
X
T
_
E
X

[2024-09-09 21:06] Matti: the last thing hitler wrote before he died

[2024-09-09 21:06] irql: lmfao ahahahahah

[2024-09-09 21:06] daax: [replying to Matti: "C
O
N
T
E
X
T
_
E
X"]


[2024-09-09 21:23] BWA RBX: [replying to daax: ""]
in his beloved bunker

[2024-09-10 23:14] davi: Does anyone know where I can find those two `ntoskrnl.exe` variations, besides installing and uninstalling those updates?
The Download links are dead.
[Attachments: image.png]

[2024-09-10 23:15] davi: I'd like to check a claim that `KB5041587` improved performance for AMD CPUs by optimizing branch prediction code.

[2024-09-10 23:18] davi: well, by coincidence I had both versions backed up, but I'd still like to know

[2024-09-10 23:18] Brit: grab it from vt

[2024-09-10 23:18] Brit: with the hash

[2024-09-10 23:19] davi: ^ doesn't downloading from VT require some kind of specific Premium account that you can't get unless you're a company, tho

[2024-09-10 23:19] Brit: yes

[2024-09-10 23:19] davi: <:tf:888783665696477195>

[2024-09-10 23:21] snowua: Fallen enjoyer

[2024-09-10 23:38] Matti: you've got the before and after patch names, so you can just make two scripts on uupdump.net that will give you a full win 11 ISO for both

[2024-09-10 23:38] Matti: wasteful? yeah kinda but it works

[2024-09-10 23:38] Matti: and it's easy

[2024-09-10 23:46] davi: aha
[Attachments: image.png]

[2024-09-10 23:47] davi: I don't know what's happening there, but it's definitely skipping some logic when the 0x38th bit is set in `KiSpeculativeFeatures`

[2024-09-10 23:47] davi: and it somewhat matches the given picture of the KB changing branch prediction mitigations

[2024-09-11 00:01] davi: `AmdDisableEarlyIBPB` bingo

[2024-09-11 08:24] 0xboga: How can an AC / AV monitor RPC calls across the system ?

[2024-09-11 12:26] dullard: [replying to 0xboga: "How can an AC / AV monitor RPC calls across the sy..."]
https://www.akamai.com/blog/security/guide-rpc-filter
https://www.tiraniddo.dev/2021/08/how-windows-firewall-rpc-filter-works.html
[Embed: How the Windows Firewall RPC Filter Works]
I did promise  that I'd put out a blog post on how the Windows RPC filter works. Now that I released my more general blog post  on the Windo...

[2024-09-11 12:27] dullard: One way to monitor / block

[2024-09-11 17:33] nu11sec: is it possible to set a conditional breakpoint that would break if a certain register say eax has a pointer to a string am looking for ,  for example break if eax contains ptr to the string "needle" in x64dbg ?

[2024-09-11 19:46] Windy Bug: [replying to 0xboga: "How can an AC / AV monitor RPC calls across the sy..."]
https://www.mdpi.com/1424-8220/24/16/5118
[Embed: Detection Strategies for COM, WMI, and ALPC-Based Multi-Process Mal...]
Behavioral malware detection is based on attributing malicious actions to processes. Malicious processes may try to hide by changing the behavior of other benign processes to achieve their goals. We s

[2024-09-11 20:28] dullard: [replying to Windy Bug: "https://www.mdpi.com/1424-8220/24/16/5118"]
Good share

[2024-09-12 01:03] expy: [replying to nu11sec: "is it possible to set a conditional breakpoint tha..."]
in windbg you could try something like this `bp strcmp ".if (strcmp(@rcx, \"desired_string\") == 0 || strcmp(@rdx, \"desired_string\") == 0) { gc }"`

[2024-09-12 07:50] RiskyDissonance: [replying to nu11sec: "is it possible to set a conditional breakpoint tha..."]
Yes but you'll need to know what encoding the string is

[2024-09-12 07:51] RiskyDissonance: Check out https://help.x64dbg.com/en/latest/commands/conditional-breakpoint-control/index.html and https://help.x64dbg.com/en/latest/introduction/Expression-functions.html

[2024-09-12 07:57] nu11sec: Say for example ascii

[2024-09-12 08:55] RiskyDissonance: [replying to nu11sec: "Say for example ascii"]
`streq(ansi(rax), "test")`

[2024-09-12 09:05] nu11sec: got it noticed it isn't as hard as i thought it would, just going through the docs

[2024-09-13 10:42] EmTee: hi, anyone here reversed BattlEye?

[2024-09-13 10:48] Loading: [replying to EmTee: "hi, anyone here reversed BattlEye?"]
https://secret.club/2020/07/06/bottleye.html
[Embed: BattlEye client emulation]
The popular anti-cheat BattlEye is widely used by modern online games such as Escape from Tarkov and is considered an industry standard anti-cheat by many. In this article I will demonstrate a method 

[2024-09-13 12:51] mrexodia: [replying to davi: "Does anyone know where I can find those two `ntosk..."]
please provide the hashes, I can reupload them

[2024-09-13 17:11] davi: [replying to mrexodia: "please provide the hashes, I can reupload them"]
`78ee32fd777f0bc53c2e1977270cea3d6d6b0bbdb95a8e2be9473b12d10e1996`, `e40b74571521d1f63d387ae441b3296e505a29550be75c770851832f064d2f40`

[2024-09-13 19:31] mrexodia: [replying to davi: "`78ee32fd777f0bc53c2e1977270cea3d6d6b0bbdb95a8e2be..."]

[Attachments: 18979750745.zip]

[2024-09-13 20:21] davi: ty

[2024-09-13 20:21] davi: you got them from the UUPs?

[2024-09-13 20:22] davi: thankfully I had saved copies locally - but it wouldn't have worked out if those updates weren't really recent

[2024-09-13 20:27] Deleted User: [replying to davi: "you got them from the UUPs?"]
I assume so

[2024-09-13 20:27] Deleted User: or maybe they just keep backups of every ntoskrnl version lol

[2024-09-13 20:34] Brit: not a bad practice

[2024-09-13 20:40] davi: so true!

[2024-09-14 08:57] EmTee: [replying to Loading: "https://secret.club/2020/07/06/bottleye.html"]
im looking for informations about bypassing the HWID ban of BE

[2024-09-14 09:06] idkhidden: [replying to EmTee: "im looking for informations about bypassing the HW..."]
Check unknowncheats

[2024-09-14 09:06] EmTee: i did already, nothing but speculations

[2024-09-14 11:27] Brit: <#835634425995853834>

[2024-09-14 13:00] Deleted User: [replying to EmTee: "i did already, nothing but speculations"]
welcome to internet

[2024-09-14 13:00] Deleted User: nobody bothers to actually check what they're saying

[2024-09-14 13:00] Deleted User: just add "afaik" to the end of all your messages and you're good

[2024-09-14 13:23] Matti: [replying to EmTee: "im looking for informations about bypassing the HW..."]
buy a new computer

[2024-09-14 13:24] Matti: or (probably cheaper) don't get banned

[2024-09-14 13:24] Matti: [replying to Matti: "or (probably cheaper) don't get banned"]
afaik

[2024-09-14 13:27] nox: Based and mattipilled

[2024-09-14 13:44] Analyze: [replying to Matti: "or (probably cheaper) don't get banned"]
(Probably safer and more ethical) don’t use hacks

[2024-09-14 13:58] Windy Bug: [replying to Windy Bug: "https://www.mdpi.com/1424-8220/24/16/5118"]
One that’s not mentioned here, and I haven’t actually tried it, but I think you can hook the RPC runtime in the server, and enable RPC state information so it’s possible to identify the client PID from the server’s context.

[2024-09-14 13:58] Windy Bug: https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/rpc-state-information-internals
[Embed: RPC State Information Internals - Windows drivers]
RPC State Information Internals

[2024-09-14 14:14] Deleted User: Hey guys, quick question
I'm just trying to figure out what this one piece means, specifically ``0x1c + 0x101 * 4``

As far as I understand, 0x1C is just the entry point offset in LDR_DATA_TABLE_ENTRY, but I can't find what the 0x101 * 4 is about 

Is it just a specific location, or something else
Would've helped a lot if this mf put notes in his code
[Attachments: Screenshot_129.png]

[2024-09-14 15:24] Deleted User: real security researchers never add comments

[2024-09-14 15:24] Deleted User: just figure it out yourself <:trollface:1091188466215297104>

[2024-09-14 15:32] Deleted User: istg

[2024-09-14 16:18] Brit: it is time to go look at ntdll

[2024-09-14 16:20] Deleted User: aight aight

[2024-09-14 18:59] Matti: this looks like the kind of code that will break on the next windows update

[2024-09-14 18:59] Matti: does it work on your machine? meaning compiled from this code

[2024-09-14 19:00] Matti: otherwise you're going to have a more difficult time

[2024-09-14 19:01] daax: [replying to Matti: "does it work on your machine? meaning compiled fro..."]
it also doesn't make sense because the atom table isn't even there, it's just using padding in ntdll to write some crap for exec

[2024-09-14 19:01] Matti: yeah I was also scratching my head at that...

[2024-09-14 19:01] Matti: but hey 'atom' == win32 to me

[2024-09-14 19:01] Matti: so I just assume it's whatever black magic

[2024-09-14 19:13] Deleted User: [replying to Matti: "does it work on your machine? meaning compiled fro..."]
I'm at my job rn, but here's the code:
https://github.com/SleepTheGod/Windows-Atom-Table-Hijacking/blob/main/exploit.c
[Embed: Windows-Atom-Table-Hijacking/exploit.c at main · SleepTheGod/Window...]
A privilege escalation vulnerability exists in Windows due to a flaw in the implementation of the Atom Table. An attacker could exploit this vulnerability by injecting malicious code into the Atom ...

[2024-09-14 19:15] Matti: uhhh yeah thanks

[2024-09-14 19:15] Matti: but that doesn't answer the question

[2024-09-14 19:16] Deleted User: just for reference 
I can't tell you since my laptop is at home

[2024-09-14 19:17] Matti: well sure, no rush

[2024-09-14 19:17] Matti: just saying I'm not gonna run this code for you

[2024-09-14 19:19] Deleted User: nah I gotchu

[2024-09-14 19:28] Deleted User: [replying to Matti: "but hey 'atom' == win32 to me"]
Atoms are basically just integer values, btw

You pass in either a string or int into it, and it gives you a value to pass around

[2024-09-14 19:44] daax: [replying to Deleted User: "Atoms are basically just integer values, btw

You ..."]
we're both familiar with atoms, but the atom table that he is iterating for and adding entries for... from ntdll base + 1c just doesn't make sense since ntdll base + 1c would be within the module headers. so unless he is building a table to shove into the padding of ntdll's headers within that target process it's a little odd

[2024-09-14 19:51] Deleted User: [replying to daax: "we're both familiar with atoms, but the atom table..."]
Ah, myb

I thought that was weird too as I was only familiar with local and global atom tables before, I hadn't seen someone actually try to build one

[2024-09-14 19:56] daax: [replying to Deleted User: "Ah, myb

I thought that was weird too as I was onl..."]
nah all good, im just stating that cause we also think it's a bit cooked

[2024-09-14 19:58] daax: i went and looked at the code and the shellcode is pushing bin sh onto stack and then ntcloseobjectauditalarm or? creating its own local atom table and then? <:PES2_Shrug:513352546341879808>

[2024-09-14 21:25] Deleted User: [replying to daax: "i went and looked at the code and the shellcode is..."]
bin sh..on windows..

[2024-09-14 21:25] Deleted User: wth

[2024-09-14 21:26] Deleted User: that works?

[2024-09-14 21:26] Deleted User: nvm
I'll check it out in a VM soon enough

[2024-09-14 22:16] Brit: I haven't looked but could this be one of those meme build event things

[2024-09-14 22:16] Brit: where you download the project and it runs some postbuild thing to rat you

[2024-09-14 22:16] Brit: nah

[2024-09-14 22:16] Brit: it's just a c file

[2024-09-14 22:16] Brit: this is cooked

[2024-09-15 00:18] x86matthew: i've seen this code before, it's chatgpt-generated nonsense

[2024-09-15 00:26] emma: i love slop

[2024-09-15 00:34] Brit: yeah no, this is some larp

[2024-09-15 00:36] x86matthew: from the same "author": https://clumsylulz.medium.com/windows-dll-injection-0day-1d99a9fa7023
[Embed: Windows DLL Injection 0day]

[2024-09-15 00:38] Brit: https://clumsylulz.medium.com/how-to-make-an-ai-e-kitten-a42ea1d6175a
[Embed: How to make an AI E-Kitten]
Prompt:

[2024-09-15 00:38] Brit: bruh

[2024-09-15 00:38] Brit: <:kekw:904522300257345566>

[2024-09-15 00:38] Brit: what the fuck

[2024-09-15 00:38] Deleted User: omfg

[2024-09-15 00:38] Brit: this is what happens when they outlaw bullying

[2024-09-15 00:39] Deleted User: lemme not even open a VM for this shit

[2024-09-15 00:39] Deleted User: not finna waste my time

[2024-09-15 00:40] Brit: what is there to vm

[2024-09-15 00:40] Brit: this is a bog standard create remote thread dll injection

[2024-09-15 00:40] Brit: written in a retarded way

[2024-09-15 00:40] Brit: he calls it 0 day because he's a mongoloid

[2024-09-15 00:40] Deleted User: yo hold tf on

[2024-09-15 00:40] Deleted User: this is your brain on esex
[Attachments: Screenshot_20240914-194034_Brave.jpg]

[2024-09-15 00:41] Deleted User: ...oddly specific

[2024-09-15 00:41] Deleted User: bro gotta be a reddit mod

[2024-09-15 00:42] Deleted User: [replying to Brit: "what is there to vm"]
I meant the Atom Bombing injection

[2024-09-15 00:43] Brit: that's also not real

[2024-09-15 01:00] daax: what the fk

[2024-09-15 01:00] Deleted User: ty

[2024-09-15 01:00] daax: i was reading that

[2024-09-15 01:00] Brit: sorry

[2024-09-15 01:00] Brit: it was degenerate

[2024-09-15 01:00] daax: also yeah that atom bombing injection thing feels fake af

[2024-09-15 01:00] Azrael: He really seems to hate doxbin.

[2024-09-15 01:00] daax: ah yeah okay was established it’s fake

[2024-09-15 01:00] daax: missed that party

[2024-09-15 01:01] daax: [replying to x86matthew: "i've seen this code before, it's chatgpt-generated..."]
i looked at it for too long like ???

[2024-09-15 01:01] daax: ntdll+1c is in headers tf atom table aint there

[2024-09-15 01:02] Deleted User: headache all for some AI-generated slop

[2024-09-15 01:02] Deleted User: thanks for clearing that up though

[2024-09-15 01:03] prick: ...did a message of mine get deleted

[2024-09-15 01:04] daax: [replying to prick: "...did a message of mine get deleted"]
?

[2024-09-15 01:04] Deleted User: prolly

[2024-09-15 01:06] prick: <:hmmm:1134933049214242987>

[2024-09-15 05:33] Deleted User: [replying to x86matthew: "from the same "author": https://clumsylulz.medium...."]
such a good 0 day

[2024-09-15 05:34] Deleted User: it's not like people have been doing exactly that for the last 20 or so years

[2024-09-15 05:38] Torph: [replying to daax: "i went and looked at the code and the shellcode is..."]
what on earth

[2024-09-15 12:30] swirl: [replying to Deleted User: "I'm at my job rn, but here's the code:
https://git..."]
this is bare bone stupid nonsense

[2024-09-15 12:39] Deleted User: that has been established, yes

[2024-09-15 12:46] 5pider: [replying to Brit: "https://clumsylulz.medium.com/how-to-make-an-ai-e-..."]
this is peak content.

[2024-09-15 13:39] daax: [replying to Torph: "what on earth"]
indeed lol

[2024-09-15 17:37] Matti: [replying to Matti: "(etc)"]
happy to announce "Matti acpi.sys v8" has finally fixed this longstanding issue and removed the 64 bogus CPUs 😎
device manager now looks the same as it does in windows 7 (not counting the stuff that is broken in that OS but which I can't fix)
[Attachments: image.png]

[2024-09-15 17:39] Matti: what a beauty
surely there will be no more reasons to ever have to update this driver again, since v8 is the final and best version of all time
[Attachments: image.png]

[2024-09-15 17:39] Matti: <@991360481493262411>

[2024-09-15 17:42] Timmy: gigachad

[2024-09-15 17:43] Matti: the cause was this shit in the ACPI DSDT
(going from 00..63 of course)
[Attachments: image.png]

[2024-09-15 17:44] Matti: windows 7 and up recognise this hardcoded name in acpi.sys and treat it as a Processor device

[2024-09-15 17:44] Matti: so hypothetically if I had vista installed it would've also had the 64 CPU issue

[2024-09-15 17:46] Matti: fortunately there was actually an existing processor device in "my" acpi.sys, it just took some reversing to find its dispatch table and device flags

[2024-09-15 17:47] Matti: after that I hijacked some ancient ACPI device name like "IBM3790" and changed its name to ACPI0007, its dispatch table to the processor one, and its flags idem dito

[2024-09-15 17:48] Matti: this gets rid of 64 failed attempts to load AMDPPM.sys during boot so that is neat

[2024-09-15 19:46] irql: lmfao

[2024-09-15 19:46] irql: no way

[2024-09-15 19:47] irql: [replying to Matti: "after that I hijacked some ancient ACPI device nam..."]
lmfao

[2024-09-15 19:47] irql: 🦾 🦾

[2024-09-15 19:47] irql: that's so funny

[2024-09-15 19:47] irql: actually got a working acpi.sys

[2024-09-15 19:51] Matti: well I wouldn't use big words like "working" just yet

[2024-09-15 19:52] Matti: but there has been some improvement

[2024-09-15 19:52] Matti: I still wouldn't mind a version that even just had a fucking PDB if nothing else

[2024-09-15 19:54] irql: lmfao

[2024-09-15 20:23] 0x208D9: https://github.com/Dump-GUY/IDA_PHNT_TYPES
[Embed: GitHub - Dump-GUY/IDA_PHNT_TYPES: Converted phnt (Native API header...]
Converted phnt (Native API header files from the System Informer project) to IDA TIL, IDC (Hex-Rays). - Dump-GUY/IDA_PHNT_TYPES

[2024-09-15 20:23] 0x208D9: if anyone finds useful

[2024-09-15 20:56] 0x208D9: also does anyone knows NtRays equivalent ?  it seems to not load on IDA 8.4 (my assumption is that , due to the massive API changes IDA went through) , will need to modify the source if necessary but still asking if anyone is aware

[2024-09-15 21:30] koyz: [replying to 0x208D9: "also does anyone knows NtRays equivalent ?  it see..."]
You have to compile it from source again against the new SDK