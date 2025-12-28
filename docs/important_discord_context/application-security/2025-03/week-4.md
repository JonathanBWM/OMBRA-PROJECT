# March 2025 - Week 4
# Channel: #application-security
# Messages: 38

[2025-03-19 03:50] Brallan: [replying to James: "Oh it is weak though."]
are you talking specifically about linear mba?

[2025-03-19 04:15] James: [replying to Brallan: "are you talking specifically about linear mba?"]
Any MBA.

[2025-03-19 04:44] Brallan: [replying to James: "Any MBA."]
do you know any papers that talks exactly about that? Cause as you said, gamba/goomba/mbablast aren't always of any use

[2025-03-19 04:50] James: [replying to Brallan: "do you know any papers that talks exactly about th..."]
Well, there's a fine line between academic work(what you can prove or say with some certainty) and what you can do in practice. It's somewhat hard to publish papers when your evaluations come with lots of "well it requires X to be true". And while X is true 99% of the time in the real world, you have to convince a reviewer of that. 

So no I don't have any papers for you sadly.  

Keep in mind by the way, that there is still no published general method for generating polynomial MBAs that doesn't involve creating constant MBAs for the coefficients of another MBA. Or using MBAs to obscure the obvious(?) structure of two composed inverse polynomials.

[2025-03-19 04:53] James: If you really want to pursue this... I would take a GOOD long look at the very first MBA paper. And ignore all existing tools relating to synthesis. SiMBA is fine.

And maybe try to avoid reliance on an oracle... Because someone will always have something that your oracle doesn't.

[2025-03-19 04:53] Brallan: [replying to James: "Well, there's a fine line between academic work(wh..."]
appreciate the detailed answer. Polynomials MBAs are easy to simplify too right?

[2025-03-19 04:53] Brallan: [replying to James: "If you really want to pursue this... I would take ..."]
much appreciate this

[2025-03-19 04:54] Brallan: [replying to Brallan: "appreciate the detailed answer. Polynomials MBAs a..."]
well when i say easy  i mean like other mbas

[2025-03-19 04:59] Brallan: [replying to Brallan: "appreciate the detailed answer. Polynomials MBAs a..."]
> And maybe try to avoid reliance on an oracle... Because someone will always have something that your oracle doesn't. 
😆 You're right..

[2025-03-19 05:00] James: well you can imagine that if i have the most basic rule `1*(a^b)+2*(a&b)`

[2025-03-19 05:00] James: And if you also believe that you can create an MBA to generate an arbitrary constant(which you can)

[2025-03-19 05:00] James: then you can imagine I can generate one to be that 1 and that 2.

[2025-03-19 05:02] James: and now you have an MBA that isn't a sum of a*e terms where a is a constant and e is a bitwise expression.

[2025-03-19 05:02] James: its "polynomial"

[2025-03-19 05:02] James: by simply iteratively applying linear mba identities

[2025-03-19 05:04] Brallan: Makes sense

[2025-03-19 05:04] Brallan: Thanks for the oracling 😆

[2025-03-19 05:05] James: and so these mbas, might seem easier to reduce, than say a different polynomial MBA

[2025-03-19 05:05] James: i forget what it is but there's one in a Zhou paper about it.

[2025-03-19 21:58] mrexodia: if you didn't read this series, yet it's quite nice: https://plzin.github.io/posts/mba <@920160971220222022>

[2025-03-19 23:54] emma: https://github.com/HexRaysSA/goomba
[Embed: GitHub - HexRaysSA/goomba: gooMBA is a Hex-Rays Decompiler plugin t...]
gooMBA is a Hex-Rays Decompiler plugin to simplify Mixed Boolean-Arithmetic (MBA) expressions - HexRaysSA/goomba

[2025-03-19 23:54] emma: Also

[2025-03-20 00:01] juan diego: https://www.slideshare.net/codeblue_jp/igor-skochinsky-enpub
[Embed: Secret of Intel Management Engine  by Igor Skochinsky]
Secret of Intel Management Engine  by Igor Skochinsky - Download as a PDF or view online for free

[2025-03-20 00:02] juan diego: [replying to emma: "https://github.com/HexRaysSA/goomba"]
thought the name igor skochinsky was familiar

[2025-03-20 00:19] James: [replying to emma: "https://github.com/HexRaysSA/goomba"]
This plugin is a bit chopped ngl. In part due to the ida's IR.

[2025-03-20 00:20] emma: [replying to James: "This plugin is a bit chopped ngl. In part due to t..."]
what does chopped mean

[2025-03-20 00:20] James: Bad.

[2025-03-20 00:20] emma: but it has a funni name!

[2025-03-20 00:22] James: well, to some people I guess that buys it something...

[2025-03-21 17:51] juan diego: anyone with crypto experience (RSA) wanna take a look at an opaque predicate scheme I'm working on? i think i have a scheme where its cryptographically hard to prove an opaque predicate has no solution/find the one solution but im a noob so i'd like to hear from someone w more experience. i'm mostly working on this for fun and learning

[2025-03-21 17:52] juan diego: (i have generator/example in python)

[2025-03-21 20:02] fvrmatteo: [replying to juan diego: "anyone with crypto experience (RSA) wanna take a l..."]
Just linking this in case you haven't seen it before: https://www.babush.me/dumbo-llvm-based-dumb-obfuscator.html
[Embed: Dumbo: LLVM-based Dumb Obfuscator]
So you want to learn how to build the most awesome obfuscator known to human being? I've got you covered! In this tutorial you will learn of sorts of obfuscation things. Like, obfuscators, what do the

[2025-03-21 20:12] juan diego: thanks lol good to verify some of my own research

[2025-03-21 20:14] juan diego: my scheme is similar but you can also use it to make always false opaque predicates

[2025-03-21 20:14] juan diego: with hopefully no way to tell it from one with a single solution

[2025-03-21 21:37] nuclearfirefly: [replying to juan diego: "with hopefully no way to tell it from one with a s..."]
you should send a sample. i'd be curious to run some solvers on it, would be cool to see how well it holds up

[2025-03-21 21:39] nuclearfirefly: or you should and post the results! constructing these is entertaining

[2025-03-21 21:45] juan diego: dm