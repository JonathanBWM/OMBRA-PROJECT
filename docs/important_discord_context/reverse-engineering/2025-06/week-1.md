# June 2025 - Week 1
# Channel: #reverse-engineering
# Messages: 24

[2025-06-01 19:42] diversenok: <@148095953742725120> I found another function to hate: `NtQueryDebugFilterState`

[2025-06-01 19:42] diversenok: It returns `(NTSTATUS)FALSE` and `(NTSTATUS)TRUE`

[2025-06-01 19:43] diversenok: I cannot unsee it

[2025-06-01 19:47] 0xatul: [replying to diversenok: "I cannot unsee it"]
thanks, Now I cant unsee that either 🙃

[2025-06-01 20:00] Matti: [replying to diversenok: "It returns `(NTSTATUS)FALSE` and `(NTSTATUS)TRUE`"]
ikr

[2025-06-01 20:00] Matti: I only found out about this because I wanted to (had to) patch this function to always return FALSE for SXS

[2025-06-01 20:01] Matti: because otherwise sxs.dll will spam the kernel debugger to death

[2025-06-01 20:01] the horse: [replying to Xits: "which package? My end goal is to be able to call t..."]
https://www.npmjs.com/package/node-processlist this one for example
[Embed: node-processlist]
Gets a list of currently running processes on Windows.. Latest version: 1.0.2, last published: 4 years ago. Start using node-processlist in your project by running `npm i node-processlist`. There is 1

[2025-06-01 20:01] the horse: from what i've seen, they're usually just shipped as extra processes that take arguments

[2025-06-01 20:01] the horse: I'm not sure how ffi works exactly with node

[2025-06-01 20:03] the horse: https://www.npmjs.com/package/systeminformation
[Embed: systeminformation]
Advanced, lightweight system and OS information library. Latest version: 5.27.1, last published: 7 days ago. Start using systeminformation in your project by running `npm i systeminformation`. There a

[2025-06-01 20:04] diversenok: [replying to Matti: "I only found out about this because I wanted to (h..."]
It's funny that even SxS devs aren't thrilled with it
```c
NTSTATUS
NTAPI
FusionpNtQueryDebugFilterState_DownlevelFallback(ULONG ComponentId, ULONG Level)
{
    return FALSE; // total abuse of NTSTATUS API but it's how NtQueryDebugFilterState is written...
}
```

[2025-06-01 20:05] Matti: 
[Attachments: image.png]

[2025-06-01 20:05] Matti: `return (NTSTATUS)TRUE;` does it hurt or what <:yea:904521533727342632>

[2025-06-01 20:07] 0xatul: Cardinal sin bro

[2025-06-01 20:09] Matti: [replying to diversenok: "It's funny that even SxS devs aren't thrilled with..."]
it's a lot more tame in newer windows versions I believe

[2025-06-01 20:09] Matti: NT 5.1/5.2 are especially bad

[2025-06-01 20:11] Matti: I'm pretty sure every SXS/FUSION message is sent with level = 0 aka error

[2025-06-01 20:12] Matti: and since they aren't one liners but entire files... yeah

[2025-06-01 20:13] diversenok: Yeah...

[2025-06-01 20:16] the horse: [replying to Matti: ""]
🙏

[2025-06-01 22:19] ruan: theres any shortcut on ida pro to give me the address of a line?

[2025-06-01 22:25] the horse: easiest would be to synchronize with ida view

[2025-06-01 22:25] the horse: one line can correspond to many instructions