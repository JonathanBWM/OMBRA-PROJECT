# March 2025 - Week 4
# Channel: #reverse-engineering
# Messages: 101

[2025-03-17 08:03] show1059: Anyone interested in being hired to bypass a hash check

[2025-03-17 10:58] Ela: [replying to 𝝅ₙ(Sⁿ) ≅ (ℤ, +): "okay, try to find that."]
I cant find it 😦

[2025-03-17 10:58] Ela: I found the sdk for the device though

[2025-03-17 10:58] Ela: will check if there is a function to decrypt the .rom file

[2025-03-17 16:06] Horsie: Can someone share some memes that break hexrays and/or crash ida?

[2025-03-17 16:06] Horsie: I know there were are least 2 blogs that posted about it in the past. Google (and others) seem to be getting worse every day

[2025-03-17 16:38] Deleted User: [replying to Horsie: "Can someone share some memes that break hexrays an..."]
https://github.com/android1337/brkida
[Embed: GitHub - android1337/brkida: C++ macro for x64 programs that breaks...]
C++ macro for x64 programs that breaks ida hex-rays decompiler tool. - android1337/brkida

[2025-03-17 20:09] juan diego: [replying to Horsie: "Can someone share some memes that break hexrays an..."]
overlapping instructions

[2025-03-17 20:09] juan diego: jumping over garbage code

[2025-03-17 20:09] juan diego: similar to that ^

[2025-03-17 20:16] juan diego: i think theres a sequence like eb ff 13 for example

[2025-03-17 20:16] juan diego: where the ff is -1 for jmp and then encodes the beginning of a call instruction

[2025-03-17 20:55] szczcur: [replying to juan diego: "jumping over garbage code"]
these don't really break / crash ida. they just frustrate pseudocode and desync some parts of the disasm stream. the fix takes a few seconds of your time and can be automated via idapy.

[2025-03-17 21:36] pinefin: [replying to szczcur: "these don't really break / crash ida. they just fr..."]
is there a script that can do this? ive been manually adjusting stack frames.............

[2025-03-17 22:34] bishop: [replying to Horsie: "Can someone share some memes that break hexrays an..."]
There is a cve

[2025-03-17 22:34] bishop: https://github.com/Azvanzed/CVE-2024-44083
[Embed: GitHub - Azvanzed/CVE-2024-44083: Makes IDA (most versions) to cras...]
Makes IDA (most versions) to crash upon opening it.  - GitHub - Azvanzed/CVE-2024-44083: Makes IDA (most versions) to crash upon opening it.

[2025-03-17 23:43] szczcur: [replying to bishop: "https://github.com/Azvanzed/CVE-2024-44083"]
https://discord.com/channels/835610998102425650/835667591393574919/1277317490753671168

[2025-03-17 23:43] szczcur: bit easy to mitigate tragically

[2025-03-17 23:57] juan diego: every sequence of a few instructions is easy to mitigate 😂

[2025-03-18 00:57] szczcur: [replying to juan diego: "every sequence of a few instructions is easy to mi..."]
yeah, agreed, recommend something better

[2025-03-18 07:52] Rithvariel: I think quarkslab had a talk or blog about ways to break the most popular decompilers.

[2025-03-18 07:53] Rithvariel: Otherwise you can probably implement something in a compiler.

[2025-03-18 09:30] bishop: [replying to szczcur: "bit easy to mitigate tragically"]
ofc it serves as a poc to demonstrate how to crash it

[2025-03-18 09:33] bishop: but 95% of peoples will not know how to detect and mitigate it

[2025-03-18 10:15] bishop: [replying to szczcur: "https://discord.com/channels/835610998102425650/83..."]
nice trick

[2025-03-18 14:06] Horsie: thanks for all of the answers!

[2025-03-18 14:06] Horsie: really cool stuff

[2025-03-18 14:07] Horsie: https://connorjaydunn.github.io/blog/posts/binaryshield-a-bin2bin-x86-64-code-virtualizer/
[Embed: BinaryShield: a bin2bin x86-64 code virtualizer | Connor-Jay's Blog]

[2025-03-18 14:07] Horsie: This blog also has a lot of cool approaches listed towards the end.

[2025-03-18 14:07] pinefin: ilfak who?

[2025-03-18 14:08] daax: [replying to pinefin: "ilfak who?"]
ilfaks dog

[2025-03-18 14:08] Horsie: [replying to Horsie: "https://connorjaydunn.github.io/blog/posts/binarys..."]
oops wrong link.

[2025-03-18 14:08] Horsie: https://blog.es3n1n.eu/posts/obfuscator-pt-1/
[Embed: Obfuscating native code for fun: Part 1 - Introduction]
In this series of posts, I will try to cover all the common knowledge you would need to create your PE bin2bin obfuscator. This year I saw a rise of interest in the topics of software obfuscation and 

[2025-03-18 14:11] daax: [replying to Horsie: "oops wrong link."]
you try that bin yet?

[2025-03-18 14:11] emma: They snapshot fuzzed IDA a while ago

[2025-03-18 14:11] emma: https://doar-e.github.io/blog/2021/07/15/building-a-new-snapshot-fuzzer-fuzzing-ida/
[Embed: Building a new snapshot fuzzer & fuzzing IDA]

[2025-03-18 14:11] Horsie: just reached home from work. about to open it up 😋

[2025-03-18 14:11] Horsie: <@609487237331288074>

[2025-03-18 14:32] 0xatul: It's 8 already damn

[2025-03-18 23:25] pinefin: the amount of times i forget to ctrl+s on ida because they changed the keybind from ctrl+w

[2025-03-18 23:46] Icky Dicky: [replying to Ela: "http://helpen.dvr163.com/index.php/Mainpage"]
from what I can find it seems like a hikvision re-skin

[2025-03-18 23:46] Icky Dicky: [replying to Ela: ""]
but this doesn't look like hikvision

[2025-03-18 23:49] Icky Dicky: It uses EseeCloud. From past experience most DVR/NVR devices normally have vulnerabilities on the port which interacts with ActiveX or Mobile

[2025-03-18 23:49] Icky Dicky: Check the ports open

[2025-03-18 23:50] Icky Dicky: https://github.com/pgross41/hassio-addons/tree/4c5f7f1afdf4c02481469bf1e81c664f517c8614/dvr163
[Embed: hassio-addons/dvr163 at 4c5f7f1afdf4c02481469bf1e81c664f517c8614 · ...]
Home Assistant Add-on for interacting with Eseenet/dvr163 NVR - pgross41/hassio-addons

[2025-03-18 23:56] Icky Dicky: oh it is a hikvision re-skin

[2025-03-18 23:59] Icky Dicky: https://gist.github.com/aSmig/e50058a54ab85428915521f233ffa3d0
https://gist.github.com/maxious/c8915a436b532ab09e61bf937295a5d2
[Embed: How to get root on your K9608-2W 8-channel Network Video Recorder]
How to get root on your K9608-2W 8-channel Network Video Recorder - K9608-2W.md
[Embed: Esee/Anran 960P 180° Wireless Fisheye Panoramic CCTV Smart Camera H...]
Esee/Anran 960P 180° Wireless Fisheye Panoramic CCTV Smart Camera HD WIFI Webcam IP - README.md

[2025-03-19 00:00] Icky Dicky: [replying to Icky Dicky: "https://gist.github.com/aSmig/e50058a54ab854289155..."]
I cannot test it but that seems like your best shot tbh, tell me how it goes i'm intrigued

[2025-03-19 00:00] Icky Dicky: An do you have the device?

[2025-03-19 19:35] NSA, my beloved<3: Hello. In Windows, does anyone have experience with thread message queues, callbacks, GetMessage() and the way it works under the hood? All in all, window stuff. I would highly appreciate some nice resources explaining what these concepts are, since whenever I try to debug GetMessage(), there is almost instantly a syscall to NtUserGetMessage and this syscall points the thread to some location from where functions are called such as KiUserCallForwarder. At some point, for some reason the debugger loses control over the thread? Since after the call to GetMessage() no breakpoints are hit, thus I can not debug further stuff. I'd assume this is somewhat related to these 'callbacks'? As a result, I can not find how the MSG buffer struct is populated to see what and where this 'calling thread's message queue' is stored. Is this data that resides in the kernel and I can ask the kernel through a syscall to copy it into my local buffer (MSG struct) that I allocted inside the process? Or is this located in the TIB? I am lost.

[2025-03-19 20:31] Pepsi: [replying to NSA, my beloved<3: "Hello. In Windows, does anyone have experience wit..."]
under the hood the kernel stack base is shifted and a transition back to usermode to a callback handler is performed

[2025-03-19 20:34] Pepsi: you might want to place a breakpoint in your msgproc and observe what happens after it returns

[2025-03-20 09:54] unknown: hi all, im trying to hook KiUserExceptionDispatcher from ntdll and running into some issues,

heres how KiUserExceptionDispatcher looks like
```x86asm
7FFDF43D13A0 - FC                    - cld 
7FFDF43D13A1 - 48 8B 05 88FE0D00     - mov rax,[7FFDF44B1230]
7FFDF43D13A8 - 48 85 C0              - test rax,rax
7FFDF43D13AB - 74 0F                 - je 7FFDF43D13BC
7FFDF43D13AD - 48 8B CC              - mov rcx,rsp
7FFDF43D13B0 - 48 81 C1 F0040000     - add rcx,000004F0
7FFDF43D13B7 - 48 8B D4              - mov rdx,rsp
7FFDF43D13BA - FF D0                 - call rax
7FFDF43D13BC - 48 8B CC              - mov rcx,rsp
7FFDF43D13BF - 48 81 C1 F0040000     - add rcx,000004F0
7FFDF43D13C6 - 48 8B D4              - mov rdx,rsp
7FFDF43D13C9 - E8 420FFBFF           - call 7FFDF4382310
```
Heres how it looks like when i patch it

```x86asm
7FFDF43D13A0 - FC                    - cld 
7FFDF43D13A1 - 48 B8 80D61449F67F0000 - mov rax,00007FF64914D680
7FFDF43D13AB - 90                    - nop 
7FFDF43D13AC - 90                    - nop 
7FFDF43D13AD - 48 8B CC              - mov rcx,rsp
7FFDF43D13B0 - 48 81 C1 F0040000     - add rcx,000004F0
7FFDF43D13B7 - 48 8B D4              - mov rdx,rsp
7FFDF43D13BA - FF D0                 - call rax
7FFDF43D13BC - 48 8B CC              - mov rcx,rsp
7FFDF43D13BF - 48 81 C1 F0040000     - add rcx,000004F0
7FFDF43D13C6 - 48 8B D4              - mov rdx,rsp
7FFDF43D13C9 - E8 420FFBFF           - call 7FFDF4382310
7FFDF43D13CE - 84 C0                 - test al,al
```
i move in rax the address of my function and then change the je to two nops, after patching it everything works fine, but thats only the case when my exe is compiled with clang, 

when i build it with gcc, there are a hundred access violations that my own function captures and with msvc it straight up crashes out.

does anyone know what could be happening here?

[2025-03-20 10:15] Abra-Cadabra: Guys , can anyone suggest a best laptop config for reverse engineering, hypervisor fuzzing and fuzzing?
I am planning to buy i9 14900 hx processor based laptops

[2025-03-20 15:19] Matti: [replying to Abra-Cadabra: "Guys , can anyone suggest a best laptop config for..."]
elaborate on what you mean by 'best'

[2025-03-20 15:19] Matti: in general this term excludes laptops full stop

[2025-03-20 15:19] Matti: and if you include performance in this metric, it also excludes intel CPUs

[2025-03-20 15:22] Abra-Cadabra: OMG, my bad  I want to know a good configuration which can do fuzzing and hypervisor fuzzing easily without any performance issues, also for reverse engineering tools installation

[2025-03-20 15:23] Matti: but it must be a laptop? and/or an intel?

[2025-03-20 15:23] Matti: there are reasons for wanting these things

[2025-03-20 15:23] Matti: I'm just wondering if in your case they are good reasons

[2025-03-20 15:23] Matti: for example, debugging VT-x code is going to be difficult on an AMD CPU

[2025-03-20 15:28] Abra-Cadabra: Yes, i am not aware of this kind of stuff, instead of crying after buying

i thought to take input from experts before buying..

If AMD debugging is tough, despite intel performance lag, 

still buying intel for my requirements is good choice?

[2025-03-20 15:29] Abra-Cadabra: Because hx variant allows overclocking

[2025-03-20 15:29] Matti: well let's start with one of your requirements

[2025-03-20 15:29] Matti: hypervisor fuzzing

[2025-03-20 15:29] Matti: what kind of hypervisor would this be

[2025-03-20 15:30] Matti: in general your choice of CPU is gonna depend on this

[2025-03-20 15:32] Abra-Cadabra: hosted hypervisor

[2025-03-20 15:32] Abra-Cadabra: I am a beginner willing to create my own hypervisor and fuzz on it

[2025-03-20 15:32] Glatcher: [replying to Matti: "and if you include performance in this metric, it ..."]
Cheat Engine's Ultimap: 🥲

[2025-03-20 15:34] Matti: [replying to Abra-Cadabra: "I am a beginner willing to create my own hyperviso..."]
no offense but I think you should read up on hypervisors some more before deciding to spend money on a machine for "fuzzing" it

[2025-03-20 15:34] Matti: in general for x86, AMD and intel CPUs use entirely different ISA extensions for running VMMs and guests in them

[2025-03-20 15:46] pinefin: quantum x86 emulation fuzzing

[2025-03-20 15:47] pinefin: but yeah

[2025-03-20 15:47] pinefin: listen to matti, learn how to actually develop a hypervisor and the ins and outs

[2025-03-20 15:48] pinefin: before actually 'fuzzing' with one

[2025-03-20 17:32] NSA, my beloved<3: [replying to Pepsi: "under the hood the kernel stack base is shifted an..."]
Okay so if I understand this right, callbacks are user-mode functions defined within the process memory. These are ran in cases where a system call is executed, and the kernel not only modifies the stack, the general purpose registers but modifies the IP as well where it will point to one of these callback functions, correct?

[2025-03-20 17:33] NSA, my beloved<3: [replying to Pepsi: "you might want to place a breakpoint in your msgpr..."]
What do you mean by "msgproc"?

[2025-03-20 17:34] NSA, my beloved<3: The MSG strucutre?

[2025-03-20 17:39] NSA, my beloved<3: Placing a write breakpoint there is never hit. Not memory nor hardware.

[2025-03-20 22:59] Pepsi: [replying to NSA, my beloved<3: "What do you mean by "msgproc"?"]
sry wrong wording, i meant the WindowProc, the function that handles the messages, sorry for the confusion

[2025-03-20 23:01] Pepsi: when you are dealing with win32k stuff, you need to understand the concept of usermode callbacks

[2025-03-20 23:02] Pepsi: maybe have a look at `KeUserModeCallback` & `KiCallUserMode` inside `ntoskrnl.exe`

[2025-03-20 23:07] Pepsi: basically when you are doing a win32k syscall, the thread might execute functions in usermode, 
before you see a sysret and the thread continues execution:

ntdll!NtUserSomething -> Syscall -> nt!KiSystemCall64 -> win32kfull!NtUserSomething -> [...] ->  nt!KeUserModeCallback -> KiCallUserMode ->  *Usermode Callback*

after the usermode callback is done, NtCallbackReturn will resume execution in the kernel:

*Usermode Callback* -> nt!NtCallbackReturn -> [...]

[2025-03-21 02:14] Ela: [replying to Icky Dicky: "https://gist.github.com/aSmig/e50058a54ab854289155..."]
thanks for sharing, I just managed to decrypt the file it was just XOR obfuscated

[2025-03-21 02:17] Ela: xortool-xor -f firmware.rom -h 509091519353529296565797559594549c5c5d9d5f9f9e5e5a9a9b5b99595898884849894b8b8a4a4e8e8f4f8d4d4c8c44848545874746868242438341818040 > decode.rom

[2025-03-21 02:17] Ela: [replying to Ela: "xortool-xor -f firmware.rom -h 5090915193535292965..."]
This command did the job

[2025-03-21 02:17] Ela: after decoding, running binwalk should be enough to extract all the file directory

[2025-03-21 02:34] UJ: [replying to Abra-Cadabra: "Guys , can anyone suggest a best laptop config for..."]
you need a desktop for real stuff. just running a couple ida's, vm or 2 and visual studio and chrome with a session of tabs open uses like 100gb of ram for me.

[2025-03-21 02:34] UJ: also hyperdbg is intel only afaik (and like most of the hypervisor tutorials/guides that beginners use to create a "hello world" hypervisor)

[2025-03-21 02:38] Abra-Cadabra: after buying capable hardware gear, if i do micro architecture attack on it, will it become brick? or i can do it on my own hypervisor

[2025-03-21 02:39] UJ: putting the cart before the horse here tbh

[2025-03-21 02:40] Abra-Cadabra: [replying to UJ: "putting the cart before the horse here tbh"]
sorry I couldn't get you

[2025-03-21 02:43] Abra-Cadabra: [replying to UJ: "putting the cart before the horse here tbh"]
I am willing to dive on micro arch attacks, so getting started on it , so thinking of good gear for it

[2025-03-21 05:41] Brit: <:LARP:1325956664314302496>

[2025-03-21 12:16] ♡Luna♡: [replying to Abra-Cadabra: "Guys , can anyone suggest a best laptop config for..."]
You can RE on a literal potato with copper wires

[2025-03-21 12:25] Abra-Cadabra: [replying to ♡Luna♡: "You can RE on a literal potato with copper wires"]
sure

[2025-03-21 13:13] idkhidden: yall have gpus ?
[Attachments: image.png]

[2025-03-21 13:30] NSA, my beloved<3: [replying to Pepsi: "when you are dealing with win32k stuff, you need t..."]
Thank you. I did some more digging, and I concluded that callbacks can be functions defined inside system libraries, that are executed after certain system calls, rather than the kernel returning to the system call stub. My own executable can also contain callback functions, whose address then has to be passed around between the system call and internal library functions as a paramter. However in the case of a user defined callback function, like WndProc, it is not called directly after a certain system call, rather there is a system call from where execution continues at a system callback function (which is the callback handler inside user32.dll?!), and eventually my callback function is called by user-mode code.

[2025-03-22 00:03] abowarda_: Hello any one can help me to make reverse engineering for bot website?