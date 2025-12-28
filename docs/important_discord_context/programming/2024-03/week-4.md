# March 2024 - Week 4
# Channel: #programming
# Messages: 201

[2024-03-19 16:20] Horsie: How would you structure very large CPU state traces for producing/consuming in analysis tooling

[2024-03-19 16:21] Horsie: I want to store certain cpu state like (changed?) registers at runtime with very little overhead to a file. (From an emulator)

[2024-03-19 16:22] Horsie: But hopefully looking for something thats smart that can write to disk in chunks, etc so my memory doesnt explode

[2024-03-19 16:22] vendor: encode as diffs with full states every X frames. the large X the smaller the trace but more walking back and forwards you have to do in post-trace analysis tools.

[2024-03-19 16:22] vendor: there are a few papers on building time travel debuggers that talk about this

[2024-03-19 16:23] Horsie: [replying to vendor: "there are a few papers on building time travel deb..."]
I'm essentially planning on implementing some simple ttd stuff on top of qemu, yeah

[2024-03-19 16:23] vendor: sometimes you can do partial states not full states as well. also split stuff like register and memory

[2024-03-19 16:23] Horsie: [replying to vendor: "encode as diffs with full states every X frames. t..."]
I think this will probably end up being more expensive in the long term

[2024-03-19 16:24] Horsie: I dont mind compromising on storage size if it means less processing overall

[2024-03-19 16:24] vendor: i guess you can compute and cache those partial states in the analysis tool rather than when recording

[2024-03-19 16:24] vendor: that way you don't end up with lots of partial states for an area of the trace you don't spend much time looking at

[2024-03-19 16:25] Horsie: [replying to vendor: "i guess you can compute and cache those partial st..."]
Yeah. I was planning on something like that for stack operations and such

[2024-03-19 16:26] Horsie: Though I'm still trying to figure out if bringing in data serialization library will do me any good or if I should just write the structures encoded as raw bytes to a file

[2024-03-19 16:26] 25d6cfba-b039-4274-8472-2d2527cb: gzip it. /thread

[2024-03-19 16:26] 25d6cfba-b039-4274-8472-2d2527cb: sorry I'll leave from derailing..

[2024-03-19 16:26] Horsie: 😔

[2024-03-19 16:38] vendor: [replying to Horsie: "Though I'm still trying to figure out if bringing ..."]
yes actually i think it would. protobuf optional fields would encode how you want i think

[2024-03-19 17:55] mibho: <@491503554528542723> have u looked at .trace64 format

[2024-03-19 17:56] mibho: it's genius but so fucking confusing

[2024-03-19 17:57] 0xatul: ai <@148095953742725120> can I DM you?

[2024-03-19 17:57] mibho: it's absolute + relative indexing and records RLE-compressed delta vals?

[2024-03-19 17:58] mibho: if you want to recover full context you have to manually 'recalculate' states for whichever part you want exact val for

[2024-03-19 18:52] Matti: [replying to 0xatul: "ai <@148095953742725120> can I DM you?"]
why not just DM me instead of asking

[2024-03-19 20:27] daax: [replying to Horsie: "I want to store certain cpu state like (changed?) ..."]
delta encoding as vendor mentioned + buffering system and do async file writes to disk + custom bin format w/ metadata (similar to fs indexing)* to map to spot in trace file (cpu #, thread #, timestamp, etc) & some type of compression if you dont want it to eat up loads of disk space (depending on target)

[2024-03-19 20:27] brymko: can i dm you <@148095953742725120>

[2024-03-19 20:27] daax: lz4 is pretty good

[2024-03-19 20:37] vendor: forgot to mention but there is https://github.com/gaasedelen/tenet

[2024-03-19 20:38] vendor: not sure what format the traces are ingested in,  i think it's some inefficient format

[2024-03-19 20:38] vendor: but might be useful to reuse the coverage display/UI part of it for whatever you are building

[2024-03-19 20:39] daax: flatbuffer is great if you dont wanna deal without unpacking data first. that or capnproto

[2024-03-19 20:39] daax: <@491503554528542723>

[2024-03-19 20:39] daax: https://capnproto.org/

[2024-03-19 20:39] daax: as opposed to protobuf

[2024-03-19 20:39] vendor: yeah protobuf is a bit allocation heavy

[2024-03-19 20:39] vendor: probably not good for spamming a million of them

[2024-03-19 20:40] vendor: https://blog.ret2.io/assets/img/wtf_buffer_overflow.gif

[2024-03-19 20:40] vendor: <https://blog.ret2.io/2021/07/21/wtf-snapshot-fuzzing/>

[2024-03-19 20:40] vendor: taken from here - that's how the tenet tool looks like

[2024-03-19 20:41] vendor: can scroll through the trace and watch memory changes etc. pretty cool

[2024-03-19 20:41] vendor: i just use lighthouse as it's easier and gets the job done but this is pretty cool

[2024-03-19 20:53] daax: <@491503554528542723> for clarification, what kind of targets are you interested in? obfuscated / hardened targets or just vanilla bins that you want to fuzz unlikely to have traps for emulators?

[2024-03-19 20:59] Horsie: [replying to daax: "<@491503554528542723> for clarification, what kind..."]
I’m designing it as more of a pure RE tool where it can work on both random stuff as well as hardened.

[2024-03-19 21:00] Horsie: The idea behind this was basically wanting to make something like tetrane, putting it simply

[2024-03-19 21:01] Horsie: [replying to daax: "delta encoding as vendor mentioned + buffering sys..."]
I did indeed look at flatbuffers but I was put off with how tedious it looked with having to deal with additional dependencies with their idl and such. I’ll give it a second glance

[2024-03-19 21:02] Horsie: I have no clue about capnproto. Will take a look tomorrow as well

[2024-03-19 21:02] Horsie: [replying to vendor: "i just use lighthouse as it's easier and gets the ..."]
Yeah. I’ve used wtf+tenet a bunch of times just for REing targets. It’s super good

[2024-03-19 21:03] Timmy: [replying to daax: "https://capnproto.org/"]
this is 100% the best name and 'slogan' I've ever seen in tech, period.

[2024-03-19 21:04] Horsie: Technically the way I’m designing my tool  I’ll end up with pretty much wtf’s architecture

[2024-03-19 21:05] Timmy: I'm thinking of abusing ms' TraceLogging for my needs

[2024-03-19 21:05] Timmy: and write a custom parser for it

[2024-03-19 21:05] Timmy: like a realtime consumer

[2024-03-19 21:05] vendor: bruh

[2024-03-19 21:05] Horsie: <@519952679657668608> I might just start off with editing their bochscpu backend for now.

[2024-03-19 21:05] vendor: stuff will just get lost

[2024-03-19 21:05] Horsie: I was hoping for something more.. real-time

[2024-03-19 21:06] daax: [replying to Horsie: "I was hoping for something more.. real-time"]
you're gonna have to build it if you want real-time

[2024-03-19 21:06] Horsie: But for a decent analysis I’ll probably need to capture entire memory state anyway

[2024-03-19 21:06] daax: most things operate on snapshots/dumps

[2024-03-19 21:06] Horsie: [replying to daax: "most things operate on snapshots/dumps"]
Yeah. So far the only open source thing I’ve seen do this is wtf+tenet

[2024-03-19 21:07] Horsie: The only other more RE focus thing being now-defunct tetrane

[2024-03-19 21:07] Horsie: Their gui is what inspires me. One sec

[2024-03-19 21:08] Horsie: https://youtu.be/gKZgRPPKMSE?si=UNJ87IpKgASsNJZu
[Embed: CVE-2019-1347 analysis - From the system crash to the input file]
CVE-2019-1347: When a mouse over a file is enough to crash your system
In this video, we demonstrate how to quickly navigate from a system crash to the input that is at its origin.
It also shows the i

[2024-03-19 21:08] Horsie: Pretty cool

[2024-03-19 21:09] vendor: yeah that's crazy

[2024-03-19 21:14] vendor: seems like they capture state per basic block

[2024-03-19 21:14] vendor: and then emulate instructions in between afterwards to infer the state in the gaps

[2024-03-19 21:16] Horsie: [replying to vendor: "and then emulate instructions in between afterward..."]
That’s a pretty good idea. I didn’t know that

[2024-03-19 21:17] Horsie: Anyway, third conversation was pretty disillusioning. Thanks @vendor @daax

[2024-03-19 21:18] Horsie: I’ve got a clearer idea of what I want to accomplish now

[2024-03-19 21:18] vendor: just guessing based on this view
[Attachments: image.png]

[2024-03-19 21:18] vendor: "transition" numbers seem to be roughly every basic block

[2024-03-19 21:18] Horsie: [replying to vendor: "just guessing based on this view"]
It’s a fair assumption

[2024-03-19 21:19] Horsie: Though that will be very expensive for memory/taint analysis, I think?

[2024-03-19 21:19] Horsie: Since you need to complete emulation before you know where the accesses are

[2024-03-19 21:23] dullard: [replying to Horsie: "https://youtu.be/gKZgRPPKMSE?si=UNJ87IpKgASsNJZu"]
thats a new way to call windbg <:KEKW:846712430079770625> 
he calls it widbug

[2024-03-19 21:24] dullard: cursed

[2024-03-20 02:57] Deleted User: [replying to Timmy: "I'm thinking of abusing ms' TraceLogging for my ne..."]
I use it for my own purposes, DbgPrints can be consumed as events and I just send a custom string format with all my data to it from km and parse it in um

[2024-03-20 02:58] Deleted User: And just directly call the etw func instead of dbgprint

[2024-03-20 03:01] Timmy: <a:smart2:849600502253355018>

[2024-03-20 03:07] 0xatul: [replying to Deleted User: "I use it for my own purposes, DbgPrints can be con..."]
smort

[2024-03-20 09:16] Matti: [replying to prick: "the examples online on how to do this is all prett..."]
I don't really think that's true
(1) your DOS symlink should be in `\DosDevices\`, not `\??\` (not entirely sure this will matter, but in the best case you are still creating an extra layer of indirection)
(2) Your device object should almost definitely be under `\FileSystem\`, not `\Device\` - though idk if this is mandatory for FS drivers or just a convention
(3) similar to (2) - all MS samples make their FltMgr communication ports in `\`, not in `\Device\`. Does it matter? IDK
(4) none of the MS samples make any device objects to begin with - I doubt this is related but it's a pretty significant difference nonetheless

compile https://github.com/microsoft/Windows-driver-samples/tree/main/filesys/miniFilter/scanner from scratch with no changes and see if that does work
[Embed: Windows-driver-samples/filesys/miniFilter/scanner at main · microso...]
This repo contains driver samples prepared for use with Microsoft Visual Studio and the Windows Driver Kit (WDK). It contains both Universal Windows Driver and desktop-only driver samples. - micros...

[2024-03-20 09:18] Matti: if you look at the different versions of FLT_REGISTRATION you'll see there were a number of changes made for vista (2x) and win 8

[2024-03-20 09:18] Matti: they are all documented as being optional but you know how docs can be wrong sometimes

[2024-03-20 09:19] Matti: the `minispy` sample is another one that also creates a communication port if you prefer that one

[2024-03-20 09:19] Matti: but `scanner` looks simplest to me

[2024-03-20 09:22] Matti: one last difference I see is your usage of FLT_PORT_CONNECT when calling `FltBuildDefaultSecurityDescriptor` (vs FLT_PORT_ALL_ACCESS), but again I doubt that is it

[2024-03-20 09:23] Matti: so, I'd just get one of those two to compile and work and simplify them until you find your error

[2024-03-20 12:45] prick: Thank you for the feedback, will try today

[2024-03-20 12:45] prick: I mean, you just got done saying docs can be wrong. I did look at the paths of stuff, found nothing that would catch my eye that they are wrong

[2024-03-20 12:46] prick: But that's why I asked here cause there are only so many examples

[2024-03-20 13:58] Matti: [replying to prick: "I mean, you just got done saying docs can be wrong..."]
what I meant by this is that as far as I can tell, the added fields in vista/windows 8 are optional, as well as the other fields in e.g. FLT_REGISTRATION you are already setting to null
so if it turns out one of these actually is required, that would be an error in the documentation

[2024-03-20 14:00] Matti: the MS samples are more complete, and they contain a (presumably working) INF for the driver, so they're a good starting point to find the issue from

[2024-03-20 14:00] prick: the docs are impressively thorough which is why it doesn't strike me that it could be incorrect at first glance

[2024-03-20 14:00] prick: i remember a few years ago the documentation was more sparse than the surface of the moon

[2024-03-20 14:01] Matti: yes, minifilters are well-documented, as in there's a lot of documentation (and presumably most of it is correct)

[2024-03-20 14:01] Matti: but quantity != quality

[2024-03-20 14:02] Matti: mistakes in MS docs are not uncommon

[2024-03-20 14:02] Matti: usually they are fairly minor, but that only makes them harder to find

[2024-03-20 19:39] rin: has anyone here tried odin before? considering picking it up

[2024-03-20 20:03] contificate: pish

[2024-03-20 20:03] contificate: author is clown

[2024-03-20 20:24] rin: [replying to contificate: "author is clown"]
many such cases.

[2024-03-20 20:24] rin: what did they do?

[2024-03-20 20:32] rin: a quick look at his twitter and yt he seems pretty normal

[2024-03-20 20:33] rin: https://clips.twitch.tv/SpicyDignifiedPassionfruitGivePLZ-NbOAkV56ZIXtT80z oh
[Embed: Tsoding - tsoding on odin]
Watch Tsoding's clip titled "tsoding on odin"

[2024-03-20 22:29] dullard: [replying to contificate: "author is clown"]
why is Tsoding a clown ?

[2024-03-20 22:30] contificate: I never called him a clown

[2024-03-20 22:30] contificate: gingerBill was a clown at on time

[2024-03-20 22:30] contificate: back when Odin was very young

[2024-03-20 22:30] contificate: his views on programming languages are very informed by his like

[2024-03-20 22:30] contificate: limited perspective

[2024-03-20 22:30] contificate: he's a nice, reasonable, guy

[2024-03-20 22:30] contificate: but just another

[2024-03-20 22:30] contificate: "mildly better C"

[2024-03-21 11:58] Swapnil: Any ideas why PPID spoofing will cause 0xC0000142(STATUS_DLL_INIT_FAILED)
I am trying to spoof the parent of my go binary to explorer.exe

```package main

import (
    "flag"
    "fmt"
    "os"
    "os/exec"
    "strings"
    "syscall"

    "golang.org/x/sys/windows"
)


func main() {
    pPPID := flag.Int("ppid", 0, "pid of the parent for spoofing")
    pSpawn := flag.String("spawn", "", "spawn process name")
    flag.Parse()

    ppid := *pPPID
    spawn := *pSpawn

    if spawn == "" {
        fmt.Println("spawn flag required")
        flag.Usage()
        os.Exit(1)
    }

    splitted := strings.Split(spawn, " ")
    var cmd *exec.Cmd
    if len(splitted) > 1 {
        cmd = exec.Command(splitted[0], splitted[1:]...)
    } else {
        cmd = exec.Command(splitted[0])
    }

    if ppid != 0 {
        h, err := windows.OpenProcess(windows.PROCESS_CREATE_PROCESS|windows.PROCESS_DUP_HANDLE|windows.PROCESS_QUERY_INFORMATION, false, uint32(ppid))
        if err != nil {
            panic(err)
        }
        cmd.SysProcAttr = &syscall.SysProcAttr{
            ParentProcess:    syscall.Handle(h),
        }
        defer windows.CloseHandle(h)
    }

    if err := cmd.Start(); err != nil {
        panic(fmt.Sprintf("error starting %s: %s", spawn, err))
    }

    fmt.Println("Spawned new process with pid", cmd.Process.Pid)

    if err := cmd.Wait(); err != nil {
        fmt.Printf("%s exit error: %s\n", splitted[0], err)
    }
}
```
This is my super simple ppid spoofing code in go 1.22.0 windows/amd64

If I try to spoof notepad's parent to explorer, it works
If I try to spoof one go binary's parent to another go binary, it works
If I try to spoof go binary's parent with explorer.exe, it doesnt work(except if the spoofer is running as service, then I can spoof it)

I am stumped, anyone got any ideas?

[2024-03-21 23:44] rin: does anyone know to draw a proper desktop after using switchdeskop. right now after calling switchdesktop it becomes the inputdesktop but the screen is just blue

[2024-03-22 01:10] Matti: wdym by 'proper'

[2024-03-22 01:10] Matti: and is this a desktop you've opened or created yourself?

[2024-03-22 01:12] Matti: the single colour background sounds like explorer just isn't the shell on that desktop, or more specifically these windows don't exist (because explorer doesn't exist on this desktop, or it's not its shell)
[Attachments: image.png]

[2024-03-22 01:13] Matti: I'm gonna guess dwm.exe is probably needed for a 'proper' desktop too, though maybe not to fix this specific thing

[2024-03-22 01:14] Matti: if you find this same window tree on your regular desktop and make the top node invisible you'll see the same effect

[2024-03-22 01:15] Matti: similar for `Shell_TrayWnd` and probably a bunch of others

[2024-03-22 01:16] Matti: well the last one will make the tray invisible obviously

[2024-03-22 01:20] rin: [replying to Matti: "wdym by 'proper'"]

[Attachments: image.png]

[2024-03-22 01:20] Matti: yeah, I read your description

[2024-03-22 01:20] Matti: the screen is blue

[2024-03-22 01:20] Matti: but what specifically do you want on it

[2024-03-22 01:20] rin: just a normal desktop

[2024-03-22 01:21] Matti: ok well explorer.exe does most of that on a regular user desktop

[2024-03-22 01:22] Matti: try `SetShellWindow()`? though it's not exactly documented...

[2024-03-22 01:22] rin: [replying to Matti: "try `SetShellWindow()`? though it's not exactly do..."]
one moment

[2024-03-22 01:23] Matti: https://stackoverflow.com/questions/2270527/how-to-code-a-new-windows-shell
[Embed: How to code a new Windows Shell?]
How would I go about coding a new Windows Vista Shell?

[2024-03-22 01:24] Matti: that's not exactly the same thing as starting explorer.exe but yeah

[2024-03-22 01:24] Matti: try just doing that first

[2024-03-22 01:24] Matti: and if it exits instantly, run it in a debugger to figure out why

[2024-03-22 01:24] Matti: because creating your own shell is a lot of work

[2024-03-22 01:27] diversenok: You can start another instance of explorer on a diffrernt desktop

[2024-03-22 01:28] Matti: yeah I thought as much

[2024-03-22 01:28] diversenok: Sysinternals Desktops tool does just that

[2024-03-22 01:29] Matti: all of the experience I have with desktops (or the shell full stop) is on the session 0 desktop which is subject to a lot of restrictions that don't apply when you do things properly

[2024-03-22 01:30] diversenok: That's quite unique, so pretty cool

[2024-03-22 01:31] rin: [replying to diversenok: "Sysinternals Desktops tool does just that"]
do you have any online references

[2024-03-22 01:31] Matti: you can just run it? you can see it start explorer.exe for each desktop

[2024-03-22 01:31] Matti: I did it just now

[2024-03-22 01:31] diversenok: I mean, run the tool, make it switch to a different desktop, observe a new instance of explorer

[2024-03-22 01:31] diversenok: Yep

[2024-03-22 01:33] rin: [replying to Matti: "I did it just now"]
it launches in the default desktop

[2024-03-22 01:33] rin: ?

[2024-03-22 01:33] Matti: no, there's a special Sysinternals Desktops desktop

[2024-03-22 01:33] rin: 
[Attachments: image.png]

[2024-03-22 01:33] Matti: you can show this in PH/SI columns per process, it's handy

[2024-03-22 01:34] Matti: [replying to Matti: "no, there's a special Sysinternals Desktops deskto..."]
for each desktop*

[2024-03-22 01:35] rin: [replying to Matti: "no, there's a special Sysinternals Desktops deskto..."]
https://learn.microsoft.com/en-us/sysinternals/downloads/desktops this?
[Embed: Desktops - Sysinternals]
This utility enables you to create up to four virtual desktops and easily switch between them.

[2024-03-22 01:35] diversenok: Yes

[2024-03-22 01:36] Matti: 
[Attachments: image.png]

[2024-03-22 01:36] Matti: 
[Attachments: image.png]

[2024-03-22 01:37] Matti: only the desktops.exe itself runs on your default desktop (it kinda has to....)

[2024-03-22 01:37] Matti: then whenever you switch to a new one, it creates a new shell process

[2024-03-22 01:39] Matti: [replying to Matti: "I'm gonna guess dwm.exe is probably needed for a '..."]
so this was wrong btw, it only exists on Default still

[2024-03-22 01:39] Matti: but again not so in S0 <:kekw:904522300257345566>

[2024-03-22 01:44] rin: why explorer.exe tho. i might just be uninformed but seems kinda random

[2024-03-22 01:44] Matti: it doesn't have to be

[2024-03-22 01:45] Matti: it's whatever the shell window is configured to be

[2024-03-22 01:45] Matti: which by default is explorer

[2024-03-22 01:45] Matti: you can set it to cmd.exe

[2024-03-22 01:45] diversenok: Because all the taskbar and icons on the desktop are drawn by explorer

[2024-03-22 01:45] diversenok: So it seems natural to run it

[2024-03-22 01:45] diversenok: You saw how it looks without it

[2024-03-22 01:45] Matti: yeah, I mean, if you want a "proper" desktop like you asked you'll want explorer

[2024-03-22 01:46] Matti: but really the reason it's launched is because it's the winlogon shell window (`HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon\Shell`)

[2024-03-22 01:46] diversenok: I wonder if the tool honors it

[2024-03-22 01:46] Matti: this is also what happens if you boot into 'safe mode with command prompt'

[2024-03-22 01:46] Matti: [replying to diversenok: "I wonder if the tool honors it"]
yeah I don't know

[2024-03-22 01:47] Matti: was wondering the same

[2024-03-22 01:47] Matti: kind of a hassle to check <:harold:704245193016344596> but I can start a VM

[2024-03-22 01:49] Matti: no it does not

[2024-03-22 01:49] Matti: heh

[2024-03-22 01:49] Matti: 
[Attachments: image.png]

[2024-03-22 01:50] Matti: it also fails to switch to any new, or the original, desktop(s) after the first switch

[2024-03-22 01:50] Matti: at least for me in vbox, with cmd.exe as shell

[2024-03-22 01:50] rin: [replying to Matti: "it also fails to switch to any new, or the origina..."]
i can still switch back

[2024-03-22 01:50] rin: also using vbox

[2024-03-22 01:50] Matti: and using which shell

[2024-03-22 01:51] rin: [replying to Matti: "at least for me in vbox, with cmd.exe as shell"]
yeah explorer

[2024-03-22 01:51] Matti: yeah

[2024-03-22 01:51] Matti: that is like, the expected behaviour

[2024-03-22 01:51] rin: lol mb

[2024-03-22 01:51] Matti: but yeah if you change the default shell it won't work as well

[2024-03-22 01:51] Matti: because explorer.exe seems to be hardcoded in it

[2024-03-22 04:11] rin: <@148095953742725120> <@503274729894051901> setting TaskbarGlomLevel to 2 in location HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced is the solution

[2024-03-22 04:12] rin: and you also need to start explorer as a normal user before anything else

[2024-03-22 04:20] Matti: uh, thanks

[2024-03-22 04:20] Matti: but the solution to what?

[2024-03-22 04:20] rin: [replying to rin: ""]
this

[2024-03-22 04:21] Matti: is that how sysinternals desktops does it...?

[2024-03-22 04:22] rin: i didnt use sysinternals desktop

[2024-03-22 04:22] Matti: no yeah, I am realising that

[2024-03-22 04:23] Matti: why not look at how it does it

[2024-03-22 11:31] diversenok: What does `TaskbarGlomLevel` give you exactly? Starting explorer as a normal user seems much more important

[2024-03-22 15:50] rin: [replying to diversenok: "What does `TaskbarGlomLevel` give you exactly? Sta..."]
It only worked with that combination

[2024-03-22 20:14] repnezz: How can we allocate RWX memory in kernel with HVCI enabled ? Have I misinterpreted the idea behind HVCI ? (vtl1 code integrity + enforcing W ^ X on kernel memory at EPT) ?