# November 2024 - Week 5
# Channel: #reverse-engineering
# Messages: 69

[2024-11-25 07:25] twelve: any advice on which one to get at there current states? i'm looking to protect an exe compiled in c++ and have the option between vmprotect/themida

[2024-11-25 08:15] dinero: LLVM <:cooding:904523055374676019> 
but to answer your question in your case   i’d go with themida if you put a gun to my head and made me chose between them

[2024-11-25 09:21] BWA RBX: [replying to twelve: "any advice on which one to get at there current st..."]
I would go with VMProtect either way you're still getting cracked, Themida not sure especially if you're wanting to protect an authentication system

[2024-11-25 09:28] dinero: can you even buy vmp  in the UK/EU/USA anymore?

[2024-11-25 09:29] dinero: https://discord.com/channels/835610998102425650/835667591393574919/1251124583738904637

[2024-11-25 09:29] dinero: as i thought they  do not sell to NATO?

[2024-11-25 09:31] dinero: id like to parrot the idea that vmp is more stable or something but the bar is low here 
note i haven’t looked at either in like a year and i can see themida improving  
but the average p2c “auth system” is an unstable piece of garbage in the first place even without obf :”)

[2024-11-25 09:31] dinero: does the spinlock still exist

[2024-11-25 09:35] snowua: [replying to BWA RBX: "I would go with VMProtect either way you're still ..."]
What is any part of this statement based on...?

[2024-11-25 09:36] dinero: 
[Attachments: IMG_2601.png]

[2024-11-25 10:04] safareto: [replying to dinero: ""]
Based on the hardware that's installed in it

[2024-11-25 11:48] BWA RBX: [replying to snowua: "What is any part of this statement based on...?"]
Personal preference, but like I said either way the application is getting cracked

[2024-11-25 11:51] koyz: I am all for Oreans, CV and Themida are great and the amount of customization you can do to their VMs is insane imo (although you will need to contact their support to get their VM validator to "sign" the VMs)
anyway I agree with rbx' statement, if you don't apply any other form of protection on your binaries you will inevitably get cracked

[2024-11-25 16:07] James: themida is not good.

[2024-11-25 16:08] James: you're probably using a leaked one based on the type of thing you're protecting

[2024-11-25 16:08] James: soooo multithreading is gone?

[2024-11-25 16:09] James: then you have to weave your way around switch statements, seh blocks, and complex arithmetic

[2024-11-25 16:09] James: the choice becomes obvious after that.

[2024-11-25 16:09] James: neither VMP nor themida will stop someone that is determined enough

[2024-11-25 16:10] James: so you might as well use the one that is easier.

[2024-11-25 16:29] irql: god.. if only there was an alternative to vmprotect/themida that supported all those things..?

[2024-11-25 17:08] James: 🤣

[2024-11-25 18:38] Deleted User: [replying to irql: "god.. if only there was an alternative to vmprotec..."]
CodeAttacker…

[2024-11-25 21:18] daax: [replying to James: "neither VMP nor themida will stop someone that is ..."]
are you implying code defender will stop someone who is determined?

[2024-11-25 21:19] James: [replying to daax: "are you implying code defender will stop someone w..."]
nothing will stop someone that is determined

[2024-11-25 21:19] James: all you can ever do is slow people down

[2024-11-25 22:21] pinefin: or, make your software so useless that no one ever has a need to crack it

abstinence saves us all

[2024-11-25 22:21] pinefin: you're welcome all

[2024-11-26 06:46] Timmy: most software already follows that advice.

[2024-11-27 18:12] pingpong: Hi folks, quick question : I remember testing an online tool that given a certain code (C IIRC) and a toolchain profile would assemble the code and easily allow you to compare it to the disassembled code from a specific routine. It was not godbolt, it was not dogbolt and it was VERY specifically aimed at console game code recreation but for some reason I can't find any references to it online (I've tried google and asking chatgpt).

I'm 100% this was not a fever dream and this tool exists but for some weird reason I can't find a trace of it so I'm reaching out as a hail mary...

Thanks beforehand!

[2024-11-27 18:34] Deleted User: [replying to pingpong: "Hi folks, quick question : I remember testing an o..."]
https://decomp.me/ ?
[Embed: decomp.me]
A collaborative decompilation platform.

[2024-11-27 18:37] pingpong: Jeez <@456226577798135808> that's exactly what I was looking for, I can't understand for the life of me how couldn't I find it anywhere

[2024-11-27 18:38] pingpong: I'm saying this mostly in jest but it's as if it was shadow banned

[2024-11-27 19:50] x86matthew: [replying to pingpong: "Jeez <@456226577798135808> that's exactly what I w..."]
google has become useless in recent months

[2024-11-27 19:55] mibho: for x86 MSVC binaries is it theoretically  possible to make naked functions so that the stack becomes funky and still valid throughout different calls <:charmanderp:848598126251212810>

[2024-11-27 20:35] jvoisin: [replying to x86matthew: "google has become useless in recent months"]
"AI first company"

[2024-11-27 22:27] pingpong: x86 being the key word, IIRC they discontinued it or never implemented it for x86_64

[2024-11-27 22:27] pingpong: Which was sad to say the least because it was a neat feature

[2024-11-27 22:28] pingpong: Also yeah, IDK what the culprit of it is in this case but both mainstream browsers and AI is heavily censored which is weird to say the least

[2024-11-28 04:18] James: [replying to mibho: "for x86 MSVC binaries is it theoretically  possibl..."]
You can just add an assembly file

[2024-11-28 05:17] Lyssa: [replying to x86matthew: "google has become useless in recent months"]
I know right

[2024-11-28 05:18] Lyssa: couldn't find the fucking ``git log`` command mentioned anywhere even when directly searching "how to view git commit history through command line"

[2024-11-28 05:18] Lyssa: how much more specific do I need to be?

[2024-11-28 07:12] zeropio: add `&udm=14` to the url search and you have (more or less) old google search

[2024-11-28 08:42] Timmy: kagi :3

[2024-11-28 11:27] roddux: [replying to x86matthew: "google has become useless in recent months"]
try years lol

[2024-11-28 11:27] roddux: [replying to zeropio: "add `&udm=14` to the url search and you have (more..."]
what does this do ?

[2024-11-28 11:29] x86matthew: [replying to roddux: "try years lol"]
perhaps, but i never really had any complaints until 6-12 months ago when i noticed a huge decline in useful/relevant results

[2024-11-28 11:31] x86matthew: eg i often use quotation marks to narrow things down but google now seems to treat them as optional

[2024-11-28 11:34] x86matthew: and it doesn't take no for an answer with the "did you mean..." suggestions

[2024-11-28 11:36] zeropio: [replying to roddux: "what does this do ?"]
web search
[Attachments: image.png]

[2024-11-28 11:36] zeropio: is the closest thing to how google used to work

[2024-11-28 11:37] zeropio: you can configure your browser to add the tag when you search, so you always have the web view enabled

[2024-11-28 16:52] Lyssa: [replying to zeropio: "web search"]
huh?

[2024-11-28 16:52] Lyssa: I don't get how that makes a difference

[2024-11-28 16:55] zeropio: [replying to Lyssa: "I don't get how that makes a difference"]
web view only show links to websites

[2024-11-28 16:55] zeropio: you avoid all of the above
[Attachments: 3-1440x2064.png]

[2024-11-28 16:57] Lyssa: ah I see

[2024-11-28 16:57] Lyssa: thank you

[2024-11-28 20:41] mibho: [replying to James: "You can just add an assembly file"]
im taking that as in it is possible thank u <:pepeSmile:786434644085571594>

[2024-11-28 21:28] James: Yes it’s very possible to mess up the stack

[2024-11-29 10:13] BWA RBX: [replying to James: "Yes it’s very possible to mess up the stack"]
Yes hexrays does it all the time

[2024-11-29 15:51] James: [replying to BWA RBX: "Yes hexrays does it all the time"]
I don’t follow.

[2024-11-29 15:57] Hunter: [replying to James: "I don’t follow."]

[Attachments: bmc.png]

[2024-11-29 15:58] Hunter: oh no back engineering member

[2024-11-29 18:37] James: ohhhhhh xD

[2024-11-29 18:37] James: yeah hate that thing

[2024-11-29 18:38] James: a lot of the time it is right though?

[2024-11-29 20:24] ScbQueue: https://streamable.com/igs12r CVE-2024-44083 being funny as usual
[Embed: Watch real | Streamable]
Watch "real" on Streamable.