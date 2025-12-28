# December 2025 - Week 1
# Messages: 214

[2025-12-01 19:10] 漆間俊: [replying to Jacob: "Oh like I know Dmakingdom sells stuff like that bu..."]
i mean if u got makcu or ferrum i guess you can flash whatever you want on it as long as you are familiar with their board,  otherwise make your own esp32 board ask some chinese company to build it and then code ur own fw for it

[2025-12-03 19:17] impost0r: Has anyone dealt with the new NEP2 anticheat? Both usermode and (new) kernel-mode. Serves to protect Unity games by messing with how global-metadata.dat is loaded. Games I'm looking at are Girl's Frontline and Girl's Frontline 2

[2025-12-03 19:18] impost0r: It's NetEase/Tencent.

[2025-12-03 19:27] phage: Can you provide a binary? I'd love to see what obfuscation it has 🙂

[2025-12-03 19:49] void: [replying to impost0r: "Has anyone dealt with the new NEP2 anticheat? Both..."]
you can turn the entire thing off, it has no heartbeat

[2025-12-03 20:06] impost0r: [replying to phage: "Can you provide a binary? I'd love to see what obf..."]
Yeah, I can. Sec.

[2025-12-03 20:07] impost0r: [replying to void: "you can turn the entire thing off, it has no heart..."]
GFL2 also dumps the decrypted assemblies into a temp folder, lol.

[2025-12-03 20:12] phage: ty!

[2025-12-03 20:12] phage: i got them

[2025-12-03 20:12] impost0r: aight

[2025-12-03 20:12] impost0r: I guess Discord believes it's malware lol

[2025-12-03 20:12] phage: they're native?

[2025-12-03 20:12] impost0r: Yep

[2025-12-03 20:14] impost0r: password is `nep2`, fuck you Discord
[Attachments: NEP2.7z]

[2025-12-03 20:14] impost0r: for anyone else who wants a look

[2025-12-03 20:15] phage: certainly a VM, just cant tell which one it is yet
[Attachments: image.png]

[2025-12-03 20:20] koyz: most likely Tencent's own VM if it is made by Tencent

[2025-12-03 20:20] phage: They have their own VM?

[2025-12-03 20:20] phage: Christmas came early this year for me 😄

[2025-12-03 20:21] ImagineHaxing: Lol

[2025-12-03 20:22] Brit: pretty sure they used to use beebyte

[2025-12-03 20:22] Brit: maybe they moved on since then

[2025-12-03 20:22] phage: something thats not just VMP or Themida 🥳

[2025-12-03 20:23] phage: or a rip off of loki but worse

[2025-12-03 20:31] impost0r: [replying to Brit: "pretty sure they used to use beebyte"]
BeeByte (or some variant) for the decrypted assemblies, yeah. Won’t even load in dotPeek, lmao.

[2025-12-03 20:34] impost0r: I’ve been wanting to make actual mods for the game but alas.
-# Also kinda funny that GFL2 only has the userland AC, the older game ships with the driver.

[2025-12-04 02:10] daax: [replying to phage: "They have their own VM?"]
yea and it’s pretty unimpressive

[2025-12-04 02:13] toro: Why do they write their own weaker VMs when things like VMP and Themida are still viable solutions?

[2025-12-04 02:13] phage: They’re not tbh

[2025-12-04 02:13] phage: They’re weak and they blown apart

[2025-12-04 02:13] toro: How so? Im not familiar with any 'public' methods to devirt Themida's more complex VMs

[2025-12-04 02:14] phage: [replying to daax: "yea and it’s pretty unimpressive"]
Yeah, wrote a very simple pattern for getting their VM enters and it’s completely one shot

[2025-12-04 02:14] toro: VMP was leaked years ago but no information about a full devirtualizer either. Ofc this doesn't mean privately things aren't going around I suppose.

[2025-12-04 02:14] phage: [replying to toro: "How so? Im not familiar with any 'public' methods ..."]
Their “complex” VMs aren’t complex

[2025-12-04 02:15] phage: [replying to toro: "VMP was leaked years ago but no information about ..."]
https://github.com/Colton1skees/Dna
[Embed: GitHub - Colton1skees/Dna: LLVM based static binary analysis framework]
LLVM based static binary analysis framework . Contribute to Colton1skees/Dna development by creating an account on GitHub.

[2025-12-04 02:15] toro: semantics aside, has there been any claims that all of CV's VMs have been devirtualized? I know at least one person who has devirt Falcon and Fish

[2025-12-04 02:16] contificate: what is the threshold for "devirtualised"

[2025-12-04 02:17] phage: [replying to toro: "semantics aside, has there been any claims that al..."]
Plenty

[2025-12-04 02:17] phage: You’re just not in the right spaces for those discussions

[2025-12-04 02:17] toro: fair enough

[2025-12-04 02:18] toro: [replying to contificate: "what is the threshold for "devirtualised""]
I'd say the threshold is successfully lifting the original logic to some degree

[2025-12-04 02:18] phage: [replying to contificate: "what is the threshold for "devirtualised""]
Most people I guess would consider “an output that is equivalent to the semantics of the original code”

[2025-12-04 02:18] contificate: well that's the identity function

[2025-12-04 02:18] contificate: I don't understand why people don't do novel VMs

[2025-12-04 02:18] the horse: i think most people would count a deobfuscating rewriter/recompiler for entire binaries

[2025-12-04 02:18] contificate: they seem to be the same shit

[2025-12-04 02:19] contificate: I understand there's practical limitations

[2025-12-04 02:19] contificate: like nobody would use something with 100x slowdown

[2025-12-04 02:19] contificate: but.. like.. for academic fun, why isn't there more novel VMs

[2025-12-04 02:19] toro: [replying to the horse: "i think most people would count a deobfuscating re..."]
I don't know if that's practically necessary in most situations

[2025-12-04 02:19] the horse: they need absolutely extensive compatibility due to a large customer-base using ancient hardware

[2025-12-04 02:19] phage: I mean there are but all their tricks are just fancier MBAs

[2025-12-04 02:20] contificate: nothing about encoding, control flow, etc.?

[2025-12-04 02:20] phage: Which to be honest just aren’t impressive for real world samples

[2025-12-04 02:20] contificate: and I don't mean instruction encoding

[2025-12-04 02:20] contificate: there are simple whole program transformations from compiler literature that would confuse most nonsense

[2025-12-04 02:20] phage: I mean most people can’t deal with linear or polynomial MBAs, so academia can spam all their non-linear they want but

[2025-12-04 02:20] phage: Most people can’t even lift x86 accurately

[2025-12-04 02:21] phage: Im including myself in that category btw

[2025-12-04 02:21] the horse: because the docs are lying

[2025-12-04 02:21] contificate: the mental block I have in writing a virtualisation project is that I don't know good algorithmic ways to generate variable specifications that aren't just noise

[2025-12-04 02:21] the horse: and the arch is not properly standardized across vendors

[2025-12-04 02:22] toro: [replying to phage: "Most people can’t even lift x86 accurately"]
yea, properly lifting x86 is arguably the more difficult task tbh and if that foundation is bad the rest gets exponentially worst.

[2025-12-04 02:22] phage: [replying to toro: "yea, properly lifting x86 is arguably the more dif..."]
Yes, literally

[2025-12-04 02:22] the horse: have to take risks with ciscs

[2025-12-04 02:28] daax: [replying to toro: "Why do they write their own weaker VMs when things..."]
Same reasoning so many places want to write their own anti-cheats so they dump millions into staffing and development just to get dumped on, or they have to beg Riot and some other more established groups for ideas/help: because they’re living on a prayer.

[2025-12-04 02:29] koyz: [replying to daax: "Same reasoning so many places want to write their ..."]
I doubt BE is even living on prayers <:Kek:1414590414123696308>

[2025-12-04 02:29] toro: Meh, there's certainly business choices behind anti-cheats I'd guess. Everybody can't be or use Riots tech, but everyone needs to protect their games/players

[2025-12-04 02:30] phage: [replying to toro: "Meh, there's certainly business choices behind ant..."]
Well I mean they could

[2025-12-04 02:30] phage: David will take money from anyone

[2025-12-04 02:30] toro: Monopoly and all that jazz.

[2025-12-04 02:30] phage: They just don’t want to pay

[2025-12-04 02:30] toro: IP protection

[2025-12-04 02:30] toro: etc, etc

[2025-12-04 02:31] daax: [replying to contificate: "I don't understand why people don't do novel VMs"]
Same reason you see the same shit in different flavors from any ass crack of the industry

[2025-12-04 02:31] daax: The world is full of morons

[2025-12-04 02:31] contificate: I'll give one a go some day

[2025-12-04 02:31] contificate: got some nonsense ideas

[2025-12-04 02:31] toasts: [replying to phage: "David will take money from anyone"]
Theia? or different product

[2025-12-04 02:31] phage: [replying to contificate: "I'll give one a go some day"]
Soon™

[2025-12-04 02:31] daax: [replying to koyz: "I doubt BE is even living on prayers <:Kek:1414590..."]
we both know they betta start <:Kappa:794707301436358686>

[2025-12-04 02:32] phage: [replying to toasts: "Theia? or different product"]
Griffin is their b2b obfuscation, theia is their anti tamper

[2025-12-04 02:32] toasts: ah my bad

[2025-12-04 02:32] daax: [replying to toro: "Meh, there's certainly business choices behind ant..."]
There are other middlewares that work more effectively than these in-house ACs lol

[2025-12-04 02:32] daax: And one of them even offers a free tier!

[2025-12-04 02:33] toro: inb4 its a China product XD

[2025-12-04 02:33] koyz: Easy Anti China <:Kappa:807349187350888499>

[2025-12-04 02:34] phage: EOS is probably just a tad better than what these in house protections are

[2025-12-04 02:34] phage: And EOS is literally a glorified handle stripper

[2025-12-04 02:40] daax: [replying to toro: "Meh, there's certainly business choices behind ant..."]
“business decisions” <:lmao3d:611917482105765918> the asymmetry b/w tooling and the capabilities of the in-house ac leaves them getting obliterated and costing them millions until they mature. as we know, the “novel” (because it’s new and original, but not good) packing/obfuscation gets undone in a matter of days. + knowledge of the game hacking scene compounds and is really against in-house groups. it’s easy to tailor existing tools to work against a different AC without all the bells and whistles in the mature middleware products.

also, talent market for ac development is very thin, and as ricochet has shown — you can have billions but still hire red team fools that think occupying debug registers in a loop is an IMPENETRABLE STRATEGY!

[2025-12-04 02:41] phage: Leaking the methods™

[2025-12-04 02:41] koyz: another banger, lets spam syscalls so much that we degrade performance, but we could in theory detect a hypervisor!

[2025-12-04 02:41] daax: NIH syndrome hits all of them and then they have politically expensive appeals to avoid internally

[2025-12-04 02:41] daax: [replying to koyz: "another banger, lets spam syscalls so much that we..."]
WHAT IF WE CHECKED FOR DOUBLE WRITES TO A BUFFER??

[2025-12-04 02:42] daax: > groundbreaking!

[2025-12-04 02:42] daax: syscall hook detected

[2025-12-04 02:42] koyz: [replying to daax: "WHAT IF WE CHECKED FOR DOUBLE WRITES TO A BUFFER??"]
YOU ARE A FUCKING GENIUS PUSH TO PROD

[2025-12-04 02:42] phage: There’s something to be said too about the engineers being boxed hard in what they can and cannot iterate on

[2025-12-04 02:42] daax: [replying to phage: "There’s something to be said too about the enginee..."]
skill issue and management issue. if youre micromanaging the experts your product will fail. PMs are not technical experts.

[2025-12-04 02:42] phage: I know from a few anticheat teams being limited very hard, causing turnover and churn

[2025-12-04 02:42] phage: [replying to daax: "skill issue and management issue. if youre microma..."]
Oh I agree wholeheartedly

[2025-12-04 02:43] daax: [replying to phage: "I know from a few anticheat teams being limited ve..."]
sucks to suck. keep leaving until management learns they aren’t the all knowing best decision makers (ah wait: they won’t)

[2025-12-04 02:44] phage: [replying to daax: "sucks to suck. keep leaving until management learn..."]
This is why I’m so appreciative about where am at now

[2025-12-04 02:44] phage: I’ve never had a company go “Yeah let’s explore this random rabbit hole you discovered that may or may not pan out”

[2025-12-04 02:44] phage: They’ve more than earned my loyalty

[2025-12-04 02:44] koyz: [replying to phage: "I’ve never had a company go “Yeah let’s explore th..."]
sir, that is called R&D

[2025-12-04 02:45] toasts: ricochet was likely pushed as a mvp just so activision could say they entered the scary spooky kernel space

[2025-12-04 02:45] toro: Amazon has an AC team? yikes

[2025-12-04 02:45] phage: Yes

[2025-12-04 02:45] daax: [replying to phage: "This is why I’m so appreciative about where am at ..."]
yeah man, when you find a good place, hold on to it. Epic and a few other places would be great to be (for the ac teams).

[2025-12-04 02:45] phage: Trail of Bits for me

[2025-12-04 02:45] daax: [replying to phage: "I’ve never had a company go “Yeah let’s explore th..."]
this is r&d tho

[2025-12-04 02:46] koyz: hey thats what I said

[2025-12-04 02:46] daax: that’s what a number of us do <:Kappa:794707301436358686>

[2025-12-04 02:46] daax: [replying to koyz: "hey thats what I said"]
ye

[2025-12-04 02:46] toro: From my history in the AV industry, R&D has always been an expense liine item for companies..Its hard to find reputable companies that allow it freely without constraints

[2025-12-04 02:46] phage: [replying to daax: "this is r&d tho"]
Even the security assurance teams get leeway on this

[2025-12-04 02:46] phage: But tbf, the entire company is a lot more geared towards r&d than most

[2025-12-04 02:47] toro: [replying to phage: "Trail of Bits for me"]
yea companies like Trail of bits are paid to R&D, but there is alot of pressure there to produce results. This is why the bar to entry is high to get hired there

[2025-12-04 02:47] daax: [replying to toasts: "ricochet was likely pushed as a mvp just so activi..."]
so would you say, 2 years later, it’s still mvp?

[2025-12-04 02:47] daax: I can tell you verifiably: it is still hilariously underdeveloped, yet they glaze it in their PR pieces.

[2025-12-04 02:48] phage: [replying to toro: "yea companies like Trail of bits are paid to R&D, ..."]
WELL

[2025-12-04 02:48] daax: [replying to toro: "yea companies like Trail of bits are paid to R&D, ..."]
DARPA doesn’t even understand how to leverage a lot of the research they pay for

[2025-12-04 02:49] phage: That last part not exactly

[2025-12-04 02:49] daax: so … really… no

[2025-12-04 02:49] phage: [replying to daax: "DARPA doesn’t even understand how to leverage a lo..."]
Better summary than I would make

[2025-12-04 02:49] toro: I liken DARPA to Blackberry..just hoard IP..

[2025-12-04 02:50] phage: [replying to toro: "I liken DARPA to Blackberry..just hoard IP.."]
Most of DARPA’s project go open source so

[2025-12-04 02:50] daax: [replying to phage: "Most of DARPA’s project go open source so"]
erm

[2025-12-04 02:50] toro: eh?

[2025-12-04 02:51] daax: idk what types ToB bids on but  there are a lot behind walls

[2025-12-04 02:51] phage: I probably can’t speak publicly about that but

[2025-12-04 02:51] phage: I don’t have the same impression

[2025-12-04 02:52] phage: From what I see, most of our stuff goes full OSS/public

[2025-12-04 02:52] daax: [replying to phage: "I probably can’t speak publicly about that but"]
It’s no surprise r&d companies bid on other types.

[2025-12-04 02:52] daax: [replying to phage: "From what I see, most of our stuff goes full OSS/p..."]
We seem to have opposite experiences

[2025-12-04 02:52] daax: even if it should be OSS

[2025-12-04 02:52] phage: Yeah, probably. I mean there’s probably partial truth in both of our experiences

[2025-12-04 02:53] phage: ToB doesn’t deal in things that usually end up heavily classified/under restriction

[2025-12-04 02:53] daax: Yeah, they definitely put stuff out, but my god they just throw money at all kinds of things and then are like

[2025-12-04 02:53] daax: https://tenor.com/view/brianregan-duh-make-face-funny-face-gif-13116114

[2025-12-04 02:53] phage: Haha

[2025-12-04 02:54] phage: Yeah I wouldn’t be surprised if they’ve forgotten about like 80% of the projects they’ve funded

[2025-12-04 02:54] phage: And at this point are just reinventing the wheel rather than re-funding the same project to be more complete

[2025-12-04 02:55] daax: LMAO

[2025-12-04 02:55] toro: lol

[2025-12-04 02:55] toro: seent it

[2025-12-04 02:55] daax: i saw that <@378353062617808896>

[2025-12-04 02:55] daax: remill cannot save us <:whyy:820544448798392330>

[2025-12-04 02:56] phage: Haha

[2025-12-04 02:56] phage: I’m fine to work on something new & better™

[2025-12-04 02:56] phage: But I am obsessed with building an accurate, open source non-dogshit x86 lifter

[2025-12-04 02:57] phage: And by God I will either have a heart attack or complete my goal

[2025-12-04 02:57] toro: https://tenor.com/view/bullish-bullish-af-buy-buy-moar-crypto-gif-4023638677059070706

[2025-12-04 02:57] phage: [replying to toro: "https://tenor.com/view/bullish-bullish-af-buy-buy-..."]
I am, don’t worry

[2025-12-04 02:57] daax: [replying to phage: "And by God I will either have a heart attack or co..."]
make sure to not be 🦐 sitting at the pc all day

[2025-12-04 02:57] toro: not the heart attack.

[2025-12-04 02:57] phage: Toiling in silence

[2025-12-04 02:57] phage: [replying to daax: "make sure to not be 🦐 sitting at the pc all day"]
I lift so it offsets the terrible habits

[2025-12-04 02:57] daax: [replying to phage: "I lift so it offsets the terrible habits"]
see that’s what i thought

[2025-12-04 02:58] daax: spoiler: it does not <:lmao3d:611917482105765918> but I certainly hope you are one of the lucky ones

[2025-12-04 02:58] phage: [replying to daax: "spoiler: it does not <:lmao3d:611917482105765918> ..."]
Haha I do my cardio too

[2025-12-04 02:58] phage: We ❤️ the bike

[2025-12-04 02:58] daax: [replying to phage: "Haha I do my cardio too"]
same. 4-6 miles per day

[2025-12-04 02:59] daax: but <@229977096078753793> just got done listening to me vent about a trapped nerve because of a degenerated disc

[2025-12-04 02:59] phage: [replying to daax: "same. 4-6 miles per day"]
Btw are you going to re//verse?

[2025-12-04 02:59] daax: TAKE CARE OF YOUR POSTURE BOYS

[2025-12-04 02:59] daax: [replying to phage: "Btw are you going to re//verse?"]
considering it

[2025-12-04 02:59] koyz: [replying to daax: "but <@229977096078753793> just got done listening ..."]
I put it on vinyl to continue listening to your rant

[2025-12-04 02:59] phage: [replying to daax: "considering it"]
I’ll be there, I’d be ecstatic to meet

[2025-12-04 03:00] toro: I feel like I've crossed paths with <@609487237331288074> some time ago..In Bruce Dangs rootkit course..

[2025-12-04 03:01] daax: [replying to phage: "I’ll be there, I’d be ecstatic to meet"]
sure, I’ll let you know if I wind up confirming the trip

[2025-12-04 03:01] daax: [replying to toro: "I feel like I've crossed paths with <@609487237331..."]
Bruce is the man, but I was not in his course haha — great guy though.

[2025-12-04 03:01] toro: yea, he's dope.

[2025-12-04 03:02] daax: [replying to phage: "But I am obsessed with building an accurate, open ..."]
how is this going so far? Duncan mentioned you were working on some interesting stuff, I’m assuming this is one of the items?

[2025-12-04 03:03] phage: [replying to daax: "how is this going so far? Duncan mentioned you wer..."]
Yes actually

[2025-12-04 03:05] toasts: [replying to daax: "I can tell you verifiably: it is still hilariously..."]
I believe it, maybe not mvp anymore compared to 2021 release, but more something that exists just for marketing push (similar to the recent secure boot + tpm requirements)

[2025-12-04 03:10] toasts: if you have any recent versions of the randgrid driver or whatever they call it and are willing to share that would be awesome <:steamhappy:1282176119298326538>

[2025-12-04 03:13] toro: Just download the game and get it

[2025-12-04 03:13] toro: warzone is free.

[2025-12-04 03:27] iPower: it's a complete joke, not even worth looking

[2025-12-04 03:27] iPower: obfuscation is ass, many shitty detections

[2025-12-04 03:39] AWAY FOR THE WEEK: In your opinion which is the best anti-cheat to date? 👀

[2025-12-04 07:55] lukiuzzz: [replying to AWAY FOR THE WEEK: "In your opinion which is the best anti-cheat to da..."]
faceit or vgk

[2025-12-04 12:08] iris8721: chinese ace puts belt to ass from what ive heard

[2025-12-04 13:55] phage: Not even close unfortunately

[2025-12-04 13:55] phage: People just don’t actually™ do their research

[2025-12-04 14:02] twigger: https://github.com/vmsplit/silkhook
[Embed: GitHub - vmsplit/silkhook: tiny arm64 hooking library (WIP)]
tiny arm64 hooking library (WIP). Contribute to vmsplit/silkhook development by creating an account on GitHub.

[2025-12-04 14:49] daax: [replying to AWAY FOR THE WEEK: "In your opinion which is the best anti-cheat to da..."]
Middleware? EAC. Specialized? VGK.

[2025-12-04 14:50] AWAY FOR THE WEEK: [replying to daax: "Middleware? EAC. Specialized? VGK."]
Cool

[2025-12-04 16:09] impost0r: EAC-EOS (Windows) and Vanguard (Windows)
macOS anticheat seems a bit lacking thusfar from what I've seen. Both for Vanguard and EAC-EOS. Just my opinion though, since Vanguard macOS/iOS/Android is mainly antitamper and virtualization stuff from what I understand. EAC-EOS isn't much special in that regard either.

[2025-12-04 16:09] impost0r: Mobile anticheat... lol

[2025-12-04 16:11] ImagineHaxing: Mobile anticheat is crazy

[2025-12-04 18:48] blob27: [replying to daax: "TAKE CARE OF YOUR POSTURE BOYS"]
saying this as im goblin mode

[2025-12-04 19:34] Brit: [replying to phage: "Btw are you going to re//verse?"]
hexacon lads?

[2025-12-04 19:37] phage: When’s 2026?

[2025-12-04 19:38] phage: [replying to Brit: "hexacon lads?"]
Also ew Paris

[2025-12-04 19:40] Brit: this is true

[2025-12-04 19:40] Brit: but it is also very shrimple for me to get there

[2025-12-04 19:41] the horse: actually the tunnel thing between UK & EU, is it underwater or some boat stuff

[2025-12-04 19:42] Brit: the tunnel is held up by a series of submarines

[2025-12-04 19:43] Brit: nah it's just underground

[2025-12-04 19:43] the horse: [yeatDance](https://cdn.discordapp.com/emojis/953933772884152350.gif?size=48&animated=true&name=yeatDance)

[2025-12-04 19:50] phage: [replying to Brit: "but it is also very shrimple for me to get there"]
Well I’ll let you know if I ever get sent to Europe but I’m front loaded next year with US trips

[2025-12-04 19:51] Brit: sadeg

[2025-12-04 20:07] phage: It’s shrimple, just come to re//verse <:topkek:904522829616263178>

[2025-12-05 05:02] Deleted User: [replying to phage: "Haha I do my cardio too"]
hm

[2025-12-06 06:22] Leg1tSoul: [replying to impost0r: "EAC-EOS (Windows) and Vanguard (Windows)
macOS ant..."]
They mostly rely on attestation on mobile

[2025-12-06 16:26] elias: https://x.com/D0cC_csgo/status/1997103973862392004?s=20
I just got vac lived AGAIN\! Thank you @CounterStrike 
2 times vac lived in 2 months, valve is a joke… 😂

[2025-12-06 16:27] elias: state of the art AI anti cheat back at flagging streamers <:yeetFRIED:611100434144428032>

[2025-12-06 16:27] ImagineHaxing: [replying to elias: "state of the art AI anti cheat back at flagging st..."]
What if they're cheating

[2025-12-06 16:28] elias: well then it would be impressive because it would mean valve can catch cheats that faceit AC was unable to for years

[2025-12-06 18:06] Xyrem: AI anticheat are garbage

[2025-12-06 18:07] Xyrem: the ones which rely on inputs that is ^