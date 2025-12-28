# July 2025 - Week 4
# Messages: 325

[2025-07-22 07:07] moshui: Does anyone know that VGK does not intercept my driver? My driver is signed with a leaked certificate. Is this a normal phenomenon? Will VGK not intercept drivers?

[2025-07-22 13:58] daax: [replying to moshui: "Does anyone know that VGK does not intercept my dr..."]
Are you using it to cheat? Odds are yes, you’re already dead in the water. Signed with a leaked cert is usually going to get you flagged out of the gate.

[2025-07-23 02:04] moshui: only signed me？

[2025-07-24 08:41] TenuousMeal: Is there any particular reason besides offsets to why most bootkit that can be found stop at Windows 11 23h2 ?

[2025-07-24 08:41] TenuousMeal: Was there a major change ?

[2025-07-24 08:43] Oliver: [replying to TenuousMeal: "Is there any particular reason besides offsets to ..."]
most likelt because winload signatures update

[2025-07-24 08:44] Oliver: and most are pasters and cant find them

[2025-07-24 08:44] TenuousMeal: I remember reading that most are a paste of Efiguard

[2025-07-24 08:44] Oliver: [replying to TenuousMeal: "I remember reading that most are a paste of Efigua..."]
ye that was the set/get var hook

[2025-07-24 08:44] Oliver: but havnt seen that in a while

[2025-07-24 08:45] Oliver: most are paste of umap or similar i guess

[2025-07-24 08:45] Oliver: but my https://github.com/Oliver-1-1/SmmInfect bootkit still works on newest windows i belive, but when i made it i tried to make it work for all windows versions
[Embed: GitHub - Oliver-1-1/SmmInfect]
Contribute to Oliver-1-1/SmmInfect development by creating an account on GitHub.

[2025-07-24 08:45] TenuousMeal: I thought maybe there were additionnal measures to protect the kernel, since updating offsets if you are writing bootkits shouldn't be too hard

[2025-07-24 08:46] TenuousMeal: [replying to Oliver: "but my https://github.com/Oliver-1-1/SmmInfect boo..."]
All that SMM stuff seems very interesting but I don't know how I feel about risking to fry my motherboard ;D

[2025-07-24 08:46] Oliver: [replying to TenuousMeal: "All that SMM stuff seems very interesting but I do..."]
do u have bios flashback?

[2025-07-24 08:46] TenuousMeal: I think Asus supports it ? This is a pretty recent motherboard

[2025-07-24 08:46] TenuousMeal: I have a Pi so I would need to buy the wires to connect right ?

[2025-07-24 08:47] Oliver: [replying to TenuousMeal: "I think Asus supports it ? This is a pretty recent..."]
ye i have asus prime b650-plus

[2025-07-24 08:47] TenuousMeal: Did you write it yourself ? Or did you use the base that I saw circulating ?

[2025-07-24 08:48] Oliver: [replying to TenuousMeal: "Did you write it yourself ? Or did you use the bas..."]
which base?

[2025-07-24 08:48] Oliver: no i wrote it myself

[2025-07-24 08:48] TenuousMeal: I remember reading a very interesting article with a solid SMM base for anticheat evasion so I thought you might have

[2025-07-24 08:49] TenuousMeal: <@188989065373155328>I'm really curious, what do you manage to run with the limitations of SMM ?

[2025-07-24 08:49] TenuousMeal: If you don't mind of course

[2025-07-24 08:49] Oliver: [replying to TenuousMeal: "<@188989065373155328>I'm really curious, what do y..."]
what limitations?

[2025-07-24 08:50] TenuousMeal: Well. I'm sorry if it sounds stupid this is not something I have worked with

[2025-07-24 08:50] TenuousMeal: I remember reading you have to rely on being called

[2025-07-24 08:50] Oliver: its ok

[2025-07-24 08:50] TenuousMeal: You do not actively run right ?

[2025-07-24 08:50] Oliver: like amd supervisor is nothing that is deployed from what ive seen

[2025-07-24 08:50] Oliver: only seen it in one laptop firmware

[2025-07-24 08:50] Oliver: [replying to TenuousMeal: "You do not actively run right ?"]
oh

[2025-07-24 08:50] Oliver: ye

[2025-07-24 08:50] TenuousMeal: I thought smm only worked on interrupts

[2025-07-24 08:50] Oliver: i trigger my smi through acpi

[2025-07-24 08:51] Oliver: tpm specifilcly

[2025-07-24 08:51] TenuousMeal: Which is a pretty big limitation

[2025-07-24 08:51] Oliver: you can also trigger them from hardware or uefi runtime functions

[2025-07-24 08:51] TenuousMeal: Yeah that stuff is called often right ?

[2025-07-24 08:51] TenuousMeal: I guess that's 's why you use it

[2025-07-24 08:51] Oliver: https://github.com/Oliver-1-1/SmmInfect/blob/adfe8420da5a020fb81685162d6952713c6dc691/SmiUmWin/SmiUm/entry.cpp#L123
[Embed: SmmInfect/SmiUmWin/SmiUm/entry.cpp at adfe8420da5a020fb81685162d695...]
Contribute to Oliver-1-1/SmmInfect development by creating an account on GitHub.

[2025-07-24 08:51] TenuousMeal: I should find the motivation to write my own APIC one day

[2025-07-24 08:52] TenuousMeal: Thanks

[2025-07-24 08:52] TenuousMeal: I guess the answer to the original question was just : pasters problems <:kekw:904522300257345566>

[2025-07-24 08:53] Oliver: [replying to TenuousMeal: "I guess the answer to the original question was ju..."]
thats my guess, i dont see windows being able to do anything regarding bootkits which would be that effective

[2025-07-24 09:00] UJ: [replying to Oliver: "do u have bios flashback?"]
what do you use to flash your bios with your cfw on it?

[2025-07-24 09:00] Oliver: [replying to UJ: "what do you use to flash your bios with your cfw o..."]
bios flashback button

[2025-07-24 09:01] Oliver: i modify it with UefiTool

[2025-07-24 09:01] Oliver: replace another smm driver that is non-essential

[2025-07-24 09:03] UJ: hmm ok. i also have a bios flashback button and when i tried using it to unbrick that after i bricked my system it didnt work lol i had to use a MSI JSPI1 cable + fashrom to get back to a working state.

[2025-07-24 09:04] Oliver: [replying to UJ: "hmm ok. i also have a bios flashback button and wh..."]
hmm

[2025-07-24 09:04] Oliver: ive bricked my plenty of times

[2025-07-24 09:04] Oliver: then i just plug the usb in my laptop and replace the firwmare with a good one

[2025-07-24 09:04] Oliver: and flash

[2025-07-24 09:05] Oliver: [replying to UJ: "hmm ok. i also have a bios flashback button and wh..."]
what mobo u got?

[2025-07-24 09:05] UJ: MSI PRO Z890-P WIFI

[2025-07-24 09:08] Oliver: do u know the bios flash back works?

[2025-07-24 09:08] Oliver: like have u tried flashing on something on it?

[2025-07-24 09:08] Oliver: so u know it works

[2025-07-24 09:09] Oliver: or it just fails?

[2025-07-24 09:09] UJ: the bios img i built was the most bootleg thing ever built but it shouldnt matter since the bios flashback is just hardware based.

[2025-07-24 09:09] Oliver: cuz the usb needs to be fat32

[2025-07-24 09:09] UJ: yeah it def works from a known working state, just not the 1 time i actually needed it to work when i bricked

[2025-07-24 09:10] UJ: [replying to Oliver: "like have u tried flashing on something on it?"]
yep

[2025-07-24 09:10] Oliver: [replying to UJ: "yep"]
hmm weird sounds like the mobo is faulty or something

[2025-07-24 09:12] UJ: yeah, in either case i have fpt and flashrom if i ever brick now. was just a weird case for me where flashback didnt work.

[2025-07-24 10:42] Matti: another limitation of at least asus' BIOS flashback that is not as well known and can bite you is the requirement that the USB stick be **MBR** FAT32.  GPT ones will simply not be detected (regardless of filesystem)
I wouldn't be surprised if MSI has the same limitation - chances are good both are essentially AMI's code with their brand name on them

[2025-07-24 10:42] Brit: [replying to Matti: "another limitation of at least asus' BIOS flashbac..."]
can confirm that it does

[2025-07-24 10:42] Brit: t. msi board owner

[2025-07-24 10:43] Matti: <:120fps:626884674349694996>

[2025-07-24 10:46] Matti: in general I don't trust modern 'flashback' type BIOS-rescue either way, as the flashing itself is implemented in firmware which means it is able to think about what you are feeding it and make (wrong) decisions based on this, like rejecting the flash because it doesn't agree that my signature is more trusted than asus's

[2025-07-24 10:46] Matti: MSI used to have actual physical switches on their dual BIOS motherboards

[2025-07-24 10:46] Brit: it is actually stupidly anti consumer

[2025-07-24 10:47] Brit: what if asus goes defunct

[2025-07-24 10:47] Brit: who will be able to patch the bios then

[2025-07-24 10:47] Matti: only those armed with the mighty CH347 will survive in the end...

[2025-07-24 10:48] Brit: a whole 5 bucks

[2025-07-24 10:48] Matti: pure hardware dd-style flashing is the only way to be sure

[2025-07-24 10:48] Brit: defintiely agree

[2025-07-24 10:49] Matti: though, I have to credit intel with making intel FPT as close to the equivalent thing in software

[2025-07-24 10:49] Matti: it's just a shame it's NDA and for the wrong CPUs

[2025-07-24 10:50] Brit: 
[Attachments: image.png]

[2025-07-24 10:50] Brit: is it time to get a hundred of these

[2025-07-24 10:50] Matti: flashrom also will just do the right thing, **if** it supports your motherboard
but that's a big if

[2025-07-24 10:50] Brit: and give them as presents to all the nerds I know

[2025-07-24 10:51] Matti: OK if you wanna buy CH347s for HW flashing (definitely a good idea) you do probably want to buy one of those 'kits' on ali rather than just literally the programmer, you can't do much with that by itself

[2025-07-24 10:51] Matti: one sec, let me try to find something

[2025-07-24 10:52] Brit: I used an rpi to flash libreboot before

[2025-07-24 10:52] Brit: that was kind of a pain

[2025-07-24 10:52] Matti: <https://de.aliexpress.com/item/1005006530290946.html?algo_exp_id=e58b6dea-d01c-4344-ac1f-f7f0ac232485-0&pdp_ext_f=%7B%22order%22%3A%227096%22%2C%22eval%22%3A%221%22%7D&pdp_npi=4%40dis!EUR!1.80!1.80!!!14.76!14.76!%40211b615317533543349198132e0d5a!12000037547272940!sea!NL!2195286222!X&curPageLogUid=OXO6z7edJNVh&utparam-url=scene%3Asearch%7Cquery_from%3A>
sorry for the gore link

[2025-07-24 10:53] Matti: lmk if it works, I've noticed ali links tend to not always work depending on where you live

[2025-07-24 10:54] Matti: get the '1 set' (or '2 set if you want') variant

[2025-07-24 10:55] Matti: and then when it arrives, **chuck the CH341A programmer in the bin** because it has a circuit design flaw, meaning it provides 5V on the 3.3V line

[2025-07-24 10:55] Brit: <:topkek:904522829616263178>

[2025-07-24 10:55] Brit: a free bricking device

[2025-07-24 10:56] Matti: also it's 10-20x slower than the CH347 (in flashrom anyway)

[2025-07-24 10:56] Horsie: Jeez.. All that for <1$

[2025-07-24 10:56] Matti: nah 5V won't brick a chip... usually

[2025-07-24 10:56] Matti: but it's not great

[2025-07-24 10:56] Matti: I've used these PoC (programmers of colour) before in a pinch and motherboards targeted have survived

[2025-07-24 10:57] Horsie: I've only bricked my board once by putting the clip on sloppily

[2025-07-24 10:57] Horsie: Think I shorted and killed the chip

[2025-07-24 10:57] Matti: to clarify it's only the black ones that have this 5V fault

[2025-07-24 10:57] Brit: [replying to Matti: "I've used these PoC (programmers of colour) before..."]
thats so HR coded bestie

[2025-07-24 10:57] Oliver: [replying to Matti: "another limitation of at least asus' BIOS flashbac..."]
i mean if he could flash sucessfully other times except if it was bricked something else must be faulty?

[2025-07-24 10:58] Matti: oh, I missed that if so

[2025-07-24 10:58] Matti: then yeah

[2025-07-24 10:58] Oliver: i can unbrick my mobo with bios flashback

[2025-07-24 10:59] Matti: I mean he could have switched to a GPT stick by chance as well, but it was already a fairly unlikely guess to begin with

[2025-07-24 10:59] Matti: I only mentioned it because it's so impossible to determine that this is the cause **if** it is the cause

[2025-07-24 10:59] Matti: [replying to Oliver: "i can unbrick my mobo with bios flashback"]
well sure, me too

[2025-07-24 11:01] Matti: but I don't use or trust it for programming a modified BIOS, I would only ever use it (and even then still prefer to flash a known good backup via HW...) to recover to a clean state with the vendor BIOS on it

[2025-07-24 11:02] Matti: I always make a backup of all SPI chips on every motherboard I buy, before turning it on

[2025-07-24 11:02] Matti: especially when dealing with the intel(R) CSME this has saved me a bunch of times

[2025-07-24 11:02] Matti: it doesn't normally like being downgraded

[2025-07-24 11:02] Matti: or modified

[2025-07-24 11:04] Matti: oh yeah <@303272276441169921> - nearly forgot to mention this but it's also imperative to only ever use chinesium flashers like the CH347 with flashrom

[2025-07-24 11:04] Matti: don't use or trust any other flashing sw

[2025-07-24 11:04] Oliver: [replying to Matti: "especially when dealing with the intel(R) CSME thi..."]
i tried to patch the amd psp blob a couple of days ago and then reflashed with a normal bios and it worked just fine

[2025-07-24 11:05] Oliver: i dont have a intel platform sadly

[2025-07-24 11:05] Matti: yeah but the PSP doesn't have these issues, it's the ME specifically

[2025-07-24 11:05] Matti: [replying to Oliver: "i dont have a intel platform sadly"]
sadly he says

[2025-07-24 11:06] Matti: well done I say

[2025-07-24 11:06] Oliver: ye amd is a blast tbh

[2025-07-24 11:06] Oliver: most firmware is one to one to edk2 repo

[2025-07-24 11:07] Oliver: planning to buy a arm64 platform and see how life is there

[2025-07-24 11:07] Matti: hmmmm

[2025-07-24 11:08] Matti: well if you're very UEFI-oriented, then it's gonna take a bit of adjusting for sure

[2025-07-24 11:08] Matti: unless you buy an rpi which does use UEFI by default I think

[2025-07-24 11:09] Oliver: the new windows arm pc boot with uefi?

[2025-07-24 11:09] Matti: most ARM SoCs don't necessarily have either UEFI or ACPI

[2025-07-24 11:09] Oliver: [replying to Matti: "most ARM SoCs don't necessarily have either UEFI o..."]
acpi is x86 right?

[2025-07-24 11:10] Matti: no, it's not specific to x86, though in reality your statement isn't a bad rule of thumb (hehe... ARM... THUMB... geddit)

[2025-07-24 11:10] Brit: <:mmmm:904523247205351454>

[2025-07-24 11:10] Oliver: [replying to Matti: "no, it's not specific to x86, though in reality yo..."]
ye u are correct did a quick google. no clue where i got that impression from

[2025-07-24 11:11] Matti: I've got a rockchip devkit board with 8 cores, it supports various combinations of UEFI or.... not UEFI, and ACPI vs devicetree or both

[2025-07-24 11:12] Matti: many of these settings are some combination of half-implemented and/or emulated via the 'native' layer

[2025-07-24 11:13] Oliver: [replying to Matti: "I've got a rockchip devkit board with 8 cores, it ..."]
ye at work we have some devicetrees guys for arm embedded linux. Thats prob why i associated acpi with x86

[2025-07-24 11:14] Matti: windows 11 does in fact boot on it, but this required me to flash a custom FW and install a specially prepared windows image with my own modified versions of drivers with fixes specific to rockchip SoCs or the devices on these boards

[2025-07-24 11:14] Matti: it was not fun

[2025-07-24 11:15] Oliver: [replying to Matti: "it was not fun"]
ye sounds like a lot of hours

[2025-07-24 11:15] Oliver: ngl

[2025-07-24 11:15] Oliver: for what benefit? xD

[2025-07-24 11:15] Matti: on the other hand, the board is much faster than an rpi, so windows is actually fairly useable

[2025-07-24 11:15] Oliver: how about the tpm requirements? did u install one?

[2025-07-24 11:16] Matti: well, I wanted an ARM dev board, and for me that means it should be able to boot at least both linux and windows in order to be useful

[2025-07-24 11:17] Matti: [replying to Oliver: "how about the tpm requirements? did u install one?"]
ah fuck no, I just applied the usual patch everyone does so the ISO will ignore SB/TPM requirements

[2025-07-24 11:17] Matti: I mean I would have a hard time booting with self-compiled nvme drivers anyway with secure boot on

[2025-07-24 11:18] Matti: I could of course install my own keys on the board but really I just prefer it to be off anyway

[2025-07-24 11:18] Oliver: [replying to Matti: "ah fuck no, I just applied the usual patch everyon..."]
ye, thats sounds like the best approach xD

[2025-07-24 11:19] Oliver: i wonder when linux desktop ever gonna take advantage of microsofts corp greed

[2025-07-24 11:24] Matti: well it'd have to become usable first

[2025-07-24 11:25] Matti: so... never?

[2025-07-24 11:25] Matti: MS and linux haven't been arch enemies for a long time though

[2025-07-24 11:25] Matti: I think more than half of azure runs linux of some kind

[2025-07-24 11:26] Matti: they even grudgingly contribute GPL code for hyper-v support to the kernel

[2025-07-24 11:29] Matti: picture of my rockchip board off aliexpress btw
[Attachments: image.png]

[2025-07-24 11:29] Matti: boot time is about as fast as my 5950X desktop

[2025-07-24 11:30] Matti: and shutdown is instant thanks to the firmware
[Attachments: image.png]

[2025-07-24 11:38] Horsie: safety anti-static carpet

[2025-07-24 11:38] Horsie: :)

[2025-07-24 11:39] Oliver: [replying to Matti: "picture of my rockchip board off aliexpress btw"]
looks cool

[2025-07-24 11:39] Oliver: [replying to Matti: "so... never?"]
facts

[2025-07-24 11:48] Matti: [replying to Oliver: "looks cool"]
literally is cool as well, note the lack of even a heatsink for passive cooling

[2025-07-24 11:51] Matti: both the anti-static bag and lack of heatsink are gone now though, I installed an aluminium case on it which doubles as a passive heatsink

[2025-07-24 11:52] Matti: it looks a lot neater now

[2025-07-24 11:53] Matti: feels very weird and pointlessly apple-like to have a case around any device I use for debugging

[2025-07-24 11:54] Matti: but I've noticed that my tendency not to use cases for anything ever is really just kind of a bad habit that turns my apt into a mess over time every time

[2025-07-24 12:06] Oliver: [replying to Matti: "literally is cool as well, note the lack of even a..."]
How is the temps?

[2025-07-24 12:07] Oliver: My raspi often gets low key hot

[2025-07-24 12:07] Oliver: Never measured it but ye

[2025-07-24 12:10] Matti: no clue tbh, I haven't really looked at this at all beyond noticing that it's an 8 core SoC that doesn't even care if there's no heatsink installed

[2025-07-24 12:11] Matti: in the pentium 4 days this would have been the quickest way to kill your CPU, just by turning on the system

[2025-07-24 12:12] Matti: I haven't run heavy loads on the thing though, neither CPU nor GPU

[2025-07-24 12:13] Matti: I mostly just got it because booting into anything on my rpi4 was just mind numbingly slow

[2025-07-24 12:14] Matti: not an issue with the rockchip

[2025-07-24 12:14] Matti: and it's still pretty affordable, I think $150-$200 on ali depending on your board config

[2025-07-24 12:15] Matti: also the rpi4 literally cannot boot windows 11 anymore these days <:lillullmoa:475778601141403648>

[2025-07-24 12:15] Matti: no ARMv9 support

[2025-07-24 12:35] ACC RESTRICTED ADD NEW ACC: [replying to Matti: "boot time is about as fast as my 5950X desktop"]
5950X is a decent CPU

[2025-07-24 12:35] ACC RESTRICTED ADD NEW ACC: I have one myself

[2025-07-25 00:29] UJ: [replying to Oliver: "i can unbrick my mobo with bios flashback"]
i just recovered from a bricked system with the bios flashback on the same usb drive (i recall it was def fat32 and mbr back then as well) on the same system. so yeah, not sure what went wrong that time.

[2025-07-25 07:20] Oliver: [replying to UJ: "i just recovered from a bricked system with the bi..."]
nice good job, sometimes u wonder why things goes wrong. Im a former avionics engineer so i can always blame cosmic radiation.

[2025-07-25 07:30] UJ: im 90% sure the issue is what matti mentioned above. the flashback is implemented in fw and it's "thinking" instead of just doing a blind copy. today i just swapped out the ME region from my bios with a clean one of the same version and for some reson it bricked and flashback got me back into a working state so i can reflash my dumped bios.

[2025-07-25 07:30] Xyrem: hate em cosmic rays

[2025-07-25 07:52] Oliver: https://tenor.com/view/no-fuck-you-mad-mean-angry-gif-5685671

[2025-07-25 19:52] unknown: [replying to UJ: "im 90% sure the issue is what matti mentioned abov..."]
> flashback is implemented in fw
pls correct if im getting it wrong but if the firmware gets corrupted how will the flashback work then?

[2025-07-25 20:01] UJ: [replying to unknown: "> flashback is implemented in fw
pls correct if im..."]
its not the same fw, it has to be its own ic for exactly the reason you said. for example when you do bios flashback with the rom from the oem website, the uuid in the bios is still an actual guid, but when i flash same rom using fpt or flash rom, the uuid changes to all FFFs (and who knows what other serials are destroyed). 

the bios flashback fw is making decisions on what to copy, and if it can make decisions on what to copy, it seems like it can also make a decision to not copy if some bytes are unexpected or w/e (even tho the bios it rejected could boot).

[2025-07-25 20:02] unknown: [replying to UJ: "its not the same fw, it has to be its own ic for e..."]
this makes sense, so like the bios/uefi from oem is on a seperate chip from the flashback firmware right?

[2025-07-25 20:08] UJ: [replying to Matti: "<https://de.aliexpress.com/item/1005006530290946.h..."]
yes. not sure if you have access to this link or not but the bios chip is the one you put that clip thing on.

[2025-07-26 02:34] Matti: [replying to ACC RESTRICTED ADD NEW ACC: "5950X is a decent CPU"]
well it depends

[2025-07-26 02:35] Matti: the CPU has 16 cores which is plenty for normal usage, but painfully low when compiling large code bases

[2025-07-26 02:35] Matti: the platform is a bigger problem for me

[2025-07-26 02:36] Matti: on consumer platforms you're limited to 128 GB of RAM and like 3.5 PCIe lanes

[2025-07-26 02:37] Matti: I have 128 GB and I have to manually limit the core count in UE's build system or I go OOM

[2025-07-26 02:38] Matti: because memory usage scales linearly with core count when compiling, if I upgrade to 64 or 128 cores this will become an even bigger issue unless I increase the amount of RAM correspondingly

[2025-07-26 02:39] Matti: and the PCIe lane problem is just simply that 3.5 lanes or whatever it is, isn't enough

[2025-07-26 02:43] Matti: an epyc turin ES CPU from ebay will give you 128 cores, 512 MB L3 and essentially unlimited 12 channel physical memory

[2025-07-26 02:43] Matti: and 160 PCIe lanes

[2025-07-26 02:43] Matti: for 3K USD

[2025-07-26 02:45] Matti: [replying to Matti: "because memory usage scales linearly with core cou..."]
the problem is DDR5 (especially ECC RDIMMs which you need) is unaffordable if you keep this in mind too

[2025-07-26 02:46] Matti: and you'll want to buy them in multiples of 12 per CPU

[2025-07-26 02:49] Matti: btw threadrippers aren't a realistic alternative because they are only really useful if you buy a TR pro, which you aren't going to find in ES form on ebay

[2025-07-26 02:50] Matti: so the 3K 128 core CPU becomes an 11K 64 core CPU instead

[2025-07-26 02:50] Matti: without fixing any of the issues related to the cost of memory because it uses the same

[2025-07-26 02:51] Matti: also TRs are consistently a generation behind epycs now

[2025-07-26 02:52] Matti: AMD isn't taking them seriously as a platform because server CPUs are like 100x as much in demand

[2025-07-26 03:36] selfprxvoked: [replying to Matti: "on consumer platforms you're limited to 128 GB of ..."]
AMD now supports 192 GB

[2025-07-26 03:37] selfprxvoked: it is still dogshit for what you want to achieve

[2025-07-26 04:26] Matti: oh yeah, you're right of course

[2025-07-26 04:26] Matti: back when I built this system 48GB modules weren't avaailable yet

[2025-07-26 04:27] Matti: there might even be 64GB non-RDIMMs now, though not sure if they bother making them as DDR4

[2025-07-26 04:28] Matti: but yeah in any case I'm pretty dead set on not buying a consumer platform ever again

[2025-07-26 04:29] Matti: the number of PCIe lanes alone is enough reason to be honest

[2025-07-26 22:46] Oliver: gotta compile chrome repo in 5 secs

[2025-07-26 23:10] Matti: I don't

[2025-07-26 23:10] Matti: not yet anyway

[2025-07-26 23:11] Matti: but that's only because the RAM for it is unaffordable right now

[2025-07-26 23:12] Matti: so why do I want one: because a turin 9755 ES is by far the best price/performance ratio CPU you can get on ebay

[2025-07-26 23:13] Matti: and it has 128 cores

[2025-07-26 23:14] Matti: a 9575F would probably be slighly more suitable for my workloads, but it's also twice the price, half the cores and half the L3 cache, so absolutely not worth it to me

[2025-07-26 23:17] Matti: [replying to Oliver: "gotta compile chrome repo in 5 secs"]
imagine wasting cpu cycles on compiling chromium

[2025-07-26 23:17] Matti: lol

[2025-07-26 23:20] Oliver: [replying to Matti: "imagine wasting cpu cycles on compiling chromium"]
I mean someone gotta do it😂

[2025-07-26 23:20] Matti: not for me! I haven't had trash installed for I dunno, 8 years?

[2025-07-26 23:21] Matti: chromium is for serving you ads

[2025-07-26 23:21] Oliver: What browser u on?

[2025-07-26 23:21] Matti: firefox

[2025-07-26 23:21] Matti: get with the times

[2025-07-26 23:21] Oliver: I do mullvad browser, edge and chrome

[2025-07-26 23:21] Oliver: Mostly chrome sadly

[2025-07-26 23:21] Matti: I do ublock origin

[2025-07-26 23:37] the horse: edge is pretty good

[2025-07-26 23:38] the horse: nowadays

[2025-07-26 23:38] the horse: my go-to browser because one of my shitty pentium laptops barely runs media w firefox

[2025-07-27 01:07] wanderer: im trying to make a program that allows you to use custom bootscreens

[2025-07-27 01:08] wanderer: to do that, i need a way of drawing on the screen in nt native mode (like when autochk runs)

[2025-07-27 01:09] wanderer: now i had some random signed driver that exposed funcs like InbvSolidColorFill over a dosdevice

[2025-07-27 01:10] wanderer: and you could draw over that and it works on vista and 7

[2025-07-27 01:10] wanderer: but in 8 they added BGRT and the BGRT implementation of SolidColorFill just sets the text color for some reason?

[2025-07-27 01:10] wanderer: so the only thing you can really do anymore is ascii art, you can't even change the font because that's checked by code integrity

[2025-07-27 01:11] wanderer: anyway the framebuffer that BGRT uses for drawing isnt exposed to usermode anywhere

[2025-07-27 01:11] wanderer: if you're in a driver, you can call ZwQuerySystemInformation(SystemBootGraphicsInformation) and the struct will hand you a pointer to the framebuffer and some details and you can draw on it after mapping

[2025-07-27 01:11] wanderer: but i dont have money for driver signing

[2025-07-27 01:12] wanderer: so i found a random signed not-blacklisted copy of WinIO that lets you r/w to kernel memory and i pattern scan for ntoskrnl, find the framebuffer, and draw on it through there

[2025-07-27 01:12] wanderer: but im worried about anticheats banning people because it seems like it'd be pretty trivial to scan random drivers on disk for WinIO strings

[2025-07-27 01:13] wanderer: in theory i could just unload the driver after im done and then delete it on disk, but that seems more questionable

[2025-07-27 01:14] wanderer: and if i go cover my tracks "properly" and miss one spot im basically guaranteeing a ban

[2025-07-27 01:16] wanderer: so i guess my question is if i unload the driver and do nothing else will most ACs ban people or will they just flag the driver if/when it gets flagged and then people can't play with it loaded? so it becomes more of a "why does this game not work" issue than a "i got banned from this game for using this" issue

[2025-07-27 01:16] wanderer: thanks

[2025-07-27 01:39] the horse: i am using windows, edge spyware is the least of my concerns

[2025-07-27 01:39] the horse: the privacy is already an illusion

[2025-07-27 01:40] the horse: I don't have to, edge works for me better than firefox, therefore I don't need to seek an alternative; especially on a "movie" laptop

[2025-07-27 01:40] the horse: I need to test some of my programs from time to time there as well, so linux not rlly viable

[2025-07-27 01:41] the horse: also not sure how DRM content works with linux nowadays

[2025-07-27 01:41] the horse: firefox just barely works with drm content

[2025-07-27 01:41] the horse: probably like 4gb

[2025-07-27 01:41] the horse: not sure

[2025-07-27 01:41] the horse: it's very lowend

[2025-07-27 01:41] the horse: celeron

[2025-07-27 01:41] the horse: or pentium or w/e

[2025-07-27 01:42] the horse: i'm not sure if it even supports virtualization

[2025-07-27 01:42] the horse: 😄

[2025-07-27 02:15] the horse: only some celeron cpus have virtualizaiton capabilities

[2025-07-27 02:16] the horse: barely but it does 😄

[2025-07-27 02:16] the horse: electron apps unfortunately destroy the system

[2025-07-27 02:16] the horse: so no discord!

[2025-07-27 02:20] the horse: rapid development tooling has its downsides

[2025-07-27 02:21] the horse: to be fair, this is mostly an issue on obsolete hardware anyway; there's not that many desktop apps for this to really make this argument have that much substance

[2025-07-27 02:22] the horse: I've seen electron apps that do well resource-wise

[2025-07-27 02:22] the horse: discord is just unbelievably cluttered

[2025-07-27 02:22] the horse: the RAM is probably mostly resources

[2025-07-27 02:22] the horse: pre-loaded server content etc

[2025-07-27 02:22] the horse: anyways; off topic

[2025-07-27 11:20] Hyun Lih: [replying to wanderer: "so i guess my question is if i unload the driver a..."]
why not use GOP?

[2025-07-27 11:20] Hyun Lih: instead of touching winIO

[2025-07-27 14:37] wanderer: [replying to Hyun Lih: "why not use GOP?"]
Well it sorta does

[2025-07-27 14:37] wanderer: it gets the gop buffer from ntoskrnl

[2025-07-27 14:37] wanderer: and draws on it

[2025-07-27 14:38] wanderer: if you mean making an efi application then I'd need to get that signed or use shim and delete the cert every boot which is a headache

[2025-07-27 14:39] wanderer: and I'm not sure how it'd work if it was purely an EFI app because it needs to execute at the same time as windows

[2025-07-27 14:39] wanderer: otherwise itd be delaying the boot

[2025-07-27 14:45] wanderer: I suppose I could paste BlackLotus but that only works until June 2026 when they start revoking that bootloader

[2025-07-27 19:05] ngildea: Anyone going to blackhat? This talk looks interesting 
 https://blackhat.com/us-25/briefings/schedule/index.html#watching-the-watchers-exploring-and-testing-defenses-of-anti-cheat-systems-46777
[Embed: Black Hat]
Black Hat

[2025-07-27 19:07] daax: [replying to ngildea: "Anyone going to blackhat? This talk looks interest..."]

[Attachments: image.png]

[2025-07-27 19:07] daax: probably not

[2025-07-27 19:11] ngildea: Nice to see a healthy mistrust of academia 🙂

[2025-07-27 19:13] daax: [replying to ngildea: "Nice to see a healthy mistrust of academia 🙂"]
> Come join our talk to learn about state-of-the-art defense & resilience techniques, as deployed in games such as Fortnite, CS2, Valorant, and more.
this was mainly the issue. state-of-the-art in CS2? <:lmao:1008172779385925662> Fortnite? Could agree. Valorant? Also would agree. CS2?

[2025-07-27 19:13] daax: Also, it's going to likely be a "survey of techniques" type shit

[2025-07-27 19:13] daax: where these academics yap about things they read because they don't actually know how to DO them or RE them (like the guy who just read UC posts and made a presentation then went to DefCon).

[2025-07-27 19:14] shalzuth: It seems like they are focusing on using anti-cheat as anti-virus?

[2025-07-27 19:14] ngildea: I suppose they're not going to say "come and listen to our misunderstanding of outdated techniques"

[2025-07-27 19:14] daax: BlackHat has become LARPhat in recent years. Handful of great talks, the rest are snooze fest metadiscussions.

[2025-07-27 19:14] ngildea: [replying to shalzuth: "It seems like they are focusing on using anti-chea..."]
The ransomware comment caught my eye

[2025-07-27 19:15] Brit: "We investigate past scenarios where anti-cheats have pioneered novel defense measures against cheating techniques, which later became relevant when deployed by serious threat actors."

[2025-07-27 19:15] Brit: once again academia lagging a decade behind the real world

[2025-07-27 19:15] Brit: and I am a hardcore academia stan

[2025-07-27 19:16] shalzuth: Maybe they are ex p2c devs

[2025-07-27 19:16] daax: [replying to Brit: "and I am a hardcore academia stan"]
same. I generally like academic papers that are doing modern research; there's some great EU universities that put out really interesting stuff... but this... just hopping on the anti-cheat overhype train to talk about how VAC loads itself via manual mapping <:Kappa:794707301436358686>

[2025-07-27 19:16] Brit: ah but you see by doing a direct syscall the anticheat has evaded hooks on ntdll, how clever and novel

[2025-07-27 19:16] ngildea: I went to a conference once and the speaker was talking about how everyone knew a technique wasn't practical to ship because of some unsolved problem, my team had been shipping the technique for a decade

[2025-07-27 19:17] daax: [replying to Brit: "ah but you see by doing a direct syscall the antic..."]
https://tenor.com/view/angry-korean-gamer-gif-20887482

[2025-07-27 19:18] daax: [replying to ngildea: "I went to a conference once and the speaker was ta..."]
it be like that.

[2025-07-27 19:20] Brit: [replying to daax: "https://tenor.com/view/angry-korean-gamer-gif-2088..."]
I need an edit of this gif but instead of a keyboard its the intel sdm

[2025-07-27 19:22] avx: [replying to daax: "https://tenor.com/view/angry-korean-gamer-gif-2088..."]
https://tenor.com/view/angry-can-air-korean-febreze-gif-25686831

[2025-07-27 19:22] avx: telemetry ingestion visualized

[2025-07-27 20:17] mtu: [replying to Brit: "once again academia lagging a decade behind the re..."]
Is byovd really a decade old

[2025-07-27 20:18] Brit: around there

[2025-07-27 20:18] Brit: because it came to be with DSE

[2025-07-27 20:18] Brit: basically as soon as vista released

[2025-07-27 20:21] mtu: [replying to daax: "> Come join our talk to learn about state-of-the-a..."]
Maybe they’ll throw shade at how VAC is still used to defend an extremely popular game for a large company while still being a decade or more out of date

[2025-07-27 20:21] mtu: But yeah I won’t be going to that talk

[2025-07-27 20:22] mtu: Where’s the talks on the ML-based server side AC and adversarial techniques against that

[2025-07-27 20:56] wanderer: Please buy my new endpoint protection. We load mhyprot.sys and vgk.sys on every endpoint

[2025-07-27 20:56] wanderer: They counteract each other you see

[2025-07-27 20:56] kralos: [replying to daax: "same. I generally like academic papers that are do..."]
why would vac load itself via manual mapping?

[2025-07-27 21:03] daax: [replying to kralos: "why would vac load itself via manual mapping?"]
prevents you from more easily just dumping them out (not really in their case). they're manually mapped from steamservice - in the past you could force them to use loadlibrary on their modules with a 1-byte patch.

[2025-07-27 21:11] kralos: makes sense, is it common in other ACs ?

[2025-07-27 21:12] rain: [replying to daax: "Also, it's going to likely be a "survey of techniq..."]
It's very likely just based on the paper they did last year
https://pure-oai.bham.ac.uk/ws/portalfiles/portal/238282817/checkmate24_collins_anti_cheat.pdf

[2025-07-27 22:22] the horse: [replying to kralos: "makes sense, is it common in other ACs ?"]
yes

[2025-07-27 22:27] kralos: [replying to the horse: "yes"]
Kernel too? so they manual map from kernel or how do they do it,  sorry if these are basic questions, I really have no idea how they work

[2025-07-27 22:35] the horse: drivers are not mapped

[2025-07-27 22:35] the horse: EAC does map their internal dll from kernel

[2025-07-27 22:36] Lucy: [replying to kralos: "Kernel too? so they manual map from kernel or how ..."]
Any memory APIs will have some kind of kernel mode equivalent, meaning you can map via a driver as well.

[2025-07-27 22:40] kralos: [replying to the horse: "EAC does map their internal dll from kernel"]
so im guessing they download the dll and map it on the fly so it doesn't touch disk