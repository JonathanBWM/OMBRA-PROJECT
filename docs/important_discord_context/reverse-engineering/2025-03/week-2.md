# March 2025 - Week 2
# Channel: #reverse-engineering
# Messages: 27

[2025-03-03 16:47] Lyssa: I am trying to reverse how something is drawing a window using D2D/DWrite

[2025-03-03 16:47] Lyssa: to replicate the drawing logic myself in my own program

[2025-03-03 16:47] Lyssa: but so far I have found absolutely nothing that can provide info about things like that

[2025-03-03 16:48] Lyssa: static analysis in IDA or something is not an option due to the drawing functions being heavily obfuscated

[2025-03-03 16:49] Lyssa: I thought about writing something using Detours to hook into as many drawing functions as possible and log all the parameters but frankly I don't have enough knowledge to do that and I doubt it would work in this context anyway

[2025-03-03 16:50] Lyssa: any tips? because I'm kinda stuck blindly trying to replicate it by eye

[2025-03-03 17:02] deja: [replying to Lyssa: "I thought about writing something using Detours to..."]
this seems like a viable option? why do you doubt it would work?

[2025-03-03 17:03] deja: I admitedly know nothing about Direct2D specifically

[2025-03-03 17:06] Lyssa: [replying to deja: "this seems like a viable option? why do you doubt ..."]
dunno 😄

[2025-03-03 17:07] Lyssa: just sounds like a pain so I'm subconsciously trying to avoid it

[2025-03-03 17:07] Lyssa: I think I can just x64dbg it though

[2025-03-03 17:07] Lyssa: so I wouldn't need to do any hooking bs

[2025-03-03 17:08] deja: maybe conditional breakpoints + logging is enough to get where you need to go?

[2025-03-03 17:08] Lyssa: yeah that's what I'm thinking

[2025-03-03 17:08] Lyssa: I have a lot more experience in that anyway

[2025-03-03 17:08] deja: if possible maybe consider Frida as well?

[2025-03-03 17:08] Lyssa: so I'll try that first

[2025-03-03 17:09] Lyssa: [replying to deja: "if possible maybe consider Frida as well?"]
wow I didn't know that existed

[2025-03-03 17:09] deja: It's very cool

[2025-03-03 17:09] Lyssa: thanks I might try that if I can't get info from debugger

[2025-03-03 17:09] Lyssa: your help is appreciated 🙃

[2025-03-03 17:10] deja: when applicable Frida's no hassle hooking is really nice, I use it for quick proof-of-concepts before moving on to something more permanent sometimes

[2025-03-08 11:32] Ignotus: Virtual COM port or Network for kernel debugging with windbg?

[2025-03-08 11:39] iPower: network kernel debugging. no question

[2025-03-08 16:33] koyz: unless of course you are a masochist

[2025-03-08 16:39] Brit: even then, there are better things to suffer for :^)

[2025-03-08 16:40] 0xatul: lmao