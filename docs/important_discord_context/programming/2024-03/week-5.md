# March 2024 - Week 5
# Channel: #programming
# Messages: 61

[2024-03-25 18:57] future_wizard: <@687117677512360003> I have a few functional programming questions for you. I did some l33tcode a while ago, and one of the many types of solutions pushed was the idea you could use a hashmap, to cut down on the number of iterations you needed to calculate the answer. Since functional programming does not allow mutation, how do you implement hash tables? Also, don't you lose out on solutions which mutate memory to save runtime? Another thought I guess would be that things like merge sort are much harder to implement in functional programming because you can't change the array. Maybe I just don't know what I am talking about. What do you think about these things?

[2024-03-25 19:07] sariaki: [replying to future_wizard: "<@687117677512360003> I have a few functional prog..."]
I am not the almighty colin himself, but afaik functional languages do allow mutation in some cases and thus can have normal hashmaps
(i think this is what monads are used for (?))

[2024-03-25 19:42] contificate: [replying to future_wizard: "<@687117677512360003> I have a few functional prog..."]
You do have hash tables in FP. Pure FP like Haskell would seemingly not permit classical hash maps, but in practice it does (perhaps even immutable wise using HAMTs etc.). Also, original merge sort was defined over a functional representation of lists, because it was not in place (it's probably the most typical sorting algo to implement in FP).

[2024-03-25 19:43] contificate: You should use an FP language and see.

[2024-03-25 19:44] future_wizard: [replying to contificate: "You do have hash tables in FP. Pure FP like Haskel..."]
Really, okay cool thanks

[2024-03-25 19:45] contificate: Yeah, see Hashtbl in OCaml.

[2024-03-25 19:45] contificate: There's also ones in Haskell which is pure FP

[2024-03-25 19:45] contificate: but pure FP is not the norm really

[2024-03-25 19:45] future_wizard: I don't know when I will get into functional, but I was thinking something like erlang or elixir

[2024-03-25 19:46] contificate: I can show you OCaml or Scheme sometime on stream

[2024-03-25 19:46] future_wizard: Is OCaml the one you like?

[2024-03-25 20:34] contificate: Yeah, but I've written decent amounts of many others.

[2024-03-25 23:23] Bloombit: https://hackage.haskell.org/package/array-0.5.6.0/docs/Data-Array-MArray.html

[2024-03-25 23:25] Bloombit: and
https://hackage.haskell.org/package/unordered-containers-0.2.20/docs/Data-HashMap-Strict.html#t:HashMap

[2024-03-26 09:16] mrexodia: [replying to Bloombit: "and
https://hackage.haskell.org/package/unordered-..."]
https://hackage.haskell.org/package/contravariant-1.4/docs/Data-Functor-Contravariant-Divisible.html

[2024-03-26 09:17] mrexodia: haskell poast

[2024-03-26 09:17] mrexodia: <:janpoast:585898175769083936>

[2024-03-26 20:35] MalcomVX: 
[Attachments: image.png]

[2024-03-26 20:36] MalcomVX: never know when the code you write will be compiled for some super niche architecture, best to use `NULL` or `nullptr` 🦾

[2024-03-27 12:28] froj: https://secret.club/2020/01/05/battleye-stack-walking.html

Have read this post regarding how Battle Eye uses VEH to hook certain functions, and am currently looking to implement something similar myself. However I can't think of a way to restore the original first instruction of said hooked functions without potentially introducing a race condition. (as to my understanding the 0xCC would have to be replaced prior to continuing execution and then the hook restored X time thereafter?)

Is there something I am misunderstanding from this post, or is there a trick I am missing?
[Embed: BattlEye stack walking]
With game-hacking being a continuous cat and mouse game, rumours about new techniques spread like fire. As such in this blog post we will take a look into one of the new heuristic techniques that Batt

[2024-03-27 12:28] froj: Also great job on the post, enjoyed

[2024-03-27 13:47] not-matthias: [replying to froj: "https://secret.club/2020/01/05/battleye-stack-walk..."]
Whenever your VEH handler gets called, remove the 0xCC, enable the trap flag and continue execution. Then on the single step exception, set 0xCC again

[2024-03-27 14:07] diversenok: That sounds exactly like a race condition

[2024-03-27 14:08] diversenok: There is a window of opportunity for other threads while you temporarily remove the breakpoint

[2024-03-27 14:11] froj: Agreed, I'm making the assumption here that BattleEye don't suffer from such an issue, and if not then I'm really not sure how you could accomplish this 😅

[2024-03-27 14:15] not-matthias: I mean one way is to execute the instruction in your exception handler and continue execution at the next instruction. Can be done using unicorn.

[2024-03-27 15:52] froj: Can't use unicorn for every time my hook is hit, also it runs in its own virtual environment 😅

[2024-03-27 15:52] froj: I like the idea of "emulating" the instruction though, e.g. for syscalls i could just perform `mov r10, rcx` myself in the EH, but seems hacky as there could be many edge cases

[2024-03-27 16:17] froj: Also happy to continue execution in some way that avoids restoring the original instruction, but am unsure as to how

[2024-03-27 16:31] vendor: [replying to froj: "I like the idea of "emulating" the instruction tho..."]
yes do this or use a trampoline to execute the instruction and jump back to after the 0xCC

[2024-03-27 16:32] vendor: but if you are using VEH you might as well be a  bit more stealthy and not inline patch the 0xCC

[2024-03-27 16:32] vendor: use page permissions if the target function isn't within a hot page

[2024-03-27 16:34] diversenok: Wouldn't a guard page give the exact same race condition?

[2024-03-27 18:46] vendor: never said guard. just regular read/ no write / no execute is fine

[2024-03-27 20:10] szczcur: [replying to froj: "https://secret.club/2020/01/05/battleye-stack-walk..."]
are you concerned about another thread executing during this entire process then?

[2024-03-27 20:17] froj: [replying to szczcur: "are you concerned about another thread executing d..."]
Yes, in this case I'm just experimenting with it as a usermode protection dll of sorts

[2024-03-27 20:18] froj: But that does mean that the race condition could result in gaps within telemetry

[2024-03-27 20:18] szczcur: [replying to froj: "But that does mean that the race condition could r..."]
gaps within telemetry?

[2024-03-27 20:19] szczcur: how so? what telemetry are you referring to? event telemetry provided by windows?

[2024-03-27 20:20] froj: Typical home-grown EDR project. Whilst it's relatively easy to bypass protections in place by a usermode dll, I intend to correlate this with data from the kernel too.

So in this case, telemetry would be called functions, suspicious events, etc

[2024-03-27 20:22] froj: Going to collect from other sources too but figured I'd play with VEH rather than the usual detours as it seemed interesting

[2024-03-27 20:22] szczcur: [replying to froj: "Going to collect from other sources too but figure..."]
what is the reasoning for writing int 3 directly if you're using veh?

[2024-03-27 20:24] froj: [replying to froj: "https://secret.club/2020/01/05/battleye-stack-walk..."]
experimenting with the method shown in this post. I'm making the assumption that EAC doesn't suffer from the same race condition that I appear to be struggling to solve

[2024-03-27 20:24] szczcur: what vendor said about modifying page perms is reasonable, just wondering if there is a particular reason for that preference, if at all

[2024-03-27 20:24] szczcur: [replying to froj: "experimenting with the method shown in this post. ..."]
and the race condition is?

[2024-03-27 20:25] szczcur: the one you're seeing*

[2024-03-27 20:26] froj: I think the abuse window is of such a small size that it's not consistently exploitable, however surely when performing analysis / restoring original bytes or whatnot there would be a window of which another thread could call the function and not be hooked

[2024-03-27 20:26] szczcur: [replying to froj: "I think the abuse window is of such a small size t..."]
i see, i suppose something funny could be to set page perms to nx&nw as vendor mentioned, but then block/suspend threads that try to execute in that particular fnc boundary during the window of your operations, unblock/resume suspended threads after

[2024-03-27 20:26] froj: [replying to szczcur: "i see, i suppose something funny could be to set p..."]
mmm interesting idea

[2024-03-27 20:29] froj: thanks for the input 🙏

[2024-03-27 20:29] froj: same goes for everyone else who responded

[2024-03-28 10:18] vendor: [replying to froj: "I think the abuse window is of such a small size t..."]
you don't need to restore the original bytes. can just leave the original page NX forever and move the instructions to another page. i did this with ntdll. made the entire module NX and reassembled the code sections at a different address fixing up any memory operands to point correctly to the original data sections.

[2024-03-28 10:18] vendor: Zydis makes it super easy to do this

[2024-03-28 10:19] vendor: only gotcha is an off-by-one bug when fixing up addresses. sometimes zydis reassembles an instruction slightly smaller due to redudant prefixes. when it does this the memory address will be off by 1 as rip relative encoding is actually next rip not current rip relative so when instruction shrinks in size the offset points lower.

[2024-03-28 10:20] vendor: solution is to prepend not append nops when instruction gets smaller

[2024-03-28 17:06] Brit: [replying to froj: "I think the abuse window is of such a small size t..."]
emulate the instruction you replaced or copy and relocate the original function and jump there are the two options you have here.

[2024-03-31 05:28] rin: god i hate js

[2024-03-31 05:39] BrightShard: js is not a real language

[2024-03-31 12:22] Deleted User: idk if im going schizo

[2024-03-31 12:22] Deleted User: but apparently when optimization is enabled, MSVC is just stupid

[2024-03-31 12:22] Deleted User: for example bitfields are completely ignored