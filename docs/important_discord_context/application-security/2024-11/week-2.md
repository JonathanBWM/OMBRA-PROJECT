# November 2024 - Week 2
# Channel: #application-security
# Messages: 21

[2024-11-08 13:27] projizdivlak: was in any windows DEP enabled by default on all user programs?

[2024-11-08 13:58] x86matthew: it's always enabled for 64bit programs

[2024-11-08 13:58] x86matthew: it is possible to globally enforce it (regardless of compatibility flags) for 32-bit programs but not a good idea

[2024-11-08 13:58] x86matthew: but this was never the default

[2024-11-08 14:37] projizdivlak: hmm, thanks

[2024-11-09 10:22] 25pwn: [replying to x86matthew: "it is possible to globally enforce it (regardless ..."]
why is it not a good idea?

[2024-11-09 10:32] Deleted User: Probably break shit and not good for security

[2024-11-09 11:16] x86matthew: [replying to 25pwn: "why is it not a good idea?"]
you'll likely get crashes with older programs (anything up to around 2005), lots of compilers that were popular at the time (eg msvc++ 6.0) didn't support it which meant they could do silly things

[2024-11-09 11:16] x86matthew: and microsoft's own ATL library (which was quite common in that era) wasn't compliant

[2024-11-09 11:17] x86matthew: and other stuff like copy-protection

[2024-11-09 11:26] 25pwn: [replying to x86matthew: "you'll likely get crashes with older programs (any..."]
if i wanted to harden a modern windows 11 system, do you think it's a "viable" option?

[2024-11-09 11:28] x86matthew: yeah it'll be fine as long as you don't plan on running ancient software

[2024-11-09 11:28] 5pider: give me the source code to elastic edr kernel driver

[2024-11-09 11:28] 5pider: <:sus:1150512793636839538>

[2024-11-09 11:29] x86matthew: sent!

[2024-11-09 11:29] 5pider: <:smiley:1184418532394545163>

[2024-11-10 00:47] projizdivlak: [replying to x86matthew: "you'll likely get crashes with older programs (any..."]
yeah i was looking into early VAC v1 implementation, they unpacked and ran the payload in data section lol (then they retired this shit in 2008 (since 2002) and i wondered why, thats why i asked about dep on default programs), compiled with msvc 6.0

[2024-11-10 12:04] x86matthew: yeah HL1/CS1.5 did use msvc++ 6.0, i remember it well

[2024-11-10 12:04] x86matthew: executing code directly from "non-executable" regions was fairly common practice back then, lots of packers were guilty of this too

[2024-11-10 12:04] x86matthew: not a big deal at the time though as DEP was opt-in

[2024-11-10 12:06] x86matthew: looks weird to see from a modern perspective though lol