# April 2025 - Week 2
# Channel: #application-security
# Messages: 213

[2025-04-07 07:58] C2: idk if this would be the right section but I thought of an idea of hiding a hwid inside ultra sonic tones that play while someone is playing the game, something they cant hear but if they uploaded a video on youtube of them playing or cheating itd be a cool way to idenify the account they played on

[2025-04-07 07:59] C2: unfortunately the idea fell through because of how hard making it reliable was, but hopefully someone reads this and comes up with other cool concepts like this

[2025-04-07 08:04] C2: I will take a look into it

[2025-04-07 08:05] C2: sounds interesting

[2025-04-07 08:05] C2: the issue arrose most likely because of compression and whatever else, the end goal was to have a function that I could pass an 8 byte variable into and have it playsounda the tones

[2025-04-07 08:06] C2: the only real downside to a setup like this is most cheat videos have music and muted video

[2025-04-07 08:06] C2: which sucks

[2025-04-07 08:07] C2: which would make you lean towards a more visual queue

[2025-04-07 08:07] C2: if they dont mutte the video we will be good

[2025-04-07 08:07] C2: but idk maybe youre right

[2025-04-07 08:08] C2: yeah it is not the most reliable which is why I was thikning of creative ways around the issue

[2025-04-07 08:09] C2: it seems just borderline impossible to have a value hidden in audio that is unaudable and resistent to other noises

[2025-04-07 08:09] C2: but it would be an awesome conept if it worked

[2025-04-07 08:09] C2: a visual queue would be objectively better

[2025-04-07 08:11] C2: nah just if I find a video of someone cheating on "my game" lets say

[2025-04-07 08:11] C2: then I know that because they play my game they have that watermark in the audio

[2025-04-07 08:12] C2: which helps me idenify the account

[2025-04-07 08:12] C2: letting me ban them

[2025-04-07 08:12] C2: yeah exactly

[2025-04-07 08:12] C2: I could just make a dll

[2025-04-07 08:12] C2: and load it

[2025-04-07 08:12] C2: that just takes the steamid

[2025-04-07 08:12] C2: then theyre cooked, and so are dogs

[2025-04-07 08:12] C2: it was just a cool concept

[2025-04-07 08:13] C2: not exactly the most practical

[2025-04-07 08:13] C2: just a cool idea

[2025-04-07 08:13] C2: yeah definately, not saying it is a flawless idea

[2025-04-07 08:13] C2: just a creative concept

[2025-04-07 08:13] C2: maybe itll spawn some more creative ideas

[2025-04-07 08:14] C2: I could just picture someone uploading a video of them rage cheating on a game and spending time blurring all the names and shit

[2025-04-07 08:14] C2: just to get smoked by wave form analysis or something

[2025-04-07 08:14] C2: LOL

[2025-04-07 08:14] C2: would be prob steamid

[2025-04-07 08:15] C2: what? the data hidden in the tones is the steamid

[2025-04-07 08:15] C2: you can use a 4 byte value for a steamid

[2025-04-07 08:15] C2: and just play that in the tones

[2025-04-07 08:15] C2: could be a hwid

[2025-04-07 08:15] C2: depends if you have that tracked

[2025-04-07 08:16] C2: like i said its a concept

[2025-04-07 08:16] C2: the data can be anything identifiable

[2025-04-07 08:16] C2: steamid would be best unless youre an AC

[2025-04-07 08:16] C2: if youre an AC then you prob got a HWID to work with

[2025-04-07 08:16] C2: sadly it is required when you play a game

[2025-04-07 08:16] C2: if the hwid is just a hash of different components serials

[2025-04-07 08:16] C2: i dont think thats intrusive

[2025-04-07 08:17] C2: then I have bad news for you...

[2025-04-07 08:17] C2: LOL

[2025-04-07 08:17] C2: well its that or deal with more cheaters

[2025-04-07 08:17] C2: I think its a good trade off

[2025-04-07 08:17] C2: in all cases

[2025-04-07 08:18] C2: and good headphones

[2025-04-07 08:18] C2: and animals

[2025-04-07 08:18] C2: and sensitive hearing

[2025-04-07 08:18] C2: and yeah compression

[2025-04-07 08:18] C2: and whatever else

[2025-04-07 08:18] C2: itd be cool if all those werent a massive issue

[2025-04-07 08:18] C2: idk I was just looking at ultrasonic

[2025-04-07 08:19] C2: I also ran into an issue where the headphones would pop a little because of the sudden changes

[2025-04-07 08:19] C2: from ultra sonic to nothing

[2025-04-07 08:19] C2: so itd need to be smoothed

[2025-04-07 08:19] C2: making it longer

[2025-04-07 08:19] C2: so a 4 byte value could take a while to play

[2025-04-07 08:19] C2: yeah

[2025-04-07 08:20] C2: yeah

[2025-04-07 08:20] C2: ```
#define FREQ_START_MARKER 21500.0 // Hz (Was 20500)
#define FREQ_END_MARKER   23500.0 // Hz (Was 22500)
#define FREQ_BIT_0        22000.0 // Hz (Was 21000)
#define FREQ_BIT_1        23000.0 // Hz (Was 22000)
```

[2025-04-07 08:20] C2: yeah i gave up already

[2025-04-07 08:21] C2: i spent a day tinkering and its just impossible to do reliably

[2025-04-07 08:21] C2: as I mentioned in the original message

[2025-04-07 08:21] C2: I was just placing it here incase it spawned cooler ideas

[2025-04-07 14:08] pinefin: [replying to C2: "which would make you lean towards a more visual qu..."]
ive always thought of this

[2025-04-07 14:08] pinefin: like you know how minecraft has random block placement shit

[2025-04-07 14:08] pinefin: like it'll randomize the block break detection based on their random generation engine and you can like determine it

[2025-04-07 14:09] pinefin: ive always thought of if they can do textures like that to do a identification method

[2025-04-07 14:09] pinefin: then again i was probably being a schizo 10 year old

[2025-04-07 14:10] C2: xD

[2025-04-07 14:10] C2: what youre saying reminds me of how someone found out you could know the exact cord someone is at based on how the block falls after its broken

[2025-04-07 14:43] pinefin: [replying to C2: "what youre saying reminds me of how someone found ..."]
thats what im talking about

[2025-04-07 14:43] pinefin: 2b2t shit

[2025-04-07 16:11] C2: [replying to pinefin: "2b2t shit"]
yeah cool concepts

[2025-04-07 16:11] C2: I already thjought about viausl stuff but I do not wanna do it

[2025-04-07 16:11] C2: its just too bulky idk

[2025-04-07 16:11] C2: wanted something more silent but whatever

[2025-04-07 16:12] pinefin: ooh

[2025-04-07 16:12] pinefin: i have an idea

[2025-04-07 16:12] pinefin: idk if its possible

[2025-04-07 16:12] pinefin: but in game

[2025-04-07 16:12] pinefin: have some like foreign language

[2025-04-07 16:12] pinefin: that shows up sometimes on billboards

[2025-04-07 16:12] pinefin: and you can decode that to an identifier

[2025-04-07 16:13] pinefin: 
[Attachments: stock-market-ticker-digital-display-055212065_prevstill.png]

[2025-04-07 16:13] pinefin: like these billboards in new york

[2025-04-07 16:13] pinefin: how they rotate text around the building and update the data with new stuff

[2025-04-07 18:27] the horse: I'm writing a x86_64 emulator, and I've got a couple questions to lead me towards a good path;

engine: capstone for disassembly, otherwise all in-house

Scope:
- Emulating a module and it's dependencies entirely; no sandboxing, direct access to memory, external module calls are completely emulated, syscalls are currently passthrough with special handlers at points of interest
- Emulating both kernel & usermode code, albeit focus currently is on usermode.

What interests me the most:
- Is there a better site with accurate x86 instruction semantics along with the "pseudocode" for how the cpu implements the instruction? When dealing with SSE/AVX, the information is more scarce.
- I'm not entirely sure how I'd emulate exceptions in kernel, I got a bit of inspiration from momo's emulator on how I could handle it on the client-side, is it fundamentally the same concept?
- What are some caveats that other emulators fall short in and trip detections in environments that attempt to prevent emulation?

[2025-04-07 23:42] James: [replying to the horse: "I'm writing a x86_64 emulator, and I've got a coup..."]
Why not just actually execute the instructions? Anything u don't support emulation of, just literally execute it. then int3 breakpoint after it to jump back to the emulator context.

you wouldn't want to fully execute everything, but there also is no reason to emulate every instruction.

[2025-04-07 23:44] James: another reason to do this would be, even with an accurate all knowing semantic lookup table, there are thousands of instructions and u do NOT want to waste ur time writing them all into an emulator

[2025-04-07 23:44] James: especially when most of them don't really matter.

[2025-04-08 00:54] daax: [replying to the horse: "I'm writing a x86_64 emulator, and I've got a coup..."]
agree with <@160202062548697108>, would recommend the same vs individually handling everything. bit absurd to try and handle every instruction, there are also errors/omissions in the spec that aren't replicated in most emulators, so anything leveraging those you'll probably fail

[2025-04-08 00:57] daax: which isn't really anyone's fault outside of the technical writers but it does make explicit emulation of everything nearly impossible to do with 100% accuracy

[2025-04-08 00:58] James: take it from me <@901468996229025873> , you don't want to write stuff for every inst. I've been doing it for ARM for a while now, and all I'm doing is register usage, not even proper semantics

[2025-04-08 00:59] daax: [replying to James: "take it from me <@901468996229025873> , you don't ..."]
arm, x86, msp430, doesn't matter. it's cooked all the way around for most any architecture.

[2025-04-08 01:01] James: the online arm docs are unbelievably bad. the manual is a bit better, but still annoying. 

also <@901468996229025873> doesn't sail exist? i know they are also inaccurate in some places but maybe not too bad. I wrote something a bit ago to parse the k-framework x86 stuff(it also had some issues though)

[2025-04-08 01:01] James: ANYTHING to avoid doing this by hand.

[2025-04-08 01:04] daax: [replying to James: "the online arm docs are unbelievably bad. the manu..."]
you mean you don't like fragmentation and inconsistent terms between sets? <:Kappa:794707301436358686>

[2025-04-08 01:06] James: [replying to daax: "you mean you don't like fragmentation and inconsis..."]
not at all

[2025-04-08 01:06] James: and im sure im making lots of mistakes in mine too

[2025-04-08 01:06] James: and im just doing register usage and encoding

[2025-04-08 01:07] James: ```Sha512su1_Vd2D_Vn2D_Vm2D,11001110011|VmR|100010|VnR|VdH,,,
```

[2025-04-08 01:07] James: here's ~2500th one ive written

[2025-04-08 01:07] James: im sure ive made mistakes

[2025-04-08 01:08] James: im thinking about offering bounty to people that find errors

[2025-04-08 01:09] the horse: the bp suggestion is pretty good

[2025-04-08 01:09] the horse: im already tired after 180

[2025-04-08 01:10] the horse: and it wouldnt compromise what i want

[2025-04-08 01:10] daax: [replying to James: "here's ~2500th one ive written"]
it'll happen with any given large instruction set. for small mcu's you sometimes will have to add in support for 20-bit register usage, 16-bit and 8-bit usage... i don't envy you in the slightest for this

[2025-04-08 01:10] daax: a bounty might be solid idea tbh

[2025-04-08 01:10] the horse: i spent a couple hours just fixing bit discarding

[2025-04-08 01:11] the horse: with basic arithmetic + sse extensions for them

[2025-04-08 01:13] daax: [replying to the horse: "with basic arithmetic + sse extensions for them"]
all the unintended side effects / what should cause exceptions vs what actually does. "complex microarchitectural conditions" as they call it. you'll have a bad time trying to make sure it all lines up with real hardware.

[2025-04-08 01:13] daax: save yourself

[2025-04-08 01:14] James: idk how much to offer for the bug bounty tbh, 20 per error found? but finding any number of errors might require someone to look over all the tables.

[2025-04-08 01:14] daax: do it how James mentioned or some other manner. easy to do with jit as well. check out xybak

[2025-04-08 01:15] daax: [replying to James: "idk how much to offer for the bug bounty tbh, 20 p..."]
if you go that route just make sure you audit prior and get close friends to do it for free maybe or for pizza or something, otherwise you might have a single mistake that was replicated across 200 items

[2025-04-08 01:16] daax: and then you're paying 200*20 <:harold:704245193016344596>

[2025-04-08 01:16] daax: im assuming you would ofc, but you know... that would suck

[2025-04-08 01:16] James: xD yeah

[2025-04-08 01:16] James: i mean if i make that mistake though, and they catch it

[2025-04-08 01:16] James: good for them

[2025-04-08 01:17] the horse: i guess the good part is that for 99% of my purposes I don't need to support any extension after 2012

[2025-04-08 01:18] the horse: because frankly i don't care about anything that doesn't run on my premium, state-of-the-art i7-3720qm

[2025-04-08 01:19] the horse: VGF2P8AFFINEINVQB 🙏

[2025-04-08 01:22] James: i would seriously just take a length decoder, decode the length of un-emulated instructions, copy them to some buffer somewhere, put an int3 after it, mov the entire emulated context in to the actual registers, and jump to the buffer. then catch the int3 and restore the original context before returning to your previous execution. 

ofc LOTS of little quirks to figure out there, also might be some problems with ur emulated stack potentially... but not sure. i think this is the easiest way to go about this though, and u can ignore stupid things like BT.

[2025-04-08 01:24] the horse: I already have context switches implemented between my "vm" and hardware

[2025-04-08 01:25] the horse: you are obth probably right

[2025-04-08 01:27] daax: [replying to the horse: "because frankly i don't care about anything that d..."]
tbh that's quite a lot anyways if you're referring to by hand. mmx, sse, sse2, sse3, sse4/4.2, aesni, avx/avx2, sha, clmul, rdrand, ... thinking about doing all of those by hand mm yikes

[2025-04-08 01:28] daax: that reminds me, whoever has to handle updating current emulators for apx -- god save them

[2025-04-08 01:29] James: lol

[2025-04-09 11:09] Deleted User: [replying to James: "Why not just actually execute the instructions? An..."]
is there an emu that actually emulates every instr?

[2025-04-09 13:34] pinefin: [replying to Deleted User: "is there an emu that actually emulates every instr..."]
behold. x86 vnni instructions.

[2025-04-09 14:10] Yoran: [replying to Deleted User: "is there an emu that actually emulates every instr..."]
Maybe bochs

[2025-04-09 15:23] daax: [replying to Deleted User: "is there an emu that actually emulates every instr..."]
bochs is great, simics

[2025-04-09 15:23] daax: <https://www.windriver.com/products/simics>

[2025-04-09 15:53] Deleted User: thanks

[2025-04-09 16:05] sync: [replying to daax: "<https://www.windriver.com/products/simics>"]
ty gonna cop and sell EAC emulator

[2025-04-10 18:00] UJ: [replying to daax: "<https://www.windriver.com/products/simics>"]
"contact us for pricing" = $$$$$$$$$$$$$$$$$$$$

[2025-04-10 18:01] pinefin: https://www.intel.com/content/www/us/en/developer/articles/tool/simics-simulator.html
[Embed: Intel® Simics® Simulator]
Learn about what is included in the public release of Intel® Simics® Simulator and Intel Simics Virtual Platform.

[2025-04-10 18:02] pinefin: <@609487237331288074> is this related to the one you sent in any capacity?

[2025-04-10 18:21] Matti: [replying to UJ: ""contact us for pricing" = $$$$$$$$$$$$$$$$$$$$"]
? the public version of simics is free

[2025-04-10 18:22] Matti: if you need the non-public version, the price isn't going to be your biggest concern anyway, it'll be the usual NDA BS

[2025-04-10 18:23] Matti: its biggest downsides compared to bochs are (1) requires VT-x, so only supports legacy intel CPUs, and (2) not open source

[2025-04-10 18:23] Matti: but it's very accurate

[2025-04-10 18:43] daax: [replying to pinefin: "<@609487237331288074> is this related to the one y..."]
yeah

[2025-04-10 18:45] Timmy: > requires VT-x, so only supports legacy intel CPUs
but the latest models have VT-x support according to the sku? Is there more to it than just what it reports?

[2025-04-10 18:46] Matti: no, that's correct

[2025-04-10 18:46] Matti: I just mean it doesn't support CPUs other than intels

[2025-04-10 18:52] UJ: [replying to Matti: "? the public version of simics is free"]
ah, i was only looking at the windriver link

[2025-04-10 18:52] Matti: yeah it was made free a few years ago

[2025-04-10 18:53] Matti: or rather a free version was made available which includes most of the features you'd want

[2025-04-10 19:36] pinefin: [replying to Matti: "or rather a free version was made available which ..."]
what features wouldnt be included? or is that behind the NDA

[2025-04-10 19:37] pinefin: just wondering if you'd know

[2025-04-10 19:37] Matti: I only saw the eclipse GUI mentioned explicitly

[2025-04-10 19:37] Matti: other than that, yeah I don't know

[2025-04-10 19:37] pinefin: eclipse as in

[2025-04-10 19:37] pinefin: the ide?

[2025-04-10 19:37] Matti: yes

[2025-04-10 19:37] UJ: yeah it seems to be thats what they use as the frontend based on the youtube videos i watched.

[2025-04-10 19:38] Matti: they're fond of it

[2025-04-10 19:38] pinefin: eh

[2025-04-10 19:38] UJ: they gotta get with the times and use vs code as the frontend.

[2025-04-10 19:38] Matti: the intel system debugger also is based on eclipse

[2025-04-10 19:38] pinefin: i love that silabs is starting to actually change to using vscode more

[2025-04-10 19:38] pinefin: moving away from the eclipse ide

[2025-04-10 19:38] pinefin: 😩

[2025-04-10 19:38] Matti: lol who cares

[2025-04-10 19:38] Matti: I can survive eclipse

[2025-04-10 19:38] pinefin: no thanks

[2025-04-10 19:39] pinefin: i enjoy my extensions

[2025-04-10 19:39] pinefin: i love my extensions actually

[2025-04-10 19:39] pinefin: id marry a vscode extension

[2025-04-10 19:39] Matti: it's not an IDE for coding in

[2025-04-10 19:39] Matti: they just use eclipse because you can implement pretty much any app you want in it with enough violence

[2025-04-10 19:40] pinefin: oh yeah

[2025-04-10 19:40] pinefin: i agree

[2025-04-10 19:40] Matti: in any case, the free version doesn't have it so I wouldn't worry about this fact

[2025-04-11 14:54] mrexodia: [replying to Matti: "its biggest downsides compared to bochs are (1) re..."]
They might have some kind of acceleration, but I was running it just fine on AMD

[2025-04-11 14:54] mrexodia: That was a very old (leaked) version though, so things might have changed since...

[2025-04-11 14:57] Matti: yeah simics definitely requires VT-x now and has done ever since they released the public version

[2025-04-11 14:57] Matti: they wouldn't forget to make that change first

[2025-04-11 14:58] Matti: it also requires an 11th gen intel(R) CPU now according to the system requirements page

[2025-04-11 15:01] Matti: with AMD the problem is the opposite... SimNow (of which simics is basically a clone) was once public but is not anymore

[2025-04-11 15:02] Matti: and it obviously does work on intel CPUs, or at least it did in the last public version

[2025-04-11 15:02] Matti: but that is from like 2013 so completely useless now

[2025-04-11 20:51] UJ: [replying to mrexodia: "They might have some kind of acceleration, but I w..."]
dumpulator on simics  👀

[2025-04-11 20:59] Matti: duncan meant simics, only from before it was made publicly available, I'm pretty sure

[2025-04-11 20:59] Matti: otherwise there wouldn't be much point in using a leaked version

[2025-04-11 21:00] Matti: but this was initially just how simics was licensed

[2025-04-11 21:01] Matti: I'm actually sure it's true that this version would work on AMD CPUs, it fits their MO so perfectly

[2025-04-11 21:04] Matti: and, by requiring VMX they are applying the lessons learned from the previous lawsuit over their software purposely disadvantaging AMD CPUs - namely the fact that AMD CPUs supported the same features [in the compiler case]

[2025-04-11 21:04] Matti: since AMD CPUs do not support VMX, this claim doesn't apply

[2025-04-11 21:22] Matti: took me a while to (re)find this, and as said before this is now fully useless software
but this is AMD SimNow 4.6.2 from 2010
[Attachments: SimNow.7z]

[2025-04-11 21:22] Matti: I love the GUI
[Attachments: image-1.png]

[2025-04-11 21:23] Matti: SimNow was actually public until 2013 from what I can find, but this is the most recent actual release I've got

[2025-04-11 21:25] Matti: also note how the "show deprecated devices" box is **not** checked here

[2025-04-11 21:28] Matti: it might give you ISA devices if you do, but given that it already defaults to PCI-X (not PCIe) for opterons that might actually be an improvement these days

[2025-04-11 21:29] Matti: this couldn't really have aged any worse <:lillullmoa:475778601141403648>

[2025-04-12 03:13] Hunter: [replying to Matti: "took me a while to (re)find this, and as said befo..."]
Nice

[2025-04-12 03:16] Hunter: This will be interesting to look at

[2025-04-13 23:42] oopsies: [replying to James: "and im sure im making lots of mistakes in mine too"]
The armv9 reference manual pdf is formatted in a way thats hard to parse in a script

[2025-04-13 23:43] oopsies: i could parse the msrs and everything but the instructions

[2025-04-13 23:46] James: [replying to oopsies: "The armv9 reference manual pdf is formatted in a w..."]
i eventually just decided to write it all myself

[2025-04-13 23:46] James: ```

Orn_Vd8B_Vn8B_Vm8B,0|0|001110111|VmR|000111|VnR|VdW,,,
Orn_Vd16B_Vn16B_Vm16B,0|1|001110111|VmR|000111|VnR|VdW,,,
```

[2025-04-13 23:46] oopsies: [replying to James: "i eventually just decided to write it all myself"]
That is insane.

[2025-04-13 23:46] James: register usage information isn't encoded anywhere, other than plain ENGLISH text

[2025-04-13 23:46] James: and even that is incorrect sometimes.