# September 2025 - Week 2
# Channel: #application-security
# Messages: 48

[2025-09-11 09:57] naru: gm guys, I'm a computer noob. I want to learn more about application security, kindly point me to a guide/resources. thanks 🫡

[2025-09-11 13:29] daax: [replying to naru: "gm guys, I'm a computer noob. I want to learn more..."]
<#835648484035002378>

[2025-09-12 09:58] W4ZM: is this not true anymore ? : "  when the process is loaded by a debugger, the `SeDebugPrivilege` privilege is enabled. ", when i checked the privilege in the debugged process `SeDebugPrivilege` was disabled

[2025-09-12 10:00] diversenok: Was it like that before?

[2025-09-12 10:01] diversenok: Sure, debuggers like enabling the debug privilege, and processes spawned by debuggers inherit their token, but that's about as far as the connection goes

[2025-09-12 10:02] W4ZM: apparently its not true when i checked (win 11)

[2025-09-12 10:05] diversenok: I mean, despite the name, `SeDebugPrivilege` doesn't have that much to do with debugging

[2025-09-12 10:10] W4ZM: 
[Attachments: image.png]

[2025-09-12 10:12] diversenok: OllyDbg? Opening CSRSS? How old is this guide?

[2025-09-12 10:13] W4ZM: its supposed to be antidebugging technique , idk im just asking if it still works or not

[2025-09-12 10:14] W4ZM: `SeDebugPrivilege` is disabled even if the process is being debugged

[2025-09-12 10:17] diversenok: It only applies if the debugger has the privilege (so runs as admin), decides to enable it (they usually do), and starts a new process (i.e., not attaches to an existing one)

[2025-09-12 10:17] diversenok: This hasn't changed

[2025-09-12 10:17] W4ZM: i have x64dbg running as admin and loading the process (no attach)

[2025-09-12 10:18] W4ZM: i checked the privilege with system informer btw

[2025-09-12 10:18] W4ZM: with no kernel driver

[2025-09-12 10:19] W4ZM: and i checked that privilege in x64dbg and it was enabled

[2025-09-12 10:20] W4ZM: but not the exe

[2025-09-12 10:21] diversenok: Yeah, can confirm that

[2025-09-12 10:21] diversenok: x64dbg.exe has the privilege enabled but the child process does not

[2025-09-12 10:21] diversenok: Well, it was always a mere side effect

[2025-09-12 10:23] W4ZM: well ig that technique doesnt work anymore

[2025-09-12 10:24] Brit: you can chose to not let the debugee inherit the token

[2025-09-12 10:24] Brit: so obviously

[2025-09-12 10:24] diversenok: Nope, processes spawned by WinDbg do have the privilege enabled for me

[2025-09-12 10:30] Brit: [replying to W4ZM: "well ig that technique doesnt work anymore"]
https://github.com/x64dbg/TitanEngine/blob/x64dbg/TitanEngine/TitanEngine.Debugger.cpp#L241
[Embed: TitanEngine/TitanEngine/TitanEngine.Debugger.cpp at x64dbg · x64db...]
TitanEngine Community Edition. Debug engine used by x64dbg. - x64dbg/TitanEngine

[2025-09-12 10:31] Brit: he specifically constructs the flags the process will be created with

[2025-09-12 10:31] Brit: to avoid inheriting things from the debugger that might give it away

[2025-09-12 10:40] W4ZM: well then ig this doesnt work anymore : "Some debuggers can be detected by using the `kernel32!OpenProcess()` function on the `csrss.exe` process. The call will succeed only if the user for the process is a member of the `administrators` group and has debug privileges." both conditions are met and still get access denied

[2025-09-12 10:41] diversenok: Outdated; csrss is a protected process since Windows 8 or 8.1

[2025-09-12 10:41] W4ZM: lol what a waste of time

[2025-09-12 10:42] Brit: not sure that trying to do antidebug through windows quirks is the right approach anyway

[2025-09-12 10:45] diversenok: Not sure how OpenProcess on csrss was supposed to help either way

[2025-09-12 10:45] diversenok: Too deep into making conclusions from side effects

[2025-09-12 10:46] W4ZM: i found this : "Starting in Windows 10, CSRSS is a protected process and can only be debugged in kernel mode."

[2025-09-12 10:47] diversenok: Yeah, maybe it became protected a bit later. Process protection exists since Win 8 and PPL since 8.1 I believe

[2025-09-12 10:48] diversenok: It still technically can be debugged from user mode as well, but the debugger needs at least the same level of protection

[2025-09-12 10:50] Brit: ppling your debugger <:yea:904521533727342632>

[2025-09-12 10:52] diversenok: MS just needs to add a few EKUs to WinDbg's signature and voila

[2025-09-12 10:53] diversenok: So close but so far

[2025-09-12 13:14] luci4: [replying to diversenok: "Yeah, maybe it became protected a bit later. Proce..."]
AFAIK protected processes have been a thing since Vista. Windows 8  introduced multiple protection levels (PPL)

[2025-09-12 13:30] naru: [replying to daax: "<#835648484035002378>"]
gm. curious, is there any difference between vulnerability research and application security?

[2025-09-12 13:37] Matti: [replying to luci4: "AFAIK protected processes have been a thing since ..."]
yep. but the vista protection type (PP, he he) was only ever applied to the system process and some DRM usermode shit process I forget the name of
the CSRSS process only received protection when PPLs were introduced

[2025-09-12 13:39] x86matthew: audiodg.exe was one i think

[2025-09-12 13:40] x86matthew: but yeah it wasn't used for much else back then

[2025-09-12 13:40] Matti: re: debugging CSRSS: apart from it being a PPL which you can get around if needed, it also has the not so nice property of freezing your system if debugged from the same session

[2025-09-12 13:40] Matti: [replying to x86matthew: "audiodg.exe was one i think"]
yeahh that sounds about right

[2025-09-12 13:41] Matti: it wasn't even microsoft's own licensing DRM, it was some really lame and music industry related BS