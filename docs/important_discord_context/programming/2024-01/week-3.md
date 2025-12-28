# January 2024 - Week 3
# Channel: #programming
# Messages: 471

[2024-01-15 16:55] nun: [replying to bocor: "hardware specs or design wise?"]
pretty sure he meant feature/design wise

[2024-01-15 16:56] nun: for example if its just a static little website then a framework like nextjs would be completely overkill and unnecessary

[2024-01-15 17:12] 25d6cfba-b039-4274-8472-2d2527cb: ah ye forgot to respond. definitely would say a js framework is quite overkill unless you need dynamic content fetched from some backend or some frontend DOM manipulation.

I'd definitely stick to something like Hugo with possibly a css framework of your choice if you're not proficient with plain css. Templated HTML to generate a static site should get you far in terms of handling the site content and making it look presentable.

[2024-01-16 05:10] Horsie: Any Linux people here know how to dynamically obtain exports from other kernel modules?

[2024-01-16 05:19] brymko: 
[Attachments: image.png]

[2024-01-16 05:34] contificate: just as god intended

[2024-01-16 05:34] contificate: I know that meme concerns learning

[2024-01-16 05:34] contificate: but a large part of being an adult

[2024-01-16 05:34] contificate: is realising you shouldn't use many languages for many domains

[2024-01-16 05:34] contificate: and many types of programs

[2024-01-16 05:35] contificate: like imagine hamfisting a beast websocket thing in 400 lines

[2024-01-16 05:35] contificate: and getting mogged by some 6 line python script that works better

[2024-01-16 06:52] Horsie: [replying to brymko: ""]
CMKR CMKR CMKR CMKR

[2024-01-16 21:03] Torph: [replying to brymko: ""]
oh my god I didn't realize premake generates CMake... I assumed it used Lua to be cross-platform and was just an alternative to CMake that generated native platform build files

[2024-01-16 22:18] Ling: [replying to Torph: "oh my god I didn't realize premake generates CMake..."]
it doesnt use cmake

[2024-01-16 22:18] Ling: https://github.com/premake/premake-core/blob/master/modules/vstudio/vs2010_vcxproj.lua
[Embed: premake-core/modules/vstudio/vs2010_vcxproj.lua at master · premake...]
Premake. Contribute to premake/premake-core development by creating an account on GitHub.

[2024-01-16 22:28] Torph: oh thank god

[2024-01-16 23:56] Deleted User: I forked an existing cmake generator for premake to use with vscode, premake is quite nice

[2024-01-17 15:07] luci4: How does the Cobalt Strike keylogger associate key strokes with the active window at the time? Does it just use `GetActiveWindow()`, or is there a better API function for that?
[Attachments: image.png]

[2024-01-17 15:13] luci4: So there is `GetFocus()` from what I found so far

[2024-01-17 15:19] contificate: I don't do Windows programming or know how the keylogger is working but I do know that Windows dispatches messages for events which processes can poll for (so, clearly, there is a mechanism in Windows for doing this mapping; ascribing events to correct thread's message queue - and since it will know which thread is doing GUI stuff, it's safe to assume they're probably benefiting from this inherent design choice).

[2024-01-17 15:24] Timmy: it's probably a low level keyboard hook through SetWindowsHookEx and it would probably get the window it was typed into from that message hook

[2024-01-17 15:25] Timmy: It could also use GetForegroundWindow and/or GetFocus to see what's being typed in to

[2024-01-17 15:25] contificate: like all things I know in Windows

[2024-01-17 15:26] contificate: if it's used by malware

[2024-01-17 15:26] contificate: it's probably used by legitimate Windows programs

[2024-01-17 15:26] contificate: 
[Attachments: 2024-01-17-152528_547x375_scrot.png]

[2024-01-17 15:26] contificate: so many programs (VS tool attached) can just readily dump the message queue

[2024-01-17 15:38] luci4: [replying to Timmy: "it's probably a low level keyboard hook through Se..."]
It could use SetWindowsHookEx with a callback function that will be called when a key event occurs?

[2024-01-17 15:38] Timmy: yup

[2024-01-17 15:41] luci4: Yeah that makes a lot of sense, the docs say `Beacon must live inside of a process associated with the current desktop. explorer.exe is a good candidate`

[2024-01-17 15:46] luci4: I wonder how they combined with `GetFocus()`. It being part of the callback function seems kinda inefficient (?)

[2024-01-17 15:56] Matti: well the GetFocus() can be used to determine whether the window with keyboard focus is being processed by the current thread

[2024-01-17 15:57] Matti: so, it could be used to check whether the window is "mine" or not

[2024-01-17 15:57] Matti: that's just a guess but it'd be a plausible reason I think

[2024-01-17 15:59] Matti: note that there's also `AttachThreadInput` to forcefully attach the input processing to your own thread's message queue instead

[2024-01-17 16:00] Matti: IMO it is the most evil win32k function out of them all

[2024-01-17 16:00] luci4: 😮

[2024-01-17 16:01] luci4: How would that work? Would my thread process all keystrokes?

[2024-01-17 16:02] Matti: well, I don't know if there's an intended scenario where it works

[2024-01-17 16:02] Matti: as in does something useful

[2024-01-17 16:02] Matti: the reason I know about its existence is this nightmare bug https://github.com/x64dbg/x64dbg/issues/1481
[Embed: x32dbg hangs itself and the debugee  · Issue #1481 · x64dbg/x64dbg]
Debugger version: x32dbg v25 (Compiled on: Feb 28 2017, 05:07:48) Operating system version and Service Pack (including 32 or 64 bits): Windows 7 x64 Service Pack 1 Brief description of the issue: W...

[2024-01-17 16:03] Matti: this turned out to be anti-debugging, by hanging the debugger process

[2024-01-17 16:03] luci4: Never heard of that technique before

[2024-01-17 16:03] Matti: (a deadlock occurs because the debuggee is processing the debugger's input, but the debugger can't let the debuggee run)

[2024-01-17 16:04] Matti: because its input is being processed by the debuggee....

[2024-01-17 16:05] Matti: <https://github.com/x64dbg/x64dbg/issues/1481#issuecomment-285231185> here is the short(ish) explanation and fix

[2024-01-17 16:06] Matti: or workaround

[2024-01-17 16:06] Matti: but yeah

[2024-01-17 16:09] luci4: [replying to Matti: "well, I don't know if there's an intended scenario..."]
Maybe you could do something like:
- get currently focused window
- get the window thread of it's respective process
- attach your thread to it
- somehow log the keystrokes

[2024-01-17 16:10] luci4: Oh it does work, I found a PoC

[2024-01-17 16:10] luci4: It's in Go though

[2024-01-17 16:10] Matti: why would that be preferable to simply hooking the keyboard event?

[2024-01-17 16:10] Matti: it's an insane design

[2024-01-17 16:13] luci4: Yeah that's fair enough.

[2024-01-17 16:15] Matti: raymond chen  agrees with me! https://web.archive.org/web/20190209010022/https://blogs.msdn.microsoft.com/oldnewthing/20130619-00/?p=4043
[Embed: AttachThreadInput is like taking two threads and pooling their mone...]
Consider this code: // Code in italics is wrong foregroundThreadId = ::GetWindowThreadProcessId(::GetForegroundWindow(), 0); myThreadId = GetCurrentThreadId(); if (foregroundThreadId != myThreadId) { 

[2024-01-17 16:17] luci4: [replying to luci4: "It's in Go though"]
Sidenote: I've seen a lot of Go/Rust malware, idk why people prefer it over C though

[2024-01-17 16:17] Matti: well if by PoC you mean code, then idc really

[2024-01-17 16:17] Matti: I can probably read go

[2024-01-17 16:18] Matti: at least enough to grasp what the idea is

[2024-01-17 16:18] luci4: [replying to Matti: "well if by PoC you mean code, then idc really"]
https://github.com/vgo0/gologger
[Embed: GitHub - vgo0/gologger: Simple proof of concept Windows Go keylogge...]
Simple proof of concept Windows Go keylogger via conventional Windows APIs (SetWindowsHookEx low level keyboard hook) - GitHub - vgo0/gologger: Simple proof of concept Windows Go keylogger via conv...

[2024-01-17 16:18] Matti: if you mean a binary then yeaahhhh

[2024-01-17 16:18] luci4: This is the one I found

[2024-01-17 16:19] brymko: literally just using winapi.SetWindowsHookEx

[2024-01-17 16:19] brymko: must be insanely difficult to port to that literally any other language with c bindings

[2024-01-17 16:21] Matti: > Window change catching leverages AttachThreadInput to new focused windows to allow for acquisition of keyboard state from within the keylogger thread
*leverages*
like collecting *more* input queues is better

[2024-01-17 16:22] Matti: gotta catch em all!

[2024-01-17 16:23] Timmy: [replying to Matti: "IMO it is the most evil win32k function out of the..."]
because win32 doesn't allow you to set a window to the top and focus it with the keyboard if you don't own the window in some way

[2024-01-17 16:23] Timmy: we have to use this

[2024-01-17 16:23] Matti: one thing raymond doesn't answer is why this API exists

[2024-01-17 16:23] Timmy: ```cpp
const auto windowThreadProcessId = GetWindowThreadProcessId(GetForegroundWindow(), nullptr);
const auto currentThreadId = GetCurrentThreadId();
AttachThreadInput(windowThreadProcessId, currentThreadId, TRUE);
BringWindowToTop(window);
ShowWindow(window, SW_SHOW);
AttachThreadInput(windowThreadProcessId, currentThreadId, FALSE);
```

[2024-01-17 16:24] Matti: [replying to Timmy: "because win32 doesn't allow you to set a window to..."]
what would be a use case for this

[2024-01-17 16:24] Timmy: accessibility software

[2024-01-17 16:24] Timmy: a window manager

[2024-01-17 16:25] Timmy: software testing and automation

[2024-01-17 16:25] Timmy: stuff like that

[2024-01-17 16:27] Matti: accessibility software I could maybe buy, but wasn't there an API collection introduced in win 10 specifically to allow doing this sort of thing? but sanely?
window manager - in windows there's dwm, and I really doubt it uses win32 APIs to do this
my bet is on some lower level win32k call that is also disgusting but in a different way

[2024-01-17 16:28] Timmy: > but wasn't there an API collection introduced in win 10 specifically to allow doing this sort of thing
What kind of api are you referring to? the automation or the window management?

[2024-01-17 16:28] Matti: [replying to Timmy: "software testing and automation"]
you can make your test runner own the windows (and their input queues if need be), no

[2024-01-17 16:28] Matti: window management

[2024-01-17 16:28] Matti: unfortunately I doubt I'm gonna be able to find the link for this

[2024-01-17 16:29] Matti: I only barely recall reading about it, from someone who RE'd this API

[2024-01-17 16:29] x86matthew: [replying to Matti: "one thing raymond doesn't answer is why this API e..."]
mostly for hacks yes, this should never have been a documented api imo

[2024-01-17 16:29] x86matthew: and it gets worse

[2024-01-17 16:30] Timmy: [replying to Matti: "window management"]
I've looked and not found 😦

[2024-01-17 16:30] x86matthew: some software devs in the early 2000s wanted to prevent other applications from attaching to their input queue

[2024-01-17 16:31] Timmy: ms' accessibility api is great tho, been around since xp iirc and even works on browsers

[2024-01-17 16:31] x86matthew: so of course they injected code into all other running processes which hooked AttachThreadInput to prevent it from being used

[2024-01-17 16:31] Timmy: great solution <:KEKW:798912872289009664>

[2024-01-17 16:32] Matti: I mean, I don't disagree

[2024-01-17 16:32] Matti: that's what MS should have done

[2024-01-17 16:32] Matti: removed it from win32k

[2024-01-17 16:32] Timmy: 100%, but they kind would have to fix their other windowing api's as well

[2024-01-17 16:33] Matti: but yeah of course if random companies start doing this just to get around the bullshit API, it's too late

[2024-01-17 16:33] x86matthew: ah yes some discussion about it here from 17 years ago: https://microsoft.public.win32.programmer.ui.narkive.com/fQCzXTEf/preventing-attachthreadinput

[2024-01-17 16:33] Timmy: imo in cases like this they shouldn't give a fk and break compatibility to fix their OS XD

[2024-01-17 16:34] Matti: I've killed this in win32k in matti WRK

[2024-01-17 16:34] Matti: just out of spite, cause I spent 6 days on that bug above

[2024-01-17 16:35] Matti: I guess titanhide could be extended to patch this as well

[2024-01-17 16:36] luci4: I wonder if something like this would work
[Attachments: image.png]

[2024-01-17 16:36] Matti: [replying to x86matthew: "ah yes some discussion about it here from 17 years..."]
> Additionally, hooking AttachThreadInput is also not a ideal solution based
> on your purpose. This is because an application can also leverage
> WH_JOURNALPLAYBACK hook and follow with SetForegroundWindow to achieve the
> same effect.
hm oh yeah, I remember <@162611465130475520> finding this as well at the time

[2024-01-17 16:37] luci4: I will probably store the last focused window handle in a global variable

[2024-01-17 16:37] Matti: I haven't tried this, idk if it will fix the exact scenario <@83203731226628096> describes

[2024-01-17 16:38] Timmy: 
[Attachments: image.png]

[2024-01-17 16:38] Matti: lol who cares

[2024-01-17 16:38] Matti: better to use an unsupported API than an evil API

[2024-01-17 16:39] Timmy: doesn't unsupported mean 'not available'

[2024-01-17 16:39] Matti: no

[2024-01-17 16:39] Matti: just means if there are bugs, MS won't care

[2024-01-17 16:39] Matti: like NTFS transactions

[2024-01-17 16:39] Matti: they still work

[2024-01-17 16:39] Matti: MS just wants them dead

[2024-01-17 16:39] Timmy: tbh

[2024-01-17 16:40] Timmy: I think we should add SetWindowsHookEx to the evil api list

[2024-01-17 16:40] Torph: [replying to luci4: "Sidenote: I've seen a lot of Go/Rust malware, idk ..."]
go is a pain in the ass to RE compared to C, but it's not that bad with a Ghidra plugin. but like it's actually difficult to find the `main` function if you don't have a plugin to handle it or aren't familiar with Go binaries

[2024-01-17 16:40] Matti: [replying to Timmy: "I think we should add SetWindowsHookEx to the evil..."]
oh I agree

[2024-01-17 16:40] luci4: Ah I see

[2024-01-17 16:40] Matti: I just don't think it's anywhere near as evil as AttachThreadInput

[2024-01-17 16:40] luci4: [replying to Matti: "oh I agree"]
Its on malapi.io

[2024-01-17 16:41] Matti: I mean I think most of win32k is evil

[2024-01-17 16:41] Matti: so the bar is pretty low

[2024-01-17 16:41] Torph: what's win32k? just 32-bit Windows?

[2024-01-17 16:42] Matti: win32k is the kernel mode component that does drawing and windowing stuff (well drawing is GDI I think)

[2024-01-17 16:42] Matti: it's user32.dll, but the kernel mode side

[2024-01-17 16:42] Torph: oh ok

[2024-01-17 16:42] Torph: so "win32-kernel" not "windows 32000" or "Windows 3.0 2000"

[2024-01-17 16:43] Matti: no the k is just "k" <:lillullmoa:475778601141403648>

[2024-01-17 16:43] Matti: kay

[2024-01-17 16:43] x86matthew: [replying to luci4: "Its on malapi.io"]
i wouldn't base anything on that, looks like pretty much everything is on that list

[2024-01-17 16:43] x86matthew: Sleep? malware

[2024-01-17 16:43] x86matthew: GetMessageA? "spying"

[2024-01-17 16:44] Matti: [replying to Torph: "so "win32-kernel" not "windows 32000" or "Windows ..."]
though, your explanation does make sense retroactively I guess
since there's win32u.dll now

[2024-01-17 16:44] luci4: [replying to x86matthew: "i wouldn't base anything on that, looks like prett..."]
Well it says what malicious usage it could have

[2024-01-17 16:44] luci4: 
[Attachments: image.png]

[2024-01-17 16:44] brymko: [replying to x86matthew: "GetMessageA? "spying""]
crypto? state sponsored

[2024-01-17 16:44] Torph: MapViewOfFile is in the injection section?? isn't that for mapping files to virtual memory??

[2024-01-17 16:45] Timmy: [replying to Matti: "I just don't think it's anywhere near as evil as A..."]
I'm still debating this in my head, I've go the feeling that registering a global user input hook is quite drastic in order to just move a window to the foreground. I think I like the isolation of the ThreadAttachInput workaround better in this specific instance

[2024-01-17 16:45] brymko: [replying to luci4: "Well it says what malicious usage it could have"]
"malicious usage" in this case is literally "bypassing" incompetent defenders

[2024-01-17 16:45] Timmy: Just because it's only in effect for a very brief time

[2024-01-17 16:46] brymko: [replying to Torph: "MapViewOfFile is in the injection section?? isn't ..."]
and what do you think you can do with a file being mapped into mem ???

[2024-01-17 16:46] Torph: GetSystemTime? is anti-debugging??

[2024-01-17 16:46] brymko: tracking for breakpoints

[2024-01-17 16:46] brymko: why do i even have to explain all this

[2024-01-17 16:46] Torph: [replying to brymko: "and what do you think you can do with a file being..."]
I mean I can understand how it could be useful for injection but it has a lot of normal use cases

[2024-01-17 16:47] brymko: exaclty matthews point

[2024-01-17 16:47] Matti: [replying to Timmy: "I'm still debating this in my head, I've go the fe..."]
to be clear I can't defend either design obviously, both are horrible IMO
but AttachThreadInput **allows a debuggee to hang the debugger**
that is not just a recipe for disaster (like a global hook), that is an actual disaster

[2024-01-17 16:47] Torph: [replying to brymko: "tracking for breakpoints"]
oh ok

[2024-01-17 16:49] Matti: <@83203731226628096>: I have an alternative design proposal

[2024-01-17 16:49] Matti: queue a special kernel APC to the thread owning the input queue

[2024-01-17 16:50] Matti: I just remembered I did this for https://github.com/NSG650/NtDOOM
[Embed: GitHub - NSG650/NtDOOM: Doom running in the NT kernel]
Doom running in the NT kernel. Contribute to NSG650/NtDOOM development by creating an account on GitHub.

[2024-01-17 16:51] Matti: it's not global, and it doesn't affect user mode debuggers since you're running in kernel mode anyway

[2024-01-17 16:51] Matti: perfect!

[2024-01-17 16:51] luci4: [replying to Matti: "to be clear I can't defend either design obviously..."]
So it's a good anti-debugging measure?

[2024-01-17 16:51] Matti: not really

[2024-01-17 16:51] luci4: Oh just annoying then

[2024-01-17 16:51] Matti: there's no guarantee that the debugger window will be the foreground window

[2024-01-17 16:51] Matti: it's just likely

[2024-01-17 16:52] Matti: that's why the bug I linked was not always reproducible

[2024-01-17 16:54] Timmy: I mean the kernel driver itself is global

[2024-01-17 16:54] Timmy: it's both better and more horrible at the same time haha

[2024-01-17 16:55] Timmy: why wouldn't a user apc work?

[2024-01-17 16:55] Matti: nah, by that logic your user mode process is also global

[2024-01-17 16:55] Matti: the effect is local

[2024-01-17 16:55] Timmy: fair point

[2024-01-17 16:55] Matti: [replying to Timmy: "why wouldn't a user apc work?"]
I think I may have ended up using a user APC in the end actually

[2024-01-17 16:55] Matti: not 100% sure, lemme check

[2024-01-17 16:56] Matti: yeah it is a user APC - <https://github.com/NSG650/NtDOOM/pull/2#issuecomment-1565371056>

[2024-01-17 16:56] Matti: or commit: <https://github.com/NSG650/NtDOOM/pull/2/commits/9f4ea7ca14e233243ea68b1fab604ab393308147>

[2024-01-17 16:58] Matti: so for whatever reason, if using a kernel APC it actually worked fine.... for a little while

[2024-01-17 16:58] Matti: and then stopped working

[2024-01-17 17:14] Torph: [replying to Matti: "I just remembered I did this for https://github.co..."]
this is wild

[2024-01-17 17:17] Matti: yeah it's an awesome repo

[2024-01-17 17:17] luci4: A giant switch statement wouldn't be more efficient than this, right?
[Attachments: image.png]

[2024-01-17 17:18] contificate: it could be

[2024-01-17 17:18] luci4: [replying to luci4: "How does the Cobalt Strike keylogger associate key..."]
I wonder how they made it like that without running like a slog

[2024-01-17 17:18] contificate: really depends on the value ranges and overall `n`

[2024-01-17 17:19] contificate: like suppose those vkCodes were contiguous, then it'd be O(1) to do the mapping, as you'd simply index an array

[2024-01-17 17:20] contificate: what usually happens with a switch under LLVM is that it tries to sort the cases w/ a bottom-up algorithm and produces a mix of binary search and jump tables

[2024-01-17 17:20] contificate: whereas there's also compile time tools that could compute a perfect hash for you

[2024-01-17 17:21] contificate: comes down to constant factors, linearly searching a small array will be faster than binary searching it for some `n`, and either will be faster than most hash table implementations, etc.

[2024-01-17 17:21] luci4: [replying to contificate: "really depends on the value ranges and overall `n`"]
Well there are 256 Virtual Key codes

[2024-01-17 17:21] contificate: then you can just have a full map/have empty indices

[2024-01-17 17:22] contificate: but obviously it means you provide them in order

[2024-01-17 17:22] contificate: like a large array of only the names, preferably

[2024-01-17 17:24] contificate: WinAPI already has a function to do the mapping, I gather

[2024-01-17 17:24] luci4: Only the names? Wouldn't it be harder to print the key like that?

[2024-01-17 17:24] contificate: no, since you'd be expecting the key code to index that array

[2024-01-17 17:24] contificate: but it doesn't really matter, merely needs to be in order

[2024-01-17 17:24] contificate: the thing is

[2024-01-17 17:25] contificate: I'm unsure how this is affected by special key sequences

[2024-01-17 17:25] contificate: like modifiers and shit

[2024-01-17 17:25] luci4: [replying to luci4: "A giant switch statement wouldn't be more efficien..."]
The way I have it, it prints KeyName.name, so I get [LeftShift]

[2024-01-17 17:26] contificate: got shit like https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-mapvirtualkeyw and https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getkeynametextw

[2024-01-17 17:26] contificate: [replying to luci4: "The way I have it, it prints KeyName.name, so I ge..."]
yeah but are you just linear searching the array

[2024-01-17 17:26] luci4: Oh that seems more efficient than my for loop

[2024-01-17 17:27] contificate: also guaranteed to be feature complete

[2024-01-17 17:27] contificate: w.r.t special key codes and modifiers and shit

[2024-01-17 17:39] luci4: [replying to contificate: "also guaranteed to be feature complete"]
This made my function much smaller!
[Attachments: image.png]

[2024-01-17 19:30] brymko: couldn't you do your business logic inside an async function

[2024-01-17 19:31] brymko: why u have to block the proc by that much

[2024-01-17 19:31] brymko: just insert the event into a worker queue

[2024-01-17 19:31] brymko: kafka is really good for this

[2024-01-17 19:31] brymko: and then you can have multiple worker nodes simultainiously pull keystrokes from the queue

[2024-01-17 19:32] brymko: which then could do the translation

[2024-01-17 19:38] luci4: To be honest I haven't heard of async functions and worker queues before, so I'm going to have to research that.

[2024-01-17 19:40] brymko: <@687117677512360003>

[2024-01-17 19:41] brymko: https://tenor.com/view/look-at-him-efe-omowale-assisted-living-he-look-so-funny-haha-gif-24147865

[2024-01-17 19:42] brymko: im just memeing

[2024-01-17 19:42] brymko: but yes you should look it up

[2024-01-17 19:48] contificate: lmao

[2024-01-17 19:56] luci4: 🥹

[2024-01-17 19:58] luci4: by `kafka is really good for this` I'm gonna assume you mean it's good for an async producer

[2024-01-17 19:58] luci4: Anyways, thanks a million, I'm gonna look over those concepts and see what I can make

[2024-01-17 20:29] brymko: i mean lookup kafka and think of how that would be useful in this case

[2024-01-17 21:21] luci4: It seems like it would be good for asynchronous processing of events

[2024-01-17 22:37] diversenok: [replying to Matti: "to be clear I can't defend either design obviously..."]
Oplocks also allow that 😇

[2024-01-18 00:23] WEMUSTDIG: [replying to brymko: "couldn't you do your business logic inside an asyn..."]
No need that code is already optimized

[2024-01-18 02:30] Matti: [replying to diversenok: "Oplocks also allow that 😇"]
lmao, I shouldn't be surprised

[2024-01-18 02:31] Matti: waiting for either you or jonas to link me some POC...

[2024-01-18 02:37] asz: filetest can do it

[2024-01-18 02:38] asz: 
[Attachments: image.png]

[2024-01-18 02:38] Matti: well yes

[2024-01-18 02:38] Matti: that part I know, I guess

[2024-01-18 02:39] Matti: but what file does one open with this to hang a debugger

[2024-01-18 02:39] Matti: a DLL? and then map it?

[2024-01-18 02:40] asz: wouldnt suprice me that just getting paused by an oplock while being debugged freeze debugger

[2024-01-18 02:41] Matti: hm maybe, idk enough about oplocks to answer this

[2024-01-18 02:41] Matti: but wouldn't it be like any other I/O wait?

[2024-01-18 02:41] Matti: I think the answer has to be something involving a file that the debugger must also open

[2024-01-18 02:42] Matti: so hence the DLL mapping thought

[2024-01-18 02:42] Matti: but idk

[2024-01-18 02:44] asz: ah -yah, that can be right

[2024-01-18 02:46] asz: oplocks are special- weirdest imo is that you can oplock on a file- then another process trigger oplock, then you move file away- make its folder to a junction folder- acknowledge oplock

[2024-01-18 02:46] asz: now the opener follow the junction folder

[2024-01-18 02:46] asz: and open what is pointed at

[2024-01-18 02:47] Matti: hahaha

[2024-01-18 02:48] Matti: I remember reading a very in depth (and scary) article on oplocks once

[2024-01-18 02:49] Matti: all I remember is that there's 20 different types

[2024-01-18 02:49] asz: heh

[2024-01-18 02:49] asz: theres some allright

[2024-01-18 02:49] asz: but modern oplock  is only i use

[2024-01-18 02:50] Matti: the fact that *you* use it is more of a counter-endorsement than anything, no offence

[2024-01-18 02:51] asz: none taken- logic

[2024-01-18 02:52] asz: ive got another way to get same effect thouggh

[2024-01-18 02:53] asz: 
[Attachments: image.png]

[2024-01-18 02:55] asz: max bucket collsions max times in obj manager

[2024-01-18 02:55] Matti: yeah I see it

[2024-01-18 02:56] Matti: that is funny

[2024-01-18 02:58] Matti: I'll try out the 'debuggee maps oplocked DLL' hypothesis tomorrow maybe

[2024-01-18 02:58] Matti: or <@503274729894051901> can give a hint

[2024-01-18 02:59] Matti: too fucking tired now though

[2024-01-18 02:59] Matti: bed time for me

[2024-01-18 02:59] asz: night night

[2024-01-18 05:33] diversenok: [replying to Matti: "waiting for either you or jonas to link me some PO..."]
Batch oplocks on either the main exe or any DLLs

[2024-01-18 05:34] diversenok: Both `DbgCreateProcessStateChange` and `DbgLoadDllStateChange` debug events provide a file handle which the kernel needs to open first

[2024-01-18 05:46] diversenok: Debuggers can get stuck on attach inside `NtDebugActiveProcess` which tries to generate fake process creation/image load events, breaks the oplock, and starts waiting for the process it just suspended...
[Attachments: image.png]

[2024-01-18 05:48] 0xatul: [replying to Matti: "I remember reading a very in depth (and scary) art..."]
Got a link bro?

[2024-01-18 09:19] Matti: [replying to diversenok: "Debuggers can get stuck on attach inside `NtDebugA..."]
yep, this was exactly the code path I had in mind as well

[2024-01-18 09:22] Matti: I think you can do this for both new process creations and debugger attaches, the kernel will send a debugger message with a handle for events like DLL loads either way

[2024-01-18 09:24] Matti: I think that is a different code path (not the 'fake' messages one but just regular debugger events), but it obviously still needs to actually open the handle

[2024-01-18 09:27] Matti: [replying to 0xatul: "Got a link bro?"]
sorry, I looked but I couldn't find the exact article or book I read (I would've recognized it for sure as it was a **huge** MS .doc file)

fortunately it seems a lot of this is now part of the standard MS WDK reference documentation on oplocks, so you can just go there instead. some nice examples:

1. (intro) what are oplocks - <https://learn.microsoft.com/en-us/windows-hardware/drivers/ifs/oplock-overview>
2. the 8(!) different types of oplocks - <https://learn.microsoft.com/en-us/windows-hardware/drivers/ifs/oplock-types>
3. how to grant an oplock - <https://learn.microsoft.com/en-us/windows-hardware/drivers/ifs/granting-oplocks>
    the table ("the following table identifies the required conditions necessary to grant an oplock") is where it all becomes totally unhinged, thanks to (2)
4. how to break an oplock - <https://learn.microsoft.com/en-us/windows-hardware/drivers/ifs/breaking-oplocks>
    so, how to break an oplock? weeeelll that depends on the type of IRP, so see one or more of the following **7** pages (depending on the IRP(s) you need to handle) :)))))

    (the `IRP_MJ_CREATE` page alone has another combinatorial explosion of conditions in table form)

[2024-01-18 09:29] Matti: there's also this .doc (and PDF version for sanity...) from the NT design workbook, it's much more readable but unfortunately out of date w.r.t. the many different types of oplocks there are now
[Attachments: oplock.doc, oplock.pdf]

[2024-01-18 09:30] Matti: since it's from 1991

[2024-01-18 09:38] Matti: in fact here's the entire NT design workbook
these are all really short and well written technical docs on the original NT kernel components, which means most of it is still relevant today
[Attachments: image.png]

[2024-01-18 09:38] Matti: 
[Attachments: NT_Design_Workbook.7z]

[2024-01-18 09:40] Matti: I cbf to convert them all to PDF though, so better find something to open .doc files with

[2024-01-18 09:41] diversenok: [replying to Matti: "I think that is a different code path (not the 'fa..."]
I think it won't work with regular image loads because event generation happens inside `NtMapViewOfSection`, which means in the context of the caller, not the debugger

[2024-01-18 09:42] diversenok: But it's a good way to deadlock yourself 😅

[2024-01-18 09:43] diversenok: Process creation might do the same and just deadlock inside `NtCreateUserProcess`

[2024-01-18 09:43] Matti: ah hm, true that I think

[2024-01-18 09:45] Matti: well there might be a second NtMapView (intended for the debugger) similar to the one in your deadlock scenario, but it wouldn't be reached for obvious reasons

[2024-01-18 09:46] Matti: I think....

[2024-01-18 09:47] Matti: [replying to Matti: "bed time for me"]
\>matti, over 6 hours ago

[2024-01-18 09:48] Matti: now I'm really going to do it though

[2024-01-18 09:48] Matti: too tired to think

[2024-01-18 09:48] luci4: Good night!

[2024-01-18 09:48] Matti: bai bai

[2024-01-18 10:16] 0xatul: bro

[2024-01-18 10:16] 0xatul: thanks so much <@148095953742725120>

[2024-01-18 10:16] Deleted User: night night matti

[2024-01-18 19:35] .mydev: What would be a decent C project involving mainly WinAPIs, such that it makes me learn and engage in security mechanisms?
Let's say I have no previous exprience in WinAPI, but only in C programming , and I want to learn constructively WinAPIs as well as Windows components (especially security-wise)

[2024-01-18 19:58] qwerty1423: you can try writing a basic password management program in C, make it encrypt the data, Utilize WinAPIs like `CryptProtectData` and `CryptUnprotectData` for that.

[2024-01-18 21:17] donna🤯: [replying to Matti: ""]
that is an awesome resource

[2024-01-18 21:17] donna🤯: god damn you are a legend

[2024-01-18 21:37] WEMUSTDIG: [replying to Matti: "there's also this .doc (and PDF version for sanity..."]
oblock bang bang

[2024-01-20 14:07] luci4: nvm 👼

[2024-01-20 14:09] mrexodia: [replying to .mydev: "What would be a decent C project involving mainly ..."]
Write a debugger

[2024-01-20 14:37] luci4: Would it be a better idea to have a batch structure instead of using global variables? I'm storing key events in batches of 1024 bytes (which is probably too big?) before sending them to a Kafka topic
[Attachments: image.png]

[2024-01-20 19:00] brymko: no fucking way

[2024-01-20 19:00] brymko: youre a madlad

[2024-01-20 19:00] luci4: [replying to brymko: "youre a madlad"]
Did I fuck it up? Lol

[2024-01-20 19:01] brymko: i just didnt think ud actually do it

[2024-01-20 19:01] brymko: when i was memeing around

[2024-01-20 19:02] luci4: [replying to brymko: "when i was memeing around"]
Oh rly? So it was a bad idea? Well atleast I learned smth, lol

[2024-01-20 19:02] brymko: eh like its pretty based

[2024-01-20 19:02] brymko: not entirely sure if bad or not

[2024-01-20 19:29] Timmy: bad depends on what you're doing, if  you're just trying to see all user inputs its fine

[2024-01-20 19:29] Timmy: if you care about your latency at all

[2024-01-20 19:29] Timmy: yes this is horrid

[2024-01-20 19:29] luci4: Well it's supposed to send the key events to my (soon to be created) server which will process them and store them in a DB

[2024-01-20 19:29] luci4: Something like

[2024-01-20 19:30] Timmy: then you're good

[2024-01-20 19:30] luci4: Google Chrome
==============
I typed this then I pressed [LeftAlt]

[2024-01-20 19:31] luci4: [replying to Timmy: "then you're good"]
Awesome! Is 1024 bytes too much though? Lol

[2024-01-20 19:31] Timmy: maybe

[2024-01-20 19:31] Timmy: idk

[2024-01-20 19:31] luci4: I'll just try it out and see

[2024-01-20 19:31] luci4: Wanna make a cool little dashboard for it, because why not

[2024-01-20 19:32] qwerty1423: [replying to luci4: "Awesome! Is 1024 bytes too much though? Lol"]
it doesn't make much difference

[2024-01-20 19:32] luci4: Per-fect!

[2024-01-20 19:32] contificate: I feel like the fact you've got an upper bound on the batch size

[2024-01-20 19:32] contificate: means you may as well just use a fixed buffer

[2024-01-20 19:32] contificate: rather than a linked list

[2024-01-20 19:33] contificate: will be more efficient to avoid `malloc`

[2024-01-20 19:33] contificate: you also never seem to free the nodes

[2024-01-20 19:33] contificate: you'd want something more like

[2024-01-20 19:33] contificate: ```c
while (it) {
  struct list* next = it->next;
  process(it);
  free(it);
  it = next;
}
```

[2024-01-20 19:46] luci4: [replying to contificate: "rather than a linked list"]
I used a linked list for the key events
```
typedef struct KeyEvent {
    int vkCode;
    struct KeyEvent* next;
} KeyEvent;
```
Because I was afraid they wouldn't register in order, but now that I think about it more I could have just used a list 🤦‍♂️

[2024-01-20 19:47] luci4: [replying to contificate: "I feel like the fact you've got an upper bound on ..."]
Well it's supposed to capture key events until it reaches 1024 bytes, which is the size of the message I will send to the Kafka topic

[2024-01-20 19:47] luci4: 
[Attachments: image.png]

[2024-01-20 19:47] contificate: it'd be far better to use a buffer

[2024-01-20 19:47] contificate: doing list node allocations is not efficient if you don't benefit from the structure of having lists

[2024-01-20 20:27] luci4: [replying to contificate: "it'd be far better to use a buffer"]
Alright, I'll just use a buffer and a list, thank you 🙏

[2024-01-20 20:29] contificate: should use the term "array" when you want to imply contiguous/adjacent allocation

[2024-01-20 20:29] contificate: I know languages like Python call its arrays "list"s

[2024-01-20 20:29] contificate: but typically list means a linked list of some description

[2024-01-20 20:29] contificate: unless you are actually telling me you're gonna use some combination of both

[2024-01-20 20:41] luci4: [replying to contificate: "should use the term "array" when you want to imply..."]
Yeah I meant to say array, my bad

[2024-01-20 20:41] luci4: Thanks for the help!

[2024-01-20 23:02] donna🤯: [replying to .mydev: "What would be a decent C project involving mainly ..."]
anticheat

[2024-01-21 11:08] Timmy: is it normal for llvm to take 50+ minutes to build on a debug build with a amd r9 5900? here are my compile commands in a 2022 vs command prompt
```
cmake -S llvm -B build-debug -DCMAKE_BUILD_TYPE=Debug -DLLVM_HOST_TRIPLE=x86_64-windows-msvc -Thost=x64 -DLLVM_ENABLE_PROJECTS="clang"
cmake --build build-debug -j 10
```
really hoping I can improve the compile times <:KEKW:798912872289009664>

[2024-01-21 11:12] contificate: for me it's always the linking time and space because I don't use chad linkers

[2024-01-21 11:12] contificate: but luckily can do it in 2 attempts with my meagre 16GB of RAM

[2024-01-21 11:12] contificate: but I basically never build LLVM on Linux, as there's no need if you're not doing in-tree work

[2024-01-21 11:15] 25d6cfba-b039-4274-8472-2d2527cb: -j 24 you are welcome!!!

[2024-01-21 11:15] 25d6cfba-b039-4274-8472-2d2527cb: 🤪

[2024-01-21 11:19] Timmy: <:KEKW:798912872289009664> guess I'll have to turn smt back on if I want more speed

[2024-01-21 11:19] Timmy: if it's possible to use the ninja generator would that be better?

[2024-01-21 11:20] Matti: that does sound very long, I've never exactly timed building llvm on my 5950X but I feel like it's a lot less than 50 mins, and that's for a bootstrap build with LTO

[2024-01-21 11:20] Matti: not debug...

[2024-01-21 11:20] Timmy: it's still going

[2024-01-21 11:21] Timmy: 
[Attachments: image.png]

[2024-01-21 11:21] Matti: [replying to Timmy: "if it's possible to use the ninja generator would ..."]
what is this generating if not ninja?

[2024-01-21 11:21] Matti: I do recommend ninja

[2024-01-21 11:21] Timmy: it's using msbuild ig

[2024-01-21 11:22] Matti: definitely try ninja instead

[2024-01-21 11:23] Timmy: ah also

[2024-01-21 11:23] Matti: msbuild is normally fine (a lot faster than people think it is), but for llvm I'd use ninja for sure

[2024-01-21 11:23] Matti: it just seems to rape msbuild

[2024-01-21 11:23] Timmy: looks like I've filled up my disk with llvm build artifacts

[2024-01-21 11:23] Timmy: 
[Attachments: image.png]

[2024-01-21 11:24] Timmy: lovely

[2024-01-21 11:24] Timmy: 70 gigs for a debug build

[2024-01-21 11:24] Timmy: TIL

[2024-01-21 11:24] Timmy: https://tenor.com/view/capisce-owl-blink-tilt-head-huh-gif-16568093

[2024-01-21 11:25] Matti: hmm, my llvm dir is only 70 GB total

[2024-01-21 11:25] Matti: including build artifacts

[2024-01-21 11:26] Timmy: well

[2024-01-21 11:26] Timmy: 
[Attachments: image.png]

[2024-01-21 11:26] Matti: 238 GB 🥲

[2024-01-21 11:27] Matti: just a few of my UE5 project dirs are over 600 GB <:kekw:728766271772033046>

[2024-01-21 11:27] Matti: each

[2024-01-21 11:30] Timmy: this ssd still hasn't died on me XD, I've had it for too long

[2024-01-21 11:31] Timmy: I have more storage on other disks

[2024-01-21 11:31] Timmy: I'll try ninja with smt turned back on and see what happens ig

[2024-01-21 11:32] Matti: yeah smt should help a lot for sure

[2024-01-21 11:32] Matti: why do you have it off? gaming?

[2024-01-21 11:32] Timmy: I've noticed disabling it makes the system feel a bit more smooth

[2024-01-21 11:32] Timmy: been told it reduces memory latency because of the architecture of this cpu

[2024-01-21 11:33] Timmy: I should measure this stuff, but no idea

[2024-01-21 11:33] Timmy: but yeah gaming basically

[2024-01-21 11:34] Matti: ugh there is a tiny tool that will do precisely what you want

[2024-01-21 11:34] Matti: but I forget what it's called

[2024-01-21 11:34] Matti: lemme google

[2024-01-21 11:36] Matti: lmao

[2024-01-21 11:36] Matti: it's the Intel® Memory Latency Checker

[2024-01-21 11:36] Matti: but it works on my AMD CPU

[2024-01-21 11:36] Timmy: I'll check it out some time sounds cool!

[2024-01-21 11:37] Matti: not entirely sure it knows how to do SMT correctly on AMD though, so watch out

[2024-01-21 11:37] Timmy: lots of those intel tools work for a bunch of other platforms

[2024-01-21 11:37] Timmy: like recently they released a benchmarking tool for reviewers just to have them produce more data when reviewing any gpu's

[2024-01-21 11:37] Timmy: hoping that it'd reflect well on their gpu line of course

[2024-01-21 11:37] Timmy: but still

[2024-01-21 11:37] Timmy: good stuff

[2024-01-21 11:38] Matti: and there's this
but it's for cache latency only, I think
[Attachments: latency.exe]

[2024-01-21 11:38] Matti: it's from cpuid.com

[2024-01-21 11:39] Matti: oh yeah

[2024-01-21 11:39] Matti: http://download.cpuid.com/misc/latency.zip

[2024-01-21 11:39] Matti: just so you know it's not my RAT

[2024-01-21 11:39] Timmy: [replying to Timmy: "like recently they released a benchmarking tool fo..."]
https://game.intel.com/us/stories/intel-presentmon/
it's this one
[Embed: Intel® Arc™ Graphics – PresentMon]

[2024-01-21 11:39] Timmy: [replying to Matti: "just so you know it's not my RAT"]
not worried but thanks haha

[2024-01-21 11:40] Matti: yeah presentmon is good

[2024-01-21 11:42] Matti: LHS = intel(R) 13th gen core i5(TM)
RHS = 5950X
[Attachments: image.png]

[2024-01-21 11:42] Matti: I know the i5 is shit but I do feel like it ought to have more than 1 cache level

[2024-01-21 11:43] Timmy: didn't know that intel was so much better on the latencies

[2024-01-21 11:43] Matti: well it's not *entirely* fair, the i5 is idle and my 5950X is compiling

[2024-01-21 11:44] Matti: but it won't beat the i5 no

[2024-01-21 11:44] Matti: cause of dual CCX

[2024-01-21 11:44] Timmy: here now it's fair

[2024-01-21 11:44] Timmy: 
[Attachments: image.png]

[2024-01-21 11:44] Timmy: 5900 is idle

[2024-01-21 11:44] Timmy: smt on btw

[2024-01-21 11:45] Timmy: going back and disabling gimme 5

[2024-01-21 11:45] Matti: your CPU will beat mine on latency, especially core to core worst case scenario

[2024-01-21 11:45] Matti: cause it has 1 CCX

[2024-01-21 11:46] Matti: mine has 2, so the worst case latency gets quite a lot worse

[2024-01-21 11:46] Timmy: I've read something about this, I thought mine had the same

[2024-01-21 11:48] Matti: used wrong term, I meant CCD*

[2024-01-21 11:48] Matti: if I'm not mistaken your 5900 will have 1 CCD

[2024-01-21 11:48] Timmy: 
[Attachments: image.png]

[2024-01-21 11:48] Timmy: smt off btw

[2024-01-21 11:48] Timmy: it's worse???

[2024-01-21 11:49] Matti: that does surprise me

[2024-01-21 11:50] Matti: it's very workload dependent though

[2024-01-21 11:51] Matti: but, I would expect a tool designed for measuring cache latency to do better with SMT off 🤔

[2024-01-21 11:52] Matti: oh yes!

[2024-01-21 11:52] Matti: there's AIDA64

[2024-01-21 11:53] Matti: it's got a combined cache and memory benchmark

[2024-01-21 11:54] Matti: it only shows raw throughput though, so different purpose

[2024-01-21 11:55] Timmy: throughput should always be higher with smt no?

[2024-01-21 11:55] Timmy: or is that illogical because it doesn't test compute?

[2024-01-21 11:55] Matti: no idea!

[2024-01-21 11:56] Matti: 
[Attachments: image.png]

[2024-01-21 11:56] Matti: mine, with SMT on obviously

[2024-01-21 12:00] Matti: intel(R) core i5(TM)
[Attachments: image.png]

[2024-01-21 12:01] Matti: that's also with SMT and all the P and E cores enabled

[2024-01-21 12:02] Timmy: I'm getting trial versioned by AIDA <:KEKW:798912872289009664>

[2024-01-21 12:02] Timmy: 
[Attachments: image.png]

[2024-01-21 12:02] Matti: I'm used to seeing this
[Attachments: image.png]

[2024-01-21 12:02] Timmy: <:KEKW:798912872289009664>

[2024-01-21 12:03] Matti: oh damn, there's a v7 now? mine is out of date

[2024-01-21 12:05] Timmy: attempt 3 to compile llvm

[2024-01-21 12:05] Timmy: hopefully it goes better now LOL

[2024-01-21 12:07] Matti: hmm something else I remember now that could be affecting my memory bandwidth/latency is the fact that I've got 4 DIMMs installed

[2024-01-21 12:07] Timmy: I have 2 dimms of 32

[2024-01-21 12:08] Timmy: but I think no matter what it's still dual channel tho

[2024-01-21 12:08] Timmy: with the a and the b banks?

[2024-01-21 12:08] Matti: 2 is faster on 99% of consumer motherboards

[2024-01-21 12:08] Timmy: I'm stupid when it comes to hw in general so idk

[2024-01-21 12:08] Matti: yeah, but there's a different tradeoff involved when it comes to 2 vs 4 DIMMs

[2024-01-21 12:10] Matti: and also there's different topologies possible to implement support for 4 DIMMs, with most motherboard vendors taking the cheap option

[2024-01-21 12:11] Timmy: do you think it's too expensive for them to do? or would it simply just not give much return on investment in general

[2024-01-21 12:12] Matti: well that's sort of the same question isn't it <:kekw:904522300257345566> only the first one is phrased nicer from the vendor's POV

[2024-01-21 12:14] Matti: https://www.youtube.com/watch?v=3vQwGGbW1AE sorry I couldn't find a coherent article on this topic, but this is a good YT video
[Embed: Rambling about motherboard memory layouts.]
AHOC Patreon/Shirts/Junkyard:http://cxzoid.blogspot.co.uk/p/support-fail.html
The Twitch:https://www.twitch.tv/buildzoid
The Facebook:  https://www.facebook.com/actuallyhardcoreoverclocking

[2024-01-21 12:19] Matti: I think 4 DIMMs is basically always going to be worse than 2 no matter what, but T-topology is definitely preferable for it

[2024-01-21 12:21] Timmy: [replying to Matti: "well that's sort of the same question isn't it <:k..."]
depends on the specific context I think. If they could deliver a 10% latency reduction but the mobo would cost 10k on the shelf with no other benefits I think it's the first one. but for example lets say it's a 10% latency reduction for a 5% throughput decrease at an extra cost of 100 bucks I think it's the second option.

[2024-01-21 12:22] Timmy: just pulling some numbers out of my arse here obviously

[2024-01-21 12:22] Matti: sure, that's true of course

[2024-01-21 12:22] Matti: I don't know the cost difference either

[2024-01-21 12:22] Timmy: I admit it's much more likely that the vendor would lie <:KEKW:798912872289009664>

[2024-01-21 12:22] Matti: but I believe it's affordable enough, since there are consumer motherboards with T-topology

[2024-01-21 12:23] Matti: taichi makes/made(?) them

[2024-01-21 12:23] Timmy: so that's what the grandma's in the park are doing...

[2024-01-21 12:23] Timmy: never knew

[2024-01-21 12:23] Matti: errr asrock*

[2024-01-21 12:23] Matti: with the motherboard being named taichi....

[2024-01-21 12:24] Matti: never heard of it, so I just assumed it was the name of some taiwanese motherboard vendor <:kekw:904522300257345566>

[2024-01-21 12:24] Timmy: very reasonable haha

[2024-01-21 12:24] Matti: there are a bunch of really obscure motherboard vendors

[2024-01-21 12:25] Timmy: that sounds scary

[2024-01-21 12:25] Timmy: given normal mobo vendors seem to struggle with their jobs

[2024-01-21 12:25] Matti: like, biostar pretty much only exists in asia I think

[2024-01-21 12:26] Matti: they're probably doing fine, just obscure to us because they don't sell on our markets

[2024-01-21 12:27] Timmy: btw with ninja it's already linking 22 minutes in now

[2024-01-21 12:27] Timmy: before it was compiling for an hour and 10 minutes

[2024-01-21 12:28] Matti: <:Pepege:640617538232909865>

[2024-01-21 12:28] Timmy: ninja + 14 more threads

[2024-01-21 12:39] Timmy: 35 minutes and done :3

[2024-01-21 12:41] Matti: yeah that sounds a lot closer to what it should be

[2024-01-21 12:41] Matti: LLVM and msbuild just don't get along

[2024-01-21 12:42] Matti: plus having SMT on tends to help