# April 2025 - Week 3
# Channel: #reverse-engineering
# Messages: 247

[2025-04-14 19:02] NSA, my beloved<3: Hello. In Ghidra the options: Commit local names / Commit Params/Return only put the variables or parameters of a function into its database, so then it can be listed and searched for in the Symbol Tree, meaning that a certain variable name can only be used once in the whole binary, am I understanding this right? If so, is it possible to do a decommit?
Another question. I Auto Create Class Stuct'ed the `this` pointer of a function. Now this function is being treated as a public function of a Class named AutoClass1. I believe there are more functions part of this Class, since before each call instruction, there is a `mov ecx, same_.data_location` before the upcoming function calls. Been struggling to decide wether these are `__thiscall`s and the functions are part of the same one Class (AutoClass1 to examplify). My other guess would be `__fastcall` where the same struct is passed as the first argument to every function, but in that case, the second 4byte argument shall be put into edx. However, there is a function with multiple 4 byte parameters and all of them are on the stack, except ecx which is again, the same .data location moved into there just as with the rest of the function calls. Is there someone more experienced than me, that could confirm that a series of functions that have the mentioned `mov ecx, same_.data_location` before their call instructions, and within these functions ecx is being accessed as a struct or a class, are indeed probably part of the same 1 class along with the variables accessed via the `this` pointer?

[2025-04-14 22:49] f00d: [replying to NSA, my beloved<3: "Hello. In Ghidra the options: Commit local names /..."]
on x86_32 virtual functions use \__thiscall calling conv (ecx is this pointer and arguments on stack) on the other hand if edx is used too it's a \__fastcall calling conv

[2025-04-15 15:41] NSA, my beloved<3: [replying to f00d: "on x86_32 virtual functions use \__thiscall callin..."]
Thank you! So my assumptions are probably correct. 🙂

[2025-04-15 20:04] UJ: Anyone got a bindiff build that works on ida 9 (for windows)?

https://github.com/google/bindiff/releases (all the artifacts from actions have expired)

[2025-04-16 05:11] Lyssa: [replying to UJ: "Anyone got a bindiff build that works on ida 9 (fo..."]
there was some weird dll that works with ida 9

[2025-04-16 05:11] Lyssa: made by the chinese

[2025-04-16 05:11] Lyssa: maybe I can find it since I need bindiff too

[2025-04-16 05:12] UJ: yea there were some on github actions on cloned repos of the google bindiff that works with ida 9 but i didnt download it and the artifacts have expired.

[2025-04-16 05:13] Lyssa: [replying to UJ: "yea there were some on github actions on cloned re..."]
no those don't work either (if we are talking about the same thing)

[2025-04-16 05:13] Lyssa: those only work on that weird leaked ida 9 beta I think

[2025-04-16 05:13] Lyssa: I'm on 9.1 and they don't work

[2025-04-16 05:40] Deleted User: i have this but idk where it came from
[Attachments: image.png]

[2025-04-16 05:40] Deleted User: i can send it but if your computer randomly starts talking to some random ass chinese ip address it aint my problem

[2025-04-16 05:43] unknown: why not compile it?

[2025-04-16 05:44] Deleted User: not everyone wants to fuck with bazel or whatever they are using ig

[2025-04-16 05:45] NSA, my beloved<3: Hey. Whenever I try to specify an official Windows header file in x64dbg Struct tab, x64dbg cannot parse pragma directives: unexpected character '#'. However, the header is using Windows defined data types but I am unable to include more headers in order for them to work. I have seen Duncan parse the same file I am trying to (https://youtu.be/I_nJltUokE0?t=2147), winternl.h, but his is at a different file path, not the official one. I am assuming he manually removes pragma directives and replaces each data type? Seems like a lot of manual work for large headers, Is this the way to do it or am I missing something?

[2025-04-16 05:51] NSA, my beloved<3: I can not even make it accept a header with a single function definition. I always receive some error, I am so confused.

[2025-04-16 06:31] rin: does anyone know how to properly setup windbg to debug a service that depends on other services. I am trying to step through termsrv.dll, I create a new service and am able to attach to my renamed copy of svchost but when I press `g` to continue. it immediately terminates.

[2025-04-16 06:31] rin: 
[Attachments: image.png]

[2025-04-16 06:33] rin: I assume that rdp requires all three following services to properly work.
[Attachments: image.png]

[2025-04-16 06:34] rin: I setup the service as follows: ```
@echo off
set SERVICENAME=DEBUGGING
set BINPATH="C:\Windows\System32\termsrv.dll"

copy C:\Windows\System32\svchost.exe C:\Windows\System32\debugsvchost.exe

sc create %SERVICENAME% binPath= "%SYSTEMROOT%\system32\debugsvchost.exe -k %SERVICENAME%" type= share start= demand

reg add "HKLM\System\CurrentControlSet\Services\%SERVICENAME%\Parameters" /v ServiceDLL /t REG_EXPAND_SZ /d %BINPATH% /f

reg add "HKLM\Software\Microsoft\Windows NT\CurrentVersion\SvcHost" /v %SERVICENAME% /t REG_MULTI_SZ /d "%SERVICENAME%\0" /f

reg add HKLM\SYSTEM\CurrentControlSet\Control /v ServicesPipeTimeout /t REG_DWORD /d 0xffffffff /f```

[2025-04-16 10:51] Matti: [replying to rin: "does anyone know how to properly setup windbg to d..."]
this isn't related to the dependencies (after all you can simply pre-start them yourself if needed), but due to the fact that you're trying to debug svchost from an interactive session

[2025-04-16 10:52] Matti: sort of unrelated but to be certain I'd also set up `termsrv` to be `type= own` rather than share, so it won't end up with other service DLLs in the same process

[2025-04-16 10:53] Matti: there are some ways to debug a service... if you only need to check one specific thing for something I'd use a kernel debugger personally

[2025-04-16 10:54] Matti: otherwise you can/could log into the session 0 desktop with some trickery to do proper debugging

[2025-04-16 10:57] Matti: I haven't fixed my own driver to support the removal of session address spaces in 24H2 though (company that I contract for is planning to EOL and then open source it, but that doesn't earn me enough to motivate me currently <:kappa:697728545631371294>)
I'm not aware of any other ways to do this on windows since win 10

[2025-04-16 10:58] Matti: if you can get away with using kd just do that

[2025-04-16 10:59] Matti: there's an article on MSDN on how to use ntsd as a user mode debugger via KD, but I've tried this in the past and IMO it's worse than simply using KD directly

[2025-04-16 11:17] Matti: [replying to Matti: "sort of unrelated but to be certain I'd also set u..."]
I just realised that's what your debugsvchost.exe is supposed to accomplish presumably - I doubt that will work as expected to be honest but unsure
`type= own` is the simplest way to ensure 1 svchost = 1 service DLL (if it uses svchost)

[2025-04-16 11:18] Matti: also, don't use regedit to modify service parameters unless you're willing to reboot afterwards

[2025-04-16 11:18] Matti: sc.exe keeps its own internal db and `sc config` commands will update it but it absolutely does not give a fuck about what's in the registry after initial startup

[2025-04-16 11:19] diversenok: You probably meant services.exe keeps its own internal db

[2025-04-16 11:19] Matti: that's what I said

[2025-04-16 11:19] Matti: jeeeez

[2025-04-16 11:20] Matti: it's just a very elaborate typo

[2025-04-16 11:23] diversenok: I wonder if there is some command you can issue to it to reload the database from the registry

[2025-04-16 11:23] diversenok: Probably not, but would be useful in some cases

[2025-04-16 11:23] Matti: yeah you would think so right

[2025-04-16 11:23] Matti: but I'm pretty sure there isn't one

[2025-04-16 11:23] Matti: I hate this design

[2025-04-16 11:24] Matti: I mean why would the command even need to exist technically - it could simply subscribe to registry notifications under HKLM\SYSTEM\CCSet\services

[2025-04-16 11:25] Matti: PH does this and it works well

[2025-04-16 11:25] diversenok: Yeah

[2025-04-16 11:28] diversenok: Hmmm actually...

[2025-04-16 11:28] diversenok: `svcctl_ServerRoutineTable` has a function called `RI_ScReparseServiceDatabase`

[2025-04-16 11:31] Matti: yea

[2025-04-16 11:31] Matti: I was thinking along the same lines it seems <:lillullmoa:475778601141403648>

[2025-04-16 11:32] Matti: should be simple enough to make a hack utility to invoke via rpc, but would it have killed them to include it in sc

[2025-04-16 11:33] Matti: time to make my own sc

[2025-04-16 11:33] Matti: with blackjack, and hookers

[2025-04-16 11:34] Matti: [replying to Matti: "I mean why would the command even need to exist te..."]
this would still have my preference but unfortunately windows doesn't like it if you replace files like services.exe

[2025-04-16 11:35] Matti: maybe IFEO + Debugger could work but that is fairly gross as well

[2025-04-16 11:35] Matti: plus lots of shit probably expects it to be named services.exe

[2025-04-16 11:37] diversenok: IFEO Debugger is a Win32-level invention inside `CreateProcess`

[2025-04-16 11:37] diversenok: Is services.exe started via CreateProcess? 🤔

[2025-04-16 11:38] diversenok: wininit.exe does have kernel32 loaded

[2025-04-16 11:38] Matti: are you sure? isn't there a flag to ignore IFEO in NtCreateUserProcess?

[2025-04-16 11:38] diversenok: There is indeed, but I have no idea what it does

[2025-04-16 11:39] diversenok: Let me check with procmon where the registry read comes from

[2025-04-16 11:39] Matti: ok great
[Attachments: image.png]

[2025-04-16 11:40] Matti: is this an answer to the question or just a bug in my 20K lines of shit code invoking NtCreateUserProcess

[2025-04-16 11:40] Matti: I have a hunch... and I don't like it

[2025-04-16 11:41] Matti: just "taskmgr.exe" gives me a different path not found

[2025-04-16 11:42] diversenok: Missing `\??\` maybe?

[2025-04-16 11:42] Matti: I think I wrote it to be DOS path compatible

[2025-04-16 11:42] Matti: but yeah

[2025-04-16 11:42] Matti: or just cd to system32

[2025-04-16 11:42] Matti: who fucking knows

[2025-04-16 11:46] diversenok: Looks like I was wrong, the query does originate inside `NtCreateUserProcess`

[2025-04-16 11:47] diversenok: And I get the same error 🙃

[2025-04-16 11:47] Matti: wait

[2025-04-16 11:47] Matti: why didn't I simply fucking look this up in my own kernel

[2025-04-16 11:47] Matti: I've implemented the syscall

[2025-04-16 11:47] Matti: I guess we are both certifiably retarded

[2025-04-16 11:50] Matti: I haven't got kd attached here.... wonder what it's trying but failing to look up

[2025-04-16 11:50] Matti: oh yeah procmon could work

[2025-04-16 11:51] diversenok: Ah, okay. So `NtCreateUserProcess` does check for the IFEO Debugger key and then returns this error, indicating `PsCreateFailExeName` fail reason and giving back the IFEO key handle
[Attachments: image.png]

[2025-04-16 11:51] Matti: yep, I get the same

[2025-04-16 11:51] Matti: ❓ ❓ ❓

[2025-04-16 11:52] diversenok: ```c
// PsCreateFailExeName
struct
{
    HANDLE IFEOKey;
} ExeName;
```

[2025-04-16 11:52] Matti: uhuh, but that is expected or at least common to not exist right

[2025-04-16 11:53] diversenok: The further processing might be up to `CreateProcess` perhaps? Cannot immediately tell

[2025-04-16 11:54] Matti: [replying to diversenok: "Ah, okay. So `NtCreateUserProcess` does check for ..."]
oh you did specify the 'ignore IFEO' flag then?

[2025-04-16 11:54] Matti: I did not

[2025-04-16 11:54] diversenok: ignore IFEO launches the process as if there is no key

[2025-04-16 11:54] diversenok: Not ignore gives this error

[2025-04-16 11:55] Matti: yeah, that probably is at least consistent with the kernel code that does this

[2025-04-16 11:55] Matti: but like, I've called this API so many times and never with this flag <:lillullmoa:475778601141403648>

[2025-04-16 11:56] Matti: is it because there **is** an IFEO for taskmgr.exe? I guess so

[2025-04-16 11:56] diversenok: I always used this flag and (thus, probably) never seen `PsCreateFailExeName` reason returned

[2025-04-16 11:57] Matti: [replying to Matti: "is it because there **is** an IFEO for taskmgr.exe..."]
it does seem to be this

[2025-04-16 11:57] Matti: funny

[2025-04-16 11:57] Matti: cmd.exe without flag works fine

[2025-04-16 11:57] diversenok: IFEO key itself or a Debugger value as well?

[2025-04-16 11:58] Matti: lemme see

[2025-04-16 11:58] Matti: uhh give me something plausible to put there for taskmgr.exe?

[2025-04-16 11:59] Matti: ah `AuditLevel=0` should work... probably

[2025-04-16 12:00] diversenok: Another exe

[2025-04-16 12:00] Matti: to me Debugger has always been the sole purpose of IFEO
[Attachments: image.png]

[2025-04-16 12:01] diversenok: taskkill, lol

[2025-04-16 12:01] diversenok: Well, it also helps when playing with mitigation options

[2025-04-16 12:02] Matti: yeah but I just do that by disabling those system wide

[2025-04-16 12:02] Matti: it's not as interesting maybe but they just get in the way

[2025-04-16 12:04] Matti: see, now I've got an x64dbg.exe running but no taskmgr

[2025-04-16 12:04] Matti: with the create having returned success

[2025-04-16 12:05] Matti: so I guess it didn't like whatever the value meant that I pasted for MitigationOptions

[2025-04-16 12:06] Matti: I just can't be bothered with the 300 flags they have now

[2025-04-16 12:06] Matti: it all goes into the trash for every program

[2025-04-16 12:06] Matti: it's fair and it's fast

[2025-04-16 12:09] Matti: STATUS_ENTRYPOINT_NOT_FOUND...? ok man
[Attachments: image.png]

[2025-04-16 12:12] Matti: strangely succeeds now in the debugger

[2025-04-16 12:12] Matti: er well

[2025-04-16 12:12] diversenok: Ahh, that might be because of some comctl32.dll imports

[2025-04-16 12:12] Matti: that's relative....

[2025-04-16 12:13] Matti: the process create succeeds lol

[2025-04-16 12:13] Matti: with IFEO present, without debugger

[2025-04-16 12:13] Matti: and then it explodes because of idfk what

[2025-04-16 12:13] Matti: [replying to diversenok: "Ahh, that might be because of some comctl32.dll im..."]
yeah that is a good candidate I agree

[2025-04-16 12:13] Matti: some sxs bullshit

[2025-04-16 12:14] diversenok: No registration with CSRSS -> no registration with SxS -> no manifest-to-activation context parsing -> no DLL redirection for `comctl32.dll` -> it loads version 5 from system32 and not version 6 from WinSxS -> no newer exports

[2025-04-16 12:14] Matti: yea

[2025-04-16 12:14] Matti: lemme try with csrss registration enabled

[2025-04-16 12:14] Matti: though this code is so old, there's no chance this still works

[2025-04-16 12:16] Matti: lol it works

[2025-04-16 12:18] diversenok: `BASE_SXS_CREATEPROCESS_MSG` hasn't really changed, there are only two versions of it: for 7, 8, 8.1, 19H1, 19H2 and for the rest of Windows 10 and 11

[2025-04-16 12:19] diversenok: I should put them into phnt some day

[2025-04-16 12:19] Matti: yeah... but the offsets of some of the fields changed in some way that absolutely gave me cancer in one of the windows 10 releases

[2025-04-16 12:19] Matti: hm good point, I'm targeting NTDDI_VERSION win7

[2025-04-16 12:20] Matti: and hence using the wrong sxs message

[2025-04-16 12:20] Matti: nevertheless it succeeds

[2025-04-16 12:20] diversenok: That sounds like 19H1 and 19H2 versions that had one extra padding member

[2025-04-16 12:20] Matti: yeah that sounds exactly like the kind of thing it was

[2025-04-16 12:20] Matti: absolutely foul

[2025-04-16 12:22] Matti: 
[Attachments: image.png]

[2025-04-16 12:22] Matti: then

[2025-04-16 12:22] Matti: `PUNICODE_STRING pAssemblyName = reinterpret_cast<PUNICODE_STRING>(reinterpret_cast<ULONG_PTR>(&CreateProcessMessage->Sxs) +
    (FIELD_OFFSET(BASE_SXS_CREATEPROCESS_MSG, AssemblyName) + SxsOffsetAdjust()));`

[2025-04-16 12:22] Matti: <:yea:904521533727342632>

[2025-04-16 12:22] Matti: mind you I'm a huge windows 10 hater, 19H1 sounds like possibly the first version I may have actually used

[2025-04-16 12:23] Matti: [replying to diversenok: "`BASE_SXS_CREATEPROCESS_MSG` hasn't really changed..."]
vista and XP also differ, btw

[2025-04-16 12:24] Matti: but not vista and 7

[2025-04-16 12:24] diversenok: Wait, no, then it's the opposite: 19H1 and 19H2 are like 7; the rest of 10 and 11 are different

[2025-04-16 12:24] diversenok: [replying to Matti: "vista and XP also differ, btw"]
These I haven't checked

[2025-04-16 12:24] Matti: weirdo

[2025-04-16 12:26] Matti: it's all basic shit really
[Attachments: image.png]

[2025-04-16 12:26] Matti: hence why half the fields are named unknown here

[2025-04-16 12:26] Matti: and also that there's no way that this is correct

[2025-04-16 12:27] Matti: it does happen to work but that's different

[2025-04-16 12:28] Matti: lmao

[2025-04-16 12:28] Matti: the taskmgr.exe windows literally just now opened

[2025-04-16 12:28] Matti: took their time

[2025-04-16 12:29] Matti: also I'm instantly reminded of why I have IFEO with Debugger set up for it

[2025-04-16 12:29] Matti: how anyone can use this is beyond me

[2025-04-16 12:32] diversenok: 
[Attachments: image.png]

[2025-04-16 12:32] Matti: 😔

[2025-04-16 12:32] Matti: in a similar vein
[Attachments: BSoD_on_Windows_NT_4_Workstation.png]

[2025-04-16 12:33] Matti: BSODs provided information once upon a time

[2025-04-16 12:33] Matti: not fucking emojis that you need to disable with a registry override

[2025-04-16 12:33] diversenok: But it was so scary for non-tech customers!

[2025-04-16 12:34] Matti: that's true

[2025-04-16 12:34] Matti: the <:BSODHappy:995019112965214248> puts them at ease

[2025-04-16 12:36] Matti: the worst part is that each taskmgr.exe uses more memory than PH

[2025-04-16 12:37] diversenok: I also don't understand why some people prefer procexp over PH

[2025-04-16 12:37] Matti: yeah..  force of habit I guess

[2025-04-16 12:38] Matti: I used to be a procexp user a long time ago

[2025-04-16 12:38] Matti: until I learned about PH

[2025-04-16 12:38] diversenok: There are just so many more features in PH nowadays

[2025-04-16 12:39] Matti: yeah procexp isn't really in the same league at all anymore

[2025-04-16 12:39] Matti: which it used to be, a long time ago

[2025-04-16 12:40] Matti: I'd still be able to survive having to use procexp though at least

[2025-04-16 12:40] Matti: not so for taskmgr

[2025-04-16 12:40] diversenok: Yeah, definitely

[2025-04-16 12:41] Brit: what is this ph you talk of

[2025-04-16 12:41] Brit: all I know is system informer

[2025-04-16 12:41] Matti: sorry do you mean system invader?
[Attachments: image.png]

[2025-04-16 12:41] Matti: common mistake

[2025-04-16 12:41] Brit: ah yes

[2025-04-16 12:41] Brit: sorry

[2025-04-16 12:43] diversenok: Version 0.0.0.0 seems a bit outdated

[2025-04-16 12:43] Matti: it's understandable, they both came out around the same time and no one knows which one is the original

[2025-04-16 12:44] Matti: [replying to diversenok: "Version 0.0.0.0 seems a bit outdated"]
I think the version is supposed to be fetched from some jay-son API somewhere that I cannot be fucked with

[2025-04-16 12:44] Matti: I like it the way it is

[2025-04-16 12:45] diversenok: How are you not lazy to keep merging upstream changes

[2025-04-16 12:46] Matti: I just really fucking hate the name

[2025-04-16 12:46] Matti: most of what I do is driven by hate of something somewhere

[2025-04-16 12:46] Brit: [replying to diversenok: "How are you not lazy to keep merging upstream chan..."]
its literally a sed command pre build <:topkek:904522829616263178>

[2025-04-16 12:46] Matti: I wish

[2025-04-16 12:46] Matti: it's more work than you'd expect

[2025-04-16 12:46] Brit: unfort

[2025-04-16 12:47] Matti: eh it's boring but necessary work

[2025-04-16 12:48] Matti: I also have a lot of actual functional changes to keep in sync

[2025-04-16 12:48] Matti: mainly related to the driver and its attempts to babysit me

[2025-04-16 12:48] Matti: that gets old

[2025-04-16 12:49] diversenok: I added support for viewing details for `\Device\Afd` handles recently, that might be interesting for you

[2025-04-16 12:49] diversenok: Lot's of IOCTLs

[2025-04-16 12:50] Matti: hmm maybe

[2025-04-16 12:50] Matti: I mean it's neat, I just don't generally do networking stuff

[2025-04-16 12:50] Matti: but it's gonna be needed anyway some day

[2025-04-16 12:51] Matti: MRK lacking kdnet is more painful than anything in the world

[2025-04-16 12:52] Matti: virtualkd is workable and fast enough, but most bugs I encounter only reproduce on real HW

[2025-04-16 12:52] diversenok: I still do kernel debugging via COM ports sometimes

[2025-04-16 12:53] Matti: cause it **always** works

[2025-04-16 12:53] Matti: right?

[2025-04-16 12:53] diversenok: I just didn't figure out how to use network debugging with Windows Sandbox

[2025-04-16 12:53] Matti: I like that it's reliable, but for real debugging it's not usable <:lillullmoa:475778601141403648>

[2025-04-16 12:54] Matti: yeah kdnet can ALMOST always work when kdcom can, even in the weirded situations

[2025-04-16 12:54] Matti: the only exceptions I know of are devices with no PCIe and no PCIe (internally) NIC

[2025-04-16 12:54] Matti: which have a USB NIC instead

[2025-04-16 12:54] Matti: for that I use kdnet EEM

[2025-04-16 12:57] Matti: ```cmd
@echo off

:: EEM = Ethernet Emulation Mode
:: Extended refs:
:: (x86) https://tandasat.github.io/blog/windows/2023/03/21/setting-up-kdnet-over-usb-eem-for-bootloader-and-hyper-v-debugging.html
:: (ARM) https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/setting-up-kernel-mode-debugging-over-usb-eem-arm-kdnet

:: TLDR:
:: 1. run this, after configuring busparams such that it points to a USB controller listed as supported for network debugging by kdnet.exe
::        note 1: the port number can optionally be changed. MSDN gives a recommended range of 50000-50039.
::        note 2: but do *not* change the host IP during this step! 169.254.255.255 is used for all USB EEM connections.
:: 2. on the host, start WinDbg as usual for KDNET: windbg -d -k net:port=52000,key=1.3.3.7
:: 3. reboot target.

bcdedit /dbgsettings net key:1.3.3.7 hostip:169.254.255.255 port:52000 busparams:1.0.5
bcdedit /set {dbgsettings} dhcp no

:: (change {default} below to a desired "debug" boot option GUID if needed)
::
:: from Satoshi's blog for reference. USB EEM seems to work for me either with or without this:
bcdedit /set {default} loadoptions EEM
bcdedit /set {default} debug on

:: expected output of "bcdedit /dbgsettings":
::busparams               <b.d.f>
::key                     1.3.3.7
::debugtype               NET
::hostip                  169.254.255.255
::port                    52000
::dhcp                    No
echo.
bcdedit /dbgsettings
echo.
pause
```

[2025-04-16 12:58] Matti: this works perfectly for me on an AMD 4700S (cursed USB-internally-NIC) and any ARM devices

[2025-04-16 12:58] Matti: the only odd thing is that in <@260503970349318155>'s blog you can clearly see a USB3 debug cable in the photo

[2025-04-16 12:59] Matti: https://www.datapro.net/products/usb-3-0-super-speed-a-a-debugging-cable.html this one to be precise
[Embed: USB 3.0 Super-Speed A/A Debugging Cable -- DataPro]
USB 3.0 Super-Speed A/A Debugging Cable

[2025-04-16 12:59] Matti: for me, nothing worked until I switched this cable out for a regular type A to C cable from the supermarket

[2025-04-16 13:00] Matti: oh, and the busparams (`1.0.5` in this example) can be found by running `kdnet.exe` with no arguments

[2025-04-16 13:02] Matti: for comparison, there is also usb EHCI and XHCI debugging, both with their own KD transport DLLs
these definitely do require dedicated (and different depending on the type) cables

[2025-04-16 13:02] Matti: I've never gotten either of those to work on a single device

[2025-04-16 13:10] diversenok: I want to see more data points to estimate how much it would cost to debug something on the moon
[Attachments: image.png]

[2025-04-16 13:11] Matti: via USB? same as debugging something 30cm away

[2025-04-16 13:11] Matti: it'll simply not work

[2025-04-16 13:12] Matti: firewire used to be quite a popular alternative to serial once upon a time, from what I've heard

[2025-04-16 13:12] Matti: as in it was fast and actually worked

[2025-04-16 13:13] Matti: but good luck getting a firewire board these days, and also it's not exactly the 40 gbit/s you get from ethernet anymore either

[2025-04-16 13:16] Matti: the infuriating thing is MRK has native USB4 support, but no NDIS past the 5.x the WRK originally came with

[2025-04-16 13:17] Matti: too bad they can't make kdusb work

[2025-04-16 13:18] Matti: well I guess ndis is technically unrelated

[2025-04-16 13:18] Matti: it's just kinda convenient to have a NIC you can actually also use as a NIC

[2025-04-16 13:19] Matti: the issue with kdnet support is related to the kernel debugger transport interface, not ndis

[2025-04-16 13:26] diversenok: So I'm still looking at process creation... `NtCreateUserProcess` opens the IFEO key via `ZwOpenKey` and then returns the handle to user-mode

[2025-04-16 13:26] diversenok: Sounds like a funny way to open any key if you manage to redirect it

[2025-04-16 13:27] Matti: with read access only though I assume right?

[2025-04-16 13:27] diversenok: Any key that has a `Debugger` value under it 😐

[2025-04-16 13:27] Matti: lol

[2025-04-16 13:27] Matti: yea

[2025-04-16 13:27] Matti: that is funny enough by itself

[2025-04-16 13:28] diversenok: [replying to Matti: "with read access only though I assume right?"]
Yep, `KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE`

[2025-04-16 13:29] Matti: [replying to Matti: "the issue with kdnet support is related to the ker..."]
ok seriously now

[2025-04-16 13:29] Matti: I decided to take a look at the USB EEM driver out of curiosity

[2025-04-16 13:30] Matti: and with that the **only** problem is ndis
[Attachments: image.png]

[2025-04-16 13:30] Matti: like fuck off

[2025-04-16 13:30] Matti: that's probably worse

[2025-04-16 13:31] Matti: netio.sys is also new but I know that one is loadable

[2025-04-16 13:31] Matti: but replacing ndis 5 with ndis 6 is not trivial

[2025-04-16 13:31] Matti: mild understatement

[2025-04-16 18:04] Satoshi: [replying to Matti: "the only odd thing is that in <@260503970349318155..."]
fyi that, for 2 of my machines, both regular A-A USB3 and the debug cables worked fine. it did not matter for my targets

[2025-04-16 18:23] Matti: oh yeah sorry, I knew this actually

[2025-04-16 18:24] Matti: just to clarify I didn't mean that the info was incorrect, just that for reasons unknown it seems to vary per board which cable will work

[2025-04-16 18:25] Matti: I'll be able to try it on an ARM SoC soonish, I'll give the orange tube cable another chance