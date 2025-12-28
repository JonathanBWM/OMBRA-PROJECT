# February 2025 - Week 2
# Channel: #programming
# Messages: 502

[2025-02-03 23:56] James: hey im nearly through all the base instructions of arm now

[2025-02-03 23:57] James: i've described what is read/written to, in what order, etc.

[2025-02-03 23:57] James: anyone want to jump on board or the SIMD, SVE and SME instructions?

[2025-02-04 00:19] elias: did anyone else encounter problems when trying to set dbgsettings in BCD in Win11 24H2?

[2025-02-04 00:24] Icky Dicky: [replying to elias: "did anyone else encounter problems when trying to ..."]
yes my computer blew up

[2025-02-04 00:24] elias: <:Exhausted:1216680844287148032>

[2025-02-04 00:25] Icky Dicky: https://media.discordapp.net/attachments/1178000532602900603/1197708449056817162/GRVNZEBOMA_-_1748105504864317672.gif

[2025-02-04 08:38] Matti: [replying to elias: "did anyone else encounter problems when trying to ..."]
no

[2025-02-04 08:38] Matti: can you be more specific

[2025-02-04 10:11] 0xatul: [replying to elias: "did anyone else encounter problems when trying to ..."]
did you turnoff secure boot before doing that?

[2025-02-04 11:55] elias: Meh, I just created a new VM. This happened after I applied the new bootmanager revocations and the bootmgfw_ex

[2025-02-04 11:55] elias: and yeah secure boot was off

[2025-02-04 12:07] Matti: *what is the 'this'*

[2025-02-04 12:08] Matti: describe your issue

[2025-02-04 12:11] elias: it would tell me the BCD store couldn‘t be opened

[2025-02-04 12:11] elias: only with /dbgsettings

[2025-02-04 12:11] elias: other bcdedit stuff worked

[2025-02-04 12:21] Matti: that does sound odd

[2025-02-04 12:22] Matti: I can reproduce this no problem, but only if I enable UAC and start a non-admin cmd prompt

[2025-02-04 12:22] Matti: 
[Attachments: image.png]

[2025-02-04 12:22] Matti: but, as you can see other commands also fail

[2025-02-04 12:23] Matti: this is 23H2 or whatever the fuck 22621 is called by the way, but I have 24H2 and server 2025 installed on other machines that don't have this issue either

[2025-02-04 12:26] Matti: [replying to elias: "Meh, I just created a new VM. This happened after ..."]
can you elaborate on the second bit

[2025-02-04 12:27] Matti: still sounds kind of unlikely to be related to me, but basically idk what this means

[2025-02-04 12:28] elias: yeah since I installed a new VM its working now again, probably not that important

[2025-02-04 12:29] elias: [replying to Matti: "can you elaborate on the second bit"]
i mean adding the new UEFI cert to DB, putting the old microsoft pca 2011 to dbx and installing the new bootmanager

[2025-02-04 12:29] Matti: OK, I've done the first two of those

[2025-02-04 12:30] Matti: but no clue what installing the new boot manager implies

[2025-02-04 12:30] elias: windows now comes with bootmgfw.efi which is still signed by old cert and bootmgfw_EX.efi which is signed by the new cert

[2025-02-04 12:31] elias: so you need to replace the old one so that windows can continue to boot

[2025-02-04 12:31] elias: 
[Attachments: image.png]

[2025-02-04 12:31] Matti: oh, yeah I have seen those

[2025-02-04 12:32] Matti: but never manually moved or replaced any files on the ESP

[2025-02-04 12:32] elias: oh okay

[2025-02-04 12:32] elias: i think the MS guide for applying the new changes instructs you to do this <:peepoDetective:570300270089732096>

[2025-02-04 12:32] elias: or just verify it got replaced correctly? Not sure anymore

[2025-02-04 12:33] Matti: that sounds kind of odd, I mean it's possible if the guide is meant for sysadmins I suppose, but normally I would expect windows update to take care of this

[2025-02-04 12:33] Matti: it can see which certs are in db and dbx

[2025-02-04 12:34] Matti: have you got a link? wouldn't mind a look

[2025-02-04 12:43] elias: [replying to Matti: "have you got a link? wouldn't mind a look"]
https://support.microsoft.com/en-us/topic/how-to-manage-the-windows-boot-manager-revocations-for-secure-boot-changes-associated-with-cve-2023-24932-41a975df-beb2-40c1-99a3-b3ff139f832d

[2025-02-04 12:43] elias: ah it just tells u to verify its the correct one
[Attachments: IMG_0572.png]

[2025-02-04 12:44] elias: I copied it manually to ESP because this whole revocation process failed for 5 times for some reason with weird errors in the event logs

[2025-02-04 12:45] elias: I think they are not ready to enforce the update yet

[2025-02-04 12:46] elias: but at least it seems they managed to fix all the mitigation bypasses for the cve now after 3 years

[2025-02-04 12:47] Matti: [replying to elias: "https://support.microsoft.com/en-us/topic/how-to-m..."]
cheers

[2025-02-04 12:48] Matti: doesn't look like the updated bootmgr/fw files are on my ESP yet on a system with a fresh 24H2 installation

[2025-02-04 12:49] Matti: so yeah it doesn't seem to be automated for now

[2025-02-04 12:49] elias: yep

[2025-02-04 12:51] elias: maybe this is the end for the absolute clownfest that's been going on for years with the bootmanager revocations <:PepeLaugh:1013910191659561001>

[2025-02-04 12:51] Matti: yeahhh

[2025-02-04 12:51] Matti: I'm sure

[2025-02-04 12:57] Matti: well, I seem to already be passing this part of the requirements
[Attachments: image.png]

[2025-02-04 12:58] Matti: the annoying thing is I'd probably need to enable SB to get the new boot manager to install itself

[2025-02-04 12:59] Matti: oh that's not even installing I think, it just copies the _ex files to the ESP? lol

[2025-02-04 12:59] Matti: and then you still need to manually actually replace the files

[2025-02-04 12:59] Matti: christ

[2025-02-04 18:05] Matti: [replying to elias: "https://support.microsoft.com/en-us/topic/how-to-m..."]
so I got this to work on one of my machines, up to and including the last two step of revoking the 2011 CA and applying the "SVN number" (Secure Version Number Number) to the boot manager

[2025-02-04 18:05] Matti: the only snag I ran into was the dbx update seemingly not taking

[2025-02-04 18:06] Matti: but it turns out the 2011 CA was also in my db list

[2025-02-04 18:06] Matti: so I had to remove it manually here
[Attachments: image.png]

[2025-02-04 18:06] Matti: after that windows could apply the dbx update OK

[2025-02-04 18:07] Matti: bit confused as to how this works because I'd assume dbx would have priority over db

[2025-02-04 18:07] Matti: but yea

[2025-02-04 18:10] Matti: no idea how this machine is not upsetting bitlocker somehow lol... especially via the HSTI
[Attachments: image.png]

[2025-02-04 18:13] diversenok: Mattitrends International, lol

[2025-02-04 18:14] Matti: yeah and that is also a bogus BIOS version and release date <:thonk:575445129863888896>

[2025-02-04 18:14] Matti: with full write access to the BIOS region from the OS

[2025-02-04 18:16] diversenok: What key protectors does bitlocker use?

[2025-02-04 18:16] luci4: [replying to diversenok: "Mattitrends International, lol"]
How did I not notice that lol

[2025-02-04 18:16] diversenok: I can see anything being possible if it skips all PCR checks

[2025-02-04 18:17] Matti: well it checks a bunch of things by default, though some of them are configurable
e.g. here is a more typical sample (my normal workstation) - this list can grow very long
[Attachments: image.png]

[2025-02-04 18:17] Matti: but the one I would definitely expect on the above screenshot would be "hardware security interface test failed"

[2025-02-04 18:18] Matti: PCR7 is fine here because I have secure boot and a TPM enabled

[2025-02-04 18:18] diversenok: More like "hardware security interface test is very confused"

[2025-02-04 18:18] Matti: yeah, or plain absent

[2025-02-04 18:18] Matti: that's the best explanation I can think of

[2025-02-04 18:19] Matti: unusual though, I haven't seen that before

[2025-02-04 18:19] Matti: if you ran chipsec on this thing it would explode

[2025-02-04 18:19] Matti: 
[Attachments: image.png]

[2025-02-04 18:20] Matti: oh yeah lmao

[2025-02-04 18:20] Matti: it has DCI enabled as well

[2025-02-04 18:20] Matti: I forgot about that

[2025-02-04 18:20] Matti: there's no way the HSTI is working

[2025-02-04 18:24] Matti: [replying to luci4: "How did I not notice that lol"]
Mattitrends is a fast growing PC manufacturer and BIOS improvisationalist
[Attachments: image.png]

[2025-02-04 18:24] elias: [replying to Matti: "after that windows could apply the dbx update OK"]
for me the dbx update also failed a few times before it randomly worked

[2025-02-04 18:25] elias: probably somehow the BCD broke during this time

[2025-02-04 18:25] Matti: IMO, my BCD breaks as soon as I enable secure boot

[2025-02-04 18:26] Matti: because half of my settings are not compliant with SB

[2025-02-04 18:26] Matti: like, fine, I get that

[2025-02-04 18:26] Matti: but why is it necessary to fucking delete them from the BCD

[2025-02-04 18:26] Matti: so then after I disable SB again I need to restore a backup

[2025-02-04 18:27] elias: <:mmmm:904523247205351454>

[2025-02-04 18:27] elias: what a mess

[2025-02-04 18:28] elias: unrelated, but is it normal for `BCryptGenRandom` to return `STATUS_NOT_SUPPORTED` on kernel mode? <:peepoDetective:570300270089732096>

[2025-02-04 18:29] luci4: [replying to Matti: "Mattitrends is a fast growing PC manufacturer and ..."]
Is it publicly traded? I'd like to invest

[2025-02-04 18:30] Matti: [replying to elias: "unrelated, but is it normal for `BCryptGenRandom` ..."]
not sure but, are you importing from ksecdd.sys or cng.sys?

[2025-02-04 18:30] elias: tried both

[2025-02-04 18:30] Matti: [replying to luci4: "Is it publicly traded? I'd like to invest"]
IPO is in the works

[2025-02-04 18:30] elias: BCryptOpenAlgorithmProvider with the RNG provider returns not supported as well

[2025-02-04 18:30] elias: but the documentation states its supported

[2025-02-04 18:31] Matti: what is your irql my friend
[Attachments: image.png]

[2025-02-04 18:31] elias: PASSIVE_LEVEL

[2025-02-04 18:33] Matti: well then

[2025-02-04 18:34] Matti: kernel debugging time

[2025-02-04 18:38] elias: <:mmmm:904523247205351454>

[2025-02-04 18:40] Matti: looking at the docs, I'm going to guess your hAlgorithm parameter doesn't support this API somehow
> `[in, out] hAlgorithm`
> The handle of an algorithm provider created by using the BCryptOpenAlgorithmProvider function. **The algorithm that was specified when the provider was created must support the random number generator interface.**

[2025-02-04 18:41] Matti: so that's uh... `BCRYPT_RNG_ALGORITHM`?

[2025-02-04 18:41] Matti: I think?

[2025-02-04 18:41] Matti: as in literally just that one

[2025-02-04 18:43] elias: You should be able to set this parameter to NULL when using the BCRYPT_USE_SYSTEM_PREFERRED_RNG flag

[2025-02-04 18:43] Matti: that is true

[2025-02-04 18:43] Matti: assuming you're at PASSIVE... but you say you are, so

[2025-02-04 18:44] Matti: I'd just try opening the RNG algorithm explicitly and setting flags to 0

[2025-02-04 18:44] elias: yeah whatever, Im gonna use RtlRandomEx but I could have sworn this worked before

[2025-02-04 18:44] elias: [replying to elias: "BCryptOpenAlgorithmProvider with the RNG provider ..."]
I tried and it doesnt work either

[2025-02-04 18:45] Matti: well if you actually wanna know why it's not working

[2025-02-04 18:45] Matti: you need a debugger

[2025-02-04 18:45] Matti: I mean you pretty much always need a debugger

[2025-02-04 18:46] Matti: but especially for this

[2025-02-04 18:52] Matti: [replying to elias: "yeah whatever, Im gonna use RtlRandomEx but I coul..."]
was looking for this... impossible-to-distinguish-from-secure PRNG without the bloat of RtlRandomEx

```c
#define Multiplier ((ULONG)(0x80000000ul - 19)) // 2**31 - 19
#define Increment  ((ULONG)(0x80000000ul - 61)) // 2**31 - 61
#define Modulus    ((ULONG)(0x80000000ul - 1))  // 2**31 - 1

ULONG RtlpRandomConstantVector[128] = {
    0x4c8bc0aa, 0x4c022957, 0x2232827a, 0x2f1e7626, 0x7f8bdafb, 0x5c37d02a, 0x0ab48f72, 0x2f0c4ffa,
    0x290e1954, 0x6b635f23, 0x5d3885c0, 0x74b49ff8, 0x5155fa54, 0x6214ad3f, 0x111e9c29, 0x242a3a09,
    0x75932ae1, 0x40ac432e, 0x54f7ba7a, 0x585ccbd5, 0x6df5c727, 0x0374dad1, 0x7112b3f1, 0x735fc311,
    0x404331a9, 0x74d97781, 0x64495118, 0x323e04be, 0x5974b425, 0x4862e393, 0x62389c1d, 0x28a68b82,
    0x0f95da37, 0x7a50bbc6, 0x09b0091c, 0x22cdb7b4, 0x4faaed26, 0x66417ccd, 0x189e4bfa, 0x1ce4e8dd,
    0x5274c742, 0x3bdcf4dc, 0x2d94e907, 0x32eac016, 0x26d33ca3, 0x60415a8a, 0x31f57880, 0x68c8aa52,
    0x23eb16da, 0x6204f4a1, 0x373927c1, 0x0d24eb7c, 0x06dd7379, 0x2b3be507, 0x0f9c55b1, 0x2c7925eb,
    0x36d67c9a, 0x42f831d9, 0x5e3961cb, 0x65d637a8, 0x24bb3820, 0x4d08e33d, 0x2188754f, 0x147e409e,
    0x6a9620a0, 0x62e26657, 0x7bd8ce81, 0x11da0abb, 0x5f9e7b50, 0x23e444b6, 0x25920c78, 0x5fc894f0,
    0x5e338cbb, 0x404237fd, 0x1d60f80f, 0x320a1743, 0x76013d2b, 0x070294ee, 0x695e243b, 0x56b177fd,
    0x752492e1, 0x6decd52f, 0x125f5219, 0x139d2e78, 0x1898d11e, 0x2f7ee785, 0x4db405d8, 0x1a028a35,
    0x63f6f323, 0x1f6d0078, 0x307cfd67, 0x3f32a78a, 0x6980796c, 0x462b3d83, 0x34b639f2, 0x53fce379,
    0x74ba50f4, 0x1abc2c4b, 0x5eeaeb8d, 0x335a7a0d, 0x3973dd20, 0x0462d66b, 0x159813ff, 0x1e4643fd,
    0x06bc5c62, 0x3115e3fc, 0x09101613, 0x47af2515, 0x4f11ec54, 0x78b99911, 0x3db8dd44, 0x1ec10b9b,
    0x5b5506ca, 0x773ce092, 0x567be81a, 0x5475b975, 0x7a2cde1a, 0x494536f5, 0x34737bb4, 0x76d9750b,
    0x2a1f6232, 0x2e49644d, 0x7dddcbe7, 0x500cebdb, 0x619dab9e, 0x48c626fe, 0x1cda3193, 0x52dabe9d
    };

ULONG
RtlUniform (
    IN OUT PULONG Seed
    )
/*++

Routine Description:
    A simple uniform random number generator, based on D.H. Lehmer's 1948
    alrogithm.

Arguments:
    Seed - Supplies a pointer to the random number generator seed.

Return Value:
    ULONG - returns a random number uniformly distributed over [0..MAXLONG]

--*/

{
    *Seed = ((Multiplier * (*Seed)) + Increment) % Modulus;
    return *Seed;
}
```

[2025-02-04 18:55] elias: interesting <:ThumbsUp:985957232065806387>

[2025-02-04 18:56] Matti: well I mean, do you want a PRNG or a cryptographically secure PRNG like bcrypt

[2025-02-04 18:57] Matti: I assumed the latter

[2025-02-04 18:57] Matti: so in that sense substituting RtlRandom[Ex] which are just variants of the above doesn't make a lot of sense

[2025-02-04 18:58] Matti: might as well go for the fastest, no?

[2025-02-04 18:58] Matti: not that that is RtlUniform...

[2025-02-04 18:59] Matti: https://github.com/espadrine/shishua
[Embed: GitHub - espadrine/shishua: SHISHUA – The fastest PRNG in the world]
SHISHUA – The fastest PRNG in the world. Contribute to espadrine/shishua development by creating an account on GitHub.

[2025-02-04 19:15] pinefin: [replying to Matti: "was looking for this... impossible-to-distinguish-..."]
magic numbe

[2025-02-04 19:38] Timmy: [replying to Matti: "Mattitrends is a fast growing PC manufacturer and ..."]
haha, I'd like to see the face of a valve employee that sees that

[2025-02-04 19:39] Timmy: "wait I thought we stopped supporting that a while ago"

[2025-02-04 19:39] pinefin: they probably wont think too far into it, with all these malware thats disguised as "stripped windows"

[2025-02-04 19:42] Matti: agree

[2025-02-04 19:42] Matti: but it's not impossible I'll find a printf vulnerability in a game with this CPU brand string some day

[2025-02-05 15:54] Torph: [replying to Matti: "if you ran chipsec on this thing it would explode"]
based

[2025-02-06 17:15] Horsie: [replying to Matti: "if you ran chipsec on this thing it would explode"]
Is that some gnome built-in?

[2025-02-06 17:15] Horsie: Very cool..

[2025-02-06 17:57] Matti: [replying to Horsie: "Is that some gnome built-in?"]
it seems to be - either that or ubuntu

[2025-02-06 17:57] Matti: since I was booting a live USB here

[2025-02-06 17:58] Matti: personally I would not associate either of those with anything that is 'cool'

[2025-02-06 17:59] Matti: just use chipsec if you want to query this information

[2025-02-06 17:59] Matti: as well as fwupd to check for out of date firmware

[2025-02-07 19:48] Matti: [replying to Matti: "OK so this is an abbreviated version of what I've ..."]
<@943099229126144030> <@758032611078701277> I took another stab at this using a different approach and it's actually working pretty well now I would say

[2025-02-07 19:49] Matti: because working on game code that runs in kernel mode is so fucking tedious, I decided to clone puredoom (the barebones make your own doom framework) instead and work on that, using the provided SDL example

[2025-02-07 19:53] Matti: the thing that helped me most of all to be honest was seeing that the SDL using sample (with a proper window, keyboard/mouse window message handling, audio, a CRT, etc)... also had fucking terrible input

[2025-02-07 19:53] Matti: especially the keyboard

[2025-02-07 19:53] Matti: it's as if you're never really holding a key, but just tapping it repeatedly

[2025-02-07 19:54] Matti: so the movement acceleration was way off for example

[2025-02-07 19:56] Matti: after moving all of the SDL input processing to a single function to be called from the game loop, I then ported over my win32k crap and made it work in user mode (not much work because win32k already thinks every caller is in user mode anyway)

[2025-02-07 19:58] Matti: one instant win I got after this was removing the Sleep() call that's currently in ntdoom's game loop

[2025-02-07 19:58] Matti: it's fucking horrible

[2025-02-07 19:59] Matti: so that alone was pretty much enough to make keyboard input work better than doing it with SDL like the sample does

[2025-02-07 20:00] Matti: unfortunately the code is pretty gross now <:harold:704245193016344596>  I blame mouse button handling for this

[2025-02-07 20:02] Matti: it turns out that the 'game mode' vs 'menu mode' decision that is so important for making kb input work well, just didn't work at all for mouse buttons

[2025-02-07 20:03] Matti: I suspect that this is related to some win32k internals that are different for mouse inputs

[2025-02-07 20:03] Matti: basically, the LSB (= has the key been pressed/released since the last call) is never true for mouse buttons

[2025-02-07 20:05] Matti: this is what I've got now, and yes all of these fucking checks really need to be there and exactly like this 😩
[Attachments: image.png]

[2025-02-07 20:05] Matti: but it does work

[2025-02-07 20:05] Matti: all that's missing now is mouse movement

[2025-02-07 20:06] Matti: I suspect that'd require raw input or something

[2025-02-07 20:07] Matti: also, I desperately need to find or else just create something that's more like an actual window than the fucking desktop

[2025-02-07 20:07] Matti: you can see how the desktop icons clip through the game here
[Attachments: 2025-02-07_20-06-52.mp4]

[2025-02-07 20:10] Matti: it losing focus is another thing

[2025-02-07 20:10] Matti: or more like, it just never has focus

[2025-02-07 20:54] Hunter: u can draw it over the command prompt that starts the service

[2025-02-07 20:57] Torph: [replying to Matti: "the thing that helped me most of all to be honest ..."]
lmao

[2025-02-07 20:59] Matti: [replying to Hunter: "u can draw it over the command prompt that starts ..."]
you can't see it in this video but if I click inside the game window (usually on some desktop icon), the desktop icon will be clicked

[2025-02-07 20:59] Matti: this isn't a win32 window

[2025-02-07 20:59] Matti: it's just a region of the screen being blitted to

[2025-02-07 21:00] Matti: so if I were to draw it over the console instead, ~~well first of all that's a bad idea because there are other ways to launch a driver but,~~ the mouse click would just go through to the console window instead and click or select something there

[2025-02-07 21:03] pinefin: [replying to Matti: "you can see how the desktop icons clip through the..."]
is this in a busy loop?

[2025-02-07 21:03] pinefin: it doesnt return driver entry success

[2025-02-07 21:04] Matti: yeah, that's something I could pretty trivially fix but I haven't done it yet

[2025-02-07 21:04] Matti: the reason driverentry doesn't return is because it waits for the game loop to return

[2025-02-07 21:04] Matti: and the game runs in an APC inside explorer.exe

[2025-02-07 21:04] pinefin: PsCreateSystemThread moment

[2025-02-07 21:04] Matti: if I quit the game using the menu, it does return

[2025-02-07 21:04] pinefin: [replying to Matti: "and the game runs in an APC inside explorer.exe"]
wwtf

[2025-02-07 21:04] pinefin: [replying to pinefin: "PsCreateSystemThread moment"]
oh

[2025-02-07 21:04] pinefin: i see

[2025-02-07 21:04] pinefin: wrong reply

[2025-02-07 21:05] pinefin: [replying to Matti: "the reason driverentry doesn't return is because i..."]
.t hiso ne

[2025-02-07 21:05] Hunter: t hiso ne

[2025-02-07 21:05] Matti: [replying to pinefin: "PsCreateSystemThread moment"]
no need, that would only overcomplicate things

[2025-02-07 21:05] Matti: I can simply make driverunload responsible for killing the game if that is needed

[2025-02-07 21:05] pinefin: i see

[2025-02-07 21:05] pinefin: i dont know how it runs

[2025-02-07 21:07] Matti: this is still more or less the design I'm using
<https://github.com/NSG650/NtDOOM/blob/master/NtDOOM/entry.c#L544> DriverEntry initializes and queues* a kernel APC here, targeting explorer in session 1
it then waits for the completed event in the struct it passes as context parameter to the APC routine

[2025-02-07 21:07] Matti: <https://github.com/NSG650/NtDOOM/blob/master/NtDOOM/entry.c#L403> this is the game loop that runs in the APC

[2025-02-07 21:08] pinefin: i see

[2025-02-07 21:08] pinefin: DOOM.WAD

[2025-02-07 21:08] Matti: yeah that is a pretty gross hack tbh, not my code

[2025-02-07 21:08] Matti: I've since changed that line

[2025-02-07 21:09] Matti: if you pass `-file somewad.wad` to doom, it will think you're playing a modified game

[2025-02-07 21:09] Matti: new code is just
```c
char* argv[] = { "doom" };
int flags = DOOM_FLAG_MENU_DARKEN_BG;
doom_init(ARRAYSIZE(argv), argv, flags);
```

[2025-02-07 21:09] Matti: but this did require writing a bunch of file I/O functions as well

[2025-02-07 21:10] Matti: that doom can then use to open its regular array of files it tries (doom.wad, doom2.wad, doom1.wad, and a few obscure ones)

[2025-02-07 21:12] Matti: the biggest change I had to make for 24H2 was to change the APC from a special kernel APC (but with processormode = UserMode) to a regular kernel APC with processormode = KernelMode

[2025-02-07 21:14] Matti: there are actually 3 APC routines now instead of 1

[2025-02-07 21:15] Matti: but the added two are boilerplate to make this design workable

[2025-02-07 21:15] pinefin: make it run off irq's

[2025-02-07 21:15] pinefin: ☠️

[2025-02-07 21:16] pinefin: how does it render?

[2025-02-07 21:16] pinefin: im assuming gdi?

[2025-02-07 21:16] Matti: the nop routine simply has to exist because you can't insert an APC into the kernel queue that doesn't have a special kernel APC routine
and the rundown routine is useful because you don't want crashing explorer.exe to make your driver impossible to unload as a side effect
[Attachments: image.png]

[2025-02-07 21:16] Matti: [replying to pinefin: "how does it render?"]
good fucking question

[2025-02-07 21:16] Matti: yes, GDI

[2025-02-07 21:16] Matti: but that's about all I know

[2025-02-07 21:17] Matti: I'm pretty sure the GDI drawing leaks memory like a sieve

[2025-02-07 21:18] pinefin: oh brother

[2025-02-07 21:18] Matti: because it bugchecks often

[2025-02-07 21:18] Matti: usually blaming dxgmm.sys for improperly freeing memory

[2025-02-07 21:18] Matti: that was of course more likely just corrupted by this driver

[2025-02-07 21:19] Matti: like, in my head a line like
`HBITMAP Result = NtGdiCreateBitmap(320, 200, 1, 32, framebuffer);`
is supposed to have a matching deletebitmap, or whatever it is called

[2025-02-07 21:20] Matti: no such things happen here

[2025-02-07 21:21] Matti: `NtGdiSelectBitmap` might be similar

[2025-02-07 21:21] Matti: because it returns the previously selected bitmap

[2025-02-07 21:56] Torph: [replying to Matti: "also, I desperately need to find or else just crea..."]
how/why are you rendering on the desktop without a window anyway

[2025-02-07 22:04] Matti: another excellent question

[2025-02-07 22:04] Matti: I've been wondering the same thing myself

[2025-02-07 22:04] Matti: just to clarify, the drawing code wasn't written by me

[2025-02-07 22:04] Matti: I've written maybe 3 lines of GUI code in my life

[2025-02-07 22:05] Matti: any rendering would have been D3D or vulkan

[2025-02-07 22:05] Matti: I have no fucking clue how the drawing works

[2025-02-07 22:06] Matti: well, insofar as it can be said to work

[2025-02-07 22:06] pinefin: i know for sure theres a way gdi can render on top of everything

[2025-02-07 22:06] pinefin: and since its in a explorer apc, im ASSUMING they're colliding

[2025-02-07 22:06] pinefin: given

[2025-02-07 22:06] pinefin: i dont know how most of windows subsystems work

[2025-02-07 22:06] pinefin: i just know that some p2c's back then used to use gdi to render because it was windowless

[2025-02-07 22:08] Matti: yeah, this will just blit an RGBA buffer to the screen over anything else that may be there

[2025-02-07 22:08] Matti: but there are still the occasional glitches where the desktop comes through, I can't explain that

[2025-02-07 22:08] Matti: it's not really the biggest issue tbh

[2025-02-07 22:08] Matti: the window not having mouse focus is worse

[2025-02-07 22:08] pinefin: [replying to Matti: "I've written maybe 3 lines of GUI code in my life"]
i wish i had your issues man

[2025-02-07 22:09] pinefin: everything i do somehow ends up at frontend

[2025-02-07 22:09] pinefin: (besides freertos work that i do)

[2025-02-07 22:10] Matti: well I've written unreal engine code, I guess that's basically GUI code

[2025-02-07 22:10] Matti: it's user facing

[2025-02-07 22:12] pinefin: theres the ocassion that i have to write swift code

[2025-02-07 22:12] pinefin: to talk to the devices i program on

[2025-02-07 22:19] Torph: [replying to Matti: "yeah, this will just blit an RGBA buffer to the sc..."]
oh ok

[2025-02-07 22:27] irql: [replying to Matti: "but there are still the occasional glitches where ..."]
it's because its using NtUserGetDc(0)

[2025-02-07 22:27] Matti: yea

[2025-02-07 22:28] irql: you can make a layered window & draw to that instead

[2025-02-07 22:28] Matti: this part I managed to figure out

[2025-02-07 22:28] irql: and it'll fix the flickering

[2025-02-07 22:28] Matti: hmmm

[2025-02-07 22:28] Matti: how do I do that?

[2025-02-07 22:28] Matti: I was hoping there might be a convenient existing victim window to hijack

[2025-02-07 22:28] Matti: but not opposed to creating one

[2025-02-07 22:29] irql: I think I have some code somewhere

[2025-02-07 22:29] irql: not sure how you'd do it from kernel mode though -- I only have user32 code

[2025-02-07 22:29] Matti: ah that'd be great

[2025-02-07 22:29] Matti: [replying to irql: "not sure how you'd do it from kernel mode though -..."]
this part is np, it's the same code just different names

[2025-02-07 22:30] Matti: it's all win32 user mode code, only win32k unfortunately does exist in kernel space technically speaking

[2025-02-07 22:34] irql: ```cpp
    POINT screen;
    screen.x = GetSystemMetrics(SM_CXSCREEN);
    screen.y = GetSystemMetrics(SM_CYSCREEN);

    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = 0;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = GetModuleHandleW(NULL);
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wcex.hbrBackground = NULL;
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"1";
    wcex.hIconSm = NULL;


    if (!RegisterClassExW(&wcex))
        return;

    HWND wnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW, L"1", L"2", WS_POPUP, 0, 0, screen.x, screen.y, NULL, NULL, GetModuleHandleW(NULL), NULL);

ShowWindow(wnd, SW_SHOW);
// create drawing thread 

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
```

sorry bro, this code is disgusting and its from like when 7 years ago or something -- this is how I was creating it

[2025-02-07 22:34] irql: this might be way too annoying for kernel mode

[2025-02-07 22:34] irql: the drawing procedure, I can post too, its just GetDC(wnd) followed by CreateCompatibleBitmap/CreateCompatibleDC, and UpdateLayeredWindow

[2025-02-07 22:35] Matti: \>this code is disgusting
well I did ask for win32 code

[2025-02-07 22:35] irql: but it would create a transparent overlay window, which you could draw to

[2025-02-07 22:35] Matti: I can hardly complain about that part

[2025-02-07 22:35] Matti: thanks a lot

[2025-02-07 22:35] irql: [replying to Matti: "it's all win32 user mode code, only win32k unfortu..."]
ahh hmmmmm

[2025-02-07 22:35] irql: I do wonder if you can actually do that from kernel mode lol

[2025-02-07 22:35] Matti: ```c
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
```
yeah this part I've been a bit worried about

[2025-02-07 22:35] irql: yea

[2025-02-07 22:35] pinefin: [replying to irql: "```cpp
    POINT screen;
    screen.x = GetSystemM..."]
yep this was a p2c

[2025-02-07 22:36] irql: thats the rough part

[2025-02-07 22:36] pinefin: WS_EX_TOPMOST

[2025-02-07 22:36] pinefin: WS_EX_TRANSPARENT

[2025-02-07 22:36] irql: it wasn't actually

[2025-02-07 22:36] pinefin: im joke

[2025-02-07 22:36] Matti: I could create a user mode thread to pump messages I suppose

[2025-02-07 22:36] Matti: in the same process

[2025-02-07 22:36] irql: lmfao I guess you probably could've ran an overlay on it tbf

[2025-02-07 22:36] irql: hmmm, I can try without the message loop actually, hold on

[2025-02-07 22:36] Matti: well

[2025-02-07 22:37] Matti: I did try that with the SDL puredoom

[2025-02-07 22:37] Matti: it did not do much in the way of input processing

[2025-02-07 22:37] pinefin: [replying to Matti: "I could create a user mode thread to pump messages..."]
wouldnt this ruin the "NtDoom" and make it usermode dependant

[2025-02-07 22:37] Matti: well, nothing really

[2025-02-07 22:37] Matti: [replying to pinefin: "wouldnt this ruin the "NtDoom" and make it usermod..."]
yeah, it does kinda run against the spirit I agree

[2025-02-07 22:37] Matti: maybe it's possible to pump messages in kernel mode

[2025-02-07 22:38] Matti: I'd have to look into that

[2025-02-07 22:38] irql: hm yea, I guess you need that message pump

[2025-02-07 22:38] Matti: not sure I'm quite *that* interested lol

[2025-02-07 22:38] Matti: but we'll see

[2025-02-07 22:38] Matti: I could start with the UM thread as a hack to see if it works as a proof of concept

[2025-02-07 22:38] irql: there's probably that stub exactly in some random windows program

[2025-02-07 22:38] irql: yea

[2025-02-07 22:38] irql: hmm

[2025-02-07 22:40] irql: ```cpp
    HDC wnd_dc = GetDC(wnd);
    HBITMAP bmp = CreateCompatibleBitmap(wnd_dc, screen.x, screen.y);
    HDC draw_dc = CreateCompatibleDC(wnd_dc);
    SelectObject(draw_dc, bmp);

while (...) {
StretchBlt(draw_dc, -(screen.x/2), 0, screen.x*2, screen.y, bmp_0_dc, 0, 0, 1200, 1200, sw);

UpdateLayeredWindow(wnd, wnd_dc, NULL, (LPSIZE)&screen, draw_dc, &src, sw ? RGB(255, 255, 255) : RGB(0, 0, 0), NULL, ULW_COLORKEY);
}
```

[2025-02-07 22:40] irql: drawing was something like that, I guess same thing as what you currently have, but with a different DC / UpdateLayeredWindow call

[2025-02-07 22:40] Matti: btw, not sure of the significance of this, but I notice SDL calls PeekMessage

[2025-02-07 22:40] Matti: not Get

[2025-02-07 22:41] Matti: and then follows with the same Translate + Dispatch

[2025-02-07 22:41] Matti: oh but also passing `PM_REMOVE`

[2025-02-07 22:41] Matti: so that's just throwing them away? 🤔

[2025-02-07 22:43] Matti: no, just removes them from the queue

[2025-02-07 22:43] Matti: not sure how that is any different from GetMessage then

[2025-02-07 22:43] irql: sounds like the same thing tbh lmfao

[2025-02-07 22:43] Matti: [replying to irql: "drawing was something like that, I guess same thin..."]
yeah I think the drawing part I can probably scrape together myself

[2025-02-07 22:44] Matti: but regular win32 window stuff and window messaging, I don't know anything about and I kinda feel like I wanna keep it that way

[2025-02-07 22:44] irql: its disgusting

[2025-02-07 22:45] irql: but for some reason, the win32 GUIs always feel better

[2025-02-07 22:46] Matti: wdym, compared to what?

[2025-02-07 22:46] irql: idk man, the old GUIs always feel so much better

[2025-02-07 22:46] irql: you get a vs code window and im vomiting everywhere

[2025-02-07 22:46] irql: or even C# winforms programs

[2025-02-07 22:46] Matti: oh so as opposed to like, electron shit

[2025-02-07 22:46] Matti: yeah agreed obviously

[2025-02-07 22:47] Matti: win32 GUIs are great to use

[2025-02-07 22:47] Matti: just not to write

[2025-02-07 22:47] irql: well, even UWP

[2025-02-07 22:47] irql: and the legacy winforms stuff

[2025-02-07 22:47] irql: [replying to Matti: "just not to write"]
yea lmfao

[2025-02-07 22:47] Matti: also agreed

[2025-02-07 22:47] Matti: I mean you are talking to the guy who used the windows 95 theme until the death of win 7

[2025-02-07 22:48] irql: lmfao

[2025-02-07 22:48] Matti: which was the last version to support the classic theme

[2025-02-07 22:48] irql: yea I loved that shit too tbf

[2025-02-07 22:51] irql: btw that NSG guy im pretty sure he's on some discords

[2025-02-07 22:51] irql: im surprised hes not in here

[2025-02-07 22:52] Matti: whomst'dve?

[2025-02-07 22:52] irql: the guy on that NtDoom git repo

[2025-02-07 22:52] Matti: ah right

[2025-02-07 22:52] Matti: I wasn't aware he was on discord

[2025-02-07 22:53] pinefin: [replying to irql: "btw that NSG guy im pretty sure he's on some disco..."]
theres other programming discords?

[2025-02-07 22:53] pinefin: im only in re & c++ & sc

[2025-02-07 22:54] pinefin: c++ is filled with webdevs that glaze the language asking how to learn it and then end up leaving

[2025-02-07 22:54] pinefin: so im never in there

[2025-02-07 22:58] irql: ok I found him

[2025-02-07 22:58] irql: sent him an invite

[2025-02-07 22:58] irql: [replying to pinefin: "theres other programming discords?"]
ehh

[2025-02-07 22:59] irql: not that I know of lmfao

[2025-02-07 22:59] irql: it's just some discord, I think i've spoken to him before

[2025-02-07 22:59] Matti: [replying to irql: "sent him an invite"]
cool, appreciate it

[2025-02-07 23:00] irql: hm yea, I thought i'd seen him around fr

[2025-02-07 23:00] irql: [replying to pinefin: "c++ is filled with webdevs that glaze the language..."]
this is real lmfao

[2025-02-07 23:01] irql: cs grads 🦾

[2025-02-07 23:05] pinefin: [replying to irql: "cs grads 🦾"]
lets get colin in there so he can give them a whole programming theory course on why they should learn ocaml instead

[2025-02-07 23:06] contificate: yes

[2025-02-07 23:09] pinefin: colin

[2025-02-07 23:09] pinefin: i will go program something in ocaml.

[2025-02-07 23:09] pinefin: when im less busy

[2025-02-07 23:09] pinefin: in your honor

[2025-02-07 23:10] contificate: better yet, man, just do some basic algorithms in your language of choice

[2025-02-07 23:21] pinefin: c++

[2025-02-07 23:21] pinefin: thats my langugae of choice

[2025-02-07 23:21] pinefin: i make algorithms every day

[2025-02-07 23:21] pinefin: i work with CANbus

[2025-02-07 23:21] pinefin: j1939 baby

[2025-02-07 23:22] pinefin: gotta convert the kelvin to fahrenheit somehow man

[2025-02-07 23:22] James: <@991360481493262411> and i agree that Rust is the best language.

[2025-02-07 23:26] irql: blocked

[2025-02-07 23:31] contificate: [replying to pinefin: "i make algorithms every day"]
now do something fun

[2025-02-07 23:31] contificate: implement a trie

[2025-02-07 23:31] contificate: implement robin hood hashing

[2025-02-07 23:32] contificate: implement merge sort

[2025-02-07 23:32] contificate: implement aho-corasick

[2025-02-07 23:32] contificate: kelvin to fahrenheit is trivial bollocks

[2025-02-07 23:32] contificate: the things above are trivial bollocks as well

[2025-02-07 23:32] contificate: but fun

[2025-02-07 23:33] contificate: after that, compute a DFA from a regular expression

[2025-02-07 23:33] Heaven: [replying to pinefin: "c++"]
i love you

[2025-02-07 23:33] Heaven: why do i have the hardware role

[2025-02-07 23:34] pinefin: [replying to contificate: "after that, compute a DFA from a regular expressio..."]
it’s ok i’ll just pull in boost for all of this

[2025-02-07 23:34] pinefin: cause everyone knows boost is the best

[2025-02-07 23:34] contificate: depressing

[2025-02-07 23:34] contificate: can't wait for people who enjoy computer programming to join this server

[2025-02-07 23:39] pinefin: 😭😭 nah but we don’t have a use for complex algorithms like those you listed

[2025-02-07 23:40] pinefin: i’m planning to stay in automotive industry for a long time

[2025-02-07 23:41] contificate: "complex"

[2025-02-07 23:41] contificate: no use? you don't know that

[2025-02-07 23:41] contificate: for fun is reason enough

[2025-02-08 01:05] Heaven: 
[Attachments: IMG_5041.png]

[2025-02-08 01:05] Heaven: I am suffering from success

[2025-02-08 01:06] Heaven: (i made 0 progress in my projects in the last couple days because i fucked up my whole project)

[2025-02-08 01:26] James: the amount of time you guys spend talking...

[2025-02-08 01:26] James: crazy really

[2025-02-08 01:47] Heaven: these are my first messages here >:(

[2025-02-08 05:39] Torph: [replying to pinefin: "c++ is filled with webdevs that glaze the language..."]
thats pretty funny
honestly i havent run into any people like that yet

[2025-02-08 11:15] daax: [replying to James: "the amount of time you guys spend talking..."]
I think that’s what chats are for.

[2025-02-08 16:49] Timmy: https://github.com/can1357/linux-pe

you can probably condense this into one file rather easily, but sure why you'd need it in a single hpp file tho.
[Embed: GitHub - can1357/linux-pe: COFF and Portable Executable format desc...]
COFF and Portable Executable format described using standard C++ with no dependencies. - can1357/linux-pe

[2025-02-08 18:56] mrexodia: [replying to Timmy: "https://github.com/can1357/linux-pe

you can proba..."]
https://github.com/Felerius/cpp-amalgamate
[Embed: GitHub - Felerius/cpp-amalgamate: cpp-amalgamate recursively combin...]
cpp-amalgamate recursively combines C++ source files and the headers they include into a single output file - Felerius/cpp-amalgamate

[2025-02-08 18:56] mrexodia: I use this for phnt, works fine

[2025-02-09 00:01] Deleted User: I would think that passing over the encrypted text with the key would restore it back into plaintext, cause symmetric encryption and yada yada

is that not how it works?
(I'm new to cryptography, I apologize)
[Attachments: rc4_fuckery.PNG]

[2025-02-09 00:21] 5pider: [replying to Deleted User: "I would think that passing over the encrypted text..."]
first of all. Both the key and data variables are sizes that does not fit the real size of the key and data. (key being size of 128 bytes and data being size of 512 bytes)
How did you encrypted the data and are you sure you copied the encrypted data correctly ? instead of using a string representation u should rather use a byte array representation (`char data[] = { 0xFF, 0xFF, 0xFF, ... }`). Doing size of on the data will return 512 which is not the real encrypted data size. instead remove the fixed size and leave it emtpy for the compiler to allocate enough memory for you. 
As long as you used the right key to encrypt the data then it should be fine but it would help if u were to share how you encrypted the data (if it was a python script, another C code, etc.)

[2025-02-09 00:25] Deleted User: ..this is the source..
[Attachments: og_code.PNG]

[2025-02-09 00:25] 5pider: do u mind sharing the link to it directly

[2025-02-09 00:26] Deleted User: https://github.com/anthonywei/rc4/blob/master/c%2B%2B/rc4.cpp
[Embed: rc4/c++/rc4.cpp at master · anthonywei/rc4]
rc4 java c++ python php . Contribute to anthonywei/rc4 development by creating an account on GitHub.

[2025-02-09 00:26] 5pider: [replying to Deleted User: "..this is the source.."]
you do it diff to the source

[2025-02-09 00:26] 5pider: instead of sizeof use strlen

[2025-02-09 00:26] 5pider: it would make more sense then

[2025-02-09 00:27] 5pider: because the rc4 implementaiton will then isntead only try to encrypt the actual string and not the entire 512 bytes buffer

[2025-02-09 00:27] 5pider: uhh

[2025-02-09 00:29] Deleted User: also I did remove the fixed size, but then the strlen func screamed at me for not being compatible with a param of const char*

[2025-02-09 00:29] 5pider: oh makes more sense

[2025-02-09 02:00] Deleted User: [replying to 5pider: "first of all. Both the key and data variables are ..."]
well
[Attachments: rc4_fuckery2.PNG]

[2025-02-09 02:34] naci: [replying to Deleted User: "well"]
do `{0xXX ...` instead

[2025-02-09 02:48] Deleted User: no commas?

[2025-02-09 02:48] Deleted User: or did you mean smth else

[2025-02-09 03:06] Deleted User: [replying to naci: "do `{0xXX ...` instead"]
still getting zalgo text

[2025-02-09 03:08] naci: yes, you still use commas

[2025-02-09 03:09] naci: are you trying to decrypt the data that you encrypted ?

[2025-02-09 03:10] naci: if you didnt encrypt it i would make sure that it was encrypted rc4, not a modified version or another algo and i would make sure the key and data is valid

[2025-02-09 03:12] Deleted User: [replying to naci: "are you trying to decrypt the data that you encryp..."]
yes
all I'm doing is decrypting the data using the same key, which is supposed to be how rc4 works, but for some reason it's still giving me an encrypted version <:nomore:927764940276772925>

[2025-02-09 03:17] Deleted User: 
[Attachments: works.PNG]

[2025-02-09 03:17] Deleted User: 
[Attachments: doesnt_work.PNG]

[2025-02-09 03:17] Deleted User: why does the first work but not the second <:mmmm:904523247205351454>

[2025-02-09 03:23] naci: `char data[] ={ 0x62, 0x69, ...`

[2025-02-09 03:24] naci: you are encrypting the string of hex representation of the bytes

[2025-02-09 03:27] Deleted User: I thought that since it was already encrypted, passing the key over it would decrypt it back to text format

[2025-02-09 03:27] Deleted User: well shit

[2025-02-09 03:29] Deleted User: but like
[Attachments: xaff1.PNG, xaff2.PNG]

[2025-02-09 03:30] Deleted User: ..I think I need a break

[2025-02-09 03:40] naci: [replying to Deleted User: "I thought that since it was already encrypted, pas..."]
https://godbolt.org/z/3nTh6srPY

[2025-02-09 03:40] naci: 
[Attachments: image.png]

[2025-02-09 03:46] Deleted User: that's the buffer that holds the encrypted data, am I missing something

[2025-02-09 03:47] naci: no, thats the buffer that holds the original data

[2025-02-09 03:47] Deleted User: wait what

[2025-02-09 03:48] naci: you copy `data` to `buf`, encrypt `data` and print the untouched `buf`

[2025-02-09 11:48] 0xdeluks: [replying to Deleted User: ""]
my man <:gigachad:904523979249815573>

[2025-02-09 14:08] Torph: [replying to Deleted User: ""]
does `strlen` make sense for binary data? maybe use `encrypted_length` everywhere and make it `sizeof(data) - 1`

[2025-02-09 14:09] Torph: also is there a reason you're `malloc`'ing your rc4 state instead of putting it on the stack?

[2025-02-09 14:28] dullard: [replying to Torph: "also is there a reason you're `malloc`'ing your rc..."]
the paste gods told him to

[2025-02-09 14:30] Deleted User: [replying to dullard: "the paste gods told him to"]
this

[2025-02-09 14:34] 0xdeluks: [replying to Deleted User: "this"]
https://media.discordapp.net/attachments/1051988905320255509/1146537451750432778/ezgif.com-video-to-gif_2.gif

[2025-02-09 15:31] Deleted User: idk anymore
[Attachments: rc4_fuckery3.PNG]

[2025-02-09 15:33] Deleted User: that should've worked
[Attachments: rc4_fuckery4.PNG]

[2025-02-09 15:34] brymko: its bc u r retarded

[2025-02-09 15:34] emma: That's not how you create an array of hex bytes ...

[2025-02-09 15:34] Deleted User: [replying to brymko: "its bc u r retarded"]
we are already aware of this, yes

[2025-02-09 15:35] Deleted User: [replying to emma: "That's not how you create an array of hex bytes ....."]
christ

[2025-02-09 15:37] Deleted User: oh wait

[2025-02-09 15:37] daax: [replying to Deleted User: "christ"]
https://stackoverflow.com/questions/33454988/c-initialize-array-in-hexadecimal-values

uchar arr[]=“\x39\x05\x48\x8b”
[Embed: C initialize array in hexadecimal values]
I would like to initialize a 16-byte array of hexadecimal values, particularly the 0x20 (space character) value.

What is the correct way?

unsigned char a[16] = {0x20};
or 

unsigned char a[16] = {"

[2025-02-09 15:37] Deleted User: I am retarded-

[2025-02-09 15:37] Deleted User: yeah

[2025-02-09 15:37] Deleted User: jfc

[2025-02-09 15:37] Timmy: https://tenor.com/view/clown-license-clown-phantomv3x-v3x-gif-834007702285978431

[2025-02-09 15:37] Deleted User: I AM SORRY

[2025-02-09 16:07] Deleted User: ..I just had to format it correctly
[Attachments: rc4_noFuckery.PNG]

[2025-02-09 16:07] 0xdeluks: <:topkek:904522829616263178>

[2025-02-09 16:09] Heaven: [replying to Deleted User: "idk anymore"]
well, here we go again

[2025-02-09 16:09] Deleted User: no NO

[2025-02-09 16:09] Deleted User: it's fixed now

[2025-02-09 16:09] Heaven: did the same mistake like 2 days ago

[2025-02-09 16:09] Heaven: HAHAA

[2025-02-09 16:10] Deleted User: tfw cyberbullying is necessary

[2025-02-09 16:10] 0xdeluks: shit this is almost as good as the file operations incident

[2025-02-09 16:10] Deleted User: please

[2025-02-09 16:10] Deleted User: PLEASE

[2025-02-09 16:10] 0xdeluks: thank you geist, made me laugh today :)

[2025-02-09 16:11] Heaven: https://pics.milf.charity/g1uPq
[Embed: g1uPq | 76.31KB]
Meow meow :3

[2025-02-09 16:11] 0xdeluks: LMAO THE LINK

[2025-02-09 16:12] Heaven: https://tenor.com/view/cruz-mewing-potato-cruz-roblox-mewing-man-face-gif-9930889414482193004

[2025-02-09 16:21] dullard: https://media.discordapp.net/attachments/1118988563577577574/1150888584778371153/YiEtFVn.gif

[2025-02-09 16:28] Deleted User: https://tenor.com/view/skeleton-gif-26826812

[2025-02-09 18:15] pinefin: geistmaster i think college is right for you

[2025-02-09 18:15] pinefin: maybe webdev will suit you

[2025-02-09 18:15] Deleted User: My WGU classes start in May

[2025-02-09 18:15] Deleted User: [replying to pinefin: "maybe webdev will suit you"]
no to this though

[2025-02-09 18:15] pinefin: what’s that stand for

[2025-02-09 18:15] Deleted User: I hate Frontend with a passion-

[2025-02-09 18:15] pinefin: women getting unified?

[2025-02-09 18:15] Deleted User: LMAO

[2025-02-09 18:15] Deleted User: Western Governors University

[2025-02-09 18:16] pinefin: well this is a basic memory operation my good sir

[2025-02-09 18:16] Deleted User: I just forgot how Hex format looked 😭

[2025-02-09 18:17] Deleted User: I literally forgot once that strings are just an array of char characters

[2025-02-09 18:35] Deleted User: 
[Attachments: for_distinguished_customer.png]

[2025-02-09 21:08] dlima: Geistmeister when he sees the ReadFile function

[2025-02-09 21:10] dlima: Where'd your twitter go? did you delete it?

[2025-02-09 21:27] Deleted User: [replying to dlima: "Where'd your twitter go? did you delete it?"]
deactivated 
might return next month tbh

[2025-02-09 21:28] Deleted User: [replying to dlima: "Geistmeister when he sees the ReadFile function"]
WriteFile
it was WriteFile

[2025-02-09 21:28] Deleted User: I'm really never gonna live that down, huh

[2025-02-09 21:37] Deleted User: too many find pattern calls

[2025-02-09 21:38] avx: wtf did i just look at

[2025-02-09 21:38] dlima: [replying to Deleted User: "deactivated 
might return next month tbh"]
How am I supposed to troll you on twitter now

[2025-02-09 21:38] dlima: I cant believe this 😢

[2025-02-09 21:39] dlima: [replying to Deleted User: "I'm really never gonna live that down, huh"]
Real OGs remember

[2025-02-09 21:58] Deleted User: [replying to avx: "wtf did i just look at"]
absolutely nothing
it has been resolved

[2025-02-09 21:58] Chocolate Milk: [replying to dlima: "I cant believe this 😢"]
Hmm

[2025-02-09 21:58] Chocolate Milk: btw Tetsuo is a scammer

[2025-02-09 21:59] Chocolate Milk: lol

[2025-02-09 21:59] Deleted User: [replying to Chocolate Milk: "btw Tetsuo is a scammer"]
we all know this

[2025-02-09 21:59] Chocolate Milk: [replying to Deleted User: "we all know this"]
wait what happend?

[2025-02-09 21:59] Deleted User: also we should probably take this to <#835646666858168320>

[2025-02-09 21:59] Chocolate Milk: bet

[2025-02-09 22:01] dlima: [replying to Chocolate Milk: "btw Tetsuo is a scammer"]
Yeah and the sky is blue, the grass is green

[2025-02-09 22:01] dlima: Tell me something i dont know

[2025-02-09 22:01] dlima: That dude turned into a crypto shill, sad really

[2025-02-09 22:05] avx: [replying to Deleted User: "absolutely nothing
it has been resolved"]
delusional take

[2025-02-09 22:12] Torph: [replying to Deleted User: "..I just had to format it correctly"]
you don't need backslashes, you can do `char data[] = 0xD9, 0x9A, 0x5E, 0xB6, 0x66, 0xDE;`

[2025-02-09 22:14] Torph: also you can do `struct rc4_state s = {0};` to avoid the malloc and guarantee it initializes to all-zeroes, then pass `&s` instead of `s` to the functions

[2025-02-09 22:16] Deleted User: noted

[2025-02-09 22:48] Xits: Is there a thread safe way to swap a pointer that calls into a code block. I’ll try to illustrate the problem 

External code (thread 1):
mov rax, mycodeblock 
jmp rax 

My code block (thread 1):
blah blah
ret 

Deallocator (thread 2):
this code frees the memory used by “my code block” and swaps the pointer back to the original function.

It’s a simple problem but I can’t think of a solution. I can not modify external code in any way

[2025-02-09 22:51] Xits: The reason I want this is because “My code block” is a thunk stub so I can use c++ lambdas. But the destruction of the object holding the thunk has a race condition if it’s hooking the code of another thread

[2025-02-09 23:20] vendor: [replying to Xits: "Is there a thread safe way to swap a pointer that ..."]
do it like detours, suspend all other threads - check their RIP and stack to ensure none are in the region, revert patch, flush cache and resume them.

[2025-02-09 23:23] vendor: alternatively, if external code is single threaded and no CET then use a rop chain to deallocate and revert on ret

[2025-02-09 23:45] Xits: [replying to vendor: "do it like detours, suspend all other threads - ch..."]
Wouldnt that not work if I catch it after mov rax, mycodeblock?

[2025-02-09 23:45] Xits: [replying to vendor: "alternatively, if external code is single threaded..."]
That’s a good idea though idk why I didn’t think of that. It is single threaded