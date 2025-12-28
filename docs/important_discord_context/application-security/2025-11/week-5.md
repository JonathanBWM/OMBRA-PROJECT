# November 2025 - Week 5
# Channel: #application-security
# Messages: 21

[2025-11-24 18:12] ml: What is the best way to link a DLL and an EXE? Or is it just to use an intermediate server?

[2025-11-24 18:24] BloodBerry: [replying to ml: "What is the best way to link a DLL and an EXE? Or ..."]
Link? U can link only .lib as I know

[2025-11-24 18:24] BloodBerry: DLL is dynamic stuff u can use LoadLibraryA(Path)

[2025-11-24 18:28] ImagineHaxing: nvm

[2025-11-24 18:30] ml: [replying to BloodBerry: "DLL is dynamic stuff u can use LoadLibraryA(Path)"]
No, I mean for them to communicate, like sending messages such as ‘the user closed the DLL,’ in a secure way.

[2025-11-24 18:31] BloodBerry: Try pipe or allocate static memory

[2025-11-24 18:31] BloodBerry: There are a lot of ways

[2025-11-24 18:31] BloodBerry: [replying to ml: "No, I mean for them to communicate, like sending m..."]
I mean FILE* pipe or smth like that

[2025-11-24 18:32] ml: [replying to BloodBerry: "I mean FILE* pipe or smth like that"]
Yeah, how do you secure them?

[2025-11-24 18:32] BloodBerry: Encryption…

[2025-11-24 18:32] ImagineHaxing: [replying to ml: "Yeah, how do you secure them?"]
maybe sending encrypted data

[2025-11-24 18:33] BloodBerry: If u asking bout the way of making it hidden then…. It’s hard enough

[2025-11-24 18:34] ml: [replying to ImagineHaxing: "maybe sending encrypted data"]
Yeah, I guess encryption is obvious, but don’t you think it’s just better to use a server as an intermediate?

[2025-11-24 18:35] BloodBerry: [replying to ml: "Yeah, I guess encryption is obvious, but don’t you..."]
Inter server = pipe-like method = encryption like

[2025-11-24 18:36] ImagineHaxing: [replying to ml: "Yeah, I guess encryption is obvious, but don’t you..."]
ur still decrypting the data locally in order to read it, so a reverser could see ur key no matter if its server or something locally and decrypt it

[2025-11-24 18:36] BloodBerry: If you will use intermediate server u still need enc

[2025-11-24 18:37] ml: ok

[2025-11-24 18:38] ml: then i would go for FILE* pipe

[2025-11-24 18:38] ml: seems a good ways

[2025-11-24 18:39] BloodBerry: [replying to ml: "seems a good ways"]
Yep but don’t forget about encryption stuff

[2025-11-24 18:42] ml: ofc