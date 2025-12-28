# October 2025 - Week 4
# Channel: #application-security
# Messages: 74

[2025-10-21 20:30] fexsped: Does anyone know what kernel objects I could use to spray the paged pool in windows low integrity context?

[2025-10-22 09:30] lukiuzzz: [replying to fexsped: "Does anyone know what kernel objects I could use t..."]
Event objects and semaphores are good for paged pool spraying from low integrity , you can create tons of them quickly to fill the pool layout.

[2025-10-22 16:16] fexsped: [replying to lukiuzzz: "Event objects and semaphores are good for paged po..."]
too small... anything that can be 0x90 with some user data control?

[2025-10-22 16:20] lukiuzzz: [replying to fexsped: "too small... anything that can be 0x90 with some u..."]
Use the WNF_STATE_NAME object

[2025-10-22 16:24] fexsped: [replying to lukiuzzz: "Use the WNF_STATE_NAME object"]
im in lpac

[2025-10-22 16:24] fexsped: sorry

[2025-10-22 16:24] fexsped: didnt know there was a difference lol

[2025-10-22 16:33] lukiuzzz: Ah lol

[2025-10-22 16:35] lukiuzzz: [replying to fexsped: "im in lpac"]
from LPAC your are extremely limited xD , best u can do is to keep hammering the Desktop heap with menu items so u  can get some size variation and a degree of pointer control that way.

[2025-10-22 16:38] diversenok: There are lots of kernel objects you can create from LPAC

[2025-10-22 16:40] lukiuzzz: Registry Key objects, BigPool chunks via named pipes, and some other certain types of Events.

[2025-10-22 16:44] 0xatul: [replying to lukiuzzz: "Registry Key objects, BigPool chunks via named pip..."]
since <@707524975552364604> mentions paged pool, you cant spray `NpAt` objects at all from LPAC

[2025-10-22 16:48] lukiuzzz: [replying to 0xatul: "since <@707524975552364604> mentions paged pool, y..."]
Yea on the newer ones doesn’t work u right

[2025-10-22 16:55] diversenok: [replying to fexsped: "im in lpac"]
This should not be an issue. Just checked, `NtCreateWnfStateName` works perfectly fine under LPAC

[2025-10-22 16:58] 0xatul: [replying to diversenok: "This should not be an issue. Just checked, `NtCrea..."]
But `NtWnfUpdateStateData` won't work

[2025-10-22 16:59] 0xatul: https://cdn.discordapp.com/attachments/1430440504474075207/1430594659452846262/image.png?ex=68fa5899&is=68f90719&hm=a02215dab32d08e624b8161f6e7656c1d597aa114884d1ebc37eb0879aeac3bf&

[2025-10-22 17:06] diversenok: Aren't you the one who provides the security descriptor?

[2025-10-22 17:07] diversenok: Granting `WNF_STATE_PUBLISH` access to `ALL RESTRICTED APPLICATION PACKAGES` should work

[2025-10-22 17:10] 0xatul: Oh well good shout let me try that

[2025-10-22 17:12] fexsped: wait so wnf does work under lpac?

[2025-10-22 17:12] diversenok: Of course

[2025-10-22 17:13] diversenok: Just checked, granted full access to Everyone + AC + LPAC on a temporary name

[2025-10-22 17:13] fexsped: cheers bro, its my first time doing windows kernel

[2025-10-22 19:46] fexsped: [replying to diversenok: "Granting `WNF_STATE_PUBLISH` access to `ALL RESTRI..."]
how do you do this?

[2025-10-22 19:47] diversenok: Well, `NtCreateWnfStateName` requires a security descriptor. You want to prepare one that allows your LPAC process to pass an access check

[2025-10-22 19:47] fexsped: ```c
InitializeSecurityDescriptor(&sdSpray, SECURITY_DESCRIPTOR_REVISION);
        NTSTATUS status = NtCreateWnfStateName(&state, WnfTempStateName, WnfScope, 0, 0, 0x1000, &sdSpray);
        if (!NT_SUCCESS(status)) {
            std::wcout << std::format(L"NtCreateWnfStateName error {:p}", (VOID*)status) << std::endl;
            _exit(1);
        }
```

[2025-10-22 19:48] fexsped: 
[Attachments: iLBRmLp.png]

[2025-10-22 19:48] fexsped: this is the code I get so im assuming im doing it wrong

[2025-10-22 19:50] diversenok: What does `WnfScope` stand for?

[2025-10-22 19:51] fexsped: ```c
WNF_STATE_NAME state;
        WNF_STATE_NAME_LIFETIME WnfTempStateName{ (WNF_STATE_NAME_LIFETIME)1 };
        WNF_DATA_SCOPE WnfScope{};
        SECURITY_DESCRIPTOR sdSpray;
        InitializeSecurityDescriptor(&sdSpray, SECURITY_DESCRIPTOR_REVISION);
        NTSTATUS status = NtCreateWnfStateName(&state, WnfTempStateName, WnfScope, 0, 0, 0x1000, &sdSpray);
        if (!NT_SUCCESS(status)) {
            std::wcout << std::format(L"NtCreateWnfStateName error {:p}", (VOID*)status) << std::endl;
            _exit(1);
        }
        char buf[0x60];
        memset(buf, 0x41, 0x60);
        status = NtUpdateWnfStateData(&state, buf, 0x60, 0, 0, 0, 0);
        if (!NT_SUCCESS(status)) {
            std::wcout << std::format(L"NtCreateWnfStateName error {:p}", (VOID*)status) << std::endl;
            _exit(1);
        }
```

[2025-10-22 19:51] fexsped: ```c
    typedef enum _WNF_DATA_SCOPE
    {
        WnfDataScopeSystem,
        WnfDataScopeSession,
        WnfDataScopeUser,
        WnfDataScopeProcess,
        WnfDataScopeMachine, // REDSTONE3
        WnfDataScopePhysicalMachine, // WIN11
    } WNF_DATA_SCOPE;
```

[2025-10-22 19:52] fexsped: its this enum thing

[2025-10-22 19:52] diversenok: Your code is trying to create a permanent name

[2025-10-22 19:52] diversenok: `(WNF_STATE_NAME_LIFETIME)1` is permanent; you need temporary

[2025-10-22 19:53] diversenok: ```c
typedef enum _WNF_STATE_NAME_LIFETIME
{
    WnfWellKnownStateName = 0,
    WnfPermanentStateName = 1,
    WnfPersistentStateName = 2,
    WnfTemporaryStateName = 3
} WNF_STATE_NAME_LIFETIME;
```

[2025-10-22 19:54] fexsped: thanks

[2025-10-22 19:54] diversenok: I tried `WnfTemporaryStateName` with `WnfDataScopeUser` and it worked

[2025-10-22 19:54] fexsped: It does work

[2025-10-22 19:54] fexsped: why does permanent not work?

[2025-10-22 19:55] diversenok: It requires `SE_CREATE_PERMANENT_PRIVILEGE`

[2025-10-22 19:55] fexsped: oh damn

[2025-10-22 19:56] fexsped: but now the `NtUpdateWnfStateDAta` returns STATUS_DENIED

[2025-10-22 19:58] fexsped: [replying to diversenok: "Granting `WNF_STATE_PUBLISH` access to `ALL RESTRI..."]
Do I need to change how I init the Security Descriptor?

[2025-10-22 20:03] diversenok: If you want to interact with the name after creation (like calling `NtUpdateWnfStateData`), the minimal one will need to have a DACL with two ACEs: one for Everyone (`S-1-1-0`) and one for LPAC (`S-1-15-2-2`)

[2025-10-22 20:04] diversenok: So yes, preparing two SIDs, allocating a DACL with two ACEs, and adding it to a security descriptor

[2025-10-22 20:12] fexsped: [replying to diversenok: "So yes, preparing two SIDs, allocating a DACL with..."]
is `WNF_STATE_PUBLISH` == 2?

[2025-10-22 20:12] diversenok: Yes. You can also just use `GENERIC_ALL`

[2025-10-22 20:14] diversenok: ```c
#define WNF_STATE_SUBSCRIBE 0x0001
#define WNF_STATE_PUBLISH 0x0002
#define WNF_STATE_CROSS_SCOPE_ACCESS 0x0010
```

[2025-10-22 20:24] fexsped: [replying to diversenok: "Yes. You can also just use `GENERIC_ALL`"]
how does one become so well versed in windows internals

[2025-10-22 20:29] fexsped: What am I doing wrong here? I still get access denied when trying to write
```cpp
VOID BuildValidSd(PSECURITY_DESCRIPTOR outSd, PACL* outAcl) {
    memset(outSd, 0, sizeof(SECURITY_DESCRIPTOR));

    // Convert SIDs
    PSID SidEveryone, SidLPAC;
    ConvertStringSidToSidA("S-1-1-0", &SidEveryone);
    ConvertStringSidToSidA("S-1-15-2-2", &SidLPAC);

    EXPLICIT_ACCESS ea[2] = {};
    ea[0].grfAccessPermissions = GENERIC_ALL;
    ea[0].grfAccessMode = GRANT_ACCESS;
    ea[0].grfInheritance = NO_INHERITANCE;
    ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea[0].Trustee.ptstrName = (LPWCH)SidEveryone;

    ea[1].grfAccessPermissions = GENERIC_ALL;
    ea[1].grfAccessMode = GRANT_ACCESS;
    ea[1].grfInheritance = NO_INHERITANCE;
    ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[1].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea[1].Trustee.ptstrName = (LPWCH)SidEveryone;

    PACL pAcl = NULL;
    SetEntriesInAcl(2, ea, NULL, &pAcl);
    InitializeSecurityDescriptor(outSd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(outSd, TRUE, pAcl, FALSE);
    *outAcl = pAcl;

    LocalFree(SidEveryone);
    LocalFree(SidLPAC);
}
```

[2025-10-22 20:36] lukiuzzz: [replying to fexsped: "What am I doing wrong here? I still get access den..."]
You're granting GENERIC_ALL to Everyone twice instead of giving LPAC its own ACE.

[2025-10-22 20:36] fexsped: oh shit

[2025-10-22 20:38] fexsped: It worked

[2025-10-22 20:38] fexsped: tysm Im blind

[2025-10-22 20:38] lukiuzzz: <:yara_lover:1148745271577157673>

[2025-10-22 20:38] fexsped: bro?

[2025-10-22 20:38] fexsped: ok

[2025-10-22 20:39] lukiuzzz: <:mmmm:904523247205351454>

[2025-10-22 20:42] fexsped: Becoming a windows expert sounds cool

[2025-10-22 20:48] fexsped: the knowledge of a thousand suns is in this discord I can feel it

[2025-10-22 20:59] diversenok: [replying to fexsped: "how does one become so well versed in windows inte..."]
What worked for me was reading books and blog posts, playing with tools like System Informer, and writing my own things in native API ¯\_(ツ)_/¯

[2025-10-22 21:01] fexsped: [replying to diversenok: "What worked for me was reading books and blog post..."]
how long have you been doing this for?

[2025-10-22 21:05] diversenok: Hmm, maybe 7 years

[2025-10-22 21:11] fexsped: nice

[2025-10-23 06:51] fexsped: is there a tried and tested way  of getting kaslr leaks from LPAC? Or do you need to leverage the bug youre exploiting for a leak as well?

[2025-10-23 07:10] lukiuzzz: [replying to fexsped: "is there a tried and tested way  of getting kaslr ..."]
You'll need to use your initial write primitive to both disclose kernel addresses and then execute your exploit

[2025-10-23 07:11] fexsped: [replying to lukiuzzz: "You'll need to use your initial write primitive to..."]
https://cdn.discordapp.com/attachments/1257252173826490378/1430815896057942036/togif.gif

[2025-10-26 15:09] fexsped: how to search for a string in the entire memory in windbg? (like pwndbg's search)

[2025-10-26 15:24] the horse: https://learn.microsoft.com/en-us/windows-hardware/drivers/debuggercmds/s--search-memory- <@707524975552364604>
[Embed: s (Search Memory) - Windows drivers]
The s command searches through memory to find a specific byte pattern.

[2025-10-26 15:36] fexsped: [replying to the horse: "https://learn.microsoft.com/en-us/windows-hardware..."]
its a bit confusing the way they word it

[2025-10-26 15:57] fexsped: man windbg sucks

[2025-10-26 16:55] daax: [replying to fexsped: "man windbg sucks"]
This is a skill issue. WinDbg is a great tool

[2025-10-26 16:55] djcivi: at first gdb sucks too but it grows on you

[2025-10-26 17:50] fexsped: [replying to djcivi: "at first gdb sucks too but it grows on you"]
it always sucked I just use pwndbg