# September 2024 - Week 5
# Channel: #reverse-engineering
# Messages: 140

[2024-09-23 00:07] nobody: [replying to MonTy: "most deobfuscation products are focused on use in ..."]
you can try https://github.com/waryas/KACE works quite decently for the common stuff
[Embed: GitHub - waryas/KACE: Emulate Drivers in RING3 with self context ma...]
Emulate Drivers in RING3 with self context mapping or unicorn - waryas/KACE

[2024-09-23 00:08] nobody: code base is a bit schitzo tho

[2024-09-23 04:40] Ignotus: cant u just run the driver and dump it from memory 🤷‍♂️

[2024-09-23 04:50] MonTy: I got the driver trace. How can I clean it? I was advised to skip its smt solver and then raise it to vtil.

[2024-09-23 04:51] MonTy: may be triton?

[2024-09-23 05:29] twopic: [replying to Azrael: "You need to launch your CUDA kernels into space."]
Cuda hater <a:DVaGiggle:777346505890594857>

[2024-09-25 10:48] Deleted User: does anyone know any information about ``CheckRemoteDebuggerPresent`` and how it works / what it checks?

[2024-09-25 10:50] sudhackar: [replying to Deleted User: "does anyone know any information about ``CheckRemo..."]
https://unprotect.it/technique/checkremotedebuggerpresent/

[2024-09-25 10:51] Deleted User: [replying to sudhackar: "https://unprotect.it/technique/checkremotedebugger..."]
ive already checked this, it doesnt have much information

[2024-09-25 10:51] sudhackar: assume https://doxygen.reactos.org/d5/d42/dll_2win32_2kernel32_2client_2debugger_8c_source.html#l00376

[2024-09-25 10:51] Deleted User: im trying to manipulate my proceses information to make ``CheckRemoteDebuggerPresent`` return true

[2024-09-25 10:51] sudhackar: disass the function in the dll mentioned

[2024-09-25 10:52] sudhackar: [replying to Deleted User: "ive already checked this, it doesnt have much info..."]
>  Internally, it also uses NtQueryInformationProcess with ProcessDebugPort as a ProcessInformationClass parameter.

[2024-09-25 10:54] brymko: [replying to Deleted User: "im trying to manipulate my proceses information to..."]
call debug attach to your own process

[2024-09-25 10:59] Deleted User: [replying to brymko: "call debug attach to your own process"]
what function is that?

[2024-09-25 11:05] brymko: https://learn.microsoft.com/en-us/windows/win32/api/debugapi/nf-debugapi-debugactiveprocess
[Embed: DebugActiveProcess function (debugapi.h) - Win32 apps]
Enables a debugger to attach to an active process and debug it.

[2024-09-25 11:08] Brit: matthew wrote about the trick you're trying to do

[2024-09-25 11:08] Brit: https://www.x86matthew.com/view_post?id=selfdebug

[2024-09-25 11:09] Brit: "*pbDebuggerPresent = DebugPort != NULL;"

[2024-09-25 11:09] Brit: so you have to either figure out a way to make ntqip return something for the debug port of your process

[2024-09-25 11:09] Brit: or attach to yourself

[2024-09-25 11:09] Brit: either way, it's not gonna stop anyone serious from debugging you

[2024-09-25 11:14] Rairii: on the subject of debugging, i've debugged all the way down to usermode from vmware/qemu gdb server. it's very annoying to do, but it is possible.

[2024-09-25 11:18] brymko: see Gog Agog Grand Nabob addition

[2024-09-25 11:31] brymko: are you ?

[2024-09-25 11:31] brymko: 
[Attachments: image.png]

[2024-09-25 15:48] x86matthew: it's just a silly PoC though, i definitely wouldn't use this for anything serious

[2024-09-25 15:49] x86matthew: it's also possible to improve further, as of windows 10 you can implement the debug handler loop within your own process

[2024-09-25 15:52] diversenok: And you can also make it a library function that doesn't require restarting the process with different command line parameters by cloning the current process and attaching from the clone

[2024-09-25 15:53] luci4: RtlCloneProcess is so weird

[2024-09-25 15:53] luci4: (or whatever it's called)

[2024-09-25 15:53] diversenok: RtlCloneUserProcess, yeah

[2024-09-25 15:53] luci4: [replying to diversenok: "RtlCloneUserProcess, yeah"]
I know of it from an article you wrote, lol

[2024-09-25 15:54] diversenok: Hehe 😄

[2024-09-25 15:56] luci4: [replying to diversenok: "Hehe 😄"]
Cool discovery tho! <:wesmart:586238041929023534>

[2024-09-26 20:55] f0rk: any chance any of yall have access to a megarac sp-x firmware blob

[2024-09-27 09:42] varaa: Has anyone here looked at pyarmor reverse / decompiling (decryption) ?

[2024-09-27 09:43] varaa: the decryption happens in mem at runtime via pytransform.dll or .pyd (python-driver)

[2024-09-28 04:55] MonTy: guys, help me assemble novmp

[2024-09-28 04:55] MonTy: 
[Attachments: 2024-09-28_075454.png]

[2024-09-28 06:28] estrellas: look at the error message

[2024-09-28 06:51] MonTy: [replying to estrellas: "look at the error message"]
and ... ?

[2024-09-28 07:31] MonTy: please help me, I'm collecting NoVmp for the first time

[2024-09-28 08:22] Matti: [replying to MonTy: "and ... ?"]
the next step is usually reading the message too
I assume you did that
so then the problem is simply that the compiler sees a symbol it doesn't know and doesn't know what to do with

[2024-09-28 08:23] Matti: why is this happening? presumably because you did not invoke cmake correctly in order to get it to install the capstone headers that include these symbols

[2024-09-28 08:25] Matti: I just cloned the repo, did `mkdir build && cd build && cmake-gui ..` and clicked tthrough the configuration steps, then generate

[2024-09-28 08:25] Matti: as much as I hate cmake, this just kind of worked

[2024-09-28 08:25] Matti: and got me the symbols you apparently are missing

[2024-09-28 08:26] Matti: because it downloads capstone for you

[2024-09-28 10:17] Matti: [replying to MonTy: "please help me, I'm collecting NoVmp for the first..."]
hey just to clear this up because people make this mistake sometimes
just because I answered a question of yours here (a pretty minimal effort one at that) - does not mean you are now granted permission to DM me to follow up with more questions... except in private, because that's  somehow better?

[2024-09-28 10:20] Matti: the reasonn we prefer people to post their questions **here** (in public, in the relevant channel, and preferably listing what you tried but failed)....
...is because other people in the server can then also learn from the answer if it turns out someone knew what the problem was

[2024-09-28 10:20] Matti: even including potential future members who use the search function!

[2024-09-28 10:20] Matti: if you DM me with garbage questions, I tend to just ignore you until you go away

[2024-09-28 10:22] MonTy: [replying to Matti: "hey just to clear this up because people make this..."]
ok, sorry

[2024-09-28 10:23] MonTy: and so I used cmake and got the following

[2024-09-28 10:23] MonTy: 
[Attachments: image.png]

[2024-09-28 10:24] Matti: 1. Open the .sln file and build it

[2024-09-28 10:25] MonTy: 
[Attachments: image.png]

[2024-09-28 10:27] Matti: well that looks pretty mega fucked

[2024-09-28 10:28] Matti: are you sure you followed the steps the same way I did? and I mean no more and no less

[2024-09-28 10:28] Matti: [replying to Matti: "I just cloned the repo, did `mkdir build && cd bui..."]
clone the repoisitory using git, then do this

[2024-09-28 10:29] Matti: one thing I notice is you seem to be on VS2019, not 2022 - it's doubtful that that's the cause but just mentioning it

[2024-09-28 10:30] Matti: if it's not "oops maybe I didn't *quite* do the same thing", or the VS version difference, then I don't know

[2024-09-28 10:32] MonTy: I cloned the repository using GitHub desktop and i using visual studio 2019

[2024-09-28 10:44] Matti: not sure what to tell you other than that it just works for me, which is honestly rare for any cmake project I clone
[Attachments: 2024-09-28_12-34-14.mp4]

[2024-09-28 10:45] Matti: so, try VS2022 I guess

[2024-09-28 10:45] Matti: although I still have no idea as to why that would matter

[2024-09-28 10:46] Matti: your ss above is just standard undecipherable cmake shit

[2024-09-28 10:46] Matti: caused by.... cmake

[2024-09-28 10:46] Matti: not MSVC

[2024-09-28 11:12] MonTy: and so I started over, installed cmake and git. Now, when building cmake, there are such errors👇
[Attachments: image.png]

[2024-09-28 11:16] MonTy: may be download keystone and capstone manually?

[2024-09-28 11:31] Matti: you're not using some version of cmake from the 1990s right

[2024-09-28 11:32] Matti: I don't know what the minimum version is to git clone through cmake but it seems to be having issues with that

[2024-09-28 11:33] Matti: for reference the version I used above was 3.30 from git

[2024-09-28 11:57] society: [replying to Matti: "not sure what to tell you other than that it just ..."]
fellow foobar2000 user i kneel

[2024-09-28 12:36] Can: y yall using cmake anyway i literally provide the sln <:Kappa:794707301436358686>

[2024-09-28 12:37] MonTy: My respects Matty
[Attachments: image.png]

[2024-09-28 12:37] MonTy: That's great.

[2024-09-28 12:39] MonTy: if I understand everything correctly, does Novmp emulate vmprotect 3.5 virtual machine and optimize?

[2024-09-28 15:19] mrexodia: [replying to Matti: "not sure what to tell you other than that it just ..."]
I will inform you that I made the CMake here

[2024-09-28 15:19] mrexodia: <:kappa:697728545631371294>

[2024-09-28 20:57] Matti: [replying to Can: "y yall using cmake anyway i literally provide the ..."]
hey man I would've done that if I'd seen it

[2024-09-28 20:57] Matti: I just see
> CMakeLists.txt
and go "ugh" then run the whatever and deal with it

[2024-09-28 20:58] Matti: so as you can see even if you accidentally do it the worst way like I did, it's still pretty fucking easy

[2024-09-28 20:58] Matti: [replying to mrexodia: "I will inform you that I made the CMake here"]
well thank you blessed cmake god

[2024-09-28 20:59] Matti: can you please do this for all cmake repositories on github ever? thanks

[2024-09-28 22:26] mrexodia: [replying to Matti: "can you please do this for all cmake repositories ..."]
one day cmkr will take over 😎

[2024-09-28 23:59] Torph: [replying to Matti: "as much as I hate cmake, this just kind of worked"]
mood

[2024-09-29 03:16] Hehron: anyone in here into cryptography and ciphers?

[2024-09-29 04:02] od8m: [replying to Hehron: "anyone in here into cryptography and ciphers?"]
heck yeah

[2024-09-29 04:02] od8m: <:yumshot:1289734272579665950>

[2024-09-29 04:11] Hehron: [replying to od8m: "heck yeah"]
check dms plz 🫶

[2024-09-29 06:59] varaa: anyone here experienced in reversing / decrypting pyarmor?

[2024-09-29 07:48] MonTy: Guys, does anyone use triton (JohnSaliwan)?

[2024-09-29 07:49] MonTy: What the hell is this?
[Attachments: 371833368-8d39a5ef-bc24-448e-a6b1-084929de4830.png]

[2024-09-29 09:39] sariaki: did you restart the console after installing triton?

[2024-09-29 09:43] MonTy: [replying to sariaki: "did you restart the console after installing trito..."]
of course

[2024-09-29 09:44] MonTy: maybe it has something to do with python 3.12.3 in xubuntu

[2024-09-29 09:47] MonTy: 
[Attachments: image.png]

[2024-09-29 10:27] Brit: I feel like if you get filtered before building vtil / installing triton maybe trying to take on a vmp'd bin is not the next logical step

[2024-09-29 10:29] contificate: tbf this is probs just suffering

[2024-09-29 10:29] contificate: ```
python -m venv env
source env/bin/activate
pip install triton-library
```

[2024-09-29 10:30] contificate: can probs just do this

[2024-09-29 10:30] contificate: https://pypi.org/project/triton-library/
[Embed: triton-library]
Triton is a dynamic binary analysis library

[2024-09-29 10:57] MonTy: [replying to Brit: "I feel like if you get filtered before building vt..."]
I am writing a diploma on deobfuscation based on neural networks. I don't see anything criminal in the fact that I want to figure out how the products are installed

[2024-09-29 10:59] Brit: awesome, coming from a cs background or math one? and if it's the former, this your first time tackling transforms on code?

[2024-09-29 11:01] MonTy: No. But I installed Novmp for the first time

[2024-09-29 11:03] Brit: I'm not taking a dig at you, I'm genuinely curious.

[2024-09-29 11:05] MonTy: [replying to Brit: "I'm not taking a dig at you, I'm genuinely curious..."]
It's strange to mock a person who can't install a program that doesn't have installation instructions.

[2024-09-29 11:07] mrexodia: [replying to contificate: "```
python -m venv env
source env/bin/activate
pip..."]
Did you try this <@1275159492656369664>?

[2024-09-29 11:07] MonTy: [replying to mrexodia: "Did you try this <@1275159492656369664>?"]
I'm trying it now

[2024-09-29 11:22] MonTy: [replying to mrexodia: "Did you try this <@1275159492656369664>?"]

[Attachments: image.png]

[2024-09-29 11:30] dwordxyz: [replying to MonTy: ""]
pip install triton-library

[2024-09-29 11:33] MonTy: [replying to dwordxyz: "pip install triton-library"]
same thing

[2024-09-29 11:33] MonTy: 
[Attachments: image.png]

[2024-09-29 11:35] MonTy: xubuntu - default

[2024-09-29 11:35] MonTy: python 3.12.3

[2024-09-29 11:38] dwordxyz: [replying to MonTy: "same thing"]
pip install --index-url=https://pypi.org/simple triton-library

[2024-09-29 11:47] MonTy: [replying to dwordxyz: "pip install --index-url=https://pypi.org/simple tr..."]

[Attachments: image.png]

[2024-09-29 12:30] 0xboga: Question about hooking data pointers, say I map a driver / or load it properly, doesn’t matter, and hook an indirect call by replacing a global data ptr with my functions address. It then goes through guard_dispatch which I believe is the CFG check? How comes I pass the check?

[2024-09-29 12:32] mrexodia: [replying to MonTy: ""]
It works fine with python 3.10 on ubuntu 22.04 LTS

[2024-09-29 12:38] mrexodia: To compile it locally you can do (no idea if it is designed to work with 3.12):
```
# system deps
sudo apt install python3 python3-dev python3-venv build-essential cmake git libz3-dev

# create and enter venv
python3 -m venv venv
source venv/bin/activate

# capstone
git clone https://github.com/capstone-engine/capstone --branch v5
cmake -B capstone/build -S capstone -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON
cmake --build capstone/build -j
sudo cmake --install capstone/build

# Triton
git clone https://github.com/JonathanSalwan/Triton
cd Triton
pip install .
```

[2024-09-29 12:46] mrexodia: to test you can do:
```
cd src/examples/python/ctf-writeups/defcon-2016-baby-re
python solve.py
```

after a while it will output:

```
[+] Win
[+] Emulation done.
[+] Final solution: bytearray(b'Math is hard!')
```

[2024-09-29 13:55] varaa: anyone here knowledgeable in decrypting pyarmor code?

[2024-09-29 13:55] varaa: please private message me

[2024-09-29 13:55] varaa: or just message here

[2024-09-29 13:56] mrexodia: why would anybody put effort into your problem?

[2024-09-29 13:56] mrexodia: you're not explaining anything you tried, it is not a technical question so there is absolutely nothing to gain for someone answering

[2024-09-29 13:57] mrexodia: you might have better luck asking a _specific_ technical question, which could also be helpful for people in general (eg as a reference later)

[2024-09-29 13:58] varaa: well i’ve dumped the pe, all it remains is compiled pyc and pyd files, but i can’t get past decrypting that. i’ve tried using pycdas and other tools to decrypt but it’s just memory patterns not anything helpful

[2024-09-29 13:58] mrexodia: https://stackoverflow.com/help/how-to-ask here is a guide that could help you formulate a question
[Embed: How do I ask a good question? - Help Center]
Stack Overflow | The World’s Largest Online Community for Developers

[2024-09-29 13:59] mrexodia: [replying to varaa: "well i’ve dumped the pe, all it remains is compile..."]
there is no tool that does what you want, you will need to dive into pyarmor on a technical level

[2024-09-29 13:59] mrexodia: (and it will be hard)

[2024-09-29 14:01] varaa: Yeah i know

[2024-09-29 14:01] varaa: I was thinking of using https://github.com/nesrak1/bonedensity to find where the pytransform function is located in IDA (which is used to decrypt in mem) breakpoint it in x64dbg and then dump it
[Embed: GitHub - nesrak1/bonedensity: for bones that are too dense]
for bones that are too dense. Contribute to nesrak1/bonedensity development by creating an account on GitHub.

[2024-09-29 14:03] mrexodia: You can also search for `pyarmor ctf` and see if there are any good writeups

[2024-09-29 14:08] varaa: [replying to mrexodia: "You can also search for `pyarmor ctf` and see if t..."]
yeah there’s multiple i’ll take a look at it later, underestimated pyarmor tbh

[2024-09-29 16:30] Timmy: [replying to varaa: "anyone here knowledgeable in decrypting pyarmor co..."]
I haven’t done anything for pyarmor but I had written a large tool that helped in reversing nuitka

[2024-09-29 16:33] Timmy: For nuitka atleast, a lot of helpful information was given just from dumping the dictionary entries