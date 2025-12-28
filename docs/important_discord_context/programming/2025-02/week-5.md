# February 2025 - Week 5
# Channel: #programming
# Messages: 37

[2025-02-24 05:19] vmx: [replying to UJ: "Do these videos look legit or a waste of time for ..."]
they're pretty decent, but writing your own headers is a waste of time

[2025-02-24 05:19] vmx: (in my opinion)

[2025-02-24 06:46] Horsie: Is there a way to read/write physical memory without admin from usermode (as admin) these days, without a memedriver?

[2025-02-24 06:46] Horsie: I thought PhysicalMemory device was available in the global namespace but it seems like that isn't the case anymore

[2025-02-24 06:47] Horsie: It doesn't have to be 'undetected', I'm not trying to deal with ACs.

[2025-02-24 07:01] Deleted User: [replying to Horsie: "Is there a way to read/write physical memory witho..."]
no supported way no

[2025-02-24 07:01] Horsie: Fair.. thanks

[2025-02-24 19:05] sync: [replying to Horsie: "Is there a way to read/write physical memory witho..."]
maybe try the csc.sys exploit?
https://github.com/michredteam/PoC-26229
[Embed: GitHub - michredteam/PoC-26229: Windows CSC Service Elevation of Pr...]
Windows CSC Service Elevation of Privilege Vulnerability - michredteam/PoC-26229

[2025-02-24 20:52] daax: [replying to Horsie: "Is there a way to read/write physical memory witho..."]
if someone has this it's very unlikely they would share it freely <:Kappa:794707301436358686>

[2025-02-24 20:53] daax: but if you're on asus | dell you can do this using their drivers lol (not memedrivers because they're not blacklisted/revoked) and come shipped with the board

[2025-02-25 09:56] NSG650: [replying to dlima: "Has anyone here written a hobby os kernel before?"]
Hi yes

[2025-02-25 09:57] NSG650: I know it's an old message

[2025-02-25 09:58] NSG650: But https://github.com/limine-bootloader/limine-c-template-x86-64
[Embed: GitHub - limine-bootloader/limine-c-template-x86-64: A simple templ...]
A simple template for building an x86-64 Limine-compliant kernel in C. - limine-bootloader/limine-c-template-x86-64

[2025-02-25 09:58] NSG650: You can start out from here

[2025-02-25 21:39] dlima: [replying to NSG650: "But https://github.com/limine-bootloader/limine-c-..."]
Yes I’ve been using this

[2025-02-25 21:39] dlima: Well the C++ template actually

[2025-02-25 21:39] dlima: Limine’s great

[2025-02-26 03:50] rin: does anyone have an opinion on executing shellcode using functions that take callbacks as parameters. does this even help with evasion in anyway since you are still allocating memory and marking it executable.

[2025-02-26 04:16] Torph: like the shellcode calls back to you? that seems convenient just for having less code in the shellcode

[2025-02-26 07:35] Timmy: i dont see how it'd help with evasion tho

[2025-02-26 07:54] rin: [replying to Timmy: "i dont see how it'd help with evasion tho"]
that's what I am saying but people still seem to do this.

[2025-02-26 07:55] rin: [replying to Torph: "like the shellcode calls back to you? that seems c..."]
no, I am talking about self injection.

[2025-02-26 07:56] rin: I assume if anything it's just for changing up the signature.

[2025-02-26 23:45] bugdigger: Sup,
How can i learn about polymorphic engines. How to code them? Some simple github projects for starters?

[2025-02-26 23:48] contificate: I bet most are effectively packers that repack themselves

[2025-02-26 23:49] contificate: as opposed to something that does more deep restructuring of the code

[2025-02-27 04:58] Horsie: [replying to daax: "if someone has this it's very unlikely they would ..."]
yeah i figured how silly the question was.

[2025-02-27 04:59] Horsie: i was just curious because i remember the physical memory object being a thing and microsoft claiming user-kernel not being a sec boundary

[2025-02-27 08:26] dullard: *admin -> kernel <:Kappa:1082189237178351666>

[2025-02-27 11:57] avx: <:kappa:865291794824560680>

[2025-02-27 17:40] Torph: huh how is user-kernel not a security boundary

[2025-02-28 01:02] daax: [replying to Torph: "huh how is user-kernel not a security boundary"]
it is, i think he meant admin->kernel

[2025-02-28 01:05] emma: 
[Attachments: bafkreihjghcxqtjatvc3nsq4dlcucxxb2randthverp7oi3gelic436sdyjpeg.webp]

[2025-02-28 01:11] Torph: [replying to daax: "it is, i think he meant admin->kernel"]
oh ok

[2025-02-28 02:11] JustMagic: [replying to emma: ""]
(UAC Bypass) is the important part

[2025-02-28 13:50] Torph: https://github.com/Wack0/entii-for-workcubes
[Embed: GitHub - Wack0/entii-for-workcubes: PowerPC Windows NT ported to Ni...]
PowerPC Windows NT ported to Nintendo GameCube/Wii/Wii U - Wack0/entii-for-workcubes

[2025-02-28 13:51] Torph: saw this in a university club discord and thought it was this server for a second