# March 2024 - Week 2
# Channel: #programming
# Messages: 299

[2024-03-06 09:51] rin: i have the following golang code, i want to create a new desktop and then print out the names of the desktops but when i go to print them out i only get the name of the window stations ```go
package main

import (
    "fmt"
    "unsafe"

    "golang.org/x/sys/windows"
)

var (
    user32DLL      = windows.NewLazyDLL("user32.dll")
    EnumDesktopsA  = user32DLL.NewProc("EnumDesktopsA")
    CreateDesktopA = user32DLL.NewProc("CreateDesktopA")
)

func EnumDesktopProc(lpStrParam *uint16, lparam uintptr) uintptr {
    desktop_name := windows.BytePtrToString((*byte)(unsafe.Pointer(lpStrParam)))
    fmt.Println(desktop_name)
    return uintptr(1)
}

func main() {
    new_desktop_name, _ := windows.UTF16PtrFromString("newdesktop")
    CreateDesktopA.Call(uintptr(unsafe.Pointer(new_desktop_name)), 0, 0, 0, 0x10000000, 0)
    EnumDesktopsA.Call(0, windows.NewCallback(EnumDesktopProc), 0)

}
```

[2024-03-06 10:00] rin: i thought i was a permission issue but then it would return an error and not the name of the window stations?

[2024-03-06 10:58] Matti: no clue why this would be printing window station names, but
1. is this a console app or a GUI app? (what I'm really asking I guess is does `GetProcessWindowStation` return null or not)
2. if it does not return null and the access mask is sufficient (I assume so), pass that instead of 0 to `EnumDesktopsA`
3. (pretty sure this is an error) you're creating a UTF-16 string in go and then passing that to CreateDesktop**A**. you'll want the **W** version when using this string, or otherwise create an ANSI string

[2024-03-06 11:01] Matti: hm maybe ignore the console vs GUI distinction - reading MSDN I think both should have a window station regardless

[2024-03-06 11:02] Matti: what I do know is that since some windows 10 version you can no longer enumerate desktops in window stations other than your own process's, but idt that is relevant here since it should already be defaulting to your current process window station

[2024-03-06 11:25] x86matthew: yeah you should be passing `GetProcessWindowStation()` to EnumDesktops, not null

[2024-03-06 11:25] x86matthew: msdn does say `If this parameter is NULL, the current window station is used.` but this is wrong

[2024-03-06 11:27] x86matthew: EnumDesktops and EnumWindowStatoins both call the same function internally (InternelEnumObjects), if the window station param is null then it enumerates all window stations, if it's not null then it enumerates the desktops within the specified window station

[2024-03-06 14:18] rin: [replying to x86matthew: "EnumDesktops and EnumWindowStatoins both call the ..."]
Ill try this out

[2024-03-06 14:20] rin: [replying to Matti: "no clue why this would be printing window station ..."]
All go types need to get cast to uintptr(unsafe.pointer()) so i dont think its an error, although i should implement  error handling since im having trouble

[2024-03-06 14:21] rin: [replying to rin: "All go types need to get cast to uintptr(unsafe.po..."]
(When used to call c function)

[2024-03-06 14:27] x86matthew: [replying to rin: "All go types need to get cast to uintptr(unsafe.po..."]
yes but it looks like you're creating the string itself as utf16 above (`UTF16PtrFromString`)

[2024-03-06 14:27] x86matthew: i've never used go though so i could be missing something

[2024-03-06 14:30] rin: [replying to x86matthew: "yes but it looks like you're creating the string i..."]
Above utf16ptrfromstring? What line are you looking at

[2024-03-06 14:31] x86matthew: `UTF16PtrFromString`

[2024-03-06 14:31] x86matthew: i'm referring to above "uintptr(unsafe.pointer())"

[2024-03-06 14:32] rin: Do you mean im creating the pointer then recasting to uintptr in the function call?

[2024-03-06 14:34] x86matthew: yes, i don't know anything about go but i'd assume that just casts the pointer, the string itself will still be stored as UTF16

[2024-03-06 14:35] rin: [replying to x86matthew: "yes, i don't know anything about go but i'd assume..."]
Yes

[2024-03-06 14:35] rin: I realize utf16 is prob wrong

[2024-03-06 14:35] x86matthew: yeah so CreateDesktopA won't like this as matti said, just change it to CreateDesktopW and you should be fine

[2024-03-06 14:36] x86matthew: but this is different than your original problem of course

[2024-03-06 14:38] rin: [replying to x86matthew: "EnumDesktops and EnumWindowStatoins both call the ..."]
Also this is actually super confusing, so i thought that passing the first arg as null would get default station but does not so instead it uses enumwindowstations but then enumwindowstations uses the enumdesktops callback?

[2024-03-06 14:41] x86matthew: it's just because MSDN is incorrect, it shouldn't mention that null is a valid value

[2024-03-06 14:41] x86matthew: microsoft use some of the same code internally for enumwindowstations and enumdesktops within user32.dll

[2024-03-06 14:42] x86matthew: and passing null to enumdesktops just happens to trigger some weird behaviour because of this

[2024-03-06 14:42] rin: Ok

[2024-03-06 14:43] rin: Ill try the changes before asking more questions, thank you for your insight

[2024-03-06 17:26] rin: <@943099229126144030> <@148095953742725120> fixed code,```go
package main

import (
    "fmt"
    "unsafe"

    "golang.org/x/sys/windows"
)

var (
    user32DLL               = windows.NewLazyDLL("user32.dll")
    EnumDesktopsW           = user32DLL.NewProc("EnumDesktopsW")
    CreateDesktopW          = user32DLL.NewProc("CreateDesktopW")
    GetProcessWindowStation = user32DLL.NewProc("GetProcessWindowStation")
)

func EnumDesktopProc(lpStrParam *uint16, lparam uintptr) uintptr {
    desktop_name := windows.UTF16PtrToString(lpStrParam)
    fmt.Println(desktop_name)
    return uintptr(1)
}

func main() {
    new_desktop_name, _ := windows.UTF16PtrFromString("newdesktop")
    CreateDesktopW.Call(uintptr(unsafe.Pointer(new_desktop_name)), 0, 0, 0, 0x10000000, 0)
    windowsStation, _, _ := GetProcessWindowStation.Call()
    EnumDesktopsW.Call(windowsStation, windows.NewCallback(EnumDesktopProc), 0)

}
```

[2024-03-06 18:29] Matti: cool, so it's working now?

[2024-03-06 18:31] Matti: just a small note: CreateDesktopW will fail if GetProcessWindowStation returns null for whatever reason (I don't know when that would be precisely, maybe if running as a service)

[2024-03-06 18:32] Matti: so I would change the order of those two calls and make the Create/Enum calls dependent on the window station being valid

[2024-03-06 18:35] Matti: [replying to x86matthew: "EnumDesktops and EnumWindowStatoins both call the ..."]
this is a pretty funny case of bad docs not only leading to non-working code, but literally invalid results

[2024-03-06 18:36] rin: [replying to Matti: "cool, so it's working now?"]
yes

[2024-03-06 18:37] Matti: alright cool

[2024-03-06 18:37] rin: [replying to Matti: "this is a pretty funny case of bad docs not only l..."]
yeah i was very confused when i first ran it

[2024-03-06 18:37] Matti: services do have a window station btw, I just checked

[2024-03-06 18:37] Matti: it would be very unusual for a process not to have one I expect

[2024-03-06 18:38] Matti: (I've seen it but I was doing funny things from kernel that you're not supposed to do)

[2024-03-06 18:38] Matti: smss doesn't have one either

[2024-03-06 18:39] rin: [replying to Matti: "services do have a window station btw, I just chec..."]
is the point of services having their own windows station being they should not take user input? I saw a security warning about that on the docs

[2024-03-06 18:41] Matti: yeah, it's security related

[2024-03-06 18:41] Matti: 
[Attachments: image.png]

[2024-03-06 18:41] Matti: but sessions also run in session 0, as opposed to 1, 2... for users

[2024-03-06 18:41] Matti: and session 0 is isolated

[2024-03-06 18:42] diversenok: Processes with the mitigation that disables win32k syscalls don't have a window station, obviously

[2024-03-06 18:42] Matti: I'm not sure of the exact difference between the two separations

[2024-03-06 18:42] Matti: [replying to diversenok: "Processes with the mitigation that disables win32k..."]
this makes sense

[2024-03-06 18:42] Matti: that would be 0 processes on my machine

[2024-03-06 18:43] diversenok: Process in the Chrome Renderer sandbox are the typical candidates

[2024-03-06 18:43] Matti: [replying to Matti: ""]
^ that is session 0 (services session), this is session 1
the Service-xxx session is for firefox.exe
[Attachments: image.png]

[2024-03-06 18:44] x86matthew: [replying to Matti: "this is a pretty funny case of bad docs not only l..."]
yeah, this one is so bad that i submitted my first revision proposal to msdn this afternoon

[2024-03-06 18:45] diversenok: [replying to rin: "is the point of services having their own windows ..."]
Probably for COM apartment support since it needs to pump window messages + other compatibility reasons

[2024-03-06 18:53] Matti: [replying to diversenok: "Process in the Chrome Renderer sandbox are the typ..."]
I just checked this using chromium and it's true

[2024-03-06 18:53] Matti: kind of funny that this is enabled for chromium but not for most services

[2024-03-06 18:54] Matti: those do already normally run in a different session though as noted above - maybe that makes it redundant?

[2024-03-06 18:55] Matti: you can still get a session 0 desktop with lots of effort if you really want to, but it's not trivial

[2024-03-06 19:03] diversenok: [replying to Matti: "you can still get a session 0 desktop with lots of..."]
You mean get it from a service? Sure, but each account (System, Local Service, Network Service) has a different window station (with a name derived from the logon session id) and thus a different desktop

[2024-03-06 19:04] diversenok: And if you want to do it cross-session, it won't really work

[2024-03-06 19:05] diversenok: Regardless of your privileges, you cannot open a window station or a desktop from another session for any meaningful access because win32k's open procedure explicitly blocks it

[2024-03-06 19:05] Matti: yeah I'm aware it's not very useful

[2024-03-06 19:05] Matti: but, it does allow you to enumerate session 0 window stations

[2024-03-06 19:05] Matti: or the desktops on them rather

[2024-03-06 19:05] diversenok: Yeah, fair

[2024-03-06 19:06] diversenok: No, not desktops, actually

[2024-03-06 19:07] diversenok: WinObjEx64 had a bug that it was opening window stations from the current session instead of the specified

[2024-03-06 19:08] diversenok: `WindowStationOpenProcedure` blocks all type-specific access masks on cross-session access

[2024-03-06 19:08] diversenok: https://twitter.com/diversenok_zero/status/1353494907240017920
[Embed: diversenok (@diversenok_zero) on X]
It is the WindowStationOpenProcedure callback from win32kfull that is responsible for this behavior. As you can see on the screenshot, it fails requests that include any window-station-specific rights

[2024-03-06 19:11] diversenok: WinObjEx64 was (at least previously) using `OpenWindowStationW` which doesn't allow specifying the full NT namespace name, so it always opens window stations in the current session

[2024-03-06 19:12] diversenok: `NtUserOpenWindowStation` gives more options

[2024-03-06 19:12] diversenok: But there is still the open procedure's that blocks `WINSTA_ENUMDESKTOPS`

[2024-03-06 19:13] diversenok: If I understand correctly, windows station data structures should be in the session space, which means they aren't even mapped if your process is in the wrong session

[2024-03-06 19:14] diversenok: So there is also a conceptual limitation

[2024-03-06 19:17] Matti: [replying to diversenok: "WinObjEx64 had a bug that it was opening window st..."]
yeah, but what I'm saying is to run winobjex64 from session 0

[2024-03-06 19:17] Matti: 
[Attachments: image.png]

[2024-03-06 19:17] diversenok: Ohh, okay

[2024-03-06 19:17] Matti: no desktops on the other ones though

[2024-03-06 19:17] Matti: not sure if they don't exist or if I can't query them

[2024-03-06 19:19] diversenok: They should exist

[2024-03-06 19:20] diversenok: What's the point of a window station if it has no desktops 🙂

[2024-03-06 19:20] diversenok: You can check open handles for processes on different window stations
[Attachments: image.png]

[2024-03-06 19:20] Matti: well that is a good question

[2024-03-06 19:23] Matti: interestingly, PH seems to be able to tell me that WmiPrvSE.exe (which is in S0) is using the default desktop on of the service window stations
[Attachments: image.png]

[2024-03-06 19:24] Matti: this PH is running in S1 itself

[2024-03-06 19:25] Matti: 
[Attachments: image.png]

[2024-03-06 19:27] Matti: there are a few other S0 processes whose desktop I can see, but they are all on WinSta0\Default

[2024-03-06 19:29] diversenok: Wait, I think the column might just read the string from PEB, no?

[2024-03-06 19:29] Matti: ahhh

[2024-03-06 19:29] Matti: that's likely

[2024-03-06 19:29] Matti: and would explain it, yeah

[2024-03-06 19:29] diversenok: Handle inspection should be more reliable

[2024-03-06 19:30] Matti: true that does also work for all N=1 svchost.exes I tried at random just now

[2024-03-06 19:30] Matti: bit of a pain in the ass though

[2024-03-06 19:31] Matti: [replying to diversenok: "But there is still the open procedure's that block..."]
they should just fix this

[2024-03-06 19:32] diversenok: [replying to diversenok: "If I understand correctly, windows station data st..."]
^ might not be easily fixable due to conceptual reasons

[2024-03-06 19:33] Matti: well it used to be not fucked

[2024-03-06 19:33] Matti: and now it is

[2024-03-06 19:33] Matti: so it should be possible to un-fuck

[2024-03-06 19:34] Matti: MS are planning to get rid of session spaces entirely though fortunately, from what I've heard anyway

[2024-03-06 19:38] diversenok: Yeah, I don't like when they block operations in kernel callbacks

[2024-03-06 19:39] Matti: the object open procedure that does the blocking, is it in ntoskrnl or in win32k?

[2024-03-06 19:40] diversenok: win32kfull.sys

[2024-03-06 19:40] Matti: hmmmm ok

[2024-03-06 19:40] Matti: [replying to Matti: ""]
I guess I should be able to patch it out in the same way I already patch lots of things in win32k to even get this ^ to work in the first place

[2024-03-06 19:42] diversenok: Oh yeah, which OS version is it? I remember they removed UI0Detect after 7 or so

[2024-03-06 19:42] Matti: this is win 11 23H3

[2024-03-06 19:42] Matti: but yeah they removed it in windows 10 RTM

[2024-03-06 19:42] Matti: and getting it back has become more and more troublesome over time

[2024-03-06 19:43] diversenok: It's cool that you managed to make it work in the first place

[2024-03-06 19:43] Matti: heh

[2024-03-06 19:43] Matti: this was only out of necessity

[2024-03-06 19:44] Matti: because the product I used for this no longer worked (which I of course only found out about like 3 years after win 10 was released, being a die hard win 10 hater)

[2024-03-06 19:44] Matti: specifically there was no working mouse or kb input

[2024-03-06 19:44] Matti: so you were just stuck in S0

[2024-03-06 19:44] Matti: so I RE'd win32k to look at what MS did and sent the company the 2 byte patch to fix this

[2024-03-06 19:45] Matti: then they hired me to write a driver to do this at runtime

[2024-03-06 19:45] Matti: it's got quite a lot more patches now....

[2024-03-06 19:45] Matti: but MS are still happy to WHQL sign it

[2024-03-06 19:45] diversenok: Haha

[2024-03-06 19:46] diversenok: Nice

[2024-03-06 19:46] Matti: the only reason I ever (rarely) use this is to debug csrss.exe

[2024-03-06 19:46] Matti: which as you might expect doesn't go very well if it's your own session's csrss

[2024-03-06 19:47] Matti: so you can debug S1 csrss with this from S0

[2024-03-06 19:47] diversenok: Interesting

[2024-03-06 19:48] Matti: I can't remember the exact reason but simply making a session 2 doesn't work very well either

[2024-03-06 19:48] Matti: session 0 is different in that it is created by wininit instead of winlogon

[2024-03-06 19:48] Matti: and it's a lot more bare as you can see from the SS

[2024-03-06 19:48] Matti: any UI there is from this product, not MS

[2024-03-06 19:49] Matti: but yeah the title bars and such give it away

[2024-03-06 19:49] diversenok: I suppose it doesn't create the theme sections and such

[2024-03-06 19:50] Matti: yep, correct

[2024-03-06 19:50] Matti: there's no DWM

[2024-03-06 19:50] diversenok: Oh, riight, so probably no composition engine and so on

[2024-03-06 19:50] Matti: that also means there's no RIT(?) (something input thread)

[2024-03-06 19:50] Matti: as of some win 10 version

[2024-03-06 19:51] Matti: so that is now being done by the legacy path in win32k which they thankfully left in

[2024-03-06 19:51] Matti: just needs a patch here and there to re-enable it

[2024-03-06 19:53] diversenok: I remember there is still some support for GUI when DWM is down in the normal sessions

[2024-03-06 19:53] Matti: yeah

[2024-03-06 19:53] Matti: input processing is the issue specifically

[2024-03-06 19:53] Matti: for some reason this is also in DWM

[2024-03-06 19:54] diversenok: If you have a topmost window in UIAccess band it will still work if you kill DWM, but not sure about the input since it restarts pretty fast

[2024-03-06 19:54] diversenok: I want to try preventing it from restarting for a while to see

[2024-03-06 19:55] Matti: lol

[2024-03-06 19:55] Matti: I think I can tell you the answer right now

[2024-03-06 19:55] Matti: so first of all it's MIT* (Master Input Thread), not RIT

[2024-03-06 19:55] Matti: 
[Attachments: image.png]

[2024-03-06 19:55] Matti: and second, input will stop working while this thread is not executing

[2024-03-06 19:56] Matti: because I double clicked it for a stack trace and PH started downloading symbols

[2024-03-06 19:56] Matti: during which time I could not even cancel the symbol download

[2024-03-06 19:56] diversenok: Hah

[2024-03-06 19:59] Matti: so this is the S0 csrss.exe with the win32k patches (and wininit.exe too now...) applied
[Attachments: image.png]

[2024-03-06 19:59] Matti: one of these 3 is doing the job DWM's MIT normally does

[2024-03-06 20:00] Matti: in S0

[2024-03-06 20:00] Matti: it's possible they all normally also exist, just taking a different code path

[2024-03-06 20:05] Matti: SS of same from S0.... amusingly PPL gets in the way here because I don't have SeLoadDriverPrivilege, but you can see which ones are GUI threads
[Attachments: image.png]

[2024-03-06 20:08] diversenok: Yep, I can move the cursor and it even changes into resizing arrows etc. but I cannot send any input

[2024-03-06 20:08] diversenok: 
[Attachments: image.png]

[2024-03-06 20:08] Matti: yeah

[2024-03-06 20:08] Matti: I had the same

[2024-03-06 20:08] Matti: it's very nearly the same on the S0 desktop, except there you cannot even move the cursor

[2024-03-06 20:09] Matti: idk the reason for the difference

[2024-03-06 21:19] Matti: [replying to Matti: "I guess I should be able to patch it out in the sa..."]
(re: win32k object open procedures) maybe fuck that after all

[2024-03-06 21:20] Matti: literally every win32k .sys file + ntoskrnl seems to be involved in this callback clusterfuck

[2024-03-06 21:20] Matti: starting with `PsEstablishWin32Callouts`

[2024-03-06 21:20] Matti: which I obviously can't just nop out, since some of it does semi-useful stuff

[2024-03-06 21:21] Matti: this is called by win32k.sys, and from there it goes thrrough win32kfull, win32kbase and win32ksgd.sys (not necessarily in that order, idk)

[2024-03-06 23:03] dullard: [replying to Matti: "yeah, but what I'm saying is to run winobjex64 fro..."]
does `psexec -sid pathtowinobjex.exe` not spawn it under SI 0 ?

[2024-03-06 23:03] Matti: I mean, probably

[2024-03-06 23:04] Matti: I can't test this because psexec requires a bunch of services I have disabled

[2024-03-06 23:04] Matti: but how are you going to switch to S0 to see this winobjex64?

[2024-03-06 23:18] dullard: It may do some magic to show a GUI

[2024-03-06 23:18] dullard: 🤷‍♂️

[2024-03-06 23:18] Matti: feel free to try it <:kekw:904522300257345566>

[2024-03-06 23:18] dullard: I wonder if the method Jonas’ uses in his alert DLL would work to spawn this proc in the SI 1 desktop

[2024-03-06 23:18] Matti: this will work on windows 7

[2024-03-06 23:19] dullard: I will if I remember tomorrow

[2024-03-06 23:19] dullard: I’m backseat gaming you rn

[2024-03-06 23:19] dullard: All cozy in bed

[2024-03-07 06:02] diversenok: [replying to dullard: "does `psexec -sid pathtowinobjex.exe` not spawn it..."]
The -i switch is going to select the interactive session, so not 0

[2024-03-07 08:46] Matti: oh yeah, that too... shows you how much I use psexec

[2024-03-07 08:47] Matti: PH/SI 'run as' covers all the same functionality AFAIK

[2024-03-07 08:51] dullard: [replying to diversenok: "The -i switch is going to select the interactive s..."]
This is true

[2024-03-07 23:31] roddux: ```
struct foo {
    int a;
}

struct foo *bar = malloc(..);

bar->a = 1;
int q = bar->a;

fork();

if (bar->a) { …
```

if `foo.a` isn’t `volatile`, can a compiler store bar->a into a register after the assignment above, and potentially miss any changes to the variable made by the forked process?

[2024-03-07 23:35] roddux: i saw the above code pattern recently but didn’t see it compile (even with -O3) to use the  same register 🤔

[2024-03-07 23:36] dullard: How does it look in compiler explorer

[2024-03-07 23:37] roddux: compiler explorer shows that gcc and clang check the variable each time

[2024-03-07 23:38] roddux: but i’m curious if a compiler is _allowed_ to optimise that double fetch away, in this scenario

[2024-03-08 03:19] Random Visitor: forked processes operate on copy on wite - your modified memory in the child process doesn't matter

[2024-03-08 03:19] Random Visitor: it'll get it's own unique copy of that page as soon as it writes, not visible to the parent at all (and vice versa)

[2024-03-08 13:49] roddux: my mistake for using fork in my example — what about in the context of a new thread, then? assuming they have the same memory

[2024-03-08 14:40] asz: i think  youd need to have volatile int a;

[2024-03-08 14:41] asz: i like to test it with a for loop- where the shared variable is the stop condition

[2024-03-08 14:42] asz: that should optimise into keeping it in a register when not volatile

[2024-03-08 22:17] roddux: [replying to asz: "i think  youd need to have volatile int a;"]
yeah, my point is that i think _not_ having the volatile is technically a bug?

[2024-03-08 22:17] roddux: because of shared access to the memory

[2024-03-08 22:17] asz: it depends

[2024-03-08 22:18] asz: but can be yes

[2024-03-09 20:46] rin: does anyone know where i can find constants for windows api?

[2024-03-09 20:46] rin: trying to find the value of PW_CLIENTONLY

[2024-03-09 20:51] Matti: how about the windows SDK

[2024-03-09 20:51] Matti: which contains the header MSDN tells you to include for the API

[2024-03-09 20:51] Matti: but failing that, just search
[Attachments: image.png]

[2024-03-09 20:52] Matti: MSDN tells you to include something else though, so up to you <:thinknow:475800595110821888>

[2024-03-09 20:53] Matti: though ig if you literally just need the value it doesn't matter

[2024-03-09 20:54] rin: [replying to Matti: "though ig if you literally just need the value it ..."]
yeah im loading dlls within golang but that doesnt include constant definitions so i need to pass values directly

[2024-03-10 08:09] rin: after creating a desktop and setting the thread i can create new processes in that desktop, but when starting certain processes like calc.exe will force my program to switch back to the default desktop and launch calc.exe in the default desktop. is anyone familiar with this behavior?

[2024-03-10 08:43] diversenok: Calc is a UWP app now, might be that

[2024-03-10 08:44] diversenok: Also, why do you need to set the thread desktop? You usually only need to pass it in the startup info during process creation

[2024-03-10 08:53] rin: [replying to diversenok: "Also, why do you need to set the thread desktop? Y..."]
i thought in order to do operations in a desktop you need to set the thread?

[2024-03-10 08:56] diversenok: Well, for operations like enumerating and creating windows sure, because you do them on the calling thread

[2024-03-10 08:56] diversenok: But spawning processes on different desktop is a bit different

[2024-03-10 08:57] diversenok: The calling thread doesn't need to attach to the desktop because it doesn't perform any operations there - only the new process does

[2024-03-10 09:00] rin: [replying to diversenok: "But spawning processes on different desktop is a b..."]
yeah ig thats why lpdesktop is a argument, if it was limited to the thread then it would be assumed your launching a desktop on the current desktop

[2024-03-10 09:03] diversenok: Technically, even new processes initially start without a desktop; once user32.dll loads, it reads the string from process parameters in PEB (prepared during proces creation), opens the window station and desktop and attaches to them

[2024-03-10 09:05] diversenok: The creator doesn't need to open the window station/desktop or pass access checks on them since it only provides a string name

[2024-03-10 09:06] diversenok: But the new process does need to be able to open and attach to them

[2024-03-10 09:10] rin: ok so i found the problem with msedge lanching in a newdesktop, if there is existing background processes of msedge it wont launch

[2024-03-10 09:25] diversenok: Makes sense

[2024-03-10 09:36] luci4: So this is ancient: https://www.youtube.com/watch?v=H4AJFblga3M, but I wonder how it was made
[Embed: Hitman Blood Money Aimbot v0.9]
Hitman Blood Money 
custom-made aimbot v0.9

[2024-03-10 09:39] luci4: (am I allowed to ask about this? It's an offline-only game from 2006)

[2024-03-10 09:50] luci4: I assume you would have to figure out how the game registers enemy hitboxes first

[2024-03-10 10:02] 25d6cfba-b039-4274-8472-2d2527cb: I mean no different from any other game really. 

1. Find player and npc lists
2. Reverse engineer the object structures to find necessary information (health, position data, status etc.)
3. Find where you can control player aiming angles
4. Find where the game does visibility checks and use/replicate that logic
5. ???
6. Win at the game.

[2024-03-10 10:04] 25d6cfba-b039-4274-8472-2d2527cb: or alternatively use cheat engine/binary patching to turn aim assist to 9001 for an aimbot like effect (like people did for mw2 on ps3)

[2024-03-10 10:06] snowua: [replying to 25d6cfba-b039-4274-8472-2d2527cb: "or alternatively use cheat engine/binary patching ..."]
The video looks like a case of this

[2024-03-10 10:07] snowua: Actually had no idea that’s how cheats for older console games were

[2024-03-10 10:07] 25d6cfba-b039-4274-8472-2d2527cb: lets just say I said that one after I took a look at the video 😄

[2024-03-10 10:08] luci4: I'm gonna try and do it both ways, since I would probably learn more tackling the first method. Thanks!!!

[2024-03-10 10:10] snowua: <@169947647199805441> I’m guessing you’re familiar with modding for console games? Or just a random thing you know

[2024-03-10 10:10] 25d6cfba-b039-4274-8472-2d2527cb: [replying to snowua: "Actually had no idea that’s how cheats for older c..."]
at least on ps3 people with hacked ps3's used to host 'infection' lobbies where they could set some client variables for the player to modify stuff like minimap dots, aim assist strength etc. and then a player could leave the mod lobby and go to a regular match and it would persist. I was a bit younger back then so when those things were going on I was on the side of joining for free aim assist and 10 prestige.

[2024-03-10 10:12] snowua: Ah I see

[2024-03-10 10:12] snowua: Out of curiosity do you know about the development process of all those xbox jailbreak mods or whatever you call them. I always kind of wondered how the development process for those went. Obviously it’s not as straight forward as debugging an executable as you do on a pc so I’m just kind of curious how game hackers tediously debugged and reverse engineered games another system.

[2024-03-10 10:13] snowua: I would assume something like dumping the game and statically analyzing on the pc? But even then that sounds painful to debug

[2024-03-10 10:14] snowua: Maybe console cheat developers were built different back then

[2024-03-10 10:14] luci4: Back on my XBOX 360 my dad paid a guy to install some weird OS on it, which allowed pirated games

[2024-03-10 10:14] 25d6cfba-b039-4274-8472-2d2527cb: <@116738729024028673> may have some insight, I don't

[2024-03-10 10:14] snowua: Bless us with some insight 🙏

[2024-03-10 10:15] luci4: 🙏

[2024-03-10 10:15] snowua: Man I remember when I was a kid I used to run a jailbroken PSP and mod GTA liberty city stories

[2024-03-10 10:16] snowua: Take me back

[2024-03-10 10:47] luci4: lol

[2024-03-10 10:47] luci4: 
[Attachments: image.png]

[2024-03-10 11:06] iMoD1998: [replying to snowua: "Maybe console cheat developers were built differen..."]
Its basically same as pc, slap game exe in Ida and have fun

[2024-03-10 11:07] iMoD1998: Those older consoles run powerpc tho

[2024-03-10 11:07] iMoD1998: Which the most based arch

[2024-03-10 11:07] iMoD1998: Debugging can be annoying especially without development console since every time you crash your entire Xbox dies

[2024-03-10 11:08] iMoD1998: So you gotta reboot it constantly

[2024-03-10 11:13] luci4: [replying to luci4: ""]
Setting it to 10 works, but I feel like a scrub doing it this way

[2024-03-10 12:53] iMoD1998: [replying to snowua: "Man I remember when I was a kid I used to run a ja..."]
Modding the old gtas on android

[2024-03-10 12:53] iMoD1998: Shit p fun

[2024-03-10 12:56] iMoD1998: Made this funny lil cheat

[2024-03-10 12:56] iMoD1998: 
[Attachments: Screen_Recording_20231106_013539.mov]

[2024-03-10 12:57] 25d6cfba-b039-4274-8472-2d2527cb: smh back in my day we got that by smashing combos on the controller

[2024-03-10 13:09] luci4: HESOYAM

[2024-03-10 13:13] vendor: [replying to 25d6cfba-b039-4274-8472-2d2527cb: "at least on ps3 people with hacked ps3's used to h..."]
wow, i never had a console but saw this on yt and always wondered how it worked.

[2024-03-10 13:13] vendor: how was the host hacking the game? i guess hardware security on the consoles was bad back then?

[2024-03-10 13:14] 25d6cfba-b039-4274-8472-2d2527cb: am just talking out of my ass in terms of how the specific worked, but how else would such a lobby work.

[2024-03-10 13:14] 25d6cfba-b039-4274-8472-2d2527cb: and yes the host had a properly jailbroken console with hacks

[2024-03-10 13:15] vendor: i wonder how similar those hacks were to modern game hacking. e.g. what kind of rendering did they gave available and was it native code or engine vm script stuff

[2024-03-10 13:38] luci4: [replying to iMoD1998: ""]
That's sick! How long did it take to reverse it enough to make that? I'm having trouble with Blood Money since it's so large, in comparison to the challenges I'm used to, lol

[2024-03-10 13:38] luci4: Gonna try and find calls to `GetCursorPos` or smth

[2024-03-10 13:52] Azalea: which game should i start with

[2024-03-10 14:34] iMoD1998: [replying to luci4: "That's sick! How long did it take to reverse it en..."]
Not too long did some messing around with the pc version so it was pretty straightforward

[2024-03-10 14:34] iMoD1998: [replying to Azalea: "which game should i start with"]
Do older ones

[2024-03-10 14:34] iMoD1998: Usually easier to reverse

[2024-03-10 14:36] iMoD1998: [replying to vendor: "i wonder how similar those hacks were to modern ga..."]
Very similar especially with games like cod

[2024-03-10 14:37] iMoD1998: Since engine is still 80% the same

[2024-03-10 14:37] iMoD1998: The old infections were pretty smart way to use the left over chat system in the console versions

[2024-03-10 14:38] iMoD1998: You could use that in combination with the games console commands to make a little menu by being crafty with keyboards

[2024-03-10 14:38] iMoD1998: Keybinds**

[2024-03-10 14:39] iMoD1998: Lots of different ways to mod on consoles since it’s pretty difficult to have devs implement any kind of security due to how locked down consoles are in the first place

[2024-03-10 14:42] luci4: [replying to iMoD1998: "Not too long did some messing around with the pc v..."]
Ah I see. Gonna have to debug Blood Money, couldn't find what I wanted by disassembling it

[2024-03-10 14:42] iMoD1998: Yeah debugging can be useful to give you a starting point

[2024-03-10 14:43] iMoD1998: Cheat engine can be good for this stuff

[2024-03-10 14:43] iMoD1998: I’ve used it in general re too tbh

[2024-03-10 14:43] iMoD1998: Is goated

[2024-03-10 14:45] luci4: [replying to iMoD1998: "Cheat engine can be good for this stuff"]
The game has an "AutoAim" in .rdata, which I patched to give me aimbot.

 It kinda sucks though, so I wanna make my own.

[2024-03-10 15:21] iMoD1998: [replying to luci4: "The game has an "AutoAim" in .rdata, which I patch..."]
Would be good to figure out where that is read and reverse how their aimbot works

[2024-03-10 15:22] iMoD1998: That would give you how to do raycasts maybe and maybe looping through entities

[2024-03-10 15:22] iMoD1998: Could be useful to reverse regardless

[2024-03-10 15:33] luci4: [replying to iMoD1998: "Would be good to figure out where that is read and..."]
This is pretty interesting
[Attachments: image.png]

[2024-03-10 15:33] luci4: it checks a lot of things like this, this is part of a giant column

[2024-03-10 15:34] luci4: lol it's a lot of if else's

[2024-03-10 15:42] iMoD1998: Yeah seems pretty interesting

[2024-03-10 15:42] iMoD1998: Some useful stuff there about hit points

[2024-03-10 15:43] iMoD1998: This might just be reading a config or something if it’s a bunch of string compares

[2024-03-10 15:43] iMoD1998: Kinda hard to see on phone

[2024-03-10 20:32] luci4: [replying to iMoD1998: "This might just be reading a config or something i..."]
Yep, it reads this:
[Attachments: image.png]

[2024-03-10 20:32] luci4: Setting even one of them to true makes the game crash on boot

[2024-03-10 21:20] fain: [replying to 25d6cfba-b039-4274-8472-2d2527cb: "at least on ps3 people with hacked ps3's used to h..."]
lmaoo I would do those

[2024-03-10 21:20] fain: on bo2 you could load gsc scripts

[2024-03-10 21:20] fain: and people would do custom gamemodes like zombieland

[2024-03-10 21:20] fain: was fun

[2024-03-10 21:44] contificate: [replying to 25d6cfba-b039-4274-8472-2d2527cb: "at least on ps3 people with hacked ps3's used to h..."]
retail-to-retail mod menus were a big deal back then, a key part of my childhood

[2024-03-10 22:40] snowua: [replying to iMoD1998: "So you gotta reboot it constantly"]
I see haha thats kind of how I assumed it went but the crashing sounds horrific

[2024-03-10 22:41] snowua: [replying to iMoD1998: ""]
Very nice lmao

[2024-03-10 22:41] snowua: San Andreas was a banger

[2024-03-10 22:45] dullard: [replying to contificate: "retail-to-retail mod menus were a big deal back th..."]
Fond memories of mw2 lobbies

[2024-03-10 23:25] luci4: [replying to luci4: "Yep, it reads this:"]
Well the reason it crashes is because every level has that file, so I would have to change it for all the levels