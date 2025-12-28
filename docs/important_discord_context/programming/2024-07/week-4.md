# July 2024 - Week 4
# Channel: #programming
# Messages: 32

[2024-07-24 04:30] grb: [replying to nuclearfirefly: "what's stopping you from swapping an iat entry or ..."]
just imagine that the IAT entry that im trying to swap/modify is paged out, and then my code tries to manually page it in, by swapping or modifying the IAT entry which is paged out, there will be a possibility that it will be a recursion

[2024-07-24 04:31] grb: [replying to szczcur: "i think this would normally be fine (outside of th..."]
well for starter i barely have any idea on how to do swapcontext hook, will it even work in the presence of PG?

[2024-07-24 04:31] szczcur: [replying to grb: "well for starter i barely have any idea on how to ..."]
yes

[2024-07-24 04:31] grb: ohh i see i see

[2024-07-24 04:31] grb: im currently trying something else out

[2024-07-24 04:32] grb: i'll keep u updated

[2024-07-26 10:35] mrexodia: <@83203731226628096> I have been doing some research about using `clangd` on Windows and actually I got it working!

[2024-07-26 10:35] Timmy: rly?

[2024-07-26 10:35] Timmy: that's really awesome

[2024-07-26 10:35] Timmy: ngl

[2024-07-26 10:35] mrexodia: Well kinda, there are two parts and I only got the includes working now

[2024-07-26 10:35] Timmy: that's already really big

[2024-07-26 10:36] mrexodia: ```json
  "clangd.arguments": [
    "--log=verbose",
    "--query-driver=*clang-cl.exe",
  ],
```

[2024-07-26 10:36] Timmy: that + language version would probably make it usable

[2024-07-26 10:36] mrexodia: You need to add this option, which will allow clangd to execute the clang-cl.exe driver to extract information about the environment

[2024-07-26 10:38] Timmy: oh shit

[2024-07-26 10:38] Timmy: neat

[2024-07-26 10:38] mrexodia: Yeah it looks like this basically solves everything, because clangd already has built-in support for `clang-cl.exe` (it switches to the MSVC driver if it detects the executable name ends with `-cl.exe`)

[2024-07-26 10:39] Timmy: banger

[2024-07-26 10:41] mrexodia: I also did some major hackery to get `clangd` to work with emscripten and next I want to get the zig toolchain to work

[2024-07-26 10:42] Timmy: https://tenor.com/view/hacker-hackerman-kung-fury-gif-7953536
[Embed: hackerman]

[2024-07-26 14:29] mrexodia: Got it working <@83203731226628096>
[Attachments: image.png]

[2024-07-26 14:34] Timmy: amazing

[2024-07-26 14:42] mrexodia: now I just need to pray that clangd will take my patches

[2024-07-26 14:42] mrexodia: <:harold:704245193016344596>

[2024-07-26 14:43] mrexodia: (so far none of my pull requests to LLVM were ever merged)

[2024-07-26 14:54] Timmy: 😦

[2024-07-26 15:13] mrexodia: https://github.com/llvm/llvm-project/pull/100759 it has commenced
[Embed: [clangd] support the zig c++ compiler wrapper by mrexodia · Pull Re...]
When using zig c++ for cross-compiling clangd removes the zig command from the command line. Because of this the system include extraction fails. This change detects that the driver executable is n...

[2024-07-26 15:39] Torph: 🙏 please god

[2024-07-26 15:39] Brit: Never gonna be merged

[2024-07-26 15:43] Torph: 😭 objectively good change wtf

[2024-07-26 17:37] ehm: I need to fetch direct imgUrls does anyone know a good proxy provider?