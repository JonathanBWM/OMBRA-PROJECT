# February 2025 - Week 2
# Channel: #application-security
# Messages: 193

[2025-02-04 00:14] xorzer0: Yes, this is the CPUID mapping for the most popular hypervisors: https://github.com/systemd/systemd/blob/main/src/basic/virt.c#L40 and serves to setup the hypercalls. In this particular case its about the Intel TDX (https://intel.github.io/ccc-linux-guest-hardening-docs/tdx-guest-hardening.html#tdx-emulation-setup) feature, which is supposed to provide better isolation on hardware level. As far as I can tell TDX hypercalls have only been added to vmware guests (https://github.com/torvalds/linux/commit/57b7b6acb41b51087ceb40c562efe392ec8c9677), so either SRE is based on ESXi, hyperv or even a fork of ACRN.

[2025-02-05 01:02] .: # The Ultimate VM Detection Technique for Usermode Software

Can you think about something that:
- Will be always different from baremetal, no matter what
- It's needed for the VM to run
- Can't be spoofed easily

Seems impossible. But let's think a little bit about it.
**What does happen when you assign all vCPUs to your VM (more logical cores than physical cores your CPU has)?**
Yes, the VM becomes unusable, or at least very slow,  several performance-degrading issues arise due to oversubscription, as the hypervisor must time-slice these threads, leading to frequent context switching.

This is why normally, VMs will not have the same number of threads as their CPU model is supposed to have. 
*Then... why not checking what the CPU model is, get the number of threads it should have, and compare with the current threads*? Well, the reason is, you would need to manually add and verify more than 5000 cpu models out there, both Intel and AMD, who would be that crazy? Yeah, probably not me.

[2025-02-05 01:02] .: But... what if you just spoof the number of logical cores that you VM has? Fortunately, there are syscalls you can execute and numerous of different methods to get the number of logical cores that your system has, which avoids the possibility of any traditional UM hook, forcing the deployment of a kernel driver. For example, just to name a few sources that you would need to spoof:
 * - GetLogicalProcessorInformationEx API (using RelationProcessorCore and RelationNumaNode)
 * - Windows Management Instrumentation via Win32_Processor
 * - GetSystemInfo function
 * - GetProcessAffinityMask function
 * - NtQuerySystemInformation function with SystemBasicInformation (which can be directly/indirectly syscalled)
 * - GetActiveProcessorCount API (across all processor groups)
 * - Windows Management Instrumentation via Win32_ComputerSystem
 * - GetLogicalProcessorInformation API
 * - The NUMBER_OF_PROCESSORS environment variable
 * - Registry query from "HARDWARE\DESCRIPTION\System\CentralProcessor" (processor packages)
 * - GetNativeSystemInfo function
 * - GetMaximumProcessorCount API
 * - NtQuerySystemInformation with SystemProcessorPerformanceInformation (processor performance info)
 * - NUMA API functions (GetNumaHighestNodeNumber and GetNumaNodeProcessorMaskEx) to enumerate processors by NUMA node
 * - Enumeration of processor groups via GetActiveProcessorGroupCount and GetActiveProcessorCount for each group
 * - thread::hardware_concurrency std function

*Note that some of those sources are redundant/internally uses another method in the list, those are just examples*

[2025-02-05 01:05] .: in vmx file configurations, there are numerous options to harden your VM, one of them is the possibility to reflect every cpuid output from your host, this is used to bypass common techniques like retrieving the hypervisor bit, the cpuid information is reflected from your host

[2025-02-05 01:07] .: but, with this approach, if you want to spoof your CPU model, you cant reflect the same values as your host, right? because if you do so, you would give us the real cpu model:

```cpp
        struct leaf {
            static constexpr u32
                func_ext = 0x80000000,
                proc_ext = 0x80000001,
                brand1 = 0x80000002,
                brand2 = 0x80000003,
                brand3 = 0x80000004,
                hypervisor = 0x40000000,
                amd_easter_egg = 0x8fffffff;
        };

        // cross-platform wrapper function for linux and MSVC cpuid
        static void cpuid
        (
            u32& a, u32& b, u32& c, u32& d,
            const u32 a_leaf,
            const u32 c_leaf = 0xFF  // dummy value if not set manually
        ) {
#if (x86)
            // may be unmodified for older 32-bit processors, clearing just in case
            b = 0;
            c = 0;
#if (WINDOWS)
            int32_t x[4]{};
            __cpuidex((int32_t*)x, static_cast<int>(a_leaf), static_cast<int>(c_leaf));
            a = static_cast<u32>(x[0]);
            b = static_cast<u32>(x[1]);
            c = static_cast<u32>(x[2]);
            d = static_cast<u32>(x[3]);
#elif (LINUX || APPLE)
            __cpuid_count(a_leaf, c_leaf, a, b, c, d);
#endif
#else
            return;
#endif
        };
```

you can also get the cpu model from a lot of sources you would need to spoof too, like HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0, and the Name property in the Win32_Processor WMI class, but those do not require privileged intervention. You could also improve the heuristic EVEN further by checking what instructions the cpu should support, (for instance, right now we're checking for SIMD instructions) and try to execute them. If a crash occurs, it's because someone managed to spoof the CPU

[2025-02-05 01:07] .: ```cpp
        // same as above but for array type parameters (MSVC specific)
        static void cpuid
        (
            int32_t x[4],
            const u32 a_leaf,
            const u32 c_leaf = 0xFF
        ) {
#if (x86)
            // may be unmodified for older 32-bit processors, clearing just in case
            x[1] = 0;
            x[2] = 0;
#if (WINDOWS)
            __cpuidex((int32_t*)x, static_cast<int>(a_leaf), static_cast<int>(c_leaf));
#elif (LINUX || APPLE)
            __cpuid_count(a_leaf, c_leaf, x[0], x[1], x[2], x[3]);
#endif
#else
            return;
#endif
        };

        [[nodiscard]] static std::string get_brand() {
            std::array<u32, 4> buffer{};
            constexpr std::size_t buffer_size = sizeof(int32_t) * buffer.size();
            std::array<char, 64> charbuffer{};

            constexpr std::array<u32, 3> ids = {{
                cpu::leaf::brand1,
                cpu::leaf::brand2,
                cpu::leaf::brand3
            }};

            std::string brand = "";

            for (const u32& id : ids) {
                cpu::cpuid(buffer.at(0), buffer.at(1), buffer.at(2), buffer.at(3), id);

                std::memcpy(charbuffer.data(), buffer.data(), buffer_size);

                const char* convert = charbuffer.data();
                brand += convert;
            }

            debug("BRAND: ", "cpu brand = ", brand);

            memo::cpu_brand::store(brand);

            return brand;
#endif
        }
```

[2025-02-05 01:09] .: [replying to .: "But... what if you just spoof the number of logica..."]
again, like in the first case, it would need the intervention of a kernel driver or hypervisor to report false information about the hypervisor bit (and other techniques), while reporting a fake cpu model and making sure all the bits equate to more than 4000:
```cpp
u32 sse3 : 1;       //!< [0] Streaming SIMD Extensions 3 (SSE3)
u32 pclmulqdq : 1;  //!< [1] PCLMULQDQ
u32 dtes64 : 1;     //!< [2] 64-bit DS Area
u32 monitor : 1;    //!< [3] MONITOR/WAIT
u32 ds_cpl : 1;     //!< [4] CPL qualified Debug Store
u32 vmx : 1;        //!< [5] Virtual Machine Technology
u32 smx : 1;        //!< [6] Safer Mode Extensions
u32 est : 1;        //!< [7] Enhanced Intel Speedstep Technology
u32 tm2 : 1;        //!< [8] Thermal monitor 2
u32 ssse3 : 1;      //!< [9] Supplemental Streaming SIMD Extensions 3
u32 cid : 1;        //!< [10] L1 context ID
u32 sdbg : 1;       //!< [11] IA32_DEBUG_INTERFACE MSR
u32 fma : 1;        //!< [12] FMA extensions using YMM state
u32 cx16 : 1;       //!< [13] CMPXCHG16B
u32 xtpr : 1;       //!< [14] xTPR Update Control
u32 pdcm : 1;       //!< [15] Performance/Debug capability MSR
u32 reserved : 1;   //!< [16] Reserved
u32 pcid : 1;       //!< [17] Process-context identifiers
u32 dca : 1;        //!< [18] prefetch from a memory mapped device
u32 sse4_1 : 1;     //!< [19] SSE4.1
u32 sse4_2 : 1;     //!< [20] SSE4.2
u32 x2_apic : 1;    //!< [21] x2APIC feature
u32 movbe : 1;      //!< [22] MOVBE instruction
u32 popcnt : 1;     //!< [23] POPCNT instruction
u32 reserved3 : 1;  //!< [24] one-shot operation using a TSC deadline
u32 aes : 1;        //!< [25] AESNI instruction
u32 xsave : 1;      //!< [26] XSAVE/XRSTOR feature
u32 osxsave : 1;    //!< [27] enable XSETBV/XGETBV instructions
u32 avx : 1;        //!< [28] AVX instruction extensions
u32 f16c : 1;       //!< [29] 16-bit floating-point conversion
u32 rdrand : 1;     //!< [30] RDRAND instruction
u32 not_used : 1;   //!< [31] Always 0 (a.k.a. HypervisorPresent)
```

[2025-02-05 01:10] .: but anyways, how would be that crazy to make a database of every Intel and AMD processor just for this technique?

[2025-02-05 01:11] .: yeah, maybe me and my friend
[Attachments: database.cpp]

[2025-02-05 01:13] .: if someone manages to make a VM actually usable and fast enough when assigning all cores to it, while also spoofing the cpu model, we're forcing them to fail against timing checks

[2025-02-05 01:15] .: i have tested a lot of timing checks in different cpus, most of them were pretty good and some of them like this one were pretty bad, but it seems that you dont need something complex when you have the overhead of having all logical cores assigned to your hypervisor, because even in my i9 cpu i couldnt manage to make my vm fast enough to bypass a trivial timing check which just forces a vmexit and count time elapsed, and tests TLB behavior
[Attachments: image.png, image.png]

[2025-02-05 01:22] .: Let me know what you guys think about it!

[2025-02-05 01:50] .: also as an extra note this new technique idea is essentially cross-platform and relatively fast (0.15ms on avg to run)

[2025-02-05 03:59] Matti: **1.** what if I return, from my hypothetical hypervisor, a CPU brand string that says "AMD Ryzen 9 5950X **1024**-Core Processor" (same as my actual one, except I upped the core count to a nicer looking number)
  ~~a. I'm going to assume/hope you're not actually going to look at the number of cores reported in the string here, since most CPUs don't include this to begin with, it just so happens that mine does. This is also the most obvious way to generate false positives that I can see, because who knows what CPUs may be released in the future and what dumb marketing names they'll use or reuse?~~
  b. so if not (a), then are you gonna conclude that, because my CPU name is not in your database, it must be fake? fair enough, in this case it absolutely is... but I also own some (real, working) x86 CPUs that I'm willing to bet are not in your database. (engineering samples for one, which may or may not report a real marketing "brand string" that may be in your database, and *if* they do, their core counts and clock speeds likely still won't match the final retail versions of CPUs with the same brand string)

**2.** what if I do the opposite, and return a CPU name and corresponding topology from the same processor family, whose only real difference compared to my 'real' CPU is having a **lower** core count? continuing my example from above, I could for example report my 5950X as being a '5900X' (12 cores), or if that is not enough headroom, even a '5800X' (6 cores).
  *but*, I hear you say, there's no way you could hide the real number of cores! there's just too many ways to query this information!
  well maybe. I don't know shit about hypervisors, but I do know a thing or two about OS kernel development, and the way the NT kernel (as well as the HAL) at least obtain the socket/physical cores per socket/logical processors per core counts really doesn't involve anything you can't just look up yourself in the SDM. it's true that once the OS has queried this information (which obviously it needs to do very early during startup), it becomes a lot harder to fix all of the "leaks" you listed. so, why not write a type 1 hypervisor? simply fooling the entire OS really seems a lot simpler to me. but maybe I'm missing something obvious.

  btw: you don't even need a hypervisor to see that this approach works: on windows, try either one of
  `bcdedit /set {current} numproc <N>`
  or
  `bcdedit /set {current} onecpu on`
  and reboot.
  You'll notice that different programs will disagree on how many cores your CPU now has - for me on my xeon with `onecpu on`, CPU-Z on correctly reports the number of cores/threads as 1/1, but HWINFO64 insists that it must be 32C/64T, not the 1 logical processor the OS actually sees and is very obviously and very painfully scheduling every single thread on.

**3.**
\- what if the user has, in the *real BIOS on their real machine*, explicitly set the number of CPU cores to some value other than the maximum number?
\- what if they've disabled SMT?
\- what if they've disabled some or all of the E-cores (for intel(R) alder lake CPUs and later), or disabled a CCD on a multi-CCD AMD CPU (for gaming performance on V-cache CPUs, which only have the extra L3 on one CCD)?
\- what if... (and so on)

[2025-02-05 04:04] Matti: (1) and (2) above I don't actually care about very much (except for the question of what you're going to do if - surprise - some CPU is not in your giant database after all, I guess).
but there are absolutely valid reasons to do any of the things listed under (3), and I don't see how your database approach is going to deal with this.

So,
> Then... why not checking what the CPU model is, get the number of threads it should have, and compare with the current threads?
I'm sorry to say this because your idea is otherwise quite clever (really!), but there simply is no such thing as 'the number of [logical processors] a CPU *should* have' (other than that it should preferably be > 0). There's a number it *could* have, and for various reasons this number may not be the same, even on real hardware, as the number of processors it *does* have.

(note: of course purely physically speaking, a CPU or system will always have the same number of logical processors (disregarding CPU hotplugging/hot removal... I guess you can add that to my list of objections as well if you really want to <:lillullmoa:475778601141403648>). I'm using 'having X processors' here as shorthand to mean 'X processors are made available for use by the system')

[2025-02-05 11:24] Timmy: [replying to Matti: "**1.** what if I return, from my hypothetical hype..."]
> why not write a type 1 hv

haven't tried it myself, but type2 hvs are already difficult and type 1 kinda adds half an operating system to the codebase :3

[2025-02-05 11:32] Matti: I kinda see it like either being the hypervisor from the get go, and before the OS initializes, vs essentially taking over a running OS without blowing it up in the process

[2025-02-05 11:33] Matti: but, no doubt both types are complex either way

[2025-02-05 11:34] Brit: "without blowing it up"
"draw the rest of the fucking owl"

[2025-02-05 11:35] Matti: yeah pretty much

[2025-02-05 11:40] Matti: I will say though, I wouldn't want to write a type 1 if legacy BIOS was still a thing

[2025-02-05 11:40] Matti: nowadays UEFI or else coreboot do make this a lot easier for sure

[2025-02-05 11:41] Matti: UEFI is almost a full blown OS by itself, with MP synchronization and everything

[2025-02-05 14:22] .: [replying to Matti: "(1) and (2) above I don't actually care about very..."]
for (3), or maybe take into account physical cores only? why would you disable physical cores?
[Attachments: image.png]

[2025-02-05 14:24] .: i'm aware that logical core count is not enough because of the reasons you listed on 3, in fact thats my only headache atm 😓

[2025-02-05 14:26] .: regarding your question about a cpu not being in my db, i will just return false, or... maybe integrate an API to connect with openai and make it research what the core count is? (nah, i'd prefer to just automatically update the db when new cpu brands are released)

[2025-02-05 14:27] .: regarding SMT, bcdedit and etc, these configs can be checked for I think? 🤔

[2025-02-05 14:29] .: but at the moment this is the best I have come up with for detecting vms in usermode

[2025-02-05 14:30] .: i will probably keep improving it tbh or make some similar logics with any other artifact, I disliked the traditional checks of scanning firmware, reg keys, doing wmi queries, and so on

[2025-02-05 14:32] .: another idea i have if this doesn't end well in production after it's fully finished is to apply like digital forensics to specific vm artifacts

[2025-02-05 14:33] .: so for example in vm detections there are a lot of people who use vm file checks and process name checks, but you can simply kill the process and delete the file to bypass those

[2025-02-05 14:34] .: but nobody thinks about scanning J$ datastreams, parsing the system resource usage monitor (or scanning Diagnostic Policy Service's memory as its linked to SRUM), scanning prefetch files, and other 30 more windows artifacts i could mention to prove a vm file that only VMs use was executed

[2025-02-05 14:35] .: but it would be like, slow af

[2025-02-05 14:36] .: and i dont like making a digital forensic scanner just for vm artifact detection

[2025-02-05 14:36] Matti: [replying to .: "for (3), or maybe take into account physical cores..."]
well I listed a few reasons (E-cores, non-vcache CCDs) that are probably more common than the reasons I personally sometimes have to disable physical cores

[2025-02-05 14:37] .: true thats a problem

[2025-02-05 14:37] Matti: but, I very recently spent a full week debugging an issue where my HAL would work with 60 CPUs, but not with 64

[2025-02-05 14:38] .: idk i need another way of fingerprinting cpus

[2025-02-05 14:38] .: and done

[2025-02-05 14:38] Matti: in general I use `/onecpu` more often than I'd like to admit... a lot more

[2025-02-05 14:39] diversenok: What's the goal anyway? There are many edge cases and grey areas that you might want to classify differntly depending on your goal and its tolerance to false negatives and false positives

[2025-02-05 14:39] .: [replying to Matti: "well I listed a few reasons (E-cores, non-vcache C..."]
this completely fucks up the idea

[2025-02-05 14:39] .: lol

[2025-02-05 14:41] .: [replying to diversenok: "What's the goal anyway? There are many edge cases ..."]
the goal is to detect vms from usermode and develop like the hardest technique to bypass but it cant false flag under any circumstance

[2025-02-05 14:41] Matti: [replying to .: "regarding SMT, bcdedit and etc, these configs can ..."]
oh they can, I was just giving an example of how doing this does not even require a hypervisor

[2025-02-05 14:42] .: I had this idea just before going to bed

[2025-02-05 14:42] .: [replying to Matti: "oh they can, I was just giving an example of how d..."]
ah i see

[2025-02-05 14:43] diversenok: [replying to .: "the goal is to detect vms from usermode and develo..."]
Nah, by the goal I mean how and where you want to use it, not what it needs to do

[2025-02-05 14:43] .: ah

[2025-02-05 14:44] .: distributing it as an open source check for vm detection that people could integrate into their programs. For example, to test if their environment is well hidden to test malware, and other reasons

[2025-02-05 14:45] .: basically releasing MIT code

[2025-02-05 14:50] Matti: [replying to .: "regarding your question about a cpu not being in m..."]
ehhh

[2025-02-05 14:50] Matti: I wanted to see what this would return

[2025-02-05 14:50] Matti: 
[Attachments: image.png]

[2025-02-05 14:50] Matti: that is uh... not close

[2025-02-05 14:51] Matti: at least for my specific one

[2025-02-05 14:51] .: it has like 64 iirc

[2025-02-05 14:51] .: right

[2025-02-05 14:51] Matti: well mine does

[2025-02-05 14:51] Matti: but another one of mine has 48

[2025-02-05 14:51] Matti: it's like chatgpt says, a placeholder string

[2025-02-05 14:52] .: mhmmmmhmhmh

[2025-02-05 14:54] .: the problem is that other features of your CPU can be spoofed without breaking the vm, i was kinda like forcing the vm to be unusable if they attempted to spoof the feature in question (in this case physical core count) or always fail another check (timing checks in this case)

[2025-02-05 14:54] .: so ig ill think more about it when im free

[2025-02-05 14:57] .: as always the problem seems to be handling legitimate edge-cases

[2025-02-05 14:57] Matti: [replying to Matti: ""]
actually kinda funny that the CPU it did suggest (8700K) doesn't match the family and stepping at all either

[2025-02-05 14:57] Matti: 
[Attachments: image.png]

[2025-02-05 14:57] Matti: ^ this is an 8700K in xeon form

[2025-02-05 14:58] Matti: well, family is correct

[2025-02-05 14:58] Brit: calling out to chatgpt to fill out your cpu LUT is definitely a vibe

[2025-02-05 14:58] Matti: chatgpt will fix this!

[2025-02-05 14:59] .: it will replace our jobs fr

[2025-02-05 14:59] Brit: I think a question that stands is why are we so intrested in knowing whether or not we're in a vm

[2025-02-05 15:00] Matti: I am interested I guess

[2025-02-05 15:00] Matti: but then we're talking about the simulation hypothesis

[2025-02-05 15:00] Matti: I know if I'm in a VM on a computer

[2025-02-05 15:00] Matti: due to it being unbearably slow

[2025-02-05 15:01] Brit: <:topkek:904522829616263178>

[2025-02-05 15:03] naci: [replying to Matti: "chatgpt will fix this!"]
just wait few years until o1337 comes out, it will truly change the world!!!!

[2025-02-05 15:03] Brit: you're laughing meanwhile all the data entry people are getting replaced :^)

[2025-02-05 15:05] Matti: unless they're entering CPU names and their core counts

[2025-02-05 15:05] Matti: those people will be safe for some time

[2025-02-05 15:07] Brit: anyway what I really want to know if what's the usecase for usermode HV / VM detection

[2025-02-05 15:08] Matti: well it could simply be for fun or learning

[2025-02-05 15:08] Matti: I mean why do I work on scyllahide when I can run titanhide in kernel mode

[2025-02-05 15:10] Brit: fair fair

[2025-02-05 15:10] .: well imagine ur vanguard and u dont allow vms

[2025-02-05 15:11] Matti: but yeah... I think it's dangerous to say a project must serve a purpose necessarily
but this one definitely seems... misguided? ill-advised?

[2025-02-05 15:11] Brit: you have more avenues to do hv & vm detections when you're a driver, esp an early boot one

[2025-02-05 15:12] .: ig anticheat devs would do vm detections

[2025-02-05 15:13] Brit: yes but not in usermode, and generally through boot attestation

[2025-02-05 15:13] Brit: knowing you've booted under hyper-v is good enough generally

[2025-02-05 15:13] Brit: (terms and conditions do apply, because what if hyperv was subverted but that's another topic)

[2025-02-05 15:14] Matti: ah sorry brit but you must have missed the memo

[2025-02-05 15:14] Matti: https://discord.com/channels/835610998102425650/835664858526646313/1336398427348799598

[2025-02-05 15:14] Brit: <:kekw:904522300257345566>

[2025-02-05 15:15] Matti: American Mattitrends International has now completely subverted attestation

[2025-02-05 15:15] Matti: for reasons TBD

[2025-02-05 15:16] Matti: I think this motherboard's BIOS is just broken beyond imagination

[2025-02-05 15:17] Matti: I even booted through efiguard for the sake of it

[2025-02-05 15:17] Matti: so I was running unsigned drivers in the background

[2025-02-05 15:18] Brit: that root of trust is definitely working fine

[2025-02-05 15:18] Brit: lgtm

[2025-02-05 15:19] Matti: what's also interesting is that the mobo has two TPM headers

[2025-02-05 15:19] Matti: one SPI and one LPC

[2025-02-05 15:19] Matti: I have both connected, but the default is SPI

[2025-02-05 15:20] Matti: switching to LPC makes no difference whatsoever

[2025-02-05 15:20] Matti: well, not entirely true, some of the PCR values were different

[2025-02-05 15:20] Matti: but not PCR 7

[2025-02-05 15:21] Matti: I really wonder what component in the chain is so broken here, and how it is that windows is not seeing this

[2025-02-05 15:27] Matti: disabled secure boot again
now this is the screen I'm used to
[Attachments: image.png]

[2025-02-05 16:05] luci4: "To Be Filled By Matti" goes hard

[2025-02-05 16:15] pinefin: if it was me i would just write 6942069420

[2025-02-05 17:55] Deleted User: [replying to Timmy: "> why not write a type 1 hv

haven't tried it myse..."]
its not that bad actually all u need to do is isolate the windows apis from ur hv, export some functions from ur hv (like the one that virtualizes that gets run per core) and allocate stuff from a loader (for type1 that can be an uefi app or driver, for type2 a windows driver)

[2025-02-05 17:56] Deleted User: and some specific vmexits for type 1 only but ya

[2025-02-05 17:56] Deleted User: the main part is making the hv itself os independent

[2025-02-05 17:56] Timmy: hmm right

[2025-02-05 17:56] Timmy: I didn't think about that actually

[2025-02-05 17:56] Timmy: you don't need that many fancy things

[2025-02-05 17:58] Deleted User: i also made the type 1 part easier for me by using an uefi app and returning an error on exit which makes it load windows automatically, instead of having to load it myself

[2025-02-05 18:49] Timmy: neat

[2025-02-05 22:45] .: [replying to Matti: "**1.** what if I return, from my hypothetical hype..."]
for (3)
[Attachments: smt-bcd-ccd.cpp]

[2025-02-05 22:47] .: sources
https://en.wikipedia.org/wiki/CPUID#cite_note-125
https://web.archive.org/web/20140714221717/http://developer.amd.com/resources/documentation-articles/articles-whitepapers/processor-and-core-enumeration-using-cpuid/
[Attachments: image.png, image.png]
[Embed: Processor and Core Enumeration Using CPUID - AMD]
This article explains how applications can use OS APIs and CPUID on a system with AMD processors to discover the number of logical and physical processors, the number of cores, ...
[Embed: CPUID]
In the x86 architecture, the CPUID instruction (identified by a CPUID opcode) is a processor supplementary instruction (its name derived from CPU Identification) allowing software to discover details 

[2025-02-05 22:50] .: for bcd
[Attachments: image.png]

[2025-02-05 22:50] .: so basically if someone manually disables SMT, CDD, E-cores now the code would not false flag and return false instead, im still looking for something reliable for bcd settings

[2025-02-05 22:51] .: a little bit more robust now ig

[2025-02-06 04:01] Matti: \-  support for >64 CPUs seems to be clearly missing in a bunch of places

\- SMT support cannot be represented as a boolean the way you are doing it (i.e.: SMT is either supported or not supported), because there is no reason whatsoever why SMT has to be limited to 2 logical processors per core. Lots of IBM CPUs have 4-way SMT, and even the Intel(R) Xeon Phi(TM), besides being very nearly the most useless CPU ever made second only to the Itanium, had 4-way SMT.

\- why are you still seriously considering querying the BCD for anything? I've said twice now that this was just a way to demonstrate to you that the OS can be limited to some maximum number of processors without even needing a hypervisor **or** the BIOS to accomplish this. But maybe I made my point too well...? So, just to be clear, no one actually does this (in this way), unless maybe they are debugging a kernel/hypervisor issue. Saner people will simply disable CPU cores, SMT, Intel(R) E-cores, AMD CCDs, ..etc, in the BIOS if needed. Most hypervisors will only really give you the ability to set the total number of vCPUs, and sometimes also the number of logical vCPUs per physical vCPU. I'm sure one could make a HV that presents an AMD CPU with 2 CCDs to the guest, with one being disabled, but like... *why*? It's way less effort to just decrease the number of vCPUs. If you *really really* want to query the BCD, for god's sake, at least use the phnt headers (`ntbcd.h`) instead of this WBEM wrapper shit.

\- why the constant urge to split each of your CPU feature queries into separate Intel(R) and AMD versions of essentially the same function? This is just making (already bugprone) code twice as prone to implementation mistakes, and also needlessly limits the total number of CPU vendors your code can ever run on to a grand total of 2.

FWIW, I ran your `bool isSMTEnabledAMD()` on my 5950X, and it correctly returned true... after determining that it's got **1** physical core with **32** logical processors in this core; i.e. 32-way SMT. Since 32 > 1, the function does correctly return true. But I would argue that this is stil pretty fucking broken.

Why not just check for x2APIC support instead, and use that if available (overhwelmingly likely the case)? That way you can also do away with the weird and unneeded intel/AMD splits. If for some reason a CPU doesn't support x2APIC, the AMD function can still be fixed for my CPU by additionally querying for AMD extended topology extensions. Your intel SMT function looks like it would fail on older CPUs that support SMT but not x2APIC (>= Pentium 4 but < Nehalem). See SS for the relevant code from my WRK
[Attachments: image.png]

[2025-02-06 04:04] Matti: this is gonna be my last reply to this thread, since frankly I think what you are doing *could* be interesting, but you're going about it entirely the wrong way and I can't seem to convince you that you can't fix this problem with a giant database lookup

[2025-02-06 14:26] .: "could" be interesting?

[2025-02-06 14:26] .: why the problem is manually disabling physical cores in the first place

[2025-02-06 14:27] .: i can just tell the user to enable all cores back if they want the program (i.e. a game, or something else) to run

[2025-02-06 14:29] .: and it would still patch all vms, i wanted a logic to determine if you're hyper-threading from usermode just to make it more robust but thats not the main issue here, i also saidi was looking for something reliable for bcd settings

[2025-02-06 14:30] .: but honestly if i have to implement this in production i wouldnt do it like this, vmaware is the project im working on with production-ready code, which will not flag a VM just because a thread mismatch was detected

[2025-02-06 14:31] diversenok: [replying to .: "i can just tell the user to enable all cores back ..."]
You said yourself that a requirement is that it "cant false flag under any circumstances"

[2025-02-06 14:31] .: [replying to .: "the goal is to detect vms from usermode and develo..."]
thats the idea i had here, yeah

[2025-02-06 14:33] .: because in an ideal scenario thats what we all would want

[2025-02-06 14:33] .: but thats not the main problem

[2025-02-06 14:33] .: or even a problem realistically speaking

[2025-02-06 14:34] .: because we're caring too much about someone disabling cores just for hypervisor/kernel debugging

[2025-02-06 14:36] .: he made it seem the idea is completely fucking useless because of someone wanting to disable cores for that kind of debugging

[2025-02-06 14:37] .: why would you **need** to disable **physical** cores

[2025-02-06 14:38] diversenok: I think the main problem is that you don't specify what exactly you want to use it for. Creating a single library suitable for all possible purposes is not going to work because there are use-cases that contradict each other by demanding the same setup to be treated as a VM and as a physical machine depending on circumstances you don't want to consider

[2025-02-06 14:40] .: I think i already said clearly whats the purpose

[2025-02-06 14:40] .: [replying to .: "distributing it as an open source check for vm det..."]
-

[2025-02-06 14:40] diversenok: [replying to .: "why would you **need** to disable **physical** cor..."]
To demonstrate that relying on this information for VM detection is not going to allow creating "the perfect solution"

[2025-02-06 14:41] .: for example, imagine malware uses this trick, why would malware care if you manually disable cores, they would start self-integrity routines if thats the case just to be sure (not decrypting payloads as an example), or if its used for a legitimate app, it could warn the user to enable all cores (they have to know how because they manually turned some of them off)

[2025-02-06 14:45] .: so in the hypothetical case this is used by malware, having all cores assigned to the VM would be a requisite for hardeners now, because security researchers would notice that malware might behave differently if they know your cpu ""should"" have x but has y cores (a pattern that you always see in VMs)

[2025-02-06 14:47] diversenok: [replying to .: "-"]
"Distributing as open source" might be a goal for you, but it's not the purpose of the open-sourced project itself. I'm asking about the purpose because this purpose provides unique requirements. And you just say "for testing VM hiding and other reasons"

[2025-02-06 14:48] diversenok: I can imagine applications for this project that would have contradicting requirements

[2025-02-06 14:48] .: maybe i didnt explain myself correctly?

[2025-02-06 14:48] diversenok: Which means it cannot exist and satisfy them at the same time

[2025-02-06 14:48] .: okay

[2025-02-06 14:48] Matti: [replying to .: "why would you **need** to disable **physical** cor..."]
I listed several genuine, realistic scenarios lmao
and you're cherrypicking because *I* happen to do this for obscure reasons

[2025-02-06 14:49] Matti: feel free to discount my use case, it's not relevant

[2025-02-06 14:49] Matti: as I've now stated 3 (three) times

[2025-02-06 14:54] Matti: you're also ignoring a slew of other unrelated problems with your approach I've pointed out

[2025-02-06 14:54] Matti: that aren't related to disabling cores, CCDs, or SMT

[2025-02-06 15:14] .: your *scenarios* are:

- what if they set the number of cores to something else
- what if they disable SMT
- what if they've disabled some of the E-cores, or disabled a CCD for gaming performance

from all you said, the only *reason* of why someone might disable cores in your message is: the OS scheduler might sometimes move threads between CCDs that don't have the L3 performance boost, and that might be a reason why people want to disable those cores. Later you also mentioned someone wanting to debug kernel/hypervisor issues, your cpu not being in my database, or someone making a type 1 hypervisor to bypass the program, which I also answered before, but you agreed those are not relevant so I focused on your scenarios

[2025-02-06 15:32] Matti: > from all you said (...)
true, I felt my post was getting long enough as is

> the only reason of why someone might disable cores in your message is: the OS scheduler might sometimes move threads between CCDs that don't have the L3 performance boost
yes, and the same reasoning applies to disabling Intel(R) E-cores on alder lake and later CPUs. the windows scheduler does not deal with these heterogenous CPU designs well at all. you can just look up some benchmarks if you think this is not a real issue. nor is it limited to just gaming (for E-cores at least)

> Later you also mentioned someone wanting to debug kernel/hypervisor issues
I did, and I (and a number of others on this server) actually do this sometimes. sure, it's an obscure use case, but that doesn't make it not real

> your [sic] cpu not being in my database
no, the issue is any CPU that exists in the world but not in your database. it is actually the biggest issue by far with your entire approach/design/idea. it is future proof exactly for the amount of time until any new CPU model is released (or even an existing one rebranded). that is a fatal flaw that should've caused you to reconsider the idea altogether

> or someone making a type 1 hypervisor to bypass the program
well, I thought detecting hypervisors was the goal here. so I thought it relevant to mention the possibility

[2025-02-06 16:01] .: [replying to Matti: "> from all you said (...)
true, I felt my post was..."]
I probably didn't express myself well about the idea and that's not your fault, as I said to Diversenok, the idea is to have the best VM detection method that can be ran from userspace, where 'beast' means it should be as hard as possible to bypass, and should have as few problems as possible due to false flags, prioritizing not having problems due to false flags

the issue of not having a CPU in the database would be temporary and definitely not a problem for me, it would growth and be automatically managed over time like a lot of things in this world, all the 8000+ models there are in the source file I sent in my first message of this topic were automatically merged. You can choose right now any of those brands and you will see the information is accurate. If a CPU model is not in the db for any reason, it will not assume it's a VM

of course someone making a type 1 hypervisor and patching all the sources I'm using to get CPU information would be a possibility, but if you have to do all of that, it means that the technique fulfils its purpose

the issues you did mention where someone might disable physical cores for legitimate reasons are fair (gaming benefits and hypervisor/kernel development), and in fact I did agree with them and I said I needed another method of 'fingerprinting' CPUs that would not lead to these scenarios, but I think that for many applications it can be solved by just asking the user to turn their cores on again..?

[2025-02-06 16:07] .: I just got pissed off about your claim; "but you're going about it **entirely the wrong way** and I can't seem to convince you that you can't fix this problem with a giant database lookup" was exaggerated and definitely with a mocking intention (as always with the typical emoji reaction added to it 😂 ), the code I sent before that message is not mine, yet I sent here to be reviewed and it ended with you calling it "WBEM wrapper shit"

[2025-02-06 16:09] diversenok: WBEM wrapper is shit because it's a slow enormous level of abstraction over a lightweight interface

[2025-02-06 16:11] .: [replying to .: "so basically if someone manually disables SMT, CDD..."]
Check out the last words of this message

[2025-02-06 16:13] .: this is not about something being shit or not, it probably is, but i wouldn't call something shit because of the rules you got in this discord

[2025-02-06 16:15] diversenok: I'm just saying it's really heavy as it spins up a WMI Provider Host and does all this RPC just to call a few functions that read a bunch of registry values

[2025-02-06 16:15] diversenok: You might as well call them directly, as bcd.dll definitions are available in phnt

[2025-02-06 16:16] diversenok: Matti is allergic to using overengineered abstractions when there are simpler more lightweight alternatives

[2025-02-06 16:19] diversenok: It's a critique toward the technology itself

[2025-02-06 16:24] .: ya

[2025-02-06 16:26] .: does someone know at what system boot stage on 22h2 the original pointer to ntoskrnl and it's doubly-linked list is invalidated so that I dont have to get again the PKLDR_DATA_TABLE_ENTRY from PsLoadedModuleList->Flink

[2025-02-06 16:35] .: NVM got it

[2025-02-06 16:48] Matti: [replying to .: "I just got pissed off about your claim; "but you'r..."]
re: "entirely the wrong way": I say this because you still don't seem to get that you **cannot realistically make a database of all existing CPUs** - much less keep it up to date in any acceptable timeframe as new ones are released.

re: "exaggerated and definitely with a mocking intention": no, I was and am entirely serious about this. "a giant database lookup" might sound like an oversimplification to you, but it really isn't. it's the core and essence of how your proposal is supposed to work in the first place, with anything else being secondary to this. not sure why or where you are seeing a mocking intention. generally when I mock people I spend a lot less time typing than I'm doing right now, and you will be sure that I am mocking you.

> the issue of not having a CPU in the database would be temporary and definitely not a problem for me, it would growth and be automatically managed over time like a lot of things in this world, all the 8000+ models there are in the source file I sent in my first message of this topic were automatically merged. You can choose right now any of those brands and you will see the information is accurate.
you are *critically underestimating* how deficient your database is going to be in the real world. and again, that is just *today*. this problem will get increasingly worse in the future, not better.
(..cont'd..)
> **If a CPU model is not in the db for any reason, it will not assume it's a VM**
this is the only thing that salvages your database approach for any real world use. But it is also a gaping hole in your stated goal: detecting hypervisors. why wouldn't a hypervisor looking to defeat your program simply change the CPU brand string and move on? the brand string doesn't even have to be a made up CPU! it just needs to not exist in your database. and again, your database is going to be *critically deficient*.

> I sent here to be reviewed and it ended with you calling it "WBEM wrapper shit"
that was a review, and in fact it was a direct answer to your query asking for alternatives here: https://discord.com/channels/835610998102425650/835656787154960384/1336831465119551530

I *do* think using WBEM is inexcusably shit to accomplish this in comparison to directly using the BCD APIs, because it will end up calling the same functions, while bloating your program and adding many dependencies (both headers and libraries) that serve no purpose whatsoever other than simply making WBEM compile and link. this absolutely qualifies WBEM as shitware for this specific purpose. it has zero benefits, yet it greatly increases the size of your binary, is slower to compile and link (and also less likely to link successfully!), it is slower to use, and harder to debug. this is why I called it "shit" in my review. maybe try to focus on the part where - answering your question - I suggested a better lightweight alternative instead?

[2025-02-06 17:37] pinefin: i love how hv detection is still a discussion in this discord after my 3 or so months being in here

[2025-02-06 17:37] pinefin: i feel like its a pattern

[2025-02-06 17:53] Brit: why wouldn't it be, it's a neat topic

[2025-02-06 20:34] Matti: [replying to .: "NVM got it"]
not sure wat you meant by this question - so what is the answer?

[2025-02-06 20:34] Matti: in what way is which list invalidated?

[2025-02-06 20:37] Matti: depending on how early the stage of boot is we're talking about, it may be possible to keep a reference to the boot loader's list of loader entries instead

[2025-02-06 20:38] Matti: these would be `BLDR_DATA_TABLE_ENTRY`s, which contain a `KLDR_DATA_TABLE_ENTRY` as the first member

[2025-02-06 20:39] Matti: the various lists in `LOADER_PARAMETER_BLOCK` are made up of these

[2025-02-06 22:31] pinefin: [replying to Matti: "not sure wat you meant by this question - so what ..."]
hes saying "non-vol-memory got it" /s

[2025-02-08 16:12] .: 1. The code doesn't false flag in any real scenario, because it doesn't have any model where it might be reasonable to disable physical cores like on AMD CCDs, and its accounted in a score system where that technique along is not enough to determine that you're in a VM. 

2. Changing the CPU brand string would not be effective at all, a lot of inconsistencies would happen that will trigger detection, such as x features that are supposed to be supported but are not, i already mentioned other CPU heuristics in the first message that you just ignored, aswell as cross-referencing everything with other sources

3. The db will likely not growth to include every cpu in the world, only the most used cpu models which is enough (other cpus will likely flag other heuristics). Isn't this strategy better than checking for useless registry keys, firmware signatures that are patched in every hardener, VM files and process names like every vm detection project does? The db is the only 'bad point' of this strategy and it's a necessary workaround, but could you really suggest something better? you have done nothing but criticize the technique instead of suggesting other better ideas to detect hypervisors when hardened

4. I already said 2 times that I wont use the bcd code i sent before, in the very first message i said "I'm still looking for something more reliable" precisely because the same reason, you suggested a better alternative and I did read that, but there are 1000 other better ways to say the same thing in better tones

5. talking about the ntoskrnl being moved to another memory location when booting windows 10 22h2, I dmed you what i meant and sent the exact source code i was trying in my EFI driver

[2025-02-08 16:18] .: [replying to pinefin: "i love how hv detection is still a discussion in t..."]
yeah but its kind of harder to get interesting ideas to detect these from um than from km, you're like so limited to the point where it seems everything is unreliable or easily bypassable

[2025-02-08 18:24] Torph: isn't the whole database easily bypassible if someone uses a CPU released after your software and then doesn't update your software?

[2025-02-08 19:15] .: [replying to Torph: "isn't the whole database easily bypassible if some..."]
yes, thats why its automatically updated

[2025-02-08 19:31] sync: [replying to .: "yes, thats why its automatically updated"]
what happens when vm core count is same to host though

[2025-02-08 19:32] sync: i.e. kvm

[2025-02-08 19:59] Torph: [replying to .: "yes, thats why its automatically updated"]
yeah, so what if they don't update it or use it offline or something?

[2025-02-08 20:15] szczcur: [replying to sync: "i.e. kvm"]
"i'll just update it to detect all kvm artifacts". i.e.. it all falls apart. it all falls apart long before that but yea.

[2025-02-08 20:31] sync: [replying to szczcur: ""i'll just update it to detect all kvm artifacts"...."]
he should just call std::detect_vm

[2025-02-08 20:31] sync: i heard this detect all vm with 99.99999% accuracy

[2025-02-08 21:22] .: [replying to sync: "what happens when vm core count is same to host th..."]
then it wont work

[2025-02-08 21:22] .: [replying to Torph: "yeah, so what if they don't update it or use it of..."]
then it wont work