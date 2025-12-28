# May 2024 - Week 3
# Channel: #programming
# Messages: 28

[2024-05-13 23:21] Windy Bug: https://x.com/osrdrivers/status/1790134714268831962?s=46&t=KmxCN1W2Ggg2br8H8_VXHw
[Embed: OSR (@OSRDrivers) on X]
Update regarding Driver Verifier not working in Windows 11: It seems like this "not working" behavior is restricted to when you enable DV via the GUI interface. If you use the command line it works as

[2024-05-14 00:00] Torph: lmao

[2024-05-14 00:00] Torph: wild

[2024-05-16 12:09] Horsie: How can I launch my usermode program quickest upon system boot as an unprivileged user?

[2024-05-16 12:11] Horsie: I'm trying to race a service that tries to launch/connect to its usermode component

[2024-05-16 12:12] Brit: replace / hook / shim the um component?

[2024-05-16 12:12] Horsie: The issue is that its in Program Files, where I dont have write access

[2024-05-16 12:13] x86matthew: how is the current one being launched?

[2024-05-16 12:14] Horsie: I'm not exactly sure. I tried the obvious stuff, autoruns, etc. I only know that the service talks to it once initialized

[2024-05-16 12:14] Horsie: The (usermode) service is likely launching it by CreateProcess

[2024-05-16 12:15] Brit: I think matthew meant how is the UM component launched

[2024-05-16 12:16] Horsie: Thats what I meant to answer, apologies if I wasnt clear.

[2024-05-16 12:16] Horsie: A usermode service can launch other processes, right?

[2024-05-16 12:16] Brit: sure

[2024-05-16 12:17] Horsie: Thats probably how its launching the thing I want to hook then..

[2024-05-16 13:21] Torph: [replying to Horsie: "The (usermode) service is likely launching it by C..."]
CreateFile can start a process??

[2024-05-16 14:34] Horsie: [replying to Torph: "CreateFile can start a process??"]
CreateProcess*. Sorry I've been sitting at my computer staring at this thing for way too long

[2024-05-16 15:00] luci4: [replying to Horsie: "CreateProcess*. Sorry I've been sitting at my comp..."]
Relatable

[2024-05-16 15:40] Torph: oh lmao

[2024-05-16 16:58] asz: createfile can start a process though

[2024-05-16 16:59] luci4: [replying to asz: "createfile can start a process though"]
It can???

[2024-05-16 16:59] asz: if do it on ```\\.\live.sysinternals.com\davwwwroot\wtf````

[2024-05-16 17:01] luci4: [replying to asz: "if do it on ```\\.\live.sysinternals.com\davwwwroo..."]
wow that's crazy

[2024-05-17 20:13] hxm: is there any sort of way to make a switch_case like dispatching mechanism using asmjit ?

[2024-05-17 20:33] Brit: <@692740168196685914> you mean you want a jump table?

[2024-05-17 23:45] hxm: yes

[2024-05-18 00:04] hxm: i wonder how to make such a 

`JumpTable: DW 0, Label2-Label1, Label3-Label1`

[2024-05-18 00:05] hxm: with nasm/masm its easy but still thinking on how to achieve it using a jitter