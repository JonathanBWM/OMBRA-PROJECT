# January 2024 - Week 4
# Channel: #programming
# Messages: 99

[2024-01-22 17:58] luci4: [replying to luci4: "Yeah I meant to say array, my bad"]
...There is a problem

[2024-01-22 18:06] luci4: Initially, I wanted to add a variable to my key event structure which would help me keep track of the window it was inputted in, but now I'm storing the vkCodes in an UINT array. 

I could just log active window changes and have the consumer map them afterwards, but that seems like an unnecessary headache.

[2024-01-22 19:28] luci4: I could also create a UINT value which, when added to the array, will help the consumer know when a window was changed.

[2024-01-22 23:22] donna🤯: [replying to luci4: "Initially, I wanted to add a variable to my key ev..."]
forgive me if im missing something as ive only skimmed what youve sent - but why dont you just have a struct with the key code and window, insert that into a queue on one thread and have a worker thread process those events seperately?

[2024-01-23 16:24] luci4: [replying to donna🤯: "forgive me if im missing something as ive only ski..."]
Sorry for the late response, was a little busy. So what I am doing is storing key events in a buffer, which will be sent to a Kafka topic where a consumer will process them. I'm sending key events in batches as to not create useless traffic.

Initially, I used a linked list for the key event batch, as I thought that would preserve their order. That turned out to be not the case, and also inefficient.

So now I want to make the batch a fixed buffer

[2024-01-23 16:26] luci4: I think I'm just gonna make a structure consisting of an UINT and char* and make an array out of them

[2024-01-23 16:54] x86matthew: it sounds like you're making this keylogger stuff a lot more complicated than it needs to be

[2024-01-23 16:54] x86matthew: what are you trying to do?

[2024-01-23 16:54] Brit: [replying to Matti: "I'm used to seeing this"]
I paid for mine and it still does this, I blame it on the auto license you get on msi boards

[2024-01-23 16:55] Matti: oh damn, you do?

[2024-01-23 16:55] Brit: yee

[2024-01-23 16:55] Brit: tempted to just patch it out

[2024-01-23 16:55] Matti: I should get one, I've probably still got an MSI motherboard lying around somewhere

[2024-01-23 16:56] Brit: but I don't use aida often enough to justify taking a look

[2024-01-23 16:56] Matti: yeah same <:kekw:904522300257345566>

[2024-01-23 16:56] Brit: it's gonna tilt me into fixing it one day, but that day is not today

[2024-01-23 16:56] Matti: its ACPI view is nice

[2024-01-23 16:57] Matti: and so is that memory benchmark

[2024-01-23 16:57] Matti: HWInfo has an ACPI tree view but it is completely broken on most of my systems

[2024-01-23 16:58] Matti: AIDA shows less total information, but it's interpreted and human readable at least

[2024-01-23 16:59] Matti: oh and AIDA's CPUID feature checkbox list is really convenient too IMO

[2024-01-23 16:59] x86matthew: this aida64 thing looks a lot like a tool called everest that i used back in the day

[2024-01-23 17:00] Matti: in linux you can just use `/proc/cpuinfo` and grep for the flag, but no such thing in windoze 😔

[2024-01-23 17:01] Matti: OTOH AIDA's output is more descriptive
[Attachments: image.png]

[2024-01-23 17:01] Matti: e.g. you can have support for X2APIC without having it enabled

[2024-01-23 17:02] Matti: which it reports correctly

[2024-01-23 17:05] Matti: I also still find myself using RWEverything surprisingly often... mainly for MSRs, PCI config space access and I/O / MMIO R/W

[2024-01-23 17:06] Matti: but it's ancient and no longer being updated, I have no idea how it even still works

[2024-01-23 17:30] luci4: [replying to x86matthew: "what are you trying to do?"]
So overall the idea is:
- store key events into a fixed buffer
- when it's full send the batch of VkCodes to a Kafka topic, where a consumer will process it (and store it into a DB maybe, haven't thought that far yet)

I also want to make it keep track of which window a key was inputted in, which is why I wanted to create that struct

[2024-01-23 17:43] x86matthew: dunno what kafka is but i'd suggest simply reading the current window title whenever a key is pressed and store it

[2024-01-23 17:43] x86matthew: if the active window title has changed since the previous key press, push the existing buffer and previous window name

[2024-01-23 17:43] x86matthew: if not, just write the key to the current buffer

[2024-01-23 17:43] x86matthew: should probably also push the buffer on "return" key press (or if the buffer gets full)

[2024-01-23 19:21] luci4: [replying to x86matthew: "dunno what kafka is but i'd suggest simply reading..."]
I've been thinking of using GetWindowText for that (I should remember to unhook User32.dll, maybe i'll use pe2shc to bring it with me)

[2024-01-23 19:24] JustMagic: <@211939939221241856> can you remind us what are you even doing in general. Why do you need the key events in an event queue/db

[2024-01-23 19:24] luci4: [replying to x86matthew: "if the active window title has changed since the p..."]
Oh my wording was unclear, my bad on that. The batch that gets sent to the Kafka server is just the buffer where the VkCodes were stored, they're not separate things. 

Or do you mean that I should create a separate buffer to store the key codes, and copy it to the batch buffer alongside the window name after a window change?

[2024-01-23 19:28] luci4: [replying to JustMagic: "<@211939939221241856> can you remind us what are y..."]
This all started with my curiosity on how the Cobalt Strike beacon's keylogger works. I'm basically making a keylogger that collects key events in a buffer, and once it reaches a certain size it gets sent to a Kafka topic, where they will be processed by a consumer (which I am yet to make).

[2024-01-23 19:30] JustMagic: You're probably better off using a lower level abstraction for sending data than a Kafka topic for data exfil

[2024-01-23 19:31] luci4: [replying to JustMagic: "You're probably better off using a lower level abs..."]
Could you give me an example, please?

[2024-01-23 19:31] luci4: You mean I should just use HTTP/S?

[2024-01-23 19:33] JustMagic: What I mean is that you should structure your code so it's easy to try different protocols

[2024-01-23 19:33] JustMagic: You also definitely don't need something as heavy as kafka

[2024-01-23 19:52] luci4: [replying to JustMagic: "What I mean is that you should structure your code..."]
I'm gonna try and make it a bit modular, so I can use something other than Kafka for sending the batch.

[2024-01-23 19:55] luci4: [replying to JustMagic: "You also definitely don't need something as heavy ..."]
Do you think there is anything inherently wrong with Kafka for this purpose?

[2024-01-23 19:56] JustMagic: [replying to luci4: "Do you think there is anything inherently wrong wi..."]
Cancer to setup and maintain. Go with redpanda at least.

[2024-01-23 20:37] qwerty1423: [replying to luci4: "You mean I should just use HTTP/S?"]
yea it should work, you can use any protocol, for an alternative you can setup RabbitMQ for your work but its a little slower than kafka.

[2024-01-23 21:11] brymko: data exfil with kafka will be 100% undetected

[2024-01-23 21:11] brymko: aint no one gonna expect that shit

[2024-01-23 21:16] luci4: [replying to brymko: "data exfil with kafka will be 100% undetected"]
ill worry about that when it works, and if its 💩 Ill just use smth else

[2024-01-23 21:24] 25d6cfba-b039-4274-8472-2d2527cb: real men pivot to company extranet servers and add themselves as users for persistent exfil

[2024-01-23 22:15] dullard: Unironically true

[2024-01-23 22:15] dullard: Or just p2p beacon to Citrix boxes and use one drive <:chad:815144781842743334>

[2024-01-23 22:15] dullard: Even easier

[2024-01-23 22:16] dullard: Real men base64 and upload to pastebin

[2024-01-23 23:04] donna🤯: [replying to luci4: "Oh my wording was unclear, my bad on that. The bat..."]
I think what hes saying is

[2024-01-23 23:15] donna🤯: have some large buffer which stores key events. you dont need to store the window name in each entry as that is obviously quite inefficient. Store the current window name somewhere and add mouse events to the buffer. When a user changes window, thats when you flush the buffer to your worker thread or however you process it. An example packet would be as follows:

```
struct events_packet
{
  CHAR window_name[MAX_WINDOW_NAME_LEN]
  kev_event kev_events[1000]
}
```

[2024-01-23 23:15] donna🤯: you'd also need to flush the buffer when its full

[2024-01-23 23:17] donna🤯: then simply cast the buffer to the packet type on your processing thread and process it how youd like

[2024-01-23 23:21] donna🤯: `key_event` here is however you store the key index, probably a uint16

[2024-01-23 23:40] donna🤯: you could also, rather then a fixed size, store the number of events and pass the size of the packet in the header

[2024-01-23 23:41] donna🤯: thats probably the better approach

[2024-01-24 09:11] M4CGYV3R: Hi everyone, I wanted some guidance on matter that I want to make a mobile app monitoring tool that basically tells user that certain action took place for e.g ss, text , audio etc this type of thing. So what should be my first step and what language should I choose

[2024-01-24 12:01] luci4: [replying to donna🤯: "have some large buffer which stores key events. yo..."]
Since I plan for the processing to be done server-side, wouldn't processing it (meaning, sending it away) on window change generate too much traffic than it's worth?

[2024-01-24 12:18] contificate: there's no hard and fast requirement to send it away when the window changes

[2024-01-24 12:18] contificate: you can continue to buffer it

[2024-01-24 12:31] donna🤯: yea you wouldnt have to

[2024-01-24 12:32] donna🤯: theres many different ways you can go about this

[2024-01-24 12:32] donna🤯: you could just insert a new header on window change

[2024-01-24 12:33] donna🤯: and on a window change, update the previous headers packet count

[2024-01-24 12:33] donna🤯: then when the buffer is full flush it

[2024-01-24 12:35] donna🤯: so as an example, the flushed buffer would have a structure something like this:

`| window: discord, count: 50, [events] | window: chrome, count: 10, [events] | window ... |`

[2024-01-24 12:35] donna🤯: then to actually parse that data you simply parse the header using the event count

[2024-01-24 14:49] luci4: [replying to donna🤯: "then when the buffer is full flush it"]
I think I understand what you mean, thanks a million. Gonna implement this and (finally) be done with the producer

[2024-01-25 21:22] mibho: if u have no dynamically allocated objects or u deal with them, is it acceptable to call exit()

[2024-01-25 22:13] Terry: [replying to mibho: "if u have no dynamically allocated objects or u de..."]
exit() is the best garbage collector

[2024-01-25 22:26] Timmy: that slow tho

[2024-01-25 22:27] Timmy: ```as
int 0x29
```

[2024-01-25 22:32] Terry: just deref null peter like a real man

[2024-01-25 22:44] Matti: [replying to Timmy: "```as
int 0x29
```"]
this is not windows 7 compliant 👮

[2024-01-25 22:48] Matti: [replying to Terry: "just deref null peter like a real man"]
this may fail to fail on very old versions of NT where mapping the null page is allowed 👮

[2024-01-25 22:51] Terry: [replying to Matti: "this may fail to fail on very old versions of NT w..."]
well obviously is implied that you would change your null peter per the nt version 👮

[2024-01-25 22:51] Matti: yes, but what if some good samaritan has already allocated this page and filled it with harmless nops

[2024-01-25 22:52] Matti: then what, huh?!

[2024-01-25 22:52] Matti: you're gonna be executing 4K nops and still fail to crash

[2024-01-25 22:54] Terry: [replying to Matti: "yes, but what if some good samaritan has already a..."]
well obviously its implied you would search for a valid null peter before you where to deref it

[2024-01-25 22:54] Matti: mmmmm

[2024-01-25 22:54] diversenok: You can let the chance to decide and jump into `KUSER_SHARED_DATA->SystemTime` 🤪

[2024-01-25 22:54] Matti: implementation  left as an exercise for the reader

[2024-01-25 22:54] Terry: [replying to Matti: "mmmmm"]
bill gates told me this is what exit does under the hood

[2024-01-25 22:55] Matti: bill gates is a phoney

[2024-01-25 22:55] Matti: I only trust what dave cutler says

[2024-01-25 22:55] Matti: 
[Attachments: cutler.mp4]

[2024-01-25 23:13] x86matthew: [replying to Matti: ""]
almost enough to run windows 11 smoothly!

[2024-01-25 23:14] Matti: nah I think we're a few decades away from that type of technology

[2024-01-25 23:20] fain: [replying to Matti: ""]
waltuh

[2024-01-25 23:58] dullard: [replying to Terry: "well obviously is implied that you would change yo..."]
null peter

[2024-01-25 23:58] dullard: https://tenor.com/view/petah-peter-gif-26294286

[2024-01-26 01:46] Terry: [replying to dullard: "null peter"]
https://tenor.com/view/family-guy-peter-griffin-dancing-gif-3428352
[Embed: Boared]

[2024-01-26 16:33] mishap: [replying to Matti: "bill gates is a phoney"]
he's good at BASIC!!!!