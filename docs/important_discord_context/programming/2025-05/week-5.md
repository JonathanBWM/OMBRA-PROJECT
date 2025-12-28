# May 2025 - Week 5
# Channel: #programming
# Messages: 59

[2025-05-26 06:16] safareto: asked this in <#835667591393574919> but thought id ask here too, does anyone have some resources for getting into anti-cheat dev? im aware of unknowncheats but im more looking for "rudimentary" resources on how to develop/get into developing anti cheats

[2025-05-26 10:38] roddux: [replying to truckdad: "or just don't want to"]
yes this is also true lol

[2025-05-26 10:38] roddux: i use tailscale 😁

[2025-05-26 12:09] Timmy: [replying to roddux: "tailscale is wireguard for people who cannot confi..."]
that's netbird or zerotier or someth you're referring to, tailscale is a bit more fancy with the ACL's and things like that. it's not *purely* wireguard

[2025-05-26 13:03] roddux: Headscale is also like a free implementation of Tailscale

[2025-05-26 17:48] elias: What km library in the WDK contains the Interlocked API for ARM64? Im linking against ntoskrnl.lib and getting unresolved external symbol

[2025-05-26 18:02] the horse: isn't interlocked api just intrinsics?

[2025-05-26 18:03] Bony: questions for the winternals pros out there. does mmgetphysicalranges include hyper-v / secure kernel pages?

[2025-05-26 18:03] the horse: not sure if the compiler implements them for arm

[2025-05-26 18:20] elias: [replying to the horse: "isn't interlocked api just intrinsics?"]
https://learn.microsoft.com/en-us/cpp/intrinsics/intrinsics-available-on-all-architectures?view=msvc-170
This page says interlocked is available for AMD64 and ARM64

[2025-05-26 18:22] the horse: are you compiling with MSVC?

[2025-05-26 18:22] elias: yep

[2025-05-26 19:04] JustMagic: [replying to elias: "What km library in the WDK contains the Interlocke..."]
Show us the output window

[2025-05-26 19:06] elias: [replying to JustMagic: "Show us the output window"]

[Attachments: image.png]

[2025-05-26 19:06] elias: you can ignore the cpuid one

[2025-05-26 19:11] JustMagic: [replying to elias: ""]
Weird. It should be supported.

[2025-05-26 19:12] JustMagic: Do other interlocked intrin sizes work?

[2025-05-26 19:13] JustMagic: I'd maybe update the toolchain + SDK as well to be sure

[2025-05-26 19:49] elias: [replying to JustMagic: "Do other interlocked intrin sizes work?"]
nope, none of the Interlocked ones

[2025-05-26 19:51] truckdad: are you sure you're including `intrin.h`?

[2025-05-26 20:24] elias: figured it out

[2025-05-26 20:24] elias: the /NODEFAULTLIB linker argument causes this error

[2025-05-26 20:24] elias: only in ARM64

[2025-05-26 21:55] Max: Hey Guys im working on a project for a backend for a website in c#.
Anyways im having a hard time finding the right Architecture for my Api.

I need endpoints for open/user/admin with jwt tokenAuth.
Im planing to do dto´s for my DB.
And the Backend/DB should run in Docker via compose.

Do you know any good guides/vids for a good architecture of rest?

[2025-05-27 00:20] UJ: [replying to Max: "Hey Guys im working on a project for a backend for..."]
do you need long lived connections with bi-direc streaming or standard rcp type requests?

[2025-05-27 00:28] Max: just rcp

[2025-05-27 00:37] the horse: what is the question exactly?
ASP.NET should provide a concrete foundation

[2025-05-27 03:24] daax: [replying to Bony: "questions for the winternals pros out there. does ..."]
no, it shouldn’t, that would violate the isolation mechanisms within VBS

[2025-05-27 20:25] Matti: [replying to elias: "the /NODEFAULTLIB linker argument causes this erro..."]
I got curious as to why this would be the case

[2025-05-27 20:25] Matti: (you are right, I managed to repro the same issue using the latest MSVC)

[2025-05-27 20:26] Matti: it turns out the reason is the the atomics are provided in a separate .obj file that's part of libcmt.lib... one native for arm64, and one for arm64ec

[2025-05-27 20:26] Matti: e.g.
[Attachments: atomics_arm64.obj, atomics_arm64ec.obj]

[2025-05-27 20:27] Matti: there are lots of .obj files in there of course - most if not all are duplicated between the two

[2025-05-27 20:28] Matti: `C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.44.35207\lib\arm64\libcmt.lib` is the path on my system; you can simply open static libraries with 7zip and they'll include a .txt listing with the contents
(or open in IDA)

[2025-05-27 20:31] Matti: also for comparison, x64 atomics (at least the interlockedexchange ones) are true compiler intrinsics, not library functions

[2025-05-27 20:32] Matti: idem dito for clang* arm64
[Attachments: image.png]

[2025-05-27 20:34] Matti: I think.... my ARM is a little rusty, not sure why an interlocked exchange should be looping

[2025-05-27 20:35] Matti: I'm not comparing the output against anything

[2025-05-27 20:36] Matti: but yeah the MSVC equivalent is clearly just calling a library function
[Attachments: image.png]

[2025-05-27 20:38] Matti: I guess there is a minimum level of ARM support required that MSVC insists on checking
[Attachments: image.png]

[2025-05-27 20:43] Matti: [replying to Matti: "idem dito for clang* arm64"]
lastly ARM 32 bit generates roughly this ^ for both compilers; no libraries needed

[2025-05-27 20:43] Matti: conclusion: ARM64EC sucks

[2025-05-27 20:43] Matti: as does MSVC... obviously

[2025-05-27 21:40] elias: good to know <:peepoDetective:570300270089732096>

[2025-05-27 21:41] elias: I was thinking about reporting this as a bug to MS

[2025-05-27 21:42] Matti: I found the fix/workaround <@234331837651091456>

[2025-05-27 21:43] Matti: pass `/d2overrideInterlockedIntrinsArm64-`

[2025-05-27 21:43] Matti: new result:
[Attachments: image.png]

[2025-05-27 21:44] Matti: it's part of a boatload of new undocumented compiler flags
[Attachments: image.png]

[2025-05-27 21:45] Matti: these are in c2.dll, hence the /d2 prefix

[2025-05-27 21:45] elias: now its working indeed, thank you <a:bongoCat:568437486607532032>

[2025-05-28 03:22] dlima: [replying to elias: "now its working indeed, thank you <a:bongoCat:5684..."]
Bro why does tiktok music start playing when I visit your website 😭

[2025-05-29 17:46] Icky Dicky: https://www.man7.org/linux/man-pages/man3/getcontext.3.html

[2025-05-29 17:56] Icky Dicky: Why would it need to enter kernel land?

[2025-05-29 17:56] Icky Dicky: iirc it's just a wrapper for inline assmebly to capture current registers

[2025-05-29 17:56] Icky Dicky: but you can check using strace

[2025-05-29 17:57] Icky Dicky: one sec lemme check

[2025-05-29 17:59] Icky Dicky: It makes a call to `rt_sigprocmask`

[2025-05-29 18:00] Icky Dicky: ```rt_sigprocmask(SIG_BLOCK, NULL, [], 8)  = 0```