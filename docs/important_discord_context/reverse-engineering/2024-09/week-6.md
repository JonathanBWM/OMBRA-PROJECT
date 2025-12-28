# September 2024 - Week 6
# Channel: #reverse-engineering
# Messages: 118

[2024-09-30 09:07] 0xboga: [replying to 0xboga: "Question about hooking data pointers, say I map a ..."]
Anyone?

[2024-09-30 11:07] ether: [replying to 0xboga: "Anyone?"]
https://tenor.com/view/miracle-musical-labyrinth-peter-griffin-peter-meme-gif-26691042

[2024-09-30 11:38] Brit: [replying to 0xboga: "Question about hooking data pointers, say I map a ..."]
why do indirect calls also not trip cfg checks <:mmmm:904523247205351454>

[2024-09-30 12:14] daax: [replying to 0xboga: "Question about hooking data pointers, say I map a ..."]
you’re testing this with vbs on or off? if vbs is off, then afaik that makes sense because kcfg will only check the va and doesnt do any of the other checks but the dispatch fncs are still in the code and just act as a passthrough.

[2024-09-30 12:37] MonTy: obfuscated code often uses try...catch exception handlers. If it is necessary to force the code to be called from the catch block, how do I find out the address of the code in this block?

[2024-09-30 13:06] 0xboga: [replying to daax: "you’re testing this with vbs on or off? if vbs is ..."]
VBS is off yes
what type of check does it do on the VA? certain range? just be it a kernel address ?

[2024-09-30 13:20] expy: ~~hello, any clue what P1Home...P6Home used for in _CONTEXT structure?~~
UPD: found some clues in WDK, it's used to pass parameters to apc

```c++
        ContextRecord->P1Home = (ULONG64)NormalContext;
        ContextRecord->P2Home = (ULONG64)SystemArgument1;
        ContextRecord->P3Home = (ULONG64)SystemArgument2;
        ContextRecord->P4Home = (ULONG64)NormalRoutine;
```

[2024-09-30 14:21] Matti: [replying to 0xboga: "VBS is off yes
what type of check does it do on th..."]
if we assume KCFG works like user mode CFG...
and we assume KCFG wasn't just completely rewritten for 24Hwhatever...
then it should store something like a bitmap of valid call targets

[2024-09-30 14:22] Matti: https://ynwarcs.github.io/Win11-24H2-CFG FYI
[Embed: CFG in Windows 11 24H2]
Hotpatching has been looming over Windows 11 for a while now, having already been shipped on the server & cloud deployments. It first came out in March that the first major version to include it will 

[2024-09-30 14:22] Matti: I haven't read this far enough yet, I fell asleep the first time trying

[2024-09-30 14:23] Matti: all this security mitigation shit is annoying AND boring

[2024-09-30 14:25] Matti: so, after a super quick skim it seems like the basic answer is still the same: there's a 'valid jmp/call targets' bitmap

[2024-09-30 14:26] Matti: except with extensive changes to be able to support hotpatching

[2024-09-30 15:04] 0xboga: [replying to Matti: "so, after a super quick skim it seems like the bas..."]
Yes that is what I assumed and that’s why I asked the question in the first place. I assumed KCFG maintains a bitmap of what would be considered valid addresses, and that’s why by replacing the pointer of a function call that goes through the guard dispatch thing it should fail, yet it hasn’t failed…

[2024-09-30 15:47] expy: I think someone here offered idb for windows kernel? could you please share that ~~currently searching what is the second parameter to RtlpCopyExtendedContext (it's close to EXCEPTION_POINTERS on stack, +0x20 offset)~~ upd: nvm that

[2024-09-30 16:17] catnip: Anyone doing the flareons? Im getting absolutely rect by the very last step of #5

[2024-09-30 16:23] Rairii: https://archive.org/details/10.0.14361.1000.rs1_release_prs.160603-2123_x86PlusPrivateSyms
https://archive.org/details/10.0.14910.1001.rs_prerelease.160819-1700_ARM64PrivateSyms
[Embed: Windows 10 v1607, build 14361.1000 (RS1_RELEASE_PRS) x86 + private ...]
Windows 10 Anniversary Update build 14361.1000; Redstone 1 pre-release signing branch. Comes under Core and Professional SKUs; includes private symbols for...
[Embed: Windows 10 v1703, build 14910.1001 (RS_PRERELEASE) ARM64-specific p...]
Private debugging symbols for Windows 10, version 1703 build 14910.1001 (RS_PRERELEASE) under the ARM64 architecture.Build tag: 10.0.14910.1001...

[2024-09-30 16:59] Matti: [replying to 0xboga: "Yes that is what I assumed and that’s why I asked ..."]
like <@609487237331288074> said, if you have VBS disabled then KCFG is (at best) in audit mode

[2024-09-30 17:00] Matti: in which a violation is probably logged to wherever, but transparent to the user

[2024-09-30 17:01] Matti: just c/p'ing from my lazy notes:
> Found in winload. These are all undocumented
> 
> To disable user shadow stacks:
>     Create DWORD HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\kernel\UserShadowStacksForceDisabled = 1
> 
> To enable kernel shadow stacks:
>     Set HKLM\SYSTEM\CurrentControlSet\Control\DeviceGuard\Scenarios\HypervisorEnforcedCodeIntegrity\Enabled = 1 (don't know why this is required, but it really is!)
> 
>     Create key HKLM\SYSTEM\CurrentControlSet\Control\DeviceGuard\Scenarios\KernelShadowStacks
>         Create DWORD HKLM\SYSTEM\CurrentControlSet\Control\DeviceGuard\Scenarios\KernelShadowStacks\Enabled = 1
>         Create DWORD HKLM\SYSTEM\CurrentControlSet\Control\DeviceGuard\Scenarios\KernelShadowStacks\AuditModeEnabled = 0 or 1

[2024-09-30 17:02] Matti: this is about the SS part of CET, not CFG, but I'm pretty sure the KCFG has an explicit opt-in like this too

[2024-09-30 17:03] Matti: as in, even enabling VBS isn't necessarily enough to guarantee that it is enabled

[2024-09-30 17:03] Matti: I don't have the undocumented flag for KCFG on hand right now sorry, but it has got to be in either winload or ntoskrnl if it exists

[2024-09-30 17:04] Matti: [replying to Rairii: "https://archive.org/details/10.0.14361.1000.rs1_re..."]
oh shit, thanks

[2024-09-30 17:04] Matti: I had the most important ones (for me) from these builds, but I had no idea they shipped an entire windows build with private PDBs

[2024-09-30 17:09] 0xboga: [replying to Matti: "like <@609487237331288074> said, if you have VBS d..."]
Makes sense I guess

[2024-09-30 17:13] Rairii: [replying to Matti: "I had the most important ones (for me) from these ..."]
it's not 100% private pdbs, some symbols are public, i know kernelbase.dll is public for instance

[2024-09-30 17:13] Rairii: and udwm is also public, but a lot of the symbols are private

[2024-09-30 17:13] Matti: alright, still good

[2024-09-30 17:14] Matti: I have a shitty batch script to find private PDBs automatically if you want

[2024-09-30 17:14] Matti: in a given dir

[2024-09-30 17:14] Matti: 
[Attachments: find_private_pdbs.cmd]

[2024-09-30 17:14] Matti: it's a bit verbose since it prints even for public PDBs, so you may need to tone it down a little

[2024-09-30 17:16] Matti: bit sad they aren't checked builds \:(

[2024-09-30 17:16] Matti: imagine that... checked windows with private PDBs

[2024-09-30 17:49] Torph: [replying to Rairii: "https://archive.org/details/10.0.14361.1000.rs1_re..."]
woah cool

[2024-09-30 17:59] vendor: [replying to Matti: "it's a bit verbose since it prints even for public..."]
does it contain hyper-v pdb? would download myself and check but won’t be at pc for a while.

[2024-09-30 18:00] vendor: think i have a really old intel one and a more recent arm but never seen amd leaked

[2024-09-30 18:00] Matti: they won't, these are x85 32 bit and arm64

[2024-09-30 18:01] Matti: or does 32 bit have a hypervisor (hvax64.exe) for some reason?

[2024-09-30 18:01] Matti: that seems like a huge waste of development time but maybe

[2024-09-30 18:02] Matti: wait uhhh

[2024-09-30 18:03] Matti: so the "x86" ISO is actually named (read URL)
<https://archive.org/download/10.0.14361.1000.rs1_release_prs.160603-2123_x86PlusPrivateSyms/14361.1000.160603_2123.rs1_release_prs_amd64fre_Windows_Private_Symbols_x86fre.iso>

[2024-09-30 18:03] Matti: so it might contain both

[2024-09-30 18:03] Matti: I'll check, one moment

[2024-09-30 18:05] Matti: ok the 'amd64fre' part is bait at least

[2024-09-30 18:05] Matti: it is 32 bit x86

[2024-09-30 18:06] Matti: no hv*x[32].exe that I can see so far

[2024-09-30 18:07] Matti: does such a thing even exist?

[2024-09-30 18:07] Matti: the exe I mean

[2024-09-30 18:08] Matti: I've never even considered the possibility of something like that existing for 32 bit x86, I'd say it's pretty unlikely

[2024-09-30 18:08] Matti: but if it does, I don't know the filename

[2024-09-30 18:09] Matti: amd64: hvax64.exe
Intel(R) EM64T(TM): hvix64.exe
ARM64: hvaa64.exe
x86 32 bit: ???.exe

[2024-09-30 18:12] vendor: [replying to Matti: "they won't, these are x85 32 bit and arm64"]
oh shit didn’t realise that. i thought they stopped making native 32 bit builds at win10?

[2024-09-30 18:13] Matti: they do still make these internally, and for windows 11 too

[2024-09-30 18:13] Matti: it'd be stupid not to, cause you can catch portability bugs very easily this way

[2024-09-30 18:13] Matti: but they don't release or support these

[2024-09-30 18:13] Matti: or admit that they do exist internally

[2024-09-30 18:14] Matti: I do the same in my WRK actually

[2024-09-30 18:15] Matti: 32 bit in 2024 is retarded, but it's been helpful in finding bugs in changes I made that would've actually unintentionally introduced a regression

[2024-09-30 18:15] Matti: same for LLVM, though to a much larger degree of course

[2024-09-30 18:20] Matti: well there may not be hvax64.exe but some of this is definitely not useless

[2024-09-30 18:20] Matti: 
[Attachments: image.png]

[2024-09-30 18:21] Matti: these are simply all PDBs containing "hv" in the name lol

[2024-09-30 18:21] Matti: anyone know what the fuck xpsrchvw.[dll|exe] is for? it PDB is huge

[2024-09-30 18:22] Matti: shvl.pdb is the only one in this dir that is NOT private, FYI

[2024-09-30 18:26] Matti: <@519952679657668608>
[Attachments: its_32_bit_shit_but_you_might_still_like_it.7z]

[2024-09-30 18:26] diversenok: [replying to Matti: "these are simply all PDBs containing "hv" in the n..."]
The *.xps file viewer

[2024-09-30 18:26] diversenok: Ops, wrong quote

[2024-09-30 18:26] Matti: cooooool

[2024-09-30 18:27] Matti: I don't mind `kdhv1394.pdb` and `winhv.pdb` though

[2024-09-30 18:27] Matti: though likely the ARM64 versions (assuming they exist there too) are gonna be more useful

[2024-09-30 18:31] Matti: [replying to diversenok: "The *.xps file viewer"]
these kinds of PDBs are annoying

[2024-09-30 18:31] Matti: like combase.pdb or wintypes.pdb

[2024-09-30 18:32] Matti: 90% is garbage, or even C++

[2024-09-30 18:32] Matti: but there are also definitely private kernel types in this XPS viewer pdb

[2024-09-30 18:32] Matti: I don't even know how that happens but it does

[2024-09-30 18:33] diversenok: Pre-compiled headers maybe?

[2024-09-30 18:33] diversenok: XAML pdbs are even bigger

[2024-09-30 18:34] diversenok: 200+ MB

[2024-09-30 18:34] Matti: yeah, I was about to post a screenshot showing the biggest pdbs by size but then realised most of these are for UWP shit, same as the public PDBs

[2024-09-30 18:35] Matti: I'll modify my batch file above a bit to aggressively purify public PDBs

[2024-09-30 18:42] diversenok: And the record goes to... `msedge.dll.pdb` with 473 MB of stuff

[2024-09-30 18:43] diversenok: Not sure which OS version is it from though, just found it in my symbol cache

[2024-09-30 18:43] Matti: yeah I also tried this years ago and also arrived at msedge.dll <:kekw:904522300257345566>

[2024-09-30 18:45] Matti: the pdb for it is so huge that someone in SC wrote an aria2c-accelerated pdb downloader out of frustration with the 1 MB/s cap

[2024-09-30 18:45] Matti: well probably not *that* pdb

[2024-09-30 18:45] Matti: but an interesting one that was sufficiently large

[2024-09-30 18:46] Matti: oh he's in here: found you <@276405047024418826>

[2024-09-30 18:47] MonTy: Guys, how can I track exceptions and their handlers in a running program?

[2024-09-30 18:48] vendor: [replying to Matti: "<@519952679657668608>"]
thanks

[2024-09-30 18:48] MonTy: as I understand it, the RtIDispatchException function is responsible for exception handling

[2024-09-30 19:13] expy: [replying to MonTy: "as I understand it, the RtIDispatchException funct..."]
hook that and NtContinue

[2024-09-30 19:14] Matti: [replying to diversenok: "And the record goes to... `msedge.dll.pdb` with 47..."]
found a new record holder: the PDB for matti-llvm
[Attachments: image.png]

[2024-09-30 19:14] Matti: although, it's possible the PDBs for matti-UT are even bigger but I don't cache those in C:\Symbols

[2024-09-30 19:15] Matti: due to not having infinity space

[2024-09-30 19:15] Matti: uhh yeah they are
[Attachments: image.png]

[2024-09-30 19:15] diversenok: This bad boy can fit so many private symbols

[2024-09-30 19:18] MonTy: [replying to expy: "hook that and NtContinue"]
NtContinue - is it used to continue the flow, in particular to handle an exception?

[2024-09-30 19:19] Matti: [replying to diversenok: "This bad boy can fit so many private symbols"]
you know what's funny

[2024-09-30 19:19] Matti: the private PDB for matti WRK is the same size as the kernel exe for win 11

[2024-09-30 19:19] Matti: 12 MB

[2024-09-30 19:20] Matti: the exe itself is 2.99

[2024-09-30 19:20] Matti: or 3.12 when compiled with LLVM

[2024-09-30 19:21] Matti: microsoft's """kernel""" loses again!

[2024-09-30 20:19] koyz: [replying to Matti: "due to not having infinity space"]
Not very Matti of you

[2024-09-30 20:25] Matti: I'm aware :/

[2024-09-30 20:25] Matti: I'm too poor to afford infinity

[2024-09-30 20:25] Matti: so I can only buy little bits of it

[2024-09-30 21:35] Rairii: i think all the actually private symbols have infoleak in the headers lol

[2024-09-30 21:35] Rairii: if you look you can see environment variables from the build machiens etc

[2024-09-30 22:21] Matti: that's normal

[2024-09-30 22:22] Matti: a lot of public PDBs have this too in fact

[2024-09-30 22:22] Matti: or uh no, I think probably .obj files in the SDK do

[2024-09-30 22:22] Matti: not sure about public PDBs but they may well do

[2024-09-30 22:23] Matti: but the WDK for example contains lots of .obj files in the form of static libs which you can just extract with 7zip

[2024-09-30 22:25] Matti: ya ok, public PDBs do too
[Attachments: image.png]

[2024-09-30 22:25] Matti: (grepped this flag because I know MS use it on everything they compile for kernel)