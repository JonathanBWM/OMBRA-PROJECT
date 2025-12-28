# August 2025 - Week 5
# Messages: 297

[2025-08-25 03:34] Deleted User: is there a way to prevent an anticheat dll from inline hooking parts of the binary, if you load first your own dll before that anticheat dll? How can I protect the binary from inline hookings of that anticheat dll without it detecting that I prevented its hooks?

[2025-08-25 05:31] lukiuzzz: [replying to Deleted User: "is there a way to prevent an anticheat dll from in..."]
By loading your DLL first, you can preemptively hook key functions (like LoadLibrary and GetProcAddress) to intercept and block the anti-cheat's attempts to install its hooks, or you can continuously scan and restore the original code of critical functions in your binary before the anti-cheat has a chance to validate its own hooks.

[2025-08-25 05:33] Deleted User: [replying to lukiuzzz: "By loading your DLL first, you can preemptively ho..."]
i cannot do the "restore the original code" way, because the anticheat detects it if I restore to its original bytes. I will try the first one.

[2025-08-25 05:54] the horse: hook the anti-cheat

[2025-08-25 05:54] the horse: if it's not your code but some other module in game

[2025-08-25 05:54] the horse: likely the hook is in prologue

[2025-08-25 05:54] the horse: so just isolate the original prologue, execute it and jump after the hook

[2025-08-25 05:55] the horse: https://github.com/weak1337/SkipHook
[Embed: GitHub - weak1337/SkipHook]
Contribute to weak1337/SkipHook development by creating an account on GitHub.

[2025-08-25 05:56] the horse: if you have low-confidence in exactly locating the hook, load from disk and emulate the function

[2025-08-26 01:07] Deleted User: [replying to the horse: "so just isolate the original prologue, execute it ..."]
I will try this. Thank you ❤️

[2025-08-26 17:52] Aj Topper: how to get  cr3 of game protected with EAC to translate physical mem..?

[2025-08-26 17:59] Eriktion: [replying to Aj Topper: "how to get  cr3 of game protected with EAC to tran..."]
Serious question?

[2025-08-26 18:01] Aj Topper: [replying to Eriktion: "Serious question?"]
yes<:clueless:1402182826165796974>

[2025-08-26 18:02] Vonnegut: [replying to Aj Topper: "how to get  cr3 of game protected with EAC to tran..."]
Big clue, work backwards from some pages you know?

[2025-08-26 18:03] Aj Topper: [replying to Vonnegut: "Big clue, work backwards from some pages you know?"]
bruteforcing from some known pages..?

[2025-08-26 18:03] Vonnegut: It's a way, didn't say it's a good one

[2025-08-26 18:05] Aj Topper: [replying to Vonnegut: "It's a way, didn't say it's a good one"]
i got kernel's system cr3 but i was knowing its pointer, for game i was kinda clueless

[2025-08-26 18:08] Eriktion: https://github.com/MapleSwan/enum_real_dirbase
That was the way I did when I used to

[2025-08-26 18:09] Eriktion: For shuffling you could sorta cache a fixed byte sequence that a specific va should translate to

[2025-08-26 18:10] Eriktion: and via that validate whether the CR3 you are currently trying is the real one

[2025-08-26 18:10] Eriktion: [replying to Eriktion: "For shuffling you could sorta cache a fixed byte s..."]
Did sth like this in my hypervisor for the period of timme when cr3 shuffling was a thing. Don't think it is still now?

[2025-08-26 18:10] Eriktion: <@786963389397467206>

[2025-08-26 19:00] valium: [replying to Aj Topper: "how to get  cr3 of game protected with EAC to tran..."]
what game

[2025-08-26 19:01] Aj Topper: [replying to valium: "what game"]
i just wanted to get a grasp, so I was trying it on Rogue Company

[2025-08-29 19:58] Xenial.-: I got a quick question, does anyone have any resources or papers that can point me in the right direction, I want to create a filter driver for mouse movements without needing to rely on PnP, preferably through WDM, ofc I could just sit ontop or under PointerClass, but that is extremely risky. Any info would be more than appreciated. I am looking for safer alternatives. Thank you in advance!

[2025-08-29 23:20] lukiuzzz: [replying to Xenial.-: "I got a quick question, does anyone have any resou..."]
You can create a MiniFilter driver that registers for the Mouse device interface class (GUID_DEVINTERFACE_MOUSE). Windows  will then automatically attach your filter to the device stack of every compatible mouse device that is present or arrives later (PnP).

[2025-08-29 23:23] lukiuzzz: https://learn.microsoft.com/en-us/windows-hardware/drivers/install/guid-devinterface-mouse
[Embed: GUID_DEVINTERFACE_MOUSE - Windows drivers]
GUID_DEVINTERFACE_MOUSE

[2025-08-30 15:44] Plutonium: Anyone got some clue if NMI can be spoofed to map drivers to unsigned regions ?

[2025-08-30 15:48] the horse: did you mean "Can you spoof checks using NMI to detect mapped drivers in unsigned regions?"

[2025-08-30 16:44] Plutonium: I meant can the nmi stack be spoofed to remove the driver ?, since EAC spams it on all regions they pick up on addresses corresponding to the unsigned regions on the NMI stack

[2025-08-30 17:06] ragna: [replying to Plutonium: "I meant can the nmi stack be spoofed to remove the..."]
ye from ring -1 or -2, i wouldnt suggest you do smth in ring 0

[2025-08-30 17:07] ragna: use a hypervisor to manipulate guest state

[2025-08-30 17:08] ragna: so you can choose what eac nmi handler is seeing on the stack

[2025-08-30 17:33] Deleted User: [replying to ragna: "use a hypervisor to manipulate guest state"]
Could use Hyper-V for that

[2025-08-30 17:33] Deleted User: :^)

[2025-08-30 17:38] Xits: cant you just set the nmi ist to a different stack?

[2025-08-30 17:38] Xits: maybe patchguard says no

[2025-08-30 17:39] Xits: I guess the old rsp is still pushed on the stack

[2025-08-30 17:41] Xits: you could not use the rsp register for your stack but that would require a lot of work and they could still check the rip

[2025-08-30 17:58] L1ney: [replying to Plutonium: "Anyone got some clue if NMI can be spoofed to map ..."]
plenty of ways and can be done from ring-0 as long as you know what are you doing

[2025-08-30 18:00] Xits: [replying to L1ney: "plenty of ways and can be done from ring-0 as long..."]
How?

[2025-08-30 18:02] L1ney: [replying to Xits: "How?"]
i`m not here to spoonfeed

[2025-08-30 18:04] Pepsi: [replying to L1ney: "i`m not here to spoonfeed"]
(he probably has no clue or just a very questionable/hacky solution)

[2025-08-30 18:04] L1ney: literally answered for only one reason, because the guy above said that he shouldn`t be do anything related to it from kernel when its absolute bullshit

[2025-08-30 18:05] Xits: So you’re only here to show us how big your dick is?

[2025-08-30 18:05] Xits: Nice

[2025-08-30 18:06] ragna: [replying to L1ney: "literally answered for only one reason, because th..."]
„wouldnt suggest“, i did not say that its not possible lol

[2025-08-30 18:06] L1ney: [replying to ragna: "„wouldnt suggest“, i did not say that its not poss..."]
i did not said that you said its not possible

[2025-08-30 18:07] L1ney: [replying to Xits: "So you’re only here to show us how big your dick i..."]
yeah kind of

[2025-08-30 18:07] L1ney: maybe laugh sometimes to read what are you writing here

[2025-08-30 18:07] L1ney: not all of you obviously, some people know what are they doing in here

[2025-08-30 18:07] ragna: [replying to L1ney: "literally answered for only one reason, because th..."]
.

[2025-08-30 18:07] L1ney: "he shouldn`t be do anything related to it from kernel"

[2025-08-30 18:07] ragna: and btw how would you do it from kernel?

[2025-08-30 18:07] L1ney: i`m not here to spoonfeed

[2025-08-30 18:08] L1ney: 
[Attachments: image.png]

[2025-08-30 18:08] Xits: Maybe just go back to reading

[2025-08-30 18:08] ragna: [replying to L1ney: "i`m not here to spoonfeed"]
i dont think you have a kernel solution.

[2025-08-30 18:09] ragna: i dont need spoonfeed😂

[2025-08-30 18:09] L1ney: 🤣

[2025-08-30 18:09] Pepsi: [replying to ragna: "i dont think you have a kernel solution."]
i mean you can buy his p2c and can likely figure that out quite fast

[2025-08-30 18:09] ragna: oh hell naw

[2025-08-30 18:10] L1ney: how did i baited bunch of pasters with 1 message what the fuck

[2025-08-30 18:10] ragna: l1ney the nmi kernel faker 😂

[2025-08-30 18:12] ragna: [replying to Pepsi: "i mean you can buy his p2c and can likely figure t..."]
i dont support pastes

[2025-08-30 18:12] L1ney: [replying to ragna: "i dont need spoonfeed😂"]
by your answer to his question it doesn`t look like it

[2025-08-30 18:13] L1ney: you probably do and a lot of it

[2025-08-30 18:13] L1ney: but we will skip that part

[2025-08-30 18:13] ragna: skid is mad 🤡

[2025-08-30 18:13] L1ney: bro doxed himself 😭

[2025-08-30 18:14] Pepsi: dunno why, i feel like this channel is getting a wipe in 3.. 2.. 1..

[2025-08-30 18:33] Pepsi: where is the catpic sheriff 😸

[2025-08-30 18:46] Xenial.-: [replying to lukiuzzz: "You can create a MiniFilter driver that registers ..."]
Hmm, thank you sir. I am trying to avoid PnP due to the way I load (No certs) I will see if I can figure out a way to brute force this, and load the drivers needed, and attach myself. But much appreciated! Thank you for your response

[2025-08-30 18:47] lukiuzzz: [replying to Xenial.-: "Hmm, thank you sir. I am trying to avoid PnP due t..."]
no problem

[2025-08-30 19:47] UJ: [replying to Xits: "cant you just set the nmi ist to a different stack..."]
NMIs already execute on a dedicated interruptible stack i believe so a stackwalk on the IST stack frames won't lead to the thread, the cpu frame is pushed however like all interrupts which can point to the thread, even if this is spoofed, EAC can read the current thread that was executing on the core from gs:0x188 before the NMI switches to the NMI IDT handler and parse its ETHREAD and read the stack/start address of that thread manually. img is from EAC's ~~non ~~maskable interrupt executing. 

just emulate KiSwapContext 😆
[Attachments: Screenshot_2025-08-30_123438.png]

[2025-08-30 20:05] Xits: Even if you hook context swaps they can still check the rip in their nmi handler

[2025-08-30 20:07] Xits: You could probably create your own idt and switch between yours and the windows one in a context swap hook

[2025-08-30 20:12] Xits: Not sure how they could detect that if you’re able to hook context swaps early

[2025-08-30 20:18] Xits: Assuming u couldn’t hook the nmi handler directly

[2025-08-30 20:25] Deleted User: [replying to Xits: "Not sure how they could detect that if you’re able..."]
We’re slowly but surely getting to a point where kernel modifications don’t need to be detected anymore

[2025-08-30 20:25] Deleted User: https://www.strafe.com/news/read/what-is-enrollaik-exe-and-why-it-runs-when-you-play-call-of-duty/

[2025-08-30 20:27] Layf: Fuck tpm

[2025-08-30 20:31] Deleted User: [replying to Layf: "Fuck tpm"]
It’s not like this can’t be bypassed

[2025-08-30 20:31] Deleted User: Lmao

[2025-08-30 20:31] Deleted User: Nontrivial but feasible

[2025-08-30 20:32] Layf: I still massively dislike how everything moves into the "protect the device from its owner" direction

[2025-08-30 20:32] Deleted User: It’s what was bound to happen

[2025-08-30 20:33] Deleted User: Wouldn’t be surprised if they utilise Pluton as well once that’s available on mainstream processors

[2025-08-30 20:33] brew002: [replying to Plutonium: "I meant can the nmi stack be spoofed to remove the..."]
I created a POC for this awhile ago. Stability is questionable, code is not great, but it was runnable on a VM. Regardless of these flaws, the idea is still there and could be improved for your purposes. https://github.com/brew02/CovertThread
[Embed: GitHub - brew02/CovertThread: Creating covert system threads on Win...]
Creating covert system threads on Windows by leveraging the page tables and IDT - brew02/CovertThread

[2025-08-30 20:33] Deleted User: [replying to Layf: "I still massively dislike how everything moves int..."]
You can hate it all day long, but it’s only going to get worse :^)

[2025-08-30 20:34] Layf: I know 😔

[2025-08-30 20:34] Deleted User: And at the end of the day, it’s an opt in restriction

[2025-08-30 20:35] Deleted User: Don’t want it? Don’t play games whose anticheat enforces it

[2025-08-30 20:35] Xits: I don’t use that garbage OS anyway. Until they start blocking websites because my tpm is untrusted idc

[2025-08-30 20:35] Deleted User: Although the concept of every TPM having a uniquely identifiable marker is fucking insane

[2025-08-30 20:36] Brit: [replying to Deleted User: "Although the concept of every TPM having a uniquel..."]
why

[2025-08-30 20:36] Layf: [replying to Deleted User: "Don’t want it? Don’t play games whose anticheat en..."]
As long as it's just games idc

[2025-08-30 20:36] Oliver: most of the hardware have a unique marker

[2025-08-30 20:36] Deleted User: [replying to Brit: "why"]
Well not “insane”, just something I hate from a privacy perspective

[2025-08-30 20:36] Layf: [replying to Deleted User: "Well not “insane”, just something I hate from a pr..."]
You can hate it all day long, but it’s only going to get worse :^)

[2025-08-30 20:37] Deleted User: True, yes

[2025-08-30 20:37] Deleted User: Lmao

[2025-08-30 20:37] Oliver: [replying to Deleted User: "Well not “insane”, just something I hate from a pr..."]
its not really a privacy concern, aik exsists aswell

[2025-08-30 20:37] Brit: you realize that by just hashing how webgl triangles are rendered you were already being tracked throughout the web to even less savory actors than the anticheat you are currently worried about

[2025-08-30 20:37] Xits: Yeah hwid isn’t the issue. I think segregation will be a problem

[2025-08-30 20:37] Deleted User: [replying to Brit: "you realize that by just hashing how webgl triangl..."]
My point is that this doesn’t just apply to anticheats

[2025-08-30 20:38] Deleted User: But valid point

[2025-08-30 20:38] Oliver: [replying to Deleted User: "My point is that this doesn’t just apply to antich..."]
the problem with tpm imo is the fact that they are locking down the pc like an xbox

[2025-08-30 20:38] Oliver: you buy something you cant use like you want

[2025-08-30 20:40] Layf: I dread the day when Microsoft will force people to use their appstore and enforce strict integrity on those spps

[2025-08-30 20:40] cat weed: [replying to Brit: "you realize that by just hashing how webgl triangl..."]
webgl disabled ❤️

[2025-08-30 20:40] Brit: [replying to cat weed: "webgl disabled ❤️"]
good job, ther's only one gorillion other ways

[2025-08-30 20:40] Deleted User: [replying to Layf: "I dread the day when Microsoft will force people t..."]
Painfully reminds me of Android locking down sideloading

[2025-08-30 20:40] Brit: unsigned code should not run

[2025-08-30 20:40] Brit: Apple had the right of it

[2025-08-30 20:41] cat weed: it’s okay anti detect browser fixes it

[2025-08-30 20:41] Brit: <:selfsuck:1390099416064458801>

[2025-08-30 20:41] Deleted User: It’s just trading in freedom for a massively reduced attack surface

[2025-08-30 20:42] sync: [replying to Brit: "you realize that by just hashing how webgl triangl..."]
this is very not true, theres no fingerprinting software on browsers that can generate a unique enough fingerprint that identifies ur system uniquely enough

[2025-08-30 20:42] Brit: [replying to sync: "this is very not true, theres no fingerprinting so..."]
you are very wrong

[2025-08-30 20:42] sync: browsers combat this very hard

[2025-08-30 20:43] cat weed: https://coveryourtracks.eff.org
[Embed: Cover Your Tracks]
See how trackers view your browser

[2025-08-30 20:43] cat weed: just check for yourselves

[2025-08-30 20:43] sync: [replying to Brit: "you are very wrong"]
uniqueness is very low on brwosers like brave, mullvad, even safari on ios 26

[2025-08-30 20:43] sync: u wont find a unique identifier if you have a lot of visitors

[2025-08-30 20:43] cat weed: mullvad browser is great stuff

[2025-08-30 20:43] sync: sure you can track 10 visitors np, try tracking tens of millions of users, like AC's do

[2025-08-30 20:44] Brit: [replying to sync: "uniqueness is very low on brwosers like brave, mul..."]
I do not think you meant to use the word entropy there

[2025-08-30 20:45] sync: my bad i

[2025-08-30 20:45] sync: meant uniqueness

[2025-08-30 20:45] Brit: just by testing my own install of FF

[2025-08-30 20:45] Brit: with all the privacy options turned on

[2025-08-30 20:45] Brit: I have 19 bits of identifiable info

[2025-08-30 20:46] Brit: that's one in 524288

[2025-08-30 20:46] Brit: that's good enough TM

[2025-08-30 20:46] sync: [replying to Brit: "I have 19 bits of identifiable info"]
ya same

[2025-08-30 20:46] sync: [replying to Brit: "that's one in 524288"]
but this is the major part

[2025-08-30 20:47] Brit: combine this with the /8 you are coming from

[2025-08-30 20:47] Brit: and that's basically good enough

[2025-08-30 20:47] sync: we also aren't bad actors

[2025-08-30 20:47] sync: we arent trying to actively prevent tracking

[2025-08-30 20:48] Brit: I have my FF install pretty tightened up

[2025-08-30 20:48] Xits: librewolf 18.5 bits gg

[2025-08-30 20:48] Brit: but that's by far not the only tricks employed

[2025-08-30 20:48] sync: lets try mullvad brwoser

[2025-08-30 20:48] Brit: so 19 bits is very optimistic

[2025-08-30 20:49] avx: let's see paul allen's privacy measures

[2025-08-30 20:49] sync: eitherway my point is

[2025-08-30 20:49] sync: atleast browsers combat fingerprinting

[2025-08-30 20:49] Brit: not very hard

[2025-08-30 20:49] Brit: especially because it goes against the incentive structure of building a browser

[2025-08-30 20:50] avx: i have 19 aswell

[2025-08-30 20:50] avx: well 18.43

[2025-08-30 20:50] sync: [replying to Brit: "not very hard"]
yes now combat fingerprinting on windows

[2025-08-30 20:50] Brit: I just do not care

[2025-08-30 20:50] Brit: there's not point in trying to not be identifiable

[2025-08-30 20:51] sync: some people do though

[2025-08-30 20:51] sync: but they cant

[2025-08-30 20:52] Brit: the financial incentives are stronger than your desires for privacy

[2025-08-30 20:52] Brit: and if there was money to be made on the back of paranoid people, there'd be companies selling privacy browsers that actually work

[2025-08-30 20:52] Deleted User: Curious what a browser like ladybird would score

[2025-08-30 20:53] sync: mullvad browser 10 bits of identifiers

[2025-08-30 20:53] sync: on couple hundred thousand

[2025-08-30 20:53] sync: imagine that on millions, no identifiers

[2025-08-30 20:53] Brit: you realize that that website is not the state of the art

[2025-08-30 20:53] Brit: 10 bits is incredibly optimistic

[2025-08-30 20:54] JustMagic: [replying to sync: "mullvad browser 10 bits of identifiers"]
there's about 10 people using the mullvad browser, if you can identify that the person is on it, that's probably already good enough to use those 10 bits accurately

[2025-08-30 20:54] sync: the whole point of having privacy is to have standardized results for privacy queries like that

[2025-08-30 20:55] sync: 10 bits is close to 0 on on actual scale

[2025-08-30 20:55] sync: wheres my privacy windows os

[2025-08-30 20:55] Layf: True, I thought about that too. I doubt that enough people use hardened Firefox or mullvad browser to make it "anonymous"

[2025-08-30 20:56] sync: [replying to Brit: "and if there was money to be made on the back of p..."]
there are

[2025-08-30 20:56] sync: a lot

[2025-08-30 20:56] sync: but not for paranoid people, just fraudsters

[2025-08-30 20:56] cat weed: yes

[2025-08-30 20:57] Brit: [replying to sync: "there are"]
peanuts

[2025-08-30 20:57] sync: yes peanuts

[2025-08-30 20:57] sync: so your whole argument against privacy is

[2025-08-30 20:57] sync: not enough people want it ?

[2025-08-30 20:58] cat weed: i’m a strong advocate for digital privacy but the truth is privacy these days is an illusion

[2025-08-30 20:58] sync: i mean this just confirm his point tbh

[2025-08-30 20:58] sync: people just dont care anymore

[2025-08-30 20:58] sync: which i agree with

[2025-08-30 20:59] cat weed: data broker psyops won 😞

[2025-08-30 20:59] Brit: my argument is that it does not matter what you or I want, the markets produce the technology that creates the most value

[2025-08-30 21:01] sync: yeee

[2025-08-30 21:01] sync: we need to protest then brother

[2025-08-30 21:01] sync: public club -> privacy club

[2025-08-30 21:01] Brit: wrong ofcourse

[2025-08-30 21:01] Brit: but if you wanna waste your life protesting go for it

[2025-08-30 21:02] sync: yeah well its true

[2025-08-30 21:02] cat weed: protest by ransoming palantir

[2025-08-30 21:02] sync: EU seems to still care a bit

[2025-08-30 21:02] Brit: LMAO

[2025-08-30 21:03] cat weed: unfortunately not

[2025-08-30 21:03] sync: rlly

[2025-08-30 21:03] cat weed: maybe some eu countries but I wouldn’t say eu in general

[2025-08-30 21:03] sync: GDPR, privacy acts

[2025-08-30 21:03] sync: i'm not up 2 date at all on if they enforce

[2025-08-30 21:03] sync: but alteast they have

[2025-08-30 21:03] sync: lmfao

[2025-08-30 21:04] sync: yeah we are cooked

[2025-08-30 21:04] Brit: the EU would rather you send your passport to a private third party to confirm you're an adult than tell little timmy's parents that it's their fault their son is gooning out on the internet.

[2025-08-30 21:04] cat weed: true and real

[2025-08-30 21:05] cat weed: [replying to sync: "GDPR, privacy acts"]
those are good things but I don’t think they really respect privacy

[2025-08-30 21:05] cat weed: and everytime they introduce something that dangers digital privacy they use child safety as a scapegoat

[2025-08-30 21:05] cat weed: works everytime

[2025-08-30 21:06] Brit: isn't the EU also working on forbidding e2e encryption for private citizens

[2025-08-30 21:06] Brit: chat control act

[2025-08-30 21:06] Brit: or whatever

[2025-08-30 21:06] cat weed: pretty sure

[2025-08-30 21:06] cat weed: yeah

[2025-08-30 21:06] Brit: yeah no EU are clowns (on this specific topic)

[2025-08-30 21:06] cat weed: https://mullvad.net/en/chatcontrol/stop-chatcontrol
[Embed: Stop Chat Control]

[2025-08-30 21:07] Layf: [replying to Brit: "isn't the EU also working on forbidding e2e encryp..."]
I feel like 99% of "child protection" stuff is just privacy infringement

[2025-08-30 21:07] cat weed: pretty much

[2025-08-30 21:08] sync: [replying to cat weed: "https://mullvad.net/en/chatcontrol/stop-chatcontro..."]
crazy work

[2025-08-30 21:08] Oliver: chat control isnt to protect us

[2025-08-30 21:08] Oliver: thats for sure

[2025-08-30 21:09] sync: October 14, 2025 they voting

[2025-08-30 21:09] sync: we host voting ceremony in vc

[2025-08-30 21:09] Oliver: and how would they even break end to end ecryption

[2025-08-30 21:10] Brit: it's forbidden

[2025-08-30 21:10] Brit: there

[2025-08-30 21:10] Brit: broken

[2025-08-30 21:10] Oliver: One could just use a system simmilar to PGP

[2025-08-30 21:10] Oliver: [replying to Brit: "broken"]
i mean that would be crazy

[2025-08-30 21:10] Oliver: imo

[2025-08-30 21:10] Xits: didnt the US govt try to do that in the 80s

[2025-08-30 21:10] Oliver: going to prison for encryption is mad

[2025-08-30 21:10] Xits: not e2e. just normal encryption

[2025-08-30 21:10] Xits: didnt end well

[2025-08-30 21:11] Oliver: [replying to Xits: "not e2e. just normal encryption"]
wdym?

[2025-08-30 21:11] cat weed: [replying to Brit: "it's forbidden"]
https://www.patrick-breyer.de/en/posts/chat-control/

[2025-08-30 21:11] cat weed: that embed

[2025-08-30 21:12] Xits: [replying to Oliver: "wdym?"]
https://en.wikipedia.org/wiki/Crypto_Wars
[Embed: Crypto Wars]
The controversy unofficially dubbed the "Crypto Wars" involves attempts by the United States (US) and allied governments to limit access to cryptography strong enough to thwart decryption by national 

[2025-08-30 21:15] toasts: “protecting the children” or attribute all your online activity to your real identity

[2025-08-30 21:15] cat weed: only two options obviously

[2025-08-30 21:15] Oliver: [replying to toasts: "“protecting the children” or attribute all your on..."]
we all know that children could and should be protected in other ways then this

[2025-08-30 21:16] Oliver: 99% of children use apps like Kick, snapchat, discord, which isnt encrypted either way

[2025-08-30 21:16] Oliver: so its just bs

[2025-08-30 21:16] Oliver: have you ever seen children using signal?

[2025-08-30 21:16] cat weed: lmfao

[2025-08-30 21:16] cat weed: true

[2025-08-30 21:17] Oliver: they are just talking out of there ass, and if a child is using signal and the parents arent the contect its sus

[2025-08-30 21:18] Oliver: there are child control on phones that could be applied etc, so ye....

[2025-08-30 21:20] sync: [replying to Xits: "https://en.wikipedia.org/wiki/Crypto_Wars"]
we all know NSA has RSA backdoor

[2025-08-30 21:26] selfprxvoked: [replying to sync: "we all know NSA has RSA backdoor"]

[Attachments: ice-cube-looking-shocked-with-his-hands-to-his-temples-in-war-of-the-worlds-2025.avif]

[2025-08-30 21:26] selfprxvoked: NSA's RSA backdoors are being leaked

[2025-08-30 21:27] daax: [replying to cat weed: "https://mullvad.net/en/chatcontrol/stop-chatcontro..."]
I understand they want to get a grip on c(s)^a/(m+material), but this is a bit of a non-starter

[2025-08-30 21:41] Layf: [replying to Brit: "it's forbidden"]
Dumb question maybe, but doesn't eu chat control only mandate local file scanning? So the traffic leaving the device can still be encrypted without any backdoors

[2025-08-30 21:42] Layf: Still extremely horrible, but a bit less horrible

[2025-08-30 22:05] sync: its not really decided yet

[2025-08-30 22:05] sync: they either intercept and send to third party

[2025-08-30 22:05] sync: or trust AI to do it locally

[2025-08-30 22:05] sync: eitherway its all flawed

[2025-08-30 22:05] sync: read the mullvad post

[2025-08-31 15:35] mtu: [replying to sync: "we all know NSA has RSA backdoor"]
I’ve never been able to decide if this is the worst possible crypto take or “NSA has backdoor in AES”

[2025-08-31 16:42] sync: https://tenor.com/view/rock-everythingeverywhereallatonce-gif-25516405

[2025-08-31 19:14] brymko: [replying to mtu: "I’ve never been able to decide if this is the wors..."]
the specification of DES had protections in their sbox's against an attack not known to academia for 30 more or so years

[2025-08-31 19:14] mtu: [replying to brymko: "the specification of DES had protections in their ..."]
So the opposite of a backdoor

[2025-08-31 19:15] cat weed: I don’t think anything is out of the question for the NSA

[2025-08-31 19:16] the horse: AES signal interference leads to sattelite saucer uploads

[2025-08-31 19:16] brymko: [replying to the horse: "AES signal interference leads to sattelite saucer ..."]
huh

[2025-08-31 19:16] the horse: i am simply trolling

[2025-08-31 19:17] the horse: it's a stupid statement

[2025-08-31 19:17] brymko: i mean you can attack aes that way

[2025-08-31 19:17] brymko: or rather specific implimentations

[2025-08-31 19:17] brymko: if you get access to the hardware

[2025-08-31 19:17] the horse: interference is powerful

[2025-08-31 19:17] the horse: i saw some keystroke recording stuff with it

[2025-08-31 19:17] the horse: also HDD sniffing

[2025-08-31 19:21] the horse: https://en.wikipedia.org/wiki/Tempest_(codename)
[Embed: Tempest (codename)]
TEMPEST is a codename, not an acronym under the U.S. National Security Agency specification and a NATO certification referring to spying on information systems through leaking emanations, including un

[2025-08-31 19:21] the horse: this type of thing

[2025-08-31 19:21] the horse: so much work just to add more cat gifs into daax's collection

[2025-08-31 19:34] daax: [replying to the horse: "AES signal interference leads to sattelite saucer ..."]
you had me in the first half

[2025-08-31 19:36] the horse: anyways this stuff is crazy

[2025-08-31 19:36] the horse: the tin foils are right

[2025-08-31 19:37] daax: [replying to the horse: "the tin foils are right"]
the gubment is in my walls, telling the anti-cheats all my secrets <:whyy:820544448798392330> there is some neat tech out there

[2025-08-31 19:38] the horse: it's the ad companies predicting what type of bypass you're making next

[2025-08-31 19:38] the horse: anyways anti-cheats are nice enough to leave you alone until you start making money off of it

[2025-08-31 19:39] the horse: except BattlEye, they will swing the hammer until you spend 2 hours getting past it

[2025-08-31 19:39] cat weed: [replying to the horse: "the tin foils are right"]
make sure to line the inside of your stylish tin foil hat with lead

[2025-08-31 19:39] the horse: [replying to cat weed: "make sure to line the inside of your stylish tin f..."]
you can actually skip this by drinking water with lead traces

[2025-08-31 19:39] cat weed: real

[2025-08-31 19:40] the horse: a little bit of fluoride and SSRIs and you're well protected

[2025-08-31 19:40] Brit: Do not look up van exk phreaking :^)

[2025-08-31 19:40] the horse: 30 seconds in and you did not lie, it's freaky

[2025-08-31 19:41] Brit: Eck*

[2025-08-31 19:41] the horse: thank god i'm larping as a horse and therefore seen as a delusional idiot by the government so this risk does not concern me

[2025-08-31 19:41] the horse: i miss terry

[2025-08-31 19:42] the horse: ||not to be decoded as 'I, mystery'||

[2025-08-31 19:42] Brit: [replying to the horse: "thank god i'm larping as a horse and therefore see..."]
The govt is very interested in the MBA simplifying horse

[2025-08-31 19:42] the horse: five

[2025-08-31 19:43] the horse: I gallop through circuits where horses align,
With mixed-boolean logic, their strides intertwine.

[2025-08-31 20:24] sariaki: Deep

[2025-08-31 20:51] daax: [replying to the horse: "anyways anti-cheats are nice enough to leave you a..."]
this seems to happen to the people who get greedy, not making money off it <:Kappa:794707301436358686> bad opsec and overreaching, which is understandable -- "make as much as you can quickly", but flying under the radar is lucrative and keeps you out of their crosshairs.