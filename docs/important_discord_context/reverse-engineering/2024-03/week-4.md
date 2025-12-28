# March 2024 - Week 4
# Channel: #reverse-engineering
# Messages: 56

[2024-03-19 18:01] TheXSVV: May I ask what you're reversing here?

[2024-03-19 18:49] brymko: https://tenor.com/view/nothing-just-hanging-around-gif-6244099902315757809

[2024-03-21 13:07] Horsie: Does windows desktop/server have smap/smep?

[2024-03-21 13:07] Horsie: Also do servers have other mitigations enabled that aren’t on desktop?

[2024-03-21 13:08] Nats: [replying to Horsie: "Does windows desktop/server have smap/smep?"]
that is enabled by default on compatible hardware

[2024-03-21 13:09] Horsie: Both, ap and ep?

[2024-03-21 13:11] Nats: Yes

[2024-03-21 13:14] Nats: <@491503554528542723> what do you want to achieve?

[2024-03-21 13:15] Horsie: [replying to Nats: "<@491503554528542723> what do you want to achieve?"]
Just trying to have fun with an old exploit for windows. It’s just a test environment for some tooling I’m trying

[2024-03-21 13:42] Timmy: <@691692572103409755> Great status sir

[2024-03-21 13:42] Timmy: <:KEKW:798912872289009664>

[2024-03-21 18:06] brymko: https://x.com/_revng/status/1770859800588431366?s=12
[Embed: rev.ng (@_revng) on X]
🚀 BIG ANNOUNCEMENT! 🚀

The full rev.​ng decompiler pipeline is now fully open source!
Also, we'll soon start to invite people to participate in the UI closed beta.

Check out our latest blog post: htt

[2024-03-21 18:10] jvoisin: I'm curious about their business model. Decompilers are useful when doing reverse engineering, but the major RE tools already have their own ~working ones qnd creating a new RE platform is highly nontrivial, if only UX-wise

[2024-03-21 18:48] Deleted User: [replying to jvoisin: "I'm curious about their business model. Decompiler..."]
they ripped of UI from vscode

[2024-03-21 18:48] Deleted User: 😢

[2024-03-21 18:56] Deleted User: it used to be a vsc plugin im not sure if it is anymore tho

[2024-03-21 19:03] jvoisin: [replying to jvoisin: "I'm curious about their business model. Decompiler..."]
Apparently, they're planning on making money with their cloud offer, where the UI lives

[2024-03-21 19:16] 25d6cfba-b039-4274-8472-2d2527cb: [replying to jvoisin: "Apparently, they're planning on making money with ..."]
might just be me being an anti-cloud fanatic, but to me a cloud reverse engineering tool makes no sense. whether its a malware sample from a private source or something else you probably don't want some random company tracking what you're doing.

[2024-03-21 19:20] jvoisin: [replying to 25d6cfba-b039-4274-8472-2d2527cb: "might just be me being an anti-cloud fanatic, but ..."]
My thoughts as well. But you can pay for get on-prem

[2024-03-21 19:21] jvoisin: But I don't see why someone who pay for this instead of IDA teams or Ghidra. Heck, I'm sure binja has collab too at this point

[2024-03-21 19:45] dullard: [replying to jvoisin: "But I don't see why someone who pay for this inste..."]
(It does)

[2024-03-21 20:15] Saturnalia: [replying to jvoisin: "But I don't see why someone who pay for this inste..."]
(it's far better than Ida teams for collab)

[2024-03-21 20:17] jvoisin: [replying to Saturnalia: "(it's far better than Ida teams for collab)"]
Better enough to be paying for it and either ditch ida entirely, or write tooling to sync both? And cheaper than spending time scripting kinks away from ida teams ? Seems like a huge gamble :/

[2024-03-21 20:18] jvoisin: Don't get me wrong, I do love to see new tools trying to dethrone IDA!

[2024-03-21 20:18] 25d6cfba-b039-4274-8472-2d2527cb: Ghidra already did it

[2024-03-21 20:18] 25d6cfba-b039-4274-8472-2d2527cb: !

[2024-03-21 20:18] 25d6cfba-b039-4274-8472-2d2527cb: all my homies use ghidra

[2024-03-21 20:25] snowua: [replying to 25d6cfba-b039-4274-8472-2d2527cb: "all my homies use ghidra"]
need some new homies

[2024-03-21 20:25] 25d6cfba-b039-4274-8472-2d2527cb: [replying to snowua: "need some new homies"]
you have been BANNED from my ghidra and BSim servers!

[2024-03-21 20:25] snowua: 😟

[2024-03-21 20:26] 25d6cfba-b039-4274-8472-2d2527cb: I'm just waiting for dns propagation but the ban appeal site will be up soon!

[2024-03-21 20:26] jvoisin: [replying to 25d6cfba-b039-4274-8472-2d2527cb: "all my homies use ghidra"]
Think about the boomers :'(

[2024-03-21 21:01] Saturnalia: [replying to jvoisin: "Better enough to be paying for it and either ditch..."]
could just be what I was working with, but IDA teams was borderline unusable for the large binaries I was trying to load while binja sort of worked

[2024-03-21 21:01] Saturnalia: also the collab stuff has a chatroom so you can shitpost which is 💯

[2024-03-21 23:40] 25d6cfba-b039-4274-8472-2d2527cb: [replying to snowua: "😟"]
you can now submit unban applications to unbans@cyberbullying.club ! very official !

[2024-03-22 14:03] snowua: [replying to 25d6cfba-b039-4274-8472-2d2527cb: "you can now submit unban applications to unbans@cy..."]
ahahahaha great domain

[2024-03-22 14:03] snowua: nice redirect <:kekw:904522300257345566>

[2024-03-22 14:04] 25d6cfba-b039-4274-8472-2d2527cb: its a placeholder mkay

[2024-03-24 00:09] Analyze: Any tips on debugging Linux binary from ISO? I was thinking about embedding debugger attaching it to the process and connecting remotely from host machine

[2024-03-24 00:51] Matti: wdym 'from ISO'? debugging a binary that's running from a live USB or virtual DVD or something?

[2024-03-24 00:53] Matti: as long as you have an ethernet connection you can debug remotely with gdb over tcp, though personally I'd probably set up SSH on the ISO just because you always need it at some point anyway

[2024-03-24 00:54] Matti: then you may as well just install gdb using whatever package manager and connect that way

[2024-03-24 00:54] Matti: both should work

[2024-03-24 00:56] Matti: if this isn't possible or not what you're trying to do, please clarify

[2024-03-24 00:58] 25d6cfba-b039-4274-8472-2d2527cb: mount and chroot <:yea:904521533727342632> (yeah probs needs some clarification)

[2024-03-24 00:58] Matti: oh yeah that's another way <:thinknow:475800595110821888>

[2024-03-24 01:58] Analyze: [replying to Matti: "wdym 'from ISO'? debugging a binary that's running..."]
Yeah basically debugging a binary used for installation process

[2024-03-24 02:00] Analyze: Will probably try to use GDB server

[2024-03-24 02:00] Analyze: thanks for help

[2024-03-24 15:58] 0xatul: ripperino

[2024-03-24 15:58] 0xatul: 
[Attachments: image.png]

[2024-03-24 16:00] 0xatul: ```
 do
      {
        v61 = _mm_loadu_si128(v57 + 3);
        v62 = _mm_loadu_si128(v57 + 2);
        v63 = _mm_loadu_si128(v57 + 1);
        v64 = _mm_add_epi32(
                _mm_add_epi32(
                  _mm_add_epi32(
                    _mm_add_epi32(
                      _mm_xor_si128(
                        _mm_xor_si128(
                          _mm_xor_si128(
                            _mm_xor_si128(
                              _mm_xor_si128(_mm_slli_epi32(v61, 0x1Au), _mm_srli_epi32(v61, 6u)),
                              _mm_slli_epi32(v61, 0x15u)),
                            _mm_srli_epi32(v61, 0xBu)),
                          _mm_slli_epi32(v61, 7u)),
                        _mm_srli_epi32(v61, 0x19u)),
                      v57[8]),
                    v60),
                  _mm_xor_si128(_mm_and_si128(_mm_xor_si128(v62, v63), v61), v63)),
                _mm_shuffle_epi32(_mm_cvtsi32_si128(v55[0xFFFFFFFE]), 0));
        v60 = _mm_add_epi32(v64, v59);
        v57[4] = v60;
        v59 = _mm_add_epi32(
                _mm_add_epi32(
                  _mm_xor_si128(
                    _mm_xor_si128(
                      _mm_xor_si128(
                        _mm_xor_si128(
                          _mm_xor_si128(_mm_slli_epi32(v54, 0x1Eu), _mm_srli_epi32(v54, 2u)),
                          _mm_slli_epi32(v54, 0x13u)),
                        _mm_srli_epi32(v54, 0xDu)),
                      _mm_slli_epi32(v54, 0xAu)),
                    _mm_srli_epi32(v54, 0x16u)),
                  v64),
                _mm_or_si128(_mm_and_si128(_mm_or_si128(v56, v54), v58), _mm_and_si128(v56, v54)));
      ```

[2024-03-24 16:00] 0xatul: on the other hand

[2024-03-24 16:00] 0xatul: lmao

[2024-03-24 16:00] 0xatul: its painful sometimes

[2024-03-24 16:51] Deleted User: what cursed thing is this lmfao