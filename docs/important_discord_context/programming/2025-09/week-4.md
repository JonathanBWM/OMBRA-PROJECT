# September 2025 - Week 4
# Channel: #programming
# Messages: 83

[2025-09-22 00:14] Addison: Once again I am committing violence against the Rust type system

[2025-09-22 00:14] Addison: https://github.com/addisoncrump/arb-ea/blob/main/tests/moors.rs#L61-L96
[Embed: arb-ea/tests/moors.rs at main · addisoncrump/arb-ea]
Demo of doing EA with tuple lists for heterogeneous output ranges - addisoncrump/arb-ea

[2025-09-22 00:17] Addison: And the link to the actual shenanigans: https://github.com/addisoncrump/arb-ea/blob/main/src/tuples.rs
[Embed: arb-ea/src/tuples.rs at main · addisoncrump/arb-ea]
Demo of doing EA with tuple lists for heterogeneous output ranges - addisoncrump/arb-ea

[2025-09-22 05:30] brymko: [replying to Timmy: "What kind of unlucky monstrocity did I run into he..."]
what is the thing with requires(T). Is it like a Sized in rust ?

[2025-09-22 05:38] the horse: void is an incomplete type <@83203731226628096>

[2025-09-22 05:39] the horse: I think the first scenario is pattern matched for compatibility

[2025-09-22 05:41] the horse: the outer requires is unnecessary in this case, and I think specifying the generic argument prevents the pattern-matched special scenario

[2025-09-22 05:41] the horse: syntactically both are correct

[2025-09-22 05:42] the horse: actually, clang tells you this already:

[2025-09-22 05:42] the horse: ```cpp
<source>:13:32: note: because '' would be invalid: argument may not have 'void' type
   13 | concept is_void_no = requires(T) { requires std::is_same_v<T, void>; };
      |                                ^
1 error generated.
```

[2025-09-22 10:18] Timmy: [replying to brymko: "what is the thing with requires(T). Is it like a S..."]
is kind of like a declval from my understanding

[2025-09-22 10:27] Timmy: [replying to the horse: "actually, clang tells you this already:"]
I saw it from the beginning, it somehow just tripped me up more.

[2025-09-22 11:17] moneytherapy: does windows provide there own version of fseek? except for _fseeki64

[2025-09-22 11:18] the horse: SetFilePointer?

[2025-09-22 11:19] the horse: https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-setfilepointer
```
You can use SetFilePointer to determine the length of a file. To do this, use FILE_END for dwMoveMethod and seek to location zero. The file offset returned is the length of the file. However, this practice can have unintended side effects, for example, failure to save the current file pointer so that the program can return to that location. It is best to use GetFileSize instead.
```
[Embed: SetFilePointer function (fileapi.h) - Win32 apps]
Moves the file pointer of the specified file. (SetFilePointer)

[2025-09-25 18:18] sync: Anyone know a place where ntoskrnl pdbs are cached?

Seems this PDB got removed from msdn symbol server:
https://msdl.microsoft.com/download/symbols/ntkrnlmp.pdb/FA409C97DF82668C9509C365F6DD28FD1/ntkrnlmp.pdb

[2025-09-25 18:38] lukiuzzz: [replying to sync: "Anyone know a place where ntoskrnl pdbs are cached..."]
Try on Archive.org or OSR

[2025-09-25 21:40] twopic: 
[Attachments: Screenshot_20250829-123319.png]

[2025-09-25 21:40] twopic: <@303272276441169921> <:anime_kmsfml:1250506885162078288>

[2025-09-25 21:43] twopic: Maybe I should work at Intel

[2025-09-26 02:12] iris8721: does he actually work for intel?

[2025-09-26 08:17] twopic: Yes sadly

[2025-09-26 08:21] the horse: isnt he a high school/uni intern

[2025-09-26 08:32] Brit: 100% intern

[2025-09-26 09:01] twopic: even for an intern i feel like you should know that 6 bit operands don't work

[2025-09-26 09:01] twopic: I learned the basics of machine code inside my computer architecture class.

[2025-09-26 10:04] the horse: trit-based architecture is needed 🙏

[2025-09-26 10:05] Brit: please no

[2025-09-26 10:05] the horse: -1, 0, 1 supremacy

[2025-09-26 10:06] the horse: once i get filthy super-rich i'll devote a quarter of my wealth towards ternary architectures

[2025-09-26 10:06] Brit: 
[Attachments: image.png]

[2025-09-26 10:06] Brit: to quote

[2025-09-26 10:06] Brit: All of this is nonsense

[2025-09-26 10:06] Brit: (Peirce inventor of triadic logic)

[2025-09-26 10:08] the horse: https://www.ternary-computing.com/ i want this cpu.

[2025-09-26 13:45] pygrum: [replying to the horse: "once i get filthy super-rich i'll devote a quarter..."]
Sentences never uttered since the dawn of humanity

[2025-09-26 13:46] the horse: unfortunately the prophet of this message is the god channel in TempleOS

[2025-09-26 13:47] Brit: https://cyber.autis.moe/WisaTuDI80.png

[2025-09-26 13:47] the horse: gibberish

[2025-09-26 13:48] the horse: 1+1=2

[2025-09-26 13:48] the horse: 1+1+1=3

[2025-09-26 13:48] the horse: problem?

[2025-09-26 13:48] Brit: All this is mighty close to nonsense

[2025-09-26 13:48] the horse: the constprop horse

[2025-09-26 13:48] the horse: ok
[Attachments: image.png]

[2025-09-26 13:49] Brit: the use def chain crawling horse

[2025-09-26 13:50] Brit: the value set analysis horse

[2025-09-26 13:50] the horse: they can't understand
[Attachments: image.png]

[2025-09-26 13:52] the horse: i will gatekeep warbird devirt next year

[2025-09-26 15:39] pinefin: [replying to Brit: "https://cyber.autis.moe/WisaTuDI80.png"]
-# off topic but hats off to you for having legible handwriting

[2025-09-26 15:40] Brit: not my handwriting

[2025-09-26 15:40] Brit: you would weep

[2025-09-26 15:40] Brit: were you to see my notes

[2025-09-26 15:40] Brit: my high school teachers all hated me and told me that the only viable career path would have been MD

[2025-09-26 15:41] the horse: prove them wrong and become a reddit moderator

[2025-09-26 15:41] Brit: that's a screenshot of Peirce's notes on triadic logic

[2025-09-26 15:41] Brit: the guy who invented the concept

[2025-09-26 15:42] pinefin: oh lmfao

[2025-09-26 15:42] pinefin: i have terrible handwriting and i probably need to take a class on it at some point. i dont even know cursive.

[2025-09-26 15:43] pinefin: even though im not in college. but i dont even have a signature or anything

[2025-09-26 15:43] Brit: okay I was not that cooked we had mandatory cursive back in my days

[2025-09-26 15:43] Brit: 👴🏻

[2025-09-26 17:32] pygrum: Pen license

[2025-09-26 17:51] iPower: [replying to Brit: "okay I was not that cooked we had mandatory cursiv..."]
i thought that a thing only here

[2025-09-26 17:52] iPower: all children had mandatory cursive. not sure how it is now

[2025-09-26 18:07] Xyrem: same here lol

[2025-09-26 18:26] the horse: same

[2025-09-26 19:50] daax: [replying to iPower: "all children had mandatory cursive. not sure how i..."]
same

[2025-09-26 19:50] daax: but rarely use it lol

[2025-09-26 19:50] daax: except when something requires "signature"

[2025-09-26 19:54] koyz: [replying to daax: "except when something requires "signature""]
your signature is cursive? what da fock

[2025-09-26 20:05] avx: very distinguished

[2025-09-26 21:41] daax: [replying to koyz: "your signature is cursive? what da fock"]
that's just how we were taught to do it

[2025-09-27 02:00] dlima: That sounds like an easy way to come up with a signature

[2025-09-27 02:01] dlima: We were never taught cursive so I had to make up some random bullshit to be my signature lol

[2025-09-27 02:08] blob27: cursive is the default here and i still made up random bullshit to be my signature

[2025-09-27 20:46] c0z: [replying to Brit: "my high school teachers all hated me and told me t..."]
Got to love it when you intentionally write in different styles each time you put the pencil/pen down. All my English teachers loved me too

[2025-09-27 21:03] valium: [replying to blob27: "cursive is the default here and i still made up ra..."]
Lol same

[2025-09-28 12:52] Xits: does anyone know what special magic is done to llvm jump tables that registers them with a MachineFunction instance.
I register a global like
```
auto *baddr = BlockAddress::get(BB);
  auto *GV =
      new GlobalVariable(M, baddr->getType(), true, GlobalValue::WeakAnyLinkage, baddr,
                         metadata + "-stopDCE");
```
If branch folding occurs it does not update the global to point to the new basic block but it does update the jump table associated with the block

[2025-09-28 12:53] Xits: I thought that llvm had no notion of a jump table at the IR level but it clearly has some way to lower them for when they are modified during codegen passes

[2025-09-28 13:01] Xits: oh there's a switch instruction 🤦‍♂️

[2025-09-28 13:04] Xits: though it's still weird it doesn't correctly update globals that reference a basic block

[2025-09-28 23:24] daax: [replying to Xits: "does anyone know what special magic is done to llv..."]
<@835638356624801793>