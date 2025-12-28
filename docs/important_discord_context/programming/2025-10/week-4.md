# October 2025 - Week 4
# Channel: #programming
# Messages: 138

[2025-10-20 07:37] Timmy: [replying to BWA RBX: "Is it appropriate to have two camera system in a g..."]
many games have multiple camera systems.

[2025-10-20 14:48] Leonard K.: [replying to BWA RBX: "Is it appropriate to have two camera system in a g..."]
Implementing UI in 3D space and dealing with projection math is a bad time.
There are some highly stylistic games with fully 3D UI, but it's incredibly rare.
In any case, you'll need to make sure that your extra camera doesn't do extra work until it needs to.

[2025-10-20 14:56] Timmy: also ui in 3d is fun for about 2 minutes when you first look at it, but after that it's just a UX nightmare

[2025-10-20 14:56] Timmy: (looking at you, borderlands)

[2025-10-20 15:11] Brit: bad implementation, example, diegetic in world ui can be very fun when done properly IMO

[2025-10-20 15:45] Timmy: Would you happen to have any examples of such UI? To me it's quite hard to imagine it working or what qualities even make it work in the first place

[2025-10-20 15:55] Brit: a very niche example would be Steel Battalion

[2025-10-20 16:29] Leonard K.: Even with 3D UI, I feel you would still want to have another camera with separate config and depth mask. 🤔

[2025-10-20 16:31] Leonard K.: I would guess going along this path you'll still end up with a distinct pass, which essentially would be the same as having a separate camera.

[2025-10-21 19:46] twopic: 
[Attachments: Screenshot_20251021-122508.png]

[2025-10-21 19:46] twopic: He should just become a business major

[2025-10-21 19:47] twopic: <@303272276441169921> let's make that happen

[2025-10-21 19:49] Brit: you massively overestimate both the amount I care about this person and also my ability to get them to become a business major

[2025-10-21 19:49] luci4: 😂

[2025-10-21 19:49] luci4: The exact response I expect (and appreciate) from you

[2025-10-21 19:50] twopic: [replying to Brit: "you massively overestimate both the amount I care ..."]
It's still funny

[2025-10-21 19:51] twopic: 
[Attachments: Screenshot_20250926-173501.png, Screenshot_20250926-173334.png]

[2025-10-21 19:54] pinefin: i dont even get what you're making fun of. he seems like he plans out his work in a respectable fashion

[2025-10-21 19:54] twopic: [replying to pinefin: "i dont even get what you're making fun of. he seem..."]
He tries to ruin people's lives

[2025-10-21 19:58] twopic: He was writing to the stack pointer

[2025-10-21 19:59] pinefin: seems like a lot of effort to bring it into a whole different server instead of voicing your opinion straight to him

[2025-10-21 19:59] pinefin: man up

[2025-10-21 20:08] twopic: Yeah because it's justice

[2025-10-21 20:10] pinefin: justice is shit talking someone in another discord?

[2025-10-21 20:16] Addison: yeah bro this is pretty weak

[2025-10-21 20:18] iPower: the next time I see this here I'm banning whoever does that

[2025-10-21 20:18] pinefin: [replying to twopic: "Yeah because it's justice"]
looking at your past messages thats all you do, just come in here complaining about this guy. given i agree, some of the electrical engineering topics he's on he probably doesnt know jack shit. but why are you coming in here and making this our business when none of us interact with him

[2025-10-21 20:18] iPower: like Addison and pinefin said, grow up and voice your opinions straight to him

[2025-10-21 20:18] iPower: this isn't your casual shitstain

[2025-10-21 20:19] iPower: from what I remember this channel is called programming, not "trying to be funny by roasting someone"

[2025-10-21 20:20] iPower: we all hate larpers and such but coming here just to be a dick isn't the right way of handling that.

[2025-10-21 20:28] pygrum: everything I’ve seen about shmarvdogg has been against my will

[2025-10-21 20:42] luci4: shmarvdogg lives rent free in his head

[2025-10-21 20:42] luci4: bills included

[2025-10-21 20:49] pinefin: its justice.

[2025-10-21 20:49] pinefin: https://tenor.com/view/batman-caped-crusader-caped-crusader-batman-batman-pose-gif-12537728247115118999

[2025-10-21 20:49] pinefin: -# ok im done now

[2025-10-22 10:59] Gestalt: obsessed with shmarvdogg

[2025-10-22 18:19] Deleted User: Hello everyone I’m an experienced hacker with full knowledge about hacking I’m here to meet new friends and help feel free to contact me regarding any help via hacking services

[2025-10-22 18:21] plpg: damn. I only have 2/3 knowledge about hacking.

[2025-10-22 18:29] Leonard K.: Full stack hacking

[2025-10-22 18:38] Brit: I only have 1/7 knowledge of hacking. it might be over for me

[2025-10-22 19:15] iris8721: https://cdn.discordapp.com/attachments/1367325577358475264/1400156521165488289/togif-1.gif

[2025-10-22 19:18] ImagineHaxing: [replying to Deleted User: "Hello everyone I’m an experienced hacker with full..."]
Can you please allegedly help me hack the world I'll allegedly pay you 2 Argentinian pesos 🤑

[2025-10-22 23:43] Brandon: <@456226577798135808> are you the same person as <@1282203232147603508>? If so your statement about having "full knowledge" is incorrect, you need to learn windows kernel-mode API hooking first.

[2025-10-23 00:29] the horse: Do compilers actually require explicit inline attributes on functions, even if they're extremely small (think vector class operators or functions), or is this some MSVC gimmick?

[2025-10-23 00:29] the horse: I'd assume this would actually get inlined in Release with full optimization, guess it needs to be stated explicitly..
[Attachments: image.png, image.png]

[2025-10-23 00:29] the horse: MY CYCLES!!!

[2025-10-23 00:30] the horse: I know that clang/gcc should support aggressive inlining optimization, but would this be even considered aggressive inlining?

[2025-10-23 00:31] the horse: I haven't messed with the 'inline function expansion' optimization options or anything either

[2025-10-23 00:49] selfprxvoked: [replying to the horse: "I'd assume this would actually get inlined in Rele..."]
Why would this function get inlined? It is massive.

[2025-10-23 00:50] the horse: I guess it's some compromise to not explode the binsize; it would make sense to me to inline because it's purely SIMD if inlined

[2025-10-23 00:50] selfprxvoked: If this code isn't hot, it is way better getting 1 instruction cache miss

[2025-10-23 00:50] the horse: it's hot

[2025-10-23 00:51] selfprxvoked: then why aren't you force inlining? lol

[2025-10-23 00:51] the horse: I'd like to understand the trade-offs here

[2025-10-23 08:59] tweakeroo: [replying to the horse: "I haven't messed with the 'inline function expansi..."]
The inline keyword on a function has nothing to do with inlining optimizations, on most compilers using some intrinsic keyword usually suggests to inline a function but is the compiler optimizer the one really deciding

[2025-10-23 09:04] tweakeroo: Msvc documentation:
```The compiler treats the inline expansion options and keywords as suggestions. There's no guarantee that functions will be inlined. You can't force the compiler to inline a particular function, even with the __forceinline keyword```

[2025-10-23 09:05] tweakeroo: __forceinline on msvc will prevent  inlining cost/benefit analysis and just inline it

[2025-10-23 09:07] tweakeroo: ```In some cases, the compiler will not inline a particular function for mechanical reasons. For example, the compiler will not inline:

A function if it would result in mixing both SEH and C++ EH.

Some functions with copy constructed objects passed by value when -GX/EHs/EHa is on.

Functions returning an unwindable object by value when -GX/EHs/EHa is on.

Functions with inline assembly when compiling without -Og/Ox/O1/O2.

Functions with a variable argument list.

A function with a try (C++ exception handling) statement.```

[2025-10-23 09:07] tweakeroo: Except for this

[2025-10-23 09:08] tweakeroo: Also clang optimizer is overall better than msvc this days

[2025-10-23 09:15] Brit: [replying to the horse: "Do compilers actually require explicit inline attr..."]
Bench it with forceinline, although this is such a classic msvc moment.

[2025-10-23 09:57] plpg: If you want a portable force inline then just use a macro

[2025-10-23 09:58] plpg: You can enclose code in { } to limit the scope of variables

[2025-10-23 09:59] plpg: Ex. `#define FROB(x) { int k = frobnicator_init(x); frobnicate(k); }`

[2025-10-23 11:12] djcivi: ```c
do { \
  func1(x); \
  func2(x); \
} while (0)
```
trick is also nice

[2025-10-23 11:50] selfprxvoked: [replying to plpg: "If you want a portable force inline then just use ..."]
Why tho?
Just use `__attribute__((always_inline)) inline` \ `__forceinline`, this isn't C from 80's to need hacky macro to have inlined functions

[2025-10-23 12:19] tweakeroo: [replying to djcivi: "```c
do { \
  func1(x); \
  func2(x); \
} while (0..."]
Wait does this actually mess with optimzer?

[2025-10-23 12:19] brymko: no

[2025-10-23 12:19] djcivi: no

[2025-10-23 12:19] brymko: its a macro]

[2025-10-23 12:20] tweakeroo: ?

[2025-10-23 12:20] tweakeroo: ```c
do { 
  func1(x); 
  func2(x); 
} while (0)
```

[2025-10-23 12:21] djcivi: if __attribute__((always_inline)) inline \ __forceinline didnt work  you could use a hacky macro like that and put the function body in that to get it 'inlined'

[2025-10-23 12:21] tweakeroo: It should generate the same llvm

[2025-10-23 12:22] plpg: [replying to selfprxvoked: "Why tho?
Just use `__attribute__((always_inline)) ..."]
Thats also valid

[2025-10-23 12:22] plpg: I didnt know about that one

[2025-10-23 12:23] plpg: Sometimes you have c from the 80s tho :p

[2025-10-23 12:24] plpg: I never bothered to read the list of GCC attributes, i should, some are useful

[2025-10-24 00:42] selfprxvoked: [replying to plpg: "I never bothered to read the list of GCC attribute..."]
You don't need, all you need is coding experience on professional or serious projects and you'll learn a lot of compiler gimmicks

[2025-10-24 00:45] selfprxvoked: [replying to djcivi: "if __attribute__((always_inline)) inline \ __force..."]
It is pretty hard for GCC/LLVM to not inline functions marked as always_inline as far as I know, only really specific circunstances, but macros are not a direct substitution either

[2025-10-24 00:47] selfprxvoked: You can end up "obfuscating" your own source code full of macros that can become totally unreadable in the near future, so it is a big no

[2025-10-24 01:03] Brit: [replying to selfprxvoked: "It is pretty hard for GCC/LLVM to not inline funct..."]
Only ever seen this with very odd EH usage.

[2025-10-24 07:53] plpg: [replying to selfprxvoked: "You don't need, all you need is coding experience ..."]
I didnt say to learn them by heart, just to get an idea whats possible to do. I've been writing C for years and I didnt remember there is a force inline attribute (though with gcc, its no surprise there is)

[2025-10-24 07:59] plpg: [replying to selfprxvoked: "You can end up "obfuscating" your own source code ..."]
Thats true for any language feature thats used in a stupid way, i've had code obfuscated beyond maintainability by overusing C++ initializer lists

[2025-10-24 09:31] fexsped: Im trying to create a basic ALPC Server-Client conversation using the NT API and while trying to connect to my server I get error message 0xC0000023 (STATUS_BUFFER_TOO_SMALL):
Server code snippet that runs on a bg thread:
```cpp

DWORD WINAPI AlpcLoop::Listen(LPVOID lpParam) {
    AlpcLoop::ListenThreadArgs* args = static_cast<AlpcLoop::ListenThreadArgs*>(lpParam);

    PALPC_MESSAGE_ATTRIBUTES pMsgAttrSend = AlpcLoop::SetupSampleMsgAttr(
        args->hPort,
        args->hSection,
        ALPC_MESSAGE_SECURITY_ATTRIBUTE
        | ALPC_MESSAGE_VIEW_ATTRIBUTE,
        args->ViewAttr
    );
    
    // Wait for initial connection
    std::wcout << L"Waiting for incoming ALPC connections on background thread...\n";
    ALPC_MESSAGE msgReceived{};
    SIZE_T ReceivedSize;
    while (1) {
                // SERVER RETURNS ERROR 0xC0000023
        NTSTATUS status = NtAlpcSendWaitReceivePort(
            args->hPort,
            ALPC_MSGFLG_NONE,
            NULL,
            NULL,
            (PPORT_MESSAGE)&msgReceived,
            &ReceivedSize,
            pMsgAttrSend,
            0
        );
        if (!NT_SUCCESS(status)) {
            std::wcout << std::format(L"NtAlpcSendWaitReceivePort failed with error {:#x}\n", static_cast<std::uint32_t>(status));
            ExitProcess(0);
        }

        wprintf(L"[+] Received data!\nType: %s\nConnectionMessage Length: %d\n",
            AlpcLoop::MsgTypeToName(msgReceived.PortHeader.u2.s2.Type),
            msgReceived.PortHeader.u1.s1.DataLength
        );
...
}
```
Client code snippet that hangs on connecting:

[2025-10-24 09:31] fexsped: ```cpp
NTSTATUS AlpcLoop::AlpcClient::Connect(LPCWSTR PortName) {
    DWORD dwPID = GetCurrentProcessId();
    DWORD dwTID = GetCurrentThreadId();
    wprintf(L"[*] Starting ALPC Client. PID: %d | TID: %d\n", dwPID, dwTID);

    UNICODE_STRING usPortName{};
    RtlInitUnicodeString(&usPortName, PortName);
    HANDLE hSrvCommPort;

    ALPC_MESSAGE pmSend{};
    AddRequestMessage(&pmSend, "AAAAAAAABBBBBBBB", 16);
    SIZE_T SendSize = sizeof(pmSend);

    PALPC_MESSAGE_ATTRIBUTES pMsgAttrReceived = AllocMsgAttribute(ALPC_MESSAGE_ATTRIBUTE_ALL);
    
    NTSTATUS lSuccess = NtAlpcConnectPort(
        &hSrvCommPort,
        &usPortName,
        NULL,
        NULL,
        ALPC_SYNC_CONNECTION, // Block until reply
        NULL,
        (PPORT_MESSAGE)&pmSend,
        &SendSize,
        NULL,
        pMsgAttrReceived,
        0 // optional timeout
    );
    if (!NT_SUCCESS(lSuccess)) {
        wprintf(L"NtAlpcConnectPort Error: 0x%X\n", lSuccess);
        std::wcout << std::endl;
        return lSuccess;
    }
    else wprintf(L"[+] Connected to %s successfully", PortName);
    std::wcout << std::endl;

    return STATUS_SUCCESS;
}
```

[2025-10-24 09:34] fexsped: The thing that I dont get is that my server never receives a message and just denies it, I probably have some param wrong...

[2025-10-24 10:47] fexsped: Fixed by setting this when calling `NtAlpcSendWaitReceivePort`
`    SIZE_T ReceivedSize = sizeof(ALPC_MESSAGE);`

[2025-10-24 10:47] fexsped: really unintuitive considering that we pass the size as a pointer

[2025-10-24 11:43] diversenok: It is the only possible way to pass an in-out parameter

[2025-10-24 11:44] diversenok: On input, it specifies the size of the buffer you allocated; on output, it receives the number of bytes used

[2025-10-24 11:52] diversenok: Also this seems highly suspisous:
```c
ALPC_MESSAGE pmSend{};
AddRequestMessage(&pmSend, "AAAAAAAABBBBBBBB", 16);
```

[2025-10-24 11:53] diversenok: If you want a bigger message, you need to allocate it first

[2025-10-24 11:54] diversenok: Or maybe your definition of `ALPC_MESSAGE` has some extra space, then it also works

[2025-10-24 20:10] fexsped: [replying to diversenok: "Or maybe your definition of `ALPC_MESSAGE` has som..."]
it does actually,
```cpp
    typedef struct _ALPC_MESSAGE {
        PORT_MESSAGE PortHeader;
        BYTE PortMessage[128];      // using a byte array of size 128 to store my actual message
    } ALPC_MESSAGE, * PALPC_MESSAGE;
```

[2025-10-24 20:10] fexsped: but Its pointless and just a test, as I plan to use the ALPC views

[2025-10-24 20:11] fexsped: as thats where my previously discussed bug lies

[2025-10-25 01:21] Matti: [replying to fexsped: "Im trying to create a basic ALPC Server-Client con..."]
1. on the server, you should use `ALPC_MESSAGE_CONTEXT_ATTRIBUTE` at the very least, as this will allow storing the client's port handle in the `PortContext`. this means the port handle you are calling `NtAlpcSendWaitReceive` on can (and should) be dependent on whether you are listening for new messages or replying to one you just received. replying is **required** if `LPC_CONTINUATION_REQUIRED` is set on the received message. you can definitely expect this to be set on connection requests, as well as most messages containing attributes, especially views

2. similarly, when replying to such a message, you should be passing `ALPC_MSGFLG_RELEASE_MESSAGE`, not 0, for flags

3. because of the reasons above, you can realistically only get away with using the same buffer for the input and output on the client. don't do this on the server, use two separate message pointers instead (you can still have one buffer, it just won't be used for two things in a single call). this also means you can omit the receive size (and send size for that matter), which is the parameter that was causing your issue in the first place

4. why are you passing **send** message attributes in **receive** loop (i.e. on the server)? leave this parameter empty, especially since you haven't even accepted a client connection yet

[2025-10-25 01:22] Matti: example setup for (static) receive message and attribute buffers:
```cpp
MYAPI_MESSAGE MessageBuffer;
PMYAPI_MESSAGE ReceiveMessage = &MessageBuffer, ReplyMessage = nullptr;
HANDLE ReplyPort = ApiPort;

UCHAR ReceiveMessageAttributesBuffer[sizeof(ALPC_MESSAGE_ATTRIBUTES) +
                                    sizeof(ALPC_CONTEXT_ATTR) +
                                    sizeof(ALPC_DATA_VIEW_ATTR)];
const PALPC_MESSAGE_ATTRIBUTES ReceiveMessageAttributes = (PALPC_MESSAGE_ATTRIBUTES)ReceiveMessageAttributesBuffer;
```

[2025-10-25 01:23] Matti: minimal loop for a server that accepts client views:
```cpp
while (true)
{
    const bool Replying = ReplyMessage != nullptr;
    const ULONG Flags = Replying ? ALPC_MSGFLG_RELEASE_MESSAGE : 0;

    // Allow context and data view attributes in messages
    AlpcInitializeMessageAttribute(ALPC_MESSAGE_CONTEXT_ATTRIBUTE | ALPC_MESSAGE_VIEW_ATTRIBUTE,
                                        ReceiveMessageAttributes,
                                        sizeof(ReceiveMessageAttributesBuffer),
                                        nullptr);

    // Send a reply if needed, and block until the next message is received.
    Status = NtAlpcSendWaitReceivePort(ReplyPort,
                                        Flags,
                                        &ReplyMessage->Header,
                                        nullptr,
                                        &ReceiveMessage->Header,
                                        nullptr,
                                        ReceiveMessageAttributes,
                                        nullptr);

    // Listen on the API port on the next call.
    ReplyPort = ApiPort;
    ReplyMessage = nullptr;
    ReceiveMessage = &MessageBuffer;

    if (IsDisconnectStatus(Status) && (!Replying || ApiPort == nullptr))
        break; // API port closed

    if (Replying)
        continue; // Reply sent - go back to listening

    if (!NT_SUCCESS(Status))
        continue; // Error handling omitted for brevity

    // Retrieve the context attribute and get our client identifier from it
    const PALPC_CONTEXT_ATTR ContextAttributes = (PALPC_CONTEXT_ATTR)AlpcGetMessageAttribute(ReceiveMessageAttributes, ALPC_MESSAGE_CONTEXT_ATTRIBUTE);
    if (ContextAttributes == nullptr)
        continue;
    const PMYAPI_CLIENT ApiClient = (PMYAPI_CLIENT)ContextAttributes->PortContext;

    // The high byte of the message type field is only used for flags like LPC_KERNELMODE_MESSAGE.
    const LPC_TYPE MessageType = (LPC_TYPE)((SHORT)ReceiveMessage->Header.u2.s2.Type & 0xFF);
    NT_ASSERT(ApiClient != nullptr || (MessageType == LPC_CONNECTION_REQUEST || (MessageType == LPC_PORT_CLOSED && ReplyPort == ApiPort)));

    switch (MessageType)
    {
        case LPC_CONNECTION_REQUEST:
        {
            // client connection request - handle with NtAlpcAcceptConnectPort
            break;
        }
        case LPC_PORT_CLOSED: // client disconnected
        case LPC_CLIENT_DIED: // client died
        {
            DeleteClient(ApiClient);
            break;
        }
        case LPC_REQUEST:
        case LPC_DATAGRAM:
        {
            ReceiveMessage->Status = ... // do something with the message here
            break;
        }
        default:
            break; // unexpected message type
    }

    // Check if the message we received requires a reply with ALPC_MSGFLG_RELEASE_MESSAGE.
    if (((SHORT)ReceiveMessage->Header.u2.s2.Type & LPC_CONTINUATION_REQUIRED) != 0)
    {
        // Check if there was a view attached to a message that shouldn't have one, and if so, don't leak the handle
        if ((ReceiveMessageAttributes->ValidAttributes & ALPC_MESSAGE_VIEW_ATTRIBUTE) &&
            (MessageType != LPC_REQUEST || (MessageType == LPC_REQUEST &&
            ReceiveMessage->ApiNumber != MyApiShareSectionView)))
        {
            const PALPC_DATA_VIEW_ATTR DataView = (PALPC_DATA_VIEW_ATTR)AlpcGetMessageAttribute(ReceiveMessageAttributes, ALPC_MESSAGE_VIEW_ATTRIBUTE);
            if (DataView != nullptr && DataView->ViewBase != nullptr)
            {
                NtAlpcDeleteSectionView(ApiPort, 0, DataView->ViewBase);
                DataView->ViewBase = nullptr;
            }
        }

        // Make our next call to NtAlpcSendWaitReceivePort a reply to the message we just received.
        ReplyPort = ApiClient != nullptr ? ApiClient->ClientPort : ApiPort;
        ReplyMessage = ReceiveMessage;
        ReceiveMessage = nullptr;
    }
}
```

[2025-10-25 01:26] Matti: I've left out some types in the code above, but `MYAPI_MESSAGE` is really nothing more than a wrapper for a `PORT_MESSAGE` (first field) plus some additional required fields and a union holding a bunch of structs, with the one to use depending on `ApiMessage->ApiNumber`

[2025-10-25 07:02] fexsped: tysm bro

[2025-10-25 08:41] fexsped: [replying to Matti: "minimal loop for a server that accepts client view..."]
How does `MYAPI_CLIENT` look like?

[2025-10-25 11:41] Obvious: hello, i want to ask something regarding hooks.
[Attachments: B80BC158-7723-43A3-88C7-9E19A56074CE.png]

[2025-10-25 11:41] Obvious: when we call VirtualProtect, we pass in the address of vmtptr

[2025-10-25 11:42] Obvious: Shouldn't we be passing *vmtptr?

[2025-10-25 11:42] Obvious: Because we're writing to the memory region where function pointers are located at

[2025-10-25 11:42] Obvious: Which would be vmt itself, not the pointer pointing to it.

[2025-10-25 11:43] Obvious: Also, it's interesting to me that the compiler doesn't regard the class pointer as a pointer to pointer to pointer. You need to explicitly tell it

[2025-10-25 11:45] archie_uwu: the first parameter to VirtualProtect is the address of the memory whose protections are to be changed
you're passing the value of vmtptr (ie. the stored address) correctly, passing *vmtptr would pass the value at the memory address (the first virtual function)

you're not passing in the address of vmtptr in the screenshot

[2025-10-25 11:47] Obvious: So I am passing the address that vmtptr points to, not the address vmtptr resides at?

[2025-10-25 11:54] archie_uwu: yes, you are passing the contents of ``vmtptr``, not its address. that would be ``&vmtptr``, which would be somewhere on your thread's stack

[2025-10-25 11:54] Obvious: and when i want to write to that address i simply dereference it

[2025-10-25 11:55] Obvious: vmtptr -> 0xFFF
*vmtptr (writes to 0XFFF)
**vmtptr (writes to where the pointer at 0XFFF points to)

[2025-10-25 11:56] Obvious: also means **vmtptr cannot be a pointer if my interpretation is correct.

[2025-10-25 11:56] Obvious: so **vmtptr cannot be passed as a pointer

[2025-10-25 11:56] Obvious: whereas vmtptr and *vmtptr can (due to the first initialization where vmtptr is stated to be pointer to pointer)

[2025-10-25 14:09] Brit: [replying to Obvious: "when we call VirtualProtect, we pass in the addres..."]
You are passing vmtptr not &vmtptr

[2025-10-25 14:15] Obvious: yh i got it now, thanks. my brain wasnt working properly

[2025-10-25 14:20] Gestalt: [replying to Obvious: "yh i got it now, thanks. my brain wasnt working pr..."]
dont worry it be like that at the start

[2025-10-25 17:10] Matti: [replying to fexsped: "How does `MYAPI_CLIENT` look like?"]
anything you want
```cpp
typedef struct _MYAPI_CLIENT
{
    LPC_CLIENT_ID ClientId;
    HANDLE ClientPort;
    ALPC_DATA_VIEW_ATTR DataView;
} MYAPI_CLIENT, *PMYAPI_CLIENT;
```
the first two are pretty much mandatory to make the context attribute useful though

[2025-10-26 14:00] fexsped: [replying to Matti: "anything you want
```cpp
typedef struct _MYAPI_CLI..."]
Thanks, btw is there a way of forcing a connection to use an ALPC view with a message smaller than 64kb?

[2025-10-26 15:29] 0xatul: wait NO, I am wrong

[2025-10-26 19:36] the horse: I think Linux had some benchmarks regarding this posted some time ago, but primarily you reduce the number of TLB misses since you do a lot less translations, you can also map more memory this way since the entries won't get exhausted as quickly

[2025-10-26 19:52] the horse: A simplest example would be a large driver. If it's close to 2mb (code section) for example, a single large page enty would satisfy it

[2025-10-26 19:53] the horse: If the OS doesn't inherently care about permissions of the individual sections, it might allocate the driver in a single entry if it doesn't pass that amount (hvix/hvax is allocated in this way afaik -- mshv)

[2025-10-26 19:53] the horse: In contrast, if large pages weren't used, you'd use up 2mb divided by 4kb amount of entries

[2025-10-26 19:53] the horse: therefore you exhaust entries in a PDPT faster

[2025-10-26 19:55] the horse: wait yeah I realize that now

[2025-10-26 19:55] the horse: you're right on that

[2025-10-26 19:57] the horse: keep me posted, i'm also interested in what difference it makes

[2025-10-26 19:57] the horse: how do you plan to benchmark it, though?

[2025-10-26 19:57] the horse: and are you planning to use a windows or a linux guest?

[2025-10-26 19:58] the horse: I can only think of benchmarking tlb misses

[2025-10-26 20:22] the horse: DTLB_LOAD_MISSES looks interesting for this

[2025-10-26 22:59] UJ: Which ones are you looking at? HyperDbg splits the 2mb into 4kb if there are multiple memory types described by the mtrr https://github.com/HyperDbg/HyperDbg/blob/22096da60ed954172b9ebaf020754755231fdfd8/hyperdbg/hyperhv/code/vmm/ept/Ept.c#L598

Basically use 4kb EPT for the fixed MTRR ranges, for variable, 2mb if the memory type described by the mtrr is the same else split into 4kb, use 1gb UC EPT for the ranges not described by the mtrr (or from 512gb onwards) until max addressable memory (mmio ranges), some mmio devices require WC and not UC however. 

---

im also interested if using 2mb vs 4kb affects performance. We can set the memory type of the memory storing the EPT paging structures themself in the EPTP which should be WB so they should get cached but its 1 less translation layer during the page walk so you never know.

So you want to measure the performance difference in doing the GVA -> HPA pagewalk itself to see if it matters?

Let me know how this goes/i can run it on my machine as well.