# May 2025 - Week 3
# Channel: #reverse-engineering
# Messages: 233

[2025-05-12 11:22] Flamingo: Hey, since ExamCookie has been updated to a C++ launcher which now stores the .NET binaries encrypted and just loads them into memory I decided to dump and deobfuscate them.

Here are all the important files: https://github.com/pinkestflamingo/ExamCookie
[Embed: GitHub - pinkestflamingo/ExamCookie: The dumped protected files of ...]
The dumped protected files of ExamCookie. Contribute to pinkestflamingo/ExamCookie development by creating an account on GitHub.

[2025-05-12 11:25] the horse: does a thing where you have AI test solving over an overlay not work on windows or something?

[2025-05-12 11:27] Flamingo: [replying to the horse: "does a thing where you have AI test solving over a..."]
What do you mean?

[2025-05-12 11:28] Flamingo: I've made a tool for that but I can't send the URL idk if its advertizing

[2025-05-12 11:28] the horse: I've seen that one asian kid apply to a bunch of jobs to promote his AI tool to pass online interviews, or cheat on tests

[2025-05-12 11:28] Flamingo: Mine is rather complicated compared to that kids tool

[2025-05-12 11:28] the horse: is WDA_EXCLUDE_FROM_CAPTURE too powerful?

[2025-05-12 11:28] Flamingo: 😂

[2025-05-12 11:28] Flamingo: Well you could do that, but it's kinda ez to detect

[2025-05-12 11:29] the horse: well i rather thought they do that

[2025-05-12 11:29] Flamingo: I take another approach

[2025-05-12 11:29] the horse: you can do 100x smarter things to hide your stuff

[2025-05-12 11:29] the horse: I wonder if they have any additional precautions like WDA_EXCLUDE_FROM_CAPTURE to prevent on-device OCR

[2025-05-12 11:29] the horse: for this type of stuff

[2025-05-12 11:29] Flamingo: SafeExamBrowser does it (in their unreleased beta)

[2025-05-12 11:30] Flamingo: Yeah what I do is hook bitblt and whenever they try to capture the screen, I only capture the exam window and replace the original screenshot with that one

[2025-05-12 11:30] Flamingo: Problem solved

[2025-05-12 11:30] the horse: imo the highest level of fuckery you can do on windows is to re-use the DRM filters, like Edge/Chrome uses on netflix

[2025-05-12 11:30] Flamingo: [replying to the horse: "imo the highest level of fuckery you can do on win..."]
That's just overkill atp

[2025-05-12 11:30] the horse: you can't capture that even with shadowplay

[2025-05-12 11:30] the horse: i'm not sure if even a capture card will get it

[2025-05-12 11:31] the horse: there's some kernel fuckery with it

[2025-05-12 11:31] diversenok: `WDA_EXCLUDE_FROM_CAPTURE`? That is not a security feature by any means

[2025-05-12 11:31] the horse: [replying to Flamingo: "That's just overkill atp"]
eh it's something you can do without a kernel anti-tamper component

[2025-05-12 11:31] diversenok: Or ypu mean DRM?

[2025-05-12 11:31] the horse: [replying to diversenok: "`WDA_EXCLUDE_FROM_CAPTURE`? That is not a security..."]
It will hide the window from screenshots/bitblt api

[2025-05-12 11:32] the horse: which would prevent on-device OCR, unless you go another level deeper of course

[2025-05-12 11:32] the horse: this is assuming you do not want to attack the software directly

[2025-05-12 11:32] the horse: attacking, be it stealing frame buffers, or injecting javascript into v8 is a lot more manageable

[2025-05-12 11:32] the horse: and harder to mitigate

[2025-05-12 11:32] Flamingo: now yall are going onto overkill rants

[2025-05-12 11:33] Flamingo: it aint that deep

[2025-05-12 11:33] the horse: eh if it stops 80% of students it's enough

[2025-05-12 11:33] the horse: high-integrity events, tests, finals, competitions

[2025-05-12 11:33] the horse: will have better security measures

[2025-05-12 11:33] the horse: (i hope)

[2025-05-12 11:33] Flamingo: They don't

[2025-05-12 11:34] the horse: perfect! time to make a business 😎

[2025-05-12 11:34] Flamingo: So far the only rather "secure" exam I've seen is Respondus Browser. Their security ain't even allat

[2025-05-12 11:34] Flamingo: [replying to the horse: "perfect! time to make a business 😎"]
dms

[2025-05-12 11:54] nox: [replying to Flamingo: "So far the only rather "secure" exam I've seen is ..."]
Respondus is shit too tbf

[2025-05-12 11:54] nox: But maybe one of the better ones out there

[2025-05-12 11:54] Flamingo: [replying to nox: "Respondus is shit too tbf"]
I like their control flow obfuscation

[2025-05-12 11:54] nox: If you do some minor effort you can run it in a VM and done.

[2025-05-12 11:54] Flamingo: yh

[2025-05-12 11:55] nox: The main reason I used to do this is because I run Linux as a daily, and didn't want to use Windows for uni

[2025-05-12 11:56] Flamingo: real

[2025-05-12 11:57] nox: I would be careful publishing this stuff though, wouldn't be the first one to get sued over this.

[2025-05-12 12:01] 25d6cfba-b039-4274-8472-2d2527cb: getting kicked out of school is definitely a possibility

[2025-05-12 12:03] nox: Or losing your degree in some cases

[2025-05-12 12:04] 25d6cfba-b039-4274-8472-2d2527cb: https://ferib.dev/blog/protecting-Packet-Tracer-myself-because-no-one-gives-a-fuck/

> Unfortunately, I have recently been expelled from school for developing a Packet Tracer Password Recovery tool, the school considered that my tool was an attempt to fraud exams. Not only my own exams, but they also accused me of helping thousands of other students to fraud their exams.

[2025-05-12 12:05] nox: Well, that went south quickly

[2025-05-12 12:14] 25d6cfba-b039-4274-8472-2d2527cb: Yeah the risks of doing anything academic dishonesty related (especially in public) tend to heavily outweigh the benefits.

[2025-05-12 12:29] Flamingo: [replying to 25d6cfba-b039-4274-8472-2d2527cb: "https://ferib.dev/blog/protecting-Packet-Tracer-my..."]
That's sad. I've made a cheat for my schools exam software which allows for ChatGPT whilst in the exam.
They instead reached out to me and asked me how they could improve the security.

[2025-05-12 12:29] Flamingo: I obv didnt use the tool myself

[2025-05-12 12:29] Flamingo: made it as a flex

[2025-05-12 19:54] Wakawakaee: Anyone knows about vac dumping? Learning it here, tryed to manual map inject my dll which detours the beginning of the GetEntryPoint function, but steam just closes. I guess its detecting it, perhaps checking the bytes? But, as i'm learning it i'm not sure. Just wondering if anyone has that info and some advice.

[2025-05-12 20:21] codeneverdies: [replying to Wakawakaee: "Anyone knows about vac dumping? Learning it here, ..."]
these might help

[2025-05-12 20:21] codeneverdies: https://cra0.net/blog/posts/archived/2015/rel-dumping-vac2-and-vac3-the-easier-way/
[Embed: Dumping VAC2 and VAC3 the easier way]
Dumping VAC2 and VAC3 the easier way

[2025-05-12 20:22] codeneverdies: https://web.archive.org/web/20240909100849/https://absceptual.me/posts/vac/
[Embed: Reversing VAC: Initalization (1)]
An introduction to the dissection of VAC.

[2025-05-12 20:22] codeneverdies: and shameless plug

[2025-05-12 20:22] codeneverdies: https://codeneverdies.github.io/posts/gh-2/
[Embed: 4. Game Hacking - Valve Anti-Cheat (VAC)]
Intro In 2002 Valve created an Anti-Cheat solution called “Valve Anti-Cheat” aka VAC. The first game they implemented VAC into was Counter-Strike. When VAC was introduced it only operated in User Mode

[2025-05-12 20:23] codeneverdies: https://whereisr0da.github.io/blog/posts/2021-03-10-quick-vac/
[Embed: Valve Anti Cheat - Part 1 : Module loading]
I already taked about VAC and how useless it is. And recently I decided to take a closer look to it, so this is my quick analysis. My goal will be to understand how VAC execute its modules, and in a P

[2025-05-13 00:07] daax: [replying to Wakawakaee: "Anyone knows about vac dumping? Learning it here, ..."]
<@1112854593613873243> I’m gonna let this play out even though we don’t normally allow discussions about game hacking; please continue to keep it civil and productive. If people can be thoughtful about it then we might allow it going forward.

[2025-05-13 03:28] codeneverdies: [replying to daax: "<@1112854593613873243> I’m gonna let this play out..."]
completely understand thanks for the heads up

[2025-05-13 11:27] Wakawakaee: [replying to codeneverdies: "these might help"]
Indeed it did. Thanks a lot. I guess since they launched the new game there aren't many modules left to load on the old version. I got it to work, but only got one module, BC7F. I'm trying to reverse it now.

[2025-05-13 11:27] Wakawakaee: So, normally these module reversals are done only static?

[2025-05-13 15:47] pinefin: [replying to Wakawakaee: "So, normally these module reversals are done only ..."]
you probably wont use dynamic as much as static if you're just diving for information on the module, if you're trying to actually see how it performs on your system, and interacts with imports/etc then you'll want dynamic. another backup if you dont want to go the dynamic route would be rebuilding routines that are used (i could be explaining this last sentence bad, i got 3 hours of sleep...my apologies)

[2025-05-13 15:49] pinefin: thus why you see console projects for be reversal (ex. "be-shellcode-tester"), it makes your life easier and you dont have to sit there watching assembly (if you're unfamiliar with it bare reversal and watching regs etc) and have a easier to perceive output for whats happening

[2025-05-13 15:49] pinefin: but like i said, you'll need static for dynamic, and you'll need dynamic to see whats happening on your machine

[2025-05-13 16:27] Xits: Anyone know of a Python runtime dumper? For dumping embedded Python

[2025-05-13 16:29] Xits: Mainly just want the byte code for each module. Not sure if that’s even held in memory or if it’s converted somehow and dropped

[2025-05-13 18:42] eternablue: [replying to Xits: "Mainly just want the byte code for each module. No..."]
not sure if that's what you want but there's an export in python.dll which is smth like Py_ExecuteFromString(), you can hook that and dup the first parameter

[2025-05-13 18:46] eternablue: `PyRun_SimpleFile()`

[2025-05-13 19:35] codeneverdies: [replying to Wakawakaee: "So, normally these module reversals are done only ..."]
I've only looked at the modules in binary ninja so far if you want help understanding what some of them do there's a repo somewhere that has them kinda reversed already

[2025-05-13 19:36] codeneverdies: maybe match up the api calls?

[2025-05-13 19:37] codeneverdies: [replying to pinefin: "you probably wont use dynamic as much as static if..."]
what pinefin said

[2025-05-13 23:33] Wakawakaee: Yeah,  tryed static analysing it  without success. I only understood some routines that handles with WinAPI calls. I'm trying to figure out what are the params passed to the modules main function to get some idea of what its doing, but i'm not going anywhere also. It seems that after steamservice.dll loads the module, it calls a function with some struct, and this structs members are passed to the modules main function. So i'm trying to debbug it to see if I can discover what the heck are those members.
[Attachments: image.png]

[2025-05-13 23:50] grb: is there any PDB parser that could let me get member offset of a type without using any external library?

[2025-05-14 00:00] f00d: [replying to Wakawakaee: "Yeah,  tryed static analysing it  without success...."]
pls stop saying tried with y

[2025-05-14 00:59] pinefin: https://youtu.be/NdJ_y1c_j_I?si=AK2KDlU5Cund8r-4
[Embed: Apple’s Widget Backdoor]
https://github.com/brycebostwick/WidgetAnimation/

This was an incredibly fun one to make. There's a lot more detail that I didn't get to cover here, and a lot of directions that only partially worked

[2025-05-14 01:16] Xits: is there a trick to break the nth time a conditon is met in x64dbg?

[2025-05-14 01:17] Xits: the $breakpointcounter increments everytime the breakpoint is hit not just when the condition is met

[2025-05-14 13:10] daax: [replying to Xits: "is there a trick to break the nth time a conditon ..."]
<@162611465130475520>

[2025-05-14 16:14] mrexodia: [replying to Xits: "the $breakpointcounter increments everytime the br..."]
It’s a reading comprehension issue

[2025-05-14 16:14] mrexodia: (Although it is confusing)

[2025-05-14 16:14] mrexodia: The counter is decimal and the expression is hexadecimal

[2025-05-14 16:15] mrexodia: So if you want to top at the 10th hit `$breakpointcounter == 0xA`

[2025-05-14 16:15] mrexodia: (Or you can use `== .10`)

[2025-05-14 16:42] Xits: This is the breakpoint. It is the 9th time createfile is called on "djd.bin". I need to set $breakpointcounter == .231 to catch the 9th time but that only works because the program is deterministic to that point
[Attachments: xxxx.png]

[2025-05-14 19:40] Sleepy: while doing some research I came a nice write-up by cocomelonc and decided to create my own working version check  it out this gives an example of a Clipboard grabber in C https://github.com/sleepyG8/Clip-BoardKiller
[Embed: GitHub - sleepyG8/Clip-BoardKiller: A program I wrote to get the Cl...]
A program I wrote to get the Clipboard data from the computer and save it to a file in just over 100 lines of code. I wrote this after reading a blog post from https://cocomelonc.github.io and I de...

[2025-05-14 20:21] dlima: [replying to Sleepy: "while doing some research I came a nice write-up b..."]
nice, good stuff

[2025-05-14 20:21] dlima: 
[Attachments: fedsfsdfsdf.PNG]

[2025-05-14 20:21] dlima: But bro what is this indenting

[2025-05-14 20:21] dlima: 😭

[2025-05-14 20:21] Brit: Clang format is an arcane technology

[2025-05-14 20:28] contificate: [replying to Sleepy: "while doing some research I came a nice write-up b..."]
You could factor out the common operations here, each of these branches seems to differ by only the mode in which the file is opened. https://github.com/sleepyG8/Clip-BoardKiller/blob/main/Clip-Killer.c#L33-L74

[2025-05-14 21:15] pinefin: [replying to dlima: ""]
in other parts, hes indenting by 1 space, sometimes 2, and sometimes 3... eventually he has a few with 4!

[2025-05-14 21:16] pinefin: i cant tell if this was wrote on phone? cause ive for sure wrote code on my phone like this before

[2025-05-14 21:17] pinefin: and not meaning to critique in a bad way, im just wondering why its so sporadic and not abiding by a format

[2025-05-14 23:26] Sleepy: [replying to dlima: ""]
It’s how vscode indents my projects if it’s indented, that means it’s a part of the larger function

[2025-05-14 23:26] Sleepy: Thanks btw 👍

[2025-05-15 00:12] pinefin: [replying to Sleepy: "It’s how vscode indents my projects if it’s indent..."]
oh huh 🤔 does it change when you’re looking in the editor vs when you push it to github?

[2025-05-15 03:12] Wakawakaee: It's probably just using spaces instead of tabs

[2025-05-15 14:20] pinefin: [replying to Wakawakaee: "It's probably just using spaces instead of tabs"]
thats what im assuming

[2025-05-15 14:20] pinefin: not to hyperfixate on it

[2025-05-16 01:51] Wakawakaee: A HANDLE to a Thread = pointer to ETHREAD struct?

[2025-05-16 01:56] Wakawakaee: I have this code on ida: *pTargetHandle + 52.  I wonder what offset 52 would be

[2025-05-16 02:06] Matti: [replying to Wakawakaee: "A HANDLE to a Thread = pointer to ETHREAD struct?"]
that's what it translates to eventually yeah if it's a valid handle with the necessary access mask

[2025-05-16 02:06] Matti: [replying to Wakawakaee: "I have this code on ida: *pTargetHandle + 52.  I w..."]
either your types or your variable names in IDA are fucked

[2025-05-16 02:08] Matti: a HANDLE is not a pointer

[2025-05-16 02:09] Wakawakaee: The comment on the function definition is this: int (__stdcall *GetThreadStartFunction())(HANDLE TargetHandle)

[2025-05-16 02:09] Wakawakaee: int *pTargetHandle
pTargetHandle = (int'*') TargetHandle

[2025-05-16 02:09] Wakawakaee: Inside it does that

[2025-05-16 02:10] Matti: right, so the type is most likely `HANDLE*`

[2025-05-16 02:10] Wakawakaee: Idk but discord is not shows the asterisc in (int*)

[2025-05-16 02:10] Matti: and the cast to int is IDA being retarded

[2025-05-16 02:10] Matti: use \` \`

[2025-05-16 02:11] Wakawakaee: [replying to Matti: "use \` \`"]
?

[2025-05-16 02:11] Matti: ugh

[2025-05-16 02:11] Matti: I'm guessing you meant
`pTargetHandle = *(int*)TargetHandle;`?

[2025-05-16 02:11] Wakawakaee: without the dereference

[2025-05-16 02:12] Matti: well then whence the fuck comes the p prefix

[2025-05-16 02:12] Wakawakaee: well, that was my idea

[2025-05-16 02:12] Matti: ok, undo it

[2025-05-16 02:13] Wakawakaee: Well, isnt it a pointer, hence (int'')

[2025-05-16 02:14] Matti: [replying to Wakawakaee: "?"]
`wrap things that confuse discord in these characters`

[2025-05-16 02:15] Matti: \`\`

[2025-05-16 02:15] Matti: [replying to Wakawakaee: "Well, isnt it a pointer, hence (int'')"]
what is? says who?

[2025-05-16 02:16] Matti: your entire series of questions is lacking some context me thinks

[2025-05-16 02:16] Wakawakaee: 
[Attachments: image.png, image.png]

[2025-05-16 02:17] Wakawakaee: Now, inside the function:

[2025-05-16 02:17] Wakawakaee: `int *pTargetHandle; // esi
pTargetHandle = (int *)hDuplicatedThread`

[2025-05-16 02:19] Matti: right

[2025-05-16 02:19] Matti: who added the asterisk and the p prefix

[2025-05-16 02:19] Matti: was it you

[2025-05-16 02:19] Matti: why

[2025-05-16 02:19] Matti: a HANDLE is not a pointer

[2025-05-16 02:19] Wakawakaee: I just renamed the variable name...

[2025-05-16 02:19] Wakawakaee: The pointer was added by ida

[2025-05-16 02:20] Matti: then that one is on IDA

[2025-05-16 02:20] Matti: congrats, both half the blame

[2025-05-16 02:20] Matti: a HANDLE is just a pointer sized integer

[2025-05-16 02:21] Matti: it doesn't point to anything (not in the way you think, at least)

[2025-05-16 02:23] Wakawakaee: So why after being passed to other fuctions it's used as this: `(*(int (__thiscall **)(_DWORD *))(*pThreadHandle + 8))(pThreadHandle)`

[2025-05-16 02:23] Matti: because IDA fucked up a type

[2025-05-16 02:24] Wakawakaee: Hmm

[2025-05-16 02:24] Matti: is there maybe a second function pointer in .data/.rdata following the one that is returned in eax

[2025-05-16 02:24] Matti: if not, then it simply got the entire function type wrong

[2025-05-16 02:25] Matti: if yes, it's probably calling function [2] in an array of N

[2025-05-16 02:25] Matti: given that this is 32 bit shite

[2025-05-16 02:26] Wakawakaee: All references to it are from vtables
[Attachments: image.png]

[2025-05-16 02:26] Matti: no, double click it

[2025-05-16 02:26] Matti: go to the address

[2025-05-16 02:26] Matti: and see what follows it

[2025-05-16 02:26] Wakawakaee: 
[Attachments: image.png]

[2025-05-16 02:27] Wakawakaee: Already did

[2025-05-16 02:27] Wakawakaee: Other types have the same pointer to that function

[2025-05-16 02:27] Matti: you cropped the screenshot exactly at the wrong place

[2025-05-16 02:27] Matti: but so it looks like it is an array of function pointers

[2025-05-16 02:28] Wakawakaee: 
[Attachments: image.png]

[2025-05-16 02:28] Matti: yep

[2025-05-16 02:28] Matti: still an array to me

[2025-05-16 02:29] Wakawakaee: Whats the conclusion here, im lost

[2025-05-16 02:29] Matti: [replying to Matti: "if yes, it's probably calling function [2] in an a..."]
.

[2025-05-16 02:30] Wakawakaee: Could it be the this pointer?

[2025-05-16 02:30] Matti: except it's more like [X] because your function is not guaranteed to be index 0

[2025-05-16 02:30] Matti: could what be? but yes in general that'd be a good guess for the first argument

[2025-05-16 02:31] Matti: if dealing with vtables

[2025-05-16 02:33] Wakawakaee: Guess ill have to reverse the caller function, the one that calls that one

[2025-05-16 02:33] Matti: that's how it goes

[2025-05-16 02:34] Wakawakaee: At somepoint ill have to get to the Main func

[2025-05-16 02:34] Wakawakaee: To crack this

[2025-05-16 02:34] Matti: just repeat the process until you have the full source code

[2025-05-16 02:34] Matti: you can stop earlier if you want but it's not needed

[2025-05-16 16:21] Ђинђић: Can someone give me good advice with how to adjust ghidra to get the maximum out of .dll when reversing?

[2025-05-16 16:23] pinefin: [replying to Ђинђић: "Can someone give me good advice with how to adjust..."]
what?

[2025-05-16 16:23] pinefin: can you elaborate on what you're asking

[2025-05-16 16:23] Ђинђић: Can ghidra be set to work better with some files?

[2025-05-16 16:23] pinefin: i still dont get what you're asking, you mean as in speed? format? layout? functionality?

[2025-05-16 16:23] Ђинђић: I took a part of rozniaks winXP Internet Games Zone server reversal

[2025-05-16 16:24] Ђинђић: And noticed that he could get some code or reference from one .dll here i couldnt.

[2025-05-16 16:25] Ђинђић: [replying to pinefin: "i still dont get what you're asking, you mean as i..."]
Functionality.
I think i cant set my ghidra to do deeper reversal and decoding

[2025-05-16 21:19] Shanks: [replying to Ђинђић: "Functionality.
I think i cant set my ghidra to do ..."]
I think you need a debugger like ollydbg to get this deep level.

[2025-05-16 22:41] david: Hello, I have a fairly large PE binary. I've always used Ghidra for reversing. The problem is that Ghidra takes quite a long time to perform the automatic analysis. How can I speed up the process without significantly losing analysis quality? Are there any settings I can adjust?

[2025-05-16 22:41] david: 
[Attachments: Screenshot_from_2025-05-16_18-37-13.png]

[2025-05-17 06:48] Ђинђић: [replying to Shanks: "I think you need a debugger like ollydbg to get th..."]
Okay but does it give everything sorted or understandable like ghidra?

[2025-05-17 10:57] elias: [replying to Ђинђић: "Okay but does it give everything sorted or underst..."]
no

[2025-05-17 10:59] Ђинђић: How will i know what is where and what is decoded?

[2025-05-17 11:24] elias: [replying to Ђинђић: "How will i know what is where and what is decoded?"]
your questions are really vague, its hard to give you good answers with that

[2025-05-17 11:24] 25d6cfba-b039-4274-8472-2d2527cb: also any recommendation to use ollydbg is bait, just a heads up

[2025-05-17 11:25] 25d6cfba-b039-4274-8472-2d2527cb: 😄

[2025-05-17 11:26] elias: [replying to david: "Hello, I have a fairly large PE binary. I've alway..."]
I recommend binja and IDA over ghidra. IDA has the fastest processing from my experience for large binaries

[2025-05-17 11:44] Brit: what you mean by fastest is slightly less agonisingly slow than the rest

[2025-05-17 11:45] Brit: but also im not even convinced in it being fastest

[2025-05-17 11:45] 25d6cfba-b039-4274-8472-2d2527cb: [replying to Ђинђић: "And noticed that he could get some code or referen..."]
but yea i'd say
1. Aggressive instruction finder in analysis options
2. Ensure the binary is not packed, if it is dump it from runtime
3. Check the places there should be references to manually and ensure you're not chasing something not there

In my experience Ghidra can easily miss things like vfuncs

[2025-05-17 12:00] david: [replying to elias: "I recommend binja and IDA over ghidra. IDA has the..."]
I let it run overnight. I might still take a look at IDA.

[2025-05-17 12:12] Ђинђић: [replying to 25d6cfba-b039-4274-8472-2d2527cb: "but yea i'd say
1. Aggressive instruction finder i..."]
Really? So i should set up all that in settings or analysis options? I just want ghidra to find and extract and decode everything that exists if possible..

[2025-05-17 19:46] Wane: Time to analyze `ntoskrnl.exe`
IDA 1m 59s
Ghidra ~7m
Binary Ninja ~5m

[2025-05-17 19:57] brymko: not bad for all of them

[2025-05-17 20:42] Brit: [replying to Wane: "Time to analyze `ntoskrnl.exe`
IDA 1m 59s
Ghidra ~..."]
and now pull the pdb and check which had the best func recovery

[2025-05-17 20:43] Wane: IDA was significantly first of the race

[2025-05-17 20:45] mrexodia: https://tenor.com/view/developer-we-have-a-bug-gif-9159002844106963379

[2025-05-17 20:45] 25d6cfba-b039-4274-8472-2d2527cb: it's been a while since I've used IDA and Binja, how's their capabilities to migrate stuff from old to newer bin version?

[2025-05-17 20:54] brymko: [replying to Wane: "IDA was significantly first of the race"]
yeah and binja is not multi threaded and ghidra runs on java

[2025-05-17 20:55] brymko: but sure we care about 4min diff in a multi day reversing session

[2025-05-17 20:55] brymko: fun fact from jordan

[2025-05-17 20:55] brymko: if you name a function exit in your code

[2025-05-17 20:55] brymko: ida stops decompling after that function

[2025-05-17 21:10] Yoran: [replying to brymko: "ida stops decompling after that function"]
?

[2025-05-17 21:10] Yoran: What does that mean?

[2025-05-17 21:11] brymko: ida stops decompling after seeing that function

[2025-05-17 21:12] Yoran: [replying to brymko: "ida stops decompling after seeing that function"]
On my IDA they have a pop up that asks if you want to continue decompiling

[2025-05-17 21:14] Redhpm: you can just edit the function and uncheck "non returning"

[2025-05-17 21:15] GG: [replying to brymko: "ida stops decompling after seeing that function"]
lmao
[Attachments: 2025-05-18_00-15-50.mp4]

[2025-05-17 21:16] GG: I didn't know this

[2025-05-17 21:17] Yoran: Oh I thought you meant something else

[2025-05-17 21:19] GG: [replying to Redhpm: "you can just edit the function and uncheck "non re..."]
yes that works !

[2025-05-17 21:19] GG: ida retypes it to `void __cdecl __noreturn exit(int Code);`, just remove `__noreturn`

[2025-05-17 21:20] GG: I wouldn't consider this to be a problem tbh, its just IDA trying to be a little bit smart

[2025-05-17 21:20] Redhpm: heuristics, sometimes it works, sometimes not... that's just the deal

[2025-05-17 21:25] TheRealHarold: [replying to brymko: "yeah and binja is not multi threaded and ghidra ru..."]
What do you mean with binja not being multi threaded?
From what I can see binja is multi threaded for non commercial version since 3.1 https://binary.ninja/2022/05/31/3.1-the-performance-release.html#non-commercial-multithreading
Current free version has it too with no limit on threads https://binary.ninja/free/

[2025-05-17 21:27] Redhpm: [replying to brymko: "yeah and binja is not multi threaded and ghidra ru..."]
ghidra's UI is java. Analysis is c++. The worst code you could lay eyes on but still.. cpp

[2025-05-17 21:27] GG: [replying to Redhpm: "ghidra's UI is java. Analysis is c++. The worst co..."]
I mean its the worst UI too

[2025-05-17 21:27] Redhpm: true

[2025-05-17 21:27] Redhpm: but hey, it's free

[2025-05-17 21:27] Redhpm: so there's that

[2025-05-17 21:28] GG: maybe because its free

[2025-05-17 21:28] brymko: hm interesting they changed that

[2025-05-17 21:28] brymko: [replying to GG: "maybe because its free"]
only took 10 billion budget to develop

[2025-05-17 21:31] GG: my only complain with IDA is scripting

[2025-05-17 21:31] GG: its very unpleasant to use

[2025-05-17 21:32] GG: I can usually do what I want to do with it, but its just bad