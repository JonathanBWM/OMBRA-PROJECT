# August 2025 - Week 1
# Channel: #programming
# Messages: 117

[2025-08-01 11:55] Tequila: I successfully redirect the IAT to my entry point, but then when the process tries to call the "IAT" ( my entry point ) it crashes when calling it. I verified all my sections are getting written correctly. in my dllmain of my testing dll, I am testing with just MessageBox() and it still just crashes when the process tries to call my entry point. the dll memory is getting allocated with execute flag. i mean I dont really know what else to test tbh. Unreal engine crash report says: 

```cpp
Unhandled Exception: EXCEPTION_ACCESS_VIOLATION 0x0000000000078b70
```

[2025-08-01 11:59] Brit: What you mean by iat is an entry in said table right? But anyway attach a debugger and see what causes the program to access that addr

[2025-08-01 12:13] Tequila: [replying to Brit: "What you mean by iat is an entry in said table rig..."]
debug maxing i’m gonna attach a debugger and see what happens

[2025-08-01 12:22] Brit: Well what else were you expecting. Hopefully you didn't fuck up and write code over the iat entry you are replacing and the code you wrote crashes, it is time to go see why

[2025-08-01 12:35] Tequila: <@671779079363624970> yes, I did mean an entry in the iat, switched over to a different game temporarily to test debugging with anticheat off

[2025-08-01 12:36] Tequila: [replying to Brit: "Well what else were you expecting. Hopefully you d..."]
no thats not why, pretty much I am writing TranslateMessage to my entrypoint  and I mean everything works fine, but the second my entry point gets hit I get an access violation

[2025-08-01 12:36] Tequila: (sorry for pings if I wasn't supposed to btw)

[2025-08-01 12:54] Tequila: ```BOOL WINAPI main ( MSG* lpMsg )
{
    MessageBoxA(0, "Payload triggered from IAT", "DLL", MB_OK);

    return TRUE;
}```

well starting off this is my stub in the games memory, and my dllmain code. But the entrypoint gets called fine and so does the call for the messagebox. The crash happens on the instruction "mov eax,1"

Some output from x64dbg:
First chance exception on 0000000000078870 (C0000005, EXCEPTION_ACCESS_VIOLATION)!

000000A73017F038  000001BF0C361022  return to 000001BF0C361022 from ???

idk im probably missing something so simple here maybe its the no sleep, but like I said before I am pretty much just writing TranslateMessage to my entrypoint

[2025-08-01 12:54] Tequila: https://media.discordapp.net/attachments/1300634554976964638/1400823200270647327/image.png?ex=688e09c4&is=688cb844&hm=e55722e29efb08f78ac6ba1d65d49525025f7f4250f28f0cd68e72479f5bbd38&=&format=webp&quality=lossless

[2025-08-01 13:35] Brit: [replying to Tequila: "```BOOL WINAPI main ( MSG* lpMsg )
{
    MessageBo..."]
The crash decidedly does not happen on mov eax, 1 it happens on the call just before because you are calling at rip + 0xfde

[2025-08-01 13:36] f00d: looks like u have unresolved import entry

[2025-08-01 13:36] Brit: Which im willing to be is not where messagebox is

[2025-08-01 13:41] Tr1x: [replying to Tequila: "```BOOL WINAPI main ( MSG* lpMsg )
{
    MessageBo..."]
Brotha go check that you've got the correct pointer to the import and go check the IAT you're redirecting from because clearly you're corrupting a shit ton of stuff. I also doubt there is any import that matches your main function define, so you're most likely destroying what is meant to be preserved registers.

[2025-08-01 13:41] Brit: I mean here the first issue is that he thinks that mov eax, 1 is crashing

[2025-08-01 13:42] Tr1x: 💀

[2025-08-01 13:42] Brit: Which tells me he has a somewhat loose comprehension of what he's doing

[2025-08-01 13:42] Tr1x: I think I might just have to back you up on this one

[2025-08-01 13:44] Pepsi: this person is unable to give any exception context + stack dump, and asked me the same in DM

[2025-08-01 13:44] Brit: He shall be the next great p2c

[2025-08-01 13:44] Brit: <:mmmm:904523247205351454>

[2025-08-01 13:45] Pepsi: if i had to guess, its probably recursion issue

[2025-08-01 13:46] Pepsi: iirc msgbox has its own msgloop

[2025-08-01 13:46] Pepsi: and he hooks translatemsg

[2025-08-01 13:46] Brit: Classic

[2025-08-01 13:46] Pepsi: [replying to Pepsi: "this person is unable to give any exception contex..."]
however, without this, its kinda hard to tell if that is actually the issue

[2025-08-01 13:48] Brit: Surely this man is not trying to call translate message in a nested way from his translate message hook

[2025-08-01 13:48] Brit: Surely

[2025-08-01 13:50] Tr1x: That's what he is redirecting, at least the define is correct. Hehe

[2025-08-01 13:51] Tr1x: [replying to Pepsi: "if i had to guess, its probably recursion issue"]
Most likely another issue on top of it but he isn't even reaching the return from TranslateMessage. Crashing calling messagebox

[2025-08-01 13:52] Tr1x: Not to mention he isn't even handling the original function call, so that's also a wonderful idea to do.

[2025-08-01 13:53] Brit: My bet is on failure when manual mapping to do import resolution

[2025-08-01 13:53] Tr1x: ^

[2025-08-01 13:54] Brit: Either way im back to my beach, you guys get to deal with yet another game cheating 🧠 genius

[2025-08-01 13:55] Tr1x: Hope you enjoy but I'm away to the lake, I've said my piece. That genius is too much for me

[2025-08-01 13:58] Pepsi: 
[Attachments: image.png]

[2025-08-01 13:59] Pepsi: [replying to Pepsi: "this person is unable to give any exception contex..."]
so the reason we don't get this, because its not even his issue, but of somebody else lmao

[2025-08-01 14:00] Brit: Tell kola to find a better place to copy paste his injection method from

[2025-08-01 14:00] Brit: The current one does not resolve imports correctly

[2025-08-01 14:08] sariaki: lol what did you guys expect - look at his profile pic for more than a second

[2025-08-01 14:09] Tequila: [replying to sariaki: "lol what did you guys expect - look at his profile..."]
hey im not the paster here

[2025-08-01 14:17] Brit: You did say something crashed with an access violation on mov eax 1

[2025-08-01 14:17] Brit: <:mmmm:904523247205351454> <:mmmm:904523247205351454>

[2025-08-01 14:18] x86matthew: [replying to Brit: "You did say something crashed with an access viola..."]
maybe he has a 14th gen i9 <:topkek:904522829616263178>

[2025-08-01 14:18] Brit: Eax is a protected register now

[2025-08-01 14:18] Brit: New intel spe

[2025-08-01 14:18] Brit: Spec*

[2025-08-01 14:18] Brit: [replying to x86matthew: "maybe he has a 14th gen i9 <:topkek:90452282961626..."]
Real

[2025-08-01 14:18] 0xatul: wtf

[2025-08-01 14:18] 0xatul: really ?

[2025-08-01 14:19] 0xatul: got a link handy ?

[2025-08-01 14:19] Edel: [replying to Tequila: "```BOOL WINAPI main ( MSG* lpMsg )
{
    MessageBo..."]
i think you'll have some sort of corruption if you don't use the proper signature for the main function

[2025-08-01 14:20] sariaki: eax is this new crazy undocumented technology. it's still reserved

[2025-08-01 14:22] Pepsi: [replying to Brit: "My bet is on failure when manual mapping to do imp..."]
i'll revoke my guess, this is probably the case

[2025-08-01 14:22] Pepsi: ```
EXCEPTION_DEBUG_INFO:
           dwFirstChance: 1
           ExceptionCode: C0000005 (EXCEPTION_ACCESS_VIOLATION)
          ExceptionFlags: 00000000
        ExceptionAddress: 0000000000078B70
        NumberParameters: 2
ExceptionInformation[00]: 0000000000000008 DEP Violation
ExceptionInformation[01]: 0000000000078B70 Inaccessible Address
First chance exception on 0000000000078B70 (C0000005, EXCEPTION_ACCESS_VIOLATION)!
```

[2025-08-01 14:23] Tr1x: [replying to sariaki: "eax is this new crazy undocumented technology. it'..."]
It's the microcode's fault

[2025-08-01 14:25] Pepsi: [replying to Brit: "Tell kola to find a better place to copy paste his..."]
i just forwarded this message

[2025-08-01 14:25] x86matthew: the `0x78B70` should be `module_base + 0x78B70` is my guess

[2025-08-01 14:25] x86matthew: but i won't be spending any time on this

[2025-08-01 14:26] Brit: The call is riprel

[2025-08-01 14:26] Brit: I bet he literally just writes his text section in

[2025-08-01 14:26] Brit: No relocations no imports

[2025-08-01 14:26] Brit: We just ball

[2025-08-01 14:29] Kola: fuck it we ball ay

[2025-08-01 14:30] Brit: You have unrecoverable brain damage

[2025-08-01 14:30] Brit: I'm sorry

[2025-08-01 14:36] Pepsi: he is going to be next easy anticheat APT after he figured out how to resolve imports

[2025-08-01 14:46] Kola: [replying to Brit: "You have unrecoverable brain damage"]
did u mean undiscoverable brain damage ?

[2025-08-01 14:48] Brit: Could be both

[2025-08-01 14:48] mtu: [replying to Tequila: "hey im not the paster here"]
You get that’s worse right

[2025-08-01 14:49] mtu: “I’m not the paster I’m just the robot that reposts their questions”

[2025-08-01 14:50] Brit: But that claim cannot be true either way

[2025-08-01 14:50] Brit: See above

[2025-08-01 14:50] Brit: Move reg, immediate generating a protection fault

[2025-08-01 14:50] Brit: Is just brain damaged

[2025-08-01 14:51] mtu: [replying to Tr1x: "It's the microcode's fault"]
The TLB ucode cache probably corrupted the l4 cache 🧠

[2025-08-01 14:51] Tequila: [replying to mtu: "“I’m not the paster I’m just the robot that repost..."]
so asking for help for a friend when I dont know the answer is not allowed/a bad thing? nice....

[2025-08-01 14:51] mtu: No but see above

[2025-08-01 14:52] mtu: Acting like you’re better than the originator of a question when you say brain dead stuff like “mov reg, imm caused a segfault”

[2025-08-01 14:52] mtu: Just own the fact you don’t know, it’s fine

[2025-08-01 14:52] Tequila: [replying to mtu: "Just own the fact you don’t know, it’s fine"]
sorry i don't know, I'll easily admit that for you, no problem with that from me 🙂

[2025-08-01 15:03] Tr1x: [replying to mtu: "The TLB ucode cache probably corrupted the l4 cach..."]
Yeah a solar flare side channel attacked my CPU and fucked up my microcode

[2025-08-01 15:03] Tr1x: [replying to Tequila: "sorry i don't know, I'll easily admit that for you..."]
There is a bit of trolling going on but we have answered your question and have suggested things to check and ensure are correct which will most likely fix your issue

[2025-08-01 15:08] Brit: I suspect there still might be layers to this issue egy, he fixes imports and then runs into the recursive calling of translate message next

[2025-08-01 15:08] Brit: Then into him clobbering regs because his func sig does not match the thing he took over in the iat

[2025-08-01 15:09] Brit: And so on and so forth

[2025-08-01 15:40] Kola: just thought you guys should see this

[2025-08-01 15:40] Kola: [+] - (x64)Import Fixed (USER32.dll)(00007FFD808B0000)(MessageBoxA) at 0000000000078B70

[2025-08-01 15:40] Kola: 💔

[2025-08-01 15:43] Brit: If the module starts at 7ffd808b0000 how can messagebox be lower 🤔

[2025-08-01 15:43] Brit: It's almost as if that's relative to modulebase

[2025-08-01 15:44] x86matthew: i happen to have user32 open in ida lol
[Attachments: image.png]

[2025-08-01 15:46] Brit: But at least they didn't poke your eye out with the orbitoclast so that's a win

[2025-08-01 15:55] Kola: sorry for being so retarded guys

[2025-08-01 15:55] Kola: thank you for all the assistance though

[2025-08-01 15:57] Brit: [replying to Kola: "[+] - (x64)Import Fixed (USER32.dll)(00007FFD808B0..."]
You have everything you need to fix this here

[2025-08-01 15:58] Brit: You have nodbase and you know the address of the function

[2025-08-01 15:58] Brit: You just add them together to get the address you need to call

[2025-08-01 16:01] Brit: Modulebase*

[2025-08-01 16:06] mtu: [replying to Kola: "[+] - (x64)Import Fixed (USER32.dll)(00007FFD808B0..."]
Log statements with the [+] make me instantly think of ai slop code

[2025-08-01 16:07] Kola: [replying to mtu: "Log statements with the [+] make me instantly thin..."]
fair, I personally just think it makes the logs look way cleaner

[2025-08-01 16:24] Lyssa: I used to use it but quit at some point for some reason

[2025-08-01 16:24] Lyssa: probably because it doesn't look good <:yara_lover:1148745271577157673>

[2025-08-01 17:20] Timmy: I still use it and still like it

[2025-08-01 17:22] Matti: [replying to mtu: "Log statements with the [+] make me instantly thin..."]
it reminds me of those hacker PoC programs

[+] found vulnerable bla at 0xbla...
[+] un-hinging the fujingy.... (patience)
[+] exploit SUCCESS: whoami: NT AUTHORITY\SYSTEM

[2025-08-01 17:23] Matti: and then [-] would be for failure I guess but first of all no one ever screenshots their PoCs failing, and second they don't contain error checking code anyway

[2025-08-01 17:24] Timmy: I use + for info, ? for warning, ! for error

[2025-08-01 17:24] Matti: but that's what stdout vs stderr are for

[2025-08-01 17:25] Matti: plus you can still colour code messages if you want

[2025-08-01 17:25] mtu: You don’t just 2>&1 everything???

[2025-08-01 17:25] Matti: yeah I should really make a keyboard shortcut to copy and paste that with 1 button...

[2025-08-01 17:26] Matti: it's the way to go but so annoying to type you know

[2025-08-01 17:26] Timmy: idk it's just habit ig, I never really thought about it again yk

[2025-08-01 17:35] dullard: [replying to mtu: "You don’t just 2>&1 everything???"]
2>nul 
if you can't see errors, there are no errors <:ThatsThinking:746085319132184617>

[2025-08-01 17:37] the horse: Hacking the mainframe... Browsing through cat gifs... Thinking about the meaning of life (in reality Sleep(5000)

[2025-08-01 17:43] Brit: Man has not learned about synchronisation primitives kind of beat

[2025-08-01 18:47] 0xatul: [replying to Timmy: "I use + for info, ? for warning, ! for error"]

[Attachments: 174db384bb0ef4b176dc07c42a7b8f4850cd2da5.png]

[2025-08-01 18:50] Timmy: <:KEKW:798912872289009664>