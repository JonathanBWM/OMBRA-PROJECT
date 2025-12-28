# July 2024 - Week 4
# Channel: #reverse-engineering
# Messages: 43

[2024-07-22 11:02] luci4: Is it possible to kill Defender via `MpUpdateServicePingRpc`?

[2024-07-22 11:03] luci4: I have a sample that calls `MpConfigInitialize` before this so its probably how it disabled Defender somehow?

[2024-07-22 11:05] luci4: This is the callstack:
[Attachments: image.png]

[2024-07-22 11:09] luci4: I tried looking at the function making those calls in IDA but frankly I couldn't understand anything

[2024-07-23 07:25] mishap: [replying to luci4: "Is it possible to kill Defender via `MpUpdateServi..."]
it's not calling that function, you don't have symbols so it's just getting the nearest named address it has, an export

[2024-07-23 07:28] luci4: [replying to mishap: "it's not calling that function, you don't have sym..."]
Oh I see. I'll try importing the mpclient pdb

[2024-07-23 07:48] luci4: Now all it seems to do is call MpManagerDisable

[2024-07-23 08:12] luci4: Welp guess this probably disables Defender

[2024-07-23 13:33] BWA RBX: [replying to mishap: "it's not calling that function, you don't have sym..."]
I tried looking up that function he mentioned and I see no documentation for it anywhere, how did you know that if you don't mind me asking mate?

[2024-07-23 13:45] luci4: [replying to BWA RBX: "I tried looking up that function he mentioned and ..."]
lol I didnt even question it

[2024-07-23 14:16] Brit: [replying to BWA RBX: "I tried looking up that function he mentioned and ..."]
do you often see funcs 461768 bytes long that recurse into themselves?

[2024-07-23 14:53] BWA RBX: [replying to Brit: "do you often see funcs 461768 bytes long that recu..."]
Thanks, just noticed that how stupid of me 😂😂😂

[2024-07-23 15:00] brymko: <@162611465130475520> there a way for xdbg to handle int3 breakpoints when i place them manually

[2024-07-23 16:00] mrexodia: [replying to brymko: "<@162611465130475520> there a way for xdbg to hand..."]
Yes in settings

[2024-07-23 16:00] mrexodia: You can pass exception_breakpoint to the debugger

[2024-07-23 16:00] mrexodia: Or enable “int3 stepping” which kinda skips int3 in the code

[2024-07-23 16:01] brymko: 
[Attachments: image.png]

[2024-07-23 16:02] brymko: maybe this ?
[Attachments: image.png]

[2024-07-23 20:09] mrexodia: Skip int3 stepping is one way

[2024-07-23 20:09] mrexodia: And handled by debugger for 800003 is also gud

[2024-07-23 20:30] 5pider: [replying to mrexodia: "Yes in settings"]
Lmfao I always used to patch it out by replacing it with a nop 💀 thanks I didn't knew this haha

[2024-07-23 20:31] Brit: Space nop enter is basically muscle memory

[2024-07-23 20:37] 5pider: lmfao so real

[2024-07-23 20:38] mrexodia: [replying to 5pider: "Lmfao I always used to patch it out by replacing i..."]
You can also swallow (like you did with CrowdStrike)
[Attachments: image.png]

[2024-07-23 21:02] Brit: Ah yes ctrl alt shift f8 right alt scroll lock numpad 7 + escape keyup :^)

[2024-07-23 21:02] mrexodia: 💯

[2024-07-23 21:02] mrexodia: `Ctrl+Alt+Shift+Win+L`

[2024-07-24 10:24] zeropio: do you guys know if hypervisor debuggers (like hyperdbg) enable KdDebuggerNotPresent and/or KdPitchDebugger? knowing that patchguard is disabled if a debugger is attached, could hyperdbg bypass those checks (at least)?

[2024-07-24 10:25] zeropio: is there any way to debug patchguard, as windbg connected to the machine, or debugging and patchguard is totally out of range

[2024-07-24 10:31] zeropio: I'm trying to do Can's recent blog on patchguard at work, so I read tetrane's document (https://blog.tetrane.com/downloads/Tetrane_PatchGuard_Analysis_RS4_v1.01.pdf) for some reference
I'm not trying to do anything specific with patchguard, just wondering if it can be debugged

[2024-07-24 14:14] szczcur: [replying to zeropio: "I'm trying to do Can's recent blog on patchguard a..."]
yes, but it clears dr7 and checks for hwbps elsewhere. you need to use an hv-based debugger to do it easily

[2024-07-24 14:15] szczcur: [replying to zeropio: "do you guys know if hypervisor debuggers (like hyp..."]
i’d hope they wouldn’t. theyre open source though so you can always look.

[2024-07-24 14:33] zeropio: okey, that answer my questions, thanks

[2024-07-25 13:45] Matti: [replying to Brit: "Ah yes ctrl alt shift f8 right alt scroll lock num..."]
CTRL

[2024-07-25 13:45] Matti: [replying to mrexodia: "💯"]
SHIFT

[2024-07-25 13:45] Matti: WIN

[2024-07-25 13:45] Matti: B
[Attachments: image.png]

[2024-07-25 14:51] Torph: wait really?? that's wild

[2024-07-26 12:02] elias: did anyone ever try to build a custom warbird dll that works with msvc?

[2024-07-26 12:02] elias: i.e reverse engineer how warbird integrates with msvc

[2024-07-26 12:51] th3: why does x64dbg show different instructions after scrolling

[2024-07-26 13:26] Brit: Desync issue

[2024-07-28 23:02] bowen: stl