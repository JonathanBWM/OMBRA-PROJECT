# February 2025 - Week 1
# Channel: #programming
# Messages: 59

[2025-02-01 04:19] Matti: [replying to Torph: "you could keep track of how many frames a key was ..."]
yeah, this is something I thought of as well as a bandaid that would probably work OK

[2025-02-01 04:20] Matti: but there are other issues with this approach

[2025-02-01 04:21] Matti: mostly that I'd need something like a frame counter, a tick counter, or a timer to keep track of when to lift the delay restriction

[2025-02-01 04:23] Matti: this driver has something resembling all three of those things in various places, but none are documented in enough detail to even know what they are keeping track of/counting, and the things I do know of what they're *supposed* to do, I have strong doubts about the implementation of

[2025-02-01 04:24] Matti: especially the timer function you're supposed to pass to doom lol

[2025-02-01 04:24] Matti: <https://github.com/NSG650/NtDOOM/blob/master/NtDOOM/entry.c#L342>

[2025-02-01 04:25] Matti: I've looked at the code that calls this, and it is also insane

[2025-02-01 04:26] Matti: all time is tracked in **1/70th second** units

[2025-02-01 04:28] Matti: now, IDK what a proper implementation of this function would look like
because I still basically don't really know what the fuck it's expected to return (I mean I could read the puredoom docs so this part is mostly my fault)
but I'm sure that this code is not it lol

[2025-02-01 04:31] Matti: [replying to Torph: "<:nomore:927764940276772925> what?? isn't the whol..."]
well there has to be an event buffer *somewhere* I suppose, due to the limitations inherent to the engine (it's one loop running in 1 thread, so you first have a chance to put input events in the buffer, and then later on in the same loop some other code gets a chance to pick up the events to do something with the input)

[2025-02-01 04:33] Matti: but the code did terrify me regardless because AFAICT the buffer size chosen is just a completely arbitrary number

[2025-02-01 04:34] Matti: all I know is that it was increased at some point in puredoom, presumably due to there being too many input events per unit of whatever the fuck measure it is he used to derive his new ideal buffer size

[2025-02-01 04:36] Matti: I actually have some input processing code now that I wanted to test, but this event queue buffer is so fucking fishy that I added an assert in the doom code for when the event counter overflows

[2025-02-01 04:37] Matti: it literally asserted within 2 seconds after starting a game

[2025-02-01 04:47] Matti: OK so this is an abbreviated version of what I've got for input processing at the moment
[Attachments: image.png]

[2025-02-01 04:48] Matti: this actually works... well I wouldn't say it works *well*, but it's better than my previous efforts

[2025-02-01 04:49] Matti: and this version does not overflow the event buffer

[2025-02-01 04:50] Matti: due to the throttle on the number of key up events (comment at L513)

[2025-02-01 04:52] Matti: but this same throttle is causing issues with other things that are slightly more subtle

[2025-02-01 04:53] Matti: I didn't think of movement accel for example, i.e. tapping W briefly will move you forward a tiny bit, but if you hold it down you start running pretty quickly

[2025-02-01 04:54] Matti: so to prevent ever increasing acceleration or to come to a full stop again you need to counteract this with sufficient key up events <:lillullmoa:475778601141403648>

[2025-02-01 04:55] Matti: so currently my guy can do 1 out of 4 cardinal directions... ever

[2025-02-01 04:55] Matti: and also forever, because you can't come to a stop again

[2025-02-01 04:57] Matti: unfortunately if I remove the throttle I get the instant assert I mentioned, about the event counter overflowing for the size of the buffer

[2025-02-01 05:45] Saturnalia: [replying to Matti: "all time is tracked in **1/70th second** units"]
Is there an NT api related reason for it, or just the author being wack

[2025-02-01 05:46] Matti: no, all NT timekeeping is in 100ns units

[2025-02-01 05:46] Matti: this is the original doom code as far as I can tell

[2025-02-01 05:47] Matti: there's even an (ifdef'd out) implementation for SGI

[2025-02-01 05:50] Matti: the entire codebase is just completely insane, tbh

[2025-02-01 05:51] Matti: like, you'd think a good and straightforward definition of a 'tick counter' could be something like: the number of times the main loop has run

[2025-02-01 05:52] Matti: but there are least 5 different globals that could all be called a tick counter in their own way, but of course these all actually do wildly different things

[2025-02-01 05:52] Saturnalia: <:kekw:904522300257345566>

[2025-02-01 05:52] Matti: in general everything interacts with everything else, since everything is a global

[2025-02-01 05:53] Saturnalia: I love early 90s spaghetti code

[2025-02-01 05:53] nonchy: [replying to Matti: "the entire codebase is just completely insane, tbh"]
isnt this kind of the case with all of iD's early work too

[2025-02-01 05:53] nonchy: keen/wolfenstein/doom/quake

[2025-02-01 05:53] Matti: no, the quake source code I remember being distinctly higher quality than this for sure

[2025-02-01 05:54] nonchy: the first 3 have much smaller teams

[2025-02-01 05:54] Matti: quake is not good by modern standards, but this is really something else

[2025-02-01 05:54] nonchy: quake did havea lot more funding iirc

[2025-02-01 05:55] Matti: yeah, quake was a huge game for id

[2025-02-01 05:55] Matti: also due to licensing revenue

[2025-02-01 05:55] nonchy: i dont know if this is accurate but i recall those first three only had romero/carmack as developers

[2025-02-01 05:56] Matti: which are those three?

[2025-02-01 05:56] Matti: keen, wolfenstein, doom?

[2025-02-01 05:56] nonchy: commander keen, wolfenstein, doom

[2025-02-01 05:56] nonchy: ya

[2025-02-01 05:56] nonchy: actually i dont even know if its true for keen thinking about it

[2025-02-01 05:57] Matti: I'm not sure about the first two to be honest, but for doom there were definitely more programmers than just the two johns

[2025-02-01 05:57] Matti: ah I was looking for this

[2025-02-01 05:57] Matti: https://fabiensanglard.net/quakeSource/

[2025-02-01 05:58] Matti: I really recommend this article (and his others, he's got a few of these for other id engines)

[2025-02-01 06:00] Matti: <https://fabiensanglard.net/doomIphone/doomClassicRenderer.php> this is the doom one

[2025-02-01 06:02] Matti: quake 3: <https://fabiensanglard.net/quake3/index.php>
doom 3: <https://fabiensanglard.net/doom3/index.php>

[2025-02-01 06:02] Matti: those are the main id source code articles I think

[2025-02-01 06:03] Matti: oh yeah, quake 2: <https://fabiensanglard.net/quake2/index.php>
important because it was the first id engine to support HW accelerated rendering

[2025-02-01 12:22] Hunter: <:mmmm:904523247205351454>

[2025-02-01 16:51] Torph: [replying to Matti: "so to prevent ever increasing acceleration or to c..."]
what??? why isn't this handled inside the doom movement code

[2025-02-01 16:52] Torph: [replying to Matti: "oh yeah, quake 2: <https://fabiensanglard.net/quak..."]
oh i love this series 
the quake 2 sort of abstracted renderer was really interesting