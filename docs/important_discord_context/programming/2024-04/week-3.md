# April 2024 - Week 3
# Channel: #programming
# Messages: 52

[2024-04-15 22:41] asz: <@151110446441562112>

[2024-04-16 16:06] North: Anyone happen to know why -Werror would have issues working with Clang? All my other flags are working just fine, even when I do -Werror=conversion it works.

[2024-04-16 22:30] mrexodia: [replying to North: "Anyone happen to know why -Werror would have issue..."]
You actually need to enable the warning you want to error on

[2024-04-17 16:17] Timmy: always thought -Werror was just like /WX

[2024-04-17 16:26] Matti: it is

[2024-04-17 16:26] Matti: but WX also only errors for things that are enabled warnings

[2024-04-17 16:27] Matti: it's just usually used in combination with /W4 (in the WDK anyway)

[2024-04-17 16:28] Matti: https://stackoverflow.com/questions/11714827/how-can-i-turn-on-literally-all-of-gccs-warnings
[Embed: How can I turn on (literally) ALL of GCC's warnings?]
I would like to enable—literally—all of the warnings that GCC has. (You'd think it would be easy...)

You'd think -Wall might do the trick, but nope! You still need -Wextra.

You'd think -Wextra mi...

[2024-04-17 16:29] Matti: you'd think `-Wall -Wextra -Weverything` would be enough

[2024-04-17 16:29] Matti: but as you can see some warnings are just not enabled unless you enable them explicitly

[2024-04-17 16:29] Matti: I'm sure MSVC has similar examples

[2024-04-17 16:31] Matti: *my bad, `-Weverything` is a clang only flag and does in fact enable everything

[2024-04-17 16:31] Matti: in GCC however `-Wall -Wextra` does not and there is no `-Weverything`

[2024-04-17 16:33] Matti: `-Weverything` is completely impractical to use though, so a probably more usable fix would be to just `-Wconversion` and `-Werror`

[2024-04-17 16:33] Matti: also making sure no one is putting `-Wno-conversion` anywhere

[2024-04-17 18:52] Windy Bug: anyone knows a workaround to debug print a float in a driver ?

[2024-04-17 18:58] Brit: write the float to a file :^) the real question is what are you doing that requires you to do floating point operations in kernel

[2024-04-17 18:59] Windy Bug: Calculating entropy

[2024-04-17 19:00] diversenok: So fixed-point would suffice

[2024-04-17 19:00] diversenok: Multiply by 100 or something

[2024-04-17 23:48] North: <@162611465130475520> <@148095953742725120> Sorry I should have mentioned I have -Wall and -Wextra on and am getting the proper warnings but they just aren't counting as errors when I add -Werror.

[2024-04-18 00:14] North: i think this might just be an issue with the settings im using to build clang 🤦‍♂️

[2024-04-18 00:30] North: or maybe something weird with clang-cl being used outside msvc

[2024-04-18 01:48] Matti: I'm lazy, can you give some sample code that should trigger `-Wconversion`? I can check here

[2024-04-18 01:48] Matti: [replying to North: "or maybe something weird with clang-cl being used ..."]
this shouldn't make a difference, unless the command line or environment is also different due to this

[2024-04-18 01:51] Matti: this trivial example works as expected at least
[Attachments: image.png]

[2024-04-18 01:52] Matti: no error or warning with `-Wno-unknown-argument` obviously

[2024-04-18 01:55] Matti: and finally `-Werror=unknown-argument` also does what you'd expect

[2024-04-18 01:55] Matti: so, it seems to be either specific to `-Wconversion`, or it's this

[2024-04-18 01:55] Matti: [replying to North: "i think this might just be an issue with the setti..."]
^

[2024-04-19 20:56] TheXSVV: why i can't use GetProcAddress for CipInitialize, but can for CiInitialize?

[2024-04-19 21:04] JustMagic: [replying to TheXSVV: "why i can't use GetProcAddress for CipInitialize, ..."]
because CipInitialize is not exported

[2024-04-19 21:05] 5pider: looks like CipInitialize is a private function ?

[2024-04-19 22:00] Matti: [replying to 5pider: "looks like CipInitialize is a private function ?"]
a 'private function' (or method) is an OOP term, doubt that's what you mean...

then you also have functions that are *private exports* (meaning they **are** exported but you can only see their ordinal, not their name)...

...and then you have functions that simply aren't exported, which is a requirement for GetProcAddress to work

[2024-04-19 22:01] Matti: [replying to JustMagic: "because CipInitialize is not exported"]
so, it's just this

[2024-04-19 22:01] Matti: the 'p' in the name is also a hint here

[2024-04-19 22:02] Matti: one is a public API and so usually exported, the other is a private (p) implementation or part of one

[2024-04-21 12:26] Deleted User: I am loading a windows driver with devcon but I am only able to load it if I have my debugger connected to VM for kernel dbg

[2024-04-21 12:26] Deleted User: why

[2024-04-21 13:54] Nats: the kernel debugger disables the driver signing enforecement

[2024-04-21 13:54] Nats: iirc

[2024-04-21 14:00] Brit: that's most likely it

[2024-04-21 14:21] asz: it do???

[2024-04-21 14:23] asz: i havenr debugged in years and maybe there was no reason to do both testsigning and kernel debug

[2024-04-21 14:23] Brit: afaik when the debugger is attached, dse is off

[2024-04-21 14:24] asz: hmm allright

[2024-04-21 19:06] froj: Huh TIL

[2024-04-21 19:12] luci4: Good to know!

[2024-04-21 19:42] Windy Bug: [replying to Brit: "afaik when the debugger is attached, dse is off"]
Wasn’t it PatchGuard ?

[2024-04-21 19:43] Brit: that too

[2024-04-21 19:43] Brit: but dse too

[2024-04-21 19:44] Brit: there's a flag you can set to re enable the behavior