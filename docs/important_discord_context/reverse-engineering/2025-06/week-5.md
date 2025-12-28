# June 2025 - Week 5
# Channel: #reverse-engineering
# Messages: 32

[2025-06-23 07:30] Horsie: [replying to mtu: "If the process is protected, the DLL has to have a..."]
Oh.. I didnt realize. Thanks :)

[2025-06-23 07:31] Horsie: So, off the top of your head, any other way I can have my process show up as `IsProtectedProcess`?
(<https://ntdoc.m417z.com/process_extended_basic_information>)

[2025-06-23 07:32] Horsie: Doesnt seem likely unless I'm missing out on some weird quirks

[2025-06-23 07:41] Pepsi: [replying to Horsie: "Doesnt seem likely unless I'm missing out on some ..."]
Unless you are a AV vendor, you are probably not gonna be a PPL on a prod system.

[2025-06-23 07:43] Pepsi: but iirc you can alter accepted certificates via testmode and create a protected process with a test signed executable that has been signed with the required extended attributes

[2025-06-23 12:36] shalzuth: [replying to Horsie: "Trying to play around with protected processes and..."]
For testing, you -might- be able to load a ssp into lsa
I did this about 4 years ago, but I don’t remember the exact specifics on how I got it to load.
If something like this would work for you, I can try to load an old project and let you know if it still works.

[2025-06-24 17:28] Horsie: [replying to shalzuth: "For testing, you -might- be able to load a ssp int..."]
If you do have a poc at hand I'd love to take a look for sure

[2025-06-24 19:14] 0xboga: Is there a way to leak the EPROCESS address of my own process purely from UM?

[2025-06-24 19:16] diversenok: If you have the debug privilege or run an older version of Windows - sure

[2025-06-24 19:17] 0xboga: I do

[2025-06-24 19:17] diversenok: Otherwise, MS is trying to prevent it

[2025-06-24 19:17] 0xboga: NTQSI? What info class gives this?

[2025-06-24 19:17] mannyfreddy: handles

[2025-06-24 19:17] diversenok: `SystemExtendedHandleInformation`

[2025-06-24 19:17] mannyfreddy: think it was infoclass 16

[2025-06-24 19:19] diversenok: [replying to mannyfreddy: "think it was infoclass 16"]
You should generally avoid `SystemHandleInformation` (info class 16) because it cannot return handle values above 65535

[2025-06-24 19:19] mannyfreddy: noted

[2025-06-24 19:21] 0xboga: [replying to diversenok: "`SystemExtendedHandleInformation`"]
So open a pseudo handle to my own process, and enumerate all opened process handles in my process for -1 handle value? Just made it up, any easier way?

[2025-06-24 19:22] diversenok: No, it only enumerates real handles, so you need to open one

[2025-06-24 19:23] diversenok: Also, you don't really "open" pseudo handles

[2025-06-24 19:24] diversenok: Otherwise, yes. Open a handle, snapshot, find

[2025-06-24 19:34] 0xboga: Cheers thanks

[2025-06-27 07:16] shalzuth: [replying to Horsie: "If you do have a poc at hand I'd love to take a lo..."]
<https://github.com/shalzuth/LsassMemShim/tree/PPL> note - you have to turn off Local Security Authority, maybe Core Isolation too
but when running that, it runs in LSASS and GetProcessInformation(7)  returns 4 (PROTECTION_LEVEL_LSA_LIGHT)

[2025-06-27 07:23] shalzuth: the app creates a dll of itself, does secur32.AddSecurityPackage to add the new dll as a security package, and then communicates between the newly added lsass security package and itself via a pipe.
i also never got around to unloading, so you might have to restart to redo it. test it in a vm/windows sandbox..

[2025-06-28 12:48] eternablue: any ideas why my IDA 7.7 crashes with windows 11's ntoskrnl ? 😭
[Attachments: image.png]

[2025-06-28 13:58] Xyrem: [replying to eternablue: "any ideas why my IDA 7.7 crashes with windows 11's..."]
have the same issue, it works fine on IDA 8+

[2025-06-28 14:00] eternablue: i sent them en email with the crashdump, we never knoow 😂
[Attachments: image.png]

[2025-06-28 14:00] eternablue: https://tenor.com/view/basketball-meme-gif-5051779013126861108

[2025-06-28 19:33] Pepsi: [replying to eternablue: "i sent them en email with the crashdump, we never ..."]
Dear Octavian Dima,

unfortunately we cannot provide any customer support, 
as the support period of your license is exceeded.

Sincerely, 
Hex-Rays Support Team

[2025-06-29 10:17] archie_uwu: hey there, I'm currently reversing Hyper-V (the AMD build), looking at the runtime memory allocator - one thing strikes me as odd, that being what looks like multiple ways of allocating.

The main function checks if the requested size is >4000 bytes - not 4096, which is the quirk that I don't fully understand. From what I can gather with my limited knowledge of how memory allocators work, if size < 4000 bytes, it allocates from a pool, and if that fails, it falls back to the big function that allocates from who knows where.

Is there a logical reason as to why they might check for 4000 bytes instead of 4096?

[2025-06-29 10:55] Timmy: The allocator might have a 'header' of some kind.

[2025-06-29 20:56] vgksys: yeh 96 byte reserved for allocator metadata