# January 2025 - Week 4
# Channel: #programming
# Messages: 59

[2025-01-20 17:33] luci4: Is there any case in which `NtQueryDirectoryFile` does *not* return `.` and `..` as the first two `FILE_DIRECTORY_INFORMATION` entries?

[2025-01-20 17:33] luci4: It seems to do so for every directory, including empty ones. I don't think there's any edge case in which it behaves differently (?)

[2025-01-20 17:48] diversenok: The root of the drive is an example

[2025-01-20 17:49] diversenok: Otherwise, I think it's up to the filesystem driver

[2025-01-20 17:52] diversenok: Another example would be when passing the filename filter to `NtQueryDirectoryFile`

[2025-01-20 19:09] diversenok: [replying to diversenok: "The root of the drive is an example"]
This also applies to mount points that redirect to volume roots, by the way. Which means it's impossible to predict just from the path name

[2025-01-20 19:34] luci4: [replying to diversenok: "This also applies to mount points that redirect to..."]
Ah well the first time I'm calling `NtQueryDirectoryFile` is on a `\Device\HarddiskVolumeX\`

[2025-01-20 19:35] luci4: Guess I will keep my reparse point check

[2025-01-20 19:37] diversenok: Not all reparse points are visible though

[2025-01-20 19:41] diversenok: i.e., it's possible to have `C:\SomeDirectory` that will not look like a reparse point (neither opening it with `FILE_OPEN_REPARSE_POINT` nor enumerating its parent will show it), yet it will not have `.` and `..` files because it gets redirected to a mounted location

[2025-01-20 19:43] diversenok: Mounted folders in Windows Sandbox are an example

[2025-01-21 15:41] Torph: on Windows I can pass a write watch flag to VirtualAlloc, then use GetWriteWatch to check the dirty bits of pages in a range. is there anything similar available on Linux? granularity isnt very important for me, it just makes things very easy if I can sync large buffers without forcing all writes through a special interface

[2025-01-21 15:42] Torph: I thought about emailing the kernel people, but I don't want to bother them unless it really doesn't exist

[2025-01-21 16:36] daax: [replying to Torph: "on Windows I can pass a write watch flag to Virtua..."]
pagemap might be worth trying

[2025-01-21 16:36] daax: <https://www.kernel.org/doc/Documentation/vm/pagemap.txt>

[2025-01-21 16:44] Torph: haha I just found that too, thanks

[2025-01-21 16:45] Torph: it's a little annoying that I can only clear soft-dirty bits for the entire process at once, but that's fine for my use case and I can work around it if it comes up

[2025-01-21 17:55] Torph: 🤔 I think I'm doing my page lookup wrong or something... it seems to always return clean, even if I write to every byte in the page

[2025-01-21 18:03] Torph: even when I have access to the PFN it doesn't show up as dirty, so doesn't seem like a permissions issue. also doesn't seem to matter whether I use `calloc` or `mmap` to get the memory

[2025-01-21 18:06] Torph: when I use `mmap` settings that only commit pages when used, the flags change after writing, so I think I have the correct page entry but it's not being marked for some reason...
[Attachments: 2025-01-21_13-05.png]

[2025-01-21 18:42] Torph: I found a proposal to implement another write-tracking scheme on top of the existing one, but it seems like they never merged it <https://www.cse.iitk.ac.in/users/kparun/ldt.pdf>

[2025-01-21 19:23] Torph: okay it works on a university debian machine, and after I got a stock kernel from my package manager it was fine on my laptop. so my custom kernel build was just fucked up

[2025-01-23 06:53] rin: Is anyone aware of an api call that can determine if clipboard content is a file or does that need to be determined within an application using something  like magic numbers.

[2025-01-23 09:10] Brit: Isclipboardformatavailable

[2025-01-25 01:46] rin: question about `CreateWindowExA`  in the documentation it says that both classname and windowname are optional but when I try to pass zero as an argument I get a incorrect parameters error. also are cursor and icon required fields when filling out the structure for use with createwindowexa?

[2025-01-25 01:49] rin: reason I am creating a window is to receive shutdown notifications, I am reading https://phrack.org/issues/71/3#article because its similar to what I am doing but its in asm so harder to comprehend.
[Embed: .:: Phrack Magazine ::.]
Phrack staff website.

[2025-01-25 01:52] .: window name can be null

[2025-01-25 01:52] .: lpClassName must reference either a registered window class name or a predefined system class

[2025-01-25 01:52] .: 
[Attachments: image.png]

[2025-01-25 01:54] rin: can hInstance be null

[2025-01-25 01:55] .: no

[2025-01-25 01:55] rin: says optional but idk

[2025-01-25 01:55] rin: [replying to .: "no"]
kk

[2025-01-25 01:55] .: cursor and icon are also not required

[2025-01-25 01:56] .: [replying to rin: "says optional but idk"]
weird. it's primarily used to associate the window with the executable module

[2025-01-25 01:58] .: the function will still succeed tho

[2025-01-25 01:58] .: it should only fail if you dont pass a class name

[2025-01-25 02:00] .: without HINSTANCE
[Attachments: image.png]

[2025-01-25 02:00] .: without class name
[Attachments: image.png]

[2025-01-25 02:03] rin: hmm

[2025-01-25 02:06] rin: ```golang
instanceHandle, _, _ := getModuleHandle.Call(0)

    hwnd, _, _ := createWindowExA.Call(0, uintptr(unsafe.Pointer(&[]byte("static\x00")[0])), 0, WS_OVERLAPPED, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, uintptr(instanceHandle), 0)
``` this works for me.

[2025-01-25 11:20] x86matthew: in win32 the hInstance value is stored in a lookup table when registering a class, it doesn't even need to be a valid base address (which is why 0 works)

[2025-01-25 11:20] x86matthew: classes are essentially identified as "hInstance.class_name" which allows multiple DLLs within a process to register classes with the same name

[2025-01-25 11:21] x86matthew: if you pass a valid class name to CreateWindowEx but hInstance of NULL, it ignores the hInstance aspect of the class identifier and uses the first one it finds with a matching name

[2025-01-25 11:21] x86matthew: this was more relevant in win3.x where multiple instances of the same exe shared the same address space

[2025-01-25 11:22] diversenok: And what is NULL class name used for?

[2025-01-25 11:30] rin: [replying to diversenok: "And what is NULL class name used for?"]
Can't have a null class name

[2025-01-25 11:32] diversenok: SAL indicates otherwise

[2025-01-25 11:33] rin: Sal?

[2025-01-25 11:33] diversenok: These annotations they use in SDK: https://learn.microsoft.com/en-us/cpp/code-quality/understanding-sal?view=msvc-170

[2025-01-25 11:33] x86matthew: [replying to diversenok: "And what is NULL class name used for?"]
not sure tbh, msdn does say it's optional but i've never seen it used like that

[2025-01-25 11:35] x86matthew: i'm tempted to say it's wrong but don't quote me on that lol

[2025-01-25 11:39] rin: [replying to diversenok: "SAL indicates otherwise"]
So does documentation but didn't work when I tried to pass null.

[2025-01-25 11:40] rin: Probably something wrong on my end though

[2025-01-25 11:40] diversenok: [replying to rin: "So does documentation but didn't work when I tried..."]
Well, it only indicates that the parameter is not optional in *some* cases

[2025-01-25 11:41] rin: [replying to diversenok: "Well, it only indicates that the parameter is not ..."]
Makes sense

[2025-01-25 12:54] brymko: [replying to x86matthew: "in win32 the hInstance value is stored in a lookup..."]
In win64 the hInstance value is stored in the balls

[2025-01-25 17:12] x86matthew: sounds like the basis of an interesting raymond chen article

[2025-01-25 22:01] Torph: does anyone know why `GetWriteWatch` can only be used on memory allocated with a special `VirtualAlloc` flag? is there some technical reason why they can't modify an already allocated/committed page to get dirty bit information? I was really surprised to read that in the docs, since almost every other property of pages can be modified by `VirtualAlloc` after allocation