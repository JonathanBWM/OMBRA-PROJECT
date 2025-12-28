# November 2025 - Week 4
# Messages: 46

[2025-11-17 13:16] vanessa: [replying to noahsx: "also last i checked (2mo ago) they still werent us..."]
is this from your own re or?

[2025-11-17 13:34] noahsx: [replying to vanessa: "is this from your own re or?"]
yeah placed hooks on KeRegisterNmiCallback, never got hit

[2025-11-17 13:34] noahsx: at least not from battleye

[2025-11-17 20:37] Xyrem: BE only has stack walks by apcs

[2025-11-18 19:30] ₊˚⊹2cookie ⋆｡˚: [replying to plpg: "I think roblox uses that"]
roblox's ac is one of the only user mode anti cheats on windows

[2025-11-18 22:20] pinefin: only one in practice at the moment*

[2025-11-18 22:33] iris8721: overwatch's warden isnt usermode?

[2025-11-18 22:38] iPower: yes it is

[2025-11-19 09:38] lukiuzzz: [replying to ₊˚⊹2cookie ⋆｡˚: "roblox's ac is one of the only user mode anti chea..."]
Faceit on mac 😭

[2025-11-23 03:40] Jacob: Hey, I just finished my first driver based external for cs2, but I don't know where to go from here, every other ac is intimidating and I don't know where to start

[2025-11-23 04:12] iPower: [replying to Jacob: "Hey, I just finished my first driver based externa..."]
focus on learning more to create tooling to analyze such acs

[2025-11-23 04:14] iPower: learn more about windows internals. write drivers to practice bypasses against certain ac techniques

[2025-11-23 04:14] iPower: read the architecture manuals (intel/amd)

[2025-11-23 04:14] iPower: write tools for dynamic analysis (dumping drivers, hooking, debugging, etc)

[2025-11-23 04:15] iPower: learn about compilers, lifting, (de)obfuscation if you're interested in binary analysis

[2025-11-23 04:16] iPower: theres so much you can do to start analyzing acs

[2025-11-23 13:12] BloodBerry: [replying to Jacob: "Hey, I just finished my first driver based externa..."]
As I know VAC doesn’t see kernel-mode modifications

[2025-11-23 13:13] BloodBerry: I played with driver-external hack, and no ban yet… Prime status hacking :))

but be ware for dwForceJump (bunny hop) maybe flagged if changed, anyway they looking for memory modifications somewhere

[2025-11-23 13:14] BloodBerry: Try on fake account with no prime (as I know cs2 is free to play too) — to to run cheats

[2025-11-23 14:05] ImagineHaxing: I heard some people some time ago that vac is testing new server side anticheat and silently collecting data but idk

[2025-11-23 14:46] daax: [replying to ImagineHaxing: "I heard some people some time ago that vac is test..."]
They’ve been saying this since 2016, and nothing has come about

[2025-11-23 14:47] daax: I’ll believe it when I see it with clearly attributable results

[2025-11-23 14:47] ImagineHaxing: [replying to daax: "They’ve been saying this since 2016, and nothing h..."]
Soon™

[2025-11-23 14:57] elias: [replying to daax: "They’ve been saying this since 2016, and nothing h..."]
This isn‘t true anymore imo, they changed a lot since 2024 and its capable of detecting even soft aimbots and very suble stuff like recoil control nowadays

[2025-11-23 14:59] daax: [replying to elias: "This isn‘t true anymore imo, they changed a lot si..."]
I can confirm it does not detect soft aimbots or recoil control lol.

[2025-11-23 14:59] koyz: VacNet is everywhere <:PES_TinFoilHat:621616606572969995>

[2025-11-23 15:00] elias: [replying to daax: "I can confirm it does not detect soft aimbots or r..."]
people find bypasses all the time and currently its very unstable but yes, over the past months it was repeatedly able to detect such things

[2025-11-23 15:00] elias: at least you could say they started seriously working on the AI thing for the first time

[2025-11-23 15:08] daax: [replying to elias: "people find bypasses all the time and currently it..."]
I haven’t seen anything indicating it works, standard soft aim without any special implementation in premiere/competitive for a handful of people and none of them have had issues over the last 12 months. If it starts doing it consistently I’ll believe there is some kind of improvement, but atm I’m thinking it’s probably other things triggering bans.

[2025-11-23 15:13] elias: yeah we‘ll see

[2025-11-23 15:13] elias: currently they are pushing and rolling back updates all the time

[2025-11-23 15:14] elias: a few weeks ago some pros/semi pros got flagged when performing too well lol

[2025-11-23 16:40] luci4: AFAIK vac live is very effective against raging and semi-raging

[2025-11-23 16:41] Jacob: [replying to iPower: "focus on learning more to create tooling to analyz..."]
thanks! really appreciate it

[2025-11-23 16:44] Jacob: [replying to BloodBerry: "I played with driver-external hack, and no ban yet..."]
yeah I was thinking about making a jump spammer thing, but i was to lazy so I just did radar, esp, aim, and trigger

[2025-11-23 17:30] BloodBerry: [replying to Jacob: "yeah I was thinking about making a jump spammer th..."]
Use WH only stuff and VAC is cooked

[2025-11-23 17:31] BloodBerry: I mean if they put traps there no chance there to get detected cuz read operations are always safe

[2025-11-23 19:43] luci4: [replying to Jacob: "Hey, I just finished my first driver based externa..."]
I'm curious, how did you start this project? I've used cheats a lot when was a kid but I've never looked into their internals.  I assume the first steps would be understanding the game's engine?

[2025-11-23 19:46] luci4: It seems like a lot of projects use this:

https://github.com/a2x/cs2-dumper
[Embed: GitHub - a2x/cs2-dumper: Counter-Strike: 2 Offset Dumper]
Counter-Strike: 2 Offset Dumper. Contribute to a2x/cs2-dumper development by creating an account on GitHub.

[2025-11-23 20:02] Jacob: [replying to luci4: "I'm curious, how did you start this project? I've ..."]
I mean I'm not a master programmer or anything but some prerequesites were learning a good amount of C++ and just understanding basic game hacking like assault cube

[2025-11-23 20:05] Jacob: And for the driver I watched some youtube videos, read source codes on UC to make a basic ioctl driver

[2025-11-23 20:06] Jacob: then I watched more youtube videos and looked for open source projects

[2025-11-23 20:06] luci4: Ah I see, I thought the entire thing was in kernel-mode, lol

[2025-11-23 20:07] luci4: Thanks!

[2025-11-23 20:07] Jacob: no problem

[2025-11-23 20:07] Jacob: [replying to luci4: "Ah I see, I thought the entire thing was in kernel..."]
also the cs2 dumperis good for structs and offsets which youll need to actually code the cheat