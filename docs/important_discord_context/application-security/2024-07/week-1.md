# July 2024 - Week 1
# Channel: #application-security
# Messages: 16

[2024-07-02 07:48] UnderShell: Hi guys! Looking for some ideas. I have an parameter called parentPath which I can use to list all files in a directory by doing  like `//127.0.0.1/c$` - but I can't seem to print files with it, only directories. Are there any special tricks or things I can try to exploit this more?

[2024-07-02 14:53] szczcur: [replying to UnderShell: "Hi guys! Looking for some ideas. I have an paramet..."]
need more information. what are you currently doing? what does the function with parentPath look like?

[2024-07-03 13:21] UnderShell: [replying to szczcur: "need more information. what are you currently doin..."]
Sadly I don't have the code to it, so it's black box. It's a get parameter in a request

[2024-07-03 13:22] UnderShell: looks like this `?parentPath=//127.0.0.1/C$&fileSystem=local`

[2024-07-03 13:57] Deleted User: [replying to UnderShell: "looks like this `?parentPath=//127.0.0.1/C$&fileSy..."]
Backend wise, are they using php, because they could be doing echo include, and you can get some LFI

[2024-07-03 13:58] Deleted User: What you can do is use brup and have a bunch of LFI payloads, but is it an rdp? Or an Linux distro

[2024-07-03 14:01] Deleted User: You can do a pram spider to see if you can find more prams via that route

[2024-07-03 14:01] UnderShell: [replying to Deleted User: "Backend wise, are they using php, because they cou..."]
I think Java or something, it's not PHP at least

[2024-07-03 14:01] UnderShell: maybe .NET

[2024-07-03 14:01] Deleted User: Ah okay, well Java not sure about

[2024-07-03 14:01] Deleted User: Are there routes

[2024-07-03 14:01] Deleted User: Or does the file end with an extension

[2024-07-03 14:02] Deleted User: Like .asp

[2024-07-03 14:02] Deleted User: Or .php

[2024-07-03 14:02] Deleted User: Also is the box, windows or linux

[2024-07-03 14:02] Deleted User: Bc if it's windows they could be using azure