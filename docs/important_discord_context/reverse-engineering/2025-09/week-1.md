# September 2025 - Week 1
# Channel: #reverse-engineering
# Messages: 162

[2025-09-01 09:03] lukiuzzz: [replying to Sussibaki: "Anybody knows how can I get more information about..."]
https://uefi.org/specifications

[2025-09-02 08:30] Matti: [replying to Sussibaki: "Anybody knows how can I get more information about..."]
this is kind of a broad question without any context, so the answer is "it depends"

[2025-09-02 08:34] Matti: the UEFI, PI and ACPI specs are definitely good starting points
there's <https://github.com/LongSoft/UEFITool/blob/new_engine/common/guids.csv> which includes at least most if not all of those and a bunch of proprietary GUIDs like those used by AMI
failing that you could just post the GUID, a friend of a friend could probably run ripgrep or winhex to see if it exists on any of his drives

[2025-09-02 08:36] Matti: but anyone can create a GUID, so if this is some application using its own protocols that aren't open source, your best bet is to search for the byte sequence in any related EFI binaries you've got

[2025-09-02 08:49] Sussibaki: Thanks ill look into it

[2025-09-03 17:52] Bony: is there an exhaustive list of all exported APIs from ntoskrnl with their parameters types somewhere?

[2025-09-03 17:59] Matti: > exhaustive list
depends, the list is different for each build number (sometimes though rarely including removals) so in that sense no
but for any given kernel, sure, just run `dumpbin /exports` to get the list of exports. IIRC if you have a PDB in your symbol store for the kernel then this will include names for exports by ordinal too
> with their parameters types
fuck no

[2025-09-03 18:01] Matti: there are lots of collections of types in different formats like the phnt headers, but they are far from exhaustive, and sometimes incorrect for APIs where reverse engineering was involved

[2025-09-03 18:56] qw3rty01: [replying to Bony: "is there an exhaustive list of all exported APIs f..."]
ntkrnlmp.pdb

[2025-09-03 18:59] brockade: [replying to Bony: "is there an exhaustive list of all exported APIs f..."]
use ntsleuth 🧌

[2025-09-03 19:59] daax: you’re announcing it here. I’d suggest maybe going to DMs?

[2025-09-03 20:36] Bony: [replying to qw3rty01: "ntkrnlmp.pdb"]
there's no parameters types in the pdb isn't there

[2025-09-03 21:03] qw3rty01: [replying to Bony: "there's no parameters types in the pdb isn't there"]
apart from reverse engineering it, you won't get the full function signature for a particular binary version, but it gives you both a list of functions in ntoskrnl and all the public types that are used in the exported APIs
outside of that you can either use msdn to look up a particular function and verify via reversing or parse the WDK headers if you need it to be automated

[2025-09-04 08:21] Matti: the list of types in a public kernel PDB is vanishingly small compared to the real list of types in a private PDB

[2025-09-04 08:22] Matti: for comparison, for a 32 bit x86 kernel I've got both the public and private PDBs for, a dump in C header format is 750KB / 20.8K LOC for the public PDB, as opposed to 4 MB / 113K LOC for the private PDB

[2025-09-04 08:25] Matti: apart from this, even assuming you do have the private PDB for a kernel, you're still left with the problem of having to somehow match these types to the (depending on architecture and calling convention) possibly unknown number of parameters for over 3000 exported functions

[2025-09-04 09:51] x0a: [replying to Bony: "is there an exhaustive list of all exported APIs f..."]
https://streamable.com/5i1ym6
[Embed: Watch 2025-09-04 12-46-58 | Streamable]
Watch "2025-09-04 12-46-58" on Streamable.

[2025-09-04 09:52] x0a: try this , hopefully it helps you

[2025-09-04 09:53] J: Yeah not exactly what he is looking for mate haha

[2025-09-04 09:54] x0a: xd

[2025-09-04 09:56] x0a: i tried to help

[2025-09-04 10:21] qw3rty01: he's looking for the exported APIs though, the public pdb will have everything he needs

[2025-09-04 10:26] Matti: [replying to qw3rty01: "he's looking for the exported APIs though, the pub..."]
I can say 'no it won't' again if you want, but I already did that, so... why do you say this?

[2025-09-04 10:28] Matti: a large number of exports (50% as a wild guesstimate) are not documented, which means their parameter types *may* be in a public PDB but probably aren't

[2025-09-04 10:29] Matti: [replying to Matti: "apart from this, even assuming you do have the pri..."]
and again, this is still the real problem to solve anyway

[2025-09-04 11:20] Brit: it'd be a cool project to collate all the leaked types from all the public pdbs into a single header.

[2025-09-04 11:20] Brit: <:mmmm:904523247205351454>

[2025-09-04 11:35] contificate: `#include`ing 1GB of conflicting declarations 👨‍❤️‍💋‍👨

[2025-09-04 11:36] the horse: `#include`ing a windows header and getting errors because you haven't included another one 😍

[2025-09-04 12:32] Brit: [replying to contificate: "`#include`ing 1GB of conflicting declarations 👨‍❤️..."]
surely no conflicts there

[2025-09-04 12:56] 001: [replying to Brit: "it'd be a cool project to collate all the leaked t..."]
phnt is closest you will get

[2025-09-04 12:56] Ignotus: ``#include``ing a windows header and getting errors because you imported it before/after some other header 😍

[2025-09-04 12:56] Brit: [replying to 001: "phnt is closest you will get"]
no really?

[2025-09-04 12:59] Matti: [replying to 001: "phnt is closest you will get"]
true (especially considering OP's question was explicitly about exports), but for me phnt is still tied with MS' own accidentally included ntosp.h and co in the 10.0.15086 WDK when it comes to kernel internal functions and types

[2025-09-04 12:59] Matti: which one I grep through first depends on what I'm looking for pretty much

[2025-09-04 13:04] Matti: correction, they did this in both the 10.0.10240.0 and 10.0.15086.0 WDK before finding out

[2025-09-04 13:04] Matti: 
[Attachments: oops.7z]

[2025-09-04 13:05] Matti: this is ~2015 and ~2017 for context

[2025-09-04 13:05] the horse: tygoodshare^^

[2025-09-04 13:10] Matti: one thing to consider is that if some type or function declaration is in both (meaning phnt and the MS private headers), and they are different, more likely than not phnt is correct for modern kernels because of this leak being some time ago now

[2025-09-04 13:15] Matti: [replying to Matti: "correction, they did this in both the 10.0.10240.0..."]
incidentally I noticed that the page that used to offer older WDK versions for download has been... changed somewhat
[Attachments: image.png]

[2025-09-04 13:16] daax: [replying to Brit: "it'd be a cool project to collate all the leaked t..."]
doesn’t give any leaked info, but ive used it regularly whenever windows updates. i have it auto categorizing based on build info and just do the entire windows folder. then i can just quickly go browsing for types and do pdb dumps

https://gist.github.com/daaximus/fb80119bb3be8cfb836ba4d9f56bb765

[2025-09-04 13:18] Matti: damn your version is a lot more sophisticated than mine

[2025-09-04 13:19] Matti: ```cmd
@echo off
set PATH=C:\Program Files (x86)\Windows Kits\10\Debuggers\x64;%PATH%;

pushd "%SYSTEMROOT%\System32"

for /R %%a in (*.efi) do (
    echo "%%a"
    symchk /v "%%a"
)

for /R %%a in (*.sys) do (
    echo "%%a"
    symchk /v "%%a"
)

for /R %%a in (*.exe) do (
    echo "%%a"
    symchk /v "%%a"
)

for /R %%a in (*.dll) do (
    echo "%%a"
    symchk /v "%%a"
)

echo Done.
pause
```

[2025-09-04 13:21] Matti: oh and let's not forget the classic `find_private_pdbs.cmd` (to be run in C:\Symbols)
```cmd
@echo off

:: this does not use if () else() for complicated and retarded windows batch related reasons
for /R %%a in (*.pdb) do (
    llvm-pdbutil dump --summary "%%a" | grep -q "stripped: false" && echo   Found private PDB: %%a || llvm-pdbutil dump --summary "%%a" | grep "stripped"
)
```

[2025-09-04 13:22] Matti: I should probably write a script to clean up some old shit next
[Attachments: image.png]

[2025-09-04 13:35] the horse: you could fit that on a ramdisk

[2025-09-04 13:35] the horse: with your pc

[2025-09-04 13:35] the horse: it's fine

[2025-09-04 13:36] Matti: this folder yeah, just about

[2025-09-04 13:36] the horse: can't wait to put 256gb of ddr5 on my next work station

[2025-09-04 13:36] Matti: but I've got another ~90GB one with exclusively private PDBs for x86 32 bit and ARM64 ~10.0.14393

[2025-09-04 13:37] Matti: [replying to the horse: "can't wait to put 256gb of ddr5 on my next work st..."]
same, I want 768 actually but it's unaffordable

[2025-09-04 13:37] Matti: DDR5 I mean

[2025-09-04 13:37] the horse: yeah just 128gb is already 2k

[2025-09-04 13:37] the horse: or so

[2025-09-04 13:37] the horse: oh nvm

[2025-09-04 13:37] the horse: 
[Attachments: image.png]

[2025-09-04 13:37] the horse: nice

[2025-09-04 13:38] Matti: nah it's not *that* much, last I checked 768 GB was about 3500 euros

[2025-09-04 13:38] Matti: yeah that's not bad

[2025-09-04 13:38] the horse: decent clock speed as well

[2025-09-04 13:38] Matti: unfortunately I need 64 GB ECC RDIMMs, 12 of them

[2025-09-04 13:38] Matti: so this wouldn't work for me

[2025-09-04 13:39] the horse: do ECC's have a double buffer for corrections?

[2025-09-04 13:39] Brit: matti will tell you this but we just need to bring back the intel optanes

[2025-09-04 13:39] Brit: this is the specific usecase

[2025-09-04 13:39] Matti: you can brit

[2025-09-04 13:40] Matti: https://www.ebay.com/itm/176549283479?_skw=optane%20300%20series&itmmeta=01K4AF8PMQ956T1BNHS07ZQ8XM&itmprp=enc%3AAQAKAAAA8FkggFvd1GGDu0w3yXCmi1fbh6zRehN9c6Dk9TPEjOybtVv8NH9Yc6S0GJTG61sAwaUoeTdvUa1c%2FIBM%2BsiLlKE27qNqeHZDvjYAgurAD%2BlRAI2YkWvX0NhuNExnnMdRCIhBopr%2F4YZThomr6V%2B20QIM1GGAccg4i%2FFhgBeHH2lhTEpCGIt4PsiKb%2BTGZSSzauIEHIqyAPcXoFzgmXntECumH1BJFnHuTupoSHJIaQDOj1ugrBw6wmhWbjMMyuPeA4mNLtc0H7MglxaG7lJ53hXfGMloExHK4uAVExLNgO0ikXnIMmjgeF1LGfA62yA0xQ%3D%3D%7Ctkp%3ABk9SR77qos-iZg&pfm=1
[Embed: Intel® Optane Persistent Memory 300 Series 128GB DDR5 4400 MT/s  [...]
These are rated at DDR5-4400 speeds adding significant performance. Speed 4400 MHz. Upgrade Type System specific.

[2025-09-04 13:40] Matti: but then you need to use this memory with an intel cpu

[2025-09-04 13:40] Matti: also note that these are only 128 GB DIMMs... kind of a step back from the 512 GB DDR4 ones I've got now

[2025-09-04 13:40] Matti: also idk if they work

[2025-09-04 13:41] Matti: as they were never released to market

[2025-09-04 13:41] Brit: 4*128 for 1500 bucks isn't even all that bad

[2025-09-04 13:41] Brit: considering

[2025-09-04 13:41] Matti: [replying to the horse: "do ECC's have a double buffer for corrections?"]
complicated question to answer, especially for DDR5

[2025-09-04 13:42] Matti: [replying to Brit: "4*128 for 1500 bucks isn't even all that bad"]
no it's not, I agree

[2025-09-04 13:42] the horse: [replying to the horse: "do ECC's have a double buffer for corrections?"]
turns out it stores 8 extra bits usually for error correction; and can only fix single-bit errors

[2025-09-04 13:42] Matti: but you can't just stick this in some board (even an intel one, even an intel one that takes ECC RDIMMs) and expect it to work

[2025-09-04 13:42] Matti: you need a C741 chipset board for one

[2025-09-04 13:42] the horse: maybe this would fix my Intel bsods 😍

[2025-09-04 13:43] Matti: so that's sapphire rapids and whatever succeeded that

[2025-09-04 13:43] the horse: I finally found a fix for WHEA_UNCORRECTABLE_ERROR for VMWare

[2025-09-04 13:43] the horse: it's a faulty NVME controller under Hyper-V host

[2025-09-04 13:43] the horse: SATA works great

[2025-09-04 13:44] Matti: [replying to Matti: "you need a C741 chipset board for one"]
and then you need an impossible DIMM configuration to actually be able to use the PDIMMs as L4 cache/system memory, aka "memory mode" as opposed to "app direct mode" for storage

[2025-09-04 13:45] Matti: the exact guidelines are NDA but you can basically find them in various intel and supermicro server board manuals in reworded form

[2025-09-04 13:45] Matti: for optane 200 series that is, so DDR4

[2025-09-04 13:45] Matti: they are absolutely insane

[2025-09-04 13:47] Matti: it's a combinatorial explosion of rules expressed in tens of different enormous tables for your exact DIMM layout (number, NxM ranks, clock speed, RDIMM vs LRDIMM vs 3D (L)RDIMM, and probably more stuff I forgot)

[2025-09-04 13:48] Matti: [replying to the horse: "SATA works great"]
was about to ban you before realising you were talking about a virtual controller

[2025-09-04 13:49] the horse: 😭

[2025-09-04 13:49] the horse: https://www.reddit.com/r/vmware/comments/1e1mhxp/possible_workaround_for_random_whea_uncorrectable/
[Embed: From the vmware community on Reddit]
Explore this post and more from the vmware community

[2025-09-04 13:50] the horse: this drove me insane for a long time

[2025-09-04 13:50] Matti: yeah I believe it, I just haven't run into this because I use only Matti VirtualBox(TM) NVMe, and of course without hypervisor on the host

[2025-09-04 13:51] the horse: probably as much as the intel overvolting issue

[2025-09-04 13:51] the horse: does anyone know if that's fixed with Meteor Lake?

[2025-09-04 13:52] Matti: lol

[2025-09-04 13:52] Matti: I expect so

[2025-09-04 13:52] the horse: because downclocking a 14900k to 5.2ghz

[2025-09-04 13:52] Matti: but why would you want to buy a meteor lake cpu even if

[2025-09-04 13:52] the horse: is insane

[2025-09-04 13:52] the horse: I need test machines

[2025-09-04 13:52] the horse: across large generation leaps

[2025-09-04 13:52] the horse: I got rid of my 14900k long ago

[2025-09-04 13:52] Brit: rent from someone else

[2025-09-04 13:53] Brit: so they incur the cost of intel cpus cooking themselves

[2025-09-04 13:53] the horse: meh

[2025-09-04 13:53] the horse: I'll have to fuzz a shit ton of stuff across multiple weeks during winter

[2025-09-04 13:53] Brit: but also the lower end skus generally dont commit suicide

[2025-09-04 13:53] the horse: will heat up my room nicely at least

[2025-09-04 13:53] Brit: so you can just buy some china boxes and reimage them

[2025-09-04 13:54] Matti: much more interesting CPU

[2025-09-04 13:54] Matti: 5 level paging support, AVX512, and broken as fuck

[2025-09-04 13:54] the horse: 👀

[2025-09-04 13:54] the horse: do you still have the memory as well?

[2025-09-04 13:55] Matti: yeah but that's mine

[2025-09-04 13:55] the horse: do you expect to upgrade your rig around dec-jan

[2025-09-04 13:55] Matti: I've got a second ICX engineering sample to replace that one

[2025-09-04 13:55] the horse: and what's broken with the cpu

[2025-09-04 13:55] Matti: yes, but the ICX system is not my rig

[2025-09-04 13:56] Matti: [replying to the horse: "and what's broken with the cpu"]
uhmm
well this is a very early stepping of ICX, and it's known to lose memory channels over time

[2025-09-04 13:56] Matti: I don't think it's guaranteed to, it's just a very well known issue

[2025-09-04 13:57] Matti: by lose, I mean any DIMM will simply fail to show up in the BIOS and never return

[2025-09-04 13:57] the horse: unfortunate

[2025-09-04 13:58] the horse: if all goes well I want to build 10-20 test rigs with varying hw for some of my projects

[2025-09-04 13:58] Matti: yeah but tbh I think it can be mitigated to an extent if you buy 1x4R DIMMs as opposed to 2x8R which is more common

[2025-09-04 13:58] the horse: and a couple of local server rigs

[2025-09-04 13:58] the horse: next year

[2025-09-04 13:58] Matti: I had to do this for the optane PDIMMs anyway

[2025-09-04 13:58] Matti: they wouldn't work otherwise

[2025-09-04 13:59] Matti: I have good(?) news if you're interested in PDIMMs... they still seem to be available

[2025-09-04 13:59] Matti: meaning the ones I got

[2025-09-04 13:59] Matti: https://www.spareit.nl/nl/componenten/memory/nmb1xxd512gps-intel-512gb-pmem-module-intel-optane-persistent-memory-200-series
[Embed: NMB1XXD512GPS Intel 512GB PMEM Module (Intel® Optane™ Persistent...]
Intel® Optane™ Persistent Memory 200 Series, 512GB (1x512GB) PMEM Module, Memory Interface: DDR-T, Number Of Modules: 1x 512GB, Bus Speed: 3200MHz DDR4-3200/PC4-25600, Data Integrity Check: ECC, Signa

[2025-09-04 13:59] Matti: theyr'e hardly free, but considering the capacity it's honestly a pretty decent deal

[2025-09-04 14:00] Matti: remember that for memory mode to work, assuming an 8 DIMM motherboard like mine, you'll need 4 of these

[2025-09-04 14:02] Matti: 1 PDIMM will work perfectly fine but it'll be limited to usage in app direct mode, which is similar to an SSD from a user POV but it works very differently at the OS level

[2025-09-04 14:02] Matti: because the memory is byte addressable, there are no sectors

[2025-09-04 14:04] Matti: alternatively you could also consider the ebay listinng I posted instead for DDR5 PDIMMs and buy an ES sapphire rapids xeon instead (they are dirt cheap as well)

[2025-09-04 14:05] Matti: but then there's a real risk that it actually just cannot work

[2025-09-04 14:05] Matti: I couldn't tell you

[2025-09-04 14:08] hxm: hey, how to set the triplet on cmkr ? <@162611465130475520> 

something like .i.g :

[vcpkg]
version = "2025.08.27"
packages = ["protobuf"]
triplet = "x64-windows-static"

[2025-09-04 14:11] hxm: found it https://github.com/build-cpp/vcpkg_template/blob/main/vcpkg-overlay/x64-windows.cmake
[Embed: vcpkg_template/vcpkg-overlay/x64-windows.cmake at main · build-cpp...]
Simple cmkr template to get you started with vcpkg right away. - build-cpp/vcpkg_template

[2025-09-04 14:39] Timmy: <@692740168196685914> 
```
[project]
name = "cmkr_is_awesome"
languages = ["C", "CXX"]
msvc-runtime = "static"
cmake-before = """
set(VCPKG_TARGET_TRIPLET x64-windows-static)
"""
```

[2025-09-04 14:39] mrexodia: [replying to Timmy: "<@692740168196685914> 
```
[project]
name = "cmkr_..."]
This is definitively the wrong approach 😅

[2025-09-04 14:47] Timmy: works on my machine moment

[2025-09-04 14:48] hxm: the overlay folder creation seems to work fine specialy if using cmkr

[2025-09-04 15:55] hxm: is there a way to `add_custom_command` on cmkr ?

[2025-09-04 18:05] mrexodia: [replying to hxm: "is there a way to `add_custom_command` on cmkr ?"]
Nein

[2025-09-04 18:06] mrexodia: I think I opened an issue for it, but you’ll have to include some cmake

[2025-09-04 18:12] daax: [replying to the horse: "across large generation leaps"]
took a few years to build up a collection, do you need one specifically right now or just in general?

[2025-09-04 23:28] ekstrawagant z aspergerem 2016: Hi need help analyzing a .jsc file (V8 bytecode) View8 lacks a binary for this version 🙁

[2025-09-05 02:05] Wane: Hello guys, I'm curious, do you guys have any *philosophy* in reverse-engineering? What reverse-engineering philosophy do you guys have?

[2025-09-05 11:35] Brit: [replying to Wane: "Hello guys, I'm curious, do you guys have any *phi..."]
the only one that accomplishes anything, "do the work".

[2025-09-05 17:13] guar: [replying to Wane: "Hello guys, I'm curious, do you guys have any *phi..."]
<:forsenPirate:988885337952645251>  if you shipped an executable to my pc your TOS don't apply

[2025-09-05 17:13] guar: don't want it be touched - should've done SAAS

[2025-09-05 17:15] the horse: [replying to guar: "<:forsenPirate:988885337952645251>  if you shipped..."]
based

[2025-09-05 19:36] iPower: [replying to Wane: "Hello guys, I'm curious, do you guys have any *phi..."]
no pain no gain

[2025-09-05 19:46] Deus Vult: [replying to Wane: "Hello guys, I'm curious, do you guys have any *phi..."]
Thou shalt not lay thine self to rest, neither shalt thou close thine eyes in slumber, until thou hast understood the manner or thy mystery of the workings of thy code.

[2025-09-06 12:47] Ignotus: Has anyone reverse engineered a .pyc that is compiled with ``pyinstaller``? I tried decompyle3 and uncompyle6, decompyle3 worked but failed to decompile some functions. Maybe there is an alternative I could try?

[2025-09-06 15:38] Rajnikanth: [replying to Ignotus: "Has anyone reverse engineered a .pyc that is compi..."]
Pylingual

[2025-09-07 04:42] Ignotus: ty that helped