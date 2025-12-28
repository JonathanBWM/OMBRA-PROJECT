# September 2024 - Week 3
# Channel: #programming
# Messages: 1174

[2024-09-09 10:58] luci4: Did Microsoft completely replace `FileCompletionInformation`  with `FileReplaceCompletionInformation` in `ZwSetFileInformation` ? The former no longer appears in MSDN, although I can't get any of them to work

[2024-09-09 11:37] Matti: [replying to luci4: "Did Microsoft completely replace `FileCompletionIn..."]
haven't tested this cause I'm at work, but pretty sure these should both still work

[2024-09-09 11:37] Matti: what status are you getting back?

[2024-09-09 11:38] Matti: if it's STATUS_INVALID_PARAMETER, you're probably opening the file synchronously

[2024-09-09 11:40] luci4: [replying to Matti: "if it's STATUS_INVALID_PARAMETER, you're probably ..."]
I get STATUS_SUCCESS, I had a parameter wrong 😅

[2024-09-09 11:41] Matti: cool
yet another problem solved 😎

[2024-09-09 11:41] luci4: Now my problem lies with ZwRemoveIoCompletion, as I'm not receiving the key I sent

[2024-09-09 11:41] luci4: It doesn't work at all actually

[2024-09-09 11:41] luci4: I can't even print its status, LOL

[2024-09-09 11:42] Matti: hmm that is odd, out of all the IOCP system calls that one is probably the simplest one IME

[2024-09-09 11:43] Matti: [replying to luci4: "I can't even print its status, LOL"]
so what happens instead

[2024-09-09 11:43] Matti: execution continues as if nothing happened?

[2024-09-09 11:43] luci4: [replying to Matti: "execution continues as if nothing happened?"]
The threads I spawned to loop on it just...wait

[2024-09-09 11:43] Matti: ah so it's blocking before reaching `printf` or whatever?

[2024-09-09 11:44] luci4: [replying to Matti: "ah so it's blocking before reaching `printf` or wh..."]
Yeah...

[2024-09-09 11:46] luci4: Could it be because I am spawning the threads looping on NtRemoveIoCompletion BEFORE starting to send anything to it?

[2024-09-09 11:46] Matti: no

[2024-09-09 11:46] Matti: that should be fine

[2024-09-09 11:47] Matti: I'm not sure off the top of my head tbh - it might be dependent on the file object and how it was opened/created

[2024-09-09 11:47] Matti: idem dito for the IOCP

[2024-09-09 11:47] Matti: can you show the code?

[2024-09-09 11:48] luci4: [replying to Matti: "can you show the code?"]
Sure, 1sec

[2024-09-09 11:52] luci4: So first and foremost I am creating a completion port and some threads:

```c
    ULONG NumberOfThreads{ NtCurrentPeb()->NumberOfProcessors };
    BYTE  CreatedThreads {};

    if (!NT_SUCCESS(NtCreateIoCompletion(&CompletionPort, IO_COMPLETION_ALL_ACCESS, nullptr, NULL)))
    {
        return 0;
    }

    do {

        HANDLE ThreadHandle{};

        if (NT_SUCCESS(RtlCreateUserThread(NtCurrentProcess(), nullptr, FALSE, 0, 0, 0, (PUSER_THREAD_START_ROUTINE)ChildThread, nullptr, &ThreadHandle, nullptr)))
        {
            NtClose(ThreadHandle);
            CreatedThreads++;
        }

        --NumberOfThreads;
    } while (NumberOfThreads);

    return CreatedThreads;
```

[2024-09-09 11:53] luci4: Then I have my function which calls `NtCreateFile` on the file I want and sends to a port:

[2024-09-09 11:53] luci4: ```c
    HANDLE            FileHandle      {};
    IO_STATUS_BLOCK   StatusBlock     {};
    UNICODE_STRING    FileName        {};
    OBJECT_ATTRIBUTES ObjectAttributes{};
    
    RtlDosPathNameToNtPathName_U(Path, &FileName, NULL, NULL);
    InitializeObjectAttributes(&ObjectAttributes, &FileName, OBJ_CASE_INSENSITIVE, NULL, NULL);

    NTSTATUS Status{};

    if (!NT_SUCCESS(Status = NtCreateFile(&FileHandle, FILE_READ_DATA | FILE_WRITE_DATA | DELETE, &ObjectAttributes, &StatusBlock,
                    NULL, FILE_ATTRIBUTE_NORMAL, NULL, FILE_OPEN, FILE_NON_DIRECTORY_FILE | FILE_NO_INTERMEDIATE_BUFFERING , NULL, NULL)))
    {
        return;
    }
```

[2024-09-09 11:54] luci4: This function then allocates some memory for the struct I am passing to the port:

[2024-09-09 11:56] luci4: ```c
   PFILE_STRUCT FileStruct = (PFILE_STRUCT)RtlAllocateHeap(NtCurrentPeb()->ProcessHeap, HEAP_ZERO_MEMORY, sizeof(FILE_STRUCT));

   if (FileStruct == NULL)
   {
       NtClose(FileHandle);
       return;
   }

   FileStruct->FileHandle     = FileHandle;
   FileStruct->BytesToCopy    = FileSize;

   // if the file is smol use the preallocated memory
   if (FileStruct->BytesToCopy < SMALL_FILE)
   {
       FileStruct->State     = STATE_READ_FILE;
       FileStruct->Buffer    = (PBYTE)SmallFileArea;
       FileStruct->ChunkSize = FileStruct->BytesToCopy;
   }

   FILE_COMPLETION_INFORMATION CompletionInformation{ .Port = CompletionPort, .Key = FileStruct };

   if (!NT_SUCCESS(Status = NtSetInformationFile(FileHandle, &StatusBlock, &CompletionInformation, sizeof(CompletionInformation), FileCompletionInformation)))
   {
       NtClose(FileHandle);
       return;
   }
```

[2024-09-09 11:57] luci4: Finally the threads previously created should be chilling here:

```c
    IO_STATUS_BLOCK IoStatusBlock{};
    NTSTATUS        Status{};

    PVOID Context{};
    auto FileStruct = reinterpret_cast<PFILE_STRUCT>(Context);

    while (TRUE)
    {        
        printf("Status: %lu\n", NtRemoveIoCompletion(CompletionPort, &Context, &Context, &IoStatusBlock, NULL));

        printf("Buffer: %p", FileStruct->Buffer);

    }
```

[2024-09-09 11:58] luci4: I'm taking the same thing in KeyContext and ApcContext because I want to share the same struct between threads later on, so they can each do something

[2024-09-09 11:59] luci4: Everything works perfectly besides `NtRemoveIoCompletion`

[2024-09-09 12:00] luci4: Maybe the NtCreateFile flags are wrong

[2024-09-09 12:01] Matti: hmm I have to say I can't see anything that looks wrong to me here

[2024-09-09 12:03] luci4: I tried running it and:

[2024-09-09 12:03] Matti: it's possible your NtCreateFile flags don't implicitly include SYNCHRONIZE, but you are correctly opening the file for async I/O so I'm pretty sure you should be receiving STATUS_PENDING if a wait is needed

[2024-09-09 12:04] Matti: what is the flag for explicit async I/O called again? have you tried adding that

[2024-09-09 12:04] luci4: 
[Attachments: image.png]

[2024-09-09 12:04] luci4: it stops there

[2024-09-09 12:05] luci4: tried it with some files

[2024-09-09 12:05] Matti: it blocking in general indicates to me the IOCP queue simply is empty

[2024-09-09 12:05] Matti: how/where are you calling NtSetIOCP?

[2024-09-09 12:05] diversenok: One small unrelated suggestion: check the result of `RtlDosPathNameToNtPathName_U`; the function can fail if the process is not long path aware and make things a bit harder to troubleshoot

[2024-09-09 12:06] Matti: oh yeah

[2024-09-09 12:06] luci4: [replying to Matti: "how/where are you calling NtSetIOCP?"]
...isn't `NtSetFileInformation` enough?

[2024-09-09 12:06] luci4: I thought that it would have received it + the key context after that

[2024-09-09 12:06] Matti: there's even a _WithStatus variant IIRC explicitly because of the old API design being dumb

[2024-09-09 12:06] Matti: [replying to luci4: "...isn't `NtSetFileInformation` enough?"]
nope!

[2024-09-09 12:06] Matti: IOCPs work like this

[2024-09-09 12:07] luci4: [replying to Matti: "nope!"]
<:nomore:927764940276772925>

[2024-09-09 12:07] luci4: I have been stuck on this for like a week on and off

[2024-09-09 12:07] Matti: you've got someone(s) doing work (producers), and notifying others (consumers) when they are done, i.e. when the work is ready to be processed

[2024-09-09 12:07] Matti: the notification part is done by calling NtSetIOCP

[2024-09-09 12:08] Matti: getting new work to process is done in a consumer thread by calling NtRemoveIOCP

[2024-09-09 12:08] luci4: Ah, so after `NtSetFileInformation` I need to call `NtSetIoCompletion` to ACTUALLY send it

[2024-09-09 12:08] Matti: yes

[2024-09-09 12:08] Matti: from 1...infinity times

[2024-09-09 12:09] Matti: every time you do, 1 item will be ready for processing

[2024-09-09 12:09] luci4: Do I still need to specify the key??

[2024-09-09 12:09] Matti: you can actually notify for multiple items at a time using NtSetIoCompletionEx, if needed

[2024-09-09 12:09] Matti: [replying to luci4: "Do I still need to specify the key??"]
yeah

[2024-09-09 12:09] Matti: err... I think so anyway

[2024-09-09 12:10] luci4: [replying to Matti: "yeah"]
I'm gonna try rn

[2024-09-09 12:10] Matti: tbh I only really use IOCPs for ALPC, not actual files

[2024-09-09 12:10] Matti: the win32 docs for whatever the win32 version of NtRemove is should state this

[2024-09-09 12:11] luci4: ```
    _In_ NTSTATUS IoStatus,
    _In_ ULONG_PTR IoStatusInformation
```
I wonder what these two do

[2024-09-09 12:11] luci4: I'll just pass null

[2024-09-09 12:11] luci4: and see what happens

[2024-09-09 12:12] Matti: I think you can specify an empty/0 key, it's just possibly useful information you can optionally pass for the consumer to use

[2024-09-09 12:12] Matti: <https://learn.microsoft.com/en-us/windows/win32/fileio/postqueuedcompletionstatus>

[2024-09-09 12:12] Matti: ^ NtSet

[2024-09-09 12:12] Matti: <https://learn.microsoft.com/en-us/windows/win32/api/ioapiset/nf-ioapiset-getqueuedcompletionstatus>
^ NtRemove

[2024-09-09 12:12] luci4: Woah it worked now!

[2024-09-09 12:12] luci4: NtRemoveIoCompletion returned status 0

[2024-09-09 12:12] luci4: I didn't receive the key for some reason

[2024-09-09 12:13] luci4: hallelujah for progress
[Attachments: image.png]

[2024-09-09 12:13] nox: Hehe

[2024-09-09 12:13] nox: Interesting

[2024-09-09 12:13] Matti: cool

[2024-09-09 12:14] Matti: this part still sounds weird to me - if you pass a key in with NtSet, you should get it back in NtRemove

[2024-09-09 12:14] Matti: [replying to luci4: "I didn't receive the key for some reason"]
this part

[2024-09-09 12:14] luci4: I passed the PFILE_STRUCT  in KeyContext and NULL in ApcContext

[2024-09-09 12:15] Matti: FWIW here's some code I use to loop over NtRemove (with an ALPC port, not file object)
```c
NTSTATUS
NTAPI
WorkerThread(
    _In_ PVOID Parameter
    )
{
    const PALPC_PORT_ASSOCIATE_COMPLETION_PORT AlpcAssociatedCompletionPort =
        static_cast<PALPC_PORT_ASSOCIATE_COMPLETION_PORT>(Parameter);

    const HANDLE CompletionPort = AlpcAssociatedCompletionPort->CompletionPort;
    ULONG_PTR CompletionKey;
    PVOID ApcContext;
    IO_STATUS_BLOCK IoStatusBlock;

    while (true)
    {
        // Wait for a new entry to be posted to the queue
        const NTSTATUS Status = NtRemoveIoCompletion(CompletionPort,
                                                    reinterpret_cast<PVOID*>(&CompletionKey),
                                                    &ApcContext,
                                                    &IoStatusBlock,
                                                    nullptr);
        if (Status != STATUS_SUCCESS)
        {
            if (Status == STATUS_INVALID_HANDLE ||
                (Status >= STATUS_ABANDONED && Status <= STATUS_ABANDONED_WAIT_63))
                break; // Port closed

            Printf(L"NtRemoveIoCompletion error 0x%08lX\n", Status);
            break;
        }

        if (IoStatusBlock.Status == STATUS_PENDING)
            continue;

        // You can optionally check the completion key here to receive custom
        // messages from other threads (using NtSetIoCompletion)

        // Execute the ALPC callback
        AlpcCallback();
    }
    
    return NtTerminateThread(NtCurrentThread, STATUS_SUCCESS);
}
```

[2024-09-09 12:16] Matti: what happens if you pass `(PVOID)0x1234` as KeyContext to NtSet?

[2024-09-09 12:16] luci4: Could it be the way I'm making the conversion?

[2024-09-09 12:17] Matti: if it crashes, it probably should be a pointer to your key

[2024-09-09 12:17] luci4: ```
    IO_STATUS_BLOCK IoStatusBlock{};
    NTSTATUS        Status{};

    PVOID Context{};
    auto FileStruct = reinterpret_cast<PFILE_STRUCT>(Context);


    while (TRUE)
    {        
        Status = NtRemoveIoCompletion(CompletionPort, &Context, &Context, &IoStatusBlock, NULL);


        printf("Buffer: %p", FileStruct->Buffer);

    }
```

[2024-09-09 12:17] Matti: I'm pretty sure the cause is just that

[2024-09-09 12:17] Matti: you're reusing the same variable for 2 'out' parameters

[2024-09-09 12:18] luci4: [replying to Matti: "you're reusing the same variable for 2 'out' param..."]
I'm gonna try changing em

[2024-09-09 12:19] luci4: This part should be fine I think, `FileStruct` is a pointer to the struct I allocated on the process heap
```c

    FILE_COMPLETION_INFORMATION CompletionInformation{ .Port = CompletionPort, .Key = FileStruct };

    if (!NT_SUCCESS(Status = NtSetInformationFile(FileHandle, &StatusBlock, &CompletionInformation, sizeof(CompletionInformation), FileCompletionInformation)))
    {
        printf("Failed to send file to completion port: %lu\n", Status);
        NtClose(FileHandle);
        return;
    }

    if (!NT_SUCCESS(Status = NtSetIoCompletion(CompletionPort, FileStruct, NULL, NULL, NULL)))
    {
        printf("Failed to send packet to completion port: %lu\n", Status);
        NtClose(FileHandle);
        return;
    }
```

[2024-09-09 12:20] Matti: ah, well I mean doing this I'm pretty sure the key context should not be literally 0 on the other end

[2024-09-09 12:21] Matti: but there's no need for the key to be meaningful to the kernel in any way

[2024-09-09 12:21] Matti: [replying to Matti: "what happens if you pass `(PVOID)0x1234` as KeyCon..."]
like this should work

[2024-09-09 12:21] nox: What I am wondering is, how do you know all this

[2024-09-09 12:22] Matti: or (probably unnecessary but I can't test here...)
```
ULONG_PTR Key = 0x1234;
NtSet(..., &Key,...);
```

[2024-09-09 12:22] Matti: [replying to nox: "What I am wondering is, how do you know all this"]
well I wrote the ALPC code above and I remember it

[2024-09-09 12:22] Matti: idk what else to say

[2024-09-09 12:22] nox: Ah, I see

[2024-09-09 12:22] Matti: by 'wrote' I also mean doing the reverse engineering work required for it to work I should add

[2024-09-09 12:23] Matti: since there was and still is no working ALPC client or server code available online anywhere

[2024-09-09 12:23] Matti: so that's why I remember it particularly well

[2024-09-09 12:23] nox: Hmmmmm seems like a pain

[2024-09-09 12:23] Matti: I've been meaning to finish and open source it for ages, but time...

[2024-09-09 12:23] luci4: [replying to Matti: "ah, well I mean doing this I'm pretty sure the key..."]
It's still 0, even after making this change:
```
    IO_STATUS_BLOCK IoStatusBlock{};
    NTSTATUS        Status{};

    PVOID Context{};
    PVOID ApcContext{};
    auto FileStruct = reinterpret_cast<PFILE_STRUCT>(Context);

    while (TRUE)
    {        
        Status = NtRemoveIoCompletion(CompletionPort, &Context, &ApcContext, &IoStatusBlock, NULL);

        printf("Status: %ul\n", Status);
        printf("Buffer: %p\n", FileStruct->Buffer);

    }
```

[2024-09-09 12:24] luci4: So the problem is definitely the key

[2024-09-09 12:24] luci4: or maybe I receive the key as the fkin apc context?

[2024-09-09 12:25] Matti: wait wait, going one step too fast here

[2024-09-09 12:25] luci4: It could also be `auto FileStruct = reinterpret_cast<PFILE_STRUCT>(Context);`

[2024-09-09 12:25] Matti: if this doesn't crash, you actually got a valid pointer

[2024-09-09 12:25] Matti: but otherwise, try just doing (PVOID)0x1234 like I said

[2024-09-09 12:25] Matti: in the Set

[2024-09-09 12:25] Matti: and then print the KeyContext, not your FileStruct

[2024-09-09 12:26] Matti: I'm not 100% sure that code is going to work the way you're expecting it to...

[2024-09-09 12:26] luci4: Sir yes sir

[2024-09-09 12:27] nox: I touched grass for too long

[2024-09-09 12:27] nox: Need to get back behind my screen

[2024-09-09 12:27] luci4: [replying to Matti: "but otherwise, try just doing (PVOID)0x1234 like I..."]
It worked!!!!

[2024-09-09 12:28] Matti: 😎

[2024-09-09 12:28] nox: Matti does it once again

[2024-09-09 12:28] Matti: I knew it would

[2024-09-09 12:28] luci4: ok watch this

[2024-09-09 12:30] luci4: ...fuck

[2024-09-09 12:31] Matti: damn
bit of an anticlimax

[2024-09-09 12:32] luci4: HEHE I DID IT

[2024-09-09 12:33] luci4: just as u thought I died

[2024-09-09 12:33] luci4: i got one of those anime flashbacks with the power of friendship and shit

[2024-09-09 12:34] luci4: [replying to Matti: "damn
bit of an anticlimax"]

[Attachments: image.png]

[2024-09-09 12:35] luci4: This fixed it

[2024-09-09 12:35] luci4: ```
    IO_STATUS_BLOCK IoStatusBlock{};
    NTSTATUS        Status{};

    PVOID Context{};
    PVOID ApcContext{};
    PFILE_STRUCT FileStruct;

    while (TRUE)
    {        
        Status = NtRemoveIoCompletion(CompletionPort, &Context, &ApcContext, &IoStatusBlock, NULL);

        printf("Status: %lu\n", Status);

        FileStruct = reinterpret_cast<PFILE_STRUCT>(Context);

        printf("File handle from struct: %p\n", FileStruct->FileHandle);

    }
```

[2024-09-09 12:35] Matti: [replying to luci4: "i got one of those anime flashbacks with the power..."]
damn

[2024-09-09 12:35] nox: Can I ask what you are writing? Ransomware?

[2024-09-09 12:36] Matti: [replying to luci4: "```
    IO_STATUS_BLOCK IoStatusBlock{};
    NTSTA..."]
yep this looks better

[2024-09-09 12:36] luci4: [replying to nox: "Can I ask what you are writing? Ransomware?"]
Nah my ransomware just uses winapi for this

[2024-09-09 12:36] Matti: your casting to PFILE_STRUCT before actually receiving the key looked suspect

[2024-09-09 12:36] luci4: it's gonna be an rclone copy since every tool I found that does that was slow af

[2024-09-09 12:37] Matti: problem: slow
solution: IOCPs

correct decision

[2024-09-09 12:37] luci4: This one will use NtApi and a custom server, so u can actually transfer files around fast

[2024-09-09 12:37] luci4: and not wait decades

[2024-09-09 12:37] nox: Interesting

[2024-09-09 12:38] luci4: What I am doing is allocating 15MB, for a memory location I call the `hot zone` since most files are under that size

[2024-09-09 12:38] luci4: So all files under 15MB will be read there

[2024-09-09 12:39] Matti: ahh, no ALPC so no ALPC completion lists 😔

[2024-09-09 12:39] luci4: The worker threads will each be doing a call

[2024-09-09 12:39] Matti: that would improve performance by putting your allocation in nonpaged pool

[2024-09-09 12:40] luci4: For example, when the program receives a file it will set the struct flag to READ and send it to the completion port

[2024-09-09 12:40] Matti: to achieve the same for a regular file object I suppose the closest you can get would be acquiring SeLockMemoryPrivilege and then allocating using `NtAllocateUserPhysicalPages`

[2024-09-09 12:41] luci4: [replying to luci4: "For example, when the program receives a file it w..."]
Which will call NtReadFile  and switch the state flag to WRITE, then send it to the completion port again

[2024-09-09 12:41] luci4: based on the state, it will know that it needs to call NtWriteFile

[2024-09-09 12:43] luci4: the algo is fairly complex but if I implement it correctly it will be very fast

[2024-09-09 12:43] Matti: just thinking... is the first step necessary?

[2024-09-09 12:44] luci4: [replying to luci4: "Which will call NtReadFile  and switch the state f..."]
this is what I mean
```
    while (TRUE)
    {        
        Status = NtRemoveIoCompletion(CompletionPort, &Context, &ApcContext, &IoStatusBlock, NULL);
        FileStruct = reinterpret_cast<PFILE_STRUCT>(Context);

        switch (FileStruct->State)
        {
        case STATE_READ_FILE:

        default:
        }

    }
```

[2024-09-09 12:44] Matti: like, can't you call NtRead right away, and then post when the file is ready for writing?

[2024-09-09 12:44] luci4: [replying to Matti: "like, can't you call NtRead right away, and then p..."]
I thought about that, but the thread that will post to the ports will also travel the paths you give it

[2024-09-09 12:44] Matti: ah if your consumers are also going to be calling NtRead (again) then maybe not

[2024-09-09 12:44] luci4: and I wouldn't want to block it much

[2024-09-09 12:45] luci4: [replying to Matti: "ah if your consumers are also going to be calling ..."]
I also plan to make it work for large files, like 10TB.

[2024-09-09 12:45] Matti: yeah, but you can have multiple threads doing post remember
just, doing path traversal multithreaded will be a complex issue

[2024-09-09 12:46] luci4: [replying to Matti: "yeah, but you can have multiple threads doing post..."]
The path traversal is done by a single thread tbh

[2024-09-09 12:46] Matti: not wrong

[2024-09-09 12:46] luci4: I just raised its priority

[2024-09-09 12:46] luci4: to CRITICAL

[2024-09-09 12:46] luci4: LOL

[2024-09-09 12:47] Matti: you could also consider doing path traversal to find all files, and storing the tree in memory
...*before* starting to read anything

[2024-09-09 12:47] Matti: but this will undoubtedly cause some issues due to files that no longer exist, or were only just newly created

[2024-09-09 12:48] Matti: well you can't really avoid such things in general, but the chance will be much greater due to the time difference

[2024-09-09 12:48] Matti: [replying to Matti: "you could also consider doing path traversal to fi..."]
the reads would then be multithreaded, ofc

[2024-09-09 12:48] Matti: otherwise there is no benefit

[2024-09-09 12:50] luci4: [replying to luci4: "I also plan to make it work for large files, like ..."]
The algo I came up goes like:

For large files It will first attempt to allocate 50% of the size. If it fails, it will try 25%, then 10%, 5% and finally 1%.
If say the 25% allocation succeeds, it will call NtReadFile to read as much as it can, then increment the read offset with the size of the allocation . After writing, it will call NtReadFile again in that same memory location, overwriting the memory with the next 25%

[2024-09-09 12:50] luci4: Meaning, it will be read in 4~ chunks

[2024-09-09 12:52] luci4: [replying to Matti: "you could also consider doing path traversal to fi..."]
Hmmmm

[2024-09-09 12:52] luci4: Rn it just applies filters before sending them away

[2024-09-09 12:52] luci4: it skips over files of 0 bytes

[2024-09-09 12:52] luci4: files with `FILE_ATTRIBUTE_SYSTEM`

[2024-09-09 12:53] Matti: [replying to luci4: "The algo I came up goes like:

For large files It ..."]
that sounds fine, just a minor optimization suggestion would be to query the total available memory (physical only or committable, up to you) and store that somewhere
depending on the file size, for very large files you will be able to skip the first N allocation attempts knowing there is no way for them to succeed anyway

[2024-09-09 12:53] nox: I was thinking something similar

[2024-09-09 12:54] nox: Trying to calculate the size of the buffer

[2024-09-09 12:55] Matti: [replying to luci4: "Rn it just applies filters before sending them awa..."]
ok if it's really just doing that then probably the overhead of calling NtSetIOCP is going to be so small as to not make a difference

[2024-09-09 12:55] Matti: compared to NtWhateverTheFuckIteratesDirectories

[2024-09-09 12:55] Matti: I hate that syscall

[2024-09-09 12:56] luci4: [replying to Matti: "compared to NtWhateverTheFuckIteratesDirectories"]
I am using FindFirstFileExW, but I'm gonna change it to the syscall equivalent

[2024-09-09 12:56] luci4: Another problem is that I preallocate a lot of memories for the directories as it is

[2024-09-09 12:57] Matti: [replying to Matti: "compared to NtWhateverTheFuckIteratesDirectories"]
NtQueryDirectoryFile[Ex]

[2024-09-09 12:57] luci4: Each one is 512 * sizeof(WCHAR)

[2024-09-09 12:57] Matti: I knew that...

[2024-09-09 12:57] luci4: Since as you know paths can get fat as shit

[2024-09-09 12:59] luci4: [replying to Matti: "that sounds fine, just a minor optimization sugges..."]
I think NtQuerySystemInformation would work for that

[2024-09-09 13:00] Matti: yep

[2024-09-09 13:01] luci4: [replying to luci4: "Each one is 512 * sizeof(WCHAR)"]
The memory leak is that I am allocation 15k of them LOL

[2024-09-09 13:01] luci4: What if the user wants to move C:\ and D:\?

[2024-09-09 13:01] Matti: that would be for physical memory, if you want committable it may be process-dependent which means NtQueryInformationProcess

[2024-09-09 13:01] luci4: [replying to luci4: "The memory leak is that I am allocation 15k of the..."]
Using a std::vector would be slow as shit

[2024-09-09 13:01] luci4: So I just make a giant allocation which I cast into an array

[2024-09-09 13:01] luci4: Accessing said array is pretty fast

[2024-09-09 13:02] luci4: [replying to luci4: "The memory leak is that I am allocation 15k of the..."]
This only puts me at 16MB though

[2024-09-09 13:02] luci4: So its not the end of the world

[2024-09-09 13:02] Matti: yeah, I would also go with allocating up front, even knowing the wastage

[2024-09-09 13:03] Matti: most people have more than 16 MB RAM nowadays

[2024-09-09 13:03] luci4: [replying to Matti: "yeah, I would also go with allocating up front, ev..."]
It also helps that I dont use recursive traversal

[2024-09-09 13:03] luci4: I came up with my own little algorithm to avoid recursion

[2024-09-09 13:03] luci4: Since DFS recursion is slow af

[2024-09-09 13:04] luci4: And I want the tool to be blazingly fast 🔥 🦀

[2024-09-09 13:04] nox: The real bottleneck is going to be network speeds tho

[2024-09-09 13:04] Matti: [replying to luci4: "Each one is 512 * sizeof(WCHAR)"]
I do think the limit is more than this nowadays if you use windows 10 long paths btw
but you can special case this per file if you get whatever status it is telling you the path is even longer

[2024-09-09 13:05] luci4: [replying to nox: "The real bottleneck is going to be network speeds ..."]
Can't really avoid it tbh

[2024-09-09 13:05] Matti: 512 is an OK compromise between 256 which is definitely too little IMO

[2024-09-09 13:05] Matti: and infinity

[2024-09-09 13:05] luci4: Winsock go brr

[2024-09-09 13:05] Matti: or whatever the new max length is

[2024-09-09 13:05] luci4: [replying to Matti: "or whatever the new max length is"]
Its like 32k LOL

[2024-09-09 13:05] Matti: yeah exactly

[2024-09-09 13:06] nox: What the?

[2024-09-09 13:06] nox: Isnt that a bit overkill

[2024-09-09 13:06] Matti: no sane person uses that, so special case this for the 1 or 2 insane file paths that may exist

[2024-09-09 13:06] dtb: [replying to Matti: "or whatever the new max length is"]
32.767 with `\\?\` prefix

[2024-09-09 13:06] dtb: iirc

[2024-09-09 13:07] Matti: `\\?\` prefix

[2024-09-09 13:07] Matti: that makes double insanity

[2024-09-09 13:07] Matti: or wait, is it a requirement for long paths in the first place

[2024-09-09 13:07] dtb: yep

[2024-09-09 13:07] Matti: no idea as I haven't used them

[2024-09-09 13:07] Matti: lol awesome

[2024-09-09 13:07] dtb: specifies extended length

[2024-09-09 13:08] dtb: idk when you would **ever** need a path that long, but it's nice to have ig

[2024-09-09 13:08] dtb: lmfao

[2024-09-09 13:08] luci4: [replying to Matti: "yeah exactly"]
This is what I allocate  
```
WCHAR(*Directories)[STRING_SIZE] = (WCHAR(*)[STRING_SIZE])RtlAllocateHeap(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_DIRECTORIES * (STRING_SIZE * sizeof(WCHAR)));
```

[2024-09-09 13:08] luci4: Accessing the directories becomes very fast since they're in an array

[2024-09-09 13:08] luci4: so no wcslen bullshit

[2024-09-09 13:23] Matti: yeah, I'd do (and do) the same thing since it's (1) easy and (2) fast

[2024-09-09 13:24] Matti: but don't overestimate (2), you're still doing file I/O which is going to be the dominant factor here

[2024-09-09 13:25] Matti: there is a registry cheat code to force NTFS to use nonpaged allocations, which *probably* means you can traverse any disk on the system with 0 actual disk accesses

[2024-09-09 13:25] Matti: but this probably requires a reboot to take effect

[2024-09-09 13:31] luci4: [replying to Matti: "but don't overestimate (2), you're still doing fil..."]
Yeah... I don't think there's getting around that and low bandwidth/slow network speed

[2024-09-09 13:34] luci4: I'm gonna take a little break, then come back and start work on the switch cases

[2024-09-09 13:36] diversenok: The max filename length is 32k chars because it's the maximum amount `UNICODE_STRING` can address

[2024-09-09 13:36] Matti: yea

[2024-09-09 13:37] Matti: well 32K - 1 I assume

[2024-09-09 13:38] diversenok: They don't have to be zero-terminated though

[2024-09-09 13:39] diversenok: NTFS allows the on-volume path to be up to the limit of 0x7FFF characters

[2024-09-09 13:39] Matti: yeah I know, but it's just one of those things Rtl APIs are gonna assert on regardless

[2024-09-09 13:39] Matti: at least on a checked kernel

[2024-09-09 13:39] diversenok: Maybe

[2024-09-09 13:39] Matti: it might work fine on a free kernel

[2024-09-09 13:40] Matti: but that is living dangerous

[2024-09-09 13:40] diversenok: The funny thing is if you create a file very close to the limit, you won't be able to address it with an absolute path

[2024-09-09 13:40] elias: [replying to Matti: "yeah I know, but it's just one of those things Rtl..."]
huh really?

[2024-09-09 13:40] diversenok: Since the volume name + the on-volume one don't fit into `UNICODE_STRING`

[2024-09-09 13:41] Matti: [replying to elias: "huh really?"]
no I made it the fuck up

[2024-09-09 13:41] elias: <:skully:940450497008132137>

[2024-09-09 13:41] diversenok: Appending `\Device\HardDiskVolumeN` can overflow the length

[2024-09-09 13:41] Matti: yep

[2024-09-09 13:42] Matti: IMO they should've prevented this by making the limit slightly less than it is, but there's an argument to be made that since you don't know the volume count, you can't know how many characters to subtract

[2024-09-09 13:43] Matti: I mean, you know the volume count at runtime, but not at compile time

[2024-09-09 13:44] diversenok: I mean, the only impact is the file don't exactly having an absolute path you can use/query

[2024-09-09 13:44] diversenok: Still possible to create/open fine with a relative one

[2024-09-09 13:44] Matti: yeah I know

[2024-09-09 13:44] Matti: but that's a pretty big impact

[2024-09-09 13:44] Matti: compared to having `insane` or `insane - EPSILON` chars available for the path

[2024-09-09 13:45] diversenok: The NT namespace allows nesting way beyond a `UNICODE_STRING` limit, and it's fine

[2024-09-09 13:45] Matti: idk if I agree with that either <:lillullmoa:475778601141403648>

[2024-09-09 13:46] dtb: [replying to diversenok: "The max filename length is 32k chars because it's ..."]
That actually makes a lot of sense, was curious why it was that high

[2024-09-09 13:46] diversenok: I think each path component like a directory object can have its relative name up to the `UNICODE_STRING` limit

[2024-09-09 13:46] dtb: Thank you!

[2024-09-09 13:46] Matti: but unfortunately symlinks make it necessary sometimes I guess

[2024-09-09 13:46] Matti: although, do they

[2024-09-09 13:46] Matti: now unsure

[2024-09-09 13:46] Matti: I'm sure <@651054861533839370> can think of a reason why nesting should be possible past 32K chars

[2024-09-09 13:47] Matti: and I'll hate the reason

[2024-09-09 13:47] diversenok: But it's funny that you can have named objects that make `NtQueryObject(ObjectNameInformation)` return `STATUS_NAME_TOO_LONG`

[2024-09-09 13:48] diversenok: 'cause it doesn't fit 😅

[2024-09-09 13:48] Matti: NtQueryObject(ObjectNameInformation) always returns an error status anyway 😎

[2024-09-09 13:48] Matti: it just picks a random one if it can't find something to complain about

[2024-09-09 13:48] Matti: at least, I suspect that's how it works

[2024-09-09 13:49] diversenok: I don't think I've seen it doing that

[2024-09-09 13:49] Matti: well no, it doesn't do that

[2024-09-09 13:49] Matti: it just feels that way cause it's a shit API that requires doing pointer math

[2024-09-09 13:49] Matti: or am I thinking of the Name**s** API

[2024-09-09 13:50] Matti: possibly both

[2024-09-09 13:50] diversenok: `ObjectTypesInformation` maybe? It packs things very strangely

[2024-09-09 13:50] Matti: ohhh yeah, that'll be the one

[2024-09-09 13:50] Matti: one(s)... I'm not letting ObjectNameInformation off the hook that easily

[2024-09-09 13:51] Matti: I'm still pretty sure it sucks in some way, I just may have forgotten

[2024-09-09 13:52] Matti: it certainly sucks for file_objects cause it can deadlock, IIRC

[2024-09-09 13:52] Matti: but like even besides that

[2024-09-09 13:53] diversenok: Well, I wouldn't really blame `NtQueryObject` since does what it can and asks the corresponding driver for the device-relative path

[2024-09-09 13:53] diversenok: It's the driver who deadlocks

[2024-09-09 13:53] Matti: sure that's fair

[2024-09-09 13:53] Matti: but calling NtQueryObject is the way for the user to get to the deadlock scenario

[2024-09-09 13:53] diversenok: Yeah

[2024-09-09 13:54] Matti: and it's not exactly obvious (nor documented, obviously) behaviour

[2024-09-09 13:55] Matti: https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/ne-ntifs-_object_information_class lmao
[Embed: OBJECT_INFORMATION_CLASS (ntifs.h) - Windows drivers]
The OBJECT_INFORMATION_CLASS enumeration type represents the type of information to supply about an object.

[2024-09-09 13:55] diversenok: A definitely-not-dealocking option would've been great, true

[2024-09-09 13:55] Matti: I think it may be missing *some* members there microsoft friendos

[2024-09-09 13:56] diversenok: Here, I fixed it 😂 https://ntdoc.m417z.com/object_information_class
[Embed: OBJECT_INFORMATION_CLASS - NtDoc]
OBJECT_INFORMATION_CLASS - NtDoc, the native NT API online documentation

[2024-09-09 13:57] Matti: man it gets better
the ones that are there supposedly return types prefixed PUBLIC_ and contain bullshit members like `Reserved[10]`

[2024-09-09 13:57] Matti: <https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/ns-ntifs-_public_object_basic_information>

[2024-09-09 13:57] Matti: now where is ACTUAL_OBJECT_BASIC_INFORMATION

[2024-09-09 13:57] diversenok: Ugh

[2024-09-09 13:58] Matti: [replying to diversenok: "Here, I fixed it 😂 https://ntdoc.m417z.com/object_..."]
awesome, are you planning to become geoff chappell's successor?

[2024-09-09 13:58] diversenok: `winternl.h` vibes

[2024-09-09 13:58] Matti: cause we need one

[2024-09-09 13:59] diversenok: Maybe

[2024-09-09 15:58] luci4: [replying to Matti: "that would be for physical memory, if you want com..."]
You would think `NtQueryInformationProcess` would be able to tell you that but nope! Not even `NtQuerySystemInformation` with `SystemProcessInformation` tells you the max commit size!

After some research I have found that I seemingly need `CommitLimitBytes` from this structure: https://ntdoc.m417z.com/system_memory_usage_information
[Embed: SYSTEM_MEMORY_USAGE_INFORMATION - NtDoc]
SYSTEM_MEMORY_USAGE_INFORMATION - NtDoc, the native NT API online documentation

[2024-09-09 15:59] luci4: It corresponds to the `SystemFullMemoryInformation` flag, which is conveniently not on MSDN

[2024-09-09 16:00] luci4: Thank you NtDoc for healing my project

[2024-09-09 16:07] luci4: Although I do wonder what the difference between `AvailableBytes` and `CommitLimitBytes` is. I guess the former also takes into acount page files. None of them give me the per-process limit though

[2024-09-09 16:14] luci4: I wonder if creating a job object and setting  `MaximumWorkingSetSize` to the previously retrieved `CommitLimitBytes` would give my process "unlimited" power

[2024-09-09 16:14] luci4: That would be a neat hack, if possible

[2024-09-09 16:19] luci4: [replying to luci4: "I wonder if creating a job object and setting  `Ma..."]
I don't even need a job object, I could simply do `SetProcessWorkingSetSize`

[2024-09-09 17:15] Matti: [replying to luci4: "You would think `NtQueryInformationProcess` would ..."]
sorry, in the context of a specific process it's probably gonna be called the process working set size - but double check the NT internals book for this (or maybe someone else knows this off the top of their head) cause there's a lot of similar sounding limits in Mm that aren't actually the same thing

[2024-09-09 17:16] luci4: [replying to Matti: "sorry, in the context of a specific process it's p..."]
It's 50 pages

[2024-09-09 17:16] Matti: I'm pretty sure `NtQueryInformationProcess(ProcessVmCounters)` (passing `VM_COUNTERS_EX`) will give you the result

[2024-09-09 17:16] Matti: I just don't know which field specifically heh

[2024-09-09 17:17] luci4: [replying to Matti: "I'm pretty sure `NtQueryInformationProcess(Process..."]
My plan now is to call SetProcessWorkingSize with 75% of CommitLimitBytes

[2024-09-09 17:17] luci4: Leave 25% for the kernel

[2024-09-09 17:17] Matti: [replying to luci4: "It's 50 pages"]
this sounds very low

[2024-09-09 17:17] luci4: Lol

[2024-09-09 17:17] luci4: [replying to Matti: "this sounds very low"]
Its the working set

[2024-09-09 17:17] Matti: ah yeah, I remember now you can actually empty it too

[2024-09-09 17:17] Matti: so clearly not the one

[2024-09-09 17:18] Matti: but there are a lot of limits in that struct - check em out

[2024-09-09 17:18] luci4: [replying to Matti: "this sounds very low"]
https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-setprocessworkingsetsize
[Embed: SetProcessWorkingSetSize - Win32 apps]
Sets the minimum and maximum working set sizes for the specified process. (SetProcessWorkingSetSize)

[2024-09-09 17:18] luci4: oh that was the minimum

[2024-09-09 17:19] luci4: the max is 345

[2024-09-09 17:19] luci4: oopsie!

[2024-09-09 17:19] Matti: it's also possible these only refer to current and maximum counts, not limits - in which case forget I said anything

[2024-09-09 17:19] Matti: in that case you'll probably need `ProcessQuotaLimits`

[2024-09-09 17:20] Matti: another one that takes two different input types... pass `QUOTA_LIMITS_EX`

[2024-09-09 17:20] luci4: [replying to Matti: "it's also possible these only refer to current and..."]
This is what I'm gonna do:

[2024-09-09 17:20] luci4: ```
    SYSTEM_MEMORY_USAGE_INFORMATION MemoryInformation{};
    DWORD                           Length{};

    if (!NT_SUCCESS(NtQuerySystemInformation(SystemFullMemoryInformation, &MemoryInformation, sizeof(SYSTEM_MEMORY_USAGE_INFORMATION), &Length)))
    {
        return NULL;
    }


    SetProcessWorkingSetSize(NtCurrentProcess(), (MemoryInformation.CommitLimitBytes * 75) / 100, (MemoryInformation.CommitLimitBytes * 75) / 100);
```

[2024-09-09 17:20] luci4: is 75% too much? lol

[2024-09-09 17:21] luci4: I need to leave some for the kernel

[2024-09-09 17:21] Matti: well it depends

[2024-09-09 17:22] Matti: it's obviously system wide, so if you actually use 75% you're hogging everything for yourself - whether that's OK is up to you

[2024-09-09 17:22] Matti: but it can be fine IMO

[2024-09-09 17:22] Matti: lots of compilers seem to not mind using 100% (or trying for 101% and causing OOM) on my system

[2024-09-09 17:22] luci4: [replying to Matti: "it's obviously system wide, so if you actually use..."]
Yeah that's kinda the point

[2024-09-09 17:22] luci4: Since well

[2024-09-09 17:23] luci4: if u wanna run something in the bg

[2024-09-09 17:23] luci4: you can just use Filezilla or whatever

[2024-09-09 17:23] luci4: Winsock is gonna bottleneck me, but what can you do?

[2024-09-09 17:23] luci4: I'll cross that bridge when I get there

[2024-09-09 17:23] Matti: also, again double check the meaning of process working set in the NT internals book cause I'm too lazy to rn

[2024-09-09 17:24] luci4: [replying to Matti: "also, again double check the meaning of process wo..."]
Sir yes sir

[2024-09-09 17:24] Matti: additionally - what if the system is already currently using 99% of the limit because someone started kazaa?

[2024-09-09 17:24] Matti: you need this info too for your calculation

[2024-09-09 17:26] luci4: [replying to Matti: "additionally - what if the system is already curre..."]
Well I am querying the commit limit beforehand. If someone started kazaa and the system is at 99%, it will set the commit limit of my process to 75% of that 1% LOL

[2024-09-09 17:26] Matti: I don't think that's true

[2024-09-09 17:26] luci4: I'll probably have a check though

[2024-09-09 17:26] luci4: to see if that calculation is lower than 345 pages

[2024-09-09 17:26] Matti: I can see the current fluctuate, but the max is constant
[Attachments: image.png]

[2024-09-09 17:26] luci4: if so I won't change it

[2024-09-09 17:26] Matti: or the limit, to use the exact wording

[2024-09-09 17:27] Matti: this is commit charge I'm referring to

[2024-09-09 17:27] Matti: on this system I've got 128 GB RAM + about 4 GB or so of pagefile for crash dumps

[2024-09-09 17:27] luci4: Ahhhhh

[2024-09-09 17:28] luci4: I wonder if the AvailableBytes field has what I'm looking for, in that case

[2024-09-09 17:28] Matti: well, that is physical memory

[2024-09-09 17:28] Matti: you can't (by default) allocate physical memory from user mode

[2024-09-09 17:29] Matti: also, there may be policy limits on your process because of reasons

[2024-09-09 17:29] Matti: like having a nazi sysadmin and being part of an AD domain

[2024-09-09 17:29] Matti: or being in a job or process that was specifically singled out to be limited

[2024-09-09 17:30] Matti: [replying to Matti: "in that case you'll probably need `ProcessQuotaLim..."]
since there's also a Set version of this one

[2024-09-09 17:31] luci4: Hmmmm...

[2024-09-09 17:32] luci4: ` You can set hard working set limits using the 
SetProcessWorkingSetSizeEx function along with the QUOTA_LIMITS_HARDWS_MAX_ENABLE flag, but 
it is almost always better to let the system manage your working set`

[2024-09-09 17:32] luci4: ` On x64 systems, 
physical memory would be the practical upper limit,`

[2024-09-09 17:33] luci4: [replying to Matti: "well, that is physical memory"]
It seems like I could just do 70-75% of `AvailableBytes` then, if that represents physical memory

[2024-09-09 17:34] Matti: mm yeah, I suppose that would be a reasonable first guess amount if you're aiming for that percentage

[2024-09-09 17:35] luci4: Perfect!

[2024-09-09 17:35] luci4: Then SetProcessWorkingSetSizeEx is what i'll do

[2024-09-09 17:35] Matti: but I do think there are reasons (like the ones mentioneed above) that could cause this allocation to fail - so obviously check its return status, but that goes without saying I guess

[2024-09-09 17:36] Matti: I guess what I mean is don't expect it to be impossible for it to fail

[2024-09-09 17:36] luci4: [replying to Matti: "but I do think there are reasons (like the ones me..."]
If it fails I'll try 50%

[2024-09-09 17:36] luci4: then 30%

[2024-09-09 17:36] luci4: up until its lower than 345 pages

[2024-09-09 17:36] luci4: in which case im just hurting myself more

[2024-09-09 17:36] Matti: the other thing is - you don't necessarily *want* to use all 75% right

[2024-09-09 17:36] Matti: you just wanna know the limits to start with

[2024-09-09 17:37] Matti: in which case, this allocation is just for testing purposes, and it may take a while

[2024-09-09 17:37] Matti: just for memory you're gonna instantly free again

[2024-09-09 17:37] Matti: though probably nothing that's unbearable

[2024-09-09 17:37] luci4: [replying to Matti: "just for memory you're gonna instantly free again"]
Yeah exactly. Say for a 10TB file, I would like to read it in chunks as big as possible

[2024-09-09 17:37] luci4: while still leaving that 16MB "hot area" I talked about before

[2024-09-09 17:38] Matti: ya

[2024-09-09 17:38] luci4: so other small files could still get processed

[2024-09-09 17:38] luci4: in the meantime

[2024-09-09 17:40] luci4: If in that time it finds another 10TB file, it will probably make it wait on a global flag `IsFatFileInProcess` or smth

[2024-09-09 17:41] Matti: [replying to luci4: "while still leaving that 16MB "hot area" I talked ..."]
remember what I said before (and you already know...) about I/O being the dominant factor in this program anyway
but... for a 16 MB "hot area" I wouldn't be able to resist at least trying for SeLockMemoryPrivilege just in case you can get it

[2024-09-09 17:41] Matti: and allocating it as physical

[2024-09-09 17:41] luci4: [replying to Matti: "remember what I said before (and you already know...."]
Oh I'm definitely NtLockingThatShitInPhysicalMemory

[2024-09-09 17:41] Matti: this is definitely far into what some people would call "premature optimization" though

[2024-09-09 17:42] Matti: I'm just saying... I wouldn't be able to resist...

[2024-09-09 17:42] luci4: No I'm def doing it

[2024-09-09 17:42] Matti: 😎

[2024-09-09 17:48] luci4: I hope the local admin has that privilege

[2024-09-09 17:48] Matti: nope

[2024-09-09 17:48] Matti: sorry

[2024-09-09 17:48] luci4: eh ill just play with the token lol

[2024-09-09 17:48] Matti: to get this, you need to explicitly assign it to a user (group) using `secpol.msc`

[2024-09-09 17:49] luci4: I also need `SE_INC_WORKING_SET_NAME` unfortunately

[2024-09-09 17:49] Matti: 
[Attachments: image.png]

[2024-09-09 17:49] luci4: what if it's not domain joined tho

[2024-09-09 17:49] Matti: [replying to luci4: "I also need `SE_INC_WORKING_SET_NAME` unfortunatel..."]
that one should be `Users` by default

[2024-09-09 17:49] Matti: i.e. pretty much anyone

[2024-09-09 17:49] luci4: [replying to Matti: "that one should be `Users` by default"]
jackpot

[2024-09-09 17:49] Matti: [replying to luci4: "what if it's not domain joined tho"]
well I'm not

[2024-09-09 17:49] Matti: so this affects my PC only

[2024-09-09 17:50] luci4: I'm just gonna elevate to SYSTEM

[2024-09-09 17:50] Matti: well the default list is empty

[2024-09-09 17:50] Matti: SYSTEM isn't in it either

[2024-09-09 17:50] luci4: 😱

[2024-09-09 17:51] Matti: though running in kernel mode would do it... then again allocating physical memory from kernel mode is not exactly magic anyway lol

[2024-09-09 17:52] luci4: [replying to Matti: "though running in kernel mode would do it... then ..."]
I'm gonna keep it real with you, this app will probably not be stable enough to run as a driver

[2024-09-09 17:52] Matti: https://learn.microsoft.com/en-us/windows/win32/memory/address-windowing-extensions this is the (win32) documentation - it's surprisingly extensive
[Embed: Address Windowing Extensions - Win32 apps]
Address Windowing Extensions (AWE) is a set of extensions that allows an application to quickly manipulate physical memory greater than 4GB.

[2024-09-09 17:53] Matti: there's no driver equivalent documentation since drivers have no need to call NtUserAllocatePhysicalPages and co

[2024-09-09 17:53] luci4: Well a good driver at least lol

[2024-09-09 17:53] Matti: nah definitely don't write a driver

[2024-09-09 17:53] Matti: just try for SeLockMemoryPrivilege, and if you can get it - bonus

[2024-09-09 17:53] Matti: otherwise continue as usual

[2024-09-09 17:53] luci4: def!

[2024-09-09 18:00] luci4: [replying to Matti: "nah definitely don't write a driver"]
Boom

[2024-09-09 18:00] luci4: ```C
    SYSTEM_MEMORY_USAGE_INFORMATION MemoryInformation{};
    DWORD                           Length{};

    if (!NT_SUCCESS(NtQuerySystemInformation(SystemFullMemoryInformation, &MemoryInformation, sizeof(SYSTEM_MEMORY_USAGE_INFORMATION), &Length)))
    {
        return NULL;
    }

    ULONGLONG NewCommitLimit = (MemoryInformation.AvailableBytes * 70) / 100;

    do 
    {
        NewCommitLimit -= (MemoryInformation.AvailableBytes * 10) / 100;

         if (NewCommitLimit < DEFAULT_MAX_WORKING_SET)
        {
            return 0;
        }
    } 
    while (SetProcessWorkingSetSizeEx(NtCurrentProcess(), NewCommitLimit, NewCommitLimit, QUOTA_LIMITS_HARDWS_MIN_ENABLE) == NULL);

    return NewCommitLimit;
```

[2024-09-09 18:01] luci4: Since Min = Max it will always stay in physical mem so its perfect

[2024-09-09 18:13] diversenok: Do you actually plan to measure the performance benefits of just do it because it seems reasonable?

[2024-09-09 18:13] diversenok: It's usually better to leave managing working set, priorities, etc. to the OS

[2024-09-09 18:14] diversenok: Unless you can see an observable difference

[2024-09-09 18:14] diversenok: And also know that it wouldn't make the overall result worse

[2024-09-09 18:16] Azrael: [replying to luci4: "```C
    SYSTEM_MEMORY_USAGE_INFORMATION MemoryInf..."]
Depending on what you're trying to achieve, Process Lasso might be a better alternative.

[2024-09-09 18:24] luci4: [replying to diversenok: "Do you actually plan to measure the performance be..."]
I'll try both approaches

[2024-09-09 18:24] luci4: The one I'm doing now

[2024-09-09 18:24] luci4: and

[2024-09-09 18:25] luci4: ugh I can't reply to myself for some reason

[2024-09-09 18:26] diversenok: Weird

[2024-09-09 18:27] diversenok: [replying to diversenok: "Weird"]
Very weird

[2024-09-09 18:27] diversenok: ¯\_(ツ)_/¯

[2024-09-09 18:27] luci4: [replying to diversenok: "¯\_(ツ)_/¯"]
Now you're just showing off 😄

[2024-09-09 18:28] diversenok: It's called testing 🤫

[2024-09-09 18:33] luci4: Well my day is ruined, that code block always returns 0

[2024-09-09 18:35] luci4: What's funny is that

[2024-09-09 18:36] luci4: `NtQuerySystemInformation` is the one that fails

[2024-09-09 18:36] luci4: LOL

[2024-09-09 18:36] luci4: Maybe `SystemFullMemoryInformation` is a scam

[2024-09-09 18:42] x86matthew: [replying to Matti: "additionally - what if the system is already curre..."]
well, somebody should have told them to download kazaa lite instead

[2024-09-09 18:43] luci4: Now what in the 9 circles of hell is `STATUS_NOT_IMPLEMENTED`

[2024-09-09 18:48] Matti: it's the status returned by `MmMemoryUsage`, among others

[2024-09-09 18:48] Matti: are you calling NtQSI with SystemFullMemoryInformation / SystemSummaryMemoryInformation ?

[2024-09-09 18:49] Matti: [replying to luci4: "Maybe `SystemFullMemoryInformation` is a scam"]
oh

[2024-09-09 18:49] Matti: yes you are

[2024-09-09 18:49] Matti: welllll

[2024-09-09 18:49] Matti: [replying to Matti: "it's the status returned by `MmMemoryUsage`, among..."]
that API may just end up calling this API

[2024-09-09 18:50] Matti: not sure if this has changed recently

[2024-09-09 18:50] Matti: but it did do so in the past

[2024-09-09 18:51] luci4: [replying to Matti: "that API may just end up calling this API"]
Well here is another interesting thing

[2024-09-09 18:51] luci4: I looked at `GlobalMemoryStatusEx` which does something similar

[2024-09-09 18:51] luci4: and this is the call it does

[2024-09-09 18:51] luci4: 
[Attachments: image.png]

[2024-09-09 18:52] luci4: This is how it gets TotalPhys

[2024-09-09 18:52] luci4: which is what I'm looking for

[2024-09-09 18:52] luci4: [replying to luci4: "` On x64 systems, 
physical memory would be the pr..."]
According to Windows internals:

[2024-09-09 18:53] Matti: [replying to luci4: ""]
what's `rcx` here

[2024-09-09 18:53] Matti: your types are a bit out of date

[2024-09-09 18:54] luci4: [replying to Matti: "your types are a bit out of date"]
Ah I have the default win7 ones

[2024-09-09 18:54] Matti: well updating them is definitely the better long term fix <:thinknow:475800595110821888>

[2024-09-09 18:54] luci4: [replying to Matti: "what's `rcx` here"]

[Attachments: image.png]

[2024-09-09 18:56] Matti: `SystemMemoryUsageInformation`

[2024-09-09 18:56] Matti: == 182

[2024-09-09 18:56] Matti: and taking `SYSTEM_MEMORY_USAGE_INFORMATION`

[2024-09-09 18:58] luci4: [replying to Matti: "and taking `SYSTEM_MEMORY_USAGE_INFORMATION`"]
Ah well this is the same struct I have been passing to it all along

[2024-09-09 18:59] Matti: uhuh yeah

[2024-09-09 18:59] Matti: but is it the same info class?

[2024-09-09 18:59] luci4: [replying to Matti: "but is it the same info class?"]
Nope, that's all that differs

[2024-09-09 18:59] luci4: and it works now lol

[2024-09-09 18:59] Matti: 😉

[2024-09-09 18:59] Matti: thought it might

[2024-09-09 19:00] Matti: [replying to Matti: "are you calling NtQSI with SystemFullMemoryInforma..."]
no idea why these exist

[2024-09-09 19:00] luci4: [replying to Matti: "no idea why these exist"]
To scam me 😦

[2024-09-09 19:00] Matti: 
[Attachments: image.png]

[2024-09-09 19:00] Matti: to prove it's not a joke

[2024-09-09 19:00] Matti: that is MmMemoryUsage

[2024-09-09 19:01] Matti: <:why:753383369546530957>

[2024-09-09 19:01] Matti: ahh I see hang on

[2024-09-09 19:01] Matti: it has an actual implementation if built for a checked kernel

[2024-09-09 19:02] luci4: Welp the call failed

[2024-09-09 19:02] luci4: `ERROR_INVALID_PARAMETER`

[2024-09-09 19:02] Matti: pretty fucking big one too

[2024-09-09 19:02] luci4: Fortunately its the call to `SetProcessWorkingSetSizeEx` so progress!

[2024-09-09 19:02] Matti: I'd forget about that API

[2024-09-09 19:02] Matti: unless you're trying to *empty* the working set

[2024-09-09 19:03] Matti: which I'm pretty sure is its main purpose, or at least the way it's used

[2024-09-09 19:03] luci4: Just raising it

[2024-09-09 19:03] Matti: [replying to diversenok: "It's usually better to leave managing working set,..."]
yeah but, consider this too

[2024-09-09 19:03] Matti: guy has a point

[2024-09-09 19:04] luci4: Yeah but the OS would never let me have 5gb allocations

[2024-09-09 19:04] Matti: did you know that *another* process can empty your WS using this API too? it's great

[2024-09-09 19:04] luci4: This is what it seems to return
[Attachments: image.png]

[2024-09-09 19:04] luci4: 70% of that is like 5gb

[2024-09-09 19:04] luci4: surely thats reasonable

[2024-09-09 19:05] Matti: that's positively modest

[2024-09-09 19:05] Matti: you should see what LLVM eats on my system if I want to compile helloworld.c

[2024-09-09 19:06] Matti: ah, but your total physical memory is ~nothing

[2024-09-09 19:06] Matti: so relatively it's fairly big yeah

[2024-09-09 19:06] Matti: still fine I'd say

[2024-09-09 19:06] luci4: [replying to Matti: "ah, but your total physical memory is ~nothing"]
well yeah i have 16gb of ram

[2024-09-09 19:06] luci4: its rough in romania ok? lol

[2024-09-09 19:07] Matti: ||I had 32 GB of RAM in 2013||

[2024-09-09 19:07] luci4: [replying to Matti: "||I had 32 GB of RAM in 2013||"]
yeah but we have mud huts here and stuff

[2024-09-09 19:07] Matti: it sucks I'm all out of DIMMs to give out

[2024-09-09 19:07] Matti: I'd send you some otherwise

[2024-09-09 19:07] Matti: not kidding

[2024-09-09 19:08] Matti: but after doing this for like, 5 people, I actually managed to run out of DIMMs in this house

[2024-09-09 19:08] Matti: other than the ones I actually use

[2024-09-09 19:08] luci4: damn really? id be in your debt

[2024-09-09 19:08] luci4: [replying to Matti: "but after doing this for like, 5 people, I actuall..."]
that's very nice of you

[2024-09-09 19:08] Matti: I have a lot of hardware I don't technically speaking *need*

[2024-09-09 19:08] Matti: I just like having it

[2024-09-09 19:09] Matti: it's not hoarding ok

[2024-09-09 19:09] Matti: it's collecting

[2024-09-09 19:09] luci4: [replying to Matti: "it's not hoarding ok"]
id only consider it hoarding if its useless tbh

[2024-09-09 19:09] luci4: like a figure

[2024-09-09 19:09] luci4: (dont throw rocks)

[2024-09-09 19:09] Matti: yeah, no, not useless...

[2024-09-09 19:09] Matti: was using those DIMMs all the time...

[2024-09-09 19:10] Matti: which is why I sent them away...

[2024-09-09 19:10] Matti: hmmm

[2024-09-09 19:10] Matti: well they were useless to *me* for sure

[2024-09-09 19:10] Matti: [replying to luci4: "like a figure"]
yeah no I only really collect hardware and weights

[2024-09-09 19:10] luci4: [replying to Matti: "well they were useless to *me* for sure"]
Well yeah, but intrinsically, they have use

[2024-09-09 19:14] luci4: Still `ERROR_INVALID_PARAMETER`, wtf

[2024-09-09 19:15] luci4: Oh....

[2024-09-09 19:15] luci4: `and less than the system-wide maximum (number of available pages minus 512 pages)`

[2024-09-09 19:16] luci4: [replying to luci4: "70% of that is like 5gb"]
That should be in-range though

[2024-09-09 19:17] Matti: sorry, which API are you calling atm? and with what args

[2024-09-09 19:17] Matti: kind of lost track a bit

[2024-09-09 19:17] luci4: ```C
    SYSTEM_MEMORY_USAGE_INFORMATION MemoryInformation{};
    DWORD                           Length{};

    if (!NT_SUCCESS(NtQuerySystemInformation(SystemFullMemoryInformation, &MemoryInformation, sizeof(SYSTEM_MEMORY_USAGE_INFORMATION), &Length)))
    {
        return NULL;
    }

    ULONGLONG NewCommitLimit = (MemoryInformation.AvailableBytes * 70) / 100;

    do 
    {
        NewCommitLimit -= (MemoryInformation.AvailableBytes * 10) / 100;

         if (NewCommitLimit < DEFAULT_MAX_WORKING_SET)
        {
            return 0;
        }
    } 
    while (SetProcessWorkingSetSizeEx(NtCurrentProcess(), NewCommitLimit, NewCommitLimit, QUOTA_LIMITS_HARDWS_MIN_ENABLE) == NULL);

    return NewCommitLimit;
```

[2024-09-09 19:17] luci4: `SetProcessWorkingSetSizeEx`

[2024-09-09 19:18] luci4: `NewCommitLimit` in this case is like 5.5 out of the 7.6GB available

[2024-09-09 19:24] Matti: man that API can return STATUS_INVALID_PARAMETER for a lot of reasons

[2024-09-09 19:25] Matti: to the point where I'd just google some code and paste that

[2024-09-09 19:26] Matti: in general you also need to have SE_INC_BASE_PRIORITY_PRIVILEGE and SE_INC_WORKING_SET_PRIVILEGE enabled - but it seems you do cause otherwise the status would be related to that

[2024-09-09 19:27] Matti: I'd definitely call the NT API instead of this to simplify things, cause win32 also does some 'helpful' stuff

[2024-09-09 19:27] Matti: so

[2024-09-09 19:27] Matti: `NtSetInformationProcess(ProcessQuotaLimits)`

[2024-09-09 19:27] luci4: [replying to Matti: "I'd definitely call the NT API instead of this to ..."]
I was JUST thinking of NtSetInformationProcess

[2024-09-09 19:27] Matti: with `QUOTA_LIMITS_EX`

[2024-09-09 19:27] luci4: When u wrote that

[2024-09-09 19:27] luci4: [replying to Matti: "`NtSetInformationProcess(ProcessQuotaLimits)`"]
Perfect!

[2024-09-09 19:27] luci4: Welp its 22:00 and I've been at this since morning

[2024-09-09 19:28] luci4: So I'm gonna go

[2024-09-09 19:28] luci4: But thanks

[2024-09-09 19:28] luci4: A LOT!

[2024-09-09 19:28] Matti: just a few snippets...
```c
//
// All unused flags must be zero
//
if (RequestedLimits.Flags & ~(QUOTA_LIMITS_HARDWS_MAX_ENABLE|QUOTA_LIMITS_HARDWS_MAX_DISABLE|
                             QUOTA_LIMITS_HARDWS_MIN_ENABLE|QUOTA_LIMITS_HARDWS_MIN_DISABLE)) {
    return STATUS_INVALID_PARAMETER;
}

//
// Disallow both enable and disable bits set at the same time.
//
if (PS_TEST_ALL_BITS_SET (RequestedLimits.Flags, QUOTA_LIMITS_HARDWS_MIN_ENABLE|QUOTA_LIMITS_HARDWS_MIN_DISABLE) ||
    PS_TEST_ALL_BITS_SET (RequestedLimits.Flags, QUOTA_LIMITS_HARDWS_MAX_ENABLE|QUOTA_LIMITS_HARDWS_MAX_DISABLE)) {
    return STATUS_INVALID_PARAMETER;
}

// ... 

//
// All reserved fields must be zero
//
if (RequestedLimits.Reserved1 != 0 || RequestedLimits.Reserved2 != 0 ||
    RequestedLimits.Reserved3 != 0 || RequestedLimits.Reserved4 != 0 ||
    RequestedLimits.Reserved5 != 0) {
    return STATUS_INVALID_PARAMETER;
}
```

[2024-09-09 19:28] Matti: those are the most obvious ones I think

[2024-09-09 19:30] Matti: confused about the first check to be honest

[2024-09-09 19:30] Matti: how are they unused

[2024-09-09 19:31] luci4: [replying to Matti: "just a few snippets...
```c
//
// All unused flags..."]
Thats's so weird lol

[2024-09-09 19:31] luci4: Why even put it as an option then

[2024-09-09 19:32] Matti: oh I'm a bit slow

[2024-09-09 19:33] Matti: the comment is trying to say 'these are the only allowed flags'

[2024-09-09 19:33] Matti: which is true

[2024-09-09 19:33] Matti: there are no other flags

[2024-09-09 19:33] luci4: Im on mobile so it looks like wank

[2024-09-09 19:33] Matti: it's just a very odd way of describing what the code is actually doing

[2024-09-09 19:34] Matti: one of those times where no comment would've been better than this comment

[2024-09-09 19:34] Matti: there are no other [unused] flags

[2024-09-09 19:35] Matti: these are all of the possible flags, i.e. any set containing other values contains an invalid flag

[2024-09-09 19:38] Matti: [replying to Matti: "in general you also need to have SE_INC_BASE_PRIOR..."]
also the API is structured in such a way that, unusually, the invalid parameter checks will cause that to be returned before even getting to the privilege checks

[2024-09-09 19:39] Matti: well, it's debatable if that's unusual... but I think in general slightly more APIs do it the other way around

[2024-09-09 19:40] Matti: anyway just keep that in mind in case you think you're off the hook for the privilege check

[2024-09-09 19:42] luci4: [replying to Matti: "anyway just keep that in mind in case you think yo..."]
Sure! I'll make the switch to NtSetInformationProcess in either case, though.

[2024-09-09 19:42] luci4: NT api feels much more natural, after a bit of getting used to

[2024-09-09 19:43] Matti: it does, doesn't it

[2024-09-09 19:43] Matti: and the singular reason (||well there are others but||) is NTSTATUS

[2024-09-09 19:44] Matti: seeing win32 code including GetLastError() makes me feel ill

[2024-09-09 19:44] Matti: as does win32 code which doesn't check for errors, which is equally common

[2024-09-09 19:44] Matti: the API design encourages it

[2024-09-09 19:44] luci4: [replying to Matti: "seeing win32 code including GetLastError() makes m..."]
I hate win32

[2024-09-09 19:44] luci4: And I also kinda got used to using RtlGetLastNt or whatever

[2024-09-09 19:45] luci4: Basically NtCurrentPeb()->LastWin32Error/LastErrorStatus

[2024-09-09 19:45] Matti: yeah but the great part is, you don't need to do that!

[2024-09-09 19:45] Matti: cause the syscalls all already return NTSTATUS

[2024-09-09 19:45] Matti: (win32k """syscalls""" excepted)

[2024-09-09 19:45] luci4: Yeah that's very useful too

[2024-09-09 19:46] luci4: I get why they wanted kernel32

[2024-09-09 19:46] luci4: The ease of use and backwards compatibility

[2024-09-09 19:46] luci4: But

[2024-09-09 19:46] luci4: Has NtAllocateVirtualMemory really changed all that much?

[2024-09-09 19:46] Matti: no

[2024-09-09 19:46] Matti: never

[2024-09-09 19:46] Matti: but, there is actually an NtAllocateVirtualMemoryEx since not very long ago

[2024-09-09 19:46] luci4: Yeah most of em have been the same for decades lol

[2024-09-09 19:47] Matti: it allows some more fine grained control over the allocation

[2024-09-09 19:47] Matti: nothing spectacular

[2024-09-09 19:47] luci4: [replying to Matti: "never"]
So there is no incentive for me to use VirtualAlloc

[2024-09-09 19:47] Matti: nope

[2024-09-09 19:47] luci4: Same goes for CreateProcess

[2024-09-09 19:48] luci4: Why the hell does csrss need to know about my process?

[2024-09-09 19:48] Matti: I only know of a single syscall that ever changed substantially in the NT version

[2024-09-09 19:48] Matti: but that syscall has no win32 equivalent anyway

[2024-09-09 19:48] Matti: presumably the reason why they felt it was OK to change in the first place

[2024-09-09 19:49] luci4: [replying to luci4: "Why the hell does csrss need to know about my proc..."]
What csrss does exactly is still a mystery to me

[2024-09-09 19:49] luci4: But ill get to it eventually

[2024-09-09 19:49] Matti: ha

[2024-09-09 19:49] luci4: [replying to Matti: "I only know of a single syscall that ever changed ..."]
Oh interesting

[2024-09-09 19:49] Matti: csrss is fun to poke around in

[2024-09-09 19:50] Matti: it's a bit annoying nowadays because of all of the mitigation shit, and PPLs

[2024-09-09 19:50] Matti: when I was reversing csrss I did so on windows 7

[2024-09-09 19:50] luci4: [replying to Matti: "it's a bit annoying nowadays because of all of the..."]
I hate PPL

[2024-09-09 19:50] luci4: And VBS

[2024-09-09 19:50] luci4: VBS killed lsass 😦

[2024-09-09 19:51] Matti: I almost feel like you're baiting me into self promoting here

[2024-09-09 19:52] Matti: <https://github.com/Mattiwatti/PPLKiller> to disable PPLs
<https://github.com/Mattiwatti/EfiGuard> to disable DSE [so you can run PPLKiller] and patchguard [so PPLKiller won't end up bugchecking your system]

[2024-09-09 19:52] Matti: but, running a bootkit isn't feasible on most systems I'm well aware

[2024-09-09 19:53] Matti: just saying, if it's an option consider it

[2024-09-09 19:54] Matti: [replying to luci4: "And VBS"]
oh yeah efiguard disables this too

[2024-09-09 19:54] luci4: [replying to Matti: "I almost feel like you're baiting me into self pro..."]
Haha not really. At work they don't let us do BYOVD, so we have to avoid lsass.

Didnt HVCI kinda kill EfiGiard? I'm currently learning UEFI to avenge EfiGuard

[2024-09-09 19:54] luci4: I found a variable that disables VBS altogether

[2024-09-09 19:54] luci4: Well I didnt do it

[2024-09-09 19:55] luci4: But I learned of it

[2024-09-09 19:55] luci4: [replying to Matti: "oh yeah efiguard disables this too"]
Oh really?

[2024-09-09 19:55] Matti: [replying to luci4: "Haha not really. At work they don't let us do BYOV..."]
yes and no

[2024-09-09 19:55] Matti: HVCI > efiguard

[2024-09-09 19:55] luci4: [replying to Matti: "HVCI > efiguard"]
Ill avenge EfiGuard or die trying

[2024-09-09 19:55] Matti: but, efiguard can disable HVCI during boot time before it is enabled

[2024-09-09 19:55] luci4: Using that same variable?

[2024-09-09 19:55] Matti: so, in that sense efiguard 'wins'

[2024-09-09 19:55] Matti: well

[2024-09-09 19:56] Matti: there are actually two undocumented variables that do what you describe 😄

[2024-09-09 19:56] Matti: one is an EFI variable, the other a registry one

[2024-09-09 19:56] Matti: efiguard uses the former

[2024-09-09 19:56] Matti: it persists until the next reboot

[2024-09-09 19:56] luci4: I was referring to the EFI variable haha

[2024-09-09 19:57] luci4: My idea was to make a bootkit that disables ALL protections and loads "reflectively" a driver

[2024-09-09 19:57] luci4: So you basically completely own that system

[2024-09-09 19:57] Matti: mm yeah

[2024-09-09 19:58] luci4: Buut I have a LOT to learn atm

[2024-09-09 19:58] Matti: I tried to make efiguard as a sort of minimum viable set to accomplish this yourself

[2024-09-09 19:59] luci4: [replying to luci4: "I was referring to the EFI variable haha"]
<@234331837651091456>

[2024-09-09 19:59] luci4: The one u told me about

[2024-09-09 19:59] elias: huh

[2024-09-09 19:59] luci4: [replying to elias: "huh"]
The efi variable that disables vbs

[2024-09-09 19:59] Matti: it allows you to load arbitrary drivers*, and it disables the code that would otherwise bugcheck you for doing so

*some people think this is the main goal of efiguard - it's actually more like a convenient side effect

[2024-09-09 19:59] elias: VbsPolicyDisabled

[2024-09-09 19:59] luci4: Yeah!

[2024-09-09 20:00] luci4: [replying to Matti: "it allows you to load arbitrary drivers*, and it d..."]
Oh so it does a lot of what I want to 😄

[2024-09-09 20:01] luci4: I want mine to store the driver in .rsrc or something and fetch it from there

[2024-09-09 20:01] luci4: Which would make you one secure boot bypass away from complete ownage

[2024-09-09 20:02] Matti: this is generally the point where I quietly exit the conversation without asking why you would want to do that

[2024-09-09 20:02] luci4: [replying to Matti: "this is generally the point where I quietly exit t..."]
Oh dont get me wrong lol

[2024-09-09 20:02] luci4: Fun

[2024-09-09 20:02] Matti: [replying to luci4: "Which would make you one secure boot bypass away f..."]
why do you need a bypass? just sign efiguard with your own SB key

[2024-09-09 20:02] Matti: this is what I do anyway

[2024-09-09 20:02] Matti: this only works on my own system obviously

[2024-09-09 20:02] dtb: [replying to luci4: "So you basically completely own that system"]
could take it to the extreme and load a vmm :p

[2024-09-09 20:03] Matti: yeah - to clarify, the reason HVCI > efiguard is because HVCI runs at a greater privilege level

[2024-09-09 20:03] Matti: it's as simple as that

[2024-09-09 20:03] luci4: [replying to Matti: "this only works on my own system obviously"]
Thats good enough then

[2024-09-09 20:04] luci4: [replying to Matti: "yeah - to clarify, the reason HVCI > efiguard is b..."]
In what sense exactly?

[2024-09-09 20:05] Matti: well, with HVCI you can have nested page tables that have different protections for the VMM and the guest

[2024-09-09 20:06] Matti: obviously you'd make something like `g_CiOptions` (the variable used to control DSE) writable only by the VMM, or after requesting and getting permission from the VMM to modify the real PTE

[2024-09-09 20:07] elias: if you really want to, you can tamper with hypervisor or securekernel from efi phase

[2024-09-09 20:07] Matti: also DSE in general works a bit differently with HVCI enabled, using skci.dll

[2024-09-09 20:07] Matti: but that's not as critically important from what I've seen

[2024-09-09 20:08] Matti: [replying to elias: "if you really want to, you can tamper with hypervi..."]
yeah, this is true

[2024-09-09 20:08] luci4: [replying to elias: "if you really want to, you can tamper with hypervi..."]
Let me finish reading the uefi spec first, to get an idea of what the hell I am doing

[2024-09-09 20:08] luci4: Because I'm fairly clueless

[2024-09-09 20:08] elias: I did patch some function in skci to allow loading self signed drivers with hvci on

[2024-09-09 20:08] luci4: Besides what you taught me noct

[2024-09-09 20:09] Matti: I've considered doing this but in the end dismissed it as too complex and too likely to break in the future due to relatively minor changes in the way VBS/HVCI are initialized

[2024-09-09 20:09] Matti: remember efiguard was designed to, and still is, to run on fossils all the way down to vista SP2

[2024-09-09 20:10] elias: convenient trick is that you can hook BlStatusPrint (even exported by winload) and you will get secure kernel address right after it was loaded

[2024-09-09 20:10] Matti: this makes it much harder to make patches like this universal

[2024-09-09 20:10] elias: [replying to Matti: "remember efiguard was designed to, and still is, t..."]
oh right damn

[2024-09-09 20:10] elias: managing that compatibility doesnt sound like a lof of fun

[2024-09-09 20:11] luci4: [replying to Matti: "this makes it much harder to make patches like thi..."]
I guess you can only tamper with things that remain consistent over nt versions

[2024-09-09 20:11] Matti: it's the only way I can think of that ensures my patches are [as close as possible to] universal across NT versions

[2024-09-09 20:11] Matti: [replying to luci4: "I guess you can only tamper with things that remai..."]
yeah, pretty much

[2024-09-09 20:11] Matti: there are a few version checks in place where they are unavoidable

[2024-09-09 20:11] Matti: but in general this is the philosophy

[2024-09-09 20:12] luci4: [replying to luci4: "I want mine to store the driver in .rsrc or someth..."]
Reading up this sounds so shady LOL

[2024-09-09 20:12] elias: its off topic, but do you have an idea how to skip a file entry from a FS mini filter callback if the caller uses NtQueryDirectoryFile with SL_RETURN_SINGLE_ENTRY?

[2024-09-09 20:12] Matti: lmao

[2024-09-09 20:13] luci4: [replying to Matti: "there are a few version checks in place where they..."]
I see. I guess in that case you could check the nt version from the efi app?

[2024-09-09 20:13] Matti: not without reading up on a lot of docs and/or examples, sorry

[2024-09-09 20:13] Matti: summoning <@693052430837088309>

[2024-09-09 20:13] luci4: ...more or less

[2024-09-09 20:14] Matti: [replying to luci4: "I see. I guess in that case you could check the nt..."]
yes
<https://github.com/Mattiwatti/EfiGuard/blob/master/EfiGuardDxe/pe.c#L457>

[2024-09-09 20:14] luci4: [replying to Matti: "yes
<https://github.com/Mattiwatti/EfiGuard/blob/m..."]
Beautiful code

[2024-09-09 20:14] Matti: aw thank you

[2024-09-09 20:15] luci4: [replying to Matti: "aw thank you"]
Welp it was really fun learning from you (thank you) but now I really gotta sleep, lol

[2024-09-09 20:15] luci4: Cya! 🫡

[2024-09-09 20:15] Matti: cool, glad to hear it

[2024-09-09 20:15] Matti: nn

[2024-09-09 20:17] elias: When calling NtQueryDirectoryFile, how does the kernel remember the current search position? Is it affiliated with the file object? <:peepoDetective:570300270089732096>

[2024-09-09 20:21] Matti: I have no fucking idea

[2024-09-09 20:22] Matti: [replying to Matti: "compared to NtWhateverTheFuckIteratesDirectories"]
remember you're talking to this guy

[2024-09-09 20:22] Matti: I try to avoid this API if it can be helped

[2024-09-09 20:25] Matti: ok <https://learn.microsoft.com/en-us/windows-hardware/drivers/ifs/irp-mj-directory-control> has the answer

[2024-09-09 20:26] Matti: `IRP_MN_QUERY_DIRECTORY` is the one for NtWhateverTheFuck

[2024-09-09 20:26] Matti: then `IrpSp->Flags` will presumably contain `SL_INDEX_SPECIFIED`, and `IrpSp->Parameters.QueryDirectory.FileIndex` the actual index

[2024-09-09 20:27] Matti: I have to say I'm still not clear on where this index gets stored 'in between'

[2024-09-09 20:27] Matti: it may be the job of the caller?

[2024-09-09 20:28] Matti: there's about 50 arguments after all

[2024-09-09 20:28] Matti: okay scratch above

[2024-09-09 20:28] Matti: > IrpSp->Parameters.QueryDirectory.FileIndex is the index of the file at which to begin the directory scan. This value is ignored if the SL_INDEX_SPECIFIED flag isn't set. This parameter can't be specified in any Win32 function or kernel-mode support routine. Currently it's used only by the NT virtual DOS machine (NTVDM), which exists only on 32-bit NT-based platforms. The file index is undefined for file systems, such as NTFS, in which the position of a file within the parent directory isn't fixed and can be changed at any time to maintain sort order.

[2024-09-09 20:28] Matti: what the fuck

[2024-09-09 20:29] Matti: I swear this API was made by satan

[2024-09-09 20:32] elias: 😭

[2024-09-09 20:33] elias: gonna have to do some tests tomorrow

[2024-09-09 20:37] diversenok: [replying to elias: "When calling NtQueryDirectoryFile, how does the ke..."]
Yes, in the file object

[2024-09-09 20:38] diversenok: The `FirstScan` parametrs defines whether to advance it or to reset

[2024-09-09 20:53] elias: thx <:ThumbsUp:985957232065806387>

[2024-09-09 20:54] dinero: [replying to Matti: "> IrpSp->Parameters.QueryDirectory.FileIndex is th..."]
<:amd:855672207475736586>

[2024-09-09 20:54] dinero: why does this still exist

[2024-09-09 20:57] Matti: how else would you run `sheep.exe` if there was no VDM?
[Attachments: 2023-08-09_03-08-00.mp4]

[2024-09-09 21:07] diversenok: Oh, I remember this one! Now I feel so old

[2024-09-09 21:07] dinero: [replying to Matti: "how else would you run `sheep.exe` if there was no..."]
Screen Mate 在线上直播带

[2024-09-09 21:07] BWA RBX: <@148095953742725120> https://ytcracker.com/v2020/

[2024-09-09 21:08] Matti: [replying to dinero: "Screen Mate 在线上直播带"]
the mojibake message is telling you you need to purchase a copy for additional sheep

[2024-09-09 21:08] Matti: this is only the shareware sheep

[2024-09-09 21:08] dinero: sheep as a service

[2024-09-09 21:08] dinero: ahead of their time

[2024-09-09 21:09] diversenok: [replying to luci4: "What csrss does exactly is still a mystery to me"]
Registering with CSRSS is mostly for your own sake since it generates and sets the default activation for your process

[2024-09-09 21:09] dinero: [replying to Matti: "the mojibake message is telling you you need to pu..."]
oh so that’s the word for it

[2024-09-09 21:09] dinero: mojibake

[2024-09-09 21:10] dinero: that’s useful

[2024-09-09 21:10] diversenok: CSRSS might need if for something else, but I'm not sure what; the most noticeable effect is definitely the activation context

[2024-09-09 21:10] diversenok: Especially for GUI applications

[2024-09-09 21:11] Matti: well csrss has a lot of server DLLs doing various stuff

[2024-09-09 21:11] Matti: that is definitely one of the major things it does though

[2024-09-09 21:11] Matti: csrsrv handles that

[2024-09-09 21:11] diversenok: Without CSR/SxS registration, the process is missing the `ActivationContextData` and `SystemDefaultActivationContextData` fields in PEB

[2024-09-09 21:11] Matti: oh yeah and sxs I suppose

[2024-09-09 21:12] Matti: another thing it does is making NtRaiseHardError modal dialogs appear in the right process/thread

[2024-09-09 21:12] diversenok: GUI applications without an activation context load the older 5.x version of `comctl32.dll`

[2024-09-09 21:12] Matti: not sure which DLL that is

[2024-09-09 21:13] Matti: on older versions of windows it also used to do pretty much anything related to console 'UI' stuff

[2024-09-09 21:13] Matti: but I think that's mostly handled by condrv.sys now

[2024-09-09 21:14] diversenok: Registering with CSR parses the fusion manifest for the executable and if the runtime themes are enabled, configures DLL redirection for `comctl32.dll` so instead of loading the older version from system32 it loads the newer 6.x version from WinSxS

[2024-09-09 21:14] Matti: I mean that's definitely important

[2024-09-09 21:14] Matti: but IIRC if you don't register with csrss, you can't even so much as create a window

[2024-09-09 21:14] diversenok: The missing activation context is why most Native API implementations of process creation are broken

[2024-09-09 21:15] Matti: though I have a vague recollection of testing this again in win 10 and having it work

[2024-09-09 21:15] Matti: without registration that is

[2024-09-09 21:15] Matti: it doesn't on win 7

[2024-09-09 21:15] diversenok: Maybe it was the case on older version, but you definitely can on 10+

[2024-09-09 21:15] Matti: yeah ok, very weird

[2024-09-09 21:15] diversenok: The windows will use old comctl32 and no themes

[2024-09-09 21:15] Matti: cause it's kind of a big difference 😛

[2024-09-09 21:16] Matti: yep yep

[2024-09-09 21:16] diversenok: So many features are will not available/broken

[2024-09-09 21:16] diversenok: I'm actually plan to put all necessary definitions for registrering with CSRSS into phnt

[2024-09-09 21:17] Matti: [replying to diversenok: "The missing activation context is why most Native ..."]
the thing I ran into when creating native processes and doing this was that
csrsrv.dll checked the image headers of the to be registered process
and if its subsystem was SUBSYSTEM_NATIVE, it returned access denied

[2024-09-09 21:17] Matti: but, since this was windows 7 I was able to patch out this check at runtime before calling the API

[2024-09-09 21:17] Matti: with PPLs this won't be quite as easy

[2024-09-09 21:18] diversenok: [replying to Matti: "the thing I ran into when creating native processe..."]
Hmm, inside which call? I remember CreateProcess does an explicit subsystem check

[2024-09-09 21:18] Matti: yeah that's true, but no I was using NtCreateUserProcess or else NtCreateProcess[Ex]

[2024-09-09 21:18] Matti: ehhh I can try to find the call if you want

[2024-09-09 21:19] Matti: one minute

[2024-09-09 21:21] diversenok: I mostly tried running regular GUI/console programs via `NtCreateUserProcess`/`NtCreateProcessEx`

[2024-09-09 21:21] Matti: ugh wrong types but you can guess what the 2 and 3 stand for
[Attachments: image.png]

[2024-09-09 21:21] Matti: it's 'lazy'register because I was calling csrss from inside the native process myself here, I think

[2024-09-09 21:22] Matti: but I also seem to remember getting the same error from the creating process

[2024-09-09 21:23] diversenok: Maybe, maybe, I haven't played much with starting native subsystem files

[2024-09-09 21:24] diversenok: Another thing is CreateProcess passes prohibited image characteristics to `NtCreateUserProcess`

[2024-09-09 21:25] diversenok: That might also fail the call

[2024-09-09 21:25] Matti: nah that isn't it

[2024-09-09 21:25] Matti: it only sets one flag by default IIRC, I don't remember which but it doesn't affect what I was doing

[2024-09-09 21:25] Matti: also again I was calling the NT API 😛

[2024-09-09 21:26] Matti: this might be the non-lazy version of the check - so there is a case where native subsystems are allowed after all
[Attachments: image.png]

[2024-09-09 21:26] Matti: just be in PID 4

[2024-09-09 21:27] Matti: but then it also must be native

[2024-09-09 21:27] diversenok: Either way, you probably can manually create and write the activation context into the target process during creation instead of using the registration API number

[2024-09-09 21:27] Matti: this is all windows 7 FWIW - clearly this is allowed now

[2024-09-09 21:27] diversenok: Shouldn't be too much trouble compared to existing `NtCreateProcessEx` code

[2024-09-09 21:27] Matti: tbh for me it was <:lillullmoa:475778601141403648>

[2024-09-09 21:28] Matti: I spent way way waaay more time on CSRSS registration than on any process creation parts

[2024-09-09 21:29] diversenok: I never actually tried it on Windows 7, so I cannot comment on that

[2024-09-09 21:29] diversenok: But doing it on Windows 10 is alright

[2024-09-09 21:29] diversenok: Let me dig the definitions

[2024-09-09 21:30] Matti: oh yeah no, I believe you

[2024-09-09 21:30] Matti: my code actually wroks on win 10 with no changes

[2024-09-09 21:30] Matti: except I can just leave out the entire CSRSS registration

[2024-09-09 21:30] Matti: when I saw that I was like, what the fuck did I spend those hundreds of hours on if it was never really needed

[2024-09-09 21:31] diversenok: Haha, yeah, it happens

[2024-09-09 21:31] diversenok: One caveat I remember is you need to request extra file access during `NtCreateUserProcess` because CSR needs something like `FILE_READ_ATTRIBUTES` on the handle you pass

[2024-09-09 21:31] Matti: yea

[2024-09-09 21:31] diversenok: It took me a while to figure out

[2024-09-09 21:32] Matti: true, debugging csrss is a skill of its own

[2024-09-09 21:32] Torph: [replying to Matti: "how else would you run `sheep.exe` if there was no..."]
omg i love this

[2024-09-09 21:32] Matti: I actually did this with a user mode debugger, if you can believe it

[2024-09-09 21:32] Matti: without freezing my desktop that is

[2024-09-09 21:32] Matti: I just had to switch to session 0 where I had the debugger

[2024-09-09 21:33] Matti: debug the whatever, then resume csrss before reverting to session 1

[2024-09-09 21:33] diversenok: Nice

[2024-09-09 21:33] Matti: it worked surprisingly well

[2024-09-09 21:33] Matti: much better than using a KM debugger I would say

[2024-09-10 11:26] luci4: [replying to Matti: "in general you also need to have SE_INC_BASE_PRIOR..."]
Welp just tried `NtSetInformationProcess` and I got `STATUS_PRIVILEGE_NOT_HELD`

[2024-09-10 11:27] luci4: Here I was thinking local admin had that... 😦

[2024-09-10 11:27] Matti: they should have it in their token

[2024-09-10 11:27] Matti: present

[2024-09-10 11:27] Matti: but not necessarily enabled

[2024-09-10 11:28] Matti: see <https://github.com/Mattiwatti/EfiGuard/blob/master/Application/EfiDSEFix/src/main.cpp#L40-L52> for an example

[2024-09-10 11:29] Matti: only subsitute your privileges of course

[2024-09-10 11:38] luci4: [replying to Matti: "see <https://github.com/Mattiwatti/EfiGuard/blob/m..."]
🥳

[2024-09-10 11:38] luci4: 
[Attachments: image.png]

[2024-09-10 11:38] luci4: It worked now!

[2024-09-10 11:40] luci4: matter of fact...

[2024-09-10 11:40] luci4: why don't I play with this?

[2024-09-10 11:40] luci4: https://ntdoc.m417z.com/io_priority_hint
[Embed: IO_PRIORITY_HINT - NtDoc]
IO_PRIORITY_HINT - NtDoc, the native NT API online documentation

[2024-09-10 11:40] Matti: is it bad if I get triggered about the fact that your hot zone size is not a power of two?

[2024-09-10 11:41] luci4: [replying to Matti: "is it bad if I get triggered about the fact that y..."]
ill change it rn

[2024-09-10 11:41] luci4: 16 << 20 😎

[2024-09-10 11:41] Matti: [replying to luci4: "https://ntdoc.m417z.com/io_priority_hint"]
this is good yeah

[2024-09-10 11:41] luci4: Since i'll be doing a lot of I/O might as well right?

[2024-09-10 11:42] Matti: never forget to use it to cheat in crystaldiskmark

[2024-09-10 11:42] Matti: well, yes

[2024-09-10 11:42] Matti: but the system isn't stupid

[2024-09-10 11:42] Matti: it will notice you're doing lots of I/O either way

[2024-09-10 11:43] Matti: but, it does help a small amount regardless, in my experience

[2024-09-10 11:43] Matti: that is assuming no other process is doing anything interesting whatsoever ofc

[2024-09-10 11:44] luci4: [replying to Matti: "but, it does help a small amount regardless, in my..."]
I'll just do `IoPriorityHigh`

[2024-09-10 11:44] luci4: And benchmark it with and without

[2024-09-10 11:45] luci4: thx 🙏

[2024-09-10 11:45] Matti: always bench and verify

[2024-09-10 11:45] Matti: but yeah in this case I'm gonna say, it's probably just going to be a miniscule improvement

[2024-09-10 11:46] Matti: beats no improvement

[2024-09-10 11:46] luci4: I'll take what I can get!

[2024-09-10 11:47] Matti: the priority classes (especially I/O priority ones) don't really 'do' much until there is contention for resources on the system

[2024-09-10 11:47] Matti: like, the system isn't throtting you because you forgot to ask for IoPriorityHigh

[2024-09-10 11:48] Matti: unless you are also compiling LLVM at the same time

[2024-09-10 11:49] Matti: but, nowadays so much shitware in windows is doing tiny amounts (but nonstop) I/O all the time that I've found I/O priorities to be a good cheat code that says "hey I'm a real program, give me priority"

[2024-09-10 11:50] luci4: [replying to Matti: "but, nowadays so much shitware in windows is doing..."]
I gotta give it to microsoft, they managed to condense so much crapware into a single OS

[2024-09-10 11:51] luci4: They outdo themselves each time

[2024-09-10 11:52] Matti: yea

[2024-09-10 11:52] Matti: I'm on my work laptop now, which is a piece of shit admittedly
but this is me doing *nothing* on it
[Attachments: image.png]

[2024-09-10 11:53] luci4: [replying to Matti: "I'm on my work laptop now, which is a piece of shi..."]
Whenever I need a new C2 profile at work I just make a new Windows VM and run a HTTP proxy for a while

[2024-09-10 11:53] luci4: Then I just try to mimick their data exfil LOL

[2024-09-10 11:54] Matti: I know some of those words

[2024-09-10 11:55] luci4: [replying to Matti: "I know some of those words"]
Basically how the HTTP request looks

[2024-09-10 11:57] Matti: do they still make it in frontpage? I guess not

[2024-09-10 11:57] Matti: that was the best

[2024-09-10 11:57] x86matthew: dreamweaver these days i think

[2024-09-10 11:57] 5pider: [replying to Matti: "I'm on my work laptop now, which is a piece of shi..."]
22gb of nothing

[2024-09-10 11:57] 5pider: LMFAO

[2024-09-10 11:57] Matti: oh fuck, I forgot about that one

[2024-09-10 11:57] Matti: was dreamweaver the same thing as flash or not?

[2024-09-10 11:58] Matti: I never found out the truth about this

[2024-09-10 11:58] x86matthew: both were created by macromedia (now adobe)

[2024-09-10 11:58] x86matthew: different products though

[2024-09-10 11:58] Matti: [replying to 5pider: "22gb of nothing"]
no it's not technically speaking nothing
it's EDR shitware, running 3 services and 6 drivers

[2024-09-10 11:58] Matti: that's one product

[2024-09-10 11:59] Matti: [replying to x86matthew: "both were created by macromedia (now adobe)"]
ahhh yeah, that makes sense

[2024-09-10 11:59] 5pider: [replying to Matti: "no it's not technically speaking nothing
it's EDR ..."]
LMFAO

[2024-09-10 11:59] 5pider: that edr surely makes use of your unused resources

[2024-09-10 11:59] luci4: EDR is actually a performance enhancing tool

[2024-09-10 11:59] luci4: Whenever I start a process the EDR makes my CPU fans go really fast

[2024-09-10 11:59] luci4: (it prepares the CPU to run the new process)

[2024-09-10 12:01] Matti: this is my corporate laptop right

[2024-09-10 12:01] Matti: I started this job in march, and apart from the parent company's IT dept it's been great

[2024-09-10 12:01] Matti: but holy fuck are they a bunch of morons

[2024-09-10 12:02] Matti: I managed to get 3 'red notices' (= you're fired) in my first 2 months

[2024-09-10 12:03] Matti: for using my laptop in a more productive way than is apparently allowed

[2024-09-10 12:03] luci4: [replying to Matti: "I managed to get 3 'red notices' (= you're fired) ..."]
What did u do?

[2024-09-10 12:03] Matti: **NOTHING**

[2024-09-10 12:03] Matti: well almost nothing

[2024-09-10 12:04] luci4: Did u make EfiGuard fight the EDR? lol

[2024-09-10 12:04] Matti: the very first one was deserved

[2024-09-10 12:04] Azrael: [replying to Matti: "I managed to get 3 'red notices' (= you're fired) ..."]
Very relatable.

[2024-09-10 12:04] Matti: I downloaded a cracked version of navicat so I wouldn't have to endure SQL server management studio

[2024-09-10 12:04] Matti: IT dept all up in arms about randomware risk and whatnot

[2024-09-10 12:05] Matti: I had to resist real hard telling them it was my own crack to begin with

[2024-09-10 12:05] Azrael: [replying to Matti: "I downloaded a cracked version of navicat so I wou..."]
Last company I worked at gave out SQL SMS access as a privilege.

[2024-09-10 12:05] luci4: [replying to Matti: "IT dept all up in arms about randomware risk and w..."]
LOL

[2024-09-10 12:06] Matti: there's a time and place to be cheeky... now even a few months later I would have said that

[2024-09-10 12:06] Matti: cuse the guy was a grade A moron

[2024-09-10 12:06] Matti: but this was my 2nd day on the job

[2024-09-10 12:07] Matti: so I 'yes sir, my bad sir'd my way out of the situation

[2024-09-10 12:07] Azrael: Experiencing that office politics.

[2024-09-10 12:08] Matti: the senior dev who was supposed to be bringing me up to speed was dying

[2024-09-10 12:08] Matti: anyway that was the first incident

[2024-09-10 12:08] Azrael: What.

[2024-09-10 12:09] Matti: the second one was for installing OpenVPN
the third was for running regedit

[2024-09-10 12:09] Matti: [replying to Azrael: "What."]
wdym what

[2024-09-10 12:09] Azrael: Did he die?

[2024-09-10 12:10] Matti: we were both dying of laughter

[2024-09-10 12:10] luci4: [replying to Azrael: "Did he die?"]
Of laughter

[2024-09-10 12:10] Azrael: Oh lol.

[2024-09-10 12:10] Azrael: [replying to Matti: "the second one was for installing OpenVPN
the thir..."]
Regedit is malicious software.

[2024-09-10 12:10] luci4: Always love when a `PROCESSINFOCLASS` member like `ProcessEffectiveIoPriority` returns `STATUS_INVALID_INFO_CLASS`

[2024-09-10 12:10] luci4: Its right there, how is it invalid???

[2024-09-10 12:11] Azrael: Can't go around running that 🙅‍♂️

[2024-09-10 12:12] luci4: [replying to Azrael: "Can't go around running that 🙅‍♂️"]
Then don't make it an option, lol

[2024-09-10 12:12] Matti: but yeah the reason I'm still working there is because my supervisor looked into what they accused me of

[2024-09-10 12:13] Matti: like properly

[2024-09-10 12:13] Matti: and said there wasn't a shred of evidence

[2024-09-10 12:13] Azrael: [replying to luci4: "Then don't make it an option, lol"]
Why would they do that when they can just fire their employees after opening it 3 times?

[2024-09-10 12:14] Matti: believe it or not but most people I work with are actually pretty competent

[2024-09-10 12:14] Matti: except for the IT department, because it is based in the headquarters in france

[2024-09-10 12:18] luci4: ```
    IO_PRIORITY_HINT IoPriority = IoPriorityHigh;

    Status = NtSetInformationProcess(NtCurrentProcess(), ProcessEffectiveIoPriority, &IoPriority, sizeof(IO_PRIORITY_HINT));
```
There is 0 reason for this to return `STATUS_INVALID_INFO_CLASS`

[2024-09-10 12:18] luci4: Unless user-mode forces me to use `ProcessIoPriority` instead

[2024-09-10 12:18] luci4: [replying to luci4: "Unless user-mode forces me to use `ProcessIoPriori..."]
That fixed it

[2024-09-10 12:18] luci4: for some reason

[2024-09-10 12:19] Matti: those aren't the same thing though 😉

[2024-09-10 12:19] luci4: [replying to Matti: "those aren't the same thing though 😉"]
I know 😦

[2024-09-10 12:19] luci4: It's probably like soft/hard affinity

[2024-09-10 12:19] luci4: It has HINT in the name...

[2024-09-10 12:22] Matti: ProcessIoPriority is what you would like it to be (1 million please), and if it's similar to thread priorities it will also be stored that way, roughly speaking

[2024-09-10 12:24] Matti: ProcessEffectiveIoPriority is what the kernel makes of it after taking care of process/thread/job limits, file object limits and people who ask for 1 million which is a bit unfair to other processses

[2024-09-10 12:25] Matti: and lots of other factors I can't think of right now

[2024-09-10 12:25] luci4: Ah I see

[2024-09-10 12:27] Matti: being a foreground process (on a non-server installation) probably also boosts your I/O a tiny bit by default

[2024-09-10 12:27] Matti: and the reverse on servers

[2024-09-10 12:34] luci4: [replying to Matti: "being a foreground process (on a non-server instal..."]
Seems fair enough

[2024-09-10 12:43] diversenok: [replying to luci4: "```
    IO_PRIORITY_HINT IoPriority = IoPriorityHi..."]
https://ntdoc.m417z.com/processinfoclass#processeffectiveiopriority-110
[Embed: PROCESSINFOCLASS - NtDoc]
PROCESSINFOCLASS - NtDoc, the native NT API online documentation

[2024-09-10 12:44] diversenok: It's only valid for query

[2024-09-10 12:44] diversenok: Because its meaning is "calculate my actual priority considering the job object restrictions"

[2024-09-10 12:46] diversenok: Setting is like "please fake the result of these calculations for me" 😂

[2024-09-10 12:46] luci4: [replying to diversenok: "Setting is like "please fake the result of these c..."]
🤣

[2024-09-10 12:47] luci4: returns an NTGASLIGHT value?

[2024-09-10 12:47] diversenok: Also it's not supported on Windows 10

[2024-09-10 12:48] diversenok: STATUS_WTF_DO_YOU_WANT

[2024-09-10 12:49] luci4: [replying to diversenok: "Also it's not supported on Windows 10"]
My goal with this app is to have it run on atleast Win7

[2024-09-10 12:51] diversenok: If you don't exit when tweaking these things fails it should be fine

[2024-09-10 12:52] luci4: [replying to diversenok: "If you don't exit when tweaking these things fails..."]
Nah, this is just the cherry on top 😄

[2024-09-10 12:52] diversenok: The nice design of functions that use info classes is that many of them existed since the dawn of time

[2024-09-10 12:53] diversenok: So importing `NtSetInformationProcess` definitely doesn't break loading, even you plan to use only the latest features

[2024-09-10 13:02] luci4: [replying to diversenok: "So importing `NtSetInformationProcess` definitely ..."]
Oh that's perfect

[2024-09-10 13:02] luci4: The final product will look something like:

```
    ULONGLONG       NewCommitLimit{ (MemoryInformation.AvailableBytes * 70) / 100 };
    QUOTA_LIMITS_EX Limits        { .MinimumWorkingSetSize = NewCommitLimit, .MaximumWorkingSetSize = NewCommitLimit };

    do
    {
        NewCommitLimit -= (MemoryInformation.AvailableBytes * 10) / 100;

        if (NewCommitLimit < DEFAULT_MAX_WORKING_SET)
        {
            return NULL;
        }

    } while ((NtSetInformationProcess(NtCurrentProcess(), ProcessQuotaLimits, &Limits, sizeof(QUOTA_LIMITS_EX)) != STATUS_SUCCESS));
```

[2024-09-10 13:03] luci4: I will also store `NewCommitLimit` in an evil global variable

[2024-09-10 13:07] luci4: Now all that remains is the allocation logic surrounding the commit limit I worked so hard to increase LOL

[2024-09-10 13:25] Matti: [replying to luci4: "My goal with this app is to have it run on atleast..."]
matti stamp of approval

[2024-09-10 13:26] Matti: if you open source it I'll make it run on XP if it doesn't already

[2024-09-10 13:26] luci4: [replying to Matti: "if you open source it I'll make it run on XP if it..."]
I definitely will

[2024-09-10 13:27] luci4: After I have a v0.1

[2024-09-10 13:27] Matti: fuck this reminds me, I was like 95% done porting python 3.12 to XP

[2024-09-10 13:28] Matti: no particular reason, just to prove that python devs are lazy as fuck

[2024-09-10 13:28] Matti: and then I remembered I hate python

[2024-09-10 13:28] Matti: and kinda gave up on it

[2024-09-10 13:31] Matti: I think it was some test case about paths that finally drove me over the limit

[2024-09-10 13:32] Matti: and then finding out it wasn't even my code, the test was just broken depending on which version of windows you ran it on

[2024-09-10 13:51] luci4: [replying to Matti: "matti stamp of approval"]
Win8 and Win10 haven't introduced any amazing APIs as far as I'm aware, so why not?

[2024-09-10 13:52] luci4: 32-bit support and Win7+ was what I have in mind for the final stretch

[2024-09-10 13:53] luci4: A few LARGE_INTEGERs there, some AWE...32-bit support can't be that hard to achieve

[2024-09-10 13:53] Matti: 32 bit support is doable enough

[2024-09-10 13:53] Matti: but like... why

[2024-09-10 13:54] luci4: [replying to Matti: "but like... why"]
mmmm

[2024-09-10 13:54] luci4: yeah ur right tbh

[2024-09-10 13:54] luci4: Even my childhood computer wasn't 32-bit lol

[2024-09-10 13:54] luci4: I thought that maybe someone would want to get their data off a super-legacy machine, but eh

[2024-09-10 13:55] Matti: yeah but even legacy machines support 64 bit now

[2024-09-10 13:55] Matti: opteron came out 21 years ago

[2024-09-10 13:56] Matti: there are some people who insist on running 32 bit XP on perfectly 64 bit capable hardware

[2024-09-10 13:56] Matti: but well, some people are braindead

[2024-09-10 13:57] luci4: [replying to Matti: "there are some people who insist on running 32 bit..."]
That doesn't make much sense

[2024-09-10 13:57] Matti: I know!!!!!

[2024-09-10 13:57] Matti: and there are dozens of them

[2024-09-10 13:58] luci4: [replying to luci4: "I thought that maybe someone would want to get the..."]
At that point, they can make a PR themselves if they really need it tbh

[2024-09-10 13:58] Matti: way more than there are XP 64 bit users, in fact

[2024-09-10 13:58] prick: [replying to Matti: "there are some people who insist on running 32 bit..."]
who

[2024-09-10 13:58] prick: hobbyists?

[2024-09-10 13:58] Matti: morons

[2024-09-10 13:58] Matti: I just said

[2024-09-10 13:59] prick: as a daily driver?

[2024-09-10 13:59] Matti: yes

[2024-09-10 13:59] Matti: I mean they are actual honest to god morons

[2024-09-10 13:59] Matti: so yes, they do that

[2024-09-10 14:00] prick: i doubt you're factoring in drivers if we're talking about daily driving some shit

[2024-09-10 14:00] prick: xp x64 had like 20% of the drivers 32 bit did

[2024-09-10 14:00] prick: it pretty much went to vista / 7 driver dev

[2024-09-10 14:01] Matti: yeah, but it was also a 64 bit OS instead of 32 or 36 with PAE

[2024-09-10 14:01] Matti: if you're missing drivers, just write them yourself

[2024-09-10 14:02] Matti: it's not like drivers from back then were very complex

[2024-09-10 14:02] Matti: [replying to prick: "it pretty much went to vista / 7 driver dev"]
I never used XP 64 bit when it was current, FWIW

[2024-09-10 14:02] Matti: but it's 2024 now

[2024-09-10 14:03] prick: ehhh you can't just say write your own drivers to like 99% of people

[2024-09-10 14:03] Matti: no that is the amazing thing

[2024-09-10 14:03] Matti: you can totally tell these people that

[2024-09-10 14:03] Matti: and they will try

[2024-09-10 14:04] Matti: make forum threads hundreds of pages long about how to port the creative x-fi driver to 64 bit*

*hypothetical scenario, I already said they use 32 bit exclusively

[2024-09-10 14:04] Matti: they are very religious about the 32 bit thing

[2024-09-10 14:05] prick: i'm not sure why anyone would actually try to daily it

[2024-09-10 14:05] prick: the only person i know that does is the reactos release manager

[2024-09-10 14:05] Matti: me neither

[2024-09-10 14:05] prick: he's the only person resistant to nt6+ stuff obviously

[2024-09-10 14:05] Matti: but if I had to use XP every day, I'd sure as fuck pick 64 bit

[2024-09-10 14:05] prick: i think he does use x64

[2024-09-10 14:05] prick: atleast

[2024-09-10 14:06] prick: ive got a sandy bridge board that works OK with xp

[2024-09-10 14:06] Matti: if I download the current x64 rectos live cd from their site right now and put it in virtualbox, wil it boot? serious question

[2024-09-10 14:06] prick: no idea

[2024-09-10 14:07] prick: i do know they actively use it for dev now

[2024-09-10 14:07] prick: try your luck

[2024-09-10 14:07] Matti: I have some suspicion after the last time I tried this
but that was a while ago, hence the question

[2024-09-10 14:07] Matti: like a few months at least

[2024-09-10 14:07] luci4: [replying to luci4: "After I have a v0.1"]
One step closer

   ``` 
    FileStruct->Handle      = FileHandle;
    FileStruct->BytesToRead = FileSize;
    FileStruct->State       = STATE_READ_FILE;

    if (FileStruct->BytesToRead < SMALL_FILE_SIZE)
    {
        FileStruct->Buffer     = HotArea;
        FileStruct->BufferSize = FileSize;
    }
    else if (FileStruct->BytesToRead < MaxCommit)
    {
        FileStruct->BufferSize = FileSize;

        do
        {
            FileStruct->BufferSize -= (FileSize * 10) / 100;

            if (FileStruct->BufferSize == NULL)
            {
                FileStruct->BufferSize = FileSize;

                Sleep(10 * 1000);
            }

        } while (!NT_SUCCESS(NtAllocateVirtualMemory(NtCurrentProcess(), &FileStruct->Buffer, NULL, &FileStruct->BufferSize, MEM_COMMIT, PAGE_READWRITE)));

    }
```

[2024-09-10 14:08] luci4: I think that's gonna be the logic for files < the commit limit I previously raised

[2024-09-10 14:08] prick: i am too lazy to use virtualbox so i'm just gonna try it in vmware

[2024-09-10 14:08] prick: all of these virtualization software conflicts make me want to genocide corporate hv devs

[2024-09-10 14:08] Matti: I woud advise against that in general

[2024-09-10 14:08] Matti: but especially when it comes to reactos

[2024-09-10 14:09] prick: 32 booted fine

[2024-09-10 14:09] prick: <:clueless:1172258895109034095>

[2024-09-10 14:09] Matti: well ye

[2024-09-10 14:09] luci4: [replying to luci4: "One step closer

   ``` 
    FileStruct->Handle   ..."]
Definitely need to change Sleep to NtDelayExecution, though, lol

[2024-09-10 14:10] Matti: well those do the same thing

[2024-09-10 14:10] Matti: I mean, it's a pretty miniscule difference

[2024-09-10 14:10] Matti: why are you sleeping in the first place? waiting until the allocation succeeds?

[2024-09-10 14:11] luci4: [replying to Matti: "why are you sleeping in the first place? waiting u..."]
Welp if the file size is under the new limit, I'm gonna try and allocate for it in one swoop

[2024-09-10 14:11] prick: is this some windows bug workaround i haven't been paying attention

[2024-09-10 14:11] luci4: For each failure I am decrementing 10% of the allocation size

[2024-09-10 14:11] luci4: If for some reason the allocation size reaches 0, and it couldn't even allocate 10% of its size

[2024-09-10 14:11] luci4: I'm gonna wait a bit for the completion port threads to free some of the memory they are using

[2024-09-10 14:12] luci4: then proceed with trying again to allocate it in full

[2024-09-10 14:14] Matti: well, I did my best but failed to find an amd64 iso on the reactos page

[2024-09-10 14:14] prick: 
[Attachments: image.png]

[2024-09-10 14:14] prick: amd64 works fine

[2024-09-10 14:14] Matti: send me the link

[2024-09-10 14:14] prick: <https://iso.reactos.org/livecd/>
[Attachments: image.png]

[2024-09-10 14:15] Matti: oh man, how stupid of me

[2024-09-10 14:15] Matti: I was on sourceforge cause that's where their site sends you when you click download

[2024-09-10 14:15] Matti: ok, on it

[2024-09-10 14:15] prick: 
[Attachments: image.png, image.png]

[2024-09-10 14:16] luci4: [replying to luci4: "I'm gonna wait a bit for the completion port threa..."]
Seems fairly reasonable

[2024-09-10 14:16] luci4: 🤷‍♂️

[2024-09-10 14:17] Matti: [replying to prick: ""]
are you trying to make a point here?
if I had to guess it'd be something like 'this is an awful website', but idt that's what you're going for

[2024-09-10 14:17] prick: iti s an awful website yes

[2024-09-10 14:17] prick: it will occasionally go down

[2024-09-10 14:18] prick: i wonder if enough of smp is merged, time to try

[2024-09-10 14:19] Matti: omitting the menu which is a kilometre away on my screen, which button do you think I clicked
[Attachments: image.png]

[2024-09-10 14:19] Matti: however

[2024-09-10 14:19] Matti: if you click nightly builds you can also get them

[2024-09-10 14:19] Matti: after switching the checkboxes around

[2024-09-10 14:20] prick: 0.4.14 is a build from 2021 since they decided it's pretty much pointless to "release" until something is tangibly better from end user pov

[2024-09-10 14:20] prick: just use nightly

[2024-09-10 14:20] prick: 0.4.15 is closing in though

[2024-09-10 14:20] Matti: > discover oour latest and greatest

[2024-09-10 14:20] Matti: maybe they should reword this then

[2024-09-10 14:20] luci4: <:topkek:904522829616263178>

[2024-09-10 14:21] Matti: anyway I normally build reactos from master

[2024-09-10 14:21] Matti: I'm just not at my PC right now

[2024-09-10 14:21] prick: the most advanced version of reactos is a combination of like 10 different NT6 feature testing branches

[2024-09-10 14:22] prick: which doesn't exist you gotta cherrypick yourself

[2024-09-10 14:22] prick: https://github.com/ReactOS-Longhorn-Initiative/reactos
[Embed: GitHub - ReactOS-Longhorn-Initiative/reactos: A free Windows-compat...]
A free Windows-compatible Operating System. Contribute to ReactOS-Longhorn-Initiative/reactos development by creating an account on GitHub.

[2024-09-10 14:22] prick: main nt6 work happens here

[2024-09-10 14:22] prick: which is meant to eventually be upstreamed

[2024-09-10 14:23] Matti: mm

[2024-09-10 14:23] Matti: great that it's a separate repo...

[2024-09-10 14:23] Matti: any easy way to see what the main changes are so far?

[2024-09-10 14:24] prick: easy?

[2024-09-10 14:24] Matti: yea

[2024-09-10 14:24] prick: meh frankly i only know this much because i'm in their chatrooms

[2024-09-10 14:24] prick: join the nt6 reactos discord

[2024-09-10 14:25] Matti: like a bullet list with the main improvements compared to current reactos

[2024-09-10 14:25] Matti: it could even be in the readme

[2024-09-10 14:25] Matti: [replying to prick: "join the nt6 reactos discord"]
I don't think I will lol

[2024-09-10 14:26] prick: well what i can tell you is shit like wddm work and a bunch of other shit driver related is currently blocked by a shit hal and acpi

[2024-09-10 14:26] prick: once that gets upgraded to vista style acpi hal

[2024-09-10 14:27] prick: a bunch of shit will start falling in place

[2024-09-10 14:27] prick: because they already did a lot of wddm/dxgk work

[2024-09-10 14:27] prick: their solution for directx will eventually be just dxvk

[2024-09-10 14:28] Matti: apart from the user mode stuff, which I don't touch at all
I feel like it's maybe arrogant but probably accurate to say this code is behind my own HAL and kernel by some.... years? idk

[2024-09-10 14:28] prick: they're going to rip out all of their home grown directx code in the interest of having functioning apps

[2024-09-10 14:28] Matti: now, it's not a fair comparison I know

[2024-09-10 14:28] Matti: cause the WRK was written by MS

[2024-09-10 14:28] Matti: but I'm talking about the changes made relative to the server 2003 (let's call it) base code

[2024-09-10 14:29] prick: well what can i tell you

[2024-09-10 14:29] prick: windows 8.1 style Po is being worked on which will enable sleep states, hibernation, etc.

[2024-09-10 14:29] Matti: but, I also feel like it's kinda lame to talk about how much better my kernel is when I can't legally open source it

[2024-09-10 14:30] prick: battery management

[2024-09-10 14:30] prick: of course that in addition to all of the other vista+ stuff

[2024-09-10 14:30] prick: i just listed

[2024-09-10 14:30] prick: im not sure what else u want

[2024-09-10 14:30] Matti: I don't want any of that <:lillullmoa:475778601141403648>

[2024-09-10 14:30] Matti: sorry to say

[2024-09-10 14:31] Matti: I only care about kernel mode

[2024-09-10 14:31] Matti: yeah battery management is kernel mode matti

[2024-09-10 14:31] prick: Po is kernel and drivers interact with it

[2024-09-10 14:31] Matti: true but it's boring

[2024-09-10 14:32] prick: eeeeeeeee they're working on ALPC's and eventually going to work on vista TLS

[2024-09-10 14:32] prick: those are some next work items

[2024-09-10 14:32] Matti: mm

[2024-09-10 14:32] prick: they need thread pools

[2024-09-10 14:32] Matti: yeah those are nice

[2024-09-10 14:33] prick: thread pools are such a clusterfuck

[2024-09-10 14:33] Matti: they aren't that bad

[2024-09-10 14:33] prick: im sure once you understand them it's ok but you know

[2024-09-10 14:33] prick: that's a lotta new api

[2024-09-10 14:34] Matti: worker factories are really just wrappers for IO completion ports, which are just KQUEUEs

[2024-09-10 14:34] Matti: the complex part is in user mode, I'd say

[2024-09-10 14:34] Matti: ntdll specifically

[2024-09-10 14:35] prick: speaking of
[Attachments: image.png]

[2024-09-10 14:35] Matti: ALPC on the other hand I have to say is just... simple?

[2024-09-10 14:35] Matti: elegant, simple and performant

[2024-09-10 14:36] Matti: one of the best things in NT other than IOCPs

[2024-09-10 14:37] Matti: EFI or not? for the reactos vm

[2024-09-10 14:37] prick: their current userland hustle is playing a snakes and ladders game of bringing up interdependent wine code up until a point wine merging can be handled with a script

[2024-09-10 14:37] prick: [replying to Matti: "EFI or not? for the reactos vm"]
idk if they support efi

[2024-09-10 14:37] prick: hasn't crossed my mind

[2024-09-10 14:37] Matti: they have a lot of code for it

[2024-09-10 14:37] Matti: I just don't know if it works

[2024-09-10 14:37] Matti: I'll leave it off then

[2024-09-10 14:37] prick: just boot the livecd

[2024-09-10 14:37] Matti: relax, I'm on it

[2024-09-10 14:38] Matti: but booting as EFI or BIOS kinda makes a difference

[2024-09-10 14:38] prick: i simply click on iso file <:clueless:1172258895109034095>

[2024-09-10 14:40] Matti: attempt #1
[Attachments: image.png]

[2024-09-10 14:40] Matti: I"ll blame this on vbox, don't know what the fuck it is doing

[2024-09-10 14:40] Matti: the ISO is obviously inserted

[2024-09-10 14:41] Matti: alright, got it

[2024-09-10 14:41] Matti: and indeed it boots

[2024-09-10 14:41] Matti: bravo

[2024-09-10 14:42] prick: what's happening right now is they're laxing a bit on having looked at MS source leaks or not

[2024-09-10 14:42] prick: with respect to

[2024-09-10 14:42] Matti: <:lillullmoa:475778601141403648>
[Attachments: image.png]

[2024-09-10 14:42] prick: currently available source leaks

[2024-09-10 14:42] Matti: this'll be live cd vs install cd

[2024-09-10 14:42] Matti: but still

[2024-09-10 14:42] Matti: funny error

[2024-09-10 14:43] prick: if you open a PR that concerns currently available source leaks they'll deny it

[2024-09-10 14:43] prick: so like

[2024-09-10 14:43] prick: if you tried to contribute it'd be some NT6 stuff you can't find in WRK / xp leak

[2024-09-10 14:45] Matti: I'm not sure I follow

[2024-09-10 14:45] Matti: either you've got a leak and it's available

[2024-09-10 14:45] Matti: or you don't have a leak

[2024-09-10 14:45] prick: hm?

[2024-09-10 14:46] prick: you're required to contribute with your real name and if they find your PR sus they'll have someone look at it to see if its leaked code

[2024-09-10 14:46] prick: it's happened a few times before

[2024-09-10 14:46] Matti: yes, this part I know

[2024-09-10 14:46] prick: what they are currently doing is

[2024-09-10 14:46] prick: laxing the policy a bit and considering allowing people that have looked at these leaks

[2024-09-10 14:47] prick: as long as they don't try to contribute features in any way present

[2024-09-10 14:47] prick: in what is currently leaked

[2024-09-10 14:47] Matti: lol, about time

[2024-09-10 14:47] Matti: would look a lot less dishonest at least

[2024-09-10 14:47] prick: it's not that dishonest they're quite religious about it

[2024-09-10 14:47] Matti: I don't think there is going to be any change here

[2024-09-10 14:50] Matti: the fact that they're religious about some PRs, doesn't seem to have prevented half of the ROS repository from looking like a WRK clone, down to naming of variables and private functions

[2024-09-10 14:50] Matti: I don't personally *care* just to clarify

[2024-09-10 14:50] Matti: after all my kernel is built on the WRK

[2024-09-10 14:51] Matti: it just always seemed very hypocritical

[2024-09-10 14:51] prick: they went through a legal battle with microsoft in the late 2000s iirc. there's definitely eras of code to it. the CURRENT developers are religious, ones from the past might've been dishonest

[2024-09-10 14:51] prick: there's not too many devs left from back then

[2024-09-10 14:51] Matti: damn this is starting to sound like a political party

[2024-09-10 14:51] prick: maybe like, 2

[2024-09-10 14:52] prick: like in userland there's some ancient mshtml ida paste that they don't like being there

[2024-09-10 14:53] prick: a pr for that these days would just be rejected (or told to properly reverse and clean it up)

[2024-09-10 14:53] prick: but nobody is willing to contribute to mshtml

[2024-09-10 14:53] prick: it's a project from the 90s and the target for it has shifted multiple times

[2024-09-10 14:53] prick: nt4... 2000... then it landed on 2003

[2024-09-10 14:54] prick: as far as i know multiuser support doesn't work

[2024-09-10 14:55] prick: they don't have a ton of userland functionality

[2024-09-10 14:55] prick: they need netbios, active directory, shit like that

[2024-09-10 15:03] luci4: I wonder if this approach:

```C
    while (TRUE)
    {        
        Status = NtRemoveIoCompletion(CompletionPort, &Context, &ApcContext, &IoStatusBlock, NULL);

        File = reinterpret_cast<PFILE_STRUCT>(Context);

        switch (File->State)
        {
        case STATE_READ_FILE:
            // NtReadFile -> Change File.State to STATE_WRITE_FILE -> back to the IOCP
            break;

        case STATE_WRITE_FILE:
            // NtWriteFile -> increase File.Offset -> Change File.State to STATE_READ_FILE -> back to IOCP
            break;


        default:
            
        }

    }
```
is worse off than having each IOCP thread just do the whole thing

[2024-09-10 15:04] luci4: I'm gonna have to benchmark this later

[2024-09-10 15:06] luci4: To avoid the overheard of calling `NtSetIoCompletion` again, I was thinking of pass the file struct again as the APC context

[2024-09-10 15:13] Matti: [replying to luci4: "I wonder if this approach:

```C
    while (TRUE)
..."]
probably yeah

[2024-09-10 15:14] Matti: after all you can always just make more consumer threads if needed

[2024-09-10 15:15] luci4: [replying to Matti: "probably yeah"]
I started out by creating `NtCurrentPeb()->NumberOfProcessors` threads using `RtlCreateUserThread`.

[2024-09-10 15:17] Matti: is that a thing? TIL

[2024-09-10 15:18] Matti: I'd just use `NtQuerySystemInformation(SystemBasicInformation)`

[2024-09-10 15:18] luci4: [replying to Matti: "is that a thing? TIL"]
Haha yeah lol, the fastest way I could think of getting that info

[2024-09-10 15:19] Matti: yeah they probably just return the same thing

[2024-09-10 15:19] Matti: but remember the PEB is in user mode memory and can be manipulated

[2024-09-10 15:19] Matti: (most often by windows itself)

[2024-09-10 15:21] Matti: I doubt it's anything you'd need to worry about, since these hacks only get made for super broken apps that are also widely used

[2024-09-10 15:21] Matti: but all the same I'd prefer a syscall over the PEB if you want reliable information

[2024-09-10 15:22] luci4: [replying to Matti: "but all the same I'd prefer a syscall over the PEB..."]
The PEB should be enough in this scenario, tbh

[2024-09-10 15:48] JustMagic: [replying to Matti: "I'd just use `NtQuerySystemInformation(SystemBasic..."]
user_shared_data > all

[2024-09-10 15:54] Matti: ya, I was gonna mention that as the one exception

[2024-09-10 15:54] Matti: does it contain a cpu count field though? I don't remember

[2024-09-10 15:54] Matti: probably does

[2024-09-10 16:01] JustMagic: [replying to Matti: "probably does"]
ActiveProcessorCount

[2024-09-11 11:50] hxm: how to force clang to not use the __builtin_memset_inline , while keep the opt on.
```c
bolt\include\bolt\Core\MCPlusBuilder.h
  }

  /// Create an inline version of memcpy(dest, src, 1).
  virtual InstructionListType createOneByteMemcpy() const {
    llvm_unreachable("not implemented");
..
```

[2024-09-11 11:51] hxm: there is also an

`virtual InstructionListType createInlineMemcpy(bool ReturnEnd) const`

[2024-09-13 20:52] BWA RBX: [replying to hxm: "how to force clang to not use the __builtin_memset..."]
What are flags are you using for compilation

[2024-09-13 21:11] BWA RBX: [replying to hxm: "how to force clang to not use the __builtin_memset..."]
Could probably use preprocessor directives to achieve it, or if needed just edit LLVM directly, not much experience with LLVM hope you find a solution soon