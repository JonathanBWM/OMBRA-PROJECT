# August 2024 - Week 5
# Channel: #reverse-engineering
# Messages: 144

[2024-08-26 04:24] shalzuth: [replying to JustMagic: "process/thread IDs are generated and tracked using..."]
Can you calculate the next table/process/thread id?

[2024-08-26 04:27] JustMagic: [replying to shalzuth: "Can you calculate the next table/process/thread id..."]
AFAIK the algorithm is deterministic

[2024-08-26 04:28] JustMagic: but you'll be racing with everything else in the system that creates things in the handle tables

[2024-08-27 14:57] Loading: hey guys, is it possible to transfer names of variables in ghidra (pic below as example) when doing version tracking between two versions of same exectuable ? names of functions can be done but i never figured this out
[Attachments: image.png]

[2024-08-27 21:50] kert: Has anyone here tried hooking into the XAML (Windows.Xaml.UI.dll) rendering pipeline or has any experience with it? It's used by a few interesting Windows processes like Xbox Bar (Windows + G).

I've discovered that windows uses XAML which uses Direct2D (with a caching system—it only calls BeginDraw/EndDraw when pixels change) to render some widgets, like the emoji panel (Windows + period). **However, I'm unable to determine what window is it rendering these widgets in**. All the windows seem to be invisible or disabled.

I tried stack walking from the CreateWindowExW function and found that it creates a new window of screen size, but that window somehow gets hidden or destroyed later in the process (it stops being visible in System Informer). This window must be significant during startup because when I hook into twinapi.appcore.CreateCoreWindowSite and return at the start for the above window only, it fails to render anything.

Hooking at the start of EndDraw or at the end of the target view’s Clear function and drawing any shape from it seems to affect only the children elements of the rendered "window." Interestingly, when you pause all the threads (including the WndProc thread and the rendering thread), the window is still movable, and when hovering over specific child elements, the cursor still changes its icon.

[2024-08-27 21:50] kert: Here is the chain for rendering that I've found, but as mentioned earlier, I was still unable to locate what it is rendering in: CXcpDispatcher::Create->CXcpDispatcher::Init->CXcpDispatcher::MessageTimerCallbackStatic->CDeferredInvoke::DispatchQueuedMessage->CXcpDispatcher::WindowProc->CXcpDispatcher::OnReentrancyProtectedWindowMessage->CXcpDispatcher::Tick->CXcpBrowserHost::OnTick->CWindowRenderTarget::Draw->CCoreServices::NWDrawTree

When we hook into CCoreServices::NWDrawTree and return at the start of it, nothing gets rendered except a small #FFFFFF black rectangle for 50-150ms, implying that some of the core logic must be there. When we hook into HWBuildGlyphRunSingleTexture, which is called somewhere inside CCoreServices::NWDrawTree, and return at the start of it, only the background gets rendered.

Here's the call chain XAML uses to reach Direct2D's BeginDraw (with HWWalk::Render being called somewhere inside NWDrawTree, whose chain was mentioned above): HWWalk::Render->HWWalk::RenderProperties->HWWalk::RenderContentAndChildren->BaseContentRenderer::TextBlockRenderContent->CTextBlock::HWRenderContent->D2DTextDrawingContext::HWRender->DCompSurface::BeginDrawWithGutters->DirectComposition::CCompositionSurface::BeginDrawWithGutters->DirectComposition::CAtlasSurfacePool::BeginDraw->D2DDeviceContextBase<ID2D1DeviceContext6,ID2D1DeviceContext6,null_type>::BeginDraw

It also seems to be using some band system (CreateWindowInBandEx).

[2024-08-27 22:24] diversenok: The Win+G widget window seems to belong to `GameBar.exe`; I see it starts covering the screen when you press the key

[2024-08-27 22:25] diversenok: This one
[Attachments: image.png]

[2024-08-27 22:30] diversenok: The use of of `CreateWindowInBandEx` seems reasonable since it belongs to the `ZBID_IMMERSIVE_NOTIFICATION` band

[2024-08-27 22:31] diversenok: Its parent is owned by explorer

[2024-08-27 22:32] diversenok: And it's normally invisible because it's cloaked

[2024-08-27 22:34] kert: [replying to diversenok: "This one"]
Interesting, I can't see it
[Attachments: image.png]

[2024-08-27 22:35] kert: What about dictation widget (Win+H)? It's top most as well but doesn't get hidden when you focus out of it. I was looking into that one more than GameBar.exe before describing my issue

[2024-08-27 22:35] kert: It's this right here
[Attachments: image.png]

[2024-08-27 22:36] kert: It's using composition ui engine with XAML

[2024-08-27 22:36] diversenok: First, here is a lifehack: you can start System Informer in the UIAccess band with Almost-on-top, and it's going to cover the gamebar window, letting you inspect it

[2024-08-27 22:38] kert: [replying to kert: "It's this right here"]
These two CoreWindow windows don't seem to be correlated anyhow as you can hide them and widget is still displayed
[Attachments: image.png]

[2024-08-27 22:39] kert: 
[Attachments: image.png]

[2024-08-27 22:39] diversenok: [replying to kert: "Interesting, I can't see it"]
And refreshing doesn't help? In my case it lingers around after dismissing the widget

[2024-08-27 22:39] diversenok: And I can see it becoming uncloaked when pressing the key combo

[2024-08-27 22:42] diversenok: I see this one for the dictation thing; hiding it hides the widget
[Attachments: image.png]

[2024-08-27 22:43] diversenok: You do click Refresh in System Informer, right?

[2024-08-27 22:43] kert: Yes, I do

[2024-08-27 22:43] kert: That's why I'm wondering why it's still being displayed

[2024-08-27 22:43] diversenok: Strange, because you also have the DictationContol thread in the list

[2024-08-27 22:44] diversenok: I dunno, I see it fine in the list ¯\_(ツ)_/¯
[Attachments: image.png]

[2024-08-27 22:45] diversenok: It doesn't change the visibility style when dismissed, it just goes into cloaked

[2024-08-27 22:47] kert: What windows version are you on?

[2024-08-27 22:48] diversenok: Latest 10; I can also check 11 if you want

[2024-08-27 22:49] kert: I'm also on latest 10

[2024-08-27 22:49] kert: Really odd

[2024-08-27 22:50] diversenok: And here is the emoji one, also in the same process
[Attachments: image.png]

[2024-08-27 22:51] diversenok: You have one TextInputHost.exe process, right?

[2024-08-27 22:51] kert: Yup
[Attachments: image.png]

[2024-08-27 22:52] kert: 
[Attachments: textinput.gif]

[2024-08-27 22:52] kert: Here take a look

[2024-08-27 22:54] diversenok: Hmm

[2024-08-27 22:54] kert: [replying to diversenok: "And here is the emoji one, also in the same proces..."]
That's why I was so confused as I couldn't see any window correlating to that widget (as it doesn't disappear when hiding/disabling it)

[2024-08-27 22:54] diversenok: Can you maybe write a simple program that calls `WindowFromPoint` and then find the window by HWND?

[2024-08-27 22:54] kert: Sure

[2024-08-27 22:55] diversenok: Well, on my machine it doesn't disappear (as in, entirely, or becoming gray in System Informer) when dismissing; it just changes cloaked state

[2024-08-27 22:56] diversenok: Oh, you know what, I vaguely remember some Windows setting that can prevent programs from enumerating immersive windows or something

[2024-08-27 22:59] diversenok: It was `NtUserSetProcessRestrictionExemption` I guess

[2024-08-27 23:09] diversenok: I just tested it in a clean VM and I don't have any issues ¯\_(ツ)_/¯

[2024-08-27 23:11] diversenok: The first time it needs to show the emoji panel or the dictation window, it spawns `TextInputHost.exe` (if it wasn't running already) and creates the corresponding windows which System Informer can see appearing in the list of windows

[2024-08-27 23:12] diversenok: And you can tell them apart by the name of the thread that owns them
[Attachments: image.png]

[2024-08-27 23:17] diversenok: Dumb question: are you sure these widget windows you see are on the same OS as System Informer? I just fell for it in a VM - I pressed the shortcut and it was handled by the host OS while I was trying to inspect the window in the guest 😅

[2024-08-27 23:17] kert: 100% sure

[2024-08-27 23:18] diversenok: Just checking 😁

[2024-08-27 23:19] diversenok: I mean, I could image a scenario when System Informer wouldn't see all windows because the API skips a few for whatever reason

[2024-08-27 23:19] diversenok: But I cannot reproduce it

[2024-08-27 23:20] diversenok: That's why I suggest a small program that queries and prints the result of `WindowFromPoint` in a loop while you manually move the widget under the point

[2024-08-27 23:22] diversenok: That's how I discoverd the `Win+G` window owner

[2024-08-27 23:23] diversenok: Because even when running in UIAccess band, it's still quite inconvenient to experiment with since it hides on almost every action

[2024-08-27 23:24] kert: [replying to diversenok: "That's why I suggest a small program that queries ..."]

[Attachments: 2.gif]

[2024-08-27 23:24] kert: 
[Attachments: image.png]

[2024-08-27 23:25] kert: Yeah, I think system informer just skips it for whatever reason

[2024-08-27 23:26] diversenok: So if you go to main menu -> view -> windows and search for the HWND, does it show anything?

[2024-08-27 23:26] kert: It shows it there?
[Attachments: image.png]

[2024-08-27 23:26] kert: 😕

[2024-08-27 23:26] kert: That is oddly confusing

[2024-08-27 23:27] kert: Thanks man! Appreciate it

[2024-08-27 23:28] diversenok: Okay, that is interesting

[2024-08-27 23:30] kert: Yeah, it is

[2024-08-29 16:26] BWA RBX: This is something I find fascinating
[Attachments: Fc76ignXgAEVLmM.jpg]

[2024-08-29 16:27] BWA RBX: The difference when you apply optimization and when you don't

[2024-08-29 16:28] contificate: it's not that fascinating

[2024-08-29 16:28] contificate: one is purposely doing the naivest lowering possible

[2024-08-29 16:28] BWA RBX: For me it is because I know very little

[2024-08-29 16:28] contificate: you know what's cooler

[2024-08-29 16:28] contificate: if you implement bswap as a loop (of fixed bound)

[2024-08-29 16:28] contificate: it will likely unroll it and match the same pattern

[2024-08-29 16:28] contificate: and turn the loop into a single bswap

[2024-08-29 16:30] BWA RBX: So if someone did this to their program could we apply these optimizations

[2024-08-29 16:30] Brit: discovers lifting and recompiling from first principles

[2024-08-29 16:30] BWA RBX: I was going to say lifting it to LLVM

[2024-08-29 16:31] contificate: yeah, but this isn't really a real obfuscation

[2024-08-29 16:31] contificate: this is just the compiler not doing various matching it would otherwise do for -O3

[2024-08-29 16:31] contificate: the issue is

[2024-08-29 16:32] contificate: many properties of concern to program transformation are undecidable

[2024-08-29 16:32] contificate: so you get approximate information at best

[2024-08-29 16:32] contificate: hence the full employment theorem for compiler writers

[2024-08-29 16:32] contificate: so to some extent

[2024-08-29 16:32] contificate: layering a bunch of novel obfuscations with high variance could just devolve into a game of cat and mouse

[2024-08-29 16:32] contificate: a lot of pattern matching

[2024-08-29 16:33] Brit: anything that gets cooked by lift and compile isn't a real obfuscation anyway

[2024-08-29 16:34] BWA RBX: Do you think that compilers like MSVC, GCC etc could do more to optimize a program, because sometimes I see stuff that the compiler might have missed that could have been optimized

[2024-08-29 16:35] contificate: yes

[2024-08-29 16:35] contificate: but you end up getting diminishing returns

[2024-08-29 16:36] contificate: like there are many little cases you could cook in and capture at the cost of slowing down compilation for a case that's not very common

[2024-08-29 16:36] BWA RBX: Is that because it's non-deterministic sometimes

[2024-08-29 16:36] contificate: bswap is a case where it's important to optimise because people often implement it by hand

[2024-08-29 16:36] contificate: compilers are not really designed to be non-deterministic

[2024-08-29 16:36] contificate: they would be very hard to test if they were non-deterministic

[2024-08-29 16:37] contificate: this bswap malarkey is likely just

[2024-08-29 16:37] contificate: https://github.com/search?q=repo%3Agcc-mirror%2Fgcc+bswap+language%3A%22GCC+Machine+Description%22&type=code&l=GCC+Machine+Description

[2024-08-29 16:37] contificate: machine description pattern matching

[2024-08-29 16:37] contificate: once you linearise out straightline sequences comprising arithmetic and bitwise shifting

[2024-08-29 16:37] contificate: you can then induce trees/dags from data dependency

[2024-08-29 16:37] contificate: and match over them

[2024-08-29 16:38] contificate: which is the basis of how GCC, LLVM, Cranelift, Go, etc. work

[2024-08-29 16:38] contificate: matching all the way down

[2024-08-29 16:38] contificate: except many handwritten fragments for complex things

[2024-08-29 16:39] BWA RBX: Thanks for explaining it

[2024-08-29 17:31] Torph: [replying to BWA RBX: "This is something I find fascinating"]
oh cool, i didnt know there was an instruction for that. I wonder if my generic endian swap macro will be optimized the same way

[2024-08-29 17:31] Deleted User: [replying to contificate: "which is the basis of how GCC, LLVM, Cranelift, Go..."]
Speaking of cranelift, what do you think of it?

[2024-08-29 17:40] Torph: [replying to Torph: "oh cool, i didnt know there was an instruction for..."]
oh awesome, it boils down to 1 or 2 instructions for everything but 64-bit
[Attachments: 2024-08-29_13-39.png]

[2024-08-29 17:50] Torph: oh, and in clang 9.0.0+ even the 64-bit is a single instruction
[Attachments: 2024-08-29_13-50.png]

[2024-08-29 17:51] contificate: [replying to Deleted User: "Speaking of cranelift, what do you think of it?"]
I think it's cool - haven't used it though

[2024-08-29 17:52] BWA RBX: [replying to contificate: "I think it's cool - haven't used it though"]
Never used it probably never will

[2024-08-29 17:52] contificate: I'm not a compilers person, I'm afraid

[2024-08-29 17:56] BWA RBX: Lol

[2024-08-29 17:56] BWA RBX: [replying to contificate: "I'm not a compilers person, I'm afraid"]
You just flip burgers

[2024-08-29 17:57] contificate: as I've explained previously, we do not flip the burgers

[2024-08-29 17:57] contificate: the only time we flip the burgers

[2024-08-29 17:57] contificate: is when we put the two sides of the box together

[2024-08-29 17:57] BWA RBX: 😂

[2024-08-29 17:57] contificate: to assemble the burger

[2024-08-29 17:57] BWA RBX: You put it in the colored trays

[2024-08-29 17:57] contificate: but that's not even how you're meant to do it

[2024-08-29 17:57] contificate: only the top bun is really meant to land on top

[2024-08-29 17:57] contificate: but for the big mac, who gives a fuck

[2024-08-29 17:58] BWA RBX: I do if someone messes my Big Mac up it costs their job

[2024-08-29 17:58] BWA RBX: Need it done to perfection if not I am constantly going back up to exchange it for a new big mac

[2024-08-29 18:00] BWA RBX: Imagine having a hangover and reverse engineering a Go sample

[2024-08-29 18:02] Azrael: [replying to BWA RBX: "Imagine having a hangover and reverse engineering ..."]
Hangover reference?

[2024-08-29 18:02] Azrael: Hangover 3 was pretty bad.

[2024-08-29 18:03] Azrael: But Hangover 1 & 2 are good.

[2024-08-29 18:03] snowua: <@687117677512360003> you’ve inspired me to pursue a career at mcdonald’s

[2024-08-29 18:04] BWA RBX: [replying to snowua: "<@687117677512360003> you’ve inspired me to pursue..."]
I'm going to apply for an interview

[2024-08-29 18:04] BWA RBX: 
[Attachments: 456787732_921572940023612_6151511442560284897_n.jpg]

[2024-08-29 18:05] contificate: [replying to snowua: "<@687117677512360003> you’ve inspired me to pursue..."]
happy to hear it

[2024-08-29 18:05] BWA RBX: You think I'm messing I'm going to quit my job that I earn 16.50 an hour

[2024-08-29 18:05] BWA RBX: to a job that I make 12.80

[2024-08-29 18:06] luci4: [replying to BWA RBX: ""]
More like publishers lol

[2024-08-29 18:06] BWA RBX: better than dealing with illiterate people who think they are entitled

[2024-08-29 18:07] BWA RBX: [replying to luci4: "More like publishers lol"]
How dare they own something they paid for

[2024-08-29 18:07] BWA RBX: No way, STEAM HANDLE THIS ASAP

[2024-08-31 01:09] th3: [replying to BWA RBX: "You think I'm messing I'm going to quit my job tha..."]
i work at mcdonalds

[2024-08-31 01:09] th3: i get 18

[2024-08-31 01:09] th3: a hour

[2024-08-31 16:31] BWA RBX: [replying to th3: "i get 18"]
Must be in America

[2024-08-31 16:32] BWA RBX: If it's Euro you are Manager or some executive or some shit

[2024-08-31 23:06] rin: [replying to th3: "i get 18"]
I bet Canadian