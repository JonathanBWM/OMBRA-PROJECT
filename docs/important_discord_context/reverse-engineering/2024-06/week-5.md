# June 2024 - Week 5
# Channel: #reverse-engineering
# Messages: 121

[2024-06-24 06:47] Glatcher: guys, how much it costs to extract data (nothing criminal, just old app that contains necessary pictures and articles) from application, which is protected by simple VM?

[2024-06-24 12:23] Deleted User: time and effort

[2024-06-24 12:43] luci4: [replying to Deleted User: "time and effort"]
🤣

[2024-06-24 12:48] Ignotus: just use both ghidra and binja

[2024-06-24 12:48] Ignotus: cross referencing makes reversing easier anyway

[2024-06-24 23:18] diversenok: Can somebody explain how these versions map onto versions/builds of Windows?
```
#define NTDDI_WIN10                         0x0A000000
#define NTDDI_WIN10_TH2                     0x0A000001
#define NTDDI_WIN10_RS1                     0x0A000002
#define NTDDI_WIN10_RS2                     0x0A000003
#define NTDDI_WIN10_RS3                     0x0A000004
#define NTDDI_WIN10_RS4                     0x0A000005
#define NTDDI_WIN10_RS5                     0x0A000006
#define NTDDI_WIN10_19H1                    0x0A000007
#define NTDDI_WIN10_VB                      0x0A000008
#define NTDDI_WIN10_MN                      0x0A000009
#define NTDDI_WIN10_FE                      0x0A00000A
#define NTDDI_WIN10_CO                      0x0A00000B
#define NTDDI_WIN10_NI                      0x0A00000C
#define NTDDI_WIN10_CU                      0x0A00000D
#define NTDDI_WIN11_ZN                      0x0A00000E
#define NTDDI_WIN11_GA                      0x0A00000F
#define NTDDI_WIN11_GE                      0x0A000010
```

[2024-06-24 23:18] diversenok: Everything up to 19H1 makes sense, after that I'm lost

[2024-06-24 23:24] flan: MN / FE / CO / NI / CU / ZN / GA / GE are consecutive elements in the periodic table

[2024-06-24 23:25] diversenok: Cool I guess

[2024-06-24 23:25] diversenok: But what versions of Windows are they

[2024-06-24 23:26] flan: mb I misread the question :d

[2024-06-25 00:13] JustMagic: [replying to diversenok: "But what versions of Windows are they"]
VB=19h2/20h1?, MN=20h2, rest are linear, just adding a half

[2024-06-25 00:17] JustMagic: The messed up part about `VB` is that `Vanadium` is 19h2 and `Vibranium` is 20h1

[2024-06-25 00:18] JustMagic: Someone at product / management probably had a stroke at the start and fucked up the naming for first bit (Vanadium -> Vibranium (probably to skip chromium lmao) -> manganese ...)

[2024-06-25 15:11] DeChaos: ```c
v8 = (*(_DWORD *)(unk + 12) & 0x600) - 0x200; // not sure what the fuck is this
if ( (v8 & 0xFFFFFDFF) != 0 )
{
  // do shit
}
else
{
  // do shit
}
```

Hi. was reversing some shit and i'm not sure why they do -0x200, is this shit happened by whacky MSVC shit or smth?

[2024-06-25 15:34] Glatcher: [replying to DeChaos: "```c
v8 = (*(_DWORD *)(unk + 12) & 0x600) - 0x200;..."]
idk, we need more information, about ur binary, disasm. Also it could be just pointer or structure, 0x200 probably disk sector size. Not enough info..

[2024-06-25 15:38] Glatcher: I'll make a guess:
`unk+12` structure's field (structure probably taken from win/macos/linux api)
`& 0x600` - restrict ur `unk+12` size
`- 0x200` - offset for `unk+12` value (idk, probably bios?)))

[2024-06-25 15:40] DeChaos: well it's actually win32 application. unk + 12 seems to be bit flag which isn't negative value. so i'm bit guessing -0x200 is "shifting" bits or smth, but still confused why would they do that if they simply wanted to check & 0x600 only. what im confused is what can, and what are they checking as deducting 0x200 and check the flag as AND op with 0xFFFFFDFF.

[2024-06-25 15:41] Glatcher: U're sure its a bit flag?

[2024-06-25 15:41] DeChaos: i wasn't sure if that was a fuckup done by compiler or smth, or actual code that is checking for more flag bits

[2024-06-25 15:41] DeChaos: it is, yes

[2024-06-25 15:42] Glatcher: Probably mistake of decompiler?

[2024-06-25 15:42] Glatcher: Is there disassembly code?

[2024-06-25 15:44] DeChaos: sure
```
//bitFlag = *(_DWORD *)(unk + 12);
mov     eax, [rcx+0Ch]
mov     rdi, rcx
//~snip~, [rcx+0Ch] is never touched
mov     eax, [rdi+0Ch]
and     eax, 600h
sub     eax, 200h
```

[2024-06-25 15:46] DeChaos: whoops, forgot inst that copies rcx to rdi

[2024-06-25 15:46] Glatcher: Just check how it works using python interpreter, probably it should work in this way

[2024-06-25 15:46] DeChaos: uhhhh, wdym

[2024-06-25 15:47] Glatcher: Or code simple C program

[2024-06-25 15:47] Glatcher: Or ask chatgpt?..

[2024-06-25 15:47] DeChaos: yeah i tried, but still was too unclear the motive of -0x200 and then comparing as & 0xFFFFFDFF

[2024-06-25 15:47] DeChaos: that's something i would never trust tho lol

[2024-06-25 15:48] Glatcher: [replying to DeChaos: "```c
v8 = (*(_DWORD *)(unk + 12) & 0x600) - 0x200;..."]
I mean just about hex maths presented here

[2024-06-25 16:56] naci: [replying to DeChaos: "yeah i tried, but still was too unclear the motive..."]
0xFFFFFDFF == ~0x200 so i think `v8 & 0xFFFFFDFF` checks if `(*(_DWORD *)(unk + 12) & 0x600)` is 0 ?

[2024-06-25 16:57] naci: or it checks if `(*(_DWORD *)(unk + 12) & 0x600) - 0x200` is 0 and `(*(_DWORD *)(unk + 12) & 0x600) ` is 0x200

[2024-06-25 16:58] mrexodia: [replying to DeChaos: "```c
v8 = (*(_DWORD *)(unk + 12) & 0x600) - 0x200;..."]
Is there any `alloca` in the function? Stuff like that happens sometimes

[2024-06-25 17:08] irql: just looks like a really weird bitwise optimization to me

[2024-06-25 17:09] irql: `(*(_DWORD *)(unk + 12) & 0x600)` is always going to be either, 0, 0x200, 0x400, or 0x600

[2024-06-25 17:10] irql: looks like the first case is hit whenever it's either 0, or 0x600?

[2024-06-25 17:12] irql: i'd probably just look at it like it were a `if ((*(_DWORD *)(unk + 12) & 0x600) == 0 || (*(_DWORD *)(unk + 12) & 0x600) == 0x600)`

[2024-06-25 17:12] irql: optimized into a single op

[2024-06-25 17:13] diversenok: It checks `(Something - 0x200) & ~0x200` which evaluates to true for `Something` being `0` and `0x600` but not `0x200` and `0x400`

[2024-06-25 17:13] irql: ^

[2024-06-25 17:13] DeChaos: oh

[2024-06-25 17:13] DeChaos: jesus, you guys are lifesaver

[2024-06-25 17:13] DeChaos: thanks a ton

[2024-06-26 22:40] Deleted User: <@219672123839348737>

[2024-06-27 00:26] .daniel.w: In IDA, is there a way to load a library with exported functions and then have it go through the current pe file and match those functions?
Right now I'm trying to get all the Lua functions. I can do it manually by generating a signature and then doing "test sig" in the executable, but that's time consuming.

Cheers

[2024-06-27 00:29] .daniel.w: Also, how do I get IDA to stop corrupting my databases? It seems like that's what it's best at 🙃

[2024-06-27 01:11] BWA RBX: [replying to Deleted User: "<@219672123839348737>"]
Hey

[2024-06-27 01:21] BWA RBX: [replying to .daniel.w: "In IDA, is there a way to load a library with expo..."]
I'm sure you can accomplish this with IDAPython they have lots of helper functions for this

[2024-06-27 01:21] Deleted User: [replying to BWA RBX: "Hey"]
pussy

[2024-06-27 01:22] Deleted User: you?

[2024-06-27 01:22] .daniel.w: [replying to BWA RBX: "I'm sure you can accomplish this with IDAPython th..."]
I was looking but meh, I really can't be arsed with all of that right now

[2024-06-27 01:23] .daniel.w: I'm surprised nothing already exists for this though

[2024-06-27 01:24] BWA RBX: [replying to Deleted User: "you?"]
Bussy

[2024-06-27 01:24] Deleted User: you are bussy?

[2024-06-27 01:24] BWA RBX: I like bussy

[2024-06-27 01:26] Deleted User: 🤔

[2024-06-27 01:26] Deleted User: busy?

[2024-06-27 01:26] BWA RBX: [replying to .daniel.w: "I was looking but meh, I really can't be arsed wit..."]
Would be easy enough to parse your library file store the function signatures and apply them in ida no?

[2024-06-27 01:27] .daniel.w: [replying to BWA RBX: "Would be easy enough to parse your library file st..."]
What do you mean exactly? 🤔

[2024-06-27 01:28] BWA RBX: [replying to Deleted User: "busy?"]
Not very busy but busy, I want to test out Nyxstone maybe someone will add an IDA Plug-in for it 🤪

[2024-06-27 01:36] BWA RBX: [replying to .daniel.w: "What do you mean exactly? 🤔"]
Basically what I would do is load my lua library and automate generating flirt signatures once it's loaded and applying it to the IDB

[2024-06-27 01:38] BWA RBX: You can do all this using IDAPython and even make your own plug-in for it if you need it more times than often

[2024-06-27 01:58] .daniel.w: I'd say I'm pretty good at naming things
[Attachments: 1719453541081.png]

[2024-06-27 02:00] .daniel.w: [replying to BWA RBX: "You can do all this using IDAPython and even make ..."]
I'll look into it, still not sure if I'll be sticking with IDA just yet

[2024-06-27 02:00] .daniel.w: Thanks :)

[2024-06-27 02:01] .daniel.w: And whilst I'm here, what the best way of updating a project? Say the executable changes, is there a way to quickly and easily update it and keep all changes that are the same?

[2024-06-27 02:08] BWA RBX: [replying to .daniel.w: "And whilst I'm here, what the best way of updating..."]
P sure bindiff does this no?

[2024-06-27 02:11] .daniel.w: [replying to BWA RBX: "P sure bindiff does this no?"]
I've used it for diffing, but I didn't know it can edit a project?

[2024-06-27 02:14] BWA RBX: [replying to .daniel.w: "I've used it for diffing, but I didn't know it can..."]
I'm pretty sure it can definitely apply changes to the new binary

[2024-06-27 02:15] .daniel.w: Hm, I'll try that out soon then, thanks

[2024-06-27 08:03] dullard: [replying to .daniel.w: "Also, how do I get IDA to stop corrupting my datab..."]
Don’t use a cracked version <:Kappa:1082189237178351666>

[2024-06-27 08:04] dullard: That 7.7 release from chinapyg did that

[2024-06-27 08:04] dullard: Not sure about the 8. Release crack 🤔

[2024-06-27 08:05] Terry: 8. been fine for me so far

[2024-06-27 08:05] Terry: china 7.7 is ptsd

[2024-06-27 11:01] .daniel.w: I'm on 8 right now, but yeah, 7 was the same

[2024-06-27 12:35] Brit: [replying to .daniel.w: "I'd say I'm pretty good at naming things"]
imagine coming back to this idb a year later

[2024-06-27 12:36] Brit: you're gonna love yourself for your naming conventions

[2024-06-27 12:37] .daniel.w: Haha yep 😄 
It's only temporary, it just makes it less confusing because it's this instead of `sub_xxx` everywhere, I know I need to find out what it does.
But this is the constructor of the primary structure, it also does a lot of other crap

[2024-06-27 12:38] .daniel.w: When generating a structure in IDA, is there a way to make it use `DWORD` instead of `_DWORD` (same with all the others)?

[2024-06-27 12:38] Timmy: I'd suggest adopting a naming scheme so you can not waste time thinking of names, not have ban names and communicate to your (future) self that you didn't know what this function did at the time of naming

[2024-06-27 12:40] .daniel.w: [replying to Timmy: "I'd suggest adopting a naming scheme so you can no..."]
I think I've got something for that now. I've only really just started this one, so the quicker I can name things, the quicker it is to figure out other things. And then once I figure out other thing, I can properly name what the other function was since I have a better understanding of where and how it's used

[2024-06-27 12:40] Timmy: I use AAUNK<OptionalClassOrNamespace>_<ProperFunctionName>

[2024-06-27 12:40] .daniel.w: That's what I done last night anyway, made it really easy to identify a ton of functions

[2024-06-27 12:41] .daniel.w: [replying to Timmy: "I use AAUNK<OptionalClassOrNamespace>_<ProperFunct..."]
For functions I do know, I name them: `Namespace::FunctionName`

[2024-06-27 12:52] .daniel.w: IDA has a tendancy to do this when generating a function signature, I need to remember to never use the first option (auto create ida pattern) 🙃
[Attachments: image.png]

[2024-06-27 14:26] sariaki: [replying to Timmy: "I use AAUNK<OptionalClassOrNamespace>_<ProperFunct..."]
what does AAUNK stand for here

[2024-06-27 14:26] sariaki: A..
A..
U..
N..ot 
K..now?

I feel like I'm missing out on something really obvious here

[2024-06-27 14:27] Timmy: AA gets the function to be at the top of the function list if you sort it by name <:OMEGALUL:662670462215782440>

[2024-06-27 14:27] sariaki: OOH

[2024-06-27 14:27] sariaki: LOIl

[2024-06-27 14:28] Timmy: unk is well

[2024-06-27 14:28] Timmy: unknown

[2024-06-27 14:28] sariaki: ye

[2024-06-27 14:28] sariaki: shii i might start using that AA

[2024-06-27 20:07] repnezz: any useful commands to debug a verifier 0xC4 violation with caller tried to free a bad pool address? 
or just in general , things to look at? I free only if ptr is not null

[2024-06-28 15:47] luci4: What exactly are "Special machine frames"? As found in `InitSpecialMachineFrames`

[2024-06-28 15:47] luci4: I couldn't find much online

[2024-06-28 15:53] Deleted User: does anybody know the type of the `MmRegistryState` global
[Attachments: image.png]

[2024-06-28 16:02] Matti: [replying to Deleted User: "does anybody know the type of the `MmRegistryState..."]
it's `MM_REGISTRY_STATE`

[2024-06-28 16:02] Matti: happy to help!

[2024-06-28 16:02] Matti: okok, and the type should be something like

```c
typedef struct _MM_REGISTRY_STATE
{
    ULONG MoveImages;
    ULONG DisablePagingExecutive;
    ULONG DisablePageCombining;
    ULONG TrackPtes;
    ULONG LargePageMinimum;
    ULONG ForceValidateIo;
    ULONG Mirroring;
    ULONG EnableCooling;
    ULONG ConsumedPoolPercentage;
    ULONG ModifiedWriteMaximumPages;
    ULONG AllocationPreference;
    ULONG CritsectTimeoutSeconds;
    ULONG MinimumStackCommitInBytes;
    ULONG ProductType;
    SIZE_T HeapDeCommitFreeBlockThreshold;
    SIZE_T HeapDeCommitTotalFreeThreshold;
    SIZE_T HeapSegmentCommit;
    SIZE_T HeapSegmentReserve;
    SIZE_T LowMemoryThreshold;
    SIZE_T HighMemoryThreshold;
} MM_REGISTRY_STATE, *PMM_REGISTRY_STATE;
```

[2024-06-28 16:02] Deleted User: <@148095953742725120> thanks very much

[2024-06-28 16:03] Matti: note this is from a 10.0.14393.0 private PDB, idk for certain if it's still up to date

[2024-06-28 16:03] Matti: but I would guess probably

[2024-06-28 16:04] Matti: if there's another field following it you can check the size at least

[2024-06-28 16:04] Deleted User: [replying to Matti: "note this is from a 10.0.14393.0 private PDB, idk ..."]
likely is, it makes sense in the context it's being used in

[2024-06-28 16:04] Matti: it should be 0x68 for 64 bit

[2024-06-28 16:04] Deleted User: just needed to check what was at +0xC

[2024-06-28 19:34] luci4: [replying to luci4: "What exactly are "Special machine frames"? As foun..."]
Still trying to figure this out

[2024-06-28 19:34] luci4: That function is called by LdrpInitialize

[2024-06-28 20:53] Matti: [replying to luci4: "Still trying to figure this out"]
these are used during unwinding exceptions in images that were compiled with `-GUARD:EHCONT` (so exception continuation data)

[2024-06-28 20:54] Matti: don't know why they are called machine frames, as far as I know they are unwind info

[2024-06-28 20:58] Matti: it might just be the address of a single unwind info, namely that of KiUserExceptionDispatcher

[2024-06-28 20:59] Matti: have to say I'm not entirely sure how that makes it useful

[2024-06-28 21:00] Matti: check out `RtlpIsContinuationContextMachineFrameEntry` for its usage during prologue unwinding btw

[2024-06-28 21:01] Matti: that function is extremely simple (it just loops and compares) but has a name that's open to interpretation

[2024-06-28 21:02] Matti: have to say I don't really feel like reversing all of what unwindprologue does nowadays to figure out the rest <:thinknow:475800595110821888>

[2024-06-29 05:54] luci4: [replying to Matti: "have to say I don't really feel like reversing all..."]
I see, thanks a lot!!! I'll reverse it and see for myself