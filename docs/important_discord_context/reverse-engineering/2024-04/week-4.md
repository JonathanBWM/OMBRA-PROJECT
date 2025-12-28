# April 2024 - Week 4
# Channel: #reverse-engineering
# Messages: 31

[2024-04-22 11:49] the eternal bleb: What game

[2024-04-22 13:17] jvoisin: sounds absolutely legit and unshady

[2024-04-22 16:05] daax: [replying to Azrael: "Because Discord has a shitty history of not being ..."]
Single player games / coop (pve) games are fine to discuss and mod here. The definition if pve is player vs ai, to avoid ambiguity.

[2024-04-22 16:05] daax: [replying to Deleted User: "it doesnt make sense to forbid talking about rever..."]
You can here, that’s not a problem.

[2024-04-22 16:07] daax: it only becomes a problem when you’re asking “how to reverse BE and be undetecc in EFT” or whatever game. building trainers for a single player or coop (PvAI) game is fine.

[2024-04-22 16:08] daax: We just don’t give nuance in the rules because generally people are like “how bypass EAC!???” which… possibly will get your discord account&server nuked, so we avoid that.

[2024-04-22 16:09] Azrael: [replying to daax: "Single player games / coop (pve) games are fine to..."]
👍

[2024-04-22 16:09] daax: [replying to Azrael: "I doubt that this is the place for hiring others, ..."]
However, I appreciate you referencing the rules and calling it how you see it.

[2024-04-22 16:10] daax: if anyone needs clarification on rules in the future that seem to blanket things, im happy to answer.

[2024-04-22 16:10] Azrael: [replying to daax: "However, I appreciate you referencing the rules an..."]
Yeah, no worries.

[2024-04-23 00:57] hxm: <@162611465130475520>  what part i should modify on xdbg to make 'shift+c' copy bytes without spaces ?

[2024-04-23 03:38] the eternal bleb: I’m new to binary exploitation and have some questions about gadgets, specifically JOP gadgets

[2024-04-23 03:41] the eternal bleb: How do jmp gadgets work, I’m assuming it would be something like this:
You have jmp, loc_blah somewhere in the target application, and RSP points to the address of loc_blah. But to exploit it you place your arbitrary address at RSP when u jmp

[2024-04-23 03:43] the eternal bleb: I would need someway to visualize the stack to understand how jmp interacts with RSP too. Are there any built in ways to do that with VS or plugins or something? I’m new to all this

[2024-04-23 03:51] szczcur: [replying to the eternal bleb: "How do jmp gadgets work, I’m assuming it would be ..."]
youre on the right track. assuming there is a gadget like `pop rax; jmp rax` then when your overwritten stack location is used (lets say you fixed canary and the return address was overwritten) it will `ret` to `pop rax` which pops the last element off the stack into rax, `jmp rax` will then branch to the address in rax, ideally another gadget. primarily you dont need to visualize the stack outside of the view in x64dbg or some other tool. you will be able to determine if it will work based on the gadgets and control you have over the source operand of the branching inst. pen and paper. no need for complex machinery.

worth noting that finding reliable jop gadgets is notably harder than rop, dont constrain  yourself unless you absolutely have to.. and keep in mind what you’d need to do if mitigations are present.. such as choosing a legitimate gadget that doesnt make cfi mad.

[2024-04-23 04:02] szczcur: there’s utility in finding `add/sub rsp, X; jmp Y` among others. dont limit yourself to 2 instruction gadgets either. 5..6..8..10 doesn’t matter if its doing stuff that will adjust state requirements to your needs.

[2024-04-23 04:29] the eternal bleb: Alright tysm

[2024-04-23 04:29] the eternal bleb: Must’ve took time writing that essay

[2024-04-23 04:29] the eternal bleb: I’ll read it rq

[2024-04-23 04:44] the eternal bleb: [replying to szczcur: "youre on the right track. assuming there is a gadg..."]
Damn didn’t know JOP was harder. I’m new to all this so js want to know as much as I can rn. I haven’t even tested out with ROP yet just asking questions rn. I think you can also do NOP gadgets if the target peforms nop operations like `mov rax, rax`. I guess the real difficultly is finding these vulnerabilities into a software

[2024-04-24 00:21] szczcur: [replying to the eternal bleb: "Damn didn’t know JOP was harder. I’m new to all th..."]
i dont imagine you will need a `mov rax, rax`, however a `xor n,n/m; jmp x` might be useful because it clears OF and CF flags, which may be useful for another gadget later that may be a condition branch to a relative address you want to hit

[2024-04-24 00:36] the eternal bleb: alright alright

[2024-04-24 00:37] the eternal bleb: I have a shit ton to learn

[2024-04-24 00:37] the eternal bleb: I will start with ROP because it seems the easier imo

[2024-04-24 01:12] szczcur: [replying to the eternal bleb: "I will start with ROP because it seems the easier ..."]
once you understand the basics of rop the rest kind of fall into place, dont stress

[2024-04-24 01:12] the eternal bleb: Yeah

[2024-04-25 22:33] the eternal bleb: <@1033421942910369823>

[2024-04-25 22:33] the eternal bleb: Hey

[2024-04-25 22:33] the eternal bleb: I like u tbh

[2024-04-25 22:33] the eternal bleb: I gtg now, bye

[2024-04-28 05:02] future_wizard: ayo? not what I thought I was going to see today