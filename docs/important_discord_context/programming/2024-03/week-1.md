# March 2024 - Week 1
# Channel: #programming
# Messages: 19

[2024-03-01 17:27] Torph: [replying to naci: "first time hearing something like that"]
I've seen it with some C# source code that called some stuff in Defender

[2024-03-01 17:28] Torph: [replying to Bored engineer: "i copied from https://www.revshells.com/"]
oh that's probably a big part of it then

[2024-03-01 17:29] Torph: [replying to Bored engineer: "because it's .py file for easy distribution of it ..."]
if you need an executable and to obfuscate the source: why write in a language that makes that extremely annoying? if those are priorities you should be using a compiled language

[2024-03-01 21:21] rin: when using createprocessA what is the proper way of passing a comandline? im trying to pass "start notepad" but cant see to get it right. only times createprocessA works for me is when i pass the absolute file path and only if the path has no spaces

[2024-03-01 21:22] mrexodia: [replying to rin: "when using createprocessA what is the proper way o..."]
'proper' is meh

[2024-03-01 21:22] mrexodia: but you should be able to pass nullptr for the first argument and then pass `start notepad` as the command line

[2024-03-01 21:23] rin: [replying to mrexodia: "but you should be able to pass nullptr for the fir..."]
thats what im doing

[2024-03-01 21:23] mrexodia: is there a start.exe?

[2024-03-01 21:23] mrexodia: you should also be able to do application = "start.exe" and then the command line

[2024-03-01 21:23] mrexodia: but if you want to start notepad

[2024-03-01 21:23] mrexodia: just do application = "notepad.exe"

[2024-03-01 21:23] rin: [replying to mrexodia: "just do application = "notepad.exe""]
ik but future use case

[2024-03-01 21:24] mrexodia: start is never useful with createprocess

[2024-03-01 21:25] Matti: yeah, why add 'start'

[2024-03-01 21:25] rin: [replying to mrexodia: "start is never useful with createprocess"]
the reason i was using start was when trying to open a new powershell window i need to use start since otherwise it just creates the process on the comand line

[2024-03-01 21:25] Matti: there's a createprocess flag to create a new window

[2024-03-01 21:25] mrexodia: you should pass the argument to not inherit the console

[2024-03-01 21:26] rin: let me looks at docs lol

[2024-03-01 21:45] rin: [replying to Matti: "there's a createprocess flag to create a new windo..."]
found it thanks