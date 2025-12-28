# August 2024 - Week 3
# Channel: #reverse-engineering
# Messages: 147

[2024-08-12 10:51] marc: in xdbg, is there any way to start a program as a service? I have to debug something a service does right when it stars, but I can only attach to it after I start the service, since if I start it with xdbg, the program exists right after the entry point

[2024-08-12 12:34] Timmy: maybe try starting the service suspended?

[2024-08-12 12:35] Timmy: *process

[2024-08-12 17:02] mrexodia: [replying to marc: "in xdbg, is there any way to start a program as a ..."]
There is an issue related to this with some suggested workaround, but generally speaking it’s not supported

[2024-08-12 17:02] mrexodia: (And the tool is named “x64dbg”, still no idea why people say xdbg <:harold:704245193016344596>)

[2024-08-12 17:03] marc: [replying to mrexodia: "(And the tool is named “x64dbg”, still no idea why..."]
😭 I thought it was the other way around

[2024-08-12 17:03] marc: [replying to mrexodia: "There is an issue related to this with some sugges..."]
I see

[2024-08-12 17:03] mrexodia: Crazy that the website is x64dbg.com <:kappa:697728545631371294>

[2024-08-12 17:03] marc: I ended up using cdb.exe and attaching with windbg

[2024-08-12 17:03] marc: it was painful working with windbg but I'm getting the hang of it

[2024-08-12 17:05] mrexodia: Windbg is beautiful

[2024-08-12 17:05] mrexodia: You are now enlightened

[2024-08-12 17:05] marc: yea I agree but the learning curve is tough I feel like

[2024-08-12 17:06] marc: x**64**dbg is much more intuitive

[2024-08-12 17:15] daax: [replying to mrexodia: "(And the tool is named “x64dbg”, still no idea why..."]
why say short word when shorter word do trik

[2024-08-12 17:22] contificate: it's actually called x96dbg.exe - <@162611465130475520> don't comment on things you know nothing about, have you ever even used x64dbg??

[2024-08-12 17:23] 5pider: ong

[2024-08-12 17:23] 5pider: who does he think he is ? the author ?

[2024-08-12 17:24] expy: hello there, could anyone recommend disassembler and hook library (something like a minhook) for the windows kernel drvier? thanks!

[2024-08-12 17:25] JustMagic: [replying to expy: "hello there, could anyone recommend disassembler a..."]
what are you planning to do

[2024-08-12 17:26] expy: [replying to JustMagic: "what are you planning to do"]
disassembler to find unexported variables, hook library to have inline hooks on the code

[2024-08-12 17:26] JustMagic: inline hooks in kernel?

[2024-08-12 17:27] expy: yup

[2024-08-12 17:27] 5pider: wouldnt this trigger patchguard ?

[2024-08-12 17:27] JustMagic: it would

[2024-08-12 17:27] JustMagic: which is why I'm asking what is he planning to do

[2024-08-12 17:27] expy: yeah, I'm disabling that

[2024-08-12 17:27] 5pider: oh fair

[2024-08-12 17:27] 5pider: i was about to ask if u were to disable it lol

[2024-08-12 17:28] expy: not me, actually, EfiGuard works well in my case 🙂

[2024-08-12 17:28] 5pider: ah right

[2024-08-12 17:28] 5pider: since u are asking about a disassembler why not using zydis?

[2024-08-12 17:29] 5pider: looks like EfiGuard is using it as well

[2024-08-12 17:29] daax: [replying to expy: "disassembler to find unexported variables, hook li..."]
modified zydis, distorm are fine

[2024-08-12 17:29] daax: no need to overcomplicate the decision.

[2024-08-12 17:30] 5pider: yup. looks like zydis is exactly what he needs

[2024-08-12 17:30] expy: zydis would be one of the options, thanks! just asking around before diving into

[2024-08-12 17:31] expy: I guess there is nothing like minhook for kernel, right?

[2024-08-12 17:31] 5pider: i recently saw one that was cross platform hooking lib for UEFI, Kernel and Userland

[2024-08-12 17:31] 5pider: let me see real quick if i can find it agin

[2024-08-12 17:32] 5pider: https://github.com/SamuelTulach/LightHook
[Embed: GitHub - SamuelTulach/LightHook: Single-header, minimalistic, cross...]
Single-header, minimalistic, cross-platform hook library written in pure C - SamuelTulach/LightHook

[2024-08-12 17:32] 5pider: here

[2024-08-12 17:32] 5pider: tho i never tested it ngl

[2024-08-12 17:32] 5pider: but let me know if it working

[2024-08-12 17:33] expy: thanks <@565617276816982046>, I'll have a look!

[2024-08-12 17:33] 5pider: cheers

[2024-08-12 17:33] daax: [replying to expy: "I guess there is nothing like minhook for kernel, ..."]
just write a few utility fncs

[2024-08-12 17:34] 5pider: well i would have personally wrote my own. more control lol

[2024-08-13 02:07] abu: [replying to expy: "I guess there is nothing like minhook for kernel, ..."]
For hooking kernel there's HookLib

[2024-08-13 03:25] expy: I've just tried zydis and LightHook, zydis worked out of the box, although I was thinking you could rebuild instruction back to bytes after disassembly.
LightHook "worked", but it doesn't modify CR4.CET flag, and it doesn't rebase the RIP-relative instructions, leading to an effective crashes if there is one in the first 6 + 8 ! (6 bytes for `jmp qword [next_rip_rel_inst]` and 8 bytes for absolute address) bytes of the original function bytes.
Will have a look at HookLib tmr.
Thanks guys!

[2024-08-13 03:50] expy: Btw are there any tricks to allocate memory as close as possible to the desired module in kernel space?

[2024-08-13 04:29] JustMagic: [replying to expy: "Btw are there any tricks to allocate memory as clo..."]
Very complicated. Better use padding bytes in the module for trampoline

[2024-08-13 09:43] Matti: [replying to expy: "I've just tried zydis and LightHook, zydis worked ..."]
first thing I thought of when I saw
> No dependencies (no full disassembler engine)

[2024-08-13 09:45] Matti: re: zydis: there is an encoder actually

[2024-08-13 09:46] Matti: it's not guaranteed to translate to the same opcodes as the ones you originally decoded (in fact that's impossible), but it works pretty well

[2024-08-13 09:48] Matti: <https://github.com/zyantific/zydis/blob/master/examples/EncodeMov.c>
<https://github.com/zyantific/zydis/blob/master/examples/EncodeFromScratch.c>

[2024-08-13 09:50] Matti: <https://github.com/zyantific/zydis/blob/master/examples/RewriteCode.c> might also be relevant for your use case

[2024-08-13 10:33] 5pider: [replying to Matti: "it's not guaranteed to translate to the same opcod..."]
small question but why would it be impossible? does it depend on the previous opcodes or the context of the instructions? 
didn't knew it would be impossible to translate to the same opcodes from the original decoded instruction

[2024-08-13 10:34] Matti: well a trivial example is an instruction with redundant prefix bytes

[2024-08-13 10:35] Matti: if you decode that, and then re-encode it, zydis will choose the optimal (shortest) form

[2024-08-13 10:35] 5pider: ohhh

[2024-08-13 10:35] 5pider: i get it now

[2024-08-13 10:35] Matti: there are also a few instructions that can be encoded in multiple ways

[2024-08-13 10:36] Matti: xor eax,eax = 33 C0 / 31 C0? I may be misremembering that

[2024-08-13 10:36] Matti: but there are a few instructions like this

[2024-08-13 10:37] 5pider: yeah i get it now. perfect cheers mate thanks for explaining 😄

[2024-08-13 10:59] vendor: [replying to Matti: "well a trivial example is an instruction with redu..."]
this can bite you in the ass if the instruction has a RIP relative operand as well since its relative to next instruction address so you will be off by 1. lost too many hours to this bug.

[2024-08-13 11:00] Matti: yep, very true

[2024-08-13 11:01] Matti: you can also create bogus instructions unintentionally if you have an unexpected spare byte (or two)

[2024-08-13 11:01] Matti: even when padding with nops

[2024-08-13 11:02] Matti: this is pretty rare but I've definitely seen it - wish I could remember the instruction sequence

[2024-08-13 12:36] marc: anyone knows if it's possible to use time travel debugging in cdb?

[2024-08-13 12:37] marc: I have to use cdb with the -server option to make a service get debugged at the start, and then attach with windbg

[2024-08-13 12:38] marc: but ttd doesn't seem to be available from there, and I'm not quite sure if cdb supports it :/

[2024-08-13 13:20] Analyze: Time travel is not possible

[2024-08-13 18:32] expy: [replying to abu: "For hooking kernel there's HookLib"]
HookLib looks good! Thanks

[2024-08-15 06:40] crash: Does anyone know why KeStackAttachProcess works in cases like this? (ModuleList being accessed whilst we're attached to another process) https://github.com/lennyRBLX/readwrite-kernel-stable/blob/9e06343148b15dc1c5d077ff08c91af9cde4177e/driver.c#L102. If we attach to another process which changes our cr3 then we shouldn't be able to access our stack where our local variables are located right?
[Embed: readwrite-kernel-stable/driver.c at 9e06343148b15dc1c5d077ff08c91af...]
a more stable & secure read/write virtual memory for kernel mode drivers - lennyRBLX/readwrite-kernel-stable

[2024-08-15 06:57] vendor: [replying to crash: "Does anyone know why KeStackAttachProcess works in..."]
only the bottom half the PML4Es change between processes. all cr3s have all kernel addresses mapped in. exception is if kva shadowing on.

[2024-08-15 07:06] crash: [replying to vendor: "only the bottom half the PML4Es change between pro..."]
Thanks, makes sense

[2024-08-15 15:14] expy: any good c++ libraries/code snippets which simplifies .pdb downloading and parsing? I known there is x64dbg code, but it's not that neat to reuse

[2024-08-15 15:25] irql: I think dbghelp has some built in stuff

[2024-08-15 15:25] irql: I also have some winhttp copy paste if you want that

[2024-08-15 15:29] Timmy: I only want chocolate paste, but I'd also like to learn about how the downloading works. How are pdbs identified? Is there some universal way that tools like IDA know that a pdb is 'incompatible' with an executable?

[2024-08-15 15:31] irql: there's some data directory

[2024-08-15 15:31] irql: the debug directory

[2024-08-15 15:31] irql: and there's a code view field

[2024-08-15 15:31] irql: I can't remember the exact definitions -- but that structure contains a name, guid & age

[2024-08-15 15:31] irql: and a few other bits to identify the PDB which matches the PE file

[2024-08-15 15:32] irql: IDA uses msdia, and im guessing that can pull the GUID/Age from a pdb file, so ida will probably just compare the two -- and if they dont match, itll complain

[2024-08-15 15:32] irql: might be slightly off about some things, its been a few years fr

[2024-08-15 15:34] 0xatul: [replying to Timmy: "I only want chocolate paste, but I'd also like to ..."]
Have you messed with dia?

[2024-08-15 15:35] irql: [replying to irql: "I also have some winhttp copy paste if you want th..."]
the URL is sym_server/download/symbols/filename/GUID/age/filename

[2024-08-15 15:35] irql: taken from the CODE_VIEW of whatever PE you're looking at

[2024-08-15 15:35] irql: just a GET request with winhttp / curl / whatever you're using

[2024-08-15 15:35] irql: [replying to Timmy: "I only want chocolate paste, but I'd also like to ..."]
oh if you want paste, i can dm it lol

[2024-08-15 15:35] irql: I missed that part

[2024-08-15 15:35] Timmy: damn! thanks

[2024-08-15 15:38] Timmy: [replying to 0xatul: "Have you messed with dia?"]
haven't at all, should I be doing that?

[2024-08-15 15:39] 0xatul: dia is the way to go now tbh

[2024-08-15 15:39] irql: primary pdb parsing lib

[2024-08-15 15:39] irql: ye

[2024-08-15 15:41] irql: [replying to expy: "any good c++ libraries/code snippets which simplif..."]
oh yea, sorry for parsing, probs msdia -- or dbghelp if you only want basic stuff (dbghelp also has downloading)

[2024-08-15 15:43] Timmy: its a com api

[2024-08-15 15:43] Timmy: <:Sadge:996152452539752490>

[2024-08-15 15:44] Timmy: ```cpp
wchar_t wszFilename[ _MAX_PATH ];
mbstowcs( wszFilename, szFilename, sizeof( wszFilename )/sizeof( wszFilename[0] ) );
if ( FAILED( pSource->loadDataFromPdb( wszFilename ) ) )
{
    if ( FAILED( pSource->loadDataForExe( wszFilename, NULL, NULL ) ) )
    {
        Fatal( "loadDataFromPdb/Exe" );
    }
}
```
gj ms, another example that'll break paths longer than 256 characters.

[2024-08-15 15:49] irql: [replying to Timmy: "its a com api"]
<:gdb:992509370908811284>

[2024-08-15 15:49] irql: I think dbghelp is good, as long as you only need basic stuff

[2024-08-15 15:49] irql: thats not COM, but i never tried it tbf

[2024-08-15 16:52] Timmy: ugh I can deal with com somewhat comfortably ever since using the accessibility api a bunch

[2024-08-15 16:52] Timmy: but it's huge copium

[2024-08-16 01:02] crash: [replying to vendor: "only the bottom half the PML4Es change between pro..."]
quick question. Is this functionality specific to KeStackAttachProcess? For example, if i just __writecr3 the other processes cr3 will the addresses still be mapped in?

[2024-08-16 01:07] brew002: [replying to crash: "quick question. Is this functionality specific to ..."]
It depends. When executing with KVAS enabled, then no, your kernel memory shouldn't be mapped in the default user cr3. If KVAS is not enabled, then your kernel memory will be in the default user cr3.

[2024-08-16 11:16] Andre Egington: is there anyone who is happy to help reverse engineer a piece of software  ?

[2024-08-16 13:23] Matti: you might wanna say what it is that you want help with, or better yet just ask a question about said thing ('does anyone want to help me' doesn't count)

[2024-08-17 11:13] crash: Anyone here know how to get the register view  (FPU panel?) back into x64dbg? It seems to have disappeared for me...
[Attachments: image.png]

[2024-08-17 11:14] mrexodia: [replying to crash: "Anyone here know how to get the register view  (FP..."]
drag it back from the side

[2024-08-17 11:14] mrexodia: 
[Attachments: image.png]

[2024-08-17 11:14] crash: Thanks lol

[2024-08-17 15:15] Torph: Has anyone here used Cutter? One of my friends showed it to me and I'm curious how it compares to Ghidra

[2024-08-17 15:16] Torph: since they're re-using the Ghidra decompiler, is it just like a Ghidra clone but native?

[2024-08-17 15:18] 0xatul: its a wrapper around rizin (which is a fork of r2?)

[2024-08-17 15:37] Torph: yeah I saw, but I've never used rizin or r2 so idk what to expect

[2024-08-17 15:41] Redhpm: [replying to Torph: "Has anyone here used Cutter? One of my friends sho..."]
the ui is cool, but it does not allow as much as the java ui

[2024-08-17 21:02] jvoisin: [replying to 0xatul: "its a wrapper around rizin (which is a fork of r2?..."]
Yup, fork of r2

[2024-08-18 07:46] Deleted User: Is maldev academy good for beginners learning RE and mal analysis

[2024-08-18 09:14] 5pider: uhh not their goal so not really lol

[2024-08-18 12:49] szczcur: [replying to Deleted User: "Is maldev academy good for beginners learning RE a..."]
the book malware analysis is good for beginners. maldev academy is for when youre too lazy to find the sources they aggregated on github.

[2024-08-18 15:01] Loading: hey, in app that im reversing i found ShellExecuteW that is supposed to open links but input can be changed so it would open cmd.exe notepad.exe etc.. for users, i was wondering if its possible to somehow execute cmd.exe + some commands so it would be worth reporting

[2024-08-18 15:32] rickoooooo: [replying to Loading: "hey, in app that im reversing i found ShellExecute..."]
https://learn.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shellexecutea

https://learn.microsoft.com/en-us/windows-server/administration/windows-commands/cmd
[Embed: ShellExecuteA function (shellapi.h) - Win32 apps]
Performs an operation on a specified file. (ShellExecuteA)
[Embed: cmd]
Reference article for the cmd command, which starts a new instance of the command interpreter, Cmd.exe.

[2024-08-18 16:13] szczcur: [replying to Loading: "hey, in app that im reversing i found ShellExecute..."]
does this program run at elevated il? if not, it’s really a non-issue. if yes, what mitigations are enabled? dep/stack cookies/etc. are these links fetched from an external resource? if no, have you found any sort of exploit primitive?

[2024-08-18 16:25] szczcur: there’s a number of ways you could approach it. if it’s running at high il, and there is no other means of achieving code execution through a registry mods/side load/acl misconfig/fs race cond or the external resource control (which might fall under acl misconfig) then youre going to have to find something in the bin to break in order to chain and modify the links and get shellexecutew to open what you want. however im willing to bet if theyre using shellexecutew that there are plenty of other bugs to leverage to perform lpe.

[2024-08-18 19:04] Loading: it does run with admin privileges, i can upload my own string to server db for lpFile in ShellExecuteW but there is no way to control lpParameters, its set to null on client, so only things i can do is download files from websites or run cmd with admin privileges on other peoples computers when other people click on my "link"

[2024-08-18 19:05] Loading: it might be impossible so i might just move to other stuff, i was just trying to find my first exploit and i thought this could be exploited

[2024-08-18 19:07] diversenok: You can try running a batch file or some other script

[2024-08-18 19:08] diversenok: But it works better if you can create files or at least convince the user to do so

[2024-08-18 19:09] diversenok: Oh, wait, it's easy

[2024-08-18 19:09] diversenok: Pass a filename on a public WebDav file share

[2024-08-18 19:10] Loading: i was thinking about smb but i dont know much about it and i dont know if i can create smb that is accessible for everyone

[2024-08-18 19:10] Loading: but it worked when i made one on localhost

[2024-08-18 19:10] Loading: i will try webdav

[2024-08-18 19:10] diversenok: Try `\\live.sysinternals.com@SSL\tools\procexp64.exe`

[2024-08-18 19:12] Loading: lol it worked

[2024-08-18 19:12] elias: LOL

[2024-08-18 19:12] elias: thats awesome

[2024-08-18 19:13] Loading: but i need to click on start for it to execute

[2024-08-18 19:13] Loading: with smb i didnt had to do that

[2024-08-18 19:14] Loading: im still happy that i my first "exploit" if it can be called that haha