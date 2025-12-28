# November 2024 - Week 4
# Channel: #programming
# Messages: 159

[2024-11-18 01:06] Torph: [replying to contificate: "and focus on each part piece by piece"]
already on it lol
but yeah you're right C is making it a pain in the ass. just getting to reasonable tokens with operators being separated without being deleted is a pain

[2024-11-18 01:07] Torph: `strtok` only has a concept of "single-character delimiter that should be completely skipped"

[2024-11-18 14:44] contificate: don't even use strtok

[2024-11-18 14:44] contificate: I really like using re2c in C

[2024-11-18 14:45] contificate: but doing a maximal munch lexer by hand is easy

[2024-11-18 14:45] contificate: purely mechanical lowering

[2024-11-18 15:38] Horsie: Any suggestions for allocating memory in a particular section with llvm? (In a PE)

[2024-11-18 15:39] Horsie: I very much doubt that llvm passes are platform aware by default, but maybe there's a way to give it a hint about which section I want to use?

[2024-11-18 15:45] flower: `section` attribute?

[2024-11-18 15:47] Horsie: [replying to flower: "`section` attribute?"]
Oh.. I didn't find that after some long googling. I'll try that. Thanks

[2024-11-18 15:49] flower: see: https://llvm.org/docs/LangRef.html#global-variables

[2024-11-18 15:49] flower: (& function section right below)

[2024-11-18 18:05] Torph: [replying to contificate: "I really like using re2c in C"]
i haven't heard of this or maximal munch... I already wrote the lexer and I *think* I'm basically doing maximal munch right now (only using `strtok` to get rid of whitespace). maybe I'll replace it with re2c later, current lexer is probably good enough and code generation complicates the build system a little

[2024-11-18 20:30] contificate: typically you don't use something like strtok

[2024-11-18 20:30] contificate: you just have a state machine that you implement and supress whitespace tokens

[2024-11-18 21:11] Torph: i haven't had much experience with state machines, my tokenizer just tokenizes the entire string in 1 pass and essentially spits out an array of `char*`

[2024-11-18 21:50] contificate: well the basis of lexers is usually a state machine computed from a regular expressions (a DFA)

[2024-11-18 21:50] contificate: but then for various semantics

[2024-11-18 21:50] contificate: it's controlled/driven by an algorithm

[2024-11-18 21:50] contificate: that can do this "maximal munch"/longest match stuff

[2024-11-18 21:50] contificate: because regex doesn't perfectly model lexing

[2024-11-18 21:50] contificate: regex is designed to kind of exhaust the input and tell you what state you're left in

[2024-11-18 21:51] contificate: whereas lexing usually goes as far as possible, finds no edge in the DFA for the given input/character, and then yields the last accepting state, resets and continues

[2024-11-18 21:51] contificate: but yeah

[2024-11-18 21:51] contificate: most people can asspull a lexer

[2024-11-18 21:51] contificate: a simple one, anyway

[2024-11-18 21:51] contificate: no shame in just splitting by whitespace and processing strings to begin with

[2024-11-18 22:25] Torph: [replying to contificate: "well the basis of lexers is usually a state machin..."]
is there a reason for using specifically regex, other than it being common/standard? i don't really feel like rolling my own regex <:kekw:904522300257345566>

[2024-11-18 22:45] contificate: most tokens are readily identifiable as being described as regular languages

[2024-11-18 22:45] contificate: so it's a natural formalism to appeal to

[2024-11-18 22:45] contificate: ther'es always some ad-hoc stuff of course

[2024-11-18 22:45] contificate: for example

[2024-11-18 22:45] contificate: nested comments, escaping strings, and doing python-like indent/dedent pairings

[2024-11-18 22:45] contificate: is just some ad-hoc shit you cook in

[2024-11-18 22:46] contificate: can still maintain a formalism for much of the other stuff

[2024-11-18 22:55] BWA RBX: I love when you go on a rant <@687117677512360003>, it's never anything stupid always something valuable to consider

[2024-11-18 22:56] BWA RBX: <@687117677512360003> VC and teach me OCaml

[2024-11-18 22:57] contificate: 😳

[2024-11-18 23:15] luci4: Damn, he's smooth

[2024-11-19 01:25] BWA RBX: [replying to contificate: "😳"]
I promise I'm not flirting

[2024-11-19 03:51] Torph: [replying to contificate: "nested comments, escaping strings, and doing pytho..."]
oh ok that makes sense

[2024-11-19 15:18] avx: any way with WinHTTP, to have a websocket handshake be synchronous and then have the websocket be async; been trying to set a status callback and it fails with WINHTTP_INVALID_STATUS_CALLBACK (yet GetLastError returns 0x0). as far as I understand WinHttpSetStatusCallback should return the previously defined callback function (aka, should be NULL in my case) yet I get that invalid status callback status

[2024-11-19 15:36] x86matthew: [replying to avx: "any way with WinHTTP, to have a websocket handshak..."]
not sure if that will work, i think you have to either specify sync/async for the entire session when creating it (WINHTTP_FLAG_ASYNC flag)

[2024-11-19 15:36] x86matthew: probably best to use async for everything instead

[2024-11-19 15:36] x86matthew: i could be wrong though, i try to avoid winhttp

[2024-11-19 15:42] avx: valid

[2024-11-19 15:42] avx: yeah wanted to avoid too much refactoring but its bound to happen it seems

[2024-11-19 15:46] avx: or i mean could just wait for a bit and then do actual async <:trollface:1151261622011183146>

[2024-11-19 15:52] Humza: Has anyone got any nice books  or beginner learning resources centred around UEFI architecture and UEFI development as a whole  ?

[2024-11-19 15:52] estrellas: https://p.ost2.fyi/courses/course-v1:OpenSecurityTraining2+Arch4021_intro_UEFI+2023_v1/about
[Embed: Architecture 4021: Introductory UEFI]

[2024-11-19 15:53] Humza: Thank u

[2024-11-19 15:53] estrellas: and well, rtfm

[2024-11-19 16:00] luci4: [replying to x86matthew: "i could be wrong though, i try to avoid winhttp"]
What other alternative is there?

[2024-11-19 16:00] luci4: Beside Wininet, and Winsock, I guess

[2024-11-19 16:29] 5pider: NtSockets

[2024-11-19 16:29] 5pider: https://www.x86matthew.com/view_post?id=ntsockets

[2024-11-19 16:29] 5pider: made by a crazy crack head

[2024-11-19 16:29] 5pider: <:smiley:1184418532394545163>

[2024-11-19 16:31] avx: not going to rawdog my own ws implementation nor statically link one

[2024-11-19 16:54] luci4: [replying to 5pider: "NtSockets"]
Been meaning to make an NT socket library to replace WinSock with

[2024-11-19 16:54] 5pider: go crazy

[2024-11-20 12:34] luci4: What is `AceType` supposed to be here?

https://ntdoc.m417z.com/rtladdmandatoryace
[Embed: RtlAddMandatoryAce - NtDoc]
RtlAddMandatoryAce - NtDoc, the native NT API online documentation

[2024-11-20 12:34] luci4: I tried `SYSTEM_MANDATORY_LABEL_ACE_TYPE` but it always returns `STATUS_INVALID_PARAMETER`

[2024-11-20 12:36] luci4: Oh

[2024-11-20 12:36] luci4: Weirdly enough, the problem was my access mask, `SPECIFIC_RIGHTS_ALL | STANDARD_RIGHTS_ALL`

[2024-11-20 12:38] luci4: Setting the access mask to 0 worked, for some reason

[2024-11-20 21:35] Matti: [replying to luci4: "Setting the access mask to 0 worked, for some reas..."]
you can actually specify any access mask that doesn't have the 3 LSB set (so clear 0x7)

[2024-11-20 21:35] Matti: don't ask me why it checks for those bits specifically, I'm just going by what I see in IDA

[2024-11-20 21:35] Matti: but

[2024-11-20 21:36] Matti: what were you trying to accomplish with this access mask on the ACE

[2024-11-20 21:36] Matti: what are you trying to do in general, I guess

[2024-11-20 21:38] Matti: `SPECIFIC_RIGHTS_ALL` is the one upsetting this API, but tbh I don't think I've ever seen a use/need for either of those two masks

[2024-11-20 21:39] Matti: in general I always try to use object type specific access masks, not the generic defines

[2024-11-20 21:40] Matti: MAXIMUM_ALLOWED and SYNCHRONIZE are two exceptions

[2024-11-20 21:46] Matti: [replying to luci4: "I tried `SYSTEM_MANDATORY_LABEL_ACE_TYPE` but it a..."]
this also seems kind of... redundant? mind you I don't know what this API is supposed to actually do, hence why I'm asking what you're trying to do

[2024-11-20 21:46] Matti: but I mean it is called RtlAddMandatoryAce

[2024-11-20 22:54] diversenok: [replying to luci4: "I tried `SYSTEM_MANDATORY_LABEL_ACE_TYPE` but it a..."]
`SYSTEM_MANDATORY_LABEL_ACE_TYPE` is correct; the error appears because of some other parameter being incorrect

[2024-11-20 22:54] diversenok: It is redundant, true

[2024-11-20 22:54] diversenok: Well, at least until they decide to add another mandatory ACE type I guess

[2024-11-20 22:57] diversenok: [replying to luci4: "Weirdly enough, the problem was my access mask, `S..."]
Mandatory ACEs don't use common access rights; in their case the field actually stores the mandatory policy value

[2024-11-20 22:57] diversenok: i.e., combinations of these:
```c
#define SYSTEM_MANDATORY_LABEL_NO_WRITE_UP         0x1
#define SYSTEM_MANDATORY_LABEL_NO_READ_UP          0x2
#define SYSTEM_MANDATORY_LABEL_NO_EXECUTE_UP       0x4
```

[2024-11-20 23:01] diversenok: It describes  whether callers with integrity level below the specified value (see the SID field) should be blocked from opening the resource for `GENERIC_WRITE`, `GENERIC_READ`, and `GENERIC_EXECUTE`, respectively

[2024-11-20 23:06] diversenok: So yeah, the function fails if you specify the wrong ACE type, use an SID that is not `S-1-16-*`, or an access mask that includes bits beyond these three defined values

[2024-11-20 23:06] luci4: [replying to diversenok: "i.e., combinations of these:
```c
#define SYSTEM_M..."]
I did figure it out, but thanks 😄

[2024-11-20 23:07] luci4: My pain is now NtCreateNamedPipeFile

[2024-11-20 23:07] diversenok: Ohh, yeah, that's a more difficult one

[2024-11-20 23:07] luci4: But! I will never give up and use kernel32

[2024-11-20 23:08] diversenok: 🫡

[2024-11-20 23:08] luci4: [replying to diversenok: "🫡"]
Funnily enough, it's STATUS_INVALID_PARAMETER again

[2024-11-20 23:09] diversenok: And there are a lot more parameters this time...

[2024-11-20 23:09] luci4: [replying to diversenok: "And there are a lot more parameters this time..."]
Just gonna mix and match until it works, lol

[2024-11-20 23:09] diversenok: Do you want to make a named pipe or an anonymous one?

[2024-11-20 23:10] luci4: [replying to diversenok: "Do you want to make a named pipe or an anonymous o..."]
Named!

[2024-11-20 23:10] luci4: I prefixed my pipename with Device\NamedPipe\

[2024-11-20 23:11] diversenok: With a `\` before that I hope

[2024-11-20 23:11] luci4: Yep yep

[2024-11-20 23:11] diversenok: `\Device\NamedPipe\...`

[2024-11-20 23:12] luci4: Its CreateOptions that got me messed up

[2024-11-20 23:13] diversenok: `CreateOptions` are the same as in `NtCreateFile`/`NtOpenFile`

[2024-11-20 23:14] luci4: [replying to diversenok: "`CreateOptions` are the same as in `NtCreateFile`/..."]
...exactly 😦

[2024-11-20 23:14] diversenok: So just pass `FILE_SYNCHRONOUS_IO_NONALERT` there

[2024-11-20 23:15] diversenok: `ShareAccess` defines pipe direction

[2024-11-20 23:17] diversenok: And don't forget to include `SYNCHRONIZE` access

[2024-11-20 23:18] luci4: [replying to diversenok: "And don't forget to include `SYNCHRONIZE` access"]

[Attachments: 20241121_011808.jpg]

[2024-11-20 23:18] luci4: I had this

[2024-11-20 23:19] diversenok: Timeout cannot be null

[2024-11-20 23:19] luci4: MaximumInstances is a random number, wanted to see if specifying the max value was causing it to cry

[2024-11-20 23:19] luci4: [replying to diversenok: "Timeout cannot be null"]
Ah...

[2024-11-20 23:21] diversenok: `ReadMode` should also be `FILE_PIPE_BYTE_STREAM_MODE`

[2024-11-20 23:21] diversenok: Not `FILE_PIPE_BYTE_STREAM_TYPE`

[2024-11-20 23:21] diversenok: Although the constants are identical

[2024-11-20 23:27] diversenok: Here, this works:
```c
int wmain(int argc, wchar_t* argv[])
{
    NTSTATUS status;
    HANDLE hPipe;
    UNICODE_STRING name;
    OBJECT_ATTRIBUTES objAttr;
    IO_STATUS_BLOCK isb;
    LARGE_INTEGER timeout;

    timeout.QuadPart = -500000;
    RtlInitUnicodeString(&name, L"\\Device\\NamedPipe\\Test");
    InitializeObjectAttributes(&objAttr, &name, OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = NtCreateNamedPipeFile(
        &hPipe,
        GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
        &objAttr,
        &isb,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_CREATE,
        FILE_SYNCHRONOUS_IO_NONALERT,
        FILE_PIPE_BYTE_STREAM_TYPE,
        FILE_PIPE_BYTE_STREAM_MODE,
        FILE_PIPE_QUEUE_OPERATION,
        2,
        0x1000,
        0x1000,
        &timeout
    );
}
```

[2024-11-21 11:12] luci4: [replying to diversenok: "Here, this works:
```c
int wmain(int argc, wchar_t..."]
Oh it worked!

[2024-11-21 11:12] luci4: Funnily enough, the problem was that I was freeing my SACL and DACL a bit *too* early

[2024-11-21 11:12] luci4: So it gave me an access violation

[2024-11-21 11:15] luci4: Now I only need to figure out what the equivalent to`ConnectNamedPipe` is. Although it probably just sends an IOCTL

[2024-11-21 12:31] luci4: Well seems like it sends `0x110008`, which corresponds with `FSCTL_PIPE_LISTEN`

[2024-11-22 03:17] jonaslyk: 
[Attachments: image.png]

[2024-11-22 03:58] Torph: <:kekw:904522300257345566>

[2024-11-22 03:58] Torph: god i love/hate macros

[2024-11-22 11:35] rin: Does anyone know of documentation for properly converting scan codes to virtual keys to characters.

[2024-11-22 11:43] Brit: vkey to char should be done with toUnicode https://learn.microsoft.com/en-gb/windows/win32/api/winuser/nf-winuser-tounicode
[Embed: ToUnicode function (winuser.h) - Win32 apps]
Translates the specified virtual-key code and keyboard state to the corresponding Unicode character or characters. (ToUnicode)

[2024-11-22 11:44] Brit: this has the funny property of having to deal with modifier keys yourself

[2024-11-22 12:04] rin: To use tounicode requires you to calculate the virtualkey which is ok. But I am trying to convert scan codes to characters by just parsing the keyboard layout and taking scan codes.

[2024-11-22 12:05] rin: No additional windows functions

[2024-11-22 12:47] Humza: [replying to rin: "To use tounicode requires you to calculate the vir..."]
Is it for AT?

[2024-11-22 14:26] diversenok: [replying to rin: "Does anyone know of documentation for properly con..."]
There is a 3-part series about writing a keylogger that does that:
https://www.synacktiv.com/en/publications/writing-a-decent-win32-keylogger-13
https://www.synacktiv.com/en/publications/writing-a-decent-win32-keylogger-23
https://www.synacktiv.com/en/publications/writing-a-decent-win32-keylogger-33

[2024-11-22 15:20] Humza: I wrote a keyboard driver a few weeks ago if u want I can send u my scancode table for it

[2024-11-22 15:20] Humza: its for AT

[2024-11-22 19:55] rin: [replying to diversenok: "There is a 3-part series about writing a keylogger..."]
I already read this and its ok, I learned how to parse kbd*.dll based on these posts but the explanation on reconstruction is somewhat lacking. I was more looking for a primary source if it exists.

[2024-11-22 19:55] rin: And my python reading comprehension is not high.

[2024-11-22 19:56] rin: [replying to Humza: "I wrote a keyboard driver a few weeks ago if u wan..."]
Sure might be useful

[2024-11-22 20:05] rin: In my current project I parse the current kbd*.dll. I know to convert scan codes to virtual keys to wchars but right now I am somewhat confused with flags and different shift state modifiers. Its also hard to compare to virtual keys returned by windows functions because they are also sometimes incorrect. For example I had issues with wrong output with the shift state of scancode 4 in kbdfc.dll

[2024-11-24 16:03] jonaslyk: https://godbolt.org/z/qjPcra1zc
[Embed: Compiler Explorer - C (x86-64 clang (widberg))]
#define SET_FLAG(FLAG_NAME)typedef typeof(struct FLAG_NAME);
#define IF_FLAG(FLAG_NAME)__if_exists(FLAG_NAME)
#define IF_NOT_FLAG(FLAG_NAME)__if_not_exists(FLAG_NAME)

SET_FLAG(WIDE_CHAR)

IF_FLAG(WID

[2024-11-24 16:04] r0asty: [replying to jonaslyk: "https://godbolt.org/z/qjPcra1zc"]
This looks super cool

[2024-11-24 16:04] jonaslyk: new way to do conditional compilation- thats not done in the preprocessing stage but integrates with the typesystem

[2024-11-24 16:04] jonaslyk: try rem SET_FLAG- you see eax change value

[2024-11-24 16:06] jonaslyk: i havent explored everything it enables that you could not do with preprocessor yet

[2024-11-24 16:06] r0asty: How long have you been programming in C

[2024-11-24 16:06] jonaslyk: around 1 year

[2024-11-24 16:06] r0asty: I only started a couple of days ago

[2024-11-24 16:07] r0asty: I don't know what most of the code you have here does, but it looks interesting

[2024-11-24 16:07] jonaslyk: yah- it will take somme time before it make sence then

[2024-11-24 16:08] jonaslyk: its about the disconnect between preprocessing and compilation

[2024-11-24 16:09] jonaslyk: because preprocessing is done first- you cannot make choices based on any c code

[2024-11-24 16:09] jonaslyk: but this way is works in the compilation stage

[2024-11-24 16:11] r0asty: Is this necessary for a real use case or just experimental or what? I feel like this would be a use case of actual C software applications

[2024-11-24 16:12] Torph: [replying to jonaslyk: "https://godbolt.org/z/qjPcra1zc"]
interesting, didn't know about `__if_exists`. is it portable? does it let you get `#ifdef`-like behavior for checking if functions exist?

[2024-11-24 16:15] jonaslyk: portable- not sure

[2024-11-24 16:15] jonaslyk: about nr 2- yes it 100% does

[2024-11-24 16:15] jonaslyk: ill show

[2024-11-24 16:16] jonaslyk: there

[2024-11-24 16:18] jonaslyk: https://godbolt.org/z/cbee9vc6P
[Embed: Compiler Explorer - C]
#ifndef new
#	include <stdio.h>  
#	define auto __auto_type
#	define this _this(0) 
#	define new($,...)({\
		__if_exists(new_##$){ new_##$(&(typeof(*( $){}.private)){__VA_ARGS__}); }\
		__if_not_exist

[2024-11-24 16:18] jonaslyk: 
[Attachments: image.png]

[2024-11-24 16:19] jonaslyk: the way new works is that if it finds a constructor function for a datatype it will use the function and enables default values encapsulation etc.

[2024-11-24 16:20] jonaslyk: if not- you can initialise the struct normally

[2024-11-24 16:28] jonaslyk: alsso notice ive got this usage in member functions-without passing anything

[2024-11-24 16:29] jonaslyk: and its only c code

[2024-11-24 16:58] InternalHigh: template enjoyers moving more and more code into headers, slowly tanking our compile times