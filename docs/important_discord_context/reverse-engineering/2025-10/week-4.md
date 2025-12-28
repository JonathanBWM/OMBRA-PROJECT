# October 2025 - Week 4
# Channel: #reverse-engineering
# Messages: 200

[2025-10-20 17:06] mrexodia: [replying to plox: "I am interested in the idea of setting up my own '..."]
Just use devcontainers

[2025-10-20 17:07] mrexodia: That docker escape stuff is complete nonsense

[2025-10-20 22:07] plox: <@162611465130475520> 
I came to conclusion containers are not the answer based off the comments above. thought I was onto something with my setup. Realized how wrong I was once I had to switch to 32bit heap based challenges.

Now I just manually create my VMs but I am still interested in learning what is considered a good environment for RE. Might make some scripts to automate proxmox to make the VMs for me, havent given it much thought since then

[2025-10-20 22:34] plpg: I find proxmox too "isolated" for RE work

[2025-10-20 22:34] plpg: for example, there is no easy way to do USB passthrough, compared to VBox or VMware

[2025-10-20 22:35] plpg: My setup is usually some variation of VirtualBox or VMware, where i set up a VM with an OS, install the tools and then make a snapshot

[2025-10-20 22:35] plpg: alternatively make clones of that VM

[2025-10-20 22:36] plpg: or another thing that's hard in proxmox is disconnecting/connecting the virtual network cable

[2025-10-22 07:48] Ghost: Hi, <@789295938753396817>!
I'm trying to run KasperskyHook(https://github.com/iPower/KasperskyHook), but it's not working now. Can you help me on it?

[2025-10-22 10:17] ImagineHaxing: [replying to Ghost: "Hi, <@789295938753396817>!
I'm trying to run Kaspe..."]
First clue is that its 1+ year old and might be outdated

[2025-10-22 11:43] Ghost: [replying to ImagineHaxing: "First clue is that its 1+ year old and might be ou..."]
Any solution for it?

[2025-10-22 11:44] ImagineHaxing: [replying to Ghost: "Any solution for it?"]
Not really

[2025-10-22 11:47] Ghost: Hi, <@456829418116087825> 
Please help me by providing a working kernel-mode API hooking example code..

[2025-10-22 11:48] HeaZzY: lol im not an AI

[2025-10-22 12:00] Brit: the @ everyone is insane

[2025-10-22 12:01] Rairii: [replying to Ghost: "Hi, <@456829418116087825> 
Please help me by provi..."]
ok you didn't specify architecture or version so here you go https://github.com/Wack0/entii-for-workcubes/blob/main/halartx/source/ldrhook.c
[Embed: entii-for-workcubes/halartx/source/ldrhook.c at main · Wack0/entii...]
PowerPC Windows NT ported to Nintendo GameCube/Wii/Wii U - Wack0/entii-for-workcubes

[2025-10-22 12:54] Ghost: [replying to Brit: "the @ everyone is insane"]
Really sorry on it.

[2025-10-22 12:54] Jacob: [replying to Ghost: "Really sorry on it."]
The hacker pfp is tuff

[2025-10-22 12:56] Addison: [replying to Brit: "the @ everyone is insane"]
you got got

[2025-10-22 12:57] Ghost: [replying to Rairii: "ok you didn't specify architecture or version so h..."]
does it work on the latest Windows 11 x64?

[2025-10-22 12:57] Addison: bro

[2025-10-22 12:57] Addison: You don't even need to click the link

[2025-10-22 13:11] Ghost: do you know a sample driver project that hooks kernel-mode APIs on Windows 10 and Windows 11?

[2025-10-22 13:11] Gestalt: [replying to Ghost: "do you know a sample driver project that hooks ker..."]
search brother

[2025-10-22 13:11] Gestalt: search

[2025-10-22 13:11] Gestalt: google is your bu(t)tler at your hands

[2025-10-22 13:12] Ghost: yeah, right. I tried, but I couldn't find a proper one.

[2025-10-22 13:12] Gestalt: [replying to Ghost: "yeah, right. I tried, but I couldn't find a proper..."]
in terms of what funny man

[2025-10-22 13:13] Ghost: some samples worked only on VM.

[2025-10-22 13:13] Gestalt: what deducation can you do based on that

[2025-10-22 13:14] Ghost: is it impossible on real PC with modern Windows?

[2025-10-22 13:15] Gestalt: [replying to Ghost: "is it impossible on real PC with modern Windows?"]
the os is the same in a vm no?

[2025-10-22 13:15] Ghost: yeah, but Security feature is different on real hardware.

[2025-10-22 13:16] Gestalt: [replying to Ghost: "yeah, but Security feature is different on real ha..."]
https://tenor.com/view/clapping-leonardo-dicaprio-leo-dicaprio-well-done-applause-gif-13688780714394676664

[2025-10-22 13:17] Ghost: no solution on real PC?

[2025-10-22 13:17] Gestalt: ofc there is a solution on a "real pc"

[2025-10-22 13:17] Ghost: please help me

[2025-10-22 13:19] Gestalt: [replying to Ghost: "please help me"]
I am sorry, but this server is not about spoon feeding

[2025-10-22 13:20] Gestalt: [replying to Ghost: "please help me"]
just curious what are you planning to hook?

[2025-10-22 13:21] Ghost: just for learning

[2025-10-22 13:21] Gestalt: [replying to Ghost: "just for learning"]
what

[2025-10-22 13:22] Gestalt: how is going from 0 to directly 100 learning mistah?

[2025-10-22 13:22] Ghost: well, I have some understanding, but I couldn't complete kernel-mode hook on real PC..

[2025-10-22 13:23] Gestalt: yeah?

[2025-10-22 13:23] Gestalt: well then I guess the learning process would be how to bypass these security mechanisms

[2025-10-22 16:23] iPower: [replying to Ghost: "Hi, <@789295938753396817>!
I'm trying to run Kaspe..."]
are you expecting me to help you when you literally give no details about the issues you're having?

[2025-10-22 16:36] ImagineHaxing: <@1282203232147603508> the first step would be to learn how to learn

[2025-10-22 16:36] ImagineHaxing: Based on my personal opinion

[2025-10-22 16:40] pinefin: im actually crying right now
[Attachments: Screenshot_2025-10-22_at_11.40.00_AM.png]

[2025-10-22 16:40] pinefin: i do really wanna know what hes trying to achieve. in terms of end goal

[2025-10-23 04:43] North: Really about that reversing life 🙏

[2025-10-23 08:31] Ghost: [replying to iPower: "are you expecting me to help you when you literall..."]
Please check your github repository..

[2025-10-23 08:40] Ghost: I tried with the latest klhk.sys by installing Kaspersky antivirus software, but it didn't work. I attached its screenshot.
Please help on it..
[Attachments: image.png]

[2025-10-23 08:50] x86matthew: [replying to Ghost: "I tried with the latest klhk.sys by installing Kas..."]
about 30 seconds of looking at the source shows that it is failing in one of these two functions https://github.com/iPower/KasperskyHook/blob/master/KasperskyHookDrv/kaspersky.cpp#L21-L117

[2025-10-23 08:50] x86matthew: which gives you more than enough information to figure it out

[2025-10-23 09:21] grb: Im currently reversing securekernel and i stumble upon a function called SkmiLockDriverNtAddresses, the function itself essentially takes 2 things, a start VA and a page count, if you can see in the while loop, it checks a few things in the PTE bits, first it checks if bit 9 is enabled, and then check other bits too. TargetMdlStartVa is a VTL0 address. Now my question is that does this function really checks the PTE at VTL0 or maybe its checking against something else? because idk why it feels weird that it checks if CopyOnWrite bit is enabled in the PTE. Btw im comparing against this https://www.vergiliusproject.com/kernels/x64/windows-11/24h2/_MMPTE_HARDWARE
[Attachments: image.png]
[Embed: Vergilius Project]
Take a look into the depths of Windows kernels and reveal more than 60000 undocumented structures.

[2025-10-23 09:58] Ghost: [replying to x86matthew: "about 30 seconds of looking at the source shows th..."]
well, service creation was failed.. please let me know why you think one of the 2 functions is the reason.

[2025-10-23 10:01] x86matthew: [replying to Ghost: "well, service creation was failed.. please let me ..."]
look at your debug output

[2025-10-23 10:01] x86matthew: and then look at this https://github.com/iPower/KasperskyHook/blob/master/KasperskyHookDrv/driver.cpp#L39

[2025-10-23 10:02] Brit: what is a man who can't be fucked with reading the github project he's trying to use going to do with winkernel hooks.

[2025-10-23 10:29] Obvious: What is your approach when you open IDA? Do you straight up decompile or read ASM instructions first?

[2025-10-23 10:30] Obvious: Or check on the disassembly when you feel something's off in the decompiled code?

[2025-10-23 10:30] Obvious: I'm fairly new to this stuff ^

[2025-10-23 10:31] Gestalt: [replying to Obvious: "Or check on the disassembly when you feel somethin..."]
depends if I am working with something obfuscated, pseduo code won't do much, most of the times

[2025-10-23 10:32] Gestalt: generally pseduo decompile always first

[2025-10-23 10:32] Obvious: also i want to ask one more thing,  is disassembly 100% correct?

[2025-10-23 10:32] Obvious: i dont think it's possible

[2025-10-23 10:32] Gestalt: disassembly or decomp?

[2025-10-23 10:32] Obvious: disassembly

[2025-10-23 10:33] Gestalt: its not always correct especially not when dealing with obfuscations, sometimes you have code that changes its own instructions

[2025-10-23 10:33] Gestalt: so they might put in some junk code

[2025-10-23 10:33] Gestalt: ofc this is super simplified what I am saying

[2025-10-23 10:34] Obvious: [replying to Gestalt: "generally pseduo decompile always first"]
what'd u advise for a beginner? stick to the disasm just for the sake of  learning?

[2025-10-23 10:34] Gestalt: but if you work with normal stuff without any obfuscation it should be mostly fine

[2025-10-23 10:34] Gestalt: [replying to Obvious: "what'd u advise for a beginner? stick to the disas..."]
I learned RE through trial and error, I dont recommend it, I have no clue what the best start would be

[2025-10-23 10:35] plpg: [replying to Obvious: "what'd u advise for a beginner? stick to the disas..."]
Hard to say for me either, but keep in mind decompilers work sensibly only on compiler-produced code

[2025-10-23 10:35] Obvious: i see, im a uni student and im interested in these topics so im coding in c++ along with some RE, also helps that i will have classes on OS, architecture etc. next year

[2025-10-23 10:36] plpg: So dont blindly trust the decompiler

[2025-10-23 10:36] Gestalt: ^^^^^^^ I agree

[2025-10-23 10:36] Gestalt: fooling the decompiler is quite easy

[2025-10-23 10:36] plpg: And also i had it quite often that a decompiled code would just skip some parts of the assembly code which were doing something

[2025-10-23 10:37] Obvious: the way i understand it is if you have experience you can make use of the pseudo-code but someone like me wouldn't notice if something was off, lol.

[2025-10-23 10:37] Lyssa: [replying to Gestalt: "fooling the decompiler is quite easy"]
the decompiler can be wrong sometimes even if nothing was done to fool it on purpose

[2025-10-23 10:37] plpg: Knowing assembly goes a long way. you dont have to learn all the instructions by heart but you should be able to tell whats a function call or what does some piece of code does

[2025-10-23 10:38] Lyssa: [replying to Lyssa: "the decompiler can be wrong sometimes even if noth..."]
I've seen IDA for example not notice some function arguments being used a few times

[2025-10-23 10:38] Lyssa: due to optimization

[2025-10-23 10:38] Lyssa: and some other somewhat common issues that are kind of predictable at least

[2025-10-23 10:39] Lyssa: decompiler is very useful but u should go through the output methodically and don't trust it blindly

[2025-10-23 10:39] Obvious: thanks, one last thing. should i be learning implementations of function calls in other operating systems such as linux?

[2025-10-23 10:39] Obvious: i believe it was called system v for linux

[2025-10-23 10:40] plpg: That depends, if you want to deal with linux then sure

[2025-10-23 10:40] Gestalt: learn with what you mainly want to deal

[2025-10-23 10:40] archie_uwu: sometimes IDA misses entire function calls and marks a function as empty in the decompiled output

[2025-10-23 10:40] Gestalt: quality over quantity

[2025-10-23 10:41] archie_uwu: mostly saw this when looking at ntoskrnl

[2025-10-23 10:41] Obvious: thanks.

[2025-10-23 10:46] Brit: disassembly will mostly be correct bar some specific memes

[2025-10-23 10:48] plpg: We do a little jumping in the middle of instructions

[2025-10-23 11:05] Brit: immediately obvious

[2025-10-23 11:05] Brit: especially given the god awful instr you have to make for it to be a valid jump target

[2025-10-23 11:28] Gestalt: Push ret my beloved

[2025-10-23 13:54] iPower: [replying to Ghost: "well, service creation was failed.. please let me ..."]
signatures are outdated

[2025-10-23 13:56] iPower: next time I'm banning you. you're warned

[2025-10-23 13:57] iPower: don't come here pinging people to solve your issues when you didn't even try to figure out yourself

[2025-10-23 13:57] Ghost: Got it. I will take a deep look and research on it by myself again.. Thanks for your response

[2025-10-23 13:58] iPower: I already gave you the answer amigo

[2025-10-23 13:58] iPower: you should be able to make it work now

[2025-10-23 19:50] archie_uwu: I'm looking at secure kernel and how transitions across VTLs work. I've found that transitions from VTL 1 to VTL 0 sometimes use different CR3s. I couldn't find any info relating to this in any writeups, and looking at public code, they all seem to just be storing info from the very first HvCallVtlReturn hypercall - is there a reason for that?
[Attachments: image.png]

[2025-10-23 19:55] archie_uwu: Relevant code:
```cpp
HvExitResult Hv::Exits::SeqHandleVtlReturn(
    IN UINT16 CoreIndex, 
    IN VMCTX* Context
)
{
    // Read the value at the top of the stack (return address)
    UINT64 thread_retaddr = 0;
    NTSTATUS last_status = Memory::MmReadVirtualMemory(
        Context->CR.Cr3, 
        Context->Regs.Rsp, 
        sizeof(UINT64), 
        &thread_retaddr
    );

    // ... error handling omitted ...

    UINT64 sk_base = 0;
    UINT32 sk_size = 0;
    
    // Grab the base address and size of the secure kernel module.
    last_status = Memory::MmPcToImageBase(
        Context->CR.Cr3,
        thread_retaddr,
        &sk_base,
        &sk_size
    );

    // ... more error handling omitted ...

    Log::LogString(Context, "Found SK.exe at %I64x with size %x (CR3 %I64x)", sk_base, sk_size, Context->CR.Cr3);
}
```
CR3 is fetched directly from the VMCB - running nested inside VMware Workstation, Windows 11 24H2

[2025-10-24 14:58] pinefin: im a noob when it comes to anything java (only ever really touched it for a class). and theres an app i wanna change some of the workflow, how can i reverse engineer and write hooks and/or patch code like i would a normal x86 assembled exe?

[2025-10-24 15:08] Bloombit: the company thatdot has a product written in scala that does anomaly detection to indicate compromise.
I used strace to find what sort of call to write to hook for suppression. hooking was the same as normal

[2025-10-24 15:13] Brandon: [replying to pinefin: "im a noob when it comes to anything java (only eve..."]
I don't know it either, but I know that Minecraft moding master java hooking and code injection:
https://youtu.be/Hmmr1oLt-V8?si=k7gadc-bvPRMPABh
[Embed: Modding is Hacking...]
In this episode of Minecraft Hacked we are going to look into client mods and talk about cheating in general.

Fabric Example Mod: https://github.com/FabricMC/fabric-example-mod
Mixin Examples: https:

[2025-10-24 15:13] Brandon: You could use the same techniques

[2025-10-24 15:13] nidi: [replying to pinefin: "im a noob when it comes to anything java (only eve..."]
https://github.com/rdbo/jnihook might help
[Embed: GitHub - rdbo/jnihook: Hook Java methods natively from C/C++]
Hook Java methods natively from C/C++. Contribute to rdbo/jnihook development by creating an account on GitHub.

[2025-10-24 15:21] pinefin: its not for minecraft, but sweet i'll definitely have to look into these resources. thanks guys

[2025-10-24 15:21] pinefin: [replying to nidi: "https://github.com/rdbo/jnihook might help"]
this might be the choice for me 🤣 thanks very much

[2025-10-24 15:21] Timmy: if you want to stay in java

[2025-10-24 15:21] Timmy: https://asm.ow2.io/

[2025-10-24 15:23] Timmy: in general the java guys have p fked up terminology; i.e we use "hooking", they use "injection"

[2025-10-24 15:24] pinefin: 🤣 fair enough

[2025-10-24 15:25] pinefin: yeah this is all great information, thank you guys. i wont mess with it for 2-4 weeks but im gettign ready for another side project

[2025-10-24 17:10] silverf3lix: [replying to pinefin: "yeah this is all great information, thank you guys..."]
https://github.com/soot-oss/soot is pretty decent I have had good experience with it for APKs. They are rewriting the thing to make it maintainable https://github.com/soot-oss/SootUp but I haven't tested the new one with apks. In c++ land there is https://github.com/facebook/redex which was super fast and I used for simple static analyses. I'm sure you can use the InstrumentationPass stuff to rewrite dex files but i did not have patience to spend more time on it.

[2025-10-24 17:10] mtu: [replying to Timmy: "in general the java guys have p fked up terminolog..."]
They’re different things, no? Java stuff doesn’t tend to try to hook methods (because holy shit that would suck to do in a JIT VM) but they’ll just straight up overwrite class files and such

[2025-10-24 17:24] archie_uwu: [replying to archie_uwu: "I'm looking at secure kernel and how transitions a..."]
Ended up figuring out why storing info from the first ``HvCallVtlReturn`` hypercall is the best. It essentially makes sure you've stored SK's CR3, and not the CR3 of another process running in VTL 1 (trustlets I guess?). The calls all originate from within ``securekernel!SkCallNormalMode``, but the first VTL return is special.

It's special in the way that it's issued from ``securekernel!ShvlpVtl1Entry``, which is guaranteed to be running in SK's context, since no other process can be running at that time. ``ShvlpVtl1Entry`` is the starting IP of the first VTL 1 call (VTL 1 is "created" and initialized from ``ShvlpInitializeVpContext``, which creates a ``HV_INITIAL_VP_CONTEXT`` on the stack as per the hypercall specification - ``ShvlpVtl1Entry`` is stored in the RIP member of the struct)

[2025-10-24 17:25] guar: [replying to Obvious: "i see, im a uni student and im interested in these..."]
If i was in uni and learning cpp i would start using godbolt a lot
(ex: https://godbolt.org/z/Wbo7GrnKG)

[2025-10-24 17:33] guar: in a sense that if you have a grip/idea what assembly certain c code should compile into then it is easier doing it backwards

[2025-10-24 19:00] Shiro: im like rlly new to RE, and I have this game called battlebit. It's a IL2CPP game and I dumped the game assembly dll with the il2cpp dumper. The function names are rlly weird. But I was wondering if there is a way I can map the class names to the stuff in IDA? Sorry if this is a dumb question
[Attachments: image.png]

[2025-10-24 19:02] the horse: use il2cppdumper

[2025-10-24 19:02] the horse: you should be able to match the VA in IDA, but the plugins will also help you load all these methods / classes

[2025-10-24 19:05] the horse: they likely obfuscate the function names

[2025-10-24 19:05] the horse: not much you'll be able to od against that usually

[2025-10-24 19:05] Shiro: Yeah I used il2cppdumper already

[2025-10-24 19:06] Shiro: I know they had a version of the game where it wasn't obfuscated, do you think I should try using that one instead to find sigs?

[2025-10-24 19:06] the horse: are the field names obfuscated?

[2025-10-24 19:07] plpg: they are obfuscated yeah

[2025-10-24 19:08] Shiro: [replying to the horse: "are the field names obfuscated?"]
not all of them but some of the classes have them obfuscated
[Attachments: image.png]

[2025-10-24 19:08] the horse: I think you will probably have more luck reversing the functions in IDA.

[2025-10-24 19:08] the horse: and determining the one you want to sig

[2025-10-24 19:09] Shiro: and then some of the classes have only like half of the functions obfuscated idk
[Attachments: image.png]

[2025-10-24 19:09] Shiro: okay sounds good

[2025-10-24 19:09] Shiro: I just wasn't sure if it would be helpful to map em out

[2025-10-24 19:16] Timmy: [replying to mtu: "They’re different things, no? Java stuff doesn’t t..."]
from what I understand you do "hook" (our version) through bytecode modification or as iirc its short for "bytecode injection", it doesn't affect the jit because it's done as the classes themselves are loaded in

[2025-10-24 19:41] Gestalt: [replying to Shiro: "im like rlly new to RE, and I have this game calle..."]
man give battlebit a rest

[2025-10-24 19:43] pinefin: [replying to Timmy: "from what I understand you do "hook" (our version)..."]
this is the same for mono+c# as well, we hook in the actual JIT wrapper of the function instead of where the function contents are.

[2025-10-24 19:47] Shiro: [replying to Gestalt: "man give battlebit a rest"]
lol to be fair Im not gonna use the hacks or release it, just tryna learn

[2025-10-24 21:36] selfprxvoked: [replying to the horse: "I think you will probably have more luck reversing..."]
Reversing IL2CPP functions with IDA is pure pain dude

[2025-10-24 21:37] the horse: i've rebuilt a lot of il2cpp funcs

[2025-10-24 21:37] the horse: once you cut the shitty abstractions

[2025-10-24 21:37] the horse: it's not that bad

[2025-10-24 21:38] selfprxvoked: Last time I tried reversing Genshin Impact I straight up gave up ngl

[2025-10-24 21:40] selfprxvoked: I don't know if it was extremely exhausting to reverse it because of my lack of skill with C# or if IL2CPP itself is very exhausting

[2025-10-24 21:40] the horse: a lot of the utils you need are unity engine anyway

[2025-10-24 21:42] selfprxvoked: yes, thats the only easy part at least

[2025-10-24 21:43] selfprxvoked: another thing that makes it a little less exhaustive to reverse is that you can extract the names, unless it is obfuscated ofc

[2025-10-24 21:45] selfprxvoked: [replying to Shiro: "and then some of the classes have only like half o..."]
it looks like the function parameter types names are obfuscated not the half of the functions names?

[2025-10-24 21:45] the horse: I've dealt with a lot of of games with obfuscated fields / values / classes, mostly the obfuscators themselves are really weak ¯\_(ツ)_/¯

[2025-10-24 21:45] the horse: extremely generic encryption routines and substitutions

[2025-10-24 21:45] selfprxvoked: GI one is the best one in the market

[2025-10-24 21:46] selfprxvoked: ever tried it?

[2025-10-24 21:46] the horse: Gl?

[2025-10-24 21:46] selfprxvoked: Genshin Impact

[2025-10-24 21:46] the horse: ah, I haven't looked at that one specifically, no

[2025-10-24 21:46] the horse: what makes it different?

[2025-10-24 21:48] Shiro: [replying to selfprxvoked: "it looks like the function parameter types names a..."]
yeah its kinda weird half of the methods in that class are obfuscated and half aren't
[Attachments: image.png]

[2025-10-24 21:48] selfprxvoked: [replying to the horse: "what makes it different?"]
at least for me, it was harder to deal with compared to other obfuscators I've seen around

[2025-10-24 21:48] selfprxvoked: but don't take it from me tho, I don't have too much experience dealing with C# obfuscators

[2025-10-24 21:48] the horse: is genshin il2cpp or C#?

[2025-10-24 21:48] selfprxvoked: IL2CPP

[2025-10-24 21:49] the horse: what obfuscation does it employ?

[2025-10-24 21:50] the horse: I would heavily suggest investigating constructors - usually the obfuscators translate very poorely in this regard

[2025-10-24 21:51] the horse: internal cheats are definitely harder to accomplish, if the function count is heavily expanded

[2025-10-24 21:51] selfprxvoked: classes, methods, fields names are obfuscated, il2cpp metadata is encrypted and the exported functions that retrieve metadata are inlined and obfuscated

[2025-10-24 21:51] the horse: for performance reason, re-ordering of fields is often not implemented (which .ctors can expose)

[2025-10-24 21:52] the horse: [replying to the horse: "for performance reason, re-ordering of fields is o..."]
this allows you to extract fields in original order, if compared to a previous unobfuscated or reversed sample

[2025-10-24 21:52] the horse: the mutations on function getters/setters is usually exhaustive (3-8 variations) -- all of which you can dynamically resolve

[2025-10-24 21:52] selfprxvoked: [replying to the horse: "I would heavily suggest investigating constructors..."]
I won't touch anything from Mihoyo ever again tbh

[2025-10-24 21:53] the horse: [replying to the horse: "the mutations on function getters/setters is usual..."]
basically, just lift it:
```rs
            if i + 2 < end_idx
                && instr.mnemonic() == Mnemonic::Shr
                && instructions[i + 1].mnemonic() == Mnemonic::Lea
                && instructions[i + 2].mnemonic() == Mnemonic::Or
            {
                let shr_reg = instr.op0_register();
                let shr_amt = instr.immediate(1) as u32;
                let lea_instr = &instructions[i + 1];
                let or_reg = instructions[i + 2].op0_register();
                let lea_dst_reg = lea_instr.op0_register();
                let lea_op1_kind = lea_instr.op1_kind();
                let lea_scale = lea_instr.memory_index_scale();

                if lea_op1_kind == OpKind::Memory
                    && lea_instr.memory_base() == Register::None
                    && lea_instr.memory_index() != Register::None
                    && or_reg == shr_reg
                    && lea_dst_reg != shr_reg
                    && (shr_reg.full_register32() == value_reg || value_reg == Register::None)
                {
                    let shl_amt = match lea_scale {
                        2 => 1u32,
                        4 => 2u32,
                        8 => 3u32,
                        16 => 4u32,
                        _ => 0u32,
                    };
                    if shl_amt > 0 && shr_amt + shl_amt == 32 {
                        operations.push(Operation::Rotr(shr_amt));
                        pattern_desc.push(format!("rotr by {}", shr_amt));
                        value_reg = or_reg.full_register32();
                        i += 3;
                        continue;
                    }
                }
            }```

[2025-10-24 21:56] selfprxvoked: [replying to Shiro: "yeah its kinda weird half of the methods in that c..."]
oh I got it now lol

[2025-10-24 23:28] Gestalt: https://tenor.com/view/mods-cut-him-down-tear-him-apart-splay-the-gore-gif-16211356125689318290

[2025-10-25 02:43] guar: [replying to Shiro: "yeah its kinda weird half of the methods in that c..."]
this looks like a standard practice where most of c# obfuscators will rename private methods+classes but leave public ones with original names <:Shruge:1048387309080416347>

[2025-10-25 03:53] Novoline: [replying to guar: "this looks like a standard practice where most of ..."]
Very simple.

[2025-10-25 03:53] Novoline: Not like DNGuard.

[2025-10-25 03:53] Novoline: Send the the dotnet application.

[2025-10-25 07:18] fexsped: how does disabling patchguard work?

[2025-10-25 07:19] fexsped: Do the patches need constant maintenance because microsoft keeps changing it to make it harder to disable?

[2025-10-25 10:34] Aʜᴍᴇᴅ「aka ɪLᴏᴠᴇBɪᴛs」🕵: https://www.linkedin.com/posts/ilovebits_malwareanalysis-malwareresearch-reverseengineering-activity-7387620277918089216-Oi20
[Embed: #malwareanalysis #malwareresearch #reverseengineering #threatintell...]
Hi everyone !!

When analyzing 𝗔𝘂𝗥𝗔𝗧 𝗠𝗮𝗹𝘄𝗮𝗿𝗲 I discoverd that contains 𝗗𝘆𝗻𝗮𝗺𝗶𝗰 𝗔𝗣𝗜 𝗥𝗲𝘀𝗼𝗹𝘂𝘁𝗶𝗼𝗻 𝗢𝗯𝗳𝘂𝘀𝗰𝗮𝘁𝗶𝗼𝗻

I have written a static deobfuscator for AuRAT Malware to make static analysis easier.

𝗧𝗵𝗶𝘀 

[2025-10-25 14:37] Rairii: [replying to fexsped: "how does disabling patchguard work?"]
from a bootkit iirc it's a single patch, because it gets disabled anyway when booting with kernel debugging enabled

[2025-10-25 14:41] Brit: [replying to fexsped: "how does disabling patchguard work?"]
many different strategies, you can look at byepg if you wish

[2025-10-25 15:07] Novoline: [replying to fexsped: "Do the patches need constant maintenance because m..."]
Most methods have gotten patched in 24H2 and 25H2 but there are some working runtime ones https://github.com/NeoMaster831/kurasagi https://blog.can.ac/2024/06/28/pgc-garbage-collecting-patchguard/ https://shhoya.github.io/windows_pgintro.html https://blog.tetrane.com/downloads/Tetrane_PatchGuard_Analysis_RS4_v1.01.pdf
[Embed: GitHub - NeoMaster831/kurasagi: Windows 11 24H2-25H2 Runtime PatchG...]
Windows 11 24H2-25H2 Runtime PatchGuard Bypass. Contribute to NeoMaster831/kurasagi development by creating an account on GitHub.
[Embed: PgC: Garbage collecting Patchguard away]
<p>I have released another article about Patchguard almost 5 years ago, ByePg, which was about exception hooking in the kernel, but let’s be frank, it didn’t entirely get rid of Patchguard
[Embed: PatchGuard Introduction | Shh0ya Security Lab]
Windows KPP Introduction

[2025-10-25 16:08] expy: hey guys, how would one include those in kernel mode code? It's mentioned in "PgC: Garbage collecting Patchguard away" article by Can https://blog.can.ac/2024/06/28/pgc-garbage-collecting-patchguard/, and the piece of related code https://github.com/can1357/xstd/blob/master/includes/ia32/memory.hpp references ia32.hpp https://github.com/can1357/xstd/blob/master/includes/ia32.hpp
```
#include <cstdint>
#include <tuple>
#include <array>
```
😕

[2025-10-25 16:46] Shiro: [replying to guar: "this looks like a standard practice where most of ..."]
Idk like I said im new to RE

[2025-10-25 20:05] plpg: [replying to expy: "hey guys, how would one include those in kernel mo..."]
Arent these standard c++ header files?

[2025-10-25 20:19] Xits: [replying to expy: "hey guys, how would one include those in kernel mo..."]
Can mentioned that some parts of that xstd library have a certain license and it’s not public or something

[2025-10-25 20:19] Xits: That could be why but those do look like std c++ headers which wouldn’t work in km

[2025-10-25 20:22] Brit: [replying to plpg: "Arent these standard c++ header files?"]
they are

[2025-10-25 20:58] UJ: [replying to expy: "hey guys, how would one include those in kernel mo..."]
should be fine. array/tuple are simple compile time size known  types that should be  freestanding/kernel compatible. some functions don't work but they are clearly documented https://en.cppreference.com/w/cpp/header/array.html

[2025-10-26 02:20] expy: [replying to plpg: "Arent these standard c++ header files?"]
sure, but trying to include those in a driver would lead to a lot of errors

[2025-10-26 04:00] UJ: [replying to UJ: "should be fine. array/tuple are simple compile tim..."]
i just tried this out and yeah, it was wrong of me to assume msvc would work as expected ~~with freestanding c++ headers in kernel drivers. ~~

array was only made freestanding in c++23 as well so not listed here https://learn.microsoft.com/en-us/cpp/standard-library/cpp-standard-library-overview?view=msvc-170 but in either case if something is freestanding, it still won't work with the WDK headers until [this](https://github.com/microsoft/STL/issues/4208) issue is resolved [here](https://developercommunity.visualstudio.com/t/fix-msvc-and-wdk-headers-so-that-important-c-use-c/1318711)

rust having std and no_std semantics up front was a good move.

[2025-10-26 23:07] guar: <:Madeg:883865452500099102> please help
is there a way in x64dbg to continuously edit memory?
e.g. inputting several bytes at once, kinda like a hex edit in imhex?
memset command only accepts 1 byte, modify value in dump seems to only let you edit a sizeof (displayed type) which is at most 8 byte for hex long long (and requires to actually input desired values backwards due to endianness)
[Attachments: imhex-gui_9KZS9NPcVL.mp4]

[2025-10-26 23:18] guar: <:PepegaPhone:830288576150765608>  rclick -> binary -> paste ignore size