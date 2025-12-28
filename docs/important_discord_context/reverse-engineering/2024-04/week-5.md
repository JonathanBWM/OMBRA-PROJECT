# April 2024 - Week 5
# Channel: #reverse-engineering
# Messages: 26

[2024-04-30 16:23] Bryce: 
[Attachments: image.png]

[2024-04-30 16:23] Bryce: how do i disable this

[2024-04-30 16:25] jvoisin: patch it?

[2024-04-30 16:25] Bryce: im super new to this

[2024-04-30 16:25] Bryce: i need to return a value of 0

[2024-04-30 16:26] Bryce: although i found this in one of the dll files

[2024-04-30 16:26] Bryce: 
[Attachments: image.png]

[2024-04-30 16:34] jvoisin: then you might want to read some documentation

[2024-04-30 17:40] froj: patch in the peb or nop out calls to it

[2024-04-30 17:41] Brit: this is a X Y problem for sure, we can teach him to patch isdebuggerPresent but it might not even be that check that's catching him in the first place

[2024-04-30 17:44] froj: Yeah seems like it, likely just found it called due to crt

[2024-04-30 19:02] Bryce: any recommendations on where to start

[2024-04-30 19:03] Bryce: however i have IDA free version

[2024-04-30 19:04] Bryce: i also found this somehow

[2024-04-30 19:06] Bryce: im assuming the only place it is found to be true is that second one

[2024-04-30 19:06] Bryce: not sure what that bottom one is doing tho

[2024-04-30 19:25] Brit: I recommend you don't cheat in multiplayer videogames

[2024-04-30 19:25] Bryce: i dont care to make a cheat that would give me an unfair advantage

[2024-04-30 19:25] Bryce: id just like to gain a better understanding of the game

[2024-04-30 19:26] Bryce: maybe make my character blue if possible, just to mess around

[2024-04-30 22:31] daax: <#835634425995853834>

[2024-04-30 22:32] daax: Single-player games are fine to RE and discuss in here, but MMO/MP games -- no go. Not trying to get this discord deleted by the sweaties covered in krispy kreme crumbs @ Discord.

[2024-04-30 22:33] Azrael: Lol.

[2024-04-30 23:54] Bryce: [replying to daax: "Single-player games are fine to RE and discuss in ..."]
thats athing/

[2024-04-30 23:54] Bryce: ?

[2024-04-30 23:55] Bryce: deleted it