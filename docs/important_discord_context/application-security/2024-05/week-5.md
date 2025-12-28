# May 2024 - Week 5
# Channel: #application-security
# Messages: 40

[2024-05-30 23:22] TRUCK: rate these protectors 1-10, themida, vmp, obsidium

[2024-05-30 23:53] daax: [replying to TRUCK: "rate these protectors 1-10, themida, vmp, obsidium"]
themida=9
vmp=7
obsidium=<:lmao3d:611917482105765918>

[2024-05-30 23:54] TRUCK: [replying to daax: "themida=9
vmp=7
obsidium=<:lmao3d:6119174821057659..."]
thanks i was thinking the same

[2024-05-30 23:54] TRUCK: i wanted to hear what other people thought

[2024-05-31 00:01] estrellas: what about viper?

[2024-05-31 00:01] estrellas: oh they changed it to CodeDefender

[2024-05-31 01:44] Matti: [replying to daax: "themida=9
vmp=7
obsidium=<:lmao3d:6119174821057659..."]
I agree with this ranking, but I would still like to say that obsidium deserves at least some points for having inspired revolutionary - some would even say *genius* - innovations in code that would never have been needed or even imagineable if obsidium had not existed to constantly set a lower bar for its own quality

[2024-05-31 01:47] Matti: https://github.com/x64dbg/ScyllaHide/commit/5ad4cb7 is a good example of absolutely revolutionary code (again, some would even call it genius) that could never have been thought up if obsidium had not been as bad as it is
[Embed: Obsidium please (2/3) · x64dbg/ScyllaHide@5ad4cb7]
- Write WOW64 x64 transition detours as far jmps

Obsidium checks syscalls for hooks, or at least it tries to. But if something only kinda looks like the original it&#39;s good enough. And thus...

[2024-05-31 01:53] Matti: people are always going on about the threats of emerging AI

[2024-05-31 01:54] Matti: but will there ever be an AI that is the *opposite* of intelligent, to the extent the author of obsidium is?

[2024-05-31 01:55] Matti: obviously not, because then how could it be AI

[2024-05-31 01:56] Matti: his job will always be safe

[2024-05-31 02:58] snowua: [replying to Matti: "https://github.com/x64dbg/ScyllaHide/commit/5ad4cb..."]
<:kekw:904522300257345566>

[2024-05-31 02:59] snowua: [replying to daax: "themida=9
vmp=7
obsidium=<:lmao3d:6119174821057659..."]
-1 point for spinlock

[2024-05-31 11:38] iPower: is obsidium still a thing lol

[2024-05-31 11:39] iPower: havent heard about it for years

[2024-05-31 11:39] Brit: they're still releasing updates

[2024-05-31 13:22] daax: [replying to Matti: "obviously not, because then how could it be AI"]
artificial unintelligence

[2024-05-31 13:46] @Cypher - Read Bio: [replying to Matti: "I agree with this ranking, but I would still like ..."]
me who owns an obsidium license

[2024-05-31 14:46] Torph: ive never heard of any of these 3 products 😂

[2024-05-31 15:13] sariaki: not even vmprotect?

[2024-05-31 17:38] Torph: i think ive heard the name somewhere but im not even really sure what it does

[2024-05-31 17:42] Deleted User: [replying to Matti: "but will there ever be an AI that is the *opposite..."]
are there any blogs abt obsidium, i couldnt find much about it. what makes it so bad in your opinion?

[2024-05-31 17:47] Matti: I can't comment re: its virualization, I've never even looked at it because I don't know shit about this anyway

[2024-05-31 17:47] Matti: someone else will have to fill in why **that** particular part of obsidium is also shit

[2024-05-31 17:51] Matti: but re: its anti-debug capabilities: everything it does is either
1. ancient and well-known to the point that it's not really meaningfully different from calling `IsDebuggerPresent()` and calling it a day
or
2. some lame and annoying window detection based technique (using a hardcoded list of "bad" program/window names of course)

[2024-05-31 17:53] Matti: if you buy (yeah I said buy) a program that is protected by obsidium, you can never use that program while having one of IDA, windbg or x64dbg running for instance

[2024-05-31 17:54] Matti: (unless you install scyllahide and just start the program straight from the debugger, because as I said everything it does is trivially bypassed)

[2024-05-31 17:58] Deleted User: [replying to Matti: "if you buy (yeah I said buy) a program that is pro..."]
isnt that the same with alot of games tho, i just have it as a habit to not run re tools when playing

[2024-05-31 17:59] Matti: Hmm well I don't play a lot of games, but pretty sure this is only true for those that have anti cheats

[2024-05-31 17:59] Matti: I've never had this issue myself at least

[2024-05-31 18:00] Matti: except with elden ring, which for some reason has an anti cheat even when you're playing single player

[2024-05-31 18:00] Torph: huh?? wtf

[2024-05-31 18:01] Torph: [replying to Matti: "if you buy (yeah I said buy) a program that is pro..."]
wait so is it like an anti-RE software packaging thing? I thought it was some sort of AV vendor

[2024-05-31 18:01] Matti: well, I don't think EAC cares about someone having IDA open - it's a bit more thorough than that
that's why the obsidium checks are so lame

[2024-05-31 18:01] Matti: [replying to Torph: "wait so is it like an anti-RE software packaging t..."]
the former

[2024-05-31 18:02] Matti: but, it is similar to an AV product for sure, in that they are both snake oil

[2024-05-31 18:23] Torph: <:kekw:904522300257345566>

[2024-05-31 20:54] gogo: I ran `ysoserial.exe -f BinaryFormatter -g TypeConfuseDelegateMono -c calc.exe`

```ysoserial.exe -f BinaryFormatter -g TypeConfuseDelegateMono -c calc.exe

Exception non gérée : System.NullReferenceException: La référence d'objet n'est pas définie à une instance d'un objet.
   à ysoserial.Generators.TypeConfuseDelegateMonoGenerator.TypeConfuseDelegateGadget(InputArgs inputArgs) dans D:\a\ysoserial.net\ysoserial.net\ysoserial\Generators\TypeConfuseDelegateMonoGenerator.cs:ligne 83
   à ysoserial.Generators.TypeConfuseDelegateMonoGenerator.Generate(String formatter, InputArgs inputArgs) dans D:\a\ysoserial.net\ysoserial.net\ysoserial\Generators\TypeConfuseDelegateMonoGenerator.cs:ligne 43
   à ysoserial.Generators.GenericGenerator.GenerateWithInit(String formatter, InputArgs inputArgs) dans D:\a\ysoserial.net\ysoserial.net\ysoserial\Generators\GenericGenerator.cs:ligne 75
   à ysoserial.Program.Main(String[] args) dans D:\a\ysoserial.net\ysoserial.net\ysoserial\Program.cs:ligne 302```

Is my command / setup wrong or is there a bug in ysoserial please? I Openned an issue.

[2024-05-31 21:46] @Cypher - Read Bio: [replying to Matti: "someone else will have to fill in why **that** par..."]
never seen any writeup abt its virt but its contantly doing unconditonal jumps . I own a personal license for obsidium and its not always easy to use their macros (opcode support). Their FindWindow is a meme but antidbg dont matter anyway. Their licensing sdk for instance is very strong and comes with lots of features.