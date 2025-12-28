# August 2025 - Week 4
# Channel: #programming
# Messages: 169

[2025-08-18 21:50] cinder: I have experimented a bit tonight: it seems like the assertion still occurs even if I call `AllocatePool()`, but on QEMU only + debug OVMF. As I have verified Saturday night, these assertions do not occur on VMWare and, as I have tested now, they do not occur on VirtualBox either. At this point I suspect there is something wrong with the debug build of OVMF, as these assertions, if they are really caused by mistakes on my end, they should be happening on release firmware as well as they are part of MdePkg which is (I think) compiled within my EFI application, but they do not. So I guess the debug build of OVMF is a no-go and I should avoid using it.

I still haven't tested VMWare and Virtualbox with GDB to see if breakpoints trigger yet

[2025-08-18 21:54] cinder: I do not exclude OVMF not working due to me improperly building it (even if I followed the github pages article and used the official Ubuntu docker container) so I will definetly come back to it at some point. Also to my surprise chainloading works but only on VMWare and Virtualbox. Here a working debug build of OVMF may help understaning why chainloading on QEMU is troublesome.

[2025-08-18 22:06] Matti: hm, this is interesting because I exclusively use qemu with debug compiled OVMF myself

[2025-08-18 22:07] Matti: I wouldn't be so sure that ASSERT() does anything in release builds for anything compiled using edk2, so including OVMF

[2025-08-18 22:08] Matti: you can open the firmware volume (or .efi files that go into them, they should end up in the build dir) to verify this

[2025-08-18 22:10] Matti: `ASSERT ZeroMemWrapper.c(47): Buffer != ((void *) 0)`
this is still the assert you're seeing right? I can try to reproduce this if you want by purposely calling it incorrectly

[2025-08-18 22:12] cinder: [replying to Matti: "`ASSERT ZeroMemWrapper.c(47): Buffer != ((void *) ..."]
yep that's the exact assert I am seeing, from the source code it should happen when ZeroMem is called with a null pointer as buffer

[2025-08-18 22:12] Matti: yep I see it

[2025-08-18 22:14] cinder: [replying to cinder: "yep that's the exact assert I am seeing, from the ..."]
which is odd because:
```
VOID *
InternalAllocateZeroPool (
  IN EFI_MEMORY_TYPE  PoolType,
  IN UINTN            AllocationSize
  )
{
  VOID  *Memory;

  Memory = InternalAllocatePool (PoolType, AllocationSize);
  if (Memory != NULL) {
    Memory = ZeroMem (Memory, AllocationSize);
  }

  return Memory;
}
```
if InternalAllocatePool returns NULL, ZeroMem should never be called thus the assert should never happen in the first place <:ThonkThonk:753302453839069416>

[2025-08-18 22:17] Matti: can you show some context? what is the calling code doing?

[2025-08-18 22:18] Matti: and you're certain it's from you calling InternalAllocateZeroPool right? just checking

[2025-08-18 22:18] Matti: as in, if you remove that call and exit, it does not assert?

[2025-08-18 22:19] Matti: uh well you wouldn't be calling that directly I guess

[2025-08-18 22:19] Matti: but yeah

[2025-08-18 22:19] Matti: AllocateZeroPool presumably

[2025-08-18 22:20] cinder: [replying to Matti: "as in, if you remove that call and exit, it does n..."]
I didn't think of this and I should try, in the meantime here is what I think is the faulting call:
```
// Finds all available volumes within the environment.
EFI_STATUS SearchForAvailableVolumes()
{
    UINTN           HandleCount     = 0;
    EFI_STATUS      Status          = EFI_SUCCESS;
    EFI_HANDLE*     Handles         = NULL;
    EFI_GUID        SimpleFsGuid    = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_GUID        FileSysInfoGuid = EFI_FILE_SYSTEM_INFO_ID;
    PINFBOOT_VOLUME Current         = NULL;

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* EnumedVolume = NULL;
    EFI_FILE_PROTOCOL*               Root         = NULL;
    EFI_FILE_SYSTEM_INFO*            FileInfo     = NULL;

    // Locate the handle buffer from the boot services
    Status = gBS->LocateHandleBuffer(ByProtocol, &SimpleFsGuid, NULL, &HandleCount, &Handles);

    if (EFI_ERROR(Status))
    {
        LOG_ERROR(L"Failed to locate handle buffer: EFI_STATUS 0x%08x", Status);
        return Status;
    }

    LOG_INFO(L"Searching for available volumes...", 0);

    // For each available handle, enumerate the volume and add it to the linked list
    for (UINTN i = 0; i < HandleCount; i++)
    {
        // Reserve zeroed memory for the current volume
        // The troublemaker?
        Current = AllocateZeroPool( sizeof(INFBOOT_VOLUME) );

        // Assert that we have allocated
        if (!Current)
        {
            // Bad news, we are out of memory
            // TODO: do not reuse code between log.c and here, make this a wrapper or a util of some sort
            Print(L"[CRITICAL]: AllocateZeroPool failed! System may be out of memory!\n");

            Status = EFI_OUT_OF_RESOURCES;
            break;
        }
        // Omitted for brevity, function is very long
    }
```
INFBOOT_VOLUME struct:
```
// Structure defining a volume instance
typedef struct _INFBOOT_VOLUME
{
    EFI_HANDLE                          Handle;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL*    Volume;
    
    BOOLEAN                             IsReadOnly;
    UINT64                              VolumeSize;
    UINT64                              FreeSpace;
    UINT32                              BlockSize;
    CHAR16*                             VolumeLabel;

    LIST_ENTRY                          Link;

} INFBOOT_VOLUME, *PINFBOOT_VOLUME;
```

[2025-08-18 22:20] cinder: Let me know if you would like to see the full function

[2025-08-18 22:21] cinder: in the meantime I'll see what happens if I get rid of that call

[2025-08-18 22:22] Matti: alright, no this should be enough for me I think

[2025-08-18 22:23] Matti: I don't see anything immediately wrong with the surrounding code so I'll just run
`AllocateZeroPool( sizeof(INFBOOT_VOLUME) );`

[2025-08-18 22:24] Matti: hm, well this is unrelated to the assert for sure, but why not use the gSomethingSomething GUIDs defined in edk2?

[2025-08-18 22:24] cinder: I totally should, I discovered them later after writing `SearchForAvailableVolumes`

[2025-08-18 22:26] Matti: yeah so instead of assigning to your local SimpleFsGuid you can reference `gEfiSimpleFileSystemProtocolGuid` instead which is declared in the same header as the define

[2025-08-18 22:26] Matti: same for the other one, I don't know it off the top of my head but yeah

[2025-08-18 23:08] Matti: <@327017971291652096> I think you may wanna just open your .efi file in IDA before debugging in qemu

[2025-08-18 23:09] Matti: I was wrong about OVMF needing to be debug compiled, if the assert is indeed caused by your application code and not OVMF itself (probably safe to assume if you're not building from master)

[2025-08-18 23:10] Matti: but for me the difference between a debug and release compiled app is pretty plain to see in IDA

[2025-08-18 23:11] Matti: only the former has asserts in ZeroMem at all (note: InternalMemZeroMem does not assert)

[2025-08-18 23:12] Matti: LHS: release, with asserts being defined to nothing so therefore ZeroMem gets inlined and InternalMemZeroMem is called directly
RHS: debug, asserts are enabled here and ZeroMem is not inlined
[Attachments: image.png]

[2025-08-18 23:12] Matti: this is using MSVC

[2025-08-18 23:14] Matti: I'm not sure why calling `AllocateZeroPool` like you are in your code would ever end up causing zeromem to be called with a null pointer though, regardless of debug vs release

[2025-08-18 23:19] Matti: <:lillullmoa:475778601141403648> even calling ZeroMem(NULL, size) directly in the release app is no issue
that surprises me
[Attachments: image.png]

[2025-08-18 23:23] cinder: [replying to Matti: "<:lillullmoa:475778601141403648> even calling Zero..."]
well that's very interesting <:LUL:763654995396263937>

[2025-08-18 23:24] cinder: [replying to Matti: "<@327017971291652096> I think you may wanna just o..."]
yep I have binary ninja but I guess it'll do the trick, I'll see if my builds all have assert logic within and I believe they should since I am compiling as debug

[2025-08-18 23:24] elias: I think its normal for most legacy uefi to map null page

[2025-08-18 23:25] Matti: yeah that is a possible explanation I hadn't even thought of

[2025-08-18 23:25] Matti: an OS will prevent you from doing this

[2025-08-18 23:25] cinder: [replying to cinder: "which is odd because:
```
VOID *
InternalAllocateZ..."]
what is really weird is that theoretically under normal circumstances this should never happen, so why is ZeroMem getting a null pointer here even if said pointer has been checked beforehand? compiler trouble? other kind of trouble?
and why does it happen only with debug OVMF and not release?

[2025-08-18 23:25] Matti: but in firmware there are legitmate reasons to use 0, especially since this is a PA

[2025-08-18 23:26] Matti: not VA

[2025-08-18 23:26] Matti: [replying to cinder: "what is really weird is that theoretically under n..."]
yeah, I don't know the answer to this either

[2025-08-18 23:28] cinder: personally I think i'll move on for now and either send an email to EDK2 mantainers or stick to release OVMF and call it a day

[2025-08-18 23:28] cinder: but I do think they would be interested in this weirdness that is happening

[2025-08-18 23:29] elias: [replying to Matti: "but in firmware there are legitmate reasons to use..."]
for new uefi it must use proper memory protections (with paging) for all pages and also restrict access to page 0 to meet ms requirements so this is a legacy thing

[2025-08-18 23:29] cinder: either way, at least on vmware, I chainloaded windows

[2025-08-18 23:29] cinder: and I saved the video as mkv

[2025-08-18 23:29] cinder: oopsie...

[2025-08-18 23:30] Matti: [replying to elias: "for new uefi it must use proper memory protections..."]
define 'new EFI'? this is OVMF built from master with PEs linked explicitly to be page aligned

[2025-08-18 23:31] Matti: I don't think the MS MAT is enabled by default in it

[2025-08-18 23:34] cinder: well this will be all for the day, as always <@148095953742725120> plenty thanks for your help <a:partyblob:1004638051047309393>

[2025-08-18 23:37] elias: [replying to Matti: "I don't think the MS MAT is enabled by default in ..."]
MAT is for the OS to know protections for runtime drivers after boot phase, but `EFI_MEMORY_ATTRIBUTE PROTOCOL` was introduced to allow for page protections directly in the boot phase and according to microsoft, they and "their partners" (I assume OEMs) are implementing this as the default with a compatibility fallback mode

[2025-08-18 23:37] elias: https://uefi.org/sites/default/files/resources/Hardening%20the%20Core%20Enhanced%20Memory%20Protection_Beebe.pdf

[2025-08-18 23:38] elias: the uefi signing requirements also require NX compatibility now

[2025-08-18 23:39] elias: the zero page stuff is also addressed there

[2025-08-18 23:39] Matti: sorry, I got them mixed up

[2025-08-18 23:39] Matti: yes, the protocol is the one I meant

[2025-08-18 23:39] Matti: I'm aware of it

[2025-08-18 23:40] elias: I guess its not widely available yet because it was only introduced in the spec in 2022

[2025-08-18 23:41] Matti: the NX part and page protections anyway... but I'm not aware of OVMF or edk2 themselves implementing it at least by default

[2025-08-18 23:41] Matti: there might be a PCD for it, unsure

[2025-08-18 23:42] elias: yeah the only device I could find it on is my surface

[2025-08-18 23:42] elias: but I think it will be implemented everywhere in the future

[2025-08-18 23:43] Matti: I expect the same

[2025-08-18 23:43] elias: or at least it should be because we shouldn’t have entire memory rwx in 2025…

[2025-08-18 23:48] Matti: OK so I checked just to make sure, but at a quick glance I don't see why compatibility mode would be enabled anywhere in my screenshot
\- edk2 seems to be linking with NX=1 nowadays, at least when using MSVC
\- my own template app has been linking with this flag for years, ever since the protocol was made part of the spec

[2025-08-18 23:50] Matti: I haven't tested executing any memory just to be clear
but the null page was presumably mapped as you can see in the ss

[2025-08-19 10:17] Yoran: [replying to cinder: "I do not exclude OVMF not working due to me improp..."]
Like all huge and buggy software, never use your own build haha

[2025-08-19 10:17] Yoran: Download a nightly prebuilt OVMF , have a small efi.h

[2025-08-19 10:17] Yoran: These things never go away

[2025-08-19 10:18] Yoran: I had insane issues with my compiled OVMF

[2025-08-19 10:28] Yoran: [replying to Matti: "<:lillullmoa:475778601141403648> even calling Zero..."]
Can you test that with hyper-v & a VM with secure boot?

[2025-08-19 11:02] Matti: does it need to be both? I can compile OVMF with SB support enabled no issue if you think that will make a difference
if the hyper-v part is also required I'm still not disinterested per se but I just haven't got the time to learn it right now

[2025-08-19 11:06] Matti: I'm fairly sure the former won't do anything re: this on its own in edk2/OVMF, it changes more things about the compilation setup than that it actually makes a noticeable difference IME

[2025-08-19 11:06] Matti: OVMF requires SMM, ACPI S3 and TPM support for SB to work

[2025-08-19 11:09] Matti: other than that I've never really noticed any difference (e.g. NX support being enabled - this is a PCD setting in edk2, not related to secure boot)

[2025-08-19 14:45] Yoran: [replying to Matti: "I'm fairly sure the former won't do anything re: t..."]
Gotcha, so i guess youre right

[2025-08-19 14:46] Yoran: And yeah i thought maybe SB has something to do with it

[2025-08-19 23:19] twopic: Has anyone been told that learning 2s complement was considered fake engineering

[2025-08-19 23:20] twopic: Some guy who works in hardware design said that 2s complement was useless memorization

[2025-08-20 06:12] brockade: knowing what it does is useful, different CPUs have different combinations of opcodes and sometimes a compiler will emit opcode1 + opcode2 and knowing "oh that's 2s complement" is useful to make sense of it

[2025-08-20 06:13] brockade: also I'd argue that knowing 2s complement doesn't matter but being able to understand it grows your skill level so other concepts are easier to pick up

[2025-08-20 08:13] Yoran: [replying to twopic: "Some guy who works in hardware design said that 2s..."]
Is he talking about knowing how to implement it in silicone?

[2025-08-20 08:13] Yoran: Cuz otherwise that's just mad

[2025-08-20 08:13] Yoran: Its so simple to learn and used everywhere

[2025-08-20 13:43] ml: Hello, when I inject my DLL via manual mapping I get this crash: Unhandled exception at 0x000000019035CDB0 in javaw.exe: 0xC0000005: Access violation reading location 0xFFFFFFFFFFFFFFFF. It appears here (in the text file)


Sometimes the DLL injects and crashes afterward, but it injects at a different stage each time (wtf?). Also, I should mention that since I disabled TLS callbacks, the DLL reaches more advanced stages of its loading (the places it reaches have nothing to do with TLS callbacks but it still ends up crashing).
[Attachments: message.txt]

[2025-08-20 14:38] Pepsi: [replying to ml: "Hello, when I inject my DLL via manual mapping I g..."]
load your dll into ida and load symbols, calculate the RVA of the address where your crash happens and navigate to that address in ida,
then you at least can see what actaully crashes

[2025-08-20 14:53] ml: [replying to Pepsi: "load your dll into ida and load symbols, calculate..."]
let me try

[2025-08-20 20:28] Pepsi: [replying to ml: "Hello, when I inject my DLL via manual mapping I g..."]
he told me in dm he doesn't have ida, but figuring out *what* code exactly crashed wasn't really necessary

[2025-08-20 20:29] Pepsi: the relevant part of the code shows access to static TLS memory

```
000000019035CD81 8B 15 E1 1F 0A 00    mov         edx,dword ptr [1903FED68h]  
000000019035CD87 48 8B F1             mov         rsi,rcx  
000000019035CD8A 65 48 8B 04 25 58 00 00 00 mov         rax,qword ptr gs:[58h]  
000000019035CD93 41 BE 30 00 00 00    mov         r14d,30h  
000000019035CD99 48 8B 3C D0          mov         rdi,qword ptr [rax+rdx*8]  
000000019035CD9D 49 8B 1C 3E          mov         rbx,qword ptr [r14+rdi]  
000000019035CDA1 48 85 DB             test        rbx,rbx  
000000019035CDA4 75 0A                jne         000000019035CDB0  
000000019035CDA6 BB 40 00 00 00       mov         ebx,40h  
000000019035CDAB 48 03 DF             add         rbx,rdi  
000000019035CDAE EB 2D                jmp         000000019035CDDD  
-> 000000019035CDB0 8B 03                mov         eax,dword ptr [rbx] 
```

[2025-08-20 20:30] Pepsi: and his mapper didn't handle static TLS

[2025-08-20 20:31] twopic: [replying to Yoran: "Is he talking about knowing how to implement it in..."]
idk i think he works in asic

[2025-08-20 20:31] twopic: even then calling it fake engineering is a bit much

[2025-08-20 23:29] Brit: [replying to Pepsi: "load your dll into ida and load symbols, calculate..."]
whats with the influx of people not just copy pasting working manual mappers.

[2025-08-20 23:32] Pepsi: [replying to Brit: "whats with the influx of people not just copy past..."]
¯\_(ツ)_/¯

[2025-08-20 23:36] Pepsi: to be fair, i kinda see why people don't want to implement everything the regular loader does,
i think it kinda defeats the purpose of manual mapping at some point

[2025-08-20 23:49] Pepsi: one either need to paste more complete mappers or 
needs to be aware of the limitations and avoid certain features.
i think a lot of people do neither, because they are missing knowlege about those things

[2025-08-20 23:50] Brit: sure but tls is kinda shrimple

[2025-08-20 23:50] Brit: not too sure about the footprint, have not done game related stuff in a long time

[2025-08-21 08:38] varaa: stop self coding keep pasting!!

[2025-08-21 09:23] Brit: [replying to varaa: "stop self coding keep pasting!!"]
arguably better outcome than getting into something blind and then being unable to diagnose what it is you even fucked up

[2025-08-21 10:00] cinder: i think it's a good learning experience debugging and fixing something you do not understand, keeps you highly motivated and productive which leads to you eventually understanding what you're working with

[2025-08-21 10:01] cinder: copy and pasting code that already works saves time but it will remain a blackbox if no time is spent trying to understand it

[2025-08-21 10:03] cinder: plus you will eventually be forced to understand it the moment you need to expand or maintain it for whatever reason, so pasting is just delaying the inevitable process of sitting down and studying the code you brought into your project

[2025-08-21 13:52] Brit: aside from the fact that these systems are incredibly documented, to the point where not being able to implement the thing you wanted is more of a skill issue than anything else, I do not disagree.

[2025-08-21 17:20] dullard: [replying to Pepsi: "the relevant part of the code shows access to stat..."]
Thank you for posting it here for everyone’s benefit, I never understood why people try to move things to dms 😂

[2025-08-21 19:20] daax: Wrong channel.

[2025-08-22 00:08] avx: [replying to dullard: "Thank you for posting it here for everyone’s benef..."]
the gnomes in your cpu cant watch you if you move to dms

[2025-08-23 12:58] Kola: [replying to daax: "Wrong channel."]
my apologies

[2025-08-23 19:09] cinder: https://c-faq.com/decl/spiral.anderson.html

[2025-08-23 19:09] cinder: an interesting article I stumbled upon

[2025-08-23 19:35] Horsie: Thats pretty neat.

[2025-08-23 19:36] Horsie: The way deep-c does drove me insane

[2025-08-23 19:40] Horsie: <https://github.com/media-lib/c_lib/blob/master/c/Peter%20van%20der%20Linden%20-%20Expert%20C%20Programming%2C%20Deep%20C%20Secrets.pdf> Page 70

[2025-08-23 19:41] Horsie: 
[Attachments: image.png]

[2025-08-24 03:45] twopic: https://cdn.discordapp.com/attachments/1344765127383322659/1408928458481991751/Screenshot_20250823_143832.png?ex=68ab8661&is=68aa34e1&hm=9452a0978ccc7e05ece729e75a3b5238c5261ed84980f37b045692f00ba7dd0e&

[2025-08-24 03:45] twopic: btw this is an intel asic designer

[2025-08-24 09:45] Brit: I do not believe you

[2025-08-24 09:48] twopic: he is... sadly

[2025-08-24 09:49] Brit: or rather I do not believe that he took even one year of EE ever

[2025-08-24 09:49] Brit: he could well be at intel

[2025-08-24 09:49] Brit: the company is going to shit

[2025-08-24 09:50] twopic: He claimed that 2 comp was useless engineering meant for fake engineers

[2025-08-24 09:50] twopic: ...

[2025-08-24 09:51] Brit: idk if Id call 2's comp engineering per say, but not understanding it is tragic

[2025-08-24 09:52] twopic: I'm not even studying ee

[2025-08-24 09:55] twopic: <@303272276441169921> I'm not a true ce or ee student. I've taken a basic ee course but they taught me only the basics of single stage / 5 stage

[2025-08-24 09:56] Brit: I'm not either, I did maths all the way through.

[2025-08-24 09:57] twopic: [replying to Brit: "I'm not either, I did maths all the way through."]
I'm a film major

[2025-08-24 09:57] twopic: But cs is my hobby

[2025-08-24 10:11] twopic: 
[Attachments: Screenshot_20250824-031138.png]

[2025-08-24 10:12] twopic: ???????????????

[2025-08-24 10:12] twopic: This person should switch to business

[2025-08-24 10:13] twopic: He doesn't even know what a single cycle is how does he expect to design a pipeline

[2025-08-24 10:13] twopic: I don't know how intel hires people like him

[2025-08-24 10:17] twopic: <@303272276441169921>

[2025-08-24 11:41] brymko: what makes you belive he works at intel

[2025-08-24 11:55] Yoran: [replying to brymko: "what makes you belive he works at intel"]
Maybe the Intel discord tag lmao

[2025-08-24 11:55] brymko: doesn't that look a bit borked
[Attachments: image.png]

[2025-08-24 11:55] brymko: intel

[2025-08-24 11:56] brymko: the i and n are higher than the t in a normal font

[2025-08-24 11:57] avx: dsc tags have 4 characters max

[2025-08-24 11:57] avx: iirc

[2025-08-24 11:57] Yoran: [replying to twopic: ""]
Wdy care so much

[2025-08-24 13:40] x0a: so this tag comes from "intel insiders community" discord server

[2025-08-24 13:41] x0a: i think its the official "intel" discord server , because it has a custom discord tag

[2025-08-24 15:07] daax: [replying to x0a: "so this tag comes from "intel insiders community" ..."]
inb4 works at intel as an intern <:Kappa:794707301436358686>

[2025-08-24 15:40] f00d: [replying to daax: "inb4 works at intel as an intern <:Kappa:794707301..."]
at intel as screen brightness adjuster

[2025-08-24 15:59] x0a: [replying to daax: "inb4 works at intel as an intern <:Kappa:794707301..."]
how was it to work in intel?

[2025-08-24 16:02] x0a: btw why high level reverse engineers go to work in cpu companies like intel and amd?

[2025-08-24 16:21] daax: [replying to x0a: "how was it to work in intel?"]
?

[2025-08-24 16:21] daax: i don't work at intel

[2025-08-24 16:21] daax: haha

[2025-08-24 16:46] contificate: inb4 works at intel as a janitor, draws infeasible circuit diagrams on whiteboards after everybody has left

[2025-08-24 16:50] x0a: [replying to daax: "i don't work at intel"]
mb xd

[2025-08-24 18:04] Deus Vult: [replying to contificate: "inb4 works at intel as a janitor, draws infeasible..."]
Sounds all too familiar

[2025-08-24 18:05] Mikewind22: [replying to contificate: "inb4 works at intel as a janitor, draws infeasible..."]
Problem is, company implements it on next day...

[2025-08-24 19:01] twopic: [replying to brymko: "what makes you belive he works at intel"]
I've seen his badge

[2025-08-24 19:09] twopic: [replying to Yoran: "Wdy care so much"]
Because that guy was a prick

[2025-08-24 19:09] twopic: Kept claiming that he was a real engineer

[2025-08-24 19:09] UJ: To get hired at these companies you just need to grind leetcode, you dont need to actually know the material for what you are going to work on.

[2025-08-24 19:10] koyz: everyone is an engineer nowadays! look at the job titles like software/development engineer <:Kappa:807349187350888499>

[2025-08-24 19:10] twopic: [replying to UJ: "To get hired at these companies you just need to g..."]
Yep I agree

[2025-08-24 19:10] twopic: 
[Attachments: Screenshot_20250824-031619.png]

[2025-08-24 19:10] twopic: > no unis teach proper logic design

[2025-08-24 19:12] twopic: Yeah this guy didn't go to school

[2025-08-24 19:36] Brit: He's out

[2025-08-24 19:36] Brit: Probably got fired from sweeping the floors

[2025-08-24 19:37] twopic: He claims to be an real engineer

[2025-08-24 19:37] twopic: He's probably just a technician worker