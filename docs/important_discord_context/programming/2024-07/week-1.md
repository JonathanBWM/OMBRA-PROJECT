# July 2024 - Week 1
# Channel: #programming
# Messages: 105

[2024-07-01 14:35] Nats: [replying to elias: "How is the VMProtect 'rolling encryption' performe..."]
The tricky part is that it can't just encrypt everything sequentially

[2024-07-01 14:36] Nats: It needs to consider jumps, loops, and conditional branches to make sure the right code is decrypted at the right time without killing performance

[2024-07-01 14:36] Nats: So yeah, it's not just a single pass through the code - it's pretty complex process of analysis, transformation, and code injection

[2024-07-01 14:37] Nats: [replying to contificate: "Do keys cross block boundaries?"]
Typically, it doesn't. Each block usually has its own initial key, often based on something static like the address of the first instruction.

[2024-07-01 14:38] Nats: [replying to contificate: "It would be more involved - but not impossible - t..."]
Yeah, that's where things get interesting. Some advanced implementations do try to factor in predecessors or use more complex key derivation

[2024-07-01 14:38] Nats: (and sorry for ping)

[2024-07-01 14:40] elias: Thanks everyone for the answers

[2024-07-01 14:53] contificate: You've described what I said

[2024-07-01 14:53] contificate: Like your first response to me is literally just replying to me saying what I said

[2024-07-01 14:54] contificate: Seems you never read beyond the first message

[2024-07-01 15:14] Nats: [replying to contificate: "You've described what I said"]
Why should keys cross block boundaries?

[2024-07-01 15:15] Nats: Actually just agreed with you lol

[2024-07-01 15:29] contificate: the part
> Each block usually has its own initial key, often based on something static like the address of the first instruction.

[2024-07-01 15:29] contificate: is just replying to me with something I already said

[2024-07-01 15:30] contificate: please stop doing this, it provides no new information

[2024-07-01 15:31] contificate: also, if it is per block, then it's not "[a] pretty complex process of analysis, transformation, and code injection"

[2024-07-01 15:31] Brit: foreach byte in block
 byte = byte ^startaddr  :^))))

[2024-07-01 15:32] contificate: I would appreciate _actual information_ from actual vmprotect: is it more complicated than what I imagine in my head?

[2024-07-01 15:35] Brit: what do you think it works like?

[2024-07-01 15:35] Brit: I need to know this to actually answer in a non meme way

[2024-07-01 15:35] contificate: What I imagine in my head is basically: choose a statically known initial key and two invertible functions. Then, assuming a stack architecture, you would rewrite:
```
ADD 2
SUB 6
MUL 2
```
like this:
```
key = 3
ADD 2 => ADD (2 * 3) => ADD 6, key = 3 + 6, key = 9
key = 9
SUB 6 => SUB (6 * 9) => SUB 54, key = 9 + 54, key = 63
key = 63
MUL 2 => MUL (2 * 63) => MUL 126, key = 63 + 126, key = 189
```

[2024-07-01 15:36] contificate: you can generalise this to register architectures by just considering `R_i`as rewriting the `i` component

[2024-07-01 15:36] contificate: then it's easy to undo

[2024-07-01 15:36] contificate: ```
key = 3
ADD 6 => ADD (6 / 3) => ADD 2, key = 6 + 3
...
```

[2024-07-01 15:36] contificate: I pulled this out of my ass at first approximation

[2024-07-01 15:36] contificate: I have no idea how vmprotect works

[2024-07-01 15:37] Brit: operands for the operations the vm does are xor'd with the key associated with that handler

[2024-07-01 15:37] contificate: I can imagne that xor is used purely because it is involutive

[2024-07-01 15:38] Brit: probably although it would be simple to undo anything that doesn't destroy information anyway

[2024-07-01 15:38] contificate: it's more about the rolling operation, it must incorporate some "encrypted" value to form the new key

[2024-07-01 15:38] contificate: which is what I'm showing above

[2024-07-01 15:38] contificate: in a very simplistic way

[2024-07-01 15:40] contificate: is vmprotect stack based?

[2024-07-01 15:40] contificate: as in it uses native register operations to simulate an architecture whose operations manipulate stack slots, right?

[2024-07-01 15:41] Brit: ye

[2024-07-01 15:41] contificate: then what I've described above probably is just how it works, with more complicated operations

[2024-07-01 15:42] contificate: must be said, it is a pretty cute trick for straightline segments that are heavy on immediate operands

[2024-07-01 15:43] contificate: would be great layered atop a general arithmetic obfuscation, as in something like openobfuscator

[2024-07-01 16:37] Deleted User: [replying to contificate: "it's more about the rolling operation, it must inc..."]
it forms the new key by xoring it with the decrypted next_handler (vip)
<https://whereisr0da.github.io/blog/posts/2021-02-16-vmp-3/#v3---rolling-key>

[2024-07-01 16:46] contificate: so it doesn't get reset at the beginning of a new block?

[2024-07-01 17:05] Deleted User: that i dont know

[2024-07-01 17:05] Brit: it's per block

[2024-07-01 17:05] Brit: iirc

[2024-07-01 17:06] contificate: I think we're conflating two different things here

[2024-07-01 17:06] Deleted User: that would still enable people to branch anywhere after a block then tho, right? so it only ensures control flow within a block

[2024-07-01 17:06] contificate: my understanding was that we're talking about operand encryption

[2024-07-01 17:06] Brit: it's the same key

[2024-07-01 17:06] Brit: for both

[2024-07-01 17:06] Deleted User: yea

[2024-07-01 17:07] Brit: bytecode[vip] & operands are decrypted with the same key

[2024-07-01 17:07] Deleted User: operands at the start, next vip at the end

[2024-07-01 17:07] Deleted User: i have a similar but slightly diff approach where its not per block but per function

[2024-07-01 17:07] contificate: so it's conceptually the same as having a `JMP next_handler`

[2024-07-01 17:07] Deleted User: yes but the next handler isnt known until it is computed ya

[2024-07-01 17:08] contificate: yeah I mean prior to encryption

[2024-07-01 17:08] contificate: the next VIP of the next handler

[2024-07-01 17:08] contificate: is just another operand

[2024-07-01 17:08] contificate: identified by its first virtual address

[2024-07-01 17:08] Deleted User: ya i think

[2024-07-01 20:14] mrexodia: [replying to contificate: "so it doesn't get reset at the beginning of a new ..."]
it has to, otherwise loops would not be possible for example

[2024-07-01 20:15] contificate: it could be though

[2024-07-01 20:15] contificate: you could devise a method using the known predecessors if your stuff is sufficiently limited and you can't jump to arbitrary addresses

[2024-07-01 20:15] mrexodia: but you still have to adjust the key stream per-block

[2024-07-01 20:16] contificate: adjustment is fine, you also adjust per instruction as well

[2024-07-01 20:16] contificate: I just mean

[2024-07-01 20:16] contificate: you could compute block traces (much like how a compiler chooses the order in which to lay out blocks)

[2024-07-01 20:17] contificate: and devise a way that factors in only valid paths integrity

[2024-07-01 20:17] mrexodia: Conceptually to decrypt the bytecode at block `D` it has to be the same coming from both `B` and `C`
[Attachments: image.png]

[2024-07-01 20:17] contificate: it could be either

[2024-07-01 20:17] contificate: you can devise a key strategy where it must be one of B or C

[2024-07-01 20:18] mrexodia: That's kinda meaningless though? The stream has to be at the same position to decrypt the code for `D` correctly

[2024-07-01 20:19] mrexodia: You can do some crypto funny to make it do both the final streams in B and C derive to the initial stream for D, but it's effectively meaningless

[2024-07-01 20:19] contificate: I suspect there is something more complicated you could do

[2024-07-01 20:19] mrexodia: And then you have this case, where a loop has to also reset the stream
[Attachments: image.png]

[2024-07-01 20:20] mrexodia: In theory you can come up with a lot of crazy stuff, but this is just XOR streams

[2024-07-01 20:20] contificate: where it'd actually be different keys and different encryption algorithms that produce the same results, for variance, then you device between them based on predecessor, with root being special cased

[2024-07-01 20:20] contificate: anyway this is off the point

[2024-07-01 20:20] contificate: so you're telling us that

[2024-07-01 20:21] contificate: vmprotect literally just does a scheme conceptually similar to what I described above, in a code block

[2024-07-01 20:21] contificate: ignoring the problems with my choice of functions

[2024-07-01 20:22] mrexodia: It's been too long since I looked at it, but if I recall correctly the block key is reset at a JCC to be the valid seed for that block

[2024-07-01 20:23] contificate: might be fun to implement this

[2024-07-05 10:32] brymko: what is it with cpp that u cant even write to a file properly man

[2024-07-05 10:34] Brit: skill tissue

[2024-07-05 10:35] brymko: yeah tissue indeed

[2024-07-05 10:35] brymko: like file isnt open

[2024-07-05 10:36] mrexodia: CreateFileA

[2024-07-05 10:36] brymko: ok fix that

[2024-07-05 10:36] brymko: yeah

[2024-07-05 10:36] brymko: have to

[2024-07-05 10:36] mrexodia: <:kappa:697728545631371294>

[2024-07-05 10:36] brymko: the cpp api is deranged

[2024-07-05 10:36] mrexodia: it isn't

[2024-07-05 10:36] mrexodia: just fopen

[2024-07-05 10:38] brymko: thats c

[2024-07-05 10:38] brymko: idk why its bot writing

[2024-07-05 10:38] mrexodia: that's also C++

[2024-07-05 10:38] mrexodia: it works fine man

[2024-07-05 10:38] brymko: its open and its good

[2024-07-05 10:38] brymko: i flush

[2024-07-05 10:38] brymko: file empty

[2024-07-05 10:39] brymko: 😭

[2024-07-05 10:39] mrexodia: you'll need to show code man

[2024-07-05 10:39] mrexodia: 100% you fucked it up with some UB

[2024-07-05 11:22] brymko: is working now