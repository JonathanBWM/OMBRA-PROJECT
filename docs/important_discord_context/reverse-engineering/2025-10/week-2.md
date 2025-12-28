# October 2025 - Week 2
# Channel: #reverse-engineering
# Messages: 70

[2025-10-07 12:57] drifter987: does anyone here happen to have knowledge on iOS jailbreaking? specifically writing one for 12.5.7 for A8(x) devices? i am trying to learn and get info on what to do after i get all the stuff set up. (payloads, pongo and checkra1n kpf)

[2025-10-07 13:01] drifter987: to summarize, im looking to learn about the ramdisk stuff that checkra1n utilizes to upload their loader and so on

[2025-10-07 14:47] lukiuzzz: [replying to drifter987: "does anyone here happen to have knowledge on iOS j..."]
https://github.com/checkra1n

If you want to really understand it, you should study the checkra1n source code, specifically how they build the ramdisk and communicate with the PongoOS USB stack.
[Embed: checkra1n]
checkra1n has 9 repositories available. Follow their code on GitHub.

[2025-10-07 14:48] drifter987: checkra1n isn't open source...

[2025-10-07 14:50] drifter987: <@1148691580899840002>

[2025-10-07 14:50] drifter987: i would study it if it was

[2025-10-07 14:50] lukiuzzz: https://theiphonewiki.com/

[2025-10-07 14:50] moneytherapy: https://github.com/axi0mX/ipwndfu
[Embed: GitHub - axi0mX/ipwndfu: open-source jailbreaking tool for many iOS...]
open-source jailbreaking tool for many iOS devices - axi0mX/ipwndfu

[2025-10-07 14:51] drifter987: thank you both

[2025-10-07 14:51] drifter987: ❤️

[2025-10-07 14:51] lukiuzzz: https://github.com/palera1n/palera1n
[Embed: GitHub - palera1n/palera1n: Jailbreak for A8 through A11, T2 device...]
Jailbreak for A8 through A11, T2 devices, on iOS/iPadOS/tvOS 15.0, bridgeOS 5.0 and higher. - palera1n/palera1n

[2025-10-07 14:53] drifter987: thank you

[2025-10-07 15:48] Andrew: I have a iOS book from Ray wonderlich on reverse engineering,  I worked through it in a book club - goes over a bunch of the LLDB for ways to reverse engineer applications.    

Great book.   

Speaking of jailbreaking -  i was hoping to get elevated access without jailbreaking - similar to pegasus.   

I have the advantage of being able to install the application so I have the advantage of trying to overflow something like a side process such as the OS loading another application (where you give permissions for an app to start another application)  -  

I don’t know shit - I’m just looking for the starting point to my journey of learning

[2025-10-07 16:32] emma: [replying to Andrew: "I have a iOS book from Ray wonderlich on reverse e..."]

[Attachments: cover3.jpg]

[2025-10-07 17:20] Andrew: lol - I know I’m shooting for the moon - but I think the process of learning is never a bad thing.

[2025-10-07 18:06] pinefin: just evaluate the steps u need and you can accomplish anything

[2025-10-07 21:08] mtu: I think you’re shooting for a moon of another planet

[2025-10-07 21:09] mtu: What you described - root-like access from a deliberately user installed app - is still a 100k bug if you sell it to Apple, or probably 5x that to brokers

[2025-10-07 21:11] Andrew: [replying to mtu: "What you described - root-like access from a delib..."]
💯   -   There have been bugs before with their "auto display of pictures" (which was how pegasus used to work) and also the wifi controller -  But as far as i know those have been fixed ...

[2025-10-07 21:17] mtu: It’s not that the general idea is infeasible it’s that you’re doing the bit of “how to find zero click RCE chrome thanks”

[2025-10-07 21:17] mtu: I’d start smaller - is there some API that leaks even a partial contact name or emergency contact details without a consent prompt? That’s a bug

[2025-10-07 21:18] Andrew: im not looking for "zero click". by using an IPA - its not zero click by definition -

[2025-10-07 21:19] Andrew: and use of the private API's (are cool). but over time they dont do anything like they used to (like showing all all installed applications) -

[2025-10-07 21:21] Andrew: anyway -  i was just looking to see if anyone had some ideas on books to read, stuff like that -   Honestly, i will most likely not figure out how to do what im trying to do.

[2025-10-07 21:23] Andrew: Thanks for any knowlege 🙇‍♂️

[2025-10-07 21:23] Andrew: im going to play with my puppy

[2025-10-07 21:24] Andrew: 
[Attachments: Screenshot_2025-10-07_at_5.24.03_PM.png]

[2025-10-07 21:26] dullard: [replying to Andrew: "anyway -  i was just looking to see if anyone had ..."]
For internals this series is good if you can bear the Christmas music 😂😂 https://youtube.com/playlist?list=PLouOX_372GvtD6jglet0i99XN2egehZ9B
[Embed: macOS and iOS Security Internals Advent Calendar 2022]

[2025-10-07 21:26] dullard: It goes into incredible detail about every relevant core concept

[2025-10-07 21:27] dullard: The music is fucking awful though

[2025-10-07 21:27] Andrew: ❤️ thank you

[2025-10-07 21:27] dullard: I don’t remember what day in the series it was but he forgets to turn the music down and it blasts the whole episode 💀

[2025-10-07 21:28] Andrew: thanks for the warning !

[2025-10-08 00:52] daax: [replying to Andrew: ""]
mine says hello
[Attachments: IMG_2150.jpg]

[2025-10-08 01:46] Andrew: German Sheps are the best -  I like the "prim and propper" paw cross -

[2025-10-08 06:45] Yoran: [replying to daax: "mine says hello"]
Pretty little baby ...

[2025-10-08 16:52] pinefin: i think that will be the next type of dog i get

[2025-10-08 16:52] pinefin: my little dog needs a friend

[2025-10-08 16:57] pinefin: 
[Attachments: image.png]

[2025-10-08 16:57] pinefin: also just noticed why this in <#835635446838067210> 🤣

[2025-10-08 17:23] guar: [replying to pinefin: ""]
eyes do becompletely empty

[2025-10-08 17:24] guar: dis be me when im staring at vectorized code

[2025-10-08 17:24] guar: 0 thoughts

[2025-10-08 18:58] Bloombit: Boston terriers are nice dogs
[Attachments: Screenshot_20251008-145832.png]

[2025-10-08 20:29] Analyze: I agree

[2025-10-09 08:36] drifter987: [replying to Bloombit: "Boston terriers are nice dogs"]
i myself have a french bulldog, attaching image soon

[2025-10-09 08:37] drifter987: 
[Attachments: image.png]

[2025-10-09 08:37] drifter987: shes turning three i think this year, still a baby, wants all the attention in the world and is playful

[2025-10-09 18:21] the horse: [replying to pinefin: ""]
this dog has seen all the wars

[2025-10-09 18:22] pinefin: [replying to the horse: "this dog has seen all the wars"]
hes good friends with the horse

[2025-10-09 18:22] the horse: the horse started all the wars

[2025-10-09 18:32] avx: [replying to the horse: "the horse started all the wars"]
she hid from all the wars
[Attachments: 20240619_192748.png]

[2025-10-09 20:31] iPower: okay that's enough this is a RE channel

[2025-10-09 20:31] iPower: I love the pictures but can you guys please post them somewhere else?

[2025-10-09 20:32] iPower: <#835646666858168320> exists for a reason

[2025-10-10 00:13] daax: Yeah, sorry, I derailed it but let’s move it back to the relevant topic as <@789295938753396817> said.

[2025-10-10 01:41] Andrew: Well i think my new goal is to make an AI that can translate Assembly to a HLL -  its not a "huge" deal, but it would be a nice functionality for Reverse Engineering - and its in my wheelhouse  rather than having to learn everything from the start.    -  
My problem with models are how much memory you need -  i was working on building a model to generate music and the facebook encoder model is nice for shaving down ram - but still takes too much memory to have a sufficient window size.

[2025-10-10 01:42] Andrew: i think i can prolly just hijack a LLM that will just do it for me ..  i dont think ill even need to finetune it ..

[2025-10-10 01:46] mtu: you don't

[2025-10-10 01:47] mtu: I've done small experiments with LLMs as an asm/p-code/other IL -> Golang pipe, few shot prompt context was enough

[2025-10-10 01:49] Andrew: what was the context window size / how much ram it took to run ?

[2025-10-10 01:53] Andrew: better question would be what model was best for you -  -  i can look up the details from that

[2025-10-10 05:16] mtu: I did it online using Gemini Flash 1.5 I think

[2025-10-10 05:18] mtu: Actual context used was well under 100k tokens, but the procedures I gave it for both the context few-shot and to test were relatively small

[2025-10-10 16:44] Loading: im trying to reverse network protocol of some binary, app is using crypto++ for encryption, i found FLIRT for IDA but there are no files online for crypto++, are there any alternatives to identify crypto++ related functions in binary ?

[2025-10-10 17:53] notsapinho: [replying to Loading: "im trying to reverse network protocol of some bina..."]
which binary?

[2025-10-10 18:27] Matti: [replying to Loading: "im trying to reverse network protocol of some bina..."]
I suggest reading the help txt file on how to generate FLIRT signatures yourself, it's pretty easy and very useful to know how to do

[2025-10-11 18:48] Y: Anyone doing flare-on this year?

[2025-10-11 18:58] Bloombit: [replying to Y: "Anyone doing flare-on this year?"]
I'm a bit into challenge 9

[2025-10-11 18:58] Y: [replying to Bloombit: "I'm a bit into challenge 9"]
Same here