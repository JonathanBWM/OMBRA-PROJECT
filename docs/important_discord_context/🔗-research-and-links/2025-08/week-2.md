# August 2025 - Week 2
# Channel: #🔗-research-and-links
# Messages: 30

[2025-08-06 19:48] Deleted User: (i thought he sold out)

[2025-08-06 19:48] Deleted User: dont we all know

[2025-08-06 19:50] Deleted User: (is this going to be another killa defcon talk? :p)

[2025-08-06 19:59] codeneverdies: https://fluxsec.red/alt-syscalls-for-windows-11
[Embed: Windows 11 Alternate Syscalls Deep Dive]
A comprehensive technical walkthrough of Windows 11 Alternate Syscalls: allocating executable thunks, building the dispatch table, patching PspServiceDescriptorGroupTable, handling PspSyscallProviderS

[2025-08-06 19:59] rin: I think he wrote a book not to long ago

[2025-08-06 20:02] Deleted User: yayyyyy

[2025-08-06 20:02] Deleted User: https://defcon.org/html/defcon-33/dc-33-speakers.html#content_60356

[2025-08-06 21:32] expy: [replying to codeneverdies: "https://fluxsec.red/alt-syscalls-for-windows-11"]
omg, thank you so much! I've been looking for a while for a good sample of a windows driver written in Rust 🙂

[2025-08-06 23:33] codeneverdies: [replying to expy: "omg, thank you so much! I've been looking for a wh..."]
From the few posts I've read by him it looks like he does a lot in rust

[2025-08-07 16:17] JustMagic: [replying to expy: "omg, thank you so much! I've been looking for a wh..."]
That's not a good sample lol

[2025-08-07 22:58] expy: [replying to JustMagic: "That's not a good sample lol"]
Have a better one?

[2025-08-07 22:59] expy: Windows Rust samples are next to nothing

[2025-08-08 01:33] daax: [replying to expy: "Have a better one?"]
https://github.com/microsoft/Windows-rust-driver-samples
[Embed: GitHub - microsoft/Windows-rust-driver-samples: Rust port of the of...]
Rust port of the official Windows Driver Samples on Github. Leverages windows-drivers-rs - microsoft/Windows-rust-driver-samples

[2025-08-08 01:38] daax: [replying to expy: "Windows Rust samples are next to nothing"]
Also it has nothing to do with the fact that it’s in Rust

[2025-08-08 02:32] JustMagic: [replying to expy: "Have a better one?"]
I don't really know of a single rust windows driver that does anything and would pass basic driver verifier scenarios. (Also a reminder about some of the crates ms published for this https://github.com/microsoft/windows-drivers-rs/blob/935d41a58f7fb3cb6e54600a5a79502ba897c7ba/crates/wdk-panic/src/lib.rs#L28)
[Embed: windows-drivers-rs/crates/wdk-panic/src/lib.rs at 935d41a58f7fb3cb6...]
Platform that enables Windows driver development in Rust. Developed by Surface.  - microsoft/windows-drivers-rs

[2025-08-08 12:11] Brit: [replying to JustMagic: "I don't really know of a single rust windows drive..."]
Steal a core instead of crashing is a vibe

[2025-08-09 12:06] juan diego: when they posting the video

[2025-08-10 01:51] juan diego: anyone got the unlisted stream

[2025-08-10 03:10] Deleted User: [replying to Deleted User: "https://defcon.org/html/defcon-33/dc-33-speakers.h..."]
I wanna go to defcon

[2025-08-10 03:11] Deleted User: But now ima kms

[2025-08-10 03:11] Deleted User: I can’t meet my glorious intel man

[2025-08-10 16:12] juan diego: so brutal have to wait a few months for them to post it

[2025-08-10 16:38] terraphax: took a bit (its just code virtualization in  a nutshell)
https://www.terraphax.com/posts/taxonomy-of-code-virtualizing-transforms
[Embed: Terraphax's Blog - Taxonomy of Code Virtualizing Transforms]
Author's Note
Here is a small little paper I have spent some of my time marking up in LaTeX lately.
Unfortunately, the blog does not render TeX that well, and so I am embedding it as a file from Googl

[2025-08-10 16:39] daax: [replying to terraphax: "took a bit (its just code virtualization in  a nut..."]
Why not upload the pdf here?

[2025-08-10 16:39] terraphax: oh true

[2025-08-10 16:40] terraphax: 
[Attachments: tocvt_finalv1.pdf]

[2025-08-10 16:40] terraphax: there 👍

[2025-08-10 16:41] terraphax: i cant get endorsement for arXiv, so didnt know what to do

[2025-08-10 16:50] Brit: [replying to terraphax: "oh true"]
Self dispatching commonly referred to as direct threading, neat doc

[2025-08-10 16:56] terraphax: [replying to Brit: "Self dispatching commonly referred to as direct th..."]
thank you! i was not aware of the term direct threading before, but it does appear to be a more proper way to call it.