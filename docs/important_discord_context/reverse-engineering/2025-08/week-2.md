# August 2025 - Week 2
# Channel: #reverse-engineering
# Messages: 109

[2025-08-04 05:46] Damon: [replying to !Dank: "well yea but without the right driver fuzzer not g..."]
so make one

[2025-08-04 14:43] !Dank: I can make one just fine just trying to find one I can use for my unsigned driver 🤡

[2025-08-04 14:45] !Dank: [replying to Damon: "so make one"]
Wrong again

[2025-08-04 19:36] mtu: I’m confused by what you think fuzzing is

[2025-08-04 19:36] mtu: Do you think you find vulnerabilities by writing an exploit then just trying that on a bunch of targets until you find a target it works for

[2025-08-04 19:50] dullard: That’s one way to fuzz <:Kapp:543420634571735047>

[2025-08-04 21:26] Brit: [replying to !Dank: "I can make one just fine just trying to find one I..."]
Markov chains also sometimes output sequences that look like a human wrote them, so you have that in common.

[2025-08-04 21:31] !Dank: I am human

[2025-08-04 21:31] !Dank: been in emoji.gg longer than you

[2025-08-04 22:34] mtu: Can you answer my question

[2025-08-04 22:35] mtu: What exactly do you think fuzzing is that you “have a fuzzer” and now just need to “find a driver to accept your ioctls”

[2025-08-04 22:35] mtu: I’m pretty sure a Markov chain wouldn’t even put those words together

[2025-08-04 22:51] pygrum: [replying to mtu: "What exactly do you think fuzzing is that you “hav..."]
iterating from 0-0xFFFFFFFF until they get a response <:WhenYourStructsAreAligned:812734901681651744>

[2025-08-05 01:12] UJ: This is interesting. For fun, Im trying to lift a virtual machine to llvm to try to deobfuscate it, following along with these 3 [posts](https://secret.club/2021/09/08/vmprotect-llvm-lifting-1.html).

Anyone know if any obfuscators (most likely vmp 3.x) uses instructions that "legitly" decode one way in ida/bn but is interpreted another way when actually executed (not rewritten). It looks like just lifting the bytecode of the vm loop to llvm it was able to "discover" these 6 units, that i don't see in the disasm that was fully disassembled. 

(A more logical explanation is i messed up here with some bytes but the result isn't sus)
[Attachments: Screenshot_2025-08-04_175928.png, Screenshot_2025-08-04_180802.png]

[2025-08-05 01:31] qw3rty01: iirc EAC used to have a jump to the middle of an instruction in their driver, binja handled it correctly but IDA used to break on it

[2025-08-05 01:34] UJ: they have their own obfuscator right, not an off the shelf one ide be able to try (like vmp etc).

[2025-08-05 01:35] qw3rty01: haven't looked at it in a few years but it used to just add junk instructions and random branches that ended up having the same logic

[2025-08-05 01:35] UJ: got it.

[2025-08-05 01:35] qw3rty01: actually made it super easy to reverse because you could just compare the instructions in both branches and the instructions that matched in both branches was the actual logic lol

[2025-08-05 01:36] UJ: the predicates were too opaque.

[2025-08-05 01:41] Damon: [replying to mtu: "Do you think you find vulnerabilities by writing a..."]


[2025-08-05 01:41] Tr1x: [replying to UJ: "This is interesting. For fun, Im trying to lift a ..."]
It's hard to pinpoint the exact reason but I have a couple of ideas, but my biggest one is encoded/obscured operands that are resolved at runtime. Have you searched directly for the opcode of any of the intrinsics yet?

[2025-08-05 01:41] UJ: also related, in [this](https://secret.club/2021/09/08/vmprotect-llvm-lifting-1.html), their recommendation when lifting each vm handler, to analyze each one, and rewrite it in `output vm_handler_name(t1 op1, t2 op2)` form, if i don't do this (bc im lazy) and just lift each handler directly and the params just end up being `void vm_handler_name(void* ram, u65 vip u64 acc)`, would i lose alot of cleanup? there are 68 vm handlers as far as i can tell.

[2025-08-05 01:43] Tr1x: [replying to UJ: "also related, in [this](https://secret.club/2021/0..."]
Your problem without it being semantically correct is optimization, you might output complete junk on the other end

[2025-08-05 01:44] UJ: makes sense

[2025-08-05 01:47] Tr1x: Doing this by hand though is annoying asf. You can make a profiler for the handlers themselves to try and generate semantically correct representations of them, dynamically. You will have to tackle the obfuscation programmatically but there is a project that did it for vmp2 and there might be one for vmp3

[2025-08-05 01:54] Tr1x: [replying to UJ: "This is interesting. For fun, Im trying to lift a ..."]
Also was this done on a user mode application?

[2025-08-05 01:55] Tr1x: If not then all is good because I see handlers for privileged instructions

[2025-08-05 02:24] UJ: yeah i don't even see the bytes for these instructions as substrings in the bin. lemme take a look at this a bit more before spending more time going down this path.

[2025-08-05 04:08] Horsie: How do yall add code to a compiled PE?

[2025-08-05 04:08] Horsie: I have this app where I want to delete a few registry keys on launch/set them to a different value.

[2025-08-05 04:09] Horsie: I was thinking of using binjas scc and yeeting it into a new section

[2025-08-05 04:09] Horsie: Alternatives?

[2025-08-05 04:11] Horsie: [replying to Horsie: "I have this app where I want to delete a few regis..."]
There are much easier ways to do this. though i like this overengineered way

[2025-08-05 05:21] Horsie: (scc is a bit shit)

[2025-08-05 05:22] Horsie: considering just generating a freestanding bin from clang and patch that in

[2025-08-05 05:22] UJ: i also went down the scc route a couple months ago and came to the same conclusion. i went with the new section thing as well. if you are going to lift the whole binary, you could try https://github.com/lifting-bits/patchestry (lemme know how it works).

[2025-08-05 05:26] UJ: these trailofbits guys make some pretty cool stuff.

[2025-08-05 09:36] Tr1x: [replying to Horsie: "How do yall add code to a compiled PE?"]
https://github.com/lief-project/LIEF
[Embed: GitHub - lief-project/LIEF: LIEF - Library to Instrument Executable...]
LIEF - Library to Instrument Executable Formats (C++, Python, Rust) - lief-project/LIEF

[2025-08-05 09:46] pygrum: [replying to Horsie: "How do yall add code to a compiled PE?"]
Shameless self promo https://github.com/badhive/stitch
[Embed: GitHub - badhive/stitch: Rewrite and obfuscate code in compiled bin...]
Rewrite and obfuscate code in compiled binaries. Contribute to badhive/stitch development by creating an account on GitHub.

[2025-08-05 11:08] Brit: [replying to Horsie: "How do yall add code to a compiled PE?"]
Add code to a new executable section and patch text to jump to it when convenient, although idk why you'd hardpatch when you can just loadlib and do stuff on dllattach

[2025-08-05 11:14] dullard: [replying to Horsie: "I have this app where I want to delete a few regis..."]
Write a loader which hooks said api calls

[2025-08-05 11:16] the horse: [replying to qw3rty01: "iirc EAC used to have a jump to the middle of an i..."]
do you mean their disassembly desynchronization?

[2025-08-05 11:16] the horse: iirc EAC had call+5 encoded which landed right into a jmp and IDA had problems with it

[2025-08-05 11:40] Brit: [replying to dullard: "Write a loader which hooks said api calls"]
This is also one of the ways, or figure out some early load dll to hijack if you want a hands-off solution

[2025-08-05 11:41] dullard: 99.999% there will be a library inside the same folder which could just be swapped out and redirected to

[2025-08-05 11:54] Brit: [replying to qw3rty01: "actually made it super easy to reverse because you..."]
One of the classic blunders when doing naive obf, opaque predicate where one branch is obviously junk, or opaque where both branches are equivalent.

[2025-08-05 15:04] Horsie: [replying to Brit: "Add code to a new executable section and patch tex..."]
just as a fun challenge really

[2025-08-05 15:04] Horsie: [replying to pygrum: "Shameless self promo https://github.com/badhive/st..."]
hmm yeah ill try this

[2025-08-05 15:04] Horsie: [replying to Tr1x: "https://github.com/lief-project/LIEF"]
ive usee lief in the past for some simple stuff. I forgot it isnt for read-only stuff. this seems like the kiss solution

[2025-08-05 15:08] Brit: [replying to Horsie: "just as a fun challenge really"]
In that case you really should be writing your own tooling to relocate all the displaced code that's about the only learnable thing here

[2025-08-05 15:09] Horsie: [replying to Brit: "In that case you really should be writing your own..."]
displaced as in, the new code added?

[2025-08-05 15:09] Horsie: or existing code being moved elsewhere to make a hole

[2025-08-05 15:09] Brit: If you add code to the middle of a section, what happens to the original code that was there and everything that follows it

[2025-08-05 15:10] Horsie: [replying to Brit: "If you add code to the middle of a section, what h..."]
Ah. I had it in mind to just patch in a call to the trailing couple bytes of the section where my code would be

[2025-08-05 15:11] Horsie: [replying to Brit: "If you add code to the middle of a section, what h..."]
isnt that also a code discovery issue? thats a bit difficult because its an obfuscated bin

[2025-08-05 15:12] Horsie: a bunch of disass is broken

[2025-08-05 15:12] Horsie: [replying to Brit: "In that case you really should be writing your own..."]
not too much of a challenge that it becomes a timesink.

[2025-08-05 15:12] Horsie: Only times ive patched in some code was when the patches were small enough to be hand patched

[2025-08-05 15:13] Horsie: This time around I would like to just script it to make it a bit less tedious

[2025-08-05 15:13] 0xatul: without trying to bring in piratesoftware, code caves are an okay solution if its a resonably sized patch (?)

[2025-08-05 15:13] Horsie: with freestanding code

[2025-08-05 15:14] Horsie: [replying to 0xatul: "without trying to bring in piratesoftware, code ca..."]
likely. but i might have to just make a new section

[2025-08-05 15:14] Horsie: if thats the case then the solution is a nobrainer

[2025-08-05 15:14] 0xatul: fair enough <:kappa:1115968816812392470>

[2025-08-05 15:54] Tr1x: [replying to Horsie: "ive usee lief in the past for some simple stuff. I..."]
Yes especially if you need something portable, if you're not looking for portability and only for behavioral analysis. Then this would do good, http://www.rohitab.com/apimonitor
[Embed: API Monitor: Spy on API Calls and COM Interfaces (Freeware 32-bit a...]
API Monitor is a software that monitors and displays API calls made by applications and services. Its a powerful tool for seeing how Windows and other applications work or tracking down problems that 

[2025-08-10 05:50] Ignotus: Can I create a tree graph / list of all functions that are called within one function in IDA? Mainly I want to know how complex the function really is and if there is any point reversing it or not. Or even better if I can build one using some debugger so that they're actually correct

[2025-08-10 10:44] Horsie: [replying to Ignotus: "Can I create a tree graph / list of all functions ..."]
yeah. in graph/linear(?) view you can right click the subroutine name and click graph xrefs to/from

[2025-08-10 10:44] Horsie: Unless I'm misunderstanding the question

[2025-08-10 12:50] hellohackers: [replying to Ignotus: "Can I create a tree graph / list of all functions ..."]
yeah but not 100%

[2025-08-10 12:50] hellohackers: because of indirect branchs, misaligned instructions &c.

[2025-08-10 12:54] Eriktion: [replying to Ignotus: "Can I create a tree graph / list of all functions ..."]
I mean you could create plugins using some graphics lib?

[2025-08-10 12:56] hellohackers: you can just export the data as json and plot anywhere you want

[2025-08-10 12:57] hellohackers: daax is writing a book

[2025-08-10 12:57] hellohackers: oh my

[2025-08-10 13:04] daax: [replying to Ignotus: "Can I create a tree graph / list of all functions ..."]
https://github.com/herosi/CTO
[Embed: GitHub - herosi/CTO: Call Tree Overviewer]
Call Tree Overviewer. Contribute to herosi/CTO development by creating an account on GitHub.

[2025-08-10 13:05] hellohackers: you are kidding me

[2025-08-10 13:05] hellohackers: 15 minutes for this

[2025-08-10 13:05] hellohackers: ?????????

[2025-08-10 13:06] daax: [replying to hellohackers: "15 minutes for this"]
what the fuck are you waiting on?

[2025-08-10 13:06] hellohackers: 😂

[2025-08-10 13:06] daax: I wrote something else to do it manually, but given the question I figure it’s easier to give him something prefab’d.

[2025-08-10 13:07] hellohackers: there is no reliable way to do this task

[2025-08-10 13:07] daax: [replying to hellohackers: "there is no reliable way to do this task"]
Yes, there is.

[2025-08-10 13:07] hellohackers: take for example themida that adds indirection with push imm ret

[2025-08-10 13:08] daax: [replying to hellohackers: "take for example themida that adds indirection wit..."]
So we’re assuming only static?

[2025-08-10 13:08] hellohackers: yeah if you trace it you can do anything

[2025-08-10 13:10] daax: [replying to hellohackers: "yeah if you trace it you can do anything"]
Duh. Why would you constrain yourself to only static if you don’t have to? Any small amount of information from a dynamic trace is useful. You’re also assuming the target is obfuscated. Lot of assumptions you made that he didn’t specify. No sense splitting hairs when he hasn’t specified what he is looking at.

[2025-08-10 13:11] hellohackers: yeah

[2025-08-10 13:12] hellohackers: maybe that is why I start things and do not finish them

[2025-08-10 13:12] hellohackers: overengineering sucks

[2025-08-10 15:12] segmentationfault: [replying to hellohackers: "maybe that is why I start things and do not finish..."]
that sounds like adderall prescription needed <:mmmm:904523247205351454>

[2025-08-10 15:12] Ignotus: Can I get IDA to show some of these normally as static numbers? ``mov     ecx, cs:dword_1FF2FC8 <----``

[2025-08-10 15:13] Ignotus: I'm waiting for the day we can just dump the assembly instructions for to an LLM and it can analyze which functions are actually relevant and reconstruct them. The graph tree has ~80 nodes 💀

[2025-08-10 15:47] the horse: [replying to Ignotus: "Can I create a tree graph / list of all functions ..."]
I think IDA has proximity graphs

[2025-08-10 15:47] the horse: that show you exactly this

[2025-08-10 15:48] the horse: 
[Attachments: image.png]

[2025-08-10 15:48] the horse: very nice thing ^^

[2025-08-10 15:51] Ignotus: [replying to the horse: ""]
can I limit it to be only from a specific function down tho?

[2025-08-10 15:51] Ignotus: for me I think it shows the whole program

[2025-08-10 15:52] the horse: you can hide nodes

[2025-08-10 15:52] the horse: that don't interest you

[2025-08-10 15:53] the horse: i think the CTO Thing sent above

[2025-08-10 15:53] the horse: is basically this

[2025-08-10 15:54] the horse: 
[Attachments: image.png]

[2025-08-10 15:54] the horse: and I think you can connfigure this too?

[2025-08-10 16:08] daax: [replying to the horse: "i think the CTO Thing sent above"]
I don’t believe this handles unresolved indirect calls, CTO does — to some extent.

[2025-08-10 16:13] daax: Also doesn’t have the clunky ass menu attached to the top of the node (and tons of nodes is so unreadable in prox browse), and doesn’t tend to follow unnecessary paths/xrefs.

[2025-08-10 17:13] the horse: oh nice