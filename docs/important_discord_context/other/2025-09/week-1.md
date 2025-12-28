# September 2025 - Week 1
# Messages: 307

[2025-09-01 23:01] abu: [replying to brew002: "I created a POC for this awhile ago. Stability is ..."]
this is super cool. Thank you

[2025-09-02 05:15] KR: [replying to mtu: "I’ve never been able to decide if this is the wors..."]
I mean, kleptography *is* a thing but moreso in underlying implementations than the algorithms themselves. With that said  the NSA *does* have a history of it. Check out NIST SP 800-90A.

[2025-09-02 05:17] KR: Easier to hide a "broken" RNG in a niche piece of silicon that no one will ever look at than publishing and convincing people to adopt a flawed cryptographic standard

[2025-09-02 11:05] mtu: As noted above, the only known instance of the NSA making a deliberate choice to alter a public cryptosystems security was to increase its security. The dual EC stuff is muddy at best, and amounts to “dual EC is broken, and because we don’t trust the NSA we assume it was deliberate.” The saying “don’t attribute to malice what can be attributed to incompetence” might apply

[2025-09-02 14:05] Matti: [replying to mtu: "As noted above, the only known instance of the NSA..."]
I really have to disagree re: dual EC_DRBG, believing this was a deliberate backdoor really doesn't *need* you to distrust the NSA as a prerequisite. (if anything, it's more of a consequence)

Timeline of highlights:
Aug 2007: <https://rump2007.cr.yp.to/15-shumow.pdf> Modest title: "On the Possibility of a Back Door in the NIST SP800-90 Dual Ec Prng"
Nov 2007: <https://www.schneier.com/blog/archives/2007/11/the_strange_sto.html> Not really any new information (though an informative background article). Schneier is a lot less hesitant to call the design an intentional backdoor, though still stops just short of saying the NSA intentionally put it there. After all they were only promoting EC_DRBG like you said. *Could* be a coincidence.

2013: Edward Snowden makes a couple of faxes

Seo 2013: <https://www.theguardian.com/world/2013/sep/21/rsa-emc-warning-encryption-system-nsa> RSA tells its customers to stop using EC_DRBG, saying it is insecure
Dec 2013: <https://web.archive.org/web/20131221000558/https://www.theguardian.com/world/2013/dec/20/nsa-internet-security-rsa-secret-10m-encryption> (archive.org link because this is the only Guardian article I've ever seen that was removed from its webiste) Reuters reports that RSA was paid $10M by the NSA to use EC_DRBG in their Bsafe encryption software.

[2025-09-02 14:08] mtu: Right, so for that to equate to “ec dbrg was deliberately backdoored” you have to believe that the design flaw was intentional, and the arrangement with RSA was an indicator of bad intent

[2025-09-02 14:10] Matti: yes

[2025-09-02 14:10] Matti: I thought that would be obvious

[2025-09-02 14:10] Matti: you do have to believe that

[2025-09-02 14:10] mtu: You started with that you don’t need to distrust the NSA as a prerequisite

[2025-09-02 14:10] Matti: lmfao

[2025-09-02 14:11] Matti: no, I only distrust them now

[2025-09-02 14:11] mtu: The whole thing is wonky and I look forward to the FOIA in like 15 years don’t get me wrong

[2025-09-02 14:11] mtu: But, like, the NSA is also responsible for the security of government systems - would they knowingly insert a backdoor into the cryptosystems they require DoD comms to implement?

[2025-09-02 14:11] Brit: yes

[2025-09-02 14:12] cat weed: how much is the nsa paying you

[2025-09-02 14:12] Brit: not enough

[2025-09-02 14:13] mtu: See: malicious vs incompetent

[2025-09-02 14:14] Brit: we can accuse the NSA of being many things incompetent is usually not one of them

[2025-09-02 14:14] mtu: I don’t doubt that it happened, but I struggle with the narrative that the NSA cryptobros somehow thought “no one will ever find this backdoor” when the entire lives of cryptobros is based on the premise that sufficient research can always uncover broken cryptosystems

[2025-09-02 14:15] Matti: if you look at the first slides, you'll see that it wasn't exactly "no one" who found this

[2025-09-02 14:15] Matti: so maybe they did

[2025-09-02 14:15] Matti: I can't say, I don't know what drives the NSA to do the things it does

[2025-09-02 14:16] Matti: [replying to Brit: "we can accuse the NSA of being many things incompe..."]
and this...

[2025-09-02 14:17] mtu: You mean the slides that also say “we don’t know how the secret number was chosen, so it’s possible that it was chosen to be insecure, but we have no evidence of that”

[2025-09-02 14:17] Matti: yes, those

[2025-09-02 14:17] Matti: because it's a mathematical proof ffs

[2025-09-02 14:18] Matti: you don't go appropriating blame to entities in proofs

[2025-09-02 14:18] mtu: [replying to Matti: "and this..."]
Ok, so if they’re not incompetent then they wouldn’t intentionally backdoor a system in a discoverable way

[2025-09-02 14:18] Brit: "discoverable" is doing a lot of heavy lifting

[2025-09-02 14:18] Brit: you need some serious crypto chops to figure out why DECDRBG is problematic

[2025-09-02 14:19] mtu: Not really, the proof Matti linked to is quite literally just going step by step through the algorithm

[2025-09-02 14:19] mtu: There was no “new math” required

[2025-09-02 14:19] mtu: Trust me I’ve rabbit holed _hard_ on this, it’s my favorite extremely controversial take

[2025-09-02 14:20] Matti: I feel like it's more like denial in the face of overwhelming evidence

[2025-09-02 14:20] mtu: The “potential backdoor” is almost exactly the same as the “potential backdoor” in DES, which is why it’s relevant that the DES “backdoor” was actually a security improvement

[2025-09-02 14:22] mtu: To be clear the total evidence is:
* A constant was chosen in a non-transparent way
* The value of that constant could be fundamentally insecure, but it cant be shown whether it is or isn’t
* the NSA paid RSA to implement it

[2025-09-02 14:24] Matti: > the NSA paid RSA to implement it
in secret, a fact which only came to light after RSA had already told its own customers to stop using EC_DRBG

[2025-09-02 14:24] mtu: Compare that to the DES story where
* s boxes were chosen in a non transparent way
* it was shown that it was possible that the choice could have been made to compromise the security of DES
* the NSA compelled industry to use DES as they designed despite it having less bits of security

[2025-09-02 14:25] Matti: why would I have to compare them? I never made any claim re: DES

[2025-09-02 14:25] Matti: this is about EC_DRBG

[2025-09-02 14:25] Brit: it was eventually leaked that the NSA has an entire project dedicated to weakening public crypto schemes to begin with

[2025-09-02 14:25] Brit: this is not even up to debate

[2025-09-02 14:26] Matti: yes

[2025-09-02 14:26] Brit: although the funniest part to me is RSA taking deals to set the default CSPRNG

[2025-09-02 14:27] Brit: like the incentives are wholy transparent

[2025-09-02 14:27] mtu: [replying to Matti: "why would I have to compare them? I never made any..."]
Because both were, in their contemporary time, examples of “NSA backdoor”

[2025-09-02 14:27] Matti: [replying to Brit: "although the funniest part to me is RSA taking dea..."]
according to the guardian article, $10M was 1/3rd of their revenue for that year 😔

[2025-09-02 14:27] Matti: [replying to mtu: "Because both were, in their contemporary time, exa..."]
yes and that's about where the similarities end

[2025-09-02 14:27] cat weed: good year for RSA

[2025-09-02 14:27] Brit: I get the financial incentive

[2025-09-02 14:28] Matti: yeah igwym

[2025-09-02 14:28] Matti: it is funny

[2025-09-02 14:28] Matti: possibly the only part you really could call incompetence on the side of the NSA

[2025-09-02 14:29] mtu: [replying to Brit: "it was eventually leaked that the NSA has an entir..."]
There’s no actual public link between those programs and the dual EC stuff, a bunch of articles conflate the two without evidence

[2025-09-02 14:30] mtu: Incidentally, there _are_ public links between those programs and RC4 weaknesses

[2025-09-02 14:30] Matti: how old are you? were you around in 2013?

[2025-09-02 14:30] Brit: do you not see the incentive?

[2025-09-02 14:31] mtu: [replying to Matti: "how old are you? were you around in 2013?"]
Yep

[2025-09-02 14:32] Matti: OK, then I'm not sure how you could refer to a bunch of articles conflating the two "without evidence"

[2025-09-02 14:32] mtu: [replying to Brit: "do you not see the incentive?"]
I see the incentive for the code breaking part of the NSA, yes. I don’t see the incentive for the NIST side - why break a CSPRNG that you’re going to compel DoD systems to use? The NSA isn’t just offensive, they’re responsible for defending DoD systems too, and the standards folks are solidly in the second category

[2025-09-02 14:33] Matti: the faxes contained a bunch of evidence as I recall

[2025-09-02 14:33] Matti: might have been quite a lot of faxes...

[2025-09-02 14:33] Matti: and yes EC_DRBG was implicated

[2025-09-02 14:34] mtu: I’ll be frankly shocked if you can find a single leaked doc that names EC DBRG as an implicated crypto system

[2025-09-02 14:35] Brit: why pay to set is a default csprng?

[2025-09-02 14:35] mtu: Iirc the leaks were basically “NSA seeks to compromise crypto” and cryptologists then pointed to the fact that EC DBRG constants weren’t transparent and _could_ be backdoored as good enough reason to stop using them, which I don’t agree with

[2025-09-02 14:36] mtu: [replying to Brit: "why pay to set is a default csprng?"]
If the people you’re supposed to defend use RSA libs and the constant you chose makes the csprng resistant to some attack only you know

[2025-09-02 14:36] Matti: [replying to mtu: "I’ll be frankly shocked if you can find a single l..."]
https://arstechnica.com/information-technology/2013/09/new-york-times-provides-new-details-about-nsa-backdoor-in-crypto-spec/
[Embed: New York Times provides new details about NSA backdoor in crypto spec]
The paper points a finger definitively at the long-suspected Dual_EC_DRBG algorithm.

[2025-09-02 14:36] mtu: Again see: “why change the s box and reduce the key size for DES?”

[2025-09-02 14:36] Matti: but ffs

[2025-09-02 14:36] Matti: [replying to Brit: "why pay to set is a default csprng?"]
it's really this simple

[2025-09-02 14:36] Matti: why indeed

[2025-09-02 14:37] mtu: [replying to Matti: "https://arstechnica.com/information-technology/201..."]
“But internal memos leaked by a former N.S.A. contractor, Edward Snowden, suggest that the N.S.A. generated one of the random number generators…”

[2025-09-02 14:38] Brit: [replying to mtu: "Again see: “why change the s box and reduce the ke..."]
you realize both can be true

[2025-09-02 14:38] mtu: I do, yes

[2025-09-02 14:38] Brit: I'm not even an NSA hater

[2025-09-02 14:39] Brit: it's just glaringly obvious

[2025-09-02 14:39] mtu: My stance isn’t “no shot the nsa would backdoor ec”

[2025-09-02 14:39] mtu: Its that it’s not the “definitely was backdoored” people claim

[2025-09-02 14:40] Brit: why make the contract secret

[2025-09-02 14:40] Brit: if it was just to ensure the security of systems relying on the RSA stds

[2025-09-02 14:41] Matti: [replying to mtu: "Its that it’s not the “definitely was backdoored” ..."]
if you scroll up to the timeline I helpfully made for you, you'll see that bruce schneier was in fact effectively saying exactly this in 2007, long before the snowden leaks

[2025-09-02 14:41] mtu: Same reason you’d keep the reason you chose the DES constants secret, to reduce the likelihood of discovery of your secret

[2025-09-02 14:41] mtu: [replying to Matti: "if you scroll up to the timeline I helpfully made ..."]
Yeah and I never disagreed with that

[2025-09-02 14:42] mtu: That’s why I pulled out the quote from the original slides on “the constant was chosen in ways we don’t know, in theory that means it could be backdoored but we can’t prove either way”

[2025-09-02 14:42] Matti: 😩

[2025-09-02 14:42] Matti: because they can't

[2025-09-02 14:42] mtu: Yeah so why does that mean “nsa definitely backdoored it”

[2025-09-02 14:43] Matti: are you asking for a formal proof that they did

[2025-09-02 14:44] Matti: I say they definitely did backdoor it because everything points to this

[2025-09-02 14:44] Matti: that is all

[2025-09-02 14:44] mtu: Or actual evidence that they did and not the total evidence being “well they mathematically could have, and they paid RSA to adopt it”

[2025-09-02 14:45] Matti: that is pretty much the evidence, yeah

[2025-09-02 14:45] Matti: if that's not enough I don't know what to tell you

[2025-09-02 14:45] mtu: To be ultra clear I’m also not saying “we should keep using algorithms we can’t be sure are secure because of wacky constants”

[2025-09-02 14:46] mtu: My original message was “the dual ec thing is murky at best”

[2025-09-02 14:47] Brit: [replying to mtu: "Yeah so why does that mean “nsa definitely backdoo..."]
my confidence interval is more like 95%

[2025-09-02 14:48] Brit: 5% chance that a sequence of incompetence happened and the NSA capitalized on it by paying RSA to make the bad algo the standard

[2025-09-02 14:49] Matti: yeah I also say "they definitely did" the same way I say "god doesn't exist", even though I can't prove this

[2025-09-02 14:49] Matti: [replying to Brit: "5% chance that a sequence of incompetence happened..."]
this isn't impossible but I still feel 95% is being generous

[2025-09-02 14:49] mtu: [replying to Brit: "5% chance that a sequence of incompetence happened..."]
Again, “bad algo” is the assumption here

[2025-09-02 14:49] Matti: I'll go 99

[2025-09-02 14:50] mtu: The constant could be chosen for any variety of reasons, some of which are bad, some are good, some are neutral

[2025-09-02 14:50] Brit: why are they unable to explicitely state how the consts were chosen

[2025-09-02 14:50] Brit: benzene group kinda beat: it came to me in a dream

[2025-09-02 14:50] mtu: Who knows, maybe it was entropy generated from a cat gif, maybe it was to defeat some classified analysis technique

[2025-09-02 14:51] mtu: We won’t know for another 15ish years

[2025-09-02 14:51] Brit: watch it be bullrun

[2025-09-02 14:51] Brit: or some other analogue

[2025-09-02 14:51] Brit: anyway, it does not matter

[2025-09-02 14:51] Brit: the NSA is not invested in the public having strong cryptography

[2025-09-02 14:51] Brit: for obvious reasons

[2025-09-02 14:52] Brit: in fact I do not think there's a govt out there currently who wants people to have access to cryptography

[2025-09-02 14:52] Brit: again

[2025-09-02 14:52] Brit: for obvious reasons

[2025-09-02 14:52] Matti: ++

[2025-09-02 14:52] mtu: As long as you don’t consider AES strong cryptography?

[2025-09-02 14:54] Matti: AES came out of a years long competition and standardization process by NIST, which while technically part of the government really isn't the same thing as the government

[2025-09-02 14:55] Matti: not that NIST can't be influenced, see the very PRNG we're discussing... but in general I would call them trustworthy

[2025-09-02 14:57] mtu: NIST is required by law to consult with the NSA on all things crypto

[2025-09-02 14:58] Matti: that hardly means the NSA gets to decide who wins the AES competition

[2025-09-02 14:58] Matti: it hardly means anything at all

[2025-09-02 14:59] mtu: My point is that saying “the agency responsible for the security of the DoD and by extension the defense industrial base is not invested in the public having strong cryptography” is just wrong

[2025-09-02 15:00] Matti: is it

[2025-09-02 15:00] Brit: there has never been any inter agency "fuckery" ever

[2025-09-02 15:00] Brit: in the history of mankind

[2025-09-02 15:00] Brit: and especially not in the civilized western world

[2025-09-02 15:00] Brit: thankfully

[2025-09-02 15:01] Matti: yeah the US agencies in particular are one tight knit happy family

[2025-09-02 15:01] mtu: [replying to Matti: "is it"]
Considering the NSA straight up publically says “yes you can use commercial encryption for classified information”…. Yes?

[2025-09-02 15:02] Matti: one counterexample does not prove your statement either way

[2025-09-02 15:03] Matti: or two, or three

[2025-09-02 15:03] mtu: That’s a pretty big counterpoint to “the NSA has no incentive to make public encryption strong”

[2025-09-02 15:03] Brit: I should ban you

[2025-09-02 15:03] Brit: for this

[2025-09-02 15:03] Matti: I looked

[2025-09-02 15:03] Matti: I don' tknow and I don't care

[2025-09-02 15:03] mtu: We’re busy discussing anti cheats

[2025-09-02 15:03] mtu: GOD

[2025-09-02 15:03] Matti: I think I might have posted otherwise

[2025-09-02 15:04] Matti: after all you can see when new posts appear in channels

[2025-09-02 15:04] Brit: consider what would happen if someone had that channel collapsed

[2025-09-02 15:04] Brit: and could not see the crucially important question of which protector to use for his p2c

[2025-09-02 15:04] Matti: collapsing channels is a PITA

[2025-09-02 15:05] Matti: I tend to just delete them

[2025-09-02 15:05] cat weed: guys enigma themida or vmp guys

[2025-09-02 15:05] cat weed: plz

[2025-09-02 15:05] Brit: use riscybusiness instead :^)

[2025-09-02 15:05] mtu: Don’t use a VM, there was a def con talk about a system that automatically devirtualizes all VMs

[2025-09-02 15:06] cat weed: yeah you’ll get hit with the vm dragon slayer

[2025-09-02 15:06] cat weed: it’s over

[2025-09-02 15:06] Brit: people who care will look in the channel anyway

[2025-09-02 15:06] Brit: no one needs an additional reminder

[2025-09-02 15:07] Matti: [replying to mtu: "That’s a pretty big counterpoint to “the NSA has n..."]
coming back to this... no it just really isn't

[2025-09-02 15:07] Matti: they obviously do have an incentive

[2025-09-02 15:08] Matti: I actually don't know the reason why they endorse an algorithm that (likely) is not backdoored, other than them having no real other options (or better ones that are not public)

[2025-09-02 15:08] Matti: but can you not see why they obviously would have an incentive for people to use weak crypto

[2025-09-02 15:09] Matti: it's because their job is to spy on people

[2025-09-02 15:22] the horse: backdooring the RNG is probably more likely than the actual algorithms being faulty

[2025-09-02 15:24] the horse: if it can be used to 'eliminate' n bits of randomness even one bit is a huge amount of work decreased

[2025-09-02 15:25] the horse: i can't really fathom how you can influence this effectively and with confidence at chip level

[2025-09-02 15:26] Brit: <:mmmm:904523247205351454>

[2025-09-02 15:26] the horse: but that's just lack of HW understanding

[2025-09-02 15:26] Brit: odd how people use other sources of randomness rather than relying on rdrand

[2025-09-02 15:27] the horse: question is what the majority will use

[2025-09-02 15:27] Matti: RtlUniform()

[2025-09-02 15:27] Matti: performance > security

[2025-09-02 15:27] Brit: return 4;

[2025-09-02 15:27] Brit: good enough

[2025-09-02 15:27] Brit: chosen by fair diceroll etc

[2025-09-02 15:27] Matti: true, true

[2025-09-02 15:28] the horse: best case is to train a monkey to shuffle a deck of cards; incorporate the time it took him to shuffle, and whether he jacked off during that time

[2025-09-02 15:28] the horse: i'd use that for lotteries

[2025-09-02 15:28] the horse: true rng

[2025-09-02 15:29] the horse: or maybe just movement-data from a busy indian street

[2025-09-02 15:29] mtu: lava lamp has entered the chat

[2025-09-02 15:30] Brit: take some fast decaying material and measure time until the parent nucleus loses 2proton +2 neutron

[2025-09-02 15:30] Brit: its nice because we know the shape of the distribution

[2025-09-02 15:30] the horse: too expensive

[2025-09-02 15:31] mtu: What is the SOTA for physics-based entropy measurement? I assume it's not still "jiggle your mouse around while waiting for TrueCrypt (tm)"

[2025-09-02 15:32] Brit: [replying to the horse: "too expensive"]
consider the fact that you get free glow in the dark features

[2025-09-02 15:32] the horse: we don't like things that glow

[2025-09-02 15:32] Brit: [replying to mtu: "What is the SOTA for physics-based entropy measure..."]
people do all sorts of things, measure cosmic background radiation, lava lamps, etc etc etc

[2025-09-02 15:33] the horse: make it wait until a cosmic ray flips a bit in the processor

[2025-09-02 15:33] Brit: shrimple

[2025-09-02 15:33] the horse: (funny thing this happened once during some France elections)

[2025-09-02 15:34] the horse: and some woman got 4096 votes because of it

[2025-09-02 15:34] the horse: and what causes my PC to bsod 😡

[2025-09-02 15:54] avx: wasn't that some random tiny Belgian town

[2025-09-02 15:54] the horse: maybe

[2025-09-02 15:54] avx: hence +4096 was super noticeable

[2025-09-02 15:54] the horse: I don't remember exactly

[2025-09-02 15:55] the horse: but yes it was something very small; maybe mayor elections or something

[2025-09-02 16:01] avx: yeeee

[2025-09-02 16:01] the horse: there was a large scale investigation

[2025-09-02 16:01] the horse: and it only lead to this

[2025-09-02 16:01] the horse: actually crazy

[2025-09-02 16:01] avx: in a big city I don't think it would have been as obvious

[2025-09-02 17:32] dullard: Meanwhile I have my LUKS key permanently corrupt when that happens 😂

[2025-09-02 17:33] dullard: Was in my final year of university and my system just didn’t accept my LUKS password, was not a fun couple of weeks of coursework grinding

[2025-09-02 17:44] avx: holy shit 😭

[2025-09-02 18:59] Vonnegut: [replying to the horse: "and what causes my PC to bsod 😡"]
I can almost guarantee you won't be seeing cosmic rays causing BSODs on your PC

[2025-09-02 22:18] KR: [replying to mtu: "Don’t use a VM, there was a def con talk about a s..."]
You mean the project that was out by "end of august", has a commit history that looks like this and reads like an AI shit post?
[Attachments: image.png]

[2025-09-02 22:19] KR: I didn't attend myself but from what I've heard... It does not inspire confidence

[2025-09-02 22:19] KR: *and still isn't released*

[2025-09-02 22:21] the horse: real men don't give titles/description to their commits

[2025-09-02 22:21] the horse: this is the way to go
[Attachments: image.png]

[2025-09-02 22:22] KR: Wait, no, nvm

[2025-09-02 22:22] KR: The entire source is in their deleted git history XD

[2025-09-02 22:22] KR: 
[Attachments: image.png]

[2025-09-02 22:22] KR: What a joke XD

[2025-09-02 22:22] Brit: [replying to the horse: "this is the way to go"]
fuck fuck fuck fuck fuck etc

[2025-09-02 22:23] KR: [replying to the horse: "this is the way to go"]
I respect it.

[2025-09-02 23:37] dan: funny how, before trying to nuke the repository, he made a desperate attempt to scrub all the AI comments
[Attachments: image.png]

[2025-09-02 23:37] dan: in a real implementation...

[2025-09-02 23:43] avx: didnt it say end of august for full release

[2025-09-03 00:30] toasts: [replying to avx: "didnt it say end of august for full release"]
never said what year

[2025-09-03 00:31] toasts: <:tomato:1352529797644812308>

[2025-09-03 00:32] avx: true <:tomato:1352529797644812308>

[2025-09-03 05:27] iris: why even publish anything at that point...

[2025-09-03 08:13] Matti: [replying to Brit: "fuck fuck fuck fuck fuck etc"]
how do you know about my private llvm commit history
[Attachments: image.png]

[2025-09-03 08:53] koyz: only 6 fucks in ~600 commits, have to up my game <a:aPES2_SadViolin:929006632858439731>
[Attachments: image.png]

[2025-09-03 09:25] avx: [replying to iris: "why even publish anything at that point..."]
the clicks !

[2025-09-03 09:26] avx: billions must acquire clout twitter said so

[2025-09-03 10:00] Matti: hmm that looks more like my unreal engine repo

[2025-09-03 10:01] Matti: but referring to epic instead

[2025-09-03 10:04] Matti: not technically a commit message but this was a merge conflict a few months after I committed this
[Attachments: image.png]

[2025-09-03 10:06] Matti: I've given up making PRs for everything, there's just too many fucking bugs

[2025-09-03 11:59] brockade: [replying to Matti: "how do you know about my private llvm commit histo..."]
I like how anyone who finally gets good at cmake spends the rest of their life just telling people how easy cmake is and why everyone should use it

[2025-09-03 12:14] Timmy: cmake sucks

[2025-09-03 12:14] Timmy: but its the best we have

[2025-09-03 13:07] sariaki: honestly, using cmake with llms makes it pretty bearable imo. i don't want to learn the specifics of some esolang and now i don't have to

[2025-09-03 13:14] dullard: [replying to koyz: "only 6 fucks in ~600 commits, have to up my game <..."]
I used to be able to open "everything" and search for fuck on my old system and have hundreds and hundreds of test files show up <:kekw:899071675524591637> 
fuck
fuck1 
fuckfuckfuck
ohfuckohshit type of beat

[2025-09-03 13:17] Brit: I have 436 matches for fuck in my git history <:yea:904521533727342632>

[2025-09-04 19:39] cinder: [replying to the horse: "this is the way to go"]
this is how it should be done, but I always keep in mind that I may open source my private stuff in the future, so I always commit professionally like I have other people watching over me

[2025-09-04 19:40] cinder: the reason I do this is that I cannot release some old stuff because I am genuinely afraid of what the commit history is and what is inside the source code, so when I was introduced to git history manipulation by my coworkers I started doing it for my private stuff as well

[2025-09-04 19:43] cinder: but I am not a good example, there is no viable reason to write professional commit messages (nor take the time to rebase them) for stuff you develop alone

[2025-09-04 19:43] cinder: i am merely stupid

[2025-09-04 19:45] cinder: now that I think about it one good thing that comes out from a clean commit history is that returning on a project after not working it for a long while is less painful

[2025-09-04 20:07] koyz: create orphan branch -> git add -> delete main -> rename current to main -> clean commit history <:Kappa:807349187350888499>

[2025-09-04 20:11] cinder: that's one issue solved

[2025-09-04 20:19] Addison: [replying to cinder: "this is how it should be done, but I always keep i..."]
ah yes professional commits for public repos

[2025-09-04 20:19] Brit: who's gonna tell him

[2025-09-04 20:24] Addison: meanwhile

[2025-09-04 20:24] Addison: https://github.com/TAMU-CSE/mitre-ectf2021
[Embed: GitHub - TAMU-CSE/mitre-ectf2021]
Contribute to TAMU-CSE/mitre-ectf2021 development by creating an account on GitHub.

[2025-09-04 20:26] Addison: This was a national competition lol

[2025-09-04 20:27] Addison: we were graded on commit quality and no one told me 😂

[2025-09-04 20:27] Addison: At least there was no swearing

[2025-09-04 22:05] mtu: I would have just force pushed everything into “Initial commit”

[2025-09-04 23:18] UJ: [replying to koyz: "create orphan branch -> git add -> delete main -> ..."]
send this to the vm slayer guy pls.

[2025-09-05 04:21] daax: [replying to Addison: "https://github.com/TAMU-CSE/mitre-ectf2021"]
MITRE CTF <:lmao3d:611917482105765918>

[2025-09-05 04:22] daax: “Painting with the blind” sounds more difficult

[2025-09-05 04:23] daax: [replying to Addison: "we were graded on commit quality and no one told m..."]
how very MITRE of them

[2025-09-05 07:22] cinder: [replying to Addison: "we were graded on commit quality and no one told m..."]
what's the point lmao

[2025-09-05 07:22] cinder: everyone has it's own style

[2025-09-05 11:34] mtu: [replying to daax: "“Painting with the blind” sounds more difficult"]
eCTF isn’t exactly trivial 

https://rules.ectf.mitre.org

[2025-09-05 14:13] daax: [replying to mtu: "eCTF isn’t exactly trivial 

https://rules.ectf.mi..."]
I would argue otherwise. ~~Might have to participate next time.~~ nvm it’s students only. either way, shame that their staff can’t even keep up in the embedded space.

[2025-09-05 14:14] mtu: “embedded dev is easy, you just have to have done it before” is something I was told years ago that probably applies here

[2025-09-05 14:14] daax: [replying to mtu: "“embedded dev is easy, you just have to have done ..."]
That I have

[2025-09-05 14:15] daax: And re/vr for embedded

[2025-09-05 14:15] daax: Reading this sounds like another day another dollar.

[2025-09-05 14:15] mtu: Yeah same logic applies

[2025-09-05 14:15] daax: MITRE is sus

[2025-09-05 14:15] mtu: I recently worked with some windows heavy RE folks and they were on the struggle bus

[2025-09-05 14:15] daax: [replying to mtu: "I recently worked with some windows heavy RE folks..."]
Two different worlds

[2025-09-05 14:16] mtu: “Wym theres no os”

[2025-09-05 14:16] mtu: “You’re telling me this arm binary is the only thing that’s running? Where’s the file system”

[2025-09-05 14:19] Brit: I refuse to believe these people are employed

[2025-09-05 14:19] daax: [replying to Brit: "I refuse to believe these people are employed"]
They are

[2025-09-05 14:19] daax: and then they try to run and hide

[2025-09-05 14:19] Brit: question being HOW ofc

[2025-09-05 14:20] daax: People hiring are also without experience and want to mask

[2025-09-05 14:20] Brit: because like if you are interviewing for an embedded RE position and your question is anything to do with some OS to interract with you are getting the boot within the week.

[2025-09-05 14:21] Brit: (caveat being RTOS etc etc you get the point)

[2025-09-05 14:23] daax: [replying to Brit: "because like if you are interviewing for an embedd..."]
I mean lots of embedded runs with se linux or some other linux distro, but yeah generally being unprepared for that is wild but it does happen. I imagine the windows people did the ctf for fun but still, gotta have a few basics

[2025-09-05 14:25] mtu: This wasn’t an embedded position

[2025-09-05 14:25] mtu: They’re friends from a workplace that focuses on “normal” stuff like windows/linux/mac/android/iphone

[2025-09-05 14:26] Brit: I guess that's understandable

[2025-09-05 14:26] mtu: Yeah it’s not like they said things like “computer must have os” they just legit didn’t know that tiny embedded computer in VR glasses was just going to be a singular cortex blob

[2025-09-05 14:27] mtu: It doesn’t help that like you said some embedded stuff does have an OS and almost all the “intro to embedded” stuff only covers those

[2025-09-05 14:37] Addison: [replying to daax: "I would argue otherwise. ~~Might have to participa..."]
You are not a student :p

[2025-09-05 14:37] Addison: But it was very difficult for the typical university bachelor's student

[2025-09-05 14:38] Addison: Just me and a couple other folks actually understood the infra enough to participate

[2025-09-05 15:08] daax: [replying to Addison: "But it was very difficult for the typical universi..."]
Sure sure, probably is difficult for students. I just have had so many poor interactions with people from certain companies, that seem to just push out coasters.

[2025-09-05 15:08] Addison: With you there for the most part

[2025-09-05 15:12] daax: [replying to Addison: "Just me and a couple other folks actually understo..."]
Did you guys wind up winning? What was the most difficult part for you guys?

[2025-09-05 15:14] Addison: Second -- there was a graduate research team from NEU that was trolling the competition lol

[2025-09-05 15:14] Addison: idk why they were allowed to participate, they were a bunch of doctoral students and the rest were undergrads

[2025-09-05 15:14] Addison: Most difficult part was probably digging through other people's code

[2025-09-05 15:14] Addison: There was some really horrible code distributed

[2025-09-05 15:15] mtu: [replying to daax: "Sure sure, probably is difficult for students. I j..."]
On the other end of the spectrum I know of a mitre employee that doxed a dude and called his wife threatening to attack them because they disagreed on politics

[2025-09-05 15:15] mtu: #justmitrethings

[2025-09-05 15:15] Addison: hell yeah, settling things the professional way

[2025-09-05 15:16] daax: [replying to mtu: "On the other end of the spectrum I know of a mitre..."]
anything to own the libs

[2025-09-05 15:16] daax: [replying to Addison: "There was some really horrible code distributed"]
just like real life <:kekW:626450502279888906>

[2025-09-05 15:16] daax: perfection

[2025-09-05 15:17] Addison: lol I think we were the only team to actually distribute code publicly after the fact

[2025-09-05 15:18] Addison: We just said "no" to the C abstraction layer they gave us and implemented our own HALs in Rust

[2025-09-05 15:18] Addison: got an award for it too lmao

[2025-09-05 15:18] Addison: https://engineering.tamu.edu/news/2021/05/csce-engineering-student-team-takes-second-in-virtual-embedded-security-system-competition.html
[Embed: Engineering student team takes second in virtual embedded security ...]
A team of students from the Texas A&M University College of Engineering recently placed second at the MITRE Corporation's 2021 Embedded Capture the Flag competition, a two-phase competition that takes

[2025-09-05 15:19] Addison: hacking 😎
[Attachments: CSCE-News-eCTF-competition-team-3May2021.png]

[2025-09-05 17:14] daax: [replying to Addison: "We just said "no" to the C abstraction layer they ..."]
based af

[2025-09-05 17:15] daax: [replying to Addison: "hacking 😎"]
i want this background for my zoom calls

[2025-09-05 17:16] Addison: [replying to daax: "i want this background for my zoom calls"]
https://www.gather.town/
[Embed: Gather | Virtual HQ for Remote Teams]
Work remotely side-by-side in digital Spaces that make virtual interactions more human.

[2025-09-07 15:41] Deus Vult: [replying to Addison: "https://engineering.tamu.edu/news/2021/05/csce-eng..."]
Nice, that is impressive!

[2025-09-07 15:41] Addison: Thanks! Was certainly proud of it a few years ago, since it was my first dabble in developing anything embedded