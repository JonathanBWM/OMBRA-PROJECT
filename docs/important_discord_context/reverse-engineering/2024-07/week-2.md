# July 2024 - Week 2
# Channel: #reverse-engineering
# Messages: 91

[2024-07-09 11:45] luci4: Could anyone explain what the last three arguments represent in this case?
[Attachments: image.png]

[2024-07-09 11:46] luci4: `DesktopInfo`, `ShellInfo` and `RuntimeData`

[2024-07-09 11:46] luci4: Couldn't find anything on them, unfortunately

[2024-07-09 11:46] luci4: Thanks!

[2024-07-09 11:58] diversenok: `DesktopInfo` is the full name of the desktop to launch the process on; see `lpDesktop` in `STARTUPINFOW`

[2024-07-09 12:02] diversenok: `ShellInfo`, I think that's where the shell passes the hotkey information when launching from .lnk files (`dwHotKey` in `SHELLEXECUTEINFOW` and `lpReserved` in `STARTUPINFOW`)

[2024-07-09 12:04] diversenok: `RuntimeData` is `cbReserved2` + `lpReserved2` in `STARTUPINFOW` and I think is unused

[2024-07-09 12:04] diversenok: So the last two are for some boring legacy stuff, the desktop one might be useful

[2024-07-09 12:05] diversenok: But you can pass NULLs in all of them

[2024-07-09 12:19] diversenok: Apparently `RuntimeData` is just a way to pass custom data and is up to the application to interpret

[2024-07-09 12:38] luci4: [replying to diversenok: "`DesktopInfo` is the full name of the desktop to l..."]
Wow thanks a lot

[2024-07-09 12:41] luci4: I'm making a hollower and I want to take advantage of these 3
```
    HANDLE StandardInput;
    HANDLE StandardOutput;
    HANDLE StandardError;
```
Which is why I'm using `RtlCreateUserProcessParameters`

[2024-07-09 12:43] diversenok: Do you need to spawn a process via Native API? Doing standard I/O redirection is easier to implement with regular CreateProcess

[2024-07-09 12:44] luci4: [replying to diversenok: "Do you need to spawn a process via Native API? Doi..."]
Not particularly, I'm just curious to see if it works

[2024-07-09 12:45] diversenok: Or maybe I'm confusing it with making the child process inherit the console

[2024-07-09 12:45] luci4: Otherwise I would just do this:

https://learn.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output
[Embed: Creating a Child Process with Redirected Input and Output - Win32 apps]
The example in this topic demonstrates how to create a child process using the CreateProcess function from a console process.

[2024-07-09 12:47] diversenok: Yeah, no, I think this example translates well into native API, so it shouldn't be a problem

[2024-07-09 12:48] luci4: I could also use `ConsoleHandle` from the same structure

[2024-07-09 12:48] luci4: Sort of like

[2024-07-09 12:48] luci4: 
[Attachments: image.png]

[2024-07-09 12:48] luci4: Curious to see if it'd work

[2024-07-09 12:49] diversenok: Do you want to use `NtCreateUserProcess` or `RtlCreateUserProcess`? Because there are some differences regarding it

[2024-07-09 12:49] luci4: Oh I'm using RtlCreateUserProcess

[2024-07-09 12:49] luci4: That's why I'm initializing that struct

[2024-07-09 12:50] diversenok: The struct is common for all native API process creation methods though

[2024-07-09 12:50] diversenok: Including `NtCreateProcessEx`

[2024-07-09 12:50] luci4: TIL, haven't used `NtCreateProcessEx` before

[2024-07-09 12:51] diversenok: It's an API where you need to do everything yourself, which is kind of tedious

[2024-07-09 13:22] luci4: [replying to diversenok: "It's an API where you need to do everything yourse..."]
Native API is always kinda tedious, lol

[2024-07-09 13:23] luci4: Guess I'm gonna use completion routines or smth to communicate with the hollowed process

[2024-07-09 13:24] diversenok: [replying to luci4: "Native API is always kinda tedious, lol"]
It might require a bit more code and proper error checking, but it's way more consistent than Win32 API in its design

[2024-07-09 13:25] diversenok: And consistency/predictability saves time

[2024-07-09 13:25] luci4: [replying to diversenok: "It might require a bit more code and proper error ..."]
There are some functions for which I never go with the kernel32 variant

[2024-07-09 13:27] diversenok: Yeah

[2024-07-09 13:30] x86matthew: [replying to diversenok: "It might require a bit more code and proper error ..."]
it's a shame they didn't follow this philosophy with win32k 🫠

[2024-07-09 13:30] luci4: [replying to luci4: "Guess I'm gonna use completion routines or smth to..."]
I take that back, I'm lazy so I'll just loop on PeekNamedPipe lol

[2024-07-09 13:30] x86matthew: it's as though they pasted legacy usermode code straight into the kernel

[2024-07-09 13:31] diversenok: win32k is the land of horrors

[2024-07-09 13:33] diversenok: Sometimes it feels as if they are intentionally making it hard to check whether an operation has succeeded

[2024-07-09 13:48] x86matthew: yeah the win32 error indicator roulette is fun

[2024-07-09 13:49] x86matthew: is it going to be `ret == 0`, `ret != 0`, `ret == -1`, `ret <= 32`, or `ret == failed && GetLastError != NO_ERROR`

[2024-07-09 14:01] Brit: 
[Attachments: 8wdbme.png]

[2024-07-09 16:27] donna🤯: [replying to Brit: ""]
incredible meme

[2024-07-09 16:40] diversenok: [replying to x86matthew: "is it going to be `ret == 0`, `ret != 0`, `ret == ..."]
And also there is a question of whether `S_FALSE` is a success or not

[2024-07-09 16:40] diversenok: At least otherwise HRESULTs try to organize this mess

[2024-07-09 17:17] th3: [replying to x86matthew: "yeah the win32 error indicator roulette is fun"]
sometimes i think i dont know basic boolean logic

[2024-07-10 20:07] Terry: Is there any easy way to get the real return address when you have a bp on a function where the caller uses some return address spoofing? It's very obvious its using some bad spoof calling because the entire call stack after the most recent caller is a bunch of garbage. I think they are "encrypting" the real return address somewhere on the stack. I could probably BP and find where and how to decrypt. Just wondering if there was some better way

one interesting thing is the immediate return address is a ud2 within a valid module. not sure if this helps anyone narrow down the specific code thats being used

[2024-07-10 20:17] irql: lol

[2024-07-10 20:18] irql: sounds like they point the ret to an instruction which will throw (inside a legit module), and then catch the exception

[2024-07-10 20:18] irql: then restore the real ret address

[2024-07-10 20:18] irql: finding the real ret would require you to reverse whatever is doing the spoof call / maybe their exception handler

[2024-07-10 20:20] Terry: [replying to irql: "sounds like they point the ret to an instruction w..."]
Thats what i thought at first, but the ud2 instruction never gets executed

[2024-07-10 20:20] Brit: [replying to irql: "sounds like they point the ret to an instruction w..."]
who have I seen do this before :^)

[2024-07-10 20:22] irql: lmfao

[2024-07-10 20:22] irql: [replying to Terry: "Thats what i thought at first, but the ud2 instruc..."]
ehhh?

[2024-07-10 20:26] Terry: [replying to irql: "ehhh?"]
unless my hardware breakpoint isnt being hit somehow?

[2024-07-10 20:26] irql: probably that ^^

[2024-07-10 20:26] Terry: i shall place an mtrr and update

[2024-07-10 20:26] irql: whatever you're reversing has an exception handler

[2024-07-10 20:27] irql: or I guess idk

[2024-07-10 20:27] irql: maybe they set a bp elsewhere and catch the return, although sounds unlikely

[2024-07-10 20:38] Terry: okay the mtrr caught it, that makes much more sense

[2024-07-10 20:38] Terry: i trusted the hardware breakpoint too much

[2024-07-10 22:04] irql: ^^

[2024-07-10 22:04] irql: figured so

[2024-07-10 22:22] Terry: [replying to irql: "figured so"]
whats interesting is the dumped module doesnt have any functionality to change rsp (at least at one call i was able to find an xref for), and the exception handler they register from that module doesnt catch the ud2 (it catches some other exceptions). i guess some kernel shenanigans are going on. it's *all* calls that change rsp. even the ones that call other functions within the module

[2024-07-10 22:22] Terry: thanks for u help tho, gonna have fun with this one lol

[2024-07-10 23:05] irql: all good, yea

[2024-07-11 08:39] abu: [replying to Brit: "who have I seen do this before :^)"]
please enlighten us brother

[2024-07-12 11:40] Deleted User: Hey, I found an interesting file in my extensions in VSCODE of course I’m not as smart as most devs in this com to say the least. Not to bash myself but I have no clue really if this is a false flag or not. But I put this file “pet.exe” into a dynamic checker on https://hybrid-analysis.com and it said that the file was suspicious and that it had Anti-VM capabilities the file is located in: C:\Users\user\.vscode\extensions\ms-python.python-2024.10.0-win32-x64\python-env-tools\bin if someone could check and get back to me on what exactly this is, I would appreciate it
[Embed: Free Automated Malware Analysis Servic...]
Submit malware for free analysis with Falcon Sandbox and Hybrid Analysis technology. Hybrid Analysis develops and licenses analysis tools to fight malware.

[2024-07-12 11:53] Brit: [replying to Deleted User: "Hey, I found an interesting file in my extensions ..."]
it's just the python environement finder

[2024-07-12 15:32] projizdivlak: why tf x32dbg doesn't bp on thread when in it's own dll loader?

[2024-07-12 15:35] projizdivlak: downloaded latest version, now it works

[2024-07-12 15:37] mrexodia: [replying to projizdivlak: "why tf x32dbg doesn't bp on thread when in it's ow..."]
might have been the 'Disable ASLR' option?

[2024-07-12 15:37] mrexodia: I didn't investigate yet, but that messes with the initialization

[2024-07-12 15:38] projizdivlak: [replying to mrexodia: "might have been the 'Disable ASLR' option?"]
didn't have it enabled

[2024-07-12 15:39] mrexodia: well

[2024-07-12 15:39] mrexodia: lmk if it happens again, cuz should work

[2024-07-12 15:41] projizdivlak: oh shit i forgot ur the dev lmao

[2024-07-12 15:41] projizdivlak: i think i just had an very old version

[2024-07-12 15:41] projizdivlak: unpatched bug or smth

[2024-07-12 15:49] 5pider: [replying to projizdivlak: "oh shit i forgot ur the dev lmao"]
no he isnt. its just someone impersonating him

[2024-07-12 15:49] 5pider: i fell for it as well 😦 😔

[2024-07-12 16:10] daax: [replying to projizdivlak: "oh shit i forgot ur the dev lmao"]
https://www.youtube.com/watch?v=AhqkzSSOhVQ
[Embed: x64dbg]
I was bored so here is a x64dbg edit.

https://x64dbg.com/
https://github.com/mrexodia

🎥 Learn to Edit videos like me: https://www.patreon.com/Dulge

⚡️Learn to hack games: https://guidedhacking.com/

[2024-07-12 16:51] sariaki: it unironically goes harder every time i play it

[2024-07-12 17:07] projizdivlak: hahahah

[2024-07-12 17:54] szczcur: [replying to sariaki: "it unironically goes harder every time i play it"]
this is how all devs should debut their tools

[2024-07-12 17:56] Timmy: <:OMEGALUL:662670462215782440>

[2024-07-12 17:56] Timmy: noted

[2024-07-12 18:05] unknowntrojan: [replying to szczcur: "this is how all devs should debut their tools"]
just send the edit of your favorite open source project as your CV

[2024-07-12 18:05] unknowntrojan: HR is gonna have a stroke