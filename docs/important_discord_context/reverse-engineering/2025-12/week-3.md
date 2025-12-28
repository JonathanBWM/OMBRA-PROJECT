# December 2025 - Week 3
# Channel: #reverse-engineering
# Messages: 192

[2025-12-15 08:42] 0xBera: [replying to the horse: "ever since i heard of wiztree i see no reason for ..."]
yeah wiztree is way better. Still, folks usually don't like CLI tools and they prefer to use a cool GUI application.

[2025-12-15 08:43] Lyssa: wiztree is a GUI application

[2025-12-15 08:46] 0xBera: [replying to Lyssa: "wiztree is a GUI application"]
right, I remember it being a CLI application <:kekw:904522300257345566>

[2025-12-15 08:50] 0xBera: i'm going off topic here, but if you want to have a massive headache and take a trip down memory lane, i recommend doing a pwn challenge on HTB called “Blinded”. I've been working on it for a while and I've found something to break, but it's really crazy😂

[2025-12-17 16:08] impl: when dealing with cr3 trashing, what should i set the host cr3 to? any kernel code can trash the pagetables and in my current implementation my host and guest share the same cr3.

[2025-12-17 17:25] plpg: Most hypervisors I saw restore Host CR3 on exit

[2025-12-17 17:25] plpg: they keep them separate

[2025-12-17 18:54] iPower: [replying to impl: "when dealing with cr3 trashing, what should i set ..."]
create your own cr3

[2025-12-17 18:54] iPower: make sure the OS can't touch the memory for your page tables

[2025-12-17 18:55] iPower: but don't do only PML4/PML5. rebuild every single level

[2025-12-17 18:59] iPower: I have a custom memory allocator which I use to allocate and manage the memory used by the hypervisor

[2025-12-17 18:59] impl: you mean i create my own pagetables?

[2025-12-17 18:59] iPower: I ensure windows can't touch this memory

[2025-12-17 18:59] iPower: [replying to impl: "you mean i create my own pagetables?"]
yes. every single level

[2025-12-17 18:59] iPower: pml5, pml4, pdpt, etc...

[2025-12-17 19:00] impl: [replying to iPower: "I have a custom memory allocator which I use to al..."]
is your allocator uefi based or something like that?

[2025-12-17 19:01] iPower: both my type 1 and type 2 share the same allocator. only difference is how I allocate memory and ensure os kernel doesn't touch

[2025-12-17 19:01] impl: oh

[2025-12-17 19:01] Hunter: in other words perform a deep copy of the memory mappings, since the guest page tables entries would be modified which wont affect ur copy

[2025-12-17 19:02] iPower: [replying to Hunter: "in other words perform a deep copy of the memory m..."]
this is exactly what I do. some people also create identity mapped pages... it all depends on what are your needs and your current implementation

[2025-12-17 19:03] iPower: as long as all of the page tables are controlled by the host and OS kernel can't touch them, whatever approach is fine

[2025-12-17 19:04] impl: i get the create your own pagetables part but i dont quite get what memory should be mapped, iiuc i map the entire memory to my host pagetables?

[2025-12-17 19:04] iPower: i only map host code and allocator pool. nothing more than that

[2025-12-17 19:05] impl: by host code you mean your svm/vtx driver right?

[2025-12-17 19:06] iPower: yeah. basically covering everything

[2025-12-17 19:06] impl: thanks <@789295938753396817> <@809608177976344607>

[2025-12-17 19:06] iPower: allocator pool which is used by host, code pages, idt, gdt, etc.

[2025-12-17 19:10] iPower: then you can optionally reserve some pages in host for accessing memory

[2025-12-17 19:10] iPower: my current setup has a per-core PTE

[2025-12-17 20:46] iPower: also btw even when you're not dealing with anti-cheats it's a good practice to isolate host from guest

[2025-12-17 20:47] iPower: isolating idt, gdt, tss, page tables is the start

[2025-12-17 20:48] iPower: then you must validate guest inputs, ensure guest doesn't try to corrupt host state

[2025-12-17 20:48] iPower: and all of that fun stuff

[2025-12-20 04:17] DanyDollaro: [replying to iPower: "both my type 1 and type 2 share the same allocator..."]
May I know how are you allocating memory for your type 1 hypervisor? I actually thought about that, from a uefi environment it can be achieved manipulating the uefi memory map passed to the kernel, instead in the case the hypervisor is suppose to virtualize the current host it could be achieved unlinking the required PFN from the PFN database, I do not know much about its inner working so this can be highly unsafe

[2025-12-20 05:05] iPower: [replying to DanyDollaro: "May I know how are you allocating memory for your ..."]
there are a lot of ways to allocate your memory and ensuring windows does not touch it. if you take a look at initialization + UEFI specs it becomes pretty clear.

[2025-12-20 05:16] iPower: see: `Table 7.10: Memory Type Usage after ExitBootServices()` in `7.2 Memory Allocation Services`

[2025-12-20 05:16] iPower: and combine that with init code

[2025-12-20 05:17] iPower: and you can also do some quick experiments

[2025-12-20 05:45] DanyDollaro: [replying to iPower: "see: `Table 7.10: Memory Type Usage after ExitBoot..."]
Thanks, I'll definitely take a look at them 👍

[2025-12-20 21:39] Shanks: [replying to Matti: "so that's great"]
My friend finally did it : https://github.com/provigz/ZoneInternetGamesServer/discussions/11 , https://github.com/provigz/ZoneInternetGamesServer/
[Embed: Hosting a public Internet Games Server: Anyone can join! · provigz...]
I am currently hosting a public Internet Games Server (v2.0 BETA 3) that anyone can connect to! Host: 34.10.75.175 Port: 28805 (the default for Windows XP/ME games) This thread will feature updates...
[Embed: GitHub - provigz/ZoneInternetGamesServer: Bringing back the functio...]
Bringing back the functionality of the Internet Games, included in Windows 7 and XP/ME. - provigz/ZoneInternetGamesServer

[2025-12-21 11:15] outletproblems: hi all, am doing some RE here and researching so i get a good foundation, i cannot exactly understand vtables, to my understanding are they essentially just a way to index virtual functions?

[2025-12-21 11:16] outletproblems: take:
```c
            (*(void (__thiscall **)(_DWORD))(**(_DWORD **)(local_player + 0x364) + 0x48))(*(_DWORD *)(local_player + 0x364));
```

[2025-12-21 11:16] outletproblems: `0x364` is a weapon class, `0x48` is a function pointer within the weapon class

[2025-12-21 11:18] outletproblems: 
[Attachments: image.png, image.png]

[2025-12-21 11:18] Lyssa: <https://www.learncpp.com/cpp-tutorial/the-virtual-table>
<https://youtu.be/sAZ6JvLPx_0>
these two should help I think

[2025-12-21 11:18] Lyssa: but yeah you've got the right idea

[2025-12-21 11:18] outletproblems: thank you!

[2025-12-21 11:22] Lyssa: that code is basically doing something like
```
Weapon* weapon = player->weapon;  // +0x364
void** vtable = *(void***)weapon; // vtable ptr at offset 0
auto fn = vtable[0x48/4]; 
fn(weapon); // 0x48/4 = 18th function in the vtable
```

[2025-12-21 11:22] Lyssa: I think

[2025-12-21 11:22] Lyssa: my understanding is kind of sloppy too

[2025-12-21 11:23] Lyssa: 0x48 being the offset there means it's a specific virtual method in that weapon class's interface, so if you find the vtable in the binary you can map out all the virtual functions

[2025-12-21 11:24] outletproblems: [replying to Lyssa: "that code is basically doing something like
```
We..."]
do we divide by 4 here as to adjust for indexing?

[2025-12-21 11:24] outletproblems: since x86

[2025-12-21 11:24] Lyssa: yeah to get the index

[2025-12-21 11:25] Lyssa: since each pointer is 4 bytes

[2025-12-21 11:25] outletproblems: exactly ok this makes sense

[2025-12-21 11:26] outletproblems: ahh right then , so:
`(*(void (__thiscall **)(_DWORD))(**(_DWORD **)(local_player + 0x364) + 0x48))` is the function and
`(*(_DWORD *)(local_player + 0x364));` is the parameter, which is the weapon

[2025-12-21 11:26] outletproblems: i get it

[2025-12-21 11:26] outletproblems: or argument ^ sorry

[2025-12-21 11:26] outletproblems: i have read the learncpp blog and I get it better now thank you

[2025-12-21 11:26] outletproblems: however, in the screenshot I sent you can see in Cheat Engine in the function debugging i have jumped to the address, and it is simply just `add [eax],al`

[2025-12-21 11:27] outletproblems: i am confused here

[2025-12-21 11:27] outletproblems: this:
[Attachments: image.png]

[2025-12-21 11:27] Lyssa: that does not look correct..?

[2025-12-21 11:27] Lyssa: I haven't used cheat engine before so idk

[2025-12-21 11:28] Lyssa: I'll leave it to someone else

[2025-12-21 11:29] outletproblems: [replying to Lyssa: "I haven't used cheat engine before so idk"]
no fair enough :p

[2025-12-21 11:30] outletproblems: [replying to Lyssa: "I'll leave it to someone else"]
thank you a lot for the help still i appreciate it

[2025-12-21 11:30] ia32e: wrong address, empty space

[2025-12-21 11:30] outletproblems: i think so too

[2025-12-21 11:30] Lyssa: yeah

[2025-12-21 11:30] Lyssa: offsets do not directly "translate" like that

[2025-12-21 11:30] Lyssa: idk how else to say it

[2025-12-21 11:31] ia32e: watcha trying to do?

[2025-12-21 11:31] outletproblems: do you mean, they need to be dereferenced? or relative to the base?

[2025-12-21 11:31] Lyssa: I remember copying memory address from x64dbg and pasting it into IDA and expecting it to work

[2025-12-21 11:31] outletproblems: [replying to ia32e: "watcha trying to do?"]
find the function it is calling dynamically

[2025-12-21 11:31] outletproblems: [replying to Lyssa: "I remember copying memory address from x64dbg and ..."]
oh yeah ofc

[2025-12-21 11:31] Lyssa: so it's probably similiar situation here

[2025-12-21 11:31] outletproblems: i am just getting used to that

[2025-12-21 11:31] outletproblems: with all the base addresses and all xd

[2025-12-21 11:31] outletproblems: still struggling a bit, but getting better at noticing what is what

[2025-12-21 11:31] outletproblems: so I have this:
```c
(*(void (__thiscall **)(_DWORD))(**(_DWORD **)(local_player + 0x364) + 0x48))(*(_DWORD *)(local_player + 0x364));
```

[2025-12-21 11:32] ia32e: show me your code

[2025-12-21 11:32] outletproblems: the above is the code

[2025-12-21 11:32] outletproblems: 
[Attachments: image.png]

[2025-12-21 11:32] ia32e: it's part of it

[2025-12-21 11:32] outletproblems: ah yes sure

[2025-12-21 11:32] outletproblems: fair point

[2025-12-21 11:32] outletproblems: ```c
         if ( v481 )
          {
            (*(void (__thiscall **)(_DWORD))(**(_DWORD **)(local_player + 0x364) + 0x48))(*(_DWORD *)(local_player + 0x364));
            v201 = v481;
            *(_DWORD *)(local_p + 0x310) = global_timer_maybe;
            *(_DWORD *)(local_p + 0x368) = v201;
            (*(void (__thiscall **)(size_t, _DWORD))(*(_DWORD *)v201 + 0x44))(v201, 0);// i think calls sub_4C8900
            local_p = local_player;
          }
```
[Attachments: image.png]

[2025-12-21 11:33] ia32e: how you getting the local player addr?

[2025-12-21 11:33] outletproblems: local_player stores the address of it, so effectively it is a pointer to the local player

[2025-12-21 11:34] outletproblems: its static

[2025-12-21 11:34] ia32e: so you just resolving it by an offset?

[2025-12-21 11:34] outletproblems: the local player no

[2025-12-21 11:34] outletproblems: 
[Attachments: image.png]

[2025-12-21 11:34] ia32e: wdym by no

[2025-12-21 11:34] outletproblems: maybe I am misunderstanding you here

[2025-12-21 11:34] outletproblems: `0x58AC00` is a pointer to the player's address

[2025-12-21 11:35] ia32e: it's a pointer

[2025-12-21 11:35] outletproblems: yes

[2025-12-21 11:35] ia32e: and how to get an offset?

[2025-12-21 11:36] outletproblems: i am not understanding, to what? any offset?

[2025-12-21 11:36] ia32e: offset to local player

[2025-12-21 11:36] outletproblems: add the module base

[2025-12-21 11:36] outletproblems: if i am understanding you right

[2025-12-21 11:36] ia32e: you forgot to do something, don't you?

[2025-12-21 11:36] ia32e: before adding the mod base

[2025-12-21 11:36] outletproblems: maybe :p

[2025-12-21 11:36] outletproblems: ```cpp
        uintptr_t player_base = module_address + 0x18AC00;
```

[2025-12-21 11:37] ia32e: what you need to do?

[2025-12-21 11:37] outletproblems: this is how i do it

[2025-12-21 11:37] outletproblems: Resolve it? Like derefence

[2025-12-21 11:37] ia32e: abs - base = off

[2025-12-21 11:37] ia32e: so

[2025-12-21 11:37] outletproblems: so the offset is 0x18AC00

[2025-12-21 11:37] outletproblems: right

[2025-12-21 11:38] ia32e: hm

[2025-12-21 11:38] ia32e: it is your code?

[2025-12-21 11:38] outletproblems: the one I am reversing no, it is of assault cube

[2025-12-21 11:38] outletproblems: the latest edition i think

[2025-12-21 11:39] ia32e: i'm talking bout cpp

[2025-12-21 11:39] outletproblems: ah yes

[2025-12-21 11:39] outletproblems: yes

[2025-12-21 11:39] outletproblems: one moment

[2025-12-21 11:39] outletproblems: example here:
```cpp
        uintptr_t module_address = 0x400000;
        uintptr_t player_base = module_address + 0x18AC00;
        uintptr_t entity_list_base = module_address + 0x18AC04;
        uintptr_t player_count_base = module_address+ 0x18AC0C;
```

[2025-12-21 11:40] outletproblems: i can read the module address; but this is hard coded for now

[2025-12-21 11:40] ia32e: yuh

[2025-12-21 11:40] ia32e: there is ASLR

[2025-12-21 11:40] ia32e: it is linux or windows?

[2025-12-21 11:40] outletproblems: windows programme

[2025-12-21 11:41] outletproblems: these are all static here

[2025-12-21 11:41] outletproblems: no ASLR

[2025-12-21 11:41] outletproblems: but i have dealt with a bit of ASLR before

[2025-12-21 11:41] ia32e: module_base = uintptr_t(GetModuleHandleA(0));

if it's in main mod

[2025-12-21 11:43] outletproblems: yes this should return `0x400000`

[2025-12-21 11:43] outletproblems: but

[2025-12-21 11:43] outletproblems: remember, the question i asked was not related to my code

[2025-12-21 11:43] outletproblems: i was looking in ReClass and CE

[2025-12-21 11:43] ia32e: ye, just resolving dynamically is much better

[2025-12-21 11:44] outletproblems: agreed, thats a given :p

[2025-12-21 11:44] ia32e: however, print target address from your module

[2025-12-21 11:44] ia32e: and jump on it

[2025-12-21 11:44] ia32e: done?

[2025-12-21 11:45] outletproblems: let me try

[2025-12-21 11:45] ia32e: i must have somewhere this

[2025-12-21 11:46] ia32e: ```c
void alloc_console() {

        // https://learn.microsoft.com/ru-ru/windows/console/allocconsole.
        AllocConsole();

        // redirect application output to our stream.
        // https://en.cppreference.com/w/c/io/freopen.
        FILE* file = nullptr;
        freopen_s(&file, "CONOUT$", "w", stdout);

        // utf-8 support.
        SetConsoleOutputCP(65001);

    }
```

[2025-12-21 11:46] ia32e: and then just do a printf thingy

[2025-12-21 11:46] outletproblems: yes i have this

[2025-12-21 11:47] outletproblems: i am setting up the dereferencing and all one sec

[2025-12-21 11:47] outletproblems: since pointer indexing bites me in ass for now xd still figuring it out

[2025-12-21 11:47] ia32e: have no idea whatcha talking bout but yeah sure

[2025-12-21 11:55] outletproblems: so i got

[2025-12-21 11:55] outletproblems: `0x411880`

[2025-12-21 11:55] outletproblems: from

[2025-12-21 11:55] outletproblems: ```cpp
    auto weapon = *reinterpret_cast<uintptr_t**>(local_player + 0x364);
    auto vtable = *reinterpret_cast<uintptr_t**>(weapon);

    uintptr_t fnc = *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(vtable) + 0x48);

    std::cout << std::hex << std::uppercase << "0x" << fnc << "\n";
```

[2025-12-21 11:56] ia32e: aw men these cpp prints

[2025-12-21 11:56] ia32e: printf("0x%p", nigptr);

[2025-12-21 11:56] ia32e: hellye

[2025-12-21 11:56] outletproblems: pffffttt

[2025-12-21 11:56] outletproblems: then next time i will use puts

[2025-12-21 11:56] outletproblems: xD

[2025-12-21 11:56] ia32e: fair

[2025-12-21 11:56] outletproblems: this gives me this

[2025-12-21 11:56] outletproblems: 
[Attachments: image.png]

[2025-12-21 11:57] ia32e: [replying to outletproblems: "so i got"]
print each var separately, and check for the correctness

[2025-12-21 11:57] outletproblems: I don't get it, so essentially this function just returns 0?

[2025-12-21 11:57] ia32e: to see where issue is belongs to

[2025-12-21 11:57] outletproblems: but this is correct

[2025-12-21 11:57] outletproblems: since the function is well defined

[2025-12-21 11:57] ia32e: it is not

[2025-12-21 11:57] outletproblems: hmm ok

[2025-12-21 11:58] ia32e: prolly vtable miss hit

[2025-12-21 11:58] ia32e: e.g wrong index hit

[2025-12-21 11:58] ia32e: bcoz looks like class constructor/deconstructor

[2025-12-21 11:58] outletproblems: maybe it is?

[2025-12-21 11:59] outletproblems: you can see from code snippet i sent, maybe it deconstructs weapon (class), and then sets new weapon?

[2025-12-21 11:59] outletproblems: I think this function in total is a match/start or match updater - and this seems to be a weapon swap or such

[2025-12-21 12:00] ia32e: i mean it's just something from rtti, i have no clue what it is

[2025-12-21 12:00] ia32e: no idb and proper code or dbg out is provided, i'm just guessing

[2025-12-21 12:00] ia32e: but happy to help in case

[2025-12-21 12:01] outletproblems: that is fair point

[2025-12-21 12:01] outletproblems: thank you for the help too i appreciate it

[2025-12-21 12:01] outletproblems: i think i will try figure out what is happening here, i am getting somewhere with the info you gave me

[2025-12-21 12:01] ia32e: i didn't do anything yet

[2025-12-21 12:01] outletproblems: yes you did

[2025-12-21 12:01] ia32e: provide info and then we'll see

[2025-12-21 12:02] outletproblems: am getting this now:
```
Thread began
Weapon: 0x00747AE0
Vtable: 0x00555338
Fnc: 0x411880

```

[2025-12-21 12:02] outletproblems: o

[2025-12-21 12:03] ia32e: no idea why vtable is so far away

[2025-12-21 12:03] outletproblems: no this must be wrong

[2025-12-21 12:05] outletproblems: let me see