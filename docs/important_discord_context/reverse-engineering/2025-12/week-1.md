# December 2025 - Week 1
# Channel: #reverse-engineering
# Messages: 35

[2025-12-01 14:03] rin: does anyone have experience bypassing single-step and access violation anti debug while using frida

[2025-12-03 12:23] hxm: i think REMILL by default will always preserve stack writes when i checked the instrinsics.

[2025-12-03 12:24] plpg: But stack is in memory

[2025-12-03 12:24] plpg: Do you mean which ones are heap and which ones are stack?

[2025-12-03 12:27] hxm: in order tog et everything folded correctly i have to find a way to get ride of all this stack ops. but not all mem ones.

[2025-12-03 12:30] estrellas: mem2reg?

[2025-12-03 12:36] hxm: i think -O3 includes that, and it doesnt really affect it

[2025-12-03 15:37] mrexodia: [replying to hxm: "in order tog et everything folded correctly i have..."]
You first need to detect and transform the stack operations

[2025-12-03 15:37] mrexodia: You can use alloca and offset into that as a base idea

[2025-12-05 10:47] freeme: what are some fun things to reverse engineer that you guys have came accross from beginner -> advanced. I wanna get back into this field and i feel like reverse engineering is the best place to start. I wanna avoid crackmes, but if its a must and the best / fastest way to increase skill / gain back memory

[2025-12-05 11:07] BloodBerry: [replying to freeme: "what are some fun things to reverse engineer that ..."]
I think the fist step is to go deep into languages… like C, C++ and etc. who they work and how the code in assembly work…

Try to reverse and hack ur own application then go next

[2025-12-05 11:09] freeme: [replying to BloodBerry: "I think the fist step is to go deep into languages..."]
ok!

[2025-12-05 11:10] BloodBerry: [replying to freeme: "ok!"]
If u wanna make a progress like a “playground/sandbox” try to hack some game without AC

[2025-12-05 11:11] BloodBerry: For ex. REPO or older games that don’t have lib2cpp obfuscation

[2025-12-06 10:08] grb: i tried to hook win32k syscalls using InfinityHook, my hook function does get executed but when it tries to call the original syscall routine, it triggers a bugcheck, KERNEL_SECURITY_CHECK_FAILURE, reason :Indirect call guard check detected invalid control transfer. what my hook function does is very simple (call RtlWalkFrameChain and log to ETW with TraceLoggingWrite) so i doubt my hook function is the one who triggers the bugcheck. anyone ever experienced this before? I do have other way to hook win32k, but i wanna try to use the same solution for other NT syscalls

[2025-12-06 19:30] freeme: [replying to BloodBerry: "If u wanna make a progress like a “playground/sand..."]
yes i do

[2025-12-06 19:30] freeme: exactly what i want to do

[2025-12-06 19:30] freeme: but i will start with the fundamentals ofc

[2025-12-06 19:30] freeme: [replying to BloodBerry: "For ex. REPO or older games that don’t have lib2cp..."]
any other games

[2025-12-06 19:30] freeme: with some really basic obfuscation

[2025-12-06 19:31] masuka: [replying to grb: "i tried to hook win32k syscalls using InfinityHook..."]
ur crash is the cfg blocking infinity hook. the hook runs but the jump back into the original win32k syscall stub fails the indirect call guard, which triggers the bugcheck. it is not your hook body. infinity hook no longer works on current Windows builds because the syscall dispatcher is now cfg protected

[2025-12-06 23:39] grb: [replying to masuka: "ur crash is the cfg blocking infinity hook. the ho..."]
well the weird thing is that other NT syscalls works just fine

[2025-12-07 02:44] masuka: win32k uses a different syscall entry than normal NT syscalls. That entry has stricter cfg checks. infinity hook modifies that entry, so the cfg validation fails there but not on the nt one. That is why nt syscalls work and win32k syscalls bugcheck

[2025-12-07 02:45] masuka: <@776638120950235167>

[2025-12-07 02:45] masuka: ig avoid win32k lol

[2025-12-07 02:45] grb: i c i c, thanks

[2025-12-07 03:36] Xyrem: <@776638120950235167> try to tailcall

[2025-12-07 03:37] grb: [replying to Xyrem: "<@776638120950235167> try to tailcall"]
jmp-ing from my hook function to the original routine?

[2025-12-07 03:38] Xyrem: Ye

[2025-12-07 03:39] grb: tried that before but for the nt syscall hooks, always triggers a shadow stack bugcheck, weird af indeed, tho i believe its just a skill issue on my aide

[2025-12-07 05:41] grb: [replying to masuka: "win32k uses a different syscall entry than normal ..."]
yeah tried straight up data ptr hook, this time doesnt even touched my hook function lmfao

[2025-12-07 05:53] masuka: [replying to grb: "yeah tried straight up data ptr hook, this time do..."]
lmao

[2025-12-07 06:07] grb: [replying to grb: "i tried to hook win32k syscalls using InfinityHook..."]
update, disabled KCFG in my driver build, now its smooth asf

[2025-12-07 11:50] grb: [replying to Xyrem: "<@776638120950235167> try to tailcall"]
btw could you perhaps guide me on how to do tailcall? i tried to use llvm-msvc so that i could use inline assembly but i guess i failed at attempting to setup the tailcall

[2025-12-07 13:41] masuka: [replying to grb: "update, disabled KCFG in my driver build, now its ..."]
nice