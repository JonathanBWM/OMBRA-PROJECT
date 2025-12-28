# May 2024 - Week 5
# Channel: #reverse-engineering
# Messages: 116

[2024-05-28 14:05] diversenok: So I made a pair of exe+pdb, x64dbg and System Informer identify private functions correctly while IDA refuses to do so. What could be the reason?

[2024-05-28 14:06] diversenok: File -> Load file -> Load PDB says PDB: total 0 symbols loaded

[2024-05-28 14:12] diversenok: 
[Attachments: image.png]

[2024-05-28 14:13] diversenok: x64dbg works fine with it
[Attachments: image.png]

[2024-05-28 14:15] diversenok: 
[Attachments: Experiments.zip]

[2024-05-28 17:45] mrexodia: [replying to diversenok: "So I made a pair of exe+pdb, x64dbg and System Inf..."]
Try replacing msdia140.dll with the latest version

[2024-05-28 17:51] diversenok: Which one? Not sure IDA uses it

[2024-05-28 17:55] mrexodia: I think IDA uses it

[2024-05-28 18:02] diversenok: Hmm, it doesn't seem to be loaded into the process nor even be available in IDA installation

[2024-05-28 18:05] mrexodia: Maybe dbghelp.dll then?

[2024-05-28 18:05] mrexodia: Or might be they switched to their own parser too?

[2024-05-28 18:07] diversenok: I just noticed that Binary Ninja also doesn't want to apply symbols from this pdb

[2024-05-28 18:09] diversenok: I have a bit of a convoluted setup since I'm generating these PDBs for Delphi executables

[2024-05-28 18:10] diversenok: The compiler doesn't support generating .pdb and outputs .map files which I need to convert

[2024-05-28 18:10] diversenok: https://github.com/rainers/cv2pdb

[2024-05-28 18:12] diversenok: The output file is probably at least somewhat atypical, I get it

[2024-05-28 18:13] diversenok: But it works perfectly fine with many other programs like x64dbg, procmon, System Informer, etc.

[2024-05-28 18:14] diversenok: So I don't understand why decompilers have an issue with it

[2024-05-28 19:20] mrexodia: Probably they are not matching the GUID correctly 🤷‍♂️

[2024-05-28 19:21] mrexodia: Or that converter isn’t supported by the rust lib

[2024-05-28 19:29] Matti: works in my IDA
[Attachments: image.png]

[2024-05-28 19:29] Matti: never trust the default pdb plugin

[2024-05-28 19:30] Matti: try moving the default plugins/pdb64.dll out of the way temporarily and using this one instead
[Attachments: pdb64.pdb, pdb64.dll]

[2024-05-28 19:30] Matti: oh and also this in cfg/ probably
[Attachments: pdb.cfg]

[2024-05-28 19:31] Matti: [replying to mrexodia: "Probably they are not matching the GUID correctly ..."]
yep, this is it

[2024-05-28 19:31] Matti: well technically anyway

[2024-05-28 19:32] Matti: I think it's more fair to say that the pdb plugin just doesn't know about or try to load any DIA versions newer than msdia90.dll(?)

[2024-05-28 19:32] Matti: so VS 2008 or so

[2024-05-28 19:34] Matti: <@503274729894051901> you'll also need to have msdia140.dll for amd64 registered via COM (probably the case if you have VS installed), or else you can put it in
`C:\Program Files\Common Files\microsoft shared\VC\` instead and I think that should probably work too

[2024-05-28 19:35] Matti: [replying to mrexodia: "I think IDA uses it"]
oh yeah and this is also an issue

[2024-05-28 19:35] Matti: it kinda no longer does

[2024-05-28 19:35] Matti: since 7.2 IDA uses its own shittier PDB parser instead of MS DIA, at least by default unless overridden in the cfg file

[2024-05-28 19:36] Matti: but yeah the DIA-using version is also quite unimpressive and will just crash on some newer PE files that do fancy things

[2024-05-28 19:37] Matti: [replying to Matti: "try moving the default plugins/pdb64.dll out of th..."]
this is just an unfucked version of the pdb plugin source code from the IDA SDK

[2024-05-28 19:38] Matti: it only supports MS DIA, not the IDA PDB parser as that code is not in the SDK

[2024-05-28 19:38] Matti: but since DIA is simply objectively better it's not really an issue

[2024-05-28 20:06] diversenok: Cool, yeah, the plugin works in newer versions

[2024-05-28 20:08] diversenok: [replying to Matti: "<@503274729894051901> you'll also need to have msd..."]
Ahh, and I see it
[Attachments: image.png]

[2024-05-28 20:08] Matti: you mean the default IDA plugin works?

[2024-05-28 20:08] Matti: and if so is that with DIA or with IDAPDB?

[2024-05-28 20:08] diversenok: I put your plugin into 8.4

[2024-05-28 20:08] Matti: ah alright

[2024-05-28 20:09] Matti: so the 8.4 plugin still sucks

[2024-05-28 20:09] diversenok: Doesn't work in 7.6
[Attachments: image.png]

[2024-05-28 20:09] Matti: nah it wouldn't

[2024-05-28 20:09] Matti: I can give you a 7.6 version if you want

[2024-05-28 20:09] Matti: I've been maintaining this for years and years

[2024-05-28 20:09] diversenok: Yes please

[2024-05-28 20:10] Matti: oh that's a bit of a lie I'm afraid

[2024-05-28 20:10] Matti: your choices are either 7.5 or 7.7

[2024-05-28 20:10] Matti: I ain't got no 7.6 for some reason

[2024-05-28 20:11] diversenok: [replying to mrexodia: "Probably they are not matching the GUID correctly ..."]
Doesn't look like it ¯\_(ツ)_/¯
[Attachments: image.png]

[2024-05-28 20:12] Matti: if I had to guess I'd say it's pretty likely the 7.7 plugin will work in 7.6

[2024-05-28 20:12] Matti: but no guarantees

[2024-05-28 20:12] Matti: I did make a bunch more fixes to that version if I remember right

[2024-05-28 20:13] Matti: gimme a few, I need to unfuck this code a bit for 2024 and probably at least check that it doesn't instantly crash

[2024-05-28 20:15] Matti: uh weird question maybe but do you need this plugin to work on windows 7, now or ever

[2024-05-28 20:15] Matti: that seems to be the only substantial fix I've made to the 8.3 plugin compared to the 7.7 one

[2024-05-28 20:15] diversenok: Not really

[2024-05-28 20:16] diversenok: I might also just get a newer IDA

[2024-05-28 20:16] Matti: yeah 8.3 is a really good deal nowadays

[2024-05-28 20:16] Matti: costs a lot less than it used to

[2024-05-28 20:17] Matti: probably... due to... 8.4 being released.... yeah

[2024-05-28 20:17] Matti: that's it

[2024-05-28 20:17] Brit: <:topkek:904522829616263178>

[2024-05-28 20:30] Matti: ah *fak*

[2024-05-28 20:30] Matti: [replying to Matti: "if I had to guess I'd say it's pretty likely the 7..."]
should've known this was too optimistic

[2024-05-28 20:30] Matti: there are some trivial string APIs from 7.7 used here and there that I'mma need to unfuck

[2024-05-28 20:31] Matti: one more minute please sir, ty for waiting

[2024-05-28 20:52] mrexodia: [replying to diversenok: "Doesn't look like it ¯\_(ツ)_/¯"]
What about the Age component though?

[2024-05-28 20:52] Matti: ok it seems to work, thank fuck

[2024-05-28 20:52] Matti: 
[Attachments: pdb.cfg, pdb.pdb, pdb64.pdb, pdb64.dll, pdb.dll]

[2024-05-28 20:53] Matti: I'm obviously not allowed to share the source code (||or even these binaries technically||) but DM me if you want to talk about whatever

[2024-05-28 20:55] Matti: it looks like 7.6 was right in the middle of a period when I just didn't touch this code for a few years in a row because it worked well enough <:kekw:904522300257345566>

[2024-05-28 20:56] Matti: so it turns out that what I thought was the code for 7.4 and 7.5 was not actually that, more like a lazy copy to at least have a history in git that definitely did not even fucking compile

[2024-05-28 20:56] Matti: I only stopped slacking again after 7.7 came out

[2024-05-28 20:58] diversenok: [replying to Matti: "I'm obviously not allowed to share the source code..."]
Share what? 😉

[2024-05-28 20:59] diversenok: Yep, just checked, it works, thanks

[2024-05-28 21:12] diversenok: [replying to mrexodia: "What about the Age component though?"]
Age is 1 in the exe; how do I check it in the pdb?

[2024-05-28 21:13] Matti: ```
llvm-pdbutil pretty Experiments.pdb
Summary for C:\Users\Matti\Desktop\Experiments\Experiments.pdb
  Size: 101376 bytes
  Guid: {10F802BB-DE2E-4A5C-9136-D5CAFED19CCD}
  Age: 1
  Attributes: HasPrivateSymbols
```

[2024-05-28 21:14] Matti: symchk.exe from the SDK can probably do it too

[2024-05-28 21:14] diversenok: Also just found the structure definition in the LLVM docs

[2024-05-28 21:14] diversenok: Age is right before the GUID

[2024-05-28 21:15] Matti: stimmt

[2024-05-30 07:32] Horsie: Does anyone know what the 0x210 (528 dec) pool type is?

[2024-05-30 07:32] Horsie: Seen in: ntfs.sys

[2024-05-30 14:50] daax: [replying to Horsie: "Does anyone know what the 0x210 (528 dec) pool typ..."]
if it has 200h in it i’d assume NonPagedPoolNx | some_internal_type (reserved). maybe <@148095953742725120> can discern

[2024-05-30 15:56] JustMagic: [replying to Horsie: "Does anyone know what the 0x210 (528 dec) pool typ..."]
Which flavor of pool type are we talking about

[2024-05-30 17:31] mibho: https://i.gyazo.com/b774da4cc430a47588cda451f3f1d194.png

(large fs:4 - large fs:8) <- this is the part making me <:nickyoung:783940642945368104> but is this (see above) cleaning/adjusting the stack?

[2024-05-30 17:47] ᲼᲼: on x86 the `fs` segment register will contain the teb/tib for the current thread in user-mode
it's doing `stack_base_stack_bottom_high_addr = teb->nt_tib.StackBase` ( start/base of the stack, the highest address ) `fs:4`
`stack_limit_stack_ceiling_low_addr = teb->nt_tib.StackLimit` ( end, lowest address ) `fs:8`

[2024-05-30 21:45] gogo: Hello. Can I code a malware using GetProcAddress(Kernel32.dll, WinExec) or must I call Kernel32.dll directly please?

[2024-05-30 22:05] x86matthew: pretty sure malware is the only thing keeping WinExec() alive in 2024

[2024-05-30 22:08] snowua: > This function is provided only for compatibility with 16-bit Windows.
<:kekw:904522300257345566>

[2024-05-30 22:28] Torph: 😭 I've never heard of this

[2024-05-30 22:37] dullard: [replying to gogo: "Hello. Can I code a malware using GetProcAddress(K..."]
surely trolling

[2024-05-30 22:37] dullard: mutual servers, OSDev, Cryptohack, Awesome Fuzzing <:msface_with_raised_eyebrow:835897311656673320> doesn't add up <:KEKW:846712430079770625>

[2024-05-31 08:03] gogo: [replying to dullard: "surely trolling"]
I have a macro to call functions from dlls. I would like to reuse same function for better code

[2024-05-31 08:03] gogo: instead of using 2 different functions

[2024-05-31 10:02] Timmy: bro is effortlessly causing my brain cells to commit sooside one by one

[2024-05-31 12:21] avx: atleast u had multiple before that

[2024-05-31 12:21] avx: 😔

[2024-05-31 14:46] Torph: [replying to gogo: "I have a macro to call functions from dlls. I woul..."]
isn't using GetProcAddress, saving that ptr, then calling the function more work than just linking to it?

[2024-05-31 14:54] dullard: [replying to Torph: "isn't using GetProcAddress, saving that ptr, then ..."]
He wants a malwares!

[2024-05-31 14:55] Brit: famously a sign of good malware is usage of getprocaddr

[2024-05-31 14:57] avx: yes very important

[2024-05-31 14:58] avx: rule 28450: always use getprocaddress

[2024-05-31 18:00] diversenok: Even to get address of GetProcAddress

[2024-05-31 19:20] avx: very important

[2024-05-31 19:24] JustMagic: it's very important to use GetProcAddress to get GetProcAddress for extra safety

[2024-05-31 19:39] brymko: ```c

// poc for increased opsec 

int main() {
  auto get_ProcAdrress_ = rand() ^ (decltype(GetProcAddress))GetProcAddress(GetModuleHandle("kernel32.dll"), "GetProcAddress"); 
  auto nt_write_PROCESS_memory = (rand() ^ get_ProcAdrress_)("NTDLL.dll", "NtWriteProcessMemory"):
}
```

[2024-05-31 19:41] Terry: for free??!

[2024-05-31 19:43] sf: [replying to brymko: "```c

// poc for increased opsec 

int main() {
  ..."]
Does this bypass windows defender? 🥹

[2024-05-31 19:44] brymko: this is propriritary code used in various high level intelligence operation

[2024-05-31 19:44] sf: Marvelous

[2024-05-31 22:25] hxm: <@162611465130475520>
[Attachments: image.png]

[2024-05-31 22:32] mrexodia: [replying to hxm: "<@162611465130475520>"]
nice work?