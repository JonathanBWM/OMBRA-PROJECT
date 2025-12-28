# March 2024 - Week 3
# Channel: #programming
# Messages: 328

[2024-03-11 15:39] contificate: [replying to dullard: "Fond memories of mw2 lobbies"]
just so amusing, all the work that went behind it

[2024-03-11 15:40] contificate: bunch of chads in Germany, France, etc.

[2024-03-11 15:40] contificate: discovering and implementing the reset glitch hack in order to make `memcmp`s yield `0`

[2024-03-11 15:40] contificate: custom hypervisor that permits all read/writes

[2024-03-11 15:40] contificate: all so some kid can get called a slur and deranked in MW2 simultaneously

[2024-03-12 20:39] vendor: anyone know how to get instrumentation callbacks to work in processes with CET enabled? i'm doing the classic RtlCaptureContext stub -> regular c handler but then calling restore context fails. NtContinue returns STATUS_SET_CONTEXT_DENIED.

[2024-03-12 22:11] vendor: nvm got it working, just inlined the path that runs without CET and then replaced the iretq with a jmp xd

[2024-03-12 22:25] Matti: are you running in wow64 mode or native x64?

[2024-03-12 22:26] Matti: [replying to vendor: "nvm got it working, just inlined the path that run..."]
this sounds like the correct fix

[2024-03-12 22:26] vendor: native x64

[2024-03-12 22:26] Matti: just curious if your original code would have worked with `NtContinueEx(Context, 0x3)`

[2024-03-12 22:26] vendor: wow64 is a problem for another day

[2024-03-12 22:26] Matti: instrumentation callbacks are useless in wow64

[2024-03-12 22:27] Matti: I mean, more useless even than in x64

[2024-03-12 22:27] vendor: wdym by 0x3? like KCONTINUE_SMTN?

[2024-03-12 22:27] Matti: ```
#define KCONTINUE_FLAG_TEST_ALERT   0x00000001
#define KCONTINUE_FLAG_DELIVER_APC  0x00000002
```

[2024-03-12 22:27] Matti: it'd definitely need 0x2

[2024-03-12 22:27] vendor: i tried 0x2 but not 0x1

[2024-03-12 22:27] Matti: 0x1, unsure

[2024-03-12 22:27] Matti: mm

[2024-03-12 22:27] Matti: well try 0x3 then

[2024-03-12 22:29] Matti: looks like it only enters the APC delivery path if test alert is also set... but as you can see this decompilation is pretty messed up
[Attachments: image.png]

[2024-03-12 22:29] vendor: just tested - works fine on non-CET process but still breaks on CET

[2024-03-12 22:29] Matti: oh well

[2024-03-12 22:29] Matti: cool ty

[2024-03-12 22:30] vendor: ```c++
    KCONTINUE_ARGUMENT arg = {};
    arg.ContinueFlags = 0x3;
    NtContinueEx(context, &arg);
```

[2024-03-12 22:30] vendor: did like this

[2024-03-12 22:30] Matti: oh yeah, I forgot it's a struct

[2024-03-12 22:30] Matti: yeah

[2024-03-12 22:30] vendor: [replying to Matti: "I mean, more useless even than in x64"]
i'm using it to do kind of like VEH based hooks but faster

[2024-03-12 22:30] Matti: that's what I meant I guess

[2024-03-12 22:32] Matti: in wow64 you should be able to do the same thing but using a `far jmp 0x23/0x33` to return to the desired segment... it's just that doing instrumentation callbacks in wow64 is pointless as the kernel will ignore them

[2024-03-12 22:32] vendor: if return RIP is KiUserExceptionDispatcher then i pull exception data and context from stack then redirect RIP and continue directly to the context so ntdll never touches the exception

[2024-03-12 22:33] vendor: but i never actually alter the original hooked page's permission back to executable

[2024-03-12 22:33] vendor: instead i disassemble + fixup + reassemble the .text section to a new page and place the hooks on that

[2024-03-12 22:33] vendor: it's in insanely stupid method

[2024-03-12 22:34] vendor: but all this for somewhat stealthy usermode hooks

[2024-03-12 22:34] Matti: mm... I have to say I wonder what it it is you're using instrumentation callbacks for that makes this worth it

[2024-03-12 22:34] Matti: logging?

[2024-03-12 22:34] Matti: in my experience they have been nearly useless, but that was when I implemented them for scyllahide

[2024-03-12 22:35] Matti: where the fact that they are invoked after the syscall is pretty much fatal

[2024-03-12 22:35] vendor: nah, entire ntdll is not executable

[2024-03-12 22:35] Matti: as well as the aforementioned lack of wow64 support

[2024-03-12 22:35] vendor: so instrumentation callback catches any attempt to execute it

[2024-03-12 22:35] vendor: i remove X from all ntdll code sections

[2024-03-12 22:36] Matti: ahh, I see

[2024-03-12 22:36] vendor: and re assemble them at a different address

[2024-03-12 22:36] Matti: that's clever

[2024-03-12 22:36] Matti: sounds very painful though lol

[2024-03-12 22:36] vendor: it took about a day to get working <:kekw:904522300257345566>

[2024-03-12 22:37] vendor: the worst part was zydis reassembles some instructions smaller

[2024-03-12 22:37] vendor: and if it has a memory operand that was rip relative and i fixed it up but then instruction reduced in size by 1, then the fixed up address would be off by 1 because displacement is from next rip not start of instruction

[2024-03-12 22:37] vendor: that bug took forever to figure out

[2024-03-14 01:38] irql: anyone know a nice page-out notification on windows?

[2024-03-14 01:39] irql: or maybe some function that would indicate a page has been repurposed / pushed back into the modified list or whatever?

[2024-03-14 01:39] irql: in the context of hooking for my own purposes

[2024-03-14 01:39] irql: i got a nice page in / trade one but maybe there's an easy one for page out too

[2024-03-14 01:40] irql: seems complicated

[2024-03-14 13:51] daax: [replying to irql: "or maybe some function that would indicate a page ..."]
there’s a few ways i can think of but trapping invlpg in ntos to keep track of mm is one way, the other is MmModifiedPageListByColor — the head of linkedlist of MMPFN instances (MMPFNLIST). Flink holds the index (not address) of the first MMPFN instance in pfndb, and mmpfn->flink is next one in the list, easy way to tell if it’s in the modified list is if mmpfn->pagelocation=3

there’s also the MiModifiedPageWriter that will be the start address of a system thread

honestly trapping tlb invalidation is simplest because when a page is moved to modified list it has the present bit cleared associated pte, wvmm (not hv) invalidates entry on current processor and ipis the rest … tracking pte changes there is probably easiest imo because there are potentially lots of other moving parts in the wvmm you’d have to track

[2024-03-14 20:37] irql: man this guy does not miss

[2024-03-14 20:37] irql: yea I started writing the code for TLB shit, but wasn't sure if I was just overcomplicating it

[2024-03-14 20:37] irql: but yea that's probably the best way

[2024-03-14 20:38] irql: yea the windows mm is insane

[2024-03-14 20:38] irql: I love it

[2024-03-14 20:38] irql: but I do appreciate the insight on the kernel side of things, that shit is the worst to reverse 🙏🏾

[2024-03-15 11:51] Horsie: I'm trying to make an RPC call to a windows endpoint, lets assume it is <https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-rsp/43a7d8d6-307d-445c-8678-d209a19926fe>

[2024-03-15 11:51] Horsie: Aka [MS-RSP]: Remote Shutdown Protocol

[2024-03-15 11:52] Horsie: The msdn page also provides 3 different IDL files
[Attachments: image.png]

[2024-03-15 11:54] Horsie: To make the calls using C, do I need to translate the idl into header/call-stub files or is there something I'm missing that allows me to directly make the calls to interface methods without having to deal with the idl myself

[2024-03-15 11:54] dullard: yes

[2024-03-15 11:54] dullard: right click + compile the individual IDL file

[2024-03-15 11:54] dullard: and it will produce _c and _s files

[2024-03-15 11:54] Horsie: I'll do that now. Thanks 👍

[2024-03-15 11:54] dullard: _c is client and _s is server

[2024-03-15 11:56] dullard: https://itm4n.github.io/from-rpcview-to-petitpotam/#creating-an-rpc-client-for-the-efsrpc-interface-in-cc 
This has some commentary about doing it from a dumped IDL but works for a legit IDL too

[2024-03-15 11:56] dullard: ```
efsr_h.h     Header file     Client and Server     Essentially function and structure definitions, well that’s a header file…
efsr_c.c     Source file     Client     Code for the RPC runtime on client side
efsr_s.c     Source file     Server     Code for the RPC runtime on server side, we don’t need this file
```

[2024-03-15 12:42] dullard: [replying to Horsie: "I'll do that now. Thanks 👍"]
Just out of curiosity what are you doing with MS-RSP ?

[2024-03-15 12:43] Horsie: Just playing with RPC for some internal automation stuff

[2024-03-15 12:43] Horsie: Thought RSP seems like a dumb enough implementation to get started with

[2024-03-15 12:43] Horsie: <@655419785106030612>

[2024-03-15 12:46] dullard: [replying to Horsie: "Thought RSP seems like a dumb enough implementatio..."]
MS-WMI is pretty easy / useful too

[2024-03-15 12:47] Horsie: I'm dealing with Access Denied for the time being when I invoke the call. Probably some configuration thing

[2024-03-15 12:47] Horsie: Surprisingly I get access denied even when running it on the machine with the rpc server as admin

[2024-03-15 12:58] Horsie: I'll just look at a different endpoint for now

[2024-03-15 12:59] Horsie: Something weird up with this endpoint

[2024-03-15 14:13] brymko: https://x.com/krisjusiak/status/1768396126904873294?s=12
[Embed: Kris Jusiak (@krisjusiak) on X]
[#cpp26] powerful and easy template meta-programming (part 1/N)
https://t.co/W1OCWz5IMb

[2024-03-15 14:36] Horsie: [replying to dullard: "MS-WMI is pretty easy / useful too"]
Do you have any idea which protofols/interfaces allow unauthenticated access?

[2024-03-15 14:36] Horsie: I tried searching for related terms but no luck so far

[2024-03-15 14:37] dullard: [replying to Horsie: "Do you have any idea which protofols/interfaces al..."]
epmapper

[2024-03-15 14:37] Horsie: Just to clarify, are you asking me to check with epmapper or does epmapper allow unauth access

[2024-03-15 14:37] dullard: [replying to Horsie: "Do you have any idea which protofols/interfaces al..."]
Did you think that MS-RSP allowed unauth shutdown ? <:KEKW:846712430079770625>

[2024-03-15 14:37] Horsie: I'm just trying to see what cool stuff I can do

[2024-03-15 14:37] dullard: [replying to Horsie: "Just to clarify, are you asking me to check with e..."]
epmapper allows unauth

[2024-03-15 14:37] dullard: [replying to Horsie: "I'm just trying to see what cool stuff I can do"]
download RpcView

[2024-03-15 14:37] Horsie: [replying to dullard: "Did you think that MS-RSP allowed unauth shutdown ..."]
<:kappa:1115968816812392470> Gotta try it

[2024-03-15 14:37] dullard: and look at the top right pane

[2024-03-15 14:38] dullard: and it will show a decent amount of info regarding what cred packages are used / the principals which are allowed

[2024-03-15 14:38] Horsie: [replying to dullard: "download RpcView"]
I have that. Let me recheck on it

[2024-03-15 14:38] dullard: RPC is a fucking ballache to understand <:kekw:899071675524591637>

[2024-03-15 14:38] dullard: I hate it

[2024-03-15 14:38] dullard: (but love it)

[2024-03-15 14:39] Horsie: The thing is that microsofts spec states which interface methods need to throw access denied

[2024-03-15 14:39] Horsie: but I couldnt find anywhere else

[2024-03-15 14:39] Horsie: gotta literally open the ~60 page spec for each protocol and check manually

[2024-03-15 14:39] dullard: Yes <:KEKW:846712430079770625>

[2024-03-15 14:40] dullard: You may be able to do some cursed stuff which identifies RPC servers (e.g. exes and dlls on a default windows 10/11/server) and checks params for `RPC_C_AUTHN_NONE`

[2024-03-15 14:40] Horsie: [replying to dullard: "You may be able to do some cursed stuff which iden..."]
Maybe. Lets try that

[2024-03-15 14:40] dullard: I'd be interested in your adventure

[2024-03-15 14:41] dullard: i've been meaning to do some research like this but I don't have time <:cry_death:855651140950818816>

[2024-03-15 14:41] Horsie: Though for some the spec says that the server can use whatever available primitive to check if client should be allowed based on certain criteria lmfao

[2024-03-15 14:42] dullard: there are a couple of ways to validate clients from what I know

[2024-03-15 14:42] dullard: it can either be in the create binding and/or in a security callback

[2024-03-15 14:43] Horsie: Yeah

[2024-03-15 14:43] Horsie: I dont know a *lot* about RPC stuff

[2024-03-15 14:44] Horsie: but most of the stuff I know is from this blog I read which is pretty comprehensive

[2024-03-15 14:44] dullard: share !

[2024-03-15 14:44] Horsie: https://csandker.io/2021/02/21/Offensive-Windows-IPC-2-RPC.html#rpc-communication-flow <@655419785106030612> !!!
[Embed: Offensive Windows IPC Internals 2: RPC]
Remote Procedure Calls (RPC) is a technology to enable data communication between a client and a server across process and machine boundaries (network communication). Therefore RPC is an Inter Process

[2024-03-15 14:44] dullard: Oh yeah

[2024-03-15 14:44] dullard: this guy is good

[2024-03-15 14:44] dullard: The other parts are great too

[2024-03-15 14:44] Horsie: really cool that people put in so much effort to share their knowledge. ^^^

[2024-03-15 14:49] vendor: [replying to brymko: "https://x.com/krisjusiak/status/176839612690487329..."]
since when can you do ...[i] bruh

[2024-03-15 14:49] vendor: have i been overcomplicating things for years

[2024-03-15 14:50] Horsie: I burst a vein somewhere in my head whenever I read code like that

[2024-03-15 14:50] Horsie: Maybe it really *is* useful and I need to start watching less short videos

[2024-03-15 14:51] vendor: [replying to Horsie: "Maybe it really *is* useful and I need to start wa..."]
yeah rip my brain from that stuff

[2024-03-15 15:03] dullard: [replying to Horsie: "Maybe it really *is* useful and I need to start wa..."]
<:KEKW:846712430079770625>

[2024-03-15 15:27] brymko: compile time reflection will always be useful

[2024-03-15 15:27] brymko: but the syntax is just getting out of hand

[2024-03-15 15:42] dullard: [replying to Horsie: "gotta literally open the ~60 page spec for each pr..."]
https://winprotocoldoc.blob.core.windows.net/productionwindowsarchives/Windows_Protocols.zip kinda cursed but cool this exists
[Attachments: l2h_pages.txt]

[2024-03-15 15:49] Horsie: [replying to dullard: "https://winprotocoldoc.blob.core.windows.net/produ..."]
blursed

[2024-03-15 15:49] Horsie: thanks. ill skim through it and see if I find anything cool

[2024-03-16 02:13] rin: when I set a new desktop thread and open a new process I am able to print the window, but when i try to send input to the windows it always fails. I used the isWindowEnabled function and it returns 1 so i should be able to send input to it. any ideas on how to solve this problem

[2024-03-16 02:23] Matti: [replying to rin: "when I set a new desktop thread and open a new pro..."]
post code

[2024-03-16 02:26] rin: ```go
func WindowStatus() {
    ret, _, _ := IsWindowEnabled.Call(window_handle)
    fmt.Println(int(ret))
}

func SetDesktop(desktopName string) {
    newdesktop_name, _ := windows.UTF16PtrFromString(desktopName)
    desktop_HWND, _, _ := CreateDesktopW.Call(uintptr(unsafe.Pointer(newdesktop_name)), 0, 0, 0, 0x10000000, 0)
    SetThreadDesktop.Call(uintptr(unsafe.Pointer(desktop_HWND)))
    currentDesktopName = desktopName
}

func SetWindow(windowName string) (uint16, uint16, error) {
    SetDesktop(currentDesktopName)
    var WindowRect win.RECT
    if windowName == currentDesktopName {
        fmt.Println("set window as desktop window")
        window_handle, _, _ = GetDesktopWindow.Call()
    } else {
        windowNamePtr, _ := windows.UTF16PtrFromString(windowName)
        EnumWindows.Call(windows.NewCallback(SelectWindowProc), uintptr(unsafe.Pointer(windowNamePtr)))
    }
    win.GetClientRect(win.HWND(window_handle), &WindowRect)
    width := WindowRect.Right - WindowRect.Left
    height := WindowRect.Bottom - WindowRect.Top
    FB_width = uint16(width)
    FB_height = uint16(height)
    return uint16(width), uint16(height), nil
}
```

[2024-03-16 02:26] rin: thats how im setting desktop and window handles

[2024-03-16 02:26] rin: ```go
func MouseButton(key uint32, Dx, Dy int32) {
    fmt.Println(Dx, Dy)
    var input MOUSE_INPUT
    input.Type = INPUT_MOUSE
    input.Mi.DwFlags = key
    input.Mi.Dx = Dx
    input.Mi.Dy = Dy
    input.Mi.MouseData = 0
    input.Mi.Time = 0
    input.Mi.DwExtraInfo = 0
    ret, _, _ := SendInput.Call(uintptr(1), uintptr(unsafe.Pointer(&input)), uintptr(uint32(unsafe.Sizeof(input))))
    fmt.Println("mouse ret", ret)
}

func MousePosition(x, y int) {
    ret, _, _ := SetCursorPos.Call(uintptr(x), uintptr(y))
    if int(ret) == 0 {
        fmt.Println("mouse failed")
    }
}
```

[2024-03-16 02:26] rin: this is how i send input

[2024-03-16 02:27] Matti: oh yeah... I forgot you were writing this in go <:harold:704245193016344596>

[2024-03-16 02:27] rin: [replying to Matti: "oh yeah... I forgot you were writing this in go <:..."]
lol

[2024-03-16 02:27] rin: should be the same

[2024-03-16 02:27] rin: technically

[2024-03-16 02:27] Matti: I can probably get that to compile, but nfc how to debug it so that would be pointless

[2024-03-16 02:28] Matti: it's not UIPI right?

[2024-03-16 02:28] rin: UIPI ?

[2024-03-16 02:28] Matti: https://en.wikipedia.org/wiki/User_Interface_Privilege_Isolation?useskin=vector
[Embed: User Interface Privilege Isolation]
User Interface Privilege Isolation (UIPI) is a technology introduced in Windows Vista and Windows Server 2008 to combat shatter attack exploits. By making use of Mandatory Integrity Control, it preven

[2024-03-16 02:28] Matti: you'd need to be of a lower integrity level for that to apply

[2024-03-16 02:30] Matti: you're not checking the `SetThreadDesktop` return code

[2024-03-16 02:30] Matti: is it succeeding?

[2024-03-16 02:31] Matti: > The SetThreadDesktop function will fail if the calling thread has any windows or hooks on its current desktop (unless the hDesktop parameter is a handle to the current desktop).

[2024-03-16 02:31] Matti: just one example why it could fail

[2024-03-16 02:31] rin: ```go
func CurrentDesktop() string {
    SetDesktop(currentDesktopName)
    buf := make([]uint16, 256)
    id := windows.GetCurrentThreadId()
    desktop_HWND, _, _ := GetThreadDesktop.Call(uintptr(id))
    GetUserObjectInformationW.Call(uintptr(unsafe.Pointer(desktop_HWND)), 2, uintptr(unsafe.Pointer(&buf[0])), 256)
    return windows.UTF16ToString(buf)
}
```

[2024-03-16 02:31] rin: this is what i use to check the current desktop and it seems to be working

[2024-03-16 02:32] Matti: uhm well that is one way I guess

[2024-03-16 02:32] Matti: string comparison of the return value of a different API

[2024-03-16 02:32] Matti: but you should really just check the return value of SetThreadDesktop

[2024-03-16 02:32] Matti: any system API call in general really

[2024-03-16 02:34] Matti: second question - what does `GetProcessWindowStation()` return at the point where you call `CreateDesktopW()`?

[2024-03-16 02:34] Matti: it should be non-null

[2024-03-16 02:34] rin: [replying to Matti: "second question - what does `GetProcessWindowStati..."]
im using the default window station,

[2024-03-16 02:35] rin: i did have trouble with undefined behavior between goroutines and the desktop thread in the past

[2024-03-16 02:35] Matti: [replying to rin: "im using the default window station,"]
I can see that from the code

[2024-03-16 02:35] Matti: it's not an answer to the question

[2024-03-16 02:36] rin: [replying to Matti: "I can see that from the code"]
what i mean is that I'm not even setting the windowstation anywhere in my code

[2024-03-16 02:36] Matti: I also guessed that

[2024-03-16 02:36] Matti: so

[2024-03-16 02:36] Matti: what does GetProcessWindowStation return

[2024-03-16 02:37] rin: one sec ill implement that

[2024-03-16 02:37] Matti: third question - can you try `OpenInputDesktop` instead of creating a desktop and see if that makes any difference

[2024-03-16 02:38] Matti: I don't know what the difference is under the hood, I only just noticed this exists

[2024-03-16 02:39] rin: [replying to Matti: "third question - can you try `OpenInputDesktop` in..."]
OpenInputDesktop returns a handle to the desktop thats currently receiving input. so if my new desktop is not receiving input i assume it would just return the default desktop but ill try

[2024-03-16 02:40] Matti: > If the function succeeds, the return value is a handle to the desktop that receives user input.

[2024-03-16 02:40] Matti: > If the function fails, the return value is NULL.

[2024-03-16 02:40] Matti: seems pretty clear to me

[2024-03-16 02:40] Matti: ahh

[2024-03-16 02:41] Matti: I think I may know what you are missing, maybe

[2024-03-16 02:41] Matti: I had to do something similar to this in the past and I had to call `SwitchDesktop()`, IIRC

[2024-03-16 02:41] Matti: after CreateDesktopW, that is

[2024-03-16 02:41] Matti: > Makes the specified desktop visible and activates it. This enables the desktop to receive input from the user.

[2024-03-16 02:42] Matti: and please check return values <:harold:704245193016344596>

[2024-03-16 02:46] rin: [replying to Matti: "I had to do something similar to this in the past ..."]
i kinda wanted to avoid this lol

[2024-03-16 02:46] rin: i think Ill implement better error handling before i do anything else to get a better understanding of the state of things

[2024-03-16 02:47] Matti: [replying to rin: "i kinda wanted to avoid this lol"]
why

[2024-03-16 02:47] Matti: it's gonna be required I'm pretty sure

[2024-03-16 02:48] Matti: looking at my own code for how I did this

[2024-03-16 02:49] Matti: 1. `PsGetProcessWin32WindowStation(Process)`
2. `NtUserSetObjectInformation(UOI_FLAGS)` to set `WSF_VISIBLE` on the window station (not needed if the window station is already visible)
3. open desktop by name and find a handle to it - in this case I needed a specific one
4. `NtUserSetThreadDesktop`
5. `NtUserSwitchDesktop`

[2024-03-16 02:50] Matti: so you'd skip (1) and (2) here assuming the window station is visible, and creating a desktop instead of opening one should be fine so long as you also call SwitchDesktop at the end

[2024-03-16 02:50] Matti: in this case the desktop I opened also did not have input enabled

[2024-03-16 02:53] rin: [replying to Matti: "so you'd skip (1) and (2) here assuming the window..."]
when switching desktops i just use createdesktop because if the desktop already exists it returns a handle to the existing desktop

[2024-03-16 02:56] Matti: what

[2024-03-16 02:56] rin: thats what it says in the docs

[2024-03-16 02:57] rin: and when i reopen with createdesktop all my windows all still there so it seems to be working as intended

[2024-03-16 02:57] Matti: the first part is what it says in the docs

[2024-03-16 02:57] Matti: SwitchDesktop does something else

[2024-03-16 02:57] Matti: that's why they are different APIs

[2024-03-16 02:57] Matti: and why I call both above

[2024-03-16 02:58] Matti: well (3) is more like calling OpenDesktop, but that is also literally what you are doing if the desktop already exists

[2024-03-16 02:58] Matti: anyway

[2024-03-16 02:58] Matti: [replying to Matti: "why"]
why

[2024-03-16 02:58] Matti: why not call SwitchDesktop

[2024-03-16 02:59] rin: yeah it does say switchdesktop opens the desktop for input

[2024-03-16 02:59] Matti: no, it doesn't open a desktop

[2024-03-16 02:59] Matti: that's what OpenDesktop/CreateDesktop do

[2024-03-16 02:59] Matti: it makes the desktop visible and interactive

[2024-03-16 03:00] Matti: note that just because it is visible does not necessarily mean you can send input to it

[2024-03-16 03:01] rin: [replying to Matti: "it makes the desktop visible and interactive"]
thats what i meant

[2024-03-16 03:02] Matti: alright, so you agree with me that it is probably advisable to call it after SetThreadDesktop?

[2024-03-16 03:02] Matti: instead of not calling it

[2024-03-16 03:04] rin: [replying to Matti: "alright, so you agree with me that it is probably ..."]
yeah but im confused why iswindowenabled would return 1 if its not receiving input, unless the window is receving input but the desktop is not so its not actually possible to send it input

[2024-03-16 03:05] Matti: that is a good question

[2024-03-16 03:05] Matti: and a plausible guess

[2024-03-16 03:05] Matti: I honestly don't know

[2024-03-16 03:05] Matti: it's just win32k shit

[2024-03-16 03:07] rin: another possible solution is that when you launch a new process you typically click it to focus it. since the desktop itself doesnt receive input the new process is not focused and cant receive input

[2024-03-16 03:07] rin: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setfocus
[Embed: SetFocus function (winuser.h) - Win32 apps]
Sets the keyboard focus to the specified window. The window must be attached to the calling thread's message queue.

[2024-03-16 03:07] rin: this might help

[2024-03-16 03:09] Matti: pretty sure SendInput should work on any window regardless of focus

[2024-03-16 03:09] Matti: what's its return status by the way?

[2024-03-16 03:09] Matti: it does succeed, but not actually send input? or what

[2024-03-16 03:10] Matti: > This function fails when it is blocked by UIPI. Note that neither GetLastError nor the return value will indicate the failure was caused by UIPI blocking.
ugh

[2024-03-16 03:10] Matti: fuck win32k

[2024-03-16 03:10] Matti: is that for real

[2024-03-16 03:11] Matti: well I doubt it is UIPI like I said before, but still

[2024-03-16 03:11] Matti: what a joke

[2024-03-16 03:12] Matti: don't run this
but if you do, you need to reboot before it takes effect
[Attachments: disable_UIPI.reg]

[2024-03-16 03:13] Matti: you're not trying to send input to csrss.exe or something right

[2024-03-16 03:13] rin: [replying to Matti: "you're not trying to send input to csrss.exe or so..."]
notepad lol

[2024-03-16 03:13] Matti: and your process is elevated and high IL?

[2024-03-16 03:13] Matti: yeah thought so

[2024-03-16 03:14] rin: ill try setfocus give me a moment

[2024-03-16 03:14] Matti: it doesn't neeed to be high IL and probably not elevated either, just IL >= target process IL

[2024-03-16 03:15] Matti: [replying to rin: "ill try setfocus give me a moment"]
ahum

[2024-03-16 03:15] Matti: did you miss the part where we discussed SwitchDesktop

[2024-03-16 03:15] Matti: I mean, feel free to try

[2024-03-16 03:15] Matti: I'm just saying I think it's that

[2024-03-16 03:19] Matti: who knows though, you may be right - in my case the entire desktop was literally not receiving mouse or kb input

[2024-03-16 03:19] Matti: if you can move your mouse it could be SetFocus

[2024-03-16 03:19] Matti: though I'd still be very surprised

[2024-03-16 03:20] Matti: I thought SendInput worked regardless of whether the target window has focus

[2024-03-16 03:20] rin: [replying to Matti: "did you miss the part where we discussed SwitchDes..."]
im trying to create a hidden window so its not an option. But typically this would be the solution

[2024-03-16 03:21] Matti: [replying to Matti: "who knows though, you may be right - in my case th..."]
but also, this was **the** desktop window (meaning, the full screen window of the desktop) so that may explain that

[2024-03-16 03:21] Matti: the one normally created by explorer.exe

[2024-03-16 03:22] Matti: [replying to rin: "im trying to create a hidden window so its not an ..."]
well thanks, that answers the 'why' at least

[2024-03-16 03:22] Matti: thank fuck

[2024-03-16 03:22] Matti: then uh, good luck? I think

[2024-03-16 03:22] rin: yeah ill tell you the solution if i find it

[2024-03-16 03:22] Matti: yeah....

[2024-03-16 04:24] rin: <@148095953742725120> i found that PostMessageW works, although its dependent on how the target application handles input, for example ```go
PostMessage.Call(uintptr(unsafe.Pointer(window_handle)), WM_CHAR, VK_A, 0)
``` works for sending input to a powershell window but wont work for sending input to notepad, even though when sending input to notepad it returns a non 0 status code?

[2024-03-16 04:24] rin: that sends a single 'a' character

[2024-03-16 04:25] rin: ```go
ret1, _, _ := SendMessage.Call(uintptr(unsafe.Pointer(window_handle)), WM_KEYDOWN, VK_A, 0)
ret2, _, _ := SendMessage.Call(uintptr(unsafe.Pointer(window_handle)), WM_KEYUP, VK_A, 0)
``` also works, not for notepad tho

[2024-03-16 04:33] Matti: uhuh

[2024-03-16 04:33] Matti: and if we pretend for a moment you didn't need a hidden window for what I'm sure are very good reasons

[2024-03-16 04:34] Matti: does SwitchDesktop make a difference or not?

[2024-03-16 04:38] rin: [replying to Matti: "does SwitchDesktop make a difference or not?"]
it just makes it the active desktop so if you used switch desktop you would interact with it the same way you would with the regular desktop

[2024-03-16 04:38] rin: [replying to Matti: "and if we pretend for a moment you didn't need a h..."]
its just research and learning

[2024-03-16 04:39] rin: :/

[2024-03-16 04:39] Matti: [replying to rin: "it just makes it the active desktop so if you used..."]
I'm aware

[2024-03-16 04:39] rin: [replying to rin: "it just makes it the active desktop so if you used..."]
assuming you used generic access all

[2024-03-16 04:39] Matti: that is what it says in the docs

[2024-03-16 04:39] rin: yes

[2024-03-16 04:40] Matti: well then I don't get what you're trying to do

[2024-03-16 04:40] Matti: you can't send input to a non-active desktop

[2024-03-16 04:40] rin: not the desktop itself i dont think

[2024-03-16 04:40] rin: only windows within the desktop

[2024-03-16 04:45] Matti: ugh

[2024-03-16 04:45] Matti: fine, don't believe me

[2024-03-16 04:45] Matti: you could just add the 1 LOC to test this you know

[2024-03-16 04:45] Matti: instead of arguing why you think it won't work

[2024-03-16 04:46] rin: im not arguing, I solved my problem

[2024-03-16 04:48] Matti: lol ok

[2024-03-16 09:09] diversenok: [replying to rin: "yeah but im confused why iswindowenabled would ret..."]
"Enabled window", "focused window", and "interactive desktop" are all different independent things

[2024-03-16 09:10] diversenok: If I understand correctly, you are trying to manipulate windows on a desktop without making the desktop interactive via `SwitchDesktop`

[2024-03-16 09:10] diversenok: Which is totally doable

[2024-03-16 09:11] diversenok: There is one interactive desktop and `SwitchDesktop` sets the given desktop to be interactive

[2024-03-16 09:13] diversenok: Focused state for a window has meaning mainly only on an interactive desktop because it defines which window accepts user input

[2024-03-16 09:14] diversenok: Since non-ineractive desktops have no user input, it doesn't really matter who is supposed to recieve it

[2024-03-16 09:15] diversenok: Enabled state of a window is yet another thing which just defines if it's ready to accept messages

[2024-03-16 09:16] diversenok: Many windows are enabled at the same time

[2024-03-16 09:16] diversenok: And they can also be enabled on non-interactive desktops

[2024-03-16 09:17] rin: [replying to diversenok: "Enabled state of a window is yet another thing whi..."]
Noted

[2024-03-16 09:17] rin: It made a lot more sense after the fact

[2024-03-16 09:18] rin: Sendinput is handled by the active desktop but postmessage goes directly to the window

[2024-03-16 09:19] diversenok: Yeah, you can fabricate something that looks to the window like user input programmatically, even on non-inteactive desktops

[2024-03-16 09:20] diversenok: And the window doesn't need to be focused for that

[2024-03-16 09:20] diversenok: But probably needs to be enabled

[2024-03-16 09:21] rin: [replying to diversenok: "Yeah, you can fabricate something that looks to th..."]
Wdym

[2024-03-16 09:22] diversenok: Well, posting messages does exactly that

[2024-03-16 09:22] rin: You mean looks identical to normal user input?

[2024-03-16 09:23] diversenok: Yeah, more or less; in the end, what windows see during input is messages

[2024-03-16 09:23] rin: [replying to rin: "<@148095953742725120> i found that PostMessageW wo..."]
This is what i did in the end

[2024-03-16 09:23] rin: For reference

[2024-03-16 09:24] diversenok: With notepad you might need to send messages to some child window, not the top-level window of notepad

[2024-03-16 09:25] diversenok: Powershell console has one window and it probably does everything

[2024-03-16 09:25] diversenok: Notepad has several nested windows like the textbox area and menus

[2024-03-16 09:26] diversenok: Usually, Windows directs the messages to focused window which might be deeper in the hierarchy

[2024-03-16 09:28] diversenok: 
[Attachments: image.png]

[2024-03-16 09:29] rin: Ill note this down, although in my windowsenum callback function i only get the top level window. in a general application it would be annoying to enumerate all child windows for each window

[2024-03-16 09:31] rin: I guess the top level window only accepts certain messages because it still returns as enabled

[2024-03-16 09:32] diversenok: Yes, and if you disable a window it automatically disables all its descendants, so it's definitely enabled

[2024-03-16 09:33] rin: [replying to diversenok: "Yes, and if you disable a window it automatically ..."]
Ic

[2024-03-16 09:33] rin: Makes sense

[2024-03-16 09:34] diversenok: Here is the hierarchy of windows that I see when I start notepad on a different desktop
[Attachments: image.png]

[2024-03-16 09:34] diversenok: Gray shows disabled windows, black shows enabled

[2024-03-16 09:35] rin: I assume edit would be where you would actually send input

[2024-03-16 09:35] diversenok: Yeah, I think so

[2024-03-16 09:36] rin: Ill let you know next time im testing

[2024-03-16 09:38] rin: [replying to diversenok: "Here is the hierarchy of windows that I see when I..."]
What system tool is this btw

[2024-03-16 09:39] diversenok: Some unfinished project of mine for inspecting windows on non-interactive desktops

[2024-03-16 09:40] rin: Should finish it lol, looks good

[2024-03-16 10:39] Matti: [replying to diversenok: "Which is totally doable"]
mm this is interesting, TIL

[2024-03-16 10:40] Matti: all of the other statements following I did know, but I wasn't aware of this one <:lillullmoa:475778601141403648>

[2024-03-16 10:42] Matti: the reason I needed to use SwitchDesktop in my project (it's this same "session 0 desktop" driver I've mentioned before) is because at some point wininit.exe or wininitext.dll used to call this, but at some point after windows 10 was released MS must've realised that this was now a mistake, because S0 is supposed to be non-interactive

[2024-03-16 10:43] Matti: so that is simply code I took from the previous build of windows 10 in which it still worked and rewrote for kernel mode

[2024-03-16 10:45] Matti: [replying to rin: "What system tool is this btw"]
PH/SI has a similar-ish  (depending on what you need) WindowExplorer plugin
[Attachments: image.png]

[2024-03-16 10:45] Matti: I don't think it shows the desktops anywhere though

[2024-03-16 10:47] Matti: it can do this for the main window (process overview), but then you run into an issue where you can't enumerate desktops of window stations other than your own

[2024-03-16 10:48] Matti: well you run into this issue no matter what I suppose

[2024-03-16 10:49] Matti: I was gonna look into making a dirty hack workaround for this in my driver for testing purposes before, but I took one look at win32k and then closed it again in disgust

[2024-03-16 10:51] Matti: if I could remember the error status returned for this (access denied?) I could maybe find the exact place where the check is done, but I don't, and I also just don't really care enough to write some test POC for it either

[2024-03-17 14:39] prick: anyone know why this works on server 2003 but not windows 10?
[Attachments: driver.cpp]

[2024-03-17 14:39] prick: FltCreateCommunicationPort fails with access denied (c0000022)

[2024-03-17 14:39] prick: googled around, nothing

[2024-03-17 17:15] daax: [replying to prick: "FltCreateCommunicationPort fails with access denie..."]
failure is coming from `ObInsertObject` when `FltCreateCommunicationPort` tries to create `FilterCommunicationPort` object. most likely to do with bad security descriptor for the object when `SeDefaultObjectMethod` gets called during insertion of the object. easiest way to determine is open windbg and start debugging.

[2024-03-17 17:20] prick: yeah i just don't know what it could be failing from

[2024-03-17 17:21] prick: the examples online on how to do this is all pretty much the same

[2024-03-17 17:21] prick: the best i can infer is some ini file issue

[2024-03-17 17:21] prick: but idk where to start with that either

[2024-03-17 18:25] prick: 8.1 and 1607 also not working