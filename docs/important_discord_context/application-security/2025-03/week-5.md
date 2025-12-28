# March 2025 - Week 5
# Channel: #application-security
# Messages: 410

[2025-03-24 02:20] C2: does anyone have any cool ideas to protect memory from usermode with low overhead?

[2025-03-24 02:21] C2: I would do like page guarding and stuff but to some extent this wouldnt help, I also saw an AI suggest intel MPK

[2025-03-24 02:22] C2: I did also see a method of intentionally corrupting instructions to cause illegal instruction violations

[2025-03-24 02:22] C2: but I feel like this one is too easy to defeat by itself

[2025-03-24 02:35] Torph: from other processes or within the process? can't you mark it `PAGE_NOACCESS` or something?

[2025-03-24 02:36] C2: [replying to Torph: "from other processes or within the process? can't ..."]
yeah but imagine I do page no access on a page that gets accessed a ton

[2025-03-24 02:36] C2: then the overhead will be pretty high

[2025-03-24 02:38] Torph: I guess I would try to move whatever untrusted code that's involved here out of process, or move whatever code that handles the sensitive data out of process

[2025-03-24 02:38] C2: i guess I could encrypt the memory, then when something tries to access with page no access I check the rip then place a breakpoint on that instruction trying to access

[2025-03-24 02:38] C2: that way when it tries to access it again I can decrypt faster

[2025-03-24 02:39] Atom: <@1271030593534169138> cant do it on the entire page, but hwbp

[2025-03-24 02:40] C2: [replying to Atom: "<@1271030593534169138> cant do it on the entire pa..."]
yeah I thought of this but there is only like 4 I can place

[2025-03-24 02:40] Atom: Ye

[2025-03-24 02:40] C2: i could resort to this but I would like a more creative solution

[2025-03-24 02:40] Atom: whats the purpose?

[2025-03-24 02:40] Torph: is constantly encrypting and decrypting much better than constantly changing page permissions? maybe you should write something that just changes the page permissions in a tight loop, then take the average time and see if this will actually be a performance problem. and if it turns out it is, do the same with the encrypt/decrypt the same number of times and see if it's faster

[2025-03-24 02:42] C2: uhh let me put this into words

[2025-03-24 02:42] C2: hope you dont mind a voice note rq

[2025-03-24 02:43] C2: 
[Attachments: voice-message.ogg]

[2025-03-24 02:44] Atom: nah its the same time

[2025-03-24 02:44] C2: ah ok

[2025-03-24 02:44] Atom: as read/write on that page

[2025-03-24 02:44] C2: also reapplying the page permission might be a problem

[2025-03-24 02:44] C2: I could single step but that is slow as fuck from what I was testing

[2025-03-24 02:44] Atom: you need to create a section for that VA, and have a mirrored page

[2025-03-24 02:45] C2: [replying to Atom: "you need to create a section for that VA, and have..."]
could you explain further

[2025-03-24 02:45] Atom: so you dont have race conditions when encrypting/decrypting

[2025-03-24 02:45] C2: give me 1 sec though gotta run to the car park and pick up a friend

[2025-03-24 02:45] C2: hold on

[2025-03-24 02:46] Atom: aka 2 pages:
One is what is being executed and has PAGE_NOACCESS protection
Two is a always RW page protection which shares the same physical page as page one, so on access violations you can decrypt page two, and it will be decrypted on page one and then you need to set page one's protect to PAGE_READ_EXECUTE

[2025-03-24 02:47] Atom: and then you can have some sort of timer to re-encrypt the page again by setting page one to NO_ACCESS and encrypting page two

[2025-03-24 02:49] Atom: I suppose you're trying to reimplement packman/hyperion style page encryption

[2025-03-24 02:51] C2: [replying to Atom: "I suppose you're trying to reimplement packman/hyp..."]
nah

[2025-03-24 02:51] C2: you can use page no access for this

[2025-03-24 02:51] C2: someone made a github on it

[2025-03-24 02:51] C2: the overhead seems negligible actually

[2025-03-24 02:52] C2: [replying to Atom: "and then you can have some sort of timer to re-enc..."]
a timer might not be ideal

[2025-03-24 02:52] Atom: [replying to C2: "you can use page no access for this"]
what are you trying to make, I dont understand

[2025-03-24 02:52] C2: what could work is somehow making a second page thenredirect it

[2025-03-24 02:52] C2: redirect the rip upon violation

[2025-03-24 02:53] C2: to the fixed page

[2025-03-24 02:53] C2: but then solving for the context differences and stuff is annoying

[2025-03-24 02:53] Atom: thats call indirection

[2025-03-24 02:53] C2: [replying to Atom: "what are you trying to make, I dont understand"]
just having fun trying to come up with creative methods to protect pages, nothing specific right now

[2025-03-24 02:54] Atom: yes but against what

[2025-03-24 02:54] C2: cheats I guess

[2025-03-24 02:54] C2: or anything I dont want to access it

[2025-03-24 02:54] Atom: [replying to Atom: "aka 2 pages:
One is what is being executed and has..."]
then you need to do this

[2025-03-24 02:54] C2: obviously they could leverage gadgets and shit

[2025-03-24 02:54] C2: but yeah idk i will look at what is accessing and stuff and maybe do some stack watching we will see

[2025-03-24 02:55] C2: [replying to Atom: "then you need to do this"]
isnt the timer a problem though

[2025-03-24 02:55] Atom: PAGE_NOACCESS isnt going to be enough with no mirror page to decrypt, if another thread tries executing the same page while under exception for decrypting, its going to blow

[2025-03-24 02:55] Atom: no?

[2025-03-24 02:55] C2: couldnt a cheat snipe the page if there is a race condition

[2025-03-24 02:55] Atom: yes nothing you can do against that

[2025-03-24 02:55] C2: [replying to Atom: "PAGE_NOACCESS isnt going to be enough with no mirr..."]
yeah true

[2025-03-24 02:56] C2: [replying to Atom: "yes nothing you can do against that"]
kinda renders it useless if sniping is on the table

[2025-03-24 02:56] C2: maybe not USELESS

[2025-03-24 02:56] C2: but to some degree not ideal

[2025-03-24 02:56] C2: xD

[2025-03-24 02:56] Atom: someone can always dump the game while running it, and have most things

[2025-03-24 02:56] C2: yeah

[2025-03-24 02:56] Atom: you cant prevent it

[2025-03-24 02:56] C2: there has to be some creative methods to protect memory to some degree though just gotta think of something that is out the box

[2025-03-24 02:57] Atom: thats why games introduce object encryption to make people's lives really messy

[2025-03-24 02:57] Atom: nothing from usermode

[2025-03-24 02:57] C2: yeah but those are pretty easy to defeat

[2025-03-24 02:57] C2: and virt can introduce overhead

[2025-03-24 02:57] C2: idk I will keep digging around and see what I can come up with

[2025-03-24 02:57] Atom: no, its because the complexity of the complexity of the encryption

[2025-03-24 02:58] Atom: most game devs just do simple xors, rol/rors and its not enough

[2025-03-24 02:58] C2: yeah

[2025-03-24 02:58] Atom: but the more complex you go, the more overhead

[2025-03-24 02:58] C2: well higher encryption might cause issues with overhead too

[2025-03-24 02:58] C2: yeah

[2025-03-24 02:58] C2: I guess xor is fine if you do a good job of hiding the key

[2025-03-24 02:58] C2: and introduce somerandomness

[2025-03-24 02:58] C2: for the key

[2025-03-24 02:58] Atom: ye so re-encrypting the object

[2025-03-24 02:58] C2: yeah

[2025-03-24 02:59] Atom: ye that wouldnt do much tbh, same issue

[2025-03-24 02:59] C2: but if the instructions that decrypt it arent protected to a good degree its basically useless

[2025-03-24 02:59] Atom: you need to go kernel to do any sort of proper protection for games

[2025-03-24 02:59] C2: since someone is just gonna breakpoint after decryption and take the value and find the xor diff

[2025-03-24 02:59] C2: [replying to Atom: "you need to go kernel to do any sort of proper pro..."]
yeah idk I am more interested in creative methods to do stuff from usermode

[2025-03-24 02:59] C2: I believe there is creative methods that exist

[2025-03-24 02:59] C2: i am just brainstorming

[2025-03-24 03:00] C2: if it is fruitless then whatever

[2025-03-24 03:00] Atom: you're going to be running in circles

[2025-03-24 03:00] C2: i dont think so, but I will find out the hard way if so

[2025-03-24 03:00] Atom: sure, just letting ya know before you throw away days or weeks worth of time

[2025-03-24 03:00] C2: [replying to Atom: "sure, just letting ya know before you throw away d..."]
yeah but you could also be wrong

[2025-03-24 03:00] C2: and I have time to kill

[2025-03-24 03:01] C2: and if something comes out of it, neat

[2025-03-24 03:01] C2: if not whatever

[2025-03-24 03:01] C2: appreciate the help anyway

[2025-03-24 03:01] Atom: you making a game?

[2025-03-24 03:01] C2: nah, just bored of making cheats

[2025-03-24 03:01] C2: wanna work on some prevention methods

[2025-03-24 03:02] Atom: imo if you're really interested you should look at what EAC and Vanguard have done

[2025-03-24 03:03] Atom: both use custom page tables for the game

[2025-03-24 03:03] C2: [replying to Atom: "both use custom page tables for the game"]
do you have a blog post on hand

[2025-03-24 03:12] Atom: [replying to C2: "do you have a blog post on hand"]
dont have one, but a google search would work

[2025-03-24 03:15] Atom: https://reversing.info/posts/guardedregions/
[Embed: In-depth analysis on Valorant's Guarded Regions]
In this post, we will analyze how Vanguard attempts to keep away bad actors by utilizing a simple yet brutally strong method

[2025-03-24 03:16] C2: oh thanks my bad

[2025-03-24 03:18] Atom: not sure if this works under HVCI though

[2025-03-24 03:19] C2: its cool but unfortunately not ideal as it is from kernel

[2025-03-24 03:19] C2: and yeah I am pretty sure it doesnt work under HVCI

[2025-03-24 03:39] juan diego: me and a friend worked on a scheme kinda like this a while ago

[2025-03-24 03:39] juan diego: but we didnt do encryption we did hashing bc we wanted to prevent writes to our memory

[2025-03-24 03:41] juan diego: it was an ac type project performance was fine because we only made the page R on a timer rather than immediately after each write

[2025-03-24 03:44] juan diego: when a thread attempts a write perform checks on the thread

[2025-03-24 03:44] juan diego: ensure the memory has not unexpectedly changed since when the page was last made R only

[2025-03-24 03:46] C2: hashing is good to prevent writes but

[2025-03-24 03:46] C2: if it is to prevent hooking and such

[2025-03-24 03:46] C2: people can just redirect in other ways

[2025-03-24 03:48] C2: guess ill play around with page guard and stuff, kinda sucks because of the sniping element but whatever

[2025-03-27 05:10] Matti: [replying to C2: "there has to be some creative methods to protect m..."]
enclaves do this

[2025-03-27 05:11] Matti: e.g.Intel(R) SGX(TM) (now defunct/deprecated)

[2025-03-27 05:11] Matti: arguably AMD SEV-ES also does this if you assume the attacker is the hypervisor

[2025-03-27 05:12] Matti: and there's MPKs, but I haven't read up on them at all yet

[2025-03-27 05:12] C2: I was hoping what daax showed me with the unmapped pages testing would help me detect external tools scanning memory

[2025-03-27 05:12] C2: unfortunately not

[2025-03-27 05:12] C2: [replying to Matti: "and there's MPKs, but I haven't read up on them at..."]
MPKs

[2025-03-27 05:12] C2: I will take a look

[2025-03-27 05:12] C2: looking for creative methods from usermode

[2025-03-27 05:13] C2: while being internal in the app

[2025-03-27 05:13] Matti: o

[2025-03-27 05:13] Matti: idt you can set memory protection keys from UM

[2025-03-27 05:13] C2: sad times

[2025-03-27 05:13] Lyssa: very true

[2025-03-27 05:13] C2: almost anything I think of is kinda useless

[2025-03-27 05:14] Lyssa: there's plenty of dumbass solutions if there's no smart solutions

[2025-03-27 05:14] Lyssa: probably

[2025-03-27 05:14] C2: if I encrypt memory and then decrypt on the fly

[2025-03-27 05:14] C2: but then someone could just snipe the memory

[2025-03-27 05:14] C2: or hook my eh and fix it manually

[2025-03-27 05:14] C2: never pass it down

[2025-03-27 05:14] C2: not ideal either way

[2025-03-27 05:14] C2: [replying to Lyssa: "there's plenty of dumbass solutions if there's no ..."]
wise words

[2025-03-27 05:15] Matti: it's not dumb if it's a solution

[2025-03-27 05:16] Matti: around here we prefer the more neutral term "not ideally aesthetically pleasing"

[2025-03-27 05:16] C2: well not even that, just the fact it has a big issue of being sniped

[2025-03-27 05:16] Matti: yes

[2025-03-27 05:16] Matti: it does

[2025-03-27 05:16] C2: ideally it would be cool to redrirect the rip at runtime to a relocated page but even then they could just watch the rip

[2025-03-27 05:17] Matti: which begs the question if it's really a solution.... but maybe I'm getting too philosophical here

[2025-03-27 05:18] C2: [replying to Matti: "which begs the question if it's really a solution...."]
true

[2025-03-27 05:18] C2: good arguments for both sides

[2025-03-27 05:18] C2: its a hack if anything maybe

[2025-03-27 05:22] Matti: in general I think you're just gonna have to live with the fact that something in UM can't do this as well as a kernel mode solution, if kernel is not an option or you just want to do it in UM for the sake of it, which isn't wrong

[2025-03-27 05:22] C2: [replying to Matti: "in general I think you're just gonna have to live ..."]
UM is just more fun in my opinion, I am sure there is a lot I can do in kernel

[2025-03-27 05:23] C2: it is just fun to get creative and think of ideas, forces you to think outside the box

[2025-03-27 05:23] C2: like the post that daax showed by watching if memory gets paged

[2025-03-27 05:23] Matti: but valorant's guarded regions (probably the best generically usable approach) plus all of the things that came to mind for me require kernel  mode

[2025-03-27 05:23] C2: or simply placing certain breakpoints and maybe even some basic custom vms

[2025-03-27 05:23] Matti: [replying to C2: "it is just fun to get creative and think of ideas,..."]
yeah I get what you mean

[2025-03-27 05:23] C2: [replying to Matti: "but valorant's guarded regions (probably the best ..."]
I did like hyperion's approach

[2025-03-27 05:24] C2: though I am not 100% sure about the overhead on it

[2025-03-27 05:24] C2: I would have to test more

[2025-03-27 05:24] Matti: I have the same thing with scyllahide and titanhide, which I both (used to...) work on

[2025-03-27 05:24] Matti: not so much lately

[2025-03-27 05:25] Matti: same goal, but there is some very uh.... creative code in scyllahide as a result of the heinous user mode restriction

[2025-03-27 05:25] Matti: yes, creative, not giant hacks

[2025-03-27 05:26] C2: LOL

[2025-03-27 05:27] C2: you are saying scyllahide has some usermode techniques you thought of to protect memory inside of it?

[2025-03-27 05:27] C2: just to confirm before I go read it

[2025-03-27 05:27] Matti: no sorry, poor phrasing

[2025-03-27 05:27] Matti: scyllahide and titanhide have the same goal

[2025-03-27 05:28] Matti: not memory protection, but antidebug protection

[2025-03-27 05:28] Matti: but one is a kernel mode driver and the other is a user mode debugger plugin

[2025-03-27 05:29] C2: ahh okay

[2025-03-27 05:30] Matti: https://github.com/x64dbg/ScyllaHide/commit/5ad4cb7 personal all time favourite if I have to be honest
[Embed: Obsidium please (2/3) · x64dbg/ScyllaHide@5ad4cb7]
- Write WOW64 x64 transition detours as far jmps

Obsidium checks syscalls for hooks, or at least it tries to. But if something only kinda looks like the original it&#39;s good enough. And thus...

[2025-03-27 05:31] Matti: TLDR: the hook looks like a transition to native x64 code, but uses the 32 bit segment selector which is the one byte that obsidium fails to check

[2025-03-27 05:32] Matti: so the far jmp is a transition into the already currently executing mode

[2025-03-27 05:33] Matti: oh and the reason it needs to look like a transition to x64 code is because that is the function being hooked here

[2025-03-27 05:34] Matti: which obsidium checks for

[2025-03-27 05:34] Matti: or tries to

[2025-03-27 05:35] Matti: since all syscalls in wow64 go through this far jmp, this essentially hooks all syscallls from one location

[2025-03-27 05:37] Matti: you can manually perform native x64 syscalls from wow64 code btw, just to be clear

[2025-03-27 05:38] Matti: it's just that no commercial protectors were doing this at the time I wrote the commit

[2025-03-27 05:38] Matti: vmprotect does now do this,unsure if there are others

[2025-03-27 05:38] Matti: obsidium is still trash

[2025-03-27 05:40] C2: [replying to Matti: "obsidium is still trash"]
the website looks like that too

[2025-03-27 05:41] C2: [replying to Matti: "which obsidium checks for"]
so you made it look like not a hook so obsidium wouldnt detect it I am guessing

[2025-03-27 05:41] Matti: ya, correct

[2025-03-27 05:41] Matti: it thinks this is the original ntdll code

[2025-03-27 05:41] C2: that is creative as fuck this is what I am talking about

[2025-03-27 05:41] C2: I love thinking of these little things

[2025-03-27 05:41] Matti: or wow64.dll, whichever, I forget

[2025-03-27 05:42] Matti: yeah but do keep in mind obsidium is like the slow kid at school

[2025-03-27 05:42] C2: it just looks outdated as fuck

[2025-03-27 05:43] C2: vmp seems to be the king rn

[2025-03-27 05:43] C2: for a balance of performance and safety

[2025-03-27 05:43] James: im hurt...

[2025-03-27 05:43] Matti: anti-debug wise yes

[2025-03-27 05:43] C2: themida seems to just have issues on issues when it comes to performance and stability

[2025-03-27 05:43] C2: plus I have had no issues with vmp + llvm obfuscator

[2025-03-27 05:43] C2: which is cool

[2025-03-27 05:44] Matti: protection wise I hear different things and I'm not knowledgeable enough to have an opinion of my own on this

[2025-03-27 05:44] C2: [replying to Matti: "protection wise I hear different things and I'm no..."]
yeah I just know what I have experienced using them both, that said though I did use the crack for themida vs the paid vmp

[2025-03-27 05:44] C2: so theres a good chance themida is just great

[2025-03-27 05:44] C2: if you pay for it

[2025-03-27 05:44] C2: <:CZ_shrug:1082248436402884658>

[2025-03-27 05:45] Matti: I do have a certain disdain for themida simply  due to the fact that it doesn't support PE files other than windows user mode shit

[2025-03-27 05:45] C2: I do not think many people require it outside of this though

[2025-03-27 05:45] Matti: unlike vmprotect which deals with drivers fine

[2025-03-27 05:45] C2: you would be one of the few I imagine but yeah

[2025-03-27 05:45] C2: VMP is goated for that if so

[2025-03-27 05:45] Matti: yeah, am aware

[2025-03-27 05:45] C2: they seem to just support basically anything

[2025-03-27 05:45] C2: I was surpised it worked with llvm obfuscator

[2025-03-27 05:45] C2: with all the options on

[2025-03-27 05:45] C2: very cool

[2025-03-27 05:45] C2: I dont use all the options but yeah

[2025-03-27 05:46] Matti: I just think I wouldn't be able to sell a product as a code protector if it had such a gaping limitation (for admittedly a tiny minority of users... but no less gaping)

[2025-03-27 05:46] Matti: btw, oreans code virtualizer does nnt have this restriction

[2025-03-27 05:47] Matti: I'm unsure of the reason for the distinction

[2025-03-27 05:47] C2: interesting

[2025-03-27 05:47] C2: you think it is better than vmp

[2025-03-27 05:48] C2: though I doubt it is much of a difference

[2025-03-27 05:48] Matti: [replying to C2: "you think it is better than vmp"]
at what?

[2025-03-27 05:48] C2: like I feel like if someone is going to go out of their way to devirt your shit I doubt this or that is gonna help

[2025-03-27 05:48] C2: [replying to Matti: "at what?"]
anti reverse engineering

[2025-03-27 05:48] Matti: no idea

[2025-03-27 05:48] James: themida is actually more difficult to devirtualize in several places BECAUSE of semantical innacuracies. imagine if your add instruction simply didn't actually perform proper addition...

[2025-03-27 05:48] C2: [replying to James: "themida is actually more difficult to devirtualize..."]
that would be pain in the ass

[2025-03-27 05:49] Matti: but yes, ^ seems to be the general opinion

[2025-03-27 05:49] James: ofc however, good luck with SEH code, code with computed branches, code that doesn't follow windows calling conventions(compiler outlining).

[2025-03-27 05:50] James: so there's that.

[2025-03-27 05:50] C2: pros and cons everywhere I guess

[2025-03-27 05:50] James: but then with VMP, you get EITHER CET or SEH, not both.

[2025-03-27 05:50] C2: [replying to C2: "like I feel like if someone is going to go out of ..."]
still think this applies though

[2025-03-27 05:51] C2: there will be a fine amount of people who can devirt vmp but not themida given the willpower

[2025-03-27 05:51] C2: I think anyway

[2025-03-27 05:51] C2: I could be horribly wrong

[2025-03-27 05:51] C2: but that is my belief

[2025-03-27 05:51] James: yeah i guess pick one based on your target reverser audience right?

[2025-03-27 05:52] James: if your target is a gov with a billion citizens to help them, not much is going to help.

[2025-03-27 05:52] C2: since I am making an AC I just made it so encryption keys are inlined using defines so I can easily change the key and revirt and upload again

[2025-03-27 05:52] C2: so that reversing it all to emulate the network would be more of a pain

[2025-03-27 05:53] C2: easier for me to fix than for you to reverse

[2025-03-27 05:53] C2: is the idea anyway

[2025-03-27 05:53] C2: [replying to James: "if your target is a gov with a billion citizens to..."]
yeah for sure LOL

[2025-03-27 05:53] C2: just dont piss off the wrong people 🙏  even though not always an option

[2025-03-27 05:53] C2: xerox's stuff?

[2025-03-27 05:53] C2: yeah I did ask him but he said he isnt selling it yet

[2025-03-27 05:54] C2: oh

[2025-03-27 05:54] C2: i guess its out now

[2025-03-27 05:54] C2: this was awhile ago actually

[2025-03-27 05:55] C2: I shall go check it out

[2025-03-27 05:55] C2: yeah I get that

[2025-03-27 05:56] C2: I am always open to trying different things

[2025-03-27 05:56] C2: pricing is whack though

[2025-03-27 05:56] C2: but eh I guess

[2025-03-27 05:56] C2: small team

[2025-03-27 05:56] C2: [replying to C2: "pricing is whack though"]
compared to the normal software

[2025-03-27 05:57] C2: also there is the worry that your software could make the protected bin look like a virus

[2025-03-27 05:57] James: the front facing part hasn't been getting much attention as of late.

[2025-03-27 05:57] C2: which then would only appeal to some people with certs

[2025-03-27 05:57] C2: to fix that maybe

[2025-03-27 05:58] C2: IV certs require building trust per binary

[2025-03-27 05:58] C2: so if your protected binary produces a false positive with popular AVs this would be a deal breaker for a lot of people who just want to protect their own code

[2025-03-27 05:58] C2: for legit people anyway

[2025-03-27 05:58] C2: I imagine cheat devs dont give a fuck

[2025-03-27 05:59] C2: but if cheat devs are paying 100 a month for protection on a binary they prob fucked up somewhere

[2025-03-27 05:59] James: i've always found the cert system idiotic...

[2025-03-27 05:59] C2: same

[2025-03-27 05:59] James: EVs are beyond easy to get

[2025-03-27 05:59] C2: yeah exactly

[2025-03-27 05:59] C2: its just money

[2025-03-27 06:00] James: and not a lot of money either mind you

[2025-03-27 06:00] C2: and on top of that its easy to make a backdoor fud

[2025-03-27 06:00] C2: if you write it yourself

[2025-03-27 06:00] C2: criminals have enough money to pay for an EV

[2025-03-27 06:00] C2: smart people have enough knowledge to write something undetected

[2025-03-27 06:00] C2: the people in the middle are just gonna be fucked

[2025-03-27 06:00] C2: by a system that is just stupid

[2025-03-27 06:01] C2: itd be better if the trust building system existed on personal certs, and NOT on the bin itself

[2025-03-27 06:01] C2: then it would actually be worth it for small devs, intial false flags can be okay since over time the cert will slowly become more trusted

[2025-03-27 06:01] C2: and if anything is caught signed with it off the shelf malware just blacklist the cert?

[2025-03-27 06:02] C2: idk I dont think its rocket science to improve the shit system that is certs but maybe I am missing something

[2025-03-27 06:05] Matti: it's not, web of trust is proven working technology

[2025-03-27 06:06] Matti: but the CA system is not going away, it's a racket that generates far too much income for the companies that are in it

[2025-03-27 08:04] idkhidden: [replying to C2: "vmp seems to be the king rn"]

[Attachments: IMG_0033.png]

[2025-03-27 18:35] oopsies: [replying to C2: "smart people have enough knowledge to write someth..."]
That is easier than you think. The problem is includes the behavior of the application. Take any generic C# infostealer. They have all these traits:
Strings (or similar string encryption, usually one function that doesnt change)
Type names
Libraries used (DotNetZip for example)
PE Metadata (far more than most realize)
Protocol (includes data serialization too)

Most obfuscators are barely modified. You can literally change/encrypt strings and fork some basic .NET obfuscator. Now delivery is a different beast. I dont want to guide anyone so I wont go there. EV certs are excessive 99.9% of the time.

[2025-03-27 18:38] oopsies: [replying to C2: "if you write it yourself"]
This has been done for decades. At the end of the day, a stealer needs a sqlite, crypto, and networking library. You can simply fork one and figure out how to get the email through (not marked as spam/suspicious) and get it past windows defender.

A lot of people simply re-invent the wheel when they don't have to.

[2025-03-27 18:45] oopsies: Anyways I would get researching and stay away from trying to make malware or an AC. these texts can incriminate you

[2025-03-27 18:45] oopsies: regardless of your intent

[2025-03-27 19:00] Brit: Aptly chosen username

[2025-03-27 21:51] oopsies: [replying to Brit: "Aptly chosen username"]
So it's better to insult him vs help him? How am I larping?

[2025-03-28 13:11] oopsies: [replying to Brit: "Aptly chosen username"]
If I gave him a step by step tutorial, source code, and tips to do this and scale it, you all would probably ban me off this server. Most of you just insult and gatekeep beginners.

What's next? Explain how to setup SMTP servers, get past anti-spam, deal with hosting bans, and target specific people? It gets to the point where you struggle to move and store all the data. I only quit for moral reasons.

[2025-03-28 13:16] emma: [replying to oopsies: "If I gave him a step by step tutorial, source code..."]
lol

[2025-03-28 15:46] James: Lol bro double texted. Big mistake.

[2025-03-28 15:58] daax: [replying to oopsies: "If I gave him a step by step tutorial, source code..."]
Well, yes, explaining how to step-by-step make an infostealer is going to get you banned. Explaining different techniques that are used and/or how to do them without putting them all together into something malicious is different. Most anything can be used for malicious purposes. If your intent is to inform someone how to make something that could be used for malicious purposes, yes, bannable offense. If it’s independent discussions about the different things that go into generic anything from DRM to malware, not really a problem.

The joke about your username, I would hazard to guess is judgement for similar statements like the last paragraph of your message. Nobody cares if you’re top hakkerman or just some guy researching stuff, there is no need to try and prove yourself. That credibility comes from what you share/what you contribute to a discussion. Not whether you claim to have written an infostealer at scale. 12 year olds were doing it 15 years ago to steal ARMA 2 OA keys and resell them on cheat sites. They also collected cc info and passwords to resell on hf. You’re not gonna get any accolades for doing the easiest thing “at scale”. Putting “FULLY UD ARMA CHEAT” on YouTube was sufficient to get thousands of keys and dumps of peoples credentials. That’s scale, but requires very little technical skill.

“I only quit for moral reasons” — namaste sensei, <:hackerman:504389083477573632> — do you also own a katana and threaten your online enemies with BJJ?

There is very little gatekeeping of information, most here are happy to help any one of any level of competence/experience. It’s when you try to perform auto-fellatio that you’ll get clowned on.

[2025-03-28 16:00] daax: Get a grip and join the discussion without trying to jork it over your past endeavors.

[2025-03-28 16:06] Matti: may I add as well, acting interesting without saying anything interesting

[2025-03-28 16:07] Matti: talking about *knowing* interesting things, but not the things

[2025-03-28 16:08] Matti: it generally reminds me of 14 year old girls who claim their brother's nephew's friend knows so and so who is a friend of britney spears

[2025-03-28 16:12] 25d6cfba-b039-4274-8472-2d2527cb: [replying to Matti: "talking about *knowing* interesting things, but no..."]
I'd tell u but its classified 😦

[2025-03-28 16:12] Matti: yeah lmao

[2025-03-28 16:12] Matti: classic

[2025-03-28 16:16] 25d6cfba-b039-4274-8472-2d2527cb: Though it is slightly too common to have things I've heard/seen but not fully remembering the context I got the information from and just having to shut the fuck up to be sure.

[2025-03-28 16:17] Matti: just out-bullshit them

[2025-03-28 16:18] Matti: oh yeah, that thing... I mean I know it's classified but to tell you the truth, it's really not up to scratch anymore compared to the latest tech

[2025-03-28 16:18] Matti: wish I could tell you more

[2025-03-28 16:20] 25d6cfba-b039-4274-8472-2d2527cb: TLP + Chatham house rule == trust me bro

[2025-03-28 16:37] pinefin: [replying to oopsies: "Anyways I would get researching and stay away from..."]
you're somewhat of a hacker, huh?

[2025-03-28 16:37] pinefin: you like hacking?

[2025-03-28 16:38] pinefin: im sensing a little bit of the dunning kruger effect
[Attachments: IMG_1019-1024x653.png]

[2025-03-28 16:40] pinefin: [replying to oopsies: "If I gave him a step by step tutorial, source code..."]
gatekeep beginners? this is a server that explicity says 

```
We define low-quality as anything in the spirit of the following: 
-Intentional attempts to derail conversations by trolling or spamming.
-Repeatedly asking or attempting to answer questions with low effort or vague responses. If you don't know, don't mislead.
Asking if you can ask a question. Of course you can.
-Unwillingness to engage with critique or criticism or answers that you may not agree with. Constructive conversations are helpful and offer different ideas. This is not meant to be an echo chamber.
```

[2025-03-28 16:42] pinefin: ```
We encourage everyone to participate in the project of creating a culture of rationality and critical thinking within the server. Please politely acknowledge when someone has errors in their reasoning. We would like to reduce the presence of cognitive biases, and logical fallacies as much as possible. Please be thoughtful.

```

[2025-03-28 17:06] Deleted User: [replying to daax: "Well, yes, explaining how to step-by-step make an ..."]
you made me search up fellatio

[2025-03-28 17:15] Matti: you didn't learn latin in school? lmao

[2025-03-28 17:15] Matti: I'm sorry but a new rule is that it is absolutely allowed to be elitist about classical education

[2025-03-28 17:16] Matti: when it comes to tech though it's generally just petty and pointless
besides we wouldn't have anyone left because actually knowledgeable people already keep themselves out without help

[2025-03-28 17:32] pinefin: [replying to Matti: "you didn't learn latin in school? lmao"]
yeah no bro i graduated w a 1.2, i took fine arts instead of foreign language

[2025-03-28 17:32] pinefin: i only know english and c++

[2025-03-28 17:32] Matti: > foreign language
singular

[2025-03-28 17:32] Matti: lmao

[2025-03-28 17:33] pinefin: language(s)

[2025-03-28 17:33] pinefin: i would have had to take multiple classes

[2025-03-28 17:33] pinefin: could have been the same language

[2025-03-28 17:33] pinefin: or could have been different ones

[2025-03-28 17:33] Matti: oh well I guess not everyone had 7 different language courses in high school

[2025-03-28 17:33] pinefin: what country are you from

[2025-03-28 17:33] pinefin: that might explain

[2025-03-28 17:33] Matti: NL

[2025-03-28 17:34] pinefin: in the US, there's no other need for another language than spanish (because of immigrants and people who are raised by close-knit mexicans)

[2025-03-28 17:34] pinefin: in my paricular state, its a little rare to see people speak other language

[2025-03-28 17:34] pinefin: my girlfriend took 4 classes of french, and 2 of spanish in school. so it's more of a "do i want to do this"

[2025-03-28 17:34] Matti: yeah, I'm aware.. and for the record I'm kidding of course

[2025-03-28 17:34] Matti: but

[2025-03-28 17:35] Matti: I do think knowing latin and ancient greek is highly underrated

[2025-03-28 17:35] Matti: don't care as much about others

[2025-03-28 17:35] avx: [replying to Matti: "oh well I guess not everyone had 7 different langu..."]
<:chad:1140193652942053396>

[2025-03-28 17:36] 25d6cfba-b039-4274-8472-2d2527cb: Everyone should learn finnish

[2025-03-28 17:36] Matti: man I' mean, I'm good at languages so it was nice enough for me

[2025-03-28 17:36] Matti: but I'd have preferred more maths instead

[2025-03-28 17:37] Matti: since it's generally more useful than knowing languages that aren't english or maybe chinese

[2025-03-28 17:37] pinefin: english, c++ and x86asm are the only languages i need brother...

[2025-03-28 17:37] Matti: like, fuck dutch for example*

[2025-03-28 17:38] Matti: completely useless language

[2025-03-28 17:38] Matti: I honestly think NL should just switch to english to save everyone a fuckload of time

[2025-03-28 17:38] avx: [replying to 25d6cfba-b039-4274-8472-2d2527cb: "Everyone should learn finnish"]
kissa 😼

[2025-03-28 17:38] Matti: my dad disagrees... but ofc he's a dutch teacher so his opinion does not count

[2025-03-28 17:38] avx: thats all i know

[2025-03-28 17:39] 25d6cfba-b039-4274-8472-2d2527cb: I lost any motivation for language studies because of the mandatory swedish we have. Would have preferred to learn russian or smth (similar motivation to english, know your enemy etc.)

[2025-03-28 17:40] Matti: I also still want to learn russian

[2025-03-28 17:40] Matti: because I've only read dostoevsky in tr*nslated form

[2025-03-28 17:41] Matti: but, I'm not super fond of russia in general right now

[2025-03-28 17:41] Matti: another time maybe

[2025-03-28 17:41] Matti: [replying to 25d6cfba-b039-4274-8472-2d2527cb: "I lost any motivation for language studies because..."]
this is why we learn german btw

[2025-03-28 17:41] 25d6cfba-b039-4274-8472-2d2527cb: My interest is/was generally fueled by hatred. Wanting to go where they reside and spread hate.

[2025-03-28 17:42] 25d6cfba-b039-4274-8472-2d2527cb: So hating russia is no problem

[2025-03-28 17:42] Matti: idk... to me it's a dead language, just like latin and ancient greek

[2025-03-28 17:43] 25d6cfba-b039-4274-8472-2d2527cb: I used to play mobas and cs so it was very much used.

[2025-03-28 17:43] Matti: once used by a society, now only outcasts living on the edge of the world

[2025-03-28 17:43] Matti: but unlike latin and ancient greek, russian is not the mother of every western european language

[2025-03-28 17:44] Matti: only in vassal states was russian ever used

[2025-03-28 17:44] 25d6cfba-b039-4274-8472-2d2527cb: After we take what is rightfully ours finnish will be booming. From the eastern border all the way to chinas will belong to us.

[2025-03-28 17:45] Azrael: [replying to 25d6cfba-b039-4274-8472-2d2527cb: "I lost any motivation for language studies because..."]
Struggling with Swedish?

[2025-03-28 17:46] Azrael: It can be quite a challenge, wouldn’t necessarily say that you need to learn all of the grammatical types.

[2025-03-28 17:47] Azrael: Was it baseline Swedish or “advanced” Swedish?

[2025-03-28 17:47] 25d6cfba-b039-4274-8472-2d2527cb: [replying to Azrael: "Struggling with Swedish?"]
Just so unnecessary and unused that I tend to forget everything. So not necessarily struggling with learning rather just retaining useless skills.

[2025-03-28 17:47] Azrael: Unused :(

[2025-03-28 17:48] Azrael: I know a couple of guys from neighboring countries that know Swedish quite well.

[2025-03-28 17:48] 25d6cfba-b039-4274-8472-2d2527cb: Ye technically I need to know it for my job but in reality haven't needed it once in close to 4 years. Dread the day I actually need to use it lol.

[2025-03-28 17:49] Azrael: Are you Finnish?

[2025-03-28 17:49] 25d6cfba-b039-4274-8472-2d2527cb: Yup

[2025-03-28 17:49] Matti: lol

[2025-03-28 17:49] Matti: how'd you guess

[2025-03-28 17:49] Azrael: Pure luck.

[2025-03-28 17:50] Matti: could it have been his innate state of hatred for all of russia

[2025-03-28 17:50] Azrael: [replying to 25d6cfba-b039-4274-8472-2d2527cb: "Yup"]
Speaking from minimal experience, but from my perspective you generally don’t need to understand Swedish or Sami unless you live close to the border.

[2025-03-28 17:51] Azrael: Could be wrong, but that’s what I’ve heard from a couple of my friends.

[2025-03-28 17:51] 25d6cfba-b039-4274-8472-2d2527cb: [replying to Azrael: "Speaking from minimal experience, but from my pers..."]
Yup except in job requirements and having school grades to prove it. Like said, never _actually_ needed it.

[2025-03-28 17:52] Brit: [replying to Matti: "could it have been his innate state of hatred for ..."]
give a Finnish man a forest some snow and pervitin, and you get dead soviets out, it's frankly efficient and economical

[2025-03-28 17:52] Matti: yea

[2025-03-28 17:52] Matti: it's like turks and greeks

[2025-03-28 17:52] Matti: mutual love

[2025-03-28 17:53] 25d6cfba-b039-4274-8472-2d2527cb: Most effective nato rearming plan would be force finns to breed more so u have like 50mil of em next to russia

[2025-03-28 17:53] Matti: although in the case of finland I do think the finnish side is definitely better at hating the other

[2025-03-28 17:54] Matti: greeks and turks REALLy hate each other

[2025-03-28 17:54] Brit: [replying to 25d6cfba-b039-4274-8472-2d2527cb: "Most effective nato rearming plan would be force f..."]
Orbann birther plan but for finland

[2025-03-28 18:16] Deleted User: [replying to Matti: "you didn't learn latin in school? lmao"]
no

[2025-03-28 18:16] Matti: well, naturally

[2025-03-28 18:16] Matti: that was one of them rhetorical questions

[2025-03-28 18:17] Deleted User: oh

[2025-03-28 18:18] Matti: it's kinda interesting that fellatio is the term that survived, while the greeks were the ones who had a word for anything in sex ever invented

[2025-03-28 18:19] Deleted User: [replying to Matti: "it's kinda interesting that fellatio is the term t..."]
freeks xd

[2025-03-28 18:19] Matti: and ofc they famously invented buttfucking*

*idk if that's actually true

[2025-03-28 18:20] Matti: [replying to Matti: "and ofc they famously invented buttfucking*

*idk ..."]
in fact it seems highly implausible lmao

[2025-03-28 18:20] Matti: but it's funny to credit them with it nevertheless

[2025-03-28 18:21] Deleted User: [replying to Matti: "in fact it seems highly implausible lmao"]
probably existed as long as humans did

[2025-03-28 18:21] Matti: and before

[2025-03-28 18:22] Matti: buttfucking exists in like every species with a penis and a butt or butt-like device

[2025-03-28 18:22] Matti: and homosexuality exists in ?every? species

[2025-03-28 18:22] pinefin: wait

[2025-03-28 18:22] Matti: inb4 an actual biologist comes in and nukes my facts

[2025-03-28 18:22] pinefin: matti how the fuck did we get here from foreign language

[2025-03-28 18:23] Deleted User: this is all related to application security

[2025-03-28 18:23] pinefin: oh ok

[2025-03-28 18:23] pinefin: carry on

[2025-03-28 18:23] Matti: well it's not really I suppose, but anything related to ancient greek scoiety is related to buttfucking

[2025-03-28 18:23] Matti: so that is how we got here

[2025-03-28 20:59] daax: [replying to Deleted User: "you made me search up fellatio"]
today-u-learned <:Kappa:794707301436358686>

[2025-03-29 03:33] C2: [replying to 25d6cfba-b039-4274-8472-2d2527cb: "I lost any motivation for language studies because..."]
I did Spanish in school and that sucked especially since I couldn't roll my Rs

[2025-03-29 03:34] C2: I tried to learn a bit of swedish because my girlfriend is swedish but honestly it's a lot of work to speak a language that will be next to useless for me

[2025-03-29 13:32] zorky: <#835646666858168320>