# July 2025 - Week 2
# Channel: #programming
# Messages: 45

[2025-07-07 08:04] Matti: [replying to bugdigger: "i9-14900HX"]
can you post your exact motherboard model as well? for the Intel(R) 7th-9th gen CPUs there's a bunch of overlapping CSME versions (11-13) that could all theoretically apply depending on your motherboard

[2025-07-07 08:12] Matti: I've got CSME system tools for all of these, but they're just fundamentally incompatible if your motherboard has a different CSME version

[2025-07-07 13:03] bugdigger: [replying to Matti: "can you post your exact motherboard model as well?..."]
Motherboard model: SO6100
System Model: Blade 16 - RZ09-0510
Motherboard Version: 4

[2025-07-07 13:11] Matti: oh god

[2025-07-07 13:12] Matti: would this happen to be a laptop

[2025-07-07 13:12] Matti: [replying to bugdigger: "i9-14900HX"]
I totally misread this by the way, this is a 14th gen, for some reason I read a 9th gen model number here

[2025-07-07 13:15] Matti: [replying to bugdigger: "Btw how can i dump the bios? 
I am also trying to ..."]
the good news is, if this is in fact a laptop, using a HW flasher is going to be torturous

[2025-07-07 13:15] Matti: the bad news is, not having a HW flasher on hand to save your ass is going to eventually bite you

[2025-07-07 13:16] bugdigger: [replying to Matti: "would this happen to be a laptop"]
Fuckin manufacturers are locking down the firmware

[2025-07-07 13:16] Matti: 
[Attachments: CSME_System_Tools_v16.1.25.1885v2_B0_Consumer-BKC_ADP-S.7z.001]

[2025-07-07 13:16] Matti: 
[Attachments: CSME_System_Tools_v16.1.25.1885v2_B0_Consumer-BKC_ADP-S.7z.002]

[2025-07-07 13:17] Matti: this is v16.1, most likely to work with a 14th gen CPU

[2025-07-07 13:17] bugdigger: [replying to Matti: "the good news is, if this is in fact a laptop, usi..."]
I have CH341a flash utility if thats what you meant

[2025-07-07 13:17] Matti: try `fpt -I` and/or `fpt -D dump.bin`

[2025-07-07 13:18] Matti: these are both harmless even if the chipset is incompatible, they'll just throw an error

[2025-07-07 13:18] Matti: [replying to bugdigger: "I have CH341a flash utility if thats what you mean..."]
yeah, that's a good flasher, assuming you know where the SPI chip is on the board and you can actually reach it with a test clip

[2025-07-07 13:18] bugdigger: [replying to Matti: "these are both harmless even if the chipset is inc..."]
hope so

[2025-07-07 13:19] Matti: and never ever use a HW flasher with any software other than flashrom

[2025-07-07 13:20] bugdigger: [replying to Matti: "yeah, that's a good flasher, assuming you know whe..."]
you know what the bad thing is?
Fuckin Razer doesnt have bios download available anyware from what I have seen.
They just have "Bios Updater", which i guess is just a loader which then downloads the bios for you and then i guess updates the bios for you

[2025-07-07 13:21] bugdigger: Mostly I am scared of bricking the machine.
Thats why I am developing a bootkit right now.
Currently trying to test it out on qemu, we'll see how it goes...

[2025-07-07 13:22] Matti: [replying to bugdigger: "you know what the bad thing is?
Fuckin Razer doesn..."]
fpt -d will get this for you as a complete FW dump, assuming you have the correct permissions (at least R) on every region

[2025-07-07 13:23] Matti: this is not the same as an official razer binary, but usually preferable

[2025-07-07 13:24] Matti: I'd need the full FW dump to be able to say what your options are re: updating/flashing

[2025-07-07 13:25] Matti: this mostly depends on the permissions in the descriptor region, e.g. it's possible you're not allowed to write to the ME region

[2025-07-07 13:26] Matti: in that case you'll need a HW flasher at least once to overwrite the DESC region with one granting you full access to all regions

[2025-07-07 13:27] Matti: on a desktop board there's also the "screwdriver trick"\*, but I'm a lot less certain about laptops

\* short 2 pins on the onboard audio chip during boot to unlock the DESC region, and yes this really works

[2025-07-07 13:29] bugdigger: [replying to Matti: "these are both harmless even if the chipset is inc..."]
harmless right? gotta double check before i run it haha 😭

[2025-07-07 13:31] Matti: well you do have a laptop, which is like the worst possible device to run any type of flashing software on (mostly due to onboard ECs)

[2025-07-07 13:32] Matti: so I'll amend to: fpt -I is safe, fpt -d *may* be dangerous on a laptop (though usually you simply won't have read access)

[2025-07-07 13:33] Matti: you can substitute `fpt -desc -d desc.bin` instead to only dump that region

[2025-07-07 13:34] bugdigger: okay thanks, still i will leave the option of HW flashing as last resort.
As i mentioned i am trying to test out my bootkit for corrupting the DMAR table

[2025-07-07 13:36] Matti: what would be the purpose of that? why not simply boot with `CONFIG_IOMMU=n` (not the same as `iommu=off` on the command line) or (for windows) disable VT-d in the BIOS?

[2025-07-07 13:37] bugdigger: [replying to Matti: "what would be the purpose of that? why not simply ..."]
i dont have option in BIOS to disable Vt-d, seems to be hardcoded

[2025-07-07 13:37] Matti: [replying to UJ: "if you just want to explore for now, you can try d..."]
oh well in that case see this post ^

[2025-07-07 13:37] Matti: it's almost certainly configurable, just hidden

[2025-07-07 13:38] Matti: but, you need a FW image dump first

[2025-07-07 13:40] bugdigger: okay thanks, i'll ping you if i need any more help

[2025-07-07 18:52] Windy Bug: Out of curiosity - has this become common knowledge around driver devs by now? https://www.osr.com/why-your-user-mode-pointer-captures-are-probably-broken-or-will-be-one-day-soon/
[Embed: Why Your User Mode Pointer Captures are Probably Broken (or will be...]
Last reviewed and updated: 14 February 2022 By: Jonathan Morrison [Editors’ Note:  Jonathan Morrison was developer in Microsoft’s Core Operating Systems group back in 2008 when he provi…

[2025-07-07 19:01] UJ: [replying to Windy Bug: "Out of curiosity - has this become common knowledg..."]
Interesting. 

Using the `volatile` keyword to do the opposite of what people usually use the keyword for. Instead of telling the compiler this value can change behind the scenes, so make sure you read from it each time, ie in a `while(cond)` loop, in this case its being used to tell the compiler this value can change behind the scenes so make sure you use the cached variable instead of re-reading it each time to avoid any TOCTOU bugs.

[2025-07-07 23:02] JustMagic: [replying to Windy Bug: "Out of curiosity - has this become common knowledg..."]
This is mostly untrue for reasonable driver dev

[2025-07-07 23:03] JustMagic: If you're compiling with MSVC /kernel flag ensures that the outlined problems don't actually happen

[2025-07-07 23:04] JustMagic: Although Microsoft is moving away from that behaviour exactly because it prevents optimization opportunities

[2025-07-08 03:51] Windy Bug: [replying to JustMagic: "Although Microsoft is moving away from that behavi..."]

[Attachments: IMG_5971.png]

[2025-07-08 05:07] JustMagic: [replying to Windy Bug: ""]
Knowing Microsoft, they'll never (intentionally) break backwards compat. You'll have to manually opt-out of it.

[2025-07-12 12:08] GG: hello all, does any one have access to this repo ? possibly cloned it or forked it before ?

https://git.scc.kit.edu/CES/clang-custom-pragma