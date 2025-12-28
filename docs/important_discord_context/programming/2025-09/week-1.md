# September 2025 - Week 1
# Channel: #programming
# Messages: 203

[2025-09-01 10:06] Aj Topper: ```uint64_t drv::VirtualToPhysical(uint64_t virtualAddr) {
    UINT64 physicalAddr = 0;

    uint16_t PML4 = (uint16_t)((virtualAddr >> 39) & 0x1FF);
    uint16_t DirectoryPtr = (uint16_t)((virtualAddr >> 30) & 0x1FF);
    uint16_t Directory = (uint16_t)((virtualAddr >> 21) & 0x1FF);
    uint16_t Table = (uint16_t)((virtualAddr >> 12) & 0x1FF);

    uint64_t PML4E = 0;

    for (size_t i = 0; i < sizeof(uint64_t); ++i) {
        PML4E |= (uint64_t)(section_map[(cr3 + PML4 * sizeof(uint64_t)) + i]) << (i * 8);
    }

    std::cout << "\t[*] PML4E at " << std::hex << PML4E << std::endl;

    uint64_t PDPTE = 0;

    for (size_t i = 0; i < sizeof(uint64_t); ++i) {
        PDPTE |= (uint64_t)(section_map[((PML4E & 0xFFFF1FFFFFF000) + (uint64_t)DirectoryPtr * sizeof(uint64_t)) + i]) << (i * 8);
    }

    std::cout << "\t[*] PDPTE at " << std::hex << PDPTE << std::endl;

    if ((PDPTE & (1 << 7)) != 0) {
        physicalAddr = (PDPTE & 0xFFFFFC0000000) + (virtualAddr & 0x3FFFFFFF);
        return physicalAddr;
    }

    uint64_t PDE = 0;

    for (size_t i = 0; i < sizeof(uint64_t); ++i) {
        PDE |= (uint64_t)(section_map[((PDPTE & 0xFFFFFFFFFF000) + (uint64_t)Directory * sizeof(uint64_t)) + i]) << (i * 8);
    }

    std::cout << "\t[*] PDE at " << std::hex << PDE << std::endl;

    if ((PDE & (1 << 7)) != 0) {
        physicalAddr = (PDE & 0xFFFFFFFE00000) + (virtualAddr & 0x1FFFFF);
        return physicalAddr;
    }

    uint64_t PTE = 0;

    for (size_t i = 0; i < sizeof(uint64_t); ++i) {
        PTE |= (uint64_t)(section_map[((PDE & 0xFFFFFFFFFF000) + (uint64_t)Table * sizeof(uint64_t)) + i]) << (i * 8);
    }

    std::cout << "\t[*] PTE at " << std::hex << PTE << std::endl;

    physicalAddr == (PTE & 0xFFFFFFFFFF000) + (virtualAddr & 0xFFF);

    return physicalAddr;
}```

[2025-09-01 10:06] Aj Topper: I have mapped physical memory to my process so i am reading physmem directly, i am able to read most of kernel properly but when i try to read the base address of drivers it throws a memory access violation or returns  0 , like reading the base of disk.sys it's at 0xfffff8024de40000 but the physical address is translatting to 0 , works fine if i try translating 0xfffff80247800000 (ntoskrnl.exe) it translates to 3000000

[2025-09-01 11:10] Eriktion: [replying to Aj Topper: "```uint64_t drv::VirtualToPhysical(uint64_t virtua..."]
For gods sake use structs

[2025-09-01 11:10] Xits: ```
    uint64_t PML4E = 0;

    for (size_t i = 0; i < sizeof(uint64_t); ++i) {
        PML4E |= (uint64_t)(section_map[(cr3 + PML4 * sizeof(uint64_t)) + i]) << (i * 8);
    }
```
why is this a for loop? and bitshift by i * 8?

[2025-09-01 11:11] Xits: yeah use the ia32 header

[2025-09-01 11:11] Xits: this is unreadable

[2025-09-01 11:11] Xits: https://github.com/ia32-doc/ia32-doc
[Embed: GitHub - ia32-doc/ia32-doc: IA32-doc is a project which aims to put...]
IA32-doc is a project which aims to put as many definitions from the Intel Manual into machine-processable format as possible - ia32-doc/ia32-doc

[2025-09-01 11:14] the horse: is this to prevent alignment issues?

[2025-09-01 12:35] roddux: `__packed__`

[2025-09-01 12:35] roddux: or use naturally aligned types

[2025-09-01 14:16] valium: [replying to Aj Topper: "```uint64_t drv::VirtualToPhysical(uint64_t virtua..."]
```cpp
UINT64 TranslateLinearAddress(UINT64 dtb, UINT64 va) {
    
    const uintptr_t mask = 0x1FF;

    const uintptr_t page_offset = va & 0xFFF;
    const uintptr_t pte_index = (va >> 12) & mask;
    const uintptr_t pde_index = (va >> 21) & mask;
    const uintptr_t pdpe_index = (va >> 30) & mask;
    const uintptr_t pml4e_index = (va >> 39) & mask;

    const uintptr_t page_offset_2mb = va & 0x1fffff;
    const uintptr_t page_offset_1gb = va & 0x3fffffff;

    const uintptr_t pfn_mask = 0x000FFFFFFFFFF000ULL;

    pml4e PML4E = { 0 };
    pdpe PDPE = { 0 };
    pde PDE = { 0 };
    pte PTE = { 0 };
    large_pde LARGE_PDE = { 0 };
    large_pdpe LARGE_PDPE = { 0 };

    if (ReadPhysicalAddress(dtb + pml4e_index * 8,&PML4E,sizeof(pml4e)) == FALSE) {
        return 0;
    }

    if (PML4E.bits.present == 0) {
        return 0;
    }

    if (ReadPhysicalAddress((PML4E.value & pfn_mask) + (pdpe_index * 8), &PDPE, sizeof(pdpe)) == FALSE) {
        return 0;
    }

    if (PDPE.bits.present == 0) {
        return 0;
    }

    if (PDPE.bits.page_size) {
        LARGE_PDPE.value = PDPE.value;
        return (LARGE_PDPE.value & 0xFFFFFC0000000) + page_offset_1gb; // mask to extract 51:30 bits
    }

    if (ReadPhysicalAddress((PDPE.value & pfn_mask) + (pde_index * 8), &PDE, sizeof(pde)) == FALSE) {
        return 0;
    }
    
    if (PDE.bits.present == 0) {
        return 0;
    }

    if (PDE.bits.page_size) {
        LARGE_PDE.value = PDE.value;
        return (LARGE_PDE.value & 0xFFFFFFFE00000) + page_offset_2mb; //mask to extract 51:21 bits
    }

    if (ReadPhysicalAddress((PDE.value & pfn_mask) + (pte_index * 8), &PTE, sizeof(pte)) == FALSE) {
        return 0;
    }
    
    if (PTE.bits.present == 0) {
        return 0;
    }

    return (PTE.value & pfn_mask) + page_offset;
}
```

[2025-09-01 14:30] iPower: what loops are for <:yea:904521533727342632>

[2025-09-01 15:03] daax: [replying to valium: "```cpp
UINT64 TranslateLinearAddress(UINT64 dtb, U..."]
this... should...be...a loop

[2025-09-01 15:03] daax: NOBODY LIKE LOOP <:whyy:820544448798392330>

[2025-09-01 15:03] daax: just like ntquerysysteminformation

[2025-09-01 15:04] daax: [replying to iPower: "what loops are for <:yea:904521533727342632>"]
<:PES_Why:644097331167232020>

[2025-09-01 15:11] the horse: https://github.com/tinganho/linux-kernel/blob/master/mm/pagewalk.c
[Embed: linux-kernel/mm/pagewalk.c at master · tinganho/linux-kernel]
imx6sl linux kernel source. Contribute to tinganho/linux-kernel development by creating an account on GitHub.

[2025-09-01 15:12] the horse: [replying to daax: "just like ntquerysysteminformation"]
WE DO NOT VERIFY IF OUR BUFFER IS LARGE ENOUGH

[2025-09-01 15:34] valium: [replying to daax: "this... should...be...a loop"]
woah, i didnt know this could be a for loop. i always did this

[2025-09-01 15:36] daax: [replying to valium: "woah, i didnt know this could be a for loop. i alw..."]
```cpp
static boolean_t check_rsvd_bits64(uint64_t page, uint32_t level, ia32_efer_t efer)
{
    switch(level)
    {
        case LEVEL_PML4:
            if(page & ENTRY_PS)
                return FALSE;
            
            break;
        case LEVEL_PDPT:
            if((!check_gb_page_support()) && (page & ENTRY_PS))
                return FALSE;
            
            if(page & ENTRY_PS)
                if(page & MASK(29,13))
                    return FALSE;
                    
            break;
        case LEVEL_PD:
            if(page & ENTRY_PS)
                if(page & MASK(20, 13))
                    return FALSE;
            
            break;
        case LEVEL_PT:
            break;
        default:
            break;
    }
    
    if(page & MASK(51, max_physical_address))
        return FALSE;
        
    if((!efer.bits.nxe) && (page >> 63))
        return FALSE;
    
    return TRUE;
}

static boolean_t va_to_pa64(uint64_t va, uint64_t* pa)
{
    uint64_t page;
    boolean_t is_user = TRUE;
    uint32_t level = LEVEL_PML4;
    
    uint64_t table_pa = __readcr3();
    
    while(TRUE)
    {
        page = *(table_pa + ((uint32_t)(va >> (12 + 9 * level))) & 0x1FF);
        
        // #pf caused by non present page
        if(!(page & ENTRY_PRESENT))
            return FALSE;
        
        // #pf caused by reserved bit set to 1
        if(!check_rsvd_bits64(page, level, __read_efer()))
            return FALSE;
        
        // #pf caused by us check
        if((__rdcpl() == 3) && !(page & ENTRY_US))
            return FALSE;
        
        if(!(page & ENTRY_RW))
        {
            if(__rdcpl() == 3)
                return FALSE;
            else
                if(__readcr0() & CR0_WP)
                    return FALSE;
        }
        
        // if the U/S flag is asserted in all of the paging structures then the address is a user-mode address
        is_user = is_user && (page & ENTRY_US);
        
        if(level == LEVEL_PT || (page & ENTRY_PS))
        {
            uint32_t ac = __read_rflags() & X86_RFLAGS_AC;
            
            // #pf would be caused by smap
            if((__rdcpl() < 3) && is_user && (__readcr4() & CR4_SMAP) && !ac)
                return FALSE;
            
            *pa = (page & MASK(51, 12 + 9 * level )) | (va & MASK_LO(12 + 9 * level));
            
            return TRUE;
        } 
        else 
        {
            table_pa = page & MASK(51, 12);
        }
        
        level--;
    }
}
```

[2025-09-01 15:46] iPower: [replying to daax: "```cpp
static boolean_t check_rsvd_bits64(uint64_t..."]
big daddy code

[2025-09-01 17:12] selfprxvoked: [replying to roddux: "`__packed__`"]
please don't

[2025-09-01 17:13] selfprxvoked: you can use structs + serialization

[2025-09-01 17:13] selfprxvoked: casting to `__packed__` structs is awful

[2025-09-01 17:54] Xyrem: Voo doo

[2025-09-01 17:55] Xyrem: ```c

#define GET_PAGE_SHIFT(Level) ((Level) * 9 + 12)
#define GET_PAGE_INDEX(VA, Level) ((uint64_t(VA) >> GET_PAGE_SHIFT(Level)) & 511)
#define GET_PAGE_TABLE_ADDRESS(VA, Table, Level) (uint64_t(Table) + GET_PAGE_INDEX(VA, Level) * 8)

        void WalkPageTablesRecursive(uint64_t VA, uint64_t Table, pt_entry_64** OutputTable, int Level)
        {
            pt_entry_64* Entry = (pt_entry_64*)TranslatePhys(GET_PAGE_TABLE_ADDRESS(VA, Table, Level));
            OutputTable[Level] = Entry;

            if (!Entry || Level == PT_MAP_INDEX || Entry->present == false || Entry->large_page)
                return;

            return WalkPageTablesRecursive(VA, Entry->page_frame_number << 12, OutputTable, Level - 1);
        }

        uint64_t MEM_VTOP(uint64_t VA, uint64_t CR3)
        {
            pt_entry_64* Tables[4]{};
            WalkPageTablesRecursive(VA, CR3, Tables);

            for (int i = PML4_MAP_INDEX; i != PT_MAP_INDEX; i--)
            {
                if (!Tables[i] || Tables[i]->present == false)
                    return 0;

                if (Tables[i]->large_page)
                {
                    if (i == PDPT_MAP_INDEX) return TranslatePhys((((pdpte_1gb_64*)Tables[i])->page_frame_number << 30) | (VA & 0x3FFFFFFF));
                    else if (i == PD_MAP_INDEX) return TranslatePhys((((pde_2mb_64*)Tables[i])->page_frame_number << 21) | (VA & 0x1FFFFF));
                    else return 0;// PANIC("LEVEL[%d] does not support LARGE pages", i);
                }
            }

            return Tables[PT_MAP_INDEX]->present ? TranslatePhys((Tables[PT_MAP_INDEX]->page_frame_number << 12) | (VA & 0xFFF)) : 0;
        }
```

[2025-09-01 17:56] iPower: [replying to Xyrem: "```c

#define GET_PAGE_SHIFT(Level) ((Level) * 9 +..."]
yeah for simple cases this should be enough. but in the case of daax (and mine, as well) we need to determine page fault reasons to emulate page faults properly

[2025-09-01 17:56] iPower: otherwise, ez clap

[2025-09-01 17:57] Xyrem: https://media.discordapp.net/attachments/724324783399108639/1095582232707600444/giphy_8.gif

[2025-09-01 19:07] UJ: [replying to valium: "woah, i didnt know this could be a for loop. i alw..."]
there is pml5 even, mr daxx is just future proofing by using loops. :smort:

[2025-09-01 19:08] Humza: [replying to daax: "```cpp
static boolean_t check_rsvd_bits64(uint64_t..."]
Why not check for canonicality and lam, assuming this is for emulating page faults

[2025-09-01 19:13] daax: [replying to Humza: "Why not check for canonicality and lam, assuming t..."]
because this is old code I sent to someone in 2022 for reference, the updated methods have lots of artifacts from the project in them.

[2025-09-01 19:17] daax: [replying to Humza: "Why not check for canonicality and lam, assuming t..."]
also I don't think anywhere I said it was for emulating page faults; you'd have to update it anyways to get the info from trapframe and* set PFEC accordingly (which, if you had ever emulated page faults you'd know that was more relevant than LAM atm).

[2025-09-01 19:19] Humza: Fairs

[2025-09-01 19:21] iPower: [replying to Humza: "Why not check for canonicality and lam, assuming t..."]
that code isn't, but was just a response to "voo doo"

[2025-09-01 19:22] iPower: it's even more voo doo when you're actually emulating page faults

[2025-09-01 19:23] Humza: Wouldn’t really call it voodoo just a lot to handle

[2025-09-01 19:23] Humza: If you know what you need to handle it’s pretty trivial

[2025-09-01 19:23] iPower: I know amigo...

[2025-09-01 19:24] iPower: guess I have to explain everything I type here from now on

[2025-09-01 19:34] DirtySecreT: [replying to daax: "```cpp
static boolean_t check_rsvd_bits64(uint64_t..."]
boolean_t <:kekw:904522300257345566>  don't think if I've ever seen that

[2025-09-01 19:48] roddux: [replying to selfprxvoked: "casting to `__packed__` structs is awful"]
i missed context, i thought it was for doing mojo with pagetables?

[2025-09-01 23:03] dlima: Yeah i dont see what the problem with that would be

[2025-09-01 23:03] dlima: Packed is fine

[2025-09-02 03:38] selfprxvoked: I guess (almost) every feature of a programming language can be useful but `__packed__` in specific is just one of this features that are either very niche or very easy to misuse. It has the same potential as `goto` to make the programmer write bad or spaghetti code.

[2025-09-02 03:40] selfprxvoked: I see it being used in network packets alot and it is fucking awful

[2025-09-02 09:03] Brit: [replying to selfprxvoked: "I guess (almost) every feature of a programming la..."]
how is packed in any way similar to goto, the only potential issue I can think of is the creation of unaligned pointers for struct members

[2025-09-02 09:09] roddux: [replying to selfprxvoked: "I guess (almost) every feature of a programming la..."]
niche or awful? what are you talking about lol, it’s a feature to give you precise layout controls of your data

[2025-09-02 17:56] selfprxvoked: [replying to Brit: "how is packed in any way similar to goto, the only..."]
It has the same misuse potential

[2025-09-02 17:59] selfprxvoked: the way I see it being used 90% of the time it is related to network packets where instead of serializing the read/write packets to a struct, they simply cast a `__packed__` pointer to the buffer

[2025-09-02 18:02] selfprxvoked: [replying to roddux: "niche or awful? what are you talking about lol, it..."]
exactly, it is niche

[2025-09-02 18:02] selfprxvoked: instead of using for this purpose people use to cast a pointer to a buffer instead

[2025-09-02 18:04] selfprxvoked: when you add cross platform to the mix, code using `__packed__` starts being a huge problem

[2025-09-02 19:33] Brit: [replying to selfprxvoked: "the way I see it being used 90% of the time it is ..."]
what are you yapping on abt, structures can be packed not ptrs, which is just information about its layout in memory.

[2025-09-02 20:02] selfprxvoked: [replying to Brit: "what are you yapping on abt, structures can be pac..."]
You can? After dereferencing the `__packed__` struct pointer to a buffer?

[2025-09-02 20:03] contificate: what?

[2025-09-02 20:04] contificate: do you know what packing does?

[2025-09-02 20:04] contificate: it's a property of the structure's layout

[2025-09-02 20:05] contificate: it's all just aliasing the memory location in a certain way

[2025-09-02 20:05] dan: [replying to selfprxvoked: "You can? After dereferencing the `__packed__` stru..."]
???

[2025-09-02 20:05] contificate: it's arguably more safe to have it all handled for you by the compiler

[2025-09-02 20:05] contificate: you mentioned network programming, it's almost certainly more dangerous to do something else.. like not use correct big endian or something

[2025-09-02 20:06] contificate: it's flimsy to alias buffered data as anything in a program really

[2025-09-02 20:16] Brit: packed pointer technology, we can fit two pointers into a word if you nuke off the upper half :^)

[2025-09-02 20:21] contificate: people have actually packed data into pointers - if you assume alignment to some boundary, the least significant bits are worthless

[2025-09-02 20:21] contificate: think they did that for crash bandicoot on the PS1

[2025-09-02 20:21] selfprxvoked: This is what I normally see:
<https://godbolt.org/z/xo1aY6Yr8>

[2025-09-02 20:22] selfprxvoked: [replying to contificate: "you mentioned network programming, it's almost cer..."]
casting a `__packet__` struct pointer to a buffer will ignore BE and LE \:)

[2025-09-02 20:23] selfprxvoked: people that normally use `__packet__` that way simply ignore serialization and just do that lazy stuff

[2025-09-02 20:26] contificate: that's the point, but they do it under the idea that `__packed__` is normalising in some way

[2025-09-02 20:26] contificate: across implementations

[2025-09-02 20:26] contificate: but like.. it's not even really safe to read a 4 byte value from a local file and assume it holds a useful value when interpreted as an `int`

[2025-09-02 20:26] contificate: so interpreting garbage you got from a socket

[2025-09-02 20:26] contificate: all takes care

[2025-09-02 20:27] contificate: it's not akin to `goto`, which is one of those things where you can use it to avoid using other things, or do unstructured/irreducible programming generally

[2025-09-02 20:27] Yoran: goto and packed are fine

[2025-09-02 20:27] Yoran: And I don't understand the packed pointer thingy

[2025-09-02 20:28] Yoran: You mean in points to an unaligned struct?

[2025-09-02 20:28] contificate: goto is fine?

[2025-09-02 20:28] Yoran: [replying to contificate: "goto is fine?"]
Yeah why not

[2025-09-02 20:28] contificate: for what, exactly? it's fine in a few cases, but best avoided

[2025-09-02 20:28] diversenok: `goto CleanupExit;`

[2025-09-02 20:28] Yoran: [replying to diversenok: "`goto CleanupExit;`"]
This

[2025-09-02 20:28] diversenok: Probably that's it

[2025-09-02 20:28] contificate: I find that case to be lazy personally but it's simple enough

[2025-09-02 20:29] contificate: people also use it to avoid finding a fix in nested loops

[2025-09-02 20:29] contificate: breaking multiple basically

[2025-09-02 20:29] contificate: all just shit code

[2025-09-02 20:29] Yoran: [replying to Yoran: "You mean in points to an unaligned struct?"]
<@931655461621600277>

[2025-09-02 20:30] Brit: [replying to contificate: "people also use it to avoid finding a fix in neste..."]
escaping deeply nested loops in bad languages

[2025-09-02 20:30] Brit: very valid

[2025-09-02 20:30] avx: do { ... break; } while ( 0 ) <:trollface:1151261622011183146>

[2025-09-02 20:30] contificate: trick is just avoiding deeply nested loops innit

[2025-09-02 20:30] Brit: yes

[2025-09-02 20:30] contificate: the manual closure conversion of C code is everywhere

[2025-09-02 20:46] selfprxvoked: [replying to Yoran: "<@931655461621600277>"]
Yes, trying to translate it from my language got all messed up 😅

[2025-09-02 20:48] Brit: [replying to Brit: "how is packed in any way similar to goto, the only..."]
could've just replied yes to this instead of embarking on a larp sequence

[2025-09-02 20:48] Brit: alas

[2025-09-02 21:22] selfprxvoked: because unaligned pointers isn't only the problem of `__packed__`

[2025-09-02 21:27] x86matthew: why would explicit packing be worse than default packing?

[2025-09-02 21:28] selfprxvoked: sorry I think I made it pretty clear that my problem isn't with the feature itself, but how it ends up being massively misused

[2025-09-02 21:29] contificate: how is it misused

[2025-09-02 21:29] selfprxvoked: [replying to selfprxvoked: "This is what I normally see:
<https://godbolt.org/..."]
I just gave an example

[2025-09-02 21:30] contificate: can you explain what I'm supposed to glean from this

[2025-09-02 21:31] selfprxvoked: (lazily) reading from a buffer using pointer to unaligned struct

[2025-09-02 21:31] selfprxvoked: ignoring endianness, sanitization, misalignment, padding, etc.

[2025-09-02 21:31] contificate: all those things you're ignoring

[2025-09-02 21:31] contificate: is why it doesn't really matter

[2025-09-02 21:32] selfprxvoked: <:mmmm:904523247205351454>

[2025-09-02 21:32] contificate: it's just 1 thing in a sea of terrible things you can do in C

[2025-09-02 21:32] contificate: but it's hardly of the expressive power of `goto` and arguments against that, which are largely opt-in and easy to avoid if you wanted to

[2025-09-02 21:33] contificate: I read your citation of packed as just a general commentary about how unsafe C is generally, not a specific critique

[2025-09-03 04:19] Yoran: [replying to selfprxvoked: "Yes, trying to translate it from my language got a..."]
A pointer to an unaligned struct is equivalent to any pointer

[2025-09-03 04:19] Yoran: It's just a pointer

[2025-09-03 04:21] Yoran: [replying to selfprxvoked: "ignoring endianness, sanitization, misalignment, p..."]
If you allocate non-aligned it's on you

[2025-09-03 15:01] Vonnegut: [replying to selfprxvoked: "when you add cross platform to the mix, code using..."]
One of the main reasons for using packed is for cross platform/application compatibility. What are you smoking?

[2025-09-03 15:14] contificate: whatever it is, I trust he will share it with the group

[2025-09-03 16:20] avx: may I have the penjamin

[2025-09-04 01:49] selfprxvoked: [replying to Vonnegut: "One of the main reasons for using packed is for cr..."]
GCC specific feature? cross platform?

[2025-09-04 01:50] selfprxvoked: again, in the context of memory layouts `__packed__` isn't a problem \:)

[2025-09-04 01:51] selfprxvoked: but I've been seeing (way too much) devs using lazily to read from a buffer

[2025-09-04 01:51] selfprxvoked: normally sockets but I've also seen it been used to read files

[2025-09-04 01:56] selfprxvoked: [replying to Yoran: "A pointer to an unaligned struct is equivalent to ..."]
obviously that they dereference this pointer to read from the buffer 😅

[2025-09-04 03:05] dlima: https://tenor.com/view/joe-biden-presidential-debate-huh-confused-gif-9508832355999336631

[2025-09-04 06:13] Addison: [replying to selfprxvoked: "but I've been seeing (way too much) devs using laz..."]
but that is the point...?

[2025-09-04 06:23] dinero: lmao yeah is that not the main reason it’s used

[2025-09-04 06:23] selfprxvoked: Nope, that it is not.

[2025-09-04 06:25] selfprxvoked: Either blindly using `memcpy` or dereferencing a pointer to an unaligned struct (`__packed__`) from a buffer (thats long af) is terrible.

[2025-09-04 06:26] selfprxvoked: `__packed__`s are meant to be used for memory layouts only (at least for me)

[2025-09-04 06:27] selfprxvoked: I'll need to elaborate the example since I'm seeing way too many people here misunderstanding what I'm saying specifically.

[2025-09-04 06:30] dinero: it’s possible we are

[2025-09-04 06:30] dinero: go for it

[2025-09-04 06:30] dinero: Truthfully, it sounds like you’re saying hammers are only for nails. Sure this is pedantically true but <:delet_this:838736832002261043>

[2025-09-04 07:34] selfprxvoked: There are several protocol formats used in network code, from XML, JSON, text, binary, etc. Well, the problem I'm referring to is limited to the scope of binary format protocols.

Some people prefer to create a custom binary protocol using the in-memory format of their data structures using the `__packed__` attribute:
```cpp
enum PacketTypeEnum { Packet_CS_Example1, Packet_CS_Example2, Packet_CS_Example3 };

struct PACKET_CS_EXAMPLE1
{
    int8_t PacketType;
    int8_t Data1;
    int16_t Data2;
    int8_t Data3;
    int32_t Data4;
} __attribute__((packed));

struct PACKET_CS_EXAMPLE2
{
    int8_t PacketType;
    int32_t NumElements;
    int32_t Elements[];
} __attribute__((packed));

struct PACKET_CS_EXAMPLE3
{
    int8_t PacketType;
    int16_t Data1;
    int32_t Data2;
    int32_t Data3;
    int8_t Data4;
} __attribute__((packed));
```

[2025-09-04 07:34] selfprxvoked: So they read from the received buffer like this:
```cpp
switch (*RecvPtr)
{
    case Packet_CS_Example1: // dereference example
    {
        auto Packet = *reinterpret_cast<PACKET_CS_EXAMPLE1*>(RecvPtr);
        RecvPtr += sizeof(PACKET_CS_EXAMPLE1);

        auto Data1 = Packet.Data1;
        auto Data2 = Packet.Data2;
        auto Data3 = Packet.Data3;
        auto Data4 = Packet.Data4;

        Handle_Packet_CS_Example1(Data1, Data2, Data3, Data4);
        break;
    }

    case Packet_CS_Example2: // unaligned example
    {
        auto Packet = *reinterpret_cast<PACKET_CS_EXAMPLE2*>(RecvPtr);

        if (Packet.NumElements < MAX_ELEMENTS)
        {
            SetEOF(FD);
            break;
        }
        
        RecvPtr += sizeof(PACKET_CS_EXAMPLE2) + Packet.NumElements * sizeof(Packet.Elements[0]);
        
        Handle_Packet_CS_Example2(Packet.NumElements, Packet.Elements);
        break;
    }

    case Packet_CS_Example3: // memcpy example
    {
        PACKET_CS_EXAMPLE3 Packet;
        memcpy(&Packet, RecvPtr, sizeof(PACKET_CS_EXAMPLE3));
        RecvPtr += sizeof(PACKET_CS_EXAMPLE3);

        auto Data1 = Packet.Data1;
        auto Data2 = Packet.Data2;
        auto Data3 = Packet.Data3;
        auto Data4 = Packet.Data4;

        Handle_Packet_CS_Example3(Data1, Data2, Data3, Data4);
        break;
    }
}
```

[2025-09-04 07:34] selfprxvoked: To ensure that the packing of the structures is the same on all platforms, they need to use `#pragma pack` and `__attribute__((packed))` to ensure that different compilers and different platforms organize the structures in memory in the same way.

Another mediocre (but better) way of doing it would be something similar to this instead:
```cpp
switch (ReadInt8(RecvPtr))
{
    case Packet_CS_Example1:
    {
        auto Data1 = ReadInt8(RecvPtr);
        auto Data2 = ReadInt16(RecvPtr);
        auto Data3 = ReadInt8(RecvPtr);
        auto Data4 = ReadInt32(RecvPtr);

        Handle_Packet_CS_Example1(Data1, Data2, Data3, Data4);
        break;
    }

    case Packet_CS_Example2:
    {
        auto NumElements = ReadInt32(RecvPtr);
        if (NumElements < MAX_ELEMENTS)
        {
            SetEOF(FD);
            break;
        }
        
        int32_t Elements[MAX_ELEMENTS] = {};
        const int32_t Result = ReadInt32N(RecvPtr, NumElements, Elements);
        if (!Result)
        {
            SetEOF(FD);
            break;
        }
        
        Handle_Packet_CS_Example2(NumElements, Elements);
        break;
    }

    case Packet_CS_Example3:
    {
        PACKET_CS_EXAMPLE3 Packet;
        
        Packet.PacketType = Packet_CS_Example3;
        Packet.Data1 = ReadInt16(RecvPtr);
        Packet.Data2 = ReadInt32(RecvPtr);
        Packet.Data3 = ReadInt32(RecvPtr);
        Packet.Data4 = ReadInt8(RecvPtr);

        Handle_Packet_CS_Example3(Packet);
        break;
    }
}
```

[2025-09-04 07:44] selfprxvoked: [replying to dinero: "Truthfully, it sounds like you’re saying hammers a..."]
I guess I'd rather agree with the guy that said that this is just one of the unsafe features that C/C++ has to offer and I'll also add that people tend to use it way too much for lazy reasons (including network and file parsers code) leading to problems in the code base such as code readability, safeness, etc., contributing to spaghetti code.

[2025-09-04 07:46] selfprxvoked: <:mmmm:904523247205351454>

[2025-09-04 08:03] x86matthew: sure, serialization is better in many scenarios, especially when you need to consider endianness etc

[2025-09-04 08:03] x86matthew: but i still don't see how packed structs lead to code readability issues or "spaghetti code"

[2025-09-04 08:03] x86matthew: i'd argue that packed structs are probably the cleaner option from a readability point of view

[2025-09-04 08:03] x86matthew: so if they're suitable for your needs then why not

[2025-09-04 08:14] selfprxvoked: [replying to x86matthew: "i'd argue that packed structs are probably the cle..."]
I guess it is because you're experienced, I've seen lots of novice C++ programmers that struggled a lot with the packed structs while the serialization one was easy to understand for them

[2025-09-04 08:14] Addison: me on my way to make a database with just packed structs

[2025-09-04 08:19] selfprxvoked: [replying to x86matthew: "but i still don't see how packed structs lead to c..."]
when it needs to deal with endianness and sanitization it starts affecting readability and thus makes it look like "spaghetti code", just like  `goto`s \:^)

[2025-09-04 08:20] selfprxvoked: which if you're doing some professional game server you *should* do proper sanitization

[2025-09-04 08:49] Matti: [replying to selfprxvoked: "I guess (almost) every feature of a programming la..."]
these arguments can be used against basically any feature of C and even more so C++

[2025-09-04 08:51] Matti: the valid use cases for packed have already been discussed plenty above, for goto see https://discord.com/channels/835610998102425650/835664858526646313/1159286732370493522

[2025-09-04 09:16] selfprxvoked: I agree

[2025-09-04 09:18] selfprxvoked: [replying to Matti: "the valid use cases for packed have already been d..."]
I also agree with the `goto` usage in this case

[2025-09-04 11:38] mtu: [replying to selfprxvoked: "GCC specific feature? cross platform?"]
The reason you see it so much in networking is because it lets you do direct memory mapping between the NIC/driver/IP stack while keeping the memory representation the exact same as the actual bits of the packet. The same logic applies to any type of wire protocol - why burn ops reshuffling data you already have into an aligned struct when you can just read it off the buffer.

[2025-09-04 12:10] selfprxvoked: [replying to mtu: "The reason you see it so much in networking is bec..."]
Unless you're doing embedded or IoT networking, you wouldn't care much about burning ops since you'll be decrypting and sanitizing the untrusted data you received anyway. It becomes even more irrelevant when you start focusing on bit packing, where there's no guarantee that different C/C++ compilers pack bit fields in memory exactly the same way and trading ops to less data transmitted it is a huge win.

[2025-09-05 08:37] donna🤯: [replying to selfprxvoked: "Unless you're doing embedded or IoT networking, yo..."]
Well what you describe is far from a small subset

[2025-09-05 08:38] donna🤯: Regarding readability - I think theres generally implied understanding of the specification / standard you are implementing

[2025-09-05 08:44] donna🤯: [replying to selfprxvoked: "when it needs to deal with endianness and sanitiza..."]
I won't outright disagree since I know poorly implemented endianness conversions can definitely be an eyesore, but I wouldn't call it spaghetti code.

[2025-09-05 08:48] donna🤯: With any half-decently written driver, the host will keep endianness conversions centralised within the peripheral interface

[2025-09-05 15:41] Brit: [replying to selfprxvoked: "Unless you're doing embedded or IoT networking, yo..."]
are you attempting to tell us that not all compilers pack structs the same when you specify 1 byte alignment with packed whether its pragma or the attrib?

[2025-09-05 15:41] selfprxvoked: [replying to Brit: "are you attempting to tell us that not all compile..."]
yeah I'm markov chaining or some sequencing <:mmmm:904523247205351454>

[2025-09-05 15:41] selfprxvoked: no I'm not

[2025-09-05 15:42] selfprxvoked: I'm telling that you'll need to use packed, pragma pack or any attrib

[2025-09-05 15:42] Brit: I swear to god you've hooked up chatgpt to your discord account

[2025-09-05 15:42] Brit: and I'm wasting time replying

[2025-09-05 15:42] selfprxvoked: I don't use LLMs

[2025-09-05 15:43] selfprxvoked: never used tbh

[2025-09-05 15:43] Brit: where does this arcane concept of compilers packing differently come from

[2025-09-05 15:44] selfprxvoked: you're saying that you can achieve it without specifying packed attribute?

[2025-09-05 15:44] Brit: if I #pragma pack(1) or _\_attribute__((__packed__)) which compiler will produce different struct layouts

[2025-09-05 15:44] selfprxvoked: lol

[2025-09-05 15:44] Brit: genuine question

[2025-09-05 15:45] the horse: [replying to Brit: "are you attempting to tell us that not all compile..."]
rust 🧌

[2025-09-05 15:45] selfprxvoked: I guess you're the only one that is understanding that way

[2025-09-05 15:45] the horse: but that's another lang..

[2025-09-05 15:45] Brit: [replying to the horse: "rust 🧌"]
repr packed

[2025-09-05 15:46] the horse: I think that doesn't guarantee re-ordering to not happen, just that it will be packed

[2025-09-05 15:46] the horse: therefore differ

[2025-09-05 15:46] the horse: 😔

[2025-09-05 15:46] Brit: pretty sure it does

[2025-09-05 15:46] Brit: but Ill go check

[2025-09-05 15:46] the horse: repr(C)

[2025-09-05 15:46] Brit: we do have godbolt on hand

[2025-09-05 15:46] the horse: should guarantee that

[2025-09-05 15:52] Brit: [replying to the horse: "repr(C)"]
yeah if you want a C struct packed you have to use both

[2025-09-05 15:52] Brit: #[repr(C, packed)]

[2025-09-05 15:52] Brit: you were right

[2025-09-05 15:52] the horse: another rust L

[2025-09-05 15:52] the horse: 🙏

[2025-09-05 15:53] the horse: jokes aside- this is something that should be added as an attribute into C++ (re-ordering)

[2025-09-05 15:53] the horse: especially with something like PGO

[2025-09-05 15:53] Brit: committee would take 15 years

[2025-09-05 15:53] Brit: at least

[2025-09-05 15:54] the horse: the amount of times i've seen absolutely massive structures because the developer doesn't reorder shit and ends up with pointer, bool, pointer, float, pointer

[2025-09-05 15:54] the horse: (unrelated to each other)

[2025-09-05 15:54] the horse: it's noob!! the horse does not appreciate.

[2025-09-05 15:55] Brit: we just do a little destroying of the cache

[2025-09-05 15:55] Brit: no worries

[2025-09-05 16:24] avx: [replying to the horse: "it's noob!! the horse does not appreciate."]
🥕 🐴

[2025-09-05 17:26] DirtySecreT: [replying to selfprxvoked: "Unless you're doing embedded or IoT networking, yo..."]
are you referring to the bitfield memory layout, the alignment or both? cause yh the standards state that how bitfields are packed+aligned in memory is "implementation-defined" so ya they could have different forms even if explicit alignmentsare requested

[2025-09-05 17:28] DirtySecreT: [replying to Brit: "are you attempting to tell us that not all compile..."]
maybe the lit is wrong but even for embedded stuff we have always said relying on specific bitfield layouts or alignment is not a good idea. most things I've read assume compatibility between compilers or platforms for structs with bitfields which isn't always true so ig I could see where he is coming from

[2025-09-05 17:34] DirtySecreT: if you have something to tell me otherwise ofc i wanna know cause yk we sometimes follow archaic rules and advice that dont apply anymore

[2025-09-05 18:42] Vonnegut: [replying to selfprxvoked: "I don't use LLMs"]
Clanker

[2025-09-06 01:05] selfprxvoked: [replying to Vonnegut: "Clanker"]
I guess I only use LLMs to rewrite something for me since brazilian it is a pretty complex language even for natives

[2025-09-06 01:06] selfprxvoked: Other than that LLMs aren't really useful to me yet

[2025-09-06 01:06] selfprxvoked: <:waitstare:1295829770990387361>