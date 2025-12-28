# October 2024 - Week 2
# Channel: #reverse-engineering
# Messages: 340

[2024-10-07 10:39] hxm: For what reason icicle may be better than unicorn ?  <@162611465130475520> (ignoring the rust fanbase)

[2024-10-07 10:42] mrexodia: [replying to hxm: "For what reason icicle may be better than unicorn ..."]
unicorn is full of semantic bugs and because of the way the project was 'forked' from qemu there is no way to keep them up to date with upstream improvements

[2024-10-07 10:42] mrexodia: icicle is based on ghidra's pcode, so it will be improved over time by the community and NSA

[2024-10-07 10:42] mrexodia: and if someone adds a new architecture to ghidra is relatively easy to get an emulator for it it incicle

[2024-10-07 10:44] hxm: I only wish if we had some c++ port of it

[2024-10-07 10:45] hxm: compiling the lib with rust and generating a header file is a bit arse

[2024-10-07 10:50] hxm: https://github.com/mozilla/cbindgen
[Embed: GitHub - mozilla/cbindgen: A project for generating C bindings from...]
A project for generating C bindings from Rust code - mozilla/cbindgen

[2024-10-07 11:08] Brit: [replying to hxm: "I only wish if we had some c++ port of it"]
or you could just learn rust, it's really not that complicated

[2024-10-07 11:31] mrexodia: [replying to hxm: "compiling the lib with rust and generating a heade..."]
I'll publish C bindings at some point

[2024-10-07 11:48] hxm: [replying to Brit: "or you could just learn rust, it's really not that..."]
Problem is about compatibility i cant turn my c++ project into rust just for the emu part. Or i ll just male a selfcontained tool for that part 🥲

[2024-10-07 11:54] Brit: then it's bindings glue for you

[2024-10-07 12:42] Possum: ```

0000166b          int32_t temp0_1
0000166b          int32_t temp1_1
0000166b          temp0_1:temp1_1 = sx.q(i_3)
0000166c          uint32_t rdx_3 = temp0_1 u>> 0x1f
00001674          int32_t rax_29 = ((temp1_1 + rdx_3) & 1) - rdx_3

```

Why does binary ninja make modulo operations look like this instead if a more strait forward pseudocode?

[2024-10-07 12:52] Redhpm: that's just how it was compiled

[2024-10-07 12:52] Redhpm: binja can try to find the high level operation again, just maybe not at that level

[2024-10-07 12:56] 25pwn: what's the password for `signatures-bundles-9.0-beta.zip`?

[2024-10-07 13:01] Possum: [replying to Redhpm: "binja can try to find the high level operation aga..."]
Its just odd.  Half the time mod operations show up 100% normal, the other like this

[2024-10-07 13:04] Leeky: [replying to Possum: "Its just odd.  Half the time mod operations show u..."]
it's actually an interesting problem similar to some division optimizations, where reconstructing the original operation without using heuristics isn't easy. I think IDA uses (or at least for a long time used) heuristics for some of the patterns (which has the funny consequence that you can get wrong decomp when you abuse this or get unlucky with weird bitvec manipulation logic)

[2024-10-07 13:07] Leeky: it's been a while but e.g. p4ctf finals 2023 had a beginner rev challenge where exactly this screwed up a lot of people - in binary ninja you got this shifting stuff which is correct but pretty unreadable, where as IDA gave you the modulo - but the modulo was entirely wrong

[2024-10-07 13:07] Leeky: I do think Ghidra got it right though

[2024-10-07 13:08] Possum: Yeah, ghidra was closer but still not right on.  Angr was painfully wrong.  I had to use chatgpt to figure out what it was lol.

[2024-10-07 13:09] Leeky: what I usually do is just reimplement the shifting logic in python and try it out for a few values with the knowledge that this stuff is usually either division, modulo, or absolute

[2024-10-07 13:10] Leeky: using chatgpt for this is honestly also fair, especially when it's easy to confirm whether that is right or not

[2024-10-07 13:10] Leeky: but yes tl;dr reconstruction of this without heuristics is a difficult problem

[2024-10-07 13:11] Possum: Atleast its consistently that way so now i can recognize it and guess values based off what the if statements are

[2024-10-07 17:12] Matti: [replying to 25pwn: "what's the password for `signatures-bundles-9.0-be..."]
honest answer: I don't know
more helpful answer hopefully: none of the files in that zip are interesting in any way, as you can tell from their filenames + sizes which are still... mediocre at best

I'm just basing this on `Microsoft.CRT-Desktop_x64.sig` being 573 KB, vs my (presumably roughly similar in terms of contents) `vcruntime143_64.sig`which is 860 KB

additionally, no kernel sigs and no corresonding TILs means this entire zip is pretty much worthless
no idea why it was passworded for beta 2

[2024-10-07 17:13] Matti: I recommend using sigmake to generate .sig files yourself instead, and then idaclang to generate the TIL files to go with them

[2024-10-07 21:08] ShekelMerchant: Are these pages not up yet? When I click on em I get redirected to the main page. Kinda disapointed lmao
[Attachments: image.png]

[2024-10-08 00:55] szczcur: [replying to ShekelMerchant: "Are these pages not up yet? When I click on em I g..."]
i dont believe so.

[2024-10-08 01:58] rin: noob question. In dumpbin when you get exports what does the question mark and @@ sign mean.

[2024-10-08 01:58] rin: example:` 8    0 000356F0 ??0CVssJetWriter@@QEAA@XZ`.

[2024-10-08 02:15] Matti: [replying to rin: "noob question. In dumpbin when you get exports wha..."]
what kind of 'what does this mean' are you looking for

in general this is C++ name mangling, MSVC name mangling specifically
if you wanna know "what does this specific sequence of characeters mean" - use `undname`:
```
Undecoration of :- "??0CVssJetWriter@@QEAA@XZ"
is :- "public: __cdecl CVssJetWriter::CVssJetWriter(void) __ptr64"
```
if you mean what do they **mean**.... well that is a good fucking question I don't know the answer to

[2024-10-08 02:16] Matti: LLVM sources no doubt will be able to help if (3) is your question

[2024-10-08 02:45] rin: [replying to Matti: "what kind of 'what does this mean' are you looking..."]
thank you, and sorry for the dumb question. I'm not a c++ person.

[2024-10-08 02:49] Matti: not a dumb question <:thinknow:475800595110821888>

[2024-10-08 02:49] Matti: but there's various ways to interpret it

[2024-10-08 02:51] Matti: things like access modifiers, `class`, `static`, ... don't exist in traditional `extern "C"` declarations (well static does - but the meaning is often very different)

[2024-10-08 02:53] Matti: so every compiler vendor invented their own way to combine as many characters as possible to still be able to represent C++ symbols in names

[2024-10-08 02:54] Matti: just another way in which C++ has improved the world

[2024-10-08 03:07] rin: ok I read a bit and it makes sense. the names are very unfortunate though because It makes it impossible to make a proxy dll in certain languages.

[2024-10-08 03:08] Matti: YMMV but I don't often see C++ (meaning mangled) export names in the wild

[2024-10-08 03:10] Matti: `extern "C"` is the standard way to name symbols that are meant to be used for FFIs

[2024-10-08 03:10] Matti: if* it's a C++ library meant to be consumed by C++ programs, then you'll seometimes see C++ names

[2024-10-08 04:59] Can: Gnu::alias

[2024-10-08 05:20] rin: [replying to Can: "Gnu::alias"]
?

[2024-10-08 08:29] 6bd835a1d0095059128d4d8cf6d16171: [replying to 25pwn: "what's the password for `signatures-bundles-9.0-be..."]
idk but the voices tell me bkcrack works

[2024-10-08 08:34] 6bd835a1d0095059128d4d8cf6d16171: `bkcrack -C signatures-bundles-9.0-beta.zip -k 7d396e83 b1c59666 08ae3e65 -D signatures-bundles-9.0-beta_nopass.zip`

[2024-10-08 09:45] MonTy: I'm debugging another obfuscated driver and ran into such an instruction
[Attachments: image.png]

[2024-10-08 09:46] MonTy: why should the driver receive this address?

[2024-10-08 10:34] Timmy: debugging driver with x64dbg? what?

[2024-10-08 10:37] MonTy: [replying to Timmy: "debugging driver with x64dbg? what?"]
There is a very cool dude mrexodia

[2024-10-08 10:37] MonTy: https://x64dbg.com/blog/2017/06/08/kernel-driver-unpacking.html
[Embed: Kernel driver unpacking · x64dbg]
Official x64dbg blog!

[2024-10-08 10:39] Timmy: ahh look at that, missed this

[2024-10-08 13:39] x86matthew: i debugged UEFI firmware with x64dbg last week

[2024-10-08 13:40] x86matthew: in a similar manner

[2024-10-08 13:48] Matti: you can also use NtCreateUserProcess to launch any arbitrary PE and debug that

[2024-10-08 13:48] Matti: this is how I've.... debugged.... UEFI in the past before discovering I prefer qemu for this

[2024-10-08 13:53] Matti: oh yeah... tiny DIY writeup from 2017 I posted at the time it seems <https://github.com/x64dbg/x64dbg/issues/1656>

[2024-10-08 16:43] mrexodia: Mattipoast <:give_heart_FB:675231233491337220>

[2024-10-08 16:48] luci4: Mattipoast <:isaac_lots_of_love:971191364517785640>

[2024-10-08 22:04] Shanks: Hi everyone, can anyone here reverse engineer Windows XP MSN games like Escargot did with Messenger Live? Note that the source code was leaked with Windows XP SP1

[2024-10-08 22:06] Shanks: 
[Attachments: 1556565974.or.27408.jpg]

[2024-10-08 23:49] Matti: so you mean compile?

[2024-10-08 23:50] Matti: 1. download torrent or other source for leak
2. hit "compile"

[2024-10-08 23:51] Brit: my favorite re situation

[2024-10-08 23:52] Brit: source available

[2024-10-08 23:52] Matti: same

[2024-10-08 23:52] Matti: I just wish they'd hurry up with the win 10 release

[2024-10-08 23:57] Matti: oh shit this reminds me

[2024-10-09 00:02] Matti: [replying to Rairii: "https://archive.org/details/10.0.14361.1000.rs1_re..."]
tyvm for these - I was able to get a pretty good amount of private PDBs out of both ISOs, tough sadly only the X86 version seems to come with an ISO with matching binaries, from what I've been able to find

X86: 7816 private PDB files, plus the ISO with matching binaries
ARM64: 5941 private PDB files, plus the closest matching ISO I could find version number wise (tested on ntoskrnl, success rate is ~60-70% I would guess)

public PDBs were deleted

all of the PDB files also contain types, those I added in hopefully mostly parseable C header format next to each PDB file

[2024-10-09 00:02] Matti: X86: https://mega.nz/file/GBglWChR#T0MoYMtq5b6A4_bafvv9IMxojYo9SDVdmy0u0j8tSLc
[Embed: 7.62 GB file on MEGA]

[2024-10-09 00:03] Matti: ARM64: https://mega.nz/file/2V53jaIC#rsCWLv3BWOCAVuGUzKiBxZNQCHEhVjjQ6kbcCDIZcrY
[Embed: 6.41 GB file on MEGA]

[2024-10-09 00:04] Matti: also note that there are a small number (but quite interesting) private PDBs from the **replacement** ISO I used to get at east somewhat similar binaries

these weren't new to me at least but still funny they just stuck them in the DVD root

[2024-10-09 00:08] Matti: mm oh yeah, I made one exception to the "private PDBs" only rule and that's for the arm64 hv launcher executable

[2024-10-09 00:09] Matti: `hvaa64.exe`

[2024-10-09 00:09] Matti: better than nothing, right

[2024-10-09 00:13] Matti: tree listings of each directory
[Attachments: tree-102-more-ARM64-PDBs.txt, tree-5951-ARM64-PDBs.txt, tree-7816-X86-PDBs.txt]

[2024-10-09 21:12] tifkin: The ARM64 also appears to contain several Windows Server roles (DCs, ADCS, DHCP, WSUS, etc)

[2024-10-09 21:15] Matti: do you mean the PDBs or the ISO? either is possible, but I know the ISO happens to be a server one

[2024-10-09 21:16] Matti: if the server specific files have matches in the PDBs, then that confirms they both are

[2024-10-09 21:17] tifkin: The PDBs. Just interesting since the ISOs may not have all the files due to the roles not being installed

[2024-10-09 21:17] tifkin: (I haven't looked at your links, but just through my own exploration the other day when I went spelunking through them)

[2024-10-09 21:17] Matti: ah, yeah I see what you mean

[2024-10-09 21:19] Matti: I'm mostly interested in kernel stuff myself of course, but other unexpected things I saw were mpengine.pdb (100 MB for X86, 50 MB for ARM64), which is apparently the DLL that powers the defender shitware on the user mode side

[2024-10-09 21:19] Matti: and PDBs for various MSVC compiler components of unknown versions

[2024-10-09 21:20] Matti: only saw those in the ARM64 PDBs, didn't check X86 since.... well does anyone care

[2024-10-09 21:22] Matti: [replying to Matti: "and PDBs for various MSVC compiler components of u..."]

[Attachments: c1_c2_cl_link_mspdbcore.7z]

[2024-10-09 21:24] Matti: obtaining the "real" (matching) PE files for the ARM64 PDBs should be possible, by the way

[2024-10-09 21:24] Matti: and not particularly hard assuming they haven't been removed from the MS servers

[2024-10-09 21:24] Matti: it's just, this was enough of this for a bit for me, and nama may oa may not look into it

[2024-10-09 21:25] Matti: all you need is the timestamp of the PE file (this is in the PDB) and its SizeOfImage, which can be guessed, calculated or brute forced in a number of ways

[2024-10-09 21:26] Matti: I originally made this handy guilde for the common scenario where one has an incomplete dump of a PE file and wants to obtain the MS original file, but the same method works for this too
[Attachments: image.png]

[2024-10-09 21:28] Matti: in this particular example so few nonzero bits remain in the sizeofimage that brute force might be too strong a term

[2024-10-09 21:28] tifkin: hehe I went down the same route and stopped as well. I've always used https://github.com/google/UIforETW/tree/master/RetrieveSymbols
[Embed: UIforETW/RetrieveSymbols at master · google/UIforETW]
User interface for recording and managing ETW traces - google/UIforETW

[2024-10-09 21:29] Matti: yep, that's pretty much it

[2024-10-09 21:30] Matti: only the minor issue is that without PE file, it's a tiny bit harder to obtain the sizeofimage

[2024-10-09 21:30] Matti: the real issue is finding someone willing to script this

[2024-10-09 21:52] expy: hello guys, any idea how to disable ASLR in the way that it maps to the same image bases across reboots? Also looking for ways to make allocations with heap predictable for research purposes. Thanks

[2024-10-09 21:55] Matti: 1. install windows 7 or below
2. import
```reg
Windows Registry Editor Version 5.00

[HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\Memory Management]
"MoveImages"=dword:00000000
```

[2024-10-09 21:55] Matti: 3. reboot

[2024-10-09 21:56] expy: thanks! I think I need win10 though

[2024-10-09 21:56] Matti: if you need win 8 or newer, well then you can guess the answer

[2024-10-09 21:56] Matti: yeah...

[2024-10-09 21:57] expy: I mean I can patch stuff, for heap I guess I just need to patch the rng source in theory

[2024-10-09 22:00] Matti: well the *entropy* source bits come from the bootloader
but I highly doubt zeroing that will make windows PRNG consistent across reboots alone

[2024-10-09 22:00] Matti: can't say what other sources there are
I vaguely recall this being in windows internals, but unsure

[2024-10-09 22:02] expy: got it, thanks anyway!

[2024-10-09 22:05] Rairii: [replying to Matti: "only the minor issue is that without PE file, it's..."]
theres a few.

i implemented something given just delta compressed binaries https://github.com/Wack0/DeltaDownloader

there's also https://github.com/starfrost013/SymX
[Embed: GitHub - Wack0/DeltaDownloader: Given delta compressed PE files, fi...]
Given delta compressed PE files, find download links for them on the Microsoft Symbol Server. No source PE file or VirusTotal access required. - Wack0/DeltaDownloader
[Embed: GitHub - starfrost013/SymX: A Microsoft Symbol Server bulk download...]
A Microsoft Symbol Server bulk download tool. Contribute to starfrost013/SymX development by creating an account on GitHub.

[2024-10-09 22:06] Rairii: the main issue is those private symbol sets are from branches that werent pushed to the public symbol server, so the files just aren't there

[2024-10-09 22:07] Matti: yeah that second part is what I was more afraid of

[2024-10-09 22:07] Matti: hmm I'm not seeing the SizeOfImage = ... line in these repos so far, but obviously I'm only  skimming

[2024-10-09 22:08] Matti: ah

[2024-10-09 22:08] Matti: the first one explains it right in the readme

[2024-10-09 22:08] Matti: yeah that'll work

[2024-10-09 22:09] Rairii: the second one is i think more forsemi-blind searches

[2024-10-09 22:10] Rairii: like you have a timestamp range and a sizeofimage range for a specificfile

[2024-10-09 22:10] Matti: just FYI, have you tried any of the PDB files (as PE URLs)? stuff has ended up on MS symbol servers by accident before

[2024-10-09 22:10] Rairii: yes

[2024-10-09 22:10] Matti: not that I don't believe you, you're most likely right

[2024-10-09 22:10] Matti: aw, sad

[2024-10-09 22:11] Rairii: i think even now only release branches get uploaded

[2024-10-09 22:11] Matti: release meaning anything intended for public distribution I assume?

[2024-10-09 22:11] Rairii: yeah

[2024-10-09 22:12] Matti: cause obviously insider builds do also have PDBs... but yea

[2024-10-09 22:12] Rairii: well not anything. xbox branches still dont

[2024-10-09 22:12] Matti: no real improvement there

[2024-10-09 22:12] Matti: no but those aren't for windows at all to begin with arguably

[2024-10-09 22:13] Rairii: obviously for a while they were reusing pe timestamps too if there were no code changes

[2024-10-09 22:13] Rairii: so i was just checking every day grabbing bootenv binaries, before blacklotus happened and ms 404'd those, then they fixed that issue because people complained about the version number differences in windbg

[2024-10-09 22:14] Matti: but: counterquestion: does this mean that the warbird.dll PDB was "supposed" to be public, only that this decision itself was a mistake?

[2024-10-09 22:14] Rairii: warbird.dll being there is a mistake

[2024-10-09 22:14] Rairii: that its still there is hilarious

[2024-10-09 22:14] Matti: I think they removed the PDB after like 3 weeks, but I might be wrong

[2024-10-09 22:14] Matti: but either way yeah

[2024-10-09 22:14] Rairii: that one ewdk where they bundled warbird and msvc private symbols is also hilarious

[2024-10-09 22:15] Rairii: it has the only E2 asm that made it out of ms.

[2024-10-09 22:15] Rairii: although zero clue on the instruction encoding from that lol

[2024-10-09 22:15] Matti: hmm, I only recall two WDKs that had private headers, not PDBs

[2024-10-09 22:16] Matti: and obviously by mistake yeah

[2024-10-09 22:16] Matti: which EWDK is/was this?

[2024-10-09 22:16] Rairii: 16225

[2024-10-09 22:16] Matti: lol

[2024-10-09 22:17] Matti: good luck to me finding that

[2024-10-09 22:17] Matti: I didn't even know they did insider EWDKs

[2024-10-09 22:18] Rairii: but yeah the E2 asm included in it is fun

[2024-10-09 22:18] Rairii: too bad they didnt accidentally bundle the compiler as well

[2024-10-09 22:18] Matti: E2 obviously standing for...

[2024-10-09 22:18] Matti: C2.dll codegen?

[2024-10-09 22:19] Matti: I'm clueless about MSVC

[2024-10-09 22:19] Rairii: https://www.theregister.com/2018/06/18/microsoft_e2_edge_windows_10/
[Embed: Now Microsoft ports Windows 10, Linux to homegrown CPU design]
MSR's E2 processor EDGEs into public view... with a little help from Qualcomm, too

[2024-10-09 22:19] Matti: *oh*

[2024-10-09 22:19] Matti: yeah that is quite a big difference

[2024-10-09 22:19] Rairii: that they ported warbird to it is interesting

[2024-10-09 22:20] Rairii: given how long it took them to port warbird to armv7

[2024-10-09 22:21] Rairii: for 10 months they gave to nv/qc/ti armv7 nt builds with spp/etc completely unobfuscated lol

[2024-10-09 22:21] Matti: only 10? I assumed they always did that by default

[2024-10-09 22:22] Matti: unironically

[2024-10-09 22:22] Matti: do they get checked builds at least?

[2024-10-09 22:22] Matti: idk how people do windows kernel dev without checked builds

[2024-10-09 22:22] Rairii: this waswin8 time

[2024-10-09 22:22] Rairii: when the armv7 port was just starting

[2024-10-09 22:22] Matti: yeah, i get that

[2024-10-09 22:22] Matti: but you seem in the know

[2024-10-09 22:23] Rairii: the drops included chk builds yeah

[2024-10-09 22:23] Rairii: one of them even included a kernel stress kit

[2024-10-09 22:23] Rairii: and a few included some private symbols for kernel, hal(ext) and graphics related binaries and few drivers

[2024-10-09 22:23] Matti: yeah, I've got checked builds of every version of NT up until 10.0.15086.0, and a checked kernel + HAL for 10.0.14393.0

[2024-10-09 22:23] Matti: after that it dried up

[2024-10-09 22:23] Rairii: i dont think i have chk th1/th2?

[2024-10-09 22:24] Matti: what the fuck is a th1/th2

[2024-10-09 22:24] Matti: please use build numbers, I can't parse that sorry

[2024-10-09 22:24] Rairii: th1 = 10240
th2 = 10586

[2024-10-09 22:26] Rairii: i know there was something related but its on IA and theyre getting ddosed by morons right now

[2024-10-09 22:26] Matti: checking for you, seems they're not on files.dog/MSDN

[2024-10-09 22:26] Matti: and the one I've got is only 10.0.10586.0, not RTM

[2024-10-09 22:26] Matti: I can send it if you want

[2024-10-09 22:27] Rairii: yeah i think the one i knew about was th2.

[2024-10-09 22:27] Rairii: i heard that th1 was given out though

[2024-10-09 22:27] Rairii: saw references to it around

[2024-10-09 22:27] elias: [replying to Matti: "checking for you, seems they're not on files.dog/M..."]
wth is files.dog <:peepoDetective:570300270089732096>

[2024-10-09 22:27] Matti: https://files.dog/MSDN/

[2024-10-09 22:27] Rairii: not on msdn though but somewhere else

[2024-10-09 22:28] Matti: yeah my """TH2""" checked ISO is from MSDN

[2024-10-09 22:28] Rairii: like some academic thing?

[2024-10-09 22:28] Matti: as in the original MSDN

[2024-10-09 22:28] Matti: unsure, I hadn't heard of this

[2024-10-09 22:28] Matti: definitely possible

[2024-10-09 22:29] Rairii: found the reference to it

[2024-10-09 22:29] Rairii: in an osr forum thread

[2024-10-09 22:31] Rairii: it was on dreamspark apparentlu

[2024-10-09 22:31] Rairii: https://community.osr.com/t/checked-debug-version-of-windows-10/51708
[Embed: Checked/debug version of Windows 10?]
So I went looking for the checked version of Windows 10, and it was nowhere to be found on the MSDN subscription download site. I have a Visual Studio Enterprise with MSDN account, which I believe is 

[2024-10-09 22:31] Matti: [replying to elias: "wth is files.dog <:peepoDetective:5703002700897320..."]
just FYI this an archive of another archive (the eye, since taken down) which itself was simply an archive of lots of things, most notably almost everything that ever existed on MSDN

[2024-10-09 22:32] elias: it seems interesting

[2024-10-09 22:32] elias: thanks for sharing

[2024-10-09 22:32] Matti: [replying to Rairii: "https://community.osr.com/t/checked-debug-version-..."]
yeah, so this is like my question but 9 years later

[2024-10-09 22:33] Rairii: basically since then ms's stance is

[2024-10-09 22:33] Rairii: chk is internal only

[2024-10-09 22:33] Rairii: distributing it is a security violation

[2024-10-09 22:34] Matti: yeahhhhh but secretly also their buddies, right

[2024-10-09 22:34] Rairii: i wonder if its related to velocity

[2024-10-09 22:34] Matti: was my original assumption and question

[2024-10-09 22:34] Matti: nvidia, intel, AMD, QC

[2024-10-09 22:34] Rairii: i dont even think eeap gets them any more

[2024-10-09 22:35] Matti: I will inquire what happened to them and if there's any plan for bringing them back

[2024-10-09 22:35] Matti: but we already know the answer I think

[2024-10-09 22:36] Rairii: but yeah i just wonder if chk builds these days do something with velocity such that all velocity-gated code is present

[2024-10-09 22:36] Matti: idk what velocity even is

[2024-10-09 22:36] Rairii: even if marked as always disabled (ie, should be stripped out by an inlined always false condition)

[2024-10-09 22:36] Matti: but CHK builds are still being built for the same reason they always were

[2024-10-09 22:36] Matti: QA

[2024-10-09 22:37] Matti: though I wonder who's testing them with no testers left

[2024-10-09 22:37] Rairii: https://betawiki.net/wiki/Feature_lockout_in_Windows#Windows_10_and_later
[Embed: Feature lockout in Windows]
On multiple occasions, Microsoft has used various mechanisms to hide early prototypes of new features in pre-release builds of Microsoft Windows before their formal introduction.

[2024-10-09 22:37] Rairii: interesting, these days a lot of security features are behind velocity checks

[2024-10-09 22:37] Matti: ohhh right

[2024-10-09 22:37] Rairii: probably so they can be disabled on systems easily where some issue occurs

[2024-10-09 22:37] Matti: I know these yeah

[2024-10-09 22:37] Matti: just not under this name

[2024-10-09 22:38] Matti: it's just A/B testing shit to me

[2024-10-09 22:38] Rairii: ah right

[2024-10-09 22:38] Rairii: yeah velocity is the official name

[2024-10-09 22:41] Matti: well for this hypothetical use case I would simply enable all of the "under test" features and then compile a CHK OS or kernel

[2024-10-09 22:42] Matti: usually if it compiles, then it will also work (not usually the case...)

[2024-10-09 22:42] Matti: and more code means more chance to fail

[2024-10-09 22:43] Matti: that said though... I'm 100% sure CHK kernels and HALs (less sure about the rest of the OS) are still used for QA

[2024-10-09 22:44] Rairii: funny thing is, i barely used chk builds myself when doing driver dev for nt on ppc mac lol

[2024-10-09 22:45] Matti: well, for driver development I have to say verifier > checked kernels

[2024-10-09 22:45] Matti: but both are critical IMO

[2024-10-09 22:45] Rairii: but i guess without proper nt kd, only emulator debugger/etc

[2024-10-09 22:45] Rairii: why bother lol

[2024-10-09 22:45] Rairii: HalDisplayString debugging, etc.

[2024-10-09 22:46] Matti: MS don't care <:lillullmoa:475778601141403648> XP SP2 CHK released in a state that made it impossible to even install

[2024-10-09 22:46] Rairii: lol

[2024-10-09 22:46] Matti: most "full" checked windows builds aren't useful

[2024-10-09 22:46] Rairii: why didnt i already know that

[2024-10-09 22:46] Rairii: [replying to Matti: "most "full" checked windows builds aren't useful"]
yeah but you get sol.exe debug menu

[2024-10-09 22:46] Rairii: lol

[2024-10-09 22:47] Matti: but the kernel and HAL (and other components like fltmgr or whatever your driver depends on)... those are worth a lot

[2024-10-09 22:47] Matti: [replying to Rairii: "yeah but you get sol.exe debug menu"]
damn, for real?

[2024-10-09 22:47] Rairii: yeah

[2024-10-09 22:47] Rairii: obviously talking about xp and earlier

[2024-10-09 22:48] Matti: wondering if I even have any checked full versions installed around here to try that

[2024-10-09 22:48] Matti: hmm, XP, alright

[2024-10-09 22:48] Matti: I don't, currently...

[2024-10-09 22:48] Matti: 32 vs 64 bit matter?

[2024-10-09 22:49] Rairii: nope

[2024-10-09 22:52] Matti: aw man don't tell me it's in some games.cab or whatever

[2024-10-09 22:52] Matti: I'm trying to cheat by just extracting the exe

[2024-10-09 22:52] Rairii: sol.ex_

[2024-10-09 22:52] Matti: odd it should've found that

[2024-10-09 22:52] Rairii: unless they removed sol from amd64?

[2024-10-09 22:52] Rairii: but given pinball is present i cant see why

[2024-10-09 22:53] Matti: no idea, I don't use user mode much on XP

[2024-10-09 22:53] Matti: but I can check

[2024-10-09 22:53] Rairii: someday i'll do more with bootos.wim and write my own nt userland

[2024-10-09 22:54] Matti: ahh it's `wsol.ex_`

[2024-10-09 22:54] Matti: so it exists, just only in syswow64

[2024-10-09 22:54] Matti: fuck you too man!
[Attachments: image.png]

[2024-10-09 22:55] Rairii: yeah you need cards.dll too lol

[2024-10-09 22:55] Matti: no doubt a bunch more shit

[2024-10-09 22:55] Rairii: nope

[2024-10-09 22:55] Matti: still easier than installing a CHK ISO

[2024-10-09 22:55] Rairii: cards.dll should be the only dep

[2024-10-09 22:55] Rairii: iirc

[2024-10-09 22:55] Matti: man that's a bit disappointing

[2024-10-09 22:55] Matti: yeah checks out
[Attachments: image.png]

[2024-10-09 22:56] elias: whats cards.dll? <:peepoDetective:570300270089732096>

[2024-10-09 22:57] Rairii: the dll used by old sol and freecell to render cards

[2024-10-09 22:57] elias: oh well

[2024-10-09 22:57] elias: that makes sense

[2024-10-09 22:58] Matti: well well
[Attachments: image.png]

[2024-10-09 22:59] Matti: illegal solitaire

[2024-10-09 22:59] Timmy: solitair gamehack!

[2024-10-09 23:01] Matti: ok interesting

[2024-10-09 23:01] Matti: so this was from server 2003 SP x64

[2024-10-09 23:01] Matti: SP1*

[2024-10-09 23:01] Matti: but it turns out XP x64 does have a 64 bit version

[2024-10-09 23:02] Rairii: same OS version

[2024-10-09 23:02] Matti: well yes

[2024-10-09 23:02] Matti: but interesting they chose to put something so insignificant on one ISO and not the other

[2024-10-09 23:02] Matti: or, only half of it

[2024-10-09 23:03] Rairii: i mean, xp amd64 is a client build of 3790 sp1

[2024-10-09 23:03] Matti: I know this

[2024-10-09 23:04] Matti: that's why I'm surprised

[2024-10-09 23:04] Rairii: that they didnt build amd64 sol?

[2024-10-09 23:04] Matti: yes

[2024-10-09 23:04] Rairii: they probably just couldnt be bothered to port it

[2024-10-09 23:04] Matti: if they'd included *no* games - fair enough

[2024-10-09 23:04] Rairii: it was originally a port from win16 anyway

[2024-10-09 23:04] Matti: sure, sure I know this too

[2024-10-09 23:04] Matti: but these were the same OS with a different theme

[2024-10-09 23:06] Rairii: amd64 nt 5.2 was in some ways obviously "yes this is the first release of this port"

[2024-10-09 23:07] Matti: yes, I'm aware <:lillullmoa:475778601141403648>

[2024-10-09 23:07] Matti: I maintain my own fork of the WRK leak

[2024-10-09 23:07] Matti: so the 64 bit port was bby "wes cherry"? or was that just the whole game
[Attachments: image.png]

[2024-10-09 23:08] Matti: in case of interest..
[Attachments: sol.exe, cards.dll, sol.hlp]

[2024-10-09 23:10] Matti: lol, quite a difference from the srv2003 version
[Attachments: image.png]

[2024-10-09 23:10] Matti: call me "Windows User" again

[2024-10-09 23:11] Rairii: [replying to Matti: "so the 64 bit port was bby "wes cherry"? or was th..."]
wes cherry wrote the original code at win 3.0 tjme

[2024-10-09 23:12] Matti: yeah makes sense, it was only ever pinball that was insane enough to need another party to port it I think?

[2024-10-09 23:15] Matti: still looks pretty good in 1440p to be honest
[Attachments: image.png]

[2024-10-09 23:16] estrellas: wonderful

[2024-10-10 00:42] Chucky: Anyone doing flareon 11 ?

[2024-10-10 00:43] Chucky: I want to ask for a little hint regarding challenge 5

[2024-10-10 01:23] 0x208D9: [replying to Chucky: "I want to ask for a little hint regarding challeng..."]
drop a dm

[2024-10-10 01:25] 0x208D9: [replying to Matti: "I maintain my own fork of the WRK leak"]
wait did u make any til's from em?

[2024-10-10 01:28] Matti: yes

[2024-10-10 01:28] Matti: they're out of date though, I need to remake them using idaclang

[2024-10-10 01:29] Matti: 
[Attachments: nt_ros_64_10.til]

[2024-10-10 01:29] Matti: this should work pretty well on a recentish X64 ntoskrnl

[2024-10-10 01:30] Matti: this sorta kinda works for KMDF drivers, wich I hate to reverse, hence why I made a TIL for them
[Attachments: kmdf_1_33_64.til]

[2024-10-10 01:31] 0x208D9: thanks ^_^

[2024-10-10 16:02] anyfun: We made this to generate kmdf til, if that helps…

https://hex-rays.com/blog/plugin-focus-ida-kmdf
[Embed: Plugin focus: ida kmdf – Hex Rays]
This is a guest entry written by Arnaud Gatignol and Julien Staszewski from the THALIUM team. The views and opinions expressed in this blog post are solely

[2024-10-10 16:07] Torph: [replying to Rairii: "the dll used by old sol and freecell to render car..."]
having a totally separate DLL just to render cards is crazy

[2024-10-10 16:11] Torph: [replying to Matti: ""]
what are TILs? I'm only finding an IDA plugin to generate them, and info about some MarioKart Wii format by the same name

[2024-10-10 16:11] Rairii: type libraries

[2024-10-10 16:13] Torph: ooh found it, just had to search about IDA instead of "windows TIL file"

[2024-10-10 17:21] 0x208D9: [replying to anyfun: "We made this to generate kmdf til, if that helps…
..."]
this is great!! thanks alot

[2024-10-10 17:22] anyfun: We are late to add support for ida 9 though 😣

[2024-10-10 19:16] Matti: [replying to anyfun: "We made this to generate kmdf til, if that helps…
..."]
oh yeah this is much better by far

[2024-10-10 19:16] Matti: I'd actually seen this but I lost the link

[2024-10-10 19:16] Matti: my KMDF TIL is just, well a TIL, and worse it's from pre-idaclang times

[2024-10-10 19:17] Matti: but the other TIL... well I daresay it is pretty fucking good

[2024-10-10 19:18] Matti: [replying to Torph: "ooh found it, just had to search about IDA instead..."]
highly recommend this resource for learning idaclang https://docs.hex-rays.com/user-guide/type-libraries/idaclang_tutorial
[Embed: IDAClang | Hex-Rays Docs]

[2024-10-10 19:19] Matti: it even unironically shows you how to make a TIL for IDA itself, which has been helpful with cracking it in the past

[2024-10-10 21:41] Shanks: [replying to Matti: "so you mean compile?"]
XP MSN games require a server and server-side application, in my opinion.

[2024-10-10 21:43] Shanks: But is anyone here interested in bringing these legendary games back to life?

[2024-10-10 21:49] Matti: was any MSN game legendary? I doubt this

[2024-10-10 21:50] Matti: but I clearly forgot about them

[2024-10-10 21:50] Matti: these won't be part of the OS source code though, not the client and definitely not the server

[2024-10-10 22:14] Shanks: Have you checked the Leaked source code for XP and 2003? I'm sure it's there.
[Attachments: IMG_20241010_231305.jpg]

[2024-10-10 22:22] Shanks: [replying to Matti: "was any MSN game legendary? I doubt this"]
You're right about the other games, but games like Checkers and Backgammon .. are a different story. Even the top players in these games are known

[2024-10-10 22:24] Matti: [replying to Shanks: "Have you checked the Leaked source code for XP and..."]
no I haven't - just to clarify, my main point was the first one

[2024-10-10 22:24] Matti: but since you've found this, now you can compile it

[2024-10-10 22:24] Matti: so that's great

[2024-10-11 23:40] 0x208D9: [replying to anyfun: "We are late to add support for ida 9 though 😣"]
check if works 😄
[Attachments: wdf.py]

[2024-10-11 23:40] 0x208D9: to be placed in %IDA_HOME%/plugins

[2024-10-12 22:08] 0x208D9: also a small addition regarding that in the plugins file:
```py
def run(self):
    for ea in idautils.Functions():
        if "DriverEntry" in ida_funcs.get_func_name(ea):
                if wdf.load_wdf():
                    wdf.apply_wdf()
                    idaapi.auto_wait()
        else:
            return False
```

[2024-10-13 00:56] Anthony Printup: https://www.youtube.com/watch?v=DrriTj0IT9E
[Embed: smm]

[2024-10-13 09:44] MonTy: Guys, How to run correctly NoVmp?
[Attachments: image0.jpg]

[2024-10-13 11:14] varaa: 
[Attachments: IMG_0033.png]

[2024-10-13 21:28] mrexodia: 
[Attachments: image.png]