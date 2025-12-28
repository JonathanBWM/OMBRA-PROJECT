# September 2025 - Week 3
# Messages: 194

[2025-09-15 00:41] abu: [replying to the horse: "otherwise it's pretty straightforward - they have ..."]
When you say "taking care of working set scans", do you mean "just don't page fault bro"?

[2025-09-15 00:42] the horse: [replying to abu: "When you say "taking care of working set scans", d..."]
no, specifically the Shared and SharedCount variables within a working set virtual memory query

[2025-09-15 00:43] the horse: if you patch a module, these will change.

[2025-09-15 00:43] abu: Thanks, do you have more information on this? (The structures where Shared/ShareCount is?)

[2025-09-15 00:44] the horse: ```cpp
void adhesive::integrity::handle_nt_query_virtual_memory ( PCONTEXT context ) {
    const auto base = context->Rdx;
    if ( base != 0 ) {
        if ( context::adhesive_text.within_bounds ( base ) ) {
            auto info = ( MEMORY_BASIC_INFORMATION* ) context->R9;
            info->Protect = PAGE_EXECUTE_READ;
            //info->State = MEM_COMMIT;
            //info->Type = MEM_IMAGE;

            logger::print ( "[exception_handler] Spoofed adhesive memory protection" );
        }

        if ( ARG3 == 4 ) { // MemoryWorkingSetExInformation
            size_t len = STKARG ( 0x28 );
            for ( auto i = 0u; i < len; i += sizeof ( MEMORY_WORKING_SET_EX_BLOCK ) ) {
                auto info = ( PMEMORY_WORKING_SET_EX_INFORMATION ) ( ARG4 + i );
                if ( context::adhesive.within_bounds ( reinterpret_cast< std::uint64_t > ( info->VirtualAddress ) ) ) {
                    logger::print ( "[exception_handler] Spoofed adhesive working set info" );
                    if ( context::adhesive_text.within_bounds ( reinterpret_cast< std::uint64_t > ( info->VirtualAddress ) ) ) {
                        info->u1.VirtualAttributes.Win32Protection = PAGE_EXECUTE_READ;
                    }
                    if ( info->u1.VirtualAttributes.Shared == 0 )
                        info->u1.VirtualAttributes.Shared = 1;
                    if ( info->u1.VirtualAttributes.ShareCount == 0 )
                        info->u1.VirtualAttributes.ShareCount = 1;

                    info->u1.VirtualAttributes.SharedOriginal = 1;
                }
            }
        }
    }
}
```

[2025-09-15 00:44] abu: thx lol. Is this all yours?

[2025-09-15 00:44] the horse: yes

[2025-09-15 00:45] abu: thx sm 🙂

[2025-09-15 00:46] the horse: 
[Attachments: message.txt]

[2025-09-15 00:49] the horse: you may find this useful:

[2025-09-15 00:49] the horse: 
[Attachments: message.txt]

[2025-09-15 00:50] the horse: 
[Attachments: message.txt]

[2025-09-15 00:50] the horse: lock all mutexes, then patch

[2025-09-15 00:51] the horse: this will suspend integrity & lots of their checks

[2025-09-15 00:51] abu: How did you find that they were using them in the first place?

[2025-09-15 00:51] the horse: by seeing _Mtx_Lock in the code lol

[2025-09-15 00:52] the horse: i was looking for the integrity function so i did a monitor with hyperdbg

[2025-09-15 00:52] the horse: they are locking the mutex at the start of the function

[2025-09-15 00:52] the horse: i don't cheat in the game or w/e just wanted to log their hwid

[2025-09-15 00:53] abu: Oh ok, thanks for sharing that. I'm on AMD so I'm unable to use hyperdbg but I assumed you just threw an access BP on some .text section and collected hits?

[2025-09-15 00:53] the horse: yep

[2025-09-15 00:53] the horse: i think they do have a semi-functional AMD variant

[2025-09-15 00:53] the horse: reddbg or w/e? (nvm abandoned)

[2025-09-15 00:53] abu: I need to make my own anyways

[2025-09-15 00:53] abu: great experience tbh

[2025-09-15 00:54] the horse: you could probably just use NO_ACCESS for AV logging

[2025-09-15 00:54] abu: Also I remember Matti complaining about something in hyperdbg

[2025-09-15 00:54] abu: [replying to the horse: "you could probably just use NO_ACCESS for AV loggi..."]
Yeah, it's kinda of ass though

[2025-09-15 00:54] abu: There was a method I used using the TLB

[2025-09-15 00:54] abu: and it worked a little

[2025-09-15 00:55] abu: just for what I needed though

[2025-09-15 00:57] the horse: anyways adhesive is pretty ass; the "best" part of it is just the annoying obfuscator

[2025-09-15 00:57] abu: I thought they used VMP

[2025-09-15 00:57] abu: or is that in FiveM

[2025-09-15 00:59] the horse: fiveM anti-cheat is adhesive

[2025-09-15 00:59] the horse: im pretty sure it's some ollvm thing

[2025-09-15 01:00] abu: 🙏

[2025-09-15 01:00] abu: Thanks for the info man I appreciate it

[2025-09-15 05:41] Xyrem: the (NOT MBA SIMPLIFYING) horse which reverses adhesive

[2025-09-15 05:52] the horse: the horse applies ethyl acetate to the adhesive

[2025-09-15 19:03] xShARk: Anyone recently dumped beclient2 on ark ascended recently ?

[2025-09-16 14:21] neznanlik: [replying to xShARk: "Anyone recently dumped beclient2 on ark ascended r..."]
this is be2 dropper. hook any winapi here and check return address for private executable memory
[Attachments: image.png]

[2025-09-17 19:01] xShARk: [replying to neznanlik: "this is be2 dropper. hook any winapi here and chec..."]
Thank boss could you check dm ?

[2025-09-17 19:01] iPower: why not asking here? others could benefit from the answers

[2025-09-17 21:17] xShARk: [replying to iPower: "why not asking here? others could benefit from the..."]
It wasn’t related directly to this, but to a common friend we have 😅

[2025-09-19 18:11] Loading: when reversing anticheat how do you know if you found everything related to anticheat so you know you wont get banned ? is it just trial and error so once you stop getting banned you know you bypassed everything ?

[2025-09-19 19:21] the horse: hook anything that can be used to scan in any way; verify those hooks are not detected, and if they're integrity check, make sure the integrity check isn't integrity checked

[2025-09-19 19:21] the horse: depending on what you're targeting this is either extremely hard or very hard

[2025-09-19 19:22] the horse: alternatively the flags have to get sent back to the server some way.. maybe you can hook the report functions

[2025-09-19 19:34] daax: [replying to Loading: "when reversing anticheat how do you know if you fo..."]
no. it is hours of reversing and testing theories, tracing, hooking different places that have a higher than usual probability of doing something. doing runtime tracing (not necessarily emulation) will give you reasonable amounts of information as a starting point.

[2025-09-19 19:35] daax: trial and error is inefficient for most things because it will get expensive or time consuming, but sometimes necessary. if you're doing that the whole time you need to rethink your approach.

[2025-09-19 22:09] the horse: no

[2025-09-19 22:09] peter drippin: [replying to the horse: "no"]
crazy

[2025-09-19 22:09] the horse: i just break it

[2025-09-19 22:09] peter drippin: [replying to the horse: "i just break it"]
why would you tho

[2025-09-19 22:09] the horse: for memes

[2025-09-19 22:09] the horse: was interested in what they do

[2025-09-19 22:09] the horse: i could care less about GTA

[2025-09-19 22:10] peter drippin: [replying to the horse: "was interested in what they do"]
prolly nothin

[2025-09-19 22:10] the horse: yeah not much

[2025-09-19 22:10] peter drippin: their anti-cheat is practically useless

[2025-09-19 22:10] peter drippin: i've reversed a bunch of p2cs

[2025-09-19 22:10] peter drippin: for 5m

[2025-09-19 22:10] peter drippin: never the anti-cheat tho couldn't care less

[2025-09-19 22:10] the horse: i mean the only real value is preventing lua execution & hwid checks

[2025-09-19 22:10] the horse: actual game cheats are worthless without an executor on FiveM

[2025-09-19 22:11] peter drippin: [replying to the horse: "i mean the only real value is preventing lua execu..."]
i don't think any of that is good either

[2025-09-19 22:12] peter drippin: ........

[2025-09-19 23:17] Monokuma: What are my options if I want to patch a DLL before  DllEntryPoint gets called? Basically, the game loads the dll as the first one, and I want to patch it before it has the chance to execute anything. I was able to load my DLL before the target dll DllEntryPoint is called using DetourCreateProcessWithDllExA, but trying to patch anything results in a crash. I know that this is not a problem with my patches, because even if I patch with the exact original bytes (effectively changing nothing), it still crashes. It would be great if someone could give me a tip on how to do this safely.

[2025-09-20 01:19] brymko: startup suspended and hook load library then resume

[2025-09-20 10:17] Aj Topper: How to disable all security features when loading pubg to inject dumper-7

[2025-09-20 11:36] daax: [replying to the horse: "no"]
wdym no?

[2025-09-20 11:42] daax: [replying to Aj Topper: "How to disable all security features when loading ..."]
Nobody is going to answer if you provide no background of what you’ve already tried, specifically what you’re referring to (are you meaning temporarily suspending xenuine, etc to inject and dump quickly or are you referring to BE? What issue are you encountering?) and you ask like this is some kind of search engine. Dumping the PUBG SDK should be a pretty quick turnaround.

[2025-09-20 11:44] Aj Topper: [replying to daax: "Nobody is going to answer if you provide no backgr..."]
i tried opening the game without BE the process dies after 10s but that's not the issue, even after loading it without BE it doesn't let me inject a dll, most usermode injection methods fail

[2025-09-20 13:58] the horse: [replying to daax: "wdym no?"]
someone asked if i dev adhesive

[2025-09-20 13:58] the horse: but deleted the message

[2025-09-20 14:09] daax: [replying to the horse: "someone asked if i dev adhesive"]
oh I see lol

[2025-09-20 15:45] Timmy: [replying to Aj Topper: "i tried opening the game without BE the process di..."]
ACs started stripping um process related handles a good while ago. You'll need to work around that somehow. Solutions could be doing the injection without um handles or unloading the ACs kernel driver.

[2025-09-20 15:47] the horse: heartbeat:

[2025-09-20 15:47] the horse: https://tenor.com/view/orange-cat-gets-flung-and-explodes-orange-cat-funny-cat-meme-explodes-gif-10706110874965244466

[2025-09-20 16:19] Brit: [replying to Timmy: "ACs started stripping um process related handles a..."]
you could launch the game without BE and get stuck at the main menu when the game released, in fact you could even get into a game for about a minute before the heartbeat yeeted you

[2025-09-20 16:49] Aj Topper: [replying to Brit: "you could launch the game without BE and get stuck..."]
I'm launching PUBG without BE but it's not allowing me to inject the 7-dumper dll

[2025-09-20 16:59] Brit: no clue what 7 dumper is or what changed since release

[2025-09-20 16:59] Brit: I have not looked at pubg in years

[2025-09-20 17:21] Monokuma: [replying to brymko: "startup suspended and hook load library then resum..."]
I didn’t specify in my question, but the DLL is loaded as the first(and only) import. LoadLibrary is only for dynamic loading, right? I think I can hook one of the functions the loader uses, though.

[2025-09-20 21:36] selfprxvoked: [replying to the horse: "someone asked if i dev adhesive"]
Swiftik developing adhesive while randomly dropping some bypasses here <:mmmm:904523247205351454>

[2025-09-20 21:37] the horse: paste or be pasted

[2025-09-20 21:37] the horse: or Jahiliyyah

[2025-09-21 00:02] lennard.cpp: can anyone recommend games (preferably without anticheat) i can learn game hacking on?

[2025-09-21 00:08] the horse: anything that doesn't have a very strong anti-cheat and you enjoy playing/cheating in

[2025-09-21 00:08] the horse: it's almost worthless if you don't like the game

[2025-09-21 00:48] Xits: [replying to lennard.cpp: "can anyone recommend games (preferably without ant..."]
dead by daylight

[2025-09-21 00:48] Xits: unreal engine and shitty eac

[2025-09-21 01:52] iPower: [replying to the horse: "it's almost worthless if you don't like the game"]
this. it becomes a very annoying task

[2025-09-21 01:52] iPower: when I first learned game hacking I made cheats for CS2D and GTA San Andreas because those are two games I liked

[2025-09-21 01:52] iPower: I tried to follow the assaultcube formula but the game fucking sucks

[2025-09-21 01:53] iPower: pretty much learned all the basics in GTA SA: hooking, function calling, pointers, etc...

[2025-09-21 03:02] daax: [replying to lennard.cpp: "can anyone recommend games (preferably without ant..."]
assault cube, soldat, gatekeeper, battlefield 2/3/4, tf2, cs 1.6/source (turn off vac of course with -insecure), halo pc, need for speed most wanted (2005), cod 4, day of defeat, americas army, any singleplayer / coop fps game will suffice to learn the basics; you can choose an mmo as well if you’d like. try to steer clear of games with anti-cheats until you understand the fundamentals, and even the don’t go straight for the hardened crap. you’ll hurt your own feelings doing that haha.

[2025-09-21 03:09] iPower: [replying to Brit: "no clue what 7 dumper is or what changed since rel..."]
i think they're talking about dumper-7, a popular SDK generator used for UE games

[2025-09-21 03:10] iPower: [replying to Aj Topper: "I'm launching PUBG without BE but it's not allowin..."]
could be their internal ac shit. did you check if there are any hooks in places like LdrLoadDll (or IAT hooks for LoadLibrary, like EAC does), LdrInitializeThunk, etc?

[2025-09-21 03:11] iPower: I'm not sure how PUBGs internal shit works because i've never played the game. but checking for hooks and trying to undo them worths a try

[2025-09-21 12:18] lennard.cpp: [replying to daax: "assault cube, soldat, gatekeeper, battlefield 2/3/..."]
when can i progress to a game with ac? im currently learning on assult cube the basics. so static offsets, entity object, entity list and all that. but at what point do i say i move on?

[2025-09-21 12:18] logic: [replying to lennard.cpp: "when can i progress to a game with ac? im currentl..."]
i'd probably progress onto cs

[2025-09-21 12:23] lennard.cpp: [replying to logic: "i'd probably progress onto cs"]
is vac that weak?

[2025-09-21 12:24] logic: [replying to lennard.cpp: "is vac that weak?"]
yes

[2025-09-21 12:29] lennard.cpp: [replying to logic: "yes"]
say i have a basic cheat for assult cube. currently its only health and shit, but my end goal is esp and maybe aim bot. is that sufficient knowledge for cs?

[2025-09-21 12:32] logic: [replying to lennard.cpp: "say i have a basic cheat for assult cube. currentl..."]
yes

[2025-09-21 12:32] lennard.cpp: ok cool thanks

[2025-09-21 12:32] lennard.cpp: also that explains why there  are a fuck ton of cheaters in cs

[2025-09-21 12:32] logic: vac anticheat basically doesnt exist

[2025-09-21 12:33] lennard.cpp: i always wonderd if all those people in my matches are smurfing or have a better gaming desk

[2025-09-21 12:33] lennard.cpp: here is my answer ig

[2025-09-21 12:34] lennard.cpp: 10/10 pfp btw

[2025-09-21 12:35] logic: thanks, i like cats

[2025-09-21 12:35] lennard.cpp: me too. you have cats?

[2025-09-21 12:35] logic: i have dogs

[2025-09-21 12:35] lennard.cpp: aww

[2025-09-21 12:35] logic: but i used to have cats

[2025-09-21 12:35] lennard.cpp: 🙁

[2025-09-21 12:45] lennard.cpp: [replying to logic: "but i used to have cats"]
sry for bothering you so much. i just have one more question. do you have any good sources for learning reverse engineering? thats my main weak point

[2025-09-21 12:47] logic: [replying to lennard.cpp: "sry for bothering you so much. i just have one mor..."]
its fine, also i suck at reverse engineering as i dont really do that, however it really is just about what tools you use and learning as you go

[2025-09-21 12:48] lennard.cpp: ok thanks

[2025-09-21 12:49] dankeane: Have a look at guidedhacking

[2025-09-21 12:49] dankeane: If your interested in game hacking

[2025-09-21 12:49] lennard.cpp: thats currentyl my main source of information

[2025-09-21 12:49] lennard.cpp: gh and unkown cheats

[2025-09-21 13:25] Bony: eac cheat moment
[Attachments: image.png]

[2025-09-21 13:57] daax: [replying to lennard.cpp: "when can i progress to a game with ac? im currentl..."]
timeline is different for everyone but given you have zero experience: when you know how to write a generic hack

[2025-09-21 14:05] daax: [replying to lennard.cpp: "also that explains why there  are a fuck ton of ch..."]
I’d avoid advice like what <@1278940556164071456> gave tbh - the only reason VAC “doesn’t exist” is because people documented in detail how it works and how to bypass it, specifically where to hook, if you don’t actually understand it you’re just going to get beamed if it ever changes or adds functionality. If you don’t know how to reverse engineer you will not do well against more involved ACs or those with little documentation. Anyone who says VAC is easy but couldn’t handle it without documentation is not someone you should follow advice from wrt game hacking. Sounds harsh, but if you want to be **good** at it, learn the why and don’t rush to the end result. Read the documentation that exists for ACs but redo the work of RE’ing manually mapped modules and finding where to hook and building your own tools to do so.

[2025-09-21 14:06] heaps: It's just that different games have different technologies and program bases. Sure, general reverse engineering knowledge is applicable, but you need to know about the game's internals. Let's say you mastered that one game for 7 years, that knowledge might not directly apply to the other game. Also different ACs have different checks. All based upon reverse and game architecture knowledge.

Right. That's what happened for me. I absolutely have no idea how ACs work (except the general knowledge) because the game I worked with didn't have a solid AC system.

[2025-09-21 14:08] Pepsi: message *disappears*

[2025-09-21 14:08] Pepsi: why is there no restorecord for discord web

[2025-09-21 14:08] Pepsi: its annoying

[2025-09-21 14:08] Brit: P2c regard with regarded opinions, nothing worth keeping

[2025-09-21 14:09] lennard.cpp: [replying to daax: "I’d avoid advice like what <@1278940556164071456> ..."]
hmmm ok. makes sense. do you have recommendations on how to learn re?

[2025-09-21 14:09] lennard.cpp: rn im using cheat engine and try to figure out the asm it spits out. worked well so far, but ik i will get into a dead end pretty soon

[2025-09-21 14:14] daax: [replying to lennard.cpp: "hmmm ok. makes sense. do you have recommendations ..."]
generally you learn re by doing. pick a binary, throw it in your tool of choice (ghidra/ida/binja/hiew/whatever) + x64dbg and start to try to make sense of what is happening. when I started I was told to write binaries so I knew how they worked with various different operations; linked list insertion, deletion, sorting, binary search, nested functions, for/while loops spamming message boxes, global classes with virtual functions and inherited classes, etc. it's a process, but it will help you begin to recognize patterns.

i'll self plug something that might help with getting introduced: <https://revers.engineering/applied-reverse-engineering-series/> -- it's by no means comprehensive, but some strategies for learning. I did parse through books like "hacker disassembly" (rip kris kaspersky), "secrets of reverse engineering" (great book), and "practical reverse engineering" when I started. The best way to learn it is just to start imo. path is different for everyone, but that's how I went about it. Having the tools and a place to ask questions is ideal. I think most people here went about it in different ways.

[2025-09-21 14:18] Pepsi: [replying to daax: "I’d avoid advice like what <@1278940556164071456> ..."]
yes people documented a shit ton about vac, but for years it didn't really do much against people doing some gamehacking for lulz (aka not being a p2c)

[2025-09-21 14:18] Pepsi: i remember playing csgo for years with a loadlib injected internal and nothing happened at all

[2025-09-21 14:19] lennard.cpp: [replying to daax: "generally you learn re by doing. pick a binary, th..."]
thanks, ill definitely have a look. are carckme's something i should try or is that a thing later down the journey?

[2025-09-21 14:22] Pepsi: [replying to Pepsi: "i remember playing csgo for years with a loadlib i..."]
those times are over now, but i think there is a reason why people mock VAC

[2025-09-21 14:27] daax: [replying to Pepsi: "those times are over now, but i think there is a r..."]
what would you recommend wrt this question? https://discord.com/channels/835610998102425650/1378136680271319121/1419324578643181768

[2025-09-21 14:27] daax: [replying to Pepsi: "yes people documented a shit ton about vac, but fo..."]
yeah ofc, I'm just saying to suggest to a newcomer that VAC basically doesn't exist (even though in the past it was even more of a meme) is setting them up for a ban unless they start pasting... which isn't a route i'd personally recommend, but ig to each their own

[2025-09-21 14:29] Brit: I think learning by re-ing a source game with --insecure or whatever it was is fine

[2025-09-21 14:29] Brit: To be exposed to a more complex game engine than assault cube

[2025-09-21 14:31] Pepsi: [replying to daax: "what would you recommend wrt this question? https:..."]
tbh, my advice is to to *just do it*, grab a game you are interested and one will stumble over the things  he needs to learn,
if you have no previous experience at all, maybe grab an older game that doesn't use vanguard/eac/eaac/be

[2025-09-21 14:31] daax: [replying to lennard.cpp: "thanks, ill definitely have a look. are carckme's ..."]
yeah why not, it can’t hurt. crackme’s and ctfs are a great way to learn

[2025-09-21 14:34] daax: [replying to Pepsi: "tbh, my advice is to to *just do it*, grab a game ..."]
ye <@963446691934130197> i think most of the advice you’ll get on the how / good resources is gonna be: just start doin it. no formal path, but ofc feel free to ask questions here or wherever you’re comfortable. an example unrelated to games, patching the winrar nag screen out was a good exercise to teach my brother basics and he dove in himself after knowing the tools to try, he then advanced organically to unlocking steam achievements and making more advanced stuff in CE for games he likes to play (like No Mans Sky or whatever, they didn’t have ACs) — timeline for this was about a year (so you don’t feel like you need to rush).

[2025-09-21 14:35] Brit: Teaching through practical things is an awesome way to get people started, I taught a friend by helping him crack one of those poker GTO calculators (he did have a legitimate license too)

[2025-09-21 14:36] DirtySecreT: [replying to Brit: "Teaching through practical things is an awesome wa..."]
that's how i learned too lol

[2025-09-21 14:37] DirtySecreT: not poker but

[2025-09-21 14:37] lennard.cpp: should i focus more on understanding asm or can i get away witht he decompiler?

[2025-09-21 14:38] DirtySecreT: [replying to lennard.cpp: "should i focus more on understanding asm or can i ..."]
i think u should be doing both

[2025-09-21 14:38] lennard.cpp: [replying to DirtySecreT: "i think u should be doing both"]
ok

[2025-09-21 14:38] DirtySecreT: decompiler will lie sometimes and u need to be able to verify

[2025-09-21 14:38] Pepsi: i have started with cs:s memhacks, later external cheats, then went internal in csgo (at that time vac actually didn't do much),
then moved on to PUBG with Battleye using the infamous lsass.exe memes and when that got mitigated i moved on to kernel stuff,
which eventually led me to start doing hypervisor stuff

[2025-09-21 14:39] DirtySecreT: [replying to Pepsi: "i have started with cs:s memhacks, later external ..."]
race to the bottom l0l

[2025-09-21 14:40] Pepsi: [replying to DirtySecreT: "race to the bottom l0l"]
what do you mean, i don't do p2c for a living

[2025-09-21 14:40] DirtySecreT: i will say i think it's ok for people to ask if they dont know a tool or something. knowing the right tools is half the battle

[2025-09-21 14:40] DirtySecreT: [replying to Pepsi: "what do you mean, i don't do p2c for a living"]
oh i wasnt meaning that

[2025-09-21 14:41] DirtySecreT: i meant its like a normal flow. some people here mentioned had a similar path

[2025-09-21 14:41] Brit: [replying to Pepsi: "i have started with cs:s memhacks, later external ..."]
Insane how long lsass memery lasted

[2025-09-21 14:41] daax: [replying to Brit: "Insane how long lsass memery lasted"]
20mb updates every 30 minutes. the peak of PUBG

[2025-09-21 14:42] Brit: Real

[2025-09-21 14:42] Brit: Noclipping through terrain features to be under the last circle

[2025-09-21 14:42] Brit: <:topkek:904522829616263178>

[2025-09-21 14:42] Brit: Fun times

[2025-09-21 14:45] Pepsi: https://streamable.com/tdtoho
[Embed: Watch we see u (1080p_60fps_H264-128kbit_AAC) | Streamable]
Watch "we see u (1080p_60fps_H264-128kbit_AAC)" on Streamable.

[2025-09-21 14:49] daax: [replying to Pepsi: "https://streamable.com/tdtoho"]
harry potter vehicles was too fun

[2025-09-21 14:50] daax: for a while in EFT there was no z-clipping so you could just walk normally in mid air lol made the sound like ground

[2025-09-21 14:52] daax: streamable no like mp4?

[2025-09-21 14:52] daax: wtf

[2025-09-21 14:54] daax: https://streamable.com/tyxsvp
[Embed: Watch promo4 | Streamable]
Watch "promo4" on Streamable.

[2025-09-21 15:02] daax: was 2 ways iirc. one was to hook fnc in createshot and check if ricochet / force it to ricochet. you could shoot through walls as well by writing the ballistic objects position though both got you banned pretty quickly. was better to just hook oncreateshot and do normal things like player prediction, bullet velo mods, silent aim, etc.

[2025-09-21 15:02] daax: these types are cool but if you use them you're gonna get slapped with a ban fast

[2025-09-21 15:04] daax: the best feature to have wasn't even aim imo, just esp + stat maxing so you could mad dash to the area you wanted and be there before anyone else without having to speedhack the traditional way.

[2025-09-21 15:07] luci4: [replying to Pepsi: "i have started with cs:s memhacks, later external ..."]
lsass meme?

[2025-09-21 15:08] luci4: (0 experience with anti-cheats, I am curious though)

[2025-09-21 15:12] Brit: [replying to luci4: "lsass meme?"]
Lsass has vmread and vmwrite

[2025-09-21 15:12] Pepsi: [replying to luci4: "lsass meme?"]
Local Security Authority Subsystem Service, lsass meme was long ago patched kernel anticheat bypass
this windows service process has read/write capable process handles to most processes,
you could inject into that process, scan for the handle and use it to access the game process

[2025-09-21 15:13] Brit: They lower the handle access mask and hook the read and write sites

[2025-09-21 15:13] Brit: To prevent you from using the handle now

[2025-09-21 15:15] luci4: [replying to Pepsi: "Local Security Authority Subsystem Service, lsass ..."]
TIL, I didn't know LSASS did that

[2025-09-21 15:16] Pepsi: [replying to Brit: "They lower the handle access mask and hook the rea..."]
mfw this was 7 years ago

[2025-09-21 15:16] Pepsi: <https://www.youtube.com/watch?v=yHNLPp4V9nU>

[2025-09-21 15:17] Brit: I recall spoonfeeding the lsass meme to folks about a decade ago so that tracks

[2025-09-21 15:17] Brit: 2017 nvm

[2025-09-21 15:19] toasts: people using bedaisy to r/w into eac protected processes was also a good one

[2025-09-21 15:19] Brit: Absolute meme

[2025-09-21 18:32] snowua: [replying to daax: "https://streamable.com/tyxsvp"]
i remember this. fall of edge = false 🤣

[2025-09-21 18:47] Brit: gliding around underground

[2025-09-21 18:47] Brit: <:mmmm:904523247205351454>

[2025-09-21 22:05] abu: I remember the name of this lol

[2025-09-21 22:05] abu: promoted everywhere