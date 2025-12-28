# May 2024 - Week 3
# Channel: #reverse-engineering
# Messages: 25

[2024-05-13 03:48] abu: Hello. Anyone here know an easy way to get the exception handler for a range of code in an x64 pe file? (The func called from C_Specific_Handler)

[2024-05-13 08:58] Brit: parse the pdata

[2024-05-13 09:00] Brit: the unwind info should have the handler if the function has one

[2024-05-13 19:07] mishap: [replying to abu: "Hello. Anyone here know an easy way to get the exc..."]
https://github.com/0vercl0k/windbg-scripts/tree/master/parse_eh_win64

[2024-05-13 19:19] Brit: although now that I think of it last I was messing with this to retrieve function sizes (to relocate them mind) I ran into issues with some binaries because the pdata was fucked

[2024-05-14 00:53] abu: [replying to mishap: "https://github.com/0vercl0k/windbg-scripts/tree/ma..."]
Legend thanks

[2024-05-14 01:38] abu: [replying to Brit: "although now that I think of it last I was messing..."]
Did you have any trouble seeing exception related structures IE. SCOPE_RECORD. Windbg preview cannot find these symbols for some reason

[2024-05-17 08:33] projizdivlak: anyone knows what is signed with rsa in steam3 app ownership ticket?

[2024-05-17 09:19] mrexodia: [replying to projizdivlak: "anyone knows what is signed with rsa in steam3 app..."]
nothing, it's a symmetric key (at least if I remember correctly, for these https://partner.steamgames.com/doc/api/SteamEncryptedAppTicket)

[2024-05-17 09:24] projizdivlak: [replying to mrexodia: "nothing, it's a symmetric key (at least if I remem..."]
im new to crypto but, iirc it uses PKCS1 1.5 and also im talking about https://partner.steamgames.com/doc/features/auth -> Session Tickets

[2024-05-17 09:24] projizdivlak: the other app ticket lol

[2024-05-17 09:25] mrexodia: that's not an app ticket then, but a session ticket for the user (it doesn't prove ownership of the application)

[2024-05-17 09:39] projizdivlak: it's passed into `SendUserConnectAndAuthenticate`, also there's an assert `Unknown Steam3 App Ownership Ticket Version: %d\n` in a function where the ticket is used

[2024-05-17 09:41] mrexodia: I never worked with these, only the encrypted app tickets, so not sure

[2024-05-17 09:44] projizdivlak: i think there's encrypted and unencrypted app tickets

[2024-05-18 23:58] abu: Hi! Are instructions such as "mov rax, gs:188h" considered Privileged instructions? Also, is there some criterion/list for privileged instructions or do I need to read definitions in my software manual

[2024-05-19 00:47] irql: in normal situations they are not privileged

[2024-05-19 00:47] irql: if you're on windows, you can access it fine

[2024-05-19 00:48] irql: gs/fs base is kinda gross, but it is all in the manual

[2024-05-19 00:49] irql: all of the exceptions & things are in the manual too

[2024-05-19 00:49] irql: if you head over to "mov"

[2024-05-19 00:50] irql: i believe its all documented there

[2024-05-19 00:50] irql: "privileged instruction" usually refers to a #GP on there

[2024-05-19 00:57] abu: thanks

[2024-05-19 01:02] irql: [replying to irql: "in normal situations they are not privileged"]
oh sorry, yea depends on GDT if you’re doing funny business