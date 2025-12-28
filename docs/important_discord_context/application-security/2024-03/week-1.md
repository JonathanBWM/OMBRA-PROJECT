# March 2024 - Week 1
# Channel: #application-security
# Messages: 22

[2024-03-01 16:23] 0xatul: [replying to Matti: "got too tired of this shit not working"]
make that shite public man

[2024-03-01 20:25] Matti: wish I could

[2024-03-01 20:26] Matti: but this is based on the IDA SDK PDB plugin code

[2024-03-01 20:27] Matti: based on past experiences (<https://github.com/x64dbg/ScyllaHide/issues/22#issuecomment-341970313>) I know that ilfak does not appreciate people redistributing the SDK in any form

[2024-03-03 11:28] Deleted User: [replying to Matti: "amazingly this limitation is from dbghelp, not eve..."]
may i ask, sorry if this might be silly but why are you formatting strings like "MESSAGE GetLastError()" instead of "Error-Code: %u" something along this line as we know GetLastError returns 0x something etc.
[Attachments: image.png]

[2024-03-03 11:29] Deleted User: so if there was an error you can have a nice clean format "Something went wrong with {wtv} | Error-Code: 0x2"

[2024-03-03 11:30] brymko: never post your opinion again

[2024-03-03 11:30] Deleted User: bruh.

[2024-03-03 11:30] Deleted User: LOL

[2024-03-03 11:31] Deleted User: i thought it would look nicer then saying GetLastError returned this code? bc what else?

[2024-03-03 11:34] 25d6cfba-b039-4274-8472-2d2527cb: Then you have to read the code first to figure out what produced the error code. If you specify that it's GetLastError the user can just open up the documentation for Windows error codes to look it up.

[2024-03-03 11:35] 25d6cfba-b039-4274-8472-2d2527cb: You know, because WinAPI surely isn't the only thing capable of producing error codes.

[2024-03-03 11:35] Deleted User: i see, i simply just thought it would look better

[2024-03-03 11:35] Deleted User: [replying to 25d6cfba-b039-4274-8472-2d2527cb: "You know, because WinAPI surely isn't the only thi..."]
this makes since thanks

[2024-03-03 17:41] Timmy: just use FormatMessage and easily get a nice localized error string.

[2024-03-03 17:41] Timmy: and at that rate you might also include a std::stacktrace

[2024-03-03 17:52] Terry: Does msvc have stacktrace yet?

[2024-03-03 18:12] Timmy: I actually only assumed it does

[2024-03-03 18:12] Timmy: it might not

[2024-03-03 18:20] qwerty1423: Debug -> Windows -> Call Stack

[2024-03-03 18:20] qwerty1423: or you can do this while debugging
```CTRL + Alt + C```

[2024-03-03 18:21] qwerty1423: it might not work in <2013 versions