# December 2024 - Week 3
# Channel: #programming
# Messages: 107

[2024-12-09 04:59] Horsie: [replying to tifkin: ""]
This seems really cool.

[2024-12-09 06:00] Horsie: I spent some time banging my head against the script only to realize the dbghelp.dll I was providing wasnt working. Tried a different dll and it started showing interface method names properly

[2024-12-09 06:27] sync: anyone knows a linux compatible pdb parser for reading m$ pdbs

[2024-12-09 06:28] sync: i’ve tried pdbparse that just doesn’t support most of the streams and just normal dbghelp w/ wine which returns ERROR_MOD_NOT_FOUND when loading pdb

[2024-12-09 06:29] sync: didnt find much info online about that

[2024-12-09 06:54] richinseattle: [replying to sync: "anyone knows a linux compatible pdb parser for rea..."]
There are a few rust crates. I haven’t used them, I have used xori from Endgame before and they have one 
https://endgameinc.github.io/xori/pdb/index.html
[Embed: pdb - Rust]
API documentation for the Rust `pdb` crate.

[2024-12-09 07:01] sync: time to learn rust ig

[2024-12-09 08:02] JustMagic: [replying to sync: "anyone knows a linux compatible pdb parser for rea..."]
What do you want to extract from them? None of the open source ones are really up to par to MS PDB parsing. Rust crate is probably the best (although it's kinda dead with basically no maintainer), but if you want C++ there's https://github.com/MolecularMatters/raw_pdb

[2024-12-09 08:55] tifkin: [replying to Horsie: "I spent some time banging my head against the scri..."]
Yeah has to be from the debugging toolkit since Window's won't pull symbols. But even without symbols it'd still work. Just wouldn't have pretty names

[2024-12-09 08:57] tifkin: [replying to Horsie: "This seems really cool."]
It has its quirks, but best thing out there right now IMO and Forshaw actively maintains it

[2024-12-09 09:26] roddux: [replying to dullard: "It’s not default powershell in case you didn’t kno..."]
I know nothing bout powershell tbh

[2024-12-09 09:26] roddux: other than by default it's blue

[2024-12-09 09:27] Deleted User: [replying to roddux: "other than by default it's blue"]
it's really the essential part

[2024-12-09 09:31] koyz: [replying to roddux: "other than by default it's blue"]
PS 7 isn't blue anymore

[2024-12-09 09:31] roddux: did you see the bit where i said i know nothing about it lol

[2024-12-09 11:01] Horsie: [replying to tifkin: "Yeah has to be from the debugging toolkit since Wi..."]
Without symbols it had this issue where many of the methods had the same name

[2024-12-09 11:01] Horsie: But only 1 override.

[2024-12-09 11:01] Horsie: I'm still trying to figure out how many arguments the method I want to call takes.

[2024-12-09 11:02] Horsie: Rpcview and ntobjectmanager say that it takes 1 arg

[2024-12-09 11:02] Horsie: But in ida I can clearly see that it's using 3 args

[2024-12-09 11:02] Horsie: I'll share some more info once I get in my pc

[2024-12-09 11:34] mrexodia: [replying to JustMagic: "What do you want to extract from them? None of the..."]
So when are you open sourcing your parser? <:kappa:697728545631371294>

[2024-12-09 11:34] mrexodia: https://github.com/can1357/selene/tree/master/pdblib

[2024-12-09 11:34] mrexodia: There is one here as well btw

[2024-12-09 15:05] sync: [replying to JustMagic: "What do you want to extract from them? None of the..."]
Struct offsets, .data offsets, function rvas

[2024-12-09 15:05] sync: Ill try both thanks!

[2024-12-09 16:13] tifkin: [replying to Horsie: "But in ida I can clearly see that it's using 3 arg..."]
Are you looking at only the RPC server code in Ida or have you found the client code? Only ask because Ida could struggle recovering correct args/calling convention on the server side since there's no actual direct invocations of the RPC method on the server side. I.e there's usually only single xref to the RPC function, and it's a function pointer that the RPC runtime uses when calling the method

NtObjectManager (and I suspect RpcView) reads the embedded NDR bytes that describe the interface/methods/params, so in in my experience tends to be reliably accurate.

[2024-12-09 16:14] Horsie: [replying to tifkin: "Are you looking at only the RPC server code in Ida..."]
Hmm.. I was feeling pretty confident about the arg count shown in IDA. I'll be able to post screenshots in like 10min

[2024-12-09 16:16] Horsie: ```cs
        public int s_TaskSchedulerFindScheduleClose(ref NtCoreLib.Ndr.Marshal.NdrContextHandle p0)
        {
            _Marshal_Helper @__m = new _Marshal_Helper(CreateMarshalBuffer());
            @__m.WriteContextHandle(p0);
            _Unmarshal_Helper @__u = SendReceive(4, @__m);
            try
            {
                p0 = @__u.ReadContextHandle();
                return @__u.ReadInt32();
            }
            finally
            {
                @__u.Dispose();
            }
        }```

[2024-12-09 16:16] Horsie: This is what NtObjectManager recovers

[2024-12-09 16:16] Horsie: Posting the IDA code in a second

[2024-12-09 16:17] Horsie: I mean, I wouldnt be surprised if IDA is wrong here. I havent taken a serious look at the asm yet but its some really stupid code

[2024-12-09 16:18] Horsie: 
[Attachments: image.png]

[2024-12-09 16:18] Horsie: It does seem to be consuming 3 args.

[2024-12-09 16:19] Horsie: Especially since the pattern with arg1/arg3 usage is consistant with one of the other interfaces

[2024-12-09 16:20] Horsie: Makes sense?

[2024-12-09 16:21] tifkin: [replying to Horsie: "Without symbols it had this issue where many of th..."]
If you open a new PS window and run Get-RpcServer without the -DbgHelp arg it'll name each method Proc0, Proc1, Proc2, etc.  If you try and load with symbols and see duplicate methods names, then that usually means all those RPC methods behave the same and the compiler optimizations replaced multiple method with just one (usually happens if the method does something simple like returning an error unsupported status code)

[2024-12-09 16:22] Horsie: [replying to tifkin: "If you open a new PS window and run Get-RpcServer ..."]
I did think that is the case but the code is definitely, both in memory and on disk

[2024-12-09 16:22] Horsie: Same can be proven in RpcView since all the methods had different fn addresses.

[2024-12-09 16:24] diversenok: [replying to Horsie: ""]
Which binary is it?

[2024-12-09 16:25] tifkin: [replying to Horsie: "```cs
        public int s_TaskSchedulerFindSchedu..."]
Does Format-RpcServer (which displays the IDL) show the same? NtObjectManager does some trickery with the C# code when pointer params are used for output

[2024-12-09 16:26] Horsie: [replying to tifkin: "Does Format-RpcServer (which displays the IDL) sho..."]
Yeah it does now, after I got a proper dbghelp.dll

[2024-12-09 16:26] Horsie: [replying to diversenok: "Which binary is it?"]
WpTaskScheduler

[2024-12-09 16:28] Horsie: [replying to tifkin: "Does Format-RpcServer (which displays the IDL) sho..."]
Now my problem is that I wanted to see if I could make it pass 3 args

[2024-12-09 16:28] diversenok: ```c
HRESULT __thiscall s_TaskSchedulerFindScheduleClose(void *this, void *IDL_handle, void **pphFindSchedule)
```

[2024-12-09 16:29] Horsie: [replying to diversenok: "```c
HRESULT __thiscall s_TaskSchedulerFindSchedul..."]
Oh wow. Thanks!

[2024-12-09 16:29] Horsie: where is this from? private symbols?

[2024-12-09 16:31] diversenok: Yeah, some older version of this DLL with more info

[2024-12-09 16:31] Horsie: Oh nice. Is this the arm thing?

[2024-12-09 16:32] diversenok: A similar one for x86

[2024-12-09 16:33] Horsie: Damn.. You guys have all the juicy stuff 😋

[2024-12-09 18:35] diversenok: Of course; all you need is `FILE_WRITE_ATTRIBUTES` access to the file

[2024-12-09 18:36] diversenok: Nah, that's the stuff to worry about for kernel callers

[2024-12-09 18:37] diversenok: https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-setfiletime
[Embed: SetFileTime function (fileapi.h) - Win32 apps]
Sets the date and time that the specified file or directory was created, last accessed, or last modified.

[2024-12-09 20:50] tifkin: [replying to diversenok: "```c
HRESULT __thiscall s_TaskSchedulerFindSchedul..."]
Pretty sure the function works for both the class call and the RPC method. Symbols say it's inlined for the class call.

[2024-12-09 21:10] BWA RBX: [replying to Horsie: ""]
What is the hexrays decompilation of the sub-routine that calls this function? I can't find any plausible reason for hexrays to give this non-sense other than it fucking up how it handles the structure access being passed

[2024-12-09 22:29] tifkin: [replying to tifkin: "Pretty sure the function works for both the class ..."]
Wanted to double check NtObjectManager is working fine, so confirmed I can invoke it (breakpoint was triggered in windbg on that function + ETW event was emitted)
[Attachments: image.png]

[2024-12-10 03:30] Horsie: [replying to BWA RBX: "What is the hexrays decompilation of the sub-routi..."]
Well nothing really calls it (from that binary) since its just a RPC function

[2024-12-10 03:31] Horsie: But I did get my hands on a binary that does indeed call it.

[2024-12-10 03:32] Horsie: 
[Attachments: image.png]

[2024-12-10 03:32] Horsie: 
[Attachments: image.png]

[2024-12-10 03:32] Horsie: 
[Attachments: image.png]

[2024-12-10 03:33] Horsie: `g_schedule` is used to store/pass the current schedule task in the find first/next/close tasks loop that lists stuff.

[2024-12-10 15:28] BWA RBX: [replying to Horsie: "Well nothing really calls it (from that binary) si..."]
That's weird, I've never came across this before, if i was to even guess what hexrays did here and I might be completely wrong but i'd think it is treating the pointer to each member access of the structure individually.

[2024-12-11 17:42] pinefin: it provides some high-level details about the formatting, 

i would recommend this ms learn article instead
https://learn.microsoft.com/en-us/windows/win32/debug/pe-format
[Embed: PE Format - Win32 apps]
This specification describes the structure of executable (image) files and object files under the Windows family of operating systems. These files are referred to as Portable Executable (PE) and Commo

[2024-12-11 17:42] pinefin: quite long but it'll have (almost) all the information you need

[2024-12-11 19:45] BWA RBX: [replying to pinefin: "it provides some high-level details about the form..."]
This is a good reference to refer back to also, but I would also suggest this guys YouTube channel it's amazing 

[AllThingsIDA](https://youtube.com/playlist?list=PLVe7_4o8VFROIjtsDfLgei-9YMiJIHiRe&si=Q-4-mRugtljUYcZ-)
[Embed: Working with IDA]
Tips and tricks on how to work with various IDA features

[2024-12-11 19:47] BWA RBX: [replying to BWA RBX: "This is a good reference to refer back to also, bu..."]
Fuck I thought this was the reverse engineering channel 😂😂 mb

[2024-12-11 19:47] pinefin: 🤣

[2024-12-13 08:10] Classy: Does anyone know how system informer is able to identify modified tokens? I disabled NT SERVICE\DiagTrack and it was able to identify it as modified. Is this system informer just tracking what I’m enabling and disabling or is there something more to it? 

I’m trying to make a program to identify modified tokens but don’t totally know how to go about it. I’m currently just flagging disabled tokens but many services have them off by default. Is system informer somehow flagging tokens that shouldn’t be disabled? Any help is appreciated! thanks in advance.
[Attachments: image.png]

[2024-12-13 11:16] Matti: [replying to Classy: "Does anyone know how system informer is able to id..."]
groups in a token can be enabled by default, just like privileges

[2024-12-13 11:17] Matti: PH isn't really 'detecting' anything so much as merely noting that this group is enabled by default in the token, but it's currently disabled, ergo it must have been modified

[2024-12-13 11:18] Matti: if you right click on the group and click reset, or enable, it will lose the modified marker again

[2024-12-13 11:21] Matti: here you can see how it works the same way with privileges - I gave this svchost.exe SeUndock, which it normally has in its privilege set, but not as default enabled
[Attachments: image.png]

[2024-12-13 11:22] Classy: [replying to Matti: "groups in a token can be enabled by default, just ..."]
Thank you 🙏

[2024-12-13 11:22] Matti: for the groups example, see <https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-token_groups_and_privileges>

[2024-12-13 11:24] Matti: ah and here's the privilege equivalent <https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-token_privileges>

[2024-12-13 11:24] Matti: so `SE_GROUP_ENABLED_BY_DEFAULT` and `SE_PRIVILEGE_ENABLED_BY_DEFAULT` respectively

[2024-12-13 11:25] Classy: THANK YOU BRO 🙏

[2024-12-14 05:16] Classy: <@281121742523465730> 

I was able to obtain each token's default state and the current state for each privilege / group. If they don't match, I flag it as modified. 

However, these processes seem to be false flagigng. 

```
Group modified in PlugPlay (PID: 444): NT SERVICE\DeviceInstall (Current: Disabled, Default: Enabled)

Group modified in PlugPlay (PID: 444): NT SERVICE\LSM (Current: Disabled, Default: Enabled)

Privilege modified in lsass.exe (PID: 904): SeCreateTokenPrivilege (Current: Enabled, Default: Disabled)

Privilege modified in dwm.exe (PID: 1308): SeIncreaseBasePriorityPrivilege (Current: Enabled, Default: Disabled)

Privilege modified in dwm.exe (PID: 1308): SeIncreaseWorkingSetPrivilege (Current: Enabled, Default: Disabled)```

[2024-12-14 05:18] Classy: That's is output from my program^ , but system informer seems to be saying the same thing. Any suggestons on how to fix it?

[2024-12-14 11:58] .: [replying to Classy: "<@281121742523465730> 

I was able to obtain each ..."]
those are not 'false flags', you're assuming a process will never modify its own tokens which is unrealistic. For example, it's needed for lsass to enable SeCreateTokenPrivilege as it allows the creation of access tokens and is critical for authentication-related functionality

[2024-12-14 12:39] diversenok: Well, yeah, lsass.exe starts with a disabled privilege and enables it later since it needs it

[2024-12-14 12:42] diversenok: PlugPlay's svchost.exe does the opposite: it hosts multiple services and receives a token with service-specific groups  enabled by default and then disables those that are not needed

[2024-12-14 15:44] Classy: [replying to .: "those are not 'false flags', you're assuming a pro..."]
Thanks man!

[2024-12-15 00:32] pinefin: so i just bought a kvm switch, im wondering how i can speed my development up even more, is there a way i can automatically deploy my driver onto my machine that im debugging?

[2024-12-15 03:23] daax: [replying to pinefin: "so i just bought a kvm switch, im wondering how i ..."]
optional post build events (if you need something signed); have a network share mapped, del /f /q \*.* and xcopy, use sc to create/start service on remote pc via unc path, use nomachine if you don’t want to keep flipping back and forth, have a windbg shortcut with the target cmd line

[2024-12-15 03:25] daax: put it all in a batch script to run after build+sign

[2024-12-15 03:31] daax: other optional thing: custom keybinds to run things / commands when remote pc is available, saves me clicks. if you dont wanna do it via shortcuts (there are some requirements wrt which key combos can be bound from shortcuts) just write your own service for it

[2024-12-15 04:04] pinefin: [replying to daax: "optional post build events (if you need something ..."]
i have something like this in my vm with a shared folder that xcopy’s

[2024-12-15 04:05] pinefin: is there a way to send a command to run a bat script post build on the target machine? 🤔

[2024-12-15 04:05] pinefin: maybe through a pipe or something

[2024-12-15 04:07] pinefin: [replying to daax: "other optional thing: custom keybinds to run thing..."]
i see

[2024-12-15 04:07] pinefin: this would be something cool, just want to automate it as much as i can

[2024-12-15 04:08] pinefin: that’s my biggest road block lol, the amount of times i can test vary on how fast i can get my pc to restart from bsod’s and such

[2024-12-15 05:27] daax: [replying to pinefin: "is there a way to send a command to run a bat scri..."]
yes

[2024-12-15 05:29] daax: winrm/wmic/psexec/powershell

[2024-12-15 05:31] daax: ssh would also work

[2024-12-15 05:33] daax: [replying to pinefin: "that’s my biggest road block lol, the amount of ti..."]
you can disable minidumps which will make reboot a bit speedier

[2024-12-15 11:33] CutDown: why not develop on a vm and take a snapshot?

[2024-12-15 14:59] pinefin: [replying to CutDown: "why not develop on a vm and take a snapshot?"]
cause i’m not enjoying the slowness of it, i have 2 beefy physical machines

[2024-12-15 15:00] pinefin: also i’m not sure how much i trust nested virtualization whereas id rather work on physical hardware

[2024-12-15 16:02] stefan: anyone have an idea of what kinda of data structure is best suited for representing a regular expression character class (basically a list of ranges), that will later be used to converted to an nfa fragment?

[2024-12-15 16:07] diversenok: So conceptually it's a set

[2024-12-15 16:08] diversenok: So any data structure for a set that can quickly check whether something is in any of the ranges or not

[2024-12-15 16:08] diversenok: Like a bit mask

[2024-12-15 17:13] stefan: [replying to diversenok: "So conceptually it's a set"]
The thing is that its compiled to a jump table anyway, so its not like id be searching for keys (just iterating over all them) and i dont think duplicates and intersections matters. I think ill stick with a linked list of tuples