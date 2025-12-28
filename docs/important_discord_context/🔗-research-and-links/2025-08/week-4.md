# August 2025 - Week 4
# Channel: #🔗-research-and-links
# Messages: 9

[2025-08-20 00:05] mrexodia: # August 2025: Bug fixes and stability

This x64dbg release has mostly been focused on fixing bugs and improving stability over the long term.

## 🐛 Bugs

The migration to Visual Studio 2022 unfortunately caused some serious issues to slip through. The following bugs were fixed:

- Systems with older versions of the Visual C++ Redistributable would crash when you started debugging.
- Pattern finding was completely broken.
- Systems with AVX-512 would crash when using x32dbg.
- CPUs AVX support would always show 0 for XMM registers (thanks to @CXVUSER).

Download: https://github.com/x64dbg/x64dbg/releases/tag/2025.08.19
[Embed: Release 2025.08.19 · x64dbg/x64dbg]
August 2025: Bug fixes and stability
This release has mostly been focused on fixing bugs and improving stability over the long term.
🐛 Bugs
The migration to Visual Studio 2022 unfortunately caused ...

[2025-08-20 16:35] Matti: > Systems with AVX-512 would crash when using x32dbg.
hm I see
that's good...
definitely not something I've been investigating the cause of in my own kernel's AVX implementation...
ha ha...

[2025-08-20 16:37] Matti: where is the \:avx512\: emoji when you need it

[2025-08-20 19:42] the horse: [replying to Matti: "where is the \:avx512\: emoji when you need it"]
💩 here u go

[2025-08-21 18:47] terraphax: version 2 of my paper is now up (available from blog + PDF uploaded here as previously wanted)
kept it short.

changes:
- appended discussion of control flow as suggested by Duncan (mrexodia), and appended acknowledgment of Duncan

https://www.terraphax.com/posts/taxonomy-of-code-virtualizing-transforms
[Attachments: tocvt_finalV2.pdf]
[Embed: Terraphax's Blog - Taxonomy of Code Virtualizing Transforms]
Version 2

Changes
- Appended discussion of control flow as per request by Duncan Ogilvie (mrexodia)
- Appended acknowledgment of Duncan Ogilvie 

Here is a direct link to the file on the Google Drive

[2025-08-22 09:53] mrexodia: https://tulach.cc/the-issue-of-anti-cheat-on-linux/
[Embed: The issue of anti-cheat on Linux | Samuel Tulach]
Why are developers so hesitant to bring anti-cheat solutions to Linux?

[2025-08-24 17:41] !cyberpunk: https://matheuzsecurity.github.io/hacking/evading-linux-edrs-with-io-uring/

[2025-08-24 17:41] !cyberpunk: https://github.com/MatheuZSecurity/RingReaper

[2025-08-24 17:41] !cyberpunk: https://github.com/MatheuZSecurity/UnhookingLinuxEdr