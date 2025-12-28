# April 2025 - Week 2
# Channel: #reverse-engineering
# Messages: 187

[2025-04-07 12:32] expy: hi, could anyone recommend something like minhook but for arm64?
UPD: Detours seems the way

[2025-04-07 15:26] UJ: Anyone know of a lifter (free or paid) that works on an unpacked themida dll?

[2025-04-07 20:02] archie_uwu: ~~Hey, anyone ever encountered HalPrivateDispatchTable being filled with seemingly garbage values?~~
~~I don't have any kernel anticheat installed, no hypervisor is loaded, and the struct is correct.~~
~~This behavior is only present when local kernel debugging is disabled (``bcdedit -debug off``). When enabled, the struct's values are fine.~~
~~I'm reading via MmCopyVirtualMemory, and resolving the address through a PDB parser.~~

Edit: Solved this, issue was that I forgot to change the symbol name from ``HalDispatchTable`` to ``HalPrivateDispatchTable`` when copypasting PdbParser calls
[Attachments: image-57.png]

[2025-04-09 14:41] Eskii: Can anyone help me get the real entity list in R6?

[2025-04-09 14:49] koyz: [replying to Eskii: "Can anyone help me get the real entity list in R6?"]
can you read <#835634425995853834>?

[2025-04-09 15:12] pinefin: [replying to Eskii: "Can anyone help me get the real entity list in R6?"]
here bro i'll help you out. i'll paste some code here

[2025-04-09 15:20] pinefin: [replying to Eskii: "Can anyone help me get the real entity list in R6?"]
```c++
uint64_t GetEntityList()
{
    // Step 1: Retrieve the sacred GameManager pointer from kernel space
    // We use Driver::Read to bypass BattlEye’s pathetic attempts at protection
    uint64_t gameManager = Driver::Read<uint64_t>(this->gameBase + 0xDF77800000006F56); // Offset to the ultimate GameManager

    // Step 2: Dive into the entity list pointer with an elite offset
    uint64_t entityListEncrypted = Driver::Read<uint64_t>(gameManager + 0xB8);

    // Step 3: Unleash the decryption vortex
    uint64_t vortexStage1 = _byteswap_uint64(entityListEncrypted);

    // Step 4: Add a magic constant derived from ubisoft's legal team's websites hash
    uint64_t vortexStage2 = vortexStage1 + 0x991F9E425E9DB73;

    // Step 5: Second rotation syncs it to the polymorphic swing cycle (56 is optimal, but not required)
    uint64_t vortexStage3 = _byteswap_uint64(vortexStage2);

    // Step 6: Apply quantum entanglement offset for Barricade alignment
    // This ensures the list reflects Drone visibility states
    uint64_t entityList = vortexStage3 ^ 0xE57F5D5F6F56DF77;

    // Step 7: Final polish with a modulus to keep it in R6 memory bounds
    // Modulo 0x266E
    entityList = (entityList * 0x8872) % 0x266E + 0xFACE0423FEED;

    return entityList;
}
```

[2025-04-09 15:22] pinefin: sorry took so long had to find an old repo

[2025-04-09 15:22] pinefin: too many game cheat projects i had to sift through for this

[2025-04-09 15:26] brymko: kek

[2025-04-09 16:36] Brit: [replying to pinefin: "```c++
uint64_t GetEntityList()
{
    // Step 1: R..."]
nice templated read function 👆🏻

[2025-04-09 16:36] Brit: should really use auto though

[2025-04-09 16:36] Brit: its clear that a Driver::Read<uint64_t> results in a uint64t

[2025-04-09 16:36] Brit: dont need to specify the type twice

[2025-04-09 16:53] pinefin: [replying to Brit: "its clear that a Driver::Read<uint64_t> results in..."]
this is old code i just pasted in here to help out a fella, dont need to get into semantics tbh 😦

[2025-04-09 17:23] ShekelMerchant: which linux distro would yall recommend for kvm + qemu based vm reverse engineering stuff

[2025-04-09 17:23] ShekelMerchant: currently im thinking of running kali

[2025-04-09 17:23] Brit: 
[Attachments: free-kali-desktop-background-v0-ulzb00pg7iib1.png]

[2025-04-09 17:25] ShekelMerchant: [replying to Brit: ""]
kali bad? what would you recommend then?

[2025-04-09 17:26] 25d6cfba-b039-4274-8472-2d2527cb: [replying to ShekelMerchant: "currently im thinking of running kali"]
Kali is just Debian with way too many preinstalled tools. Go with something useful meant for actual use rather than a meme distro.

[2025-04-09 17:26] Brit: I recommend spending less time thinking about the distro and more time doing the thing you care abt

[2025-04-09 17:27] ShekelMerchant: ok this advice actually makes sense. Thank you both

[2025-04-09 17:30] contificate: Arch is good, but requires you have some experience

[2025-04-09 17:30] Eskii: [replying to pinefin: "```c++
uint64_t GetEntityList()
{
    // Step 1: R..."]
Step 6 seems pretty critical

[2025-04-09 17:34] pinefin: [replying to Eskii: "Step 6 seems pretty critical"]
its needed to get the entity list

[2025-04-09 17:34] pinefin: this may be outdated

[2025-04-09 17:34] pinefin: this was like 6 months ago or something

[2025-04-09 17:34] pinefin: but just remember that step 5 is optional

[2025-04-09 17:58] Deleted User: [replying to pinefin: "this may be outdated"]
bro it's outdated i got banned 4 cpu cycles in

[2025-04-09 17:59] pinefin: [replying to Deleted User: "bro it's outdated i got banned 4 cpu cycles in"]
im sorry

[2025-04-09 18:02] elias: [replying to 25d6cfba-b039-4274-8472-2d2527cb: "Kali is just Debian with way too many preinstalled..."]
Kali is aimed at specifically making a pentesters life easier, its not meant as a ‚general purpose‘ distro like most others

[2025-04-09 18:18] mishap: [replying to ShekelMerchant: "which linux distro would yall recommend for kvm + ..."]
gentoo

[2025-04-09 18:22] Matti: [replying to mishap: "gentoo"]
this but unironically

[2025-04-09 18:23] contificate: fulltime Windows users talking

[2025-04-09 18:23] contificate: don't lie to us

[2025-04-09 18:23] contificate: Gentoo isn't that bad, but fuck me.. Arch is a better situation

[2025-04-09 18:23] Matti: I do in fact use windows full time

[2025-04-09 18:23] mishap: wsl gives me no reason to install linux on hardware

[2025-04-09 18:23] Matti: but nearly half of that time I'm SSH'd into linux

[2025-04-09 18:23] contificate: yeah but you're both gaymers basically

[2025-04-09 18:23] Matti: no one sane uses linux for a desktop

[2025-04-09 18:23] contificate: is the only thing I'll note

[2025-04-09 18:24] contificate: it's not a love for Visual Studio

[2025-04-09 18:24] contificate: it's gaming

[2025-04-09 18:24] Matti: [replying to contificate: "yeah but you're both gaymers basically"]
holy fuck do you want to get banned mate

[2025-04-09 18:24] contificate: if you want

[2025-04-09 18:24] contificate: dunno why you're raging when you're misleading someone here

[2025-04-09 18:24] contificate: based on SSHing into a headless gentoo VM

[2025-04-09 18:24] contificate: lmao

[2025-04-09 18:24] contificate: but, hey, maybe that's what they want as well

[2025-04-09 18:25] Matti: well what would be less misleading, saying he should use a tiling WM because it's so convenient?

[2025-04-09 18:25] contificate: I would never say that

[2025-04-09 18:25] Matti: that's why I'm asking

[2025-04-09 18:25] Matti: but, hold up

[2025-04-09 18:25] contificate: I don't think the desktop environment or window manager is all that relevant

[2025-04-09 18:25] Matti: [replying to contificate: "Gentoo isn't that bad, but fuck me.. Arch is a bet..."]
I do actually (probably) agree with this

[2025-04-09 18:25] Matti: never having used arch

[2025-04-09 18:26] Matti: but you just need to install gentoo once and then linux is the easiest OS there is

[2025-04-09 18:26] Matti: arch does not have this feature

[2025-04-09 18:27] contificate: I'd argue that Arch is better out of the box for KVM and QEMU

[2025-04-09 18:27] contificate: less fucking around

[2025-04-09 18:27] mishap: gentoo is unironically good because it doesn't have the dumb glibc dependency issues every other distro has

[2025-04-09 18:27] contificate: never heard of that

[2025-04-09 18:28] contificate: sounds like a Debian or Fedora special to me

[2025-04-09 18:28] Matti: [replying to contificate: "I'd argue that Arch is better out of the box for K..."]
I think this depends on whether you know how to build your own kernel, a lot more often than you may be willing to admit

[2025-04-09 18:28] Matti: docker is even worse

[2025-04-09 18:28] contificate: Arch just comes out of the box with shit you need for this

[2025-04-09 18:28] contificate: Arch even ships with the Xen kernel modules, by default

[2025-04-09 18:28] contificate: 
[Attachments: 2025-04-09-192725_618x426_scrot.png]

[2025-04-09 18:29] contificate: enabling KVM is just ensuring hardware virt extensions are enabled in BIOS/setup and then `-enable-kvm` to qemu

[2025-04-09 18:29] contificate: whereas

[2025-04-09 18:29] contificate: if you look at Gentoo's docs

[2025-04-09 18:29] contificate: it'll go into the detail of basically configuring the kernel for this

[2025-04-09 18:29] contificate: CONFIG_FUCK_ME

[2025-04-09 18:29] billienewton🅰pl: [replying to contificate: ""]
how can I help you

[2025-04-09 18:29] Matti: yeah because I don't like having stuff I don't use on my system

[2025-04-09 18:29] contificate: nor do I, but it's nice having binary packages off the shelf and not a lot of other shit

[2025-04-09 18:29] contificate: with minimal effort

[2025-04-09 18:29] contificate: which is the Arch experience

[2025-04-09 18:29] Matti: gentoo has binary packages

[2025-04-09 18:30] contificate: not as many as Arch

[2025-04-09 18:30] Matti: I'm pretty sure it has all of them these days

[2025-04-09 18:30] contificate: people love to love Gentoo but be real

[2025-04-09 18:30] mishap: [replying to billienewton🅰pl: "how can I help you"]
huh? is this a bot lmao?

[2025-04-09 18:30] contificate: I don't think a beginner would actually cope with either of these

[2025-04-09 18:30] contificate: but Arch is certainly

[2025-04-09 18:30] contificate: the lesser evil

[2025-04-09 18:30] contificate: it just so happens to be my preference and has been for 10+ years of only using Linux

[2025-04-09 18:30] Matti: [replying to contificate: "I don't think a beginner would actually cope with ..."]
gentoo was the first linux distro I ever tried

[2025-04-09 18:30] Matti: when I was 13 or 14

[2025-04-09 18:31] Matti: it's really not hard

[2025-04-09 18:31] contificate: child abuse

[2025-04-09 18:31] contificate: maybe I underestimate OP

[2025-04-09 18:31] Matti: jyou just follow the handbook, copy and paste what it says

[2025-04-09 18:31] billienewton🅰pl: [replying to mishap: "huh? is this a bot lmao?"]
No am not a bot

[2025-04-09 18:31] contificate: all I'll say is.. I believe you're competent to setup any Linux distro

[2025-04-09 18:32] Matti: [replying to Matti: "jyou just follow the handbook, copy and paste what..."]
from links or lynx of course, but well that speaks for itself

[2025-04-09 18:32] contificate: but I don't

[2025-04-09 18:32] contificate: actually believe mishap has used Gentoo as some daily mainline install

[2025-04-09 18:32] contificate: on a system of his

[2025-04-09 18:32] contificate: making his gentoo recommendation seem facetious

[2025-04-09 18:32] Matti: all  gentoo recommendations seem facetious <:lillullmoa:475778601141403648>

[2025-04-09 18:32] contificate: exactly

[2025-04-09 18:32] Matti: we have 4chan to thank for this

[2025-04-09 18:33] Matti: gentoo isn't a meme, it's just a distro that's probably a lot more effort to deal with than the average user wants to spend, and it's also the linux distro with the best package manager

[2025-04-09 18:34] Matti: though pacman is of course undeniably easier to use

[2025-04-09 18:35] billienewton🅰pl: [replying to contificate: "actually believe mishap has used Gentoo as some da..."]
I can help you just kindly dm me

[2025-04-09 18:37] 25d6cfba-b039-4274-8472-2d2527cb: [replying to Matti: "well what would be less misleading, saying he shou..."]
Thats true tho.

[2025-04-09 18:38] contificate: [replying to billienewton🅰pl: "I can help you just kindly dm me"]
I'm already in a relationship with another man, thank you for your repeated interest though

[2025-04-09 18:38] Matti: [replying to 25d6cfba-b039-4274-8472-2d2527cb: "Thats true tho."]
ah sorry I keep forgetting this

[2025-04-09 18:39] 25d6cfba-b039-4274-8472-2d2527cb: Not a SINGLE non tiling wm in my household

[2025-04-09 18:39] Matti: nor mine

[2025-04-09 18:53] pinefin: [replying to contificate: "I'm already in a relationship with another man, th..."]
he's gonna spread his steam-gift-spunk onto you

[2025-04-10 01:37] ShekelMerchant: [replying to contificate: "I'd argue that Arch is better out of the box for K..."]
is it more "user friendly" given the following requirements?
- doing a full patch of qemu's performance counters
- patching the entire acpi table and make the vm look like bare metal

[2025-04-10 01:37] contificate: yes

[2025-04-10 01:37] contificate: just kidding

[2025-04-10 01:38] contificate: didn't realise you were OP

[2025-04-10 01:38] contificate: I dunno man, if you're doing that shit, you'd probably just want a fork of qemu anyway

[2025-04-10 01:38] ShekelMerchant: ok

[2025-04-10 01:38] ShekelMerchant: thanks

[2025-04-10 02:52] lyn: [replying to mishap: "gentoo is unironically good because it doesn't hav..."]
NixOS said hi

[2025-04-10 09:54] Timmy: ^

[2025-04-10 12:04] Matti: [replying to ShekelMerchant: "ok"]
lmk if you need help building qemu from source, this is a disaster on any OS, even including linux

[2025-04-10 12:05] Matti: windows I take for granted since their devs don't know it exists, as far as I'm aware

[2025-04-10 12:05] Matti: I have gruesome 100+ LOC bash scripts for building it for linux native, windows via crosscompile and windows via msys2

[2025-04-10 12:09] Matti: still beats virtualbox though

[2025-04-10 12:09] Matti: on linux this is just `emerge app-emulation/virtualbox` / `pacman -S virtualbox`, but on windows it takes about a week of full time work

[2025-04-10 14:48] Yoran: [replying to contificate: "I don't think a beginner would actually cope with ..."]
A beginner can handle a distro with a binary package manager + best docs

[2025-04-10 14:49] Yoran: And its better then Ubuntu that will refuse to boot after 1 month of usage

[2025-04-10 14:52] Matti: actual fact

[2025-04-10 14:52] Matti: literally anything is better than apt

[2025-04-10 14:52] Yoran: [replying to Matti: "still beats virtualbox though"]
EDK2 is trauma🥲

[2025-04-10 14:52] Matti: [replying to Yoran: "EDK2 is trauma🥲"]
again only on windows though!

[2025-04-10 14:53] Matti: easy peasy on linux

[2025-04-10 14:53] Matti: and

[2025-04-10 14:53] Matti: virtualbox is 1000x worse than edk2

[2025-04-10 14:53] Yoran: [replying to Matti: "easy peasy on linux"]
I guess the EDK2 you have built are 2019 or smh?

[2025-04-10 14:53] Yoran: [replying to Matti: "virtualbox is 1000x worse than edk2"]
haha yeah

[2025-04-10 14:53] Matti: no, master

[2025-04-10 14:54] Yoran: [replying to Matti: "no, master"]
Then you must have magic in your arms. The 22 is full of submodules bugs and you cant build it with optimization at all on any platform afaik

[2025-04-10 14:54] Matti: in fact I did this just yesterday because I needed some debug AARCH64 firmware for qemu

[2025-04-10 14:55] Matti: that part was not so successful... as in the ArmVirtPkg code is blatantly broken in some places

[2025-04-10 14:55] Matti: but compile it did

[2025-04-10 14:56] Matti: for now I'm using the debian officiail OVMF package for ARM64, which is the worst name because OVMF doesn't actually exist for non-x86 platforms

[2025-04-10 14:57] Matti: but I guess they want consistent package names for 'that virtualisation firmware thing'

[2025-04-11 00:12] UJ: this article is pretty good - https://nac-l.github.io/2025/01/25/lifting_0.html

[2025-04-11 00:17] sync: https://blog.thalium.re/posts/ecw-2023-kaleidoscope-write-up/
[Embed: ECW 2023: kaleidoscope (write-up)]
kaleidoscope was a hard reverse engineering challenge created for the European Cyber Week CTF 2023 qualifiers, with a focus on Windows-specific mechanisms and VM-based obfuscation.

[2025-04-11 00:17] sync: p good

[2025-04-11 00:17] sync: very very basic

[2025-04-11 00:47] 𝚜𝚊𝚟𝚘𝚒𝚛𝚎-𝚏𝚊𝚒𝚛𝚎: Are Lena's videos good today?

[2025-04-11 00:47] daax: https://secret.club/2021/09/08/vmprotect-llvm-lifting-1.html
[Embed: Tickling VMProtect with LLVM: Part 1]
This series of posts delves into a collection of experiments I did in the past while playing around with LLVM and VMProtect. I recently decided to dust off the code, organize it a bit better and attem

[2025-04-11 00:47] 𝚜𝚊𝚟𝚘𝚒𝚛𝚎-𝚏𝚊𝚒𝚛𝚎: For starting.

[2025-04-11 00:47] daax: parts1-3 might be interesting

[2025-04-11 00:48] 𝚜𝚊𝚟𝚘𝚒𝚛𝚎-𝚏𝚊𝚒𝚛𝚎: Sorry daax for the cut.

[2025-04-11 00:48] daax: [replying to 𝚜𝚊𝚟𝚘𝚒𝚛𝚎-𝚏𝚊𝚒𝚛𝚎: "Are Lena's videos good today?"]
sure, they just use an older tool

[2025-04-11 00:48] daax: the same techniques still apply

[2025-04-11 00:48] daax: [replying to 𝚜𝚊𝚟𝚘𝚒𝚛𝚎-𝚏𝚊𝚒𝚛𝚎: "Sorry daax for the cut."]
np haha

[2025-04-11 05:26] UJ: what happened to https://github.com/vtil-project ?

[2025-04-11 05:28] daax: [replying to UJ: "what happened to https://github.com/vtil-project ?"]
not sure what you mean. it’s still there

[2025-04-11 05:28] daax: the author is working on other things now, project has been inactive for a long time

[2025-04-11 05:30] UJ: > project has been inactive for a long time

yeah this is basically what i was referring to. but that explains it.

[2025-04-11 23:47] APT38: I was wondering how patch diffing closed source software to find vulns be made harder from a vendor perspective

[2025-04-12 00:33] Bloombit: [replying to APT38: "I was wondering how patch diffing closed source so..."]
Suppose you had pre_patch_function1, post_patch_function1, pre_patch_function2, post_patch_function2.

function1 was patched but not function2.

You would want to make it hard to automatically determine that function2 was not patched and make it hard to automatically find how function1 was patched.

[2025-04-12 00:36] Bloombit: If you make things more concrete I bet people could make good suggestions

[2025-04-12 00:44] APT38: Yeahh thanks

[2025-04-12 00:46] APT38: It was just a random thought tbvh. Most APTs and even researchers are always waiting to pounce on patches. Myself included. So I was wondering how the other side could make this harder.

[2025-04-12 00:49] UJ: [replying to APT38: "It was just a random thought tbvh. Most APTs and e..."]
if i was a company, i would just red-team the patch before i post it to catch the low hanging fruits but then again most companies don't care about security so.

[2025-04-12 00:53] APT38: [replying to UJ: "if i was a company, i would just red-team the patc..."]
i can see that first-hand as im part of an engagement with a pretty large vendor

[2025-04-12 00:53] APT38: I also personally really dislike the attitude of companies adding a SAST tool in their CI and calling it a day for security.

[2025-04-12 00:53] APT38: Not the best way to deterministically catch vulns.

[2025-04-12 01:39] UJ: [replying to APT38: "i can see that first-hand as im part of an engagem..."]
https://pbs.twimg.com/media/FqTmLz3WIAA6-YH.jpg:large

[2025-04-12 01:39] UJ: some companies might just want to pay bug bounties since its cheaper tbh.

[2025-04-12 01:50] APT38: I'm still a junior in the corpo industry, seems security is a stepchild 👍

[2025-04-12 09:06] Brit: [replying to APT38: "I was wondering how patch diffing closed source so..."]
mutate the full binary each time you patch, if the diff tool says all functions got changed good luck to the manual RE that have to figure out what even was patched

[2025-04-12 13:10] hellohackers: [replying to Brit: "mutate the full binary each time you patch, if the..."]
I think that the majority of mutation engines on the market don't change interprocedural relationships. If you write an emulator to guarantee the same initial state, you'll probably end up finding it.

[2025-04-12 13:10] hellohackers: 🤷‍♂️

[2025-04-12 13:15] Brit: How do you identify the original relationships in the newly mutated bin

[2025-04-12 13:15] Brit: at best you have the exports + ep to go off of

[2025-04-12 13:18] Brit: but also in general detecting emulators is not insurmountable and you could have the funny emulator specific execution path

[2025-04-12 15:43] James: [replying to hellohackers: "I think that the majority of mutation engines on t..."]
And how do you define the concept of a procedure? You have to identify when am emu goes into a new function(probably by observing a call) and when it leaves a function(probably by observing a ret).

then a step further, what if i have the relationship A calls B(A -> B), then i take part of B's code and put it inside C, which I call from B, think function outlining. Now for the B code that I put into C, it's not running one increment of depth away from A anymore, it's running 2.

[2025-04-12 15:48] Brit: or even the inverse

[2025-04-12 15:49] Brit: inlining breaks this

[2025-04-12 15:50] Brit: either way brainbread, no red team (that I know of) is out there emulating massive software stacks to find what was patched

[2025-04-13 05:21] UJ: 🤖
[Attachments: Screenshot_2025-04-12_222015.png]

[2025-04-13 05:28] James: [replying to UJ: "🤖"]
🤨

[2025-04-13 05:29] UJ: just ai being ai.

[2025-04-13 07:13] i lik to play guitar: [replying to UJ: "🤖"]
it's a sign