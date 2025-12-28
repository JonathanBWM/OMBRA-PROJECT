# November 2025 - Week 3
# Channel: #reverse-engineering
# Messages: 34

[2025-11-10 16:14] ImagineHaxing: now im getting a captcha every 177 requests, before it was every 15 (i forgot to add a header) but idk what could be the issue im using full headers i dont think im missing anything

[2025-11-10 20:01] ImagineHaxing: [replying to ImagineHaxing: "now im getting a captcha every 177 requests, befor..."]
i think everyone gets a captcha after 177 requests no matter what

[2025-11-10 23:14] pinefin: [replying to ImagineHaxing: "now im getting a captcha every 177 requests, befor..."]
you just gave me a really strange idea

[2025-11-11 04:05] ImagineHaxing: [replying to pinefin: "you just gave me a really strange idea"]
?

[2025-11-12 10:54] Gestalt: I know this is kinda a meme, but how do you guys handle MBA, do you treat it as a blackbox and just throw stuff in and see what comes out, or do you use tools to simplify them?

[2025-11-12 12:20] 0xdeluks: I summon a horse to simplify these expressions for me

[2025-11-12 12:39] Brit: [replying to Gestalt: "I know this is kinda a meme, but how do you guys h..."]
Bit blasting sort of works

[2025-11-12 13:25] 0xdeluks: sth like this could maybe work too
https://gist.github.com/superfashi/563425ee96d505c0263373230335e41a#7---the-boss-needs-help
[Embed: Flare-On 12 Write-Up]
Flare-On 12 Write-Up. GitHub Gist: instantly share code, notes, and snippets.

[2025-11-12 14:31] Gestalt: [replying to 0xdeluks: "sth like this could maybe work too
https://gist.gi..."]
That is huge

[2025-11-12 14:31] Gestalt: [replying to 0xdeluks: "sth like this could maybe work too
https://gist.gi..."]
You have no clue how much trouble you saved, this is super sick it actually saved me a lot of headaches

[2025-11-12 14:32] Gestalt: thank you for sharing this with me

[2025-11-12 14:56] the horse: [replying to 0xdeluks: "I summon a horse to simplify these expressions for..."]
FAKE

[2025-11-12 15:01] 0xdeluks: [replying to the horse: "FAKE"]
my fault twin

[2025-11-12 15:02] 0xdeluks: maybe it got patched in the latest update

[2025-11-12 15:02] the horse: MBA is hard.. most of the public stuff has oracles for limited amount of vars and nodes

[2025-11-12 15:02] the horse: and it grows kinda exponentially

[2025-11-12 15:02] the horse: so you'd sort of want to apply all optimizations you can + SMT solver and store the solved expressions in your own oracle

[2025-11-12 15:03] the horse: so the next time you encounter them you can speedrun through them

[2025-11-12 15:03] the horse: or.. get the input variables

[2025-11-12 15:03] 0xdeluks: thank you horse

[2025-11-12 15:04] the horse: for most apps it's usually static values +- relocs

[2025-11-12 15:08] the horse: the horse hates mba
[Attachments: ir_after_optimization.txt, ir_before_optimization.txt]

[2025-11-12 15:15] Gestalt: [replying to the horse: "the horse hates mba"]
gawd dayum

[2025-11-12 15:16] Gestalt: the simplifying horse is real, you just need to believe and know

[2025-11-12 15:17] the horse: simplifying horse is real

[2025-11-12 15:18] the horse: mba simplifying horse is not real yet

[2025-11-12 15:18] the horse: the holy roman empire's horse will defeat hungary once again

[2025-11-14 14:59] pygrum: for people who have used remill, is there a way to recover the address of the instruction that the IR came from? Like IMark in VEX

[2025-11-14 15:33] pygrum: nvm got it

[2025-11-15 00:33] elias: Does anyone know how I could tell binary ninja that `FveCallbacks` is at offset 0x48 without having to calculate and put the `Unknown` padding member before?
[Attachments: image.png]

[2025-11-15 00:36] the horse: nah

[2025-11-15 00:38] the horse: psure it's clang syntax

[2025-11-15 00:38] the horse: doesn't support anything like it to my knowledge

[2025-11-15 00:38] the horse: except maybe alignment for a specific variable but then you just move the padding from one line to another lol