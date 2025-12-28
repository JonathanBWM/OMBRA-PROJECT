# February 2024 - Week 3
# Channel: #programming
# Messages: 145

[2024-02-13 01:09] donna🤯: Does anyone have information that could point me in the right regarding PCI device tree enumeration as a kernel driver on Windows? Thanks in advance 🙂

[2024-02-13 11:32] not nezu: ```toml
# Reference: https://build-cpp.github.io/cmkr/cmake-toml
[project]
name = "test-libecc"

[fetch-content.libecc]
git = "https://github.com/libecc/libecc"
tag = "v0.9.7"

[target.libecc]
alias = "libecc::libecc"
type = "static"
headers = ["${libecc_SOURCE_DIR}/include"]
sources = ["${libecc_SOURCE_DIR}/src/*.c"]

[target.example]
type = "executable"
sources = ["src/main.c"]
link-libraries = ["libecc::libecc"]
```
`[cmake] [cmkr] error: filesystem error: directory iterator cannot open directory: No such file or directory [/home/nezu/git/test-libecc/${libecc_SOURCE_DIR}/src]`

[2024-02-13 11:32] not nezu: what is the correct way to do this?

[2024-02-13 11:32] brymko: <@162611465130475520> <@162611465130475520> <@162611465130475520> cmakr skill issue

[2024-02-13 11:33] brymko: 🤯

[2024-02-13 11:34] mrexodia: [replying to not nezu: "what is the correct way to do this?"]
You cannot do this

[2024-02-13 11:34] mrexodia: Manually list the files

[2024-02-13 11:36] mrexodia: https://github.com/build-cpp/cmkr/issues/74
[Embed: Add support for grepping based on a variable · Issue #74 · build-cp...]
Currently ${xxx_SOURCE_DIR}/blah/*.cpp doesn't work because the file doesn't exist in the cmkr universe. Most likely this would have to be expanded to a GLOB expression (which is not ideal)

[2024-02-13 11:37] not nezu: so this is specific to the fact that I'm trying to glob?

[2024-02-13 11:37] mrexodia: Globbing works, but only for files that already exist when cmkr runs

[2024-02-13 11:38] mrexodia: You are trying to glob based on a CMake variable and cmkr cannot glob for you in that case

[2024-02-13 11:38] mrexodia: Adding support for this would be quite easy, please try to implement it in cmkr

[2024-02-13 11:39] not nezu: [replying to mrexodia: "Adding support for this would be quite easy, pleas..."]
by adding support you mean `Most likely this would have to be expanded to a GLOB expression (which is not ideal)`?

[2024-02-13 11:39] mrexodia: Yes

[2024-02-13 11:39] mrexodia: Detect this case with .contains("${") and then handle it appropriately

[2024-02-13 11:40] Brit: don't like it? add support based oss mindset

[2024-02-13 11:40] mrexodia: 🤷‍♂️ it’s not hard

[2024-02-13 11:40] Brit: I'm actually not clowning you

[2024-02-13 11:40] mrexodia: But if you don’t want to you can also use a custom find module

[2024-02-13 11:40] Brit: I think it's based

[2024-02-13 11:40] mrexodia: And manually call FetchContent there

[2024-02-13 11:41] mrexodia: [replying to Brit: "I'm actually not clowning you"]
Haha, I know 😅

[2024-02-13 16:36] donna🤯: [replying to donna🤯: "Does anyone have information that could point me i..."]
So I ended up figuring out how to do it. We can enumerate the `pci.sys` device list using `IoEnumerateDeviceObjectList`. This will give us each device object associated with `pci.sys`. Then with each device object check that its a valid PDO by checking the `DO_BUS_ENUMERATED_DEVICE ` flag. Now we have a valid PCI PDO, so we can use the `IRP_MN_READ_CONFIG` to read the configuration space of the associated PDO. 

An example to read the vendor ID is as follows:
```C
io_stack_location->MinorFunction     = IRP_MN_READ_CONFIG;
io_stack_location->Parameters.ReadWriteConfig.WhichSpace = PCI_WHICHSPACE_CONFIG;
io_stack_location->Parameters.ReadWriteConfig.Buffer     = VendorId;
io_stack_location->Parameters.ReadWriteConfig.Offset     = PCI_VENDOR_ID_OFFSET;
io_stack_location->Parameters.ReadWriteConfig.Length     = sizeof(USHORT);
```

Then you can use the `!pcitree` windbg command to cross reference and check your results.

[2024-02-13 16:43] Matti: [replying to donna🤯: "So I ended up figuring out how to do it. We can en..."]
thanks, I was interested in this myself actually, but I haven't had the time to look into it

[2024-02-13 16:45] donna🤯: [replying to Matti: "thanks, I was interested in this myself actually, ..."]
no prob. was a bit of a nightmare to figure out but in hindsight its pretty straightforward tbh

[2024-02-13 17:47] daax: [replying to donna🤯: "So I ended up figuring out how to do it. We can en..."]
Ah you wanted the way that was coupled with Windows, not bare metal processing

[2024-02-13 18:03] daax: I'm assuming this is for the open-source AC project you've been putting together. If of interest you can also use the ACPI MCFG table to get the information about the config space and process it manually, more effort but less of an issue when it comes to determining whether or not you're getting trimmed results. Removes the ability for someone to interpose on IofCallDriver (via the `IopPerfCallDriver -> IopPerfLogCallEvent`) since you wind up having to after setting up the Irp. You could use both as well to cross-reference results at runtime, and validate the config space itself for corruption.

[2024-02-13 18:11] Matti: yeah but don't forget that ACPI is also not universal ground truth

[2024-02-13 18:11] Matti: and can be modified

[2024-02-13 18:11] daax: [replying to Matti: "yeah but don't forget that ACPI is also not univer..."]
You don't need to use it either

[2024-02-13 18:12] Matti: but a driver in NT is way more likely to take your approach ofc

[2024-02-13 18:12] daax: We can nickle and dime everything down to the IO space required, it all can be. The level of effort required goes up.

[2024-02-13 18:12] donna🤯: [replying to daax: "I'm assuming this is for the open-source AC projec..."]
Yea ill definitely implement some form of crosschecking thats a good idea.

[2024-02-13 18:12] Matti: [replying to daax: "We can nickle and dime everything down to the IO s..."]
I know I know

[2024-02-13 18:12] Matti: I agree

[2024-02-13 18:13] Matti: it's the same thing as with all anti cheat efforts really

[2024-02-14 15:00] Matti: I'm gonna answer your questions in reverse order since it's easier to follow along that way I think

[2024-02-14 15:03] Matti: in short: yes

There is a UEFI (well, actually PI) protocol specifically for this purpose: communicating with the SMM core (in this case to notify it of an event) by generating a software SMI if needed.
This is called a **synchronous** SMI, as opposed to an asynchronous SMI which would normally come from hardware instead. See `SmmEntryPoint` to see how the common SMI entry point in PiSmmCore distinguishes between the two types and deals with them

Also see `EFI_SMM_COMMUNICATION_PROTOCOL` in whatever header, and `SmmCommunicationCommunicate()` in `PiSmmIpl.c` in particular for its typical usage for these kinds of notifications

This is the protocol, as you can see it's pretty basic

```c
typedef
EFI_STATUS
(EFIAPI *EFI_MM_COMMUNICATE)(
  IN CONST EFI_MM_COMMUNICATION_PROTOCOL   *This,
  IN OUT VOID                              *CommBuffer,
  IN OUT UINTN                             *CommSize OPTIONAL
  );

struct _EFI_MM_COMMUNICATION_PROTOCOL {
  EFI_MM_COMMUNICATE    Communicate;
};
```
(I know these are named 'MM' and not SMM, but SMM uses the same protocol, just with a different typedef)

[2024-02-14 15:11] Matti: When `PiSmmIpl`'s entry point is called (this is a module that is part of the SMM core driver sources, but actually a separate driver normally running in DXE), it registers notifcation callbacks for a whole bunch of events that are in an predefined array, exactly the same way a regular DXE driver would.

Most of these notification calbacks will go to `SmmIplGuidedEventNotify`, which is actually completely generic and passes the notification-specific data on via the context pointer it received. ExitBootServices also goes there

Again as you can see, nothing very interesting here either

```c
VOID
EFIAPI
SmmIplGuidedEventNotify (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  UINTN  Size;

  //
  // Use Guid to initialize EFI_SMM_COMMUNICATE_HEADER structure
  //
  CopyGuid (&mCommunicateHeader.HeaderGuid, (EFI_GUID *)Context);
  mCommunicateHeader.MessageLength = 1;
  mCommunicateHeader.Data[0]       = 0;

  //
  // Generate the Software SMI and return the result
  //
  Size = sizeof (mCommunicateHeader);
  SmmCommunicationCommunicate (&mSmmCommunication, &mCommunicateHeader, &Size);
}
```

[2024-02-14 15:15] Matti: So, the answer to this is: `EFI_SMM_COMMUNICATION_PROTOCOL` exists specifically for this purpose

Note that this is still not the protocol that generates the actual SMI - that is `EFI_SMM_CONTROL2_PROTOCOL`, but this is more of an implementation detail for PiSmmIpl to worry about

[2024-02-14 15:17] Matti: So now that we know how the whole SMM clusterfuck chain can even work (or be invoked) at all, I'll try to answer your original question

[2024-02-14 15:25] Matti: 1. `ExitBootServices()` is called by bootloader at some point

2. DXE core does the usual notification dispatching for the event, causing `SmmIplGuidedEventNotify()` to be called eventually since it is registered for it

3. `SmmCommunicationCommunicate()` now decides what to do, as follows:

    **Q:** Are we in SMM? (not a real question for us, since in this case we know we are in DXE)
        **N** (our case) -> generate a software SMI via `EFI_SMM_CONTROL2_PROTOCOL` to enter SMM synchronously.
            This will get us out of PiSmmIpl in DXE and enter `SmmEntryPoint` in the actual SMM core, running in SMM (see 4)
        **Y** (impossible in our case, but not impossible in general) -> call `SmiManage` directly, which will find and invoke the actual handler for this specific type of SMI

4. `SmmEntryPoint` is reached in the SMM core driver, and we are executing in SMM
    This function validates some stuff (that we are really in SMM, that `SetVirtualAddressMap` has not been called, and that SMRAM is unlocked)
    And then finally it ends up calling the same `SmiManage` if everything checked out, and the SMI handler for the notification will be invoked

[2024-02-14 15:34] Matti: ---

Additional note probably worth adding: at least in EDK2 (and presumably in most firmwares) there are actually **two** types of notification handlers for ExitBootServices and co that go to SMM:

1. The stuff I just mentioned, which will be done by PiSmmIpl (which is part of the PI spec, as well as the protocols it uses that I mentioned above)

2. The SMM core driver itself (again, in edk2) has its own way of registering notification callbacks which are in an array of`SMM_CORE_SMI_HANDLERS`.
 Oddly enough this type does also seem to be a part of the PI spec, but the GUIDs used by the SMM core when registering the notification callbacks are definitely platform-specific - e.g. `gEdkiiSmmExitBootServicesProtocolGuid`, which as you can probably guess is EDK2 specific and not part of any spec AFAIK

[2024-02-14 15:40] Matti: found the API edk2 uses for the SMM core type notifications in (2)...

```c
/**
  Register a callback function be called when a particular protocol interface is installed.

  The MmRegisterProtocolNotify() function creates a registration Function that is to be
  called whenever a protocol interface is installed for Protocol by
  MmInstallProtocolInterface().
  If Function == NULL and Registration is an existing registration, then the callback is unhooked.

  @param[in]  Protocol          The unique ID of the protocol for which the event is to be registered.
  @param[in]  Function          Points to the notification function.
  @param[out] Registration      A pointer to a memory location to receive the registration value.

  @retval EFI_SUCCESS           Successfully returned the registration record
                                that has been added or unhooked.
  @retval EFI_INVALID_PARAMETER Protocol is NULL or Registration is NULL.
  @retval EFI_OUT_OF_RESOURCES  Not enough memory resource to finish the request.
  @retval EFI_NOT_FOUND         If the registration is not found when Function == NULL.
**/
typedef
EFI_STATUS
(EFIAPI *EFI_MM_REGISTER_PROTOCOL_NOTIFY)(
  IN  CONST EFI_GUID     *Protocol,
  IN  EFI_MM_NOTIFY_FN   Function,
  OUT VOID               **Registration
  );
```
so this is also standard and part of PI

[2024-02-14 15:41] Matti: the only thing that's not standard is the GUID it registers the actual notification for

[2024-02-14 15:42] Matti: not sure why there is no common type for that and only that

[2024-02-14 15:48] Matti: for reference, this is the actual callback function SMM core installs for ExitBootServices:

```c
/**
  This is the Event call back function is triggered in SMM to notify the Library
  the system is entering runtime phase.

  @param[in] Protocol   Points to the protocol's unique identifier
  @param[in] Interface  Points to the interface instance
  @param[in] Handle     The handle on which the interface was installed

  @retval EFI_SUCCESS SmmAtRuntimeCallBack runs successfully
 **/
EFI_STATUS
EFIAPI
S3BootScriptSmmAtRuntimeCallBack (
  IN CONST EFI_GUID  *Protocol,
  IN VOID            *Interface,
  IN EFI_HANDLE      Handle
  )
{
  if (!mS3BootScriptTablePtr->AtRuntime) {
    mS3BootScriptTablePtr->BootTimeScriptLength = (UINT32)(mS3BootScriptTablePtr->TableLength + sizeof (EFI_BOOT_SCRIPT_TERMINATE));
    SaveBootTimeDataToLockBox ();

    mS3BootScriptTablePtr->AtRuntime = TRUE;
    SaveSmmPriviateDataToLockBoxAtRuntime ();
  }

  return EFI_SUCCESS;
}
```

The **registration** is actually done by the S3 boot script, which is yet another kind of horror, related to but not the same as the SMM core driver itself, using the `EFI_MM_REGISTER_PROTOCOL_NOTIFY` protocol I just posted above

[2024-02-14 15:49] Matti: I would suggest pretending this just doesn't exist at all, or at least ignoring its existence as much as you can otherwise

[2024-02-14 15:51] Matti: the S3 boot script is special even compared to other SMM shit, and SMM is already quite shit

[2024-02-14 15:53] Matti: I can never decide which one I hate more, SMM or ACPI

[2024-02-14 15:53] Matti: probably ACPI

[2024-02-14 16:45] Timmy: Matti writing beautiful blog post as a discord message

[2024-02-14 16:45] Timmy: love it

[2024-02-14 16:50] Matti: well it ended up being a bit of a longer post than I intended, because of the 'additional note' bit at the end that isn't actually all that relevant to the question in general I think

[2024-02-14 16:52] Matti: but OTOH, depending on what you're trying to make it could just as well be very important information, as in you need to be aware that this dogshit exception exists

[2024-02-14 16:52] Matti: idk what the answer is for OP

[2024-02-15 01:22] birdy: Y'all got a good WinDbg(Preview) guide? I don't know if Preview fixed the issue with paging re-entry index being assigned at random index (for !pte command, Windows 10+). 
I am looking for a sort of brief summary of each command, since I'm still not very strong with knowing that many commands.

[2024-02-15 01:27] Matti: [replying to birdy: "Y'all got a good WinDbg(Preview) guide? I don't kn..."]
not really a fan of 'guides' myself for this sort of thing since I have the short term memory of a goldfish
but this might be something for you? https://github.com/repnz/windbg-cheat-sheet
[Embed: GitHub - repnz/windbg-cheat-sheet: My personal cheat sheet for usin...]
My personal cheat sheet for using WinDbg for kernel debugging - repnz/windbg-cheat-sheet

[2024-02-15 01:27] Matti: ignore the repo name, I'd say it's way too wordy to be a cheat sheet

[2024-02-15 01:28] Matti: which is what I was actually trying to google... my goto is this one https://theartofdev.com/windbg-cheat-sheet/
[Embed: WinDbg cheat sheet]
Working with WinDbg is kind of pain in the ass and I never remember all the commands by heart, so I write down the commands I used. Loading stuff .loadby sos mscorwks Load SOS extension (will ident…

[2024-02-15 01:30] Matti: commands in windbg and windbg preview are with very few if any exceptions exactly the same, so that's nice

[2024-02-15 01:31] Matti: windbg preview does have a new javascript based scripting engine that old windbg doesn't support AFAIK

[2024-02-15 01:32] Matti: if you're interested in learning about *that*, go to https://github.com/microsoft/WinDbg-Samples
[Embed: GitHub - microsoft/WinDbg-Samples: Sample extensions, scripts, and ...]
Sample extensions, scripts, and API uses for WinDbg. - microsoft/WinDbg-Samples

[2024-02-15 01:49] birdy: [replying to Matti: "if you're interested in learning about *that*, go ..."]
Thank you.

[2024-02-15 02:04] Matti: no worries

[2024-02-15 02:05] Matti: I didn't know the answer myself so I learned something too looking this up <:thinknow:475800595110821888>

[2024-02-15 02:08] Matti: SMM is a disaster, I prefer to avoid anything to do with it

[2024-02-15 02:09] Matti: but then that also means I feel like I don't know about it as in depth as I probably should

[2024-02-15 03:47] birdy: Any blogs on EFI stuff? I never learned much about it yet.

[2024-02-15 03:59] Matti: [replying to birdy: "Any blogs on EFI stuff? I never learned much about..."]
anything in particular you want to learn about it? UEFI development, reverse engineering? or more like an overview of important concepts in genneral just to get an idea basiccally?

I think https://secret.club/2020/05/26/introduction-to-uefi-part-1.html will work fine as an introduction if you're new to UEFI
but tbh it doesn't go into a lot of detail about anything in particular, so if you're looking for something more in depth you'll have to wait for part 2, which will never come out
[Embed: Introduction to UEFI: Part 1]
Hello, and welcome to our first article on the site! Today we will be diving into UEFI. We are aiming to provide beginners a brief first look at a few topics, including:

[2024-02-15 04:01] Matti: then  https://standa-note.blogspot.com/2021/04/reverse-engineering-absolute-uefi.html is a great article on how to reverse an entire UEFI BIOS image or module(s) in one
[Embed: Reverse engineering (Absolute) UEFI modules for beginners]
This post introduces how one can start reverse engineering UEFI-based BIOS modules. Taking Absolute as an example, this post serves as a tut...

[2024-02-15 04:03] Matti: as well as https://standa-note.blogspot.com/2020/12/experiment-in-extracting-runtime.html, but that article is more focused on something that is very interesting by itself, but probably not as useful in practice as the first two

[2024-02-15 04:07] Matti: last one - this is 100% focused on UEFI development, not reversing, but it covers so many things you'll commonly see in UEFI that it's worth reading regardless tbh
https://tianocore-docs.github.io/edk2-UefiDriverWritersGuide/draft/edk2-UefiDriverWritersGuide-draft.pdf

[2024-02-15 04:14] snowua: [replying to Matti: "anything in particular you want to learn about it?..."]
I didn't know you guys dropped an introductory UEFI blog. Always been interested in UEFI

[2024-02-15 04:14] snowua: Will have to check it out

[2024-02-15 04:15] Matti: I hate it <:harold:704245193016344596>

[2024-02-15 04:16] Matti: but that's mostly because shatter thought it would be a great idea to name it 'part 1', and then we both decided it would be a great idea to never do a part 2

[2024-02-15 04:16] Matti: as an introductory article it's fine I guess

[2024-02-15 04:21] birdy: [replying to Matti: "anything in particular you want to learn about it?..."]
I'm very strong already in reverse engineering, just never was exposed to anything UEFI related

[2024-02-15 04:21] birdy: Thanks for resources.

[2024-02-15 04:30] Matti: yeah I assumed as much tbh - so I think the first link will probably be most useful to learn about UEFI in general, but there are definitely some quirks to the way it all comes together in the final firmware image (e.g. tons of GUIDs that seem to be all-pervasive and totally meaningless) that make reading the second more reversing-focused article absolutely worth it too

[2024-02-15 04:33] Matti: nothing about reversing in UEFI is particularly difficult tbh, but some of it will make no sense at all if you're not at least somewhat familiar with most of this

[2024-02-15 04:48] birdy: Alright I'll read through them. 😎

[2024-02-15 04:48] birdy: Once I know the basics I can just reverse to learn the rest.

[2024-02-16 16:50] kert: Hello, I have encountered a problem that I've been trying to solve for some time now. I'm using clang-cl LLVM with VS22, and I'm attempting to force the compiler to place all floats on the stack instead of declaring them in the .rdata section. I've tried dozens of approaches, including a significant number of clang-cl parameters, but they have yielded no results. I am aware that I can force the compiler to load a single float from the register by passing it to a function which does `asm("" : "=r"(value) : "0"(value) : )`, but this solution is not optimal as I have 365 files in my project, and replacing each implicitly and explicitly used float with such a macro is tedious. I also know I could probably write an LLVM pass, but I haven't done so in the past, and the deadline is approaching quickly, which makes the task much more challenging.

[2024-02-16 16:53] okokmasta: hello i have a little question about something i am working on PE files, i change the AddressOfEntryPoint, its originally : ```0x4016A5```  
and i change it to ```0x4022748```  but the executeable still go by the original entry point before starting to execute my new one 

If anyone has already successfully changed the entrypoint to patch the previous entrypoint code before it execute 

looks like that some loader functions are executing ...
Any hints, thanks ! 🙂

[2024-02-16 17:37] qwerty1423: [replying to okokmasta: "hello i have a little question about something i a..."]
DLLs or loader functions might be expecting the old EntryPoint address if you hadn't done it correctly

[2024-02-16 17:37] qwerty1423: http://www.rohitab.com/discuss/topic/33534-how-to-change-entry-point-of-pe-file/
[Embed: How to change entry point of pe file - Programming]
How to change entry point of pe file - posted in Programming: I put in (for example) winamp.exe at the bottom of the file calc.exe, and i want when someone run winamp to first run calc.exe, then winam

[2024-02-16 17:38] qwerty1423: some of the people in this forum are a bit salty btw

[2024-02-16 17:41] okokmasta: Thanks mate! Yeah there are loader code exécuting and its returning on oldEp i'll read through this ressource and comeback to you if i still habe questions 🙂

[2024-02-16 17:49] okokmasta: Damn thats exactly what i'm doing :/

[2024-02-16 18:43] birdy: [replying to qwerty1423: "some of the people in this forum are a bit salty b..."]
NaCl*

[2024-02-16 18:43] qwerty1423: yea...

[2024-02-16 18:43] birdy: Sodium Chloride.

[2024-02-16 19:40] okokmasta: soooo... haven't found proper solution yet...

[2024-02-16 19:50] okokmasta: this is indeed some ntdll loaders funtions being executed in text section... i need to avoid that and get my EntryPoint executed as Real EntryPoint

[2024-02-16 20:10] x86matthew: check for TLS callbacks

[2024-02-16 21:00] okokmasta: yeah ! this is it, IDA dbg does automatic breakpoint, on it, and label it as TLS callback

[2024-02-16 22:44] mrexodia: [replying to kert: "Hello, I have encountered a problem that I've been..."]
Smells like a game hack

[2024-02-16 22:44] mrexodia: <:sus:899071675415560203>

[2024-02-16 22:44] mrexodia: Or malware

[2024-02-16 22:45] mrexodia: But you can check out the latest secret.club post for an LLVM pipeline that will support this

[2024-02-16 22:45] okokmasta: hmm how could i achieve  my goal tho... i have encoded the text section, and i have my decoder in the same .text, decoder is labbelled as entrypoint... but have some tls callback that are executed in .text sect,  goal would be to programatically locate this address (to not encode/decode it ) ; or prevent its execution before decoding routine

[2024-02-16 22:48] okokmasta: i have made beautiful 100% pseudo code polymorphic decoder, packing routine works flawless with elf but wanna adapt it to pe dont want to rebuild full pe tho would like to do it without having to rebuild a full pe wich is already well known

[2024-02-16 22:53] mrexodia: [replying to okokmasta: "hmm how could i achieve  my goal tho... i have enc..."]
You should pretty much not write malware

[2024-02-16 22:53] mrexodia: 🧠

[2024-02-16 23:27] okokmasta: not cool mate

[2024-02-16 23:27] okokmasta: im not writing malware tho

[2024-02-16 23:27] okokmasta: 🧠

[2024-02-16 23:27] okokmasta: https://github.com/araout42/python-packer
[Embed: GitHub - araout42/python-packer]
Contribute to araout42/python-packer development by creating an account on GitHub.

[2024-02-16 23:28] okokmasta: 
[Attachments: full.png]

[2024-02-16 23:32] okokmasta: i've already made packer that rebuilt a pe, wich is not my goal here, i'd like only to patch/inject with full encoding of .text

[2024-02-16 23:32] okokmasta: [replying to mrexodia: "You should pretty much not write malware"]
and why is that ? 🤔

[2024-02-16 23:39] qwerty1423: [replying to okokmasta: "hmm how could i achieve  my goal tho... i have enc..."]
attach a debugger and single step till you reach the end of the loop

[2024-02-16 23:41] qwerty1423: with given details i can't really know what else should be done

[2024-02-16 23:45] okokmasta: well i will give you futher details tomorrow if your willing to help me with this, thank you very much !!! ❤️

[2024-02-17 01:57] Deleted User: [replying to okokmasta: "and why is that ? 🤔"]
its mean..

[2024-02-17 01:57] okokmasta: yes 😦 but its okey man is very talented and have big background, me im nobody ^^

[2024-02-17 16:39] kert: [replying to mrexodia: "But you can check out the latest secret.club post ..."]
Thanks

[2024-02-17 20:13] okokmasta: i guess i've found my solution, might remove TLS callback, or add new TLS callback instead of changing entryPointAddress and then patch it

[2024-02-18 14:48] liͥmͣaͫ: [replying to okokmasta: "hmm how could i achieve  my goal tho... i have enc..."]
you can use the linker to layout your code in a certain order, ms supports $ suffixes to section names which are ordered alphanumerically, this is how the initerm array for global constructor/destructor calls is build
```
.text$001unencoded
.text$002startmarker
.text$003encoded
.text$004endmarker
```
make sure they dont collide with any other markers used by stuff like the stl, the compiler itself or the runtime

[2024-02-18 14:49] liͥmͣaͫ: you may also have to tell the linker explicitly to include some certain symbols so they dont get removed as unreferenced

[2024-02-18 14:51] okokmasta: Thanks <@367331073648099329> i am not compiling.. tho, i am only doing patching Exe that are already compiled

[2024-02-18 14:51] liͥmͣaͫ: in that case create a new section, or try to expand an existing one as long as you dotn change relative positions between data and text sections

[2024-02-18 14:53] okokmasta: Yep thats what im doing i extend the text section and insert some code into it, that will decode the encoded code section

[2024-02-18 14:53] okokmasta: Problem is that i encode Tls callback code, that is executed before EP

[2024-02-18 14:55] liͥmͣaͫ: you can move the original content of the text section back within the image and put the packing/decoder stuff before it, that is if you are able to rewrite all rva's used in the image, aka headers and reloc section to adapt for the changes made by shifting the image in memory

[2024-02-18 14:56] liͥmͣaͫ: or again just append the decoder to the image as a new section

[2024-02-18 14:56] liͥmͣaͫ: the decoder will never be able to be encoded itself tho

[2024-02-18 14:58] okokmasta: [replying to liͥmͣaͫ: "you can move the original content of the text sect..."]
Oh yes, but anyway whatever i do tls callback will be executed before my entryPoint so as it's within text section its encoded and crash

[2024-02-18 14:59] liͥmͣaͫ: yeah so you cannot have that encoded ?! how is this not obvious

[2024-02-18 14:59] okokmasta: Everthing works, patching decodind, injection all working perfect Just problem is that:(

[2024-02-18 15:00] okokmasta: [replying to liͥmͣaͫ: "yeah so you cannot have that encoded ?! how is thi..."]
Yes Either that, or remove it, or find a way to get decoder exec first

[2024-02-18 15:00] okokmasta: I wanna try put decoder as first tls callback instead of changing EntryPoint

[2024-02-18 15:02] liͥmͣaͫ: you know you gonna have to make sure the exe/dll does not have tls callbacks itself in the first place, or defer them yourself, right ?

[2024-02-18 15:02] liͥmͣaͫ: or just decide not to support them

[2024-02-18 15:02] okokmasta: If i add a new tls callback entry ?

[2024-02-18 15:02] liͥmͣaͫ: its gotta be the first entry that does all the decode/unpacking

[2024-02-18 15:02] okokmasta: Yes!

[2024-02-18 15:05] okokmasta: I'd have to modify the tls struct in the file and add entry at first position, from what i saw tls function finish with ret instruction so might not be too much complex

[2024-02-18 15:07] okokmasta: Or i can locate tls code in the file, and not encode it, but decoder will encode it so if its executed afterwards it'll crash

[2024-02-18 15:07] okokmasta: Thanks a lot <@367331073648099329> ❤️

[2024-02-18 16:20] diversenok: Small note: the first parameter of `CoCreateInstanceEx` is not an IID, it's a CLSID

[2024-02-18 16:20] diversenok: What object are you trying to create? And why do you specify `CLSCTX_REMOTE_SERVER`?

[2024-02-18 16:21] diversenok: Also, you do `CoInitializeEx` for `COINIT_MULTITHREADED` which might not work for all objects