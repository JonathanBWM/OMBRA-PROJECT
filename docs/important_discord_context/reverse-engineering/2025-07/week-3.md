# July 2025 - Week 3
# Channel: #reverse-engineering
# Messages: 90

[2025-07-14 08:18] Opium: [replying to daax: "hello. check out <#835648484035002378>"]
are there any good vids about reversing stuff and when u debug file what u mainly look for

[2025-07-14 08:18] Opium: ah nvm <#835648484035002378> is very good 👍

[2025-07-15 08:33] Horsie: <:mmmm:904523247205351454>

[2025-07-15 08:34] safareto: <:npc:1146025805038764093>

[2025-07-15 08:47] Allexns: [replying to safareto: "<:npc:1146025805038764093>"]
?

[2025-07-15 08:52] Lyssa: Could someone help me reverse engineer this file?
[Attachments: xdS1SyP.png]

[2025-07-15 09:15] Horsie: [replying to Lyssa: "Could someone help me reverse engineer this file?"]
I had no idea you were the massgrave/tsforge dev

[2025-07-15 09:15] Deleted User: [replying to Lyssa: "Could someone help me reverse engineer this file?"]
can you even have code past 4gb

[2025-07-15 09:16] Horsie: Love your work

[2025-07-15 09:17] safareto: [replying to Horsie: "I had no idea you were the massgrave/tsforge dev"]
oh shit

[2025-07-15 09:17] safareto: what a funny coincidence lmao

[2025-07-15 09:18] Timmy: that's a very chunky file

[2025-07-15 09:27] Lyssa: [replying to Horsie: "Love your work"]
thank you!

[2025-07-15 09:52] Brit: for sure man, I ran it and a command prompt came up for a sec, probably should start investigating there 🙂‍↕️

[2025-07-15 11:39] archie_uwu: [replying to Deleted User: "can you even have code past 4gb"]
I don't think the PE loader even lets the executable run if it's above 4GB

[2025-07-15 11:47] diversenok: It doesn't

[2025-07-15 11:48] diversenok: Any file above 4GB is automatically invalid PE

[2025-07-15 14:22] daax: [replying to Allexns: "?"]
Do not post binaries without the extension chopped off and no explanation.

[2025-07-15 17:47] Allexns: when it starts it should give windows a blue screen I need a way to remove that part

[2025-07-15 17:49] Allexns: [replying to daax: "Do not post binaries without the extension chopped..."]
I posted this because I wanted to ask if anyone could help me remove the part that causes the crash.

[2025-07-15 18:00] absceptual: [replying to Allexns: "I posted this because I wanted to ask if anyone co..."]
is it your application?

[2025-07-15 18:07] daax: [replying to Allexns: "I posted this because I wanted to ask if anyone co..."]
You didn't give any explanation about the binary, where it came from, etc. It's strange at best.

[2025-07-15 18:16] Allexns: [replying to absceptual: "is it your application?"]
It's a custom application made for me but I think the creator has closed the project so I can't use it anymore

[2025-07-15 18:20] Yoran: [replying to Allexns: "It's a custom application made for me but I think ..."]
Wdym by `I can't use it anymore`? Did the creator just give you a freestanding binary?

[2025-07-15 18:22] pinefin: if its made FOR you, and THEY closed the project, who really has the rights here?

[2025-07-15 18:22] pinefin: learn to set binding contracts

[2025-07-15 18:22] pinefin: this is not any of our problems here

[2025-07-15 18:26] Allexns: [replying to Yoran: "Wdym by `I can't use it anymore`? Did the creator ..."]
I mean it used to work now it makes windows blue screen

[2025-07-15 18:26] absceptual: [replying to Allexns: "I mean it used to work now it makes windows blue s..."]
what type of application is it

[2025-07-15 18:27] Lyssa: what does it do... maybe....

[2025-07-15 18:27] Lyssa: why should anyone spend time figuring out this specific exe in particular which has vmprotect on it

[2025-07-15 18:27] absceptual: [replying to Lyssa: "what does it do... maybe...."]
i sure do wonder

[2025-07-15 18:28] Yoran: [replying to Allexns: "I mean it used to work now it makes windows blue s..."]
Can you diff the older version with the newer one? Maybe is you that f`d up something?

[2025-07-15 18:28] Yoran: And if there isnt an older version, i dont know why we assume he updated something

[2025-07-15 18:31] Brit: [replying to Lyssa: "what does it do... maybe...."]
Im willing to bet without even looking at the bin that its a p2c loader

[2025-07-15 18:31] Allexns: [replying to pinefin: "if its made FOR you, and THEY closed the project, ..."]
By made for me I mean that the functions within the program are made to work only on my computer.

[2025-07-15 18:32] pinefin: ok well if you're not gonna share any information about the application itself that you want US to solve your issue, no ones gonna help you

[2025-07-15 18:33] pinefin: so stop whining and either give information or give it up

[2025-07-15 18:33] pinefin: hard work isnt cheap

[2025-07-15 18:33] Yoran: BTW for next time if you share a binary please say that it BSODed your PC

[2025-07-15 18:33] safareto: [replying to Allexns: "By made for me I mean that the functions within th..."]
it's just kind of really weird that you join the server and immediately start asking for a very sus thing lmao

[2025-07-15 18:33] pinefin: [replying to safareto: "it's just kind of really weird that you join the s..."]
its not "kind of" it IS.

[2025-07-15 18:34] safareto: [replying to pinefin: "its not "kind of" it IS."]
trying to be polite

[2025-07-15 18:34] Allexns: [replying to Yoran: "Can you diff the older version with the newer one?..."]
there is a previous version but it had some problems so the creator made another version to fix them

[2025-07-15 18:35] Yoran: [replying to Allexns: "there is a previous version but it had some proble..."]
So... tell him that it crashed your PC

[2025-07-15 18:35] Yoran: 0.6 its a scam of some kind 0.3 its a troll 0.1 benefit of the doubt

[2025-07-15 18:36] Lyssa: [replying to Allexns: "there is a previous version but it had some proble..."]
lol wat

[2025-07-15 18:36] pinefin: <:topkek:904522829616263178>

[2025-07-15 18:36] Lyssa: oh sorry it was supposed to crash your pc pls download this update

[2025-07-15 18:36] Lyssa: very important bugfix

[2025-07-15 18:38] Lyssa: [replying to Brit: "Im willing to bet without even looking at the bin ..."]
this is probably it

[2025-07-15 18:41] absceptual: [replying to Lyssa: "this is probably it"]
"custom application" and "creator has closed the project" gave it away

[2025-07-15 18:41] absceptual: latter probably means my key expired or exit scam 💔

[2025-07-15 19:30] pinefin: i mean its another case of "help me with this"......."and then i never contribute to this server again"

[2025-07-15 19:30] pinefin: either that or the exe installs a rat

[2025-07-15 19:31] pinefin: but once again, same dilemma

[2025-07-16 04:55] nexohk1337: where can I get thermida or vmp that is safe

[2025-07-16 05:08] the horse: Through their official stores 👍

[2025-07-16 05:32] absceptual: [replying to the horse: "Through their official stores 👍"]
next you're gonna tell me you b-bought ida pro 😰😰😰

[2025-07-16 05:33] the horse: [replying to absceptual: "next you're gonna tell me you b-bought ida pro 😰😰😰"]
well he asked where to get it safe

[2025-07-16 05:33] the horse: ¯\_(ツ)_/¯

[2025-07-16 05:33] the horse: i bought binja does that count

[2025-07-16 05:33] the horse: it expired 💔
[Attachments: image.png]

[2025-07-16 05:33] absceptual: when i get rich ill buy ida pro

[2025-07-16 05:34] absceptual: actually i really only use the x86-64 decomp so ida home would be enough

[2025-07-16 05:35] the horse: speaking of hex-rays, seems like they refreshed their site again

[2025-07-16 05:35] the horse: and a 9.2 teaser 😍

[2025-07-16 05:35] the horse: https://hex-rays.com/blog/unlocking-risc-v-and-arm-next-level-switch-detection
[Embed: Unlocking RISC-V and ARM: Next-Level Switch Detection in IDA Pro]
Discover how Hex-Rays enhances switch detection in IDA Pro 9.2 for RISC-V and ARM, making decompilation faster, cleaner, and more accurate.

[2025-07-17 12:07] Matti: [replying to absceptual: "actually i really only use the x86-64 decomp so id..."]
pro tip: the paid version now ships with all decompiler DLLs included no matter what

[2025-07-17 12:07] Matti: of course your license won't let you use the others, but I mean... people have been known to crack IDA in the past

[2025-07-17 13:29] Pepsi: does somebody here know what `FSCTL_QUERY_VOLUME_CONTAINER_STATE` `0x90390` (filesystem driver ioctl code) is supposed to do?

[2025-07-17 13:34] Pepsi: it indeed is, because the documentation you linked is for a different ioctl code

[2025-07-17 13:34] Pepsi: if it was documented, i wouldn't ask here

[2025-07-17 13:36] Pepsi: well its an ioctl to be send to filesystem drivers

[2025-07-17 13:37] Pepsi: ntfs.sys answers with `STATUS_INVALID_DEVICE_REQUEST`, so i have been wondering what its used for

[2025-07-17 13:52] Pepsi: tell me something new

[2025-07-17 13:52] Pepsi: you could have just left this to somebody who might know

[2025-07-17 13:53] Pepsi: [replying to Pepsi: "does somebody here know what `FSCTL_QUERY_VOLUME_C..."]
so if somebody has any clue, let me know :)

[2025-07-17 22:39] diversenok: How is it related?

[2025-07-17 22:41] Pepsi: he was trying to imply im too stupid to use a search engine, because he knows me from educational gamemodding server

[2025-07-17 22:41] diversenok: [replying to Pepsi: "does somebody here know what `FSCTL_QUERY_VOLUME_C..."]
It's an FSCTL that determines whether the volume is subject to WCI virtualization (used in containers)

[2025-07-17 22:44] diversenok: [replying to diversenok: "It's an FSCTL that determines whether the volume i..."]
Handled by `wcifs.sys` in `WcFsctlQueryContainerState`

[2025-07-17 22:44] diversenok: Returns `(BOOL)TRUE` when (what I assume means) WCI layering is enabled

[2025-07-17 22:45] Pepsi: [replying to diversenok: "It's an FSCTL that determines whether the volume i..."]
thanks that was very helpful

[2025-07-17 22:53] diversenok: The output type is `CONTAINER_VOLUME_STATE` (defined in ntifs.h)

[2025-07-18 18:33] elias: Did anyone already get to see the new IDA UI in 9.2?

[2025-07-18 20:20] snowua: Is there new UI?

[2025-07-18 20:25] elias: according to their blog 9.2 received a modernized UI

[2025-07-18 20:27] snowua: i’ll take a peek at the beta right now 👀

[2025-07-18 20:29] f00d: [replying to elias: "according to their blog 9.2 received a modernized ..."]
qt version switch <:mmmm:904523247205351454>