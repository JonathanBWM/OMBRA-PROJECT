# June 2025 - Week 2
# Messages: 21

[2025-06-07 20:23] bugdigger: How can i reverse engineer BE or EAC using HyperDbg or CheatEngines DBVM?
I saw that both of those require to disable DSE (by using EfiGuard which patches some field) or by putting my OS in test signing mode.
But aren't both of those methods easily detectable by anticheats? Test signing mode obviously but they also check integrity of critical kernel structures which are KPP protected?

Am I missing something?
Sorry if this is obvious to some ppl I am still learning.

Btw do i have to disable secure boot to use efiguard?

[2025-06-08 02:20] NOTWOLF: I have a cheat engine that is DBVM and works i can scan and edit values, but i cant script, also i realised that the addresses are not the real ones, is there a way of going around or if i can get my hands on the BE disabler or something

[2025-06-08 04:15] daax: [replying to bugdigger: "How can i reverse engineer BE or EAC using HyperDb..."]
Sign their drivers with a cert that isn’t blocked, or use CKS. Alternatively, downgrade to a vulnerable Windows, find a default driver (not blacklisted) that allows km rd/write and disable DSE (1809/1903 are good for leveraging itw bugs that have PoCs if you are just looking to do some c&p); EfiGuard doesn’t work with secure boot by default, so you will need to disable it to load and toy with to start.

[2025-06-08 07:47] Horsie: For Introspecting windows stuff, i wonder how difficult it would be to set up a very very rudimentary debugger on hyperv

[2025-06-08 07:48] Horsie: just a break, read, write interface.

[2025-06-08 07:48] Horsie: Something that you can use via exdi perhaps

[2025-06-08 07:49] Horsie: Thats the reason I've never really looked at ACs much. Back when I was actively interested in bypassing them, I didnt have the sufficient skill necessary to reverse tbeir static protections (probably still dont).

[2025-06-08 07:50] Horsie: The looming threat of a hwid ban had kept ke away from reversing them but not cheating as I usually ran my stuff on a lower level than the ac

[2025-06-08 07:51] Horsie: Though I rememver that EAC (some games) /BE  used to be fine running under KVM.

[2025-06-08 07:51] Horsie: [replying to Horsie: "For Introspecting windows stuff, i wonder how diff..."]
Theres also a meme debugger some chinese guys made, for SMM

[2025-06-08 07:52] Horsie: https://csis.gmu.edu/ksun/publications/Malt-sp2015.pdf

[2025-06-08 07:55] Horsie: The source isnt public but its fairly easy to implement on your own.

[2025-06-08 08:29] valium: okay i dont know anything about hardware debuggers or ring 0 debuggers but this looks cool

https://github.com/behnamshamshirsaz/CrackMaster
[Embed: GitHub - behnamshamshirsaz/CrackMaster: x86/x64 Ring 0/-2 System Fr...]
x86/x64 Ring 0/-2 System Freezer/Debugger. Contribute to behnamshamshirsaz/CrackMaster development by creating an account on GitHub.

[2025-06-08 08:30] valium: this guy posts about his debugger on twitter too and has some demos

[2025-06-08 08:30] valium: i think he also provides a commercial version

[2025-06-08 08:55] Horsie: I had a stroke 3 times while going through the readme

[2025-06-08 08:57] Horsie: Seems interesting but since he has no source available I dont see much value in it

[2025-06-08 08:59] Horsie: [replying to valium: "okay i dont know anything about hardware debuggers..."]
Have you tried it?

[2025-06-08 09:00] valium: [replying to Horsie: "Have you tried it?"]
no

[2025-06-08 19:09] iPower: [replying to bugdigger: "How can i reverse engineer BE or EAC using HyperDb..."]
hyperdbg will straight up crash against EAC

[2025-06-08 19:34] koyz: [replying to iPower: "hyperdbg will straight up crash against EAC"]
wdym one of the best hypervisors that is definitely not overhyped with definitely no design flaws would be DoS'd by arguably one of the easier to mitigate DoS'??? /s