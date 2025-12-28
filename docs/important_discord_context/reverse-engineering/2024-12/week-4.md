# December 2024 - Week 4
# Channel: #reverse-engineering
# Messages: 99

[2024-12-17 09:53] kurt: hello, i was wondering what security measurements (along with internal-checks) i can implement into my app, alongside a bin2bin obsfucator (codedefender). most stuff is against static analysis but was wondering what id do against runtime analysis. Any recommendations are much appreciated 🙂

[2024-12-17 10:10] CutDown: [replying to kurt: "hello, i was wondering what security measurements ..."]
it is really OS dependant, but a lot of anti-vm and anti-debugging techniques can be applied

some anti-checks can be obfuscated, and be executed in arbitrary times by different threads and etc

there are many unique solutions which can greatly increase the dynamic analysis time and bypasses necessary to overcome different protection techniques

what operating system are you aiming?

[2024-12-17 10:12] kurt: [replying to CutDown: "it is really OS dependant, but a lot of anti-vm an..."]
We are currently building it for native windows x64-x86 10-11 systems.

[2024-12-17 10:12] kurt: Only for Windows operating systems actually

[2024-12-17 10:12] CutDown: sure, i can share some anti techniques w hich might be interesting for bypassing dynamic sandbox executions if its interesting

[2024-12-17 10:13] kurt: [replying to CutDown: "sure, i can share some anti techniques w hich migh..."]
I'd appreciate that, anything helps!

[2024-12-17 10:13] CutDown: anti debugger checks would i think work with windows, if you try to write a debugger your self, and dynamically debug your own application (which should fail if its already analyzed by a debugger)

[2024-12-17 10:14] CutDown: regarding the sandboxing, give me a second i've wrote a sandbox driver in the past, so i'll share with you some of the things i've already bypassed

[2024-12-17 10:14] kurt: [replying to CutDown: "anti debugger checks would i think work with windo..."]
All I know is basic anti-debug stuff, peb, CheckRemoteDebuggerPresent, and QueryInformationProcess

[2024-12-17 10:15] kurt: patching/hooking them would be childs play and allow the process to be easily debugged anyhow

[2024-12-17 10:16] CutDown: vm hardware checks could be :
```rdtsc
cpuid
rdtsc```

rdtsc is an x86 opcode which takes machine time

using cpuid, a vm exit should happen and it might be a bit slower
from a sandbox point of view, i think you can implement the rdtsc \ cpuid differently to bypass this technique

[2024-12-17 10:16] kurt: I'd also love if u had any reverse-engineering or security analysis blogs so I could read them

[2024-12-17 10:16] kurt: [replying to CutDown: "vm hardware checks could be :
```rdtsc
cpuid
rdtsc..."]
ahh

[2024-12-17 10:17] kurt: noted!

[2024-12-17 10:18] CutDown: i'll share a few analysis related strings, i cant send them in one message because i dont have nitro, one second

[2024-12-17 10:18] CutDown: ```PWSTR pUnicodeAntiVmStrings[] = { L"vmacthlp", L"vmware", L"vmtools", L"virtual", L"vmhgfs", L"vmscsi", L"vmwvmci", L"vmci", L"vbox", L"virtualbox", L"virtual", L"wireshark", L"charles", L"fiddler" };```

[2024-12-17 10:18] CutDown: ```PWSTR pUnicodeVmPaths[] = { L"\\??\\HGFS", L"\\??\\VMCI", L"\\??\\VBOXGUEST", L"\\??\\GLOBAL\\HGFS", L"\\??\\VBOXMINIRDRDN", L"DRIVERS\\VBOXMOUSE.SYS", L"DRIVERS\\VBOXGUEST.SYS", L"DRIVERS\\VBOXSF.SYS", L"DRIVERS\\VBOXVIDEO.SYS", L"DRIVERS\\VMHGFS.SYS", L"DRIVERS\\VMMOUSE.SYS", L"SYSTEM32\\VBOXTRAY.EXE", L"SYSTEM32\\VBOXSERVICE.EXE", L"SYSTEM32\\VBOXOGLPASSTHROUGHSPU.DLL", L"SYSTEM32\\VBOXOGLPACKSPU.DLL", L"SYSTEM32\\VBOXOGLFEEDBACKSPU.DLL", L"SYSTEM32\\VBOXOGLERRORSPU.DLL", L"SYSTEM32\\VBOXOGLCRUTIL.DLL", L"SYSTEM32\\VBOXOGLARRAYSPU.DLL", L"SYSTEM32\\VBOXARRAYSPU.DLL", L"SYSTEM32\\VBOXOGL.DLL", L"SYSTEM32\\VBOXMRXNP.DLL", L"SYSTEM32\\VBOXHOOK.DLL", L"SYSTEM32\\VBOXDISP.DLL", L"SYSTEM32\\VBOXCONTROL.EXE", L"SBIEDLL.DLL", L"VMWAREUSER.EXE", L"VMWARETRAY.EXE", L"ORACLE\\VIRTUALBOX GUEST ADDITIONS\\" };```

[2024-12-17 10:18] CutDown: ```PWSTR pUnicodeVmRegistryKeys[] = {L"wireshark", L"charles", L"fiddler", \
                                L"HARDWARE\\ACPI\\DSDT\\VBOX__", L"HARDWARE\\ACPI\\FADT\\VBOX__", L"HARDWARE\\ACPI\\RSDT\\VBOX__", L"SOFTWARE\\ORACLE\\VIRTUALBOX GUEST ADDITIONS", L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Oracle VM VirtualBox Guest Additions", L"\\REGISTRY\\MACHINE\\SOFTWARE\\VMWARE, INC.\\VMWARE TOOLS", L"SOFTWARE\\VMWARE, INC.\\VMWARE TOOLS", L"SOFTWARE\\VMWARE, INC.\\VMWARE DRIVERS", L"\\REGISTRY\\MACHINE\\SOFTWARE\\VMWARE, INC.", L"SOFTWARE\\WINE", L"\\REGISTRY\\USER\\SOFTWARE\\WINE", L"SYSTEM\\ControlSet001\\Services\\VBOXGUEST", L"SYSTEM\\ControlSet001\\Services\\VBOXMOUSE", L"SYSTEM\\ControlSet001\\Services\\VBOXSERVICE", L"SYSTEM\\ControlSet001\\Services\\VBOXSF", L"SYSTEM\\ControlSet001\\Services\\VBOXVIDEO" };```

[2024-12-17 10:18] CutDown: ```PWSTR pUnicodeProcesses[] = { L"vmtoolsd.exe", L"vmwaretrat.exe", L"vmwareuser.exe", L"vmacthlp.exe", L"vboxservice.exe", L"vboxtray.exe", \
                            L"wireshark.exe", L"charles.exe", L"ollydbg.exe", L"idaq.exe", L"procmon.exe", L"procexp.exe", L"sndbox-test.exe" };```

[2024-12-17 10:19] CutDown: antivm strings - would probably be checked as files in the operating system, its a generic list and not an exact match
you should search for them very specifically, as in 
where in `C:\windows\...` you would have these files or whatever

[2024-12-17 10:20] CutDown: same for registry keys which might be an interesting check, and ofcourse `Process32snapshot` (I dont remember the exact API)

[2024-12-17 10:21] CutDown: you mentioned your self `ZwQuerySystemInformation`, some interesting checks could be (its a switchcase kind of API):

SystemBasicInformation
SystemProcessorInformation
SystemPerformanceInformation
SystemProcessInformation
SystemProcessorPerformanceInformation

[2024-12-17 10:21] CutDown: all of these checks are related to sandbox detection (due to CPU count, memory, hard disk, etc)

[2024-12-17 10:23] kurt: perfect, thank you! noted down everything, along with some info from https://github.com/can1357/hvdetecc
[Embed: GitHub - can1357/hvdetecc: Collection of hypervisor detections]
Collection of hypervisor detections. Contribute to can1357/hvdetecc development by creating an account on GitHub.

[2024-12-17 10:24] CutDown: another fun anti sandbox check would be:

```GetTickCount()
sleep(10)
GetTickCount()```

Its interesting as GetTickCount takes the tickcount from a hardcoded value inside a hardcoded address actually, its called `K_USER_SHARED_MEMORY` <--- I made this name up, but its something like that anyway, i dont remember the exact name but googling it should work

The idea here is that it would be hard to change the tick count from this shared memory across all processes on the operating system, and it can be tricky to do from the kernel (touching page tables and etc, its a bit more advanced then just hooking)

[2024-12-17 10:26] CutDown: also you could check entrypoints of functions to check if they are hooked or not
I think some antiviruses are hooking usermode functions regardless, so you should double verify that part

hooked functions would usually have their entry changed
on older windows x86 code you would have something like that:


```
NOP
NOP
NOP
NOP
NOP
---> MOV EDI, EDI ; <-- Function Code starts here
Entry
```
5 NOPS is exactly the size of `JMP <4 bytes>` opcode
and `MOV EDI, EDI` <-- is 2 bytes, which is the size of the opcode to `JMP -5` for hooks

[2024-12-17 10:26] Brit: there are many ways to hook that won't change the prolog / entry

[2024-12-17 10:26] CutDown: along with what i've written earlier about ZwQuerySystemInformation, you should also check out
`ZwQueryVolumeInformationFile`

[2024-12-17 10:27] CutDown: [replying to Brit: "there are many ways to hook that won't change the ..."]
yeah you could always hook inline, but that wont work in scale (CrowdStrike BSOD ahm ahm)

[2024-12-17 10:27] CutDown: not sure if its related, but you would want to avoid crashing things across versions

[2024-12-17 10:29] Brit: [replying to CutDown: "yeah you could always hook inline, but that wont w..."]
the crowdstrike issue wasn't related to a hook

[2024-12-17 10:29] Brit: just fyi

[2024-12-17 10:29] Brit: type missmatch in two parsers for the same format iirc

[2024-12-17 10:29] CutDown: oh, well i have no idea lol
i dont read into these stuff so much anymore

[2024-12-17 10:45] varaa: [replying to kurt: "hello, i was wondering what security measurements ..."]
all you really need is advanced antidbg, antiattach, anti-dmp, anti-everything tbh.

[2024-12-17 10:57] x86matthew: [replying to varaa: "all you really need is advanced antidbg, antiattac..."]
just implement anti-detection instead

[2024-12-17 10:57] x86matthew: then you don't need the rest

[2024-12-17 11:01] kurt: [replying to x86matthew: "just implement anti-detection instead"]
what would i need for a great anti-detection?

[2024-12-17 11:02] CutDown: what is anti detection?
what is being detected that you dont want it to be detected? as the software developer (the owner of the target application)

[2024-12-17 11:03] kurt: yea i was confused in anti-detection as what would the code be trying to do

[2024-12-17 11:03] CutDown: its the first time i hear that term lol

[2024-12-17 11:03] kurt: wouldnt anti-detection just be antidbg antiattach antidump antiinject just all in one?

[2024-12-17 11:03] x86matthew: [replying to CutDown: "its the first time i hear that term lol"]
there's good reason for that, i just made it up

[2024-12-17 11:03] x86matthew: i'm not being serious lol

[2024-12-17 13:57] mishap: larp hooks go crazy 🚀🚀🚀🚀💍💲😏

[2024-12-17 14:20] Brit: new technique, hadn't heard of it yet, is it like hwbp based ones?

[2024-12-17 14:48] x86matthew: yeah they're pretty advanced

[2024-12-17 14:48] x86matthew: even malcore can't detect them

[2024-12-17 14:59] Brit: <:kekw:904522300257345566>

[2024-12-17 17:51] emma: easiest way to protect software is not to write software 🧠

[2024-12-17 17:52] emma: write nothing, run nowhere

[2024-12-17 17:53] sync: [replying to emma: "easiest way to protect software is not to write so..."]
yep

[2024-12-17 17:53] sync: run ur code serverside

[2024-12-17 17:54] sync: then just say user issue when they say "Why this lag"

[2024-12-17 17:55] pinefin: [replying to emma: "easiest way to protect software is not to write so..."]
abstinence saves us all, once again

[2024-12-17 20:18] .: [replying to CutDown: "another fun anti sandbox check would be:

```GetTi..."]
how much would the difference be between the first and last GetTickCount call in ms whe running inside a vm

[2024-12-17 20:18] .: also for sleeping your thread are you using std::this_thread::sleep_for(std::chrono::milliseconds(10)), Sleep, SleepEx or NtDelayExecution

[2024-12-17 20:28] CutDown: [replying to .: "also for sleeping your thread are you using std::t..."]
for what i recall, sleep calls SleepEx
and everything eventually calls NtDelayExecution

[2024-12-17 20:28] CutDown: (You can also sleep using a timer with WaitForSingleObject and similar api)

[2024-12-17 20:28] CutDown: bottom line - it doesnt matter what you use

[2024-12-17 20:29] CutDown: GetTickCount accuracy is much like milliseconds

[2024-12-17 20:30] CutDown: so if you would do 
a = GetTickCount()
Sleep(10000)
b = GetTickCount()

true == (b-a > 9000)

[2024-12-17 20:30] CutDown: more or less something like that

[2024-12-17 20:31] CutDown: you could also.. just check, its the easiest thing to play with

[2024-12-17 20:58] .: in a real host the delta value would be like 10-30ms right?

[2024-12-17 21:01] Brit: better to just test it

[2024-12-17 21:01] Brit: and get ground truth

[2024-12-17 21:15] .: 
[Attachments: image.png]

[2024-12-17 21:16] Brit: the thing with this is that there's no guarantee that you won't get fucked by the scheduler, so you can't just do this test once

[2024-12-17 21:21] CutDown: try to stress test the system in addition
also, its an anti sandbox technique
so if you do sleep for like 100 seconds, the sandbox would usually reduce it drastically to something like 1 or 10 seconds (on the simpler ones)
so the differences would be very high than what you would expect

[2024-12-17 21:21] CutDown: and again, this is all the most basic form of anti sandboxing lol

[2024-12-17 21:23] Hunter: theres ways to get a precise sleep without busy-wait-locks but for the default system-wide clock resolution windows uses its set to 64hz iirc which results mostly in multiples of 15.625ms when calling sleep

[2024-12-17 21:31] .: [replying to CutDown: "for what i recall, sleep calls SleepEx
and everyth..."]
and yeah ik

[2024-12-17 21:31] .: later ill test in a vm

[2024-12-17 22:37] .: left is vm, right is host
[Attachments: image.png, test.cpp]

[2024-12-17 22:42] .: It's not a sandbox for malware analysis so it wont work because it will not truncate/accelerate sleep durations, maybe triage, any.run or malcore might do that, but it was cool to test if you would get way more chance of spikes in vms

[2024-12-18 03:15] daax: [replying to .: "left is vm, right is host"]
is there a specific reason for the avoidance of standard methods like rolling average of tsc+cpuid or the like?

[2024-12-18 04:03] Deleted User: hey i am trying to get the idb of multiple exes at once with some options in loading the binary so i tried with -B in ida command line it worked but i wanna load the file header also so any idea ?
- i tried the -S arg with loading a script as idapython and tried ida_loader to try to load it but its only accepting exssiting idb and when i am creating and empty idb and add to it , it dont work , so anyidea how to solve this problem ?

[2024-12-18 13:02] dennis: hello everyone

[2024-12-18 13:03] dennis: please know me where can buy or sell online game bot source

[2024-12-18 13:27] daax: [replying to dennis: "please know me where can buy or sell online game b..."]
start by reading the <#835634425995853834>.

[2024-12-18 17:03] pinefin: [replying to daax: "start by reading the <#835634425995853834>."]
he owns a mercedes hat though 😦🤣

[2024-12-18 17:06] BWA RBX: [replying to pinefin: "he owns a mercedes hat though 😦🤣"]
You know what that's a valid point

[2024-12-19 16:49] Sapphire: [replying to Deleted User: "hey i am trying to get the idb of multiple exes at..."]
I couldn't exactly get it working with this method, but you could use something like `idat -Ohexrays:<output-file>:ALL -A input` and just wrap it in a bash or batch script to iterate over files in a directory

[2024-12-20 02:26] sunbather: I have a question about modern anticheat but it's not for writing cheats or circumventing it. I'm writing a program that hooks keyboard events and changes certain keys to others (or multiple others) when a certain window is focused (Discord in this case).

I was wondering if anticheat looks for such programs that use `SetWindowsHookEx()` to hook the keyboard. It sounds like it might be a reliable detection for input scripts and such. I am not very knowledgeable in anticheat internals nor in Windows, which is why I am asking. The intention here is just to avoid getting banned for using my program. Is this safe?

[2024-12-20 04:45] daax: [replying to sunbather: "I have a question about modern anticheat but it's ..."]
why would you need your program running while gaming?

[2024-12-20 04:46] daax: in any case, yes, some do check

[2024-12-20 06:49] Deleted User: [replying to Sapphire: "I couldn't exactly get it working with this method..."]
i tried it , its not loading the file header

[2024-12-20 06:50] Deleted User: and you might pass the option to plugin with - to generate the command from idahexrays 
just something like that 
```
idat.exe -Ohexrays:-new:outfile:ALL -A input.exe
```

[2024-12-20 10:28] sunbather: [replying to daax: "why would you need your program running while gami..."]
Well my intention was to have the program start at boot and just leave it running in the background. Don't accessibility tools and/or macro-creation software hook the keyboard like this?

[2024-12-20 10:46] Brit: They certainly do, I have something a bit similar to manage key re-binding and have never had issues

[2024-12-20 10:46] Brit: Granted I sign my tool

[2024-12-20 10:50] sunbather: I see. Thanks!

[2024-12-20 13:49] sunbather: [replying to Brit: "Granted I sign my tool"]
If you want to share - which CA did you buy your cert from? I'm guessing using something self-signed is the same as just not signing it in my case

[2024-12-20 14:48] Brit: Azure sign

[2024-12-20 14:57] Brit: [replying to sunbather: "If you want to share - which CA did you buy your c..."]
But they probably won't bonk you unless you're doing more sus shit anyway

[2024-12-20 14:57] Brit: I ran that piece of code with a lot of acs before signing and never had issues

[2024-12-22 22:23] Glatcher: Guys, I'm developing overlay, which reads/writes game's memory. Will a cert help prevent AV from triggering on my exe file?

[2024-12-22 22:24] .: yes