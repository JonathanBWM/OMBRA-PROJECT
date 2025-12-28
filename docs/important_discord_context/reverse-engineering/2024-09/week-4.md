# September 2024 - Week 4
# Channel: #reverse-engineering
# Messages: 196

[2024-09-16 01:30] crash: Does anyone know a place in ntoskrnl to perform a .text patch and log all syscalls?

[2024-09-16 01:38] abu: KiSystemCall64

[2024-09-16 01:39] estrellas: isnt it protected by patchguard?

[2024-09-16 01:59] abu: [replying to estrellas: "isnt it protected by patchguard?"]
he asking for text patch

[2024-09-16 01:59] crash: Yeah thanks, I guess this is the only sure way

[2024-09-16 13:47] khan |Blending code & creativity: Hallo all friends, any one looking for a freelancer ( game reverse engineering expert ) hmu.

[2024-09-17 08:22] 0xboga: Anyone willing to share his custom TIL for IDA? especially for kernel RE

[2024-09-17 11:30] Matti: [replying to 0xboga: "Anyone willing to share his custom TIL for IDA? es..."]
sure, have any of these beautiful TILs here FREE of charge (offer only valid today)
[Attachments: wdk10_win10_km_64.til, kmdf_1_33_64.til, nt_ros_64_10.til, uefi_edk2_64.til]

[2024-09-17 11:31] Matti: but, do you know that saying that's like... something like... about teaching a man how to fish?

[2024-09-17 11:31] Matti: https://hex-rays.com/tutorials/idaclang/
[Embed: Using the IDAClang plugin for IDA Pro]
A powerful disassembler and a versatile debugger

[2024-09-17 11:32] Matti: the TILs above were (and still are) made with tilib, because idaclang didn't exist yet at the time

[2024-09-17 11:33] Matti: it was an absolute fucking pain in the ass

[2024-09-17 11:33] Matti: and the TILs aren't better for it, more like the opposite

[2024-09-17 11:34] Matti: nowadays you can just feed an entire SDK, or proprietary code of questionable legality, into idaclang and it will spit out a TIL for you without falling over every other semicolon

[2024-09-17 11:36] Matti: FREE tip #2: combine .sig files made with sigmake with .til files made with idaclang for best results

[2024-09-17 11:43] Matti: [replying to 0x208D9: "https://github.com/Dump-GUY/IDA_PHNT_TYPES"]
<@836981274496991252> this is an excellent example of using idaclang for the exact thing you're asking for, in case you missed it

[2024-09-17 11:45] Matti: based solely on filesize I'm pretty sure my nt_ros_64_10 TIL contains 50-100% more types than the phnt64_win11.til from this repo
but consider that those types are much more up to date

[2024-09-17 11:46] Matti: not to mention actually maintainable

[2024-09-17 11:47] Matti: unlike anything you'll need to hack up to the point where tilib will parse it without complaints

[2024-09-17 11:54] 0x208D9: [replying to Matti: "nowadays you can just feed an entire SDK, or propr..."]
idaclang is gold, those days were actually horrifying where u had to do it manually

[2024-09-17 11:54] Matti: I know

[2024-09-17 11:54] Matti: `#ifdef __TILIB_HACKS__`

[2024-09-17 13:26] expy: hello, is there a way in Unicorn to recover after exception without re-creating the instance? Just updating the RIP and continue with `emu_start` brings up previous exception.

[2024-09-17 13:33] mrexodia: [replying to expy: "hello, is there a way in Unicorn to recover after ..."]
There is a secret way

[2024-09-17 13:33] mrexodia: <:shush:818975290977812501>

[2024-09-17 13:34] mrexodia: But you should use https://github.com/icicle-emu/icicle-python
[Embed: GitHub - icicle-emu/icicle-python: Python bindings for the Icicle e...]
Python bindings for the Icicle emulator. Contribute to icicle-emu/icicle-python development by creating an account on GitHub.

[2024-09-17 16:06] zeropio: hey, has anyone tried the sdk for the ida 9 beta? 👀

[2024-09-17 16:12] Deleted User: ida 9 is buggy in general, I wouldn't use it

[2024-09-17 16:13] Brit: blackhat leak user be like

[2024-09-17 16:13] Brit: <:mmmm:904523247205351454>

[2024-09-17 16:14] zeropio: idk what are you talking about, I'm testing a legal beta

[2024-09-17 16:14] zeropio: <:copium:1201998428066816091>

[2024-09-17 16:15] Brit: they've updated the beta since the "it's crashing" is kind of a self report is all I'm saying

[2024-09-17 16:18] Deleted User: [replying to Brit: "blackhat leak user be like"]
were you talking to me lol

[2024-09-17 16:18] Deleted User: I haven't used Ida 9, it's just what I've heard from someone who has

[2024-09-17 16:20] Brit: ¯\_(ツ)_/¯

[2024-09-17 16:52] Delirium Mode: Asa: So I am recently bridging in to reverse engineering as a hobby. I have some decent understanding of c++

I am currently wondering why when I print the address of a function is it different to the address in a debugger like IDA?

[2024-09-17 16:55] Brit: how is ida supposed to know where windows loaded your process at

[2024-09-17 16:56] Brit: you can always just rebase your your bin in ida to wherever the running one is

[2024-09-17 17:05] brymko: the address knows where it is by knowing where it isn't. By subtracting where it isn't to where it was it can calculate where it is

[2024-09-17 17:09] Delirium Mode: Asa: Obviously I am still missing some core fundementals, I will take what you have both just said and apply it to google, thank you.

[2024-09-17 17:14] Delirium Mode: Asa: Ah I see when i run IDA debugger the address has changed, that makes sense.

[2024-09-17 17:16] Delirium Mode: Asa: Shooting in the dark here;

So if I know my program starts at 0, and the function is at 4 in IDA, can I infer this difference?

If I run the program in windows and the start in memory is say 5, knowing this then I know my function is at 9?

[2024-09-17 17:17] Brit: the relative address to modulebase is generally what people use when they talk about a function's address

[2024-09-17 17:18] Brit: because windows might load anything anywhere in vmem

[2024-09-17 17:18] Delirium Mode: Asa: That makes sense.

[2024-09-17 23:20] Tr1x: [replying to expy: "hello, is there a way in Unicorn to recover after ..."]
RIP + Exception Instruction Length then emu_start

[2024-09-17 23:58] expy: [replying to Tr1x: "RIP + Exception Instruction Length then emu_start"]
what's Exception Instruction Length?

[2024-09-17 23:58] hxm: i noticed on some cases even destorying the whole IAT,

 ntdll is still able to map the imports and write each func address on the section ... ? anyone has an explanation
[Attachments: image.png]

[2024-09-18 00:04] expy: <@692740168196685914>  you've confused IAT (Import Address Table) with import directory, there is still a pointer in import data directory to IAT

[2024-09-18 00:09] hxm: [replying to expy: "<@692740168196685914>  you've confused IAT (Import..."]
yes but apparently the value of Import Address Table is useless

[2024-09-18 08:44] 0x208D9: [replying to zeropio: "hey, has anyone tried the sdk for the ida 9 beta? ..."]
imho idek what made hexrays think it would be a good idea to replace ida_structs with udm and udts

[2024-09-18 08:44] 0x208D9: and remove enums all together and merge those into typeinfo

[2024-09-18 08:44] 0x208D9: like wtf

[2024-09-19 01:26] emma: how do i reverse windows shellcode?

[2024-09-19 02:16] expy: [replying to emma: "how do i reverse windows shellcode?"]
compile it, jump on it with a breakpoint in the debugger, alternatively open it in ida and press 'c' (for code)

[2024-09-19 09:45] dullard: [replying to emma: "how do i reverse windows shellcode?"]
BlobRunner is decent

[2024-09-19 09:46] dullard: https://github.com/OALabs/BlobRunner
[Embed: GitHub - OALabs/BlobRunner: Quickly debug shellcode extracted durin...]
Quickly debug shellcode extracted during malware analysis - OALabs/BlobRunner

[2024-09-19 22:20] Delirium Mode: Asa: Doing some learning before bed, attempting to call a hidden func inside a simple console app.
Is this sane? Am i missing something?
[Attachments: devenv_DkB4Hq0o7O.png, ida64_i1VvB8J62i.png]

[2024-09-19 22:29] Deleted User: [replying to Delirium Mode: Asa: "Doing some learning before bed, attempting to call..."]
make that 0x117E0

[2024-09-19 22:29] Deleted User: in the code

[2024-09-19 22:30] Deleted User: assuming the imagebase of that executable is 0x140000000

[2024-09-19 22:30] Deleted User: because you're offseting from the base

[2024-09-19 22:30] Delirium Mode: Asa: Could you expain how I am reading it wrong?

[2024-09-19 22:30] Delirium Mode: Asa: Ah

[2024-09-19 22:30] Deleted User: can you scroll all the way up in ida and uhh

[2024-09-19 22:30] Deleted User: there should be a comment field called image base

[2024-09-19 22:31] Deleted User: you basically substract that from address you see on the left in ida view

[2024-09-19 22:31] Delirium Mode: Asa: Imagebase   : 140000000

[2024-09-19 22:31] Delirium Mode: Asa: Ahh fuck

[2024-09-19 22:31] Delirium Mode: Asa: I did not know that

[2024-09-19 22:31] Delirium Mode: Asa: So whats the deal with IDA applying an image base and not starting at 0000000, I assume its done for a reason?

[2024-09-19 22:32] Brit: [replying to Brit: "because windows might load anything anywhere in vm..."]
we literally had this conversation a few days ago

[2024-09-19 22:32] Brit: you can just substract imagebase

[2024-09-19 22:32] Brit: from the addr you yoinked from ida

[2024-09-19 22:33] Delirium Mode: Asa: I thought I had it already 0 based, which obviously I do not

[2024-09-19 22:36] Delirium Mode: Asa: Still crashing but that is one less issue, thank you.

[2024-09-19 22:39] selfprxvoked: [replying to Delirium Mode: Asa: "Still crashing but that is one less issue, thank y..."]
ASLR

[2024-09-19 22:49] Delirium Mode: Asa: ASLR?

[2024-09-19 22:51] Para: [replying to Delirium Mode: Asa: "ASLR?"]
Address Space Layout Randomization

[2024-09-19 22:52] iPower: [replying to Delirium Mode: Asa: "I thought I had it already 0 based, which obviousl..."]
no you need to rebase it in ida

[2024-09-19 22:52] Delirium Mode: Asa: [replying to Para: "Address Space Layout Randomization"]
Ah yeah I read a tiny bit about this

[2024-09-19 22:52] Delirium Mode: Asa: [replying to iPower: "no you need to rebase it in ida"]
I thought I had already done that but apparently not

[2024-09-19 22:53] iPower: ida will always load the binary at the preferred image base unless you rebase it yourself

[2024-09-19 22:53] iPower: just go to Edit -> Segments -> Rebase program

[2024-09-19 22:54] Delirium Mode: Asa: Ah thank you so much! Super novice up here so some majorly basic things going over my head

[2024-09-19 23:04] Matti: I'd generally avoid rebasing anything in IDA (because I don't trust it enough), so unless this is a dump I'd just let IDA load it at the preferred image base in the headers, i.e. the default value

[2024-09-19 23:04] Matti: just subtract the image base, it's not hard

[2024-09-19 23:04] Matti: or use something like https://github.com/RomanRybachek/Copy_RVA
[Embed: GitHub - RomanRybachek/Copy_RVA: Plugin for ida pro that copies RVA...]
Plugin for ida pro that copies RVA under cursor to clipboard. - RomanRybachek/Copy_RVA

[2024-09-19 23:05] iPower: [replying to Matti: "I'd generally avoid rebasing anything in IDA (beca..."]
yeah i usually just run `address - idaapi.get_imagebase()`

[2024-09-19 23:05] iPower: rebasing often breaks vtable references

[2024-09-19 23:05] Matti: for dumps you will always want to load them at the runtime address, especially since relocation info is probably gone anyway

[2024-09-20 17:28] emma: is there a way to x32dbg to resolve ntdll function names?

[2024-09-20 17:28] emma: get

[2024-09-20 17:31] emma: 
[Attachments: image.png]

[2024-09-20 17:32] dullard: [replying to emma: "is there a way to x32dbg to resolve ntdll function..."]
Are your symbols borked ?

[2024-09-20 17:33] emma: [replying to dullard: "Are your symbols borked ?"]
How do i check?

[2024-09-20 20:35] x86matthew: right click, download symbols

[2024-09-20 20:39] emma: Neat, thanks

[2024-09-20 20:52] 0x208D9: ||anyone have the tilib64.exe available with them for IDA 9.0? (if this is against the rules ima remove)||

[2024-09-20 20:59] Rairii: [replying to 0x208D9: "||anyone have the tilib64.exe available with them ..."]
isn't the entire thing on archive.org somewhere

[2024-09-20 21:00] 0x208D9: [replying to Rairii: "isn't the entire thing on archive.org somewhere"]
thats the only one utility which wasnt archived

[2024-09-20 21:00] 0x208D9: and i cant check if the types are loaded in my til

[2024-09-20 21:00] 0x208D9: so if anyone can volunteer for checking that, it would also be fine

[2024-09-20 21:08] emma: Is there a way to set an x32dbg breakpoint when entering a module

[2024-09-20 21:10] 6bd835a1d0095059128d4d8cf6d16171: [replying to 0x208D9: "||anyone have the tilib64.exe available with them ..."]
don’t think it was present

[2024-09-20 21:10] 6bd835a1d0095059128d4d8cf6d16171: i seem to remember it 404ing

[2024-09-20 21:10] 0x208D9: oh ok

[2024-09-20 22:39] Delirium Mode: Asa: Can anyone recommend a dll injector with a GUI? having too mnany issues with extreme injector

[2024-09-20 22:41] Azrael: [replying to Delirium Mode: Asa: "Can anyone recommend a dll injector with a GUI? ha..."]
Stop cheating in games.

[2024-09-20 22:42] Delirium Mode: Asa: I am not cheating in games.

[2024-09-20 22:42] Azrael: Whatever you say.

[2024-09-20 22:42] Delirium Mode: Asa: Okay lmao

[2024-09-20 22:43] Azrael: [replying to Delirium Mode: Asa: "Okay lmao"]
Anyways, just use something like kdmapper.

[2024-09-20 22:44] Delirium Mode: Asa: I am just looking to keep things simple and inject a dll right now.

[2024-09-20 22:44] Azrael: Oh, so no cheating?

[2024-09-20 22:45] Delirium Mode: Asa: I am learning to use IDA as a hobby, I built a simple console with some basic functions inside, and I am attempting to call them using an external dll I am injecting.

[2024-09-20 22:45] Delirium Mode: Asa: The most bare bone simple example you could ever imagine.

[2024-09-20 22:46] irql: if you dont need all the goofy features with things like extreme injector -- old process hacker versions have it

[2024-09-20 22:46] Delirium Mode: Asa: Yeah i don't think i need the features, extreme injector is just causing the app to crash anyway.

[2024-09-20 22:46] irql: I have 2.39 & it has it under Misc on the context menu

[2024-09-20 22:46] Azrael: Oh yeah, I forgot about that one.

[2024-09-20 22:47] iPower: [replying to Delirium Mode: Asa: "I am just looking to keep things simple and inject..."]
you can always use cheat engine

[2024-09-20 22:47] Delirium Mode: Asa: To inject a dll?

[2024-09-20 22:47] iPower: yeah

[2024-09-20 22:47] irql: I didnt even know they have that lmfao

[2024-09-20 22:47] Delirium Mode: Asa: Had no idea that could do that wtf

[2024-09-20 22:47] iPower: attach to process, go to memory browser and inject a dll

[2024-09-20 22:48] Azrael: You can also just inject code straight up.

[2024-09-20 22:49] Delirium Mode: Asa: Oh fricc I see it

[2024-09-20 22:49] iPower: [replying to irql: "if you dont need all the goofy features with thing..."]
system informer still has it, just under different options

[2024-09-20 22:49] irql: oh fr? 🤣

[2024-09-20 22:49] irql: i have both versions

[2024-09-20 22:49] Azrael: Oh, where?

[2024-09-20 22:49] iPower: yeah. go to modules -> options -> load module

[2024-09-20 22:49] irql: I thought they had to scrap some things

[2024-09-20 22:49] irql: oh lmfao

[2024-09-20 22:49] irql: oh yea

[2024-09-20 22:50] Delirium Mode: Asa: Ahh still crashing, prob bad code.
I will dive in to it tomorrow, super tired from work.

[2024-09-20 22:51] Azrael: Oh yeah.

[2024-09-20 22:51] Azrael: It actually does.

[2024-09-20 22:53] iPower: [replying to irql: "I didnt even know they have that lmfao"]
cheat engine has everything

[2024-09-20 22:53] iPower: it's magical

[2024-09-20 22:53] iPower: even when I'm not doing game RE I still make heavy use of it

[2024-09-20 22:53] iPower: it's such a great tool for general purpose stuff

[2024-09-20 22:54] irql: I got a love hate relationship with it

[2024-09-20 22:54] irql: I am the same, I use it for a lot too lmfao

[2024-09-20 22:54] irql: but oh my god

[2024-09-20 22:54] irql: why's the symbol loading so broken for it

[2024-09-20 22:54] Brit: because pascal

[2024-09-20 22:54] Azrael: [replying to iPower: "it's such a great tool for general purpose stuff"]
The installer is sketchy as hell though.

[2024-09-20 22:55] irql: always causing the GUI to hang for me & idk why it takes so long?

[2024-09-20 22:55] irql: like damn, surely its got a symbol cache?

[2024-09-20 22:55] irql: eh

[2024-09-20 22:55] Azrael: I install it via chocolatey anyways so it doesn't really bother me anymore.

[2024-09-21 08:28] sp1derc4t: [replying to Azrael: "The installer is sketchy as hell though."]
build your own, it's open source

[2024-09-21 14:50] mrexodia: [replying to sp1derc4t: "build your own, it's open source"]
Mister funny over here

[2024-09-21 15:29] emma: I ❤️ x64dbg

[2024-09-21 15:29] emma: so much better than gdb

[2024-09-21 16:22] irql: <:gdb:992509370908811284>

[2024-09-22 13:31] MonTy: guys, are there any working ways to remove the obfuscation from the kernel driver?

[2024-09-22 13:33] MonTy: I use a hypervisor for tracing

[2024-09-22 13:33] MonTy: What should I do then? put it in some smt solver or triton?

[2024-09-22 13:47] MonTy: yes

[2024-09-22 17:01] Timmy: lol

[2024-09-22 17:17] rin: [replying to MonTy: "guys, are there any working ways to remove the obf..."]
You need to remove the polymorphic

[2024-09-22 17:17] rin: First

[2024-09-22 17:34] MonTy: [replying to rin: "You need to remove the polymorphic"]
polymorphic?

[2024-09-22 17:34] rin: Yes

[2024-09-22 17:35] MonTy: [replying to rin: "Yes"]
Can I be a little more specific

[2024-09-22 17:36] rin: Remove polymorphic > read assembly > open source

[2024-09-22 17:40] MonTy: I'm sorry, I have an obfuscated driver code. What does it mean to remove polymorphic?

[2024-09-22 17:43] MonTy: and are there any driver emulation tools? unicorn engine or x64dbg....

[2024-09-22 18:12] toasts: [replying to rin: "You need to remove the polymorphic"]
certified pirate software classic

[2024-09-22 18:28] Azrael: [replying to rin: "Remove polymorphic > read assembly > open source"]
You forgot about the fact that you need to unpolyglot the driver.

[2024-09-22 18:28] Azrael: Extrapolate the hidden kernel code.

[2024-09-22 18:35] Deleted User: you need to extract the kernel from the micro codes

[2024-09-22 18:36] Torph: [replying to toasts: "certified pirate software classic"]
what does this mean

[2024-09-22 18:38] MonTy: [replying to Deleted User: "you need to extract the kernel from the micro code..."]
please, you can learn more

[2024-09-22 18:40] Azrael: [replying to Deleted User: "you need to extract the kernel from the micro code..."]
You need to launch your CUDA kernels into space.

[2024-09-22 18:41] MonTy: [replying to Azrael: "You need to launch your CUDA kernels into space."]
please tell me how

[2024-09-22 18:43] Azrael: Do you have Elon Musk on speed dial?

[2024-09-22 18:43] MonTy: [replying to Azrael: "Do you have Elon Musk on speed dial?"]
Is this a joke?

[2024-09-22 18:44] Torph: [replying to MonTy: "please tell me how"]
they are messing with you bc your original post was really vague

[2024-09-22 18:44] Azrael: [replying to MonTy: "Is this a joke?"]
Yes.

[2024-09-22 18:45] Azrael: [replying to MonTy: "guys, are there any working ways to remove the obf..."]
Refactor your original question and you *might* get a serious response.

[2024-09-22 18:46] Torph: i would offer real advice if I could, but this isn't really my area

[2024-09-22 18:47] MonTy: [replying to Torph: "they are messing with you bc your original post wa..."]
most deobfuscation products are focused on use in the user space. I want to find out how to remove the obfuscation in the driver (kernel space). Maybe there are some kernel space in user space emulation products

[2024-09-22 19:15] toasts: [replying to MonTy: "most deobfuscation products are focused on use in ..."]
if you want to emulate a driver in usermode use https://github.com/mandiant/speakeasy or https://github.com/mrexodia/driver_unpacking by resident debugger expert <:give:835809674480582656>

[2024-09-22 19:19] MonTy: [replying to toasts: "if you want to emulate a driver in usermode use ht..."]
this is the first thing I tried. I came across spinlock, mutex, threads. Mrexodia said it no longer supports the project

[2024-09-22 20:26] luci4: The MSDN page for `RtlAddAcl` says:  `If an empty ACL is applied to an object, the ACL implicitly denies all access to that object`. I changed the DACL for a process to an empty ACL, and it indeed didn't let me do anything to it. What I don't understand is: why was I able to see the "normal" ACLs for the process after running procexp/task manager as admin?

[2024-09-22 20:26] luci4: I updated the security descriptor using `NtSetSecurityObject`, so the changes should have reflected in the object struct (?)

[2024-09-22 20:42] luci4: Oh, task manager has SeDebugPrivilege

[2024-09-22 20:42] luci4: That sort of explains it

[2024-09-22 21:37] Rairii: [replying to MonTy: "most deobfuscation products are focused on use in ..."]
for quick unpacking of a driver I actually used wine

[2024-09-22 21:37] Rairii: had to implement some stubs but other than that i could let it unpack and then dump