# January 2025 - Week 5
# Channel: #programming
# Messages: 550

[2025-01-27 18:12] rin: does anyone know what hard links are used for in windows.

[2025-01-27 18:14] diversenok: You want a conceptual explanation or examples where?

[2025-01-27 18:15] diversenok: Examples are files in system32 that are also under WinSxS

[2025-01-27 18:15] rin: [replying to diversenok: "You want a conceptual explanation or examples wher..."]
both, I recently used the api but never seen them used in the actual system/didn't notice.

[2025-01-27 18:16] diversenok: Hardlinks are just extra names for a file that can exist in multiple directories

[2025-01-27 18:16] diversenok: Two or more filenames pointing to the same content

[2025-01-27 18:18] diversenok: You can use them to save space (not to store multiple copies)

[2025-01-27 18:18] rin: makes sense.

[2025-01-27 18:20] diversenok: Most files under system32 are hardlinks to specific revisions of these files under WinSxS. Windows update replaces the files in system32 but can keep older versions in WinSxS

[2025-01-27 18:20] rin: an interesting behavior I found with hard links is that if your malware gets detected, win defender will remove the hardlink but not the file its pointing to.

[2025-01-27 18:20] rin: assuming you executed from the hardlink

[2025-01-27 18:21] diversenok: Deletion is a per-filename operation

[2025-01-27 18:21] diversenok: Content modification is not, on the other hand

[2025-01-27 18:22] rin: [replying to diversenok: "Deletion is a per-filename operation"]
you would think that win defender would follow the chain though

[2025-01-27 18:23] diversenok: File content is reference counted. Deleting one filename that points there doesn't delete the content

[2025-01-27 18:23] rin: [replying to diversenok: "Content modification is not, on the other hand"]
wdym by this. I assume if you modify the file through a hardlink the change will propagate to the original file and other links pointing to the file?

[2025-01-27 18:24] diversenok: [replying to rin: "wdym by this. I assume if you modify the file thro..."]
Sure. Except, well, there isn't really an "original" file. There are just multiple names for the same data

[2025-01-27 18:25] diversenok: The first created name is technically distinguishable, but it has no special role

[2025-01-27 18:26] Brit: actually one of the saner parts of fs design

[2025-01-27 18:26] diversenok: Content modification is per-data operation. File deletion is a per-name operation

[2025-01-27 18:26] Brit: imagine having to keep a copy for each name

[2025-01-27 18:26] rin: [replying to diversenok: "Content modification is per-data operation. File d..."]
noted

[2025-01-27 18:27] diversenok: When you delete a file, you actually delete a name. Usually it results in content deletion (when there was only one name). But if there are more names, the content is still reachable through others

[2025-01-27 18:28] diversenok: So I think Defender's behavior makes sense

[2025-01-27 18:29] Brit: I've not looked, but that does seem insane, if you know the data's bad you might as well get rid of it, and all the aliases it might have

[2025-01-27 18:30] rin: [replying to Brit: "I've not looked, but that does seem insane, if you..."]
what I was thinking

[2025-01-27 18:31] Brit: not that it matters too too much, presumably if you're getting detected on one path, all other possible invocation paths will also get detected

[2025-01-27 18:31] diversenok: There are EDRs that overwrite the data when quarantining it. I thought Defender does both deletion + overwrite, but I might confuse it with something else

[2025-01-27 18:32] Brit: I have not looked at what happens to a file defender decides to quarantine in a hot minute, I'm just going off of what rin said

[2025-01-27 18:32] rin: [replying to Brit: "not that it matters too too much, presumably if yo..."]
provides extra room for ideas though

[2025-01-27 18:33] rin: imo

[2025-01-27 18:33] diversenok: There can be lots of filesystem attacks on security products, no doubt

[2025-01-27 18:34] Brit: I'd hit up jonas on these matters

[2025-01-27 18:34] Brit: he's always blowing my mind with his ntfs / webdav / fs based tricks

[2025-01-27 18:34] diversenok: Especially since some vendors are not even trying to make filesystem-related logic robust

[2025-01-27 18:35] diversenok: Oh yeah, he has a huge collection of tricks

[2025-01-27 18:37] rin: another thing I found recently, I don't know if its new. but if you append two dots to a unc file path the file becomes impossible to delete in explorer. this is useful for imitation because when trying to delete a running file it will also show you the signature.

[2025-01-27 18:37] rin: meaning you create OneDrive.exe.. beside OneDrive.exe and it will try to delete onedrive.exe and show you its microsoft cert.

[2025-01-27 18:38] rin: you can execute the original file using a hardlink with a different name.

[2025-01-27 18:49] diversenok: It's an issue of Win32 path normalization, yeah

[2025-01-27 18:50] diversenok: It removes final dots and spaces, etc.

[2025-01-27 18:53] diversenok: There is whole class of names representable in NT format that have no Win32 analog due to various Win32-to-NT path conversion rules

[2025-01-27 23:08] Torph: [replying to diversenok: "Sure. Except, well, there isn't really an "origina..."]
so if you created a file and a hardlink to it, you could delete the "original" and keep using the hardlink as a regular file?

[2025-01-27 23:10] Torph: [replying to rin: "another thing I found recently, I don't know if it..."]
woah, that's interesting

[2025-01-27 23:13] diversenok: [replying to Torph: "so if you created a file and a hardlink to it, you..."]
Absolutely

[2025-01-29 02:29] James: I dislike the naming scheme of the arm machine specification files. `ADD_32_addsub_imm` is quite ugly and i would prefer `ADD_WdWsp_WdWsp_imm_shift`. Is my best option to parse the assembly grammar and extract operand names that way? I've been doing this for a bit now but sometimes it produces results I don't like.

[2025-01-29 02:30] James: If someone responds, please @ me so I take notice.

[2025-01-29 02:30] James: Thank you.

[2025-01-29 02:31] James: *I have not looked at the tablegen files for arm yet... Maybe those have what I'm after?

[2025-01-29 02:48] Torph: I actually didn't know about the machine readable spec, only a few hours ago I was wishing something like that existed. that'll help me a ton

[2025-01-29 02:53] James: 
[Attachments: Screenshot_2025-01-28_at_6.52.11_PM.png]

[2025-01-29 02:53] James: The problem I've run into parsing the assembler rules is this^

[2025-01-29 02:53] 0x208D9: [replying to James: "I dislike the naming scheme of the arm machine spe..."]
this is a pain istg

[2025-01-29 02:54] James: If only the "display" attribute is used for each rule, you get identical names.

[2025-01-29 02:54] 0x208D9: [replying to James: "The problem I've run into parsing the assembler ru..."]
+1

[2025-01-29 02:54] James: So I've started manually selecting which rules to recuse into and which to just take "display"

[2025-01-29 02:54] James: Which cannot be good.....

[2025-01-29 02:54] 0x208D9: [replying to James: "If only the "display" attribute is used for each r..."]
what i did for that is get an hash for each sets but depends on what u are implementing

[2025-01-29 02:55] James: hash for each sets?

[2025-01-29 02:55] James: I'd like to derive some human readable names.

[2025-01-29 02:55] 0x208D9: oh lmao, good luck with that

[2025-01-29 02:55] James: That also don't contain any useless info like `addsub` in `ADD_32_addsub_imm`

[2025-01-29 02:55] 0x208D9: i mean u can but its a pain

[2025-01-29 02:56] 0x208D9: [replying to James: "That also don't contain any useless info like `add..."]
make an enum with the opcode respective to ur preferred name maybe?

[2025-01-29 02:57] James: I don't want to do anything manually.

[2025-01-29 02:58] 0x208D9: [replying to James: "I don't want to do anything manually."]
ask chatgpt or claude or deepseek to create one for u then fix the necessary

[2025-01-29 02:59] James: Can they really be that good? There are 4200~ unique codes.

[2025-01-29 02:59] 0x208D9: [replying to James: "Can they really be that good? There are 4200~ uniq..."]
i didnt try all of em, my usecase was different but best of luck or maybe look at how capstone does the trick

[2025-01-29 03:00] 0x208D9: [replying to James: "Can they really be that good? There are 4200~ uniq..."]
https://github.com/capstone-engine/capstone/tree/next/arch/AArch64
[Embed: capstone/arch/AArch64 at next · capstone-engine/capstone]
Capstone disassembly/disassembler framework for ARM, ARM64 (ARMv8), Alpha, BPF, Ethereum VM, HPPA, LoongArch, M68K, M680X, Mips, MOS65XX, PPC, RISC-V(rv32G/rv64G), SH, Sparc, SystemZ, TMS320C64X, T...

[2025-01-29 03:00] James: Good idea.

[2025-01-29 11:06] Yoran: I think it's better doing it manually because I actually tried generating tables from the XMLs and it's a dead end IMHO

[2025-01-29 11:07] Yoran: Like you probably could but you're better off doing it manually once and each 3 months they provide a diff pdf that summaries the diff (so there's little to change every 3 months)

[2025-01-29 11:50] Yoran: [replying to James: "Good idea."]
It's not

[2025-01-29 17:00] Torph: theres a tool that can generate a pseudocode program for decoding instructions from the XML and then another tool to translate that pseudocode to C
<https://github.com/alastairreid/mra_tools>
<https://github.com/alastairreid/asl-interpreter>

[2025-01-29 17:00] Torph: not sure how practical that is

[2025-01-29 19:03] contificate: I don't really follow what you're trying to do - can't you just parse the XML or write rewrite rules

[2025-01-29 19:04] pinefin: am i understanding this correctly? XML JIT compiler?

[2025-01-29 19:05] contificate: you are not

[2025-01-29 19:06] James: [replying to Yoran: "I think it's better doing it manually because I ac..."]
There is a nice json file

[2025-01-29 19:07] James: Which made generating the encoders/decoders given fields quite easy

[2025-01-29 19:07] James: But the names of the opcodes are so ugly

[2025-01-29 19:07] James: And mostly meaningless

[2025-01-29 19:07] James: I guess I could try to do all 4200 manually??

[2025-01-29 19:07] contificate: what are you doing

[2025-01-29 19:07] James: Trying to humanize the given names

[2025-01-29 19:08] contificate: why can't you pattern match and replace

[2025-01-29 19:08] James: It does not work

[2025-01-29 19:09] James: The rules are not described in a consistent way

[2025-01-29 19:09] James: See screenshots above of COMPACT

[2025-01-29 19:09] James: [replying to Yoran: "It's not"]
Yeah figured that out.

[2025-01-29 19:09] contificate: there will be edge cases

[2025-01-29 19:10] James: Have you done it?

[2025-01-29 19:10] James: Are we talking about the same file btw?

[2025-01-29 19:10] contificate: no, but I'm confident

[2025-01-29 19:10] James: The BSD json file

[2025-01-29 19:10] contificate: you mean this file
[Attachments: 2025-01-29-190915_424x318_scrot.png]

[2025-01-29 19:10] James: Not the xml ones

[2025-01-29 19:10] James: Sort of

[2025-01-29 19:10] James: Those are the names

[2025-01-29 19:10] contificate: the operands are also specified, right

[2025-01-29 19:11] James: Yes but that’s where it gets ugly

[2025-01-29 19:11] James: Because of how the vector width is described

[2025-01-29 19:11] James: It is I know

[2025-01-29 19:11] contificate: how is vector width a problem

[2025-01-29 19:11] contificate: can't you write a parser for it

[2025-01-29 19:12] James: The given DISPLAY attribute for the vector width parameter is <T>

[2025-01-29 19:12] James: But the widths divide the opcodes

[2025-01-29 19:12] James: So u end up with colliding names

[2025-01-29 19:12] Yoran: [replying to James: "It is I know"]
Same thing = same problems. For example they split 32/64bit to separate instructions. Hard to parse but possiable. Trade-off does not worth it imho. The json is not nice

[2025-01-29 19:12] contificate: surely 90% of the file is reasonable

[2025-01-29 19:12] Yoran: [replying to James: "But the names of the opcodes are so ugly"]
names of the opcodes?

[2025-01-29 19:13] Yoran: if you mean the mnemonices, then dont pick the xml files name but you have a good attribute for it

[2025-01-29 19:13] Yoran: those are fine

[2025-01-29 19:13] Yoran: [replying to James: "I guess I could try to do all 4200 manually??"]
Well, you first start with the base ISA and then move to the extensions

[2025-01-29 19:13] Yoran: the base is 800 instruction iirc

[2025-01-29 19:14] Yoran: a bit less because of instruction aliasing

[2025-01-29 19:14] Yoran: and also, you know, its very easy to note sf bit and generally with N bits and such the patterns are there

[2025-01-29 19:15] Yoran: We've been through worse. Its just sadge that ARM which is a huge corp cant get it right

[2025-01-29 19:15] James: Yeah that part annoys me…

[2025-01-29 19:15] Yoran: [replying to James: "The BSD json file"]
oh its missing the good mnemonics use the xmls

[2025-01-29 19:15] Yoran: well

[2025-01-29 19:16] James: No it has them

[2025-01-29 19:16] James: In the symbols

[2025-01-29 19:16] Yoran: no im probabbly just not remembring correct

[2025-01-29 19:16] Yoran: oh so its fine

[2025-01-29 19:16] James: Yeah but like

[2025-01-29 19:16] Yoran: I know lad its not trivial. Nothing we can do about it

[2025-01-29 19:16] Yoran: (and it should be)

[2025-01-29 19:16] James: I have to selectively pick to recurse into subrules

[2025-01-29 19:17] James: T__3 -> T_BS

[2025-01-29 19:17] Yoran: [replying to James: "I have to selectively pick to recurse into subrule..."]
No idea what that means

[2025-01-29 19:17] James: They both have display of just <T>

[2025-01-29 19:17] Yoran: [replying to Yoran: "No idea what that means"]
.

[2025-01-29 19:17] James: [replying to James: ""]
This

[2025-01-29 19:17] James: See the <T>

[2025-01-29 19:18] contificate: are you trying to provide instruction syntax specs with alternative notations?

[2025-01-29 19:18] Yoran: [replying to James: ""]
Deal with extenstions later. The T matters to you? iirc its just a number

[2025-01-29 19:18] James: Both the displays have just <T>

[2025-01-29 19:18] James: But one instruction restricts to first two sizes

[2025-01-29 19:18] James: Second restricts to just the second two

[2025-01-29 19:19] James: But the display for it is the same

[2025-01-29 19:19] James: So generating opcodes from the visible display creates collisions

[2025-01-29 19:19] contificate: mnemonics?

[2025-01-29 19:19] Yoran: [replying to James: "So generating opcodes from the visible display cre..."]
Dont generate opcodes from any visiable display, what?

[2025-01-29 19:20] Yoran: provide the json rq

[2025-01-29 19:22] contificate: they should just provide every instruction as an OCaml ADT

[2025-01-29 19:22] Yoran: [replying to James: ""]
Wait whats the problem? Look at bit 23. And sorry if im slow prob a language barrier

[2025-01-29 19:22] Yoran: [replying to contificate: "they should just provide every instruction as an O..."]
https://tenor.com/view/little-big-planet-suprised-bruh-what-littlebigplanet-gif-18119100423231820417

[2025-01-29 19:23] contificate: could extract such a thing from the ASL interpreter <:gigachad:904523979249815573>

[2025-01-29 19:23] contificate: but alas, I have reread this several times

[2025-01-29 19:23] contificate: and have no idea what James wants, so I will take my leave

[2025-01-29 19:28] James: [replying to Yoran: "Wait whats the problem? Look at bit 23. And sorry ..."]
Yeah I was only looking at the assembly rules, not the actual encodings as well. Sorry I’m out right now I’ll be back later.

[2025-01-29 21:03] James: ok <@317912264256520192> so a good example woulkd be COMPACT as i said above

[2025-01-29 21:04] James: there are two encodings that share exactly the same display string according to the rules.

[2025-01-29 21:04] James: The problem lies in the vector register size

[2025-01-29 21:05] James: for example, the .S and .D version has a specific encoding(with one bit different)

[2025-01-29 21:05] James: and it has a rule for it in the assembly syntax called `"T_SD"`

[2025-01-29 21:05] James: ```
"T_SD": {
      "_type": "Instruction.Rules.Choice",
      "choices": [
        {
          "_type": "Instruction.Assembly",
          "description": null,
          "symbols": [
            {
              "_type": "Instruction.Symbols.RuleReference",
              "rule_id": "T_S"
            }
          ]
        },
        {
          "_type": "Instruction.Assembly",
          "description": null,
          "symbols": [
            {
              "_type": "Instruction.Symbols.RuleReference",
              "rule_id": "T_D"
            }
          ]
        }
      ],
      "description": null,
      "display": "<T>"
    },
```

[2025-01-29 21:05] James: However as u can see from the rule, the display for it is just `<T>`

[2025-01-29 21:07] James: So one option is to just recurse into the `choices` of the rule and create my own display using the concatinations of the `"display"` attributes of the referenced children rules...

[2025-01-29 21:07] Yoran: [replying to James: "for example, the .S and .D version has a specific ..."]
Do you have a bit specifying that?

[2025-01-29 21:08] James: Sort of, bit23.

[2025-01-29 21:08] Yoran: If two instruction are the same encoding, one of them is called an ALIAS and it's fine and ARM defines those. Otherwise, you're wrong

[2025-01-29 21:08] James: It's not an alias

[2025-01-29 21:08] Yoran: [replying to James: "Sort of, bit23."]
So you need to check for that bit. Nothings wrong or am I missing something?

[2025-01-29 21:08] Yoran: [replying to James: "It's not an alias"]
Because the encoding is different

[2025-01-29 21:08] Yoran: This bit is part of the encoding

[2025-01-29 21:08] James: `

[2025-01-29 21:08] James: ```
{
                            "_type": "Instruction.Encodeset.Bits",
                            "range": {
                              "_type": "Range",
                              "start": 23,
                              "width": 1
                            },
                            "should_be_mask": {
                              "_type": "Values.Value",
                              "meaning": null,
                              "value": "'0'"
                            },
                            "value": {
                              "_type": "Values.Value",
                              "meaning": null,
                              "value": "'0'"
                            }
                          },
```

[2025-01-29 21:09] James: Yeah its not a field though, so deriving a meaningful *human readable* name from that bit being set alone isn't plausible

[2025-01-29 21:10] Yoran: [replying to James: "Yeah its not a field though, so deriving a meaning..."]
You are correct

[2025-01-29 21:10] Yoran: [replying to Yoran: "Same thing = same problems. For example they split..."]
This falls here

[2025-01-29 21:11] Yoran: They did a shitty job. Don't spend time on it.

If you know people who have done it for you (all the <@687117677512360003> Ocaml shit that I don't know) try to use it or reshape it. Else, do it manually

[2025-01-29 21:12] James: Well that's just great 😔

[2025-01-29 21:12] Yoran: [replying to James: "Well that's just great 😔"]
Its fine, btw why'd you need it for?

[2025-01-29 21:13] James: i have my own encoder/decoder and im just getting tired of typing `ADD_32_addsub_imm`

[2025-01-29 21:13] Yoran: [replying to James: "i have my own encoder/decoder and im just getting ..."]
So why do you need it for?

[2025-01-29 21:13] Yoran: Generating mnemonics? Because you don't need to do all that typing

[2025-01-29 21:14] James: not mnemonics

[2025-01-29 21:14] James: more specific codes

[2025-01-29 21:14] James: likely something with all the operands

[2025-01-29 21:14] James: So maybe:

[2025-01-29 21:14] James: `Add_Xd_Xn_Xm_ext`

[2025-01-29 21:14] Yoran: [replying to James: "more specific codes"]
You need specific encodings? So use the official arm ASL bullshit

[2025-01-29 21:16] Yoran: You're building your own custom en/de/coder that needs to be good? Yeah shape a format and do it manually or do it manually-manually and by that I mean embedding everything to your code

[2025-01-29 21:17] contificate: low key what you want seems worse than the current situation

[2025-01-29 21:17] contificate: is your thing more precise, James?

[2025-01-29 21:21] Torph: [replying to contificate: "I don't really follow what you're trying to do - c..."]
rewrite rules? you could also parse the XML directly yeah

[2025-01-29 21:24] Yoran: [replying to Torph: "rewrite rules? you could also parse the XML direct..."]
~Could but should?~

[2025-01-29 21:26] Torph: [replying to Yoran: "Well, you first start with the base ISA and then m..."]
is that this section? I didn't realize there was a set of base instructions
[Attachments: 2025-01-29_16-25.png]

[2025-01-29 21:27] Yoran: [replying to Torph: "is that this section? I didn't realize there was a..."]
Yeah I guess.  And it's a common thing in (maybe all, not just risc idk) risc architectures - you have the base instructions that are not canonical to the ISA and then you have your extensions 

Keep in mind I'm likely to be wrong

[2025-01-29 21:28] Torph: [replying to Yoran: "~Could but should?~"]
I think I'm gonna go the "never underestimate the bandwidth of a truck full of flash drives" route, and just use a macro to copy all the instruction names from the ASL to a C enum

[2025-01-29 21:31] Yoran: [replying to Torph: "I think I'm gonna go the "never underestimate the ..."]
Sure, sounds good

[2025-01-29 21:32] Torph: it's that or like a 5-layer branching monstrosity in my decoder lol
I need an intermediate representation so I'm planning to have an opcode field in my tree of instructions

[2025-01-29 21:32] contificate: find it sad that people doing lifting etc. don't use languages with ADTs in the first place

[2025-01-29 21:32] Torph: is that A for "abstract" or "algebraic"?

[2025-01-29 21:32] contificate: like look at the tagged union clusterfuck capstone expects us to write matching code for

[2025-01-29 21:32] contificate: algebraic

[2025-01-29 21:32] Yoran: [replying to Torph: "it's that or like a 5-layer branching monstrosity ..."]
Arm ISA is so hierarchical thats such a bummer

[2025-01-29 21:33] Yoran: Hierarchical bitch

[2025-01-29 21:33] contificate: I'll get a job there and do what Alistair Reid was doing!

[2025-01-29 21:33] Torph: I like the organized decoding actually it's just a lot of ground to cover

[2025-01-29 21:33] Torph: [replying to contificate: "find it sad that people doing lifting etc. don't u..."]
I'm just don't really know anything about them

[2025-01-29 21:34] Yoran: [replying to contificate: "find it sad that people doing lifting etc. don't u..."]
I got no idea what you actually mean

[2025-01-29 21:34] Torph: i've heard of them and definitely seen them explained but I've never used them

[2025-01-29 21:34] contificate: you know like

[2025-01-29 21:34] contificate: every time you write C

[2025-01-29 21:34] contificate: and have to use a tagged union

[2025-01-29 21:35] contificate: and then write manual pattern matching code to discern which kind of value you have

[2025-01-29 21:35] contificate: that's what ADTs and pattern matching alleviate you from

[2025-01-29 21:35] Torph: oh ok
I've seen some Rust tagged unions and they looked more convenient

[2025-01-29 21:35] Yoran: [replying to contificate: "and then write manual pattern matching code to dis..."]
Ahhhh

[2025-01-29 21:36] contificate: ```c
struct expr {
  enum { INTEGER, VAR, BOP } tag;
  union {
    int integer;
    char var[16];
    struct { enum bop op; struct expr* left, *right; } bop;
  } as;
};
```

[2025-01-29 21:36] contificate: don't sue me for that left, right listing

[2025-01-29 21:36] contificate: I don't actually do that

[2025-01-29 21:36] contificate: but that's what I mean

[2025-01-29 21:36] contificate: you get that with capstone

[2025-01-29 21:36] contificate: the actual cs_insn or whatever

[2025-01-29 21:36] contificate: is effectively a tagged union, as many things are

[2025-01-29 21:37] contificate: ELF headers, ASTs in compilers, llvm::Instr, etc.

[2025-01-29 21:37] Torph: I'm definitely going to use a tagged union in my IR instruction tree

[2025-01-29 21:38] contificate: I can't write C without using them

[2025-01-29 21:39] Torph: yeah I can't think of any other reasonable structure for what I'm doing

[2025-01-29 21:39] contificate: honestly one of the most important ideas in programming

[2025-01-29 21:40] Torph: I did that in a C++ program so I could have an array of abstract data with constant size instead of a pointer rat's nest. showed it to my stepdad and he was like "oh wow that's terrible I can't believe you'd do this"

[2025-01-29 21:40] contificate: C++ is sub-par for this

[2025-01-29 21:40] contificate: just brutal

[2025-01-29 21:40] contificate: people use class hierarchies to encode tagged unions there

[2025-01-29 21:40] Yoran: [replying to contificate: "and have to use a tagged union"]
C doesn't have tagged unions. I'm so confused

[2025-01-29 21:40] contificate: to the point where LLVM has its own dyn_cast stuff because it's so pervasive to have to downcast

[2025-01-29 21:40] contificate: what?

[2025-01-29 21:40] Yoran: [replying to contificate: "and then write manual pattern matching code to dis..."]
What's?

[2025-01-29 21:41] contificate: C has unions and a way to tag them

[2025-01-29 21:41] contificate: "tagged union" is actually more akin to the name given for how you achieve discriminated sums in C

[2025-01-29 21:41] Torph: [replying to Yoran: "C doesn't have tagged unions. I'm so confused"]
not as a first-class feature, you can make one with a union and an enum in a struct

[2025-01-29 21:41] contificate: the real terminology is ADT or variant

[2025-01-29 21:42] contificate: Rust treats it as a generalisation of an enum, so they're enums in Rust

[2025-01-29 21:42] Torph: [replying to contificate: "C++ is sub-par for this"]
definitely... it doesn't handle constructors very well when unions are involved, so I have to initialize the member I actually want to zero in a switch statement

[2025-01-29 21:43] Torph: otherwise it'll do union things and some random field will be some random number

[2025-01-29 21:47] Yoran: Do whatever, I guess

[2025-01-29 21:49] Yoran: [replying to contificate: "people use class hierarchies to encode tagged unio..."]
That makes a lot of sense where every instruction is 32bit if I understand what you mean

[2025-01-29 21:50] Yoran: Union
  Structs with bit fields for the instruction types 

And that's about it

[2025-01-29 21:50] Torph: wait im stupid why am I not using a struct with bitfields for opcode decoding

[2025-01-29 21:51] Yoran: [replying to Torph: "wait im stupid why am I not using a struct with bi..."]
Hahaha

[2025-01-29 21:51] Yoran: So I guess I didn't understand correctly

[2025-01-29 21:51] Yoran: [replying to Torph: "wait im stupid why am I not using a struct with bi..."]
(because that will be such a drudge workkkk)

[2025-01-29 21:52] Torph: infinitely better than a sprawling set of functions full of bitshifting and bitwise ops
[Attachments: 2025-01-29_16-51.png]

[2025-01-29 21:53] Yoran: [replying to Torph: "infinitely better than a sprawling set of function..."]
Well yeah you do now want to do that I guess

[2025-01-29 21:53] Yoran: (the picture)

[2025-01-29 21:53] Yoran: Doing a lot of bitwise and bit shifting is actually very good

[2025-01-29 21:54] Yoran: Especially when decoding

[2025-01-29 21:55] Torph: i mean it works but with a big tagged union I could just go like `instr.branch.type` instead of doing all that bitshifting to get `op0`

[2025-01-29 21:55] Yoran: [replying to Torph: "i mean it works but with a big tagged union I coul..."]
Oh you mean so

[2025-01-29 21:56] Yoran: If it matters to you then sure

[2025-01-29 21:57] Torph: honestly half of why I stopped working on this was having random bitshifts and one-line helper functions littered everywhere

[2025-01-29 21:58] James: the arm stuff is quite easy to parse out fields,

[2025-01-29 21:58] James: generating encoder/decoder from that is very nice

[2025-01-29 21:58] Yoran: [replying to Torph: "honestly half of why I stopped working on this was..."]
Do you know how immediates works?

[2025-01-29 21:58] James: Not nicely.

[2025-01-29 21:59] Yoran: [replying to James: "Not nicely."]
Actually very nicely, they are quite clever

[2025-01-29 21:59] Torph: [replying to Yoran: "Do you know how immediates works?"]
like how they're encoded in the instruction? yeah

[2025-01-29 21:59] James: [replying to Yoran: "Actually very nicely, they are quite clever"]
I've always taken issue with 4 required instructions to load a 64bit immediate

[2025-01-29 21:59] Yoran: [replying to Torph: "like how they're encoded in the instruction? yeah"]
Yeah, with the shift

[2025-01-29 22:00] James: without assembler macros, arm is a horrible thing to write by hand.

[2025-01-29 22:00] Yoran: [replying to James: "I've always taken issue with 4 required instructio..."]
In x64 it's two~. You rarely want to use 64 bit immediates so it's fine ig

[2025-01-29 22:00] James: It is not two...

[2025-01-29 22:00] Yoran: [replying to James: "without assembler macros, arm is a horrible thing ..."]
Build a metaprogram mohahaha

[2025-01-29 22:00] Torph: [replying to James: "I've always taken issue with 4 required instructio..."]
thats annoying but ig it makes sense since they're limiting themselves to 32-bit instructions

[2025-01-29 22:00] James: It is one.

[2025-01-29 22:01] contificate: [replying to Torph: "definitely... it doesn't handle constructors very ..."]
yeah, better to use classes than `union`

[2025-01-29 22:02] Torph: oh damn so you have to do 4 loads and a branch-to-register to do an absolute address branch

[2025-01-29 22:03] James: well. if u put the immediate in a "nearby" spot

[2025-01-29 22:03] James: u can reach it with fewer

[2025-01-29 22:03] James: and load from memory

[2025-01-29 22:31] Torph: [replying to Torph: "wait im stupid why am I not using a struct with bi..."]
oh lol I forgot you can't really encode a "don't care" bit state with unions and enums

[2025-01-30 00:01] pinefin: [replying to Torph: "infinitely better than a sprawling set of function..."]
we got the old head font

[2025-01-30 00:19] Torph: lol

[2025-01-30 00:19] Torph: its ProFont2

[2025-01-30 00:21] Torph: <https://www.programmingfonts.org/#profont>

[2025-01-30 00:32] mrexodia: [replying to Torph: "<https://www.programmingfonts.org/#profont>"]
is this intentionally made to look like a migraine attack?

[2025-01-30 00:35] Torph: <:kekw:904522300257345566> the font or the color-coding

[2025-01-30 00:36] mrexodia: font

[2025-01-30 00:36] mrexodia: nice site though, will be bookmarking it

[2025-01-30 00:40] James: [replying to Yoran: "Build a metaprogram mohahaha"]
Just realized I’m also interested in register usage(read/written) to. Is that specified anywhere? Really starting to hate this format…

[2025-01-30 00:48] Torph: i didnt see it at a glance, I was just going to keep track of it while parsing the instructions

[2025-01-30 03:16] James: Yeah I think it isn’t present

[2025-01-30 03:16] James: I thought it might be encoded in the field names

[2025-01-30 03:17] James: For example Rd means destination

[2025-01-30 03:17] James: n and m are reads

[2025-01-30 03:17] James: But then I found Rt

[2025-01-30 03:17] James: Which is nothing…

[2025-01-30 04:47] rin: Is there any significant reason for the fact that calls to gdi32 must be made from the main thread of a program.

[2025-01-30 05:25] Torph: is there even such a thing as a "main thread" in Windows' eyes? maybe there's some per-thread initialization function you need to call that's getting handled under the hood somewhere in the main thread

[2025-01-30 05:59] rin: [replying to Torph: "is there even such a thing as a "main thread" in W..."]
Maybe a golang specific issue. But you need to use original goroutine or else things don't work.

[2025-01-30 06:29] Yoran: [replying to James: "Just realized I’m also interested in register usag..."]
Yes

[2025-01-30 06:29] Yoran: [replying to James: "But then I found Rt"]
Just value iirc

[2025-01-30 06:31] James: [replying to Yoran: "Just value iirc"]
I don't follow.

[2025-01-30 06:31] James: Just value?

[2025-01-30 06:32] James: Something like this in the MSR instruction:

[2025-01-30 06:32] James: ```
{
                    "_type": "Instruction.Encodeset.Field",
                    "name": "Rt",
                    "range": {
                      "_type": "Range",
                      "start": 0,
                      "width": 5
                    },
                    "should_be_mask": {
                      "_type": "Values.Value",
                      "meaning": null,
                      "value": "'00000'"
                    },
                    "value": {
                      "_type": "Values.Value",
                      "meaning": null,
                      "value": "'xxxxx'"
                    }
                  }
```

[2025-01-30 06:32] Yoran: Uhhh sorry I just woke up

[2025-01-30 06:32] Yoran: Rn Rd Rt are registers

[2025-01-30 06:32] James: yeah i know that, i was hoping usage information was encoded somewhere

[2025-01-30 06:32] James: eg: Rn is a source

[2025-01-30 06:32] James: Rd is a dest

[2025-01-30 06:33] James: and it seems to follow that, until Rt, which is either a source or dest depending on the instruction

[2025-01-30 06:33] Yoran: Rt is target

[2025-01-30 06:33] James: its not.

[2025-01-30 06:33] James: MSR for example.

[2025-01-30 06:34] James: MRS*

[2025-01-30 06:34] James: reads from Rt and puts into the architecture register

[2025-01-30 06:34] Yoran: [replying to James: "reads from Rt and puts into the architecture regis..."]
Ahhh

[2025-01-30 06:34] Yoran: So transfer?

[2025-01-30 06:34] Yoran: Probably transfer

[2025-01-30 06:34] James: Which isn't helpful unfortunately ;(

[2025-01-30 06:35] James: Becoming increasingly frustrated with whoever decided to design these docs this way.

[2025-01-30 06:35] Yoran: [replying to James: "Which isn't helpful unfortunately ;("]
Why?

[2025-01-30 06:35] Yoran: You transfer from / to memory

[2025-01-30 06:35] James: Yeah so transfer has a different meaning when its a load or store.

[2025-01-30 06:35] James: load -> transfer means destination

[2025-01-30 06:35] James: store -> transfer means source

[2025-01-30 06:36] Yoran: Well sure

[2025-01-30 06:36] Yoran: These names are meaningles

[2025-01-30 06:36] Yoran: But I'm most cases you do have some pattern

[2025-01-30 06:36] Yoran: In*

[2025-01-30 06:37] James: The closest thing I've thought of would be looking at the position of the field corresponding assembler symbol. If it's the furthest left, that would mean it's a destination.

[2025-01-30 06:38] Yoran: Are you trying to generate an emulator?

[2025-01-30 06:38] James: No.

[2025-01-30 06:38] Yoran: So what is you doing?

[2025-01-30 06:38] James: Updating an old decoder/encoder of mine.

[2025-01-30 06:39] James: To provide more information and look nicer: Register usage information and human readable opcodes.

[2025-01-30 06:39] Yoran: Compare everything you support against arm ASL interpreter, don't make it hard on you

[2025-01-30 06:42] James: What files does this take as input btw?

[2025-01-30 06:43] James: ah i see, these .asl files.

[2025-01-30 09:49] nonchy: [replying to rin: "Is there any significant reason for the fact that ..."]
havent messed with gdi stuff in like a year or two so i could be totally wrong.....but im guessing this isnt a golang issue and is due to many(all?) gdi drawing functions having a check to make sure the call is coming from a valid win32 gui thread, https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-isguithread. the code i have for spoofing GDI calls from a non GUI thread i was replacing data in the k/ethread structure for process, cid->process, cid->thread, and win32thread

if you arent worried about ac or the likes you can probably force a thread to become a gui thread somehow like creating a message injest queue or doing something like createwindow in the thread

[2025-01-30 10:56] diversenok: GUI-related calls usually convert a thread into a GUI thread on demand so you don't generally need to worry about that

[2025-01-30 10:58] diversenok: However some might require the thread to have a loop that actively pumps window messages

[2025-01-30 15:54] Torph: [replying to rin: "Maybe a golang specific issue. But you need to use..."]
oh yeah idk much in detail about golang

[2025-01-30 17:45] James: Anyone interested in joining forces to specify additional information that is missing in the arm files? We would probably make it json like the original, and simply an add-on to the original files that gives more information such as how registers are used, and what fields are registers, and of what type. This would probably involve creating the specification for at least 600 instructions.

[2025-01-30 17:45] James: So if we had 6 of us. That’s a days work!

[2025-01-30 17:56] Brit: you want to do this manually?

[2025-01-30 17:56] Brit: jfc

[2025-01-30 18:06] James: Only some info…

[2025-01-30 18:06] James: Human readable names

[2025-01-30 18:06] James: And additional info for each field

[2025-01-30 18:06] James: It doesn’t exist anywhere specified online… so yes I want to be the first to do it

[2025-01-30 18:06] Brit: have you thought of first getting the field access types / flag /etc from the sail formalism?

[2025-01-30 18:07] James: sail?

[2025-01-30 18:07] Brit: human readable names will be the only part you actually need to do manually

[2025-01-30 18:07] Brit: https://github.com/rems-project/sail-arm/
[Embed: GitHub - rems-project/sail-arm: Sail version of Arm ISA definition,...]
Sail version of Arm ISA definition, currently for Armv9.3-A, and with the previous Sail Armv8.5-A model - rems-project/sail-arm

[2025-01-30 18:08] Brit: it's used to prove the soundness of an ISA originally but it should have the information about reg access that you care about

[2025-01-30 18:08] Brit: then the only manual part is instr naming

[2025-01-30 18:09] James: Ngl parsing that looks like more work than writing it manually

[2025-01-30 18:09] Brit: yes but humans make mistakes

[2025-01-30 18:09] James: If I do one instruction a minute I could do the whole thing in 4000 minutes

[2025-01-30 18:09] Brit: while you can only fuck up the parser in so many ways

[2025-01-30 18:09] James: Meanwhile writing something to parse that

[2025-01-30 18:09] James: And find the info I need…

[2025-01-30 18:10] James: 😬

[2025-01-30 18:10] Brit: bang it out in a weekend

[2025-01-30 18:10] Brit: 🚀

[2025-01-30 18:12] Brit: and the riscv folks for example use the formalism to gen their docs

[2025-01-30 18:12] Brit: so there should be some code to do this in the main sail repo somewhere

[2025-01-30 18:12] Brit: probably

[2025-01-30 18:16] James: Overall I’m exceedingly upset with arm

[2025-01-30 18:16] Brit: spill bestie

[2025-01-30 18:17] James: Case study for why the British shouldn’t design anything important to our world

[2025-01-30 18:17] James: Except ofc for their bottle cap thing. That’s a modern marvel.

[2025-01-30 18:18] Brit: what has you so upset with arm

[2025-01-30 18:18] Brit: it's an amazing arch

[2025-01-30 18:18] Brit: :^)

[2025-01-30 18:27] James: The supposed “machine readable specifications” is what James upset

[2025-01-30 18:28] Brit: I genuinely think you'll have better luck by using sail to do docgen

[2025-01-30 19:10] pinefin: doxygen

[2025-01-30 20:31] Torph: [replying to Brit: "https://github.com/rems-project/sail-arm/"]
wow, that's an awesome repo

[2025-01-30 21:28] Matti: [replying to nonchy: "havent messed with gdi stuff in like a year or two..."]
specifically a thread 'becomes' a GUI thread upon the first system call with a syscall number outside of ntos' own syscall range (meaning it has to be a win32k syscall)

[2025-01-30 21:29] Matti: see `PsConvertToGuiThread` for the implementation - this function is called from `KiSystemCall64` -> `KiConvertToGuiThread`

[2025-01-30 21:30] Matti: there are a bunch of restrictions to ensure only user mode threads will be converted (previousmode must be user, thread must not be a system thread)

[2025-01-30 21:31] Matti: oh and the thread's process must be in an interactive session

[2025-01-30 21:34] Matti: I haven't tried this myself but if you wanted to create a kernel thread that can do GDI calls you can cheat some of the above flags temporarily, attach to a session process, get win32k to bless your ETHREAD with a W32THREAD, and then selectively revert some of the previous steps maybe

[2025-01-30 21:35] Matti: even if this were to work though, in general anything to do with win32k is a huge pain from kernel mode because it always assumes the caller is a user mode thread

[2025-01-30 21:35] Matti: not sure about GDI as I've never used it

[2025-01-30 21:36] Matti: [replying to Torph: "is there even such a thing as a "main thread" in W..."]
not really no

[2025-01-30 21:36] Matti: there is obviously a first thread, but it's not more important to the kernel in any way than any other thread

[2025-01-30 21:37] Matti: the only place I know of where a distinction is made is in ntdll, there's a TEB flag for the 'initial thread'

[2025-01-30 21:37] Matti: I forget its exact effects but they are pretty much negligible from what I recall

[2025-01-30 21:39] Matti: [replying to Matti: "there is obviously a first thread, but it's not mo..."]
one simple way to see this is to create a suspended thread (manually in a 0-thread process or automatically as part of NtCreateUserProcess), create a second thread (suspended or not), and then kill the first

[2025-01-30 21:40] Matti: this will work with no issues

[2025-01-30 22:08] Matti: [replying to Matti: "not sure about GDI as I've never used it"]
oh wait this is actually a lie! I totally forgot about https://github.com/NSG650/NtDOOM
[Embed: GitHub - NSG650/NtDOOM: Doom running in the NT kernel]
Doom running in the NT kernel. Contribute to NSG650/NtDOOM development by creating an account on GitHub.

[2025-01-30 22:09] Matti: I used an APC there to make it sort of workable

[2025-01-30 22:11] Matti: it's unlikely this will still work as is in win 11 24H2 due to the deletion of session address spaces, but I haven't tried

[2025-01-31 04:13] Torph: [replying to Matti: "specifically a thread 'becomes' a GUI thread upon ..."]
i didnt realize there were syscalls related to win32 api

[2025-01-31 05:17] Matti: yeah, there are actually two syscall tables

[2025-01-31 05:17] Matti: the first is for ntoskrnl and the second is for win32k.sys

[2025-01-31 05:18] Matti: the dispatch table is still located in win32k.sys, but nowadays a lot of the GUI or rendering related syscalls are actually implemented in different drivers (e.g. dxgkrnl.sys) which win32k.sys just forwards to

[2025-01-31 05:19] Matti: even the win32k syscalls themselves are all in either win32kbase or win32kfull.sys

[2025-01-31 05:20] Matti: you can distinguish win32k syscalls by looking at the syscall ID - they start at 0x1000

[2025-01-31 05:21] Matti: and winobjex64 can be used to view the table
[Attachments: image.png]

[2025-01-31 05:23] Matti: [replying to Matti: "the first is for ntoskrnl and the second is for wi..."]
oh yeah... a long long time ago there was actually an exported ntoskrnl function that would let you add your own syscall table

[2025-01-31 05:23] Matti: if I remember correctly IIS used this functionality for some fucked up reason

[2025-01-31 05:26] Matti: `KeAddSystemServiceTable`, that's the one

[2025-01-31 05:28] Matti: oh huh, it is actually still or again in x64 kernels today <:lillullmoa:475778601141403648> that's a surprise

[2025-01-31 05:29] Matti: they seem to have repurposed it for pico providers... I *think*

[2025-01-31 05:35] Matti: ok no, it's still used by win32k, to add a third table... which isn't actually a dispatch table, but rather it's used as a mask for the win32k syscall filter mitigation policy

[2025-01-31 05:37] Matti: what a waste

[2025-01-31 06:17] Torph: [replying to Matti: "oh yeah... a long long time ago there was actually..."]
what?? that's deranged

[2025-01-31 06:19] Matti: yeah it sorta is, but remember that in those days it was completely normal to hook the regular SSDT or the IDT

[2025-01-31 06:19] Matti: so this was actually sort of the civilized option in a way

[2025-01-31 06:19] Torph: [replying to Matti: "what a waste"]
yeah, wow. thought that'd be used to add new syscalls under a hypervisor or something

[2025-01-31 06:23] Matti: [replying to Matti: "it's unlikely this will still work as is in win 11..."]
damn it wasn't even this
it just got killed by the fun police for no good reason
[Attachments: image.png]

[2025-01-31 06:26] Matti: and what's with the accusatory tone

[2025-01-31 06:26] Matti: that call was perfectly valid until someone added an invalid call to KeBugCheck

[2025-01-31 06:26] Torph: i still don't understand why they have a separate syscall table for win32k instead of one big table, let alone 2 unused ones and the ability to overwrite them at runtime

[2025-01-31 06:27] Matti: well, I can think of two reasons

[2025-01-31 06:27] Matti: firstly, the above mentioned PsConvertToGuiThread actually increases the available stack size for a thread

[2025-01-31 06:28] Torph: oh ok

[2025-01-31 06:28] Matti: on the assumption it will be needed due to the insane callback chains into user mode originating from win32k

[2025-01-31 06:28] Torph: I thought it was just a *completely* arbitrary restriction on who can call GUI functions <:kekw:904522300257345566>

[2025-01-31 06:28] Matti: but more importantly I think... win32k is loaded much later than the kernel itself

[2025-01-31 06:29] Matti: so the second syscall table isn't always available

[2025-01-31 06:29] Torph: ohhh so they add the table at runtime once it's able to actually accept syscalls

[2025-01-31 06:29] Torph: that makes sense actually

[2025-01-31 06:29] Matti: yes, precisely

[2025-01-31 06:30] Matti: and win32k is not even loaded until after the first user mode process (smss) is created

[2025-01-31 06:30] Matti: there is actually an NtSetSystemInformation class that it uses for this

[2025-01-31 06:33] Matti: `SystemExtendServiceTableInformation`

[2025-01-31 06:34] Matti: this class requires SeLoadDriverPrivilege (obviously), but it also requires you to be smss

[2025-01-31 06:34] Matti: if you are, then the kernel recursively calls **Zw**SetSystemInformation with the same class

[2025-01-31 06:35] Matti: and since previous mode is now kernel, it will then actually load win32k

[2025-01-31 09:52] Matti: [replying to Matti: "damn it wasn't even this
it just got killed by the..."]
fuck the police
[Attachments: image.png]

[2025-01-31 09:54] Matti: I moved the game loop to a normal kernel APC so it runs at passive level now which is good

[2025-01-31 09:55] Matti: one issue is that keyboard input now seems to be extremely sensitive

[2025-01-31 09:56] Matti: as if there is no key repeat delay, or something like that

[2025-01-31 09:57] Matti: makes it kinda hard to navigate the menu to even start the game

[2025-01-31 09:58] Matti: anyone got a clue? this is the input processing function: <https://github.com/NSG650/NtDOOM/blob/master/NtDOOM/entry.c#L355>

[2025-01-31 10:36] x86matthew: [replying to Matti: "anyone got a clue? this is the input processing fu..."]
changing
 `DoomProcessKeys(NtUserGetKeyState);` to `DoomProcessKeys(NtUserGetAsyncKeyState);`
and
`Function(VK_xxx) & 0x8000` to `Function(VK_xxx) & 1`

[2025-01-31 10:37] x86matthew: would probably make the menu nicer to use

[2025-01-31 10:37] x86matthew: but potentially screw up the game controls

[2025-01-31 10:37] x86matthew: would have to look at the code closer

[2025-01-31 10:37] Matti: yeahhh I was actually wondering about the NtUserGetKeyState, and why the guy is not using the async version instead

[2025-01-31 10:38] Matti: I'll definitely give it a try

[2025-01-31 10:39] Matti: was just getting lost in the ~50 exports from win32kbase that all seem to be doing stuff related to 'the key state'

[2025-01-31 10:39] Matti: in IDA

[2025-01-31 10:40] Matti: there's also an **Update**AsyncKeyState (not a syscall, but exported)

[2025-01-31 10:41] Matti: which is used by win32k in a bunch of other horribly named functions like `xxxUpdateGlobalsAndSendKeyEvent`

[2025-01-31 10:45] x86matthew: ```
    if (Function(VK_LEFT) & 0x8000)
        doom_key_down(DOOM_KEY_LEFT_ARROW);
    if (!(Function(VK_LEFT) & 0x8000))
        doom_key_up(DOOM_KEY_LEFT_ARROW);
```
he should probably replace the second `if` for `else` too lol

[2025-01-31 10:45] x86matthew: if only for my sanity

[2025-01-31 10:46] Matti: lmao

[2025-01-31 10:46] Matti: yeah the code is certainly special

[2025-01-31 10:47] Matti: this is just how your mind goes after dealing with win32k for too long I think

[2025-01-31 10:48] Matti: [replying to Matti: "which is used by win32k in a bunch of other horrib..."]
which is obviously part of the call chain
`CKeyboardSensor::ProcessInput -> CKeyboardProcessor::ProcessInput -> CKeyboardProcessor::ProcessInputNoLock -> ProcessKeyboardInputWorker -> xxxProcessKeyEvent -> xxxKeyEventEx -> xxxUpdateGlobalsAndSendKeyEvent -> UpdateAsyncKeyState -> PostUpdateKeyStateEvent -> ProcessUpdateKeyStateEvent`

[2025-01-31 10:48] Matti: about half of these are also exported

[2025-01-31 10:48] Matti: seemingly at random

[2025-01-31 10:49] Matti: like who is expected to be calling these from kernel mode to begin with

[2025-01-31 10:55] Matti: [replying to x86matthew: "changing
 `DoomProcessKeys(NtUserGetKeyState);` to..."]
after reading <https://stackoverflow.com/questions/64901061/what-is-the-difference-between-using-getasynckeystate-by-checking-with-its-ret> I'm now pretty sure that using GetAsyncKeyState and checking for the LSB like your second suggestion is the way to go

[2025-01-31 10:56] Matti: I think it can probably just be changed to `Function(VK_xxx) & 0x8001` to include the 'was the key pressed since the last call' bit

[2025-01-31 10:57] Matti: oh wait, maybe I'm misreading

[2025-01-31 10:57] Matti: hmm

[2025-01-31 10:57] x86matthew: you'd probably have the same issue with 0x8001

[2025-01-31 10:57] Matti: yeah

[2025-01-31 10:58] x86matthew: you want to mask out the MSB ideally, that's the current state

[2025-01-31 10:58] x86matthew: LSB is "pressed since last check"

[2025-01-31 10:58] x86matthew: which is good for the menu as it imitates how windows deals with text fields etc

[2025-01-31 10:58] x86matthew: but less so for the game itself

[2025-01-31 10:58] Matti: yeah, makes sense

[2025-01-31 10:58] x86matthew: (where MSB would make more sense)

[2025-01-31 11:00] Matti: well for the game itself I could just get rid of the obnoxious Sleep(33); in the main loop... but not sure if I wouldn't be having the same issue again due to the frame rate going from 30 to 1000

[2025-01-31 11:00] Matti: I'll go experiment a bit <:harold:704245193016344596>

[2025-01-31 11:10] x86matthew: my guess is these functions need to be expanded on:
https://github.com/NSG650/NtDOOM/blob/master/NtDOOM/DOOM/DOOM.c#L668

[2025-01-31 11:10] x86matthew: the original game probably provides additional context here

[2025-01-31 11:10] x86matthew: key repeater flags etc

[2025-01-31 11:11] Matti: I'm *reasonably* confident that code is correct, or at least correct enough to not make the game unplayable

[2025-01-31 11:12] Matti: that should be puredoom code

[2025-01-31 11:12] Matti: not this guy's

[2025-01-31 11:12] Matti: (puredoom = DIY framework for making doom run on your toaster or whatever device that has no CRT or some other strange deficiency)

[2025-01-31 11:13] x86matthew: ah i see - just found this though which might be useful : https://foss.heptapod.net/zandronum/zandronum-stable/-/blob/a57882ac83bd05e4760b3cb89d8609636e00c23c/src/sdl/i_input.cpp#L409

[2025-01-31 11:13] x86matthew: seems to be doing something like i mentioned with repeaters

[2025-01-31 11:13] x86matthew: but maybe you need to implement something like that in your code

[2025-01-31 11:14] x86matthew: essentially when deciding whether or not to call `doom_key_down()` / `doom_key_up()`

[2025-01-31 11:17] Matti: hmmm... I'm looking at the (pure)doom code and I'm basically not sure whether or not I can express something like this in the provided types lol

[2025-01-31 11:18] Matti: with variable names like these, you'd think a physicist was responsible
[Attachments: image.png]

[2025-01-31 11:19] Matti: <https://github.com/NSG650/NtDOOM/blob/master/NtDOOM/DOOM/d_event.h>

[2025-01-31 11:20] Matti: 'data1' seems to be checked for just about anything

[2025-01-31 11:21] Matti: usually in literal `& 8` style checks

[2025-01-31 11:21] Matti: not a fucking clue what the bits are supposed to mean

[2025-01-31 11:23] Matti: jfc it is used both as a bitfield and to store the actual key code

[2025-01-31 11:30] Matti: well the good news is that the game does seem to distinguish between 'UI'/menu and 'game' input events: <https://github.com/NSG650/NtDOOM/blob/master/NtDOOM/DOOM/d_main.c#L144>

[2025-01-31 11:31] Matti: so `M_Responder` is for the menu code and gets the first chance, if it doesn't do anything with the input it goes to `G_Responder`

[2025-01-31 11:32] Matti: the entire rest of the code I've seen so far kinda just makes me want to cry

[2025-01-31 11:32] Matti: e.g.
```c
//
// GLOBAL VARIABLES
//
#define MAXEVENTS (64 * 64) // [pd] Crank up the number because we pump them faster

extern event_t events[MAXEVENTS];
extern int eventhead;
extern int eventtail;
```

[2025-01-31 11:34] Matti: so MAXEVENTS is a *tunable*? how the fuck am I even supposed to know I'm 'pumping' too many events

[2025-01-31 11:35] Matti: they just get tossed silently

[2025-01-31 11:49] x86matthew: [replying to Matti: "well the good news is that the game does seem to d..."]
lol, in that case i'd use NtUserGetAsyncKeyState in both cases

[2025-01-31 11:49] x86matthew: check for `& 1` in menu mode

[2025-01-31 11:50] x86matthew: `& 0x8000` in game mode

[2025-01-31 11:50] x86matthew: should work well enough i reckon

[2025-01-31 11:51] Matti: yeah I just tried out your suggestion, & 1 works perfectly to fix the menu issues

[2025-01-31 11:51] Matti: and as you predicted, made the game itself play terribly

[2025-01-31 11:53] Matti: I was hoping that tweaking the dumb Sleep(33ms) call would have some positive effect on this but that doesn't seem to be the case

[2025-01-31 11:53] Matti: it's just generally pretty terrible no matter if I lower/remove or even increase it

[2025-01-31 11:54] Matti: [replying to x86matthew: "`& 0x8000` in game mode"]
yeah, I'm positive you're right that this will just work

[2025-01-31 11:55] Matti: the only issue is that I don't have a way of telling which mode I'm currently in... at least, I'm pretty sure I don't

[2025-01-31 11:57] Matti: the guy's NtUserGet(Async)KeyState calling function ends up going to doom's `D_PostEvent()` with whatever it's found

[2025-01-31 11:57] Matti: and then it gets put in that lovely `events[]` array until someone somewhere picks it up

[2025-01-31 12:03] x86matthew: ah sorry i misread your message, i read it as *you* could distinguish between ui/game modes

[2025-01-31 12:03] x86matthew: (well, it's definitely possible of course, the question is whether or not it's already exposed in a somewhat friendly way lol)

[2025-01-31 12:04] Matti: yeah, I mean I sort of can... <:harold:704245193016344596> but mostly the game itself seems to make a distinction at least is what I meant to say

[2025-01-31 12:05] Matti: preferably I wanna not touch any of the doom code and keep everything ntdoom related in its one file

[2025-01-31 12:06] Matti: I think I may be able to hack something together by abusing one of the hundreds of globals there are in every file

[2025-01-31 12:06] Matti: like
```c
doom_boolean inhelpscreens;
doom_boolean menuactive;
```

[2025-01-31 12:33] x86matthew: lol yeah, the code isn't exactly clear and a lot of the global states appear to not be mutually exclusive

[2025-01-31 12:33] x86matthew: from a preliminary look, i think something like this would work: `if(gamestate == GS_LEVEL && !menuactive && !paused) // (use "in-game" key mode)`

[2025-01-31 12:34] Matti: yeah that looks about right to me as well

[2025-01-31 12:35] Matti: I'll give this a try later, gonna take a break for now for the sake of my sanity I think

[2025-01-31 12:35] Matti: still happy I got it to not BSOD anymore at least <:kekw:904522300257345566>

[2025-01-31 12:36] Matti: well, unless `C:\DOOM.WAD` does not exist

[2025-01-31 12:36] Matti: then you do BSOD

[2025-01-31 12:36] Matti: but like, should've read the readme

[2025-01-31 12:36] Hunter: [replying to Matti: "which is obviously part of the call chain
`CKeyboa..."]
is this the usual call chain for getting a key state?

[2025-01-31 12:36] Matti: lmao I have absolutely no idea

[2025-01-31 12:37] Hunter: offtopic currently just wanted to know cause ive never looked at it

[2025-01-31 12:37] Matti: that is just one of many you can find if you look for something like this in IDA

[2025-01-31 12:37] Matti: I can't say I've found anything resembling any sort of logic or consistency in how anything in win32k works

[2025-01-31 12:38] Matti: it works merely by chance and coincidence

[2025-01-31 12:38] Hunter: lol its very cluttered

[2025-01-31 12:38] x86matthew: i'm convinced it's where they put people who get kicked off the main kernel team

[2025-01-31 12:38] x86matthew: either that or new recruits

[2025-01-31 12:38] Hunter: lmao

[2025-01-31 12:39] Matti: lmao I've thought this exact same thing many times

[2025-01-31 12:39] Hunter: new recruits code it then the people who got kicked off check it (they dont)

[2025-01-31 12:40] Matti: I know some people at MS who work in the kernel team... they're all die hard win32k haters, and pretty openly too

[2025-01-31 12:40] Hunter: a long time ago i wanted to know how windows grabs inputs from external devices and sends virtual inputs throughout the os and so on but didnt bother digging too deep into win32k so just ignored it

[2025-01-31 12:46] Hunter: <@148095953742725120> how many people do u actually know who work for microsoft? as in the number

[2025-01-31 12:46] Matti: very few actually

[2025-01-31 12:47] Matti: 3, maybe 4 depending on your definition of 'know'

[2025-01-31 12:47] Hunter: do they all work on the windows kernel?

[2025-01-31 12:47] Matti: no

[2025-01-31 12:48] Matti: they do all work on kernel mode components

[2025-01-31 12:48] Matti: e.g. the HAL, bootloader, hypervisor, and so on

[2025-01-31 12:48] Hunter: interesting

[2025-01-31 12:49] Matti: only one person I know at MS literally works in the 'kernel' team, I think

[2025-01-31 12:50] Matti: but he also touches code elsewhere that's kernel related, they have a pretty broad definition

[2025-01-31 12:50] Hunter: [replying to Matti: "they do all work on kernel mode components"]
i shouldve worded it better but thats what i meant more specifically

[2025-01-31 12:53] Hunter: i wonder how many of their employees are involved in hypervisor development <:muskdoubt:1124837924911992892>

[2025-01-31 12:55] Matti: all of them

[2025-01-31 12:55] Matti: no wait, that's azure

[2025-01-31 12:56] Hunter: just the windows hypervisor platform

[2025-01-31 15:18] Torph: [replying to Matti: "e.g.
```c
//
// GLOBAL VARIABLES
//
#define MAXEVE..."]
<:nomore:927764940276772925> what?? isn't the whole point of the pumps that you don't need to have a big buffer and you can just handle them all in a loop?

[2025-01-31 15:21] Torph: [replying to Matti: "as if there is no key repeat delay, or something l..."]
you could keep track of how many frames a key was repeated in your code and emulate the repeat delay by only reporting it's pressed after a certain amount of time

[2025-01-31 15:22] Torph: I did something like that in my code but to *remove* the repeat delay for movement binds