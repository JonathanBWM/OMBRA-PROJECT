# April 2025 - Week 3
# Channel: #programming
# Messages: 323

[2025-04-14 00:00] diversenok: Ah, here it is
[Attachments: image.png]

[2025-04-14 00:01] diversenok: There were several named pipe handles and I needed to click things in the UI to make one of them start hanging

[2025-04-14 00:02] diversenok: So looks like it's a file object lock issue

[2025-04-14 00:02] Matti: ah yeah

[2025-04-14 00:03] Matti: `IopQueryXxxInformation`takes a lock

[2025-04-14 00:03] Matti: on the file object

[2025-04-14 00:03] Matti: I'm pretty sure there's an alternate path that doesn't, but it's only used internally for kernel callers or something like that

[2025-04-14 00:04] diversenok: I'll see if I can trace where this handle comes from to make a PoC

[2025-04-14 00:04] Matti: which is specifically documented to exist to avoid a deadlock

[2025-04-14 00:05] Matti: `IopQueryNameInternal`
[Attachments: image.png]

[2025-04-14 00:06] Matti: both end up creating an IRP for `IRP_MJ_QUERY_INFORMATION` and running the query with `IoCallDriver()`

[2025-04-14 00:07] Matti: only `IopGetFileInformation` is missing the usual wait if pending returned because it assumes a synchronously opend file object

[2025-04-14 00:07] Matti: other than that the file object lock seems to be the only real difference

[2025-04-15 15:16] diversenok: Sure. I found that it's actually really use to reproduce: create a blocking anonymous pipe, and pass the reading end to `ReadFile` on a dedicated thread (which will block it). Now attempting to query the name of the same handle hangs

[2025-04-15 15:16] qw3rty01: he just spammed every channel with that message so probably just a bot

[2025-04-15 15:17] diversenok: Oh well

[2025-04-15 15:39] Matti: [replying to diversenok: "Sure. I found that it's actually really use to rep..."]
cheers, thanks for looking into this

[2025-04-15 15:39] Matti: that does sound easy enough to reproduce

[2025-04-15 15:41] Matti: I guess the name querying logic clearly wasn't built with 'forever' blocking objects in mind, but that is still stupid (at least in hindsight) design that I think could definitely be handled properly

[2025-04-15 15:42] Matti: I guess it would need to be possible for name queries to fail

[2025-04-15 15:42] Matti: or alternatively give a timeout parameter to obquerynamestring

[2025-04-15 15:44] diversenok: Oh, another funny addition. If you suspend the thread that blocks the pipe via `NtReadFile` and then attempt to cancel its I/O,`NtCancelSynchronousIoFile` also hangs
[Attachments: image.png]

[2025-04-15 15:45] diversenok: Huh, despite the name `NtCancelSynchronousIoFile` accepts a thread handle, not a file

[2025-04-15 15:45] Matti: I guess the most common / general case could even be predetermined accurately like 99% of the time.... but it's not a guaranteed fix, it'd just make the failure status come back more quickly in the common case

[2025-04-15 15:46] Matti: [replying to diversenok: "Huh, despite the name `NtCancelSynchronousIoFile` ..."]
I believe this cancels the thread's top IRP or something.... and yeah I've seen better named functions

[2025-04-15 15:47] Matti: ah no, think I'm thinking of a different cancel API

[2025-04-15 15:47] Matti: well it might still do that <:lillullmoa:475778601141403648>

[2025-04-15 15:48] Matti: weird, I don't even have this implemented in MRK

[2025-04-15 15:48] Matti: time for IDA I guess...

[2025-04-15 15:48] Matti: oh I see, it was also introduced in 22000

[2025-04-15 15:48] Matti: so it's fairly recent

[2025-04-15 15:49] James: does MRK stand for what I think it does?

[2025-04-15 15:49] diversenok: Wait, I thought it was old; definitely works on Win 10

[2025-04-15 15:49] Matti: no wait, earlier than that

[2025-04-15 15:49] Matti: it's only exported by ntoskrnl since then

[2025-04-15 15:49] diversenok: phnt says Vista

[2025-04-15 15:49] Matti: yeah

[2025-04-15 15:50] Matti: that's bad

[2025-04-15 15:50] Matti: how do I not have that

[2025-04-15 15:50] Matti: [replying to James: "does MRK stand for what I think it does?"]
uhm... depends

[2025-04-15 15:50] James: matti research kernel

[2025-04-15 15:51] Matti: if you are thinking of something like, my own version of the microsoft research kernel, then no

[2025-04-15 15:51] Matti: that would obviously be illegal

[2025-04-15 15:51] Matti: ha ha

[2025-04-15 15:51] Matti: (but yes that's what it is)

[2025-04-15 15:53] James: 🤣

[2025-04-15 15:53] James: nice

[2025-04-15 15:53] Matti: yeah ok it sort of was what I was thinking of.... sort of

[2025-04-15 15:53] Matti: only not just the top level IRP but all of the thread's IRPs

[2025-04-15 15:53] Matti: 
[Attachments: image.png]

[2025-04-15 15:55] Matti: I remember this a bit more clearly now... the implementation of *that* function is in fact surprisingly complex, and furthermore it turned out no one fucking calls this syscall

[2025-04-15 15:55] Matti: [replying to diversenok: "Oh, another funny addition. If you suspend the thr..."]
making this a world first

[2025-04-15 15:57] Matti: I similarly have got NtCancelIoFile but not NtCancelIoFileEx

[2025-04-15 15:57] Matti: seems I'm not big on cancelling

[2025-04-15 15:58] Matti: and the reason is that this API too ends up in `IopCancelIrpsInThreadListForCurrentProcess` eventually <:thinknow:475800595110821888>

[2025-04-15 15:58] Matti: well, process, thread list, what's the difference

[2025-04-15 15:58] diversenok: Oh, `NtCancelIoFileEx` is quite interesting; didn't know about its ability to identify IRPs by `IO_STATUS_BLOCK`

[2025-04-15 15:59] Matti: it's ass to implement

[2025-04-15 15:59] Matti: yea it is

[2025-04-15 16:00] Matti: what a bunch of stupid fucking asserts everywhere in here
[Attachments: image.png]

[2025-04-15 16:01] Matti: this sounds like problem solved to me

[2025-04-15 16:03] Matti: [replying to Matti: "seems I'm not big on cancelling"]
oh well, I've still got `TmCancelPropagationRequest` and `ZwAlpcCancelMessage` 😎

[2025-04-15 16:04] Deleted User: <@687117677512360003> do you use windows

[2025-04-15 16:04] Matti: are you posting a bait question right now while a moderator is talking in the same fucking channel

[2025-04-15 16:04] Deleted User: [replying to Matti: "are you posting a bait question right now while a ..."]
?

[2025-04-15 16:04] Deleted User: it's a genuine question

[2025-04-15 16:04] Matti: of course

[2025-04-15 16:05] Matti: take it somewhere else anyway

[2025-04-15 16:05] Deleted User: why

[2025-04-15 16:05] Deleted User: i'm asking about Ocaml on windows

[2025-04-15 16:05] Deleted User: that's to do with <#835664858526646313>

[2025-04-15 16:05] Matti: 😩

[2025-04-15 16:07] Deleted User: not sure at all how that's bait but ok

[2025-04-15 16:10] Matti: no colin loves windows, we all know this

[2025-04-15 16:11] Yoran: [replying to Deleted User: "<@687117677512360003> do you use windows"]
Its trivial he hates it

[2025-04-15 16:11] Yoran: Putting my balls on it

[2025-04-15 16:12] Deleted User: [replying to Matti: "no colin loves windows, we all know this"]
i don't know him as much as you do

[2025-04-15 16:13] diversenok: [replying to Matti: "or alternatively give a timeout parameter to obque..."]
Timeout or not, it still takes time to realize the operation is not going to return soon. Now duplicate the problematic handle a few thousand times and observe how handle inspection tools hang for hours

[2025-04-15 16:13] contificate: [replying to Deleted User: "<@687117677512360003> do you use windows"]
I don't

[2025-04-15 16:14] diversenok: At least `NtQueryInformationFile` with `FileVolumeNameInformation` doesn't hang

[2025-04-15 16:14] Matti: [replying to diversenok: "Timeout or not, it still takes time to realize the..."]
yeah for sure, the timeout parameter would be for kernel mode callers by definition though
but I also have to say I don't see much use for calling a function that may return instantly or in 20 seconds

[2025-04-15 16:14] Deleted User: [replying to contificate: "I don't"]
dont tell me im going to have to use linux for ocaml

[2025-04-15 16:14] contificate: you can use WSL

[2025-04-15 16:15] Matti: [replying to Deleted User: "i don't know him as much as you do"]
look mate, if that's true I believe you, but I've seen enough of your own posts by now that I'm fairly sure no one would mind if you lurked a little more and posted a little less

[2025-04-15 16:15] Yoran: Or just use ocaml on windows?

[2025-04-15 16:16] Deleted User: [replying to Yoran: "Or just use ocaml on windows?"]
there's barely support for it

[2025-04-15 16:16] Yoran: [replying to Deleted User: "there's barely support for it"]
enjoy wsl

[2025-04-15 16:17] contificate: at my last job, which was all OCaml, I did actually use WSL2

[2025-04-15 16:17] contificate: because of company policy

[2025-04-15 16:17] contificate: it was fairly alright

[2025-04-15 16:17] Deleted User: alright

[2025-04-15 16:17] contificate: it's not a setup I would use at home, but emacs worked perfectly

[2025-04-15 16:17] contificate: it was flawless, actually

[2025-04-15 16:17] contificate: only downside was that the company required Debian or Fedora

[2025-04-15 16:17] contificate: but I mean.. can't build the product without those

[2025-04-15 16:18] contificate: in the end

[2025-04-15 16:18] contificate: I actually use Debian and used distrobox to overlay a Fedora instance

[2025-04-15 16:18] contificate: where I did my work, a I needed koji etc. build tooling

[2025-04-15 16:18] contificate: as the product is an operating system

[2025-04-15 16:22] contificate: I've considered installing Windows as of late, for some specific development work

[2025-04-15 16:22] contificate: want to relive my childhood with VS2010/12 and Xbox 360 Neighborhood

[2025-04-15 16:29] Matti: excellent choice

[2025-04-15 16:30] Matti: may I suggest windows 7 and VS2010 SP1

[2025-04-15 16:30] Matti: neither of those got any better after those versions

[2025-04-15 16:50] Lyssa: kill

[2025-04-15 16:50] Lyssa: discord

[2025-04-15 16:50] Lyssa: now

[2025-04-15 16:50] Lyssa: 💀 sorry for the ping matti

[2025-04-15 16:51] Lyssa: i didn't mean to

[2025-04-15 16:51] Matti: how did I know you meant me despite the lack of ping

[2025-04-15 16:52] Lyssa: we share at least a few braincells

[2025-04-15 16:52] Lyssa: telekinesis

[2025-04-15 16:52] Lyssa: wait no that's wrong

[2025-04-15 16:52] Lyssa: brain to brain communication

[2025-04-15 16:52] Matti: I can't kill discord, then I'll really have nothing to do

[2025-04-15 18:12] unknown: is there a way for windbg to display contents at an address as a struct

[2025-04-15 18:13] unknown: like ik at addr 0xAAAAAAAA the struct is ContextRecord

[2025-04-15 18:18] unknown: nvm i found it

[2025-04-15 18:38] truckdad: for anyone seeing it and wondering: `dt` (or `dx` to be a bit fancier, especially if you have symbols and the expression’s type is already known)

[2025-04-15 18:53] unknown: `dt [symbol] [addr]`

[2025-04-15 21:41] unknown: im reversing an exe and looking at some seh,
this is the `RUNTIME_FUNCTION` struct whose last member holds the unwind info address. this is how it looks like in ida 
```
.pdata:0000000140004018                 RUNTIME_FUNCTION <rva start, rva stru_140002020, rva stru_140002020>```

heres what the `UNWIND_INFO` struct looks like
```.rdata:0000000140002020 stru_140002020  UNWIND_INFO_HDR <1, 1, 0, 0, 0, 0>
.rdata:0000000140002024                 dd rva sub_140001020
.rdata:0000000140002028                 dd rva sub_140001000
```
according to my understanding the `sub_140001020` will be called when an exception occurs in `start` but why is the `sub_140001000` there? can anyone tell about this?

[2025-04-15 21:51] James: [replying to unknown: "im reversing an exe and looking at some seh,
this ..."]
screenshot these things in ida

[2025-04-15 21:51] James: instead of sending text

[2025-04-15 21:52] James: and ill tell u

[2025-04-15 21:54] unknown: [replying to James: "and ill tell u"]

[Attachments: image.png, image.png]

[2025-04-15 21:55] James: no

[2025-04-15 21:55] James: thats not what it means

[2025-04-15 21:55] James: that header is entirely invalid

[2025-04-15 21:55] James: oh wait

[2025-04-15 21:55] James: wait wait

[2025-04-15 21:56] James: hover

[2025-04-15 21:56] James: the structure

[2025-04-15 21:56] James: the ehader structure

[2025-04-15 21:56] unknown: which one?

[2025-04-15 21:56] James: it will tell u what each field is

[2025-04-15 21:56] unknown: ehader?

[2025-04-15 21:56] James: header

[2025-04-15 21:56] unknown: ok

[2025-04-15 21:56] unknown: [replying to James: "the ehader structure"]
yeah i hovered over it

[2025-04-15 21:56] unknown: ik what it means

[2025-04-15 21:57] unknown: but why is the dd rva there

[2025-04-15 21:57] unknown: the second one

[2025-04-15 21:57] James: thats metadata

[2025-04-15 21:57] James: it can be anything, depending on the handler itself

[2025-04-15 21:58] James: can u send me the binary?

[2025-04-15 21:58] unknown: [replying to James: "thats metadata"]
look heres this
[Attachments: image.png]

[2025-04-15 21:59] unknown: [replying to James: "can u send me the binary?"]
sorry i cant

[2025-04-15 21:59] unknown: 
[Attachments: image.png]

[2025-04-15 21:59] unknown: acc to ms docs

[2025-04-15 21:59] unknown: the flag is set to 1

[2025-04-15 22:00] unknown: [replying to James: "thats metadata"]
idk why UNWIND_INFO_HDR is different than the one provided by ms

[2025-04-15 22:00] James: looks fine to me

[2025-04-15 22:01] James: can u click on the first function

[2025-04-15 22:01] unknown: [replying to James: "looks fine to me"]

[Attachments: image.png]

[2025-04-15 22:01] James: and show me the disassembly of it

[2025-04-15 22:01] unknown: [replying to James: "and show me the disassembly of it"]
yeah wait

[2025-04-15 22:01] James: [replying to unknown: ""]
this is the entire metadata

[2025-04-15 22:01] James: ida's is just the header

[2025-04-15 22:02] unknown: ```x86asm
sub_140001020 proc near
mov     rax, [r9+38h]
mov     eax, [rax]
add     rax, [r9+8]
mov     [r8+0F8h], rax
xor     eax, eax
retn
sub_140001020 endp```

[2025-04-15 22:02] James: so....

[2025-04-15 22:02] James: if i had to guess

[2025-04-15 22:02] James: without and structure definitions(which you can go add)

[2025-04-15 22:02] unknown: yeah

[2025-04-15 22:03] James: this is first loading the address of the metadata into rax

[2025-04-15 22:03] James: then loading the 4 byte rva

[2025-04-15 22:03] James: then adding that to image base to get an absolute address

[2025-04-15 22:03] James: then writing that to RIP

[2025-04-15 22:03] James: before returning continue execution

[2025-04-15 22:03] James: meaning this is going to cause an exception, the handler will catch it, and redirect flow to that second rva, stored in the metadata

[2025-04-15 22:05] unknown: [replying to James: "this is first loading the address of the metadata ..."]
you meant by this?
```cpp
typedef struct _DISPATCHER_CONTEXT {
    ULONG64 ControlPc;
    ULONG64 ImageBase;
    PRUNTIME_FUNCTION FunctionEntry;
    ULONG64 EstablisherFrame;
    ULONG64 TargetIp;
    PCONTEXT ContextRecord;
    PEXCEPTION_ROUTINE LanguageHandler;
    PVOID HandlerData;
} DISPATCHER_CONTEXT, *PDISPATCHER_CONTEXT;```

[2025-04-15 22:05] James: yeah so thats in R9

[2025-04-15 22:05] James: so yeah this is whats happening

[2025-04-15 22:05] unknown: i got what u meant by loading the image addr

[2025-04-15 22:05] unknown: but i dont quite get how does it load the metadata into rax

[2025-04-15 22:06] unknown: [replying to unknown: "you meant by this?
```cpp
typedef struct _DISPATCH..."]
0x38 here is PVOID HandlerData

[2025-04-15 22:06] unknown: is this the metadata?

[2025-04-15 22:06] James: ```
sub_140001020 proc near
mov     rax, [r9+38h]   ; load address of metadata(points to 4 byte rva after the handler rva)
mov     eax, [rax]      ; load those 4 bytes... 
add     rax, [r9+8]     ; load image base from dispatcher_context and add to the disp
mov     [r8+0F8h], rax  ; move into RIP
xor     eax, eax        ; return 0 to continue execution at RIP.
retn
sub_140001020 endp
```

[2025-04-15 22:06] James: yeah

[2025-04-15 22:06] James: handler data is the metadata

[2025-04-15 22:06] unknown: oh fuck

[2025-04-15 22:07] unknown: [replying to James: "handler data is the metadata"]
it makes sense now

[2025-04-15 22:07] unknown: [replying to James: "handler data is the metadata"]
i actually find no info what is HandlerData

[2025-04-15 22:07] unknown: its not documented ig

[2025-04-15 22:07] James: yeah not well documented

[2025-04-15 22:08] unknown: [replying to James: "yeah not well documented"]
do ya have one?

[2025-04-15 22:09] James: no i don't even remember where i learned it tbh

[2025-04-15 22:09] unknown: welp

[2025-04-15 22:09] unknown: i understand it now

[2025-04-15 22:09] unknown: thanks for the help

[2025-04-15 22:10] James: np

[2025-04-16 16:45] Matti: [replying to unknown: "0x38 here is PVOID HandlerData"]
https://github.com/dotnet/runtime/blob/main/src/coreclr/unwinder/amd64/unwinder.cpp#L319-L373
[Embed: runtime/src/coreclr/unwinder/amd64/unwinder.cpp at main · dotnet/r...]
.NET is a cross-platform runtime for cloud, mobile, desktop, and IoT apps. - dotnet/runtime

[2025-04-16 16:45] Matti: this is as close an explanation as you're going to find I think

[2025-04-16 16:46] Matti: given <https://github.com/dotnet/runtime/blob/main/src/coreclr/unwinder/amd64/unwinder.cpp#L440-L448>

[2025-04-16 16:46] Matti: this comment

[2025-04-16 17:36] unknown: [replying to Matti: "https://github.com/dotnet/runtime/blob/main/src/co..."]
thank you!

[2025-04-16 22:10] the horse: i miss jonas lyk

[2025-04-16 22:10] the horse: your notes remind me so much of the stuff he sent

[2025-04-16 22:10] the horse: tygoodshare ^^

[2025-04-17 07:08] sariaki: fligh high king 👑 🕊️

[2025-04-17 07:29] roddux: [replying to the horse: "i miss jonas lyk"]
he’s still around

[2025-04-17 07:30] roddux: he lurks on the smaller c/c++ serv

[2025-04-17 09:13] Matti: [replying to the horse: "your notes remind me so much of the stuff he sent"]
I love jonas as much as the next guy, but I hope you're referring more to to contents of my posts and not so much their readability lol

[2025-04-17 12:34] 0x208D9: [replying to roddux: "he lurks on the smaller c/c++ serv"]
he lurks on the "Together C and C++" server these days

[2025-04-17 12:35] 0x208D9: also did anyone got this weird issue from the discord? idk why i cant join python discord server even tho their admins says that im not banned

[2025-04-17 12:38] roddux: it's probably doing you a favour tbh

[2025-04-17 17:53] the horse: [replying to Matti: "I love jonas as much as the next guy, but I hope y..."]
yes

[2025-04-17 17:53] the horse: content

[2025-04-17 17:53] the horse: random crazy useful stuff

[2025-04-17 17:54] Matti: aw, thanks

[2025-04-17 17:54] Matti: I appreciate it

[2025-04-18 09:11] unknown: if someone were to write a binary rewriter whos extremely new to the topic. what things should i keep in mind

[2025-04-18 09:53] brymko: relative jumps are probably your biggest issue starting out

[2025-04-18 15:28] James: [replying to unknown: "if someone were to write a binary rewriter whos ex..."]
If you search for iizerd/BDASM it’s something I wrote quite a while ago and was proud of at the time. If you visit the instruction file you can see how I keep track of fixups I need to make when reassembling instructions.

[2025-04-18 19:33] pinefin: are there any other menu frameworks kind of like imgui for c++? this would be for a desktop application, and i like iced/egui in rust but i really dont wanna make this in rust

[2025-04-18 19:33] pinefin: as well as GL compatible cause it'd be shipped on all 3 different major os's

[2025-04-18 19:34] pinefin: or would i just have to make a middleware between rust/c++

[2025-04-18 19:58] sariaki: [replying to pinefin: "are there any other menu frameworks kind of like i..."]
https://github.com/Immediate-Mode-UI/Nuklear
[Embed: GitHub - Immediate-Mode-UI/Nuklear: A single-header ANSI C immediat...]
A single-header ANSI C immediate mode cross-platform GUI library - Immediate-Mode-UI/Nuklear

[2025-04-18 19:59] sariaki: oh or do you mean something like imgui that's actually developed for c++?

[2025-04-18 20:01] pinefin: [replying to sariaki: "oh or do you mean something like imgui that's actu..."]
yeah i mean something more modern, but like imgui

[2025-04-18 20:01] pinefin: like i said iced/egui really catch my eye

[2025-04-18 21:09] roddux: imgui has c++ bindings?

[2025-04-18 22:18] pinefin: [replying to roddux: "imgui has c++ bindings?"]
well duh. im just trying to not use imgui

[2025-04-18 22:19] roddux: but why lol

[2025-04-18 22:19] roddux: you asked for things like it, specifically

[2025-04-18 22:19] roddux: why not just use it? genuine question

[2025-04-18 22:36] Matti: what is not modern about imgui

[2025-04-18 22:37] Matti: it can target pretty much any renderer for any OS that's used on desktops

[2025-04-18 22:40] Matti: <https://github.com/ocornut/imgui/tree/master/backends> depending on the OS, especially on windows, there's almost too many backends to choose from

[2025-04-18 22:40] Matti: but vulkan, metal and opengl2(!)+ are also there

[2025-04-19 07:37] Timmy: <@918151599807946752> raylibs raygui?

[2025-04-19 09:54] NSA, my beloved<3: In Windows, x86, so the SysWOW64 version of kernel32.BaseThreadInitThunk calls the entry point of a thread with 2 parameters: ecx -> PEB, and it pushes the pointer to the thread parameter that can be specificed in CreateThread which then can be accessed with esp+4. The characteristics of this only matches the `__thsicall` calling convention, where the first parameter (this) is inside ecx, rest is on the stack. However I can not declare main using `__thiscall` to access both ecx for the PEB and the stack value before the function return that is the thread parameter (mainCRTStartup is omitted, my main function is the direct entry of the executable) because MSVC will complain that `__thiscall` can only be used on nonstatic member functions. Is my only option here inline assembly to access both the PEB and the stack thread parameter? 🤔

[2025-04-19 11:18] x86matthew: [replying to NSA, my beloved<3: "In Windows, x86, so the SysWOW64 version of kernel..."]
there is no `thiscall` involved, the loader passes the peb base address as the parameter to the main thread entry-point if this is what you mean?

[2025-04-19 11:18] x86matthew: if you've defined a custom entry-point, you can just read it like this `entry_point(PEB *peb)`, nothing special needed

[2025-04-19 11:24] NSA, my beloved<3: [replying to x86matthew: "if you've defined a custom entry-point, you can ju..."]
Right, given that the first parameter is treated as the ecx, which would translate to a `__fastcall`. However, in the SysWOW64 version, the thread parameter is located on the stack. The parameter that can be specified when creating the thread. To access it, you would need to climb the stack back up, and grab the value that is pushed onto the stack before the call to the entry point.

[2025-04-19 11:25] NSA, my beloved<3: And so I was wondering, which calling convention would allow me to define main as: `entry_point(PEB *peb, LPVOID lpParameter)` and have the compiler treat peb as ecx and lpParameter as a stack value.

[2025-04-19 11:29] x86matthew: peb is the parameter for the entry-point though

[2025-04-19 11:29] NSA, my beloved<3: One of them, the other one should be the user defined thread parameter through CreateThread.

[2025-04-19 11:30] NSA, my beloved<3: To give you more context, I disassembled a binary which has a function that acts as the entry point for a thread that was made by CreateThread.

[2025-04-19 11:30] NSA, my beloved<3: And in here they are accessing the thread parameter they used for CreateThread, through the stack.

[2025-04-19 11:31] NSA, my beloved<3: And if you look at the SysWOW64 implementation of BaseThreadInitThunk, you can see that right before the function call there is a mov ecx and a push.

[2025-04-19 11:31] x86matthew: [replying to NSA, my beloved<3: "One of them, the other one should be the user defi..."]
there's only one parameter that matters, for the main entry-point it's PEB, for other threads it's user-defined

[2025-04-19 11:32] NSA, my beloved<3: Let me visualize it, hold on.

[2025-04-19 11:32] truckdad: you know you can always get the PEB through the TEB, which you can always access via fs/gs, right?

[2025-04-19 11:32] NSA, my beloved<3: Yes.

[2025-04-19 11:34] truckdad: there are intrinsics to do so, you don’t need inline assembly

[2025-04-19 11:34] truckdad: (and thread entry points must always be `__stdcall`)

[2025-04-19 11:35] NSA, my beloved<3: The highlited line is the data they are accessing from their entry point.
[Attachments: image.png]

[2025-04-19 11:35] NSA, my beloved<3: Here is the disassembly.
[Attachments: image.png]

[2025-04-19 11:36] truckdad: i mean yeah, that’s where you would get the parameter from in a `__stdcall` function

[2025-04-19 11:36] NSA, my beloved<3: Right, but this is not the PEB. It is now the user-defined thread parameter.

[2025-04-19 11:36] NSA, my beloved<3: However, the PEB is still in ecx.

[2025-04-19 11:37] truckdad: that doesn’t inherently indicate that it’s being passed as an argument

[2025-04-19 11:38] Matti: it's not the PEB anymore at the point you're looking at

[2025-04-19 11:38] Matti: because ntdll calls this function

[2025-04-19 11:39] NSA, my beloved<3: I see now. Do you know by any chance in which cases the the user-defined parameter is passed as the argument and in which is it the PEB? If not I can do dynamic analysis and try to figure it out. Other than that, my question still stands on if I could define `entry_point(PEB *peb, LPVOID lpParameter)` and have the compiler treat peb as ecx and lpParameter as the stack?

[2025-04-19 11:39] Matti: this will be whatever user defined parameter was given to createthread

[2025-04-19 11:39] Matti: [replying to Matti: "it's not the PEB anymore at the point you're looki..."]
actually it never was the PEB for a thread entry point, but yeah ntdll calls both types of kernel32 init functions

[2025-04-19 11:40] Matti: [replying to NSA, my beloved<3: "I see now. Do you know by any chance in which case..."]
the PEB is the **P**rocess environment block

[2025-04-19 11:41] Matti: you are talking about a thread entry

[2025-04-19 11:42] NSA, my beloved<3: So is it not guaranteed that ecx is always holding the address of the PEB by the time BaseThreadInitThunk gets to calling the custom/executable's entry point?

[2025-04-19 11:42] NSA, my beloved<3: I swear I read that somewhere.

[2025-04-19 11:42] Matti: uhm, yeah it is not

[2025-04-19 11:43] NSA, my beloved<3: Sorry, made a correction.

[2025-04-19 11:43] NSA, my beloved<3: I may be confusing the main thread and it calling the executable entry point and user created threads with custom entry points. In terms of the PEB parameter.

[2025-04-19 11:43] x86matthew: you're probably confusing it with rcx on x64 binaries (still only true for the main entry-point though, not all threads)

[2025-04-19 11:44] Matti: I think so too

[2025-04-19 11:45] diversenok: You might be talking about native subsystem executables. They do receive PEB as their parameter (see NtProcessStartup)

[2025-04-19 11:45] NSA, my beloved<3: All right, stuff is clearer now for sure. I'll see what happens if I don't define any lpParameters. I wonder if kernel32.dll passes the PEB then.

[2025-04-19 11:45] Matti: [replying to x86matthew: "you're probably confusing it with rcx on x64 binar..."]
fun fact: this is a different register on XP x64/server 2003 x64

[2025-04-19 11:45] NSA, my beloved<3: Thank yo guys for the help, appreciate it.

[2025-04-19 11:45] Matti: I forget which one though

[2025-04-19 11:46] Matti: I changed it in MRK to make it easier for my brain to deal with <:kekw:904522300257345566>

[2025-04-19 11:47] Matti: ah hm wait, looking at the git history now it seems what I'm talking about is actually the win32 thread EP, not the process/executable one

[2025-04-19 11:48] Matti: from some user mode program of mine:
```
// Context register the kernel stores Thread->Win32StartAddress in after creating the TEB
#if defined(_M_AMD64)
    #define WIN32_THREADSTART_FROM_CONTEXT(Context)            ((Context).Rcx)
#elif defined(_M_IX86)
    #define WIN32_THREADSTART_FROM_CONTEXT(Context)            ((Context).Eax)
#elif defined(_M_ARM)
    #define WIN32_THREADSTART_FROM_CONTEXT(Context)            ((Context).R0)
#elif defined(_M_ARM64)
    #define WIN32_THREADSTART_FROM_CONTEXT(Context)            ((Context).X0)
#else
    #error Platform not supported
#endif
```

[2025-04-19 11:48] Matti: this was not originally rcx for xp x64

[2025-04-19 11:48] diversenok: That looks like your process hollowing demo 🙂

[2025-04-19 11:49] Matti: ya

[2025-04-19 11:49] Matti: well spotted!

[2025-04-19 11:54] Matti: god damn what a fatty

[2025-04-19 11:55] Matti: it's now 14 KB in release mode for ARM64 <:worrystare:1301924942396522677>

[2025-04-19 11:55] Matti: getting closer to that 20 KB deletion limit every commit....

[2025-04-19 11:56] Matti: I blame the stupid XML manifest it it needs to include in order for any win32 GUI programs to halfway work

[2025-04-19 12:00] diversenok: You don't technically need it; you can achieve the same effect in runtime by generating and applying an activation context with comctl32 redirection before calling GUI functions

[2025-04-19 12:01] Matti: yes I know <:kekw:904522300257345566>

[2025-04-19 12:01] Matti: that would surely push it over 100 KB, not 20

[2025-04-19 12:01] Matti: not to mention I'd probably implement it in an almost-correct but actually broken way

[2025-04-19 12:01] Matti: fuck sxs

[2025-04-19 12:03] diversenok: Haha. Nah, I'm talking about a `CreateActCtxW` against any built-in executable with the right manifest + adjusting two pointers in PEB

[2025-04-19 12:04] Matti: well but that would violate my 1 import from kernel32 max rule

[2025-04-19 12:04] Matti: that being ConsoleWriteW or whatever it's called

[2025-04-19 12:05] Matti: WriteConsoleW*

[2025-04-19 12:05] Matti: obviously...

[2025-04-19 12:07] diversenok: Okay.  How about copying an already prepared activation context region from another process?

[2025-04-19 12:07] Matti: no can do, sorry

[2025-04-19 12:07] Matti: I mean I could also just create a dummy GUI process with the manifest

[2025-04-19 12:07] Matti: and use that for the hollowing

[2025-04-19 12:07] Matti: but I think that's rude

[2025-04-19 12:08] Matti: it'd be making some innocent notepad.exe or whatever look like the criminal, instead of Hollowing.exe

[2025-04-19 12:09] diversenok: Manually load the right comctl32 from WinSxS?

[2025-04-19 12:09] diversenok: You can read somebody's activation context to get the path

[2025-04-19 12:10] Matti: hmmm, I don't remember *for sure* that I tried this, but I think I did and it didn't actually work due to all kinds of hidden state in sxs.dll causing issues

[2025-04-19 12:11] Matti: or the activation context responsible for the DLL being redirected rather, I think

[2025-04-19 12:11] Matti: SXS is pure gore

[2025-04-19 12:11] Matti: I hate it for another reason too

[2025-04-19 12:11] Matti: its abuse of DbgPrintEx

[2025-04-19 12:12] Matti: if you `ed nt!Kd_Win2000_Mask 0x1f;g`, you'll soon be greeted with pages and pages of.... XML

[2025-04-19 12:12] Matti: forever

[2025-04-19 12:12] Matti: making the system unusable

[2025-04-19 12:13] Matti: because it's a shitload of XML

[2025-04-19 12:13] diversenok: Oh yeah, I saw that

[2025-04-19 12:14] diversenok: [replying to Matti: "hmmm, I don't remember *for sure* that I tried thi..."]
I tried late SxS registration (after the process started running) and it was funny. GUI created before registration was using comctl32 v5 and created after was using comctl32 v6. It looked like a Frankenstein

[2025-04-19 12:14] Matti: oh yes! I also remember somehow causing this once

[2025-04-19 12:14] Matti: good stuff

[2025-04-19 12:15] Matti: I think I was misunderstanding or misusing SXS isolation in some dumb way in a DLL

[2025-04-19 12:16] Matti: https://omnicognate.wordpress.com/2009/10/05/winsxs/ what a perfect title
[Embed: Everything you Never Wanted to Know about WinSxS]
I created this blog in order to post this article about WinSxS. It is based on my own reading of the docs and some experimentation. It does not represent the views of Microsoft or my employer. WinS…

[2025-04-19 12:16] Matti: going to read this now

[2025-04-19 12:18] Matti: > The first thing to get straight is the relationship between WinSxS and .NET assembly binding. Both have “assemblies”, “assembly identities” and “manifests”, and they resemble each other in many other ways: WinSxS has the system assembly cache (winsxs folder), .NET has the GAC; WinSxS has Activation Contexts, .NET has AppDomains; WinSxS has “publisher configuration files”, .NET has “publisher policy files”, etc, etc.
I never even considered this

[2025-04-19 12:18] Matti: but he's not wrong

[2025-04-19 12:36] Matti: [replying to Matti: "https://omnicognate.wordpress.com/2009/10/05/winsx..."]
I should maybe add a gore warning to this link

[2025-04-19 12:36] Matti: terrifying read

[2025-04-19 12:37] Matti: simply playing "spot the fatty" on say csrss.exe is also telling tbh

[2025-04-19 12:39] Matti: csrsrv.dll, which as we know is basically the entire CSRSS, is 100 KB
and sxs.dll, the DLL with the limited and utterly broken API described above, is 652 KB

[2025-04-19 14:08] 0xatul: [replying to Matti: "https://omnicognate.wordpress.com/2009/10/05/winsx..."]
thanks for the link