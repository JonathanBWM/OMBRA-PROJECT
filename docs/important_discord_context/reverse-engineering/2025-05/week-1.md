# May 2025 - Week 1
# Channel: #reverse-engineering
# Messages: 158

[2025-05-01 15:25] BlitzByte: I'm starting to think it'll be easier to just reverse engineer this program based on what it does over USB instead of rebuilding the program through decompiling it

[2025-05-01 16:57] Matti: [replying to BlitzByte: "I'm starting to think it'll be easier to just reve..."]
I have no clue of the context of what you're talking about
and it's possible you already know how to do all of this
but if not check out https://freebsdfoundation.org/wp-content/uploads/2021/11/Simple_USB_Driver_for_FreeBSD.pdf

[2025-05-01 16:58] Matti: TLDR: use usbpcap to reverse protocol, then write <whatever> using it

[2025-05-01 16:59] Matti: I wouldn't recommend freebsd like they did here... it's mistakenly licensed as BSD instead of GPL

[2025-05-01 16:59] Matti: but other than that great tutorial tbh

[2025-05-01 18:10] BlitzByte: it's a shitty chinese USB camera I got off of AliExpress. I thought I could RE the application to reimplement it, but it's probably going to be easier to just figure out how the application talks to thw camera by sniffing that traffic and reimplement that

[2025-05-01 18:12] BlitzByte: and that honestly sounds like more fun

[2025-05-01 18:13] Matti: yeah I agree with both

[2025-05-01 18:14] Matti: you're not exactly going to be MITM'ing TLS traffic to google.com

[2025-05-01 18:15] Matti: it's hard to say for sure, but I'm guessing you've already looked at the FW code at least briefly in order to consider this route

[2025-05-01 18:16] Matti: aliexpress devices could generally be running just about any kind of ISA and if you can't read it, you're already fucked

[2025-05-01 18:17] daax: [replying to diversenok: "I haven't looked much into it, just saw that LSM i..."]
Yeah, I recall seeing it a while back, went back and looked it's under IPrivateRpc/ISessionManagerInternal and you're right. cbf to reverse the args lol. if only they'd make it easy! seems there might be a way to do it via winsta exports too.

[2025-05-01 18:20] daax: 
[Attachments: image.png]

[2025-05-01 18:28] daax: something strange i've always wondered is why RPC doesn't allow denial of calls based on a minimum client authentication level. you can do it the in the if-callback but it seems like relying on someone to implement the check to grant/deny access versus a simple "specify this minimum level in the registration"

[2025-05-01 18:29] Matti: you know the RPC funcs are at least publicly exported by name right? although IDK why there are so many different interfaces in one module (lsm.dll)

[2025-05-01 18:30] Matti: I always use rpcview for this kinda stuff

[2025-05-01 18:30] daax: [replying to Matti: "you know the RPC funcs are at least publicly expor..."]
yeah

[2025-05-01 18:30] Matti: but yeah, decompiling is nasty

[2025-05-01 18:30] daax: but the types of the args aren't available and it's all indirect calls and bs

[2025-05-01 18:30] Matti: ```c
long Proc2_RpcCreateSession(
    [in]struct Struct_286_t* arg_1, 
    [in]/* enum_16 */ short arg_2, 
    [in]long arg_3, 
    [in]long arg_4, 
    [out]long *arg_5, 
    [out]struct Struct_22_t* arg_6);
```

[2025-05-01 18:30] Matti: yea

[2025-05-01 18:30] daax: was able to find the args (the actual types) to it through winsta funny enough

[2025-05-01 18:30] Matti: I can't believe it but I don't even have a single private pdb for lsm.dll

[2025-05-01 18:31] Matti: ah cool

[2025-05-01 18:31] Matti: yeah that works more often than you'd think

[2025-05-01 18:31] Matti: just find the types in some other semi related module's PDB

[2025-05-01 18:31] daax: yeah usually my goto

[2025-05-01 18:32] Matti: or in combase/wintypes.pdb since they seem to be black holes for types

[2025-05-01 18:32] daax: all i was seeing in lsm was a bunch of indirect bullshit and i wanted to spend 5 mins to see sthg not 5 hrs yk <:Kappa:794707301436358686>

[2025-05-01 18:32] daax: [replying to Matti: "or in combase/wintypes.pdb since they seem to be b..."]
rip uxtheme

[2025-05-01 18:33] Matti: lsm.dll is relatively new annoyingly enough

[2025-05-01 18:33] Matti: in win 7 at least it was lsm.exe

[2025-05-01 18:33] Matti: not sure about 8/8.1

[2025-05-01 18:33] Matti: and whether it had an RPC interface or something else

[2025-05-01 18:34] daax: [replying to Matti: "lsm.dll is relatively new annoyingly enough"]
the dll seems to be earliest available that i can see win10 1507

[2025-05-01 18:34] daax: btw this is a great little toolkit from akamai

[2025-05-01 18:34] daax: https://github.com/akamai/akamai-security-research/tree/main/rpc_toolkit

[2025-05-01 18:35] Matti: yeah I mean, it doesn't really make a difference unless they're very similar I suppose... in which case, why was there a need to change to a hosted DLL

[2025-05-01 18:35] Matti: I'm gonna guess the RPC interface changed a shit ton between those versions anyway

[2025-05-01 18:36] Matti: [replying to daax: "https://github.com/akamai/akamai-security-research..."]
just say no to RPC

[2025-05-01 18:36] Matti: and use wholesome ALPC

[2025-05-01 18:37] daax: [replying to Matti: "and use wholesome ALPC"]
wholesome is a choice

[2025-05-01 18:37] daax: for describing it

[2025-05-01 18:37] Matti: it is wholesome

[2025-05-01 18:40] daax: [replying to Matti: "it is wholesome"]
unrelated but
[Attachments: image.png]

[2025-05-01 18:41] daax: love running into this type of shit in windows bins

[2025-05-01 18:41] daax: under some unassuming api name <:lillullmoa:475778601141403648>

[2025-05-01 18:41] daax: QueryMaxSessions in winsta

[2025-05-01 18:41] daax: warbird moment

[2025-05-01 18:41] Matti: kek

[2025-05-01 18:42] Matti: you'd never guess what function we **don't** want you to look into <:hmm:475800667177484308>

[2025-05-01 18:42] opium: <:WeSmart:356949019119058945>

[2025-05-01 18:43] Matti: [replying to Matti: "it is wholesome"]
avg rpc client call vs avg ALPC client call
[Attachments: image.png, image.png]

[2025-05-01 18:44] Matti: no wonder this shit is so huge **and** slow

[2025-05-01 18:45] Matti: all IPC in windows that matters in any way is based on ALPC, including unfortunately RPCRT

[2025-05-01 18:45] Matti: but also worker factories and the thread pool APIs

[2025-05-01 18:47] Matti: the only thing it doesn't do is networking, but if you need that you've got bigger problems anyway

[2025-05-01 18:48] daax: [replying to Matti: "the only thing it doesn't do is networking, but if..."]
ASUS loves their "networking rpc"

[2025-05-01 18:50] avx: [replying to daax: "under some unassuming api name <:lillullmoa:475778..."]

[Attachments: befrDYV.png]

[2025-05-01 18:50] Matti: yeah if you use rpcrt4.dll for that I'm gonna guess it's gonna be the same story except it'll wrap an NDIS call under a 1000 layers of abstraction instead of an ALPC call

[2025-05-01 18:50] daax: [replying to avx: ""]
it's funny since it's just string "encryption" for their getprocaddress

[2025-05-01 18:51] daax: no idea why they need it

[2025-05-01 18:51] daax: there

[2025-05-01 18:51] Matti: and ALPC >>>>>>> NDIS for perf <:hmm:475800667177484308>
why do networking, it's just too slow

[2025-05-01 18:51] Matti: [replying to daax: "it's funny since it's just string "encryption" for..."]
based

[2025-05-01 18:51] Matti: someone got paid to write that

[2025-05-01 18:52] avx: [replying to daax: "it's funny since it's just string "encryption" for..."]
WHAT <:megajoy:847100430059175976>

[2025-05-01 18:52] avx: im cryuing

[2025-05-01 18:53] daax: yes lol and then they call their usual bullshit dispatch table that has a bunch of stub functions that get replaced

[2025-05-01 18:54] avx: a getprocaddress ? ive heard enough, triple the warbird department budget

[2025-05-01 18:54] daax: [replying to avx: "a getprocaddress ? ive heard enough, triple the wa..."]
and they do that only to leave the functions that indicate what its doing in the code that follows

[2025-05-01 18:54] daax: lol

[2025-05-01 18:54] daax: 
[Attachments: image.png]

[2025-05-01 18:54] daax: HMM V73 WAS RESULT? I WONDER?

[2025-05-01 18:56] avx: [replying to daax: "and they do that only to leave the functions that ..."]
jfc

[2025-05-01 18:56] daax: now I actually wanna know why they're doing this crap in this function

[2025-05-01 18:56] daax: aight welp, rabbit hole

[2025-05-01 18:56] avx: it CANNOT be that serious MS

[2025-05-01 18:57] avx: [replying to daax: "aight welp, rabbit hole"]
the microsoft-made horrors stared back 😔

[2025-05-01 18:58] daax: 
[Attachments: QueryMaxSessions-Winsta.c]

[2025-05-01 18:58] daax: Here you go, not that it's usable for anything other than just seeing it.

[2025-05-01 19:02] 5pider: Isn't notepad protected by warbird as well? I see they are protecting their most valuable software only

[2025-05-01 19:02] daax: [replying to 5pider: "Isn't notepad protected by warbird as well? I see ..."]
yeah seems that's one of their testing grounds lol

[2025-05-01 19:03] 5pider: <:kekw:904522300257345566>

[2025-05-01 19:11] pinefin: i must be behind and never seen it...what da hell is warbird

[2025-05-01 19:11] pinefin: after seeing the ida screenshot you attached, i have never seen something like that

[2025-05-01 19:11] avx: microsoft's internal code protection

[2025-05-01 19:11] avx: or something along those lines

[2025-05-01 19:11] pinefin: huh. never heard of it

[2025-05-01 19:12] avx: im seeing matti type he will give you the proper answer

[2025-05-01 19:12] Matti: this is a handy cheat code to know about re: session creation... TIL
[Attachments: image.png]

[2025-05-01 19:12] pinefin: im sure, i didnt even know that existed

[2025-05-01 19:12] Matti: [replying to avx: "im seeing matti type he will give you the proper a..."]
no idgaf

[2025-05-01 19:12] avx: appreciate it matti

[2025-05-01 19:13] Matti: [replying to Matti: "this is a handy cheat code to know about re: sessi..."]
other than that I'd look at SmStartCsr  <@609487237331288074> , any of the Rpc* prefixed shit is just layers upon layers of garbage

[2025-05-01 19:14] Matti: but that's the function that ends up making the (ALPC <:hmm:475800667177484308>) call to smss to start a new csrss for the new session

[2025-05-01 19:14] Matti: [replying to Matti: "no idgaf"]
well untrue, I just don't care about warbird being applied in random contexts <:lillullmoa:475778601141403648>

[2025-05-01 19:15] avx: [replying to pinefin: "im sure, i didnt even know that existed"]
yeah which is why the snippet daax shared looks so horrid

[2025-05-01 19:15] avx: and yeah its used in notepad

[2025-05-01 19:15] pinefin: cuwious

[2025-05-01 19:15] pinefin: https://github.com/KiFilterFiberContext/microsoft-warbird
[Embed: GitHub - KiFilterFiberContext/microsoft-warbird: Reimplementation o...]
Reimplementation of Microsoft's Warbird obuscator. Contribute to KiFilterFiberContext/microsoft-warbird development by creating an account on GitHub.

[2025-05-01 19:15] pinefin: first thing to pop up on google

[2025-05-01 19:21] Matti: [replying to Matti: "but that's the function that ends up making the (A..."]
mm oh yeah most of the relevant types for this are in phnt, see ntsmss.h (and also the winsta one)

[2025-05-01 19:40] luci4: [replying to daax: "something strange i've always wondered is why RPC ..."]
They could've just made another flag like RPC_IF_ALLOW_SECURE_ONLY which you could use when registering an interface

[2025-05-01 19:41] luci4: 🤷‍♂️

[2025-05-01 19:46] Matti: this has always grated me yeah, rpcrt is so bad that it a server hosted by it will still have to first accept the ALPC connection (which it could refuse!) in order to make it possible for **you** to hopefully maybe implement a function that checks the caller security

[2025-05-01 19:48] Matti: meanwhile `NtAlpcAcceptConnectPort` just takes a boolean AcceptConnection

[2025-05-01 19:48] Matti: which should obviously be set to false if anything doesn't look right

[2025-05-01 19:49] Matti: instead rpcrt ALWAYS accepts ALPC connections, and then decides later on whether something is secure or not

[2025-05-01 19:50] Matti: meanwhile the connecting client has already obtained a (fairly useless admittedly, but still) connection to the server

[2025-05-01 21:29] the horse: [replying to pinefin: "https://github.com/KiFilterFiberContext/microsoft-..."]
tygoodshare^^

[2025-05-02 03:52] UJ: is notepad still protected by warbird?

[2025-05-02 05:02] Lyssa: [replying to UJ: "is notepad still protected by warbird?"]
yes

[2025-05-02 05:02] Lyssa: it's their testing bed I guess

[2025-05-02 05:50] sudhackar: [replying to UJ: "is notepad still protected by warbird?"]
I hear warbird - I remember someone likely got fired for submitting the Apple TV crash lol

[2025-05-02 05:52] sudhackar: See https://seclists.org/fulldisclosure/2024/Jun/7

[2025-05-02 06:03] SYAZ: anyone ever had that issue?
[Attachments: image.png]

[2025-05-02 06:09] Lyssa: no

[2025-05-02 14:09] pinefin: [replying to SYAZ: "anyone ever had that issue?"]
oh lord

[2025-05-02 14:10] pinefin: is this just on this binary? one time thing? or on every binary

[2025-05-02 14:58] qw3rty01: [replying to SYAZ: "anyone ever had that issue?"]
I’ve seen that happen after rebasing the program, one of the menu options has a "recalculate view" or something like that

[2025-05-02 15:07] SYAZ: [replying to pinefin: "is this just on this binary? one time thing? or on..."]
Nope on anything

[2025-05-02 15:08] SYAZ: when I try to go to Font options it just crashes

[2025-05-02 15:09] pinefin: [replying to SYAZ: "Nope on anything"]
yeah, i think you should probably reinstall. never had something like this happen to me

[2025-05-02 15:10] SYAZ: Already tried + tried using a previous version + tried resetting fonts in windows

[2025-05-02 15:10] SYAZ: idk

[2025-05-02 15:27] pinefin: i would honestly try and hop on with hexrays support and see if they can assist you with it and fix it if they can, seems like a very 1-off issue with fonts that you're having (at least ive never seen anyone have the same issue)

[2025-05-02 18:36] NSA, my beloved<3: How would you guys go about finding out what code previously moved data to a stack location? There is a function which is accessing a local variable, that is [ebp - 0x34]. I did dynamic analysis to find out that this is a location on the stack that already contains data, despite the function never actually writing anything there, it only reads data from there. I presume a previous function populated it, however I can't just follow the call stack and search for a reference to [ebp -0x34] since ebp will obviously be different in each function, not to mention the point of the reference to ebp, so offset will difear. Even worse, if it was push'ed there.

[2025-05-02 18:44] NSA, my beloved<3: Ironic how a google search returns this as the first find: https://revers.engineering/applied-re-the-stack/

[2025-05-02 18:44] NSA, my beloved<3: <@609487237331288074>

[2025-05-02 18:44] NSA, my beloved<3: 🥹

[2025-05-03 06:55] eversinc33: TIL about `KeCapturePersistentThreadState` to get `KDDEBUGGER_DATA`

[2025-05-03 09:58] eversinc33: [replying to NSA, my beloved<3: "How would you guys go about finding out what code ..."]
data breakpoint maybe?

[2025-05-03 15:13] NSA, my beloved<3: [replying to eversinc33: "data breakpoint maybe?"]
That won't work for two reasons: 1. Because of ASLR the stack address is always different, not to mention that the application is multithreaded, thus there are multiple stacks. 2. If I calculate the offset from the stack base then add that to the new stack base and I place a write breakpoint there, it will be triggered like a million times since the rest of the code uses the stack as well utilizing that stack space, so I can't scope it down.

[2025-05-03 16:13] truckdad: rr/ttd are great options in situations like this

[2025-05-03 16:13] truckdad: go to the function where you know about the reference, set a data breakpoint, then go backwards

[2025-05-03 19:01] NSA, my beloved<3: [replying to truckdad: "rr/ttd are great options in situations like this"]
Could you please tell me what rr and ttd stand for?

[2025-05-03 19:01] truckdad: https://rr-project.org

[2025-05-03 19:01] truckdad: https://learn.microsoft.com/en-us/windows-hardware/drivers/debuggercmds/time-travel-debugging-overview
[Embed: Time Travel Debugging - Overview - Windows drivers]
This section describes time travel debugging.

[2025-05-03 19:01] NSA, my beloved<3: [replying to truckdad: "go to the function where you know about the refere..."]
That won't work, since the data has been already set by the time the function is called.

[2025-05-03 19:02] NSA, my beloved<3: And I am not sure which previous function sets the data.

[2025-05-03 19:02] truckdad: yes, that is precisely what rr and ttd help with

[2025-05-03 19:02] truckdad: that is why i mentioned them before saying that 🙂

[2025-05-03 19:02] NSA, my beloved<3: I see. Checking them out.

[2025-05-03 19:03] truckdad: basically: you record program execution, then can make time go backwards, including with breakpoints

[2025-05-03 19:03] NSA, my beloved<3: TTD sounds awesome. Isn't this what x64dbg trace feature is supposed to do?

[2025-05-03 19:04] NSA, my beloved<3: It's just really slow and has an instruction cap so it's really hard to use it for things like this. Or I may use it in the wrong way?

[2025-05-03 19:04] truckdad: i don't know how x64dbg does it, but ttd does not have the same restrictions

[2025-05-03 19:04] truckdad: the only real downside is that it eats disk space

[2025-05-03 19:06] NSA, my beloved<3: Yeah, I'd expect that.

[2025-05-03 19:13] NSA, my beloved<3: By the way, logging instructions until a specific point is great, however I still won't be able to tell which instruction set the value there since the ebp offsets will diffear, right?

[2025-05-03 19:13] NSA, my beloved<3: So looking through previous instructions won't really work in this case.

[2025-05-03 19:14] NSA, my beloved<3: Maybe if there is a way to only log instructions which wrote to a specific address from a specific point, then place breakpoints at them and see which is the one I need.

[2025-05-03 19:14] truckdad: no, you set a breakpoint on the specific address

[2025-05-03 19:14] NSA, my beloved<3: Ah, makes sense.

[2025-05-03 19:15] NSA, my beloved<3: That's really handy. Thanks!

[2025-05-03 19:16] truckdad: [replying to truckdad: "go to the function where you know about the refere..."]
if it's not clear here, when i said "go to the function where you know about the reference", i mean dynamically, e.g. get there with a breakpoint so you have a specific address to work with

[2025-05-03 19:16] NSA, my beloved<3: Oh yeah, I see what you meant. That is what I initially did yeah.