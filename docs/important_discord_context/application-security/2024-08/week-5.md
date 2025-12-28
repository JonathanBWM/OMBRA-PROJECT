# August 2024 - Week 5
# Channel: #application-security
# Messages: 35

[2024-08-26 17:02] elias: Did someone do research on detection vectors for hyperdbg before?

[2024-08-26 17:08] Deleted User: has loads of them

[2024-08-26 17:09] Deleted User: https://github.com/jonomango/nohv
[Embed: GitHub - jonomango/nohv: Kernel driver for detecting Intel VT-x hyp...]
Kernel driver for detecting Intel VT-x hypervisors. - jonomango/nohv

[2024-08-26 19:03] szczcur: [replying to elias: "Did someone do research on detection vectors for h..."]
<@609487237331288074> wrote about some in here..you might have to dig them up i can’t find the chat

[2024-08-28 12:27] vendor: Code Virtualizer [3.2.0.0] (28-Aug-2024)
[+]
ARM64/x86/x64: Added parallel execution within each VM instance (32 virtual cores by default)

[2024-08-28 12:28] vendor: <:give:835809674480582656>

[2024-08-28 14:18] snowua: [replying to vendor: "Code Virtualizer [3.2.0.0] (28-Aug-2024)
[+]
ARM64..."]
Surely this is a dream?

[2024-08-28 14:19] snowua: There’s a hard limit? I bet the fix is mega stupid

[2024-08-28 14:19] snowua: <:topkek:904522829616263178>

[2024-08-28 14:23] kian: [replying to vendor: "Code Virtualizer [3.2.0.0] (28-Aug-2024)
[+]
ARM64..."]
waiting for the jmp table & exception handling update next 🙏

[2024-08-28 14:23] vendor: [replying to snowua: "There’s a hard limit? I bet the fix is mega stupid"]
it’s just going to be an array of size 32 vm contexts in the .data rather than 1

[2024-08-28 14:24] vendor: so still spin spin spin if more than 32 threads try execute a virtualised function.

[2024-08-28 14:24] vendor: ivan’s stack machine stays winning

[2024-08-28 14:24] snowua: [replying to vendor: "it’s just going to be an array of size 32 vm conte..."]
mhm

[2024-08-28 14:24] snowua: my bets are on this

[2024-08-28 14:52] Deleted User: 😭 crazy

[2024-08-29 22:31] Taks: [replying to vendor: "it’s just going to be an array of size 32 vm conte..."]
Jajaja

[2024-08-31 07:34] Eriktion: [replying to kian: "waiting for the jmp table & exception handling upd..."]
Out of curiosity, why aren’t jump tables supported?

[2024-08-31 08:51] vendor: [replying to Eriktion: "Out of curiosity, why aren’t jump tables supported..."]
i think they are - if they aren’t it’s because they can be a bit challenging to extract sometimes

[2024-08-31 08:52] vendor: there are quite a few variations between compilers and then you have shit like 2 level tables etc.

[2024-08-31 09:05] Brit: "a bit" actually undecidable

[2024-08-31 09:05] Brit: <:deadman:1271808245610057730>

[2024-08-31 09:15] kian: [replying to Eriktion: "Out of curiosity, why aren’t jump tables supported..."]
its cause themida, for reasons unknown, don't extract jump tables. instead the docs tell you to put markers between each case instead of the whole thing. same goes for try/except blocks. here's the doc: https://www.oreans.com/help/tm/

[2024-08-31 09:16] kian: (Protecting an application, Protection macros)

[2024-08-31 10:09] Brit: https://codedefender.io/blog/2024/07/02
[Embed: Technical Challenges of Indirect Control Flow]
This article discusses the challenges any binary analysis framework will face with indirect control flow. It covers indirect calls, jump tables (indirect jumps), and details our approach.

[2024-08-31 10:10] Brit: I recc reading this

[2024-08-31 11:14] Oliver: [replying to Brit: "https://codedefender.io/blog/2024/07/02"]
i love control flow graphs

[2024-08-31 15:40] Torph: I tried to do something similar to this and everything went to shit when I started looking into how to do a branch-to-register lmao

[2024-08-31 15:44] Torph: the project I was looking at (copying the design for a different arch) just kept a giant array of [32-bit address space]/4 pointers so anytime it needed to branch somewhere it looked up the destination address there or got  a function pointer telling it to translate

[2024-08-31 16:21] Brit: "WhY DiDn'T tHeY ImPlEmEnT JuMp TaBlEs" type of beat

[2024-08-31 16:22] Brit: unbounded ones are impossible to statically decide afaik

[2024-08-31 16:43] Torph: [replying to Brit: "unbounded ones are impossible to statically decide..."]
yeah that's what really tripped me up, I think that dynamic lookup might be the only way to do it... maybe if the target arch had >= the number of registers of the source, you could do a mostly 1-to-1 translation

[2024-08-31 18:23] Deleted User: [replying to Brit: ""WhY DiDn'T tHeY ImPlEmEnT JuMp TaBlEs" type of be..."]
i implemented them in my virtualizer but only for llvm based compilers specifically but thats not too bad, you can determine jump table size even if they are next to eachother by checking references usually

[2024-08-31 18:27] Brit: I meant in a more general sense

[2024-08-31 18:28] Brit: I suspect I could trick your jump table recovery by writing a little bit of assembly manually