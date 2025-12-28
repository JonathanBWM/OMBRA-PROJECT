# February 2024 - Week 3
# Channel: #reverse-engineering
# Messages: 149

[2024-02-12 09:33] .mydev: \x00 is always a bad char in buffer overflows?....
I've noticed that automatically it's omitted by people

[2024-02-12 09:37] brymko: no

[2024-02-12 09:37] brymko: null byte is only bad for strings

[2024-02-12 09:37] brymko: \n is bad for gets

[2024-02-12 09:37] brymko: other forms may apply

[2024-02-12 09:38] brymko: such if it has to be valid unicode

[2024-02-12 09:38] brymko: and other situational restrictions

[2024-02-12 09:50] .mydev: [replying to brymko: "null byte is only bad for strings"]
As I thought
Bad resources out there...

[2024-02-13 11:26] .mydev: Why do I see many people take an Assembly file, and first executes `nasm` on it, and then `gcc` on the output object file?
Can't we use GCC directly on an Assembly file?

Also, why `ld` requires entry point being `_start` and `GCC` required it being `main`? What's the effective difference between them eventually?

[2024-02-13 11:31] contificate: 1) You can use `gcc` directly, but many don't like to because then you don't have the Intel syntax they know and love - you can use `.intel_syntax noprefix` or something like this, but it's not fully accurate, but close enough. That said, at&t syntax is dominant on `*nix` projects, it's quite rare to see `nasm`.
2) If you link against libc, it requires its own entry point to do initialisation stuff (think about `stdout`, `stdin`, etc.), so its entry code must be at `_start` and then does something similar to `exit(main(argc, argv, envp))` or similar.  The default entry point symbol under *nix is typically `_start`, `main` may exist but it is not the entry point. You can technically tell your linker to use an arbitrary symbol, that you specify, as the entry point.

[2024-02-13 12:33] .mydev: [replying to contificate: "1) You can use `gcc` directly, but many don't like..."]
1. GCC knows to compile AT&T syntax only?
2. Gotcha. By the way, when I write a program with raw assembly, must it contain at least one section - `.text` (in order to be compiled)? True for all the Assemblers?

[2024-02-13 12:34] contificate: 1. It's just got better support for at&t and it's its default syntax. You get intel syntax that's almots compatible, but off in a few obscure ways afaik.
2. You could theoretically use assemblers to just assemble binary data in whatever sections you like, but typically you want to be writing code, so you wanna put them in .text, as that will be the section mapped w/ executable protections.

[2024-02-13 12:35] .mydev: [replying to contificate: "1. It's just got better support for at&t and it's ..."]
2. So section usage is a recommendation basically?

[2024-02-13 12:35] contificate: In effect, yeah. The convention is just to use the appropriate sections in the way they are intended

[2024-02-13 12:37] .mydev: [replying to contificate: "In effect, yeah. The convention is just to use the..."]
Gotcha. So basically sections are just directives indicating the Assembler the required permissions/protections for each segment in the assembly

[2024-02-13 12:37] contificate: do you know how object files are laid out

[2024-02-13 12:37] contificate: like the structure of them?

[2024-02-13 12:37] .mydev: Not exactly

[2024-02-13 12:37] contificate: there is usually a section called `.text`

[2024-02-13 12:37] .mydev: makes sense, ok

[2024-02-13 12:37] contificate: so these sections are not so much indications, but literally informing it how to lay out the object file

[2024-02-13 12:38] .mydev: so what happens if no section is given? all the assembly instructions fall under .text, no?

[2024-02-13 12:38] contificate: guess it depends on the assembler

[2024-02-13 12:38] contificate: typically, assemblers take `file.s` and produce `file.o`

[2024-02-13 12:39] contificate: later, a linker may wish to merge a bunch of `file.o`s

[2024-02-13 12:39] contificate: wouldn't be surprised if some assemblers let you assemble instructions in arbitrary sections

[2024-02-13 12:40] .mydev: interesting..

[2024-02-13 12:40] .mydev: So the takeaway from here - sections are only directives for the assemblers, and not mandatory
they may hint the assembler as to which permission each segment should hold in the object file

[2024-02-13 12:42] contificate: they're intended for inform the assembler about how the object file it's assembling into should be laid out

[2024-02-13 12:42] contificate: remember, it's more involved than just assembling instructions

[2024-02-13 12:42] contificate: there's all kinds of complexities in a toolchain beyond encoding and backpatching instructions

[2024-02-13 12:43] contificate: I really recommend learning how object files ELFs, PEs, etc.

[2024-02-13 12:43] contificate: are laid out

[2024-02-13 12:43] contificate: i.e. assemble some shit, then tear it apart in a disassembler tool

[2024-02-13 12:53] .mydev: [replying to contificate: "they're intended for inform the assembler about ho..."]
I know that.
I only wanted to know if it's mandatory and what happens if....

[2024-02-13 14:53] Matti: [replying to .mydev: "So the takeaway from here - sections are only dire..."]
not really a GCC user, but I can't imagine gas **ignoring** a directive to place something in a specific section
that's why it's a directive, not a hint

[2024-02-13 14:53] Matti: whether the resulting object file will make sense in terms of section permissions (to name just one thing) though, that's something else

[2024-02-13 14:54] Matti: you may also just get a linker error instead because the linker reached that same conclusion

[2024-02-13 15:02] Matti: this may help understand actually: https://raw.githubusercontent.com/Mattiwatti/WinMain-is-usually-a-function/master/src/main.c
(it's C, and MSVC-specific, but the same principle applies)

[2024-02-13 15:03] Matti: GCC equivalent would be `__attribute__((section(".text")))`, though the program itself is also Windows specific so that still won't get you far

[2024-02-13 15:03] Matti: but anyway

[2024-02-13 15:03] Matti: this is an executable program only because the code forces the linker to place  it in `.text`

[2024-02-13 15:05] Matti: if you omit the directive, or use `.data` or `.rdata` instead, the program will compile and link (with only a warning IIRC), but it won't be able to run since the entry point is not in an executable section

[2024-02-13 15:06] Matti: in fact there won't be any executable section

[2024-02-13 16:00] .mydev: [replying to Matti: "not really a GCC user, but I can't imagine gas **i..."]
Now I'm confused 🙂
Not writing directives explicitly may lead to linkage/runtime errors according to what you say?

[2024-02-13 16:09] Matti: well no, that's not really what I was trying to say

[2024-02-13 16:10] Matti: in general you shouldn't need these directives, though there are exceptions

[2024-02-13 16:10] Matti: but this program is an example of an array of integers(!) that is executable

[2024-02-13 16:11] Matti: but only because of the linker directive forcing this

[2024-02-13 16:12] Matti: so it's more like: if you do add a section directive, make sure you understand the implications, because otherwise you can expect link time or runtime errors

[2024-02-13 16:21] .mydev: [replying to Matti: "so it's more like: if you do add a section directi..."]
Yeah. you mean - don't do this (for example):
```
.data
  mov eax, 1
```

[2024-02-13 16:23] Matti: yep

[2024-02-13 16:23] Matti: precisely

[2024-02-13 16:25] .mydev: Another basic question I guess:
I've seen in GDB that:
`rep movsb BYTE PTR es:[edi], ds:[esi]`

I've read about these segment registers, but didn't fully understand

[2024-02-13 16:25] .mydev: it says that `es` is to access data other than data section

[2024-02-13 16:26] .mydev: `ds` == points to the start of `.data` section?
`es` == points to the start of `.bss` section?

[2024-02-13 16:29] Matti: lol that is a good question

[2024-02-13 16:29] Matti: I don't think I've ever seen `es` used

[2024-02-13 16:29] Matti: but, `.bss` sections are also very uncommon on windows

[2024-02-13 16:29] Matti: so that may still check out

[2024-02-13 16:31] Matti: but - ds does not point to the data section (or any section) **necessarily**, I think

[2024-02-13 16:32] Matti: rather there is a way to specify the segment address or something.... I'm gonna stop typing now because I need the SDM to give a non-bullshit answer to this, and I should really be working

[2024-02-13 16:33] Matti: in general you can ignore segments almost always though

[2024-02-13 16:33] Matti: no need to specify them yourself at least

[2024-02-13 16:35] Matti: oh ok right, so `DS` is really just a register, that points to the segment

[2024-02-13 16:35] Matti: but idk at what point it is set and by whom

[2024-02-13 16:39] Matti: oh yes... the GDT (global descriptor table) and LDT (local)

[2024-02-13 16:39] Matti: see SDM, 3.5, segment descriptor types if you really want to

[2024-02-13 16:40] Matti: but the TLDR is that segments are not the same thing as sections

[2024-02-13 16:41] Matti: segments are a CPU feature, sections are part of executable file formats

[2024-02-13 17:17] .mydev: Thanks for the answer dude but nothing seems to be decisive lol

[2024-02-13 17:28] daax: <@162611465130475520> merch on lock
[Attachments: IMG_7527.jpg]

[2024-02-13 17:29] Matti: [replying to .mydev: "Thanks for the answer dude but nothing seems to be..."]
yeah I'm aware <:harold:704245193016344596>  sorry
basically segmented addressing is a legacy x86 holdover, and while some segment registers (`fs`/`gs`) do have special meanings in windows, they are so rarely used in the way you see them in your example above that my knowledge of them is correspondingly shit

[2024-02-13 17:31] Matti: if you want to know the definitive answer you should check out the SDM chapter I mentioned, as well as the linux source code probably

[2024-02-13 17:31] Matti: as it is responsible for setting up the segment descriptors

[2024-02-13 17:32] Matti: [replying to daax: "<@162611465130475520> merch on lock"]
sweet dude

[2024-02-13 17:32] Matti: etawen?

[2024-02-13 17:33] mrexodia: [replying to daax: "<@162611465130475520> merch on lock"]
🥳

[2024-02-13 18:54] piracy0: [replying to daax: "<@162611465130475520> merch on lock"]

[Attachments: 0034793257_36.png]

[2024-02-13 18:54] piracy0: <:yara_lover:1148745271577157673>

[2024-02-13 20:02] x86matthew: [replying to .mydev: "Thanks for the answer dude but nothing seems to be..."]
you can ignore them as matti said, all modern systems use a flat memory model, only gs/fs have a different usage in practice

[2024-02-13 20:03] x86matthew: 16-bit code used a segmented model to allow access to more than 64kb of memory but this isn't relevant to you

[2024-02-13 22:07] Matti: why are you using some mod launcher to launch a modified APK of (I'm assuming) the original game

[2024-02-13 22:08] Matti: and would this mod launcher not get very confused about the fact that your APK does not have the same name as the game it is meant to launch (or mods for it)

[2024-02-13 22:09] Matti: I haven't got this mod launcher, but I tried your steps and I can install and run the game fine, minus the obvious warning about my certificate being untrusted

[2024-02-13 22:09] Matti: since it's not a play store certificate

[2024-02-13 22:11] Matti: 
[Attachments: image.png]

[2024-02-13 22:11] Matti: 
[Attachments: image.png]

[2024-02-13 22:12] Matti: after 'install anyway' it works fine for me

[2024-02-13 22:13] Matti: kinda reminds me of these equally useless prompts in XP
[Attachments: Windows-XP.png]

[2024-02-13 22:16] Matti: so, my guess is that your issue is either with this mod launcher, or else it's the fact that your signing certificate is not a google play store one

[2024-02-13 22:17] Matti: but neither is mine, hence the warning

[2024-02-13 22:45] Matti: well I believe you

[2024-02-13 22:45] Matti: but you get why changing the game name from the original might confuse an app meant to launch this specific game right?

[2024-02-13 22:51] Matti: I'm aware, I did find all this

[2024-02-13 22:51] Matti: I'm just explaining my hypothesis for the failure

[2024-02-13 22:52] Matti: ok, so according to the documentation you should name your mod this? `com.zane.stardewvalley`

[2024-02-13 22:53] Matti: well I'm not asking what you want, sorry

[2024-02-13 22:53] Matti: I'm asking what the developers are telling you to name your mod

[2024-02-13 22:54] Matti: if they are telling you to name it something specific, there's probably a good reason for that

[2024-02-13 22:56] Matti: if they say you can name it whatever you want, then there is a different issue

[2024-02-13 22:57] Matti: it's not entirely clear to me from this readme which one is the case, and I have to say I don't feel like installing this on my phone

[2024-02-13 23:03] Matti: OK, but then I'm even more confused as to why you are using it to test this

[2024-02-13 23:03] Matti: since you are renaming the APK of the game

[2024-02-13 23:06] dullard: 🫃🏿

[2024-02-13 23:10] dullard: Ok

[2024-02-13 23:13] dullard: I am Groot

[2024-02-13 23:16] Matti: I'm not really sure what part is unclear here...

So here's roughly how I would go about writing this mod launcher:

1. Find the APK whose name is `com.chucklefish.stardewvalley` (the original game name, which is also the app identifier)
2. (optional - probably N/A here) Verify this really is the original game by checking the APK signature or something
3. Launch the APK
4. Do whatever mod stuff you want to do to the app you just launched

You understand that renaming the game would make this launcher fail at step (1), right?

[2024-02-13 23:17] Matti: if you want this to work, you would **also** need to modify this mod launcher to teach it about your clone

[2024-02-13 23:18] Matti: renaming the game itself works perfectly fine as I demonstrated above

[2024-02-13 23:19] Matti: yes

[2024-02-13 23:19] Matti: I just did what you described

[2024-02-13 23:19] Matti: decompile, rename, build, sign and install

[2024-02-13 23:21] qwerty1423: [replying to Matti: "I'm not really sure what part is unclear here...

..."]
i remember bringing up such topics were forbidden in other discord servers

[2024-02-13 23:21] Matti: yeah I'm beocming increasingly uncomfortable about helping this guy

[2024-02-13 23:22] Matti: what are you trying to actually do here <@356286421960753153>

[2024-02-13 23:22] Matti: what is the purpose of renaming this game

[2024-02-13 23:25] Matti: well I could see issues with both the literal cloning of a game as well as making a 'mod launcher' for it.... depending on this and that

[2024-02-13 23:26] Matti: is this a multiplayer game? lol

[2024-02-13 23:27] Matti: hmmmm

[2024-02-13 23:28] qwerty1423: isn't the game available for everyone? why would someone play the cracked version?

[2024-02-13 23:30] Matti: lol I missed the fact that this app costs actual money in the android store

[2024-02-13 23:31] Matti: this convo is over, go somewhere else

[2024-02-13 23:35] Matti: yeah haha

[2024-02-13 23:36] Matti: no problem haha

[2024-02-14 21:20] qwerty1423: 
[Attachments: image.png]

[2024-02-14 21:25] qwerty1423: what has happened to v14 and v15 here?

[2024-02-14 21:27] 冰: hmm

[2024-02-15 01:20] birdy: Looks great.

[2024-02-15 09:17] mishap: [replying to qwerty1423: "what has happened to v14 and v15 here?"]
Nothing looks weird here

[2024-02-16 00:53] 25d6cfba-b039-4274-8472-2d2527cb: I'm curious has anyone tried out Ghidras BSim yet? I've not had time/energy to RE stuff recently but I'm curious about the feature. Wondering if its as good as it seems or just a waste of time.

[2024-02-16 10:50] optyx: oh bsim seems very useful. i've only used ghidra a handful of times, but i might check it out for this

[2024-02-18 12:31] .mydev: repe is turned to repz by nasm. Why? In my original assembly I used repe along with je and when I debugged it after compiling with nasm, I see it’s converted to repz and jz

[2024-02-18 12:40] diversenok: These are different names for the same instruction

[2024-02-18 12:42] qwerty1423: https://c9x.me/x86/html/file_module_x86_id_279.html
[Embed: Sun: x86 Instruction Set Reference]
x86 assembly tutorials, x86 opcode reference, programming, pastebin with syntax highlighting

[2024-02-18 13:06] .mydev: [replying to diversenok: "These are different names for the same instruction"]
Yea but why nasm didn’t leave it as is?

[2024-02-18 13:07] diversenok: But how would the debugger know which name you used to begin with?

[2024-02-18 13:07] 25d6cfba-b039-4274-8472-2d2527cb: because such metadata does not exist in the assembled code

[2024-02-18 13:11] .mydev: [replying to diversenok: "But how would the debugger know which name you use..."]
They have actually the very same opcode?

[2024-02-18 13:11] diversenok: Yes

[2024-02-18 13:11] asz: lol

[2024-02-18 13:11] .mydev: Ah

[2024-02-18 13:12] .mydev: 🤨

[2024-02-18 13:12] asz: https://tenor.com/view/leave-her-alone-crying-sad-meme-gif-16919280

[2024-02-18 13:12] asz: leave the opcodes alone!

[2024-02-18 13:12] Deleted User: never my opcode

[2024-02-18 14:34] .mydev: lodsb instruction must be preceded by ecx initialization only if that’s a repeating lodsb?

[2024-02-18 18:59] qwerty1423: [replying to .mydev: "They have actually the very same opcode?"]
take a look at the url i sent

[2024-02-18 19:08] .mydev: [replying to qwerty1423: "take a look at the url i sent"]
Thanks, no it mustn’t