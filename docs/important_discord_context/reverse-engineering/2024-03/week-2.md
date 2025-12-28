# March 2024 - Week 2
# Channel: #reverse-engineering
# Messages: 131

[2024-03-04 15:12] repnezz: hey, is it possible to “skip” over a device in the device stack ?
I want to attach my filter and call not the lower device , but one below it
if so how do I get that device object pointer ?

[2024-03-04 17:36] Windy Bug: https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/nf-ntifs-iogetlowerdeviceobject
[Embed: IoGetLowerDeviceObject function (ntifs.h) - Windows drivers]
Learn more about the IoGetLowerDeviceObject function.

[2024-03-05 01:41] Random Visitor: off the top of my head, that sounds useful for ignoring edr drivers :p

[2024-03-05 08:49] Bored engineer: anyone used SEninja plugin of binary ninja knows how to use it ? https://github.com/borzacchiello/seninja
[Embed: GitHub - borzacchiello/seninja: symbolic execution plugin for binar...]
symbolic execution plugin for binary ninja. Contribute to borzacchiello/seninja development by creating an account on GitHub.

[2024-03-05 13:58] Bloombit: [replying to Bored engineer: "anyone used SEninja plugin of binary ninja knows h..."]
do you see the ui widgets? https://github.com/borzacchiello/seninja?tab=readme-ov-file#ui-widgets

[2024-03-05 13:59] Bored engineer: [replying to Bloombit: "do you see the ui widgets? https://github.com/borz..."]
Yes

[2024-03-05 13:59] Bloombit: Do you have a specific question or is it not working for you?

[2024-03-05 13:59] Bloombit: Any error messages in the log?

[2024-03-05 14:02] Bored engineer: [replying to Bloombit: "Any error messages in the log?"]
It is loading fine my query is how to use it practically on a binary there is no tutorial on it

[2024-03-06 14:38] supermoon: <@911338672392138783> https://www.youtube.com/watch?v=lay3PtTtubM
[Embed: Solving the Hex-Rays CTF using path driven symbolic execution.]
Here I demonstrate how to solve the CTF using a binary ninja plugin called SENinja. Note that this version of the plugin shown in the video is part of a non-merged pull request of the plugin's master 

[2024-03-06 14:39] Bored engineer: [replying to supermoon: "<@911338672392138783> https://www.youtube.com/watc..."]
watched understood some part of it , and also tried but kind of getting error when i am trying on certain branches

[2024-03-06 14:53] Bored engineer: more tuts will be helpful specially from the makers of it

[2024-03-06 18:07] mibho: has anyone here looked at safedisc?

[2024-03-06 18:08] mibho: i been going thru it by trace analysis + ida but its not fast approach

[2024-03-06 18:09] mibho: im doin it cus its got interesting stuff so i dont mind

[2024-03-06 18:10] mibho: but itd be cool to see if there are other "proper?" approaches

[2024-03-06 23:51] jvoisin: Tracing is unfortunately the way to go

[2024-03-07 14:36] Horsie: Can windbg dump a file (from a system crashdump) similar to x64dbg's Scylla?

[2024-03-07 14:37] Horsie: (File in question in a driver)

[2024-03-07 14:39] Horsie: I assume it is possible with `.writemem` but theres perhaps one that also performs fixups?

[2024-03-07 14:52] Matti: [replying to Horsie: "I assume it is possible with `.writemem` but there..."]
I use this <https://www.hexacorn.com/d/PEFix.pl>

[2024-03-07 14:52] Matti: yes really

[2024-03-07 14:52] Matti: at least I think that's the one

[2024-03-07 14:53] Matti: how many perl scripts to fix PE dumps can there be

[2024-03-07 14:53] Horsie: Thanks...?

[2024-03-07 14:54] Matti: ```
.writemem C:\raw.bin
perl PEFix.pl raw.bin
```

[2024-03-07 14:54] Matti: done!

[2024-03-07 14:55] Matti: it really verks

[2024-03-07 14:55] Matti: if it doesn't, it's not the one I'm using

[2024-03-07 14:56] Matti: not at home atm so I can't check

[2024-03-07 14:58] Matti: this will fix section alignments to shrink them back to file alignment

[2024-03-07 14:59] Matti: it won't rebase anything if that's what you're looking for

[2024-03-07 14:59] Matti: since the reloc dir will have been discarded, it won't be available in the raw dump normally speaking

[2024-03-07 14:59] Horsie: I'm giving it a try now

[2024-03-07 15:00] Horsie: As soon as msys installs perl

[2024-03-07 15:00] Horsie: 
[Attachments: image.png]

[2024-03-07 15:00] Horsie: Though I suspect the dump might be a little broken

[2024-03-07 15:00] Matti: this happens

[2024-03-07 15:00] Matti: depending on the filesize it might be OK

[2024-03-07 15:00] Horsie: IDA opens it *almost* alright though

[2024-03-07 15:00] Matti: I often just do `.writemem C:\bla.bin L?0x100000000`

[2024-03-07 15:01] Matti: and see how much I can read

[2024-03-07 15:01] Horsie: I see

[2024-03-07 15:03] Horsie: 
[Attachments: image.png]

[2024-03-07 15:03] Matti: [replying to Matti: "since the reloc dir will have been discarded, it w..."]
if you have the original PE file you can obviously get this too (and same for other discardable sections), but idk of any tool that will automate this

[2024-03-07 15:03] Horsie: L?x didnt make a difference as far as I can see

[2024-03-07 15:03] Horsie: [replying to Matti: "if you have the original PE file you can obviously..."]
This sucks

[2024-03-07 15:03] Matti: nah I didn't think it would

[2024-03-07 15:04] Matti: this is just easier lol

[2024-03-07 15:04] Horsie: Fair lol

[2024-03-07 15:04] Matti: for me it's usually enough to have the alignments correct

[2024-03-07 15:05] Matti: what do you need to relocate it (back?) for?

[2024-03-07 15:05] Matti: diffing?

[2024-03-07 15:06] Horsie: Yeah, I have a trace I need to load for coverage

[2024-03-07 15:06] Horsie: Ideally I was hoping that the *overall structure* (whatever I mean by that) is retained as seen in memory in windbg so I can also use ret-sync with it

[2024-03-07 15:07] Horsie: which I cant if I have discarded stuff in memory and I try to sync it up with the original file in IDA

[2024-03-07 15:07] Horsie: If that makes sense

[2024-03-07 15:07] Matti: yeah

[2024-03-07 15:07] Matti: but having section size differences is worse

[2024-03-07 15:08] Matti: in-memory sections are usually page size VA alignment, vs 512 bytes on disk

[2024-03-07 15:09] Matti: if you get the PE back to disk section alignment, you can (I think...?) also use the original PE file to put the reloc dir and other discardable things back

[2024-03-07 15:09] Matti: if the section alignments are different, you can't do anything

[2024-03-07 15:09] Horsie: I'll check on that

[2024-03-07 15:11] Matti: scylla has an option to just use the entire on-disk PE header

[2024-03-07 15:11] Matti: not sure if that's the default but it exists

[2024-03-07 15:12] Matti: I think I have it enabled

[2024-03-07 15:12] Matti: but yeah, before you can just overwrite the PE header you need the rest of the file to have the sections at the expected places

[2024-03-07 15:13] Horsie: Fixed the file with the perl script and it seems to have done a decent job, or so it seems.

[2024-03-07 15:13] Horsie: 
[Attachments: image.png]

[2024-03-07 15:13] Horsie: ^The dump

[2024-03-07 15:14] Matti: lol

[2024-03-07 15:14] Matti: could be worse given the input

[2024-03-07 15:14] Horsie: The original:

[2024-03-07 15:14] Horsie: 
[Attachments: image.png]

[2024-03-07 15:15] Matti: the way that I usually check if it worked as expected is just check the imports

[2024-03-07 15:16] Matti: they should just look like normal imports if they are by name

[2024-03-07 15:16] Horsie: Oh it has 0 imports but I thought that would be sort of expected

[2024-03-07 15:17] Matti: uhm no, well not if the .sys file does have imports

[2024-03-07 15:19] Matti: hmmm I'm pretty sure this actually is a different perl script than whatever it is I use lol

[2024-03-07 15:19] Matti: since it rebased to 0

[2024-03-07 15:19] Matti: I'll check when I'm home later

[2024-03-07 15:21] Matti: ah, found it (by filename....)

[2024-03-07 15:21] Matti: <https://github.com/kentnf/RSQuantification/blob/master/PE_align.pl>

[2024-03-07 15:22] Matti: well I think that's it anyway!

[2024-03-07 15:22] Matti: `perl PE_align.pl raw.bin fixed.sys`

[2024-03-07 15:22] Matti: ?

[2024-03-07 15:22] Matti: something like that

[2024-03-07 15:23] Horsie: [replying to Matti: "since it rebased to 0"]
That was me. And the file rebased to 0 was actually the original file, which I did to make more sense of my pre-fixup dumps

[2024-03-07 15:23] Matti: [replying to Matti: "<https://github.com/kentnf/RSQuantification/blob/m..."]
ok maybe not this one

[2024-03-07 15:23] Matti: ffs

[2024-03-07 15:24] Horsie: The 'dump' is fixed up which assumes the base to be `fffff800906fc000`, which is where it was in memory during the dump

[2024-03-07 15:24] Matti: aha ok

[2024-03-07 15:25] Matti: but, no imports? and the .sys file on disk does have them?

[2024-03-07 15:25] Horsie: Yeah, the original has a bunch

[2024-03-07 15:26] Matti: mm alright, yeah kinda hard to say, your raw input does have a LOT of fucking sections

[2024-03-07 15:26] Matti: but this is the sanity check I usually use and the perl script always makes that work for me at least

[2024-03-07 15:29] Horsie: Also, PAGE_K + myoffset have different content in both the original and the dump

[2024-03-07 15:30] Horsie: which makes little sense to me, considering at least some kind of structure/continuity(?) should be maintained intra-segments
(They at least start off with the same content then something goes bad I guess)

[2024-03-07 15:32] Matti: yeah

[2024-03-07 15:32] Matti: that is broken

[2024-03-07 15:33] Horsie: https://tenor.com/view/linus-torvalds-linus-nvidia-fuck-you-gif-19475186

[2024-03-07 15:34] Matti: nah I mean the perl script shouldn't do that no matter what

[2024-03-07 15:34] Matti: here's a DIY version if you prefer: https://rufusmbrown.github.io/docs/posts/Aligning-Dumped-PE-File-from-Memory/
[Embed: Aligning Dumped PE File from Memory]
A modern, high customizable, responsive Jekyll theme for documention with built-in search.

[2024-03-07 15:34] Horsie: Might as well

[2024-03-07 15:37] Bored engineer: any pros know how to use Seninja in binary ninja to run symbolic execution for a function in a big ass binary i am trying Seninja on this program : https://github.com/stephenbradshaw/vulnserver trying to reach function3
[Embed: GitHub - stephenbradshaw/vulnserver: Vulnerable server used for lea...]
Vulnerable server used for learning software exploitation - stephenbradshaw/vulnserver

[2024-03-07 15:37] Matti: there's also stuff like https://waleedassar.blogspot.com/2012/03/anti-dumping-part-2.html to be aware of, though idk how well this works against windbg
[Embed: Anti-Dumping - Part 2]
In this post i am shedding some light on something i have recently found which turns out to be an effective anti-dumping trick. It should w...

[2024-03-07 15:38] Matti: especially since you're not using sizeofimage

[2024-03-07 15:38] Matti: I can't find part 1 of the series, who knows, maybe that one is worse <:harold:704245193016344596>

[2024-03-07 16:02] Horsie: development: Code that lied in PAGE_K (on disk) is now in .text (in memory)

[2024-03-07 17:59] Mysterio: What x64 asm would you use to convert a rip relative:
`mov rbx, [rip+0x1337]` to absolute? FOr the least amount of bytes

[2024-03-07 18:06] Mysterio: Thinkig something like 
```
mov rbx, [rip+0]
0xdeadbeefdeadbeef
```

[2024-03-08 13:11] hxm: what this means ?
[Attachments: image.png]

[2024-03-08 13:11] hxm: also i noticed many drivers major funcs are hooked by NDIS ...

[2024-03-08 13:12] hxm: any short explanation please ?

[2024-03-08 15:37] jordan9001: [replying to hxm: "any short explanation please ?"]
You don't have to implement every major func for a driver object. Those are not filled out

[2024-03-09 00:36] Hunter: ^

[2024-03-09 00:36] Hunter: thats why they point to InvalidDeviceRequest, they arent implemented

[2024-03-09 00:37] p1ink0: [replying to Hunter: "thats why they point to InvalidDeviceRequest, they..."]
Hi

[2024-03-09 00:39] Hunter: [replying to p1ink0: "Hi"]
hi

[2024-03-09 01:36] ᲼: [replying to Hunter: "hi"]
hi

[2024-03-09 01:56] Hunter: [replying to ᲼: "hi"]
Hi

[2024-03-09 02:31] donna🤯: [replying to hxm: "what this means ?"]
before DriverEntry is called, the kernel will set all major function pointers to IopINvalidDeviceRequest, so any function pointers not set in driver entry will remain there

[2024-03-10 05:42] snowua: Does binja allow you to define an instruction parameter type for the IL pseudo

[2024-03-10 05:42] snowua: For instance I have vmread

[2024-03-10 05:42] snowua: And I want variables/constants that are used as an argument for the instruction to show as a defined enum that I imported

[2024-03-10 05:44] snowua: I know it sound stupid... but checking each vmread field manually is a pain when I can just import an enum

[2024-03-10 06:08] snowua: ah you can just change display as -> enum member silly me

[2024-03-10 10:33] Tarkev: [replying to Horsie: "https://tenor.com/view/linus-torvalds-linus-nvidia..."]
This guy is a male Karen. I swear.

[2024-03-10 10:34] Tarkev: Nvidia is bigger than him.

[2024-03-10 15:32] Windy Bug: Any cool windows / kernel related one week research idea? Im thinking P&P notify routines / reversing and rewriting some av anti wiper component but would love to hear your ideas

[2024-03-10 17:36] hxm: [replying to Windy Bug: "Any cool windows / kernel related one week researc..."]
maybe a hypervisor related thing ?