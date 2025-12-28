# January 2024 - Week 2
# Channel: #reverse-engineering
# Messages: 229

[2024-01-08 02:57] Matti: what compelled you to make this post

[2024-01-08 06:43] Deleted User: it be like dat sometimes

[2024-01-08 06:43] Deleted User: you never know

[2024-01-08 08:36] BWA RBX: Damn are you coming out finally?

[2024-01-08 08:40] BWA RBX: [replying to vendor: ""]
Do you want to replace this line of code with a more readable structure access, I'm confused, what do you want hexrays to display?

[2024-01-08 08:51] BWA RBX: Bitch

[2024-01-08 08:51] BWA RBX: This is reverse engineering I think you need to watch your mouth little bro 😫

[2024-01-08 08:52] vendor: [replying to BWA RBX: "Do you want to replace this line of code with a mo..."]
this is not usermode code, there is no TEB at the gs segment

[2024-01-08 08:52] BWA RBX: [replying to vendor: "this is not usermode code, there is no TEB at the ..."]
You can delete segments in IDA

[2024-01-08 08:53] vendor: i’ve looked at the segments and don’t see anything

[2024-01-08 08:53] BWA RBX: [replying to vendor: "i’ve looked at the segments and don’t see anything"]
Use the command to kill a segment

[2024-01-08 08:54] vendor: there is no segment, ida is just trying to be smart and guess what is at the gs:0

[2024-01-08 08:55] BWA RBX: [replying to vendor: "there is no segment, ida is just trying to be smar..."]
You can remove it at the address specified no?

[2024-01-08 08:58] BWA RBX: If this isn't usermode code then do you need the function signatures?

[2024-01-08 08:59] BWA RBX: I mean can you not select signature for kernel mode etc

[2024-01-08 09:59] vendor: no, this isn’t kernel mode either

[2024-01-08 09:59] vendor: code loads its own gs base and i want to define the struct for it

[2024-01-08 09:59] vendor: rather than this windows bullshit ida insists on showing

[2024-01-08 10:37] Matti: the types come from whatever TILs were loaded in the IDB

[2024-01-08 10:37] Matti: without some NT user mode .til IDA has no clue what is in `gs`

[2024-01-08 10:38] Matti: I don't think unloading the TIL will work to reset the type for this (but idk, try it), but this is how you can prevent this from happening in the future at least

[2024-01-08 10:40] Matti: check the type libraries, figure out which one(s) have typedefs for the TEB (or KPRCB for kernel mode... this problem can happen both ways)

[2024-01-08 10:40] Matti: and make sure it isn't loaded when you create a new IDB

[2024-01-08 10:42] Matti: [replying to vendor: "code loads its own gs base and i want to define th..."]
and so the answer to how to do this is: make a TIL file for it

[2024-01-08 10:48] vendor: [replying to Matti: "I don't think unloading the TIL will work to reset..."]
yep i tried this

[2024-01-08 10:48] vendor: [replying to Matti: "and make sure it isn't loaded when you create a ne..."]
fuck i did so much analysis in this one

[2024-01-08 10:49] vendor: urgh, so i have to make a new IDB and just avoid loading the UM TIL initially

[2024-01-08 10:49] vendor: there must be a better way to do this

[2024-01-08 10:50] vendor: actually i wonder how does IDA handle golang, because they swap gs segemnt in usermode for their own scheduler ?

[2024-01-08 10:50] Matti: well similarly if you load your new `MY_GS0_TYPE`-defining TIL, I don't know if that will fix existing instances where IDA thinks it's the TEB

[2024-01-08 10:50] vendor: they restore gs when calling external c functions but mid binary it's fucked

[2024-01-08 10:51] vendor: [replying to Matti: "well similarly if you load your new `MY_GS0_TYPE`-..."]
ah true, i'll see

[2024-01-08 10:53] Matti: I have to say I'm not 100% clear on how IDA derives 'TEB' from a `gs` read using TILs alone, since they only contain.... well types, hence the name

[2024-01-08 10:53] Matti: so you may actually need a sig file for your `MyReadGs0()` function, and a TIL for the type it returns

[2024-01-08 10:53] vendor: yeah there seems to be a missing part of the puzzle

[2024-01-08 10:54] vendor: i'll check the TIL and see if there are any signs of how it points Gs to TEB

[2024-01-08 10:54] asz: they also contain macros i think

[2024-01-08 10:55] Matti: yeah TILs are a bit fucky <:harold:704245193016344596>

[2024-01-08 10:55] Matti: it could also be forceinlines in headers that it includes in 'types'

[2024-01-08 10:55] Matti: you can dump the contents of a TIL with `tilib -l <file>` to check

[2024-01-08 10:59] vendor: 
[Attachments: image.png]

[2024-01-08 10:59] vendor: i unloaded this fucker but he's returned?

[2024-01-08 11:00] vendor: 
[Attachments: image.png]

[2024-01-08 11:00] vendor: WOOOO

[2024-01-08 11:00] vendor: 
[Attachments: image.png]

[2024-01-08 11:00] vendor: and i think i can just set the return type inline here?

[2024-01-08 11:00] Matti: [replying to vendor: "i unloaded this fucker but he's returned?"]
some are autoloaded for certain sig files, see `sig/pc/autoload.cfg`

[2024-01-08 11:01] vendor: hmn, i see

[2024-01-08 11:01] Matti: [replying to vendor: "and i think i can just set the return type inline ..."]
uhh you can but I don't think that will affect more than just this single inline call

[2024-01-08 11:01] Matti: hence why you make a til

[2024-01-08 11:01] vendor: yeah need to figure out how it's applied globally

[2024-01-08 11:03] Matti: FWIW, the way `NtCurrentTeb()` is defined in the windows SDK precisely is
```c
__forceinline
struct _TEB *
NtCurrentTeb (
    VOID
    )

{
    return (struct _TEB *)__readgsqword(FIELD_OFFSET(NT_TIB, Self));
}
```

[2024-01-08 11:04] Matti: since this is winnt.h, it obviously doesn't include the actual TEB type.... but in a header for a TIL you can just do that properly of course

[2024-01-08 11:05] vendor: yeah that's fine i want to define the struct in the normal structs window as im building it on the fly

[2024-01-08 11:05] vendor: gonna just make it 10k bytes and blank then fill in the fields as i go

[2024-01-08 11:05] vendor: this is hvax64.exe im reversing so i don't think anybody has publicly documented the struct

[2024-01-08 11:06] Matti: oh I see, the type is a WIP

[2024-01-08 11:06] Matti: well you don't *have* to define it in the TIL, you  could just do like above

[2024-01-08 11:06] Matti: and then make the type in IDA manually later

[2024-01-08 11:06] Matti: whichever you prefer

[2024-01-08 11:06] vendor: yeah, thanks for the help

[2024-01-08 17:57] qwerty1423: are there any plugins for IDA to search for a pattern of instructions? not a regex

[2024-01-08 18:58] dullard: [replying to qwerty1423: "are there any plugins for IDA to search for a patt..."]
I’m sure you can do it quite easily with the API

[2024-01-08 18:58] dullard: Fundamentally just sigscanning

[2024-01-08 19:04] mrexodia: [replying to dullard: "I’m sure you can do it quite easily with the API"]
You should see the amount of code required to do it in Ghidra <:KEKW:912974817295212585>

[2024-01-08 19:05] dullard: [replying to mrexodia: "You should see the amount of code required to do i..."]
I haven’t really used Ghidra

[2024-01-08 19:05] dullard: Least intuitive GUI which looks like it was designed in the 90s

[2024-01-08 19:06] mrexodia: The scripting is so fire tho

[2024-01-08 19:23] sariaki: [replying to qwerty1423: "are there any plugins for IDA to search for a patt..."]
you mean a plugin to search for sigs?

[2024-01-08 19:24] qwerty1423: yep

[2024-01-08 19:27] sariaki: i can't seem to find the one i use that has a sig testing functionality, but https://github.com/therealzoomgod/pySigMaker seems to have what you want
[Embed: GitHub - therealzoomgod/pySigMaker: Port of IDA plugin SigMaker-x64...]
Port of IDA plugin SigMaker-x64 to IDAPython. Contribute to therealzoomgod/pySigMaker development by creating an account on GitHub.

[2024-01-08 19:27] sariaki: no idea how good it is, since i haven't used it

[2024-01-08 20:30] Matti: [replying to vendor: "this is hvax64.exe im reversing so i don't think a..."]
oh yeah... I totally forgot to reply to this

[2024-01-08 20:30] Matti: you're right, I don't think so either for hvax64

[2024-01-08 20:30] Matti: **but**

[2024-01-08 20:31] Matti: are you aware there's a build of hvaa64 with a private PDB available?

[2024-01-08 20:31] Matti: so the ARM64 version

[2024-01-08 20:32] vendor: i don't have it but would love it

[2024-01-08 20:32] vendor: i've heard of it

[2024-01-08 20:32] Matti: it'll be completely different in implementation of course, but I assume hvix64/hvax64/hvaa64 will all have some structures in common, as well as the general design

[2024-01-08 20:32] vendor: i did a bit of googling around and couldn't find it

[2024-01-08 20:32] Matti: 
[Attachments: hvaa64.exe, hvaa64.pdb]

[2024-01-08 20:32] vendor: oh my wow

[2024-01-08 20:32] vendor: thanks you

[2024-01-08 20:33] Matti: nps! enjoy

[2024-01-08 20:33] vendor: that's brilliant thank you a ton

[2024-01-08 22:08] mrexodia: [replying to Matti: ""]
Matti being a general chad as usual

[2024-01-09 09:21] Saturnalia: [replying to dullard: "I haven’t really used Ghidra"]
scripting with java <:kekw:904522300257345566>

[2024-01-09 10:16] mrexodia: [replying to Saturnalia: "scripting with java <:kekw:904522300257345566>"]
You laugh, but Ghidra scripting is a million times better than IDA

[2024-01-09 10:16] mrexodia: Additionally you can actually debug your scripts (and the Ghidra functions you call) with a single click

[2024-01-09 10:25] Saturnalia: [replying to mrexodia: "Additionally you can actually debug your scripts (..."]
<:binja:760198644904362035>

[2024-01-09 10:26] Saturnalia: yes idapython sucks but at least you can debug scripts with a janky vscode plugin

[2024-01-09 10:26] mrexodia: [replying to Saturnalia: "yes idapython sucks but at least you can debug scr..."]
Yeah but even if you can debug the scripts the IDA API is still super inconsistent and weird.

[2024-01-09 10:26] Saturnalia: oh 100%

[2024-01-09 10:27] mrexodia: The Ghidra API actually makes sense + you get type checking for your code which makes it better

[2024-01-09 10:27] Saturnalia: my favourite part about idapython is they bothered doing typing stuff on some bits of it

[2024-01-09 10:27] Saturnalia: but only some stuff

[2024-01-09 10:27] mrexodia: I'm sure it'll be decent in 5 years

[2024-01-09 10:28] mrexodia: But then there is the decompiler API <:harold:704245193016344596>

[2024-01-09 10:28] mrexodia: The ghidra one is fairly nice there too

[2024-01-09 10:28] Saturnalia: ctrees 💀

[2024-01-09 10:28] mrexodia: and in IDA you go read the verify.hpp brr

[2024-01-09 10:29] Saturnalia: yeah, ghidra API was nice when I used it but it felt super verbose (for obvious reasons)

[2024-01-09 10:29] Saturnalia: binja api is ❤️ though

[2024-01-09 10:29] Saturnalia: and they actually merge my PRs when I hit a bug in it

[2024-01-09 10:41] mrexodia: Ghidra so far has fixed the issues I reported in less than a week

[2024-01-09 10:41] mrexodia: PRs don't get merged though, indeed

[2024-01-09 13:25] Timmy: [replying to mrexodia: "Yeah but even if you can debug the scripts the IDA..."]
theres a fix for that actually, someone wrote an wrapper api called Sark. its very pleasant to use.

[2024-01-09 13:26] Timmy: its on github

[2024-01-09 13:26] mrexodia: yeah I'm aware of that thing, but it's not really a fix as such

[2024-01-09 13:26] mrexodia: paying 20k to use someone's incomplete script wrapper?

[2024-01-09 13:26] mrexodia: 🤔

[2024-01-09 13:26] Timmy: not for the debug stuff for sure

[2024-01-09 13:26] Timmy: wdym 20k

[2024-01-09 13:26] Timmy: its free

[2024-01-09 13:27] mrexodia: IDA is 20k

[2024-01-09 13:27] Timmy: <:gigachad:904523979249815573>

[2024-01-09 13:27] asz: when you can pay some chinaman 10k for reskinned x64dbg

[2024-01-09 13:27] mrexodia: <:BebeHarold:982979787503050762>

[2024-01-09 13:31] 25d6cfba-b039-4274-8472-2d2527cb: I use ghidra but haven't touched the api tbh. I'm afraid of Java code

[2024-01-09 13:32] mrexodia: this is a first

[2024-01-09 13:32] mrexodia: <:kappa:697728545631371294>

[2024-01-09 13:35] 25d6cfba-b039-4274-8472-2d2527cb: every time I read java code it involves 50x20 line java classes for 1 feature

[2024-01-09 13:35] 25d6cfba-b039-4274-8472-2d2527cb: its insane

[2024-01-09 13:35] 25d6cfba-b039-4274-8472-2d2527cb: need some super IDE to keep track of that shit

[2024-01-09 13:35] mrexodia: true dat, IntelliJ

[2024-01-09 13:35] mrexodia: but modern java is _almost_ C#

[2024-01-09 13:35] mrexodia: (eg almost good)

[2024-01-09 13:36] mrexodia: hella stable tho

[2024-01-09 13:36] veritas: you mean ENTERPRISE code ™️

[2024-01-09 13:36] veritas: https://github.com/EnterpriseQualityCoding/FizzBuzzEnterpriseEdition
[Embed: GitHub - EnterpriseQualityCoding/FizzBuzzEnterpriseEdition: FizzBuz...]
FizzBuzz Enterprise Edition is a no-nonsense implementation of FizzBuzz made by serious businessmen for serious business purposes. - GitHub - EnterpriseQualityCoding/FizzBuzzEnterpriseEdition: Fizz...

[2024-01-09 13:36] mrexodia: skill issue honestly

[2024-01-09 13:36] mrexodia: I know the meme

[2024-01-09 13:36] mrexodia: but there is also a pragmatic reality

[2024-01-09 13:36] veritas: is GitHub dead right now

[2024-01-09 13:37] asz: [replying to mrexodia: "but there is also a pragmatic reality"]
https://cdn.discordapp.com/attachments/984082381759737876/993069687644618832/unknown.png?%3f.gif

[2024-01-09 13:38] 25d6cfba-b039-4274-8472-2d2527cb: [replying to veritas: "https://github.com/EnterpriseQualityCoding/FizzBuz..."]
this pretty much. not really the languages fault but I just dislike it <:bama:524677138327273472>

[2024-01-09 13:38] asz: i like that its not perl

[2024-01-09 13:40] mrexodia: [replying to 25d6cfba-b039-4274-8472-2d2527cb: "this pretty much. not really the languages fault b..."]
crazy innit, you dislike bad code

[2024-01-09 13:41] mrexodia: 🤔

[2024-01-09 13:41] 25d6cfba-b039-4274-8472-2d2527cb: but its too common with jaba

[2024-01-09 13:41] 25d6cfba-b039-4274-8472-2d2527cb: tho nothing is worse than untyped python

[2024-01-09 13:43] mrexodia: no

[2024-01-09 13:43] mrexodia: there is lua

[2024-01-09 13:43] 25d6cfba-b039-4274-8472-2d2527cb: no

[2024-01-09 13:43] 25d6cfba-b039-4274-8472-2d2527cb: ok lua wins

[2024-01-09 13:43] veritas: [replying to 25d6cfba-b039-4274-8472-2d2527cb: "tho nothing is worse than untyped python"]
`s/untyped//`

[2024-01-09 13:43] dullard: 1984’d?

[2024-01-09 13:43] veritas: i had to clone our backend for work and i was the only one with an ARM mac. i wanted to die because i couldnt get deps to install 😭

[2024-01-09 13:44] 25d6cfba-b039-4274-8472-2d2527cb: typed python is at least readable

[2024-01-09 13:44] mrexodia: [replying to dullard: "1984’d?"]
ok mister glow

[2024-01-09 13:44] dullard: I’m absorbing light that’s how much I’m the opposite <:Kappa:1082189237178351666>

[2024-01-09 13:44] 25d6cfba-b039-4274-8472-2d2527cb: [replying to dullard: "I’m absorbing light that’s how much I’m the opposi..."]
only to emit it back out in the dark?

[2024-01-09 13:45] Timmy: lua is very readable

[2024-01-09 13:45] dullard: Yes ofc

[2024-01-09 13:45] Timmy: there's just a lot of bad lua out there

[2024-01-09 13:45] dullard: I have fond memories of lua in modded Minecraft, programming those computer blocks to do things for me 😂😂

[2024-01-09 13:46] 25d6cfba-b039-4274-8472-2d2527cb: in game computers are meant to be programmed in Expression2, everything else is heresy

[2024-01-09 13:46] Timmy: and you can add super nice intellisense level typing

[2024-01-09 13:46] 25d6cfba-b039-4274-8472-2d2527cb: [replying to Timmy: "there's just a lot of bad lua out there"]
especially the kind where indexing starts at 1

[2024-01-09 13:47] veritas: u have to use roblox lua https://luau-lang.org/
[Embed: Luau]
Luau (lowercase u, /ˈlu.aʊ/) is a fast, small, safe, gradually typed embeddable scripting language derived from Lua.

[2024-01-09 13:47] Timmy: such that you have type info of everything including classes

[2024-01-09 13:47] Timmy: roblox luau sucks

[2024-01-09 13:47] Timmy: luajit is da wae

[2024-01-09 13:48] veritas: yea it is a bit of a meme

[2024-01-09 13:48] veritas: but if you left it up to me we'd all be using js as a scripting lang so ignore me

[2024-01-09 13:48] Timmy: [replying to 25d6cfba-b039-4274-8472-2d2527cb: "especially the kind where indexing starts at 1"]
just convention, nothing stopping you from using 0

[2024-01-09 13:49] 25d6cfba-b039-4274-8472-2d2527cb: if you asked me the proper way to extend software functionality would be to use shared objects and dlls

[2024-01-09 13:49] Timmy: some brazillian thought it was a good idea to start at 1

[2024-01-10 00:08] BWA RBX: ```python
y = 0
X = 1
z = 2
```

Python is great because we get to assign values to variable like so

[2024-01-10 00:09] BWA RBX: Python such good language 🥵

[2024-01-10 00:11] BWA RBX: I think hexrays IDAPython API is awesome for scripting and creating plug-ins in general, don't think I've seen IDALua though

[2024-01-10 00:11] BWA RBX: Would be too hard to make a wrapper for it no?

[2024-01-10 03:50] Torph: [replying to BWA RBX: "```python
y = 0
X = 1
z = 2
```

Python is great b..."]
why is this a plus? you can get all sorts of wacky shit where a variable is set to the result of some obscure function, which has no documentation and returns a custom dataclass or something

[2024-01-10 03:53] Torph: [replying to 25d6cfba-b039-4274-8472-2d2527cb: "if you asked me the proper way to extend software ..."]
I would be down for this, especially if there were a language-level built-in way to import from a shared library. something like the MSVC pragma thing where you can define a symbol as coming from a DLL without an import library, but as a standard part of C/C++

[2024-01-10 03:55] Torph: would make it way easier to write plugins for a program in a cross-platform way, without needing to grab their import lib binary or building their code

[2024-01-10 04:19] asz: like  the extern keyword?

[2024-01-10 04:24] Torph: that doesn't tell it which shared library to import from, so you still need the separate import library (this might be just a Windows DLL thing though, not sure)

[2024-01-10 04:25] Torph: the MSVC feature I was talking about automatically adds it to the import section without any extra linker flags

[2024-01-10 04:26] asz: idonteven

[2024-01-11 15:36] roddux: anyone use ImHex here?

[2024-01-11 15:38] roddux: I'm trying to figure out how to filter pattern data.

I got a pattern like this..:
```
struct blah {
  u8 name[10];
  u8 flag1;
  u8 flag2;
  // etc
}
fields[100] = 0x0;
```

— and i'm trying to filter the view to _just_ the `name` field. I've tried `fields[*].name`, `fields[].name`, and various other things - to no avail

[2024-01-11 15:49] roddux: it _appears_ I can get what I'm after with `fields.*.name` 🤔

[2024-01-11 18:06] Torph: you can mark fields with `[[hidden]]` to hide them from the pattern data window, if that's what you mean:
```c
struct blah {
    u8 name[10];
    u8 flag1 [[hidden]];
    u8 flag2 [[hidden]];
    // etc
}
```

[2024-01-11 18:07] Timmy: never knew! is that standard?

[2024-01-11 18:09] Torph: i think so, it's on the wiki
https://docs.werwolv.net/pattern-language/core-language/attributes
[Embed: Attributes]

[2024-01-11 18:10] Torph: you can also use format attributes to decode bytes before displaying them if maybe they're encrypted or compressed or something

[2024-01-11 18:11] Torph: there's an image previewer too, but it doesn't support many extra formats that I need for my use case

[2024-01-13 07:58] daax: A Windows CTF for anyone interested in dealing with a binary that requires a mixed approach and circumventing various anti-tamper mechanisms. Should be pretty straightforward to do once you do some initial RE.

The program can be run, just remove the `.bin` extension. 

The flag is in `xxxx-xxxxxxxxxx[...]xxx` format.
[Attachments: ctf0.exe.bin]

[2024-01-13 16:21] Horsie: can anyone here that has windows 10 running on bochs share their bochsrc?

[2024-01-13 16:27] Horsie: When I try to boot from a vdi the bios says: "A disk read error occured"

[2024-01-13 16:28] Horsie: Using the bximage tool to convert vdi to sparse type and booting it, it boots but windows fails with "process1 initialization failed"

[2024-01-13 16:33] Horsie: Trying to boot from the windows iso makes it bootloop

[2024-01-13 20:58] daax: That's a bold objective, keep me posted how it goes!

[2024-01-13 23:21] vendor: probably compiler generated section for retpoline mitigation related stuff

[2024-01-14 05:51] mibho: [replying to daax: "A Windows CTF for anyone interested in dealing wit..."]
man this is cool as shit

[2024-01-14 05:51] mibho: is it possible to do it completely statically?

[2024-01-14 06:01] mibho: https://i.gyazo.com/eddea3ab39cc7b3c64a54e29c9f3eeb4.png
from context can tell it gets PE header offset but i thought ((PEB + 0x18) + 0x10) = flags for processheap

[2024-01-14 06:04] mibho: also ive had it 'loading' in ida for like 1 hr and it's still going <:nickyoung:783940642945368104>

[2024-01-14 06:09] bowen: What are some beginner crackmes I could mess around with in IDA or x32 debug to get the hang of reverse engineering

[2024-01-14 07:12] daax: [replying to mibho: "also ive had it 'loading' in ida for like 1 hr and..."]
haha, try it in ghidra <:Kappa:794707301436358686> jk

[2024-01-14 07:15] daax: [replying to mibho: "is it possible to do it completely statically?"]
yeah though it’s extremely time consuming

[2024-01-14 07:19] daax: [replying to bowen: "What are some beginner crackmes I could mess aroun..."]
make some of your own applications with a “guess the password” or something, and then pop them ida if you cant find any initially

[2024-01-14 07:19] daax: pinning in case anyone else wants to try the challenge

[2024-01-14 07:27] daax: [replying to mibho: "https://i.gyazo.com/eddea3ab39cc7b3c64a54e29c9f3ee..."]
Peb + 18 is Ldr

[2024-01-14 08:25] asz: whats pg13 then

[2024-01-14 09:01] Deleted User: [replying to asz: "whats pg13 then"]
we dont know

[2024-01-14 09:01] Deleted User: never heard

[2024-01-14 10:35] vendor: [replying to daax: "A Windows CTF for anyone interested in dealing wit..."]
it's cool, learning new tricks already

[2024-01-14 12:44] qwerty1423: the anti tampering is a little unnerving at some points

[2024-01-14 17:53] daax: [replying to qwerty1423: "the anti tampering is a little unnerving at some p..."]
how so?

[2024-01-14 17:59] daax: [replying to vendor: "it's cool, learning new tricks already"]
heh, awesome. once someone solves this one i’ll put out the second one that has some other funnies

[2024-01-14 18:02] vendor: [replying to daax: "heh, awesome. once someone solves this one i’ll pu..."]
i reversed like 5 of the 18 anti-everything checks and then lost patience. will resume it at another point. the string decryption is neat, and holy hell is there a lot of "things" it checks for xD

[2024-01-14 18:03] vendor: where the hell did you get that list from 🤣

[2024-01-14 18:03] daax: [replying to vendor: "where the hell did you get that list from 🤣"]
my imagination lol + a few tried and true methods that have been around the block

[2024-01-14 18:06] daax: [replying to vendor: "i reversed like 5 of the 18 anti-everything checks..."]
very nice, you don’t have to deal with them all if you don’t want btw, if you know what the hint is referring to - don’t wanna give away too much though haha

[2024-01-14 18:07] vendor: [replying to daax: "very nice, you don’t have to deal with them all if..."]
yeah i was looking at them just out of curiosity, i realized not all are relevant depends how you run it but i won't say anymore.

[2024-01-14 18:08] Deleted User: 🙂 looks like a fun challenge, will give a try later

[2024-01-14 18:20] dullard: run using dgenerate, profit ?

[2024-01-14 21:55] diversenok: Does x64dbg support loading symbols from .dbg files?

[2024-01-14 22:33] mrexodia: [replying to diversenok: "Does x64dbg support loading symbols from .dbg file..."]
no, only pdb

[2024-01-14 23:12] Deleted User: it’s quite a dense challenge for sure 🙂

[2024-01-14 23:12] Deleted User: a lot of inlined code (STL and api hashing) and garbage bytes

[2024-01-14 23:16] Deleted User: With a bit of manual work, you can use f5 pretty well to be fair

[2024-01-14 23:27] daax: [replying to Deleted User: "With a bit of manual work, you can use f5 pretty w..."]
did you solve it?

[2024-01-14 23:37] Deleted User: [replying to daax: "did you solve it?"]
not yet, but i am working on it. pretty cool chal so far 🙂

[2024-01-14 23:43] daax: [replying to Deleted User: "not yet, but i am working on it. pretty cool chal ..."]
thanks mate

[2024-01-14 23:44] daax: so far nobody has DM'd me the flag, so we'll see who nabs it first <:hackerman:504389083477573632>

[2024-01-14 23:44] daax: not that it's a competition*