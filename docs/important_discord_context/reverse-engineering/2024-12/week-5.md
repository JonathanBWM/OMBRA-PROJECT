# December 2024 - Week 5
# Channel: #reverse-engineering
# Messages: 23

[2024-12-23 15:06] toox: [replying to Glatcher: "Guys, I'm developing overlay, which reads/writes g..."]
Is this a real question

[2024-12-23 15:06] Glatcher: [replying to toox: "Is this a real question"]
yes

[2024-12-23 15:07] Glatcher: 
[Attachments: image-5.png, image-4.png]

[2024-12-23 15:14] Brit: <:kekw:904522300257345566>

[2024-12-23 15:34] BWA RBX: [replying to Glatcher: ""]
Why mix between languages

[2024-12-23 16:08] Glatcher: [replying to BWA RBX: "Why mix between languages"]
its not my screenshot

[2024-12-23 16:34] BWA RBX: [replying to Glatcher: "its not my screenshot"]
Okay

[2024-12-29 13:44] marc: <@543396717614333962> I had a question for you about the bug in csgo's protobuf comms, how did you figure out `CSVCMsg_SplitScreen` message could have an OOB access?
did you reverse the function that handles it first and then figured it could be vulnerable? or when reading the proto definition you just thought it could potentially lead to OOB if it wasn't checked properly

[2024-12-29 14:01] brymko: [replying to marc: "<@543396717614333962> I had a question for you abo..."]
all correct except we didn’t reverse we read the source

[2024-12-29 14:03] marc: oh the message handling was open source?

[2024-12-29 14:08] brymko: leaked source from 2018

[2024-12-29 14:08] marc: I was looking into another app that has Client <=> Server <=> Client protobuf communication, it's quite big, many .proto structures and the server seems to be quite restrictive about what you actually send in protobuf messages, so I was wondering aobut the methodology you guys followed

[2024-12-29 14:08] marc: [replying to brymko: "leaked source from 2018"]
ahhhh I see

[2024-12-29 14:08] brymko: but we did write a fuzzer for c2s

[2024-12-29 14:08] brymko: for the proto defs

[2024-12-29 14:08] marc: did you use libprotobuf-mutator?

[2024-12-29 14:08] brymko: no

[2024-12-29 14:09] brymko: it was house made

[2024-12-29 14:09] marc: I see

[2024-12-29 14:09] brymko: quick n stupid

[2024-12-29 14:09] marc: yeah I thought about doing that as well, it may be the best idea

[2024-12-29 14:09] marc: did you find anything with the fuzzer?

[2024-12-29 16:58] brymko: nah their server side was sound