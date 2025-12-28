# August 2024 - Week 1
# Channel: #reverse-engineering
# Messages: 16

[2024-08-01 06:27] dullard: [replying to SomethingElse: "i tried to use it for Mach-O's but then switched t..."]
Hopper is much better as of now, binja has got better but Hopper just does the job well

[2024-08-02 07:24] 0x208D9: how do i tell IDA that in the function "HvpDoAllocateCell" the fourth argument must be the variable `Context` and not `a4a`?
[Attachments: image.png]

[2024-08-02 11:22] Deleted User: [replying to 0x208D9: "how do i tell IDA that in the function "HvpDoAlloc..."]
are you asking how to rename a function?

[2024-08-02 11:22] Deleted User: press N

[2024-08-02 11:23] Deleted User: does it only needs to emulate instructions to see if a jmp is taken or not to simplify VMP protected code, what else should be done? i know also that you need to lift it to LLVM bytecodes (or anything similar) and compile it again to get rid of VMP way of doing things
what else is needed

[2024-08-02 11:30] 0x208D9: [replying to Deleted User: "are you asking how to rename a function?"]
nope not renaming, i want to change the variable a function takes and make it another variable

[2024-08-02 11:32] Timmy: y

[2024-08-02 11:36] Hunter: ^ N changes the name Y changes the type

[2024-08-02 13:49] 0x208D9: [replying to Hunter: "^ N changes the name Y changes the type"]
the type is right but the variable assignment is just wrong thats the issue

[2024-08-02 15:56] daax: [replying to 0x208D9: "the type is right but the variable assignment is j..."]
change type of a4a to that of Context and hit = (when selecting a4a) and map ctx=>a4a. Could break output, but that should do what youre asking

[2024-08-02 15:57] 0x208D9: [replying to daax: "change type of a4a to that of Context and hit = (w..."]
interesting ok

[2024-08-02 16:01] 0x208D9: thanks alot, its done

[2024-08-02 21:02] Deleted User: <@651054861533839370>

[2024-08-02 21:02] Deleted User: how is it going pal

[2024-08-02 21:16] asz: terrible

[2024-08-04 08:17] akouma: [replying to asz: "terrible"]
<:CatJawdrop:1265309996707614870>