# February 2024 - Week 4
# Channel: #application-security
# Messages: 83

[2024-02-25 06:01] snowua: 
[Attachments: image.png]

[2024-02-25 06:01] snowua: Does anyone know if IDA has some kind of symbol filename limit. Cant seem to fit in any better 😦

[2024-02-25 06:04] snowua: 
[Attachments: image.png]

[2024-02-25 11:08] Brit: <@839216728008687666> the code that spawns that prompt is in pdb/64.dll I took a quick peek and it doesn't seem like there's a limit (but also it's qt garbage so I might not be aware of some intrinsic limit)

[2024-02-25 11:39] Brit: nvm

[2024-02-25 11:39] Brit: should be 260chars at most

[2024-02-25 12:09] qwerty1423: [replying to Brit: "<@839216728008687666> the code that spawns that pr..."]
one quick question
isn't there already a file name length limit on windows (MAX_PATH) which limits it by 255 characters?
i don't think it has anything to do with ida, correct me if i'm wrong.

[2024-02-25 12:10] Brit: ye

[2024-02-25 12:14] Brit: it's 260 though

[2024-02-25 12:14] Brit: MAX_PATH that is

[2024-02-25 12:14] Brit: unless you have long paths enabled

[2024-02-25 12:14] Brit: which is a thing since 1607 iirc

[2024-02-25 13:02] Timmy: wonder what is responsible for setting such an arbitrary limit

[2024-02-25 13:02] Torph: on Windows' part or IDA? my guess is probably just tradition & old code

[2024-02-25 13:12] Timmy: No windows' part

[2024-02-25 13:12] Timmy: I doubt it was some kind of arbitrary limitation in ntfs

[2024-02-25 13:12] Timmy: so it must be even stupider right

[2024-02-25 13:14] alpaca: current working dir is saved for all drives

[2024-02-25 13:14] alpaca: and in the dos days that was a lot of memory

[2024-02-25 13:15] Timmy: You'd think that if that was the case they'd have thought more about their naming right?

[2024-02-25 13:16] Timmy: C:\Users\Administrator\Documents and Settings\My Games\Microsoft Solitair\Save Files\1.tmp

[2024-02-25 13:31] Torph: 😂

[2024-02-25 13:33] Matti: I don't know the actual reason, but I'm willing to bet a fair amount of money it's related to FAT

[2024-02-25 13:33] Matti: being a shit filesystem

[2024-02-25 13:34] Matti: not necessarily DOS - though MS DOS used FAT so one follows from the other

[2024-02-25 13:34] Matti: and the 255* specifically is obviously UCHAR_MAX - sizeof(char)

[2024-02-25 13:35] diversenok: [replying to Timmy: "I doubt it was some kind of arbitrary limitation i..."]
For NTFS, each path component cannot exceed 255 chars; plus the entire on-volume path must fit into 32767 chars because it needs to fit into `UNICODE_STRING`

[2024-02-25 13:35] Matti: hmm I did knot know this part about NTFS

[2024-02-25 13:35] Matti: so you can't have a folder name exceeding 255 chars?

[2024-02-25 13:36] diversenok: 
[Attachments: image.png]

[2024-02-25 13:36] Matti: intredasting

[2024-02-25 13:36] Matti: but, not nearly as bad as the total path limit being 255

[2024-02-25 13:37] diversenok: Yeah, so the limit for total of 32k chars obviously applies to all filesystems

[2024-02-25 13:37] diversenok: Because the API cannot physically address more via `UNICODE_STRING`

[2024-02-25 13:38] Matti: yea

[2024-02-25 13:38] diversenok: And you can get some funny behavior near the limit

[2024-02-25 13:39] diversenok: Such as when the on-volume path fits but the string with appended `\Device\Harddiskvolume1\` doesn't

[2024-02-25 13:40] diversenok: So querying `FileNameInformation` info class works correctly, but querying `ObjectNameInformation` overflows

[2024-02-25 13:40] diversenok: It looks really cursed
[Attachments: 2022-09-25_Filename_overflow.png]

[2024-02-25 13:42] Matti: huh, neat

[2024-02-25 13:43] Matti: I guess ObjectNameInformation is being slightly too strict if you look at it from the FS' perspective - but the only fix I see would be returning multiple unicode strings or something <:kekw:904522300257345566>

[2024-02-25 13:43] Matti: well that class is already cursed in so many ways

[2024-02-25 13:43] diversenok: Yeah, overall it sounds like a conceptual problem

[2024-02-25 13:44] Matti: when it comes to calculating lengths and offsets of members

[2024-02-25 13:44] Matti: or is that Name**s**Information

[2024-02-25 13:44] x86matthew: [replying to Timmy: "I doubt it was some kind of arbitrary limitation i..."]
the max overall length is just a compatibility issue at this point, it came about in win9x but MAX_PATH is now hardcoded everywhere in binaries

[2024-02-25 13:44] Matti: one of those

[2024-02-25 13:44] x86matthew: so applications need to opt-in as long path aware

[2024-02-25 13:47] qwerty1423: 
[Attachments: image.png]

[2024-02-25 13:47] qwerty1423: not like this

[2024-02-25 13:47] diversenok: [replying to Matti: "or is that Name**s**Information"]
Maybe `ObjectTypesInformation`? It applies some weird alignment to type name strings

[2024-02-25 13:47] Matti: ohhh yeah

[2024-02-25 13:47] Matti: that's the one I was thinking of yeah

[2024-02-25 13:47] Matti: fucking

[2024-02-25 13:47] diversenok: https://ntdoc.m417z.com/object_types_information
[Embed: OBJECT_TYPES_INFORMATION - NtDoc]
OBJECT_TYPES_INFORMATION - NtDoc, the native NT API online documentation

[2024-02-25 13:47] Matti: cursed

[2024-02-25 13:48] Timmy: I wonder why and how MAX_PATH ever became so frequently used

[2024-02-25 13:49] diversenok: Too many buffers allocated on the stack?

[2024-02-25 13:50] diversenok: It's entirely Win32 API layer problem; Native functions don't care about these things

[2024-02-25 13:52] qwerty1423: 
[Attachments: image.png]

[2024-02-25 13:53] x86matthew: it was a documented "guaranteed" limit at the time

[2024-02-25 13:54] x86matthew: which is important for developers to know

[2024-02-25 20:41] snowua: [replying to Brit: "should be 260chars at most"]
😞😞

[2024-02-25 20:59] Matti: amazingly this limitation is from dbghelp, not even IDA's pdb plugin (which is also pretty shit at most things in general, hence me suspecting it)

[2024-02-25 20:59] Matti: e.g.
[Attachments: image.png]

[2024-02-25 21:00] Matti: though I'm only looking at the source code of IDA's MS DIA-using pdb plugin, the default PDBIDA is too shit to even use to begin with so I always compile this from source

[2024-02-25 21:00] Matti: it's possible the regular pdb64.dll has additional MAX_PATHs hardcoded somewhere

[2024-02-25 21:03] Matti: the only usage of MAX_PATH in the pdb plugin itself that I can see is part of a hack I added myself, lol

[2024-02-25 21:04] Matti: got too tired of this shit not working
[Attachments: image.png]

[2024-02-25 21:04] Matti: time to fix 🤓

[2024-02-25 21:05] Matti: I'm sure symchk.exe won't have this problem....

[2024-02-25 21:05] Matti: [replying to x86matthew: "the max overall length is just a compatibility iss..."]
so: basically it's this ^ all the way down

[2024-02-25 21:09] Torph: [replying to Matti: "though I'm only looking at the source code of IDA'..."]
what's wrong with it?

[2024-02-25 21:10] Matti: it just doesn't know nearly as much about (meaning, can't obtain the same info from) PDB files that MS DIA can

[2024-02-25 21:10] Matti: additionally it crashes on anything even slightly non-default

[2024-02-25 21:10] Brit: [replying to Matti: "amazingly this limitation is from dbghelp, not eve..."]
they have that node stuff and they read the pdb path into a 260 byte long buff

[2024-02-25 21:11] Matti: oh, see there's another reason

[2024-02-25 21:11] Matti: [replying to Matti: "additionally it crashes on anything even slightly ..."]
uh uh uh try loading chromium.dll's PDB with it

[2024-02-25 21:11] Matti: or an xbox game PDB

[2024-02-25 21:12] Matti: basically anything that isn't a win64 PE will blow up IDA

[2024-02-25 21:12] Matti: and chromium.dll uses /pdbpagesize which is too much for it to handle

[2024-02-25 21:12] Torph: oh damn

[2024-02-25 21:12] Torph: that's dumb