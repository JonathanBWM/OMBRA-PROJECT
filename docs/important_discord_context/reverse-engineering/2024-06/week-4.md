# June 2024 - Week 4
# Channel: #reverse-engineering
# Messages: 61

[2024-06-19 03:15] Deadman: Quick question. From my understanding, when a Userland application is calling a winapi function, it loads the parameters into rax, rbx, rcx, etc. then proceeds. When the kernel is making a call to the driver, what registers does it use? WinDbg lists Kernel Registers as: cr0, cr2, cr3, cr4, cr8, gdtr, gdtl, idtr, idtl, tr, ldtr, kmxcsr, kdr0, kdr1, kdr2, kdr3, kdr6, kdr7, xcr0

But I dont know the order.

[2024-06-19 05:12] iPower: [replying to Deadman: "Quick question. From my understanding, when a User..."]
cr0, cr2, cr3, cr4 and cr8 are called control registers. they are used for controlling the behavior of a processor (or reporting something, like cr2 for page faults).
dr0, dr1, dr2, dr3, dr6 and dr7 are called debug registers. they are part of the debug capabilities exposed by the architecture.
others like gdtr, idtr are used for segmentation, interrupts and are important parts for protected mode operation.

what I'm trying to say is that the registers you listed aren't actually used for parameter passing because they have a clear purpose. the windows kernel itself follows the x64 ABI. for more information: https://learn.microsoft.com/en-us/cpp/build/x64-calling-convention?view=msvc-170
[Embed: x64 calling convention]
Learn about the details of the default x64 calling convention.

[2024-06-19 05:15] iPower: my suggestion is looking the architecture manuals for AMD or Intel. if you want something more digestible then you can try opensecuritytraining2's x64 courses

[2024-06-19 14:10] Deadman: [replying to iPower: "my suggestion is looking the architecture manuals ..."]
Will chech this out. Thank you

[2024-06-20 14:57] expy: Hello there! 
Has anyone played with Jonas' DTrace `dg.bat` lately? https://x.com/jonasLyk/status/1568450498579111936 Despite symbols seems okay, it complains about `_EX_FAST_REF`
```
C:\temp>"C:\Program Files\dtrace\dtrace" -w -q -yc:\symbols -xstrsize=256 -xdynvarsize=512m -xbufsize=1024m -xswitchrate=100000hz -s  c:\temp\dg.bat
dtrace: failed to compile script c:\temp\dg.bat: line 370: operator . cannot be applied to a forward declaration: no struct nt`_EX_FAST_REF definition is available
```

[2024-06-20 16:04] elias: <@651054861533839370>

[2024-06-20 16:05] asz: load ntoskernel in wiindbg- if it work there also dtrace

[2024-06-20 16:06] asz: the error say it cant find the pdb

[2024-06-20 16:54] expy: thanks! there is a hardcoded path for symbols in .bat itself, fixed that and it's running now.
However it doesn't seem to respond to any parameters I give to it: `dg.bat 1234` or `dg.bat cmd.exe` -- logs everything for every process. Is that okay?

[2024-06-20 17:24] expy: UPD: patched `trigger_conditions` to filter syscalls only for process needed

[2024-06-20 19:29] asz: you in vm?

[2024-06-21 01:15] expy: Nope, was testing it on a host. Nice job btw, that dtrace language is creepy, how did you manage to write that monstrous script? 🙂

[2024-06-21 01:37] asz: im just weird and ocd

[2024-06-21 15:23] 0xc3.ard: I’m reversing an app but I have to patch few bytes everytime the app crashes. I’m having hard time figuring out how I could patch those bytes because on every startup the instructions load on different address. How should I approach this problem?

[2024-06-21 15:26] jvoisin: [replying to 0xc3.ard: "I’m reversing an app but I have to patch few bytes..."]
Disable aslr?

[2024-06-21 15:35] Brit: [replying to 0xc3.ard: "I’m reversing an app but I have to patch few bytes..."]
you know the bytes you wanna patch? how about scanning for them?

[2024-06-21 15:41] 0xc3.ard: [replying to jvoisin: "Disable aslr?"]
I just tried, it's better now but they are still not always in the same place for e.g. it was on: 00007FFA1515B3CC and on another try it was on: 00007FFA1515AFCC

[2024-06-21 15:42] 0xc3.ard: it's a .NET app which I never really tried reversing before

[2024-06-21 15:44] estrellas: maybe you should use RVAs instead of hardcoded VAs

[2024-06-21 15:51] 0xc3.ard: RVAs are wrong too I believe, and I want to patch them permanently anyway. I don't even know where do the instructions come from

[2024-06-21 16:47] dullard: JIT ?

[2024-06-21 17:17] 0xc3.ard: I'm kinda loooost. It loads the sections that contains the instructions I have to patch from somewhere? but is it possible to patch them somehow when they're loaded again it's my new bytes?

[2024-06-21 17:43] Matti: pretty sure you're just seeing the CLR doing JIT stuff

[2024-06-21 17:43] Matti: it has to write the instructions somewhere after all

[2024-06-21 17:44] Matti: if it is that, you probably want to patch the CIL, not the native (x86/arm/whatever) instructions

[2024-06-21 17:47] Matti: not only are there no guarantees about RVAs for the generated native code, the actual instructions themselves may also change depending on the .NET runtime

[2024-06-21 17:48] Matti: patching the CIL is easier and less brittle IMO

[2024-06-21 18:03] 0xc3.ard: Oki, I'm going to try that

[2024-06-21 18:10] expy: btw, <@651054861533839370> is it possible to convert unicode string to a regular one in D lang?
UPD: also there seem to be no memcmp-like functions
UPD2: is there any way of modifying the results of a syscall with a Dtrace?
thanks

[2024-06-21 20:32] asz: i do that in my code

[2024-06-21 20:33] asz: you can modify result yes

[2024-06-21 23:39] elias: Does anyone know good literature about code obfuscation (especially through code virtualization)

[2024-06-22 00:10] estrellas: [replying to elias: "Does anyone know good literature about code obfusc..."]
https://gist.github.com/assarbad/83e7def48a986727b12fcc644da1aa57?t

[2024-06-22 00:43] expy: > i do that in my code
didn't find that, it's possible to print it via "%ws" but I'm looking for a way I can compare string which came from wchar_t* array with literal in a script

[2024-06-22 00:45] expy: > you can modify result yes
should I modify arg0 in the :return function? got an error, something like "you can't modify a read-only variable"

[2024-06-22 01:35] asz: its there

[2024-06-22 01:36] asz: search unicode_string

[2024-06-22 01:36] asz: oh you mean return value? not sure

[2024-06-23 07:16] Horsie: has anyone tried the new binja decompiler?

[2024-06-23 07:16] Horsie: Any thoughts?

[2024-06-23 08:12] kcahres: looks good, but need some time/efforts to shape it

[2024-06-23 08:12] kcahres: this is why I encourage to write issues if you notice anything weird

[2024-06-23 21:41] BWA RBX: Does anyone think that binja will ever surpass Hexrays

[2024-06-23 21:42] North: [replying to BWA RBX: "Does anyone think that binja will ever surpass Hex..."]
Depends on what features you value

[2024-06-23 21:42] Brit: not for decomp

[2024-06-23 21:42] Brit: imo

[2024-06-23 21:43] BWA RBX: Fair mate, just trying to figure out whether paying for binja is worth it obviously I can't afford IDA

[2024-06-23 21:43] Brit: ghidra is probably what you should be looking at

[2024-06-23 21:44] North: [replying to BWA RBX: "Fair mate, just trying to figure out whether payin..."]
Personally I recommend binja if you are starting out and want to get more serious

[2024-06-23 21:44] BWA RBX: I can afford Binja would you say Ghidra is better?

[2024-06-23 21:44] North: The thing is ghidra is good but the ui is just brain cancer

[2024-06-23 21:45] North: It’s hard to recommend a paid product if you haven’t tried reversing before

[2024-06-23 21:45] BWA RBX: I will save up for IDA Pro but for now I think I'll purchase binja

[2024-06-23 21:45] Brit: I'd definitely recc ghidra before binja personally

[2024-06-23 21:46] BWA RBX: I'll check it out

[2024-06-23 21:46] Brit: especially if you're not leveraging the API but just literally looking at decomp

[2024-06-23 21:46] BWA RBX: Never really tried out ghidra

[2024-06-23 21:46] North: Make sure you follow along with a tutorial for it

[2024-06-23 21:46] North: It’s easy to get overwhelmed by the options

[2024-06-23 21:46] BWA RBX: They have documentation?

[2024-06-23 21:47] BWA RBX: I'll check the docs out