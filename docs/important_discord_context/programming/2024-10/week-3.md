# October 2024 - Week 3
# Channel: #programming
# Messages: 166

[2024-10-14 06:09] rin: Anyone know of any articles detailing the new window system in windows 11.

[2024-10-14 13:46] 0x208D9: [replying to rin: "Anyone know of any articles detailing the new wind..."]
do u mean the one related to dwm?

[2024-10-14 18:17] rin: [replying to 0x208D9: "do u mean the one related to dwm?"]
I don't know, i just ran into a problem with screen shots with new apps. Like you can get a handle and getting a bitmap like normal doesn't return any errors but its completely black or partially  black.

[2024-10-14 20:39] diversenok: If it's black only for a specific window, then it might be the result of window affinity (i.e., a DRM feature)

[2024-10-14 20:39] diversenok: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowdisplayaffinity
[Embed: SetWindowDisplayAffinity function (winuser.h) - Win32 apps]
Stores the display affinity setting in kernel mode on the hWnd associated with the window.

[2024-10-14 20:39] diversenok: `WDA_MONITOR` makes the window region black on screenshots

[2024-10-14 20:40] rin: one sec Ill get an example for you.

[2024-10-14 20:41] diversenok: `WDA_EXCLUDEFROMCAPTURE` makes the window transparent during full-screen capture (and probably black on window-specific capture)

[2024-10-14 20:45] rin: this is how windows 11 file explorer looks when you try to take a screen shot.
[Attachments: image.png]

[2024-10-14 20:46] rin: its missing the directory listing and top navigation section

[2024-10-14 20:47] rin: all new windows apps have similar problems

[2024-10-14 20:48] rin: and im not doing anything weird, this is on active desktop on current desktop thread.

[2024-10-14 20:50] rin: example of notepad
[Attachments: image.png]

[2024-10-14 20:51] diversenok: I was about to ask whether you sure that the region you screenshot is correct (i.e., there is no bitmap cropping), but the icon shows that it apparently is fine

[2024-10-14 20:51] rin: I don't think its a drm issue.

[2024-10-14 20:51] diversenok: Hmm

[2024-10-14 20:51] rin: [replying to diversenok: "I was about to ask whether you sure that the regio..."]
works fine on win10

[2024-10-14 20:51] diversenok: Yeah, not with explorer/notepad anyway

[2024-10-14 20:52] rin: [replying to diversenok: "I was about to ask whether you sure that the regio..."]
when you are on active desktop should not matter, the parent handle is all you should need

[2024-10-14 20:53] diversenok: I meant that BitBlt accepts a bunch or coordinates that are possible to mess up

[2024-10-14 20:55] rin: desktop handle works
[Attachments: image.png]

[2024-10-14 20:57] rin: [replying to diversenok: "I meant that BitBlt accepts a bunch or coordinates..."]
yeah but what would change between win10 and win11 that would cause it to break on apps only with the new windows system.

[2024-10-14 20:57] diversenok: ¯\_(ツ)_/¯

[2024-10-14 20:58] diversenok: What exactly are you capturing from? The window device context, composition/redirection surface, anything else?

[2024-10-14 20:58] x86matthew: are you trying to write a hidden desktop type thing?

[2024-10-14 20:58] rin: [replying to x86matthew: "are you trying to write a hidden desktop type thin..."]
this is active desktop

[2024-10-14 20:59] x86matthew: well, are the windows that you're trying to capture in the background or foreground?

[2024-10-14 20:59] rin: foreground

[2024-10-14 21:00] x86matthew: in that case why not just capture the desktop and use bitblt to only copy the co-ordinates of the target window

[2024-10-14 21:02] rin: yeah but I also want to know what changed between win10 and win11, my guess is that you need to capture child windows separably now.

[2024-10-14 21:02] rin: even on active desktop which use to not be the case.

[2024-10-14 21:04] x86matthew: could be related to hw acceleration

[2024-10-14 21:04] rin: ill work on this problem and post a solution here if I find it

[2024-10-14 21:04] x86matthew: are you sending a wm_print message to the window

[2024-10-14 21:04] rin: no

[2024-10-14 21:04] x86matthew: or calling printwindow()

[2024-10-14 21:04] rin: active desktop doesnt need it

[2024-10-14 21:04] x86matthew: try printwindow with undocumented PW_RENDERFULLCONTENT flag

[2024-10-14 21:05] diversenok: Yeah, that's why I'm asking what exactly are you capturing

[2024-10-14 21:05] diversenok: BitBlt captures device context

[2024-10-14 21:05] diversenok: PW_RENDERFULLCONTENT captures redirection surface

[2024-10-14 21:06] x86matthew: also just check for any weirdness with spy++ or similar if you haven't done this already

[2024-10-14 21:10] diversenok: Do you have trouble exclusively with the non-client area of windows?

[2024-10-14 21:11] diversenok: It looks so from the screenshots

[2024-10-14 21:12] diversenok: Win 11 might have some changes regarding how DWM renders them

[2024-10-14 21:16] rin: [replying to x86matthew: "try printwindow with undocumented PW_RENDERFULLCON..."]

[Attachments: image.png]

[2024-10-14 21:16] rin: works

[2024-10-14 22:31] luci4: [replying to x86matthew: "are you trying to write a hidden desktop type thin..."]
Is there any implementation of that on GitHub?

[2024-10-14 22:31] luci4: I couldn't find one that doesn't use VNC

[2024-10-14 22:35] luci4: Oh I just did <:slightlyembarrassed:834483370117300336>

[2024-10-14 23:48] rin: [replying to luci4: "I couldn't find one that doesn't use VNC"]
how would you have a hvnc without vnc lol?

[2024-10-14 23:51] rin: this is the one I originally learned on but its not relevant anymore https://github.com/Meltedd/HVNC
[Embed: GitHub - Meltedd/HVNC: Standalone HVNC Client & Server | Coded in C...]
Standalone HVNC Client & Server | Coded in C++ (Modified Tinynuke) - Meltedd/HVNC

[2024-10-15 00:14] 5pider: [replying to luci4: "Is there any implementation of that on GitHub?"]
https://github.com/WKL-Sec/HiddenDesktop
[Embed: GitHub - WKL-Sec/HiddenDesktop: HVNC for Cobalt Strike]
HVNC for Cobalt Strike. Contribute to WKL-Sec/HiddenDesktop development by creating an account on GitHub.

[2024-10-15 05:13] x86matthew: [replying to rin: "how would you have a hvnc without vnc lol?"]
none of those samples use vnc

[2024-10-15 05:13] x86matthew: despite the name

[2024-10-15 05:13] x86matthew: vnc is the protocol

[2024-10-15 05:13] rin: [replying to x86matthew: "vnc is the protocol"]
ik

[2024-10-15 05:13] rin: I use vnc as a general term tho

[2024-10-15 05:16] x86matthew: whoever wrote that code doesn't know how to use network sockets properly either, i'm surprised it works at all

[2024-10-15 05:16] rin: which one

[2024-10-15 05:17] x86matthew: in fact the second one is just a paste of the first

[2024-10-15 05:17] x86matthew: both

[2024-10-15 05:17] rin: [replying to x86matthew: "in fact the second one is just a paste of the firs..."]
true lol

[2024-10-15 05:20] rin: I see people also use the term HRDP, though I don't really know if this is also just a loose definition for the same thing.

[2024-10-15 05:20] rin: or actually has some form of authentication

[2024-10-15 18:35] lytocahtqreu: anyone know the reason ?
[Attachments: image.png, image.png]

[2024-10-15 18:44] dullard: [replying to lytocahtqreu: "anyone know the reason ?"]
get rid of ; before "inline" keyword

[2024-10-15 18:44] dullard: it still looks a bit funky, idk C++ so 🤷‍♂️

[2024-10-15 18:45] Azrael: [replying to lytocahtqreu: "anyone know the reason ?"]
Those errors aren't connected.

[2024-10-15 18:46] Azrael: Looks weird though, maybe have the I on another line.

[2024-10-15 18:50] lytocahtqreu: i changed it to a namespace and still getting this error

[2024-10-15 18:52] mishap: brother

[2024-10-15 18:52] Azrael: Still not related.

[2024-10-15 18:52] Brit: you have to stop pasting csgo cheats

[2024-10-15 18:53] Azrael: Oh, that's what it is?

[2024-10-15 18:53] Azrael: Lmfao.

[2024-10-15 18:53] Brit: 100%

[2024-10-15 18:53] Azrael: Considering that those errors aren't related to the code that he's showing, I don't know what he wants.

[2024-10-15 18:53] snowua: std::uint64_t

[2024-10-15 18:53] snowua: https://tenor.com/view/dies-of-cringe-cringe-gif-20747133

[2024-10-15 18:54] Azrael: Have you tried navigating to the error?

[2024-10-15 18:55] contificate: won't be laughing when he headshots you in ranked matches buddy

[2024-10-15 18:55] Azrael: I don't even play any games, well except for Batman: AK. But that's a single player game.

[2024-10-15 18:55] lytocahtqreu: [replying to Azrael: "Considering that those errors aren't related to th..."]
so explain then

[2024-10-15 18:55] contificate: honestly ask chatgpt at this point

[2024-10-15 18:55] Azrael: [replying to lytocahtqreu: "so explain then"]
I'm going to put a bullet through my skull.

[2024-10-15 18:56] lytocahtqreu: high ego huh

[2024-10-15 18:56] Azrael: For sure.

[2024-10-15 18:56] lytocahtqreu: lmao

[2024-10-15 18:56] Azrael: It's levitating right now, it's super high.

[2024-10-15 18:57] lytocahtqreu: good

[2024-10-15 18:57] lytocahtqreu: im high of weed so brain not 100% rn

[2024-10-15 18:57] Azrael: Yeah, it's pretty good.

[2024-10-15 19:41] Deleted User: [replying to lytocahtqreu: "anyone know the reason ?"]
smartest csgo cheater

[2024-10-16 08:44] 0x208D9: [replying to contificate: "won't be laughing when he headshots you in ranked ..."]
wont be laughing when i look at the ground and he gets headshot on the T spawn

[2024-10-16 08:45] 0x208D9: [replying to Azrael: "Oh, that's what it is?"]
"CBaseClient" gives it away, pretty much in source sdk atp

[2024-10-16 08:46] Azrael: I don't cheat.

[2024-10-16 08:46] 0x208D9: ^^ loyalty 🫵🥰

[2024-10-16 08:47] 0x208D9: idk man would sound like an asshole if i dropped "you cheat on cs, i cheat with ur girl, we aint same br0"

[2024-10-16 08:48] Azrael: [replying to 0x208D9: "idk man would sound like an asshole if i dropped "..."]
They're both shit.

[2024-10-16 08:48] 0x208D9: also nice resource if anyone wanna pick up : https://cp-algorithms.com/navigation.html
[Embed: Navigation - Algorithms for Competitive Programming]
The goal of this project is to translate the wonderful resource http://e-maxx.ru/algo which provides descriptions of many algorithms and data structures especially popular in field of competitive prog

[2024-10-16 08:48] 0x208D9: [replying to Azrael: "They're both shit."]
real

[2024-10-17 13:29] Torph: [replying to 0x208D9: "also nice resource if anyone wanna pick up : https..."]
Snake!! Don't abbreviate Competitive Programming! SNAAAKE!!

[2024-10-17 13:35] 0x208D9: [replying to Torph: "Snake!! Don't abbreviate Competitive Programming! ..."]
whoever reports my link to discord is the true snake

[2024-10-17 23:56] Deleted User: Does anyone know how to get the x64 version of PEB through GetThreadContext
I can only find resources for 32-bit:
```if (GetThreadContext(pi.hThread, &ctx)) {
    ReadProcessMemory(pi.hProcess, (PVOID)(ctx.Ebx + 8), &targetPEB, sizeof(PEB), 0);  
    printf("[+] Located PEB from target at address: 0x%08p\n\n", (PUINT)ctx.Ebx);   
}```

[2024-10-17 23:57] Yaki: Cheating can be fun offline too

[2024-10-18 00:23] elias: [replying to Deleted User: "Does anyone know how to get the x64 version of PEB..."]
Couldnt you use NtQueryInformationProcess instead of GetThreadContext?

[2024-10-18 00:24] Deleted User: I definitely could

That's what I get for trying to be all fancy, ig

[2024-10-18 00:24] elias: PROCESS_BASIC_INFORMATION has a PebBaseAddress field

[2024-10-18 00:26] Deleted User: <Winternl.h>, here we go

[2024-10-18 00:26] elias: [replying to Deleted User: "I definitely could

That's what I get for trying t..."]
Im not sure using some undocumented characteristics instead of documented features is considered fancy <:peepoDetective:570300270089732096>

[2024-10-18 00:26] Deleted User: shhHHHHHHhhhhhh

[2024-10-18 00:40] Deleted User: Annnnnnnnnnd, everything's working fine now

[2024-10-18 00:40] Deleted User: <:mmmm:904523247205351454>

[2024-10-18 00:41] Deleted User: Gotta stop overthinking shit fr

[2024-10-18 04:25] Matti: [replying to Deleted User: "<Winternl.h>, here we go"]
don't use winternl.h

[2024-10-18 04:25] Matti: ever

[2024-10-18 04:26] Matti: were you trying to read a native PEB address from a native exe, a 32 bit PEB from a native exe or a native PEB from a 32 bit exe]

[2024-10-18 04:26] Matti: which API you need to use differs for each case

[2024-10-18 04:26] Matti: [replying to Deleted User: "Annnnnnnnnnd, everything's working fine now"]
sooooooo how did you solve it

[2024-10-18 04:28] Matti: [replying to Deleted User: "Does anyone know how to get the x64 version of PEB..."]
the reason this won't work is because of 32 bit and 64 bit calling convention differences

[2024-10-18 04:28] Matti: pretty obvious tbh

[2024-10-18 04:29] Matti: whatever Ebx is supposed to be here anyway even on 32 bit...

[2024-10-18 04:32] Matti: oh now I get it

[2024-10-18 04:32] Matti: this code is using three kinds of stupid to get the PEB address

[2024-10-18 04:33] Matti: by using some thread's register value to read it from there... mmm

[2024-10-18 04:36] Matti: bonus question: let's say you DID want to do it this way, but the (still 32 bit) process is now running on a 64 bit OS, and your own program is 64 bit too. how do you get the thread context in that case?

[2024-10-18 04:37] Matti: the 32 bit context that is, from which you can raad Ebx...

[2024-10-18 04:46] Deleted User: Yeah

This is the part where I go to bed

[2024-10-18 04:47] Matti: `NtQuery/SetInformationThread` with `ThreadWow64Context`,  passing WOW64_CONTEXT, of course!

[2024-10-18 04:48] Deleted User: Still a lot I have to study, I see
I'll put that in the notes

[2024-10-18 04:49] Matti: here, it's super simple
only like 70-100 LOC for the concept, 50 for relocations, and the other 400 is for wow64 stuff
[Attachments: hollowing.cpp]

[2024-10-18 04:50] Deleted User: SIMPLE-

[2024-10-18 04:50] Deleted User: okayokay

[2024-10-18 04:50] Deleted User: christ

[2024-10-18 04:51] Matti: I can't guarantee it, but if you read it you might learn something

[2024-10-18 04:51] Matti: [replying to Deleted User: "Does anyone know how to get the x64 version of PEB..."]
unlike this code

[2024-10-18 04:52] Matti: by the way I'm pretty sure this is missing some ARM32/64 relocation types nowadays

[2024-10-18 04:53] Matti: as well as dynamic relocation in general, but idt that is relevant here

[2024-10-18 04:53] Deleted User: dawg please speak english, I'm going to hurl

[2024-10-18 04:54] Matti: pretty sure it's english

[2024-10-18 04:54] Deleted User: on second thought, don't even bother, I actually do gotta go to bed
gotta wake up early for my shitty sales job

[2024-10-18 04:54] Matti: but if you've got questions feel free to ask

[2024-10-18 04:54] Deleted User: ...tomorrow

[2024-10-18 04:55] Matti: [replying to Deleted User: "on second thought, don't even bother, I actually d..."]
sucks to be you, my weekend just started

[2024-10-18 04:55] Matti: meaning I'll also be sleeping

[2024-10-18 04:56] Deleted User: https://tenor.com/view/angry-kids-gif-13253444

[2024-10-18 04:56] Deleted User: fml

[2024-10-18 04:56] Deleted User: anyway, peace out

[2024-10-18 04:56] Matti: peace out

[2024-10-18 04:56] Matti: IDF in

[2024-10-18 07:16] brymko: <:topkek:904522829616263178>

[2024-10-18 09:39] Deleted User: [replying to Matti: "IDF in"]
Israeli Defense Forces...? I can't keep up with all these new acronyms for everything 

Also jeez, don't you people ever sleep

[2024-10-18 10:00] Azrael: No.

[2024-10-18 11:17] elias: legends never sleep

[2024-10-19 18:07] psykana: Is anyone familiar with typeinfo structs used by RTTI?
I am parsing debug symbols of an older game to reconstruct source tree and generate dummy class and function defs for binary matching.
Now I want to walk typeinfo structs and establish class inheritance etc but I can't make sense of them.

[2024-10-19 18:09] psykana: Like here the third entry sometimes has a pointer to another class typeinfo, sometimes it doesn't. Those `0x2` `0x402` flags also don't tell much.
[Attachments: F15E3E94-2D42-4153-AC70-F2989F715E76.png]

[2024-10-19 18:10] psykana: I have the source code for GCC that was used for this binary, but I don't understand much there...

[2024-10-19 21:32] North: [replying to psykana: "I have the source code for GCC that was used for t..."]
Here's a good post explaining how RTTI structs work on MSVC https://www.lukaszlipski.dev/post/rtti-msvc/
[Embed: RTTI Internals in MSVC]
Internals of the RTTI on Windows using MSVC

[2024-10-19 21:33] North: Didn't read through all of it but looks like GCC makes all the RTTI structs in this file

[2024-10-19 21:33] North: https://chromium.googlesource.com/chromiumos/third_party/gcc/+/refs/heads/factory-beltino-5140.14.B/gcc/cp/rtti.c

[2024-10-20 00:47] psykana: [replying to psykana: "Like here the third entry sometimes has a pointer ..."]
Found just what I needed:
https://hex-rays.com/hubfs/freefile/Recon-2012-Skochinsky-Compiler-Internals.pdf
https://github.com/nccgroup/SusanRTTI/blob/master/gcc.py
[Embed: SusanRTTI/gcc.py at master · nccgroup/SusanRTTI]
Another RTTI Parsing IDA plugin. Contribute to nccgroup/SusanRTTI development by creating an account on GitHub.

[2024-10-20 00:49] psykana: Now I gotta figure out how to traverse a tree with 32 bit pointers with 64 bit code 🗿

[2024-10-20 08:41] 0x208D9: [replying to psykana: "Found just what I needed:
https://hex-rays.com/hub..."]
idk how easy using that is for u, but what i would recommend is https://github.com/herosi/classinformer
[Embed: GitHub - herosi/classinformer: IDA Class Informer plugin for IDA 8....]
IDA Class Informer plugin for IDA 8.x and 9.x. Contribute to herosi/classinformer development by creating an account on GitHub.

[2024-10-20 09:34] Deleted User: msvc rtti isn't gcc rtti

[2024-10-20 09:35] 0x208D9: [replying to Deleted User: "msvc rtti isn't gcc rtti"]
oh right, he was looking for gcc