# June 2025 - Week 3
# Channel: #programming
# Messages: 35

[2025-06-11 13:59] elias: Does anyone here have some experience with kernel streaming pins?

[2025-06-11 14:05] zeropio: [replying to elias: "Does anyone here have some experience with kernel ..."]
I have a bit, tiny tiny bit

[2025-06-11 14:08] elias: [replying to zeropio: "I have a bit, tiny tiny bit"]
I‘m trying to connect to a microphone pin but it always fails with STATUS_NO_MATCH and even ksstudio (gui app for testing pins) fails with the same error. I (and ksstudio) are using the same format that the pin advertises so it should definitely be supported.

[2025-06-11 14:09] elias: If I try to use the microphone in an app like chrome it works, so Im not sure how this is possible

[2025-06-11 14:11] zeropio: [replying to elias: "I‘m trying to connect to a microphone pin but it a..."]
have you tested with administrator? it could be some privileges issues
if not, you can debug a bit with event viewer, check if there are any logs when

[2025-06-11 14:11] zeropio: ig you can see the microphone in the device manager

[2025-06-11 14:11] zeropio: you can try checking with wasapi, just to troubleshoot

[2025-06-11 14:12] zeropio: https://learn.microsoft.com/en-us/windows/win32/coreaudio/wasapi
[Embed: About WASAPI - Win32 apps]
About WASAPI

[2025-06-11 14:50] elias: I think this is a bug in the vmware drivers

[2025-06-11 14:50] elias: works on my real hardware

[2025-06-13 22:22] rin: so question about windows api in rust, i include a feature in my cargo file and import the module in my program but then a majority of the functions that should be included are not included. using https://microsoft.github.io/windows-docs-rs/doc/windows/Win32/System/index.html#modules . anyone experience this issue before?

[2025-06-13 22:28] qw3rty01: [replying to rin: "so question about windows api in rust, i include a..."]
https://microsoft.github.io/windows-rs/features
[Embed: Rust for Windows - Features]
Search for items in the windows-rs crate and instantly retrieve the required features for cargo.toml.

[2025-06-13 22:28] qw3rty01: all the functions are feature gated, you can search that page for a function you want and it’ll tell you which features to enable

[2025-06-13 22:29] rin: makes sense, thank you

[2025-06-13 22:29] rin: i was under the impression that each feature corresponded to a module.

[2025-06-13 22:30] qw3rty01: yea not really sure the logic behind it but some functions require multiple features to be included

[2025-06-13 22:30] qw3rty01: might be the argument types or something

[2025-06-13 22:32] rin: [replying to qw3rty01: "might be the argument types or something"]
my error was that the function was not in scope so i think its more then arguments

[2025-06-14 16:31] mrexodia: <@687117677512360003> gonna lose it: https://oxcaml.org/

[2025-06-14 16:43] snowua: 
[Attachments: image.png]

[2025-06-14 17:07] 0xdeluks: [replying to mrexodia: "<@687117677512360003> gonna lose it: https://oxcam..."]
so tl;dr this is sth like a ocaml++, right?

[2025-06-14 17:13] Timmy: looks like there's finally actual windows support

[2025-06-14 17:53] Timmy: what shall we call a mutable variable?

[2025-06-14 17:53] Timmy: reference!

[2025-06-14 17:53] Timmy: > OCaml supports imperative programming. Usually, the let … = … syntax does not define variables, it defines constants. However, mutable variables exist in OCaml. They are called references.
<:PES_Facepalm:838743826784845874>

[2025-06-14 17:57] Timmy: so we don't have constant variables, we have constants, but we still need to explicitly mention we're talking about variables that are variable, so lets add 'mutable' here as well. After all that they make sure to confuse everyone even more by breaking your mental model of the distinction between values and references. <:PES2_Party:685143619501293616>

[2025-06-15 05:57] abu: all praise be to ocaml

[2025-06-15 06:00] rin: does anyone have the raymond chen articles where he shows how to create a medium integrity process of a user from a high integrity process of a differerent or same user using explorer?

[2025-06-15 06:11] rin: should clarify its not token hijacking

[2025-06-15 09:18] diversenok: https://devblogs.microsoft.com/oldnewthing/20131118-00/?p=2643
[Embed: How can I launch an unelevated process from my elevated process and...]
Going from an unelevated process to an elevated process is easy. You can run a process with elevation by passing the runas verb to Shell­Execute or Shell­Execute­Ex. Going the other way is trickier. F

[2025-06-15 12:22] contificate: [replying to mrexodia: "<@687117677512360003> gonna lose it: https://oxcam..."]
known about this for years and it's based

[2025-06-15 12:22] contificate: it's believed that jane street have an internal fork where they backported effects from ocaml 5 to 4 etc

[2025-06-15 13:04] mrexodia: Yeah apparently they released everything recently?

[2025-06-15 23:15] contificate: No idea bro

[2025-06-15 23:16] contificate: I don't program any more, just drive about