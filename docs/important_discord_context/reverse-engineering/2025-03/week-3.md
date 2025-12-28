# March 2025 - Week 3
# Channel: #reverse-engineering
# Messages: 215

[2025-03-11 14:46] Iconoclastic: virtual network adapter on esxi

[2025-03-15 00:36] pinefin: would ida 9.0 completely load binaries faster than a version like 8.x would?

[2025-03-15 00:43] Pepsi: why don't you just try it?

[2025-03-15 02:12] pinefin: i am

[2025-03-15 02:12] pinefin: i cant tell if its faster or not, thus why im asking

[2025-03-15 02:12] pinefin: it feels like it but at the same time it doesnt

[2025-03-15 02:27] James: [replying to pinefin: "would ida 9.0 completely load binaries faster than..."]
Have you read something that would make you think that it would?

[2025-03-15 02:33] pinefin: [replying to James: "Have you read something that would make you think ..."]
i was looking through the notes, i updated from 7.x

[2025-03-15 02:33] pinefin: i was assuming thats something that they'd try and tackle because loading huge binaries does take quite a while

[2025-03-15 02:33] pinefin: especially on a beefy computer like mine

[2025-03-15 02:34] pinefin: i see the same speeds on my mac vm vs my beefy windows pc

[2025-03-15 02:34] pinefin: this was on 7.x and feels the same now

[2025-03-15 02:36] James: Well they could try, but I'd say the creul reality is that a lot of the analyses they have to do are simply not linear in time.

[2025-03-15 02:37] James: And so as the binaries get larger, the time to analyze grows exponentially.

[2025-03-15 16:14] pinefin: i see

[2025-03-15 16:14] pinefin: yeah i close all tabs and that makes it faster

[2025-03-15 16:14] pinefin: but on gigabyte sized binaries it makes it hard to sit there and let it analyze all of it

[2025-03-15 16:14] pinefin: so i'm considering booting up my second computer as just a idle ida machine

[2025-03-15 17:37] James: I've got an m4 macbook and holy is IDA fast on there. Significantly faster than my desktop. If you have a mac I'd recommend letting it rip on there.

[2025-03-15 18:02] contificate: upsetting when Apple is the only serious laptop creator

[2025-03-15 18:03] James: [replying to contificate: "upsetting when Apple is the only serious laptop cr..."]
unbelievably true. I'd like a windows laptop, but when the macbook is beating my intel desktop, there's no way any intel latop is gonna come close

[2025-03-15 18:04] James: i guess it's more than that actually, looking at the internals of the macbook vs other laptops, there's no comparison

[2025-03-15 18:05] contificate: yeah, I'll need to get a macbook at some point

[2025-03-15 18:05] contificate: new thinkpads are trash

[2025-03-15 18:05] contificate: soldered on RAM, no replaceable battery, worse keyboards, etc.

[2025-03-15 18:05] contificate: so might as well get a macbook if I'm already accepting those things

[2025-03-15 18:05] James: the build quality on them is really amazing

[2025-03-15 18:25] Bloombit: yeah my m4 apple is faster than my 14900k desktop with 3 large radiator fans haha

[2025-03-15 18:31] James: [replying to Bloombit: "yeah my m4 apple is faster than my 14900k desktop ..."]
same experience, its mind boggling

[2025-03-15 20:02] Tr1x: Currently working on a Windows (Version: 26100.3476) Bootloader R&D project, was able to get my prototype (Which involves patching of bootmgfw) working in VMWare but when attempting to execute it on the host I receive a black screen of death. I have patched **ImgpValidateImageHash** and **ImgpFilterValidationFailure** to return zero which still yields the same result, I thought it might be something to do with CFG as I've seen there have been some updates to it with this version of Windows but I would imagine if it was that the behaviour would repeat itself in VMWare. Currently the bootkit executes, only when it returns control flow back to the Windows Bootloader does this occur. Any pointers or suggestions would be great even for getting some diagnostics 👍

[2025-03-15 20:50] donna🤯: [replying to contificate: "yeah, I'll need to get a macbook at some point"]
Ive got an m1 and its fantastic

[2025-03-15 20:52] donna🤯: there really are no laptops that compare

[2025-03-15 20:52] donna🤯: honestly the trackpad itself is enough for me to buy an apple laptop lol

[2025-03-15 20:53] donna🤯: every other laptops trackpad sucks in comparison

[2025-03-15 20:56] contificate: I don't like the tapping shit, I don't have that enabled

[2025-03-15 20:56] contificate: thinkpad trackpads had buttons at bottom, so goos

[2025-03-15 20:57] BWA RBX: [replying to donna🤯: "Ive got an m1 and its fantastic"]
Compiles JSC really well great for Webkit stuff

[2025-03-15 20:57] contificate: apple buttons are fucked within 3 weeks and are like those silly ones that aren't a dedicated button

[2025-03-15 20:57] BWA RBX: [replying to contificate: "apple buttons are fucked within 3 weeks and are li..."]
Oh ye is that right, well explain why I still have all my buttons?

[2025-03-15 20:58] contificate: on what

[2025-03-15 20:58] contificate: your left and right lower click buttons are fucked if you use them more than tapping on the trackpad

[2025-03-15 20:58] BWA RBX: Nope you are wrong it's still like new

[2025-03-15 20:59] contificate: ime they're sloppy and fucked and horrible

[2025-03-15 20:59] BWA RBX: I'm not an angry Scotts man that's why only Irish

[2025-03-15 20:59] contificate: needless to say mac is a joke os as well

[2025-03-15 20:59] contificate: makes it hard for me

[2025-03-15 21:00] BWA RBX: [replying to contificate: "needless to say mac is a joke os as well"]
Blasphemy

[2025-03-15 21:00] contificate: I'm a Linux chad, sorry

[2025-03-15 21:01] contificate: would prefer NetBSD to MacSlopOSX

[2025-03-15 21:01] BWA RBX: You really do hate Mac OSX

[2025-03-15 21:09] contificate: yeah, you try to find some useful tool that's just a free piece of software on Linux

[2025-03-15 21:09] contificate: and get hit with html5up landing pages trying to sell you worse software for 30 dollars paypal

[2025-03-15 21:09] contificate: just an economy of worstcunts

[2025-03-15 21:23] BWA RBX: Honestly I only ever used my Mac for Webkit stuff and that's because I had problems building webkit on Windows even with a VM had issues fuzzing it

[2025-03-15 21:23] BWA RBX: Overall that experience was mitigated using a mac

[2025-03-15 22:07] vmx: [replying to contificate: "soldered on RAM, no replaceable battery, worse key..."]
soldered on ram ??

[2025-03-15 22:07] vmx: jesus christ thinkpads have gone down the shitter

[2025-03-15 22:14] BWA RBX: Soldered on ram but is there room for upgrades?

[2025-03-15 22:15] contificate: yeah

[2025-03-15 22:15] contificate: but it's like

[2025-03-15 22:15] contificate: if the RAM chucks it on your shit, it's over

[2025-03-15 22:15] contificate: had this happen to me on an X1 Carbon

[2025-03-15 22:15] contificate: and never bought that laptop again

[2025-03-15 22:16] pygrum: regular ddr5 p14s gen 4s got upgradeable ram but severely limited, 48gb max iirc

[2025-03-15 22:18] BWA RBX: I've only ever had one laptop come in for repair that had faulty ram that was soldered onto the motherboard it was a lenovo but ultimately I can see why ram that is soldered is a bad choice for a laptop etc

[2025-03-15 22:20] contificate: yeah well pretty sure it's what makes macs nice and fast

[2025-03-15 22:20] contificate: literally unified on the new macs

[2025-03-15 22:22] BWA RBX: Honestly I never had ram issues with mac that come in for repairs only ever lcd replacements or touchpad or rebuilds other than that nothing, and I do think a lot of them might be floating around but overall it seems stable

[2025-03-15 22:22] contificate: think the thing is

[2025-03-15 22:22] contificate: apple users are probably lifers

[2025-03-15 22:22] contificate: probs got the income to support their habit of good hardware, shit OS

[2025-03-15 22:23] contificate: I've only ever owned a iPad Touch 2G

[2025-03-15 22:23] contificate: not into Apple at all

[2025-03-15 22:23] mrexodia: bro

[2025-03-15 22:23] mrexodia: i have two macbooks, two iphones and an ipad

[2025-03-15 22:23] mrexodia: three macbooks*

[2025-03-15 22:23] pygrum: condolences

[2025-03-15 22:23] mrexodia: it's fucking amazing, I'm gonna buy the M4 air

[2025-03-15 22:25] BWA RBX: I honestly think Apple did a good job on some of their stuff, I just haven;'t come across those issues but generally the Macs factory soldering is okayish and should do for the devices lifetime expectancy

[2025-03-15 22:26] mrexodia: my 2015 air is still going strong

[2025-03-15 22:26] mrexodia: only had to replace the battery

[2025-03-15 22:26] pygrum: i cant hate tbf the only macbook i owned was an intel work one

[2025-03-15 22:26] Yoran: [replying to mrexodia: "i have two macbooks, two iphones and an ipad"]
Try apple tv Duncan

[2025-03-15 22:26] 25d6cfba-b039-4274-8472-2d2527cb: if only a mac mini would be economical to get with more ram, would love it as a server box

[2025-03-15 22:26] 25d6cfba-b039-4274-8472-2d2527cb: but gotta go with amd for high ram

[2025-03-15 22:26] contificate: you are wealthy, though, Duncan

[2025-03-15 22:26] BWA RBX: [replying to mrexodia: "only had to replace the battery"]
That too, very rarely but yes batteries are one of the older models I'll have to replace

[2025-03-15 22:26] contificate: all those sales of x32gdb

[2025-03-15 22:27] mrexodia: yeah I wish bro

[2025-03-15 22:27] 25d6cfba-b039-4274-8472-2d2527cb: I heard he's a millionaire from x64dbg linux SLA's

[2025-03-15 22:27] contificate: I'm actually enjoying the port to `ptrace` for *nix architectures http://linux.x64dbg.com/

[2025-03-15 22:27] BWA RBX: I honestly heard <@162611465130475520> you've got a lambo and have stocks

[2025-03-15 22:27] mrexodia: you heard wrong

[2025-03-15 22:27] mrexodia: LOL

[2025-03-15 22:28] BWA RBX: 😂

[2025-03-15 22:28] contificate: he has stockings

[2025-03-15 22:28] contificate: that's about it

[2025-03-15 22:28] mrexodia: <:breadweary:772343763225935882>

[2025-03-15 22:28] mrexodia: I thought you would keep this between us <@687117677512360003>

[2025-03-15 22:29] BWA RBX: public club secrets being exposed in <#835635446838067210> channel, who would've thought

[2025-03-15 22:30] contificate: what happens at x33fcon stays on xvideos

[2025-03-15 22:30] mrexodia: you didn't even come to x33fcon bro

[2025-03-15 22:30] BWA RBX: [replying to contificate: "what happens at x33fcon stays on xvideos"]
BRO SAID XVIDEOS

[2025-03-15 22:30] contificate: too poor

[2025-03-15 22:30] mrexodia: we offered to pay

[2025-03-15 22:30] contificate: well, actually

[2025-03-15 22:30] contificate: when was this

[2025-03-15 22:30] BWA RBX: https://tenor.com/view/jpj-liar-jim-carrey-liarliar-gif-26213491

[2025-03-15 22:31] mrexodia: two years ago I offered

[2025-03-15 22:31] mrexodia: and then again last year

[2025-03-15 22:31] contificate: and I seem to recall nobody followed up about that, can't make plans to fly to Poland on "aha I gotchu bro" in VC

[2025-03-15 22:31] contificate: I was gonna go to the London thing in December

[2025-03-15 22:31] contificate: but then I got laid off

[2025-03-15 22:31] contificate: and had to move home to Scotland the day before

[2025-03-15 22:31] contificate: then they asked me to stay last week

[2025-03-15 22:31] 25d6cfba-b039-4274-8472-2d2527cb: poland is like a 15€ wizz air flight away innit,,,

[2025-03-15 22:31] contificate: comedy

[2025-03-15 22:31] contificate: wizz air? do you get a free casket as well

[2025-03-15 22:32] contificate: or is that pointless because they'll settle for a mass grave in rural Germany

[2025-03-15 22:32] BWA RBX: At least if you go to Poland you will be drunk on 2/3 beers because of your British tolerance

[2025-03-15 22:32] contificate: can't really assemble burnt sludge into a casket

[2025-03-15 22:32] nox: [replying to BWA RBX: "At least if you go to Poland you will be drunk on ..."]
Lol

[2025-03-15 22:32] 25d6cfba-b039-4274-8472-2d2527cb: [replying to contificate: "or is that pointless because they'll settle for a ..."]
you'll burn in the fire, don't worry

[2025-03-15 22:32] nox: In belgium he would be drunk from 1

[2025-03-15 22:32] contificate: praise be

[2025-03-15 22:32] contificate: well, idiots

[2025-03-15 22:32] contificate: I'll have to know

[2025-03-15 22:32] contificate: my country has the highest ABV beer

[2025-03-15 22:32] contificate: so try again

[2025-03-15 22:32] nox: Lies

[2025-03-15 22:32] BWA RBX: [replying to nox: "In belgium he would be drunk from 1"]
Would be funny to see a drunk Colin ramble on about OCaml

[2025-03-15 22:33] nox: what is the ABV

[2025-03-15 22:33] contificate: alcohol by volume

[2025-03-15 22:33] nox: [replying to BWA RBX: "Would be funny to see a drunk Colin ramble on abou..."]
I'd pay money for it to see

[2025-03-15 22:33] nox: [replying to contificate: "alcohol by volume"]
yeah whats the alcohol by volume

[2025-03-15 22:33] nox: like

[2025-03-15 22:33] contificate: > Brewmeister Snake Venom is currently recognised as the strongest beer in the World. It is brewed in Moray from smoked, peated malt using two varieties of yeast, one beer and one Champagne.

[2025-03-15 22:33] contificate: 67.5%

[2025-03-15 22:33] BWA RBX: [replying to nox: "I'd pay money for it to see"]
His poor boyfriend has to deal with it

[2025-03-15 22:33] contificate: I'm from Scotland, not England

[2025-03-15 22:33] nox: [replying to contificate: "67.5%"]
This is not beer

[2025-03-15 22:33] nox: this is distilled

[2025-03-15 22:33] contificate: it is

[2025-03-15 22:33] nox: Beer isn't distilled

[2025-03-15 22:33] BWA RBX: [replying to contificate: "I'm from Scotland, not England"]
Bro that is England

[2025-03-15 22:34] contificate: we're not even allowed to buy alcohol before 10am and after 10pm in Scotland

[2025-03-15 22:34] 25d6cfba-b039-4274-8472-2d2527cb: he's from skohtlend

[2025-03-15 22:34] BWA RBX: Remember how Scotland never won their independence but Ireland did?

[2025-03-15 22:34] contificate: remember how Ireland still reeling from being starved of potatoes by the British

[2025-03-15 22:34] BWA RBX: LMFAO

[2025-03-15 22:34] nox: wow

[2025-03-15 22:35] BWA RBX: [replying to contificate: "remember how Ireland still reeling from being star..."]
Scots where very lucky to have the queen on their side and king

[2025-03-15 22:35] contificate: Scotland should become independent someday

[2025-03-15 22:35] contificate: just not now

[2025-03-15 22:35] contificate: only profitable place in UK is London

[2025-03-15 22:35] BWA RBX: Honestly you should but why do you all vote no?

[2025-03-15 22:35] contificate: we don't

[2025-03-15 22:35] contificate: imagine you're in a car

[2025-03-15 22:36] contificate: there's 4 of you travelling

[2025-03-15 22:36] contificate: and someone says "why don't we vote as to whether the driver should fuck the car right into the central reservation of this motorway"

[2025-03-15 22:36] contificate: and then 2 people vote yes, and 2 people vote no

[2025-03-15 22:36] contificate: but the people voting yes are slightly fatter

[2025-03-15 22:36] contificate: so it's really 2.1 against 2

[2025-03-15 22:36] BWA RBX: LMFAO

[2025-03-15 22:36] contificate: and so the driver kills them all with one quick turn to the right

[2025-03-15 22:37] BWA RBX: God bless the Scots

[2025-03-15 22:37] BWA RBX: LMFAO

[2025-03-15 22:43] Humza: [replying to contificate: "I'm from Scotland, not England"]
https://tenor.com/view/scream-if-you-love-scotland-scotland-brock-lesnar-scream-i-you-love-screaming-gif-17404451258138738829

[2025-03-15 22:44] x86matthew: [replying to contificate: "soldered on RAM, no replaceable battery, worse key..."]
yeah most models are e-waste now sadly

[2025-03-15 22:44] x86matthew: P-series are the closest to the original proper thinkpads, i have the P16 which is pretty nice

[2025-03-15 22:45] Humza: [replying to contificate: "Scotland should become independent someday"]
Fr

[2025-03-15 22:45] Humza: Some guy came up to me in Glasgow city centre a few days ago and started going oab independence to me for like 20 mins

[2025-03-15 22:45] Humza: Couldn’t get away from the guy, kept yapping

[2025-03-15 22:45] Humza: And started making me sign a bunch of petitions and shit

[2025-03-15 22:45] contificate: I'd have hit him with the haymaker

[2025-03-15 22:45] Humza: Real

[2025-03-15 22:51] Tr1x: [replying to Tr1x: "Currently working on a Windows (Version: 26100.347..."]
Update on this. Decided to run it on my other machine same CPU security settings, same windows version and settings. It worked. Probably gonna do a fresh reinstall on my main machine.

[2025-03-15 23:17] Saturnalia: [replying to Humza: "Some guy came up to me in Glasgow city centre a fe..."]
most sane scotsman

[2025-03-15 23:26] avx: [replying to contificate: "wizz air? do you get a free casket as well"]
atleast they tape you to the side of the plane

[2025-03-16 08:19] vmx: [replying to x86matthew: "P-series are the closest to the original proper th..."]
Got a P52 and it's amazing

[2025-03-16 08:19] vmx: that thing's a tank

[2025-03-16 13:47] Ela: Hi guys, I'm trying to reverse engineer a .rom file of a Network Video Recorder however, when I try to use binwalk it doesn't show any information

[2025-03-16 13:47] Ela: does it mean that the full file is encrypted?

[2025-03-16 13:48] Ela: sorry I'm new to reverse engineering

[2025-03-16 13:50] Ela: on all the tutorials I've seen people usually deal with .bin files

[2025-03-16 13:50] Ela: i don't know if that means that I have to find the .bin file for the NVR or the .rom file provides the same information

[2025-03-16 13:53] 𝝅ₙ(Sⁿ) ≅ (ℤ, +): run file on it to see what type of file it detects it as, could be binwalk didn’t detect any signatures as well. But if you want to check if it’s encrypted; I believe it’s -B and check entropy

[2025-03-16 13:53] 𝝅ₙ(Sⁿ) ≅ (ℤ, +): Might be -E, haven’t used it in forever

[2025-03-16 13:53] 𝝅ₙ(Sⁿ) ≅ (ℤ, +): -E, yeah

[2025-03-16 13:57] 𝝅ₙ(Sⁿ) ≅ (ℤ, +): Also cannot remember, but I believe that you’d have to install pyqtgraph or matplotlib through pip, I don’t believe it comes with binwalk either incase you get an error when trying to check the entropy

[2025-03-16 13:57] 𝝅ₙ(Sⁿ) ≅ (ℤ, +): (Whichever it uses, more inclined to say matplotlib)

[2025-03-16 14:02] Ela: [replying to 𝝅ₙ(Sⁿ) ≅ (ℤ, +): "run file on it to see what type of file it detects..."]
yeah I ran file on it and it doesn't detect anything it just says "data"

[2025-03-16 14:02] 𝝅ₙ(Sⁿ) ≅ (ℤ, +): Okay, check entropy

[2025-03-16 14:02] 𝝅ₙ(Sⁿ) ≅ (ℤ, +): binwalk -E file.rom

[2025-03-16 14:03] Ela: 
[Attachments: image.png]

[2025-03-16 14:03] Ela: this is the entropy graph

[2025-03-16 14:03] 𝝅ₙ(Sⁿ) ≅ (ℤ, +): Okay yeah, looks encrypted.

[2025-03-16 14:03] Ela: seems like all of it is encrypted

[2025-03-16 14:03] Ela: the worst part is that I have no clue of what encryption algo they are using

[2025-03-16 14:04] 𝝅ₙ(Sⁿ) ≅ (ℤ, +): Try running something like strings on it. The header part looks odd

[2025-03-16 14:06] Ela: yeah it seems unecrypted but the strings are just nonsense

[2025-03-16 14:06] Ela: this is the web page where I got the firmware from

[2025-03-16 14:06] Ela: http://helpen.dvr163.com/index.php/Mainpage
[Embed: Mainpage]

[2025-03-16 14:07] 𝝅ₙ(Sⁿ) ≅ (ℤ, +): Ah okay, yeah it’s hard to tell; not sure if there’s any sort of software that would help with cryptanalysis. One method that I’ve seen before

[2025-03-16 14:07] 𝝅ₙ(Sⁿ) ≅ (ℤ, +): So read the manual; depending on the software I’ve seen it where a base level firmware is required and that firmware would be used to decrypt the newer firmware and turns out the base firmware was not encrypted

[2025-03-16 14:07] 𝝅ₙ(Sⁿ) ≅ (ℤ, +): So you could find the algorithm for decryption, reverse it, then implement your own to decrypt the newer firmware.

[2025-03-16 14:08] Ela: mn... you know what, that might be the case, because I found the file in the upgrades section

[2025-03-16 14:09] 𝝅ₙ(Sⁿ) ≅ (ℤ, +): ah okay, yeah check the original firmware image

[2025-03-16 14:12] 𝝅ₙ(Sⁿ) ≅ (ℤ, +): If I don’t reply, I passed out. It’s late af where I’m at but I’ll reply when I wake up. Gather all the stuff you can; base firmware, documents (manuals, patch notes, etc.) could clue you in to things that might help. In the case I mentioned before, it states that you must have v blah.blah.blah in order to upgrade (doesn’t say this, but it’s because the decryption method is in the firmware lol)

[2025-03-16 14:22] Ela: [replying to 𝝅ₙ(Sⁿ) ≅ (ℤ, +): "ah okay, yeah check the original firmware image"]
I can't find the original firmware image, but I was looking at the upgrade instructions, the system has a full GUI that seems to be running on a windows OS, the instructions only mention pressing some buttons in the GUI, however the GUI says the following If failed, please insert USB storage and make sure help.rom exists.

[2025-03-16 14:22] Ela: 
[Attachments: image.png]

[2025-03-16 14:23] 𝝅ₙ(Sⁿ) ≅ (ℤ, +): Do you have help.rom?

[2025-03-16 14:23] Ela: maybe that help.rom file is the one performing decryption

[2025-03-16 14:23] Ela: [replying to 𝝅ₙ(Sⁿ) ≅ (ℤ, +): "Do you have help.rom?"]
nope

[2025-03-16 14:24] 𝝅ₙ(Sⁿ) ≅ (ℤ, +): okay, try to find that.