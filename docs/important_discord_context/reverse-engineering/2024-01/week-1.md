# January 2024 - Week 1
# Channel: #reverse-engineering
# Messages: 8

[2024-01-03 00:34] Deleted User: hey, little confused about something. I'm currently reverse engineering malware and trying to find their anti-debugging methods and I came across an NtGlobalFlag check however they'd use a bitwise and operation on the flag value with 0x100. I'm kind of confused as to why they'd do that. anyone seen something similar or if i'm missing out on something? I don't see how (0x70 >> 8) & 1 being 0 would help find out if the process was created in a debugger
```x86asm
mov rcx, gs:60h
mov edx, [rcx + 0BCh] 
shr edx, 8
test dl, 1 
```

[2024-01-03 00:52] Pepsi: its checking `FLG_APPLICATION_VERIFIER`, maybe this code is supposed to detect dlls being injected using avrf ?

[2024-01-03 03:29] Deleted User: [replying to Pepsi: "its checking `FLG_APPLICATION_VERIFIER`, maybe thi..."]
is there any information regarding this for malicious purposes, or vice versa? it's all in usermode so I'm somewhat confused as to what they could be doing if they were using this flag. I figured it would have something to do with detecting a debugger since if the conditions are met it'll forcefully exit.

[2024-01-03 11:48] Brit: https://github.com/namazso/SecureUxTheme/blob/master/AVRF.md
[Embed: SecureUxTheme/AVRF.md at master · namazso/SecureUxTheme]
🎨 A secure boot compatible in-memory UxTheme patcher - namazso/SecureUxTheme

[2024-01-03 17:34] daax: [replying to Brit: "https://github.com/namazso/SecureUxTheme/blob/mast..."]
<@116246611734036484> now this is based

[2024-01-07 22:06] vendor: how can i get IDA to forget what is in the GS segment? i don't want it trying to put windows structures over there

[2024-01-07 22:17] vendor: 
[Attachments: image.png]

[2024-01-07 22:17] vendor: like this shit