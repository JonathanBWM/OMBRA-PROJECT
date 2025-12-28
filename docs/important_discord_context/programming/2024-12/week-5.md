# December 2024 - Week 5
# Channel: #programming
# Messages: 38

[2024-12-28 20:38] vendor: <@687117677512360003> any recommendations for an OCaml x86 disassembler library? this project <https://github.com/BinaryAnalysisPlatform/bap>  seems to have bindings for LLVM. I had a play around with creating Zydis bindings to see how it would look and seems quite disgusting going from C structs with enums (e.g. mnemonics and registers) to proper variants. I think you would have to create a record type that mimics the basic `ZydisInstruction` / `ZydisOperand` with ints instead of enums and then generate matching code that constructs variants from them. also Zydis has enum values of None so will have to handle turning those into options. would be a pain to get all that working but the final API could be really nice and even support re-assembly. not sure how the BAP stuff compares.

[2024-12-28 20:39] contificate: make one g

[2024-12-28 20:39] contificate: There's capdtone ones

[2024-12-28 20:39] contificate: but we'll make zydis belters

[2024-12-28 20:39] vendor: capstone sucks

[2024-12-28 20:39] vendor: and i want re-assemble

[2024-12-28 20:39] vendor: i think i'll make chad Zydis bindings

[2024-12-28 20:40] vendor: but need to figure out how generated code works

[2024-12-28 20:40] vendor: never done that in ocaml before other than using ppx shit

[2024-12-28 20:46] Brit: make iced bindings instead

[2024-12-28 21:14] vendor: [replying to Brit: "make iced bindings instead"]
yeah i am not figuring out how to do rust bindings lol

[2024-12-28 21:16] Brit: could have had a nice disasser

[2024-12-28 21:16] Bloombit: Maybe xed?

[2024-12-28 21:17] vendor: actually i just saw how ocaml represents variant constructors at the C level

[2024-12-28 21:17] vendor: i have some very bad ideas now

[2024-12-28 21:17] vendor: i can make Zydis work

[2024-12-28 21:17] vendor: without huge switch statements or any bullshit

[2024-12-28 21:36] James: [replying to Brit: "could have had a nice disasser"]
it really is the best

[2024-12-28 21:36] James: after using xed zydis and crapstone, iced is very nice

[2024-12-28 21:36] Brit: xor reg, reg being a write only is so sane

[2024-12-28 21:37] James: pretty small instruction structure too compared to the others

[2024-12-28 21:37] Brit: iced my beloved

[2024-12-28 21:37] James: [replying to Brit: "xor reg, reg being a write only is so sane"]
this is correc though isn't it?

[2024-12-28 21:37] Brit: yes

[2024-12-28 21:37] Brit: recently I was diffing liveness analysis when implemented on different dissasemblers

[2024-12-28 21:37] Brit: so I found a few very nice quirks

[2024-12-28 21:38] James: another case when rust is just better than everyone else i guess

[2024-12-28 21:39] James: have u tried this <@303272276441169921> ?

[2024-12-28 21:39] James: https://github.com/emproof-com/nyxstone
[Embed: GitHub - emproof-com/nyxstone: Nyxstone: assembly / disassembly lib...]
Nyxstone: assembly / disassembly library based on LLVM, implemented in C++ with Rust and Python bindings, maintained by emproof.com - emproof-com/nyxstone

[2024-12-28 21:39] James: actually maybe <@162611465130475520> better to ask

[2024-12-28 21:40] Brit: I have not

[2024-12-28 21:41] Brit: everywhere I look I see him https://cyber.autis.moe/SaFubOza96.png

[2024-12-28 21:41] James: 
[Attachments: image.png]

[2024-12-28 21:41] James: might just be assembling strings actually

[2024-12-28 21:41] James: so maybe not good to compare to anything other than capstone

[2024-12-28 22:14] mrexodia: [replying to James: "actually maybe <@162611465130475520> better to ask"]
I just ported it to Windows, didn't use it much outside of basic testing

[2024-12-28 22:14] mrexodia: But yeah, it's just for strings iirc

[2024-12-29 03:15] abu: [replying to vendor: "<@687117677512360003> any recommendations for an O..."]
I was looking at using OCaml for Binary analysis as well but I'm leaning towards C# since BAP is pretty difficult to learn. If you want to use Haskell instead, hdis86 exists