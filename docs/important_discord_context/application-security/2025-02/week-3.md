# February 2025 - Week 3
# Channel: #application-security
# Messages: 149

[2025-02-12 05:53] Matti: [replying to .: "1. The code doesn't false flag in any real scenari..."]
Careful mate, if you keep moving the goalposts like this every single time you reply, people might just start to pick up on it, and maybe even think you're full of shit and that your idea really is terrible, just like I've been saying for a little while now.
But by all means we can keep the conversation going. You may not be aware of it yourself but you're very good at teaching by example! It's just that you're consistently instructive by demonstrating precisely what **not** to do, by coming up with and explaining time and again the worst possible way to do every single thing you are trying to do. (Or *not* trying to do - precisely what your goal **is** seems to vary somewhat, depending on which of two conflicting and irreconcilable goals you are claiming to be accomplishing in a given post.)

> 1. The code doesn't false flag in any real scenario, because it doesn't have any model where it might be reasonable to disable physical cores like on AMD CCDs, and its accounted in a score system where that technique along is not enough to determine that you're in a VM.
You have got to be fucking kidding me. First the "database to end all CPU databases" (\*except for ones that aren't contained in it, due to not existing in a 'real scenario' of course), but now you've added a **weighted scoring system** to it to make the very many parts of your idea that are obviously not going to work cancel each other out? Well, with careful tuning it could work out to be a *fair* algorithm I suppose, if you can cancel out the FPs equally with FNs. But you could also just `return rand() % 50;` instead.

Pay attention to the fact that you seem to be using 'real world scenario' as a diminutive term of sorts: it's like the world we're in now, except without that annoying Matti asshole coming up with contrived counterexamples that would hypothetically break the system (under some artificial conditions that would obviously never happen in reality.)

I on the other hand, use the term 'real world scenario' to mean something that happens when, eventually you or any programmer, change something from a working idea in the safe space in your head to code in the real world, where it inevitably tends to break instantly because of some obvious shit you didn't think of. *That* is a real world scenario. The fact that I, and several others here (but apparently not you so far), are already seeing gaping deficiencies in your idea merely by *thinking about it*, does not bode well for what will happen when you try this anywhere outside of your own head - the last safe space where you can just detect hypervisors from user mode in peace by looking up the (hypervisor-provided) CPU information in your giant infallible database. Unfortunately, programs run on CPUs, not inside your head. And CPUs can only really execute instructions, not fatally flawed ideas.

On the bright side: the CPU doesn't know or care that the idea is and always was fucking stupid! It just means *the code is not going to work*. No different from an off-by-one error somewhere causing your entire result to be incorrect, which even the best programmers have done at some point. Hardly the end of the world. But sadly that's really where the analogy ends, because you can fix an off-by-one error once you've found it, but your idea is not fixable. **Ever**. (I've already said this a number of times, just repeating for emphasis.)

> 2. Changing the CPU brand string would not be effective at all, a lot of inconsistencies would happen
> 
> (...snip...)
> 
> *[insert false claim about 'crossreferencing' being an effective counter here, plus the insinuation that I intentionally ignored a post by you about this because you were secretly right all along and this would have been highly inconvenient for me, rather than because I either simmply didn't see the post, or else, saw it and deemed it undeserving of replying to at all]*
Yes it actually would be effective. Moving on.

[2025-02-12 05:57] Matti: > 3. The db will likely not growth to include every cpu in the world, only the most used cpu models which is enough (**other cpus will likely flag other heuristics**).
O...K. I really wanted to say "moving on" again, but I just have to ask: what was the goal again? Detecting hypervisors, right? Or else at least not returning a false positive for bare metal machines? Actually nevermind I don't care, it just seems to me that these hitherto unknown 'other heuristics' may be relevant here to help figure out what the fuck you are actually trying to do. It's not **really** 'detecting hypervisors' it seems, since apparently that is just one of several 'heuristics' in your... what? What was it you said you were making again? ||(For the audience: stick around until the end of this post to find the shocking answer revealed! You would never guess what it is!)||

> Isn't this strategy better than checking for useless registry keys, firmware signatures that are patched in every hardener, VM files and process names like every vm detection project does?
Not really. I'd estimate it'd be about equally terrible in terms of results (likely simply trading FNs for FPs). More importantly, it's definitely going to do *a lot worse* on some other key negative performance indicators, such as 'time spent implementing a terrible idea' and 'time spent vigorously defending a terrible idea in internet arguments'.

> The db is the only 'bad point' of this strategy and it's a necessary workaround
This is like saying that "a bunch of planes were the only 'bad point' of 9/11", which was otherwise a perfectly fine day. *The database is the entire reason your idea is bad!* **Unfixably** bad!

> but could you really suggest something better? you have done nothing but criticize the technique instead of suggesting other better ideas to detect hypervisors when hardened
Holy fuck, you caught me. This claim was verified by my team of fact checkers just moments ago, and it turns out that you are correct. I did not in fact do your homework for you.

I assumed that the overhwelming evidence against your idea presented by me and others here would be considered a contribution in its own right, due to the untold hours saved from wasting time on this, if only you didn't have the worst case of Dunning-Kruger I've ever seen in my life.
But if *coming up with a hypervisor detection for you* (one that fits your many peculiar and ever changing requirements at that) is the bar, then it looks like you'll be wasting those hours after all. I have no interest in either developing or detecting hypervisors. Maybe ask someone else here and hope they wll be generous enough to write your code for you (lol). There are plenty of people with vast amounts of HV knowledge around here, I'm just not one of them. I only have the knowledge required to point out some of the very many and very obvious reasons why your idea is not a good one.

Sorry for being so *unconstructive*, but it's hardly my fault that my seriously deficient knowledge of hypervisors still beats your seemingly nonexistent understanding of them as well as CPUs in general. I have to say that your total inability or unwillingness to listen to reason isn't really helping, but that's fine. Everyone learns at their own pace. I just hope that we can get you to understand this before the heat death of the universe.

[2025-02-12 05:59] Matti: > 4. I already said 2 times that I wont use the bcd code i sent before, in the very first message i said "I'm still looking for something more reliable" precisely because the same reason, you suggested a better alternative and I did read that, but there are 1000 other better ways to say the same thing in better tones
If my *tone* is the problem in my messages, and not the contents, then I fear it's not going to get better for you man. Nooooooooooope. Getting some bad vibes about my tone coming up right about now...

> 5. talking about the ntoskrnl being moved to another memory location when booting windows 10 22h2, I dmed you what i meant and sent the exact source code i was trying in my EFI driver
This is because the kernel is relocated during boot time you fucking moron. (Even your own message seems to imply that you are actually aware of this at some level, but somehow still not really *understanding* it.) I'm sorry I asked - it was really my mistake to assume there could have been a change to the loader list bookkeeping or something more interesting like that. Instead what is happening is a much more obvious explanation, as helpfully clarified by the "EFI Clicker" bootkit code you kindly sent me in your DM.

You are storing the list head of the boot loader's driver list, which is obviously created by the boot loader before the kernel is even executing, let alone done relocating itself and the other boot loaded drivers, and, equally obviously, the VAs in the boot loader parameter block structures are never fixed up afterwards, because this is an undocumented, internal and transient data structure that was never intended to be used by anyone other than the kernel, or even survive past stage 0 of boot at all.

For clarity, I've highlighted the code making that is making the incorrect assumptions about the bootloader list in this screenshot. Hope that helps clarify where exactly the issue lies.

...Oh, well I guess that also answers my question about what the fuck you are actually trying to do! Couldn't have wished for better really.
Btw, have you checked out <#835634425995853834> lately?
[Attachments: image.png]

[2025-02-12 09:45] xorzer0: [replying to xorzer0: "Yes, this is the CPUID mapping for the most popula..."]
Additional digging suggests this is running OpenVMM. If somebody is interested in learning more about HVs they are looking for support on the paravisor https://openvmm.dev/guide/user_guide/openhcl/run/openvmm_linux.html
[Embed: On Linux - The OpenVMM Guide]

[2025-02-12 11:24] Vonnegut: [replying to Matti: "> 4. I already said 2 times that I wont use the bc..."]
Really hard to figure out what this is for 🤣 🤣 🤣

[2025-02-12 11:31] x86matthew: an accessibility tool of course ♿

[2025-02-12 11:31] Matti: it's most likely a cookie clicker accelerator, I'm thinking

[2025-02-12 11:33] Brit: My man is out there working his ass off to prevent millions of people from getting RSI and you flame him like this?

[2025-02-12 11:33] Brit: <:topkek:904522829616263178>

[2025-02-12 11:33] Matti: I just enjoy writing essays okay

[2025-02-12 11:36] Matti: it just so happens that this one contained a minimal and measured amount of bullying, due to a 100% self-inflicted wound

[2025-02-12 11:37] Matti: namely deciding that the best course of action here would be to send me this code to clarify something else he was also misunderstanding

[2025-02-12 11:37] avx: the fact i read all of it, nice thesis

[2025-02-12 11:38] Matti: why thank you, I appreciate it

[2025-02-12 11:38] Brit: [replying to Matti: "namely deciding that the best course of action her..."]
Fatal blunder

[2025-02-12 11:39] Brit: This is why I only send matti binaries

[2025-02-12 11:39] Brit: Lest he mock me publicly

[2025-02-12 11:40] Matti: sending me binaries of EFI bootkits also doesn't tend to go well usually
[Attachments: image.png]

[2025-02-12 11:42] Brit: <:kekw:904522300257345566>

[2025-02-12 11:42] Matti: this code was so awful that I couldn't look at it for long enough to determine if it contained any efiguard code, FWIW

[2025-02-12 11:42] Matti: the GPL is sacred but I also value what little sanity I've got left

[2025-02-12 11:43] avx: [replying to Matti: "sending me binaries of EFI bootkits also doesn't t..."]
AHAHAHAHHAHA

[2025-02-12 11:44] Eriktion: Nicely written

[2025-02-12 11:59] Timmy: this was like reading a hq /r/askreddit post back in the day!

[2025-02-12 12:04] avx: `Sorry for being so unconstructive, but it's hardly my fault that my seriously deficient knowledge of hypervisors still beats your seemingly nonexistent understanding of them as well as CPUs in general. `

[2025-02-12 12:04] avx: https://tenor.com/view/meme-gif-23357291

[2025-02-12 12:05] Timmy: murderedbywords XD

[2025-02-12 16:33] sync: That’s a fucking crazy post

[2025-02-12 16:34] sync: Should be pinned in a hall of fame

[2025-02-12 22:29] .: [replying to Matti: "Careful mate, if you keep moving the goalposts lik..."]
Oh, bravo—your rant is a real tour de farce. It's truly impressive how you manage to mix pseudo-expertise with a level of arrogance that could power its own data center, all while spewing the kind of empty insults that scream "I’m overcompensating for my lack of actual insight." Honestly, your so-called “fact-checking team” is nothing more than the echo of your own delusions, and your grasp on basic technical concepts is as reliable as a leaky database. Enjoy your self-appointed role as the guardian of all that’s “real,” because clearly, your expertise is limited to regurgitating the same tired, pompous drivel while completely missing the mark on every point. I looked at this fucking shit and I have to admit you made my day, how to start?

[2025-02-12 22:29] .: What exactly do I have to watch out for, some fat Afroameriasian who sits in a chair all day replying to my posts and licking my dick like a fan, and then I get a 500 line essay of fucking bullshit proving once again how your two neurons in the head indigenous brain literally can't process more than 4 posts in parallel where I explain over and over what the point of all this is?

[2025-02-12 22:29] .: Everyone can make unconstructive criticisms, I hope you're not going to cry to Daax about my ‘tone’ now so he deletes my posts like happened in the other channel of this discord after busting your ass. Let's start by cleaning up all the nonsensical shit you said, the ‘score system’ refers to the technique, as mentioned in the FIRST FUCKING post of this whole thread, is a combination of a count of the number of logical processors TOGETHER with timing checks to detect VMs and other heuristics. If someone scrolls up they can see perfectly well that the only ‘bypass’ your neanderthal mind you have is to change the CPU brand string and do a ‘type 1 hypervisor’. The solution is not cross-referencing as you mention, your idea is basically fucking stupid because in the second or third message I sent in the thread mentions doing heuristics on the CPU to see if it's legit or not. One of the techniques I mentioned early in the thread was using instructions that the CPU you're faking is supposed to support. For example, if you have an i7-12700KF that has 16 threads and you spoof it as an i5-10600K to only have 12 threads, you'll be able to bypass the first idea, but not the fact that your CPU will never be able to properly emulate a ton of instructions such as using AVX512 instructions, because an i5-10600K doesn't support AVX512. You can verify that the CPU is not who it claims to be in thousands of ways, which I normally use not only limited to Vector Neural Network instructions like VPDPBUSD, verifying that the CPU name matches various regexes in accordance with the CPU manufacture like, to name a few: "AuthenticAMD", "CentaurHauls", "CyrixInstead","GenuineIntel", "GenuineIotel", "TransmetaCPU", "GenuineTMx86", "Geode by NSC", "NexGenDriven", and a lot more. I have tried to convince you repeatedly that your idea of ​​changing the CPU model is retarded and will be useless, like most of the things you have said, but it seems that I can't convince you.

[2025-02-12 22:29] .: I don't know why you think I ever asked a monkey like you to make me a code where you give me something better, I don't need it because I know you won't be able to. I have already put into production many ideas that have worked such as detecting the presence of previously existing VM artifacts via forensic analysis, such as parsing the SYSTEM registry hive and scanning the unallocated space to detect deleted NK/VK keys that expose VM presence, detecting previously executed files using Prefetch, the EventLog API, scanning for registry entries in UserSettings controlled by the bam.sys driver, scanning appcrash, Pca (and its PcaSvc process memory), CdpUserSvc, scanning NTFS filesystems in the $J datastream for deleted files, or even detecting remote connections that the VM manufacturers themselves made at any time in the past by scanning the Diagnostic Policy Service and DusmSvc processes to indirectly read the contents of the System Resource Usage Monitor database, all of this while being limited to usermode. Would a bypasser think they have to bypass all of this to harden their environment? No! Thinking about techniques that go beyond the typical box of methods that are traditionally used is what makes them good, not the bullshit you mention. Seriously man, you may know 'a lot' but what's the point of having all of that knowledge if your brain cant process it bro, please get a better brain at amazon

[2025-02-12 22:29] .: Meanwhile, while you say with complete certainty that you have more knowledge than me on this topic, you said you're interested in hypervisor detection since a lot of time, but what is the most that a fucking hot tub of chromosomes like you has managed to do with your "incredible intellect" in terms of coming up with halfway decent ideas on this? The only thing that could be mentioned about you is that you are the author of "AL-Khaser" (a project that you don't even maintain), where, to mention some of the techniques it has: MAC Addresses, file system artifacts (even my grandmother would notice that she just has to delete the file bro), reading registry bullshit that false flags, seeing if there are processes or services with certain specific names running, and other shit that is not even from the project itself, it is taken from other people's pull requests. So, why even attempting to make other people's ideas look bad and then not expecting to get railed because of your ignorance?

And with your fucking 80-year-old balls soaked in vinegar you go and say that those techniques are just as bad as my idea of ​​"*forcing hypervisors to use the largest number of physical cores possible with heuristics in the CPU in order to improve the effectiveness of observing the overhead of these in the timing checks that are based on detecting hypervisors that share the same timer across multiple vCPUs by spamming a trapping instruction in multiple cores while another core, for example, measures discrepancies between functions that don't have much overhead (_ReadWriteBarrier, GetProcessHeap...) and others that do (the most typical example, cpuid with a sleep to induce cache flushing)"*!!!!! You HAVE to be KIDDING me, and my idea is apparently "unfixable"?? You weren't lying when you mentioned your seriously deficient "knowledge of hypervisors"
PS: Had to write all of this in English which is a language I don't speak, hope it's not hard to read! :;)

[2025-02-12 22:29] .: Nobody has "threw my idea to shit just by thinking a little", the only thing they have said that is halfway coherent is "what's going on with KVM". While this method is not intended to be cross-platform and is dedicated only to Windows, it only assigns all cores in situations where the guest OS is the same as the host OS on Linux because then it can do resource optimisations by sharing all the cores more efficiently. I hope you don't feel too bad about what I've told you, but you started off as a hypocrite, considering the idea as "clever" and you've humiliated yourself by trying to make the idea seem ridiculous because of a "giant database" of CPU models. It's a shame that most of the circus clowns you have here have sucked your dick as usual and praised you by putting 15 emoji reactions on the internet. I see that you felt great and even believed that you bullied me as if we were in a kindergarten, poor soul.

[2025-02-12 22:31] Deleted User: [replying to .: "Oh, bravo—your rant is a real tour de farce. It's ..."]
why did you chatgpt that

[2025-02-12 22:32] Deleted User: 
[Attachments: image.png]

[2025-02-12 22:33] Deleted User: opening with the ai to seem smart, then with the personal non ai insults is kinda cringe <:catBRUH:1148058667942412338>

[2025-02-12 22:33] contificate: [replying to .: "Oh, bravo—your rant is a real tour de farce. It's ..."]
I'm gay

[2025-02-12 22:33] .: I did not. :)

[2025-02-12 22:33] contificate: > Good work @can't code, won't code, you just advanced to level 15!
bot is an ally

[2025-02-12 22:34] Brit: [replying to contificate: "I'm gay"]
it's 2025, no longer cool

[2025-02-12 22:34] contificate: 😔 will get a girlfriend and stop living a lie, then

[2025-02-12 22:52] f00d: [replying to .: "Oh, bravo—your rant is a real tour de farce. It's ..."]
https://tenor.com/view/schizo-cat-the-voices-gif-25415230

[2025-02-12 23:13] .: NOO PEOPLE ARE REACTING TO MY MESSAGES WITH EMOJIS AND GIFS MAN

[2025-02-12 23:13] .: WHAT IM GONNA DO

[2025-02-12 23:13] .: [replying to f00d: "https://tenor.com/view/schizo-cat-the-voices-gif-2..."]
https://tenor.com/view/cat-shocked-shocked-cat-gif-11170546809524778591

[2025-02-12 23:47] Deleted User: did he leave..

[2025-02-13 00:04] dullard: [replying to contificate: "> Good work @can't code, won't code, you just adva..."]
Level 15 gay !

[2025-02-13 00:05] contificate: !

[2025-02-13 00:07] daax: > Everyone can make unconstructive criticisms, I hope you're not going to cry to Daax about my ‘tone’ now so he deletes my posts like happened in the other channel of this discord after busting your ass.
I didn't delete your post mate. You keep saying this, but I didn't delete anything lol.

[2025-02-13 00:16] sync: [replying to .: "Nobody has "threw my idea to shit just by thinking..."]
btw you can run windows on kvm lol

[2025-02-13 00:16] sync: so i mean ur idea is inherently flawed from the get go

[2025-02-13 00:16] daax: he already left <@1115777645519577149>

[2025-02-13 00:16] sync: lmfao

[2025-02-13 00:16] sync: poor guy

[2025-02-13 00:16] sync: he got buillied on the internet

[2025-02-13 00:18] daax: [replying to .: "I don't know why you think I ever asked a monkey l..."]
many of these are on the mind of many people... but it's unnecessary because most people can't even mitigate the timing attacks outside of rdtsc/cpuid/rdtsc. oh well.

[2025-02-13 00:19] sync: yeh i mean his idea is "cool" and all, but like who is it even targeting

[2025-02-13 00:19] sync: tbh

[2025-02-13 00:19] sync: who even runs anything on a vm anymore

[2025-02-13 00:20] sync: i just have a gutted hackbox

[2025-02-13 00:20] szczcur: [replying to daax: "many of these are on the mind of many people... bu..."]
no daax.. he came up with all those novel super original ideas himself! such grace and modesty. do people harden against these things from the get go? no.. but.. you know that mitigating these takes like 20 seconds if you have well designed framework.

[2025-02-13 00:21] sync: lets stop talking about this guy when he isnt here

[2025-02-13 00:21] sync: he gonna come back and stroke his ego

[2025-02-13 00:22] sync: "ooooh yes, talk about me more 🔥"

[2025-02-13 00:22] szczcur: he'll continue to engage in autofellatio and nobody will save him. he is god of computers and detecting vmware.. meanwhile he doesnt even think of cmos. WHAT A PEASANT! see what i mean? who cares.

[2025-02-13 00:22] daax: <@1033421942910369823> lol delet

[2025-02-13 00:23] sync: <:give:835809674480582656>

[2025-02-13 00:23] szczcur: [replying to daax: "<@1033421942910369823> lol delet"]
we have to keep it clean ikik

[2025-02-13 00:30] dlima: 💀

[2025-02-13 00:31] szczcur: [replying to .: "Nobody has "threw my idea to shit just by thinking..."]
i dont think anyone was sucking his dick.. the consensus is just that youre a babbling buffoon who likes to beat his chest but is 5'2. dork vader needs to stop trying to sound like a big dog.

[2025-02-13 00:32] szczcur: aw he did leave his name changed colors. k back to the regularly scheduled programming.

[2025-02-13 00:33] Brit: [replying to szczcur: "i dont think anyone was sucking his dick.. the con..."]
that's a brand new one

[2025-02-13 00:33] Brit: dork vader 🚀

[2025-02-13 00:34] sync: [replying to Brit: "that's a brand new one"]
babbling buffoon is old??

[2025-02-13 00:34] Brit: dork vader

[2025-02-13 00:34] sync: i think ur a dork vader brit

[2025-02-13 00:34] sync: is this true?

[2025-02-13 00:34] szczcur: [replying to Brit: "dork vader 🚀"]
it's fitting

[2025-02-13 10:24] kai77: [replying to sync: "babbling buffoon is old??"]
💀

[2025-02-13 12:13] Vonnegut: [replying to .: "Nobody has "threw my idea to shit just by thinking..."]
Throw me a bin, I will beat this assbackwards check

Edit: too late 😭

[2025-02-13 12:20] Vonnegut: This backwards compatibility between versions and CPU features for HV detection is a stupid idea. Yes some features are missing on newer CPUs (rare) normally it's the inverse where newer CPUs have the features but the old don't (this isn't an issue in this use case) but I literally have matrices for CPUID compatibility across a fuck ton of processors (guess my job)

I can find a ones that match and also match the topology of my VMs no issue, or I can change the god damn topology.

[2025-02-13 12:25] Vonnegut: I'll throw you a bone though as you're semi close to actually coming up with a good idea.

This technique isn't publicly used anywhere for VM detection but it works and I don't really care either way.

Someone spoofing the CPU identity? Cache is described here too. First is if advertised cache doesn't match what CPU it declares. Okay but if the buffoon has at least managed to spoof that too.

Guess what else you can do now? Just write an application that saturates the cache and then monitors for cache misses. Using cache misses you can correctly determine the size available and infer if that matches what the "CPU" is providing.

However this can be bypassed by using cache allocation technology to synthetically limit what's available.

[2025-02-13 14:25] DIMM: blud left the server after this

[2025-02-13 14:25] Matti: yeah but it's still an interesting topic

[2025-02-13 14:25] Matti: his having left certainly improved the quality of posts overall here

[2025-02-13 14:26] DIMM: I hope I can get into hypervisors soon

[2025-02-13 14:26] DIMM: They're such a cool concept

[2025-02-13 14:26] DIMM: Long way to go though

[2025-02-13 14:27] Matti: [replying to Vonnegut: "I'll throw you a bone though as you're semi close ..."]
question: what about my naive example from before where you take a CPU that's for all intents and purposes (cores, logical processors, cache sizes) half of your actual CPU?
e.g. 5950X (16C/32T, 8 MB L2, 64 MB L3) vs 5800X (8C/16T, 4 MB L2, 32 MB L3)

[2025-02-13 14:27] Matti: there is a slight clock speed difference but that is so minor as to be negligible, and also people both overclock and underclock CPUs

[2025-02-13 14:27] DIMM: That's what I thought too but maybe you can limit cache access somehow

[2025-02-13 14:28] DIMM: I thought that's what he meant by "using cache allocation technology to synthetically limiting what's abailable"

[2025-02-13 14:28] DIMM: available mb

[2025-02-13 14:30] Matti: definitely agree re: feature bit checking, newer CPU (even by the same vendor in the same product series) does not imply it has the same set or a superset of the CPUID bits (let alone MSRs but he was talking about user mode so alright)

[2025-02-13 14:31] Matti: [replying to DIMM: "That's what I thought too but maybe you can limit ..."]
you can, and that is a correct interpretation I think

[2025-02-13 14:32] Matti: I wouldn't know how to do this but <@235901710994767872> no doubt does

[2025-02-13 14:33] Matti: cache allocation*, not cache access, btw

[2025-02-13 14:34] Matti: but I don't know who is allocating the requested sizes in this scenario

[2025-02-13 14:34] DIMM: [replying to Matti: "I wouldn't know how to do this but <@2359017109947..."]
I wouldn't know it either I don't have enough knowledge on hypervisors

[2025-02-13 14:35] Matti: well that's why I asked vonnegut, not you <:thinknow:475800595110821888>

[2025-02-13 14:35] DIMM: Ah yeah that's fair

[2025-02-13 14:36] DIMM: I'll be reading along to hopefully pick up on some things

[2025-02-13 14:37] Matti: the best approach when you don't know something is to find someone who does or might, and then ask good questions and pay attention
and/or read up yourself of course, this is always an option

[2025-02-13 14:37] Matti: since I'm lazy I went for the first approach

[2025-02-13 14:38] DIMM: I mostly go for the second approach but when I have questions I'll try to ask someone who knows a lot about it

[2025-02-13 14:39] DIMM: But for now I just need to learn more about cpus in general before I can start digging into things like hypervisors

[2025-02-13 14:39] DIMM: Really cool stuff though

[2025-02-13 14:40] pinefin: [replying to DIMM: "But for now I just need to learn more about cpus i..."]
i’d say if you know kernel already the best path to go down is get as close as you can to the HAL and virtualization and then learn about the cpu

[2025-02-13 14:40] pinefin: follow the path from software to firmware and then to hardware

[2025-02-13 14:40] pinefin: it’s easier to understand that way

[2025-02-13 14:43] DIMM: [replying to pinefin: "i’d say if you know kernel already the best path t..."]
Currently I'm reading winternals 1 & 2 (managed to score two physical copies yipee!) so I'm getting to know the kernel, HAL is very very cool concept and i hope they go in depth on it

[2025-02-13 14:44] DIMM: For now I'll just have to get to know the windows kernel

[2025-02-13 14:44] DIMM: But eventually I'll write a small hypervisor, hopefully in a month or two :))

[2025-02-13 15:04] Vonnegut: [replying to Matti: "question: what about my naive example from before ..."]
Yeah then it's fucked too. This is why doong it purely on semantics like this isn't great

[2025-02-13 15:05] Vonnegut: You can start potentially monitoring cache hit frequency as a whole. Hypervisor will cause more misses if it pre-empts to vmroot but it's probably not stable enough to be definitive as things like NMI and SMI can have the same effect

[2025-02-13 15:09] Vonnegut: [replying to Matti: "I wouldn't know how to do this but <@2359017109947..."]
Intel "Cache Allocation Technology" has the ability, it has it's drawbacks though.

[2025-02-13 16:16] daax: [replying to Vonnegut: "I'll throw you a bone though as you're semi close ..."]
He did mention cache side channels, and that's low hanging fruit as there's 2147483647 papers on that. Cache side channels / cache mismatches are a dime a dozen for vm detection -- you don't even need to go this far in complexity unless you want to inflict maximum pain, because the majority of people have no idea how to mitigate these even if you write it out for them.

[2025-02-13 16:19] daax: His problem is a skill issue, that he wouldn't know how to implement any reasonable check even if someone sent him the pseudo-impl for it.

[2025-02-13 16:28] daax: [replying to DIMM: "But for now I just need to learn more about cpus i..."]
it's a great way to have to learn the various concepts of the cpu, so long as you don't git clone some public hv repo and do nothing.

[2025-02-13 16:34] szczcur: [replying to daax: "He did mention cache side channels, and that's low..."]
i don't think what he mentioned is the same realm of cache side channel

[2025-02-13 16:34] daax: [replying to szczcur: "i don't think what he mentioned is the same realm ..."]
no no, I agree. I was trying to say there are even easier ways that the other guy just ignored

[2025-02-13 16:36] daax: wasn't a dig <@235901710994767872>'s idea, was about the guy acknowledging them but just suggesting mmh no he wants something "better"

[2025-02-13 16:37] szczcur: [replying to daax: "wasn't a dig <@235901710994767872>'s idea, was abo..."]
yea.. from usermode you still have options. hell measure the avx penalty.

[2025-02-13 16:37] Vonnegut: [replying to daax: "wasn't a dig <@235901710994767872>'s idea, was abo..."]
I don't care 😂 I was just continuing on the meme

[2025-02-13 16:37] Vonnegut: The deleted post was much worse

[2025-02-13 16:38] daax: [replying to Vonnegut: "I don't care 😂 I was just continuing on the meme"]
ik just clarifying that it was confusing why requiem passed up reasonably useful checks that dont rely on artifacts

[2025-02-13 17:49] avx: [replying to daax: "it's a great way to have to learn the various conc..."]
Jarvis clone every repo from tandasat

[2025-02-13 18:11] Matti: [replying to szczcur: "yea.. from usermode you still have options. hell m..."]
AVX penalty <:hmm:475800667177484308> 
Intel(R) user detected <:hmm:475800667177484308>

[2025-02-13 18:12] Matti: well AMDs do have an AVX penalty, but since turin it's now the same for AVX/AVX2/AVX512

[2025-02-13 18:14] Matti: zen 5* actually, apparently consumer ""processors"" benefit too

[2025-02-13 18:14] Matti: http://www.numberworld.org/blogs/2024_8_7_zen5_avx512_teardown/ here's a pretty interesting breakdown of the changes

[2025-02-13 18:18] szczcur: [replying to Matti: "AVX penalty <:hmm:475800667177484308> 
Intel(R) us..."]
oh yes.. pain inside(tm)

[2025-02-13 18:19] Matti: [replying to Matti: "well AMDs do have an AVX penalty, but since turin ..."]
OK not actually true as shown by the benchmarks, it depends highly on core count

[2025-02-13 18:20] Matti: but the difference in how the AVX penalty is applied (intel vs AMD I mean) is actually more interesting

[2025-02-13 18:20] Matti: the section starting at 'Does Zen5 throttle under AVX512?' I mean

[2025-02-13 18:21] Matti: why can this guy make such detailed technical writeups but not manage to add section links to them

[2025-02-14 05:16] Torph: [replying to Matti: "> 3. The db will likely not growth to include ever..."]
what are FNs and FPs?

[2025-02-14 05:30] Torph: [replying to Vonnegut: "I'll throw you a bone though as you're semi close ..."]
I guess I just don't understand why you'd need to know this badly if you're in a VM unless you're a virus

[2025-02-14 09:12] x86matthew: [replying to Torph: "what are FNs and FPs?"]
false negatives / false positives

[2025-02-14 11:18] SYAZ: [replying to Torph: "I guess I just don't understand why you'd need to ..."]
Less likely someone is going to re something if u can’t on a vm

[2025-02-14 19:47] contificate: [replying to Torph: "what are FNs and FPs?"]
frame numbers and frame pointers 😼

[2025-02-15 08:31] phage: Reposting from other server

Looking to analyze some of the more esoteric native software protectors out there, anyone have one that's not any of the following?
- VMProtect
- Themida
- Obsidium
- Loki
- CodeDefender
- Denuvo
- Tigress
- Enigma

[2025-02-15 10:13] brockade: [replying to phage: "Reposting from other server

Looking to analyze so..."]
https://obfuscator.re/
[Embed: Open Obfuscator: A free and open-source solution for obfuscating m...]
A free and open-source obfuscator for mobile applications

[2025-02-15 10:14] brockade: if you're going down the mobile path verimatrix do protection too, just trying to find samples is hard, let me know if you find a good one

[2025-02-15 14:47] Torph: <:kekw:904522300257345566> why did they make LLVM backwards

[2025-02-15 14:48] Torph: Machine Virtual Level Low

[2025-02-15 14:52] elias: <:skully:940450497008132137>