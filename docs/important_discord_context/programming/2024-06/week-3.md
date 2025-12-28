# June 2024 - Week 3
# Channel: #programming
# Messages: 44

[2024-06-11 08:48] Deleted User: is there a better way of enumerating current process modules than doing EnumProcessModules with GetCurrentProcess

[2024-06-11 08:52] Koreos: [replying to Deleted User: "is there a better way of enumerating current proce..."]
I don't know if it's better but this https://learn.microsoft.com/en-us/windows/win32/toolhelp/taking-a-snapshot-and-viewing-processes maybe
[Embed: Taking a snapshot, viewing processes - Win32 apps]
This code example retrieves a list of running processes.

[2024-06-11 08:56] Deleted User: yep that seems more suited for my use case as the struct seems to have the module name in it

[2024-06-11 08:56] Deleted User: thanks

[2024-06-11 09:38] dullard: [replying to Deleted User: "is there a better way of enumerating current proce..."]
<@148095953742725120> parse the PEB

[2024-06-11 09:38] dullard: In order memory module list

[2024-06-11 10:43] averageavx512enjoyer: [replying to Deleted User: "is there a better way of enumerating current proce..."]
NtQuerySystemInformation should also work yeah?

[2024-06-11 11:02] Koreos: [replying to dullard: "In order memory module list"]
I just realized that my eyes skipped over the word module and thought he was just doing process enum

[2024-06-11 11:02] Koreos: My bad bt

[2024-06-11 12:08] Matti: [replying to dullard: "<@148095953742725120> parse the PEB"]
go do it yourself

[2024-06-11 12:08] Matti: what the hell man

[2024-06-11 12:08] luci4: lol

[2024-06-11 12:13] dullard: [replying to Matti: "go do it yourself"]
Wait wtf 😂😂

[2024-06-11 12:14] dullard: How did I manage to ping you ??? 😂😂

[2024-06-11 12:14] dullard: lmao

[2024-06-11 12:14] Matti: <:angryping:844034669787742238>

[2024-06-11 12:15] Matti: don't worry, I'm used to it

[2024-06-11 12:15] luci4: You WILL parse the PEB

[2024-06-11 12:15] luci4: 🤣

[2024-06-12 23:07] Deleted User: uhh another question

[2024-06-12 23:07] Deleted User: can i somehow make windows not care about dll dependencies

[2024-06-12 23:07] Deleted User: and not unmap it if it fails

[2024-06-12 23:09] irql: LoadLibraryExW with DONT_RESOLVE_DLL_REFERENCES

[2024-06-12 23:11] Deleted User: thx

[2024-06-13 12:28] Torph: [replying to dullard: "How did I manage to ping you ??? 😂😂"]
muscle memory <:kekw:904522300257345566>

[2024-06-13 17:31] DeltaV: Hello there, anyone knows how to use Windows Hypervisor Platform API ?
I'm running a 4 layer page table (long mode emulation) and trying to catch mmio/page faults (expecting to make it continue running) but can't for the life of me make it exit with `WHvRunVpExitReasonMemoryAccess`
if the `ExceptionExitBitmap` property includes the bit for `WHvX64ExceptionTypePageFault` then it can exit with a `WHvRunVpExitReasonException` of that type.
If the bit isn't set, then it exits with a `WHvRunVpExitReasonUnrecoverableException` instead.
There's basically no doc about this, I checked qemu and other sources but none of them do anything to "enable" it or otherwise.
I had the Hypervisor Platform feature enabled, it didn't work, then I disabled it, still didn't work, now i have a windows update pending and it wants to reboot to change the feature back to enabled :/
W10 22H2 btw

[2024-06-14 15:39] rzr (pending deletion, no dm): hey

[2024-06-14 15:40] rzr (pending deletion, no dm): *this is a trap don't answer me*

[2024-06-14 15:49] Deleted User: [replying to rzr (pending deletion, no dm): "*this is a trap don't answer me*"]
what

[2024-06-14 15:52] luci4: [replying to Deleted User: "what"]
Oh come on you ruined the perfect sting operation

[2024-06-14 15:52] rzr (pending deletion, no dm): [replying to luci4: "Oh come on you ruined the perfect sting operation"]
yes fuck this guy

[2024-06-14 15:53] rzr (pending deletion, no dm): [replying to Deleted User: "what"]
<:kappa:1115968816812392470>

[2024-06-14 17:39] Matti: [replying to DeltaV: "Hello there, anyone knows how to use Windows Hyper..."]
> anyone knows how to use Windows Hypervisor Platform API ?
if someone does, it sure as fuck isn't me

I just know that I also never managed to get qemu to work using the WHVP (whpx accel in qemu terms) on windows 10
whereas rebooting into windows 11 and executing the exact same qemu command, it would work

[2024-06-14 17:41] Matti: this failure was a different one and happened very early in WHVP init code in qemu, so I'm not saying this is definitely related... but like, it could be

[2024-06-14 18:04] DeltaV: welp that's not great

[2024-06-14 18:05] DeltaV: I'd rather not change os

[2024-06-14 18:05] DeltaV: thanks tho

[2024-06-15 19:14] DeltaV: turns out i was just being a massive idiot

[2024-06-15 19:15] DeltaV: no entry at all in the page table hierarchy -> pagefault exception
an entry but no matching guest physical -> memoryaccess

[2024-06-15 19:17] DeltaV: but then i have no idea how you're supposed to tell which address did the fault in the first case

[2024-06-16 16:12] donna🤯: Hello, recently I have been overhauling some internal data structures in my anti cheat (https://github.com/donnaskiez/ac) and I am looking for some feedback and/or ideas regarding how I’ve gone about it.  For an anticheat, its important to keep track of threads, processes and associated images. Currently I have 2 core data structures: 

1. The first being a red-black tree which keeps track of threads using the threadid as the key. Efficient insertions and deletions without the added balance requirements of something like an AVL tree.
2. A striped hashmap with buckets, each list entry then containing a list of images associated with that process. Recently created processes are inserted at the head (as its very common for process to start and quickly stop)

Now so far this works great, it's efficient and allows me safe access to any kernel object and its information. However, something I have been wondering about is whether it's worth combining these two into a singular data structure that tracks everything? My main concern here is that due to the nature of threads rapidly being created and destroyed, it could lead to starvation when attempting to acquire the lock - the solution here would be some reader/writer lock but I think for this scenario its not as feasible. Essentially, I'm not sure if combining everything into a single data structure, even though it helps for indexing reasons (i.e., process -> threads + images), is worth the potential performance sacrifice due to the constant lock contention since threads are spawned and destroyed rapidly and constantly. The frequent contention for locks might outweigh the benefits of having a unified structure, leading to potential bottlenecks in the system.

Any ideas? What I’ve got right now works great and I’m happy with it I’m just wondering if there are potentially better and cleaner solutions. Thanks in advance 🙂

[2024-06-16 17:39] JustMagic: [replying to donna🤯: "Hello, recently I have been overhauling some inter..."]
Generally speaking tracking processes and threads is not a high volume or expensive thing to do

[2024-06-16 17:41] JustMagic: You'd probably be much better off simplifying the code. Reduce number of locks you have, use refcounting on the objects instead, because I'd imagine 90% of the time you'll be simply looking at immutable data inside your data structures.

[2024-06-16 23:55] donna🤯: [replying to JustMagic: "You'd probably be much better off simplifying the ..."]
main reason is safe enumeration of running threads / processes (since we dont have "real" access to the locks that govern those structures)