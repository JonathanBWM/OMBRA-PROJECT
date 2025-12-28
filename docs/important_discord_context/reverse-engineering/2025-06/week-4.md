# June 2025 - Week 4
# Channel: #reverse-engineering
# Messages: 71

[2025-06-17 02:24] rin: can someone speak on the reliability of pseudo c in binary ninja, I have the following function which I think is duplicatetokenex, but the parameters make no sense. It looks like theres an access mask in two locations and a lack of an output token.
[Attachments: image.png, image.png]

[2025-06-17 04:08] GG: [replying to rin: "can someone speak on the reliability of pseudo c i..."]
no decompiler is fully realible you have to check the disassembly

[2025-06-17 04:09] GG: you can make the decompiler produce better results by fixing things your self

[2025-06-18 13:26] Horsie: Is there a way to halt (suspend/freeze/whatever) a thread in kernel with windbg?

[2025-06-18 13:28] Horsie: I want to do it such that only one specific thread is frozen at some particular in the kernel, let other threads do something, then resume the thread.

[2025-06-18 13:28] Horsie: I looked at suspend/freeze in windbg docs but it seems like they apply only for usermode?
One way I can think of is just to patch a spinlock there but I want something a bit less dirty. Some of the places I want to halt are within a higher irql region so I dont want it to potentially die to a watchdog timeout.

[2025-06-18 13:31] pkb: [replying to rin: "can someone speak on the reliability of pseudo c i..."]
Hello,a bit off, but what is this theme ?

[2025-06-18 14:26] valium: [replying to pkb: "Hello,a bit off, but what is this theme ?"]
i think reflection

[2025-06-18 14:26] valium: its inbuilt

[2025-06-18 20:54] elias: Are the MajorFunctions of drivers like usbaudio or hdaudio patchguard protected?

[2025-06-18 22:35] mrexodia: [replying to elias: "Are the MajorFunctions of drivers like usbaudio or..."]
you could easily find out

[2025-06-19 19:43] NSA, my beloved<3: Does anyone have any ideas why Microsoft's signtool.exe, which has the subsystem set to Windows Console, does not make windows create a new terminal process, rather the output is printed into the terminal the process was launched with? Whenever I create a Windows Console subsystem binary and launch it from a terminal, for a brief moment a new terminal window pops up then disappears and obviously there is no output in the initial terminal. I have taken a look at what signtool.exe does by disassembling it, and they don't call AllocConsole or do any terminal checks, they straight up just use printfs. So not entirely sure why their binary behaves different if launched throughout a shell.

[2025-06-19 21:07] diversenok: [replying to NSA, my beloved<3: "Does anyone have any ideas why Microsoft's signtoo..."]
This is the default behavior for console-subsystem applications?

[2025-06-19 21:07] diversenok: When launched from a process that doesn't have a console, they create their own

[2025-06-19 21:07] diversenok: When launched from a process with a console, they inherit it

[2025-06-19 21:08] diversenok: Unless overwritten by a flag at process creation

[2025-06-19 21:09] diversenok: > Whenever I create a Windows Console subsystem binary and launch it from a terminal, for a brief moment a new terminal window pops up then disappears and obviously there is no output in the initial terminal
That sounds like it's not a console application or the application calls FreeConsole followed by AllocConsole. That should not happen by default

[2025-06-20 08:58] f00d: [replying to NSA, my beloved<3: "Does anyone have any ideas why Microsoft's signtoo..."]
well since the program is done executing, it just exits.

[2025-06-20 09:00] f00d: or i'm not getting u

[2025-06-20 15:33] ruan: is possible to make ida also color lines that has a breakpoint in the pseudo code view? 
i'm using ida pro 9.1 it only colors the lines on the asm window

[2025-06-20 15:52] pinefin: im not sure if this helps, but does synchronizing the view allow this functionality?

[2025-06-20 15:52] pinefin: i really feel like ive ran into this before

[2025-06-20 15:55] ruan: it doesnt

[2025-06-20 15:57] ruan: the line that has the vtable comment should also be colored red
[Attachments: image.png]

[2025-06-20 15:57] 0xatul: just clck on the blue breakpoint thingy

[2025-06-20 15:58] ruan: im setting the breakpoints with a py script `idaapi.add_bpt(self.address, 0, idc.BPT_SOFT)`

[2025-06-20 16:03] 0xatul: have you enabled source level debugging ?

[2025-06-20 16:05] ruan: yes

[2025-06-20 16:07] 0xatul: I had this problem once,let me grep if I fixed it or not

[2025-06-20 16:08] 0xatul: looks like I just rage quit with that

[2025-06-20 16:08] 0xatul: ping me if you get a soluton

[2025-06-20 17:49] NSA, my beloved<3: [replying to diversenok: "> Whenever I create a Windows Console subsystem bi..."]
Both of them are indeed Windows Console applications, this is the reason that is causing the confusion.

[2025-06-20 17:50] NSA, my beloved<3: [replying to f00d: "well since the program is done executing, it just ..."]
I know. I am trying to launch a process using a terminal and I want to see the output printed into this terminal rather than the process creating a new terminal and exiting that one.

[2025-06-20 17:51] NSA, my beloved<3: Could be because I don't use CRT, which includes code to handle this kind of behavior?

[2025-06-20 17:51] diversenok: Could be. How do you use the console APIs?

[2025-06-20 17:52] f00d: possibly

[2025-06-20 17:52] NSA, my beloved<3: Changing directory and then launching both by typing in the file names.

[2025-06-20 17:53] NSA, my beloved<3: But again, signtool.exe does not use CRT either and it straight up calls printf and that works.

[2025-06-20 17:53] diversenok: It does

[2025-06-20 17:53] NSA, my beloved<3: Let me re-check.

[2025-06-20 17:54] diversenok: Lots of CRT imports
[Attachments: image.png]

[2025-06-20 17:54] NSA, my beloved<3: I meant CRT as in having the compiler generate the binary entry point.

[2025-06-20 17:54] NSA, my beloved<3: The boilerplate one.

[2025-06-20 17:55] diversenok: Not sure I understand

[2025-06-20 17:55] diversenok: Either way, the default behavior is to inherit the console

[2025-06-20 17:56] NSA, my beloved<3: Oh yeah, they do indeed use CRT, my bad.

[2025-06-20 17:56] NSA, my beloved<3: Let's see if that fixes the issue.

[2025-06-20 17:56] diversenok: How do you output to the console?

[2025-06-20 17:57] NSA, my beloved<3: It did not.

[2025-06-20 17:57] NSA, my beloved<3: Calling printf.

[2025-06-20 17:57] diversenok: Without CRT?

[2025-06-20 17:57] NSA, my beloved<3: Found the issue.

[2025-06-20 17:58] NSA, my beloved<3: I enabled UAC in order for the process to be ran under elevated priviliges. Launching the process from a terminal under the normal user results in a new process, but if I launch it throughout Administrator the output is printed in the right way.

[2025-06-20 17:59] diversenok: Ahh, so it's doing silent elevation? Then yeah, in this case it makes sense that it doesn't inherit the console

[2025-06-20 18:01] NSA, my beloved<3: And CRT is not required either.

[2025-06-20 18:03] NSA, my beloved<3: I wonder, whenever I launch a program throughout a terminal and a shell, the shell process looks for the file on disk, if it finds it, it does a Windows API call which specifies that this file should be turned into a process and a syscall is done by ntdll. After the kernel created the new process object, thread objects, etc... and the process' code is ran, how does the kernel decide where the output should go?

[2025-06-20 18:05] diversenok: Console support is not really kernel's department

[2025-06-20 18:06] diversenok: What ends up happening is `CreateProcess` specifies some attributes that make the system inherit the console handles into the new process

[2025-06-20 18:08] rin: [replying to NSA, my beloved<3: "I wonder, whenever I launch a program throughout a..."]
theres parameters in STARTUPINFO that define sources for stdin and stdout

[2025-06-20 18:08] rin: if thats what you mean

[2025-06-20 18:09] diversenok: The kernel doesn't decide where the output shoud go; it's up to the user-mode implementation of printf to choose

[2025-06-20 18:10] diversenok: Which ends up using these console hanldes inherited from the parent

[2025-06-20 18:11] diversenok: And, yes, you can overwrite this behavior and specify arbitrary handles for standard I/O

[2025-06-20 18:12] diversenok: Or clear them to make the new process create its own console

[2025-06-20 18:13] diversenok: Because kernelbase/kernel32 initialization code will automatically create a console if the executable's subsystem is console and there are no inherited standard I/O handles

[2025-06-21 09:29] NSA, my beloved<3: Ah, the inheritance of handles makes sense.

[2025-06-21 09:30] NSA, my beloved<3: So a file read/write piped to a file handle, in this case standard input/ouput, would correspond to the parent process' handles.

[2025-06-22 06:28] aдриан: Ok so an android app I’m trying to reverse engineer using frida has frida detection and auto shuts down. Any suggestions on how I can bypass it and mask the presence of frida?

[2025-06-22 10:24] Horsie: Is anyone aware of any signed (with `Windows Media Certificate` as per winternals) that are vulnerable to sideloading a dll into it or get some kind of code execution?

[2025-06-22 10:24] Horsie: Trying to play around with protected processes and would like to run some code in them.

[2025-06-22 11:31] mtu: If the process is protected, the DLL has to have a signature with the same (or better) PPL level