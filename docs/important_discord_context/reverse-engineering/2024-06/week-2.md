# June 2024 - Week 2
# Channel: #reverse-engineering
# Messages: 63

[2024-06-04 15:42] Windy Bug: Windows, how would you detect that a process crashed? Hypothetically, imagine you want to detect it in driver and you have the power to use undoc offsets.

And don't say werfault.exe

[2024-06-04 15:44] brymko: werfault.exe

[2024-06-04 16:01] diversenok: https://x.com/PetrBenes/status/1794747827303682102
[Embed: Petr Beneš (@PetrBenes) on X]
Since asking bizarre questions attracts so much engagement, let's step it up a notch :)

In Windows, how would you detect that a process crashed? Hypothetically, imagine you want to detect it in drive

[2024-06-04 16:16] Windy Bug: yep I saw this too lol wanted to share it here

[2024-06-04 16:16] Windy Bug: see if anyone comes up with different approaches

[2024-06-04 17:00] Brit: [replying to Windy Bug: "Windows, how would you detect that a process crash..."]
load matti's bootkit (we don't wanna deal with pg), replace the idt, unwind, if no exception handler for your unwind, the process is abt to crash.

[2024-06-04 17:03] JustMagic: [replying to Brit: "load matti's bootkit (we don't wanna deal with pg)..."]
bajillion apps with crashpad say hi

[2024-06-04 17:03] Brit: filter them

[2024-06-04 17:03] Brit: who cares

[2024-06-04 17:03] Brit: no one said this had to be practical

[2024-06-04 17:03] Brit: <:yea:904521533727342632>

[2024-06-04 17:07] Brit: oh wait, I see what you mean

[2024-06-04 17:07] Brit: well let's hope the process in question isn't crashpadded

[2024-06-04 18:40] dullard: [replying to brymko: "werfault.exe"]
Not _always_, WER can be turned off  <:Kappa:1082189237178351666>

[2024-06-04 18:41] dullard: I see it turned off in enterprise environments as the dmp it produces can contain creds etc

[2024-06-04 20:06] daax: [replying to Windy Bug: "Windows, how would you detect that a process crash..."]
track process termination via etw on the event (30B & 501902) or km audit api terminate process; psacquireprocessexitsynchronization and poll rundown ref count (bleh); notification routines (simplest); self debug then install cb on debugobject close; pspprocessrundownworkitem rebind; registerapplicationrecoverycallback (which uses wer to call callback so); use WMI InstanceDeletionEvent (user mode not km); setwindowshookex w/ destroy callback (same as previous); KeWaitForMultipleObjects or ZwWaitForSingleObject/RegisterWaitForSingleObject (effectively copy the internals to construct threadpool to wait on object); lpc termination ports; i suppose you could be very naive and create a threadpool to poll the number of handles to the \KnownDlls & \KnownDlls32 directory — which is the number of processes (usually) and then try to correlate a previous run with the process that crashed (dumb). some are better than others, but the methods are plenty, and this is barring injecting, messing with exception table for the process, using mpeng notifications, and probably some shenanigans you can do with wnf callbacks.

[2024-06-04 20:28] szczcur: [replying to daax: "track process termination via etw on the event (30..."]
alr mr walking encyclopedia. didn’t know about the instancedeletionevent that’s cool. <https://learn.microsoft.com/en-us/windows/win32/wmisdk/--instanceoperationevent>

[2024-06-04 20:40] repnezz: [replying to daax: "track process termination via etw on the event (30..."]
many of those catch “standard” process exists  as well don’t  they ?

[2024-06-04 20:48] daax: [replying to repnezz: "many of those catch “standard” process exists  as ..."]
yeah, but without default error reporting enabled i was just tossing out what might be a starting point. if you just cant use werfault.exe but the minidumps are there than just a directory change notification on minidumps folder or an attempt to load faultrep.dll. i assume no werfault means no mechanism to log crashes at all, so etw or something else that’s a bit scuffed <:PES2_Shrug:513352546341879808>

[2024-06-05 07:55] irql: anyone know what does DVRT on ntoskrnl?

[2024-06-05 07:56] irql: thought it was MxRelocatePageTables at first, but maybe MmInitializeCfg or something?

[2024-06-05 07:56] irql: pretty sure its gota be in MmInitNucleas or whatever its called

[2024-06-05 07:56] irql: maybe someone knows 👁️

[2024-06-05 12:37] zdart: does anyone have installer of Intel System Studio (Windows to Windows) before NDA was enforced?

[2024-06-05 14:15] szczcur: [replying to irql: "anyone know what does DVRT on ntoskrnl?"]
where did you see this referenced?

[2024-06-05 14:17] irql: hm?

[2024-06-05 14:17] irql: dynamic value relocation im talking about

[2024-06-05 14:17] irql: the shit that moves page tables all over the show

[2024-06-05 14:31] szczcur: [replying to irql: "dynamic value relocation im talking about"]
oh i misread your question

[2024-06-05 14:31] szczcur: read like bad english 😂

[2024-06-05 14:31] irql: oh my yea 🤣

[2024-06-05 14:31] irql: my bad

[2024-06-05 14:32] irql: anyone know what "relocates" ntoskrnl ** lmfao

[2024-06-05 14:34] szczcur: mirebasedynamicrelocationregions then miperformdynamicfixups

[2024-06-05 14:35] szczcur: [replying to irql: "anyone know what "relocates" ntoskrnl ** lmfao"]
bl applies static relocs, then kernel mm applies dynamic relocs. p sure it happens in miinitializesystemva

[2024-06-05 14:36] szczcur: <@148095953742725120> is this accurate?

[2024-06-05 14:41] irql: oh bro

[2024-06-05 14:41] irql: my man

[2024-06-05 14:41] irql: yea, lemme look into those 👁️

[2024-06-05 14:41] daax: [replying to szczcur: "<@148095953742725120> is this accurate?"]
im not matti, but yeah. MiRebaseDynamicRelocationRegions -> MiApplyDynamicRelocations -> (VslApply | LdrApply)

[2024-06-05 14:42] irql: [replying to szczcur: "bl applies static relocs, then kernel mm applies d..."]
ah yea, i saw that

[2024-06-05 14:42] irql: appreciate it 🙏

[2024-06-05 14:43] szczcur: [replying to daax: "im not matti, but yeah. MiRebaseDynamicRelocationR..."]
ah not called miperformdynamicfixups anymore then

[2024-06-05 14:44] szczcur: i couldve opened ida oh well lol

[2024-06-05 14:45] szczcur: [replying to irql: "appreciate it 🙏"]
np what're you lookin' into it for? out of curiosity

[2024-06-05 14:46] irql: wish I could say something really cool -- love the idea behind all the DVRT stuff but ehhh

[2024-06-05 14:46] irql: I have a hypervisor, and having windows map page tables in the same place is kinda annoying

[2024-06-05 14:46] irql: i could probably come up with a better solution, like maybe even using la57, but I kinda like using their mappings alongside my own

[2024-06-05 14:46] szczcur: aah gotcha

[2024-06-05 14:47] irql: couldnt find them from scanning through symbosl anyways lol

[2024-06-05 14:47] irql: should've tried searching for "dynamic"

[2024-06-05 14:48] irql: thought it might be in winload initially

[2024-06-05 18:24] Matti: [replying to daax: "im not matti, but yeah. MiRebaseDynamicRelocationR..."]
my name is matti and I approve this message

[2024-06-05 18:26] Matti: [replying to szczcur: "ah not called miperformdynamicfixups anymore then"]
this still exists, it is called by MiApplyDynamicRelocations

[2024-06-05 19:14] szczcur: [replying to Matti: "this still exists, it is called by MiApplyDynamicR..."]
it seems in the latest win11 its called something else but not a big deal. all the same

[2024-06-05 19:19] Matti: oh I was actually looking at 10.0.14393.0 on purpose, because that's when they added support for this

[2024-06-05 19:20] Matti: but yeah definitely possible they renamed it, or some inlining somewhere caused it to get lost

[2024-06-07 08:09] repnezz: correct me if I’m wrong but disk I/O  cannot be performed at dispatch level since it takes infinite time from the CPU perspective and the caller thread has to be suspended and rescheduled 
I’m wondering , why  file system calls (say ZwReadFile) are permitted only at passive level , and paging I/O can be done at apc level irql?  suspending a thread is done via apc doesn’t it? so how does it work when the caller is already at apc level?

[2024-06-07 20:16] CreedTX: Hi Everyone,

I have C# code under .NET Framework 4.8, but it is protected with BouncyCastle.Crypto and possibly Fake obfuscation as well. Since you all are very knowledgeable about this, could you help me disassemble it? I need the source code of the application. I'm willing to pay for your work if necessary.

Thank you.

[2024-06-08 02:41] Lamentomori: hey guys whats up

[2024-06-08 02:52] ash: [replying to Lamentomori: "hey guys whats up"]
despair and suffering 💀

[2024-06-08 03:28] averageavx512enjoyer: anyone has a clue on how often HAL_PRIVATE_DISPATCH->HalClearLastBranchRecordStack gets called?

[2024-06-08 05:56] iPower: [replying to averageavx512enjoyer: "anyone has a clue on how often HAL_PRIVATE_DISPATC..."]
should be called constantly because it's called inside SwapContext