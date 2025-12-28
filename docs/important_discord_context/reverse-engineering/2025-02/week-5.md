# February 2025 - Week 5
# Channel: #reverse-engineering
# Messages: 33

[2025-02-24 03:59] Deleted User: [replying to KulaGGin: "I'm not sure why I couldn't catch it in debuggers ..."]
when hooking NtSetInformationThread in ur dll u replace/redirect the original function so when ur program calls NtSetInformationThread it ends up calling ur detour instead of the original function. if u set a breakpoint on the original address or in a place u expect the syscall to occur its possible might not be reached because the control flow was diverted

[2025-02-24 07:08] Ignotus: [replying to Xed0sS: "https://github.com/GraxCode/threadtear"]
ey if you're still here, do u happen to know if there's an alternative for newer java versions? just curious

[2025-02-24 11:18] Matti: [replying to diversenok: "The only supported thread flag in `NtCreateUserPro..."]
oh hm, that is kind of surprising

[2025-02-24 11:19] Matti: I've never used the thread creation flags parameter before so I just assumed they would do the obvious thing

[2025-02-25 03:58] diversenok: Yeah, it's a bit lame that they limited the flag use

[2025-02-25 14:57] elias: What setup would you guys use if you wanted to debug the Windows Secure Launch (tcbloader.dll/tcblaunch.exe) at runtime? Is it even possible without a physical machine?

[2025-02-26 13:09] elias: Do anti cheats virtualize/protect stuff like kernel callbacks? If yes, how does it affect system performance?

[2025-02-26 15:23] daax: [replying to elias: "Do anti cheats virtualize/protect stuff like kerne..."]
Yes, and nominally.

[2025-02-26 15:31] elias: oke thanks

[2025-02-26 15:31] elias: seems like I have some optimization to do

[2025-02-26 20:30] James: for the standard kernel callbacks it doesn't matter how long it takes to execute.

but if you're hooking something, say for example in swapcontext, it definitely does matter how long your code takes and if you take too long you'll get a watchdog timeout

[2025-02-26 20:40] elias: [replying to James: "for the standard kernel callbacks it doesn't matte..."]
well if your registry callback is taking too long (in terms of microseconds) the whole system will start to lag

[2025-02-26 20:42] James: oh yeah that's true for sure as well

[2025-02-27 03:11] daax: [replying to James: "for the standard kernel callbacks it doesn't matte..."]
execution time for callbacks of the common object types does matter.

[2025-02-27 03:38] daax: other things on the system may not play nice. it’s best to treat them like a fire and forget, no waiting, no polling, etc. not to mention waiting for an event completely defeats the purpose of queuing work to be done async. ex: drivers callback is hit, drivers queues worker and waits for event to indicate it completed before carrying on. if some lock is being held during a callbacks operations, and this work item that’s queued by this callback tries to open some handle to whatever resource… but opening a handle to that resource triggers filters, and those filters spin up additional work to also check this <something loaded>, but you’re not releasing a resource, or taking too long to complete because you’re waiting still waiting on this resource/event to become available/signaled you risk deadlocking the system. 

if you’re only doing dev on toy systems without third-party products, you may not have an immediate issue… still terrible practice to believe that execution time of those callbacks doesn’t matter.

[2025-02-27 03:44] James: [replying to daax: "execution time for callbacks of the common object ..."]
really? now im curious, in what sense? cuz ive done lots of mischevious things that take a long time in them before

[2025-02-27 03:44] James: in the sense that stuff will just start to slow down? or an actual timer somewhere that panics after a bit?

[2025-02-27 03:45] daax: [replying to James: "really? now im curious, in what sense? cuz ive don..."]
See the above message.

[2025-02-27 03:46] James: ohhh ok yeah

[2025-02-27 03:46] daax: [replying to James: "really? now im curious, in what sense? cuz ive don..."]
and yes, so have I, on toy systems it’s not unusual to avoid any adverse interactions. 

> if you’re only doing dev on toy systems without third-party products, you may not have an immediate issue… still terrible practice to believe that execution time of those callbacks doesn’t matter.

[2025-02-27 03:46] daax: when third parties join in, you have to be very careful.

[2025-02-27 03:46] daax: it’s a very frustrating thing

[2025-02-27 03:47] James: well, would those third parties ALSO have to be doing something considered "bad practice" for it to cause a deadlock?

[2025-02-27 03:47] daax: [replying to James: "well, would those third parties ALSO have to be do..."]
no

[2025-02-27 03:47] James: which ofc they will be*

[2025-02-27 03:47] James: and you're right its bad to assume that they wont

[2025-02-27 03:48] daax: they do not unfortunately (even if they are … let’s be real, but it doesn’t need to be a mutual-fault event)

[2025-02-27 09:56] Matti: [replying to daax: "other things on the system may not play nice. it’s..."]
these are definitely the most common reasons for things taking too long in drivers in general really, but definitely including in callbacks (since you're blocking everything else waiting on this resource to become (un)available)

[2025-02-27 09:58] Matti: but I'd say <@234331837651091456>'s example of registry callbacks is a good example of a type of callback that just gets invoked extremely frequently, relative to loadimage or object callbacks for example

[2025-02-27 09:59] Matti: so they have to be fast simply because of the overwhelming combined volume/amount of them

[2025-02-27 10:00] Matti: they're like mini-ISRs almost in this respect

[2025-02-27 10:02] Matti: generally what you do in an ISR is do the absolute minimum needed to return from the interrupt, and queue a DPC for any additional processing that can be done asynchronously

[2025-02-27 10:03] Matti: and then repeating this pattern, in the DPC it is not uncommon to queue an APC for work that needs to be done at lower IRQL, again with an additional delay incurred (but only for yourself, not the whole system)