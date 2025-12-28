# October 2025 - Week 4
# Messages: 46

[2025-10-20 06:17] Woah.: Yeah

[2025-10-20 12:49] oopsies: I am re-hashing PCRs and spoofing nvram,  but certain ones I cant do right on other PCs (oem-specific stuff)

[2025-10-20 12:50] oopsies: does vanguard check pcr3 (oem-specific) and its event logs?

[2025-10-20 12:50] oopsies: Im trying to make this usable on any device

[2025-10-20 13:04] lukiuzzz: [replying to oopsies: "does vanguard check pcr3 (oem-specific) and its ev..."]
Yeah Vanguard almost certainly checks PCR3 and its log to see if any OEM components or firmware were tampered with before boot, since that's a classic way to inject a bootkit.

[2025-10-20 13:05] oopsies: [replying to lukiuzzz: "Yeah Vanguard almost certainly checks PCR3 and its..."]
they mightve won..

[2025-10-20 13:06] oopsies: if this dont work im forced to go back to trying to fix hyper v

[2025-10-20 13:06] lukiuzzz: [replying to oopsies: "if this dont work im forced to go back to trying t..."]
Hyper v might be a good solution 😂

[2025-10-20 16:35] jordan9001: Anti-cheat checking PCRs is interesting. How do they have a known good for the PCRs? Just trust whatever it sees first combined with a hwid? How do they handle fw updates?

[2025-10-20 17:04] lukiuzzz: They  take a fingerprint of your PCRs on a clean install and add it to a whitelist, and then just alert on any deviation from that baseline. Firmware updates would force a re-baseline, which is why you often see Vanguard require a reboot after a system change.

[2025-10-20 17:11] LabGuy94: i haven't dabbled with TPM, but does it just pretty much kill the ability to load bootkits

[2025-10-20 17:12] LabGuy94: in the sense of your PCRs will register that something was manipulated pretty much no matter what you do

[2025-10-20 20:30] 001: i mean its still possible but you would need to patch bios, specifically to intercept when the tpm is set up

[2025-10-20 20:33] 001: or if ur bios is bugged and leaks the owner keys

[2025-10-20 20:33] 001: either way its a herculean task

[2025-10-20 20:33] 001: just cheat in another way

[2025-10-22 07:54] safareto: saw vxunderground posting this but it looks really cool: https://www.unknowncheats.me/forum/anti-cheat-bypass/718917-elysium-uefi-bootkit-framework-attacks-boot-time-code-integrity.html
[Embed: Elysium - UEFI Bootkit Framework that attacks boot-time Code Integrity]
Background A couple of months ago, while reversing winload.efi, I got the idea to patch the boot driver certificate check so unsigned drivers could be

[2025-10-22 10:09] Xits: I don’t understand what’s so cool here? It’s just a standard bootkit

[2025-10-22 10:20] safareto: [replying to Xits: "I don’t understand what’s so cool here? It’s just ..."]
i dont know much about bootkits but found it nice, might just be standard but liked it and thought of sharing it here

[2025-10-22 10:21] Xits: It’s a good write up

[2025-10-22 11:40] Rairii: [replying to LabGuy94: "i haven't dabbled with TPM, but does it just prett..."]
it depends where and how you're loading the bootkit. for example some windows bootloader exploits allow for code execution before it measures values in some pcrs

[2025-10-22 11:47] Rairii: (and not just exploits, there's a trick to get code exec in winload as a feature - finally removed in i think Ge (iirc Co still has it), and again this occurs before winload actually measures anything into PCRs)

[2025-10-22 14:38] LabGuy94: [replying to safareto: "saw vxunderground posting this but it looks really..."]
its kinda just an overhyped article about patching a single function on boot but cant ask for more on uc

[2025-10-22 14:39] safareto: [replying to LabGuy94: "its kinda just an overhyped article about patching..."]
im too much of a noob to recognise that sort of thing lol

[2025-10-22 15:12] Horsie: I think EfiGuard is a far cooler name.

[2025-10-22 15:13] Horsie: Guards your computer from evil microsoft code.

[2025-10-22 21:38] qfrtt: Give this man a medal

[2025-10-22 21:58] oopsies: I bypass measured boot by putting cert in db and spoofing db before I installed any game or AC. I hope it's not a delay ban. Some PCR digests are almost always calculated the same across OEMs. They also collect the windows boot logs and TPM event logs. The only issue with this is that you need a new PC or a spoofer.

[2025-10-24 00:22] jenrix: Quick question, I’m hooking the WoW64 transition to hide DRx and used TitanHide’s trick with NtGet/NtSet/NtSetInformationThread/NtQueryInformationThread. Any idea why KiUserExceptionDispatcher doesn’t detect it? Could it be because TitanHide runs in the kernel?.

[2025-10-24 04:55] lukiuzzz: [replying to jenrix: "Quick question, I’m hooking the WoW64 transition t..."]
KiUserExceptionDispatcher is checking the debug registers directly from the kernel or using a different thread context, so it sees the real values that your usermode hook can't spoof.

[2025-10-24 05:48] LabGuy94: [replying to Horsie: "I think EfiGuard is a far cooler name."]
It's almost as if...

[2025-10-25 00:13] Matti: [replying to jenrix: "Quick question, I’m hooking the WoW64 transition t..."]
not sure I get your question
what would you expect KiUserExceptionDispatcher to detect, and why? its job generally isn't to detect things, it's the... user mode exception dispatcher

[2025-10-25 00:14] Matti: the name is a bit misleading, but this function is an ntdll export that is looked up by the kernel once user mode is started

[2025-10-25 00:16] Matti: `NtSetInformationThread` (and Get) with `ThreadWow64Context` have some pretty specific restrictions, it this is what you're using

[2025-10-25 00:17] Matti: these are essentially NtGet/SetContextThread for wow64 processes, but they can only be called from user mode, and only from native 64 bit code

[2025-10-25 00:18] Matti: they are really only used by the wow64 user mode layer as far as I know, when a wow64 process in 32 bit mode tries to call NtGet/SetContextThread

[2025-10-25 15:52] Novoline: [replying to safareto: "saw vxunderground posting this but it looks really..."]
This hasn't been done at least 20 times before, the UC kitties mana bar is 1% higher than it was before.

[2025-10-25 17:08] iris8721: bio is very tuff

[2025-10-26 02:31] abu: Hello, I was researching TPM and wanted to confirm some information that I have about it. I understand that there is a private key (EKPriv) and a public key (EKPub). I know that anti-cheats can/will use the EKPub as some hardware identifier but can't someone clear their keys and regenerate a different EKPub? For the EKCert, i only saw a SerialNumber field that was "unique" to the certificate. Would they use this as well or just hash EKPub and attest through the respective portal? Thanks!

[2025-10-26 02:41] iris8721: serial number will be used too i assume

[2025-10-26 02:41] iris8721: although for some reason using qflash plus on my machine totally regenerates it (???)

[2025-10-26 04:44] abu: [replying to iris8721: "although for some reason using qflash plus on my m..."]
This is odd. I thought the Tpm uses a public key encryption scheme. Wouldn’t there only be one? Unless when you flashed, it somehow changed the Endorsement primary seed but I believe that it exists inside the Tpm

[2025-10-26 04:45] abu: Nevermind, lookup: TPM_ChangeEPS

[2025-10-26 12:48] Anthony Printup: [replying to abu: "Hello, I was researching TPM and wanted to confirm..."]
the identifiers don't matter at all, TPMs are designed to be anonymous

what's interesting for anti cheats is a valid certificate which signs the ekpub, backed by the seed
as soon as you change your primary seed, you can no longer attest your TPM's validity

[2025-10-26 23:39] abu: [replying to Anthony Printup: "the identifiers don't matter at all, TPMs are desi..."]
I read that changing the seed through the previously mentioned API would make the TPM generate another random ,but valid ,seed to attest the TPM’s validity

[2025-10-26 23:40] abu: [replying to Anthony Printup: "the identifiers don't matter at all, TPMs are desi..."]
Nevermind, thanks