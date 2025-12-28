# January 2024 - Week 3
# Channel: #reverse-engineering
# Messages: 66

[2024-01-15 00:23] mibho: my vm freezes when i run the .exe <:PU_PepeWHY:587034819905454180>

[2024-01-15 00:37] Deleted User: :brutal:

[2024-01-15 00:40] mono: [replying to mibho: "my vm freezes when i run the .exe <:PU_PepeWHY:587..."]
tf

[2024-01-15 00:46] snowua: [replying to mibho: "my vm freezes when i run the .exe <:PU_PepeWHY:587..."]
vmescape

[2024-01-15 00:47] snowua: get ratted!

[2024-01-15 00:58] Deleted User: trully brutal
[Attachments: image.png]

[2024-01-15 01:05] abu: teehee

[2024-01-15 01:17] daax: [replying to mibho: "my vm freezes when i run the .exe <:PU_PepeWHY:587..."]
https://tenor.com/view/but-it-giorgio-tsoukalos-gif-10965162

[2024-01-15 01:19] qwerty1423: [replying to Deleted User: "trully brutal"]
no wonder why ida took 90 minutes to analyze the file

[2024-01-15 01:27] Deleted User: [replying to qwerty1423: "no wonder why ida took 90 minutes to analyze the f..."]
a lot of anti-analysis indeed 🙂

[2024-01-15 01:27] Deleted User: || the anti-disassembly makes the graph look horrible and fucks the entire function analysis (adds code from other functions  which are not related as function tail). Can be fixed relatively easy though||

[2024-01-15 01:28] Deleted User: this has like > 20000 nodes, so IDA gets laggy af lmfao

[2024-01-15 01:38] Deleted User: badf00d

[2024-01-15 01:38] Deleted User: good stuff

[2024-01-15 01:38] Deleted User: https://tenor.com/view/cat-stare-angry-cat-funny-cat-memes-gif-14595486679217135725

[2024-01-15 01:42] daax: [replying to Deleted User: "good stuff"]
did you do it?

[2024-01-15 01:43] Deleted User: nah just started like 5 minutes ago

[2024-01-15 01:43] Deleted User: LMAO

[2024-01-15 01:43] daax: <:Kappa:794707301436358686>

[2024-01-15 01:43] daax: nice

[2024-01-15 01:43] Deleted User: he be badf00ding

[2024-01-15 05:23] Deleted User: [replying to daax: "my imagination lol + a few tried and true methods ..."]
That list is very long lmao

[2024-01-15 05:24] Deleted User: I was impressed by how many things it checks tbh lol

[2024-01-15 16:52] Horsie: any smart ways to intercept syscalls on a windows system with a hypervisor?

[2024-01-15 16:52] Horsie: I thought of unsetting the sce bit but wondering if theres anything simpler I can do

[2024-01-15 16:52] Horsie: Working with kvm, if thats relevant

[2024-01-15 18:24] mono: [replying to Horsie: "Working with kvm, if thats relevant"]
wouldn't know a better way.
modifying kvm for it would be rather annoying to do.

if you only want to monitor it you could probably just set a bp on `KiSystemServiceUser` .
you could probably also mess around with the passed arguments or return values if you do some scripting with the debugger.
performance hit might be too insane tho, haven't monitored syscalls with the debugger yet

[2024-01-15 20:21] daax: [replying to Horsie: "any smart ways to intercept syscalls on a windows ..."]
https://revers.engineering/syscall-hooking-via-extended-feature-enable-register-efer/
[Embed: Syscall Hooking via Extended Feature Enable Register (EFER) - Rever...]
Since the dawn of KVA Shadowing (KVAS), similar to Linux’s KPTI, which was developed by Microsoft to mitigate Meltdown vulnerabilities, hooking syscalls among other potentially malicious things has be

[2024-01-15 20:21] daax: Reliable, straightforward to implement, no sense making it harder than necessary

[2024-01-15 20:37] Deleted User: [replying to daax: "https://revers.engineering/syscall-hooking-via-ext..."]
This is also used by kaspersky iirc ye?

[2024-01-15 20:39] daax: [replying to Deleted User: "This is also used by kaspersky iirc ye?"]
No, they use a #BP in KVAS — either or is a viable approach

[2024-01-17 06:03] Horsie: [replying to daax: "https://revers.engineering/syscall-hooking-via-ext..."]
ooh thanks!

[2024-01-17 06:04] Horsie: Especially for the emulate syscall/ret code

[2024-01-17 06:04] Horsie: x86 segmentation often confuses me

[2024-01-17 06:08] Horsie: I'll disable kvas as well from the registry so I land directly into the kernel

[2024-01-17 10:22] Horsie: Working with the kvm code base is a nightmare

[2024-01-17 10:22] Horsie: I’ve given up with this for the time being. I’ll probably use infinityhook or whatever the cool kids use these days

[2024-01-17 10:28] Horsie: I’ll port it over to my simplesvm later

[2024-01-17 11:11] Deleted User: Wrong ping mb

[2024-01-17 11:11] Deleted User: <@491503554528542723>

[2024-01-17 11:11] Deleted User: https://revers.engineering/fun-with-pg-compliant-hook/
[Embed: Fun with another PG-compliant Hook - Reverse Engineering]
Abuse the HalPrivateDispatchTable to hook SYSCALL system-wide while maintain compliance with PatchGuard on Windows 10 and 11.

[2024-01-18 06:00] naci: [replying to daax: "A Windows CTF for anyone interested in dealing wit..."]
I cant dm you, you have dms turned off

[2024-01-18 10:05] qwerty1423: [replying to naci: "I cant dm you, you have dms turned off"]
you got the flag?

[2024-01-18 10:05] naci: yup

[2024-01-18 10:07] qwerty1423: nice job

[2024-01-18 10:10] naci: thanks I solved it unintended way though XD

[2024-01-18 12:37] Deleted User: gj

[2024-01-18 12:38] Deleted User: <@1068097912938180638> how long did it take you?

[2024-01-18 13:30] naci: [replying to Deleted User: "<@1068097912938180638> how long did it take you?"]
ty, it took like 4hr~

[2024-01-18 13:31] Deleted User: ah

[2024-01-18 13:31] Deleted User: I spent like 1h, and decided to stop my rapid loss of sanity

[2024-01-18 13:31] Deleted User: too much inlined code gives headache

[2024-01-18 14:20] naci: 3h of fixing code and watching videos while waiting for ida to register the stuff

[2024-01-18 14:20] naci: 30m to actually reverse and 30m to solve lol

[2024-01-18 14:32] daax: [replying to naci: "I cant dm you, you have dms turned off"]
Send it over

[2024-01-18 14:34] daax: [replying to naci: "thanks I solved it unintended way though XD"]
it’s any means necessary so i’d say there isn’t an unintended way if you got the desired result

[2024-01-18 14:36] Deleted User: how many solvers before that second chal? heh

[2024-01-18 14:37] daax: [replying to Deleted User: "how many solvers before that second chal? heh"]
Depending on if he got the flag or not, probably just the 1 then will post second

[2024-01-18 14:38] Deleted User: oh kk

[2024-01-18 14:40] Deleted User: i was about to spend some time on it but only one solve is neccesary then fine :p

[2024-01-18 14:45] daax: <@1068097912938180638> cleaned house, clean and simple solution which is always the best, noice work

[2024-01-18 14:46] daax: [replying to Deleted User: "i was about to spend some time on it but only one ..."]
well, gonna be a few days til i do so can’t hurt to try it yourself

[2024-01-18 14:46] Deleted User: good, might try it out these next 2 days

[2024-01-18 14:47] Deleted User: [replying to naci: "3h of fixing code and watching videos while waitin..."]
relatable lmfao

[2024-01-18 23:03] shalzuth: anyone ever try dumping out the stack via CaptureStackBackTrace with c#/.net?

[2024-01-20 21:29] Ignotus: I’m reversing a java program and I need a debugger for it since its packed. IntelliJ works but it uses FF decompiler which is not very good, since it cant decompile complex methods which means I can only add method breakpoints. Any way to attach a debugger which would use CFR instead?