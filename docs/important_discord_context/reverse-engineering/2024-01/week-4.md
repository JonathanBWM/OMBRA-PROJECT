# January 2024 - Week 4
# Channel: #reverse-engineering
# Messages: 48

[2024-01-25 16:16] qwerty1423: [replying to Ignotus: "I’m reversing a java program and I need a debugger..."]
go to IntelliJ's settings, "Build, Execution, Deployment / Debugger / Java / add new decompiler", put this config in the opened dialog :

Name: CFR
Path: "exampledir\cfr-0.149.jar"
VM Options: -javaagent:"<path-to-cfr>\cfr.jar"

[2024-01-25 16:16] Ignotus: oh nice

[2024-01-25 16:17] Ignotus: any way to set method breakpoints to break on method exit by default? rn I have to toggle it every time for new breakpoints

[2024-01-25 16:21] qwerty1423: [replying to Ignotus: "any way to set method breakpoints to break on meth..."]
not possible in intelliJ as far as i know, I'll DM you if i found anything

[2024-01-26 23:24] Depression™: Currently reverse engineering an AES key from an java application. Is there any reasoning behind having every byte be bitwise operated & by ```4294967295``` I get that it's just 32 1's and so will give the original value back. Does this have something to do with removing conflict on java seeing it as a 2 complements number?

[2024-01-26 23:26] Depression™: For example every byte goes through this step: ```public aes_byte_key(long j) {
        this.input = j & 4294967295L;
    }
```

[2024-01-26 23:27] Depression™: Any insight would greatly appreciated 🙂

[2024-01-26 23:34] mrexodia: [replying to Depression™: "Any insight would greatly appreciated 🙂"]
because a `long` is 64-bits and `4294967295` is `0xFFFFFFFF` (the maximum value of an unsigned 32-bit number)

[2024-01-26 23:35] Depression™: So any value passed to it gets trimmed to a 32bit number basically

[2024-01-26 23:39] mrexodia: indeed

[2024-01-26 23:40] mrexodia: most likely this is because there is no unsigned math in java

[2024-01-26 23:42] Depression™: Yeah they wrote their own implementation so it's a bit of a mess to de-obfuscate. Appreciate the the input, thnx!

[2024-01-27 01:53] mibho: "x64dbg will save all registers at the start of trace, and every 512 instructions"

[2024-01-27 01:54] mibho: i believed this and kinda got fucked hard <:mmmm:904523247205351454>

[2024-01-27 10:28] dullard: Any COM/RPC wizards about, I'm trying to take some interface definitions from RPCView and use them in a project but i'm getting the following errors on two out params for one of the procs 

```c
MIDL2121[out] only parameter must not derive from a top - level[unique] or[ptr] pointer / array : [Parameter 'arg_9' of Procedure 'Proc1' (Interface 'DefaultIfName')]
``` I've managed to get the compiler to shut up but I'm not entirely sure why it happens 🤔 ```c
    error_status_t Proc1(
        [in]struct Struct_12_t* arg_1,
        [in][ref][range(0, 256)][string] wchar_t* arg_2,
        [in][ref][range(0, 260)][string] wchar_t* arg_3,
        [in][ref][range(0, 256)][string] wchar_t* arg_4,
        [in]long arg_5,
        [in][unique][range(0, 1024)][string] wchar_t* arg_6,
        [in][range(0, 512)] long arg_7,
        [out]long* arg_8
        // Error    MIDL2121[out] only parameter must not derive from a top - level[unique] or[ptr] pointer / array : [Parameter 'arg_9' of Procedure 'Proc1' (Interface 'DefaultIfName')] SSOCSServerClient    C : \Something.idl    35
        // [out] /* [DBG] FC_BOGUS_ARRAY */[size_is(arg_7)]/*[range(0,512)]*/  /*  */[unique][string] wchar_t* arg_9,
        // Error    MIDL2121[out] only parameter must not derive from a top - level[unique] or[ptr] pointer / array : [Parameter 'arg_10' of Procedure 'Proc1' (Interface 'DefaultIfName')] SSOCSServerClient    C : \Something.idl    35
        // [out] /* [DBG] FC_BOGUS_ARRAY */[size_is(arg_7)]/*[range(0,512)]*/  /*  */[unique][string] wchar_t* arg_10
    );
```

[2024-01-27 12:01] Ignotus: [replying to qwerty1423: "go to IntelliJ's settings, "Build, Execution, Depl..."]
By the way I looked under ”Debugger” but there is no ”Java” there

[2024-01-27 13:02] qwerty1423: [replying to Ignotus: "By the way I looked under ”Debugger” but there is ..."]
sorry, mine was outdated

[2024-01-27 13:03] qwerty1423: ``File -> settings -> Build, Execution, Deployment / Debugger``

[2024-01-27 13:03] qwerty1423: you should find the "add decompiler" dialog there

[2024-01-27 13:05] qwerty1423: if the pattern i sent you in the last response didn't work, just bring the decompiler .jar file into the same directory as intellij

[2024-01-27 13:05] Ignotus: ty ill check when on pc

[2024-01-27 13:07] qwerty1423: btw if you had compatibility problems, use the ``-Xbootclasspath/a`` flag instead of ``javaagent``

[2024-01-27 13:08] qwerty1423: booting up JVM might be a little tricky sometimes

[2024-01-27 21:10] SomethingElse: was this "party" column removed from x64dbg?
[Attachments: image.png]

[2024-01-27 21:22] jvoisin: <@162611465130475520> ^

[2024-01-27 22:21] mrexodia: [replying to SomethingElse: "was this "party" column removed from x64dbg?"]
removed?

[2024-01-27 22:21] mrexodia: I don't think so

[2024-01-27 22:21] mrexodia: or maybe?

[2024-01-27 22:21] mrexodia: for me it's there, maybe you hid it?

[2024-01-27 22:28] root: can double click column name to reveal

[2024-01-27 22:37] SomethingElse: facepalm, it wasnt available in the version i was using, just downloaded the latest one and it showed up <:kekw:904336707439173634>

[2024-01-27 22:39] mrexodia: thanks jonas

[2024-01-27 22:39] mrexodia: for this useful comment

[2024-01-27 22:39] asz: huh

[2024-01-27 22:39] mrexodia: I saw you typing <:kappa:697728545631371294>

[2024-01-27 22:40] asz: cant you at least wait till im done writing stupid comment-before you assume its stupid

[2024-01-27 22:40] mrexodia: xDDDD

[2024-01-27 22:40] asz: as i was about to say- you need a version that supports lemon

[2024-01-27 22:41] asz: because it aint no lemonparty without lemons

[2024-01-27 22:44] root: lol

[2024-01-27 23:13] 25d6cfba-b039-4274-8472-2d2527cb: Yeah u need to run it with the same proton prefix

[2024-01-27 23:14] 25d6cfba-b039-4274-8472-2d2527cb: I remember there being some tools for it

[2024-01-27 23:14] 25d6cfba-b039-4274-8472-2d2527cb: I can't remember the exact one though

[2024-01-27 23:15] 25d6cfba-b039-4274-8472-2d2527cb: I think I remember using https://github.com/sonic2kk/steamtinkerlaunch to launch stuff in proton prefixes
[Embed: GitHub - sonic2kk/steamtinkerlaunch: Linux wrapper tool for use wit...]
Linux wrapper tool for use with the Steam client for custom launch options and 3rd party programs - GitHub - sonic2kk/steamtinkerlaunch: Linux wrapper tool for use with the Steam client for custom ...

[2024-01-27 23:16] 25d6cfba-b039-4274-8472-2d2527cb: but I also remember x64dbg specifically having some instability

[2024-01-28 22:17] Glatcher: how to gather call stack info during tracing (x64dbg)? Each trace point should contain info about call stack

[2024-01-28 22:30] Glatcher: guess there is not built-in feature for that, so I have to code script that is what I figured out, sorry for disturbing

[2024-01-28 23:26] daax: [replying to Glatcher: "how to gather call stack info during tracing (x64d..."]
https://help.x64dbg.com/en/latest/gui/views/CallStack.html