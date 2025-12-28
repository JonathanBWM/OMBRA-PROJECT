# October 2025 - Week 3
# Channel: #reverse-engineering
# Messages: 44

[2025-10-14 08:42] Obvious: <@609487237331288074> Are you going to continue your series applied-reverse engineering? If not, what would be your recommendation to do next?

[2025-10-14 21:24] miikul: [replying to Y: "Anyone doing flare-on this year?"]
been stuck on 7 for a while, currently banging my head against a wall, but gj getting to the end

[2025-10-15 16:21] onyx: Guys, recommend something to devirtualize the virus-encryptor

[2025-10-15 16:22] onyx: Stack virtual machine (looks like a custom one)

[2025-10-15 16:24] plpg: if its really custom you gotta write something own for it

[2025-10-15 17:10] onyx: [replying to plpg: "if its really custom you gotta write something own..."]
Triton, llvm or …? Is there anything that can simplify the analysis? Modern tools?

[2025-10-15 17:12] plpg: not sure, I would probably try to isolate the code or run it in a debugger (if its not anti-debug protected)

[2025-10-15 17:12] plpg: or run it in an emulator, or do static analysis

[2025-10-15 17:13] onyx: [replying to plpg: "not sure, I would probably try to isolate the code..."]
The code is protected from debugging

[2025-10-15 17:14] onyx: I used intel pintool

[2025-10-15 17:14] plpg: if its not protected from running in a VM you can try to debug it like that. or you can try emulating some snippets that are unclear

[2025-10-15 17:15] plpg: nothing else comes to my mind

[2025-10-15 17:17] onyx: [replying to plpg: "if its not protected from running in a VM you can ..."]
Are there really no working tools to combat virtualization and obfuscation? Chatgpt/deepseek advise llvm ir

[2025-10-15 17:17] plpg: there probably are but if you say its custom then there probably are not for that particular one

[2025-10-15 17:18] plpg: I dont know any. I mostly do static analysis

[2025-10-15 17:20] onyx: [replying to plpg: "I dont know any. I mostly do static analysis"]
I'm afraid that static analysis will take a lot of time

[2025-10-15 17:21] onyx: Although the most effective way

[2025-10-15 17:23] plpg: you can try to defeat the anti debug then

[2025-10-15 17:23] plpg: or you can try to circumvent it

[2025-10-15 17:39] Brit: There is not going to be an off the shelf tool, if the binary is heavily obfuscated you could consider lifting it to ir to optimise some of it away (see remill mergen etc)

[2025-10-15 17:42] onyx: [replying to Brit: "There is not going to be an off the shelf tool, if..."]
I've heard about the triton framework... a symbolic performance. How relevant is it?

[2025-10-15 19:07] plox: I am interested in the idea of setting up my own 're/dev' docker container. so far I have it just install my regular command line tools and bind mount the current directory.

But when I got to setting up my editor I became unsure if I even wanted to include that. Now I am questioning if this even worth it.

My main thing is I want to be able to be on my host system and run potentially unsafe code.

Has anyone done something similar or have recommendations?  ( I am not personally interested in setting up a VM right now, just containers )

[2025-10-15 19:09] plpg: I doubt docker will be enough to run unsafe code, if thats the reason you are using docker id advise against it

[2025-10-15 19:11] plox: [replying to plpg: "I doubt docker will be enough to run unsafe code, ..."]
im not entirely too worried about it, if someone puts a 0day or has a sanbox escape then its whatever I deserve it. But im just doing old CTF challenges

[2025-10-15 19:12] plpg: imo a VM is more useful, because you can virtualize an entire graphical environment

[2025-10-15 19:12] plox: I guess if its really bad and not even worth the effort I am open to new ideas. Really just dont like working in a remote desktop environment

[2025-10-15 19:13] plpg: not sure how docker handless that but it's also more secure (not entirely secure still, but its a case of how much effort you are willing to make)

[2025-10-15 20:00] pinefin: docker inside of a vm

[2025-10-15 20:01] plpg: [replying to pinefin: "docker inside of a vm"]
Whyy

[2025-10-15 20:01] pinefin: im jp

[2025-10-15 20:01] pinefin: you can nest your virtualization but theyre definitely not thinking about nesting a sandbox exit >:)

[2025-10-15 20:01] plpg: True

[2025-10-15 20:01] pinefin: no but i recommend just a whole separate pc thats not connected to any network besides the debugger. nothings like bare hardware

[2025-10-15 20:02] plpg: Something which i find very shocking is that there are many people who only have one computer for everything

[2025-10-15 20:02] Brit: for old ctf challs?

[2025-10-15 20:02] pinefin: [replying to Brit: "for old ctf challs?"]
docker suffices for this

[2025-10-15 20:02] Brit: just run them on your host machine

[2025-10-15 20:02] Brit: its whatever

[2025-10-15 20:02] pinefin: wait yeah for ctf?

[2025-10-15 20:02] plpg: For ctf yes but that enforces bad habits..

[2025-10-15 20:02] plpg: Whatever

[2025-10-15 20:05] plpg: My "suspicious executables" vm is running windows XP and some ida pro version

[2025-10-16 21:01] Uихуахуа: anyone have reverse cr3 routine from battlefield 6 , how they do it ? from where ? javelin / eeac

[2025-10-19 18:11] BWA RBX: [replying to Uихуахуа: "anyone have reverse cr3 routine from battlefield 6..."]
Get good