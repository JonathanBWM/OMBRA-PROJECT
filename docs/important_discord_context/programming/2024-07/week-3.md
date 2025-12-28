# July 2024 - Week 3
# Channel: #programming
# Messages: 92

[2024-07-17 01:48] Deleted User: <@221286210851569664>  https://github.com/JustasMasiulis/xorstr/tree/master
[Embed: GitHub - JustasMasiulis/xorstr: heavily vectorized c++17 compile ti...]
heavily vectorized c++17 compile time string encryption. - JustasMasiulis/xorstr

[2024-07-17 01:48] Deleted User: i don't get how u ensure the string literals aren't placed in .rdata here

[2024-07-17 01:49] JustMagic: [replying to Deleted User: "i don't get how u ensure the string literals aren'..."]
https://github.com/JustasMasiulis/xorstr/blob/066c64eea5104f4e3cfbc49e39031400e086425a/include/xorstr.hpp#L88

[2024-07-17 01:49] Deleted User: here is the implementation but could u elaborate?
```cpp
        // forces compiler to use registers instead of stuffing constants in rdata
        XORSTR_FORCEINLINE std::uint64_t load_from_reg(std::uint64_t value) noexcept
        {
#if defined(__clang__) || defined(__GNUC__)
            asm("" : "=r"(value) : "0"(value) : );
            return value;
#else
            volatile std::uint64_t reg = value;
            return reg;
#endif
        }
```

[2024-07-17 01:49] Deleted User: yeah

[2024-07-17 01:49] JustMagic: MSVC is basically just hoping for the best and unless something changed, volatile will work for this purpose

[2024-07-17 01:50] JustMagic: as for clang you declare an input+output register operand of inline assembly that does nothing

[2024-07-17 01:51] JustMagic: the inline assembly is less jank than volatile because it's kind of explicit in requirements and won't cause double loads like volatile

[2024-07-17 01:51] Deleted User: [replying to JustMagic: "MSVC is basically just hoping for the best and unl..."]
not sure i follow, might be because of my lack of understanding but why will `volatile` means it's not just chucked into .rdata and a register is used?

[2024-07-17 01:52] Deleted User: this is like the only string obfuscator i've found that doesn't place in .rdata

[2024-07-17 01:53] JustMagic: [replying to Deleted User: "not sure i follow, might be because of my lack of ..."]
generally speaking you're right, however with volatile you have stricter code requirements, so the compiler has less choices of what to do. Main thing is that it can't vectorize the accesses to that variable, so doing a more cost-effective SIMD load from RDATA to stack is not allowed. At that point MSVC just decides to put it in registers. The approach with GNU style assembly is much more explicit and I'd think reliable.

[2024-07-17 02:01] Deleted User: [replying to JustMagic: "generally speaking you're right, however with vola..."]
ah i see, but what do u mean it can't vectorise accesses to that variable? isn't it possible, but for optimisation purposes MSVC will decide to use registers?

[2024-07-17 02:01] JustMagic: [replying to Deleted User: "ah i see, but what do u mean it can't vectorise ac..."]
it isn't possible because the variable has been marked volatile

[2024-07-17 02:01] JustMagic: basically all optimizations on it will be disabled because of that

[2024-07-17 02:02] Deleted User: [replying to JustMagic: "basically all optimizations on it will be disabled..."]
ah, so what exactly do you mean by 'vectorise' here

[2024-07-17 02:02] JustMagic: [replying to Deleted User: "ah, so what exactly do you mean by 'vectorise' her..."]
vectorized == 16 byte loads from rdata into reg then onto stack (depending on your target you could ofc get 32 byte loads with AVX, etc, etc)

[2024-07-17 02:03] Deleted User: [replying to JustMagic: "vectorized == 16 byte loads from rdata into reg th..."]
oh okay

[2024-07-17 02:03] Deleted User: i kindof understand

[2024-07-17 02:03] Deleted User: thanks very much

[2024-07-17 02:04] Deleted User: i think it's just i gotta learn more about c++

[2024-07-17 02:04] JustMagic: vectorization is just one of the main optimizations compilers make that rewrite your code to use SIMD instructions

[2024-07-17 02:05] Deleted User: [replying to JustMagic: "vectorization is just one of the main optimization..."]
right, because simd instructions allow u to operate on loads of data simultaneously so it can speed up the execution of ur code?

[2024-07-17 02:06] JustMagic: Yup. And the compiler will see that it's more cost-efficient/faster to do the loads from RDATA than to do the loads using immediate values. That piece of code tries to prevent this.

[2024-07-17 02:06] Deleted User: [replying to JustMagic: "Yup. And the compiler will see that it's more cost..."]
Ahhh makes sense, thank you

[2024-07-17 02:07] Deleted User: i mean, so, couldn't u just disable optimisations to achieve the same thing?

[2024-07-17 02:15] JustMagic: [replying to Deleted User: "i mean, so, couldn't u just disable optimisations ..."]
it might be possible to use pragmas to control optimization fine grained, but I haven't tested it much. You definitely don't want to disable optimizations across your whole program though

[2024-07-17 02:20] Deleted User: [replying to JustMagic: "it might be possible to use pragmas to control opt..."]
👍 thanks

[2024-07-18 22:42] eura4002: I'm testing an AMD hypervisor open-source code. In this open-source code, if an MSR is invalid, it injects a General Protection Exception (GPE). The hypervisor runs, but a blue screen occurs after 30 seconds. I've tried running it on both the host PC and VMware, but the blue screen only occurs on the host PC. And when I don't inject the GP, no blue screen occurs. Do you know anything about this?

[2024-07-18 22:42] eura4002: 
[Attachments: image.png]

[2024-07-18 23:52] Deleted User: [replying to eura4002: ""]
hyper-v msr range

[2024-07-18 23:53] Deleted User: if its inbetween these u should also inject gp
[Attachments: image.png]

[2024-07-18 23:53] eura4002: This is the range of amd version.

[2024-07-18 23:54] Deleted User: [replying to eura4002: "This is the range of amd version."]
no this is independent from intel/amd msr range

[2024-07-18 23:55] Deleted User: mhm actually thats weird if it happens on host not vmware i had issues with vmware only

[2024-07-18 23:55] Deleted User: its probably not related to that then

[2024-07-20 02:30] grb: is it possible to manually page in a paged out memory just by doing some page hackery?

[2024-07-20 03:03] JustMagic: [replying to grb: "is it possible to manually page in a paged out mem..."]
what does that even mean

[2024-07-20 03:41] grb: [replying to JustMagic: "what does that even mean"]
my physmem translation kept failing bcs the VA that im trying to translate is paged out, how can i page it in?

[2024-07-20 03:45] grb: what i tried currently is to copy the PML4 entry of the target VA (other process) im trying to translate to my current process dirbase, and i tried to access the VA directly, and it always gave me access violation

[2024-07-20 03:50] grb: idk why? is it bcs of the TLB? or something else?

[2024-07-20 03:51] grb: [replying to grb: "what i tried currently is to copy the PML4 entry o..."]
this method works and i successfully accessed the VA from my own process if i page in the memory from the original process (just by reading/writing from that memory)

[2024-07-20 03:57] snowua: [replying to grb: "is it possible to manually page in a paged out mem..."]
You want to take a look at how pages are paged in/out on Windows. I recommend taking a look at the page fault interrupt handler and:
```
MmAccessFault
MiDispatchFault
MiResolvePageFileFault
MiGetPagingFileOffset
```

[2024-07-20 03:58] snowua: Several ways to get access to that memory. Just gotta do the research

[2024-07-20 04:00] snowua: You can also disable paging. But the consequences are severe...

[2024-07-20 04:02] szczcur: [replying to grb: "is it possible to manually page in a paged out mem..."]
yes

[2024-07-20 04:16] szczcur: [replying to grb: "what i tried currently is to copy the PML4 entry o..."]
i have no idea what you’re doing.. but if you want pages to be made resident in a designated process <https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-mmprobeandlockselectedpages>

ioallocatemdl, mmprobeandlockpages, mmprobeandlockspecifycache, operate on mapping, yay. if you’re trying to do this to cheat in some video game (sounds like it) then reverse those functions and that part of win mm and implement the shorthand

[2024-07-20 04:27] JustMagic: [replying to grb: "my physmem translation kept failing bcs the VA tha..."]
as snowman said reverse MmAccessFault if you want to do everything manually

[2024-07-20 04:35] szczcur: [replying to JustMagic: "as snowman said reverse MmAccessFault if you want ..."]
or do it in a supported manner and don’t be a crackhead. this is how these payhack idiots wind up like crowdstrike.

[2024-07-20 04:37] szczcur: not that you guys are wrong.. it’s just a common thing. like when so many tried to reimplement kestackattachprocess

[2024-07-20 05:02] grb: [replying to snowua: "You want to take a look at how pages are paged in/..."]
AH YES THANKS

[2024-07-20 05:03] grb: [replying to szczcur: "i have no idea what you’re doing.. but if you want..."]
yea i've tried that before on kernel, not that i dont have access to kernel but im doing it all currently on usermode, and also, it also requires me to attach to the process first before calling those ioallocatemdl, mmprobeandlockpages

[2024-07-20 05:04] grb: [replying to snowua: "You want to take a look at how pages are paged in/..."]
this tho, very much helpful cause i have no idea where should i look for

[2024-07-20 05:05] grb: anyway, thanks a lot <@839216728008687666> <@1033421942910369823> <@221286210851569664> , i really appreciate it, DAMN WHY DIDNT I BE ACTIVE AT THIS SERVER BEFORE 😭

[2024-07-20 11:47] grb: i wonder if there's a way to TRIGGER the page fault from an external process, instead of me trying to replicate it?

[2024-07-20 14:01] elias: Hello, how can I manually tell the ms linker to create an IAT entry by specifying which module contains the target function at runtime?

[2024-07-20 14:04] irql: you can make a .lib file

[2024-07-20 14:04] irql: and link against it

[2024-07-20 14:04] irql: msvc has `lib`

[2024-07-20 14:04] irql: `lib /def:module.def /out:module.lib`

[2024-07-20 14:04] irql: and then link against module.lib

[2024-07-20 14:05] irql: and your module.def file contains the exports

[2024-07-20 14:06] elias: great, thank you

[2024-07-20 14:48] szczcur: [replying to grb: "anyway, thanks a lot <@839216728008687666> <@10334..."]
np

[2024-07-20 17:05] szczcur: [replying to grb: "i wonder if there's a way to TRIGGER the page faul..."]
if the page is present for a time while you're loaded, increment the reference count so the vmm leaves that page in an active state. this is essentially what the page locking does. sharecount can be zero but the ref count, if always > 0, will make sure the page itself isnt repurposed aka physical page is left available

[2024-07-20 17:35] grb: [replying to szczcur: "if the page is present for a time while you're loa..."]
uhh wheres there reference count tho? idk tbh. and also, one of my case in the program i use is a UM process unhooker, one of the thing it does is cleanse the .text section of the modules in a process, usually the problem is that modules that are from shared memories, stuff like this image below, is not paged in to the process and i failed to translate the address to phys addr, maybe do u know how to trigger the page fault?
[Attachments: image.png]

[2024-07-20 17:35] grb: my other case is freshly allocated memories, stuff from VirtualAlloc or NtAllocateVirtualMemory

[2024-07-20 17:36] grb: btw thanks in advance 🫡

[2024-07-20 17:36] szczcur: [replying to grb: "uhh wheres there reference count tho? idk tbh. and..."]
ref count is in mmpfn

[2024-07-20 17:39] grb: [replying to szczcur: "ref count is in mmpfn"]
ohh yea, and i can get those from the list MmPfnDatabase right?

[2024-07-20 17:57] nuclearfirefly: [replying to grb: "uhh wheres there reference count tho? idk tbh. and..."]
> maybe do u know how to trigger the page fault?
call MmAccessFault(0, va, 0, 0). youd need to lock the working set with MiLockWorkingSetShared after calling MiGetAnyMultiplexedVm(MiWorkingSetTypePagedPool, PteBase) and then MiMakeSystemAddressValid(Pte, 0, 0, IrqlResFromMiLockWsShared, 4) then unlock and call MmAccessFault. and if you need to handle cow then you call MiCopyOnWrite and then flush single entry in tb for that

[2024-07-20 19:06] grb: [replying to nuclearfirefly: "> maybe do u know how to trigger the page fault?
c..."]
but we obviously need to attach to the target process, am i right?

[2024-07-20 19:07] grb: which is plainly not possible from UM

[2024-07-20 19:08] grb: i can call kernel routines, but only on my own process context

[2024-07-20 19:54] nuclearfirefly: [replying to grb: "which is plainly not possible from UM"]
i thought you were doing this in km?

[2024-07-20 19:57] nuclearfirefly: [replying to grb: "but we obviously need to attach to the target proc..."]
you only need the target process dtb to do translations. the idea of "attaching" has fucked you game hackers brains. modify apc state, write cr3 to be the target process cr3... or just use the target process dtb for translation without overwriting cr3 and no "attachment" needed by you.

[2024-07-20 19:59] nuclearfirefly: writing your own attach without all the underlying telemetry is simply keeping the apc state correct and setting new cr3.

[2024-07-20 20:01] nuclearfirefly: [replying to grb: "what i tried currently is to copy the PML4 entry o..."]
maybe im missing context, but what you said in what is quoted here sounds like youre working on stuff in km

[2024-07-21 00:17] grb: [replying to nuclearfirefly: "maybe im missing context, but what you said in wha..."]
i aint no game hackers 😭 basically i made a library/framework that let you have a similar access on kernelmode at usermode

[2024-07-21 00:19] grb: but yeah we cant change the CR3 of my process to the target process, the reason why process attaching on KM works is bcs KM memories are always mapped to any UM process's dirbase, if i do this on UM then the thread cant continue execution because the code/instructions isnt there anymore, unless i also map my module page entries to the target process

[2024-07-21 00:21] grb: > use the target process dtb for translation without overwriting cr3 and no "attachment" needed by you.
physmem translation sure, but what about the MmAccessFault calls you gave me?

[2024-07-21 00:22] grb: btw, thank you for replying to my stupid-ahh messages, i really appreciate the help and the convo 🫡

[2024-07-21 02:17] szczcur: [replying to grb: "but yeah we cant change the CR3 of my process to t..."]
im not following what you’re doing much anymore myself. you want to map the memory of one process into another, but have the modifications propagate to the original process? or what? i don’t understand how this thing youre making works which makes it difficult to answer. youre asking questions that are relevant to km, but you’re saying you can’t do it in um.. you need to specify how this thing works altogether if you want helpful answers otherwise you’re gonna be doing this same thing telling us why you cant do something.. if we understood what you’re doing then we could give better answers. based on what you sent im more confused now than before haha

[2024-07-21 03:39] grb: so basically i want to read/write a memory in a remote/external process using physmem, but i cant because the memory is paged out and i cant translate the virtual address to physical memory. Now what im able to do currently is access physmem, which makes me able to do some page fuckery, and i can call kernel functions in KM but in the context of my own process, but i can not do any CR3 swaps or process attach

[2024-07-21 10:27] nu11sec: Hello, could someone help me with a guide on how to write and build LLVM passes on visual studio 2022 or 2019 ty

[2024-07-21 10:39] mrexodia: [replying to nu11sec: "Hello, could someone help me with a guide on how t..."]
https://github.com/LLVMParty/LLVMCMakeTemplate
[Embed: GitHub - LLVMParty/LLVMCMakeTemplate: Collection of scripts and CMa...]
Collection of scripts and CMake files to easily link to LLVM into your project (Windows, Linux, macOS). - LLVMParty/LLVMCMakeTemplate

[2024-07-21 10:47] nu11sec: Ty let me try this

[2024-07-21 11:32] mrexodia: [replying to nu11sec: "Ty let me try this"]
Ah forgot to finish my instructions. You first need to build LLVM and you can find how here: https://github.com/emproof-com/nyxstone?tab=readme-ov-file#prerequisites
[Embed: GitHub - emproof-com/nyxstone: Nyxstone: assembly / disassembly lib...]
Nyxstone: assembly / disassembly library based on LLVM, implemented in C++ with Rust and Python bindings, maintained by emproof.com - emproof-com/nyxstone

[2024-07-21 13:41] nu11sec: got it

[2024-07-21 16:27] nuclearfirefly: [replying to grb: "so basically i want to read/write a memory in a re..."]
what's stopping you from swapping an iat entry or queueing an apc to a thread in that process to execute a function that dereferences the target address (causing a #PF within that target process' thread) and the memory getting paged in, to which you can then adjust the ref count as szczcur recommended and effectively have it locked

[2024-07-21 16:28] nuclearfirefly: the pfndb is also your friend here.

[2024-07-21 16:38] szczcur: [replying to nuclearfirefly: "what's stopping you from swapping an iat entry or ..."]
i think this would normally be fine (outside of the threat intel telemetry in apc fncs), but he has arbitrary constraints.. which is odd, maybe for tlb considerations. that said, if <@776638120950235167> can rwx in km, there's nothing stopping him from hooking in swapcontext and in that hook checking if target process is the one doing the context switch and then doing akin to what you mentioned.. or just outright repurposing/allocating mem, then swapping thread rip to execute his shellcode and jump back to where it was prior when it returns control to um

[2024-07-21 16:42] szczcur: there's arbitrary constraints here, and there's numerous ideas that could work. it really depends on his level of confidence in implementing them and understanding that there will be caveats to every method if he wants 'an easy way'..