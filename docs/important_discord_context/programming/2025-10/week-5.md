# October 2025 - Week 5
# Channel: #programming
# Messages: 51

[2025-10-27 14:05] noahsx: the ideal way for performance is to have them at 2mb or even 1gb unless you need to change a singular range.

e.g. you have them 2mb mapped and you need a 4kb pte for some reason say to change the pfn.
You then split that 2mb pde to 4kb entries

Ideally you have them mapped as large as possible to avoid tlb misses

[2025-10-27 14:05] noahsx: [replying to noahsx: "the ideal way for performance is to have them at 2..."]
and in the example if you no longer need 4kb granularity in that range (say, a page is no longer being monitored), then you merge the entries back

[2025-10-27 14:06] noahsx: larger mappings can be cached better

[2025-10-27 14:26] noahsx: https://forum.osdev.org/viewtopic.php?t=57409

[2025-10-27 14:26] noahsx: check this post out

[2025-10-27 14:26] noahsx: especially with e-cores enabled

[2025-10-27 14:26] noahsx: the cache gets flushed quite fast

[2025-10-27 14:26] noahsx: so the performance decrease with 4kb mappings is a lot more noticable

[2025-10-27 14:27] noahsx: ```
Code that executes on any E-core and translates to a 4KiB (2MiB pages split into 512 4KiB pages) in the EPT paging structure runs very slow. It is most noticeable when playing games.

The same code runs perfectly fast (300+ FPS in Counter-Strike 2) on an old CPU (8700K) however when trying it on the 14700KF performance drops very hard (90 FPS to a maximum of 200 FPS).
```

[2025-10-27 14:27] noahsx: a post quote

[2025-10-27 14:28] noahsx: yeah just have everything in large pages unless you need a single pte for some reason

[2025-10-27 14:28] noahsx: so have it in large pages

[2025-10-27 14:28] noahsx: and say you need to hide a single 4kb range or hook it

[2025-10-27 14:28] noahsx: then split that range

[2025-10-27 14:28] noahsx: no problem

[2025-10-27 14:28] noahsx: have a great day

[2025-10-27 23:46] Addison: Is the convex hull of boolean dimensions just the set of observed combinations of boolean values?

[2025-10-27 23:47] Addison: like is there a set of unique points in boolean dimensions for which the convex hull is not the set itself?

[2025-10-27 23:48] Addison: It is at least in two and three dimensions so I assume it continues for higher dimensions

[2025-10-28 07:06] Addison: "Any set of boolean points is convex" according to my mathy friend 🫡

[2025-10-29 01:18] UJ: Is anyone aware of a good way to boot efi runtime drivers via pxe? Its possible if put the runtime driver as shellcode in a loader efi app and boot the efi application via pxe but this method is skibidi toilet for lack of a better word. 

booting efi shell via pxe and doing "load runtimedriver.efi" obviously wouldn't work because that driver isn't local to the fs.

[2025-10-29 13:25] abu: [replying to Addison: ""Any set of boolean points is convex" according to..."]
What sort of programming things are you doing that require Convex Hulls? If you don't mind sharing that is

[2025-10-29 13:58] Addison: Bah just trying to think of different ways to process a very large list of boolean values

[2025-10-29 14:20] Brit: how large are we talking?

[2025-10-29 16:11] Addison: [replying to Brit: "how large are we talking?"]
Hundreds to tens of thousands potentially

[2025-10-29 16:11] Addison: And millions of such lists

[2025-10-29 16:13] Brit: no cute idea then, were it more tractable sizes chess engines pack board states (occupied) into 64bit words which allows them to do some cute stuff but nothing quite so large

[2025-10-29 18:03] oopsies: [replying to UJ: "Is anyone aware of a good way to boot efi runtime ..."]
unless you replace the pei and dxe drivers (and deal with pk/kek), the PCRs will always be extended

[2025-10-29 18:07] UJ: Not related to cheating. just want to make developing this easier instead of unplugging, copying file, plugging in each build to improve the inner dev loop. 

anyways the solution here is to just boot uefishell via pxe and use the ftp or http protocol to download my driver i think. doesn't look like uefishell comes with these commands built in anymore and the uefishell_full.efi isnt being build as part of new releases as well. ill just write a quick one to do this (the pxe server i use also has a httpd support built in as well).

[2025-10-29 22:32] cleb: i need someone whos good in python with reqs and stuff like that dm me

[2025-10-29 22:35] Addison: [replying to cleb: "i need someone whos good in python with reqs and s..."]
you can discuss it here

[2025-10-29 22:42] the horse: What's a nice way to get the primary user's SID if you're running from NT_AUTHORITY? WTS is unavailable.
Currently I do a ghetto token clone from explorer.exe

[2025-10-30 06:12] ImagineHaxing: [replying to the horse: "What's a nice way to get the primary user's SID if..."]
I think the only way is by getting their token

[2025-10-30 10:33] Matti: who even is the "primary user"

[2025-10-30 10:35] Matti: if you've got multiple users logged in, chances are even your explorer token clone can get the wrong explorer.exe

[2025-10-30 11:28] R1perXNX: Anyone running qiling IDA plugin on Windows platform?

[2025-10-30 19:22] Cozydabest: .

[2025-10-30 19:43] Cozydabest: One of my friends got ratted via a software server etc
The person who ratted him is asking for money in return, How do I bypass or get around this without paying

Someone in the server opposing as support and

And then tried to "help" but sends him a download still opposing as support and ratted customer and asking for 80 for removal of malware

[2025-10-30 19:44] plpg: that's a cute text mode screen, i wonder if its real text mode or just some graphics emulating it

[2025-10-30 19:44] plpg: as for bypassing it, make a backup of the drive, and try to identify what is the RAT

[2025-10-30 19:45] plpg: if they just used some malware builder there are chances there's a decryptor somewhere

[2025-10-30 19:45] plpg: if not, restore from backups (if you have them), or just pay the ransom and hope you get the decryption key

[2025-10-30 20:01] Cozydabest: Oh thank you

[2025-10-30 20:30] ImagineHaxing: I doubt paying will give the key

[2025-10-30 20:31] plpg: i doubt too, but then not giving the key makes the word spread and new victims stop paying ransoms

[2025-10-30 20:31] plpg: hard to say

[2025-10-30 20:32] ImagineHaxing: Isnt that the nopetya screen

[2025-10-30 20:32] ImagineHaxing: The flashing one

[2025-10-30 21:05] Tequila: [replying to ImagineHaxing: "Isnt that the nopetya screen"]
yes but nopeta always put their links, eg. visit [there website here followed by installation key for coin] in their screen from what I have read in the past. I think its either some copycat trying to grab some money or just fake all together (more likely) tbh

[2025-10-30 21:07] ImagineHaxing: [replying to Tequila: "yes but nopeta always put their links, eg. visit [..."]
Maybe a copycat

[2025-10-30 22:48] rin: [replying to plpg: "i doubt too, but then not giving the key makes the..."]
They are attempting to extort someone for the equivalent of 1 shift at McDonald's. I don't think they put that much thought into their operation.