# February 2025 - Week 4
# Channel: #application-security
# Messages: 106

[2025-02-19 18:12] Matti: [replying to contificate: "frame numbers and frame pointers 😼"]
hmm... depending on the context I would probably make that trade

[2025-02-19 18:12] Matti: oh no wait, it was trading FNs for FPs

[2025-02-19 18:13] Matti: no I would not make that trade

[2025-02-19 18:15] Matti: frame pointers are a holdover from times (or operating systems) from before the invention of embedded Microsoft(R) unwind metadata in PE files

[2025-02-19 18:16] contificate: I never use `rbp`

[2025-02-19 18:16] Matti: nah me neither, there's no need even on nix

[2025-02-19 18:17] Matti: but `-fomit-frame-pointers` doesn't help with stacktraces in my experience

[2025-02-19 22:47] James: [replying to Matti: "frame pointers are a holdover from times (or opera..."]
incorrect, they still serve a VERY important purpose today. they exist specified in the metadata for a reason.

[2025-02-19 22:49] Matti: are you trying to say what I said (namely that PE unwind data now contains this information) while starting a sentence with "Incorrect," ?

[2025-02-19 22:51] James: your statement in NO WAY suggests that the unwind metadata specifies the frame pointer register.

[2025-02-19 22:51] James: instead, any reader that isn't yourself would likely assume you meant that the metadata was a replacement for unwinding via frame pointer iteration

[2025-02-19 22:52] James: without a frame pointer, and by extension dynamic stack allocation(without some serious trickery/custom metadata and handler) msvc wouldn't be compliant with all C versions.

[2025-02-19 22:53] James: gtg nap, but i'll read whatever you say when i'm back.

[2025-02-19 22:54] Matti: well, we'll leave the exact set of people who did not understand this for another time I think
but
> any reader that isn't yourself [Matti]
and
> any reader whose name is James
do have one element in common

[2025-02-19 22:54] Matti: more than this I don't know

[2025-02-19 22:55] Matti: maybe I overestimate my audience by assuming people know about MS x64 exception handling

[2025-02-19 23:00] Matti: [replying to James: "without a frame pointer, and by extension dynamic ..."]
> msvc wouldn't be compliant with all C versions.
I doubt there is a single C version that MSVC is compliant with
> gtg nap, but i'll read whatever you say when i'm back.
ok have a good night mate

[2025-02-20 00:04] contificate: > without a frame pointer, and by extension dynamic stack allocation(without some serious trickery/custom metadata and handler) msvc wouldn't be compliant with all C versions.
compilers can dedicate another GPR for this purpose for alloca/vlas

[2025-02-20 00:04] contificate: doesn't need to be a globally applied thing

[2025-02-20 00:20] Matti: also true, there's no reason the frame pointer needs to be used for this

[2025-02-20 00:21] Matti: I was just too upset by the notion that MSVC would be compliant with anything as a given to look further

[2025-02-20 00:25] Matti: it's not like C++ where standards support just lags behind clang and gcc by a few years or so

[2025-02-20 00:25] Matti: C compliance is explicitly not a goal of MSVC, as per the MSVC devs

[2025-02-20 00:26] Matti: which makes it even funnier that it does have `/std:c11` and so on

[2025-02-20 00:29] contificate: fuck MSVC tbh

[2025-02-20 00:29] James: WOAHHHH

[2025-02-20 00:29] James: [replying to contificate: "> without a frame pointer, and by extension dynami..."]
100%

[2025-02-20 00:29] James: it can be any nonvolatile gpr besides rsp

[2025-02-20 00:29] James: [replying to Matti: "C compliance is explicitly not a goal of MSVC, as ..."]
NC? they've said this?

[2025-02-20 00:29] contificate: it could be volatile as well

[2025-02-20 00:30] James: [replying to contificate: "it could be volatile as well"]
it cannot

[2025-02-20 00:30] contificate: maybe you're right, thought I remembered some trick

[2025-02-20 00:30] James: func A calls B

[2025-02-20 00:30] James: A has a frame pointer RAX

[2025-02-20 00:30] contificate: don't condescend me

[2025-02-20 00:30] James: it adjusts RSP, calls B

[2025-02-20 00:30] contificate: I know more about this than you, the point is you can have a dynamic allocation that's set once

[2025-02-20 00:31] Matti: [replying to James: "NC? they've said this?"]
that is what I said, yes

[2025-02-20 00:31] contificate: and refer relative to it afterwards

[2025-02-20 00:31] contificate: and store a slot there at the top

[2025-02-20 00:31] contificate: so it's preserved across the call

[2025-02-20 00:31] contificate: but not in a non-volatile

[2025-02-20 00:31] James: B throws an exception, which will NOT restore A's RAX

[2025-02-20 00:31] contificate: C doesn't have exceptions

[2025-02-20 00:31] James: oh my, of course you're right.

[2025-02-20 00:31] 5pider: watch me force one

[2025-02-20 00:31] James: I see the error of my ways.

[2025-02-20 00:32] contificate: I'm just stating the theoretical here, what compilers actually do is far more basic

[2025-02-20 00:32] James: and by golly thank christ it doesnt! because we both know LLVM can't compile anything with exceptions correctly on windows at all

[2025-02-20 00:32] contificate: y'know

[2025-02-20 00:32] contificate: I find it acceptable to use setjmp and longjmp in C

[2025-02-20 00:32] contificate: for casual usage

[2025-02-20 00:32] James: as do I

[2025-02-20 00:32] contificate: good, good

[2025-02-20 00:32] contificate: 🍻

[2025-02-20 00:33] Matti: [replying to James: "and by golly thank christ it doesnt! because we bo..."]
hmm https://discord.com/channels/835610998102425650/835664858526646313/1212218387439296532

[2025-02-20 00:33] Matti: that's SEH, to clarify, which must be what you meant

[2025-02-20 00:34] James: i assure you, it does not pass all of microsofts exception tests

[2025-02-20 00:34] contificate: might add exceptions to C at some point

[2025-02-20 00:34] James: i was playing with it just a week ago

[2025-02-20 00:34] James: [replying to contificate: "might add exceptions to C at some point"]
i can see that i stepped on a landmine with you

[2025-02-20 00:34] Matti: ah, oh well then

[2025-02-20 00:35] Matti: I'll take your word for it

[2025-02-20 00:35] James: language's  are not by any means my expertise and I don't want to fight a language lawyer

[2025-02-20 00:35] contificate: I am not a language lawyer, either

[2025-02-20 00:35] James: [replying to Matti: "I'll take your word for it"]
if it makes u feel better, it's only when doing stupid things that no real program ever does, like VLAs and setjmp longjmp

[2025-02-20 00:36] James: so frankly, even though I get to say it's broken, for all intents and purposes it's not

[2025-02-20 00:36] James: and the codegen of llvm

[2025-02-20 00:36] James: so vastly superior

[2025-02-20 00:36] James: so in the end, well there's a clear winner

[2025-02-20 00:36] Matti: I wouldn't mind a godbolt of some broken setjmp/longjmp - less interested in the VLA case but sure

[2025-02-20 00:37] James: https://github.com/microsoft/windows_seh_tests/tree/main
[Embed: GitHub - microsoft/windows_seh_tests: Windows ABI Structured Except...]
Windows ABI Structured Exception Handling Tests. Contribute to microsoft/windows_seh_tests development by creating an account on GitHub.

[2025-02-20 00:38] James: and what llvm is emitting IS well formed, in that its deterministic... but it just doesn't match the msvc output

[2025-02-20 00:38] Matti: ugh, which one in particular at least so I can godbolt it myself

[2025-02-20 00:38] Matti: I believe you, don't worry

[2025-02-20 00:38] James: ofc it's ALSO possible I suppose that the msvc output is incorrect, and llvm did it correct. which would certainly embarass me.

[2025-02-20 00:38] James: somewhat difficult to verify what the correct value printed should be sometimes

[2025-02-20 00:39] Matti: that doesn't sound like a great test, honestly

[2025-02-20 00:40] Matti: isn't the one thing it should do, tell you whether it succeeded or failed

[2025-02-20 00:40] James: yeah hold on, im looking

[2025-02-20 00:43] James: here we go

[2025-02-20 00:43] James: https://github.com/microsoft/compiler-tests
[Embed: GitHub - microsoft/compiler-tests: This repo contains Microsoft com...]
This repo contains Microsoft compiler-tests to validate Windows platform particulars. - microsoft/compiler-tests

[2025-02-20 00:46] Matti: actually I might be more interested in the first repository now

[2025-02-20 00:46] Matti: <https://github.com/microsoft/windows_seh_tests/blob/main/src/xcpt4/xcpt4pg.c> looks like a comment left by accident

[2025-02-20 00:46] Matti: how unfortunate

[2025-02-20 00:46] James: yeah i saw that as well

[2025-02-20 00:47] James: im not really sure what it was talking about though

[2025-02-20 00:48] Matti: PG uses some unusual exception handlers (and alsoo unusual exceptions) as part of its operation

[2025-02-20 00:48] James: oh thats RIGHT they do have that barrel like call/except thing right?

[2025-02-20 00:48] James: func5->func4->func3->...

[2025-02-20 00:49] Matti: not sure if that's done via SEH, but probably

[2025-02-20 00:49] Matti: 
[Attachments: Tetrane_PatchGuard_Analysis_RS4_v1.01.pdf]

[2025-02-20 00:50] Matti: this contains most of the details

[2025-02-20 00:51] Matti: a lot of this was already public knowledge for a long time (2006?) due to being featured in skywing's PG articles that focused on SEH in particular

[2025-02-20 00:52] Matti: this PDF is the most up to date and complete PG reference though I would say

[2025-02-20 00:54] Matti: http://uninformed.org/index.cgi?v=3&a=3&t=pdf Uninformed (Skywing) article v1
http://www.uninformed.org/?v=6&a=1&t=pdf v2
http://uninformed.org/index.cgi?v=8&a=5&t=pdf v3

if you're more interested in the SEH-specific details

[2025-02-20 00:56] Matti: ah, one method not covered in those that is in the tetrane PDF is III.A.1

[2025-02-20 00:57] Matti: in which a non-canonical pointer is used

[2025-02-20 00:58] Matti: [replying to James: "oh thats RIGHT they do have that barrel like call/..."]
not quite sure but III.A.2 might be what you mean here

[2025-02-20 02:32] Torph: [replying to contificate: "fuck MSVC tbh"]
yea i had to go write special handling code in my build system for them because they don't support inline assembler for the `.incbin` macro

[2025-02-20 03:45] daax: [replying to Matti: "in which a non-canonical pointer is used"]
pg loves their non-canonical ptrs. surprised the original uninformed didn’t describe them.

[2025-02-20 03:46] Matti: yeah, but I'm not actually sure if those were always in PG, especially PG 1.0 (XP x64) and 2.0 (vista)

[2025-02-20 03:48] Matti: PG 1.0 was very rudimentary, there are some vestiges of it in the WRK in precompiled blob form heh

[2025-02-20 03:49] Matti: as well as bits of source code that are doing mysterious things like "testing division errata" (read: div by 0 to initialize PG)

[2025-02-20 03:53] Matti: ok yeah the canonical ptr filter was there even in PG 1.0
[Attachments: image.png]

[2025-02-20 03:53] Matti: (foltz.obj)
[Attachments: image.png]