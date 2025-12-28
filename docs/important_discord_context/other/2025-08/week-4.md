# August 2025 - Week 4
# Messages: 139

[2025-08-21 00:06] WolverinDEV: Does anyone know if vgk or faceit ac actually do attestation of the measured boot?

[2025-08-21 07:08] lukiuzzz: [replying to WolverinDEV: "Does anyone know if vgk or faceit ac actually do a..."]
No only forcing to enable secure boot and tpm

[2025-08-21 11:53] Tr1x: [replying to WolverinDEV: "Does anyone know if vgk or faceit ac actually do a..."]
They can and will utilize data from the TPM if they can. https://github.com/mattifestation/TCGLogTools you can use this to output your TPM logs into json
[Embed: GitHub - mattifestation/TCGLogTools: A set of tools to retrieve and...]
A set of tools to retrieve and parse TCG measured boot logs. Microsoft refers to these as Windows Boot Confirguration Logs (WBCL). In order to retrieve these logs, you must be running at least Wind...

[2025-08-21 12:09] WolverinDEV: [replying to Tr1x: "They can and will utilize data from the TPM if the..."]
I do know that actually. I was more wondering however they do attestation in practice hence spoofing the TCG log would be possible.

[2025-08-21 16:53] Aj Topper: How to find vulnerable drivers
So basically I am aware of the existing methods to probe for vulnerable drivers (ex. Vulnerable Driver Scanner) or how to analyze the found drivers to find vulns, but my main issue is
How do I download a big list of drivers?
Like is there a scraper available online, or a list of drivers somewhere?
I know there are categories of drivers being more "vuln" let's say, like RGB  but how does one find these drivers in the first place?

[2025-08-21 17:02] Hunter: two years ago or so i scraped all the files that had a “.sys” file extension from a website that provides windows files incase ur system doesnt have them, and then validated and sorted them by architecture and manually analyzed them in ida

[2025-08-21 17:07] cinder: [replying to Aj Topper: "How to find vulnerable drivers
So basically I am a..."]
there is a website that provides something like 47GBs of drivers to download through torrent

[2025-08-21 17:09] cinder: not sure if it is well-known, but it could be a start. let me see if I can find it

[2025-08-21 17:09] Aj Topper: [replying to Hunter: "two years ago or so i scraped all the files that h..."]
microsoft windows catlog or what...?

[2025-08-21 17:10] Aj Topper: [replying to cinder: "not sure if it is well-known, but it could be a st..."]
if you find it pls ping me

[2025-08-21 17:11] cinder: found it, dm-ing to you

[2025-08-21 17:12] cinder: not sure if it is aganist the rules to share it here, so I'll thread safely and not do that

[2025-08-21 17:25] daax: [replying to cinder: "not sure if it is aganist the rules to share it he..."]
it's not against the rules.

[2025-08-21 17:27] daax: [replying to Aj Topper: "How to find vulnerable drivers
So basically I am a..."]
https://vx-underground.org/2025%20Vulnerable%20Driver%20Project

[2025-08-21 18:24] cinder: [replying to daax: "it's not against the rules."]
i see, then i'll gladly share

[2025-08-21 18:25] cinder: very public i think but it's good practice and you may find something good in there

[2025-08-21 18:27] daax: why not just send the link

[2025-08-21 18:27] daax: to the offline archive

[2025-08-21 18:28] cinder: not home so that's the best i can do from the gym

[2025-08-21 18:28] cinder: i could share the torrent file once i get home

[2025-08-21 18:28] cinder: if that's okay

[2025-08-21 18:36] daax: https://uploadrar.com/4hmiektgb29i
[Embed: Uploadrar - Easy way to share your files]
Uploadrar - Free file upload service

[2025-08-21 18:36] daax: <@786963389397467206>

[2025-08-21 18:36] daax: or https://dl.driverpack.io/DriverPack-Offline.torrent

[2025-08-21 19:03] cinder: thanks

[2025-08-21 19:18] Aj Topper: [replying to daax: "<@786963389397467206>"]
thanks saar, very cool

[2025-08-22 05:53] Horsie: This basically thread reads like this lol.
[Attachments: 1665668630-557990265.jpg, Screenshot_20250822-112154.png]

[2025-08-22 05:53] Horsie: Chompie on top left, Low learning on bottom left <:topkek:904522829616263178>
But LLL is known for having extremely bad takes so I don't mind turning into a contrarian just to piss him off

[2025-08-22 05:54] Horsie: <https://x.com/chompie1337/status/1958380849562574907>

[2025-08-22 06:24] brockade: I think this sums up their differences though: https://x.com/chompie1337/status/1958419386727035255
@ruostu “Instead of focusing frustration on game companies, consider redirecting that energy toward issues like OEM vendors shipping RGB lighting drivers with vulnerabilities that allow read/write mem

[2025-08-22 06:25] brockade: saying "no meaningful reason" implies game devs haven't already spent literally decades trying to accomplish this without kernel

[2025-08-22 07:50] Horsie: Fair point

[2025-08-22 13:56] daax: [replying to brockade: "saying "no meaningful reason" implies game devs ha..."]
Correct. He likes to oversimplify many things and I’m personally not havin’ it anymore. He can do his little play pretend “I make/reverse malware!” like so many others — whatever makes you feel good, this was just a step too far in the wrong direction, and he has progressively been saying very inaccurate things about all sorts of topics wrt platform security / os security.

[2025-08-22 14:06] cinder: are we talking about low level learning?

[2025-08-22 14:06] brockade: yeah and <@162611465130475520>'s link from this morning basically rehashes and then settles this argument that we've all already seen enough times over the past however many years https://discord.com/channels/835610998102425650/835655011115466772/1408388717751570486

[2025-08-22 14:06] daax: [replying to Horsie: "Chompie on top left, Low learning on bottom left <..."]
It’s easier to accept her difference of opinion because she’s not an egotistical nightmare and has extensive experience in VR and kernel development. He does not. He does oversimplify concepts though for the Redditors who like to LARP as security experts and have strong opinions about how best to build an anti-cheat or catch malware. I don’t agree with chompie on the whole anyways, but she’s not annoying about it.

[2025-08-22 14:08] daax: [replying to brockade: "yeah and <@162611465130475520>'s link from this mo..."]
Blog post doesn’t help much here because pretty sure most people here understand the why. Feel free to share that with the YouTubers though.

[2025-08-22 14:24] Horsie: [replying to daax: "It’s easier to accept her difference of opinion be..."]
Of course, I'm just poking some fun at yall. Respect you both :P

[2025-08-22 14:25] Horsie: [replying to daax: "Correct. He likes to oversimplify many things and ..."]
Maybe I should start using em dashes in my messages unironically too.

[2025-08-22 15:57] Tr1x: [replying to WolverinDEV: "I do know that actually. I was more wondering howe..."]
Well right now there is holes in the attestation, as there is an accessible transportation layer which the application have to communicate through. But if for example Microsoft Pluton starts becoming more popular it can be verified from the cloud, when that happens it can't be as easily manipulated.

[2025-08-22 16:22] iris: so glad people are finally calling him out on his bs

[2025-08-22 16:25] cinder: [replying to iris: "so glad people are finally calling him out on his ..."]
about time indeed

[2025-08-22 16:26] iris: saw his video about the val/bf6 thing that went viral, noticed hes now selling multiple courses at once

[2025-08-22 16:26] iris: 😭

[2025-08-22 16:26] cinder: he turned into that kind of youtuber

[2025-08-22 16:27] cinder: i enjoyed his first videos but at some point the thumbnails became clickbait, the content became slop and here we are today

[2025-08-22 16:29] iris: yerp sucks to see

[2025-08-22 16:30] cinder: he became cocky out of the blue

[2025-08-22 16:31] cinder: his content 4 years ago vs. now
spoiler tag because rage inducing
[Attachments: image.png, SPOILER_image.png]

[2025-08-22 16:39] iris: probably when he realized that he could open strings view in ghidra and claim he had reversed something and 99% of people on youtube wouldnt care

[2025-08-22 16:39] cinder: this is the gold standard

[2025-08-22 16:43] elias: [replying to Tr1x: "Well right now there is holes in the attestation, ..."]
normal TPMs are enough to do remote attestation

[2025-08-22 16:43] elias: pluton aims to prevent physical attacks like bus sniffing

[2025-08-22 17:57] 0x22: <@111472230424154112>

[2025-08-22 17:57] 0x22: ❤️

[2025-08-22 18:10] carl: yoo

[2025-08-22 18:11] 5pider: hih

[2025-08-22 18:11] carl: what's happening in here

[2025-08-22 18:11] 5pider: i know the admin from here, i  can get u banned

[2025-08-22 18:11] carl: [replying to 0x22: "<@111472230424154112>"]
oldest member of them all tagging me without context

[2025-08-22 18:12] qfrtt: [eyes_shaking](https://cdn.discordapp.com/emojis/909523150096719912.gif?size=48&animated=true&name=eyes_shaking)

[2025-08-22 18:12] 0x22: [replying to carl: "oldest member of them all tagging me without conte..."]
isnt that ok

[2025-08-22 18:12] 0x22: haha

[2025-08-22 18:12] carl: always

[2025-08-22 18:12] 0x22: im gonna visit your country in 7 days

[2025-08-22 18:12] carl: ooo where to?

[2025-08-22 18:12] 0x22: dk ?

[2025-08-22 18:12] carl: selvfølgelig

[2025-08-22 18:12] 0x22: absolutt

[2025-08-22 18:13] 0x22: too bad dkk is so fuckin high

[2025-08-22 18:15] carl: \> literal oil money citizen

[2025-08-22 18:15] carl: \> wow this foreign currency is expensive

[2025-08-22 18:15] iPower: guys this is the anti-cheat channel...

[2025-08-22 18:15] carl: we are all against cheating

[2025-08-22 18:16] carl: #anti #cheating #foryou

[2025-08-22 18:18] 0x22: [replying to iPower: "guys this is the anti-cheat channel..."]
sorry boss

[2025-08-22 18:58] daax: [replying to Horsie: "Maybe I should start using em dashes in my message..."]
eh? I've always done em dashes <:Kappa:794707301436358686>

[2025-08-22 18:59] daax: you can see my uc posts from 2016 -- llms ARE RUINING IT

[2025-08-22 18:59] daax: <:whyy:820544448798392330>

[2025-08-22 19:11] avx: 2016 <:old_man_yells_at_microsoft:1352528846720729108>

[2025-08-22 19:28] Tr1x: [replying to elias: "normal TPMs are enough to do remote attestation"]
Kind of, you're able to do remote attestation but it isn't as easy as you're saying. If you're a game's company you need to maintain a list of certificates from manufacturers of TPM chips for verifying that the endorsement keys that signed that piece of data are legitimate, integrate the cloud infrastructure for supporting it then continuously keep informed about changes to the standards etc. Pluton does more then just stopping bus sniffing, it is an entire platform that is very closely integrated with Microsoft's cloud infrastructure, making it more viable for developers to integrate their software with. Game's company aren't going to bother until more system make it a requirement and they have easy systems to integrate with that doesn't require them to maintain their own.

[2025-08-22 20:54] Pepsi: [replying to Tr1x: "Kind of, you're able to do remote attestation but ..."]
Microsoft already maintains a list of CAs that includes all major TPM vendors. I don't think it is that complex as you are making it look like.

[2025-08-22 20:59] Pepsi: [replying to elias: "pluton aims to prevent physical attacks like bus s..."]
Not only can you do sniffing, you could even build a device that intercepts the bus and feeds spoofed measurements to the TPM.

[2025-08-22 21:04] Pepsi: [replying to Tr1x: "Kind of, you're able to do remote attestation but ..."]
Also is this AI slop?

[2025-08-22 23:29] Tr1x: [replying to Pepsi: "Microsoft already maintains a list of CAs that inc..."]
Can you send me a download to this universal list of TPM vendors CA's? Because if you're talking about Host Guardian Service, it isn't used for this case.

[2025-08-22 23:32] Pepsi: [replying to Tr1x: "Can you send me a download to this universal list ..."]
this list is excactly what you are looking for, its a package with all(?) TPM Root CAs

[2025-08-22 23:34] Pepsi: inlcuding amd/intel ftpm, microsoft, infineon, Nuvoton, etc

[2025-08-22 23:34] Pepsi: <https://learn.microsoft.com/en-us/windows-server/security/guarded-fabric-shielded-vm/guarded-fabric-install-trusted-tpm-root-certificates>
to quote:
> A collection of trusted TPM root and intermediate certificates is published by Microsoft for your convenience.

[2025-08-22 23:35] WolverinDEV: [replying to Tr1x: "Well right now there is holes in the attestation, ..."]
Attestation is designed to work even in the presence of possible MITM attacks; defending against those is already part of the security considerations.
The interesting question is about wherever typical ac solutions have the backend infrastructure to actually perform attestations.

[2025-08-22 23:37] WolverinDEV: Personally if I were to develop an ac I would upload the TCG logs, PCR registers and the attestation data to my server and on demand perform such verification if I support the TPM vendor and the player has been flagged as suspicious.

[2025-08-23 00:17] Tr1x: [replying to Pepsi: "this list is excactly what you are looking for, it..."]
This isn't a bad list I thought this was only integrated into HGS I didn't think it was released separately for download to the public, this might be fine for managing millions of consumer PCs but I don't have the data to know that and if it lacks some CA's. But you still have the problem of standardization which will surely be solved in the future and even with this list there is still technical challenges as well as overhead that exists, especially if you aren't relying on cloud infrastructure managing this at scale. Full and even partial attestation at scale is I don't know but there is a reason why Microsoft manages a cloud solution for this.

[2025-08-23 00:25] Tr1x: [replying to WolverinDEV: "Attestation is designed to work even in the presen..."]
With proper integration I agree but if there is a will there is a way. As for the backend infrastructure to properly perform attestations, I think they'll just rely on the cloud solutions that are being pushed out for it, there isn't a point in my eyes to build that.

[2025-08-23 00:28] Pepsi: which cloud solutions are you even reffering to?

[2025-08-23 00:29] Pepsi: like, is there even anything that can be used by external partys? which apis are we talking about? pls link some resources

[2025-08-23 00:29] Tr1x: [replying to Pepsi: "like, is there even anything that can be used by e..."]
https://azure.microsoft.com/en-us/products/azure-attestation

[2025-08-23 00:34] Pepsi: microsoft marketed pluton stuff as "chip to cloud security", but to me it looks like a glorified TPM with OTA update

[2025-08-23 00:43] Tr1x: [replying to Pepsi: "microsoft marketed pluton stuff as "chip to cloud ..."]
I wouldn't say you're entirely wrong but regardless it isn't solely about Pluton itself. It is ultimately about ecosystem and developer experience, they own the market as well so 🤷🏿‍♂️

[2025-08-23 01:36] daax: [replying to Pepsi: "Also is this AI slop?"]
Yea

[2025-08-23 02:01] daax: Problem solved. Don’t use AI to respond to technical questions (or any really). It varies in how apparent it is, but people pick up on the patterns. If you don’t have anything to add to a discussion and have to resort to AI: don’t respond.

[2025-08-23 02:17] iris: the consequences of one too many em dashes 😔

[2025-08-23 03:07] daax: [replying to Tr1x: "Kind of, you're able to do remote attestation but ..."]
https://github.com/nsacyber/HIRS
https://keylime.dev/
https://github.com/google/go-attestation

[2025-08-23 06:40] KR: [replying to WolverinDEV: "Attestation is designed to work even in the presen..."]
Thats all well and good but TPM emulation is a well practiced technique, especially within VMs. Further, you need to consider these vendors are heavily limited in what they can actively test for, you can't exactly ban someone because they cleared their TPM, or even swapped it out entirely.

There also is not much stopping you from hooking the calls to tpm.sys/tbs.dll before it ever reaches the TPM. As far as attestation goes it leaves a lot to be desired.

[2025-08-23 07:55] Brit: [replying to KR: "Thats all well and good but TPM emulation is a wel..."]
Show me this "well practiced" tpm 2.0 emulator...

[2025-08-23 07:57] KR: [replying to Brit: "Show me this "well practiced" tpm 2.0 emulator..."]
https://github.com/stefanberger/swtpm/wiki
[Embed: Home]
Libtpms-based TPM emulator with socket, character device, and Linux CUSE interface. - stefanberger/swtpm

[2025-08-23 08:02] KR: https://github.com/stefanberger/libtpms
[Embed: GitHub - stefanberger/libtpms: The libtpms library provides softwar...]
The libtpms library provides software emulation of a Trusted Platform Module (TPM 1.2 and TPM 2.0) - stefanberger/libtpms

[2025-08-23 08:24] Horsie: When we talk about attestation here, its still just about measured boot right?

[2025-08-23 08:25] Horsie: How well does measured boot translate across different kinds of hardware? I assume, firmware PCRs dont translate well at all across vendors?

[2025-08-23 08:26] Horsie: I guess the windows bootloader/kernel measurements are the only ones that might be useful. Please correct me if I'm missing something here

[2025-08-23 08:27] Horsie: If thats the case, TPMs don't really add much if you have secure boot right? Its just orthogonal?

[2025-08-23 08:29] Horsie: Because when I read attestation, the usual SGX style cpu-based stuff comes to mind. Where you have root of trust in your own silicon and you can use it for quoting and remote attestation stuff.
Do the way I see it, TPMs provide attestation by tpm vendor keys instead of intel's.

This was just a long winding way to ask how this is a topic of interests for ACs.

[2025-08-23 08:30] Oliver: [replying to Pepsi: "microsoft marketed pluton stuff as "chip to cloud ..."]
the root of trust is what's desired so sounds like a correct observation

[2025-08-23 11:35] Pepsi: [replying to KR: "Thats all well and good but TPM emulation is a wel..."]
You might emulate a TPM and it functions, but you can't emulate having a trusted EK

[2025-08-23 13:23] daax: [replying to KR: "Thats all well and good but TPM emulation is a wel..."]
You can bypass *tbs.sys* and go directly to communicating with the device. This is what some ACs opt for. There is some significant overcomplication going on here lol.

[2025-08-23 14:39] Oliver: [replying to Pepsi: "You might emulate a TPM and it functions, but you ..."]
emulating the tpm is a no go zone for a serious ac

[2025-08-23 14:42] Pepsi: [replying to daax: "You can bypass *tbs.sys* and go directly to commun..."]
do you know whats their motivation behind this?

[2025-08-23 14:52] daax: [replying to Horsie: "How well does measured boot translate across diffe..."]
you can read the tfa docs + tcg efi spec, but it can differ -- hash algos might be different or the active pcr banks can be different; but that's what the getcapability (defined in efi spec) allows you to determine. the pcr values don't translate well, since theyre a cumulative hash of every specific code/data measured on that *particular platform*. so, pcr from one vendor and boot config will be diff from another but they will have to conform to the standard.

[2025-08-23 15:01] daax: [replying to Horsie: "If thats the case, TPMs don't really add much if y..."]
uhhh, no. distinct functions but kind of reinforce the other. sb is an srtm mechanism, verify boot components by checking the signatures against the "trusted keys" (which you can control). measured boot with the tpm is inteded to record hashes (measurements) of whatever resource at various stages of the boot chain prior to that resources use. easiest way to think of it is a boot audit, so it can be verified later. secure boot was made to "prevent" tampered crap from loading, measured bot is to be able to detect if boot state had any deviations from the "known good" even if sb was enabled / if sb was bypassed. either way the end goal is drtm + tpm + stm/ppam.

[2025-08-23 15:04] Bony: [replying to Pepsi: "do you know whats their motivation behind this?"]
why wouldn't you go as low as possible

[2025-08-23 15:05] Horsie: [replying to daax: "uhhh, no. distinct functions but kind of reinforce..."]
I see. So if I'm understanding it correctly, for firmware, unless you have root of trust in the hardware (read as BootGuard/PSB), the thing doing the measurement is executing at the same privilege/integrity level as the code to be measured. So instead of having a strong guarantee that integrity is preserved, it just becomes another thing that you have to essentially manipulate by sending the TPM known-good measurement data?

[2025-08-23 15:06] Pepsi: [replying to Bony: "why wouldn't you go as low as possible"]
as <@609487237331288074> said, it is a significant overcomplication and i don't see what problem they are trying to solve by doing this

[2025-08-23 15:06] Horsie: Would it be possible to mitigate that with the SGX attestation where the code used for measurement is running on some king of hardware enclave? Hmm

[2025-08-23 15:07] Horsie: Because for root of trust in hardware, I'm sure BootGuard is not a popular pro-consumer choice.

[2025-08-23 15:08] Horsie: Had a stroke mid-typing. Mind the grammar. fixing it

[2025-08-23 15:08] daax: [replying to Horsie: "Because when I read attestation, the usual SGX sty..."]
to this: ek is primary interest to acs. also they could just be based and do what cod did with enrollaik lol.

[2025-08-23 15:12] Horsie: Not well versed with TPMs enough to know what info you can derive with just that. Does it establish that the TPM vendor is legit? Reading this in the context of the prior conversation but still a bit confused. <https://learn.microsoft.com/en-us/azure/attestation/overview>. Can TPM-only attestation be as strong as SGX/SEV

[2025-08-23 15:16] daax: [replying to Horsie: "I see. So if I'm understanding it correctly, for f..."]
well sure whatever rot is picked by that environment is typically assumed to be trusted and act as this "root of trust". also i think you're confusing how it works. the measured information in the tpm is immutable. you can only do one-way extensions of existing data + there is an event log (though it could be faked to *some requestors* you could just read it directly) + this is all intended to be done in conjunction with senter/skinit (drtm). i don't know how you expect to easily control any of that.

[2025-08-23 15:18] Horsie: I see. I'm definitely missing big parts about how it works, now that you put it like that. I'll read up TPMs a bit more before continuing this.

[2025-08-23 15:20] daax: [replying to Horsie: "Not well versed with TPMs enough to know what info..."]
tpms come with an endorsement key (ek) that's fused in -- burned into the tpm... you're not going to be able to change it or remove it (barring sophisticated physical attacks that probably still won't net any useful results)

[2025-08-23 15:21] Horsie: [replying to daax: "tpms come with an endorsement key (ek) that's fuse..."]
Interesting to ACs for HWID?

[2025-08-23 15:21] daax: [replying to Horsie: "Interesting to ACs for HWID?"]
then they can just process the cert and verify if it came from a genuine manufacturer. and yes, eks are unique to the tpm

[2025-08-23 15:22] Horsie: Hmm...

[2025-08-23 15:23] Anthony Printup: ig this is a good article if you're interested <@491503554528542723> https://tpm2-software.github.io/2020/06/12/Remote-Attestation-With-tpm2-tools.html
[Embed: Remote Attestation With Tpm2 Tools]
Table of Contents

[2025-08-23 15:23] Anthony Printup: tl;dr
get ek
get certs
make credential server side
activate credential -> extract secret -> verified tpm owns EK

[2025-08-23 15:24] Anthony Printup: and then you can issue quotes using an AK tied to the EK

[2025-08-23 15:24] daax: or do what cod does and just run certreq*

[2025-08-23 15:24] daax: <:Kappa:794707301436358686>

[2025-08-23 15:24] Anthony Printup: i gotta check that out, never heard of it xd

[2025-08-23 15:26] Anthony Printup: okay that's hilarious haha

[2025-08-23 19:43] Pepsi: [replying to Anthony Printup: "ig this is a good article if you're interested <@4..."]
this is a helpful article, thanks for posting