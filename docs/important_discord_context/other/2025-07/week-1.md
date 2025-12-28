# July 2025 - Week 1
# Messages: 410

[2025-07-01 18:49] Oliver: [replying to selfprxvoked: "Wouldn't that alert AVs about side channel attacks..."]
i dont really understand what you mean

[2025-07-01 19:26] Brit: he means nothing, that person is markov chaining.

[2025-07-01 20:41] unmasked: anyone know of any specialized stuff for (static) devirtualization of Arxan? one of my favorite games uses a built-in, hardcoded anticheat and also BattleEye (lol). I'd prefer not to touch it with any dynamic analysis tools cause for example IDA Pro's kernel will just die if I open it while Destiny 2 is open.

[2025-07-01 20:41] unmasked: favorite game = Destiny 2

[2025-07-01 20:42] unmasked: I'd love to take a look at its custom userland AC but Arxan memes.

[2025-07-02 14:26] Deleted User: [replying to unmasked: "anyone know of any specialized stuff for (static) ..."]
IDA Pro's kernel???

[2025-07-02 14:26] Deleted User: What do you mean

[2025-07-02 16:32] 5pider: he means the windows kernel

[2025-07-02 17:19] pinefin: [replying to unmasked: "I'd love to take a look at its custom userland AC ..."]
-# just dont make a p2c. they have lawyers on retainer

[2025-07-03 02:56] varaa: but what if p2c also have lawyers on retainer ⁉️

[2025-07-03 06:05] sariaki: What if I’m a lawyer with a retainer⁉️

[2025-07-03 08:27] Xyrem: What if I am a retainer with a lawyer ⁉️

[2025-07-03 08:28] dagger: What if I am NOT a retainer with a lawyer ⁉️

[2025-07-03 08:50] 0xatul: Why are lawyers on these ?
[Attachments: images6.jpg]

[2025-07-03 08:50] 0xatul: Must probably require really nice teeth to présent their arguments

[2025-07-03 09:21] dagger: they have jaw gnashing due to late night anxiety so they neeed it to preform better in debates

[2025-07-03 10:20] sariaki: those acs prolly have their lawyers on invisalign

[2025-07-03 10:20] sariaki: shits great

[2025-07-03 12:27] ACC RESTRICTED ADD NEW ACC: [replying to Xyrem: "Possible to say it behaves like a rat? You clearly..."]
fr

[2025-07-03 12:28] ACC RESTRICTED ADD NEW ACC: oh hello xyrem

[2025-07-03 12:28] ACC RESTRICTED ADD NEW ACC: [replying to 0xatul: "Why are lawyers on these ?"]
lmfao

[2025-07-03 12:29] ACC RESTRICTED ADD NEW ACC: [replying to Matti: "so, my hypothesis is that driver loading still doe..."]
there are quite a few vulnerable drivers not on the mvdb ready to exploit

[2025-07-03 12:29] ACC RESTRICTED ADD NEW ACC: just exercise due caution and cleanup

[2025-07-03 12:33] Matti: well no shit

[2025-07-03 12:33] Matti: manual blacklisting is a terrible approach

[2025-07-03 12:34] Matti: I don't know why they never made something to get around the issues with CRL servers that runs in the background in user mode or something

[2025-07-03 12:35] Matti: it's not ideal because there could be a few hours' delay but it's a lot better than not doing recovocation checks at all

[2025-07-03 12:35] ACC RESTRICTED ADD NEW ACC: heuristic analysis?

[2025-07-03 12:35] Matti: one guess is that many (sub)CAs don't even bother with CRLs in the first place

[2025-07-03 12:35] Matti: since CRL querying is also horrible and overly complex

[2025-07-03 12:36] Matti: including the server side of it

[2025-07-03 12:36] ACC RESTRICTED ADD NEW ACC: [replying to Matti: "one guess is that many (sub)CAs don't even bother ..."]
i forget, ci.dll handles this?

[2025-07-03 12:37] Matti: but, there are still drivers in the Disallowed store (i.e. they are revoked by either the issuer or MS) so revocation checks without using CRL queries would not be *useless*

[2025-07-03 12:37] unmasked: [replying to pinefin: "-# just dont make a p2c. they have lawyers on reta..."]
not looking to make a p2c thankfully. I just want to reverse engineer the game's built-in anticheat

[2025-07-03 12:38] Matti: [replying to ACC RESTRICTED ADD NEW ACC: "i forget, ci.dll handles this?"]
uhhh I think in windows, only user mode things may potentially query CRL servers

[2025-07-03 12:38] unmasked: [replying to 5pider: "he means the windows kernel"]
No, IDA Pro has a "kernel" of some sort that Destiny 2 will crash if you open it when Destiny 2 is running. Not seen any documentation on it, Matti or some of the s.c folks may have a better insight on it.

[2025-07-03 12:39] Matti: not me, I only know of the term kernel in the context of IDA as kernel.hpp

[2025-07-03 12:39] ACC RESTRICTED ADD NEW ACC: [replying to pinefin: "-# just dont make a p2c. they have lawyers on reta..."]
tbf who doesn't

[2025-07-03 12:40] unmasked: Anyhow, if anyone knows anything about deobfuscation/devirtualization of Arxan, either manually or automatically, do let me know. I *think* it's Arxan, anyways.

[2025-07-03 12:40] Matti: [replying to Matti: "uhhh I think in windows, only user mode things may..."]
even this is usually implemented by 3rd party libs and done by browsers, not windows components

[2025-07-03 12:41] ACC RESTRICTED ADD NEW ACC: typical microsoft slop

[2025-07-03 12:41] impost0r: [replying to unmasked: "Anyhow, if anyone knows anything about deobfuscati..."]
Yes, it is Arxan. Unfortunately no information for you on devirtualization of Arxan though as someone who plays the game a ton I'd also like to look at the internals.

[2025-07-03 12:42] ACC RESTRICTED ADD NEW ACC: [replying to unmasked: "Anyhow, if anyone knows anything about deobfuscati..."]
are there no signatures to a exec being protected by arxan?

[2025-07-03 12:42] ACC RESTRICTED ADD NEW ACC: watermarks etc?

[2025-07-03 12:43] ACC RESTRICTED ADD NEW ACC: with hyperion https://x.com/vxunderground/status/1545085464813633537
Hyperion, an anti\-tampering software developed by @ByfronTech, doesn't care about you or your feelings\.

Image via @AntiCheatPD

[2025-07-03 12:43] unmasked: There likely are. if I had the patience to download it on an ARM64 VM I'd download it

[2025-07-03 12:43] unmasked: and yeah, I've seen the hyperion thing lol

[2025-07-03 12:45] ACC RESTRICTED ADD NEW ACC: i'd do a dynamic analysis on it tbh but that's just me not giving a fuck as I'm not really in the p2c business anyways
most of my reversal is for sole use and knowledge

[2025-07-03 12:45] ACC RESTRICTED ADD NEW ACC: and i'm not in the game of devirtualisation

[2025-07-03 12:46] ACC RESTRICTED ADD NEW ACC: I'll leave that to the p2cs

[2025-07-03 12:46] impost0r: neither am i in the business of p2cs

[2025-07-03 12:46] impost0r: but it is interesting

[2025-07-03 12:46] impost0r: also <@946775951562391632>
[Attachments: Screenshot_2025-07-03_at_7.45.49_AM.png]

[2025-07-03 12:48] impost0r: source: https://github.com/pr701/fix-arxan
please don't cheat in my shitty mmorpgfps they'll sue you to hell and back
[Embed: GitHub - pr701/fix-arxan: Arxan binary fixer using unpacked dump]
Arxan binary fixer using unpacked dump. Contribute to pr701/fix-arxan development by creating an account on GitHub.

[2025-07-03 12:48] unmasked: [replying to impost0r: "source: https://github.com/pr701/fix-arxan
please ..."]
thanks for the link; yeah, don't intend to

[2025-07-03 12:53] Eskii: Y’all know how to deal with obfuscation on strings, seige just added some bs

[2025-07-03 12:53] impost0r: speaking of anti-cheat, anyone besides me taken a look at league (macOS) Vanguard implementation? surprised they didn't register an endpoint security handler for it; it seems more defensive than anything. which is nice. doesn't fix a fundamental issue on games on macOS though.

[2025-07-03 12:53] impost0r: [replying to Eskii: "Y’all know how to deal with obfuscation on strings..."]
this isn't a channel for p2c, you're better off finding your help elsewhere afaik.

[2025-07-03 12:57] impost0r: anyways Vanguard ™ for macOS seemingly hasn't fixed the most glaring issue with having a game on macOS... codesigning.

[2025-07-03 13:00] ACC RESTRICTED ADD NEW ACC: [replying to impost0r: "speaking of anti-cheat, anyone besides me taken a ..."]
i really haven't heard much about cheating on macs

[2025-07-03 13:00] impost0r: it's a really niche market

[2025-07-03 13:00] ACC RESTRICTED ADD NEW ACC: other than that calamari or whatever cheat it was back in 2018 or something

[2025-07-03 13:00] ACC RESTRICTED ADD NEW ACC: for roblox

[2025-07-03 13:00] impost0r: underdeveloped on both ends

[2025-07-03 13:00] ACC RESTRICTED ADD NEW ACC: [replying to impost0r: "underdeveloped on both ends"]
yikes

[2025-07-03 13:01] ACC RESTRICTED ADD NEW ACC: i have a mac... old 2011 mac mini

[2025-07-03 13:01] ACC RESTRICTED ADD NEW ACC: have been looking into getting an m4 or something macbook pro for my next laptop

[2025-07-03 13:01] ACC RESTRICTED ADD NEW ACC: i really love the apple ecosystem but for obvious reasons i'm a die hard windows user

[2025-07-03 13:02] impost0r: Diehard macOS user and they're killing off x64 support with the next macOS (27) from what I know

[2025-07-03 13:02] ACC RESTRICTED ADD NEW ACC: [replying to Eskii: "Y’all know how to deal with obfuscation on strings..."]
split your strings and render each char to your menu or something, do something pretty niche or memey but yeah i wouldn't discuss p2cing here
obfuscate then destroy the original variable containing your plaintext or initially encrypted string
remember gc/cleanup, whatever is in memory is visible

[2025-07-03 13:03] ACC RESTRICTED ADD NEW ACC: p2c/ac is a cat and mouse game

[2025-07-03 13:04] ACC RESTRICTED ADD NEW ACC: fyi I think dma is just a glorified gimmick

[2025-07-03 13:05] ACC RESTRICTED ADD NEW ACC: there are troves of undocumented shit

[2025-07-03 14:24] Xyrem: Dma is fake and false

[2025-07-03 14:25] dullard: xyrem is real and true

[2025-07-03 14:47] pinefin: [replying to ACC RESTRICTED ADD NEW ACC: "have been looking into getting an m4 or something ..."]
just get an m3 pro

[2025-07-03 14:47] pinefin: and make sure to get more than 8gb of memory

[2025-07-03 14:47] Deleted User: [replying to ACC RESTRICTED ADD NEW ACC: "p2c/ac is a cat and mouse game"]
My p2c is a dog

[2025-07-03 14:48] ACC RESTRICTED ADD NEW ACC: [replying to Deleted User: "My p2c is a dog"]
is pasting school still a thing

[2025-07-03 14:48] pinefin: [replying to ACC RESTRICTED ADD NEW ACC: "fyi I think dma is just a glorified gimmick"]
as much as i want to agree with you, dma is a valid way of cheating and not getting caught......but dma is for pussies

[2025-07-03 14:48] ACC RESTRICTED ADD NEW ACC: [replying to pinefin: "just get an m3 pro"]
ive heard even the m1s are lightning quick esp for a laptop

[2025-07-03 14:48] pinefin: [replying to ACC RESTRICTED ADD NEW ACC: "ive heard even the m1s are lightning quick esp for..."]
theres a bar graph with performance ratings if you want to look it up

[2025-07-03 14:48] pinefin: ive seen people cheat on valorant with a hackintosh

[2025-07-03 14:49] ACC RESTRICTED ADD NEW ACC: [replying to pinefin: "as much as i want to agree with you, dma is a vali..."]
im not spending allat on hardware and messing with firmware etc. when memes suffice

[2025-07-03 14:49] pinefin: [replying to ACC RESTRICTED ADD NEW ACC: "im not spending allat on hardware and messing with..."]
exactly

[2025-07-03 14:49] pinefin: its for pussies

[2025-07-03 14:50] pinefin: id get it if you were like an everyday 40 year old man wanting to cheat in tarkov and last for 5 years on your LIMITED EDITION EOD account...but uh...if you want to actually cheat use an ext/int

[2025-07-03 14:50] pinefin: going on a rant here, and i'll stop it here, i just think its meh and ruins the fun of the cat&mouse game

[2025-07-03 14:51] Deleted User: [replying to pinefin: "id get it if you were like an everyday 40 year old..."]
Dma is used for external cheats🤔

[2025-07-03 14:51] pinefin: [replying to Deleted User: "Dma is used for external cheats🤔"]
technically, yes. but you know exactly what i mean

[2025-07-03 14:51] pinefin: driver+overlay externals

[2025-07-03 14:52] pinefin: not FIRMWARE+SECOND PC+PCIE BUS+ETC just for an external

[2025-07-03 14:52] pinefin: like i said, its meh

[2025-07-03 14:52] pinefin: thats what im saying

[2025-07-03 14:53] Deleted User: Idk English

[2025-07-03 14:53] pinefin: you spoke english good

[2025-07-03 14:53] pinefin: dont know why u deleted your message

[2025-07-03 14:53] pinefin: im done ranting about dma and how i dislike it, its just a circle

[2025-07-03 14:53] Deleted User: It made sense in my head but i when I read it didn’t make sense

[2025-07-03 14:53] Deleted User: <:topkek:904522829616263178>

[2025-07-03 14:53] Deleted User: <:skill_issue:1210171860063617074>

[2025-07-03 17:38] ?: <@775688014176452608> hey hello

[2025-07-03 22:07] varaa: mods gonna have a field day with the amount of p2c/cheat talk in this channel

[2025-07-03 22:07] varaa: <:kekw:904522300257345566>

[2025-07-03 23:10] mtu: [replying to Matti: "but, there are still drivers in the Disallowed sto..."]
Disallowed store certs aren’t always also revoked

[2025-07-03 23:13] mtu: The way i made sense of that is that if you revoke the cert, no one with secure boot can run it even if they really really need to for their bespoke high voltage power relay driver. Having a list of certs that the kernel denies allows the admin to continue to run their signed-but-vulnerable driver

[2025-07-03 23:23] brymko: [replying to varaa: "mods gonna have a field day with the amount of p2c..."]
https://tenor.com/view/wesker-albert-albert-wesker-resident-evil-resident-evil-4-gif-5143688443782660001

[2025-07-03 23:26] Matti: [replying to mtu: "Disallowed store certs aren’t always also revoked"]
yeah me and <@943099229126144030> were literally just discussing this in SC

[2025-07-03 23:27] Matti: a really revoked cert apparently gives this pretty obscure message?
[Attachments: image.png]

[2025-07-03 23:28] Matti: second tab does contain the same string, but as an X509 property
[Attachments: image.png]

[2025-07-03 23:28] mtu: That’s…. Wild

[2025-07-03 23:29] Matti: err no not even, the string I got was "the certificate was explicitly revoked by its issuer"

[2025-07-03 23:29] mtu: They just insert x509 metadata into the view so it doesn’t match what you’d get from OpenSSL?

[2025-07-03 23:29] Matti: which wasn't even true

[2025-07-03 23:29] Matti: [replying to mtu: "They just insert x509 metadata into the view so it..."]
I'm trying to figure out how the fuck this "works" right now <:lmao3d:611917482105765918>

[2025-07-03 23:30] mtu: Good luck lmao

[2025-07-03 23:30] Matti: note that the message also doesn't say anything about the disallowed **store**, but a property

[2025-07-03 23:30] mtu: If I wasn’t trying to fix my AC I’d hop in and take a look

[2025-07-03 23:31] Matti: it's not in the store
and -dump seems perfectly content with it as well as far as I can see
[Attachments: image.png, image.webp]

[2025-07-03 23:35] mtu: That’s what I’d expect yeah

[2025-07-03 23:36] mtu: Wild of the UI viewer to toss extra info into Details, I thought that was supposed to be a 1:1 map to cert attributes

[2025-07-03 23:37] Matti: in general everything about signing things in windows is 100% shit

[2025-07-03 23:37] Matti: the concept, the implementation and the tooling provided

[2025-07-03 23:38] Matti: if I need to generate keys I unironically use openssl, also not exactly known for its user friendliness

[2025-07-03 23:38] Matti: still better than certutil or certmgr.msc

[2025-07-03 23:38] Matti: and the fucking explorer shell extension

[2025-07-03 23:39] mtu: I didn’t even know you _could_ generate certs with certutil lmao

[2025-07-03 23:39] mtu: Openssl go brrrt

[2025-07-03 23:40] Matti: [replying to mtu: "The way i made sense of that is that if you revoke..."]
btw, just to clarify, windows (driver or not) certificate revocation is unrelated to secure boot, other than the fact that presumably secure boot enables the blacklist by default?

[2025-07-03 23:40] mtu: [replying to Matti: "which wasn't even true"]
How was it revoked if not by the issuer

[2025-07-03 23:41] mtu: [replying to Matti: "btw, just to clarify, windows (driver or not) cert..."]
Driver signature enforcement is an aspect of secure boot, no?

[2025-07-03 23:41] Matti: by a script I wrote

[2025-07-03 23:41] mtu: At least for kernel mode drivers

[2025-07-03 23:41] Matti: ya, for sure

[2025-07-03 23:41] mtu: The blocklist isn’t part of sig enforcement, it’s part of HVCI

[2025-07-03 23:42] Matti: [replying to Matti: "by a script I wrote"]
pw: infected
you need to run the thing as admin and then reboot for it to claim that the cert was revoked by the issuer
[Attachments: InnoTek.7z]

[2025-07-03 23:43] Matti: [replying to mtu: "The blocklist isn’t part of sig enforcement, it’s ..."]
yeah I'm aware of this I guess, but unlike HVCI which actually runs in kernel, the driver blocklist seems laughably easy to bypass

[2025-07-03 23:43] Matti: since it's just a registry value

[2025-07-03 23:44] Matti: I might be wrong, but that's my initial assessment, also based on how much windows doesn't seem to care about revocation of kernel drivers historically

[2025-07-03 23:44] mtu: I used to work on ICS so we needed this “feature”

[2025-07-03 23:45] mtu: Had to disable blocklist without requiring a reboot

[2025-07-03 23:45] mtu: But our drivers were still signed

[2025-07-03 23:45] Matti: of course you can't just toggle the registry flag with SB on, but there's still going to be some user mode component involved most likely, not kernel, when reading its value and performing the check

[2025-07-03 23:46] Matti: also, I mean, if you configure SB corectly you own the PK and you can literally boot into windows with a bootkit and SB will still be on

[2025-07-03 23:47] Matti: that's a lot of help in the unlikely case you can't just get around it from user mode to begin with

[2025-07-03 23:52] Matti: [replying to Matti: "second tab does contain the same string, but as an..."]
I can sort of see their point when they say revocation checks are pointless
[Attachments: image.png]

[2025-07-03 23:53] Matti: webmaster = eric@devrandom.be FYI

[2025-07-03 23:53] Matti: I'd better email him I guess

[2025-07-03 23:56] Matti: [replying to mtu: "I used to work on ICS so we needed this “feature”"]
trying to follow what you're saying here
by the feature, you mean secure boot, HVCI, or the blocklist specifically? or you needed to load something signed with revoked/expired certs?

[2025-07-03 23:57] Matti: [replying to mtu: "Had to disable blocklist without requiring a reboo..."]
following... disabling blocklist fixed the issue

[2025-07-03 23:57] Matti: [replying to mtu: "But our drivers were still signed"]
wdym "still signed"?

[2025-07-04 00:02] mtu: The drivers were signed legitimate drivers from the vendor

[2025-07-04 00:03] mtu: They also had a “undocumented feature” of kernel R/W, so on the blocklist they went

[2025-07-04 00:04] mtu: We could keep running them in that state by disabling the blocklist, but didn’t have to disable any other security features

[2025-07-04 00:04] Matti: ah, I see

[2025-07-04 00:05] mtu: [replying to Matti: "wdym "still signed"?"]
“Still valid” would probably have been a better way to say this, the cert was not revoked or expired

[2025-07-04 00:05] Matti: yeah that would be my expectation as well

[2025-07-04 00:05] Matti: the blocklist is a really fragile and shitty bandaid IMO

[2025-07-04 00:06] mtu: 100%, but you can’t _not_ do something when it’s public enough to get a GTFO list

[2025-07-04 00:06] mtu: Story of windows, “yeah we have to support legacy crap so all new features are made in insane ways”

[2025-07-04 00:07] Matti: sorta random sounding question, but do you know where the blocklist is stored? like the actual physical file?

[2025-07-04 00:08] Matti: I have a hypothesis re: why they don't do the obvious and query the Disallowed store instead, but it would take the driver blocklist being accessible by the boot loader for it to work

[2025-07-04 00:09] mtu: It’s very possibly hardcoded in ci.dll

[2025-07-04 00:10] mtu: It doesn’t update online, it’s only patched with updates

[2025-07-04 00:10] Matti: like, how are you going to block a boot start driver that is on the shitlist, if Cm isn't initialized to the point where you can query the store? mind you, that's only one of several places certs in the "same store" can be stored

[2025-07-04 00:11] Matti: [replying to mtu: "It’s very possibly hardcoded in ci.dll"]
yeah I expect *something like this* is the case as well

[2025-07-04 00:12] Matti: but the bootloader can open and read any file on the FS in principle, though generally it limits itself to stuff in system32

[2025-07-04 00:13] Matti: and that disgusting acpi hack table, that one might be in C:\Windows\something

[2025-07-04 00:14] Matti: another example of a hardcoded blob and sort of a shitlist... well more like a list of shame

[2025-07-04 00:16] Matti: appatch\drvmain.sdb would be my guess

[2025-07-04 00:16] Matti: and that they simply merged them

[2025-07-04 00:17] Matti: I see both NT driver filenames and ACPI identifiers in there

[2025-07-04 08:11] Windy Bug: [replying to Matti: "sorta random sounding question, but do you know wh..."]
Isn’t it driversipolicy.p7b in the %windir%\system32\CodeIntegrity?

[2025-07-04 08:11] Windy Bug: Might be wrong

[2025-07-04 08:14] Matti: nope, pretty sure that's right

[2025-07-04 08:16] Matti: it could also be that the .p7b contains the signers for the list (surprise: it's MS), and that the actual blocklist is driver.stl in the same folder

[2025-07-04 08:26] Matti: OK no, it's not that one either

[2025-07-04 08:26] Matti: clearly I forgot what an STL file is

[2025-07-04 08:28] Matti: eurgh this certutil -dump output is impossible to understand

[2025-07-04 08:29] Matti: for the STL file, of which I've already forgotten what it even stands for

[2025-07-04 08:38] Matti: it's hard to say for sure to me, it's very confusing in that the dump output for the p7b clearly contains strings of "revoked" (blacklisted) certificates, e.g.  Avast

[2025-07-04 08:39] Matti: but it's not really clear to me what the context/meaning of this is since certutil just gives a giant hex dump and names it 'nesting level 1'

[2025-07-04 08:42] Matti: after that there is also this, which is a lot easier to follow... at least I *think*
the PCA 2011 cert was not revoked but it has been deprecated in favour of their 2023 signing cert because of the black lotus malware
[Attachments: image.png]

[2025-07-04 08:43] Matti: that would match `CERT_TRUST_HAS_PREFERRED_ISSUER`

[2025-07-04 08:44] Matti: meanwhile back in the driver.stl dump you'll find the exact same thing about MS' own certs eventually

[2025-07-04 08:44] Matti: but there is a lot less meaningless hex dump and....

[2025-07-04 08:45] Matti: this
[Attachments: image.png]

[2025-07-04 08:45] Matti: which looks suspiciously like what a hash-based blocklist might look like, IMO

[2025-07-04 08:47] Matti: oh my god I hate certutil so fucking much
[Attachments: image.png]

[2025-07-04 17:25] ACC RESTRICTED ADD NEW ACC: [replying to Windy Bug: "Isn’t it driversipolicy.p7b in the %windir%\system..."]
leme know i’m drunk asf rn but i’ll look into the mvdb

[2025-07-04 19:49] Matti: the tldr of *my* conclusion above is that it seems to be driver.stl that contains the actual list of hashes (both sha1 and sha256, with a few more of the latter for some reason)

[2025-07-04 19:50] Matti: but I'm not certain this is the full story at all, after all the p7b definitely contains names of "naughty" driver certificates

[2025-07-04 21:26] dagger: hey guys i got a question, why would a cheat for battleye need to create its own bootloader for a bypass, is there something im missing? are bootloaders more secure when it comes to evading anticheats, if so, why would it need that especially for battleye, its just making me skeptical that the cheat itself could be malware (ie, rootkit bootkit)

[2025-07-04 21:27] Matti: there are (mostly very bad) reasons to do this that will make it easier to cheat, yes

[2025-07-04 21:28] Matti: ask for the source code

[2025-07-04 21:28] Matti: if you don't get it, don't run it

[2025-07-04 21:28] Matti: same rule that applies to all software, apart from absolute necessities like windows

[2025-07-04 21:28] dagger: i see, but unfortunately it is a private p2c project but i am intruiged by it, many people use it but it still doesnt change the fact it could be malware. ill just stay away from it

[2025-07-04 21:28] Matti: inb4 linux users

[2025-07-04 21:29] Matti: [replying to dagger: "i see, but unfortunately it is a private p2c proje..."]
can you obtain a binary somehow

[2025-07-04 21:29] dagger: I can, yes

[2025-07-04 21:29] dagger: it would be a loader though

[2025-07-04 21:29] Matti: with luck I may be able to sue them for you

[2025-07-04 21:29] dagger: requiring login

[2025-07-04 21:29] dagger: haha

[2025-07-04 21:30] Matti: not literally every bootkit is a copy/paste of efiguard anymore, only every other one

[2025-07-04 21:30] dagger: hm? are you saying i shouldnt be worried?

[2025-07-04 21:31] Matti: no sorry this is related to the suing

[2025-07-04 21:31] dagger: ohh xd

[2025-07-04 21:31] Matti: it would be a violation of the GPL

[2025-07-04 21:31] dagger: but bootkits, in themselves, are hard to remove right? harder than an average popup malware

[2025-07-04 21:31] dagger: [replying to Matti: "it would be a violation of the GPL"]
i see

[2025-07-04 21:31] dagger: reverse engineering it to see if efiguard is used

[2025-07-04 21:31] dagger: pretty technical

[2025-07-04 21:32] Matti: [replying to dagger: "but bootkits, in themselves, are hard to remove ri..."]
I mean it depends

[2025-07-04 21:32] dagger: ah yes, like black lotus

[2025-07-04 21:32] Matti: is it obfuscated? is it malware? if Y, then likely also harder to remove

[2025-07-04 21:32] dagger: probably both considering thats what people do now adays

[2025-07-04 21:32] Matti: but if you take efiguard as an example again, that does not need installing and therefore neither does it need uninstalling

[2025-07-04 21:33] Matti: if it's a p2c I'm gonna guess it's at least obfuscated and massively annoying to remove

[2025-07-04 21:33] Matti: dunno what the threshold for actual malware is

[2025-07-04 21:34] dagger: [replying to Matti: "but if you take efiguard as an example again, that..."]
sorry im unfamiliar with efiguard, so you are saying it would be impossible to remove or easy to remove?

[2025-07-04 21:34] Matti: again it depends

[2025-07-04 21:34] Matti: on how big of an asshole the author(s) is/are

[2025-07-04 21:34] Matti: [replying to dagger: "pretty technical"]
just send me the binary

[2025-07-04 21:34] dagger: okay

[2025-07-04 21:34] dagger: would you like it in dms?

[2025-07-04 21:35] Matti: if it's obfsucated I won't be able to tell either, but then the decision is also pretty trivial

[2025-07-04 21:35] Matti: don't run it

[2025-07-04 21:35] Matti: [replying to dagger: "would you like it in dms?"]
sure that's fine

[2025-07-04 21:35] dagger: [replying to Matti: "don't run it"]
i know this for sure

[2025-07-04 21:35] dagger: [replying to Matti: "if it's obfsucated I won't be able to tell either,..."]
i assume its heavily obfuscated because its public but if you want to have a go at it why not

[2025-07-04 21:37] Matti: if it's not obfuscated or poorly, I'll be able to tell if it's an efiguard clone in about 5 secs in IDA

[2025-07-04 21:38] Matti: if it is, see above

[2025-07-04 21:41] dagger: alrighty i sent it

[2025-07-04 21:44] Matti: definitely obfuscated (using vmprotect), but almost as surely not efiguard regardless

[2025-07-04 21:46] Matti: considering this is a win32 exe that drops a lot lot of unwanted shit (namely chromium) but no PEs that are EFI files as far as I can tell

[2025-07-04 21:46] Matti: are you sure this would install the alleged bootkit if I were to run it

[2025-07-04 21:47] Matti: or is it gonna make you do some authentication via their chromium to ensure you're really a paying customer of this shitware

[2025-07-04 21:47] Matti: and maybe then give you a link to an EFI binary

[2025-07-04 21:51] dagger: <@148095953742725120> its a loader of the software, i ran it on tria.ge and it gave me a bootloader error so i assume it tries to do something even before the login is shown

[2025-07-04 21:51] dagger: but i think it does authentication then

[2025-07-04 21:51] dagger: so im not entirely sure

[2025-07-04 21:51] Xyrem: Almost every p2c does authentication of some sort

[2025-07-04 21:53] Xyrem: but most of the time, the developer/owner is a complete moron. So they'll just embed the actual dll/driver/bootkit in the loader itself, so if you unpack it then search for the MZ header. 6/10 times, you'll be able to pull some sort of asset used.

[2025-07-04 21:57] dagger: [replying to Xyrem: "but most of the time, the developer/owner is a com..."]
im sure the case is this, it runs the bootkit then the loader, it obviously has some priorities

[2025-07-04 21:57] dagger: 🤣

[2025-07-04 22:45] the horse: [replying to Matti: "not literally every bootkit is a copy/paste of efi..."]
efiguard is useless in cheat scene

[2025-07-04 22:46] the horse: it's either a driver mapper / efi runtime driver for communication protocol / pre-patchguard hooks on ntoskrnl (for BE likely) / voyager paste -- 95% of cases

[2025-07-04 22:47] the horse: disabling DSE is not needed as there's many circumventions around it with vuln drivers
patchguard disabling at boot-time is inefficient because anti-cheats can find that patchguard is inactive/context is not initialized

[2025-07-04 22:48] the horse: you can patch ntoskrnl without worrying about patchguard (during efi) and a BattlEye internal bypass requires like 6 hooks to be invisible

[2025-07-04 22:48] Matti: [replying to the horse: "efiguard is useless in cheat scene"]
you don't have to tell me this

[2025-07-04 22:48] Matti: tell it to the people who insist on using it for cheating regardless

[2025-07-04 22:49] Matti: [replying to the horse: "it's either a driver mapper / efi runtime driver f..."]
those driver mappers are also based on efiguard lol

[2025-07-04 22:50] Matti: only with an expanded API that does literally one thing - run some arbitrary function pointer

[2025-07-04 22:50] Matti: I wonder why I didn't include that...

[2025-07-04 22:51] Matti: probably because it's an idiotic idea and I have no use for it personally

[2025-07-04 22:51] the horse: i mean.. are they?

[2025-07-04 22:51] the horse: mostly https://github.com/btbd/umap/tree/master is used
[Embed: GitHub - btbd/umap: UEFI bootkit for driver manual mapping]
UEFI bootkit for driver manual mapping. Contribute to btbd/umap development by creating an account on GitHub.

[2025-07-04 22:51] the horse: which seems very different from efiguard

[2025-07-04 22:51] Matti: my use cases are more important than cheaters' use cases in efiguard unfortunately

[2025-07-04 22:52] Xits: [replying to the horse: "it's either a driver mapper / efi runtime driver f..."]
How is voyager detected nowadays? I was theory crafting how it could be possible recently. all I came up with was secure boot checks or if windows caches the loaded efi modules after the exitbootservices call somehow

[2025-07-04 22:52] Matti: [replying to the horse: "which seems very different from efiguard"]
sure, you're right

[2025-07-04 22:52] the horse: [replying to Xits: "How is voyager detected nowadays? I was theory cra..."]
bootmgfw attributes mostly

[2025-07-04 22:52] Matti: I'm only talking about a specific bootkit that did this

[2025-07-04 22:53] Matti: but which is quite popular on the usual suspects for negative IQ posts, like UC

[2025-07-04 22:53] Xits: [replying to the horse: "bootmgfw attributes mostly"]
Got a source for that? I’d like to read about it if so

[2025-07-04 22:54] the horse: yes open bootmgfw.efi properties

[2025-07-04 22:54] the horse: and look at the time/date modified

[2025-07-04 22:56] the horse: there's some additional theoretical detections people described based on number of ept'd pages, pages marked as unused that would normally be used by the system, since they're free (due to the extended, hidden allocation for the payload)

[2025-07-04 22:56] the horse: they're all mitigable

[2025-07-04 22:57] the horse: you don't need to allocate any new executable memory

[2025-07-04 22:57] the horse: or extend anything

[2025-07-04 22:57] the horse: you don't need to replace bootmgfw either

[2025-07-04 22:59] Matti: you know what's funny... efiguard has the same "issues" in that I intentionally made it trivial to query from user mode

[2025-07-04 23:00] Matti: if you wanna use the wrong tool to cheat then hiding it is DIY homework

[2025-07-04 23:00] the horse: some anti-paste is always welcome

[2025-07-04 23:00] Matti: and yet the last time I tried it, it worked with EAC, vanguard and I think battleye

[2025-07-04 23:01] Matti: idk why they don't just block it

[2025-07-04 23:02] Matti: it's *possible* that they actually agree efiguard isn't malware, or a cheat

[2025-07-04 23:02] Matti: and simply wait for the idiot user to load their driver, and then ban for that

[2025-07-04 23:02] Matti: just a wild hypothesis, I truly don't know

[2025-07-04 23:03] the horse: well you can dump the runtime driver

[2025-07-04 23:03] the horse: maybe they check if it's been modified

[2025-07-04 23:03] the horse: or they flip a flag to just ban on other anomalies

[2025-07-04 23:03] the horse: ¯\_(ツ)_/¯

[2025-07-04 23:03] Matti: modified compared to what? there's no reference binary of efiguard

[2025-07-04 23:03] the horse: EAC at least seems heavily data-based where a combination of things would heavily delay ban you

[2025-07-04 23:03] Matti: it's open source

[2025-07-04 23:04] the horse: yeah true

[2025-07-04 23:04] Matti: I only sometimes make releases for convenience reasons because EDK2 is such a pain in tthe ass

[2025-07-04 23:04] the horse: EDK2 is the reason i wrote my bootkit in rust

[2025-07-04 23:04] the horse: uefi-rs 🙏

[2025-07-04 23:05] the horse: it's lacking compared to edk2 in a lot of things but nothing that can't be worked around 🙂

[2025-07-04 23:05] Matti: yeah I also want to write my next project (not a bootkit, long story but I need a PEI driver to patch some bullshit in firmware on a specific board) in rust

[2025-07-04 23:05] the horse: + I don't have to rewrite most of the stuff to discard edk2 strings

[2025-07-04 23:06] elias: [replying to the horse: "there's some additional theoretical detections peo..."]
i guess the most reliable way to detect something like that would be with the TPM

[2025-07-04 23:07] the horse: valorant will just tell you to fuck off on windows 11 because secure boot is off

[2025-07-04 23:07] the horse: not sure what you mean with using TPM for this

[2025-07-04 23:07] Matti: 🤷 why would you need to disable secure boot to run a bootkit

[2025-07-04 23:07] Matti: you can and should install your own keys

[2025-07-04 23:08] the horse: unviable for cheats

[2025-07-04 23:08] Matti: but yes, a TPM will log this

[2025-07-04 23:08] elias: [replying to the horse: "not sure what you mean with using TPM for this"]
ideally you would use the TPM to make sure secure boot is really enabled, and it also contains more information in the measured boot logs like loaded boot apps in PCR[4] etc

[2025-07-04 23:08] Matti: [replying to the horse: "unviable for cheats"]
I have to disagree, I mean I literally just said it works with valorant

[2025-07-04 23:08] the horse: [replying to Matti: "I have to disagree, I mean I literally just said i..."]
well, for p2c

[2025-07-04 23:08] the horse: not cheats themselves

[2025-07-04 23:08] Matti: or tbf, worked last time I tried, it's not exactly something I do daily

[2025-07-04 23:09] the horse: [replying to elias: "ideally you would use the TPM to make sure secure ..."]
yeah more stuff you need to take care of, if you're hyper-v hijacking you can also hook on TPM commands

[2025-07-04 23:09] the horse: I think with ntoskrnl hooks as well, if you hook MmMapIoSpace

[2025-07-04 23:09] elias: [replying to the horse: "yeah more stuff you need to take care of, if you'r..."]
yeah but you can't spoof a tpm quote without a genuine tpm

[2025-07-04 23:10] elias: you dont have the keys to sign it

[2025-07-04 23:10] the horse: yeah true..

[2025-07-04 23:11] the horse: maybe you could emulate tpm, expose different keys for the guest and act as some middleware

[2025-07-04 23:11] Matti: [replying to the horse: "not cheats themselves"]
not sure I get the difference

[2025-07-04 23:11] the horse: but that's just.. too much work

[2025-07-04 23:11] the horse: [replying to Matti: "not sure I get the difference"]
easy for a dev, painfully hard for a customer

[2025-07-04 23:12] Matti: I'm talking about efiguard standalone, I don't even have a cheat to test with

[2025-07-04 23:12] Matti: yeah idc about either of those two types of users

[2025-07-04 23:12] Matti: if by dev you mean p2c dev

[2025-07-04 23:13] Matti: I wrote efiguard to simplify loading drivers that patchguard doesn't like

[2025-07-04 23:13] the horse: <@234331837651091456> does TPM generate those logs? Or does a part of the windows boot system generate, and encrypt them with TPM?

[2025-07-04 23:13] Matti: decrease the BSOD ratio somewhat

[2025-07-04 23:13] Xits: [replying to the horse: "valorant will just tell you to fuck off on windows..."]
Can’t you just load a vulnerable boot loader image for code execution?

[2025-07-04 23:14] the horse: [replying to Xits: "Can’t you just load a vulnerable boot loader image..."]
valorant's anti-cheat runs at boot-time

[2025-07-04 23:14] the horse: therefore they will see this

[2025-07-04 23:14] Matti: the fact that it can disable DSE is completely secondary (and unneeded if you have an EV cert), it's merely convenient since I'm not willing to pay for an EV cert

[2025-07-04 23:14] the horse: you can use a previously unknown vulnerable driver

[2025-07-04 23:14] Xits: [replying to the horse: "valorant's anti-cheat runs at boot-time"]
you run before valorant though?

[2025-07-04 23:14] the horse: it's still logged

[2025-07-04 23:14] the horse: because the hooks are in place since system start

[2025-07-04 23:15] elias: [replying to the horse: "<@234331837651091456> does TPM generate those logs..."]
each part of the boot chain measures the next component, so for example the firmware measures secure boot configuration and boot apps before starting them

[2025-07-04 23:15] elias: the firmware code is also measured

[2025-07-04 23:15] elias: linux shim measures any bootloaders it chainloads too

[2025-07-04 23:16] Matti: IDK if you're talking at/past me <@901468996229025873> but I'm aware of all this, and I intentionally make no effort to "fix" the fact that a TPM correctly logs which drivers were loaded

[2025-07-04 23:16] the horse: i'm talking to xits

[2025-07-04 23:16] Matti: same re: various methods to load without disabling DSE

[2025-07-04 23:17] Matti: ok, but your messages do kinda contradict what I'm saying **I** wrote efiguard for

[2025-07-04 23:17] Matti: for myself

[2025-07-04 23:18] Matti: anyone using it or code based on it for cheating is using a cannon to shoot at a mouse

[2025-07-04 23:18] the horse: i mean the whole convo started on the fact that you thought with decent probability that it would be re-using your project

[2025-07-04 23:18] the horse: which i wanted to dismiss as extremely unlikely

[2025-07-04 23:18] Xits: [replying to the horse: "because the hooks are in place since system start"]
Do windows boot drivers run before the call to exitbootservices? I have no idea

[2025-07-04 23:18] Matti: [replying to the horse: "i mean the whole convo started on the fact that yo..."]
true, it did

[2025-07-04 23:19] elias: [replying to Xits: "Do windows boot drivers run before the call to exi..."]
NT drivers don't

[2025-07-04 23:19] Matti: [replying to the horse: "which i wanted to dismiss as extremely unlikely"]
I disagree

[2025-07-04 23:19] the horse: for the most common big boy anti-cheats, imo very unlikely

[2025-07-04 23:19] the horse: for CS and such, sure, seen it being used

[2025-07-04 23:20] Matti: regardless the question wasn't very much probability related - I need proof of something being based on efiguard, and we didn't get any

[2025-07-04 23:20] Matti: if we did,, and it was, then I would be able to sue for violation of the GPL

[2025-07-04 23:20] Matti: that is the reason I asked

[2025-07-04 23:20] the horse: 💸

[2025-07-04 23:21] Matti: money spent defending the GPL is not money wasted

[2025-07-04 23:23] Matti: non-free software is already shit enough, taking free software and redistributing it as non-free is even worse

[2025-07-04 23:25] Matti: lastly, just to clarify something about the cheating aspect here

[2025-07-04 23:25] Matti: I wouldn't epxect any code to be based on *EfiGuard*, though I guess it is possible

[2025-07-04 23:26] Matti: rather this rewrite using gnu-efi seems to be the most commonly used "base" by braindead people
https://github.com/btbd/umap/tree/master
[Embed: GitHub - btbd/umap: UEFI bootkit for driver manual mapping]
UEFI bootkit for driver manual mapping. Contribute to btbd/umap development by creating an account on GitHub.

[2025-07-04 23:27] Matti: oh that's the same one you linked

[2025-07-04 23:27] Matti: I lose track of which is which these days

[2025-07-04 23:27] Matti: but anyway yeah, if you go to the initial commit instead of master it will look a lot more familiar I expect

[2025-07-04 23:28] Xits: I wish I knew about gnu-efi back when I was doing some efi stuff with edk2 😭

[2025-07-04 23:28] Matti: gnu-efi is worse

[2025-07-04 23:29] Matti: and that's coming from someone who fucking hates edk2

[2025-07-04 23:29] Xits: Why? With edk2 I could only get it to compile with my app in the edk2 source directory

[2025-07-04 23:29] Xits: Was aids for me

[2025-07-04 23:29] Matti: hopefully rust-rs will be an improvement over these

[2025-07-04 23:30] Matti: [replying to Xits: "Why? With edk2 I could only get it to compile with..."]
because it uses GCC (probably more important: its linker too)

[2025-07-04 23:31] the horse: uefi-rs is epic, you just setup toolchain, add the crate

[2025-07-04 23:31] the horse: and it works

[2025-07-04 23:31] elias: I managed to compile it to msvc .lib <:steamhappy:1374745522098929778>

[2025-07-04 23:31] the horse: there's no bootservices/system table handling

[2025-07-04 23:31] the horse: it does it for you

[2025-07-04 23:31] Matti: this is always a mistake if you're trying to make a PE file, which EFI binaries are

[2025-07-04 23:32] Xits: Ah well I’m a clang enjoyer. I just assumed it’d work with clang

[2025-07-04 23:32] Matti: hmmmmm

[2025-07-04 23:32] the horse: ```rs
#[inline(never)]
pub fn file_device_path<'a>(
    device_handle: &Handle,
    file_name: &CStr16,
    buffer: &'a mut [MaybeUninit<u8>],
) -> &'a DevicePath {
    let mut builder = DevicePathBuilder::with_buf(buffer);

    let base_path;
    match uefi::boot::open_protocol_exclusive::<DevicePath>(device_handle.clone()) {
        Ok(_path) => base_path = _path,
        Err(_) => unsafe { unreachable_unchecked() },
    }

    for node in base_path.node_iter() {
        match builder.push(&node) {
            Ok(_new) => builder = _new,
            Err(_) => unsafe { unreachable_unchecked() },
        }
    }

    let path = FilePath {
        path_name: file_name,
    };
    match builder.push(&path) {
        Ok(_new) => builder = _new,
        Err(_) => unsafe { unreachable_unchecked() },
    }

    match builder.finalize() {
        Ok(_str) => return _str,
        Err(_) => unsafe { unreachable_unchecked() },
    }
}

#[inline(never)]
pub fn get_bootmgfw_path<'a>(buffer: &'a mut [MaybeUninit<u8>]) -> &'a DevicePath {
    let handles;
    match uefi::boot::locate_handle_buffer(SearchType::from_proto::<SimpleFileSystem>()) {
        Ok(_handles) => handles = _handles,
        Err(_) => {
            unsafe { unreachable_unchecked() };
        }
    }
    for handle in handles.iter() {
        let mut fs: uefi::boot::ScopedProtocol<SimpleFileSystem>;
        match uefi::boot::open_protocol_exclusive::<SimpleFileSystem>(*handle) {
            Ok(fsystem) => fs = fsystem,
            Err(_) => continue,
        }
        let mut root;
        match fs.open_volume() {
            Ok(_root) => root = _root,
            Err(_) => continue,
        }

        match root.open(
            WINDOWS_BOOTMGFW_PATH,
            FileMode::Read,
            FileAttribute::empty(),
        ) {
            Ok(file) => {
                file.close();
                return file_device_path(handle, WINDOWS_BOOTMGFW_PATH, buffer);
            }
            Err(_) => continue,
        }
    }

    unsafe {
        unreachable_unchecked();
    }
}
```

[2025-07-04 23:32] Matti: honestly not sure

[2025-07-04 23:32] the horse: eh in the Ok(_root) you can just => _root and such (let root = match ...)

[2025-07-04 23:32] Matti: but knowing GNU code, I'm going to go with probably not

[2025-07-04 23:32] the horse: gotta update

[2025-07-04 23:33] Matti: besides, if you did use llvm you'd still have to fix gnu-efi to get rid of all of the hacks it uses to force bfd to generate a PE file that may actually load

[2025-07-04 23:33] Matti: and replace it with a sane linker invocation

[2025-07-04 23:33] Matti: otherwise no real benefit

[2025-07-04 23:34] elias: most annoying thing I noticed with gnu efi is that some of the PE related typedefs for x64 have wrong types lol

[2025-07-04 23:35] elias: so when you try to parse some data directories you will have a fun time

[2025-07-04 23:35] Matti: yeah.... types, calling conventions, export names

[2025-07-04 23:35] Matti: GCC's linker can do all of those wrong

[2025-07-04 23:35] Matti: not sometims randomly, but always

[2025-07-04 23:36] Matti: tbf though, EDK2 is also broken in the same way, to a lesser extent but still

[2025-07-04 23:36] Matti: some NT types in it are simply wrong

[2025-07-04 23:37] Matti: <https://github.com/Mattiwatti/EfiGuard/blob/master/EfiGuardDxe/pe.h> this file in particular has to work around quite a bunch of edk2-isms

[2025-07-04 23:38] Matti: by taking the correct types and giving them some other name that is not yet claimed

[2025-07-05 05:16] UJ: [replying to the horse: "uefi-rs is epic, you just setup toolchain, add the..."]
When i was messing with UEFI stuff a couple months back, i found this project that pretty much allowed me to go from empty folder to right click on edk2 sln + build with very little steps. - https://github.com/Th3Spl/SimpleUEFI

Creator of this project is also in this server i believe. 

I might go with rust for the next thing tho since uefi-rs looks interesting and i don't want to take a dependency on visual studio.

[2025-07-05 05:18] the horse: very nice 🙂

[2025-07-05 17:59] Oliver: [replying to UJ: "When i was messing with UEFI stuff a couple months..."]
settings up a edk2 enviroment in windows is not that hard. VisualUefi is quite outdated sadly and does not provide all the packages

[2025-07-05 18:00] Oliver: but its for sure way to complicated then it should be

[2025-07-05 18:02] Oliver: Here is a very good guide on how to do it: https://www.youtube.com/watch?v=jrY4oqgHV0o
[Embed: EDK II: Build system setup using MSVC on Windows]
1. Install Git: https://git-scm.com/
2. Install Visual Studio or Build Tools: https://visualstudio.microsoft.com/downloads/ 
3. Install Python 3 and check "Add python.exe to path", so EDK can discover

[2025-07-05 18:03] Oliver: I think maybe you could go the cygwin route aswell. But i recommend this

[2025-07-05 18:06] Deleted User: [replying to Oliver: "Here is a very good guide on how to do it: https:/..."]
literally explains nothing though

[2025-07-05 18:08] Oliver: [replying to Deleted User: "literally explains nothing though"]
how so? it shows how to set it up and compile?

[2025-07-05 18:09] Deleted User: [replying to Oliver: "how so? it shows how to set it up and compile?"]
yeah but in the end it modifies some specific packages and doesn't explain why

[2025-07-05 18:09] Oliver: [replying to Deleted User: "yeah but in the end it modifies some specific pack..."]
can you give me the time line?

[2025-07-05 18:10] Deleted User: [replying to Oliver: "can you give me the time line?"]
from 8:00

[2025-07-05 18:11] Oliver: so target.txt is the file used to give build instructions. So he is specifing which packet he wants to build

[2025-07-05 18:11] Deleted User: [replying to Oliver: "so target.txt is the file used to give build instr..."]
but why those packets?

[2025-07-05 18:11] Oliver: [replying to Deleted User: "but why those packets?"]
its just to show. He could make a custom package and build that if he wants

[2025-07-05 18:12] Deleted User: [replying to Oliver: "its just to show. He could make a custom package a..."]
ah, i see

[2025-07-06 11:07] Eriktion: Quick question:
what features have you guys implemented in your hypervisor tracer/tracing engine to monitor acs and similar? I’ve sort of ran out of ideas

[2025-07-06 20:09] daax: [replying to Eriktion: "Quick question:
what features have you guys implem..."]
start with what features you currently have and what you’re using

[2025-07-06 20:09] daax: maybe there can be some improvements to those depending on how you implemented them