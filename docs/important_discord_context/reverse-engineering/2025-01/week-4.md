# January 2025 - Week 4
# Channel: #reverse-engineering
# Messages: 122

[2025-01-20 14:21] expy: [replying to mrexodia: "Just published a release for convenience https://g..."]
is it possible to use structs from pdf btw? e.g. something similart to `dt` in windbg

[2025-01-20 14:35] mrexodia: [replying to expy: "is it possible to use structs from pdf btw? e.g. s..."]
no

[2025-01-20 14:35] mrexodia: but <@839216728008687666> is working on support for parsing C headers using libclang

[2025-01-20 14:35] mrexodia: perhaps after that

[2025-01-20 14:38] Brit: I'll implement it if you port x64dbg to rust :^)

[2025-01-20 14:40] diversenok: Will it literally be structs from pdf then?

[2025-01-20 14:42] diversenok: Who needs pdb 😛

[2025-01-20 14:45] expy: [replying to diversenok: "Who needs pdb 😛"]
having the UI to navigate PEB, TEB, KUSD would be nice probably

[2025-01-20 14:46] diversenok: Definitely, I'm just joking about a typo

[2025-01-20 14:50] Brit: presumably if header parsing is working you just throw phnt (single header) at it, and it should have peb teb and kusd

[2025-01-20 15:15] mrexodia: [replying to expy: "having the UI to navigate PEB, TEB, KUSD would be ..."]
Yeah but you can do this for 8 years already 😅
[Attachments: winternals.h]

[2025-01-20 15:16] mrexodia: Just load this header and it works

[2025-01-20 15:18] expy: [replying to mrexodia: "Just load this header and it works"]
apparently you're the only one who knew that

[2025-01-20 15:40] jonaslyk: 
[Attachments: out.h]

[2025-01-20 15:41] jonaslyk: added a few

[2025-01-20 15:56] diversenok: If only most of them did not depend on the OS version

[2025-01-20 16:24] Torph: 29MB wow

[2025-01-20 17:36] Glatcher: [replying to mrexodia: "Just published a release for convenience https://g..."]
That looks great!!!
Now I want to see the same feature in Cheat Engine xD

[2025-01-20 17:40] Glatcher: Wanted to say that Cheat Engine automatically recognizes types, especially pointers, however, I think its not necessary (at least for me)

[2025-01-21 16:42] 0x208D9: [replying to jonaslyk: ""]
bro didnt reply

[2025-01-21 16:43] jonaslyk: huh

[2025-01-21 16:43] jonaslyk: to what

[2025-01-21 16:43] 0x208D9: [replying to jonaslyk: "huh"]
check dm

[2025-01-21 16:43] 5pider: no no, flash us here

[2025-01-21 16:59] pinefin: i wanna see the flash

[2025-01-21 17:11] 5pider: with all due respect, behave yourself.

[2025-01-23 20:35] pinefin: is there a way to merge two .i64's that use the same version of executable?

[2025-01-23 20:35] pinefin: if not all is good, just wondering cause i have functions that are reversed in one and not in the other

[2025-01-23 20:35] pinefin: as well as local types

[2025-01-23 20:37] Brit: the low tech version of this is to export an idc and reimport while you're on the other db

[2025-01-23 20:37] Brit: the "good" way is to use ida teams's merge func

[2025-01-23 20:40] pinefin: i dont think i have access to ida teams

[2025-01-23 20:40] Brit: then idc it is

[2025-01-23 20:40] pinefin: yippee

[2025-01-23 20:40] pinefin: i'll test it out

[2025-01-23 20:41] pinefin: in

[2025-01-23 20:41] pinefin: t-3 hours

[2025-01-23 20:41] pinefin: thank you

[2025-01-23 20:43] pinefin: if only ida wasnt thousands of dollars

[2025-01-23 20:44] pinefin: im not saying i dont get it, but theres not really a great option for enthusiasts, ida home is okay but you only get 1 disassembler

[2025-01-23 20:44] pinefin: i was considering buying it but they only let you choose 1 (and i have more use cases than just x86/64)

[2025-01-24 19:53] pinefin: does anyone possibly know of any minimal hooking libraries for arm-v7 32bit?

[2025-01-25 02:36] elias: The Windows driver signing policy defines following exception for allowing cross signed drivers:

The PC was upgraded from an earlier release of Windows to Windows 10, version 1607.

I wonder, did anyone ever take a look at how Windows determines if the system was upgraded from an earlier version or not?

[2025-01-25 03:48] jonaslyk: think its a reg value in HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control

[2025-01-25 13:29] elias: <:peepoDetective:570300270089732096>

[2025-01-25 13:29] elias: and what prevents malware from changing that value so that it can load cross signed drivers

[2025-01-25 13:31] mrexodia: I think the policy is set up when you boot

[2025-01-26 16:13] dullard: Anyone have suggestions on how I would begin identifying why I'm unable to load a dll into a process. 

This isn't AC related but i'm sure there is some knowledge overlap.

[2025-01-26 16:15] .: what injection method are you using

[2025-01-26 16:16] dullard: very very dumb open proc > allocate > wpm > crt

[2025-01-26 16:16] .: does your dll load any external library

[2025-01-26 16:17] .: if so, what's the runtime library you're using
[Attachments: image.png]

[2025-01-26 16:18] dullard: Hmm, not sure if statically linking will affect it (I've tested against a simple target and my DLL is doing what its supposed to be doing)

Will give it a try anyway 🙂

[2025-01-26 16:19] .: is the process you're trying to inject protected with mitigation policies?

[2025-01-26 16:19] dullard: Nope, none of that

[2025-01-26 16:20] .: [replying to dullard: "Anyone have suggestions on how I would begin ident..."]
attach a debugger to the process before injecting and check the unload image reason if any

[2025-01-26 16:20] dullard: There wasn't an unload reason, it didn't even log a message

[2025-01-26 16:20] dullard: Worked on a test target but not the target i'm trying to load into

[2025-01-26 16:22] .: can you show the dll entry point

[2025-01-26 16:23] .: are you calling DisableThreadLibraryCalls and creating a new thread when injecting?

[2025-01-26 16:23] dullard: new thread

[2025-01-26 16:23] .: does the remote thread get created in the target process?

[2025-01-26 16:25] dullard: I have about a 5 second window while i'm doing everything 😄 and the target needs to be spawned by another process (making everything quite annoying 😄 to deal with )

Let me confirm a new thread is being created, one sec

[2025-01-26 16:28] dullard: Hmm so I am getting thread creation
```
Thread 11868 created, Entry: <kernel32.LoadLibraryA> (76652340), Parameter: 05730000
Thread 11868 exit
DLL Loaded: 663B0000 C:\Windows\SysWOW64\sxs.dll
Thread 1856 created, Entry: <kernel32.LoadLibraryA> (76652340), Parameter: 05740000
Thread 1856 exit
Thread 9696 created, Entry: <kernel32.LoadLibraryA> (76652340), Parameter: 05750000
```
(I spammed the injector to load 3 times)

[2025-01-26 16:32] .: is the main thread of your dll alive while the other thread is initializing/working?

[2025-01-26 16:34] .: assuming the injection is fine, does the dll exit because any error condition you're checking for? did you try to debug the code by spawning MessageBox, by logging a message, etc?

[2025-01-26 16:35] dullard: This is the code right now 
```c
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <phnt_windows.h>
#include <phnt.h>

#if defined _M_IX86
#pragma comment( lib, "C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.18362.0\\um\\x86\\ntdll.lib" )
#elif defined _M_X64
#pragma comment( lib, "C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.18362.0\\um\\x64\\ntdll.lib" )
#endif

extern "C" __declspec(dllexport)
BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD ul_reason_for_call,
    LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DbgPrint("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
```
Its compiled as an x86 multithread Debug DLL  

I attach with x86dbg and spam the injector and the thread gets spawned but not debug output

[2025-01-26 16:36] .: call DbgPrint in another thread different than the main one, or allocate a console first and output debug messages to it

[2025-01-26 16:36] dullard: It may be the injector tbh, I'm just using some random copy pasta, didn't fully read the code <:kekw:899071675524591637>

[2025-01-26 16:36] Torph: maybe make an unconditional debug print just for sanity check

[2025-01-26 16:36] Torph: [replying to dullard: "It may be the injector tbh, I'm just using some ra..."]
there's only so much that can go wrong with this method tho

[2025-01-26 16:37] dullard: I know its not the DbgPrint

[2025-01-26 16:37] dullard: x86dbg catches those

[2025-01-26 16:37] dullard: and even if its sent from DllMain

[2025-01-26 16:37] dullard: Something else must be up

[2025-01-26 16:38] Torph: i actually didnt know you could debug injected DLLs I've been doing just print debugging this whole time

[2025-01-26 16:38] dullard: No, I attach to a test program, i.e. just getch() in main, and then inject the dll

[2025-01-26 16:39] .: just to be sure, can you spawn a new thread, allocate a console and printf a message to it

[2025-01-26 16:40] .: ```cpp
void SpawnConsole() {
    if (!AllocConsole()) {
        return;
    }

    FILE* file;
    if (freopen_s(&file, "CONOUT$", "w", (__acrt_iob_func(1))) != 0 ||
        freopen_s(&file, "CONOUT$", "w", (__acrt_iob_func(2))) != 0 ||
        freopen_s(&file, "CONIN$", "r", (__acrt_iob_func(0))) != 0) {
        return;
    }

    std::cout.clear();
    std::cerr.clear();
    std::cin.clear();

    HANDLE hConout = CreateFileA("CONOUT$", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hConout == ((HANDLE)(LONG_PTR)-1)) {
        return;
    }

    if (!SetStdHandle(((DWORD)-11), hConout) || !SetStdHandle(((DWORD)-12), hConout)) {
        CloseHandle(hConout);
        return;
    }
}
```

[2025-01-26 16:40] .: spawn the console in the new thread and do some std::cout

[2025-01-26 16:41] dullard: As I thought the injector was bad 😄 
Used one from a previous project and 
```
DLL Loaded: 62E30000 C:\username\project\Debug\SimpleMessageBox.dll
DebugString: "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
Thread 13412 exit
```

[2025-01-26 16:41] dullard: 🥴

[2025-01-26 16:42] dullard: Thanks for <:rubberDukeDebugging:1136434905144897627>

[2025-01-26 16:43] .: weird to see an injector making a loadlibrary injection wrongly

[2025-01-26 16:43] dullard: Its likely my fuck up, I changed some logic so I more than likely broke it

[2025-01-26 16:53] diversenok: So what you're saying is, you tried to debug DLL loading without being sure whether the target process attempts to load it at all

[2025-01-26 16:56] dullard: Somewhat. 

Initially I was just manually loadlibrary loading the instrumentation dll into a test program where I was manually calling functions which I was planning to hook. 

Copy pasted a random loadlibrary injector and tested it against the same test program with no problems. Added a while true + break loop where if the process gets spawned it loads the DLL and broke it 😄

[2025-01-26 16:56] Brit: you know how it is, you have some code that works, you assume it'll keep working don't read too much into it

[2025-01-26 16:57] dullard: I kinda jumped to the conclusion that the target was protecting itself due to the nature of the software

[2025-01-26 16:57] dullard: <:skill_issue:1210171860063617074>

[2025-01-26 16:57] Brit: injecting into chrome :^)

[2025-01-26 16:57] dullard: No, thats easy-ish to bypass

[2025-01-26 16:57] .: wait does chrome have any protection

[2025-01-26 16:57] dullard: Its once again software which confirms your pc is in a "safe" state to connect to company resources

[2025-01-26 16:57] dullard: yes it does

[2025-01-26 16:57] dullard: edge / chrome has some mitigation flags applied to certain processes

[2025-01-26 16:58] dullard: not all of them have the flags applied but iirc some of the renderer processes do

[2025-01-26 16:58] dullard: i.e. 

DEP (permanent); ASLR (high entropy); Win32k system calls disabled; Extension points disabled; CF Guard; Signatures restricted (Microsoft only); Non-system fonts disabled; Images restricted (remote images, low mandatory label images); Child process creation disabled; SMT-thread branch target isolation

[2025-01-26 16:58] .: oh

[2025-01-26 16:59] .: didnt know they used SetMitigationPolicies

[2025-01-26 17:00] .: [replying to dullard: "Somewhat. 

Initially I was just manually loadlibr..."]
iat hook?

[2025-01-26 17:00] diversenok: Browsers can also patch `LdrInitializeThunk` or similar to neutralize remote threads

[2025-01-26 17:00] dullard: [replying to .: "iat hook?"]
I'm just using detours as I had alot of boilerplate code left over from a very similar project (which was known to be working)

[2025-01-26 17:01] .: yes but what are you detouring

[2025-01-26 17:01] .: im curious

[2025-01-26 17:01] dullard: things n stuff

[2025-01-26 17:01] dullard: Just simple Win32/COM hooking for something which does software/os version/registry/filesystem checks

[2025-01-26 17:02] dullard: Annoyingly I'm not as fortunate as I was when I did the same thing with Zscaler where the process was permanantly running 😄 this software does it ad-hoc and the process is alive for 3/5 seconds

[2025-01-26 17:03] diversenok: Either way, a good way to debug DLL loading (such dependency resolving, etc.) is to enable loader snaps for the given process via IFEO and then attach a debugger. Ldr will generate lots of useful debug messages for you

[2025-01-26 17:04] .: [replying to diversenok: "Browsers can also patch `LdrInitializeThunk` or si..."]
i always assumed browsers and that kind of stuff would not get into those kind of things

[2025-01-26 17:05] dullard: Oh very nice, I wasnt sure If I could use IFEO with non-microsoft executables 😄 

and for whatever reason I wasn't able to get appinitdlls to work on server 2022 <:ThinkDifferent:321299297456881664> (no secureboot)

[2025-01-26 17:05] .: like, if remote threads are patched, people would do exception hooking or 100 other different methods

[2025-01-26 17:06] diversenok: [replying to .: "i always assumed browsers and that kind of stuff w..."]
It's mostly to stop other shitty software from crashing them (and causing users to blame the browser rather then a dozen tweakers they installed). It's not a security measure by any means

[2025-01-26 17:08] diversenok: [replying to dullard: "Oh very nice, I wasnt sure If I could use IFEO wit..."]
AppInit was officially deprecated and might not work I think. Or at least require the DLL to be signed or something

[2025-01-26 17:08] dullard: [replying to diversenok: "AppInit was officially deprecated and might not wo..."]
There was `RequireSignedAppInit_DLLs`

[2025-01-26 17:08] dullard: ```
reg add "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Windows" /v AppInit_DLLs /d "C:\path\here\dll.dll" /f 
reg add "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Windows" /v LoadAppInit_DLLs /d 1 /f 
reg add "HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Windows" /v RequireSignedAppInit_DLLs /d 0 /f 
```

[2025-01-26 17:09] dullard: May be the case that its deprecated !

[2025-01-26 17:09] diversenok: ¯\_(ツ)_/¯

[2025-01-26 17:09] dullard: Will have a play with IFEO

[2025-01-26 17:09] diversenok: There are also tools that expose these per-executable IFEO settings in GUI

[2025-01-26 17:10] diversenok: GFlagsX, for instance. Just click on the Image page, select the filename, and enable loader snaps

[2025-01-26 17:14] dullard: ~~Are IFEO keys process arch specific ?~~