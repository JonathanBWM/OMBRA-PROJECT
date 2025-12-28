# November 2025 - Week 5
# Channel: #reverse-engineering
# Messages: 42

[2025-11-25 13:08] the horse: Did anyone notice that sometimes the scheduler does not attempt to load debug registers when switching context?
(primarily when yielding is done)

[2025-11-26 04:03] Possum: Any of yall know of any blog posts focused in reversing vxworks rtos?

[2025-11-26 10:22] plpg: [replying to Possum: "Any of yall know of any blog posts focused in reve..."]
This is something i wanted to do, but so far I only found a way to emulate some older vxworks kernels on qemu

[2025-11-26 10:22] plpg: Happy to bounce off ideas if you want to chat

[2025-11-26 11:13] Possum: I have some firmware im trying to go through in ghidra,  kinda just trying to figure out where to start on it.

[2025-11-26 11:32] Gestalt: [replying to Possum: "I have some firmware im trying to go through in gh..."]
do you need help?

[2025-11-26 11:37] plpg: [replying to Possum: "I have some firmware im trying to go through in gh..."]
Do you have a flash image or did you extract the executables yet?

[2025-11-26 11:44] Possum: It was a .bin with the bootloader, and then the rest of it in lzma.  I extracted the lzma data and have that on its own , thats kinda where im at now with it.

[2025-11-26 11:44] Possum: Pretty sure the bulk of it is what was in that lzma and prior to that was unpacking and uboot

[2025-11-26 12:15] plpg: VxWorks usually comes as a single file kernel linked with the user application code. The kernel does its stuff and then calls UsrAppInit (i dont remember the name exactly)

[2025-11-26 12:15] plpg: If its in elf, then you can seach for it and see if it has symbols

[2025-11-26 12:16] plpg: If its u-boot, then the default bootcmd which is saved in the u-boot binary, or in u-boot environment should give you an idea about the load adress and formats

[2025-11-26 12:17] plpg: Is it arm or some more obscure architecture?

[2025-11-26 12:33] Possum: Mips be

[2025-11-26 12:57] plpg: Oh nice

[2025-11-26 13:37] plpg: I think i have some mips vxworks kernel images i could take a look at

[2025-11-26 13:37] plpg: Do you know which version it is?

[2025-11-26 13:54] Possum: It gave me VXWORKS_0915_CH109211

[2025-11-26 13:54] Possum: Not positive that is a version number tho

[2025-11-26 14:39] plpg: Hard to say

[2025-11-26 14:46] plpg: Ping me in a few hours and ill see what strings are in my copies

[2025-11-26 14:54] Possum: 👍 thx

[2025-11-26 23:30] plpg: I think mips kernels should use either ELF or COFF format

[2025-11-26 23:34] plpg: [replying to Possum: "It gave me VXWORKS_0915_CH109211"]
That looks like it could be a version number or name, they sometimes have CVS source markers inside them

[2025-11-26 23:34] plpg: the @ .... $ stuff

[2025-11-27 01:28] Possum: Ok. Ill take a look when im back by a computer after the Holiday

[2025-11-27 11:57] plpg: Interesting(?) archive of leftovers after dumping android with Cellebrite: https://cyberplace.social/@GossiTheDog/115621064931840111
Cellebrite payload dump from a Samsung phone in this thread\: [donotsta.re/objects/482cd0c0-4…](https://donotsta.re/objects/482cd0c0-449b-4dbb-8cd4-d63fb86b6334)

I’d suggest two things

a\) people mi

[2025-11-27 11:58] plpg: I doubt there is anything of value in it  but I have not checked whats inside apart from a file list

[2025-11-27 12:21] Brit: This is why the graphene people have it right

[2025-11-27 12:22] Brit: When the phone is locked they disconnect the data lines  of the USB port in firmware and in software

[2025-11-27 12:56] plpg: Yep

[2025-11-27 14:02] luci4: [replying to Brit: "This is why the graphene people have it right"]
I will never go back from my Pixel

[2025-11-27 14:47] Brandon: One simple security feature from GrapheneOS that invalidate many kinds of brute force attack is the auto reboot after X hours locked.

[2025-11-27 14:48] Brit: Set to 18 by default which seems a tad high

[2025-11-27 14:49] Brit: Given you can get a celebrite toolkit on scene in an hour or so in most countries using them

[2025-11-27 14:50] Brandon: True, but many brute force attacks take more then one day to crack a pin. It will not help if your firmware is outdated and vulnerable.

[2025-11-28 13:19] Obvious: <@609487237331288074> revers.engineering is down, yk why?

[2025-11-28 14:16] daax: [replying to Obvious: "<@609487237331288074> revers.engineering is down, ..."]
looks like it’s up to me

[2025-11-28 14:16] daax: not loading for you?

[2025-11-28 14:19] koyz: [replying to daax: "not loading for you?"]
was also down for me on mobile, said "account suspended", works fine on PC though

[2025-11-28 14:21] daax: [replying to koyz: "was also down for me on mobile, said "account susp..."]
weird. works on mobile on my end. lmk if it persists though

[2025-11-28 18:12] Obvious: it's back now, weird