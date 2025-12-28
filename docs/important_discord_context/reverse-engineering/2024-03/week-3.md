# March 2024 - Week 3
# Channel: #reverse-engineering
# Messages: 174

[2024-03-11 02:43] donna🤯: yea just make a hypervisor should only take a day or 2

[2024-03-11 02:43] donna🤯: preferrable type 1 aswell

[2024-03-11 02:56] Timmy: yeah type 2 is too easy

[2024-03-11 03:26] daax: [replying to Windy Bug: "Any cool windows / kernel related one week researc..."]
P&P notify routines meaning what?

[2024-03-11 03:53] koyz: Maybe PnP?

[2024-03-11 05:43] Windy Bug: [replying to daax: "P&P notify routines meaning what?"]
How are they organised internally , write some code to enumerate registered ones , and how are they being used in security I guess , and yes I meant plug and play

[2024-03-11 06:23] fain: [replying to donna🤯: "yea just make a hypervisor should only take a day ..."]
How hard is type 1? Currently writing a type 2 hv

[2024-03-11 11:41] donna🤯: [replying to fain: "How hard is type 1? Currently writing a type 2 hv"]
lot more work, ur essentially writing an operating system that handles ya guests

[2024-03-11 11:50] contificate: just run an operating system on a privileged domain to handle ya guests with basic hypercalls

[2024-03-11 15:36] daax: [replying to fain: "How hard is type 1? Currently writing a type 2 hv"]
neither are very difficult

[2024-03-11 15:36] daax: to get developed, the difficulty comes with adding capabilities

[2024-03-11 15:37] daax: [replying to Timmy: "yeah type 2 is too easy"]
i'm sure this is a joke, but neither are difficult - they're just time consuming

[2024-03-11 15:37] daax: they're also not the end-all-be-all of technology, dunno what the obsession is these days with them

[2024-03-11 15:46] daax: [replying to Windy Bug: "Any cool windows / kernel related one week researc..."]
if you are interested in something that's not well documented, reversing some other modules outside of NTOS - clipsp, warbird, etc have some very interesting things going on in there and leverage some undocumented behaviors of APIs - would be a fun research project to dig into those and see how they work within the system

[2024-03-11 15:51] daax: clipsp has lots of transformations applied to their code, but only a subset - then they have loads of indirection which can be rebound to get information or spoof without disabling PG - they use a lot of opaque constants, nothing tooooo crazy but interesting nonetheless (imo)

[2024-03-11 15:51] asz: 
[Attachments: image.png]

[2024-03-11 15:52] asz: 
[Attachments: image.png]

[2024-03-11 15:53] asz: 
[Attachments: image.png]

[2024-03-11 15:54] asz: so much to look at

[2024-03-11 23:37] hxm: <@651054861533839370>  what this addr reffers to ? 0xFFFFF780000002EC

[2024-03-11 23:38] asz: 
[Attachments: image.png]

[2024-03-11 23:40] Matti: [replying to hxm: "<@651054861533839370>  what this addr reffers to ?..."]
SharedUserData->SafeBootMode
right

[2024-03-11 23:40] Matti: if he didn't answer already...

[2024-03-11 23:40] Matti: <https://www.geoffchappell.com/studies/windows/km/ntoskrnl/inc/api/ntexapi_x/kuser_shared_data/index.htm>

[2024-03-12 10:10] abu: Hey everyone. Given a base address and a pdb file. Does anyone know how to load the pdb in radare2? I've tried idp but it doesn't load at the correct base address. Even when I specify -B.

[2024-03-12 10:11] abu: I can load the binary, but it doesn't give me function name information

[2024-03-12 11:14] abu: oh for fucks sakes it reading in wrong endianess

[2024-03-12 11:22] Azalea: ooof

[2024-03-12 13:04] Mysterio: If you wanted to hook a large amount (say 10+) functions in a shared library (say ntdll or cryptbase) ... how would you do it?

[2024-03-12 13:06] Mysterio: Emphasis is on the "large amount" here... not general hooking question fyi. I don't want to write all those function prototypes and boiler plate.

[2024-03-12 13:24] dullard: [replying to Mysterio: "If you wanted to hook a large amount (say 10+) fun..."]
What do you want to achieve with the hooks ?

[2024-03-12 13:25] dullard: apimonitor can be used if you want simple reading of the params / returns

[2024-03-12 14:01] Mysterio: [replying to dullard: "What do you want to achieve with the hooks ?"]
something similar to apimonitor where i'm trying to track the flow of a program... but not sure if it supports the functions I want and the target has anti-debug.. not sure how apimonitor works

[2024-03-13 04:53] BWA RBX: <@609487237331288074> nice blog post regarding object initializers in windows

[2024-03-13 05:06] daax: [replying to BWA RBX: "<@609487237331288074> nice blog post regarding obj..."]
thanks, it’s just for fun, not recommended for production or anything outside poking at things. there is a stable way to do the debugobject method in a PG compliant manner on Windows 10/11, just need to allocate time to write it out or append it

[2024-03-13 05:08] daax: [replying to Mysterio: "something similar to apimonitor where i'm trying t..."]
so you only really need the ability to log or you want to have a hook and potentially modify results/returned data to the caller?

[2024-03-13 08:50] dullard: If you don’t mind extremely verbose output <@651054861533839370> ‘ degenerate is nice to trace a process

[2024-03-13 08:51] dullard: If you want to hook 10+ functions isn’t too much effort with detours

[2024-03-13 15:29] Mysterio: [replying to daax: "so you only really need the ability to log or you ..."]
At this moment... only log, but probably modify once I do some tracing.

[2024-03-13 15:30] Mysterio: [replying to dullard: "If you don’t mind extremely verbose output <@65105..."]
Do you have a link?

[2024-03-13 16:11] dullard: [replying to Mysterio: "Do you have a link?"]
https://twitter.com/jonaslyk/status/1568450498579111936
[Embed: Jonas L (@jonasLyk) on X]
Presenting D-Generate , syscall tracing as its supposed to be!

https://t.co/8qXjp9R381

usage:

dg cmd.exe - displays all syscalls done by process with cmd.exe as imagefile.
dg 4736 - by pid 4736
dg 

[2024-03-13 16:11] dullard: google

[2024-03-13 17:58] Mysterio: thank you. looks like they essentially wrote a whole bunch of boiler plate to achieve that, with the help of some metaprogramming scripts i hope

[2024-03-13 17:59] Mysterio: looks like the answer to my question is... write the boiler plate :|... maybe with the help of some scripts

[2024-03-13 18:00] Matti: I am 100% serious when I say all code you read that was written by jonas, was written by jonas

[2024-03-13 18:28] dullard: [replying to Mysterio: "looks like the answer to my question is... write t..."]
No, just run dtrace with his D and you’ll get full tracing

[2024-03-13 18:28] dullard: *full ish

[2024-03-13 18:35] Mysterio: only for ntdll

[2024-03-13 18:36] Mysterio: which is nice... but also interested in other libs

[2024-03-13 21:38] dullard: Hmm true

[2024-03-13 21:38] dullard: Well, how familiar are you with hooking ?

[2024-03-13 21:38] dullard: What are you looking to hook?

[2024-03-13 21:40] mrexodia: [replying to dullard: "Well, how familiar are you with hooking ?"]
I'm pretty familiar dullard

[2024-03-13 21:40] mrexodia: 🙄

[2024-03-13 21:43] dullard: [replying to mrexodia: "I'm pretty familiar dullard"]
🙄🙄🙄

[2024-03-13 21:44] 25d6cfba-b039-4274-8472-2d2527cb: I personally would call myself even a professional hooker

[2024-03-13 21:51] Mysterio: I'm a high end escort

[2024-03-13 21:54] Matti: [replying to Mysterio: "thank you. looks like they essentially wrote a who..."]
how stupid is your target? if it cares at all

[2024-03-13 21:54] Mysterio: It's pretty smart if you want to call it that

[2024-03-13 21:54] Mysterio: Have to do some debug bypasses

[2024-03-13 21:54] Mysterio: I'm internal at this point

[2024-03-13 21:55] Matti: I once sort of did this by IAT hooking a driver and pretending to be ntoskrnl.exe by modifying the loaded module list and putting my own driver first

[2024-03-13 21:55] Mysterio: It wouldn't detect that

[2024-03-13 21:56] Matti: the way I made this fake ntoskrnl.exe was.... via regex <:harold:704245193016344596>

[2024-03-13 21:56] Matti: starting with `dumpbin /exports ntoskrnl.exe`

[2024-03-13 21:56] Mysterio: At this point I've just dredged it up and wrote 20 function prototypes to do some hooks

[2024-03-13 21:56] Mysterio: But curious what would be the suggestion if I wanted to do .. 500

[2024-03-13 21:57] Matti: eventually ending up with about 3000 functions which go something like
```c
Log("NameOfExport entry\n");
jmp real_address
```

[2024-03-13 21:58] Matti: [replying to Mysterio: "But curious what would be the suggestion if I want..."]
well so this worked for 3000 or so, but it was painful as fuck to make

[2024-03-13 21:58] Matti: and I honestly can't remember much about this project (fortunately)

[2024-03-13 21:58] Matti: I should still have the disgusting source code somewhere though

[2024-03-13 21:59] Matti: it was highly ntoskrnl specific

[2024-03-13 22:00] Mysterio: I'm using minhook 
, x64. I was thinking I could just use python or whatever to write a bunch of func(void* a1, void* a2... Up to say 10) and just always return a void*

[2024-03-13 22:00] Mysterio: That shouldn't cause any crashes

[2024-03-13 22:00] Mysterio: Basically don't care what the function prototype actually is

[2024-03-13 22:00] Matti: for x64, and for most functions, that should be fine yeah

[2024-03-13 22:01] Matti: but if you really want this to work generically you probably can't avoid saving regs, doing your log, restoring them and ending with `jmp`

[2024-03-13 22:02] Matti: but tbh the python approach should work for most x64 user mode executables on windows, I expect

[2024-03-13 22:02] Matti: so long as they don't use vectorcall or something like that

[2024-03-13 22:03] dullard: [replying to Mysterio: "I'm using minhook 
, x64. I was thinking I could j..."]
That’s a quick and dirty way to do it 😄

[2024-03-13 22:03] dullard: I like it haha

[2024-03-13 22:03] Matti: this isn't that far off from what I did

[2024-03-13 22:03] Matti: only I used C

[2024-03-13 22:03] Matti: I mean my own thing had to be loaded as a driver as well so

[2024-03-13 22:04] Matti: 
[Attachments: image.png]

[2024-03-13 22:04] dullard: The only issue with that is if you want to intercept any COM functions that aren’t directly exported from a DLL I.e. something not like WMI

But if it’s all win32/ntapi/aux lib funcs should be ezpz

[2024-03-13 22:04] Matti: obviously the call is some log

[2024-03-13 22:04] Matti: + function address getter

[2024-03-13 22:04] Matti: yeah, my approach only works for dllexports

[2024-03-13 22:05] Matti: if you want to hook vtables it won't help

[2024-03-13 22:05] dullard: I found a very cheeky way to do COM hooking without messing with vtables by abusing the class factory

[2024-03-13 22:05] dullard: Well “abusing”

[2024-03-13 22:05] dullard: And swapping out the pointer to your own class

[2024-03-13 22:07] dullard: I was writing a “spoofer” for a VPN which had posture policies and it was checking for bitlocker using an interesting COM module, I was too smooth brain to wrap my head around vtables so I did it a different way 😄

[2024-03-13 22:07] dullard: I want to write it up / share the code at some point but I have no time :/

[2024-03-13 22:07] dullard: [replying to Matti: "I should still have the disgusting source code som..."]
If you find it, @ me, I’m curious to see this 😄 haha

[2024-03-13 22:08] Matti: I have found it

[2024-03-13 22:08] Matti: <@655419785106030612>

[2024-03-13 22:08] Matti: duly @'d

[2024-03-13 22:08] Matti: you will not be seeing this code

[2024-03-13 22:09] Matti: it is too gross

[2024-03-13 22:09] dullard: ⁉️

[2024-03-13 22:09] dullard: Haha

[2024-03-13 22:09] dullard: I’ll show mine if you show yours <:FlushedFluent:865333543193280522>

[2024-03-13 22:09] dullard: I can say for sure that my code is more gross

[2024-03-13 22:10] Matti: not only is it gross code, it turns out most of the functions exported have type information (parameters and return type)

[2024-03-13 22:10] Matti: which makes the solution not generic

[2024-03-13 22:10] dullard: You must’ve put the time in to manually setup the prototypes

[2024-03-13 22:10] dullard: Or use symbols to generate them 🤔

[2024-03-13 22:11] Matti: well give me a sec

[2024-03-13 22:11] Matti: I'm reversing my own thing here

[2024-03-13 22:11] Matti: because there's also things like
```c
STUB
NTAPI
STUB_ExDisableResourceBoostLite(
    );
```

[2024-03-13 22:11] Matti: no idea what happens with that

[2024-03-13 22:11] dullard: [replying to Matti: "no idea what happens with that"]
Future me reading past me’s code

[2024-03-13 22:12] dullard: 😂😂

[2024-03-13 22:12] Matti: but there are hundreds like
```c
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NTAPI
ExCreateCallback(
    _Outptr_ PCALLBACK_OBJECT *CallbackObject,
    _In_ POBJECT_ATTRIBUTES ObjectAttributes,
    _In_ BOOLEAN Create,
    _In_ BOOLEAN AllowMultipleCallbacks
    )
{
    return ((t_ExCreateCallback)IddqdGetSystemRoutineAddress("ExCreateCallback"))(CallbackObject, ObjectAttributes, Create, AllowMultipleCallbacks);
}
```

[2024-03-13 22:12] Matti: which yeah I did actually generate via regex

[2024-03-13 22:12] Matti: a lot of regex

[2024-03-13 22:12] dullard: Cursed !

[2024-03-13 22:14] Matti: it passes first inspection until you notice it's 235 KB
[Attachments: image.png]

[2024-03-13 22:15] Matti: not sure what happened with the timedatestamp there

[2024-03-13 22:15] Matti: this is more from like 2016

[2024-03-13 22:15] Matti: also, only 1630 exports, not 3000

[2024-03-13 22:17] Matti: hum.... it seems the stubs are windows 8/10 shit

[2024-03-13 22:17] Matti: so I got rid of them to decrease the amount of regexing I had to do

[2024-03-13 22:18] Matti: this driver worked on windows 7 at the time

[2024-03-14 10:44] brymko: https://tenor.com/view/squidward-daring-today-spongebob-spongebob-meme-daring-today-arent-we-gif-23808696

[2024-03-14 10:46] brymko: theres this little thing called don't ask to ask 🤭

[2024-03-14 10:48] Supplant: lol

[2024-03-14 10:50] brymko: there will be no discussion around game reversing as it is against discord tos <:flushed_orange:606580514438250497>

[2024-03-14 11:39] asz: [replying to Mysterio: "only for ntdll"]
no

[2024-03-14 11:40] asz: i do it in ring 0

[2024-03-14 13:53] daax: Read the <#835634425995853834>

[2024-03-14 14:19] Mysterio: [replying to asz: "i do it in ring 0"]
yeah it works generically but there's only boiler plate for ntdll

[2024-03-14 14:20] asz: no

[2024-03-14 14:20] asz: theres only boiler plate for syscalls

[2024-03-14 15:01] hxm: <@651054861533839370>  in what context the usage of IoAllocateMdl is a must ?

[2024-03-14 16:37] Deleted User: hey we are looking for someone who could help with the reversal of a grpc stream in a mobile application, message me if you are interested in a paid project

[2024-03-14 18:14] [Janna]: [replying to Deleted User: "hey we are looking for someone who could help with..."]
i have no clue but maybe this is better in <#902892977284841503>  ? xD

[2024-03-15 02:44] mibho: is there some secret to bit 11 for dr7 on x86? wiki says reserved/0  <:Charmanderp:785324601047121930> 
 
dr7 & 0x500    

0101 0000 0000
bit 9 = global exact bp enable
bit 11 = ??

[2024-03-15 03:01] daax: bit 11 is RTM

[2024-03-15 03:01] daax: [replying to mibho: "is there some secret to bit 11 for dr7 on x86? wik..."]
dont use wikis

[2024-03-15 03:01] daax: use the SDM

[2024-03-15 03:24] mibho: what do those mean so i can google <:KEKWsob:778131219870253066>

[2024-03-15 04:12] daax: [replying to mibho: "what do those mean so i can google <:KEKWsob:77813..."]
use the SDM

[2024-03-15 04:12] daax: DR7

[2024-03-15 04:12] daax: 
[Attachments: image.png]

[2024-03-15 04:13] daax: for standard arch things like this it'll be your bff

[2024-03-15 05:07] mibho: [replying to daax: "use the SDM"]
wat is sdm 😭

[2024-03-15 05:08] Deleted User: You can just google it?

[2024-03-15 05:08] daax: [replying to mibho: "wat is sdm 😭"]
Intel sdm

[2024-03-15 05:11] mibho: oh the big ass manual

[2024-03-15 05:11] mibho: TIL

[2024-03-15 05:11] mibho: thank u

[2024-03-15 10:41] [Janna]: [replying to mibho: "wat is sdm 😭"]
uh-uh let see.. s-software defined manual? xD

[2024-03-15 10:41] [Janna]: jokes aside

[2024-03-15 11:55] not-matthias: Has anyone here ever looked into how API Monitor works? Attached to the monitored process but couldn't really see direct hooks. Also doesn't seem to use PAGE_GUARD hooks, as the permissions are not changed.

Any insights?

[2024-03-15 11:56] dullard: [replying to not-matthias: "Has anyone here ever looked into how API Monitor w..."]
it uses shims iirc

[2024-03-15 11:56] dullard: I cant remember how it installs the shims

[2024-03-15 12:49] diversenok: [replying to not-matthias: "Has anyone here ever looked into how API Monitor w..."]
IAT hooks

[2024-03-15 19:28] mrexodia: [replying to not-matthias: "Has anyone here ever looked into how API Monitor w..."]
It depends on the mode you select in the options

[2024-03-15 20:17] yeperthi: hi, i put a breakpoint on send() function in windbg and when it hit, i took the buf*'s address out of the stack and tried to put a hardware bp on it to see when it gets read / changed but it just doesn't work, it always says "the specified address can not be resolved" or just puts a useless blank bp in my breakpoint list, i'm sure i'm doing something wrong here because it looks like that this address is out of bounds.
[Attachments: image.png]

[2024-03-15 22:56] Matti: [replying to yeperthi: "hi, i put a breakpoint on send() function in windb..."]
you want `ba` for a hardware bp, not `bp`

[2024-03-15 22:57] Matti: e.g. `ba r 8 0x12345678`

[2024-03-16 19:08] Windy Bug: What is the name of the mitigation that causes diffs in text sections of modules on disk compared to memory? Having a blackout rn

[2024-03-16 19:12] diversenok: Module tampering protection?

[2024-03-16 19:12] diversenok: https://windows-internals.com/understanding-a-new-mitigation-module-tampering-protection/
[Embed: Understanding a New Mitigation: Module Tampering Protection]

[2024-03-16 19:14] Windy Bug: Nah I don’t think so

[2024-03-16 19:14] Windy Bug: It was something to do with indirect calls iirc

[2024-03-16 19:14] Windy Bug: maybe I’m waffling tho I completely forgot what that was

[2024-03-16 19:16] Windy Bug: Found it , Retpoline

[2024-03-16 20:45] hxm: is there a way with IDA c++ SDK to get the number of basicblocks of a function ?

[2024-03-16 21:08] brymko: definitely

[2024-03-16 21:08] brymko: but i don't remember how,