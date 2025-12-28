# August 2024 - Week 4
# Channel: #reverse-engineering
# Messages: 56

[2024-08-19 00:16] daax: [replying to Loading: "im still happy that i my first "exploit" if it can..."]
nice dude

[2024-08-19 00:51] Azrael: [replying to Loading: "lol it worked"]
You can also add Sysinternals live as a network location/drive.

[2024-08-19 12:09] Mysterio: Confused how running process explorerer helps the guy with his exploitation when he can't control the args

[2024-08-19 12:10] Mysterio: Is it not the same as being able to run cmd already. I guess make your own exploiting binary and use the network trick?

[2024-08-19 12:11] Brit: are you confused how running an arbitrary binary at admin privs is considered an exploit?

[2024-08-19 12:21] Mysterio: It wasn't arbitrary in that example, but I see you can do any file on a WebDav

[2024-08-20 14:58] xor: [replying to Loading: "but i need to click on start for it to execute"]
Use jar files

[2024-08-20 14:59] xor: Custom registered extension by java, it won't ask for execution first

[2024-08-20 15:00] xor: [replying to Loading: "with smb i didnt had to do that"]
It probably doesn't ask on internal locations

[2024-08-20 15:38] Loading: too lazy to start my own webdav and upload jar file there, i already reported it and they said it will be fixed, that company doesnt even have bug bounty so i think it would be just waste of time

[2024-08-22 12:09] 0x208D9: <@162611465130475520> sorry for the ping but are there any updated ways to develop an x64dbg plugin with python? found this repo : https://github.com/x64dbg/x64dbgpy

but the docs says its for python 2.7 , cant find any python3 supported docs for scripting engine of the debugger ;-;
[Embed: GitHub - x64dbg/x64dbgpy: Automating x64dbg using Python, Snapshots:]
Automating x64dbg using Python, Snapshots:. Contribute to x64dbg/x64dbgpy development by creating an account on GitHub.

[2024-08-22 12:11] 0x208D9: got this thirdparty repo : https://github.com/ElvisBlue/x64dbgpython

but its full of bugs as well which keeps crashing x64dbg while scripting on the fly
[Embed: GitHub - ElvisBlue/x64dbgpython: x64dbg plugin for running python3 ...]
x64dbg plugin for running python3 script. Focus on doing malware analyst and unpacking - ElvisBlue/x64dbgpython

[2024-08-22 12:13] 0x208D9: [replying to 0x208D9: "got this thirdparty repo : https://github.com/Elvi..."]
in the worst case scenario i need to patch this thing and make it working, just asking if theres already a solution

[2024-08-22 12:13] mrexodia: There is no real python support, because there was no interest whatsoever in x64dbgpy

[2024-08-22 12:14] mrexodia: At the time python 2.7 was very popular for reversing tools, but nobody used it so it wasn't developed further

[2024-08-22 12:14] mrexodia: What is your use case exactly?

[2024-08-22 12:15] 0x208D9: [replying to mrexodia: "What is your use case exactly?"]
need to try out a symbolic execution and make a breakpoint while its running at a specific function

[2024-08-22 12:16] 0x208D9: couldnt find any plugin for x64dbg that supports that

[2024-08-22 12:16] mrexodia: So you want to break somewhere, do some symbolic execution (using Triton/miasm?) and then set another breakpoint and resume?

[2024-08-22 12:16] 0x208D9: [replying to mrexodia: "So you want to break somewhere, do some symbolic e..."]
yes pretty much

[2024-08-22 12:17] 0x208D9: its a vm based software so i need to trace where the execution is leading to and put a breakpoint there

[2024-08-22 12:17] 0x208D9: else its difficult to trace

[2024-08-22 12:18] mrexodia: yeah that's going to be tough without some programming work

[2024-08-22 12:19] mrexodia: you could use a breakpoint command to execute your code (C# using dotx64dbg or C/C++ with the regular plugin system) and call out to python there 🤷‍♂️

[2024-08-22 12:19] 0x208D9: [replying to mrexodia: "you could use a breakpoint command to execute your..."]
umm sure but would seem like alot of gluecode

[2024-08-22 12:20] 0x208D9: lets see, ima try that, thanks

[2024-08-22 12:20] mrexodia: System.Diagnostics.Process.Start(python.exe) <:kappa:697728545631371294>

[2024-08-22 12:21] mrexodia: I think the ElvisBlue plugin is also a fine starting point though 🤷‍♂️

[2024-08-22 12:21] 0x208D9: [replying to mrexodia: "I think the ElvisBlue plugin is also a fine starti..."]
yep thinking of patching it for now

[2024-08-22 12:21] 0x208D9: was just checking out if theres already a built in solution

[2024-08-22 12:21] mrexodia: https://github.com/x64dbg/x64dbgpy3/tree/master/src/x64dbgpy
[Embed: x64dbgpy3/src/x64dbgpy at master · x64dbg/x64dbgpy3]
WIP python3 plugin for x64dbg. Contribute to x64dbg/x64dbgpy3 development by creating an account on GitHub.

[2024-08-22 12:21] mrexodia: I also started something a few years back

[2024-08-22 12:22] mrexodia: And there's also this Chinese plugin that does stuff over sockets

[2024-08-22 12:22] mrexodia: https://github.com/lyshark/LyScript
[Embed: GitHub - lyshark/LyScript: x64dbgpy automated testing plugin]
x64dbgpy automated testing plugin. Contribute to lyshark/LyScript development by creating an account on GitHub.

[2024-08-22 12:23] 0x208D9: no source seems fishy : http://lyscript.lyshark.com/
[Embed: LyScript]
An automated testing plugin developed for the x64dbg debugger, used to quickly build Python based test scripts to accelerate the development of exploit programs, assist in vulnerability mining, and an

[2024-08-22 12:23] 0x208D9: [replying to mrexodia: "https://github.com/x64dbg/x64dbgpy3/tree/master/sr..."]
will progress with this

[2024-08-22 12:23] mrexodia: it's also protected with vmprotect, but I analyzed it and it was clean (that version)

[2024-08-22 12:23] 0x208D9: in that case lemme try

[2024-08-22 12:23] mrexodia: but you know, always run stuff in a VM and everything

[2024-08-22 12:24] 0x208D9: yep, will let you know if i encounter any bugs, thanks for the support, appreciate it !

[2024-08-22 12:31] mrexodia: [replying to 0x208D9: "yep, will let you know if i encounter any bugs, th..."]
lmk if you make it work

[2024-08-22 12:31] mrexodia: I'm not against doing a proper python3 plugin, but it needs users who actually use it

[2024-08-22 13:57] Mysterio: [replying to mrexodia: "but you know, always run stuff in a VM and everyth..."]

[Attachments: image0.gif]

[2024-08-22 17:36] Yaki: [replying to Mysterio: ""]
I learned that you always run everything on your main system and use your main network too, it's definitely worth checking out OALabs Patreon for more pro tips, and if you don't believe me just ask this bald guy <@677248504917131298>

[2024-08-22 17:43] Rolo Pudding: Is anyone experienced with Wireshark for USB analysis? I'm trying to find a specific data packet sent to my mouse by the computer/mouse software when I change the DPI settings. I'm having trouble because there are too many packets being sent to my mouse every second. If anyone can help me create an effective filter to find the data I'm looking for, please DM me.

[2024-08-22 17:43] Azrael: [replying to Rolo Pudding: "Is anyone experienced with Wireshark for USB analy..."]
Turn down the polling rate then?

[2024-08-22 17:44] Rolo Pudding: will try

[2024-08-22 17:45] Azrael: Find the packets that describe delta movement.

[2024-08-22 17:45] Azrael: Then exclude those.

[2024-08-22 17:45] Azrael: Easiest to just drag your mouse around.

[2024-08-22 17:46] Azrael: Inspect the packets manually until you find a common ground.

[2024-08-22 17:46] Azrael: Exclude based on that.

[2024-08-22 23:11] 0x208D9: [replying to mrexodia: "lmk if you make it work"]
gave up with it and finally did that part with : https://github.com/Nalen98/AngryGhidra (cuz had less time in hand)

however the https://discord.com/channels/835610998102425650/835635446838067210/1276154295376674837 seems good, i would like to build up on it for a python3 scripting engine inside x64dbg
[Embed: GitHub - Nalen98/AngryGhidra: Use angr in Ghidra]
Use angr in Ghidra. Contribute to Nalen98/AngryGhidra development by creating an account on GitHub.

[2024-08-25 10:20] luci4: Could anyone explain what the author was referring to in that sentence? That's as far as the provided explanation goes
[Attachments: image.png]

[2024-08-25 10:21] luci4: thanks <:give:835809674480582656>

[2024-08-25 10:25] JustMagic: [replying to luci4: "Could anyone explain what the author was referring..."]
process/thread IDs are generated and tracked using the same code/structures as handles. Every process has a handle table to track what handles it owns and the kernel reuses the same handle table mechanism for process and thread id tracking