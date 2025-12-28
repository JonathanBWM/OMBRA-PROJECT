# August 2025 - Week 2
# Messages: 201

[2025-08-05 10:35] vtl0: silly question, not really interested in cheat development but rather detection: is there a way DMA attacks can achieve anything relevant without helper software/driver MmMapIoSpace'ing memory, for example? all the "completely undetectable" claims sounds rather misleading

[2025-08-05 10:36] vtl0: I guess one could try and read the whole RAM and identify where key structures are, but that isn't trivial either (and things move around in memory)

[2025-08-05 10:37] vtl0: surely I am missing something

[2025-08-05 11:08] truckdad: [replying to vtl0: "I guess one could try and read the whole RAM and i..."]
i’m not sure anyone would claim it’s trivial, but it’s certainly not impossible

[2025-08-05 12:54] daax: [replying to vtl0: "silly question, not really interested in cheat dev..."]
Yes

[2025-08-05 16:27] the horse: [replying to vtl0: "silly question, not really interested in cheat dev..."]
It's actually not that hard to find all the stuff you need (loaded drivers, etc) as you can bruteforce ntoskrnl base pretty efficiently

[2025-08-05 16:27] the horse: R/W is all you need

[2025-08-05 16:55] Tr1x: [replying to vtl0: "silly question, not really interested in cheat dev..."]
Why is the approach detection and not prevention, detection is a losing battle. They're still confined to hardware memory management rules that being the IOMMU, giving you control over what they see.  Unless of course they go some route like jtag or some shit, but not much you can do then and it isn't the most practical.

[2025-08-05 17:27] daax: [replying to Tr1x: "Why is the approach detection and not prevention, ..."]
jtag isn’t the most practical? what. it wouldn’t be practical at all for you to attach to a target via jtag with the intent on using it for some game hack. this is also barring all of the obstacles to that point on modern boards*

as for detection vs prevention? I would encourage you to try to *prevent* DMA attacks on any modern consumer system without significant infrastructure in place to control device inputs. They’re trying very hard to prevent DMA attacks, but it’s obviously not a perfect solution atm. Aside that, if I buy some arbitrary device adapter placed into a pcie slot, and you block an otherwise innocuous device — I’d be annoyed and I can imagine many others would be as well. Detection is difficult, there are methods to determine with reasonable or complete confidence that a device is legitimate or masked, but it depends. Those methods require quite a bit of time investment and staff that know what to do. Afaik, AC teams choose to pursue both to some degree rather than put all their eggs in one basket, but they’re limited in what they can do for both. It’s not an easy all-or-nothing, black and white problem.

[2025-08-05 17:54] UJ: [replying to daax: "jtag isn’t the most practical? what. it wouldn’t b..."]
Do you know of any anti cheats that detect DCI/JTAG (cpu debugging) on and prevent the game from running like it does when windbg debug is on?

[2025-08-05 18:02] Timmy: I can test this if you like

[2025-08-05 18:02] Timmy: LOL

[2025-08-05 18:02] Timmy: my laptop is totally fked out of the factory

[2025-08-05 18:02] Timmy: and has it on

[2025-08-05 18:02] Timmy: > cpu debugging

[2025-08-05 18:03] UJ: what gen cpu?

[2025-08-05 18:03] Timmy: 8250u iirc

[2025-08-05 18:03] UJ: the only ac i see that would even attempt to detect this would be vgk.

[2025-08-05 18:08] Tr1x: [replying to daax: "jtag isn’t the most practical? what. it wouldn’t b..."]
If the context is I'm restricted by what anti cheats capabilities currently are then yes of course in the windows ecosystem it becomes significantly harder to implement any prevention and of course in current consumer hardware, it isn't easy to prevent DMA related attacks. No one said it is. Detection, of course there is no reason not to implement it as well but it is ultimately a losing battle, that you will not win.

Why do you need to block an entire device, now the direction especially of Microsoft is to ensure trust factor of an entire machine and I can't see why that can't bleed into your standard dma devices used by an everyday consumer (they're heading towards an Xbox like system) where the TPM can verify the trust of a device, where only trusted devices attached to your computer are allowed during runtime of the game. Now this is speculation, but just thought dumping.

Moving on to the actual answer to "Why do you need to block an entire device", a dma device should never have to access the portion of the memory that is let's say responsible for storing game and player related data. Through the IOMMU you can restrict this range from being remapped by any DMA device, hyperv already does this for itself. Now of course this requires hypervisor level code running but hyperv is exposing more and more control slowly to the OS in order to setup the configuration of these systems.

As for people being annoyed, as long as the bottom line of revenue isn't significantly affected and gamers get a better experience. They couldn't care less, false positives happen even in the modern systems but they keep the numbers low enough not to stir the social media backlash.

[2025-08-05 18:58] daax: [replying to Tr1x: "If the context is I'm restricted by what anti chea..."]
You asked why is the approach detection and not prevention, I explained it’s not one over the other — it’s a mix of both and there are technical limitations on the platforms currently, but ACs, game publishers, etc are all advocating for preventative controls being implemented in hardware. So until all the features are widely supported in silicon OS vendors and AC teams are going to be doing slow roll outs of features. The whole end goal is, yeah, turning the PC into an Xbox, but currently they’re using a mix of the two where they can without impacting tons of players. 

As for the implications of all the preventative features they want to add, we’ve always had feelings on those: https://secret.club/2021/06/28/windows11-tpms.html
[Embed: Windows 11: TPMs and Digital Sovereignty]
The problem with enforcing TPMs on consumers

[2025-08-05 19:01] daax: Preventing is also a losing battle. Increasing potential attack surface or nuances of implementations just invites abuse. People thought secure boot was solution way back, look how that went. Neither is going to be perfect solution.

[2025-08-05 21:52] Tr1x: [replying to daax: "Preventing is also a losing battle. Increasing pot..."]
Well, if we're speaking pragmatically, my whole opinion changes, and I agree with this. A gamer's machine is inherently untrusted and insecure they have full control, and the environment isn't verifiable (which is not a bad thing). I stand with digital sovereignty.

All cheats come down to a fundamental limitation networking. The server is trustworthy, and if the data isn't there or has a significant amount of validation, you can inherently limit possibilities.

Server-side occlusion, aim assist, behavioral, and statistical analysis these are just some examples that are already implemented in production environments. The only issue is that games without in-house security teams or the capital to contract one face implementation challenges.

A server-first approach, in my opinion, is the way to go. If you don't have player data streamed to you, good luck getting that information to be displayed. If you're spinning around and speeding all over the place, forget it. Even with a subtle aimbot, without proper humanization, synthetic aim is not difficult to detect. Of course, there has to be some leeway, but cheaters will mess up over time, and the heuristics pile up.

As for my opinion on DMA approaches, that was just my angle of approach if I had to worry about or attack it. But in current system environments, especially for anti-cheats, I agree it is not feasible. So, you have to rely on a mix but there is a bigger lean toward detection more so.

As for Secure Boot, complete enforcement of this protection in games has only existed recently with the release of Windows 11. This has been quite effective against some approaches, though, of course, it’s subvertible since the state isn't fully verifiable, so it’s not full proof. But nothing is, and that’s not the goal.

[2025-08-05 22:12] Tr1x: If there were no restrictions it would be awesome exploring https://www.usenix.org/sites/default/files/conference/protected-files/atc22_slides_gu-jinyu.pdf for per thread based security and secure enclaving for memory data isolation and secure execution environments. But this is wishful thinking 😭

[2025-08-05 23:49] apekros: [replying to daax: "jtag isn’t the most practical? what. it wouldn’t b..."]
lately on this real devices that you will buy will be prevented from being used in some cases! i.e if you bought an xHCI controller but didn't plug anything into it, if you hot plugged a USB device mid game it wouldn't work if nothing was previously plugged into it as it cannot issue DMA transfers (bme off)

[2025-08-06 00:30] daax: [replying to apekros: "lately on this real devices that you will buy will..."]
yeah, but I don’t think that’s an issue for the majority of games yet <:Kappa:794707301436358686> it’s only gonna happen on a few games under 2 diff ACs (afaik lol- the more abundant middlewares aren’t doing that… yet). If you’re playing anime cs though you have bigger problems than your devices getting bme flipped.

[2025-08-06 00:30] daax: still funny to imagine someone trying to alt tab and submit an assignment but their usb won’t work

[2025-08-06 00:36] apekros: [replying to daax: "still funny to imagine someone trying to alt tab a..."]
bahaha yes, it shouldn’t really be a problem for the onboard ones though, as they can determine which ones are those and which are external

[2025-08-06 00:37] apekros: [replying to daax: "yeah, but I don’t think that’s an issue for the ma..."]
EAC started recently too, CN ACE as well for some time

[2025-08-06 00:46] daax: [replying to apekros: "EAC started recently too, CN ACE as well for some ..."]
When did you see they started doing this? And what games? I’ve not had this issue with other EAC games. I’m always hot plugging items and sometimes don’t close the game in b/w games. I’m assuming Rust and Fortnite.

[2025-08-06 00:51] daax: [replying to apekros: "EAC started recently too, CN ACE as well for some ..."]
Yeah the CN ACE is one I was referring to, same with VGK, there’s some CS league ACs that did this kind of goofiness too… not for USB though, at least when I looked. I wonder if they are now. It’s kinda silly imo lol

[2025-08-06 00:52] apekros: [replying to daax: "When did you see they started doing this? And what..."]
Fortnite and only very recently (last few days), hot plugging into internal xHCI controller, not one you've bought to get more USB slots though right?

[2025-08-06 00:53] apekros: [replying to daax: "Yeah the CN ACE is one I was referring to, same wi..."]
Yes the blood money CS league AC is also very aggressive

[2025-08-06 00:53] apekros: I sent a DM too, not sure if you have them off

[2025-08-06 00:54] daax: [replying to apekros: "Fortnite and only very recently (last few days), h..."]
it’s a hub from amazon, my board slots are all taken <:whyy:820544448798392330> , so yeah external; but I don’t play FN and only use Rust to diff cf against the other games running EAC, which I haven’t done in a few months — so ig missed the drop on that one for Rust.

[2025-08-06 01:58] Matti: [replying to Timmy: "I can test this if you like"]
I mean... I don't wanna be a dick but while having CPU debugging enabled in the CSME config on a laptop is indeed highly unusual (and probably a mistake), and also the most critical part in getting DCI to work on a system....
you're still like 1 million other required steps away from actually being able to debug your laptop

[2025-08-06 01:59] Matti: it's a lot easier to just buy one of those AEON dev boards that are intentionally made for this if you want to try DCI

[2025-08-06 02:06] Matti: ^ above post is not meant to be taken as an endorsement or recommendation of any kind by the way, I just want to make that perfectly clear

[2025-08-06 02:07] Matti: I would always strongly recommend against buying any product if this ends up giving money to the intel CPU division via some way

[2025-08-06 02:08] Matti: so it's only a hypothetical or theoretical suggestion, not actual advice

[2025-08-06 02:09] Matti: if we never learn from mistakes made in history then intel will never die

[2025-08-06 02:45] daax: [replying to Matti: "if we never learn from mistakes made in history th..."]
but… if AMD is mucked, but in a different way what’s a person to do

[2025-08-06 02:46] daax: I can’t just castaway my intel manuals

[2025-08-06 02:46] Matti: no I don't mean this in reference to JTAG, I mean this is general advice

[2025-08-06 02:47] daax: [replying to Matti: "no I don't mean this in reference to JTAG, I mean ..."]
fair enough

[2025-08-06 02:47] daax: hey matti. intel > amd

[2025-08-06 02:47] Matti: but yeah JTAG on AMD is just as fucked if not more so

[2025-08-06 02:48] Matti: [replying to daax: "hey matti. intel > amd"]
<:lillullmoa:475778601141403648>

[2025-08-06 02:48] Matti: idgaf about AMD

[2025-08-06 02:48] Matti: let it be ARM if that's what it takes

[2025-08-06 02:48] daax: damn it. ragebait failed

[2025-08-06 02:48] Matti: I just want intel to die

[2025-08-06 02:49] daax: fair. ARM is probably the best bet

[2025-08-06 02:49] daax: let x86 tank

[2025-08-06 02:49] UJ: [replying to Matti: "I just want intel to die"]
your wish might be granted. looka t the stock 🙁

[2025-08-06 02:49] Matti: oh I am

[2025-08-06 02:49] Matti: every day

[2025-08-06 02:49] daax: [replying to UJ: "your wish might be granted. looka t the stock 🙁"]
the stock aint gonna mean the company is gonna shit the bed though

[2025-08-06 02:49] Matti: yeah, that

[2025-08-06 02:49] daax: it needs to be burned down from the inside out

[2025-08-06 05:53] shalzuth: [replying to Matti: "I just want intel to die"]
There’s probably some intel employees working the skeleton crew here… be considerate to them!

[2025-08-06 06:00] Matti: I want them to die

[2025-08-06 06:01] Matti: well no

[2025-08-06 06:01] Matti: they can just quit their jobs that's fine too

[2025-08-06 06:01] Matti: but this is a war

[2025-08-06 06:01] Matti: you can't be considerate to your enemy

[2025-08-06 06:01] Timmy: [replying to Matti: "I mean... I don't wanna be a dick but while having..."]
the more you know <:kekw:904522300257345566>

[2025-08-06 12:27] apekros: I think rumour, cn community makes up lies a lot sadly

[2025-08-06 22:04] Deleted User: i'm looking into reversing easy-anti-cheat

[2025-08-06 22:04] Deleted User: and i know their imports are retrieved at runtime

[2025-08-06 22:04] Deleted User: are they just stored in memory as tons of pointer

[2025-08-06 22:04] Deleted User: which i can just trace over the eac module to see calls to these pointer?

[2025-08-06 22:48] Tr1x: Yeah the Chinese are great at marketing

[2025-08-06 22:49] Tr1x: They can sell sand to the desert

[2025-08-06 23:12] Deleted User: ```c++
 ((void (__fastcall *)(void *))(__ROR8__(~v2, 0x2F) ^ 0xFAAB144E338B6738uLL))(&unk_FFFFF80989138D38);
```

[2025-08-06 23:12] Deleted User: the packer that eac uses

[2025-08-06 23:13] Deleted User: im pretty sure this random combo of not,xor,ror uses this value "0xFAAB144E338B6738uLL" which is relative to the binary?

[2025-08-06 23:13] Deleted User: meaning i calculate the output

[2025-08-06 23:14] Deleted User: i would get the address of the function?

[2025-08-07 06:13] moshui: ACE use IOMMU to detect DMA

[2025-08-07 06:13] moshui: nobody reverse EAAC?

[2025-08-07 07:42] apekros: [replying to moshui: "ACE use IOMMU to detect DMA"]
Show proof

[2025-08-07 08:50] moshui: force on VTD<:kekw:904522300257345566>

[2025-08-07 09:08] apekros: [replying to moshui: "force on VTD<:kekw:904522300257345566>"]
Anything substantial? This is just same Chinese rumour I’ve been hearing but nothing actually said. Just “VTd” “IOMMU”

[2025-08-07 09:11] moshui: If you don't believe it, there's nothing you can do. Just reverse it and you'll know.

[2025-08-07 09:13] moshui: This is not a difficult task, IOMMU is originally used to manage these DMAs

[2025-08-07 09:15] iPower: share binaries

[2025-08-07 09:17] moshui: Just download wegame and download DF

[2025-08-07 09:17] moshui: I have never played it before.<:kekw:904522300257345566>

[2025-08-07 09:19] apekros: [replying to moshui: "I have never played it before.<:kekw:9045223002573..."]
So you are just spreading rumour too?

[2025-08-07 09:19] apekros: Nice!

[2025-08-07 09:19] moshui: ...

[2025-08-07 09:20] apekros: [replying to moshui: "..."]
You said you’ve never played it, so like i said you are just saying things

[2025-08-07 09:20] moshui: 
[Attachments: image.png]

[2025-08-07 09:21] moshui: [replying to apekros: "You said you’ve never played it, so like i said yo..."]
There is a lot of evidence that it is being tested

[2025-08-07 09:22] apekros: [replying to moshui: ""]
I know Chinese companies and the CCP are entirely truthful and would never lie to me

[2025-08-07 09:22] apekros: i now believe you, thanks

[2025-08-07 09:29] varaa: the chinese just excel in everything

[2025-08-07 09:38] Pepsi: I don't know that much about PCIe / IOMMU.

But couldn't anti-cheats just let this handle the OS by enforcing "Kernel DMA Protection"? To my understanding this feature is just Hyperv-V using IOMMU to effectively prevent PCIe devices from accessing memory outside of their designated memory regions.

[2025-08-07 09:46] apekros: [replying to Pepsi: "I don't know that much about PCIe / IOMMU.

But co..."]
doesn’t work in current state

[2025-08-07 09:46] Pepsi: can you technically explain to me why? just curious

[2025-08-07 09:47] apekros: [replying to Pepsi: "can you technically explain to me why? just curiou..."]
you can still access outside of those regions

[2025-08-07 09:48] Pepsi: how? isn't IOMMU specifically designed to stop PCIe devices from doing that? like where is the loophole?

[2025-08-07 09:48] Nats: yes

[2025-08-07 09:49] apekros: [replying to Pepsi: "how? isn't IOMMU specifically designed to stop PCI..."]
yes it is

[2025-08-07 09:49] Nats: its supposed to sandbox PCIe devices so they cant just read whatever memory they want

[2025-08-07 09:58] Pepsi: https://astralvx.com/tag/bus-master-enable/

so from what I understand is, that PCIe devices in general are not supposed to just randomly read any memory, devices already get assigned memory ranges in which they are supposed to operate

These DMA attacks exploit the fact that there is no infrastructure to actually prevent memory access outside of these assigned ranges.

However with IOMMU these assignments to certain memory ranges can actually be enforced, and it's already implemented by Hyper-V Kernel DMA Protection.

[2025-08-07 09:59] Nats: exactly you got it

[2025-08-07 09:59] Nats: the whole system is basically built on trust

[2025-08-07 09:59] Nats: hey device, heres your memory range, please only use that

[2025-08-07 10:00] Pepsi: [replying to apekros: "you can still access outside of those regions"]
So can can you explain to me how one is able to do (pcie-based) DMA attacks when kernel DMA Protection is enabled? Are there any limitations to that protection I am not aware of yet?

[2025-08-07 10:00] Nats: and without IOMMU, theres literally nothing stopping a device from just ignoring that and reading whatever it wants

[2025-08-07 10:00] Nats: the article shows it perfectly with those TLP examples. when they craft that read request for address 0x001AD000 (the PML4), they're just... asking for it

[2025-08-07 10:01] Nats: and the root complex is like "sure, heres your data"

[2025-08-07 10:01] Nats: because without IOMMU theres no bouncer checking if that device is even supposed to access that address

[2025-08-07 10:01] Nats: whats wild is that Bus Master Enable bit they mention

[2025-08-07 10:02] Nats: even if the firmware doesnt set it, a malicious device can still send TLPs up the bus

[2025-08-07 10:02] Nats: its literally just a suggestion lol

[2025-08-07 10:03] apekros: [replying to Nats: "even if the firmware doesnt set it, a malicious de..."]
If the root port has BME off this isn’t possible

[2025-08-07 10:03] apekros: [replying to Pepsi: "So can can you explain to me how one is able to do..."]
yes those regions aren’t enforced for internal devices, I believe the protection mainly targeted at Thunderbolt, they have mentioned expanding it

[2025-08-07 10:04] Nats: MS kernel DMA protection is basically forcing this on newer systems

[2025-08-07 10:04] Nats: but tons of machines still vulnerable

[2025-08-07 10:04] Nats: especially during that pre-boot window before the hypervisors even loads

[2025-08-07 10:05] apekros: [replying to Nats: "MS kernel DMA protection is basically forcing this..."]
I have it all on, IOMMU everything. Modern system. I can read from addresses that aren’t related to my device driver from the device

[2025-08-07 10:05] Nats: interesting...

[2025-08-07 10:06] Nats: if youve got IOMMU fully enabled and still reading arbitrary memory, theres gotta be something wrong with the config or you're hitting an allowed range somehow

[2025-08-07 10:06] Nats: what addresses are you successfully reading?

[2025-08-07 10:06] Nats: system structures like PML4 or just random memory?

[2025-08-07 10:07] Nats: and whats your device presenting as (network card, storage controller?)

[2025-08-07 10:07] apekros: [replying to Nats: "system structures like PML4 or just random memory?"]
Yes structures, game memory etc.

[2025-08-07 10:07] apekros: It’s not uncommon it’s not my config or setup

[2025-08-07 10:07] apekros: there’s a reason anti cheats can’t just turn this feature on and dma is gone

[2025-08-07 10:08] apekros: [replying to Nats: "and whats your device presenting as (network card,..."]
all work, nic, NVMe controller, SATA, xhci etc

[2025-08-07 10:08] Pepsi: [replying to apekros: "yes those regions aren’t enforced for internal dev..."]
Seems like an odd design choice to me, maybe someone with any clue about io virtualization can enlighten me if that is true and why that is the case?

[2025-08-07 10:09] apekros: [replying to Pepsi: "Seems like an odd design choice to me, maybe someo..."]
I am not certain as to why either, I’m not an expert and thought this was all properly implemented but you can see based on the tone of this article too:

https://learn.microsoft.com/en-us/windows/security/hardware-security/kernel-dma-protection-for-thunderbolt
[Embed: Kernel DMA Protection]
Learn how Kernel DMA Protection protects Windows devices against drive-by Direct Memory Access (DMA) attacks using PCI hot plug devices.

[2025-08-07 10:09] Nats: yeah if it was that simple, every game would just flip the IOMMU switch and DMA cheats would be dead. but they dont, and there is good reasons. the reality is IOMMU has massive holes in practice. proper IOMMU remapping adds latency to every memory address. for gaming thats a nightmare. anticheats enabling strict IOMMU would cause FPS drops and people would rage

[2025-08-07 10:10] apekros: [replying to Nats: "yeah if it was that simple, every game would just ..."]
they do enforce it if you trigger flags on the big acs (vgk/faceit)

[2025-08-07 10:10] Nats: [replying to apekros: "I am not certain as to why either, I’m not an expe..."]
windows implementation is meh... even with kernel dma protection on, windows still has compatibility paths and exceptions

[2025-08-07 10:10] Nats: its not a hard block, its more like guidelines

[2025-08-07 10:11] Nats: enough pressure from hardware vendors means microsoft cant go full lockdown

[2025-08-07 10:12] Nats: the DMA cheat devs basically found that if you present your device the right way and exploit these compatibility/performance tradeoffs

[2025-08-07 10:12] Nats: IOMMU becomes more of a speedbump than a wall

[2025-08-07 10:12] Nats: thats why DMA cheats still work even on "protected" systems

[2025-08-07 10:12] Nats: they're exploiting the fact that nobody can afford to actually enforce strict isolation without breaking everything else

[2025-08-07 10:12] apekros: is this reply LLM generated?

[2025-08-07 10:13] Nats: lol yeah, reading it back it does sound kinda AI-ish doesnt it?

[2025-08-07 10:13] Nats: but nah man, i just organize my thoughts like that sometimes when explaining technical stuff

[2025-08-07 10:14] Nats: probably comes from writing too much docs and explanations over the years

[2025-08-07 10:14] Nats: gets worse when im trying to cover all the angles of why somethings broken

[2025-08-07 10:14] apekros: man that reply is 100% LLM generated

[2025-08-07 10:15] Nats: yup you caught me

[2025-08-07 10:16] Nats: i fell into that boring explanation mode

[2025-08-07 10:16] Nats: all neat and tidy and shit

[2025-08-07 10:16] Nats: sounds like chatgpt explaining why the sky is blue or something XD

[2025-08-07 10:17] Nats: <@223799148556582913> you're reading game memory through IOMMU cause the whole thing is a joke

[2025-08-07 10:17] Nats: probably just spoofing device IDs or using one of those kmbox that looks legit enough to get wide access

[2025-08-07 10:18] Nats: what game you cheating?

[2025-08-07 10:20] apekros: I don’t want to chat with OpenAI tonight reply to me tomorrow when GPT5 drops

[2025-08-07 10:21] Nats: if you think im openai then this convo is cooked

[2025-08-07 10:21] Nats: cant blame you

[2025-08-07 10:22] Nats: IOMMU stuff is still true though

[2025-08-07 10:22] Nats: it really is that broken

[2025-08-07 10:22] Nats: but whatever, if you're done you're done lol

[2025-08-07 10:24] Lyssa: [replying to apekros: "is this reply LLM generated?"]
I do not understand what about that reply makes you think it's AI generated

[2025-08-07 10:26] Lyssa: it's rude to immediately dismiss someone like that for no reason

[2025-08-07 10:27] apekros: [replying to Lyssa: "I do not understand what about that reply makes yo..."]
read the contradiction between the responses about IOMMU not working then the explanation below. That explanation below is exactly how LLMs explain it and the exact same writing style.

[2025-08-07 10:27] apekros: and is completely different than their reply above.

[2025-08-07 10:28] Lyssa: uh I read everything and it seemed fine

[2025-08-07 10:28] Lyssa: but okay

[2025-08-07 10:29] apekros: [replying to Lyssa: "uh I read everything and it seemed fine"]

[Attachments: IMG_3454.png, IMG_3453.png]

[2025-08-07 10:29] apekros: I’m not meaning to be rude but the reply on the left is 100% LLM generated. I thought everything before that point was fine too

[2025-08-07 10:30] moshui: [replying to apekros: "I know Chinese companies and the CCP are entirely ..."]
Obviously, the truth can only be concealed through political attacks<:kekw:904522300257345566>

[2025-08-07 10:34] Pepsi: [replying to apekros: "all work, nic, NVMe controller, SATA, xhci etc"]
(?)
[Attachments: Screenshot_20250807-123153.png]

[2025-08-07 10:35] apekros: [replying to Pepsi: "(?)"]
I am confused by this too, but it doesn’t work / isn’t enforced. You can still read outside of the regions you’re supposed to.

[2025-08-07 10:36] Pepsi: 🤔

[2025-08-07 10:37] Lyssa: [replying to apekros: "I’m not meaning to be rude but the reply on the le..."]
I am not seeing what you're seeing lol, makes perfect sense to me

[2025-08-07 10:38] Lyssa: it's good to be discrete about AI stuff online but I think ur reading way too deep into it

[2025-08-07 13:22] daax: [replying to Pepsi: "So can can you explain to me how one is able to do..."]
It depends. internal LPC, for example, can perform DMA requests but requires that some signals (LDRQ# among other optional signals) are supported for that platform; and right, not all PCIe devices can do arbitrary read/writes. It also depends on the IOMMU is isolating that device or is configured to 'whitelist only' essentially. If IOMMU is on but devices attached to the LPC bus are not isolated and support LDRQ# signals, then there is a possibility that DMA would work.

[2025-08-07 13:30] daax: [replying to Pepsi: "Seems like an odd design choice to me, maybe someo..."]
Because internal devices don't pose the same kind of risk as a device connected externally. thunderbolt also allows pcie devices to be connected externally... like if you connect an adapter for pcie over tb (tons of adapters for this) then you could perform the same DMA operations as if you were placed directly into the pcie slot on the main board. all the DMA protection does (or is supposed to when properly implemented) is prevent external devices from rd/wr arbitrary memory not within their allowed range; also if you enforce DMA protection for some internal devices you could wind up fucking your system

[2025-08-07 13:41] Pepsi: [replying to daax: "It depends. internal LPC, for example, can perform..."]
how is LPC connected to the system on modern platforms?

[2025-08-07 13:43] daax: [replying to Pepsi: "how is LPC connected to the system on modern platf..."]
some TPMs are connected via LPC, depends on manufacturer. in any case, just an example of abusing something for DMA that might not be covered by protections.

[2025-08-07 13:44] Pepsi: nah I wasn't asking what's connected to LPC, I was asking how LPC is connected, is it just a PCIe device or part of the Chipset?

[2025-08-07 13:46] daax: [replying to Pepsi: "nah I wasn't asking what's connected to LPC, I was..."]
LPC is an internal bus, part of chipset

[2025-08-07 13:47] Pepsi: [replying to daax: "some TPMs are connected via LPC, depends on manufa..."]
yes, ms states that some other interface types are not covered by the protection, that's why I was specifically asking for pcie based dma

[2025-08-07 13:48] Pepsi: so what i got from this conversation, is that basically protection via IOMMU is only applied if the driver supports dma remapping, which doesn't seem to be the case for a lot of devices

[2025-08-07 13:48] daax: [replying to Pepsi: "yes, ms states that some other interface types are..."]
gotcha. well either way attacks via LPC are still pretty neat. wouldn't be hard to isolate them anyways, plus the platform has to support the appropriate signals for it to be possible

[2025-08-07 14:20] daax: [replying to Pepsi: "so what i got from this conversation, is that basi..."]
also figure i should be clear to avoid people splitting hairs, you could apply it to anything that can be identified through various means like BDF or ANDD (SID, etc -- the core of DMAR is based on source-id, which might be determined impl-specific depending on platform usually SID=RID tho). it can all be done independently of "drivers" -- just the "depends" answer. if you want control, from the OS, to pick and choose then yeah requires the appropriate driver/firmware support.

[2025-08-07 14:56] Pepsi: [replying to daax: "also figure i should be clear to avoid people spli..."]
well before I read about it, I just assumed microsoft applies IOMMU transparently in the background with some kind of identity mapping, without drivers needing to be explicitly compatible

[2025-08-07 15:00] Pepsi: I was maybe a little naive thinking that  😅

[2025-08-07 16:02] daax: [replying to Pepsi: "well before I read about it, I just assumed micros..."]
right, well you're not explicitly wrong in what you were thinking though. the os can create domains and assign devices to them and use the iommu to enforce dma domain protection that way too. how you explained it is one way of doing it; driver registers its io region(s) with windows and then windows assigns the region(s) to specific domains. if no compatibility then you can assign through a few different means depending on device availability.

[2025-08-07 16:06] daax: kind of funny history too is that on an old version of w10 all the devices shared a single io page map so the memory exposed to one device was exposed to the rest lol

[2025-08-07 16:08] daax: now that's an early implementation... it's much more complex nowadays; but just evidence to the fact that no matter how much ppl try to lock down the systems there is always gonna be something that can be abused

[2025-08-08 15:38] Tequila: https://media.discordapp.net/attachments/1366105266700947600/1403381631833870346/IMG_1571.png?ex=6897587e&is=689606fe&hm=b6d3273a210cbc5386d833a89c93e3bc6d552b7172e202288a7114ad5cde416d&=&format=webp&quality=lossless&width=387&height=839

[2025-08-08 16:42] the horse: solves nothing ✅

[2025-08-08 21:53] eval00: [replying to Tequila: "https://media.discordapp.net/attachments/136610526..."]
"secureboot requirements for gaming"

[2025-08-08 21:54] eval00: "secureboot is the shittest thing to ever exist" - says several random OS developers

[2025-08-08 21:54] eval00: i honestly don't disagree secureboot is utter garbage

[2025-08-08 21:55] eval00: uefi standard does a good job explaining it, and vendors do an absolutely SHIT job at implementing it

[2025-08-08 21:56] elias: measured boot (if utilized properly) is a much stronger security mechanism than secure boot imo

[2025-08-08 21:57] elias: the secure boot revocation mechanism is so bad that linux and windows had to implement their own separate revocation mechanisms

[2025-08-10 13:59] nexohk1337: [replying to Tequila: "https://media.discordapp.net/attachments/136610526..."]
will def stop me from injecting ricochet.dll into the game

[2025-08-10 15:48] the horse: [replying to nexohk1337: "will def stop me from injecting ricochet.dll into ..."]
😍