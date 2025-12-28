# October 2024 - Week 3
# Channel: #reverse-engineering
# Messages: 58

[2024-10-14 00:53] KnightCoder: Hello
[Attachments: image.png]

[2024-10-14 00:54] KnightCoder: Could you anyone know  what "VMProtect(new 30 jmp 3)" mean?

[2024-10-14 04:04] 0x208D9: better option is : `intext:"jmp3" VmProtect`
lmao

[2024-10-14 07:20] KnightCoder: [replying to 0x208D9: "better option is : `intext:"jmp3" VmProtect`
lmao"]
intext: "jmp3" VmProtect?

[2024-10-14 07:47] Azrael: [replying to KnightCoder: "intext: "jmp3" VmProtect?"]
Google searching hacks.

[2024-10-14 08:16] dullard: "dorking"

[2024-10-14 09:44] Azrael: I hate that name.

[2024-10-14 18:22] anyfun: [replying to 0x208D9: "also a small addition regarding that in the plugin..."]
Thanks! I’ll give it a try tomorrow

[2024-10-15 00:19] nobody: [replying to KnightCoder: "Could you anyone know  what "VMProtect(new 30 jmp ..."]
just check the rule files ????

[2024-10-15 00:53] Deleted User: hi im looking for someone extremely experienced with reverse engineering / ethical game hacking dm me if interested veryvery profitable

[2024-10-15 01:18] Azrael: [replying to Deleted User: "hi im looking for someone extremely experienced wi..."]
I actually do unethical game hacking, sorry.

[2024-10-15 01:19] Deleted User: [replying to Azrael: "I actually do unethical game hacking, sorry."]
👨‍💻

[2024-10-15 01:20] Azrael: [replying to Deleted User: "👨‍💻"]
It’s a kink of mine silly 😛

[2024-10-15 01:20] Deleted User: oulala

[2024-10-15 01:20] Deleted User: dirty dawg 💻

[2024-10-15 01:43] Deleted User: [replying to Deleted User: "hi im looking for someone extremely experienced wi..."]
Definitely a North Korean spy

[2024-10-15 01:44] Deleted User: [replying to Deleted User: "Definitely a North Korean spy"]
ur so fed js look the other way wtf

[2024-10-15 01:44] Deleted User: That's exactly what a North Korean spy would say <:mmmm:904523247205351454>

[2024-10-15 01:45] Deleted User: yeah ur getting nuked

[2024-10-15 04:12] 0x208D9: [replying to anyfun: "Thanks! I’ll give it a try tomorrow"]
lemme know if it works, also its a great project

[2024-10-15 21:38] Deleted User: bbl drizzzzy

[2024-10-15 21:38] Deleted User: bbllll drizzyy

[2024-10-15 23:28] Possum: Working on trying to get into a smart lightbulb.     What would yall use to try and decrypt tls if im acting as a mitm.  Can i even do that?

[2024-10-16 02:27] Possum: Nevermind.   Definately isnt practical for this.  Gonna look into the bluetooth instead

[2024-10-16 07:24] Timmy: maybe abuse this? idk, https://mitmproxy.org/

[2024-10-16 08:42] 0x208D9: [replying to Possum: "Working on trying to get into a smart lightbulb.  ..."]
self signed certs <a:1clowning:1087091036662268054>

[2024-10-16 08:59] Possum: [replying to 0x208D9: "self signed certs <a:1clowning:1087091036662268054..."]
If i had access beyond just being able to pcap them this would work.  But i dont and it checks for a trusted cert for all communications it seems

[2024-10-16 09:01] 0x208D9: [replying to Possum: "If i had access beyond just being able to pcap the..."]
oh wait are u looking for attack vectors in there?

[2024-10-16 09:01] Possum: Yeah

[2024-10-16 09:01] 0x208D9: that makes sense go for bluetooth with bettercap

[2024-10-16 09:01] 0x208D9: u might find something

[2024-10-16 09:02] Possum: Yeah im just gonna need to order a bluetooth adapter first.  Seems like it may be less protected tho

[2024-10-16 19:53] ellacrity: I am working on adding a new feature to my PE analyzer library and I am trying to come up with a relatively simple approach to a particular problem. I want to try to guess whether a PE file/image is disk backed or memory mapped. More specific, I want to inspect the image to attempt to find out whether it is using file alignment (512 bytes) or section alignment (4096 bytes).

My idea is to basically peek into the image and see if there is null padded to indicate it is section aligned. However, my worry is that I will read into a guard page or other memory region that will cause a crash due to access violation(s)

[2024-10-16 19:54] ellacrity: Is there a safe way to do this? Do I need to use `VirtualQuery` or similar function from WinAPI? Right now the only dependency I have is on WinAPI type bindings, but I am not using any actual Win32 or NT API calls

[2024-10-17 06:02] 25pwn: what's the consensus on binary analysis frameworks(eg. angr, triton etc.)?

[2024-10-17 06:19] Timmy: [replying to ellacrity: "Is there a safe way to do this? Do I need to use `..."]
on disk sections will be the on disk size and properly mapped you'll see sections are the virtual size. Just the first thing that comes to mind.

[2024-10-17 11:48] x86matthew: [replying to ellacrity: "I am working on adding a new feature to my PE anal..."]
not sure i understand what you're trying to do, if the image is mapped into memory then it'll be mapped accordingly

[2024-10-17 11:49] x86matthew: if you're reading it from disk then it won't be

[2024-10-17 11:49] x86matthew: in what situation would you not know where you're reading it from?

[2024-10-17 11:49] x86matthew: or are you trying to scan for embedded PE files in memory or something

[2024-10-17 12:11] Redhpm: Maybe for forensics analysis, binwalk could give you both I guess?

[2024-10-17 14:03] hxm: Can someone point me out the function used to dump the xdbg trace into csv 

https://github.com/x64dbg/x64dbg/tree/49c87f21d3bab4ae7da766482f6e339bce7a054e/src/gui/Src/Tracer
[Embed: x64dbg/src/gui/Src/Tracer at 49c87f21d3bab4ae7da766482f6e339bce7a05...]
An open-source user mode debugger for Windows. Optimized for reverse engineering and malware analysis. - x64dbg/x64dbg

[2024-10-17 14:10] 0x208D9: https://github.com/x64dbg/x64dbg/blob/development/src/gui/Src/Utils/MiscUtil.cpp
[Embed: x64dbg/src/gui/Src/Utils/MiscUtil.cpp at development · x64dbg/x64dbg]
An open-source user mode debugger for Windows. Optimized for reverse engineering and malware analysis. - x64dbg/x64dbg

[2024-10-17 14:11] 0x208D9: and https://github.com/x64dbg/x64dbg/blob/development/src/gui/Src/Tracer/TraceBrowser.cpp
[Embed: x64dbg/src/gui/Src/Tracer/TraceBrowser.cpp at development · x64dbg/...]
An open-source user mode debugger for Windows. Optimized for reverse engineering and malware analysis. - x64dbg/x64dbg

[2024-10-17 14:11] 0x208D9: line number 1800

[2024-10-17 14:12] 0x208D9: [replying to hxm: "Can someone point me out the function used to dump..."]
see if that solves

[2024-10-17 15:53] hxm: [replying to 0x208D9: "see if that solves"]
thanks, but i meant what makes writes to the trace64 file, im looking for this format :
[Attachments: image.png]

[2024-10-17 15:59] hxm: found it 
void DbSave(DbLoadSaveType saveType, const char* dbfile, bool disablecompression)

[2024-10-17 16:00] 0x208D9: [replying to hxm: "thanks, but i meant what makes writes to the trace..."]
oh i thought u were looking for the function that exports to csv

[2024-10-17 21:23] ellacrity: [replying to x86matthew: "in what situation would you not know where you're ..."]
I think this is the correct answer. This is what I was just thinking to myself... there are not many or maybe not any situations where I will not know ahead of time whether it is mapped or on disk.

[2024-10-17 21:23] ellacrity: Sorry for not phrasing my question better. I think I was trying to solve a problem that does not really exist

[2024-10-17 22:15] mrexodia: [replying to hxm: "thanks, but i meant what makes writes to the trace..."]
https://github.com/mrexodia/dumpulator/blob/main/tests/x64dbg-tracedump.py
[Embed: dumpulator/tests/x64dbg-tracedump.py at main · mrexodia/dumpulator]
An easy-to-use library for emulating memory dumps. Useful for malware analysis (config extraction, unpacking) and dynamic analysis in general (sandboxing). - mrexodia/dumpulator

[2024-10-18 02:00] hxm: [replying to mrexodia: "https://github.com/mrexodia/dumpulator/blob/main/t..."]
i was looking for a way to read the struct, i needed it for a tool i crafted that rewrite binary from trace. thankx

[2024-10-18 05:56] Matti: I suppose he could have some file of 'unknown origins', but in that case it will also immediately obvious if the file was dumped as an image

[2024-10-18 05:56] Matti: argh

[2024-10-18 05:56] Matti: replying to old posts 😔

[2024-10-18 05:57] Matti: [replying to ellacrity: "Sorry for not phrasing my question better. I think..."]
yeah - in addition consider that section alignment and file alignment may be the same

[2024-10-18 05:57] Matti: they are often both 32 for EFI files, and sometimes both 4096 for images that are made to run as XIP