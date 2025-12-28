# April 2025 - Week 1
# Channel: #reverse-engineering
# Messages: 198

[2025-04-01 00:33] valium: [replying to Humpty/Tony: "So luci4 is right, you _are_ trying to upload an m..."]
i think hes saying like when you install an apk from a non trusted source, play protect warns you about it and asks whether you want to install it or not and also whether you want to send the sample to google

[2025-04-01 00:33] Humpty/Tony: Play protect does that? Never had this happen

[2025-04-01 00:34] valium: [replying to Humpty/Tony: "Play protect does that? Never had this happen"]
happens on mine

[2025-04-01 00:34] Humpty/Tony: [replying to valium: "happens on mine"]
Pixel?

[2025-04-01 00:34] valium: [replying to Humpty/Tony: "Pixel?"]
samsung

[2025-04-01 00:34] Humpty/Tony: Uh weird I'm on samsung too and never had that

[2025-04-01 00:35] Humpty/Tony: 🤷‍♂️

[2025-04-01 00:35] valium: is it enabled in settings?

[2025-04-01 00:37] Humpty/Tony: [replying to valium: "is it enabled in settings?"]

[Attachments: SmartSelect_20250331_203716_Settings.jpg]

[2025-04-01 00:38] valium: [replying to Humpty/Tony: ""]
in the playstore settings

[2025-04-01 00:38] valium: not here

[2025-04-01 00:38] valium: [replying to Humpty/Tony: ""]
open playstore, tap on your account at top right, go to play protect, tap the settings at top right

[2025-04-01 00:40] Humpty/Tony: [replying to valium: "open playstore, tap on your account at top right, ..."]
Yup its enabled

[2025-04-01 00:40] valium: [replying to Humpty/Tony: "Yup its enabled"]
yeah u can see 2 options there

[2025-04-01 00:40] Humpty/Tony: Yep both are enabled

[2025-04-01 00:40] valium: this actually happened to me when i was installing a custom discord apk

[2025-04-01 00:40] valium: it was aliucord or smthing

[2025-04-01 00:40] Humpty/Tony: Never had this interfere 🤷‍♂️

[2025-04-01 00:41] valium: [replying to Humpty/Tony: "Never had this interfere 🤷‍♂️"]
maybe, but for certain apps it always does interfere

[2025-04-01 00:41] valium: https://www.reddit.com/r/revancedextended/comments/13iua9k/google_play_protect_warning_is_shown_should_i/
[Embed: From the revancedextended community on Reddit: Google Play Protect ...]
Explore this post and more from the revancedextended community

[2025-04-01 00:41] valium: heres an example

[2025-04-01 00:42] valium: this is like ms smartscreen

[2025-04-01 00:42] Humpty/Tony: Yeah I get what it does lol

[2025-04-01 00:42] valium: like when u download and the browser blocks the download

[2025-04-01 00:42] valium: the op basically doesnt want this popup to occur ig

[2025-04-01 00:42] Humpty/Tony: I understand very well what it does 😅

[2025-04-01 00:43] valium: oh, i think u were saying that he wanted to upload/publish a payload infected app to playstore

[2025-04-01 00:45] Humpty/Tony: The way he initialed framed the initial question suggested that :p

[2025-04-01 00:47] Humpty/Tony: [replying to Bored engineer: "anyone knows how to bypass google play protect onc..."]
Did you try loading the malicious code from a second stage thats fetched dynamically?

[2025-04-01 00:49] valium: [replying to Bored engineer: "anyone knows how to bypass google play protect onc..."]
im pretty sure its for skid purposes, but you can use ollvm to obfuscate your code/payload in your apk, last time i used ollvm it worked

[2025-04-01 01:09] Humpty/Tony: On a sidenote msfvenom payloads fucking glow in the dark lol

[2025-04-01 01:14] valium: [replying to Humpty/Tony: "On a sidenote msfvenom payloads fucking glow in th..."]
yeah they are most likely to get sigged lol

[2025-04-01 04:53] Bored engineer: [replying to valium: "i think hes saying like when you install an apk fr..."]
exactly

[2025-04-01 04:55] Bored engineer: [replying to Humpty/Tony: "Did you try loading the malicious code from a seco..."]
well tried dropper as well with app install permissions still detects it !! but when I am deploying my custom made APK from the android studio it simply gives a warning and do not block it and gives the option to install the app -->  which was not happening earlier for the app generated via msfvenom

[2025-04-01 04:59] Bored engineer: [replying to valium: "im pretty sure its for skid purposes, but you can ..."]
Mostly for Red team work ! there was blog post earlier not only available in exploit db as pdf , you might enjoy reading it title: from apk to golden ticket 

https://www.exploit-db.com/docs/english/44032-from-apk-to-golden-ticket.pdf

[2025-04-01 05:00] Bored engineer: [replying to valium: "yeah they are most likely to get sigged lol"]
they were sigged earlier too but was bypassable using encoders but these days it is also detecting those

[2025-04-01 16:42] valium: [replying to Bored engineer: "Mostly for Red team work ! there was blog post ear..."]
did you tried ollvm?

[2025-04-01 16:42] Bored engineer: [replying to valium: "did you tried ollvm?"]
nope not yet

[2025-04-01 16:42] valium: [replying to Bored engineer: "nope not yet"]
what is the payload about? reverse shell?

[2025-04-01 16:43] Bored engineer: [replying to valium: "what is the payload about? reverse shell?"]
it's a full blown RAT

[2025-04-01 16:43] Bored engineer: [replying to Bored engineer: "it's a full blown RAT"]
full regalia mode

[2025-04-01 16:43] valium: https://github.com/0xlane/ollvm-rust
[Embed: GitHub - 0xlane/ollvm-rust: out-of-tree llvm obfuscation pass plugi...]
out-of-tree llvm obfuscation pass plugin (dynamically loadable by rustc). || rust toolchain with obfuscation llvm pass. - 0xlane/ollvm-rust

[2025-04-01 16:43] valium: maybe try virtualizing it with this

[2025-04-02 16:42] Tr1x: [replying to Bored engineer: "well tried dropper as well with app install permis..."]
You need to avoid applying modifications which are drastically increasing your APK's entropy as that is a massive red flag, your app is probably close to nearly 100% entropy when doing all that bullshit to it. All you need is to destroy basic signature scanning operations behold that unless you fine tune it or you use it correctly most other obfuscation techniques are not worth applying (If you're doing this for red teaming, it is over once they get your APK no matter how much anti analysis techniques used) the goal is to fly under the radar. I recommend using an obfuscation technique like substitution/mutation and small amounts of MBAs as depending on the entropy calculations shouldn't go crazy, this is ultimately going to break any signature scanning based operations.

As for defeating the other techniques used in detection systems good luck as if you're having trouble with this you're surely going to have massive trouble with the others.

[2025-04-02 16:43] Tr1x: Also: https://obfuscator.re/
[Embed: Open Obfuscator: A free and open-source solution for obfuscating m...]
A free and open-source obfuscator for mobile applications

[2025-04-02 16:44] Bored engineer: [replying to Tr1x: "You need to avoid applying modifications which are..."]
Have u done fud apk ?

[2025-04-02 16:46] Tr1x: [replying to Bored engineer: "Have u done fud apk ?"]
I don't really use that term. I build, break and research shit. If I run into a problem I reverse the system causing that problem for me and mitigate it.

[2025-04-02 16:47] Bored engineer: [replying to Tr1x: "I don't really use that term. I build, break and r..."]
okiz

[2025-04-02 17:28] pinefin: [replying to Tr1x: "You need to avoid applying modifications which are..."]
thats a name i havent heard in a while

[2025-04-02 17:28] pinefin: tr1x

[2025-04-02 18:40] valium: i would say write your payload in rust, that shit is enough for obfuscation <:topkek:904522829616263178>

[2025-04-02 22:42] InboardMars354: anyone know about programming with lua and CE?

[2025-04-03 01:16] Xyll: [replying to InboardMars354: "anyone know about programming with lua and CE?"]
Sure, what's up?

[2025-04-03 01:19] InboardMars354: [replying to Xyll: "Sure, what's up?"]
i have problems with mono_invoke_method because return a null value when i try to invoke some function that return a class, but if the value that should return is whatever type of value (like int32, char, string.....) return it without problem, but if i try to call a function that return a class, always return a null value,

[2025-04-03 01:21] InboardMars354: for example,  if i invoke this, i get a numeric value
[Attachments: mono-invoke-method-return-a-null-value-when-i-try-to-invoke-v0-9pnyt6zk4ise1.png]

[2025-04-03 01:22] InboardMars354: but if i invoke this, i get a null value
[Attachments: mono-invoke-method-return-a-null-value-when-i-try-to-invoke-v0-wiircm5p4ise1.png]

[2025-04-03 01:22] InboardMars354: the class have these fields
[Attachments: mono-invoke-method-return-a-null-value-when-i-try-to-invoke-v0-7hop6vt05ise1.png]

[2025-04-03 01:23] Xyll: You're seeing this on any functions that return a class/object (non-primitive type) - or just specific function(s)?

[2025-04-03 01:24] InboardMars354: [replying to Xyll: "You're seeing this on any functions that return a ..."]
with functions that return a class

[2025-04-03 01:24] InboardMars354: this is the class where the method is allow
[Attachments: mono-invoke-method-return-a-null-value-when-i-try-to-invoke-v0-mwdkoekw4ise1.png]

[2025-04-03 01:25] InboardMars354: and this value captured from `currency` with `mono_class_enumFields`
[Attachments: mono-invoke-method-return-a-null-value-when-i-try-to-invoke-v0-x7lyvkz75ise1.png]

[2025-04-03 01:31] Xyll: Not quite sure what's going wrong here. Unfortunately I myself haven't used Mono at all with CE. Just CE & LUA. Wish I could help you out more here.

You might have some luck searching CE forums or GuidedHacking

[2025-04-03 01:33] InboardMars354: [replying to Xyll: "Not quite sure what's going wrong here. Unfortunat..."]
unfortunately no 😭, i search in each forum about it and nobody talk about it, im working in it with pure lua and address that i capture from the JIT and the memory map

[2025-04-03 01:35] InboardMars354: but thxs for the support 👍

[2025-04-03 01:37] Xyll: Yeah ofc. If something comes to my mind I'll let you know. Also be interested in hearing what the solution was once you get it sorted

[2025-04-03 01:42] InboardMars354: [replying to Xyll: "Yeah ofc. If something comes to my mind I'll let y..."]
ok

[2025-04-03 06:59] UJ: Hey guys, i have a DLL that is packed with Themida (and some functions are protected with virtualization/obfuscation). I am able to get the unpacked binary using https://github.com/ergrelet/unlicense. I'm also able to get a dump of the unpacked dll when its running just by using scylla. 

Problem is with the unlicense version, its rebuilt as much as can be but the global variable initialization doesnt work because it uses TLS https://stackoverflow.com/questions/56458581/why-msvc-thread-safe-initialization-of-static-local-variables-use-tls to initialize the variable and the msvc guard for each variable evaluates to false on injection of dll.

So far i've been just manually patching the asm to remove the msvc/tls checks (inc the _Init_thread_header, _Init_thread_footer which im not sure is even safe) and just assign the initial value to the variable and it seems to work so far. just wanted to see if anyone has dealth with this before and know of a faster/better way of doing this?

[2025-04-03 07:14] Matti: I can't help with your actual issue, but just in case you were wondering: the flag to prevent this shit from being autogenerated for every potentially concurrently accessible static variable in **your own code** is `/Zc:threadSafeInit-` in MSVC

[2025-04-03 07:14] Matti: obviously little help given that your binary clearly wasn't compiled with this flag... but yea

[2025-04-03 07:18] Matti: oh the SO post you linked even mentions this too

[2025-04-03 07:19] UJ: yeah looks like its a requirement from the c++ standard.

[2025-04-03 07:19] Matti: the *really* long explanation about how TLS var init works is here: http://www.nynaeve.net/?p=190
[Embed: Thread Local Storage, part 8: Wrap-up]

[2025-04-03 07:20] Matti: in brief, it varies depending on the windows version (pre-vista or post), the compiler and the CRT

[2025-04-03 07:20] Matti: and IMO there is no good version

[2025-04-03 07:21] Matti: [replying to UJ: "yeah looks like its a requirement from the c++ sta..."]
yeah it is, but I've had this flag enabled by default ever since it was added to MSVC

[2025-04-03 07:21] Matti: so I've never actually checked this but... my guess would be that clang is likely to generate less terrible code than MSVC

[2025-04-03 07:22] Matti: just because it tends to do that as a general rule

[2025-04-03 07:22] Matti: idk how it solves this problem precisely though

[2025-04-03 07:23] Matti: you'd also need to use the llvm CRT presumably

[2025-04-03 07:26] UJ: yeah ik it only runs once but the overhead of this thing is insane for something that isnt needed in the majority of cases. 

https://github.com/Chuyu-Team/VC-LTL/blob/a879b308d3d68c7d298db0747203937da235a047/src/14.21.27702/vcruntime/thread_safe_statics.cpp#L240

[2025-04-03 07:26] Matti: yep I absolutely agree

[2025-04-03 07:26] Matti: this is why I have the flag enabled by default for my own code

[2025-04-03 07:27] Matti: it saves megabytes of code on something as big as say UE5

[2025-04-03 07:27] Matti: of binary code, rather

[2025-04-03 07:28] Matti: it's a surprise +20 MB after you compile the code

[2025-04-03 07:29] Matti: (made up number, idr exactly but it was an outrageous amount for UE even with monolithic linking and LTO)

[2025-04-03 07:40] UJ: [replying to Matti: "it's a surprise +20 MB after you compile the code"]
ironically, the repo linked above with that tls impl is `An elegant way to compile lighter binaries.`

[2025-04-03 07:41] Matti: 😔

[2025-04-03 13:57] f00d: are there any major differences between MSVC 8.0 CRT and current?

[2025-04-03 13:58] f00d: im thinking of making a tiny crt based on that, if i remember correctly it's 10-20kb and current is like 80kb

[2025-04-03 13:58] f00d: with just return 0 from main

[2025-04-03 14:08] Matti: [replying to f00d: "are there any major differences between MSVC 8.0 C..."]
yes

[2025-04-03 14:08] Matti: I mean the filesize does kind of tell you this

[2025-04-03 14:08] Matti: it's not just zero bytes I'm afraid

[2025-04-03 14:08] f00d: yeah but i don't think u need that much

[2025-04-03 14:08] Matti: I agree

[2025-04-03 14:09] Matti: II don't even link against the CRT when writing user mode code

[2025-04-03 14:09] f00d: there's like error messages and other strings that basically noone "uses"

[2025-04-03 14:09] f00d: nice

[2025-04-03 14:09] Matti: well it depends

[2025-04-03 14:10] Matti: if I were writing GUI stuff I might reconsider this position

[2025-04-03 14:10] f00d: for a long time i patched and fixed the MSVC 6.0 CRT and used that, no troubles except one new operator overload and RTC debug or something like that

[2025-04-03 14:10] Matti: but generally what I write that runs in user mode is only for testing other things I wrote that run in kernel mode

[2025-04-03 14:10] f00d: but that was only for x86-32

[2025-04-03 14:12] Matti: operator new <:wow:762710812904914945>

[2025-04-03 14:12] Matti: yeah you may need a CRT if you want to do that heh

[2025-04-03 14:12] Matti: even if it's just a minimal one

[2025-04-03 14:13] Matti: C++ exceptions, RTTI, same

[2025-04-03 14:13] f00d: yeah contructors etc

[2025-04-03 15:47] pinefin: [replying to Matti: "yeah you may need a CRT if you want to do that heh"]
you can make your own with winapi HeapAlloc

[2025-04-03 15:48] pinefin: HeapFree

[2025-04-03 15:48] pinefin: 🔥

[2025-04-03 15:48] pinefin: 🔥

[2025-04-03 15:48] pinefin: given, this isnt a good method to do so

[2025-04-03 15:48] pinefin: but it works

[2025-04-03 15:49] pinefin: https://github.com/guilryder/clavier-plus/blob/b8b1bc040382c491a066457e59ca169c728527df/StdAfx.cpp#L36
[Embed: clavier-plus/StdAfx.cpp at b8b1bc040382c491a066457e59ca169c728527df...]
Clavier+ keyboard shortcuts manager for Windows. Contribute to guilryder/clavier-plus development by creating an account on GitHub.

[2025-04-03 15:49] pinefin: found on github

[2025-04-03 16:41] f00d: new/delete is the smallest portion of crt

[2025-04-03 17:01] pinefin: [replying to f00d: "new/delete is the smallest portion of crt"]
of course buddy, its stepping stones...

[2025-04-03 17:01] pinefin: i was just telling matti how that's possible with doing it this way

[2025-04-03 17:01] pinefin: maybe i read it wrong

[2025-04-03 17:02] pinefin: i have some minimal stl/crt remakes on my github that got me by, if you need anything like what matti said (c++ exc / rtti / etc) you would need to do more heavy lifting and digging

[2025-04-03 17:03] pinefin: https://github.com/pinefin/stl_remakes
[Embed: GitHub - pinefin/stl_remakes]
Contribute to pinefin/stl_remakes development by creating an account on GitHub.

[2025-04-03 17:03] f00d: i meant i needed to implement multiple operator new overloads which is just calling malloc again

[2025-04-03 17:03] pinefin: [replying to pinefin: "https://github.com/guilryder/clavier-plus/blob/b8b..."]
afaik those are covered here

[2025-04-03 17:03] pinefin: oh

[2025-04-03 17:03] f00d: i don't get why people make intrinsic functions and then call it a crt

[2025-04-03 17:04] pinefin: because if you want to remove crt, you still need the barebones intrinsics for everything to run correctly

[2025-04-03 17:04] pinefin: what is it

[2025-04-03 17:04] pinefin: cxxframehandler4

[2025-04-03 17:04] pinefin: well

[2025-04-03 17:04] pinefin: no bad example ~~because i think you just nop it or something liek that~~

[2025-04-03 17:06] pinefin: anyways im just here to say, i just needed the barebones intrinsics to run without linking crt. for eh i just used a veh

[2025-04-03 17:09] pinefin: i see what you're trying to do

[2025-04-03 17:13] f00d: VC6 CRT
[Attachments: image.png]

[2025-04-03 17:14] pinefin: from further investigation

[2025-04-03 17:14] pinefin: malloc / free were previously just straight implemented in crt, in the latest they're delegated to windows heap management

[2025-04-03 17:14] pinefin: so

[2025-04-03 17:14] pinefin: HeapAlloc/HeapFree are the correct use cases like i said

[2025-04-03 17:14] pinefin: file management im p sure was added from now and 8.0

[2025-04-03 17:15] pinefin: (given ive only been programming for 5-6 years, 8.0 is older than me)

[2025-04-03 17:15] f00d: [replying to pinefin: "HeapAlloc/HeapFree are the correct use cases like ..."]
yes it has a bit more to it but in the end it calls HeapAlloc

[2025-04-03 17:15] pinefin: std::mutex/std::thread now use windows apis for it

[2025-04-03 17:15] pinefin: [replying to f00d: "yes it has a bit more to it but in the end it call..."]
of course

[2025-04-03 17:15] pinefin: i had that remade at one point

[2025-04-03 17:15] pinefin: let me see if i have it to send here

[2025-04-03 17:16] pinefin: i went through remaking the heap delegate

[2025-04-03 17:17] pinefin: huh

[2025-04-03 17:17] pinefin: maybe not on this computer

[2025-04-03 17:18] pinefin: oh fuck i sold the laptop that those projects were on

[2025-04-03 17:18] pinefin: shit

[2025-04-03 17:19] pinefin: i'll find the project i found that info from though

[2025-04-03 18:49] James: i think you are confusing some terms...

[2025-04-03 18:49] James: there is the crt

[2025-04-03 18:49] James: C RunTime

[2025-04-03 18:49] James: and STL

[2025-04-03 18:49] James: standard template library

[2025-04-03 18:49] James: or maybe IM confusing the terms

[2025-04-03 18:49] James: idk

[2025-04-03 19:15] x86matthew: [replying to f00d: "VC6 CRT"]
i hope you #define _MT if you're using the msvc++ 6.0 CRT, it wasn't thread-safe by default lol

[2025-04-03 19:16] f00d: always

[2025-04-03 19:16] x86matthew: 8.0 was a big improvement over 6, the standards were basically non-existent in msvc 6.0

[2025-04-03 19:16] x86matthew: the ide was great though

[2025-04-03 19:16] f00d: ye

[2025-04-03 19:55] Matti: [replying to pinefin: "i was just telling matti how that's possible with ..."]
I mean I know how to implement operators new and delete <:lillullmoa:475778601141403648>

[2025-04-03 19:56] Matti: aren't there literal implementations on cppreference.com

[2025-04-03 19:56] Matti: anyway, those aren't what make the CRT huge nowadays as was already pointed out

[2025-04-03 19:57] Matti: [replying to x86matthew: "8.0 was a big improvement over 6, the standards we..."]
and yeah 8.0 >>>> 6.0

[2025-04-03 19:57] Matti: it was the first semi usable CRT I would say

[2025-04-03 19:57] Matti: and it even still came with a makefile and all

[2025-04-03 19:58] Matti: [replying to James: "or maybe IM confusing the terms"]
no that's correct

[2025-04-03 19:58] Matti: you can use STL features (though far from all) without the CRT and vice versa

[2025-04-03 19:59] Matti: [replying to pinefin: "malloc / free were previously just straight implem..."]
no they never were

[2025-04-03 19:59] Matti: they just wrap win32 APIs

[2025-04-03 19:59] Matti: which wrap NT APIs

[2025-04-03 20:00] Matti: the heap management itself (because you don't want to allocate virtual memory regions for each malloc call) is also done by the ntdll heap manager

[2025-04-03 20:00] pinefin: [replying to Matti: "no they never were"]
they probably allocated a big chunk and then managed heap on their own

[2025-04-03 20:00] pinefin: thats just what i read

[2025-04-03 20:00] Matti: yeah, that's ntdll doing that

[2025-04-03 20:01] Matti: `RtlAllocateHeap()` /  `RtlFreeHeap()`

[2025-04-03 20:01] pinefin: ye, what im saying is, they call HeapFree/HeapAlloc now, previously they allocated huge chunk for heap and then partitioned it out whenever using new/delete

[2025-04-03 20:02] pinefin: at least how i understood it

[2025-04-03 20:02] pinefin: idk how it works now

[2025-04-03 20:02] Matti: no I think you're just misunderstanding who is doing what

[2025-04-03 20:02] pinefin: oh

[2025-04-03 20:02] Matti: your interpretation is right

[2025-04-03 20:02] Matti: but malloc does nothing, and neither does HeapAlloc

[2025-04-03 20:02] Matti: all of the heap is implemented in ntdll

[2025-04-03 20:03] Matti: there might be 2 or 3 lines for spec compliance in each C/C++ function

[2025-04-03 20:03] Matti: but mostly they just forward

[2025-04-03 20:04] Matti: as a general rule, anything in windows user mode that actually works is located in ntdll in the end

[2025-04-05 17:46] UJ: Im not too familiar with every little edge case of .dll/PE files. I want to add a new section (executable) to a dll file at the end and then when injecting, the process closes with heap corruption (nothing else changed).

My understanding is that this should be safe since adding it at the end, the other sections relative positions wont change and the code should just work as it and the diff looks fine (new dll is on the left, the sc in the new section isnt even coming into play yet, exception happens earlier). any ideas of where to look next?

```c#
https://secanablog.wordpress.com/2020/06/09/how-to-add-a-section-to-a-pe-file/

using PeNet;
using System.IO;
  
namespace StripSection
{
    class Program
    {
        static void Main(string[] args)
        {
            using var peFile = new PeFile("calc.exe");
      
            // Add section with 100 bytes in size
            peFile.AddSection(".newSec", 100, (ScnCharacteristicsType)0x40000040);
 
            // Save the changed binary to disk
            File.WriteAllBytes("calc-newsec.exe", peFile.RawFile.ToArray());
        }
    }
}
```
[Attachments: Screenshot_2025-04-05_104603.png]

[2025-04-05 18:34] JustMagic: [replying to UJ: "Im not too familiar with every little edge case of..."]
look at the PE headers rather than a hexdump

[2025-04-05 18:40] archie_uwu: CFF Explorer is a good program to do that ^

[2025-04-05 18:47] UJ: [replying to archie_uwu: "CFF Explorer is a good program to do that ^"]
yep, when i just did it in cff explorer, it made it past where the heap corruption happened so i think this works.

[2025-04-05 18:50] valium: you can use pe bear too

[2025-04-05 19:08] baro kar: hi everyone, anyone in here fiddled around with the windows handle table (aheList)? as in DKOM? removing my window from the kernel side works but removing it from the usermode cache seems to give me a bluescreen, does anyone have any informations about this? thanks:)

this method works on w11 21h2-23h2 but w10 (20h2-22h2) and w11 24h2 have this usermode cache in a NO_ACCESS region

[2025-04-06 13:07] baro kar: [replying to baro kar: "hi everyone, anyone in here fiddled around with th..."]
fixed, my deletion in kernel space was just so quick, the real window wouldnt get cached