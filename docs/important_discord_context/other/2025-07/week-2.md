# July 2025 - Week 2
# Messages: 119

[2025-07-09 15:05] Lyssa: and?

[2025-07-09 15:05] Lyssa: add like, "i make cheats and things, i am so very cool for doing that"

[2025-07-09 16:17] Lyssa: he gone poofed 🤯

[2025-07-09 17:06] magicbyte: I am a bit confused

[2025-07-09 17:08] magicbyte: if you can somehow get code into DRAM that can be executed by the CPU upon a request to do so, or in some other ways, be this a driver or shellcode, what's stopping you from modifying an anti-cheat's driver's code? you could nop their detections. heartbeats? if you achieve faking the network packets, what else is left to stop you from modifying the driver in memory to avoid detections?

[2025-07-09 18:11] Timmy: if you're already into the position where you're executing code, other than knowing where what is there's nothing stopping you.

[2025-07-09 18:12] Timmy: other than that the um side is expecting certain results from the driver module I presume

[2025-07-09 18:53] magicbyte: right, so I don't understand why people's goals are to bypass detections and find methods that don't trigger them, rather than having the goal to disable them

[2025-07-09 18:53] magicbyte: and not worry about constantly having to evade different kinds of measures

[2025-07-09 19:16] the horse: heavily obfuscated anti-cheat drivers and their integrity checks

[2025-07-09 19:17] the horse: it's not as straightforward as having code being executed

[2025-07-09 19:17] the horse: that's the easy part nowadays

[2025-07-09 19:17] the horse: so yes, while you can find all of them, patch them, you still need to discover what data is expected to be sent to the usermode, and replicate that

[2025-07-09 19:18] the horse: which again, is a problem because the code is heavily mutated

[2025-07-09 19:18] the horse: it might not be such a problem with some new, simple anti-cheats that just monitor & strip handles for example

[2025-07-09 19:19] the horse: but for any larger commercial solution this quickly falls apart; especially if they have a frequent update cycle

[2025-07-09 19:19] the horse: the prerequisite work for making this somewhat automatic is extremely extensive, time-consuming and prone to failure

[2025-07-09 19:20] the horse: and at the point that you may seem confident that you can do all of this, you realize that it's going to be easier longterm to just completely emulate that component

[2025-07-09 19:21] the horse: and since anti-cheats nowadays consistent of multiple components (kernel driver, usermode mapped module, game sdk, additional modules monitoring processes like lsass, ...) that's quite a lot of things to take care of

[2025-07-09 19:26] phage: [replying to magicbyte: "right, so I don't understand why people's goals ar..."]
wait til anti-cheats start emulating DRM solutions with "stolen byte" segments spammed everywhere

[2025-07-09 20:01] magicbyte: [replying to the horse: "and since anti-cheats nowadays consistent of multi..."]
yeah, as always, these things probably sound *way way way* simpler in theory compared to execution

[2025-07-09 20:02] the horse: hell this sounds even more complex in theory

[2025-07-09 20:02] the horse: 😄

[2025-07-09 20:02] magicbyte: I presume reversing the code and coming up with new ways of slipping under them is a faster and easier process than trying replicating heartbeats and required data

[2025-07-09 20:03] magicbyte: and obfuscation

[2025-07-09 20:03] the horse: I think even deobfuscation so you have something to statically reverse is extremely painful

[2025-07-09 20:03] the horse: most people I know just give up on that entirely and do dynamic analysis instead

[2025-07-09 20:04] magicbyte: yeah I mean, the way I see it is you always have to combine static and dynamic

[2025-07-09 20:04] magicbyte: for the fastest results

[2025-07-09 20:06] magicbyte: but yeah, it still surprises me how I never ever see people discussing the option to disable detections rather than to evade them

[2025-07-09 20:06] magicbyte: this might have become a common practice even though it may not be the best choice in some cases

[2025-07-09 20:08] the horse: because evading is easy compared to disabling

[2025-07-09 20:08] the horse: usually..

[2025-07-09 20:24] Xits: [replying to magicbyte: "but yeah, it still surprises me how I never ever s..."]
https://secret.club/2020/07/06/bottleye.html
[Embed: BattlEye client emulation]
The popular anti-cheat BattlEye is widely used by modern online games such as Escape from Tarkov and is considered an industry standard anti-cheat by many. In this article I will demonstrate a method 

[2025-07-09 20:25] Xits: it wasnt that hard to emulate but figuring it out probably took 100s of hours of reversing

[2025-07-09 20:25] Xits: and battleeye is not a very good AC

[2025-07-09 20:30] Brit: [replying to phage: "wait til anti-cheats start emulating DRM solutions..."]
its insane that they dont already, why should the game be in a runnable state without the ac running. Im not convinced that stolen constants is the most elegant solution but having a strong coupling between the ac running and normal cflow of the protected game would be pretty cute.

[2025-07-09 20:32] phage: [replying to magicbyte: "but yeah, it still surprises me how I never ever s..."]
Anyone that is running around with true AC emulation wouldn't be posting about it publicly, considering the financial implications

[2025-07-09 20:32] phage: [replying to Brit: "its insane that they dont already, why should the ..."]
Agreed, it would be more cancerous than it already is

[2025-07-09 20:38] daax: [replying to magicbyte: "right, so I don't understand why people's goals ar..."]
I encourage you to take a look at EAC, assuming no prior knowledge, and disable all detections and emulate their network comms and validation -- attempting this will probably answer this question most effectively. People can argue all the caveats, but locating, deobfuscating and then emulating that is not a trivial task.

[2025-07-09 20:38] daax: And anyone who says it's trivial or not that time consuming is lying / posturing and has never attempted | done it.

[2025-07-09 20:40] Xits: [replying to phage: "wait til anti-cheats start emulating DRM solutions..."]
What’s a stolen byte segment?

[2025-07-09 20:41] daax: [replying to Xits: "What’s a stolen byte segment?"]
simplest sense is that the instructions or parts of an instruction/constant are not present until specific conditions have been met

[2025-07-09 20:42] daax: it's meant to break execution or deviate control flow if there is some anomaly in what is considered "nominal execution"

[2025-07-09 20:43] daax: lots of products use drm that implements this technique to prevent easy keygen'ing and emulation of whatever component they don't want touched

[2025-07-09 20:43] phage: [replying to Xits: "What’s a stolen byte segment?"]
remove 5-6 instructions, replace with an interrupt that saves cpu context, performs a network request, instructions ran on server, server send back new context (usual implementation), continue execution

[2025-07-09 20:43] daax: adobe does it, binja does it, etc

[2025-07-09 20:44] daax: [replying to phage: "remove 5-6 instructions, replace with an interrupt..."]
doesn't have to be this either, this is one way of doing it

[2025-07-09 20:44] phage: That's why I added the qualifier in parenthesis

[2025-07-09 20:44] daax: [replying to phage: "That's why I added the qualifier in parenthesis"]
I wasn't correcting you - I was adding to it for the guy who asked

[2025-07-09 20:44] Brit: [replying to phage: "remove 5-6 instructions, replace with an interrupt..."]
stalling on netw for each stolen constant in insane

[2025-07-09 20:44] Brit: imagine

[2025-07-09 20:44] Brit: <:topkek:904522829616263178>

[2025-07-09 20:44] phage: I mean

[2025-07-09 20:45] phage: Wasn't that hogwarts whole schtick?

[2025-07-09 20:45] Brit: you just get decr keys once

[2025-07-09 20:45] Brit: and then unfuck the cosntants from the license file // segm or whatever

[2025-07-09 20:45] Brit: or whatever the nomenclature was

[2025-07-09 20:45] phage: surprised EAC still uses VMP 3.5 on windows usermode

[2025-07-09 20:47] the horse: because that component is not important

[2025-07-09 20:47] the horse: it's just a downloader & driver set up stub, metadata parser

[2025-07-09 20:47] the horse: the actual module that's meant to provide the anti-cheat is the kernel & the manually mapped image

[2025-07-09 20:47] phage: Uhhhhhh

[2025-07-09 20:47] the horse: the EOS sdk is for games

[2025-07-09 20:47] the horse: doesn't count

[2025-07-09 20:47] the horse: just a middlelayer

[2025-07-09 20:47] phage: The manually mapped portion still uses VMP 3.5

[2025-07-09 20:48] the horse: on what game?

[2025-07-09 20:48] phage: All

[2025-07-09 20:48] phage: It's VMP 3.5 in the "classic" VM style with a jump table

[2025-07-09 20:48] phage: it's a hidden option for 3.0-3.6, added back to UI in 3.7+

[2025-07-09 20:49] the horse: oh yeah you're right

[2025-07-09 20:50] the horse: I thought they switched to their stuff like the driver

[2025-07-09 20:50] the horse: mb 🙂

[2025-07-09 20:50] phage: The driver utilizes some custom meme

[2025-07-09 20:50] phage: but the UM portion on windows is still VMP 3.5

[2025-07-09 20:51] phage: IIRC MacOS uses 3.8 ATM and Linux uses themida

[2025-07-09 20:51] phage: for their usermode components

[2025-07-09 20:51] the horse: didn't macos also use themida?

[2025-07-09 20:51] phage: I may have the two switched up

[2025-07-09 20:52] phage: but one used a newer version of VMP, or at least a version using IP relative handlers instead of a jump table

[2025-07-09 20:55] Brit: I'm going to need you to explain what you meant by this and if indeed it was  a meme, you're gonna catch a week long timeout. 😃

[2025-07-09 20:57] daax: [replying to Xits: "What’s a stolen byte segment?"]
you can read some similar ideas via:
<https://www.usenix.org/system/files/conference/osdi12/osdi12-final-11.pdf>
<https://sci-hub.ru/https://doi.org/10.1145/2741948.2741977>

these aren't specific to DRM but you can apply the same concepts.

forgot to add this one:
<https://sites.cs.ucsb.edu/~chris/research/doc/usenix13_moviestealer.pdf>

also patents sometimes hint at things if you look for them from drm companies:
<https://patents.google.com/patent/US20160085946A1/en>
<https://patents.google.com/patent/US7188241B2/en>

> A server-side method aspect includes receiving, from the client, one of the substantially unique hardware information and the information derived therefrom, transforming the at least one nominal constant using one of the hardware information and the information derived therefrom, and transmitting, to the client, the at least one transformed constant.

[2025-07-09 21:02] Xits: Thanks!

[2025-07-09 21:08] daax: [replying to Xits: "Thanks!"]
added some others for information but np

[2025-07-09 21:13] daax: [replying to Xits: "Thanks!"]
from the second patent url. using the classification of the patent (like `(G06F21/125)`) will give you a lot of fun things to look into
[Attachments: image.png]

[2025-07-09 23:05] DirtySecreT: [replying to daax: "you can read some similar ideas via:
<https://www...."]
thanks for sharing this!

[2025-07-10 00:38] Deleted User: [replying to daax: "from the second patent url. using the classificati..."]
was this patent made by valve lol

[2025-07-10 01:37] selfprxvoked: [replying to daax: "you can read some similar ideas via:
<https://www...."]
you're great! <:give:835809674480582656>

[2025-07-10 02:23] absceptual: [replying to daax: "from the second patent url. using the classificati..."]
very interesting.. i've wondered if such a method could be used for loaders a long time ago but i didn't know it has actually been done in practice!

[2025-07-10 03:31] daax: [replying to absceptual: "very interesting.. i've wondered if such a method ..."]
it certainly can be and has been done by people in allllll sorts of different industries to protect their assets. lots of fun things to learn from patents. it’s rare to find a new and 100% novel method to something. like a 1983 patent had* the idea for encrypted blobs getting decrypted by a stub and inserting interrupts to validate parts of the newly decrypted code block (not very novel to us now, back then it wasn’t common) — kinda funny that was 40+ years ago … and that patent cited others that were a decade prior that were in a less advanced form.

[2025-07-10 03:32] daax: [replying to Deleted User: "was this patent made by valve lol"]
no, the owners are in the link: pace anti piracy inc

[2025-07-10 03:32] absceptual: [replying to daax: "it certainly can be and has been done by people in..."]
wont lie ive never in my life considered reading patents for fun til today

[2025-07-10 03:32] absceptual: gotta step outside ny comfort zone it seems

[2025-07-10 03:38] daax: [replying to absceptual: "wont lie ive never in my life considered reading p..."]
oh yeah. (G06F21/125) in googles patent search, one of many, will give you lots of things to inspire some kind of idea for a derivative method to something like these

[2025-07-10 06:29] magicbyte: [replying to Xits: "https://secret.club/2020/07/06/bottleye.html"]
this is neat, exactly what I meant by packet manipulation

[2025-07-10 06:30] magicbyte: [replying to phage: "Anyone that is running around with true AC emulati..."]
makes sense

[2025-07-10 06:32] magicbyte: [replying to daax: "And anyone who says it's trivial or not that time ..."]
oh yeah, for sure I do agree with the fact that achieving something like this takes insane amounts of skill and time, that's why I said this must sound easier in theory than actually achieving it

[2025-07-10 06:33] magicbyte: but I was still wondering why people avoid this approach, money would explain it

[2025-07-10 06:36] magicbyte: but that secret club article is exactly what I meant thus shows that there indeed are people who chose this approach and even succeed

[2025-07-11 00:14] absceptual: who is that

[2025-07-11 00:26] iPower: this isnt a channel for shitposting

[2025-07-11 00:26] iPower: do it again and im timing you out

[2025-07-11 00:29] brymko: oh shoot mod abuse

[2025-07-11 01:21] varaa: oh shoot mod abuse

[2025-07-11 03:29] Deleted User: does anyone here have successfully debug gepard (user mode anti cheat) in user mode debugger only

[2025-07-11 09:49] iPower: [replying to Deleted User: "does anyone here have successfully debug gepard (u..."]
you mean the ragnarok one? hooking the wow64 transition should do it iirc

[2025-07-11 09:50] iPower: and KiUserExceptionDispatcher for other memes

[2025-07-11 09:52] iPower: i dont remember exactly which syscalls you need to hook for gepard but if you follow those "anti-debugging checklists" you will be able to make a bypass easily

[2025-07-12 22:21] absceptual: does anyone have any general information about the internal anti-cheat used by epic in fortnite? i'm trying to create a dump of the game and i've heard that they have their own in-house anticheat

[2025-07-13 00:46] junkjunk: [replying to absceptual: "does anyone have any general information about the..."]
+ easy anticheat

[2025-07-13 00:47] absceptual: [replying to junkjunk: "+ easy anticheat"]
have you reversed it?

[2025-07-13 00:47] junkjunk: [replying to absceptual: "have you reversed it?"]
hell no im noob

[2025-07-13 00:48] absceptual: how is it an easy anticheat then?

[2025-07-13 00:48] junkjunk: [replying to absceptual: "how is it an easy anticheat then?"]
They using easy anticheat and own anticheat for serverside as i know

[2025-07-13 00:48] absceptual: oh i thought you meant it's an easy anticheat (to reverse)

[2025-07-13 09:35] ngildea: I guess "easy to use but difficult to defeat anticheat" is a bit of a mouthful

[2025-07-13 10:15] lava_hound: wow6432node

[2025-07-13 18:55] kantana: [replying to absceptual: "does anyone have any general information about the..."]
You can load into a pm, with the ac disabled then dump the game.