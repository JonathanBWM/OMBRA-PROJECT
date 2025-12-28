# May 2024 - Week 4
# Channel: #programming
# Messages: 124

[2024-05-22 11:46] Horsie: I have a this bs AV at work which wont let me read some files it ownes in the program files dir

[2024-05-22 11:46] Horsie: Any tool that would help me read those?

[2024-05-22 11:46] Horsie: Probably a filesystem filter

[2024-05-22 12:05] diversenok: Do it offline or boot into safe mode are probably the easiest options

[2024-05-22 12:06] diversenok: And by offline I mean without booting the OS, maybe by attaching the drive somewhere else

[2024-05-22 13:48] Horsie: [replying to diversenok: "Do it offline or boot into safe mode are probably ..."]
Thats kind of too complicated for my usecase

[2024-05-22 13:49] Horsie: Because I need to run a script that analyses some of the db files it protects every day

[2024-05-22 13:49] Horsie: Maybe a signed driver that will let me do it?

[2024-05-22 13:51] diversenok: What exactly does it mean when you say it "wont let read files"?

[2024-05-22 13:51] diversenok: What token do you run with, what security descriptor do files have

[2024-05-22 13:52] diversenok: Does it give you access denied or sharing violation

[2024-05-22 13:52] diversenok: What access you can still open the files for, if not `FILE_READ_DATA`

[2024-05-22 13:56] diversenok: Without this info you are effectively asking for a universal pill that solves all of your problems

[2024-05-22 15:38] Horsie: [replying to diversenok: "What token do you run with, what security descript..."]
Well I'm trying to just copy them with the UI for now

[2024-05-22 15:39] Horsie: I'm logged in as Administrator (the Properties->Security tab says everyone has Read access, at least) and trying to copy it elsewhere

[2024-05-22 15:39] Horsie: Ends up with access denied.

[2024-05-22 15:39] Horsie: Says I need to be SYSTEM

[2024-05-22 15:39] Horsie: AV is kaspersky, if relevant

[2024-05-22 15:39] Horsie: Cant unload their filesystem filter with PH, etc

[2024-05-22 15:40] Horsie: I'd prefer if this has a simple solution.

[2024-05-22 17:15] Deleted User: what would be the least invasive and most universal way of getting a dll loaded into a process automatically

[2024-05-22 17:34] luci4: Sideloading would be a good option

[2024-05-22 17:35] luci4: If by automatically you mean on start

[2024-05-22 17:36] Matti: [replying to Horsie: "I'd prefer if this has a simple solution."]
figure out which driver is the FS minifilter driver doing the blocking, take ownership of it (as TrustedInstaller likely owns it), and then add a Deny ACE for the Everyone group solely for 'execute'
reboot and pray your system keeps working

[2024-05-22 17:37] Matti: this is assuming it works like WdBoot/WdFilter and protects itself from being disabled - if not, simply
`sc config shitware_driver start= disabled`
and reboot

[2024-05-22 17:37] Deleted User: yeah i think ill just go with a simple loader to create the process suspended and CreateRemoteThread LoadLibraryA what i need

[2024-05-22 17:38] Deleted User: kernel32 stuff should be at the same addresses for 32bit processes right

[2024-05-22 17:41] Matti: [replying to Matti: "figure out which driver is the FS minifilter drive..."]
second alternative - use a ReFS volume (preferably a trusted dev drive with the fsutil setting to disallow AV minifilters on those, but lots of minifilters just shit the bed on any non-NTFS volume regardless)

[2024-05-22 17:43] Matti: 
[Attachments: image.png]

[2024-05-22 17:44] Deleted User: damn didn't know refs can do that

[2024-05-22 17:44] Matti: it's not refs really

[2024-05-22 17:45] Matti: it's just something MS decided to arbitrarily limit to refs

[2024-05-22 17:45] Matti: you can disable all of the same FS minifilters on NTTFS volumes too if you put in enough effort

[2024-05-22 17:46] Matti: but, refs is looking to be a superior filesystem soon if not already regardless

[2024-05-22 17:46] Matti: [replying to Matti: ""]
btw, do **not** copy the `/L:disable` from this screenshot, that was just for performance testing

[2024-05-22 17:46] Matti: it disables "integrity".... whatever that means

[2024-05-22 17:47] Matti: but overall I would say FS integrity is probably desirable

[2024-05-22 18:02] froj: [replying to Deleted User: "what would be the least invasive and most universa..."]
path hijacking is extremely common albeit not universal as you'd need to recompile, but depends what you mean by automatically here - no external scripts needed?

[2024-05-22 18:03] froj: [replying to Deleted User: "yeah i think ill just go with a simple loader to c..."]
Ah yeah this'll be fine

[2024-05-22 18:03] froj: [replying to Deleted User: "kernel32 stuff should be at the same addresses for..."]
Yeah anything in KnownDlls will be at the same base address system wide, same goes for KnownDlls32

[2024-05-22 18:03] froj: Just make sure the arch matches and you'll be fine

[2024-05-22 19:18] JustMagic: [replying to Matti: "but, refs is looking to be a superior filesystem s..."]
I was kind of under the impression that refs was a failure until the most recent dev drive stuff where they seem to be pushing it

[2024-05-22 19:20] Matti: well I haven't used it long enough to say for sure, but if it lives up to the promise of not constantly corrupting the FS like NTFS does I'll continue using it

[2024-05-22 19:20] Matti: perf wise it's... interesting

[2024-05-22 19:20] Matti: it's much faster than NTFS on my consumer SSDs (samsung 980/990 and such), but slower on my optanes

[2024-05-22 19:21] Matti: not by a huge amount either way, but enough to be noticeable

[2024-05-22 19:21] JustMagic: [replying to Matti: "it's much faster than NTFS on my consumer SSDs (sa..."]
With the same filters on top?

[2024-05-22 19:21] Matti: yea

[2024-05-22 19:21] Matti: I already have nearly every FS filter disabled on NTFS

[2024-05-22 19:22] Matti: I tried a variety of settings, like disabling or enabling TXF, and the minifilters obviously

[2024-05-22 19:22] Matti: none seemed to make much of a difference

[2024-05-22 19:22] JustMagic: Interesting. I was under the impression that it's a little slower than NTFS

[2024-05-22 19:23] Matti: it is on my optanes

[2024-05-22 19:23] Matti: dunno what's up with the samsungs

[2024-05-22 19:23] JustMagic: [replying to Matti: "it is on my optanes"]
Insert the aliens dude meme

[2024-05-22 19:23] Matti: only 3d xpoint represents real world performance anyway

[2024-05-22 19:23] Matti: samsung is just SLC caching everything

[2024-05-22 19:25] Matti: intel(R) optane(TM) P5800X
[Attachments: image.png]

[2024-05-22 19:25] Matti: s*msung 980 or 990, I forget
[Attachments: image-1.png]

[2024-05-22 19:29] Matti: the second comparison isn't entirely fair because I'm allowing 0 minifilters on refs vs like 3 on NTFS.... but like I said the difference isn't as big as you might expect

[2024-05-22 19:30] Matti: plus win 11 comes with like 30 minifilters by default, so 3 is already quite a reduction

[2024-05-22 19:31] Matti: one thing I'm not gonna fucking try for the foreseeable future is booting windows from refs

[2024-05-22 19:31] Matti: no thank you

[2024-05-22 19:47] Matti: [replying to Matti: "well I haven't used it long enough to say for sure..."]
oh look here we go again
[Attachments: image.png]

[2024-05-22 19:48] Matti: > Windows has made corrections to the file system.
> No further action is required.
yeah cause it's dead

[2024-05-22 19:48] Matti: thanks

[2024-05-22 21:57] diversenok: What do you even with NTFS so it breaks so much?

[2024-05-22 21:58] diversenok: I did once manage to get `STATUS_FILE_CORRUPT_ERROR` with FileTest but it wasn't that bad

[2024-05-22 21:59] contificate: runs `mkfs.ext4` on it

[2024-05-22 22:01] diversenok: [replying to Matti: "figure out which driver is the FS minifilter drive..."]
I suppose if he wants to continuously collect some AV logs he might want the AV to be at least partially functioning

[2024-05-22 22:03] Matti: yeah I figured the same

[2024-05-22 22:03] Matti: but those goals are like, conflicting

[2024-05-22 22:03] Matti: [replying to diversenok: "What do you even with NTFS so it breaks so much?"]
in this case, resized a volume

[2024-05-22 22:03] Matti: not even shrinking

[2024-05-22 22:03] Matti: extending

[2024-05-22 22:03] diversenok: Yeah, weird

[2024-05-22 22:04] diversenok: Anyway, he might as well get an error due to the file being opened exclusively

[2024-05-22 22:06] diversenok: Win32/shell functions like reporting half of the underlying error codes as "access denied"

[2024-05-22 22:07] diversenok: Need more data to tell what's exactly going on

[2024-05-22 22:07] Matti: hmmm don't quote me on this but I thought win32 actually said something else useless

[2024-05-22 22:07] Matti: like "file is in use"

[2024-05-22 22:07] Matti: but yeah access denied is still vague

[2024-05-22 22:08] diversenok: Yeah, CreateFile at least does report sharing violations correctly

[2024-05-22 22:08] diversenok: But let me find a list known NTSTATUS codes that simply convert into `ERROR_ACCESS_DENIED`

[2024-05-22 22:08] diversenok: There was at least a few dozen of those

[2024-05-22 22:10] Matti: oh for sure <:kekw:904522300257345566>

[2024-05-22 22:10] Matti: `generr.c` should have this I think

[2024-05-22 22:13] diversenok: Okay, it's not as bad as I thought, here is a list for `ERROR_ACCESS_DENIED`
```
STATUS_ACCESS_DENIED
STATUS_ALREADY_COMMITTED
STATUS_CANNOT_DELETE
STATUS_DECRYPTION_FAILED
STATUS_DELETE_PENDING
STATUS_ENCLAVE_IS_TERMINATING
STATUS_ENCRYPTION_FAILED
STATUS_FILE_DELETED
STATUS_FILE_IS_A_DIRECTORY
STATUS_FILE_RENAMED
STATUS_INVALID_LOCK_SEQUENCE
STATUS_INVALID_VIEW_SIZE
STATUS_NO_EFS
STATUS_NO_RECOVERY_POLICY
STATUS_NO_USER_KEYS
STATUS_PORT_CONNECTION_REFUSED
STATUS_PROCESS_IS_TERMINATING
STATUS_THREAD_IS_TERMINATING
STATUS_WRONG_EFS
```

[2024-05-22 22:14] Matti: rage inducing

[2024-05-22 22:14] Matti: info in, garbage out

[2024-05-22 22:16] Matti: all 3 of the STATUS_FILE_XXX ones are surely super common

[2024-05-22 22:16] Matti: how do they not have their own win32 codes

[2024-05-22 22:16] Matti: as well as delete pending

[2024-05-22 22:19] diversenok: Here is a similar list for `ERROR_INVALID_PARAMETER`, which is probably the least descriptive error of all:
```
STATUS_BAD_MASTER_BOOT_RECORD
STATUS_BAD_WORKING_SET_LIMIT
STATUS_DEVICE_CONFIGURATION_ERROR
STATUS_FAIL_CHECK
STATUS_INCOMPATIBLE_FILE_MAP
STATUS_INVALID_CID
STATUS_INVALID_INFO_CLASS
STATUS_INVALID_MESSAGE
STATUS_INVALID_PAGE_PROTECTION
STATUS_INVALID_PARAMETER
STATUS_INVALID_PARAMETER_MIX
STATUS_INVALID_THREAD
STATUS_INVALID_WEIGHT
STATUS_LPC_INVALID_CONNECTION_USAGE
STATUS_LPC_RECEIVE_BUFFER_EXPECTED
STATUS_PORT_ALREADY_SET
STATUS_SECTION_NOT_IMAGE
STATUS_SECTION_PROTECTION
STATUS_THREAD_ALREADY_IN_SESSION
STATUS_THREAD_NOT_IN_SESSION
STATUS_THREAD_NOT_RUNNING
STATUS_UNABLE_TO_DELETE_SECTION
STATUS_UNABLE_TO_FREE_VM
```

[2024-05-22 22:20] diversenok: So yeah, motivates to use Native API and don't touch Win32

[2024-05-22 22:22] Matti: yeah, although to be fair STATUS_INVALID_PARAMETER is also by far the most common NTSTATUS code

[2024-05-22 22:22] Matti: it's only a hair better than STATUS_UNSUCCESSFUL

[2024-05-22 22:22] diversenok: Yeah, true

[2024-05-22 22:22] Matti: I like the STATUS_INVALID_PARAMETER_1..9 ones, they should use those more often

[2024-05-22 22:24] diversenok: They sound great at first, but there is a conceptual problem with them

[2024-05-22 22:24] diversenok: You cannot really forward them upstream because odds are the parameter order is different

[2024-05-22 22:24] Matti: that's true

[2024-05-22 22:25] diversenok: I definitely encountered some API returning something like `STATUS_INVALID_PARAMETER_5` from a function that only has three or so

[2024-05-22 22:25] Matti: but the lowest level APIs tend to either return a boolean or even throw exceptions (Ke), or else the parameter order is very similar for the kernel/internal kernel APIs and the syscalls (Mm/Ps)

[2024-05-22 22:25] Matti: [replying to diversenok: "I definitely encountered some API returning someth..."]
yeah that's bad lol

[2024-05-22 22:26] diversenok: So then you boot IDA to see what is the underlying call, lol

[2024-05-22 22:27] Matti: a better programming language would help a lot here

[2024-05-22 22:29] diversenok: I remember I read somewhere that HRESULTs were initially supposed to track the source of the error and give much more context (that's why the type starts with H, for handle) but were redesigned before release into basic numbers with bit structure for facility/code/etc

[2024-05-22 22:30] diversenok: Ah, there it is: https://devblogs.microsoft.com/oldnewthing/20180117-00/?p=97815
[Embed: Why does HRESULT begin with H when it's not a handle to anything? -...]
Well, it used to be a handle.

[2024-05-22 22:32] diversenok: Looking more through the list of NTSTATUS-to-Win32 error conversions... Somebody decided that `STATUS_IMAGE_ALREADY_LOADED`, `STATUS_IMAGE_ALREADY_LOADED_AS_DLL`, and `STATUS_REDIRECTOR_STARTED` all should translate into `ERROR_SERVICE_ALREADY_RUNNING`... like wtf

[2024-05-22 22:36] Matti: [replying to diversenok: "Ah, there it is: https://devblogs.microsoft.com/ol..."]
interesting

[2024-05-22 22:36] Matti: I always thought the H stood for Hate

[2024-05-22 22:39] diversenok: HRESULT for hardly-a-result

[2024-05-22 22:39] Matti: ha-haa

[2024-05-22 22:41] diversenok: My favorite NTSTATUS is probably `STATUS_TOO_LATE` because it sounds so desperate

[2024-05-22 22:42] diversenok: Then you look up its description which reads "*A write operation was attempted to a volume after it was dismounted.*"

[2024-05-22 22:43] diversenok: And, well, I guess the name makes sense 😂

[2024-05-22 23:37] JustMagic: [replying to diversenok: "My favorite NTSTATUS is probably `STATUS_TOO_LATE`..."]
I've actually seen this one (somewhat common)

[2024-05-22 23:37] JustMagic: Happens with pretty much any registry operation during shutdown

[2024-05-23 02:22] Torph: [replying to Matti: "I always thought the H stood for Hate"]
<:kekw:904522300257345566>

[2024-05-23 19:13] Horsie: [replying to Matti: "figure out which driver is the FS minifilter drive..."]
Thanks! I'll try these out and report back on how it goes

[2024-05-24 23:11] ᲼᲼: any way i can get the physical address of the base of the system PTE region from user-mode?

[2024-05-25 08:01] Timmy: [replying to ᲼᲼: "any way i can get the physical address of the base..."]
only with vuln drivers I think

[2024-05-25 08:36] ᲼᲼: [replying to Timmy: "only with vuln drivers I think"]
thought so too, thanks