# June 2025 - Week 3
# Messages: 59

[2025-06-10 12:39] AstroB: Hey, so I'm a windows n00b coming from other platforms and I have a question.
What exactly are the restrictions on windows driver privileges and what makes them possible? Anything that runs above a driver / kernel module?

[2025-06-10 13:37] daax: [replying to valium: "okay i dont know anything about hardware debuggers..."]
Benham has some cool tools, my only issue is the closed source nature from a seemingly arbitrary party; but it is clean.

[2025-06-10 13:38] daax: [replying to Horsie: "Seems interesting but since he has no source avail..."]
Can’t paste, no good!

[2025-06-10 13:44] Horsie: [replying to daax: "Can’t paste, no good!"]
hes trying to sell it

[2025-06-10 13:44] Horsie: and the cool ones arent public

[2025-06-10 13:44] Horsie: The windows driver provides no value for hypervisors

[2025-06-10 13:45] daax: [replying to Horsie: "The windows driver provides no value for hyperviso..."]
The world doesn’t revolve around hypervisors, even for anti-cheats.

[2025-06-10 13:45] Horsie: If he had published the smm drbugger maybe people wouldve used it for debugging their hypervisors

[2025-06-10 13:46] Horsie: [replying to daax: "The world doesn’t revolve around hypervisors, even..."]
Not on my computer but I don't remember seeing much value in this compared to the standard kernel debugger that Windows already has.

[2025-06-10 13:46] Horsie: I did skim through the doc pdf

[2025-06-10 13:47] daax: [replying to Horsie: "Not on my computer but I don't remember seeing muc..."]
It’s just an interesting tool. We don’t really need to debate it, I’m not an avid user of it — but it works and is a fun throwback before “hypervisor” permeated the brainrotten generations minds as the only tool necessary and useful.

[2025-06-10 13:48] Horsie: [replying to daax: "It’s just an interesting tool. We don’t really nee..."]
Fair. I don't mean to shit talk about the tool. Its probably better than anything I couldve thrown together in a reasonable timeframe

[2025-06-10 13:49] Horsie: That was just my 2 cents lol. I was just bummed after going through the readme that discussed the existence of a smm debugger and then seeing the release not having it

[2025-06-10 13:55] Horsie: In hindsight perhaps I was a bit too harsh. Considering the tool is older than me, it was probably around when windbg wasnt great.

[2025-06-10 14:56] Xits: [replying to AstroB: "Hey, so I'm a windows n00b coming from other platf..."]
there's patchguard and hvci(if enabled)

[2025-06-10 14:58] Xits: can1357 wrote a pretty cool post to disable patchguard with the interrupt controller. though idk if it still works

[2025-06-10 15:00] Xits: https://blog.can.ac/2024/06/28/pgc-garbage-collecting-patchguard/
[Embed: PgC: Garbage collecting Patchguard away]
<p>I have released another article about Patchguard almost 5 years ago, ByePg, which was about exception hooking in the kernel, but let’s be frank, it didn’t entirely get rid of Patchguard

[2025-06-10 15:05] Xits: kinda surprised there isnt any patchguard routine in a valid module to bypass this approach

[2025-06-10 22:17] iPower: [replying to Xits: "can1357 wrote a pretty cool post to disable patchg..."]
"disable patchguard with the interrupt controller?"

[2025-06-10 22:52] Brit: the idt also known as interrupt controller 🫠

[2025-06-11 02:23] Xits: [replying to Brit: "the idt also known as interrupt controller 🫠"]
yea I guess I thought those are the same thing

[2025-06-11 02:23] Xits: What’s the distinction?

[2025-06-11 02:28] Xits: I guess the controller is in the cpu and interrupt handler is the correct term?

[2025-06-11 02:49] koyz: [replying to Xits: "I guess the controller is in the cpu and interrupt..."]
The IDT is a lookup table mapping interrupt vectors to their Interrupt Service Routine

[2025-06-11 02:51] Humza: [replying to Xits: "I guess the controller is in the cpu and interrupt..."]
Look into apic + lapic’s

[2025-06-11 16:23] AstroB: [replying to Xits: "kinda surprised there isnt any patchguard routine ..."]
if both run at the same privilege level, isn't it a losing fight?

[2025-06-11 16:24] AstroB: i guess HVCI actually prevents such a thing

[2025-06-11 17:30] Xits: [replying to AstroB: "i guess HVCI actually prevents such a thing"]
Yeah although I saw this awhile ago https://tandasat.github.io/blog/2023/07/05/intel-vt-rp-part-1.html#intel-vt-redirect-protection-vt-rp
[Embed: Intel VT-rp - Part 1. remapping attack and HLAT]
EPT-based security and an attack against it Bypassing KDP with the remapping attack Demo - making ci!g_CiOptions zero under KDP Intel VT Redirect Protection (VT-rp) HLAT and the remapping attack Demo 

[2025-06-11 17:33] Xits: I would assume Microsoft handles this nowadays though

[2025-06-11 17:34] Xyrem: HLAT is on by default on 24h2 (afaik)

[2025-06-11 17:40] Xits: Ah ok. Well the blog mentions amd doesn’t have an equivalent feature at least

[2025-06-11 20:08] Xyrem: wait 5-10 years, maybe they'll add it 😄

[2025-06-12 19:58] Loading: any interesting blogposts about warden ? cant find anything recent, there was a guy like 1-2 years ago who made blog post about warden and blizzard noticed it and made him take it down lol

[2025-06-13 01:34] space: [replying to Loading: "any interesting blogposts about warden ? cant find..."]
What is warden?

[2025-06-13 02:22] daax: [replying to space: "What is warden?"]
WoW’s anticheat

[2025-06-13 02:22] daax: [replying to Loading: "any interesting blogposts about warden ? cant find..."]
you check internet archive?

[2025-06-13 02:22] daax: might be there

[2025-06-13 02:27] daax: <@1042580136303796236> https://topic.alibabacloud.com/a/font-classtopic-s-color00c1deblizzardfont-and-hacker-war-5-incomplete-technical-analysis-of-warden_8_8_32117217.html
[Embed: Blizzard and hacker war 5: incomplete technical analysis of Warden]
The previous article mentioned the basic working principles of warden. This article describes how warden works in terms of its implementation. The first thing to note is that I didn't spend a lot of e

[2025-06-13 02:30] daax: https://whileydave.com/publications/Yang20_MSc.pdf

[2025-06-13 02:30] daax: this has some analysis too — page 16 where it starts

[2025-06-13 02:40] daax: https://www.ownedcore.com/forums/world-of-warcraft/world-of-warcraft-bots-programs/wow-memory-editing/1030859-warden-private-server.html

[2025-06-13 02:41] daax: something to start from i suppose

[2025-06-13 14:31] Loading: thanks

[2025-06-13 14:33] pinefin: [replying to daax: "WoW’s anticheat"]
the one that pirate software said he worked on? or is that something else he claimed to do

[2025-06-13 14:33] pinefin: -# apologies if this type of conversation/question isnt allowed, but im pretty sure this is the same?

[2025-06-13 14:33] Loading: yes its blizzards anti cheat used for all of their games

[2025-06-13 14:52] dwordxyz: [replying to Loading: "yes its blizzards anti cheat used for all of their..."]
Anything specific you're interested in?

[2025-06-13 14:52] dwordxyz: or just overall Warden information

[2025-06-13 14:56] dwordxyz: [replying to pinefin: "the one that pirate software said he worked on? or..."]
Yes

[2025-06-13 14:57] dwordxyz: https://cdn.discordapp.com/attachments/1160039232388202556/1285705059707125850/speechmemified_pasteimage_5.gif

[2025-06-13 23:42] Loading: [replying to dwordxyz: "or just overall Warden information"]
yes just overall warden info, i would like to try reverse my first anti cheat, thinking about warden and fifa anti cheat

[2025-06-13 23:43] dwordxyz: Fifa? ahhh EAAC?

[2025-06-15 16:49] Eriktion: The Lion just steals Valorants shadow cr3 and injects itself without permission
[Attachments: image.png]

[2025-06-15 19:42] brymko: 
[Attachments: image.png]

[2025-06-15 19:46] 5pider: insane

[2025-06-15 20:07] dinero: [replying to brymko: ""]
HAHAHAHAHAH

[2025-06-15 20:07] dinero: actual banger

[2025-06-15 20:07] dinero: nvm I can’t code

[2025-06-15 20:27] snowua: The Lion is detected