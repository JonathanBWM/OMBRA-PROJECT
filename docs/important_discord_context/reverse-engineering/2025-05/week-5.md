# May 2025 - Week 5
# Channel: #reverse-engineering
# Messages: 61

[2025-05-26 08:54] Windows2000Warrior**: [replying to Ђинђић: "I need to know how to extract the best in decoding..."]
i found a good place for decoding , i try it : https://gchq.github.io/CyberChef/#recipe=Magic(3,true,false,'')&input=OTMgNjkgNWYgMDkgYzggMjcgMzYgNDUgZjkgMjcgMDYgNDUgZmIgMjcgMzYgNDUNCmJkIDYyIDY0IDAzIGY4IDI3IDM2IDQ1IGY4IDI3IDM2IDQ1IGY4IDI3IDM2IDQ1DQpkMSAxZiA1ZSBhMiAzZCA0ZCAzMCAwOSA3YSAwZSBjNyAwMiA5YSA4MiBjMyBiYQ&ieol=CRLF&oeol=CRLF
[Embed: CyberChef]
The Cyber Swiss Army Knife - a web app for encryption, encoding, compression and data analysis

[2025-05-26 08:56] Windows2000Warrior**: 
[Attachments: image.png]

[2025-05-26 08:59] Ђинђић: [replying to Windows2000Warrior**: "i found a good place for decoding , i try it : htt..."]
Might give it a try, thanks

[2025-05-26 14:28] luci4: I'm slightly confused by the CSRSS's unhandled exception filter:

So if no kernel debugger is attached, terminate the process, fair enough. Otherwise, it calls `RtlUnhandledExceptionFilter` which raises the exception to the debugger (?). If the debugger couldn't do anything about it raises a hard error which bug-checks?
[Attachments: image.png]

[2025-05-26 14:31] luci4: My guess is that `goto LABEL_8` is there in case `NtRaiseHardError` does not for bug-check, for some reason

[2025-05-26 16:29] Timmy: it might be just ida decompiler not detecting noreturn and thinking it can return. In that case if the codegen is such that the NtRaiseHardError block is before the NtTerminateProcess block it could think this 'loop' exists.

[2025-05-26 16:30] Timmy: but that's just my guess

[2025-05-26 17:36] luci4: [replying to Timmy: "it might be just ida decompiler not detecting nore..."]
This was exactly it.

[2025-05-26 17:36] luci4: The problem was my over-reliance on F5. Thank you!

[2025-05-27 01:02] ruan: how to make IDA show all functions from a given parent function recursively? like seen in the "Graph print page layout" example
current it show just a  few functions, is it possible to it transverse all its children?
[Attachments: ida64_q8i90SqT9y.png]

[2025-05-27 11:58] x86byte: [replying to ruan: "how to make IDA show all functions from a given pa..."]
- Options 
- Options->General
- Options->General->Graph

and up the N of nodes from [Max Number of nodes]
[Attachments: image.png, image.png, image.png, image.png]

[2025-05-27 18:03] ruan: [replying to x86byte: "- Options 
- Options->General
- Options->General->..."]
ty, have you ever experienced an issue on ida pro that it doesnt name the function correctly? then when you press N on it, it updates the function ame

[2025-05-27 18:40] ruan: 
[Attachments: ida64_J8MN0h6UpW.gif]

[2025-05-27 18:52] x86byte: [replying to ruan: "ty, have you ever experienced an issue on ida pro ..."]
ur welcome, No, absolutely i never faced something like that, which version that u have ?

[2025-05-27 18:53] ruan: 9.1

[2025-05-27 18:53] x86byte: cracked?

[2025-05-27 18:53] ruan: no

[2025-05-27 18:53] ruan: 9.1 isnt the latest?

[2025-05-27 18:54] iPower: [replying to ruan: ""]
did you change the base address by chance?

[2025-05-27 18:54] ruan: i didnt

[2025-05-27 18:55] iPower: hm doesnt happen for me

[2025-05-27 18:55] iPower: when rebasing it also updates the default function names automagically

[2025-05-27 18:56] ruan: i think i have rebased it before

[2025-05-27 18:56] ruan: i will try with a new database

[2025-05-27 18:57] iPower: but not sure if your issue is really an issue. you can change the function name to whatever you want so it's not a real issue

[2025-05-27 18:57] ruan: im running a script to dump all functions starting from a "parent" function

[2025-05-27 18:57] ruan: but some functions are reporting wrong name/these address doesnt exist

[2025-05-27 18:58] x86byte: [replying to ruan: ""]
this "N" problm happened with any program u load or just that one?

[2025-05-27 18:59] x86byte: [replying to iPower: "but not sure if your issue is really an issue. you..."]
true

[2025-05-27 21:17] ruan: it was an issue from the rebase, weird

[2025-05-27 22:31] iPower: [replying to ruan: "it was an issue from the rebase, weird"]
yeah figured it out based on your screenshot

[2025-05-27 22:40] x86byte: [replying to ruan: "it was an issue from the rebase, weird"]
xD

[2025-05-28 08:47] ruan: is it possible somehow to update a plugin without having to restart ida?

[2025-05-28 09:58] Timmy: [replying to ruan: "is it possible somehow to update a plugin without ..."]
native or python?

[2025-05-28 09:58] ruan: [replying to Timmy: "native or python?"]
py or c++

[2025-05-28 09:59] Timmy: native I've not gotten to work before

[2025-05-28 09:59] ruan: wdym native?

[2025-05-28 09:59] Timmy: c/c++

[2025-05-28 10:06] Timmy: in python I did <@390180464393977857>

[2025-05-28 10:06] Timmy: hope this helps

[2025-05-28 10:06] Timmy: ```py
# plugin.py

from importlib import reload
import idaapi

import loader

class Plugin(idaapi.plugin_t):
    flags = 0
    comment = "reload example"
    help = ""
    wanted_name = "reload example"
    wanted_hotkey = "Ctrl+Shift+Alt+R"
    version = "0.0.1"

    def init(self):
        loader.load()
        return idaapi.PLUGIN_KEEP

    def run(self, arg):
        loader.unload()
        reload(loader).load()

    def term(self):
        loader.unload()

def PLUGIN_ENTRY():
    return Plugin()


# loader.py file in same directory

from importlib import reload

import test as plugin

def load():
    reload(plugin).load()


def unload():
    plugin.unload()


# test.py file in same directory

import idaapi

import importlib

def run():
    # do thing


def callback():
    # can reload your plugin dependencies here as well
    run()


hotkeys = []

def load():
    hotkeys.append(idaapi.add_hotkey("Ctrl+Shift+A", callback))


def unload():
    for hotkey in hotkeys:
        idaapi.del_hotkey(hotkey)
```

[2025-05-28 10:07] ruan: ty, ill try it

[2025-05-29 16:49] hxm: is there any PORT of KLEE https://github.com/klee/klee to windows or something similar to klee, i tried to port it but too many functions are POSIX dependant.
[Embed: GitHub - klee/klee: KLEE Symbolic Execution Engine]
KLEE Symbolic Execution Engine. Contribute to klee/klee development by creating an account on GitHub.

[2025-05-29 19:50] Pigreco: [replying to hxm: "is there any PORT of KLEE https://github.com/klee/..."]
https://github.com/LLVMParty/klee/commits/cmake-windows/
[Embed: Commits · LLVMParty/klee]
[fork] KLEE Symbolic Execution Engine. Contribute to LLVMParty/klee development by creating an account on GitHub.

[2025-05-29 21:17] hxm: [replying to Pigreco: "https://github.com/LLVMParty/klee/commits/cmake-wi..."]
still not compiling, many fixes are needed

[2025-05-29 21:20] Pigreco: [replying to hxm: "still not compiling, many fixes are needed"]
not many fixes, few fixes. If you use C++ and llvm you should be fine

[2025-05-29 21:52] mrexodia: [replying to hxm: "still not compiling, many fixes are needed"]
pretty sure that klee compiles on Windows without any changes for me

[2025-05-29 21:52] mrexodia: you just won't have standard library

[2025-05-30 23:49] jordan9001: this was a fun little post to write up and it has some techniques that might be useful to beginners: 
https://www.atredis.com/blog/2025/5/19/in-game-ads
[Embed: A Peek into an In-Game Ad Client — Atredis Partners]
A little bit ago I re-installed the racing game Trackmania, and I noticed I got product ads displayed at me in-game alongside the racetrack. Where were those coming from?

[2025-05-30 23:50] the horse: wow, in game ads

[2025-05-31 00:29] daax: [replying to jordan9001: "this was a fun little post to write up and it has ..."]
you should share this in <#1378136917501284443> as well that’s pretty neat

[2025-05-31 03:03] Xits: does anyone know how I would go about sending ipc commands to the main nodejs process of an electron app at runtime? or generally instrumenting natively

[2025-05-31 03:04] Xits: Its all open source but It composes some huge code bases and Im lost as to whether node, chromium, or electron is actually responsible for the process isolation

[2025-05-31 03:32] the horse: iirc electron is the main executable, v8 is the vm?

[2025-05-31 03:32] the horse: you should be able to implement a form of ffi

[2025-05-31 03:33] the horse: electron has some support for pipes, stdio, ...

[2025-05-31 03:34] the horse: one way that 100% works into inputting data to electron from another process is simply creating a process and electron will read stdout into the app, think theres a node package for getting running executables that does exactly this

[2025-05-31 03:35] the horse: electron create process (specify pipe in pi for stdin, stdout) -> native cpp app -> writes to stdout pipe

[2025-05-31 03:35] the horse: theres definitely better ways

[2025-05-31 03:36] the horse: thats the main hack i know around it, there might be packages that add direct support for pipes (if electron does not have it already)

[2025-05-31 06:54] Xits: [replying to the horse: "one way that 100% works into inputting data to ele..."]
which package? My end goal is to be able to call the "native apis" that the main nodejs process implements from C. Which I believe are implemented via ipc between the renderer and main process