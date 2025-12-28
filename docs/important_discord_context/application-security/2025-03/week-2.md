# March 2025 - Week 2
# Channel: #application-security
# Messages: 87

[2025-03-04 15:44] pinefin: [replying to contificate: "I am not a language lawyer, either"]
AHEM

[2025-03-05 23:15] thismightwork: I have a question on obfuscating x64:

How do you preserve eflags when doing MBA or basic substitution?

Why am I asking? If I save them (in many ways), then it's easy to mark when it starts/end.

[2025-03-05 23:34] pinefin: this might work....

[2025-03-05 23:34] pinefin: (i dont have a serious answer i just like your name)

[2025-03-06 00:05] contificate: There's probably ways to obfuscate the preservation of flags

[2025-03-06 00:05] contificate: recall, though, that you the "live-in" EFLAGS to each instruction is obvious and it will probably also be likely that the instruction that last set it is obvious as well

[2025-03-06 00:06] contificate: when you lift, you make sure to model the effect explicitly

[2025-03-06 00:06] contificate: so I guess it comes down to whether you think the value of the important flag is easy to know - or whether it'd require evaluation, if you use a bunch of memory locations, how can it know statically

[2025-03-06 00:07] contificate: the transformation you're doing has to be semantics preserving, it must preserve the semantic change to EFLAGS, this is something you can prove correct

[2025-03-06 00:08] thismightwork: [replying to contificate: "when you lift, you make sure to model the effect e..."]
Sorry, I don't lift anything. I just directly do it all.

[2025-03-06 00:09] contificate: you should lift

[2025-03-06 00:10] thismightwork: [replying to contificate: "you should lift"]
Sorry, what are the pros an cons of lifting to IR? I am new.

[2025-03-06 00:10] contificate: more flexibility, but of course you really need to retain the semantics x86->x86 w.r.t the eflags stuff

[2025-03-06 00:11] contificate: can imagine a few scenarios here

[2025-03-06 00:11] contificate: assuming all straightline replacements

[2025-03-06 00:12] contificate: you obfuscate a computation whose value then immediately is used by some kind of test instruction - so the important flags are gleaned explicitly from the result, so a semantics preserving translation only need to ensure the result is the same

[2025-03-06 00:13] contificate: then some other scenario where it's less clear but can be made clear in the same way, e.g. some invariant that applies to the original code and the obfuscated code that allows you to recover the flags from the result, that you then also include, to turn it into the first scenario

[2025-03-06 00:14] contificate: then, with a full semantics of the instructions you care about

[2025-03-06 00:14] contificate: you can probably prove correct the translations w.r.t to flags registers

[2025-03-06 00:15] contificate: e.g. for the same inputs, you will get the same flags at the end

[2025-03-06 00:16] thismightwork: [replying to contificate: "e.g. for the same inputs, you will get the same fl..."]
I'll most likely write a new MBA system that relies on bitwise math then. This way things dont break.

[2025-03-06 00:16] contificate: just need a model of the instructions you transform to

[2025-03-06 00:18] thismightwork: [replying to contificate: "just need a model of the instructions you transfor..."]
like the exact instructions minus the operands?

[2025-03-06 00:19] contificate: effectively the instruction as it is decomposed into smaller operations

[2025-03-06 00:20] contificate: such that it explicitly models changes to flags

[2025-03-06 00:20] contificate: wonder if Z3 could just do this

[2025-03-06 00:21] contificate: e.g. find a counterexample

[2025-03-06 00:21] contificate: you would convert the before and after code an SMT-LIB routine that emulates its semantics

[2025-03-06 00:21] contificate: then assert that there does not exist an input such that the output of the flags (that you care about) differs

[2025-03-06 00:22] contificate: I expect there are some proof theoretic things it can do

[2025-03-06 00:22] contificate: but maybe it'll literally try to exhaust the input space

[2025-03-06 00:22] contificate: which will be a painful hassle for any arithmetic over 32 bits

[2025-03-06 00:22] contificate: this is an interesting little task

[2025-03-06 00:23] contificate: if you obfuscate at a higher level, you need not care about this

[2025-03-06 00:23] contificate: but indeed many handwritten assembly snippets rely on a careful understanding of what flags are going to be set - but I expect the chains of chose are rather short

[2025-03-06 00:23] contificate: such that my second suggestion holds

[2025-03-06 00:23] thismightwork: It's fascinating how this is the most difficult part of obfuscation.

[2025-03-06 00:24] contificate: it's not, but it's neat

[2025-03-06 00:24] contificate: I really think that like

[2025-03-06 00:24] contificate: most arithmetic instructions probably effect much of EFLAGS, right

[2025-03-06 00:24] contificate: to the point where a straightline fragment kills them

[2025-03-06 00:24] contificate: except for the last instruction

[2025-03-06 00:25] contificate: so if you see that a computation's result is live after and has no intermediary dependencies with flag dependencies

[2025-03-06 00:25] contificate: you can simply include, to the end of the obfuscation, an expression that recovers the flags

[2025-03-06 00:25] contificate: e.g. emulates the check done by the last part of the original expression's internal implementation

[2025-03-06 00:26] contificate: maybe it's just a non-issue since flags have short dependency chains

[2025-03-06 00:26] thismightwork: [replying to contificate: "you can simply include, to the end of the obfuscat..."]
That means you can make boundaries for the obfuscation.

[2025-03-06 00:27] contificate: you do that anyway

[2025-03-06 00:27] contificate: most arithmetic obfuscation is done over straightline fragments

[2025-03-06 00:27] contificate: or is done several times

[2025-03-06 00:28] contificate: the nice thing about obfuscating at the level of LLVM is you can pattern match across blocks easily due to the simple def-use chains implied by SSA

[2025-03-06 00:28] contificate: but then the failure comes

[2025-03-06 00:28] contificate: when you write the obfuscated version somewhere

[2025-03-06 00:28] contificate: and it's not shuffled around at all

[2025-03-06 00:28] contificate: or the instruction selection schedules it using the data dependency

[2025-03-06 00:29] contificate: god darn compilers lowering the effectiveness of some obfuscations

[2025-03-06 00:29] thismightwork: [replying to contificate: "most arithmetic obfuscation is done over straightl..."]
I just looked that term up.. I failed that stuff in grade school and college. Those variables and the polynomials and all that was insane.

[2025-03-06 00:30] contificate: what, "arithmetic" that's just the name of this, the A in MBA

[2025-03-06 00:32] thismightwork: [replying to contificate: "what, "arithmetic" that's just the name of this, t..."]
The purpose of asking was to figure out how to improve my obfuscator. At this level, no big company or legit business will buy my stuff.

Many years ago I would sell and make obfuscators/packers. They were 99% "hackers". I guess it's them again or I throw in the towel.

[2025-03-06 00:33] contificate: alas, obfuscators have to play by the same rules as compilers

[2025-03-06 00:33] contificate: making it effectively a compilation endeavour

[2025-03-06 00:33] contificate: with different acceptance criteria

[2025-03-06 01:30] juan diego: https://github.com/DenuvoSoftwareSolutions/GAMBA
[Embed: GitHub - DenuvoSoftwareSolutions/GAMBA: Simplification of General M...]
Simplification of General Mixed Boolean-Arithmetic Expressions: GAMBA - DenuvoSoftwareSolutions/GAMBA

[2025-03-06 01:30] juan diego: recommended for testing ur MBA

[2025-03-06 03:00] James: welllllll...... I'm not sure if you want to use this as a metric. if GAMBA cant reduce a certain MBA, that does not necessarily mean it is "strong". basing your mba's "strength" off of any of the public mba reduction tools is going to give you a LOT of false confidence

[2025-03-06 04:11] juan diego: of course but it will shred through many

[2025-03-06 09:35] contificate: basing your obfuscation's strength solely on MBA is a meme anyway

[2025-03-06 09:35] contificate: but nice to have coverage

[2025-03-06 14:37] juan diego: out of the literature I've seen I think this is one of the strongest that's all

[2025-03-06 17:10] James: [replying to contificate: "basing your obfuscation's strength solely on MBA i..."]
what makes you say this? you've played around with MBA?

[2025-03-06 17:11] James: i agree with you though, mba is very dumb.

[2025-03-06 17:18] contificate: I know it's good, but I mean, I'd like to layer it with more things in general

[2025-03-06 17:18] contificate: not all eggs in one basket

[2025-03-06 18:27] James: Do u have any personal intuition or evidence as to why mba is weak?

[2025-03-06 18:28] James: [replying to contificate: "not all eggs in one basket"]
Agree with this too

[2025-03-06 18:39] contificate: I never said it's weak

[2025-03-06 18:50] contificate: I'm at desk, so let me clarify

[2025-03-06 18:51] contificate: it's weak to assume that applying MBA only is sufficient

[2025-03-06 18:51] contificate: imagine a program whose structure is roughly identifiable even though arithmetic is all fucked

[2025-03-06 18:51] contificate: like it's an obvious flow of calls to known functions

[2025-03-06 18:51] contificate: you want to layer other shit on top

[2025-03-06 18:51] contificate: always

[2025-03-06 21:17] James: [replying to contificate: "I never said it's weak"]
Oh it is weak though.

[2025-03-06 21:21] contificate: alright, I don't care for your conversation any more

[2025-03-06 21:22] contificate: don't have time to be led down these corridors

[2025-03-06 21:22] contificate: if you want to have a coherent conversation, feel free to engage in good faith

[2025-03-07 10:43] dullard: belabs_colin when ? <:Kappa:1082189237178351666>