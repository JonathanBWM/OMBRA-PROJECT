# August 2024 - Week 1
# Channel: #programming
# Messages: 32

[2024-08-02 23:04] elias: <@456226577798135808> If you don't mind me asking, did you protect strings in your custom vm somehow?

[2024-08-02 23:05] Deleted User: [replying to elias: "<@456226577798135808> If you don't mind me asking,..."]
nope

[2024-08-02 23:08] 5pider: [replying to elias: "<@456226577798135808> If you don't mind me asking,..."]
Isnt it easier to write your string protector in the C++ files directly ? And have somr sort of constexpr that does this for you ?

[2024-08-02 23:08] 5pider: I am doing it like that lol

[2024-08-03 00:43] mibho: how to properly read these? <:cringepepe:681351213828276244>  

int (* ( ***sub_10011075())[2])()

somedata = (int (* * (*)[2])())someVTable

[2024-08-04 10:46] Deleted User: Hello, why the opcodes in `add al, spl` are `40 00 e0`, the REX `40` `Rex.B` `Rex.X` `Rex.R` `Rex.W` are all set to 0, then what is the purpose of adding REX? especially that i am adding a 8 bit reg to another 8 bit reg

[2024-08-04 12:20] kian: Register `SPL` accesses the least significant 8 bits of `RSP` which is only allowed in 64bit mode. Outside of 64bit mode, you can only access the least significant 8 bits of `RAX, RBX, RCX, RDX`. So the REX prefix is needed, but technically doesn't do anything.

[2024-08-04 12:48] Deleted User: [replying to kian: "Register `SPL` accesses the least significant 8 bi..."]
oh, okay thanks i appreciate it

[2024-08-04 13:58] 冰: hmm

[2024-08-04 14:02] elias: whats the lastest API set version on Windows 11?

[2024-08-04 14:02] 5pider: 6

[2024-08-04 14:02] elias: what API set version was used on original Windows 10?

[2024-08-04 14:02] 5pider: i think also 6 ?

[2024-08-04 14:02] elias: that would be great

[2024-08-04 14:02] 5pider: i think windows 10 and 11 both use 6

[2024-08-04 14:03] elias: nice thank you !

[2024-08-04 14:03] 5pider: i know that older than 10 (maybe 7) used version 4

[2024-08-04 14:03] elias: yup

[2024-08-04 14:03] elias: I was hoping that I just have to implement 6 :D

[2024-08-04 14:03] elias: because Im not gonna support win 7 and 8

[2024-08-04 14:04] 5pider: lfmao real

[2024-08-04 14:04] 5pider: when i did ApiSet parsing i only did version 6

[2024-08-04 14:14] mrexodia: You don't need to actually implement ApiSet parsing

[2024-08-04 14:14] mrexodia: nerds

[2024-08-04 14:15] 5pider: [replying to mrexodia: "You don't need to actually implement ApiSet parsin..."]
wdym

[2024-08-04 14:15] mrexodia: You can do it by reading the actual `api-ms` DLL files

[2024-08-04 14:15] mrexodia: https://github.com/x64dbg/x64dbg/blob/938138072ad5b5a0c9ca7a9869655fd7b3363ec4/src/dbg/module.cpp#L1454-L1543
[Embed: x64dbg/src/dbg/module.cpp at 938138072ad5b5a0c9ca7a9869655fd7b3363e...]
An open-source user mode debugger for Windows. Optimized for reverse engineering and malware analysis. - x64dbg/x64dbg

[2024-08-04 14:17] 5pider: oh wait let me take a look

[2024-08-04 14:22] 5pider: oh wow u are right

[2024-08-04 14:22] 5pider: this is something i didnt knew

[2024-08-04 14:22] 5pider: cheers thx

[2024-08-04 15:39] luci4: Wow good to know, thanks!