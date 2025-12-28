# April 2024 - Week 2
# Channel: #reverse-engineering
# Messages: 384

[2024-04-08 07:50] repnezz: noob question - why do we have to disable interrupts here? https://secret.club/2021/01/12/callout.html
[Embed: Hiding execution of unsigned code in system threads]
Anti-cheat development is, by nature, reactive; anti-cheats exist to respond to and thwart a videogame’s population of cheaters. For instance, a videogame with an exceedingly low amount of cheaters wo

[2024-04-08 10:32] vendor: [replying to repnezz: "noob question - why do we have to disable interrup..."]
it doesn't mention it in the article but they might be asynchronously reading each core's interrupt stacks as well. if you don't disable interrupts then your fake rsp and invalid rip will get leaked to those whenever one occurs.

[2024-04-08 10:36] vendor: also if you disable interrupts and swap the stack like mentioned in that article then you don't need to disable APCs at all. disabling APCs itself is probably a detection vector.

[2024-04-08 10:38] vendor: but this method isn't sufficient anymore because they also use NMIs

[2024-04-08 17:11] daax: [replying to repnezz: "noob question - why do we have to disable interrup..."]
<@285957040688463873>

[2024-04-08 17:12] drew: [replying to repnezz: "noob question - why do we have to disable interrup..."]
the issue we are trying to avoid is an interrupt routine seeing that rip is inside of our code, so we avoid it by disabling interrupts altogether

[2024-04-08 17:12] drew: but as someone mentioned it's insufficient to do this because of NMIs

[2024-04-09 15:30] the eternal bleb: is a dxgkrnl gdi hook still an efficient way of drawing? or are there detection vectors for it now a days

[2024-04-09 16:18] Mysterio: just use some overlay lib

[2024-04-09 16:18] Mysterio: and save yourself the headache of injecting or hooking

[2024-04-09 18:12] the eternal bleb: [replying to Mysterio: "just use some overlay lib"]
would do that but modern day AC's have a shit ton of detection vectors on it

[2024-04-09 18:12] the eternal bleb: I currenty hijack CrosshairX overlay but they can detect when I change window attributes

[2024-04-09 18:12] the eternal bleb: they also flag topmost windows

[2024-04-09 19:09] JustMagic: [replying to the eternal bleb: "would do that but modern day AC's have a shit ton ..."]
and you think a shitty copy pasted hook in kernel isn't going to be detected?

[2024-04-09 19:09] the eternal bleb: [replying to JustMagic: "and you think a shitty copy pasted hook in kernel ..."]
that's why I asked

[2024-04-09 19:09] the eternal bleb: because no I don't know

[2024-04-09 20:45] daax: [replying to the eternal bleb: "that's why I asked"]
Please consult the <#835634425995853834>.

[2024-04-11 15:55] Horsie: Is there a way to completely disable ASLR and KASLR in on Windows so all module address are deterministic?

[2024-04-11 15:57] Horsie: Does `“MoveImages”=dword:00000000` affect Kernel and UM images?

[2024-04-11 15:58] Horsie: Also is there something similar for Windows' efi modules (particularly when booting with OVMF)

[2024-04-11 17:23] Nats: [replying to Horsie: "Does `“MoveImages”=dword:00000000` affect Kernel a..."]
only user-mode

[2024-04-11 19:24] irql: [replying to Horsie: "Also is there something similar for Windows' efi m..."]
also looking for this

[2024-04-11 19:24] irql: don't think there is though

[2024-04-11 19:24] irql: you can parse DVRT and undo a lot of shit but eh

[2024-04-11 21:53] Matti: [replying to Horsie: "Is there a way to completely disable ASLR and KASL..."]
only on windows 7 and below

[2024-04-11 21:53] Matti: [replying to Horsie: "Does `“MoveImages”=dword:00000000` affect Kernel a..."]
so, same answer

[2024-04-11 21:54] Matti: actually maaaaybe KASLR is mandatory since vista, and only UM addresses are affected by this in vista/7

[2024-04-11 21:54] Matti: I know on 2003/XP64 it disables both, let me check win 7 x64 real quick

[2024-04-11 21:57] Matti: ya, sorry for the confusion

[2024-04-11 21:57] Matti: MoveImages doesn't disable KASLR on vista/7

[2024-04-11 21:57] Matti: only UM ASLR

[2024-04-11 21:58] Matti: you can set the relocations stripped flag in a driver file to prevent it from being relocated, if that's an option

[2024-04-11 21:59] Matti: well nowadays that's probably illegal... but assuming we're using an old version of windows anyway

[2024-04-11 22:29] Matti: I don't actually know the answer to your question

[2024-04-11 22:30] Matti: but pretty safe to say that whatever it is is either an issue with gdb or your usage of it

[2024-04-11 22:30] Matti: IDA is just a frontend for it, same as when you use it with windbg

[2024-04-11 22:30] Matti: what's the target? can you attach gdb standalone?

[2024-04-11 22:53] Matti: I only said it's a front end

[2024-04-11 22:54] Matti: I'd never use ida for debugging myself <:lillullmoa:475778601141403648>

[2024-04-11 22:54] Matti: does using tcg instead of kvm make a difference?

[2024-04-11 22:54] Matti: or is using kvm the whole point

[2024-04-11 22:55] Matti: you can do nested virt inside qemu itself I thought, although it will be unbearably slow

[2024-04-11 22:55] Matti: another suggestion - try lldb instead of gdb

[2024-04-11 22:55] Matti: I've had it work in the past on things gdb had issues with

[2024-04-11 23:06] Matti: as a windows user, I'm a big fan of windbg + EXDI for gdb stub debugging

[2024-04-11 23:08] Matti: though, using the gdb stub with qemu on windows requires tcg as whvp doesn't support it :/

[2024-04-11 23:08] Matti: it's fast enough for my purposes though since I rarely need to debug past boot

[2024-04-11 23:09] Matti: also, this is just me but ||I also often simply debug a physical machine instead||

[2024-04-11 23:09] Matti: it's less work than you'd think

[2024-04-11 23:20] Matti: well it's a bit of an investment to be sure, but you hardly need to be rich

[2024-04-11 23:21] Matti: just buy any cheap motherboard and make sure it has a COM header for an RS232 serial cable

[2024-04-11 23:21] Matti: then put in whatever hardware is cheapest while still meeting the requirements of whatever it is you wanna debug

[2024-04-11 23:22] Matti: most of the time you can buy all this second hand

[2024-04-11 23:25] Matti: which part? exdi or physical debugging?

[2024-04-11 23:26] Matti: mind you I just realised, physical debugging is probably not gonna help you if you're having issues with either a type 1 hypervisor or firmware/UEFI drivers

[2024-04-11 23:26] Matti: unless you want to go the DCI route

[2024-04-11 23:26] Matti: which you don't

[2024-04-11 23:26] Matti: personally I just use this with windbg

[2024-04-11 23:27] Matti: it works fine for either regular kernel or hypervisor debugging with NT

[2024-04-11 23:27] Matti: [replying to Matti: "mind you I just realised, physical debugging is pr..."]
for these kinds of issues you'd want EXDI + qemu

[2024-04-11 23:28] Matti: if gdb isn't working for some reason

[2024-04-11 23:30] Matti: https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/setting-up-a-network-debugging-connection - for kdnet (recommended if you can use it)
https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/attaching-to-a-virtual-machine--kernel-mode- - for kdcom/serial (horrible but sometimes unavoidable)
[Embed: Set Up KDNET Network Kernel Debugging Manually - Windows drivers]
Learn how Debugging Tools for Windows supports kernel debugging over a network. This article describes how to set up network debugging manually.
[Embed: Setting Up Kernel-Mode Debugging of a Virtual Machine Manually Usin...]
Debugging Tools for Windows supports kernel debugging of a virtual machine using a Virtual COM Port.

[2024-04-11 23:30] Matti: and yeah you'll need to do this to get windbg to work

[2024-04-11 23:30] Matti: I thought there was an MSDN article on this too...

[2024-04-11 23:31] diversenok: Jonas shared how simple it is to do kernel debugging against Windows Sandbox and I use it ever since

[2024-04-11 23:32] Matti: yeah that is another option

[2024-04-11 23:32] Matti: but it requires a windows host

[2024-04-11 23:32] Matti: with hyper-v <:harold:704245193016344596>

[2024-04-11 23:32] Matti: right?

[2024-04-11 23:32] diversenok: Yeah, and it comes with some limitations like the debuggee will be the same OS version as the host

[2024-04-11 23:33] diversenok: Otherwise, two commands to set up and one-two clicks to launch

[2024-04-11 23:33] Matti: hmm <@824294337584169001> I can't find the article I was thinking of now, but really it should be as simple as setting up your windows VM to use a bridged ethernet adapter

[2024-04-11 23:35] Matti: either that or forward a physical serial port to the VM as its COM1

[2024-04-11 23:35] Matti: depending on which of the two transports you're going to use

[2024-04-11 23:35] Matti: kdnet is generally both easier to set up and much faster/better to use

[2024-04-11 23:36] Matti: but especially when doing things like debugging nested virtualization you also want/need a secondary serial connection

[2024-04-11 23:36] Matti: so it's good to make sure you can use both

[2024-04-11 23:37] Matti: unfortunately I don't know shit about kvm so I can't help you with this part

[2024-04-11 23:38] Matti: well then there is some config issue

[2024-04-11 23:38] Matti: windbg serial is the only thing that *always* works

[2024-04-11 23:38] Matti: kdnet only usually works

[2024-04-11 23:39] Matti: but like I said, it's easier to set up as well

[2024-04-11 23:39] Matti: so may want to start with that

[2024-04-11 23:40] Matti: just remember you need a bridged adapter - if the debuggee can't ping your VM, it won't work

[2024-04-11 23:41] Matti: as for serial, my guess is that you'll need to forward `/dev/ttyS0` (physical, on the host - this can be from a USB adapter) to your VM somehow in KVM

[2024-04-11 23:42] Matti: how, I don't know

[2024-04-11 23:42] Matti: also note that the debuggee* serial port needs to be on the motherboard

[2024-04-11 23:42] Matti: it can't be a USB/PCIe to serial adapter because of reasons

[2024-04-11 23:44] Matti: (actually on linux it can be a USB adapter.... but on NT you're stuck with what the HAL understands which isn't much)

[2024-04-11 23:46] Matti: this is what you need on a regular ATX motherboard
[Attachments: image.png]

[2024-04-11 23:54] Matti: ah cool - I have to say, funnily enough I've never actually used kdcom with qemu.... only kdnet and exdi

[2024-04-11 23:54] Matti: well you already have gdb right?

[2024-04-11 23:54] Matti: why not just connect and see what happens?

[2024-04-11 23:55] Matti: and same with lldb, they're very similar in usage

[2024-04-11 23:55] Matti: leave IDA out lol

[2024-04-11 23:56] Matti: and don't forget qemu itself also has a very rudimentary debugger

[2024-04-11 23:57] Matti: monitor mode? I think it's called

[2024-04-11 23:57] Deleted User: [replying to diversenok: "Jonas shared how simple it is to do kernel debuggi..."]
is there any automated way to setup kernel debugging on vmware?

[2024-04-11 23:58] Matti: yeah but you only need to learn like 3 commands to check this

[2024-04-11 23:58] Matti: literally `help` is probably enough to get you there

[2024-04-11 23:59] Matti: well, in lldb anyway.... gdb might be a bit much

[2024-04-12 00:00] Matti: [replying to Deleted User: "is there any automated way to setup kernel debuggi..."]
what part needs automating

[2024-04-12 00:01] Matti: if you want to use it, kdnet.exe can do like 90% of the work for you

[2024-04-12 00:01] Matti: but even without that it's like 3 steps

[2024-04-12 02:27] abu: [replying to Matti: "just buy any cheap motherboard and make sure it ha..."]
U got any more info on this? Like blogs, tuts, anything really.

[2024-04-12 02:38] Matti: [replying to abu: "U got any more info on this? Like blogs, tuts, any..."]
you need one of these
[Attachments: 510V4qIoDOL.jpg]

[2024-04-12 02:38] Matti: and then a "null modem cable"

[2024-04-12 02:39] Matti: https://www.datapro.net/products/rs232-null-modem-cable-db09ff.html
[Embed: RS232 Null-Modem Cable DB09FF -- DataPro]
RS232 Null-Modem Cable DB09FF

[2024-04-12 02:39] Matti: other than that for the HW side, just follow https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/setting-up-a-null-modem-cable-connection
[Embed: Setting Up Kernel-Mode Debugging Over a Serial Cable Manually - Win...]
Debugging Tools for Windows supports kernel debugging over a null-modem cable.

[2024-04-12 02:40] Matti: like I said before, you can use a USB to serial adapter on the debugger side (this is usually more convenient)

[2024-04-12 02:40] Matti: only the debuggee needs to have this on the board

[2024-04-12 02:41] Matti: [replying to Matti: "you need one of these"]
<https://www.amazon.com/StarTech-com-Serial-Motherboard-Header-Panel/dp/B0067DB6RU> if you don't know what to search for

[2024-04-12 02:43] abu: Thanks! Is it possible to still do this while the system isn't in debug mode?

[2024-04-12 02:44] Matti: do what? just connect the two systems?

[2024-04-12 02:44] Matti: or debug whilst not in debug mode

[2024-04-12 02:44] Matti: if the latter, no

[2024-04-12 02:44] Matti: it's just another kd transport method

[2024-04-12 02:47] abu: Yeah I was hoping to debug while not in debug mode

[2024-04-12 02:47] Matti: btw, I don't recommend *using* kdcom (serial) if you can at all avoid it

[2024-04-12 02:47] abu: Alright lol

[2024-04-12 02:47] Matti: meaning if you can use kdnet

[2024-04-12 02:47] Matti: it is excruciatingly slow

[2024-04-12 02:48] Matti: but on some systems it's the only available option

[2024-04-12 02:48] Matti: [replying to abu: "Yeah I was hoping to debug while not in debug mode"]
does it need to be a physical system?

[2024-04-12 02:49] Matti: exdi would work for this, but I haven't seen it working with bare metal systems

[2024-04-12 02:49] Matti: only qemu and vmware

[2024-04-12 02:49] abu: Yeah I was gonna look at that

[2024-04-12 02:50] Matti: that's not to say it can't be done... I know MS use it internally

[2024-04-12 02:50] abu: I used qemu tcg and it worked suprisingly well but I couldn't launch the file on it for some reason

[2024-04-12 02:50] Matti: for ARM device debugging

[2024-04-12 02:50] Matti: but it'd be very complex to set up for a physical machine, frankly I wouldn't know where to start

[2024-04-12 02:51] Matti: the MS docs don't cover that

[2024-04-12 02:51] Matti: [replying to abu: "I used qemu tcg and it worked suprisingly well but..."]
which file?

[2024-04-12 02:51] abu: [replying to Matti: "which file?"]
A Game

[2024-04-12 02:51] abu: The launcher specifically

[2024-04-12 02:51] abu: Tested with KVM works fine

[2024-04-12 02:51] abu: I'm instead looking into vm introspection

[2024-04-12 02:52] Matti: mm oh yeah, it's one of those tricky games I guess

[2024-04-12 02:52] Matti: that don't like it when you do certain things

[2024-04-12 02:54] Matti: does kvm itself not have a gdb stub, or some other debugging facility?

[2024-04-12 02:55] Matti: I guess it'd just be linux kernel debugging? since kvm is part of the kernel

[2024-04-12 02:57] Matti: another option you could consider (assuming it works with the game...) is virtualbox

[2024-04-12 02:57] Matti: it has a very good debugger since v7.x

[2024-04-12 02:58] Matti: as well as a gdb stub (I think) actually, but I mean its built in debugger

[2024-04-12 03:28] Terry: does anyone know how I can (properly) filter for the mov that is based off relative address only with zydis?

7FF65995F2ED mov [rsi+0x108], eax <-- dont want
7FF65995F2F3 mov [0x00007FF65B9609D8], rsi <- want

[2024-04-12 03:34] Matti: [replying to Terry: "does anyone know how I can (properly) filter for t..."]
the first of these will have `base = rsi` in the operand 0 (dst) info
the second will have `base = none`

[2024-04-12 03:35] Matti: there are some other differences as well, see zydisinfo output
[Attachments: image.png]

[2024-04-12 03:37] Matti: your second instruction isn't valid as is by the way, I smudged it a bit to make it a 32 bit destination address

[2024-04-12 03:37] Terry: hmm maybe im on an outdated version of zydis or something, I cant seem to find the base under the operands or instruction

[2024-04-12 03:37] Terry: [replying to Matti: "your second instruction isn't valid as is by the w..."]
yeah it calcuated the absolute address in the formatter

[2024-04-12 03:38] Matti: ok right, what you need is not the formatter output but the decoder input

[2024-04-12 03:38] Matti: if you want to use zydisinfo anyway

[2024-04-12 03:38] Matti: since the bytes should match

[2024-04-12 03:38] Matti: it's just a more convenient way of looking at the decoder data structures, for me anyway

[2024-04-12 03:39] Terry: yeah i didnt even know zydisinfo existed until now

[2024-04-12 03:39] Matti: but you do need to give it matching input

[2024-04-12 03:41] Matti: [replying to Terry: "yeah i didnt even know zydisinfo existed until now"]
I use it very often, it's a great tool IMO

[2024-04-12 03:41] Matti: even though it's technically "only" a sample that comes with zydis

[2024-04-12 03:48] Terry: [replying to Matti: "there are some other differences as well, see zydi..."]
Okay this should work then, however in the decode "base" is RIP not 0, I think the tool just doesn't have the context so it assigns 0? RIP makes more sense to me

[2024-04-12 03:52] Matti: it'll be a RIP-relative instruction I assume

[2024-04-12 03:52] Matti: show the full bytes

[2024-04-12 03:53] Terry: of just this insturction?
48 89 35 DE160002     - mov [program.exe+8C509D8],rsi

[2024-04-12 03:53] Terry: i mean, it is

[2024-04-12 03:53] Terry: this solution does work perfect for me tho, tyvm!

[2024-04-12 03:54] Matti: alright, it is yeah

[2024-04-12 03:55] Matti: see that's why it's important to feed it the exact bytes you're giving the encoder <:lillullmoa:475778601141403648> what I put in was a guesstimate based off encoding (more or less) your instruction

[2024-04-12 03:55] Matti: [replying to Terry: "this solution does work perfect for me tho, tyvm!"]
nps nps

[2024-04-12 05:14] future_wizard: [replying to Matti: "there are some other differences as well, see zydi..."]
what program is that zi?

[2024-04-12 05:14] future_wizard: zydis, nvm

[2024-04-12 05:18] Matti: yeah, or well specifically the zydisinfo app that comes with it

[2024-04-12 05:18] Matti: some people only ever compile the library so they never realise it exists

[2024-04-12 15:42] Torph: [replying to Matti: "it is excruciatingly slow"]
does this affect boot time for Hyper-V VMs being debugged? I have mine set up with kdcom over a virtual COM port

[2024-04-12 15:48] Matti: if you boot into debug mode, yes

[2024-04-12 15:49] Matti: it will always be slower than it would be otherwise

[2024-04-12 15:49] Matti: this is true for physical machines and VMs

[2024-04-12 15:49] Matti: if you don't boot into debug mode, there is no difference

[2024-04-12 15:49] Matti: it's only *excruciatingly* slow when you start actually debugging though

[2024-04-12 15:50] Matti: otherwise it's just somewhat slower

[2024-04-12 15:50] Torph: [replying to Matti: "it will always be slower than it would be otherwis..."]
is it on the order of several seconds or higher slower boot?

[2024-04-12 15:50] Matti: for me, for sure

[2024-04-12 15:50] Matti: I've not tried it with hyper-v yet but I doubt it will be different

[2024-04-12 15:51] Matti: it's worse on physical machines though

[2024-04-12 15:51] Torph: [replying to Matti: "it's only *excruciatingly* slow when you start act..."]
I'll definitely try kdnet, it seemed fine to me but I've only ever used it on kdcom... maybe some slowdown I attributed to WinDbg was caused by the serial connection

[2024-04-12 15:52] Matti: yeah or just try booting in no-debug mode

[2024-04-12 15:53] Matti: (little known fact: there is actually a `/NODEBUG` boot flag - it doesn't do a whole lot compared to just not booting with `/DEBUG` though)

[2024-04-12 15:54] Matti: in bcdedit terminology that would be `/set debug no`

[2024-04-12 15:54] Matti: probably

[2024-04-12 15:55] Matti: in regular/not-debug mode there definitely shouldn't be a slowdown, even if you have a virtual COM port

[2024-04-12 15:56] Matti: if it's still slow the cause is probably something else

[2024-04-12 15:56] Matti: for me on a physical machine it's pretty noticeable though, it easily adds a minute or more to my boot time

[2024-04-12 15:57] Matti: VMs, a few seconds, just long enough to notice a difference

[2024-04-12 20:36] Torph: [replying to Matti: "yeah or just try booting in no-debug mode"]
I usually don't have a reason to boot in no-debug mode for a Hyper-V VM where I'm testing a driver

[2024-04-12 20:37] Torph: [replying to Matti: "for me on a physical machine it's pretty noticeabl..."]
oh wow

[2024-04-12 22:40] Matti: [replying to Torph: "I usually don't have a reason to boot in no-debug ..."]
I understand that, I'm only saying to do it so you can see for yourself if this is the cause or not

[2024-04-12 22:43] Matti: I usually have a boot menu set up anyway where I can choose between the two (and other things like having the MS hypervisor on or not), so for me this would literally just be choosing a different option in the boot menu

[2024-04-12 22:44] Matti: saves you time setting up kdnet in case this turns out not to be the cause

[2024-04-13 02:45] Torph: [replying to Matti: "I understand that, I'm only saying to do it so you..."]
oh, ok

[2024-04-13 02:45] Torph: [replying to Matti: "I usually have a boot menu set up anyway where I c..."]
I have a boot menu like that too, but the debugger seems to work fine whether I select debug mode or not.

[2024-04-13 03:40] Matti: show your `bcdedit /enum` for the supposed regular (non-debug) boot entry

[2024-04-13 03:40] Matti: as well as `bcdedit /enum {dbgsettings}` and `bcdedit /enum {bootmgr}` maybe

[2024-04-13 03:42] Matti: mm oh yeah and maybe even `bcdedit /enum {hypervisorsettings}` if you use those

[2024-04-13 03:43] Matti: windows doesn't just accept a kd connection by default

[2024-04-13 03:43] Matti: something is making it enable debug mode

[2024-04-13 03:44] Torph: I'll check when I'm on Windows again
I basically just blindly pasted some commands to setup the menu from MSDN, so I probably just copied the wrong line somewhere

[2024-04-13 03:44] Matti: yeah, I think so

[2024-04-13 03:46] Matti: I hate to admit this but I use this abandoned freeware tool pretty often for finding stuff in the BCD store
[Attachments: image.png]

[2024-04-13 03:46] Matti: cause it's so much faster to navigate

[2024-04-13 03:46] Matti: not open source though 😐

[2024-04-13 03:47] Matti: oh it's named bootice... I see it doesn't say that in the ss

[2024-04-13 03:51] Matti: this tool also does UEFI boot entries btw, like linux `efibootmgr`

[2024-04-13 03:52] Matti: I've been wanting to write an open source replacement for ages

[2024-04-13 03:53] Matti: because all tools for both this and BCD editing on windows suck

[2024-04-13 03:53] Matti: well bcdedit doesn't suck, but it's not exactly good at giving you an overview of things

[2024-04-13 03:54] Matti: but... fuck writing GUI code

[2024-04-13 04:03] Torph: lol mood

[2024-04-13 04:04] Torph: could use ImGui? it's not very efficient, but it looks pretty ok for very little effort

[2024-04-13 04:06] Matti: nah sorry, you can't get me to write GUI code with any framework in the world

[2024-04-13 04:06] Matti: but if I were to do it I'd probably stick with plain win32 like this above

[2024-04-13 04:08] Matti: the problem isn't really that MFC is difficult, or tedious (well it is but that's beside the point)

[2024-04-13 04:08] Matti: otherwise yeah I would use imgui, or qt or whatever

[2024-04-13 04:09] Matti: it's more that writing GUI code is just a braindead activity

[2024-04-13 04:09] Matti: it's not interesting

[2024-04-13 04:41] Torph: but wouldn't you rather do it with something that takes very little effort so you spend less time on the braindead activity

[2024-04-13 09:46] Timmy: I haven't tried it yet, but this gui lib seems really good

[2024-04-13 09:46] Timmy: https://libui-ng.github.io/libui-ng/index.html

[2024-04-13 12:08] mrexodia: [replying to Timmy: "https://libui-ng.github.io/libui-ng/index.html"]
This (no-op ) UI takes 500+ LOC <:harold:704245193016344596>
[Attachments: windows.png]

[2024-04-13 12:09] mrexodia: Doesn't mean it's bad of course, might be useful as a small library

[2024-04-13 12:09] mrexodia: But generally this is the pain with UI libraries in C/C++, you always end up writing **a ton** of code and it becomes a major PITA

[2024-04-13 12:12] Timmy: 500 doesn't sound too bad to me

[2024-04-13 12:12] Timmy: my main concern is the build system

[2024-04-13 12:13] Timmy: it's not cmkr

[2024-04-13 14:02] Matti: [replying to Torph: "but wouldn't you rather do it with something that ..."]
no, you misread

[2024-04-13 14:02] Matti: I said I'd rather just not do it

[2024-04-13 14:06] Matti: like, whether I'm using a modern GUI lib or win32 MFC really doesn't matter, this pales in comparison to the tedium inherent to GUI programming itself

[2024-04-13 14:06] Matti: there is no problem to solve

[2024-04-13 14:07] Matti: normally when you write a program, you've got something in mind that you want it to do, i.e. you want it to solve some problem

[2024-04-13 14:08] Matti: and you have to think about how to solve the problem of programming this program

[2024-04-13 14:08] Matti: making a GUI is completely the other way around

[2024-04-13 14:10] Matti: the problem is already solved in my head, the only challenge is putting all the elements in the right place, making things scale, making it work with keyboard navigation, and writing some glue code to invoke the actual program

[2024-04-13 14:10] Matti: and like 20 other tiny shit things that are just obvious when they are wrong in a GUI

[2024-04-13 14:12] Matti: making GUIs is just torture because there is nothing to solve, all you have to do is fight whatever shit tool or framework you're using to do all of the millions of trivial steps

[2024-04-13 14:14] contificate: depends, many applications demand custom components which can be non-trivial and bring a host of problems

[2024-04-13 14:15] Matti: well sure, you can make it non-trivial easily enough

[2024-04-13 14:15] Matti: I don't think making a GUI scale well is trivial at all

[2024-04-13 14:15] Matti: but it's something that I already see in my head

[2024-04-13 14:15] Matti: I know how it needs to work

[2024-04-13 14:16] contificate: always changes as you go

[2024-04-13 14:16] contificate: unless you are describing like trivial widget applications

[2024-04-13 14:16] Matti: the only issue is then figuring out how to do this in the tool/language you're using to make your GUI

[2024-04-13 14:17] Matti: [replying to contificate: "always changes as you go"]
yeah but then I can still make a sketch that incorporates whatever change is required, in less than 5 mins

[2024-04-13 14:17] Matti: and again in my head I already see how it has to work

[2024-04-13 14:17] Matti: it just doesn't

[2024-04-13 14:18] Matti: because you need to do the 10 million boilerplate shit steps first

[2024-04-13 14:18] contificate: gotta bet on non-degenerate frameworks

[2024-04-13 14:18] contificate: like Qt

[2024-04-13 14:18] Matti: I've used Qt

[2024-04-13 14:18] contificate: in actual reality

[2024-04-13 14:18] contificate: many programs just do it entirely custom

[2024-04-13 14:18] Matti: this very beautiful tab in x64dbg was entirely made by me
[Attachments: image.png]

[2024-04-13 14:19] Matti: it was pretty easy

[2024-04-13 14:19] contificate: sublime text uses a GtkWindow but then defers entirely to its own Skia-based UI library

[2024-04-13 14:19] Matti: but I still hated it

[2024-04-13 14:19] contificate: simply managing GDK events

[2024-04-13 14:20] Matti: [replying to contificate: "many programs just do it entirely custom"]
I'm aware of this btw

[2024-04-13 14:20] Matti: I'm not saying making GUIs is trivial

[2024-04-13 14:21] Matti: it's just that the order of problem solving is the wrong way around for me

[2024-04-13 14:21] Matti: so I don't enjoy it

[2024-04-13 14:21] contificate: I can understand the frustration but the issue is

[2024-04-13 14:21] contificate: suppose you didn't use those UI frameworks that you have to kind of fight against

[2024-04-13 14:21] contificate: you'd invent your own which is more work than people imagine

[2024-04-13 14:21] contificate: and then

[2024-04-13 14:21] contificate: you'd run into the same issues eventually

[2024-04-13 14:22] contificate: "ah fuck, I designed this tab focusing garbage in a fucked way"

[2024-04-13 14:22] contificate: "ah fuck, these progress bars don't have indeterminate state animations"

[2024-04-13 14:22] Matti: [replying to contificate: "you'd invent your own which is more work than peop..."]
idk if I would man

[2024-04-13 14:22] contificate: just a proliferation of concerns

[2024-04-13 14:22] Matti: I can live without GUIs

[2024-04-13 14:22] contificate: when you can just say

[2024-04-13 14:22] contificate: "fuck it, no popup notifications then"

[2024-04-13 14:22] Matti: whenever I use linux, I just have a terminal and that's it

[2024-04-13 14:22] contificate: ok windows boy

[2024-04-13 14:23] Matti: cause the  desktop environments and window managers are too shit to bother with

[2024-04-13 14:23] Matti: so

[2024-04-13 14:23] Matti: I type

[2024-04-13 14:23] Matti: like in the DOS days

[2024-04-13 14:23] contificate: WMs are fine honestly

[2024-04-13 14:23] Matti: some people do say this

[2024-04-13 14:24] 25d6cfba-b039-4274-8472-2d2527cb: use gnome. gnome devs know what a desktop needs and everyone else is just a moron

[2024-04-13 14:24] 25d6cfba-b039-4274-8472-2d2527cb: they make the perfect de

[2024-04-13 14:24] Matti: that is true

[2024-04-13 14:24] Matti: gnome devs always know best

[2024-04-13 14:24] contificate: groomed by Windows

[2024-04-13 14:24] contificate: to believe you need to see pointless icons all the time

[2024-04-13 14:24] contificate: and a task bar

[2024-04-13 14:24] contificate: and all other levels of clownage

[2024-04-13 14:25] Matti: I use linux quite a lot you know

[2024-04-13 14:25] 25d6cfba-b039-4274-8472-2d2527cb: gnome removed all of ur features because fuck you? use extensions moron. oh they break every major update? deal with it.

[2024-04-13 14:25] Matti: despite this minor handicap

[2024-04-13 14:25] Matti: like I said I can live without GUIs

[2024-04-13 14:26] Matti: I'd rather just shoot myself than work on fixing linux desktop code

[2024-04-13 14:26] Matti: or

[2024-04-13 14:26] Matti: I can just use linux

[2024-04-13 14:27] Matti: it works without GUI

[2024-04-13 14:27] 25d6cfba-b039-4274-8472-2d2527cb: linux desktop is simple. just say no and when someone asks for reasoning make it up

[2024-04-13 14:27] 25d6cfba-b039-4274-8472-2d2527cb: "uh security concerns, unmaintainable bloat and I wouldn't use it"

[2024-04-13 14:28] Matti: true

[2024-04-13 14:28] Matti: did you know that X can be forwarded over TCP??

[2024-04-13 14:28] Matti: big security risk

[2024-04-13 14:28] Matti: this is why I don't use X

[2024-04-13 14:29] 25d6cfba-b039-4274-8472-2d2527cb: did u know u can programmatically listen to and send key events to windows? uh keylogger anyone??

[2024-04-13 14:29] 25d6cfba-b039-4274-8472-2d2527cb: remove feature

[2024-04-13 14:29] Matti: fuck is that true

[2024-04-13 14:29] 25d6cfba-b039-4274-8472-2d2527cb: configuration files are NIH

[2024-04-13 14:29] Matti: nothing is safe anymore is it

[2024-04-13 14:30] Deleted User: Mods let's keep this chat in the channels context

[2024-04-13 14:30] Deleted User: <:kappa:1115968816812392470>

[2024-04-13 14:31] Matti: mods plural?

[2024-04-13 14:31] 25d6cfba-b039-4274-8472-2d2527cb: just dont snitch bro

[2024-04-13 14:31] Matti: there's only one mod here

[2024-04-13 14:31] Matti: and one wannabe mod

[2024-04-13 14:31] Matti: and me

[2024-04-13 14:31] Deleted User: <:yea:904521533727342632>

[2024-04-13 14:38] Bloombit: Qt and sdl what else do you need

[2024-04-13 14:42] Matti: well see, sdl is already quite a different thing

[2024-04-13 14:42] Matti: you can make a video game engine in it

[2024-04-13 14:42] Matti: that is fun

[2024-04-13 14:42] Matti: qt is just an improved visual basic

[2024-04-13 14:44] Bloombit: I use a gui to make qt guis lol

[2024-04-13 14:44] Bloombit: Or sdl

[2024-04-13 14:45] Brit: [replying to Matti: "qt is just an improved visual basic"]
you're about to summon duncan

[2024-04-13 14:45] Matti: I'm not saying qt is as shit as VB

[2024-04-13 14:45] Matti: it's clearly much better

[2024-04-13 14:45] Matti: it's just... still doing the same uninteresting thing

[2024-04-13 14:46] Matti: damn, how could I forget... I have **another** thing I made in Qt

[2024-04-13 14:46] Matti: the IDA about dialog ofc
[Attachments: image.png]

[2024-04-13 14:46] Matti: I have quite a lot of Qt experience, see

[2024-04-13 14:47] Matti: it's pretty nice to use as far as GUI frameworks go

[2024-04-13 14:47] Brit: I love that tweet cap

[2024-04-13 14:47] Matti: I just hate making GUIs

[2024-04-13 14:49] Matti: I do still have VB6
[Attachments: image.png]

[2024-04-13 14:50] Matti: don't ask me why

[2024-04-13 14:50] Lewis: [replying to Matti: "I do still have VB6"]
LOL

[2024-04-13 14:50] Lewis: I use the c# one

[2024-04-13 14:50] Lewis: its same

[2024-04-13 14:50] Matti: 
[Attachments: image.png]

[2024-04-13 14:50] Lewis: LOL

[2024-04-13 14:50] Lewis: fook that

[2024-04-13 14:51] Matti: it's still pretty much the same thing as winforms today

[2024-04-13 14:51] Matti: WPF is different though

[2024-04-13 14:51] mrexodia: Yeah it’s more garbage

[2024-04-13 14:51] mrexodia: Fuck that xaml shit

[2024-04-13 14:51] Matti: yeah I can't say I like it

[2024-04-13 14:52] mrexodia: At least be honest about it and use html

[2024-04-13 14:52] Matti: I've tried to

[2024-04-13 14:52] Lewis: [replying to Matti: "it's still pretty much the same thing as winforms ..."]
yeah

[2024-04-13 14:52] mrexodia: .net 4.7 with winforms is peak UI efficiency

[2024-04-13 14:52] mrexodia: Just drag, double click on the button and do the thing

[2024-04-13 14:52] mrexodia: No fuss, just hacky UIs

[2024-04-13 14:52] Lewis: yeah fr

[2024-04-13 14:53] Lewis: so useful

[2024-04-13 14:54] mrexodia: Just unfortunate C++ doesn’t have something light that is similar

[2024-04-13 14:54] Matti: wdym

[2024-04-13 14:54] Matti: there's UWP
[Attachments: image.png]

[2024-04-13 14:54] Matti: ez

[2024-04-13 14:55] mrexodia: <:harold:704245193016344596>

[2024-04-13 14:55] Matti: this is real code btw

[2024-04-13 14:55] Matti: real generated code but still

[2024-04-13 14:55] Matti: it takes 2 minutes to compile a basic empty form app

[2024-04-13 14:55] Matti: on my 16 core PC

[2024-04-13 14:56] Lewis: shame

[2024-04-13 14:56] Lewis: C++ user friendly UI builder would be cool

[2024-04-13 14:56] Lewis: but I guess imgui is kinda similar despite there is no UI to build it

[2024-04-13 14:57] Matti: qt creator is the closest I've seen to what you mean

[2024-04-13 14:57] Matti: but I have to say, for productivity it can't match winforms

[2024-04-13 14:57] Matti: though again I hate both

[2024-04-13 14:58] mrexodia: [replying to Lewis: "but I guess imgui is kinda similar despite there i..."]
Imgui is fundamentally kind of broken for making regular applications though

[2024-04-13 14:58] Lewis: [replying to mrexodia: "Imgui is fundamentally kind of broken for making r..."]
shame

[2024-04-13 14:59] Lewis: [replying to Matti: "though again I hate both"]
LOL

[2024-04-13 14:59] Lewis: on another note: I got some cookies

[2024-04-13 15:00] Lewis: all we got is oat milk

[2024-04-13 15:00] Lewis: <:08_emote_cry:1185232463203684442>

[2024-04-13 18:32] Ignotus: what are you, santa claus?

[2024-04-14 07:09] Deleted User: [replying to Matti: "the IDA about dialog ofc"]
ilfak guilfanov himself.

[2024-04-14 07:09] Deleted User: can i get autogram