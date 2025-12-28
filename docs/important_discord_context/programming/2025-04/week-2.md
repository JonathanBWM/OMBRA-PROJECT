# April 2025 - Week 2
# Channel: #programming
# Messages: 123

[2025-04-12 02:07] rin: I am trying to read a file that is in use by another process to achieve this I am using `NtQuerySystemInformation`, right now I am at the point of looping through the `SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX` structures. does anyone have a good resource on how to interpret this structure. I was reading https://web.archive.org/web/20240122161954/https://www.x86matthew.com/view_post?id=hijack_file_handle but it doesn't really match my use case.

[2025-04-12 02:11] Matti: sure

[2025-04-12 02:11] Matti: 
[Attachments: main.cpp]

[2025-04-12 02:11] Matti: this is a few years old so a bit of DIY may be needed

[2025-04-12 02:12] Matti: it also doesn't do nearly all of the things mentioned in that blog post

[2025-04-12 02:12] Matti: this literally just enumerates and prints open handles

[2025-04-12 02:14] Matti: I mean, in general it doesn't try to do anything clever to begin with lol, maybe that's a better way to word it

[2025-04-12 02:14] Matti: but the API you are talking about does suck in a bunch of ways

[2025-04-12 02:15] Matti: the one that's probably least obvious and most annoying to debug is the one that caused me to add the line
`SYSTEM_HANDLE_INFORMATION_EX Dummy; // To prevent getting STATUS_INFO_LENGTH_MISMATCH twice`

[2025-04-12 02:15] Matti: both the regular and -Ex versions of the API have this strange (IMO broken/buggy) behaviour IIRC

[2025-04-12 02:20] rin: [replying to Matti: ""]
right now I think my program does pretty much the same thing. I guess what I am asking is information on interpreting the actual data like is it possible do like test if its a file handle -> if its a file handle what is the path of the file.

[2025-04-12 02:20] Matti: sure, for the first one you can enumerate `\ObjectTypes`

[2025-04-12 02:21] Matti: you can rely on the TypeIndex only in some ways

[2025-04-12 02:21] Matti: namely

[2025-04-12 02:21] Matti: if the index of handle A is the same as that of handle B, they are of the same type

[2025-04-12 02:21] Matti: but the indices themselves have no meaning and differ across versions

[2025-04-12 02:22] Matti: so if you want the names of all object types, I'd enumerate `\ObjectTypes`

[2025-04-12 02:23] Matti: if you only need the type ID of a specific object, one thing I tend to do myself is just create an object of this type and then query its type ID

[2025-04-12 02:24] Matti: I don't remember off the top of my head if you can get the type IDs from enumerating the NT object directory

[2025-04-12 02:24] Matti: but usually you only need one or the other IME

[2025-04-12 02:24] Matti: for your second question - that's a lot harder

[2025-04-12 02:25] Matti: at least generally speaking, and if you want to cover all corner cases

[2025-04-12 02:25] rin: [replying to Matti: "if you only need the type ID of a specific object,..."]
I thought about this but I thought is would be better to use  constants if they are available, I have not been able to find any.

[2025-04-12 02:25] Matti: querying a file's path is surprisingly complex

[2025-04-12 02:25] Matti: but it's not complex for the usual/regular case

[2025-04-12 02:25] Matti: [replying to rin: "I thought about this but I thought is would be bet..."]
nope, no constants

[2025-04-12 02:26] Matti: the IDs are generated at boot time in fact

[2025-04-12 02:26] Matti: so depending on your BCD config you can even have different type IDs after a reboot

[2025-04-12 02:27] rin: [replying to Matti: "nope, no constants"]
cool design 😄 🔫

[2025-04-12 02:27] Matti: it is

[2025-04-12 02:27] Matti: why would they be constants

[2025-04-12 02:27] rin: [replying to Matti: "but it's not complex for the usual/regular case"]
GetFinalPathNameByHandleA?

[2025-04-12 02:28] Matti: there is a data structure that distributes the type objects into buckets, which in turn affect the type ID.... or something like this, the details are vague

[2025-04-12 02:28] Matti: but it's simply to make lookup more efficient

[2025-04-12 02:28] rin: [replying to Matti: "why would they be constants"]
idk just makes sense to have a certain handle type have a constant. but I am probably missing something.

[2025-04-12 02:29] Matti: well yes, for example object types are added all the time

[2025-04-12 02:30] Matti: and I think at least once even removed(?) (whatever object type was used for NtWaitHighLowEventPair)

[2025-04-12 02:30] rin: I am trying to read this in a library but the author added a bunch of anime pictures so impossible https://rayanfam.com/topics/reversing-windows-internals-part1/
[Embed: Reversing Windows Internals (Part 1) - Digging Into Handles, Callba...]
We write about Windows Internals, Hypervisors, Linux, and Networks.

[2025-04-12 02:31] Matti: how did I manage to guess who the author was before opening the link

[2025-04-12 02:34] Matti: in general I would recommend checking out the winobjex64 source code for this

[2025-04-12 02:40] Matti: [replying to rin: "GetFinalPathNameByHandleA?"]
oh I almost didn't see this question... but which  user mode API is 'best' to use  does depend on a few factors

[2025-04-12 02:40] Matti: for this one PH is the source code to refer to

[2025-04-12 02:41] Matti: doing this from kernel mode is what is fraught with unexpected dangers

[2025-04-12 02:42] Matti: in general all of the user mode APIs are safe in that sense of course, but some are 'better' because they either require fewer process access rights, or else just because they are faster

[2025-04-12 02:42] rin: [replying to Matti: "oh I almost didn't see this question... but which ..."]
it worked btw, I just tested it.

[2025-04-12 02:43] Matti: ok cool

[2025-04-12 02:43] Matti: I would be surprised if it didn't to be honest

[2025-04-12 02:47] rin: [replying to Matti: "I would be surprised if it didn't to be honest"]
I read this wrong I am suprised it worked.

[2025-04-12 02:48] Matti: ```c
/**
 * Gets the file name of a process' image.
 *
 * \param ProcessId The ID of the process.
 * \param FileName A variable which receives a pointer to a string containing the file name. You
 * must free the string using PhDereferenceObject() when you no longer need it.
 *
 * \remarks This function only works on Windows Vista and above. There does not appear to be any
 * access checking performed by the kernel for this.
 */
NTSTATUS PhGetProcessImageFileNameByProcessId(
    _In_opt_ HANDLE ProcessId,
    _Out_ PPH_STRING *FileName
    )
{
    NTSTATUS status;
    PVOID buffer;
    USHORT bufferSize = 0x100;
    SYSTEM_PROCESS_ID_INFORMATION processIdInfo;

    buffer = PhAllocate(bufferSize);

    processIdInfo.ProcessId = ProcessId;
    processIdInfo.ImageName.Length = 0;
    processIdInfo.ImageName.MaximumLength = bufferSize;
    processIdInfo.ImageName.Buffer = buffer;

    status = NtQuerySystemInformation(
        SystemProcessIdInformation,
        &processIdInfo,
        sizeof(SYSTEM_PROCESS_ID_INFORMATION),
        NULL
        );

    if (status == STATUS_INFO_LENGTH_MISMATCH)
    {
        // Required length is stored in MaximumLength.

        PhFree(buffer);
        buffer = PhAllocate(processIdInfo.ImageName.MaximumLength);
        processIdInfo.ImageName.Buffer = buffer;

        status = NtQuerySystemInformation(
            SystemProcessIdInformation,
            &processIdInfo,
            sizeof(SYSTEM_PROCESS_ID_INFORMATION),
            NULL
            );
    }

    if (!NT_SUCCESS(status))
    {
        PhFree(buffer);
        return status;
    }

    // Note: Some minimal/pico processes have UNICODE_NULL as their filename. (dmex)
    if (RtlIsNullOrEmptyUnicodeString(&processIdInfo.ImageName))
    {
        PhFree(buffer);
        return STATUS_UNSUCCESSFUL;
    }

    *FileName = PhCreateStringFromUnicodeString(&processIdInfo.ImageName);
    PhFree(buffer);

    return status;
}
```
(PH)
this was the API I was thinking of, but as you can see it's not gonna be very useful for your use case (handles, not PIDs)

[2025-04-12 02:48] Matti: it's surprising in that it requires 0 access to the process

[2025-04-12 02:49] rin: [replying to rin: "I read this wrong I am suprised it worked."]
tho not really, it only returned the unc path location of the exe or file handle, not the actual name.

[2025-04-12 02:50] rin: when tried on all available handles

[2025-04-12 02:50] Matti: well you are asking for a win32 path no?

[2025-04-12 02:50] Matti: if you are really using that API

[2025-04-12 02:51] Matti: step 1 would be to ask for an NT path instead, regardless of whether that even fixes this particular issue

[2025-04-12 02:52] Matti: [replying to rin: "tho not really, it only returned the unc path loca..."]
but wdym by this? can you give an expected vs actual output example

[2025-04-12 02:53] rin: [replying to Matti: "but wdym by this? can you give an expected vs actu..."]
one sec

[2025-04-12 02:57] Matti: mm, I was thinking of an NTQIF class, but I think `NtQueryFullAttributesFile` is probably the 'generic' API most suited for doing this

[2025-04-12 02:57] Matti: if in doubt, using filetest is faster

[2025-04-12 09:56] diversenok: [replying to Matti: "sure, for the first one you can enumerate `\Object..."]
Enumerating `\ObjectTypes` requires admin and there is no um function to open type objects anyway, so you only get names. You probably meant `NtQueryObject` with `ObjectTypesInformation`. It returns both names and hanle type indexes

[2025-04-12 09:58] diversenok: [replying to rin: "tho not really, it only returned the unc path loca..."]
The only actual name is the native one (`\Device\something\something`). A Win32 name is not guaranteed to exist or be the one you expect

[2025-04-12 09:59] diversenok: And yeah, there are so many ways and related caveats with querying file names

[2025-04-12 10:00] diversenok: For instance, it can deadlock on some handles (e.g., pipes)

[2025-04-12 10:02] diversenok: Internally, the name of the device and the on-device part of the name are two different queries

[2025-04-12 10:03] diversenok: `NtQueryInformationFile` with `FileVolumeNameInformation` gives the device name (like `\Device\HarddiskVolume1`) and is pretty safe, as it doesn't need much from the target driver and just looks up its name

[2025-04-12 10:05] diversenok: `NtQueryInformationFile` with `FileNameInformation` retrieves the on-device name (like `Windows\system32\ntdll.dll`) and requires the driver to implement it and cooperate. Thus, it can fail, deadlock, or produce weird results

[2025-04-12 10:06] diversenok: There is also `NtQueryInformationFile` with `FileNormalizedNameInformation` that is similar to the previous one but asks the filesystem to normalize the name (i.e., expand DOS 8.3 names, for instance). Same caveats but less support from drivers

[2025-04-12 10:08] diversenok: You can also use `NtQueryObject` with `ObjectNameInformation`; internally, it will query both `FileVolumeNameInformation` and `FileNameInformation` and combine them. It can also deadlock on some handles due to the latter call

[2025-04-12 10:11] diversenok: The Win32 `GetFinalPathNameByHandle` relies on these (which exactly - depends on the flags) and does extra processing that attempts to find a suitable drive letter

[2025-04-12 10:13] diversenok: It issues some IOCLTs to `\\.\MountPointManager` for that

[2025-04-12 10:13] rin: <@503274729894051901> in the end duplicate handle worked

[2025-04-12 10:13] diversenok: What was the goal again, I got distracted 😅

[2025-04-12 10:14] rin: handle hijacking

[2025-04-12 10:15] diversenok: Ahh, using locked files

[2025-04-12 10:15] diversenok: Keep in mind that giving random handles to `GetFinalPathNameByHandle` can hang it for indefinite time

[2025-04-12 10:16] diversenok: System Informer does all filename querying on a dedicated thread with timeouts

[2025-04-12 10:16] diversenok: Ready to kill it if it hangs

[2025-04-12 10:16] rin: noted

[2025-04-12 10:17] rin: [replying to diversenok: "System Informer does all filename querying on a de..."]
so sysinformer also just goes through handles assuming its a file handle?

[2025-04-12 10:17] diversenok: It checks the type index of the handle entry first

[2025-04-12 10:18] diversenok: `NtQueryObject` with `ObjectTypesInformation` gives you the list of all type names, and indexes (and other info)

[2025-04-12 10:19] diversenok: But yeah, when displaying handles of a process or searching through all handles on the system, it needs to duplicate a lot

[2025-04-12 10:21] rin: my question would be why would the handles returned by `NtQuerySystemInformation` be invalid but duplicating them makes them valid.

[2025-04-12 10:22] diversenok: It doesn't return handles, it returns their values

[2025-04-12 10:22] diversenok: Handles are per-process

[2025-04-12 10:22] rin: [replying to diversenok: "Handles are per-process"]
I was thinking something similar

[2025-04-13 23:10] Matti: [replying to diversenok: "Enumerating `\ObjectTypes` requires admin and ther..."]
right you are

[2025-04-13 23:11] Matti: I should give you an edit button to fix up my posts for me, you seem to do a better job than me most of the time

[2025-04-13 23:18] rin: <@148095953742725120> <@503274729894051901> observation you guys might be interested in. when calling `NtQueryObject` with `ObjectInformationLength` of zero seems to get rid of the hanging problem.

[2025-04-13 23:19] Matti: wdym? the hang AFAIK isn't reliably reproducible in the first place

[2025-04-13 23:20] Matti: and if it were, I still don't see how doing this could fix it if another CPU is doing file I/O (causing this to happen) at the same time

[2025-04-13 23:23] rin: [replying to Matti: "wdym? the hang AFAIK isn't reliably reproducible i..."]
mb, I thought I saw something but maybe wrong

[2025-04-13 23:26] diversenok: [replying to Matti: "wdym? the hang AFAIK isn't reliably reproducible i..."]
A blocking pipe with a pending read (or something along those lines) always hangs name querying

[2025-04-13 23:27] diversenok: I'll try the zero-length query idea. Not sure how it can help though

[2025-04-13 23:28] Matti: oh that's interesting. and shit lol

[2025-04-13 23:29] Matti: is that the only case though? from the source code that attempts to prevent it I get the idea that it can also semi-randomly happen in other cases

[2025-04-13 23:30] Matti: otherwise I don't get the fairly convoluted second thread design

[2025-04-13 23:43] diversenok: Also some console handles, I think

[2025-04-13 23:44] rin: [replying to diversenok: "Also some console handles, I think"]
I think I saw this

[2025-04-13 23:45] rin: I hanged on a console handle

[2025-04-13 23:46] diversenok: [replying to Matti: "otherwise I don't get the fairly convoluted second..."]
The reason for this second thread design is that you don't get a prior warning that it will hang when you attempt to query names from whatever handles exist right now on the system

[2025-04-13 23:47] diversenok: It is random in a sense that it depends on what other programs are currently doing

[2025-04-13 23:47] Matti: yeah

[2025-04-13 23:47] diversenok: But the same handle in the same state can trigger it reliably

[2025-04-13 23:48] Matti: yeah that follows, so the second thread is still necessary because you can't pause the rest of the system to query a single filename each time

[2025-04-13 23:48] diversenok: Not sure what you mean by pausing the system

[2025-04-13 23:49] Matti: [replying to diversenok: "It is random in a sense that it depends on what ot..."]
re: this

[2025-04-13 23:50] Matti: it's still inherently a race condition on a multi cpu system even if you had a reliable (sounds possible now maybe) `WillQueryHang()`

[2025-04-13 23:50] diversenok: I don't think it's a race condition per se

[2025-04-13 23:51] Matti: hm yeah maybe not, I was thinking of the case when given a handle, but referencing the handle is fine

[2025-04-13 23:51] Matti: in which case you have a FILE_OBJECT pointer

[2025-04-13 23:51] Matti: which can't suddenly become a different file with the same value in the way a HANDLE can

[2025-04-13 23:52] diversenok: Like, one example where I know I can reliably get a handle that hangs `NtQueryObject` is Beyond Compare. It keeps a pipe handle in some state that causes it. It doesn't matter if I suspend the process as it won't change anything about the handle

[2025-04-13 23:52] Matti: [replying to diversenok: "A blocking pipe with a pending read (or something ..."]
but, this sounds more like the state of a file object

[2025-04-13 23:52] diversenok: Indeed it is

[2025-04-13 23:53] Matti: so not really sure what I think now, I'd need some code to reproduce I guess

[2025-04-13 23:53] Matti: [replying to diversenok: "Like, one example where I know I can reliably get ..."]
neat

[2025-04-13 23:55] Matti: standard edition should suffice right?

[2025-04-13 23:55] Matti: I've used BC in the past and I don't actually really like it very much lol

[2025-04-13 23:57] diversenok: Well, I said reliably and now I cannot reproduce, lol

[2025-04-13 23:57] diversenok: It worked 10 minutes ago

[2025-04-13 23:57] Matti: <:lillullmoa:475778601141403648>

[2025-04-13 23:57] Matti: typical