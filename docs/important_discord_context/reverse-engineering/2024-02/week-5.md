# February 2024 - Week 5
# Channel: #reverse-engineering
# Messages: 10

[2024-02-26 10:32] _mrfade_: Yo guys trying to get a better understanding of reversing strucutres

[2024-02-26 10:34] _mrfade_: Could it safe to assume that V10 here is a structure of sorts? 🤔  double clicking V10 brings me to an undefined structure
[Attachments: image.png, image.png]

[2024-02-26 18:46] qwerty1423: [replying to _mrfade_: "Could it safe to assume that V10 here is a structu..."]
need a little more details
it ain't that easy to detect an array or structure like this

[2024-02-26 18:50] Horsie: -snip- wanted to check if named pipe is for IPC or RPC. Ended up using rpcview

[2024-02-28 16:20] froj: [replying to _mrfade_: "Could it safe to assume that V10 here is a structu..."]
If it's commonly referenced with various offsets, then I'd say likely. However it could just as easily be an array or something else - as qwerty said, need more details

[2024-02-28 16:53] daax: [replying to _mrfade_: "Could it safe to assume that V10 here is a structu..."]
probably, without seeing access patterns then we can't determine for sure but most likely a structure

[2024-02-28 21:35] duereturn: anyone here good with ghidra for mips16? basically i have some instructions in mips16e2 which are not supported by ghidra
does mips16e2 have 16bit instructions? the manual only lists 32bit ones https://s3-eu-west-1.amazonaws.com/downloads-mips/documents/MD01172-2B-MIPS16e2-AFP-01.00.pdf
also in ghidra how do i search for examples of a sequence of 32 bits i'm getting no results when i search the hex in program text

[2024-02-29 01:19] Azalea: [replying to duereturn: "anyone here good with ghidra for mips16? basically..."]
implement it yourself in pcode <:topkek:904522829616263178>

[2024-02-29 23:35] duereturn: [replying to Azalea: "implement it yourself in pcode <:topkek:9045228296..."]
yeah i've been doing that so far

[2024-02-29 23:35] duereturn: i need to find out what instructions i need to add to ghidra and ida pro 
and i want a program that takes the documentation for mips16e2 and then tells me what possible ones it could be based on constraints in the documentation 
i'm about to write it now unless there is one that already exists