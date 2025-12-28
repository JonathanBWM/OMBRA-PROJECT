# July 2024 - Week 2
# Channel: #application-security
# Messages: 34

[2024-07-09 12:38] th3: hello is there a way to block handle access or close handle access of another process from a remote process

[2024-07-09 12:51] Brit: many different ways, easiest would probably be to just hook whatever you don't want opening handles to you

[2024-07-09 12:54] th3: [replying to Brit: "many different ways, easiest would probably be to ..."]
thank you, what is the other ways?

[2024-07-09 12:57] Brit: if you are doing legitimate things register an obcallback

[2024-07-09 12:57] Brit: and deny it there

[2024-07-09 12:57] Brit: from a driver

[2024-07-09 13:02] diversenok: Does the process you want to protect from run as admin?

[2024-07-09 13:03] diversenok: If not, just use a security descriptor to deny access

[2024-07-09 13:03] diversenok: No need to all this hooking and other custom stuff

[2024-07-09 13:07] Brit: I'm assuming it does

[2024-07-09 13:07] Brit: usually these things are :^)

[2024-07-09 13:14] diversenok: Then usually it's not "from a remote process" but from a driver, no?

[2024-07-09 13:15] diversenok: So hooking doesn't help much either

[2024-07-09 13:15] Brit: true enough

[2024-07-09 13:36] th3: [replying to Brit: "true enough"]
thank you guys, no not admin mode

[2024-07-09 13:45] Brit: [replying to diversenok: "If not, just use a security descriptor to deny acc..."]
wait the thing that opens the handle you wanna prevent isn't priviledged? then you can just do this above

[2024-07-09 14:02] th3: [replying to Brit: "wait the thing that opens the handle you wanna pre..."]
yeah but first im going to read about it 😄

[2024-07-10 02:00] th3: [replying to Brit: "wait the thing that opens the handle you wanna pre..."]
im not sure if im doing anything, but it seems like it still is getting access

[2024-07-10 02:07] th3: oh wait mb its working

[2024-07-14 20:48] elias: Did someone ever exploit VM based protectors like VMProtect to get unsigned code execution in kernel with hvci enabled?

[2024-07-14 20:52] Deleted User: why cant u still just use exploitable drivers

[2024-07-14 20:53] elias: you can't allocate executable memory

[2024-07-14 20:54] Deleted User: you can abuse exploitable signed drivers though to call apis etc no?

[2024-07-14 20:54] Deleted User: how would vmprotect be used to get code execution in tis case anyways

[2024-07-14 20:57] elias: [replying to Deleted User: "how would vmprotect be used to get code execution ..."]
load a legitimate vm protected driver along with its vm handlers, then independently use a kernel read/write to load your own shellcode into the kernel in a language that the vm we previously loaded can understand and cause a vmentry at this manually loaded position

[2024-07-14 20:58] elias: [replying to Deleted User: "you can abuse exploitable signed drivers though to..."]
afaik its getting harder

[2024-07-14 20:58] Brit: [replying to elias: "load a legitimate vm protected driver along with i..."]
enjoy extracting a useful turing machine from handlers that are specific to the protected program

[2024-07-14 20:58] Deleted User: [replying to elias: "load a legitimate vm protected driver along with i..."]
i think ur better off with the abusing signed drivers approach lmao

[2024-07-14 20:58] Deleted User: its a funny idea but def not viable

[2024-07-14 20:59] elias: I see

[2024-07-14 20:59] Deleted User: and u would have to need kernel access anyways to use a kernel read/write

[2024-07-14 20:59] elias: yeah

[2024-07-14 20:59] elias: but thats doable

[2024-07-14 20:59] elias: getting unsigned code execution not so much