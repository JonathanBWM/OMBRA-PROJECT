# May 2024 - Week 1
# Channel: #reverse-engineering
# Messages: 134

[2024-05-01 05:30] Ryan: Top mobile malwares & RE courses recommendations please

[2024-05-01 05:54] Ryan: Didn't see anything android there!

[2024-05-01 05:58] Koreos: [replying to Ryan: "Didn't see anything android there!"]
LaurieWired

[2024-05-01 06:20] PeasantSeville: [replying to Ryan: "Top mobile malwares & RE courses recommendations p..."]
https://www.ragingrock.com/AndroidAppRE/
[Embed: Android App Reverse Engineering 101]
Learn to reverse engineer Android applications!

[2024-05-01 06:21] PeasantSeville: if you're willing to tolerate google translate there's https://dreamhack.io
[Embed: 해커들의 놀이터, Dreamhack]
해킹과 보안에 대한 공부를 하고 싶은 학생, 안전한 코드를 작성하고 싶은 개발자, 보안 지식과 실력을 업그레이드 시키고 싶은 보안 전문가까지 함께 공부하고 연습하며 지식을 나누고 실력 향상을 할 수 있는 공간입니다.

[2024-05-02 00:42] Bryce: 
[Attachments: IsDebuggerPresentMap.PNG]

[2024-05-02 00:42] Bryce: how do i blovk this mf

[2024-05-02 00:46] szczcur: [replying to Bryce: "how do i blovk this mf"]
what do you mean block? circumvent isdebuggerpresent? patch the peb directly is simplest solution unless there is missing context

[2024-05-02 00:47] Bryce: what tool would u suggest? ive looked at it in cheat engine, ida, x65dgb, binary ninja etc

[2024-05-02 00:51] szczcur: [replying to Bryce: "what tool would u suggest? ive looked at it in che..."]
```
mov rax, qword ptr[gs:0x60]
mov byte ptr ds:[rax+2], 0
```

[2024-05-02 00:52] Bryce: [replying to szczcur: "```
mov rax, qword ptr[gs:0x60]
mov byte ptr ds:[r..."]
hmm

[2024-05-02 00:52] Bryce: where do i put it tho

[2024-05-02 00:52] Bryce: ive tried to nop out all of this

[2024-05-02 00:53] Bryce: and that did not work

[2024-05-02 00:55] szczcur: [replying to Bryce: "where do i put it tho"]
how new to this are you? asking because i dont want to recommend something thats out of your depth

[2024-05-02 00:55] Bryce: very very new. i think ur gna suggest hex editor?

[2024-05-02 00:55] asz: well- hes not even debugging

[2024-05-02 00:55] Bryce: how the fuck

[2024-05-02 00:55] Bryce: this guy

[2024-05-02 00:56] Bryce: is everywhere

[2024-05-02 00:56] Bryce: legend

[2024-05-02 01:00] szczcur: [replying to Bryce: "very very new. i think ur gna suggest hex editor?"]
ah no, i was going to suggest some reading: <https://www.apriorit.com/dev-blog/367-anti-reverse-engineering-protection-techniques-to-use-before-releasing-software>

[2024-05-02 01:00] Bryce: i know i need to get it to return a value of 0

[2024-05-02 01:00] Bryce: i just dont know what tool to try to use

[2024-05-02 01:00] Bryce: to edit it

[2024-05-02 01:02] szczcur: [replying to Bryce: "i just dont know what tool to try to use"]
x64dbg, put breakpoint after isdebuggerpresent, clear rax

[2024-05-02 01:04] Bryce: cant even get x64 to run it

[2024-05-02 01:04] Bryce: it will auto close

[2024-05-02 01:04] szczcur: you could also try frida and then just write a simple script to attach and `onLeave: function(retval) { retval.replace(0); }`

[2024-05-02 01:05] szczcur: [replying to Bryce: "cant even get x64 to run it"]
options > preferences > which boxes are ticked

[2024-05-02 01:06] szczcur: once you have it waiting at a breakpoint you can navigate to symbols tab and find isdebuggerpresent, click it and hit f2

[2024-05-02 01:06] szczcur: to set a breakpoint

[2024-05-02 01:07] szczcur: then do like i mentioned above with step, then clear rax

[2024-05-02 01:07] Bryce: 
[Attachments: image.png]

[2024-05-02 01:07] Bryce: i have scyllahide plug in too

[2024-05-02 01:08] szczcur: try threadentry and threadcreate

[2024-05-02 01:09] szczcur: the ones you have set should should hit at least once though

[2024-05-02 01:10] szczcur: it shouldnt just close

[2024-05-02 01:15] Bryce: oh shit its working

[2024-05-02 01:15] Bryce: ok i think i can actually set the nbreakpoint now

[2024-05-02 01:17] Bryce: crashed my .exe tho within a min

[2024-05-02 01:18] Bryce: 
[Attachments: image.png]

[2024-05-02 04:40] Azalea: why not scyllahide

[2024-05-02 14:32] Horsie: Any cool windows RCEs that involve some sort of binary exploitation?

[2024-05-02 14:33] Horsie: I've taken a look at EternalBlue and its pretty cool. Anything else that's a fun vuln to RE?

[2024-05-02 14:34] Horsie: I was considering chompie's SigRed. (<https://chomp.ie/Blog+Posts/Anatomy+of+an+Exploit+-+RCE+with++SIGRed>)

[2024-05-02 14:35] Horsie: Unfortunately it seems like it's going to be difficult to install an older version of the DNS server feature on my win server 2012 vm

[2024-05-02 14:39] Brit: make a vm with a winver that has that dns server?

[2024-05-02 14:42] Matti: just fukkit yolo copy over the older binary over the current one

[2024-05-02 14:42] Matti: from the original ISO

[2024-05-02 14:42] Horsie: [replying to Brit: "make a vm with a winver that has that dns server?"]
I'll double check. I thought the DNS server was something that you had to install via the dashboard

[2024-05-02 14:42] Matti: it is

[2024-05-02 14:43] Matti: but the binaries for it should be in winsxs regardless

[2024-05-02 14:43] Horsie: I'll see if I can copy over the files. I saw that it's only a single exe+dll

[2024-05-02 14:44] Matti: yeah, you'll probably need to fix up the permissions a bit, but usually this does mostly work

[2024-05-02 14:44] Horsie: Any other vulns I could try? For some reason I used to think Winblows had a lot of RCEs for older versions

[2024-05-02 14:45] Matti: exceptions are system critical files like DLLs in knowndlls or drivers, those aren't always compatible with RTM versions after a few years

[2024-05-02 14:45] Horsie: Now that I'm googling, there's maybe 3-ish in the last couple years

[2024-05-02 14:45] Horsie: Smb, DNS and RPC are the ones I've found so far

[2024-05-02 14:45] Matti: [replying to Matti: "yeah, you'll probably need to fix up the permissio..."]
oh and make sure the dll/exe are not running or loaded in some other process obviously

[2024-05-02 14:46] Horsie: [replying to Matti: "oh and make sure the dll/exe are not running or lo..."]
Yeah I'll try that. I'm optimistic

[2024-05-02 14:47] Matti: [replying to Horsie: "Smb, DNS and RPC are the ones I've found so far"]
smb has like a vuln a day no? just don't ask me which

[2024-05-02 14:47] Matti: <@651054861533839370>

[2024-05-02 14:47] Matti: 'RPC' is kinda broad, but yes that one too

[2024-05-02 14:48] Brit: prblems with SMB on windows?

[2024-05-02 14:48] Brit: surely you jest

[2024-05-02 14:48] Horsie: [replying to Matti: "'RPC' is kinda broad, but yes that one too"]
This guy has a good writeup https://s1ckb017.github.io/2022/06/17/CVE-2022-26809-Server-Side-vulnerable-point-reachability.html
[Embed: CVE-2022-26809 Reaching Vulnerable Point starting from 0 Knowledge ...]
Lately, along to malware analisys activity I started to study/test Windows to understand something more of its internals. The CVE here analyzed, has been a good opportunity to play with RPC and learn 

[2024-05-02 14:49] Matti: [replying to Horsie: "This guy has a good writeup https://s1ckb017.githu..."]
yeah this kind of code is why I unironically find ALPC easier to write and use than MS RPC <:harold:704245193016344596>

[2024-05-02 14:49] Matti: which is built on ALPC, when doing local IPC

[2024-05-02 14:50] Matti: and not just because this is an IDA decompilation either

[2024-05-02 14:55] Matti: all of the stuff mentioned in that blog that's attacker controllable is 100% related to the insane RPC protocol and the DLL responsible for it

[2024-05-02 14:55] Matti: an ALPC server using plain NT object security could have prevented this

[2024-05-02 16:30] asz: smg had a vuln regarding doing ioctrl for opening with bypassing filesystem layer dma acces

[2024-05-02 16:30] asz: not that long ago

[2024-05-02 17:09] Matti: coincidentally

[2024-05-02 17:10] Matti: \> see this
\> get an instant hardon because apparently there's an SSH client and/or server in the kernel, implemented using ALPC
[Attachments: image.png]

[2024-05-02 17:10] Matti: turns out ssh is 'sleep study helper' 😡

[2024-05-02 17:11] Matti: which is some Po shit to keep track of your system's ACPI sleep states

[2024-05-02 17:11] Torph: LMAO

[2024-05-02 17:30] luci4: [replying to Matti: "turns out ssh is 'sleep study helper' 😡"]
It's related to this thing?
https://learn.microsoft.com/en-us/windows-hardware/design/device-experiences/modern-standby-sleepstudy
[Embed: Modern standby SleepStudy]
The SleepStudy tool provides overview information about each modern standby session.

[2024-05-02 17:31] Matti: ya, I'm aware of what it is

[2024-05-02 17:31] luci4: TIL

[2024-05-02 17:31] Matti: it's just not the first thing I associate the letters 'SSH' with

[2024-05-02 17:31] luci4: Windows has so many features

[2024-05-02 17:31] Matti: ~~features~~shit

[2024-05-02 17:32] luci4: [replying to Matti: "~~features~~shit"]
Well it does seem kinda useless

[2024-05-02 17:32] Matti: why would my PC be in a sleep state

[2024-05-02 17:32] luci4: I was just reading the article

[2024-05-02 17:32] Matti: why is it not compiling instead of sleeping, would be my question

[2024-05-02 17:32] Matti: for example

[2024-05-02 17:33] Matti: well the real reason I hate it is because every system driver imports from this sleepstudyhelper.sys now

[2024-05-02 17:33] Matti: which has been a nuisance in the past

[2024-05-02 17:34] Matti: same for WppRecorder.sys

[2024-05-02 17:40] Azrael: [replying to luci4: "It's related to this thing?
https://learn.microsof..."]
Why does this even exist?

[2024-05-02 17:42] luci4: [replying to Azrael: "Why does this even exist?"]
As I said, doesn't seem very useful 🤣 . I guess if you're trying to diagnose power issues it might be handy

[2024-05-02 17:47] Matti: > Watch this video to [...] use SleepStudy to find and fix components that cause unexpected battery drain.
you could already do this (minus the exact mWh numbers shown in the video, of which I question the accuracy and usefulness anyway) with xperf.exe to trace ETW power state events

[2024-05-02 17:48] Matti: > In some cases, the capacity ratio will exceed 100 percent.
mhm

[2024-05-02 18:26] Brit: [replying to Matti: "why is it not compiling instead of sleeping, would..."]
it's wasting cycles on the antimalware service anyway

[2024-05-02 18:28] Matti: yeah that's another good point

[2024-05-02 18:29] Brit: I don't even know how to disable it on up to date versions without breaking everything

[2024-05-02 18:29] Matti: however, matti WRK now also supports this sleep study helper, as well as that Wpp shit driver, and purposely 'supports' (stubs outs) ELAM driver support

[2024-05-02 18:29] Brit: blessed

[2024-05-02 18:29] Matti: so it could be useful there... if I didn't already have xperf

[2024-05-02 18:30] Matti: [replying to Brit: "I don't even know how to disable it on up to date ..."]
there's an F8 boot option you can choose to disable ELAM drivers I think right?

[2024-05-02 18:30] Matti: mind you, you need to do it manually every time

[2024-05-02 18:31] Brit: time to bootkit myself to make my computer usable

[2024-05-02 18:31] Brit: <:mmmm:904523247205351454>

[2024-05-02 18:31] Matti: only crazy people think like that

[2024-05-02 19:44] asz: nah- only input it if advanced options enabled

[2024-05-02 19:45] asz: you can though just bcdedit set it

[2024-05-02 19:45] asz: documentation is wrong

[2024-05-02 19:46] asz: 
[Attachments: image.png]

[2024-05-02 20:29] Matti: oh neat, that's good to know

[2024-05-02 20:30] Matti: I know that for some images (the kernel, HAL, some boot drivers) you have to press F8 every time, regardless of `testsigning`

[2024-05-02 20:30] Matti: because `disableintegritychecks` was made ineffective unless you either manually type it via F10, or choose it via F8

[2024-05-02 20:30] Matti: if you modified them I mean obviously

[2024-05-02 20:31] Matti: I expected disableelamdrivers to get the same lame treatment

[2024-05-03 01:33] Torph: [replying to Matti: "why would my PC be in a sleep state"]
when your laptop is closed?

[2024-05-03 01:41] Matti: I....

[2024-05-03 01:41] Matti: why would I have a laptop....

[2024-05-03 01:41] Matti: very confusing question

[2024-05-03 03:11] Torph: ok that's pretty funny

[2024-05-03 03:11] Torph: I haven't owned a desktop since 2012 (when I was 6)

[2024-05-03 03:12] Torph: it was some old machine from like 1999 I used to play Minecraft

[2024-05-05 08:13] Windy Bug: anyone came across AVs that leverage the kernel shim engine for PG compatible monitoring ?

[2024-05-05 08:14] Windy Bug: personally I haven’t which I find surprising considering some of them go as far as using techniques like infinity hook ?

[2024-05-05 09:03] Timmy: interesting

[2024-05-05 09:03] Timmy: https://blackhoodie.re/assets/archive/Kernel_Shim_Engine_for_fun_-_pwissenlit.pdf

[2024-05-05 09:03] Timmy: found some slides on the topic

[2024-05-05 09:04] Timmy: https://www.geoffchappell.com/studies/windows/km/ntoskrnl/api/kshim/index.htm

[2024-05-05 09:04] Timmy: surprised to learn about this

[2024-05-05 09:06] Timmy: http://publications.alex-ionescu.com/Recon/Recon%202016%20-%20Abusing%20the%20Kernel%20Shim%20Engine.pdf

[2024-05-05 14:06] daax: [replying to Windy Bug: "anyone came across AVs that leverage the kernel sh..."]
kaspersky

[2024-05-05 16:50] JustMagic: [replying to Windy Bug: "personally I haven’t which I find surprising consi..."]
well, KSE and infinityhook give you ability to hook different things. They're not really interchangable