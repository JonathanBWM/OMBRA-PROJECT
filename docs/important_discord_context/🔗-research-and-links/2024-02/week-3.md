# February 2024 - Week 3
# Channel: #🔗-research-and-links
# Messages: 18

[2024-02-12 17:48] mono: https://www.msn.com/en-ca/money/other/microsoft-s-bitlocker-tpm-encryption-combo-defeated-with-a-10-raspberry-pi-and-a-bit-of-braininess/ar-BB1i1Ywg

[2024-02-13 02:06] mibho: x64dbg .trace parser <:yea:904521533727342632> 
https://github.com/mibho/x64dbgTraceReader
[Embed: GitHub - mibho/x64dbgTraceReader: easily extract, edit, or analyze ...]
easily extract, edit, or analyze data from x64dbg traces (.trace32/64) and output data back into a valid .trace file or w/e format desired.  - GitHub - mibho/x64dbgTraceReader: easily extract, edit...

[2024-02-13 03:26] root: https://www.youtube.com/watch?v=RwzIq04vd0M
[Embed: Hacking into Kernel Anti-Cheats: How cheaters bypass Faceit, ESEA a...]
Players rave and rant about the wonders of kernel level anti cheats, and how games like Valorant barely have any cheaters compared VAC secured Counter Strike servers.
But are they as good as people cl

[2024-02-14 20:11] root: https://smlx.dev/posts/goodwe-sems-protocol-teardown/
[Embed: Reverse-engineering an encrypted IoT protocol | @smlx's blog]
software, cloud, infosec, and miscellaneous other stuff.

[2024-02-15 20:40] mishap: https://github.com/aemmitt-ns/radius
[Embed: GitHub - aemmitt-ns/radius: radius2 is a fast binary emulation and ...]
radius2 is a fast binary emulation and symbolic execution framework using radare2 - aemmitt-ns/radius

[2024-02-15 23:47] mrexodia: [replying to mishap: "https://github.com/aemmitt-ns/radius"]
unfortunate its radrae

[2024-02-16 10:50] optyx: [replying to mrexodia: "unfortunate its radrae"]
needs ported to rizin

[2024-02-16 14:17] Horsie: [replying to mishap: "https://github.com/aemmitt-ns/radius"]
unfortunate its rust

[2024-02-16 20:02] nuclearfirefly: for anyone else
https://x.com/daaximus/status/1758581472607232024

pretty nifty <@609487237331288074>
[Embed: Daax (@daaximus) on X]
Decided to dust off an old draft and append another use case. Covers some fun ideas for when you don't want to use documented callback mechanisms.

https://t.co/V0G89jlX91

[2024-02-16 20:02] nuclearfirefly: <https://revers.engineering/beyond-process-and-object-callbacks-an-unconventional-method/>

[2024-02-16 20:03] nuclearfirefly: guess i could just post the direct link

[2024-02-18 08:48] 0xatul: dumping type indices is quite nifty with windbg too :p 
```
 dx -g ((nt!_OBJECT_TYPE*[69])((__int64)&nt!ObTypeIndexTable + 0x10))->Select(o => new {Name = o->Name, Index = o->Index, DumpProcedure = o->TypeInfo->DumpProcedure ? o->TypeInfo->DumpProcedure : "nullptr",OpenProcedure = o->TypeInfo->OpenProcedure ? o->TypeInfo->OpenProcedure : "nullptr",CloseProcedure = o->TypeInfo->CloseProcedure ? o->TypeInfo->CloseProcedure : "nullptr",DeleteProcedure = o->TypeInfo->DeleteProcedure ? o->TypeInfo->DeleteProcedure : "nullptr",ParseProcedure = o->TypeInfo->ParseProcedure ? o->TypeInfo->ParseProcedure : "nullptr",ParseProcedureEx = o->TypeInfo->ParseProcedureEx ? o->TypeInfo->ParseProcedureEx : "nullptr",SecurityProcedure = o->TypeInfo->SecurityProcedure ? o->TypeInfo->SecurityProcedure : "nullptr",QueryNameProcedure = o->TypeInfo->QueryNameProcedure ? o->TypeInfo->QueryNameProcedure : "nullptr",OkayToCloseProcedure = o->TypeInfo->OkayToCloseProcedure ? o->TypeInfo->OkayToCloseProcedure : "nullptr"})
```

[2024-02-18 09:37] [Janna]: idk if this has been posted but
- https://www.cryptool.org/en/cto/
- https://github.com/cristianzsh/forensictools
[Embed: CrypTool Portal]
[Embed: GitHub - cristianzsh/forensictools: Collection of forensic tools]
Collection of forensic tools. Contribute to cristianzsh/forensictools development by creating an account on GitHub.

[2024-02-18 11:51] [Janna]: added that forensic tool as well;

[2024-02-18 11:51] [Janna]: screenshot from the github itself*
(kind of similar to undetectable's  net toolkit, in <#835648484035002378> )
[Attachments: image.png]

[2024-02-18 14:35] daax: [replying to 0xatul: "dumping type indices is quite nifty with windbg to..."]
nice, I did it with the driver, was faster than scripting with windbag, but whatever works

[2024-02-18 14:36] 0xatul: no worries man.

[2024-02-18 14:36] 0xatul: I took some time to mess with windbg's `dx` expression things