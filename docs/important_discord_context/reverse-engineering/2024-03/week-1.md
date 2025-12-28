# March 2024 - Week 1
# Channel: #reverse-engineering
# Messages: 45

[2024-03-01 12:50] hxm: i m looking at a driver doing these : 

```c++
__int64 __fastcall sub_140016000(__int64 a1)
{
  POOL_TYPE v2; // ecx
  char *PoolWithTag; // rax
  void *v4; // rbx
  int v5; // edi
  __int64 v6; // rax
  struct _OSVERSIONINFOW VersionInformation; // [rsp+20h] [rbp-148h] BYREF

  VersionInformation.dwOSVersionInfoSize = 276;
  if ( RtlGetVersion(&VersionInformation) >= 0
    && (VersionInformation.dwMajorVersion > 6
     || VersionInformation.dwMajorVersion == 6 && VersionInformation.dwMinorVersion >= 2) )
  {
    PoolType = 0x200;
  }
  *&DeviceObject.Type = 0i64;
  DeviceObject.DriverObject = &unk_140009538;
  DeviceObject.Timer = 1;
  DeviceObject.NextDevice = 0i64;
  DeviceObject.CurrentIrp = 0i64;
  sub_14000D478();
  DeviceObject.CurrentIrp = 0i64;
  sub_14000D82C();
  *(a1 + 104) = sub_14000D8C0;
  if ( off_14000B010 != &off_14000B010 && (HIDWORD(off_14000B010->Timer) & 1) != 0 && BYTE1(off_14000B010->Timer) >= 4u )
    sub_140001000(off_14000B010->AttachedDevice, 0xAu, &unk_1400097A8);
  v2 = PoolType;
  if ( PoolType == PagedPool )
    v2 = PagedPool;
  PoolWithTag = ExAllocatePoolWithTag(v2, 0x80ui64, 0x31504146u);
  v4 = PoolWithTag;
  if ( PoolWithTag )
  {
    *(PoolWithTag + 15) = 0i64;
    *(PoolWithTag + 29) = 0;
    PoolWithTag[112] = 0;
    *(PoolWithTag + 5) = 0i64;
    *(PoolWithTag + 34) = 0x10000;
    PoolWithTag[32] = 2;
    *(PoolWithTag + 3) = 0i64;
    *(PoolWithTag + 2) = 0i64;
    *(PoolWithTag + 1) = a1;
    *PoolWithTag = 0i64;
  }
  else
  {
    v4 = 0i64;
  }
...
```

wonder teh reason behing, any suggestions ?

[2024-03-01 13:00] rad: which bit are you asking about what?

[2024-03-01 13:01] rad: it checks if no execute pool is available, calls some functions, and then allocates a structure

[2024-03-01 13:09] asz: its  mapping a file

[2024-03-01 14:20] Horsie: So far when I was trying to reverse simple windows drivers that create a single device- I could just look at what is being set to `DriverObject->MajorFunction[14]`

[2024-03-01 14:21] Horsie: Now I was looking at this driver in vrtuletree and it seems to be making multiple device objects(?) each of which seem to be offering different functionality when interacted

[2024-03-01 14:22] Horsie: How do drivers figure out which MajorFn->IOCTLHandler to call when there are multiple devices created?

[2024-03-01 14:22] Horsie: Is it like a strcmp? I havent found one so far

[2024-03-01 14:29] asz: its layered io stack

[2024-03-01 14:45] donna🤯: [replying to Horsie: "How do drivers figure out which MajorFn->IOCTLHand..."]
if a higher level driver isnt meant to process the request, itl pass it to a lower device in the device stack

[2024-03-01 14:45] donna🤯: https://learn.microsoft.com/en-us/windows-hardware/drivers/kernel/passing-irps-down-the-driver-stack
[Embed: Passing IRPs Down the Driver Stack - Windows drivers]
Passing IRPs down the Driver Stack

[2024-03-01 14:50] Horsie: Maybe I misunderstand what they mean by ‘driver’ here. But the examples with multiple drivers here seems to be different than what I want to ask about

[2024-03-01 14:51] Horsie: Instead of dealing with multiple drivers in a stack, I mean to instead ask about a single driver that produces 2 or more devices

[2024-03-01 14:54] Horsie: My objective here is: to be able to see a breakpoint on the function that handled ioctl requests to a specific device.

[2024-03-01 14:55] Horsie: [replying to donna🤯: "if a higher level driver isnt meant to process the..."]
Does that make sense?

[2024-03-01 15:03] donna🤯: [replying to Horsie: "My objective here is: to be able to see a breakpoi..."]
then it would simply be the driverobject->majorfunction for the specific device

[2024-03-01 15:04] donna🤯: most drivers have multiple device objects (a FDO and a PDO)

[2024-03-01 15:04] donna🤯: but the IOCTL handlers are associated with the driverobject and not the device object

[2024-03-01 15:15] donna🤯: https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/nf-ntifs-ioenumeratedeviceobjectlist
[Embed: IoEnumerateDeviceObjectList function (ntifs.h) - Windows drivers]
Learn more about the IoEnumerateDeviceObjectList routine.

[2024-03-01 15:15] donna🤯: can use this to find all device objects associated with a driver object

[2024-03-01 15:30] Horsie: [replying to donna🤯: "but the IOCTL handlers are associated with the dri..."]
Thanks. Could very well be

[2024-03-01 15:31] Horsie: But I was reading a blog which implied that the two different devices created by the same driver carried out different tasks. I'll do some research into this

[2024-03-01 15:33] donna🤯: [replying to Horsie: "But I was reading a blog which implied that the tw..."]
could you link the blogpost?

[2024-03-01 15:34] donna🤯: because a driver object is essentially just an object representing a driver image

[2024-03-01 15:35] donna🤯: so creating a new device object doesnt create some new system for handling events

[2024-03-01 15:37] Horsie: I see... Thanks for helping me figure it out in dms 👍

[2024-03-01 21:58] Windy Bug: [replying to Horsie: "I see... Thanks for helping me figure it out in dm..."]
on a technical level a driver usually stores device object specific info needed to carry I/O operations in the DeviceObject.DeviceExtension (which is simply a NonpagedPool allocation for your device, you can  store there whatever …)

Since the target device object is passed to the driver’s dispatch routine you’d have access to it (and at any IRQL) to determine how to carry the I/O operation and retrieve necessary context (such as  which device sits below you in the given stack etc) 

I’m not sure what use case the blogpost was referring to so lmk if that answers anything

[2024-03-02 06:22] Horsie: [replying to Windy Bug: "on a technical level a driver usually stores devic..."]
Yep! I figured it out thanks to you guys

[2024-03-02 06:23] Horsie: I looked a bit deeper and found that the iocontrol handling function checked DeviceObj->DeviceType to call into different handling routines which then refer their respective extensions for context

[2024-03-02 08:33] Koreos: Anyone know where I can find all the values of Crypt* winapi flags?

[2024-03-02 08:33] Koreos: Like CryptAcquireContext etc

[2024-03-02 11:05] Matti: [replying to Koreos: "Anyone know where I can find all the values of Cry..."]
`um/wincrypt.h`

[2024-03-02 11:05] Matti: apparently

[2024-03-02 11:06] Matti: I just uh, kinda searched for one of the flags in the SDK include dir to find that

[2024-03-02 11:07] Matti: yeah this does seem to be the one if I had to guess
[Attachments: image.png]

[2024-03-02 11:13] Koreos: Ah nice thanks!

[2024-03-02 15:08] daax: [replying to Matti: "`um/wincrypt.h`"]
probably already known, but the crypt error messages are in winerror, not wincrypt.h - i really wish the error codes for specific apis would be their specific subsystem headers

[2024-03-03 09:19] Horsie: I'm debugging a physical debug target from another physical machine using windbg over network

[2024-03-03 09:21] Horsie: If a leave the machine idle for a minute or so, the connection randomly breaks and the remote target just freezes with attempts to reconnection failing which forces me to do a hard reset

[2024-03-03 09:22] Horsie: ```4: kd> lm
start             end                 module name
fffff802`45a00000 fffff802`46a46000   nt         (pdb symbols)          c:\symbols\ntkrnlmp.pdb\D7ABE9B23BAD553213DE9BB10F1677B81\ntkrnlmp.pdb
... Retry sending the same data packet for 64 times.
The transport connection between host kernel debugger and target Windows seems lost.
please try resync with target, recycle the host debugger, or reboot the target Windows.
... Retry sending the same data packet for 128 times.
... Retry sending the same data packet for 192 times.
... Retry sending the same data packet for 256 times.```

[2024-03-03 19:08] Mysterio: Do most loaded PEs actually listen to their section permissions (rwx) ? If you create sections like rw, rx, rw, rx, rw, rx and properly page aligned... will the PE Loader load it like that?

[2024-03-03 19:12] vendor: [replying to Mysterio: "Do most loaded PEs actually listen to their sectio..."]
yes but the alignment of sections is different on disk and when it's mapped.

[2024-03-03 19:12] vendor: 
[Attachments: image.png]

[2024-03-03 19:13] vendor: and it's not most - it's all. the pe loader will never ignore the section permissions and do something else. (apart from this edge case trick https://secret.club/2023/06/05/spoof-pe-sections.html)

[2024-03-03 19:15] Mysterio: reading that post now after some googling about PE sections 🙂