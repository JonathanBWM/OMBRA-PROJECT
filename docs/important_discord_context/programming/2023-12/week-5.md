# December 2023 - Week 5
# Channel: #programming
# Messages: 716

[2023-12-25 08:37] luci4: [replying to WhiteBerry: "Will this cover the mathematics?"]
Somewhat, yes. If you want to learn math I would recommend checking out the math modules of Cryptohack. Or just grab a Number Theory book

[2023-12-25 22:12] North: has anyone tried CLion Nova yet? and if so are you liking it / do you notice a difference?

[2023-12-25 22:34] qwerty1423: hey, i tried to write a simple disassembler, it identifies and records every opcode just fine, i want it to write the operands and arguments of each instruction infront of it aswell but idk how, can anyone help?

[2023-12-25 22:35] qwerty1423: (shit idea) but i wrote it in python for some reason

[2023-12-25 22:35] qwerty1423: 
[Attachments: image.png]

[2023-12-25 22:37] contificate: looks a bit like you're just testing each byte and trying to match it against an opcode

[2023-12-25 22:37] contificate: in reality, the encoding scheme can be more complex

[2023-12-25 22:37] contificate: you'll find that RISC architectures typically have a much simpler encoding scheme (I guess AArch64 is a bit involved, compared to ARMv7)

[2023-12-25 22:37] contificate: because each instruction is typically fixed width (ignoring thumb or compressed modes in ARM, RISC-V, etc.)

[2023-12-25 22:38] contificate: whereas the encoding scheme for x86(_64) is more of a pain

[2023-12-25 22:39] contificate: 
[Attachments: 2023-12-25-223911_673x558_scrot.png]

[2023-12-25 22:39] contificate: this is what I mean 0- this is ARMv7, where a bunch of mask patterns followed by bitwise extraction is basically all you need

[2023-12-26 02:26] vendor: [replying to North: "has anyone tried CLion Nova yet? and if so are you..."]
ooo didn’t know this was released. i tried clion before and liked it but the linux version was crashing for me so ditched it in the end. i really like their IDEs so i want to believe in it.

[2023-12-26 05:02] Torph: i haven't heard of Nova, but i use CLion on Linux and it works great

[2023-12-26 05:04] Terry: Intelisense* is just too shit, I switched to CLion just because of that. Msvcs integration with cmake is also not as nice as CLions IMO

[2023-12-26 09:20] vendor: i use resharper c++ for vs

[2023-12-26 11:44] qwerty1423: [replying to contificate: ""]
this was something i couldn't figure out, how should i know which byte is an opcode so i can identify it and expect it's operands in it's next 4 bytes (just an example)
since many instructions have different total length it was a little challenging for me to understand how its done, let alone making a parser who does it for me.

[2023-12-26 11:45] contificate: in that example, there's masks you check

[2023-12-26 11:46] contificate: i.e. the hardcoded `0`s and `1`s in each row

[2023-12-26 11:46] contificate: in other architectures, there's just a fixed opcode field that's in the same place with every instruction type

[2023-12-26 11:46] contificate: but the true functionality may depend on other fields

[2023-12-26 11:46] contificate: but x86_64 is more complex than that example

[2023-12-26 11:47] contificate: it used to be a common task to write a "length disassembler"

[2023-12-26 11:47] contificate: which only determines the boundaries of common instructions

[2023-12-26 11:47] contificate: such that you can automatically place a trampoline hook and copy the correct number of bytes to the end of the hook

[2023-12-26 11:53] qwerty1423: [replying to contificate: "such that you can automatically place a trampoline..."]
ok but its written in python, idk if its possible or not

[2023-12-26 11:53] qwerty1423: are there any websites you would recommend in order to learn it?

[2023-12-26 11:53] contificate: it is definitely possible

[2023-12-26 11:54] contificate: http://www.c-jump.com/CIS77/CPU/x86/lecture.html

[2023-12-26 11:54] contificate: I never fully learned this nonsense so perhaps someone else has a better resource

[2023-12-26 11:55] qwerty1423: i'll wait for more resources, thanks

[2023-12-26 16:57] daax: <@491503554528542723> why you deleting stuff that's probably useful to others in your discussion?

[2023-12-26 17:17] Horsie: [replying to daax: "<@491503554528542723> why you deleting stuff that'..."]
I remember asking something then realizing that the question was sorta wrong itself/something I was doing wrong trivially

[2023-12-26 17:18] Horsie: Can you remind me the topic again?

[2023-12-26 17:18] Torph: [replying to Terry: "Intelisense* is just too shit, I switched to CLion..."]
agreed. I've been moving away from CMake a little bit because it's more steps on CLI than Make, but I can't argue with the portability. I might look into Zig as a build system, then I could also make projects with linker scripts more portable

[2023-12-26 20:41] contificate: lmao @ https://github.com/tree-sitter/tree-sitter-ocaml/blob/master/ocaml/src/parser.c 32MB generated C encoding the LR(1) tables for OCaml

[2023-12-26 21:39] qwerty1423: [replying to contificate: "it is definitely possible"]
ok take a look at this
[Attachments: image.png]

[2023-12-26 21:40] contificate: would be interested to know that many industrial impls of such things are largely table driven

[2023-12-26 21:40] contificate: wish someone who cared about x86_64's encoding would post

[2023-12-26 21:41] contificate: as I really don't care for it and am unfamiliar with it, beyond knowing a few opcodes

[2023-12-26 21:41] qwerty1423: nice

[2023-12-26 21:42] contificate: but yeah the general point is that

[2023-12-26 21:43] contificate: you can't disassemble all of x86_64 for looking for a leading byte that corresponds to an opcode

[2023-12-26 21:43] contificate: can't even do that for simpler architectures

[2023-12-26 21:45] qwerty1423: its just x86

[2023-12-26 21:46] Deleted User: [replying to contificate: "it used to be a common task to write a "length dis..."]
hde?

[2023-12-26 21:46] Deleted User: HDE is outdated as heck tho

[2023-12-26 21:46] Deleted User: i mean you can also just use a more complex disassembler, but i think thats the smallest one i can think of

[2023-12-26 21:46] Deleted User: and outdated yeah

[2023-12-26 21:46] Deleted User: I mean it will break on newer instructios

[2023-12-26 21:46] Deleted User: yea

[2023-12-26 21:46] Deleted User: im using icedx86, i think for c++ its zydis

[2023-12-26 21:46] Deleted User: its fast as fk tho

[2023-12-26 21:46] Deleted User: I use bddisasm

[2023-12-26 21:46] Deleted User: idk if zydis has as much info as iced, but it should have length

[2023-12-26 21:46] Deleted User: zydis is too huge

[2023-12-26 21:47] Deleted User: iced too ya

[2023-12-26 21:47] Deleted User: but its fast as fck :3c

[2023-12-26 21:47] Deleted User: https://media.discordapp.net/attachments/1188543806480588982/1189275199863005294/chirpani_ghost_file_140530842.gif?ex=659d91d7&is=658b1cd7&hm=3a731d27344298d3e149e77d3f5457edbba589e9495e15612d4cdf1ef183ce01&

[2023-12-26 21:49] Deleted User: [replying to Deleted User: "I use bddisasm"]
i really dislike that libraries like that are just one large file lmao

[2023-12-26 21:49] Deleted User: but it has readable code yay :3

[2023-12-26 21:49] Deleted User: [replying to Deleted User: "i really dislike that libraries like that are just..."]
Ye

[2023-12-26 21:49] Deleted User: meanwhile iced has like 40k line files with autogenerated traits so not much better

[2023-12-26 21:50] Deleted User: I mean if it works

[2023-12-26 21:50] Deleted User: 🤷

[2023-12-26 21:50] Deleted User: I mean max to max

[2023-12-26 21:50] Deleted User: its just 1mb extra

[2023-12-26 21:50] Deleted User: yea it helps compile time safety ig and so performance maybe too

[2023-12-26 21:50] Deleted User: although i mostly used it for assembling instead of disassembling

[2023-12-26 21:50] Deleted User: I mean

[2023-12-26 21:51] Deleted User: I use it for disassembling and emulating

[2023-12-26 21:51] Deleted User: thats ab it

[2023-12-26 21:51] Deleted User: :D

[2023-12-28 14:31] qwerty1423: 
[Attachments: image.png]

[2023-12-28 14:31] qwerty1423: what does this line of code do? i can provide more parts of the code if needed

[2023-12-28 14:40] Deleted User: create a regex, but what exactly thats for no clue

[2023-12-28 14:40] Deleted User: maybe chatgpt can explain, regexes is something its decent at

[2023-12-28 19:55] daax: [replying to qwerty1423: "what does this line of code do? i can provide more..."]
compiles a regexp that looks for a string following that pattern -- the site tells ya

[2023-12-28 19:56] daax: maybe for `<username>: <something>`

[2023-12-28 19:56] daax: not certain <:PES2_Shrug:513352546341879808>

[2023-12-28 19:57] daax: this breaks all the time on goofy stuff though, it's a great tool, just frustrating occasionally

[2023-12-28 20:11] brymko: weird regex

[2023-12-28 20:26] dullard: [replying to qwerty1423: "what does this line of code do? i can provide more..."]
can you paste it please

[2023-12-28 21:31] qwerty1423: yea its re

[2023-12-28 21:55] qwerty1423: [replying to dullard: "can you paste it please"]
i only have this block of code available right now

```def ext_ffo(fid):
    pattern = re.compile(r"^\s*[\w@]+:(?![\w@])", re.MULTILINE)
    ffo = [match.group(0) for match in re.finditer(pattern, fid)]
    return ffo```

[2023-12-28 21:56] qwerty1423: re module is used*

[2023-12-28 22:16] daax: what? It only matches things with alphanumeric names followed by colon

[2023-12-28 22:16] daax: Y’all just need to run it in regex101

[2023-12-28 22:17] daax: it will match it if has alphanumeric and/or @ followed by :

[2023-12-28 22:17] daax: Maybe python or chat convos <handle@channel>:

[2023-12-28 22:17] daax: idk

[2023-12-28 22:18] qwerty1423: since its trying to find(?) something related to C++ in a text with the given pattern

[2023-12-28 22:19] daax: without more context it could be anything, unless you’re saying it does search cpp source files or something

[2023-12-28 22:24] qwerty1423: [replying to daax: "without more context it could be anything, unless ..."]
this code is written by one of my classmates which i can't contact rn, it's job is to take an assembly code and find the user-defined functions  (C++ and C),  it looked weird enough at first because the rest of the code isn't related to the "finding" process and only this block of code (and a little more) was defined to do the job.

[2023-12-28 22:27] qwerty1423: weird because as far as i know, there are no patterns in assembly code to find user defined functions with

[2023-12-28 22:27] qwerty1423: but this code works in its own way somehow, so i wanted to know how this part works,

[2023-12-28 22:36] qwerty1423: yea that makes sense, thanks for help anyways

[2023-12-29 20:48] Googloqe: does anyone know how to make NtRaiseHardError to work on Window 11 with 'OptionOkNoWait' option? It used to show a message box, but now it shows a buggy notification that is not popping up from time to time.

[2023-12-30 00:50] Googloqe: [replying to Googloqe: "does anyone know how to make NtRaiseHardError to w..."]
for anyone wondering i come up with creating a dummy process and injecting my shellcode into it just to replicate old NtRaiseHardError behaviour...

[2023-12-30 02:45] Matti: [replying to Googloqe: "does anyone know how to make NtRaiseHardError to w..."]
what do you need `OptionOkNoWait` for?

[2023-12-30 02:46] Matti: I can tell you how to do this (probably...) but it's a stupid amount of work

[2023-12-30 02:46] Matti: that particular option is far more restrictive regarding who and especially when you're allowed to use it

[2023-12-30 02:47] Matti: it requires `ExReadyForErrors` to be 1, which is not trivial to accomplish

[2023-12-30 02:47] Matti: `OptionOk` on the other hand works fine for me
[Attachments: image.png]

[2023-12-30 03:16] Matti: I'm actually really curious what you are using `OptionOkNoWait` for that `OptionOk` can't do

[2023-12-30 03:18] Matti: because as far as I'm aware `OptionOkNoWait` is only intended for the kernel's own internal usage, i.e. when it knows that the system hard error port has been created and is ready to receive messages

[2023-12-30 03:19] Matti: if CSRSS isn't accepting error messages right at the exact moment you call the API with this option, the kernel just does nothing and returns STATUS_UNSUCCESSFUL

[2023-12-30 03:20] Matti: whereas all that calling with `OptionOk` does is simply ensure that there is a connection to CSRSS available so that the message box can actually be shown

[2023-12-30 03:24] Matti: this goes for every single response option except for `OptionOkNoWait`, by the way

[2023-12-30 03:25] Matti: I'm pretty sure that if you're passing this and you are **not** the kernel talking to CSRSS.... you're doing something wrong

[2023-12-30 04:33] Deleted User: @5680

[2023-12-30 05:57] Googloqe: [replying to Matti: "what do you need `OptionOkNoWait` for?"]
i want a process to be terminated and error message to stay. well, if `OptionOkNoWait` is indeed so untrustworthy, then I came up with right solution.

[2023-12-30 06:08] Matti: that sounds obnoxious as fuck, are you an AV vendor maybe?

[2023-12-30 06:08] Matti: but I do agree

[2023-12-30 06:09] Matti: if your new implementation kills the process from a newly created process anyway, you may as well just `MessageBoxA`

[2023-12-30 06:10] Matti: but I suspect your original implementation would have also worked the same as before if you'd used `OptionOk` instead

[2023-12-30 06:11] Matti: that's just my educated guess without having seen the code... and I don't think I want to necessarily

[2023-12-30 10:04] Googloqe: [replying to Matti: "but I suspect your original implementation would h..."]
OptionOk suspends thread execution, which prevents me from leading a program to its termination, but even then if i were to use threads a message box would just disappear at exit. 
i think what i'm trying to achieve is a common technique, because iirc even EasyAntiCheat and VMProtect runtime use it.

btw, do i still need to load user32.dll in the dummy process, even if I only use NtRaiseHardError with OptionOk, right?

[2023-12-30 10:09] Matti: it "suspends thread execution" as much as MessageBoxA does

[2023-12-30 10:09] Matti: that is to say, of course the thread will be stuck waiting for the user to acknowledge the message

[2023-12-30 10:09] Matti: that's not suspended by the way

[2023-12-30 10:10] Matti: > btw, do i still need to load user32.dll in the dummy process, even if I only use NtRaiseHardError with OptionOk, right? 
this has never been needed for NtRaiseHardError

[2023-12-30 10:10] skinnym_: [replying to Googloqe: "OptionOk suspends thread execution, which prevents..."]
didnt find it n vmprotect

[2023-12-30 10:11] skinnym_: nether OptionOk nor OptionOkNoWait

[2023-12-30 10:11] Googloqe: [replying to Matti: "it "suspends thread execution" as much as MessageB..."]
doesn't MessageBoxA internally use NtRaiseHardError?

[2023-12-30 10:11] Matti: no....

[2023-12-30 10:11] Googloqe: [replying to skinnym_: "didnt find it n vmprotect"]
bp NtRaiseHardError

[2023-12-30 10:11] Matti: those are entirely different functions

[2023-12-30 10:12] Matti: NtRaiseHardError will go to the exception port, debug port or error port (in that order, if any are present - 99% of the time this means it goes to csrss.exe)

[2023-12-30 10:12] Matti: your process will be requesting a message box from csrss via ALPC

[2023-12-30 10:13] Matti: messageboxA just calls some trash win32k API that does god knows what

[2023-12-30 10:13] Matti: the end result for the user is the same though

[2023-12-30 10:13] Matti: but, you don't need user32.dll for ALPC

[2023-12-30 10:14] Matti: 
[Attachments: image.png]

[2023-12-30 10:14] Matti: 
[Attachments: image.png]

[2023-12-30 10:14] Matti: see how it is doing ALPC

[2023-12-30 10:15] Matti: and how ExpRaiseHardError is the internal kernel function responsible for this (you can also call **Ex**RaiseHardError in kernel mode because of this)

[2023-12-30 10:16] skinnym_: so this probably has nothing to do with all the aspects involved with OptionOkNoWait?

[2023-12-30 10:17] Matti: wdym

[2023-12-30 10:17] Matti: I explained this above

[2023-12-30 10:17] Matti: the kernel itself uses this option when talking to CSRSS

[2023-12-30 10:17] skinnym_: i was just saying

[2023-12-30 10:17] skinnym_: that either it is something deep or nothing

[2023-12-30 10:17] skinnym_: and he isnt responding

[2023-12-30 10:18] skinnym_: so i dont know

[2023-12-30 10:19] Matti: you can simply check the places where ExpRaiseHardError is called internally in the kernel, with this argument

[2023-12-30 10:19] Matti: it is always after a check for the hard error port availability

[2023-12-30 10:19] Googloqe: [replying to Matti: "messageboxA just calls some trash win32k API that ..."]
well, does it mean that this blog misleads? https://vollragm.github.io/posts/kernel-message-box/#messageboxw
[Embed: Showing a MessageBox from kernel-mode]
Introduction Message boxes provide a simple way to show feedback to the user. In user-mode, a message box can be shown with the MessageBoxW API function. However, this API does not exist in kernel-mod

[2023-12-30 10:19] Matti: and never related to the original user input

[2023-12-30 10:20] Matti: [replying to Googloqe: "well, does it mean that this blog misleads? https:..."]
I don't think so? all of this is correct as far as I know

[2023-12-30 10:21] Matti: "a message box" is not the same as "MessageBoxA"

[2023-12-30 10:22] Matti: in fact, this stacktrace is more informative than the last one
[Attachments: image.png]

[2023-12-30 10:22] Matti: this is csrss.exe

[2023-12-30 10:22] Matti: not my exe

[2023-12-30 10:23] Matti: and csrss has a user32.dll loaded

[2023-12-30 10:31] Matti: [replying to Matti: "I don't think so? all of this is correct as far as..."]
well minus the "it goes to the exception port" part - that is a bit more complicated

[2023-12-30 10:31] Matti: with 3 potential ports used

[2023-12-30 10:31] Matti: but I explained this above

[2023-12-30 10:32] Matti: it's only a minor detail in most cases, since most processes will only have the error port

[2023-12-30 10:33] Matti: but a process being debugged will have an exception port, which is why the debugger gets the firsst chance notification

[2023-12-30 10:47] skinnym_: can you give a process an exception po rt without a debugger attached?

[2023-12-30 10:49] Matti: yes

[2023-12-30 10:49] Matti: pass one to `NtCreateProcess`

[2023-12-30 10:50] Matti: `NtCreateUserProcess` only has an attribute for the debug port, as far as I know

[2023-12-30 10:50] Matti: `NtCreateProcess[Ex]` takes both

[2023-12-30 12:08] diversenok: [replying to Matti: "pass one to `NtCreateProcess`"]
Not anymore; at some point they replaced the exception port parameter with the token handle parameter

[2023-12-30 12:08] diversenok: Same for `NtCreateProcessEx`

[2023-12-30 12:17] diversenok: I think it happened either in Vista or 7

[2023-12-30 12:22] diversenok: So probably the only remaining way is via `NtSetInformationProcess` with `ProcessExceptionPort`, which requires SeTcbPrivilege

[2023-12-30 12:26] diversenok: What is the error port, by the way? I never looked into that

[2023-12-30 12:27] diversenok: Is it the one set via `NtSetDefaultHardErrorPort`?

[2023-12-30 12:32] vendor: chatgpt is hopeless with avx

[2023-12-30 12:34] vendor: it was gaslighting me into thinking this function was correct 
```c++
__forceinline bool is_page_all_zeros(const u64 page)
{
    const __m256i zero_vector = _mm256_setzero_si256();
    const auto* page_ptr = reinterpret_cast<__m256i*>(page);

    for (auto i = 0ul; i < 4096ul / sizeof(__m256i); i++)
    {
        if (!_mm256_testz_si256(page_ptr[i], zero_vector))
        {
            return false;
        }
    }

    return true;
}```

[2023-12-30 12:34] vendor: i even explicitly asked it what testz does and if it would actually work passing zero since it's AND and it still denied it

[2023-12-30 12:58] JustMagic: [replying to vendor: "chatgpt is hopeless with avx"]
clang generates pretty good code if you just use its vector extensions

[2023-12-30 13:32] vendor: [replying to JustMagic: "clang generates pretty good code if you just use i..."]
i'm an msvc loser for now

[2023-12-30 16:54] Brit: L bozo etc

[2023-12-30 18:49] Matti: [replying to diversenok: "Not anymore; at some point they replaced the excep..."]
you're right actually, I even knew this and I still forgot

[2023-12-30 18:50] Matti: because my headers are using the old name and I'm torn on which version would be more correct <:lillullmoa:475778601141403648>

[2023-12-30 18:50] Matti: I should probably update them, but OTOH XP and below are really the only versions of Windows you'd ever want to call this API on to begin with

[2023-12-30 18:51] Matti: apart from some niche uses like creating a process with 0 threads

[2023-12-30 18:51] Matti: [replying to diversenok: "Is it the one set via `NtSetDefaultHardErrorPort`?"]
yes, that's the one

[2023-12-30 18:53] Matti: this took me a while to find (again) as well, because I could swear this port used to be a global named object in the past... but either it's not anymore or I was just mistaken and thinking of some other port

[2023-12-30 18:54] Matti: but it's always been communicated to the kernel via this syscall, which then sets it *once* and refuses to change it if someone calls the API again

[2023-12-30 18:57] Matti: could be funny to try and replace CSRSS as the owner of this port by registering yourself as one of those boot-time startup EXEs like checkdisk <:hmm:475800667177484308>

[2023-12-30 19:25] diversenok: [replying to Matti: "I should probably update them, but OTOH XP and bel..."]
Yeah, if you want to start a process purely in native API without any tampering, `NtCreateUserProcess` is a much more programmer- and antivirus-friendly option

[2023-12-30 19:28] diversenok: You still need to do all the CSR/SxS registration stuff but at least delivering process parameters and creating the initial thread are not your concerns anymore

[2023-12-30 19:28] diversenok: Also not sure if `NtCreateProcessEx` is capable of creating WoW64 processes

[2023-12-30 19:48] Matti: sure, it is

[2023-12-30 19:48] Matti: after all XP/2003 x64 had wow64 but no NtCreateUserProcess

[2023-12-30 19:49] Matti: I agree that `NtCreateUserProcess` >>> `NtCreateProcessEx`

[2023-12-30 19:49] Matti: but the real pain on XP IMO is `NtCreateThread`

[2023-12-30 19:49] Matti: and not having `NtCreateThreadEx`

[2023-12-30 19:51] Matti: both of those were such a PITA to use that the vista replacements were actually the first syscalls I added to matti WRK

[2023-12-30 19:52] Matti: even though no one benefits from this other than myself <:kekw:904522300257345566>

[2023-12-30 19:52] Matti: because you need to manually syscall them

[2023-12-30 19:54] Matti: [replying to Matti: "but the real pain on XP IMO is `NtCreateThread`"]
reason for this is that in the non-Ex version, user mode (so normally kernel32) was responsible for setting up the initial thread stack and context

[2023-12-30 19:54] Matti: like fuck off

[2023-12-30 19:55] Matti: that's a huge amount of code that belongs in the kernel that kernel32.dll was doing in user mode instead

[2023-12-30 19:55] Matti: and it's not trivial

[2023-12-30 20:38] Matti: [replying to Matti: "this took me a while to find (again) as well, beca..."]
seems there used to be an `\ErrorLogPort`, so I wasn't entirely making things up
[Attachments: image.png]

[2023-12-30 20:39] Matti: but it's owned by services.exe, so it's definitely not the same thing

[2023-12-30 20:46] diversenok: [replying to Matti: "after all XP/2003 x64 had wow64 but no NtCreateUse..."]
Hmm, trying to create a process from a 32-bit section via `NtCreateProcessEx` gives a non-WoW64 process without the 32-bit PEB

[2023-12-30 20:46] diversenok: Was it also the same on XP? Am I supposed to do something else to make it WoW64?

[2023-12-30 20:47] Deleted User: <@148095953742725120> Question; why do you use win xp in 2023

[2023-12-30 20:47] Matti: [replying to Deleted User: "<@148095953742725120> Question; why do you use win..."]
to answer this question obviously

[2023-12-30 20:47] Matti: I don't *use* XP

[2023-12-30 20:47] Matti: [replying to diversenok: "Was it also the same on XP? Am I supposed to do so..."]
I don't know actually, lemme check

[2023-12-30 20:52] Matti: [replying to diversenok: "Was it also the same on XP? Am I supposed to do so..."]
the specific check seems to be in `MmInitializeProcessAddressSpace`, and it's based on the machine type from the image header

[2023-12-30 20:52] Matti: so, if it's `IMAGE_FILE_MACHINE_I386`

[2023-12-30 20:53] Matti: if so, the EPROCESS gets its `Wow64Info` initialized, which is in the end the thing that decides whether a process is wow64 or not

[2023-12-30 20:55] Matti: there's `MiInitializeWowPeb` in which the 32 bit PEB gets created shortly after this, but again only if the wow64info was initialized

[2023-12-30 20:55] dullard: [replying to Deleted User: "<@148095953742725120> Question; why do you use win..."]
I guess its just a tinkering OS

[2023-12-30 20:56] Matti: yeah, I have it so I have an environment to test my kernel in

[2023-12-30 20:56] Matti: the kernel can't do a whole lot by itself

[2023-12-30 21:23] Matti: hmm <@503274729894051901> how are you calling `NtCreateProcessEx`? like, show the code

[2023-12-30 21:24] diversenok: It's `NtOpenFile` + `NtCreateSection` + `NtCreateProcessEx`

[2023-12-30 21:24] diversenok: Let me find my sample C code for it; I tested it in Delphi

[2023-12-30 21:24] Matti: looking at this reactOS code there seems to be some hacky shit going on with where the PEB gets returned <:harold:704245193016344596>
```c
    /* Save the current TIB value since kernel overwrites it to store PEB */
    TibValue = Teb->NtTib.ArbitraryUserPointer;

    /* Tell the kernel to create the process */
    Status = NtCreateProcessEx(&ProcessHandle,
                               PROCESS_ALL_ACCESS,
                               ObjectAttributes,
                               NtCurrentProcess(),
                               Flags,
                               SectionHandle,
                               DebugHandle,
                               NULL,
                               InJob);

    /* Load the PEB address from the hacky location where the kernel stores it */
    RemotePeb = Teb->NtTib.ArbitraryUserPointer;

    /* And restore the old TIB value */
    Teb->NtTib.ArbitraryUserPointer = TibValue;
```

[2023-12-30 21:24] Matti: also

[2023-12-30 21:25] Matti: the parent process should be NtCurrentProcess()

[2023-12-30 21:25] Matti: I guess you're doing that already

[2023-12-30 21:25] Matti: but it seems to matter, reading the kernel code

[2023-12-30 21:25] Matti: some other process is probably also fine assuming you've got PROCESS_CREATE_PROCESS access

[2023-12-30 21:25] Matti: so long as it's not null

[2023-12-30 21:26] diversenok: What is this magic... why

[2023-12-30 21:26] Matti: ikr

[2023-12-30 21:26] diversenok: Do they want to know PEB address without querying basic information or what

[2023-12-30 21:27] Matti: well the real kernel32.dll in NT does this too

[2023-12-30 21:27] Matti: but I guess so?

[2023-12-30 21:27] Matti: honestly no idea

[2023-12-30 21:27] Matti: 
[Attachments: image.png]

[2023-12-30 21:29] Matti: later on your process creation will probably still fail if you try to message CSRSS about it, unless you pass the correct magic wow64 message

[2023-12-30 21:29] Matti: but NtCreateProcessEx should succeed

[2023-12-30 21:31] diversenok: [replying to Matti: "later on your process creation will probably still..."]
What do you mean by that? `CsrClientCallServer` with `BasepCreateProcess`?

[2023-12-30 21:31] Matti: yeah

[2023-12-30 21:32] Matti: it might be something as simple as setting the wow64 peb

[2023-12-30 21:32] diversenok: Nah, CSRSS doesn't really care, it maps the activation context data regardless

[2023-12-30 21:32] Matti: it's just that it's unimplemented in reactos, so there might be more to it than that

[2023-12-30 21:32] Matti: hmmmm

[2023-12-30 21:32] Matti: interesting

[2023-12-30 21:32] Matti: in ROS they really fail the process create entirely

[2023-12-30 21:32] Matti: on purpose

[2023-12-30 21:33] Matti: if it's wow64

[2023-12-30 21:33] Matti: even though NtCreateProcessEx succeeded

[2023-12-30 21:33] Matti: (well, that's a big assumption... it is reactos after all)

[2023-12-30 21:34] diversenok: This is the closest thing I have in C https://gist.github.com/diversenok/0baca1373baee8b6d7cbdbdba12e7892

[2023-12-30 21:35] diversenok: It doesn't notify CSR though and other things

[2023-12-30 21:35] Matti: sure that's fine

[2023-12-30 21:35] Matti: lemme see if I can reproduce

[2023-12-30 21:35] diversenok: If you want to test a complete version, I have this: https://github.com/diversenok/TokenUniverse

[2023-12-30 21:35] diversenok: This one does CSR registration and stuff

[2023-12-30 21:35] Matti: oh this is more code than I expected <:lillullmoa:475778601141403648>

[2023-12-30 21:36] Matti: I'll trim it down a bit to just NtCreateProcessEx

[2023-12-30 21:36] Matti: since that's what's failing

[2023-12-30 21:36] diversenok: Wait, I noticed something

[2023-12-30 21:37] diversenok: `NtCreateProcessEx` fails with `STATUS_INVALID_IMAGE_FORMAT` on executables from SysWoW64

[2023-12-30 21:37] diversenok: It works on 3-rd party files though

[2023-12-30 21:37] Matti: lol

[2023-12-30 21:37] Matti: I wonder what funny bug that is

[2023-12-30 21:38] Matti: just to clarify
`PCWSTR fileName = argc >= 2 ? argv[1] : L"C:\\Windows\\System32\\cmd.exe";`
you are passing syswow64\cmd.exe here instead right

[2023-12-30 21:38] diversenok: I think it's deliberate based on something in the header

[2023-12-30 21:38] diversenok: I play with TokenUniverse for now; C code is just a reference

[2023-12-30 21:39] Matti: kk, well give me some time to get something compilable that may or may not run

[2023-12-30 21:41] diversenok: Effectively, everything works fine for 64-bit executables for me; trying to use a 32-bit file creates a process that still has one PEB and CSR registration succeeds

[2023-12-30 21:42] Matti: hmm ok

[2023-12-30 21:42] Matti: I'll query the wow64 section after making the process then

[2023-12-30 21:42] Matti: or the PEB rather

[2023-12-30 21:42] Matti: whatever that info class is called

[2023-12-30 21:43] diversenok: Querying `ProcessWow64Information` returns NULL

[2023-12-30 21:43] Matti: yeah that one

[2023-12-30 21:43] Matti: alright

[2023-12-30 21:45] diversenok: At the same time, querying `ProcessImageInformation` returns `SECTION_IMAGE_INFORMATION` with `Machine` set to `IMAGE_FILE_MACHINE_I386`

[2023-12-30 21:53] diversenok: Ah, yes `I386 (64-bit)` 😅
[Attachments: image.png]

[2023-12-30 21:54] diversenok: PH checks the same process info classes

[2023-12-30 21:59] Matti: lmao

[2023-12-30 21:59] Matti: well I can reproduce on win 11

[2023-12-30 22:00] Matti: I used some of my own code I had lying around since that ended up being easier.... but this does essentially the same thing as yours

[2023-12-30 22:00] Matti: 
[Attachments: image.png]

[2023-12-30 22:02] Matti: and also getting this yeah <:lillullmoa:475778601141403648>
[Attachments: image.png]

[2023-12-30 22:03] Matti: [replying to diversenok: "`NtCreateProcessEx` fails with `STATUS_INVALID_IMA..."]
so this I can't reproduce

[2023-12-30 22:03] Matti: is this on XP or 11?

[2023-12-30 22:03] Matti: I have yet to try XP

[2023-12-30 22:04] diversenok: I tried everything on 10

[2023-12-30 22:04] Matti: hm ok, well lemme try but I doubt that will make a difference

[2023-12-30 22:04] Matti: 10 vs 11 I mean

[2023-12-30 22:05] diversenok: Let me also try it in a clean VM with minimal code, just in case it's some driver doing responding to this in a weird way

[2023-12-30 22:05] Matti: which exe did you try specifically? cmd.exe?

[2023-12-30 22:05] diversenok: Yes

[2023-12-30 22:06] Matti: no, seems to work the same for me on 10

[2023-12-30 22:06] diversenok: Sandboxie driver, for instance, prevents process creation from deleted files; although, at the stage of initial thread creation, of course

[2023-12-30 22:07] diversenok: So not sure what happens

[2023-12-30 22:07] Matti: let me double check there isn't some subtle difference in our code

[2023-12-30 22:07] Matti: but I don't think there is

[2023-12-30 22:13] diversenok: No, still the same effect in a VM: `STATUS_INVALID_IMAGE_FORMAT` in `NtCreateProcessEx` on syswow64 cmd.exe ¯\_(ツ)_/¯

[2023-12-30 22:18] diversenok: I tried different access on the file and page protections on the section, same

[2023-12-30 22:20] diversenok: Both when calling it from 64-bit and 32-bit process

[2023-12-30 22:20] diversenok: And for you it just works? Well, in a `I386 (64-bit)` way

[2023-12-30 22:21] Matti: yep

[2023-12-30 22:21] Matti: still trying to find the reason for the difference....

[2023-12-30 22:22] diversenok: Can you just send me your executable?

[2023-12-30 22:23] Matti: 
[Attachments: CreateWow64_x64.exe, CreateWow64_x64.pdb]

[2023-12-30 22:23] Matti: mind you I don't use argc/argv

[2023-12-30 22:23] Matti: so the path is the path

[2023-12-30 22:23] Matti: [replying to diversenok: "And for you it just works? Well, in a `I386 (64-bi..."]
but yeah this works for me, in this way

[2023-12-30 22:24] diversenok: ¯\_(ツ)_/¯
[Attachments: image.png]

[2023-12-30 22:25] Matti: <:thonk:883361660100821032>

[2023-12-30 22:25] Matti: 
[Attachments: image.png]

[2023-12-30 22:25] diversenok: 
[Attachments: image.png]

[2023-12-30 22:26] Matti: 
[Attachments: image.png]

[2023-12-30 22:27] diversenok: Windows <:yea:904521533727342632>

[2023-12-30 22:27] Matti: I wonder if it's some shit like `luafv.sys` interfering

[2023-12-30 22:27] Matti: FS virtualization

[2023-12-30 22:27] Matti: or some other FS filter shit

[2023-12-30 22:28] Matti: 
[Attachments: image.png]

[2023-12-30 22:30] Matti: and/or one of these?
[Attachments: image.png]

[2023-12-30 22:30] diversenok: It's running for me, let me disable it and reboot; although, I also tested creating the process with a token with dissallowed virtualization flag

[2023-12-30 22:31] Matti: you gotta kill the gpedit.msc policy too

[2023-12-30 22:31] diversenok: Disabling luafv didn't help

[2023-12-30 22:31] Matti: otherwise you still get whiny complaints about luafv not being able to start in the event log, and I don't trust that

[2023-12-30 22:32] diversenok: 
[Attachments: image.png]

[2023-12-30 22:32] Matti: ok, was worth a try

[2023-12-30 22:32] Matti: mysterious this

[2023-12-30 22:33] diversenok: Can you send me your cmd.exe from syswow64? Mine doesn't launch even if I copy it outside

[2023-12-30 22:35] Matti: 
[Attachments: cmd.exe, cmd.exe.mui]

[2023-12-30 22:35] Matti: you need the .mui in an `en-US` folder relative to cmd.exe

[2023-12-30 22:35] Matti: well or in system32\en-US, that's where I got this one

[2023-12-30 22:35] diversenok: Same effect

[2023-12-30 22:35] Matti: but most system32/syswow64 exes fail to launch when copied outside

[2023-12-30 22:35] Matti: because of that

[2023-12-30 22:36] Matti: ok

[2023-12-30 22:37] diversenok: Ah, nevermind, it's the exact same file as mine

[2023-12-30 22:37] Matti: I guess you're gonna have to debug your machine <:lillullmoa:475778601141403648>

[2023-12-30 22:37] Matti: in NtCreateProcessEx

[2023-12-30 22:37] Matti: it's weird because it's normally NtCreateSection that would return this status

[2023-12-30 22:38] diversenok: Yeah, an I get the same effect on both my machine and a VM

[2023-12-30 22:39] Matti: I've copied all of your code to double check, but no difference as expected

[2023-12-30 22:41] Matti: the only difference I noticed is that you pass `PAGE_READONLY` to NtCreateSection, vs my `PAGE_EXECUTE_READ`

[2023-12-30 22:41] Matti: but this doesn't make a difference

[2023-12-30 22:42] Matti: kinda odd that it doesn't, or well that PAGE_READONLY works is unexpected to me I guess

[2023-12-30 22:42] Matti: but yeah that's not it

[2023-12-30 22:44] diversenok: Oh, hey, it also gives me the same result on that `DaRT10.iso` file you shared earlier
[Attachments: image.png]

[2023-12-30 22:45] Matti: yeah but winpe doesn't have wow64 support

[2023-12-30 22:45] Matti: that's different

[2023-12-30 22:45] Matti: though interesting that it's the same failure status/place

[2023-12-30 22:46] Matti: [replying to Matti: "yeah but winpe doesn't have wow64 support"]
or that's what I thought anyway, never actually tested this

[2023-12-30 22:46] Matti: but I've heard it claimed often

[2023-12-30 22:47] diversenok: Yeah, it doesn't want to run 32-bit executables anyway

[2023-12-30 22:48] Matti: one more tiny difference I found is that I initially used OBJ_CASE_INSENSITIVE, vs your 0

[2023-12-30 22:48] Matti: but that also doesn't matter here

[2023-12-30 22:48] Matti: in fact those obj attrs aren't even sent to NtCreateProcessEx

[2023-12-30 22:48] Matti: it'd be NtCreateFile failing

[2023-12-30 22:48] Matti: / NtOpenFile

[2023-12-30 22:49] Matti: yet another difference <:lillullmoa:475778601141403648>

[2023-12-30 22:50] diversenok: Yeah, the winpe case doesn't say much because even `NtCreateUserProcess` fails 32-bit executables with the same status
[Attachments: image.png]

[2023-12-30 22:51] diversenok: Does your demo work only on your machine or also in VMs?

[2023-12-30 22:51] Matti: I've tried on two machines

[2023-12-30 22:51] Matti: lemme start a VM

[2023-12-30 22:52] Matti: yeaahhhhh we got a repro

[2023-12-30 22:53] Matti: I started the lamest VM I could find, which was win 11 insider preview

[2023-12-30 22:53] Matti: and now I get the same failure you do

[2023-12-30 22:54] diversenok: Can it be something related to CI policy?

[2023-12-30 22:54] Matti: also reproduces on my win 11 22621 VM

[2023-12-30 22:54] diversenok: Even audit CI mode can block `NtCreateProcessEx`, although with `STATUS_NOT_SUPPORTED`

[2023-12-30 22:54] Matti: hmm, well what'd be different about the CI policies?

[2023-12-30 22:55] Matti: hmmm

[2023-12-30 22:55] diversenok: You can test it via `ConvertFrom-CIPolicy -XmlFilePath C:\Windows\schemas\CodeIntegrity\ExamplePolicies\AllowAll.xml -BinaryFilePath C:\Windows\System32\CodeIntegrity\SIPolicy.p7b`

[2023-12-30 22:55] Matti: well I do have efiguard installed on all of these machines cause I fucking hate CI

[2023-12-30 22:55] Matti: but that includes the VMs too....

[2023-12-30 22:56] diversenok: [replying to diversenok: "You can test it via `ConvertFrom-CIPolicy -XmlFile..."]
And then reboot; better do it in a VM

[2023-12-30 22:57] diversenok: I don't have CI configured at all though

[2023-12-30 22:58] Matti: well there is definitely some bullshit going on in this VM
[Attachments: image.png]

[2023-12-30 22:58] diversenok: From admin

[2023-12-30 22:58] Matti: ohhh yeah

[2023-12-30 22:59] Matti: I keep forgetting that people have UAC enabled

[2023-12-30 22:59] Matti: that's exactly why I have this VM

[2023-12-30 22:59] Matti: and why I hate it

[2023-12-30 22:59] diversenok: Yeah, now it's a different story
[Attachments: image.png]

[2023-12-30 23:00] Matti: I still get the access denied even from an admin powershell

[2023-12-30 23:01] Matti: lmao

[2023-12-30 23:01] Matti: I copied it to C:\ using explorer and now the command works

[2023-12-30 23:01] Matti: what the fuck

[2023-12-30 23:02] Matti: [replying to diversenok: "Yeah, now it's a different story"]
this is just as strange

[2023-12-30 23:02] Matti: oh but this is due to CI being enabled, right

[2023-12-30 23:02] diversenok: [replying to Matti: "I still get the access denied even from an admin p..."]
And it was also failing to read a file which is accessible to all users... maybe it was running the command in a constrained mode or something

[2023-12-30 23:03] Matti: mb, I actually have UAC disabled on this VM <:thonk:883361660100821032>

[2023-12-30 23:03] Matti: I initially had it on because I had to reproduce some issue related to UAC

[2023-12-30 23:03] Matti: but I couldn't live with it being on so I eventually killed it anyway

[2023-12-30 23:04] Matti: every process is created with admin privileges

[2023-12-30 23:04] diversenok: Yet it fails to read a file like this 😂
[Attachments: image.png]

[2023-12-30 23:05] Matti: well yes

[2023-12-30 23:05] Matti: it's obviously very fucky

[2023-12-30 23:06] Matti: no idea though, I mean this entire VM is set up to be as cancerous as possible so that I can contain testing stuff like this to one place

[2023-12-30 23:06] Matti: also maybe it's just powershell being shit as usual

[2023-12-30 23:06] Matti: especially on insider preview

[2023-12-30 23:07] Matti: I don't wanna debug this since I don't care lol

[2023-12-30 23:07] Matti: the NtCreateProcessEx failing with STATUS_INVALID_IMAGE_FORMAT is interesting though

[2023-12-30 23:09] Matti: [replying to diversenok: "You can test it via `ConvertFrom-CIPolicy -XmlFile..."]
is this command supposed to create an SIPolicy.7b, where previously there wasn't one?

[2023-12-30 23:09] Matti: aw, thank you

[2023-12-30 23:09] Matti: what makes you say this

[2023-12-30 23:11] Matti: hm

[2023-12-30 23:11] Matti: well it's true, I try to

[2023-12-30 23:12] Matti: glad to hear that's working as intended <:lillullmoa:475778601141403648>

[2023-12-30 23:12] diversenok: [replying to Matti: "is this command supposed to create an SIPolicy.7b,..."]
Yes

[2023-12-30 23:13] Matti: alright, and this policy should take effect after a reboot? and then potentially influence the creation status?

[2023-12-30 23:13] Matti: like, is it a magic hardcoded path

[2023-12-30 23:13] Matti: where winload checks

[2023-12-30 23:16] diversenok: Yeah

[2023-12-30 23:17] diversenok: Although I never tried deleting it back after enabling

[2023-12-30 23:17] Matti: lol

[2023-12-30 23:18] Matti: well it should revert to whatever the default is, no? since the file doesn't exist originally

[2023-12-30 23:18] Matti: I tried it and it made no difference, FWIW

[2023-12-30 23:18] Matti: still 07B

[2023-12-30 23:19] Matti: also tested with efiguard off on the other machine, but it still works OK there

[2023-12-30 23:21] diversenok: [replying to Matti: "well it should revert to whatever the default is, ..."]
Who knows, maybe they do some trickery with persisting it; I remember there was something based on firmware environment variables... I think something that prevented disabling LSA PPL protection after it was enabled or alike

[2023-12-30 23:22] diversenok: [replying to Matti: "I tried it and it made no difference, FWIW"]
Enabling CI made no difference?

[2023-12-30 23:22] Matti: ah yeah hm

[2023-12-30 23:22] Matti: that's for HVCI and deviceguard locks, but they might be doing the same for CI policies

[2023-12-30 23:23] Matti: [replying to diversenok: "Enabling CI made no difference?"]
well I did use the AllowAll.xml sample

[2023-12-30 23:23] Matti: I guess I need a stricter one to get 0BB?

[2023-12-30 23:24] diversenok: No, any policy should work

[2023-12-30 23:24] diversenok: Oh, wait, you are on insider preview, right? Maybe they fixed it

[2023-12-30 23:24] diversenok: https://twitter.com/jxy__s/status/1717931789535105201

[2023-12-30 23:24] Matti: lol

[2023-12-30 23:25] Matti: but yeah I am on insider

[2023-12-30 23:25] Matti: this 'smart app control' is set to off

[2023-12-30 23:28] Matti: I don't believe I'm interested in doing a system reinstall just to see if I can get it to be enableable

[2023-12-30 23:28] diversenok: What's the security descriptor there?

[2023-12-30 23:28] Matti: of the p7b file?

[2023-12-30 23:28] Matti: good question, I did initially create it to C:\

[2023-12-30 23:28] diversenok: On the xml

[2023-12-30 23:29] Matti: o

[2023-12-30 23:29] diversenok: Okay, deleting the p7b file reverted the policy for me after the reboot; `NtCreateProcessEx` works again for 64-bit files

[2023-12-30 23:30] Matti: 
[Attachments: image.png]

[2023-12-30 23:30] Matti: I'll still retry the command, this time creating the file directly to system32

[2023-12-30 23:30] Matti: [replying to diversenok: "Okay, deleting the p7b file reverted the policy fo..."]
wait so this policy blocks any call to NtCreateProcessEx?

[2023-12-30 23:31] Matti: and it's called **AllowAll**?

[2023-12-30 23:31] diversenok: Haha, exactly

[2023-12-30 23:31] Matti: fuck windows

[2023-12-30 23:31] diversenok: Well, any call that specifies the section handle

[2023-12-30 23:31] diversenok: Cloning is still allowed

[2023-12-30 23:31] Matti: I see

[2023-12-30 23:31] diversenok: But you cannot create threads then

[2023-12-30 23:31] Matti: lol

[2023-12-30 23:31] Matti: even better

[2023-12-30 23:32] diversenok: They canont block calls to `NtCreateProcessEx` without section handle because they are used by the documented process snapshotting API (the Pss functions)

[2023-12-30 23:33] Matti: yeah

[2023-12-30 23:33] Matti: makes sense

[2023-12-30 23:33] Matti: ok no, I reran the command via powershell as TrustedInstaller, using the original path to the XML and outputting to system32

[2023-12-30 23:33] Matti: still 07B, not 0BB

[2023-12-30 23:34] Matti: but I do have this 'smart app control' disabled right

[2023-12-30 23:34] Matti: or

[2023-12-30 23:34] diversenok: I don't have smart app control in the options because I'm not on insider

[2023-12-30 23:35] Matti: it would return 0BB if I tried it with a system32 exe

[2023-12-30 23:35] Matti: maybe

[2023-12-30 23:35] Matti: I'm still trying syswow64\cmd.exe

[2023-12-30 23:36] Matti: nope

[2023-12-30 23:36] Matti: it just works 😅

[2023-12-30 23:36] diversenok: Maybe they fixed it; I'm also getting insider to take a look

[2023-12-30 23:37] Matti: I'm on 25982, FWIW

[2023-12-30 23:37] Matti: that's not the latest insider, but windows update broke as usual

[2023-12-31 00:30] diversenok: I installed 26016 from the iso; out of the box, it has no CI poly but has smart app control in the evaluation mode

[2023-12-31 00:31] diversenok: Smart app control blocks `NtCreateProcessEx` with `STATUS_NOT_SUPPORTED`; disabling it fixes the issue immediately (without reboot)

[2023-12-31 00:32] Matti: hmm alright

[2023-12-31 00:33] Matti: so then that's 1 out of 2 statuses explained?

[2023-12-31 00:33] Matti: the other one being 07B for wow64 exes on some machines and not others

[2023-12-31 00:34] diversenok: Yeah, so with smart app control both 64 and 32 give the same result

[2023-12-31 00:34] Matti: yeah makes sense

[2023-12-31 00:34] Matti: it's about blocking the API, not wow64

[2023-12-31 00:35] diversenok: I'm slowly testing all scenarios

[2023-12-31 00:35] Matti: I have a theory

[2023-12-31 00:35] diversenok: Oh, I got the same powershell error!

[2023-12-31 00:36] Matti: can you show your `SharedDataFlags` here?
[Attachments: image.png]

[2023-12-31 00:36] Matti: I suppose maybe MitigationPolicy as well

[2023-12-31 00:37] Matti: but my theory is it's in SharedDataFlags

[2023-12-31 00:38] Matti: all of my machines on which the wow64 create **works**, are missing `DbgInstallerDetectEnabled`
[Attachments: image.png]

[2023-12-31 00:38] Matti: ^ this one for example

[2023-12-31 00:38] diversenok: One moment

[2023-12-31 00:39] diversenok: Now I know why the powershell command fails
[Attachments: image.png]

[2023-12-31 00:39] diversenok: For stupid reasons

[2023-12-31 00:39] Matti: R/W access <:yea:904521533727342632>

[2023-12-31 00:39] Matti: fuck powershell

[2023-12-31 00:43] Matti: nevermind, hypothesis proven false

[2023-12-31 00:43] Matti: I enabled the installer detection crap on my other machine, but the create still works

[2023-12-31 00:44] Matti: maybe it's still what I'm thinking it is but I was thinking of the wrong flag.... unsure

[2023-12-31 00:45] Matti: I was basing this off of this ROS code, which I do believe will be called for wow64 sections
[Attachments: image.png]

[2023-12-31 00:45] Matti: in particular `IsShimInfrastructureDisabled()`

[2023-12-31 00:46] Matti: which ends up querying a bunch of stuff like this
[Attachments: image.png]

[2023-12-31 00:46] Matti: but on second thought, I'm not sure this explanation makes sense anyway since it would only explain kernel32.dll CreateProcess failing, not NtCreateProcessEx

[2023-12-31 00:46] Matti: unless the latter also checks these policies

[2023-12-31 00:46] Matti: and it's duplicated code

[2023-12-31 00:48] diversenok: I doubt `NtCreateProcessEx` calls into csrss, although we can try to verify it

[2023-12-31 00:48] Matti: nah, it doesn't

[2023-12-31 00:48] Matti: this is stuff happening after NtCreateSection, before NtCreateProcessEx

[2023-12-31 00:48] Matti: in kernel32

[2023-12-31 00:49] Matti: with kernel32 querying section image information itself, to get the machine type and some other stuff

[2023-12-31 00:49] diversenok: Interesting, looks like AllowAll CI policy doesn't block `NtCreateprocessEx` anymore on insider builds

[2023-12-31 00:49] Matti: lol

[2023-12-31 00:49] diversenok: So I managed to reproduce that

[2023-12-31 00:49] Matti: what an achievement

[2023-12-31 00:49] Matti: AllowAll actually allows all!

[2023-12-31 00:49] Matti: gj windows team

[2023-12-31 00:50] diversenok: Smart app control in evaluation mode on the other hand... 😐

[2023-12-31 00:50] Matti: hmm

[2023-12-31 00:50] Matti: maybe it goes to event log?

[2023-12-31 00:50] diversenok: At least it explains the difference between 10 and insider

[2023-12-31 00:52] diversenok: Which one? CI or Smart App Control?

[2023-12-31 00:52] Matti: well both I guess

[2023-12-31 00:52] Matti: but CI

[2023-12-31 00:53] Matti: or maybe app control <:lillullmoa:475778601141403648> idk

[2023-12-31 00:53] Matti: if it's in evaluation mode, it has to be logging the audit data somewhere right

[2023-12-31 00:53] diversenok: True

[2023-12-31 00:53] diversenok: I don't see new events in CI log when using `NtCreateProcessEx`

[2023-12-31 00:54] Matti: oh nvm

[2023-12-31 00:55] Matti: 'evaluation mode' is literally telemetry, it's not some kind of auditing
[Attachments: image.png]

[2023-12-31 00:55] Matti: I mean it still probably logs somewhere, just in a proprietary format and in a place you don't know about

[2023-12-31 00:55] diversenok: Yeah, I don't see a dedicated place in the audit log for it

[2023-12-31 00:58] diversenok: [replying to Matti: "can you show your `SharedDataFlags` here?"]
This is the insider VM
[Attachments: image.png]

[2023-12-31 00:59] diversenok: This is my machine
[Attachments: image.png]

[2023-12-31 00:59] Matti: yeah sorry, I already determined it wasn't that

[2023-12-31 00:59] diversenok: Well, there are also some other bits

[2023-12-31 01:00] Matti: that's true

[2023-12-31 01:00] Matti: but my win 10 machine, which has the same kernel as yours, works either with or without the flags I was suspecting

[2023-12-31 01:01] Matti: and we both seem to have mitigationflags = 0xA on all machines, so that rules that out

[2023-12-31 01:02] Matti: [replying to diversenok: "This is the insider VM"]
ActiveConsoleId = 0x4 😓

[2023-12-31 01:02] Matti: wtf are the other three for

[2023-12-31 01:03] diversenok: I was reconnecting a few times, enhanced sessions hyper-v do that for some reason

[2023-12-31 01:04] Matti: ah

[2023-12-31 01:04] Matti: yeah

[2023-12-31 01:04] Matti: RDP too

[2023-12-31 01:09] diversenok: They are probably related

[2023-12-31 01:10] diversenok: The dialog it shows during connection looks too familiar
[Attachments: image.png]

[2023-12-31 01:12] Matti: weeeiird

[2023-12-31 01:12] Matti: I should try hyper-v someday....

[2023-12-31 01:18] diversenok: Hyper-v also allows Windows Sandbox which is quite interesting

[2023-12-31 01:20] diversenok: It's full of MS magic like this
[Attachments: image.png]

[2023-12-31 01:20] Matti: gross

[2023-12-31 01:22] diversenok: And it's VM that updates with the main OS and is always clean every time you run it

[2023-12-31 01:22] diversenok: Which brings some limitations, but is also super convenient

[2023-12-31 01:22] Matti: I can't even get windows update to keep working on my main OS anyway

[2023-12-31 01:22] Matti: it just kills itself every 2-3 months

[2023-12-31 01:23] diversenok: ¯\_(ツ)_/¯

[2023-12-31 01:24] diversenok: It also uses lightweight differencing vhdx images with many WCI reparse points that mount existing directories/files from the host

[2023-12-31 01:25] Matti: yeah so like

[2023-12-31 01:25] Matti: that is cool

[2023-12-31 01:25] Matti: but I don't trust it

[2023-12-31 01:25] Matti: I don't trust them to get all of it right everywhere

[2023-12-31 01:26] diversenok: Fair, it's less isolated then regular VMs, but good enough for quick tests

[2023-12-31 01:26] Matti: yeah

[2023-12-31 01:26] Matti: I do get the point of it

[2023-12-31 01:27] Matti: but since I don't even have hyper-v installed I'm not likely to try it out anytime soon

[2023-12-31 01:42] Matti: haha

[2023-12-31 01:43] Matti: remember your original question?

[2023-12-31 01:43] Matti: about creating a wow64 process on XP/2003 x64

[2023-12-31 01:43] Matti: I still hadn't tested that yet due to the weird `STATUS_INVALID_IMAGE_FORMAT` bs

[2023-12-31 01:43] Matti: but it works!
[Attachments: image.png]

[2023-12-31 01:43] Matti: and what's more, I got a wow64 peb

[2023-12-31 01:44] Matti: so something about the way this works clearly changed a lot

[2023-12-31 01:45] Matti: 
[Attachments: image.png]

[2023-12-31 01:50] diversenok: Cool, so it used to be built into process creation itself

[2023-12-31 01:50] Matti: yep

[2023-12-31 01:51] diversenok: Not something you need to do later on an existing process

[2023-12-31 01:51] Matti: basically the way I already explained it works

[2023-12-31 01:51] Matti: only, turns out that it doesn't work like that anymore

[2023-12-31 01:51] diversenok: I started looking into the issue with the kernel debugger

[2023-12-31 01:51] diversenok: `PspAllocateProcess` fails because `MmInitializeProcessAddressSpace` fails

[2023-12-31 01:52] Matti: interesting

[2023-12-31 01:52] Matti: that's exactly the function that on XP allocates the Wow64Process struct

[2023-12-31 01:52] Matti: if needed depending on the machine type

[2023-12-31 01:57] diversenok: [replying to diversenok: "`PspAllocateProcess` fails because `MmInitializePr..."]
And it fails because `MiMapProcessExecutable` fails

[2023-12-31 01:58] Matti: I think I managed to fix the status!

[2023-12-31 01:58] diversenok: `MiCfgInitializeProcess` is the reason

[2023-12-31 01:58] Matti: by killing mitigation options

[2023-12-31 01:59] Matti: only, not sure which one is responsible exactly

[2023-12-31 01:59] Matti: but I expect it'll be related to top down ASLR

[2023-12-31 01:59] diversenok: It's CFG

[2023-12-31 01:59] Matti: ah hm <:thonk:883361660100821032>

[2023-12-31 02:00] Matti: thinking about why wow64 would be incompatible with cfg

[2023-12-31 02:00] Matti: ASLR is easy to explain

[2023-12-31 02:01] Matti: does CFG actually do anything for wow64 processes? like, is it expected to be useless / does it cause a linker error even

[2023-12-31 02:01] Matti: I know `/guard:ehcont` requires x64

[2023-12-31 02:02] Matti: but is that part of CFG? no right?

[2023-12-31 02:02] diversenok: This line
[Attachments: image.png]

[2023-12-31 02:02] diversenok: Hmm, yeah, good question why

[2023-12-31 02:03] Matti: [replying to diversenok: "This line"]
but this looks related to ASLR to me

[2023-12-31 02:03] Matti: since it affects the highest user address

[2023-12-31 02:03] diversenok: It's inside `MiCfgInitializeProcess`, so ¯\_(ツ)_/¯

[2023-12-31 02:03] Matti: hmm

[2023-12-31 02:04] diversenok: Maybe both?

[2023-12-31 02:04] diversenok: Cannot map CFG bitmap if the highest address is low or something

[2023-12-31 02:04] Matti: I guess it could be somehow a requirement for CFG, that due to ASLR being enabled, doesn't execute

[2023-12-31 02:04] Matti: yeah

[2023-12-31 02:04] Matti: I'll go experiment with the options a bit to figure out which one(s) cause this

[2023-12-31 02:05] diversenok: Now, why does it work on your machine then

[2023-12-31 02:05] Matti: because I disable all that shit

[2023-12-31 02:05] Matti: performance >>>>> everything

[2023-12-31 02:06] Matti: I don't really care about security as much

[2023-12-31 02:06] diversenok: I just don't see a path how it won't get there and fail

[2023-12-31 02:06] diversenok: The call seems pretty unconditional on the first glance

[2023-12-31 02:07] diversenok: Oh, nvm
[Attachments: image.png]

[2023-12-31 02:07] Matti: wdym

[2023-12-31 02:07] Matti: the highest user address is initialized elsewhere, no?

[2023-12-31 02:07] Matti: for the eprocess

[2023-12-31 02:08] Matti: well I expect so anyway

[2023-12-31 02:08] diversenok: `MiCfgInitializeProcess` exits immediately without the check if mitigation option 1 is disabled

[2023-12-31 02:08] Matti: aha

[2023-12-31 02:08] Matti: so it will be CFG in the end after all then

[2023-12-31 02:09] Matti: for reasons... that I don't know

[2023-12-31 02:11] diversenok: [replying to diversenok: "`MiCfgInitializeProcess` exits immediately without..."]
Which is `ControlFlowGuardEnabled`

[2023-12-31 02:11] Matti: yeah

[2023-12-31 02:11] Matti: I verified enabling/disabling CFG toggles the status

[2023-12-31 02:12] diversenok: System-wide or for the specific executable?

[2023-12-31 02:12] Matti: are the syswow64 exes not CFG compatible because they don't need to be, or because they can't be?

[2023-12-31 02:12] Matti: system-wide

[2023-12-31 02:12] Matti: IDK how to do this for an executable

[2023-12-31 02:12] diversenok: They are, and they get two CFG bitmaps - for 64 and 32-bit code

[2023-12-31 02:13] Matti: so weird

[2023-12-31 02:13] Matti: this almost seems like a bug to me then

[2023-12-31 02:14] Matti: but

[2023-12-31 02:14] diversenok: https://www.alex-ionescu.com/closing-heavens-gate/
[Embed: Closing “Heaven’s Gate”]

[2023-12-31 02:14] Matti: kernel32 can launch wow64 exes?

[2023-12-31 02:14] Matti: yeah it clearly can

[2023-12-31 02:14] diversenok: You mean via NtCreateUserProcess? Sure

[2023-12-31 02:15] Matti: wonder if it's passing some flag to NtCreateUserProcess

[2023-12-31 02:15] diversenok: `NtCreateUserProcess` works fine with WoW64 executables out of the box

[2023-12-31 02:16] Matti: but weren't you getting a failure from it earlier?

[2023-12-31 02:16] Matti: with the failure status set to the same

[2023-12-31 02:16] Matti: or was that something else

[2023-12-31 02:16] diversenok: No, everything was from `NtCreateProcessEx`

[2023-12-31 02:17] diversenok: Either invalid image or not supported

[2023-12-31 02:17] diversenok: Ohh, yes, yes

[2023-12-31 02:17] diversenok: I did get the same error from `NtCreateUserProcess`... on winpe

[2023-12-31 02:17] Matti: ah

[2023-12-31 02:17] Matti: oh yes

[2023-12-31 02:18] Matti: which we know to not support wow64

[2023-12-31 02:18] Matti: so that's the mystery resolved then

[2023-12-31 02:18] diversenok: 🥳

[2023-12-31 02:19] Matti: at this point they might as well delete NtCreateProcess

[2023-12-31 02:19] Matti: there was a bug in it for years on win 10 where calling it with a NULL section would BSOD

[2023-12-31 02:19] Matti: only I guess they need it for reflection

[2023-12-31 02:19] diversenok: Really? Null section is supposed to clone the address space

[2023-12-31 02:20] Matti: er, I might be misremembering, lemme check

[2023-12-31 02:20] Matti: something null

[2023-12-31 02:20] diversenok: Should work fine since 8.1 where they introduced Pss snapshot API

[2023-12-31 02:20] Matti: https://github.com/Mattiwatti/BSOD10/blob/master/src/main.c
[Embed: BSOD10/src/main.c at master · Mattiwatti/BSOD10]
Crash Windows 10 up to RS2 from an unprivileged process - Mattiwatti/BSOD10

[2023-12-31 02:21] Matti: so not a null section

[2023-12-31 02:21] Matti: just a regular call

[2023-12-31 02:21] Matti: that's how rarely used it is

[2023-12-31 02:21] diversenok: [replying to Matti: "at this point they might as well delete NtCreatePr..."]
It's the only way to clone address space of another process

[2023-12-31 02:22] diversenok: `NtCreateUserProcess` can only clone the calling process for conceptual reasons

[2023-12-31 02:22] Matti: yeah I know

[2023-12-31 02:22] Matti: but that's what I'm saying

[2023-12-31 02:22] Matti: maybe it's not reflection that uses this, but something in ntdll surely does

[2023-12-31 02:23] Matti: or WER

[2023-12-31 02:24] diversenok: Yep

[2023-12-31 02:24] diversenok: https://diversenok.github.io/images/ProcessCloning/05-taxonomy.png

[2023-12-31 02:25] Matti: <:lmao3d:611917482105765918>

[2023-12-31 02:26] Matti: clearly we need an `NtCloneUserProcess`

[2023-12-31 02:27] brymko: we have that already

[2023-12-31 02:27] brymko: called fork

[2023-12-31 02:27] Matti: yeah

[2023-12-31 02:27] Matti: it's useful

[2023-12-31 02:27] Matti: I wonder why they decided to cripple address space cloning in NtCreateUserProcess

[2023-12-31 02:28] diversenok: Well, `NtCreateUserProcess` clones more than the process

[2023-12-31 02:28] diversenok: Also the calling thread

[2023-12-31 02:29] Matti: hm, but that's useful right

[2023-12-31 02:29] diversenok: It's the local fork, I suppose

[2023-12-31 02:30] Matti: [replying to diversenok: "`NtCreateUserProcess` can only clone the calling p..."]
oh I see what you meant now

[2023-12-31 02:30] Matti: yeah ok, I get it

[2023-12-31 02:30] Matti: but fork surely also clones the calling thread right

[2023-12-31 02:30] Matti: and then you see which one you are (parent or clone) from the return status

[2023-12-31 02:31] diversenok: It does the exact same thing, yeah, returning `STATUS_PROCESS_CLONED` in the clone and everything else in the parent

[2023-12-31 02:32] Matti: ok cool

[2023-12-31 02:32] Matti: so the biggest limitation then is that you can't clone other processes I guess

[2023-12-31 02:32] Matti: which is kinda useful sometimes

[2023-12-31 02:33] diversenok: Reflection does it via the classical hack of "if you cannot do something cross-process, create a remote thread and do it in-process"

[2023-12-31 02:34] Matti: yeahhh <:harold:704245193016344596>

[2023-12-31 02:34] Matti: I don't love this approach

[2023-12-31 02:35] Matti: you're invading the target process before you've even cloned it

[2023-12-31 02:35] Matti: thus affecting both it and the clone

[2023-12-31 02:35] Matti: in subtle ways

[2023-12-31 02:35] diversenok: Yeah, you can potentially crash it, especially if it already started to crumble

[2023-12-31 02:36] diversenok: WER can do a final blow this way

[2023-12-31 02:36] Matti: yep

[2023-12-31 02:36] Matti: I've seen this happen a few times

[2023-12-31 02:37] Matti: even once where it caused WER to kick into action because of the crash, clone the process and....

[2023-12-31 02:37] Matti: well you get the idea

[2023-12-31 02:37] Matti: a fork bomb via WER

[2023-12-31 02:37] Matti: only all the processes were suspended

[2023-12-31 02:38] diversenok: There was also some documented API that asks WER to restart your process if it crashes

[2023-12-31 02:38] Matti: ah classic <:hmm:475800667177484308>

[2023-12-31 02:38] diversenok: `RegisterApplicationRestart`

[2023-12-31 02:39] diversenok: Oh, they do take potential problems into account:
> To prevent cyclical restarts, the system will only restart the application if it has been running for a minimum of 60 seconds.

[2023-12-31 02:40] diversenok: Anyway, I wrote a blog post about cloning some time ago: https://diversenok.github.io/2023/04/20/Process-Cloning.html
[Embed: The Definitive Guide to Process Cloning on Windows]
This article (that I wrote for Hunt & Hackett) aims to provide the reader with a comprehensive guide to the technical details and the underlying design decisions of process cloning on Windows and how 

[2023-12-31 02:42] diversenok: [replying to Matti: "there was a bug in it for years on win 10 where ca..."]
There was also a crash that you can cause by releasing the section during `NtCreateUserProcess`-based cloning

[2023-12-31 02:43] diversenok: `PROCESS_CREATE_FLAGS_RELEASE_SECTION`

[2023-12-31 02:44] diversenok: `EPROCESS->SectionObject` becomes NULL which just asks to be dereferenced

[2023-12-31 02:45] Matti: [replying to diversenok: "Anyway, I wrote a blog post about cloning some tim..."]
cool
I should read your blog posts some time really, I've seen other good ones before

[2023-12-31 02:46] Matti: [replying to diversenok: "`EPROCESS->SectionObject` becomes NULL which just ..."]
yeah this sounds like something that shouldn't have passed code review, or even concept review <:lillullmoa:475778601141403648>

[2023-12-31 02:48] Matti: though it's not as bad as that one time when someone added NTFS transactions as a joke

[2023-12-31 02:48] Matti: and they can't kill it

[2023-12-31 02:52] diversenok: NTFS transactions are awesome

[2023-12-31 02:52] diversenok: They break so many assumptions

[2023-12-31 02:55] asz: breaks more when you break transaction safety

[2023-12-31 02:56] asz: then filelocks no longer exist

[2023-12-31 02:57] diversenok: I mean, the whole idea of, effectively, multiple files with different content but the exact same full is already fun

[2023-12-31 02:58] diversenok: When you map a transacted file, you cannot tell which one it is by querying `MemoryMappedFilenameInformation`

[2023-12-31 02:59] diversenok: [replying to Matti: "and they can't kill it"]
They did disable TxF miniversion and savepoint support already though

[2023-12-31 03:00] diversenok: And some other TxF FSCTLs are kind of broken

[2023-12-31 03:01] Matti: yeah they're trying

[2023-12-31 03:02] Matti: [replying to diversenok: "And some other TxF FSCTLs are kind of broken"]
this is precisely the NtCreateProcess situation

[2023-12-31 03:02] Matti: it leads to unexpected issues often

[2023-12-31 03:03] Matti: they should just drop TxF support entirely if they're going to give up on it (which they should)

[2023-12-31 03:03] Matti: not stop maintaining or testing it while saying it's only *deprecated* instead of removed

[2023-12-31 03:04] diversenok: [replying to Matti: "they should just drop TxF support entirely if they..."]
But my attack surface... 😰

[2023-12-31 03:05] Matti: but my performance 😎