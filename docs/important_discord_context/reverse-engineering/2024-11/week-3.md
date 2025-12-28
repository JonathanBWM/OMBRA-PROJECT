# November 2024 - Week 3
# Channel: #reverse-engineering
# Messages: 139

[2024-11-11 23:11] Deleted User: Does anyone know the NTAPI equivalent of SetProcessMitigationPolicy

[2024-11-11 23:26] dullard: [replying to Deleted User: "Does anyone know the NTAPI equivalent of SetProces..."]
open kernelbase.dll in IDA and look at what it calls

[2024-11-11 23:26] Deleted User: [replying to dullard: "open kernelbase.dll in IDA and look at what it cal..."]
thank you

[2024-11-11 23:28] dullard: NtSetInformationProcess probably

[2024-11-11 23:30] dullard: Yeah it is

[2024-11-11 23:30] dullard: 
[Attachments: message.txt]

[2024-11-11 23:46] Deleted User: oh neat thanks a lot

[2024-11-12 00:59] diversenok: [replying to Deleted User: "Does anyone know the NTAPI equivalent of SetProces..."]
https://ntdoc.m417z.com/processinfoclass#processmitigationpolicy-52
[Embed: PROCESSINFOCLASS - NtDoc]
PROCESSINFOCLASS - NtDoc, the native NT API online documentation

[2024-11-12 01:03] Deleted User: [replying to diversenok: "https://ntdoc.m417z.com/processinfoclass#processmi..."]
life saver

[2024-11-12 08:25] 0x208D9: anyone know how i can add the windows cert to efi authentication?

[2024-11-12 08:26] 0x208D9: also the typedef of windows cert uefi guid as well

[2024-11-12 11:39] elias: [replying to 0x208D9: "anyone know how i can add the windows cert to efi ..."]
wdym?

[2024-11-12 11:39] elias: secure boot db?

[2024-11-12 11:42] 0x208D9: [replying to elias: "secure boot db?"]
yeah basically the uefi CA

[2024-11-12 11:42] 0x208D9: from windows

[2024-11-12 11:43] elias: [replying to 0x208D9: "yeah basically the uefi CA"]
it‘s already in the db for any consumer motherboard <:peepoDetective:570300270089732096>

[2024-11-12 11:43] 0x208D9: [replying to elias: "it‘s already in the db for any consumer motherboar..."]
yeah but do u know how it loads it for the EFI authentication?

[2024-11-12 11:44] elias: huh

[2024-11-12 11:44] elias: not sure what you mean

[2024-11-12 11:46] 0x208D9: [replying to elias: "not sure what you mean"]
like i wanna know how the secure boot keys can be updated by windows and how i can verify if i have the CA in the db

[2024-11-12 11:47] 0x208D9: basically the process of the bootloader updates

[2024-11-12 11:47] elias: [replying to 0x208D9: "like i wanna know how the secure boot keys can be ..."]
you can see the currently trusted certs using GetVariable efi runtime service on „db“ variable in default namespace and set new certs using SetVariable

[2024-11-12 11:48] elias: but its authenticated variable so your code needs to be signed by KEK in order to be able to modify secure boot db

[2024-11-12 11:49] 0x208D9: [replying to elias: "but its authenticated variable so your code needs ..."]
fml, no ways to do otherwise? i just wanna hook the update method thats all dont need to modify the db per say

[2024-11-12 11:49] 0x208D9: [replying to 0x208D9: "basically the process of the bootloader updates"]
.

[2024-11-12 11:50] elias: [replying to 0x208D9: "fml, no ways to do otherwise? i just wanna hook th..."]
iirc the update happens on boot

[2024-11-12 11:50] elias: if secure boot is on, you dont have code exeuction at this point

[2024-11-12 11:51] elias: whats your goal? <:peepoDetective:570300270089732096>

[2024-11-12 12:15] 0x208D9: [replying to elias: "whats your goal? <:peepoDetective:5703002700897320..."]
reverse engineering the updater and patcher

[2024-11-12 12:16] elias: well look into bootmgfw

[2024-11-12 12:18] 0x208D9: [replying to elias: "well look into bootmgfw"]
i did but static analysis isnt helping so i wanna hook the function and check if that makes sense

[2024-11-12 12:21] elias: So the way I understand it is Windows receives new database entries as signed blobs through Windows update and stores them in the system folder. When a certain registry key is set, the bootmanager will apply the updates at boot. The actual db update is just a call to SetVariable

[2024-11-12 12:22] elias: but Im sure <@1270065858323939338> has better understanding than me

[2024-11-12 12:23] 0x208D9: aight , it would be really helpful if u can gimme a blog? or the function address u are referring to?

[2024-11-12 12:23] elias: function address?

[2024-11-12 12:23] 0x208D9: the function which recieves the updates and stores em

[2024-11-12 12:24] 0x208D9: oh no nvm, i misunderstood what u meant, my bad

[2024-11-12 12:24] 0x208D9: i thought it makes network requests directly in the bootmgr

[2024-11-12 12:25] elias: no

[2024-11-12 12:25] elias: they are stored on disk before

[2024-11-12 12:25] 0x208D9: i see, interesting

[2024-11-12 12:26] 0x208D9: [replying to elias: "they are stored on disk before"]
so i can see it anyways , without messing with secure boot right???

[2024-11-12 12:27] 0x208D9: the thing is idk if the updater checks for signature or else i would have modified the patches to hook those functions

[2024-11-12 12:28] elias: [replying to 0x208D9: "so i can see it anyways , without messing with sec..."]
ye you can see the blobs under Windows/SecureBoot or Windows/System32/SecureBoot

[2024-11-12 12:28] elias: i dont remember which one

[2024-11-12 12:28] elias: [replying to 0x208D9: "the thing is idk if the updater checks for signatu..."]
that shouldnt be possible because that would be a security vulnerability

[2024-11-12 12:28] elias: are you trying to add your own cert to db?

[2024-11-12 12:29] 0x208D9: [replying to elias: "are you trying to add your own cert to db?"]
no, im just trying to see the delta patching method inside the bootmgr

[2024-11-12 12:29] Rairii: EFI SetVariable does all the sigchecks

[2024-11-12 12:30] Rairii: db and dbx updates are signed as AppendWrite

[2024-11-12 12:31] 0x208D9: i didnt even knew these as its a pretty new territory for me, ima look around more to see if i can hook it by disabling secure boot in that case (but has no point cuz i dont think it works without secure boot)

[2024-11-12 12:31] Rairii: still wont work

[2024-11-12 12:32] Rairii: the only way you can touch them without sigchecks is in uefi setup which runs before ReadyToBoot event

[2024-11-12 12:32] 0x208D9: [replying to Rairii: "the only way you can touch them without sigchecks ..."]
can u elaborate on that?

[2024-11-12 12:32] Rairii: the PE inside uefi firmware that handles booting from disk

[2024-11-12 12:33] Rairii: raises uefi ReadyToBoot event

[2024-11-12 12:33] 0x208D9: so i have to modify the firmware itself?

[2024-11-12 12:33] Rairii: which locks out any chance of touching authenticated variables without sigcheck

[2024-11-12 12:33] elias: [replying to 0x208D9: "so i have to modify the firmware itself?"]
you cant

[2024-11-12 12:33] elias: with proper protections

[2024-11-12 12:33] Rairii: this happens in smm (amd64) or trustzone (arm/arm64)

[2024-11-12 12:34] Rairii: which is where the setvariable code is implemented too

[2024-11-12 12:34] 0x208D9: [replying to elias: "you cant"]
idk if that works but doesnt virtualbox has its own implementation of the uefi?

[2024-11-12 12:34] 0x208D9: where it handles the boot?

[2024-11-12 12:35] elias: huh

[2024-11-12 12:36] 0x208D9: [replying to elias: "huh"]
https://forums.virtualbox.org/viewtopic.php?t=107383

they surely have something implemented to handle those??

[2024-11-12 12:36] Rairii: [replying to 0x208D9: "idk if that works but doesnt virtualbox has its ow..."]
yes, as does vmware and hyper-v

[2024-11-12 12:37] 0x208D9: [replying to Rairii: "yes, as does vmware and hyper-v"]
right so technically i can modify the virtualbox implementation and touch those areas right?

[2024-11-12 12:37] Rairii: yes

[2024-11-12 12:38] 0x208D9: makes sense, thanks will try and see if there are any additional caveats

[2024-11-12 12:38] 0x208D9: thanks for the help and time 😄

[2024-11-12 12:51] KnightCoder: Help me make simple game mod
I want to add a combo box on game setting window.
I'll pay $200 for this work.
This is paid work; if you're interested, send DM, please

[2024-11-12 13:57] 0x208D9: dunno seems like microsoft techsupport

[2024-11-12 13:58] Humza: Bros name is not robert

[2024-11-12 13:59] 0x208D9: ^^ someone got the reference

[2024-11-12 14:02] Humza: https://tenor.com/view/scammer-scam-redeeming-whyareyouredeeming-kitboga-gif-19964723

[2024-11-12 15:06] vykt: sir do not redeem

[2024-11-12 16:57] expy: is there no HEAP struct in phnt or am I missing something?  https://www.vergiliusproject.com/kernels/x64/windows-11/23h2/_HEAP

[2024-11-12 17:06] diversenok: [replying to expy: "is there no HEAP struct in phnt or am I missing so..."]
It changes too often to put into phnt; some portions of it are in another header: https://github.com/winsiderss/systeminformer/blob/master/SystemInformer/include/heapstruct.h

[2024-11-12 23:52] Torph: [replying to Humza: "https://tenor.com/view/scammer-scam-redeeming-whya..."]
<:kekw:904522300257345566>

[2024-11-13 13:45] jl: ill tell anyone 4 different ways of breaking secure boot if they call me

[2024-11-13 13:45] jl: im denied talking with people- and i have nothing to loose

[2024-11-13 15:43] flower: poor guy

[2024-11-13 15:43] flower: hes just lonely

[2024-11-13 15:59] Windy Bug: <@651054861533839370>

[2024-11-13 16:07] Humza: [replying to jl: "im denied talking with people- and i have nothing ..."]
https://tenor.com/view/my-man-trust-chest-pound-brotherhood-gif-17669441

[2024-11-13 16:07] Humza: I’ll call u if u want bro

[2024-11-13 18:55] 0x208D9: [replying to jl: "ill tell anyone 4 different ways of breaking secur..."]
im afraid to call ya

[2024-11-13 18:56] 0x208D9: cuz u too good and i have imposter syndrome

[2024-11-13 19:07] naci: [replying to jl: "ill tell anyone 4 different ways of breaking secur..."]
dont wait for someone else to call you, pick up the phone and dial random numbers and make them listen

[2024-11-13 19:20] jl: i will not

[2024-11-13 19:20] jl: i will just kill myself

[2024-11-13 19:21] 0x208D9: [replying to jl: "i will just kill myself"]
dont

[2024-11-13 19:22] jl: then GIVE ME A FUCKING CLANG COMPILER THAT WORKS

[2024-11-13 19:22] 0x208D9: mission impossible : clang

[2024-11-13 19:24] contificate: [replying to jl: "then GIVE ME A FUCKING CLANG COMPILER THAT WORKS"]
that's crazy bro you must use a toy operating system not intended for developer use
[Attachments: 2024-11-13-192402_861x410_scrot.png]

[2024-11-13 19:31] 0x208D9: [replying to contificate: "that's crazy bro you must use a toy operating syst..."]
there is also clang-cl

[2024-11-13 21:07] Matti: [replying to jl: "im denied talking with people- and i have nothing ..."]
to* people
lose*

[2024-11-13 21:07] Matti: [replying to jl: "i will just kill myself"]
tldr, also this is off topic

[2024-11-13 21:07] Matti: [replying to jl: "then GIVE ME A FUCKING CLANG COMPILER THAT WORKS"]
finally

[2024-11-13 21:07] Matti: how does it not work

[2024-11-13 21:07] jl: i am sorry

[2024-11-13 21:08] jl: well- i am trying to finish this poc of c++ features in c

[2024-11-13 21:08] jl: 
[Attachments: message.txt]

[2024-11-13 21:09] Matti: looks great, what's wrong

[2024-11-13 21:09] jl: i got encapsulation, constructors, cast operator, member functions working

[2024-11-13 21:09] jl: i dont have admin privs on my pc

[2024-11-13 21:09] jl: so i cant install visual studio

[2024-11-13 21:10] jl: i just need to be able to run clang

[2024-11-13 21:10] Matti: seems like whoever is the admin on your pc made a wise decision

[2024-11-13 21:10] jl: maybe

[2024-11-13 21:10] jl: maybe not

[2024-11-13 21:10] jl: i still want a clang

[2024-11-13 21:11] Matti: oh I see, I get the misunderstanding now

[2024-11-13 21:11] Matti: clang works

[2024-11-13 21:11] Matti: but your pc doesn't

[2024-11-13 21:11] Matti: I'm sorry but this isn't really a tech support server

[2024-11-13 21:12] jl: its fine thank you for your time

[2024-11-13 21:13] jl: at least you read it- im happy just knowing that

[2024-11-13 22:34] Matti: look man, I'll give you a pass since you joined 2 days ago
but this isn't really a server or at least not the channel for posting meme gifs either

[2024-11-13 22:35] Matti: unless it's a jonas level gif that BSODs windows or something maybe

[2024-11-13 22:37] Humza: I see, my bad

[2024-11-13 22:37] Humza: It won’t happen again

[2024-11-14 02:28] James: A functioning c++ exception supporting compiler based on llvm?

[2024-11-14 02:28] James: Impossible

[2024-11-14 02:59] Deleted User: [replying to James: "A functioning c++ exception supporting compiler ba..."]
Hey CodeDefender

[2024-11-14 02:59] James: 🙏

[2024-11-14 02:59] Deleted User: <:FancyPepe:1281320050145366067>

[2024-11-16 20:23] EfraimDays: Guys, I'm watching some tutorials about reverse engineering, and they use reclass.net, but it looks a little outdated to me. Is this tool still used? Do I need to complement it with a plugin to make it undetectable if I want to use it? Are there any better tools than this?

[2024-11-16 21:04] Torph: wdym "undetectable"?

[2024-11-16 22:55] EfraimDays: [replying to Torph: "wdym "undetectable"?"]
The target process can block memory reading, so I don't know if ReClass.NET is still useful or if there is a better tool.

[2024-11-16 22:57] Humza: Y not patch out the part where it tries to block memory reading

[2024-11-16 22:58] EfraimDays: I see it has this plugin, but the last commit was 5 years ago
https://github.com/niemand-sec/ReClass.NET-DriverReader
[Embed: GitHub - niemand-sec/ReClass.NET-DriverReader: Plugin for ReClass.N...]
Plugin for ReClass.Net (using vulnerable driver to read process memory) - niemand-sec/ReClass.NET-DriverReader

[2024-11-16 22:58] Humza: Like nop it or use a gadget to jump past it maybe

[2024-11-16 22:58] Humza: Do u need dynamic analysis for this one

[2024-11-16 23:01] EfraimDays: [replying to Humza: "Do u need dynamic analysis for this one"]
Yes, to reply to the tutorial that I'm watching. But the question is, is ReClass.NET useful for modern games?

[2024-11-17 08:20] Deleted User: Yeah it is

[2024-11-17 08:20] Deleted User: But not if you just download it lol

[2024-11-17 09:58] k: ```un3thical@DESKTOP-GAQ7TRH:~/binwalk/target/extractions/firmware/20400/_decompressed.bin.extracted$ cd _300010.extracted/
un3thical@DESKTOP-GAQ7TRH:~/binwalk/target/extractions/firmware/20400/_decompressed.bin.extracted/_300010.extracted$ ls                                                                              0      28E59  9340  9578C  96597         _10600.extracted  _28E5D.extracted  _9540.extracted   _957A0.extracted                                                                                                                                                                                                              10600  28E5D  9540  957A0  _0.extracted  _28E59.extracted  _9340.extracted   _9578C.extracted  _96597.extracted                                                                                                                                                                                                              un3thical@DESKTOP-GAQ7TRH:~/binwalk/target/extractions/firmware/20400/_decompressed.bin.extracted/_300010.extracted$ cd _0.extracted/                                                                                                                                                                                        un3thical@DESKTOP-GAQ7TRH:~/binwalk/target/extractions/firmware/20400/_decompressed.bin.extracted/_300010.extracted/_0.extracted$ ls                                                                                                                                                                                         0  10600  28E59  28E5D  9340  9540  9578C  957A0  96597                                                                                                                                                                                                                                                                      un3thical@DESKTOP-GAQ7TRH:~/binwalk/target/extractions/firmware/20400/_decompressed.bin.extracted/_300010.extracted/_0.extracted$```
i cant understand why this happens. is it obfuscation or something? It have _decompressed, that folder have compressed data over and over again. Like a loop