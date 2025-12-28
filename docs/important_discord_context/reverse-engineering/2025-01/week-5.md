# January 2025 - Week 5
# Channel: #reverse-engineering
# Messages: 17

[2025-01-27 21:57] segmentationfault: 
[Attachments: GiVBey2WQAA3Uj2.png]

[2025-01-27 23:20] Torph: sending this to my ARM enjoyer friend

[2025-01-28 04:44] elf: [replying to segmentationfault: ""]
arm assembly should kill itself

[2025-01-28 04:44] elf: x86/64 is where its at

[2025-01-28 13:26] avx: https://tenor.com/view/steve-harvey-family-feud-kill-steve-harvey-kill-family-feud-kill-gif-21764693

[2025-01-28 14:38] :): For Linux kernel exploitation, the msg_msg header is 48 bytes. Does that mean if I have a vulnerable object:

```
struct VulnerableObject {
    char header[48];
    void (*fn)(void);
};
```
Would sending a message like:

```
struct my_msg {
    long int mytype;
    char mybuf[8];
};
```

Suppose I have a UAF scenario where I invoke `VulnerableObject.fn` from an Ioctl If I spray the slab with messages like

```
struct my_msg m = { 1, <someaddress> };
```
And then spray `m`, is that guaranteed to work? Will my address be wrong when I spray msg_msg? What is wrong with this approach, if any? I’m on Linux kernel 5.4 FYI.

I’m worried about alignment and want to ensure that `m.mbuf` is aligned with `VulnerableObject.fn` so that I don’t get a see fault because my address 0x11223344556677<garbage> instead of `0x0011223344556677` (ie, the right aligment).

[2025-01-29 02:24] James: [replying to elf: "x86/64 is where its at"]
it is x86...

[2025-01-29 02:25] James: [replying to Torph: "sending this to my ARM enjoyer friend"]
it is x86.

[2025-01-29 02:39] Torph: <:kekw:904522300257345566>  oh you right
she'll like it anyway she's also doing x86

[2025-01-29 02:43] elf: [replying to James: "it is x86..."]
i meant to be replying to torph

[2025-01-29 02:43] avx: https://cdn.discordapp.com/attachments/1259084292915466281/1314816967076745266/togif.gif

[2025-01-29 02:43] avx: burn it all to the ground

[2025-01-29 17:10] Icky Dicky: [replying to avx: "https://cdn.discordapp.com/attachments/12590842929..."]
LOL

[2025-01-29 17:10] Icky Dicky: I-

[2025-01-29 17:10] Icky Dicky: https://cdn.discordapp.com/attachments/1259084292915466281/1314816967076745266/togif.gif

[2025-01-30 00:11] dullard: 
[Attachments: togif.gif]

[2025-01-30 14:17] Mysterio: https://cdn.discordapp.com/attachments/835635446838067210/1334315135845797899/togif.gif?ex=679cbe0e&is=679b6c8e&hm=caf88f2eb4204db0b0f8908272150f6aba433267def4445f7fc92a12354e8611&