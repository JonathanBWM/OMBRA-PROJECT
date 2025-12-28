# December 2024 - Week 3
# Channel: #reverse-engineering
# Messages: 87

[2024-12-10 01:46] projizdivlak: sometimes i see in sections that virtual size < raw size, is this a linker error? to me it seems like raw gets aligned but virtual doesn't
[Attachments: PE-bear_mo4KmoaZKA.png]

[2024-12-10 09:15] x86matthew: [replying to projizdivlak: "sometimes i see in sections that virtual size < ra..."]
it's not a linker error, the original PE  "spec" just said file size should be aligned but virtual size doesn't need to be

[2024-12-10 09:16] x86matthew: in reality the windows loader would probably round it up to filealignment anyway

[2024-12-10 09:39] Matti: spec.docx <:hmm:475800667177484308>

[2024-12-11 16:46] elias: Does anyone else have the issue that VirtualBox vm is stuck for around 5 minutes at boot if a kernel debugger is attached?

[2024-12-11 16:47] elias: Tried net and serial debugging and both have same issue

[2024-12-12 08:49] roddux: asking for someone on another server: is there a tool or accepted method to lift symbols from macho/elf to PE files? 

context is a program shipped for MacOS with symbols included, but not present on Win

[2024-12-12 08:51] roddux: I figure you could use Bindiff to help do this manually? not sure if there is any tooling to do it, though

[2024-12-12 10:07] CutDown: [replying to elias: "Tried net and serial debugging and both have same ..."]
is it because of your debugger? or the OS? which one is stuck?
sometimes if you dont save your debug symbols, when the debugger is attached, it will redownload all of them, and it takes time and its slow and sucks

[2024-12-12 11:53] elias: [replying to CutDown: "is it because of your debugger? or the OS? which o..."]
its the OS, symbols are downloaded within a few seconds

[2024-12-12 13:18] CutDown: Ahh weird
Any logs?

[2024-12-12 13:53] elias: nah I switched back to vmware now

[2024-12-12 14:09] Deleted User: love vmware

[2024-12-12 14:32] 0x208D9: <@609487237331288074>  sorry for the ping, but had a question:

https://revers.engineering/beyond-process-and-object-callbacks-an-unconventional-method/

do u have the ida database file (.idb) available by any chance or the structs and types (.tils) ?
[Embed: Experimenting with Object Initializers in Windows - See PG-complian...]
Overview In this article, I wanted to introduce a fun approach to performing functions similar to those enabled by Windows Object Callbacks but through an alternative means (experimentally). It’s well

[2024-12-12 14:34] daax: [replying to 0x208D9: "<@609487237331288074>  sorry for the ping, but had..."]
no

[2024-12-12 14:34] daax: you use the pdb from ntos and symbolize

[2024-12-12 14:34] daax: it takes 15 minutes

[2024-12-12 14:34] daax: this also does not work anymore, jfyi.

[2024-12-12 14:35] 0x208D9: [replying to daax: "this also does not work anymore, jfyi."]
yeah i checked the disclaimer thats why i asked cuz the structs aint the same

[2024-12-12 14:36] 0x208D9: [replying to daax: "you use the pdb from ntos and symbolize"]
can you elaborate on the "symbolize" part? did u do it manually or used some kinda tracing util?

[2024-12-12 14:38] daax: [replying to 0x208D9: "can you elaborate on the "symbolize" part? did u d..."]
using context from the surrounding functions, and applying types to local variables in the pseudo

[2024-12-12 14:39] 0x208D9: ok so manually, makes sense, aight 😄

[2024-12-12 14:41] 0x208D9: ms has stripped alot of names in the dev channel pdb smh

[2024-12-12 14:41] 0x208D9: but fair will take a look

[2024-12-12 17:57] pinefin: when in doubt paste pseudo code

[2024-12-12 20:00] Matti: [replying to 0x208D9: "ms has stripped alot of names in the dev channel p..."]
most of the fields there really shouldn't be a challenge to just iteratively annotate until you end up with something that makes sense, IMO

[2024-12-12 20:00] Matti: but, there are also (older) private PDBs: https://discord.com/channels/835610998102425650/835635446838067210/1293362872377937941

[2024-12-12 20:01] Matti: these are from 2016 or so so take care

[2024-12-12 20:01] Matti: the more obscure a type is, the less likely it is that it's still the same

[2024-12-12 20:03] Matti: also, just because a type logically belongs in ntoskrnl, does not necessarily mean that it (or lots of other interesting undocumented types) are in the ntoskrnl PDB

[2024-12-12 20:05] Matti: some other good ones are acpi, AppXDeploymentClient, classpnp, combase, fltMgr, ndis, NetAdapterCx, pci, pshed, storport, and WinTypes

[2024-12-14 12:13] Horsie: [replying to Kyle Escobar: "and the modem acts like a dns proxy to send you ma..."]
Does anyone have more info on how someone could have possibly done this?

[2024-12-14 12:13] Horsie: Seems extremely cool. Would love to discuss this a bit

[2024-12-14 12:14] Horsie: From what I can tell the malware pivoted itself into windows from a router.
Was that done only by scanning for known vulns in a system?
The messages make it sound like it also uses the compromised network to serve forged updates? Surely this should not be possible (easily?) to pull off. Unless you can spoof the certificate that is used to check the update packages as well.

[2024-12-14 15:08] dullard: It’s all nonsense

[2024-12-14 15:09] dullard: Especially from Kyle, he waffled for a couple of hours last time this “happened” and failed to provide any samples

[2024-12-14 15:09] dullard: Boy who cried wolf moment

[2024-12-14 15:33] Horsie: [replying to dullard: "It’s all nonsense"]
Rip.. that sucks

[2024-12-14 15:35] Horsie: [replying to jonaslyk: "thats what hit me"]
I thought it might be true because Jonas did mention he has had a similar experience here <@655419785106030612>

[2024-12-14 15:35] Horsie: That kinda puts me in a tough spot.

[2024-12-14 15:36] Horsie: Unless Jonas is trolling big time, which I find to be a bit unlikely

[2024-12-14 15:39] Horsie: Not claiming you're wrong. I'm just trying to get to the root of this

[2024-12-14 16:00] diversenok: This perhaps? https://discord.com/channels/835610998102425650/835656787154960384/853301587219578951

[2024-12-14 16:05] dullard: Oh I believe it’s possible, just not that it happened to Kyle 😂

[2024-12-14 16:47] Horsie: [replying to diversenok: "This perhaps? https://discord.com/channels/8356109..."]
Excellent link! I never knew about this

[2024-12-14 16:47] Horsie: Thanks ❤️

[2024-12-14 18:32] Brit: [replying to dullard: "Oh I believe it’s possible, just not that it happe..."]
I hear that it's common to see comments in deployed malware, in fact they send you source with the sploit to make sure.

[2024-12-14 18:50] dullard: [replying to Brit: "I hear that it's common to see comments in deploye..."]
I heard they gave tricks to get Linux shellcode to run on windows <:Kappa:1082189237178351666>

[2024-12-14 19:36] vmx: [replying to Brit: "I hear that it's common to see comments in deploye..."]
they actually have to do that for linux rootkits, as they'd otherwise violate the GPL license!

[2024-12-14 19:46] Brit: 😕

[2024-12-15 09:14] carl: \> has mitm windows update exploit

[2024-12-15 09:14] carl: \> uses it against random "security researchers" instead of actual important people

[2024-12-15 09:14] carl: 👍

[2024-12-15 09:28] jonaslyk: 
[Attachments: ctrl2cap.amd.sys]

[2024-12-15 09:28] jonaslyk: actually it would be enough to jusr do md2 hash collide to create a immediate cert signed by the same root ca as that driver

[2024-12-15 09:29] jonaslyk: which - if you test will load just fine on any windows box

[2024-12-15 09:31] jonaslyk: and sometimes its not that importnant if you hack someone considered importnant or not- if that person have access to more importnant targets

[2024-12-15 09:33] jonaslyk: one thing i am quite sure about though is

[2024-12-15 09:33] jonaslyk: 
[Attachments: Tg3ig8hJ.png]

[2024-12-15 09:33] jonaslyk: that was not me logging in there

[2024-12-15 12:58] dullard: [replying to jonaslyk: "one thing i am quite sure about though is"]
https://tenor.com/view/sea-level-rise-climate-change-climate-central-climate-flood-gif-24674503

[2024-12-15 13:03] avx: [replying to carl: "\> uses it against random "security researchers" i..."]
it's very good decision making

[2024-12-15 15:51] Timmy: So disabling windows update wasn't so stupid after all <:kekw:904522300257345566>

[2024-12-15 16:01] Brit: you're assuming this isn't a giant larp

[2024-12-15 16:03] dullard: spoiler (it is)

[2024-12-15 17:10] Horsie: [replying to jonaslyk: ""]
Lol..
[Attachments: image.png]

[2024-12-15 17:14] jvoisin: https://en.wikipedia.org/wiki/MD2_(hash_function)

> The algorithm is optimized for 8-bit computers
[Embed: MD2 (hash function)]
The MD2 Message-Digest Algorithm is a cryptographic hash function developed by Ronald Rivest in 1989. The algorithm is optimized for 8-bit computers. MD2 is specified in  IETF RFC 1319. The "MD" in MD

[2024-12-15 17:49] Horsie: [replying to jonaslyk: "which - if you test will load just fine on any win..."]
What the hell??

[2024-12-15 17:49] Horsie: 
[Attachments: image.png]

[2024-12-15 17:50] Horsie: I thought windows won't load drivers signed with revoked certs, ~~especially leafs~~ leaf is okay I think?

[2024-12-15 17:50] Horsie: I'm on windows 10 w/o hyper-v

[2024-12-15 17:50] Horsie: Can someone on default-ish Win11 please sanity check on this?

[2024-12-15 17:52] diversenok: Yeah, it loads under Win 11 with Secure Boot

[2024-12-15 17:52] Horsie: Wow..

[2024-12-15 17:53] Horsie: So a revoked certificate means nothing on windows?

[2024-12-15 17:53] Horsie: I'll have to read up on how cert chains work.

[2024-12-15 17:54] diversenok: User mode doesn't consider it valid, but apparently the kernel does

[2024-12-15 17:59] diversenok: The chain seems valid, so I think the difference is only in the handling of the root certificate

[2024-12-15 18:00] Horsie: [replying to diversenok: "The chain seems valid, so I think the difference i..."]
"seems valid"- other than the fact that the root is revoked right?

[2024-12-15 18:00] Horsie: Hmm

[2024-12-15 18:00] diversenok: It says disallowed on the root
[Attachments: image.png]

[2024-12-15 18:01] Horsie: A very peculiar case. So hypothetically if someone creates a collission for the intermediate cert, you can sign whatever you want on a windows system (verified so far: kernel drivers) and get away with it.

[2024-12-15 18:01] Horsie: 🥶

[2024-12-15 18:01] Horsie: [replying to diversenok: "It says disallowed on the root"]
Fair.

[2024-12-15 18:02] Horsie: Only if people pooled resources for actually important things like finding fun hash collissions than doing boring things like folding proteins. Topple the EV overlords.

[2024-12-15 20:05] roddux: you can probably collide md2 on a smartphone

[2024-12-15 23:52] avx: "Mr the president they collided the intermediary cert with a smart fridge"