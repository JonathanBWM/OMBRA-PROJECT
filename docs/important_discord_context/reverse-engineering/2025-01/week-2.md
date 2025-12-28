# January 2025 - Week 2
# Channel: #reverse-engineering
# Messages: 109

[2025-01-06 16:05] mrexodia: [replying to Nobody: "Do you want me to send you the task I'm researchin..."]
I'd also be curious!

[2025-01-06 16:33] Mysterio: started using ida on windows, using the dark theme, when you minimize and maximize do you guys also get a white screen for 100ms?

[2025-01-06 17:06] Deleted User: nop

[2025-01-06 18:23] pinefin: how do you guys transfer ida databases from computer to computer

[2025-01-06 18:24] pinefin: i dont like storing them on google manually, does anyone use github for this

[2025-01-06 18:30] contificate: Save em on shared storage

[2025-01-06 18:33] pinefin: no i mean

[2025-01-06 18:33] pinefin: from multiple different places

[2025-01-06 18:33] pinefin: i use ida at work sometimes

[2025-01-06 18:33] pinefin: and sometimes at home

[2025-01-06 18:34] pinefin: i think git just might be my solution for ida databases

[2025-01-06 18:37] elias: yeah git sounds like a good solution

[2025-01-06 18:43] diversenok: Git on binary files though...

[2025-01-06 18:43] diversenok: Do you really need a version control for that

[2025-01-06 18:43] diversenok: GitHub is not a file sharing service

[2025-01-06 18:44] 5pider: i also sometimes use it as a file sharing system

[2025-01-06 18:48] pinefin: [replying to diversenok: "GitHub is not a file sharing service"]
it doesnt really matter, its made so simple to just push, commit and explain what has changed in the commit

[2025-01-06 18:49] pinefin: and its a private repository so i'm not bloating anyone elses use of the service besides less than 5mb for a ida db (microsoft can afford 5mb with all the bs them and their shareholders push)

[2025-01-06 18:50] elias: btw, did anyone here manage to get his github account flagged for spam before?

[2025-01-06 18:50] pinefin: whos

[2025-01-06 18:52] diversenok: > less than 5mb for a ida db
More like, for less than 5mb for each new commit of a ida db

[2025-01-06 18:52] diversenok: But yeah, I mean, it's gonna work, sure

[2025-01-06 18:53] pinefin: [replying to diversenok: "> less than 5mb for a ida db
More like, for less t..."]
this is true

[2025-01-06 18:53] diversenok: Just saying that it might not be the best tool for that

[2025-01-06 18:53] diversenok: ¯\_(ツ)_/¯

[2025-01-06 18:53] pinefin: eh i pay for copilot

[2025-01-06 18:53] pinefin: they can eat it

[2025-01-06 18:53] pinefin: its simple for me

[2025-01-06 18:54] pinefin: holy shit that’s a coincidence
[Attachments: IMG_4053.png]

[2025-01-06 18:54] pinefin: just now they pulled money out

[2025-01-06 18:54] pinefin: what the fuck

[2025-01-06 18:54] elias: XD

[2025-01-06 18:54] pinefin: theyre fucking listening

[2025-01-06 18:54] diversenok: Hahaha

[2025-01-06 18:55] pinefin: thats the creepiest thing thats happened in my life

[2025-01-06 19:28] 𝚜𝚊𝚟𝚘𝚒𝚛𝚎-𝚏𝚊𝚒𝚛𝚎: Hehe

[2025-01-06 19:37] contificate: [replying to pinefin: "from multiple different places"]
shared storage

[2025-01-06 19:37] contificate: do you just not know what iSCSI is

[2025-01-06 19:37] contificate: do you just not know what NFS is

[2025-01-06 19:37] contificate: or gfs2

[2025-01-06 19:37] pinefin: no i dont sir

[2025-01-06 19:38] contificate: brilliant

[2025-01-06 19:38] pinefin: ☠️

[2025-01-06 19:38] pinefin: im not into networking, i just use my computer and program

[2025-01-06 20:29] Mysterio: [replying to pinefin: "how do you guys transfer ida databases from comput..."]
256gb usb-c stick

[2025-01-06 20:29] Mysterio: git on large files blows

[2025-01-06 20:29] Mysterio: if you're talking liek 5mb idbs then sure, but a 50mb exe can be a 1gb idb

[2025-01-06 20:31] Mysterio: always keep the idbs on the flash drive lets you work on it wherever.... just don't knock it else your changes won't be saved and ida willcrash... but i haven't actually had any corruption

[2025-01-06 20:41] pinefin: yeah thats what i want to stray away from and not have to bring a usb everywhere i go

[2025-01-07 09:37] nu11sec: How you guys handle large PE files with large .text sections in IDA keeps freezing already analyzed everything

[2025-01-07 11:41] Matti: [replying to pinefin: "holy shit that’s a coincidence"]
I had this issue with github due to a game repository I've got which has about 1 TB of LFS files

[2025-01-07 11:41] Matti: I got around it by switching to azure LFS

[2025-01-07 11:42] Matti: it's free with unlimited bandwidth and storage so long as your team is under 5 users

[2025-01-07 11:42] Matti: so I made two accounts, one for me and one with read-only access for everybody else

[2025-01-07 11:50] Brit: <:topkek:904522829616263178>

[2025-01-07 11:50] Brit: is it ethical to abuse ms for free bandwidth

[2025-01-07 12:38] Matti: yes

[2025-01-07 12:38] Matti: what a question

[2025-01-07 13:55] Mysterio: Is it abuse if it's allowed

[2025-01-07 13:56] Mysterio: Abuse would be creating 50 Google accounts and weaving them together for 500gb of storage

[2025-01-07 14:52] Deleted User: [replying to Matti: "it's free with unlimited bandwidth and storage so ..."]
how do i set that up 👀

[2025-01-07 15:46] Matti: [replying to Deleted User: "how do i set that up 👀"]
https://dev.azure.com

Log in with any MS account (or create an @outlook.com one, that's what I did for the second account)
Create an organization and (a) repo(s) in the organization

You'll need to set up access permissions for each user and a way for them to authenticate. For myself I added my public SSH key, for the 'everyone' user I made a PAT (note: these expire after a max of 1 year, but you can edit their expiration date at any time, so set a reminder in your calendar to do this every ~360 days or so)

---

Add Azure repo as remote:
Default git auth: `git remote add azure https://username@dev.azure.com/username/reponame/_git/reponame`
SSH: `git remote add azure-ue-ssh git@ssh.dev.azure.com:v3/username/reponame/reponame`
PAT: `git remote add azure https://username:<PAT string>@dev.azure.com/username/reponame/_git/reponame`

Note that the last one will have the PAT in plaintext. This is obviously not secure, but in my case since the everyone user does not have write access and it's even worse to hand out the private key for an SSH identity, I went with that

---

Work around some broken stuff in LFS and/or Azure:
```
[http]
    # https://stackoverflow.com/questions/66211105/pushing-local-git-repo-with-lfs-enabled-to-an-empty-azure-devops-fails
    version = HTTP/1.1

[lfs "https://dev.azure.com/username/reponame/_git/reponame/info/lfs/objects/"]
    access = basic
    locksverify = false
```

[2025-01-07 15:48] Deleted User: tysm

[2025-01-07 15:49] varaa: i get ptsd when i see the word azure

[2025-01-07 15:50] varaa: all those az-500 courses about azure ad/entra

[2025-01-07 15:54] Matti: yeah but there is no need to use the actual azure environment for anything, I'm still hosting the repo itself on github with the azure copy only acting as a mirror that also has the LFS objects

[2025-01-07 16:01] pinefin: [replying to Matti: "https://dev.azure.com

Log in with any MS account ..."]
thank you matti

[2025-01-08 01:37] BingeBot: [replying to pinefin: "how do you guys transfer ida databases from comput..."]
i use hexrays hexvault, if you aren't trying to purchase a full license for it im pretty sure it got leaked in the 9.0 beta also, but you might have to patch the license check on the server and maybe tls on the client

[2025-01-08 04:23] expy: [replying to p1ink0: "That’s dope"]
Check out this new one, really curious what you think
https://youtu.be/TfHqQ0EPPoM
[Embed: Side channel attacks on obfuscated code]
Start at https://shifting.codes

[2025-01-08 04:57] Xits: Are there settings I can change in x64dbg to decrease lag in the debugee? I am only interested in catching when the application crashes due to an unhandled exception. So i dont care about memory updates... etc until then

[2025-01-08 04:58] Xits: I am running it through wine so I think that's why but maybe some x64dbg wizard knows some solution

[2025-01-08 09:10] p1ink0: [replying to expy: "Check out this new one, really curious what you th..."]
For sure

[2025-01-08 10:05] 5pider: [replying to Xits: "I am running it through wine so I think that's why..."]
are you debugging a windwos executable under Linux with x64dbg running under wine ?

[2025-01-08 10:44] Xits: [replying to 5pider: "are you debugging a windwos executable under Linux..."]
Yes

[2025-01-08 10:49] Xits: I figure it’s some thing x64dbg is constantly updating which is slow under wine. But I’d just like x64dbg to do nothing until the process crashes

[2025-01-08 10:54] Brit: Why not just write a tiny tool then? DebugActiveProcess & WaitForDebugEvent

[2025-01-08 10:54] Brit: https://learn.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-debug_event
[Embed: DEBUG_EVENT (minwinbase.h) - Win32 apps]
Describes a debugging event.

[2025-01-08 10:55] Brit: pass everything except eventcode == 1

[2025-01-08 11:02] Xits: Yea that doesn’t look hard. I could also just write a tool to hang the process on an unhandled exception and attach x64dbg probably

[2025-01-08 11:04] Brit: idk what you're doing ofc so you might actually need x64dbg but if it's just tracking down one specific exception, I like small specific tools

[2025-01-08 22:01] kcahres: Does anybody here have a little bit of knowledge of ghidra decompiler? I can't understand how decompiler <-> ghidra communicates

[2025-01-09 16:01] mrexodia: [replying to Xits: "I figure it’s some thing x64dbg is constantly upda..."]
can you reproduce this slowness?

[2025-01-10 04:08] Nobody: [replying to mrexodia: "I'd also be curious!"]
Really?) No problem 🙂 
Denuvo on just cause 4 😊

[2025-01-10 07:57] Horsie: <:topkek:904522829616263178>

[2025-01-10 07:57] Horsie: I'm sure sexodia can help with that

[2025-01-10 08:33] Nobody: [replying to Horsie: "I'm sure sexodia can help with that"]
He can help with the sale of my source code to denuvo🤣

[2025-01-10 10:53] mrexodia: [replying to Nobody: "He can help with the sale of my source code to den..."]
If only I still worked for Denuvo, could have made bank 😩

[2025-01-10 10:54] Nobody: [replying to mrexodia: "If only I still worked for Denuvo, could have made..."]
I know you don't work there anymore. You have your own security audit company, if my memory serves me correctly. Just kidding, because you used to work there)

[2025-01-10 10:56] mrexodia: Happy to connect you if you actually want to chat with them though!

[2025-01-10 10:57] Nobody: Denuvo has a relatively simple obfuscator, the only thing is that they have a slightly different obfuscator for il2cpp. Denuvo without multipatterns is somewhat similar to arxan in its loader, and transitions are obfuscated the same way Blizzard does it

[2025-01-10 11:02] mrexodia: 🤔
[Attachments: image.png]

[2025-01-10 11:02] Nobody: [replying to mrexodia: "🤔"]
I was comparing it to newer games with denuvo)

[2025-01-10 11:03] mrexodia: Yeah, individual instruction replacements will always be simple because of their nature

[2025-01-10 11:04] mrexodia: But Arxan definitely isn't similar in difficulty from my (limited) observations

[2025-01-10 11:05] Nobody: [replying to mrexodia: "But Arxan definitely isn't similar in difficulty f..."]
I'm just talking about Denuvo's non-multipatterned obfuscator in the context of arxan. 
And it's pretty clear that denuvo itself is more complicated.

[2025-01-10 11:08] mrexodia: Not really sure what you mean with 'non-multipatterned obfuscator', but I'm afraid I cannot discuss in much more detail. You are of course free to discuss as you wish though (keeping in mind the rules)

[2025-01-10 11:18] Brit: surely they mean replacing N > 1 long sequences

[2025-01-10 20:01] Deleted User: is there a way to change windbgx diasm architecture? its disassembling x360 32bit ppc as 64bit x86

[2025-01-10 20:03] pinefin: i wish there was a way to make windbg faster

[2025-01-10 20:03] pinefin: thats my question

[2025-01-10 20:59] dullard: [replying to pinefin: "i wish there was a way to make windbg faster"]
where is it slow ?

[2025-01-10 20:59] dullard: And how are you connecting to target ?

[2025-01-10 20:59] dullard: kdnet is pretty speedy (at least compared to my experiences with serial)

[2025-01-10 21:03] pinefin: [replying to dullard: "where is it slow ?"]
like if i do !address

[2025-01-10 21:03] pinefin: it takes forever to load pages or whatever

[2025-01-10 21:03] pinefin: i forget what it hangs on

[2025-01-10 21:03] pinefin: but it takes like an hour to map out all addresses to everything

[2025-01-10 21:19] dullard: how are you connected to your target ?

[2025-01-10 21:20] dullard: and what are the targets specs