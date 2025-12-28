# August 2024 - Week 2
# Channel: #reverse-engineering
# Messages: 12

[2024-08-05 11:06] Horsie: Does the latest Windbg (classic) not work on Windows 7?

[2024-08-05 11:07] Horsie: It segfaults on launch for me

[2024-08-05 16:36] Matti: [replying to Horsie: "Does the latest Windbg (classic) not work on Windo..."]
yeah

[2024-08-05 16:36] Matti: this happens from time to time

[2024-08-05 16:36] Matti: fixed version for this SDK/WDK
[Attachments: Debuggers.26100.7z]

[2024-08-05 16:36] Matti: (it's really only dbgeng.dll that's broken)

[2024-08-07 08:58] marc: [replying to marc: "hey, do you guys have any tips for reversing an ap..."]
thanks for the tips guys, after some days of reversing I managed to find the AES key used 😄

[2024-08-07 19:46] Mysterio: Is there a way to shrink/collapse thunk methods in ida?
Working with an unoptimized build and feels like every function is a jmp to the real function and this is annoying to RE.
Thinking the only way is to modify the bin/idb and patch all calls that only jmp to a call to the real func

[2024-08-07 21:28] BWA RBX: [replying to Mysterio: "Is there a way to shrink/collapse thunk methods in..."]
https://hex-rays.com/blog/igors-tip-of-the-week-31-hiding-and-collapsing/ pretty sure you can automate this using IDAPython to suit your specific needs

[2024-08-07 22:34] Mysterio: [replying to BWA RBX: "https://hex-rays.com/blog/igors-tip-of-the-week-31..."]
Not quite what I meant

[2024-08-07 22:43] BWA RBX: [replying to Mysterio: "Not quite what I meant"]
Yeah you can probably check the analysis options see if they have it as option it's tedious I know but surely they should have something like this as option

[2024-08-08 02:19] Mysterio: setting the "outlined" option almost works, but it breaks back