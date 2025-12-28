# July 2024 - Week 2
# Channel: #programming
# Messages: 153

[2024-07-10 08:09] luci4: Hey so I created a child process with redirected I/O and I want to communicate with it. This is probably not the best approach as there must be a way get alerted when a pipe receives a message, but I made two functions `ReadFromPipe` and `WriteToPipe` and created a thread for each of them:
[Attachments: image.png, image.png]

[2024-07-10 08:10] luci4: Printing the output seems to work fine, but none of the commands I send to the child's stdin pipe seem to be received

[2024-07-10 08:10] 5pider: you can peek from the pipe if anything has been written into it using `PeekNamedPipe`

[2024-07-10 08:11] 5pider: [replying to luci4: "Printing the output seems to work fine, but none o..."]
hhow did you create the stdin pipe ?

[2024-07-10 08:11] 5pider: is it a handle you have created using CreatePipe ?

[2024-07-10 08:12] luci4: [replying to 5pider: "is it a handle you have created using CreatePipe ?"]
Yep!

[2024-07-10 08:12] luci4: 
[Attachments: image.png]

[2024-07-10 08:12] luci4: the if tower

[2024-07-10 08:13] luci4: The main thread just waits:
[Attachments: image.png]

[2024-07-10 08:21] luci4: should I have used `CreateNamedPipe`?

[2024-07-10 09:00] x86matthew: hard to see what you're doing from the random screenshots of code blocks

[2024-07-10 09:00] x86matthew: but CreatePipe creates a blocking pipe in byte-mode, and since you have a dedicated thread for reading from the pipe you don't need to use PeekNamedPipe

[2024-07-10 09:00] x86matthew: just create a fixed buffer and call ReadFile in a loop

[2024-07-10 09:00] x86matthew: if there's no data to receive it'll just wait

[2024-07-10 09:49] luci4: [replying to x86matthew: "hard to see what you're doing from the random scre..."]
I'm creating the pipes in `CreatePipes` and passing them to a child process as stdin/stdout/stderr. Then I'm creating two threads to be able to read its output and pass commands to it

[2024-07-10 09:54] luci4: The child doesn't seem to respond to any commands though

[2024-07-10 10:01] x86matthew: what does your STARTUPINFO struct look like?

[2024-07-10 10:07] luci4: [replying to x86matthew: "what does your STARTUPINFO struct look like?"]

[Attachments: image.png]

[2024-07-10 10:16] x86matthew: looks fine, i assume you are writing to the stdin_write pipe to write to stdin?

[2024-07-10 10:16] x86matthew: if so, check that WriteFile is actually being called there

[2024-07-10 10:17] x86matthew: and check if you are missing a newline character at the end or something, it might be in the buffer but not being flushed

[2024-07-10 10:17] x86matthew: try changing your child process to cmd.exe for testing, not sure what you're using atm

[2024-07-10 10:17] luci4: [replying to x86matthew: "looks fine, i assume you are writing to the stdin_..."]
Yep:

```
    while (TRUE)
    {    
        std::string Command;
        std::getline(std::cin, Command);

        DWORD BytesWritten{};

        if (!WriteFile(ChildStdinW, Command.c_str(), Command.length(), &BytesWritten, NULL))
        {
            break;
        }
    }
```

[2024-07-10 10:26] luci4: The WriteFile call also seems to work, I added a check after:
[Attachments: image.png]

[2024-07-10 10:27] luci4: OMG

[2024-07-10 10:27] luci4: `Command += '\n';` this was all it took

[2024-07-10 10:27] luci4: I thought `std::getline` added that 💢

[2024-07-10 10:29] x86matthew: [replying to x86matthew: "and check if you are missing a newline character a..."]
^ yeah i suspected it might be related to newlines

[2024-07-10 10:29] x86matthew: dunno about `std::getline` though, i only use plain C

[2024-07-10 10:31] luci4: [replying to x86matthew: "^ yeah i suspected it might be related to newlines"]
Well thanks A LOT, I'm never going to admit how much time I spent debugging this

[2024-07-10 10:50] x86matthew: happens to us all 🫠

[2024-07-10 10:53] diversenok: You don't need to use `PeekNamedPipe`; the code from the MS example didn't use them

[2024-07-10 10:53] diversenok: Instead, afte you create the pipe and let the child process inherit the handles you should close these handles in the parent process

[2024-07-10 10:54] diversenok: This way you can read/write using another pipe end in a blocking mode until the child process disconnects

[2024-07-10 10:54] luci4: [replying to diversenok: "Instead, afte you create the pipe and let the chil..."]
I did!

[2024-07-10 10:55] luci4: [replying to diversenok: "You don't need to use `PeekNamedPipe`; the code fr..."]
I changed that, this is what I ended up with:
[Attachments: image.png]

[2024-07-10 10:55] luci4: Which is much shorter than what I initially did lol

[2024-07-10 10:55] diversenok: Yeah, that looks like it

[2024-07-10 11:00] luci4: [replying to diversenok: "Yeah, that looks like it"]
Finally! The most satisfying `Clean Solution` yet

[2024-07-10 16:49] mrexodia: [replying to luci4: "I changed that, this is what I ended up with:"]
The problem is that in the CRT initialization code, it checks the stdout/stdin handles in the PEB. If they are a console handle the buffering is disabled, but if they are a pipe (or file) then things will be buffered.

[2024-07-10 16:50] mrexodia: Because of this if you have `printf("Enter your name: ")` (without a newline) you will never receive this prompt in your stdout pipe. And the same the other way around

[2024-07-10 16:50] mrexodia: (I ran into this when trying to add proper console support to x64dbg, but it's essentially impossible without dealing with that new conhost API)

[2024-07-10 16:54] luci4: [replying to mrexodia: "The problem is that in the CRT initialization code..."]
Welp fortunately it works now. TIL!

[2024-07-10 16:54] luci4: Of course I ran into another weird problem

[2024-07-10 16:55] luci4: As it tries to look for `GetTempPath2W` in my PE

[2024-07-10 19:44] luci4: [replying to luci4: "Of course I ran into another weird problem"]
This is pretty weird:
[Attachments: image.png]

[2024-07-10 19:45] luci4: I thought it was because I was using the single header `phnt` but switching to `Windows.h` did nothing

[2024-07-10 19:45] luci4: I am linking against `kernel32.lib`, I just checked

[2024-07-10 19:47] luci4: Changing to `GetTempPathW` fixed it, but I wonder what caused this in the first place

[2024-07-10 19:47] diversenok: Windows version is too old?

[2024-07-10 19:48] diversenok: This function requires Windows 11

[2024-07-10 19:50] diversenok: It says that the system cannot satisfy a dependency while loading your exe; it's not actually looking for this function in your exe

[2024-07-10 19:59] luci4: [replying to diversenok: "This function requires Windows 11"]
I was on Windows 10. MSDN says Windows 10 is supported though

[2024-07-10 19:59] luci4: 
[Attachments: image.png]

[2024-07-10 20:00] diversenok: If you are on 22H2 you have a lower build number

[2024-07-10 20:00] diversenok: Compare 20348 to what winver.exe says

[2024-07-10 20:02] luci4: Oh...

[2024-07-10 20:02] diversenok: Well, I think 20348 was actually supposed to be 21H1; I don't really understand how it works with their different numbering systems

[2024-07-10 20:02] diversenok: Anyway, the latest Win 10 doesn't have this function

[2024-07-10 20:04] luci4: [replying to diversenok: "Anyway, the latest Win 10 doesn't have this functi..."]
That's pretty interesting

[2024-07-10 22:51] Matti: there's no such thing as a windows 10 build 20348, that build number is for server 2022

[2024-07-10 22:52] Matti: so the 'minimum supported client' listed on that page is simply wrong

[2024-07-10 22:54] Matti: win 10 ended at 19041
server 2022 = 20348
win 11 = 22000, 22621 (and the latest SDK/WDK version is 26100, which is kinda unusual since these are usually released at the same time as the OS builds)

[2024-07-11 01:46] th3: how to stop a program from reading information from physicaldrive

[2024-07-11 01:48] th3: oh wait im going to try something

[2024-07-11 23:16] Bloombit: Hooks might work

[2024-07-11 23:19] th3: [replying to Bloombit: "Hooks might work"]
i think i would have to go kernel hooks would not work

[2024-07-11 23:24] Bloombit: Without info about the program, who knows lol

[2024-07-12 00:38] th3: [replying to Bloombit: "Without info about the program, who knows lol"]
i tried

[2024-07-12 00:39] th3: unless there are other ways to stop a program from opening a handle on um i think it would have to be on the kernel

[2024-07-12 03:56] sync: [replying to th3: "unless there are other ways to stop a program from..."]
you can hook NtCreateFile inside the program itself

[2024-07-12 03:56] sync: depending on the checks it has, a simple minhook implementation will be much easier than hooking the kernel itself

[2024-07-12 06:40] th3: [replying to sync: "you can hook NtCreateFile inside the program itsel..."]
yeah I tried

[2024-07-13 19:08] 5pider: do you have the source of the application? If yes why not apply some LLVM pass to obfuscate it at compile time. Besides that I would have said vnprotect as well loll

[2024-07-13 19:23] x86matthew: rename your variables, eg change `int aimbot_enabled` to `int fb854246696d4a3d`, the longer the better

[2024-07-13 19:23] x86matthew: if you really want to go hardcore then xor encrypt your strings (don't share this with anyone, but 0x35 is the best key)

[2024-07-13 19:26] Terry: going off what x86matthews solid advice, id recommend creating a macro that defines some variables and loops through them, changing the values. then place it on every line.

[2024-07-13 19:26] Terry: (make sure the variable names are really long, for the best obfuscation)

[2024-07-13 19:27] Terry: its more of an advanced technique, but you should be able to figure it out

[2024-07-13 19:27] snowua: [replying to x86matthew: "if you really want to go hardcore then xor encrypt..."]
why would you leak this? now i have to find a new one

[2024-07-13 19:28] Brit: wait you were also xoring with 0x35

[2024-07-13 19:28] Brit: I also need to change my key

[2024-07-13 19:28] Brit: wtf

[2024-07-13 19:29] Terry: 0x35 got leakd awhile ago
[Attachments: image.png]

[2024-07-13 19:29] Terry: 350k `codenz` available on the website

[2024-07-13 19:29] diversenok: Try 0x0035 then

[2024-07-13 19:29] Azrael: You're using 0x35? Lucky me, I'm using 53.

[2024-07-13 19:33] Deleted User: if you are getting ur memory signature scanned then i think something is probably already on the wrong path :p that wouldnt help much

[2024-07-13 19:38] x86matthew: 0x20 would've been a better joke in hindsight 🫠

[2024-07-13 19:40] 5pider: [replying to x86matthew: "rename your variables, eg change `int aimbot_enabl..."]
thank you Matthew.

[2024-07-13 19:41] Brit: I like xoring my strings with themselves

[2024-07-13 19:55] snowua: Its a good idea to xor them or else when someone opens the binary inside of IDA they will be able to see the variable names

[2024-07-13 19:58] 冰: hmm

[2024-07-13 19:59] 冰: hmm

[2024-07-13 20:36] elias: How does VMProtect deal with kernel callbacks being registered by protected code?

[2024-07-13 20:41] Deleted User: isnt that just a function call, it will probably just execute outside of the vm

[2024-07-13 22:03] rin: Ok real talk, whats the best youtube channel to download free csgo cheats from?

[2024-07-13 22:08] Deleted User: all of them!!

[2024-07-13 22:11] 冰: [replying to elias: "How does VMProtect deal with kernel callbacks bein..."]
hmm

[2024-07-13 22:13] elias: [replying to 冰: "hmm"]
yes of course

[2024-07-13 22:14] elias: but obviously bad things are gonna happen if the kernel wants to invoke a callback inside of the custom opcode region

[2024-07-13 22:53] Deleted User: [replying to elias: "but obviously bad things are gonna happen if the k..."]
why would the callback be inside any opcode region

[2024-07-13 22:53] Deleted User: if you register a callback dont you pass a pointer to that callback into the function, and doesnt that callback usually reside somewhere else in a seperate function?

[2024-07-13 22:54] Brit: you do, and it is shrimple to have a stub that just re enters the arguments of the callback into the vm

[2024-07-13 22:55] elias: [replying to Brit: "you do, and it is shrimple to have a stub that jus..."]
yeah thats what I had in mind but I wanted to ask just in case theres a smarter way to do this

[2024-07-13 22:56] Deleted User: i just execute calls unvirtualized (vmexit, then reenter) and that would solve this i think

[2024-07-13 22:57] Deleted User: u just virtualize the callback function aswell then no problem, obviously ur vm would have to support multithreading but that shouldnt be an issue

[2024-07-13 23:06] elias: [replying to Deleted User: "i just execute calls unvirtualized (vmexit, then r..."]
yes that is clear, what I meant was the following

Inside the VM we want to register a callback using `CmRegisterCallback`. The first argument of this function is a pointer to the callback function (lets call it `MyCallbackFunc`) that the kernel will call when a relevant event occurs on the system. To call the `CmRegisterCallback` we obviously have to perform a vmexit, no problem. However, if we just translated the calling convention from our custom vm to x86 then now (assuming `MyCallbackFunc` is also a virtualized function) the first argument of `CmRegisterCallback` will point to a virtualized function that will cause a crash once the kernel tries to invoke it.

So what I had in mind is that the VM will intercept these callback registration calls and instead of simply converting the calling conventions like with normal functions it will here register an internal non virtualized wrapper and maintain internal callback list. Once the callback is triggered, the non virtualized 'wrapper' will get executed, read the internal callback list and perform a vm enter for each...

[2024-07-13 23:07] Deleted User: calling convention issues? i dont think i have this issue

[2024-07-13 23:08] elias: no its not about calling convetion

[2024-07-13 23:08] Brit: [replying to elias: "yes that is clear, what I meant was the following
..."]
register a eh handler and eat &MyCallbackFunc's memory worth of exception,resume execution with context to that address that re enters  🧠

[2024-07-13 23:08] Deleted User: i make sure the stack does not get modified on vmexits and vmenters (anything above rsp i mean) and i preserve volatile and non volatile regs

[2024-07-13 23:08] Deleted User: so idk i dont fully get it ig it depends on your vm implementation

[2024-07-13 23:08] Deleted User: because the virtualized function in my scenario wouldnt be a crash since it points to a diff virtualized function which would just jump to a vmentry

[2024-07-13 23:09] Deleted User: they use the same vm but different bytecode since they are seperate functions

[2024-07-13 23:10] elias: [replying to Deleted User: "because the virtualized function in my scenario wo..."]
well I dont invoke the callback myself, the kernel will invoke the callback and it doesnt know about any virtualized code, it will just try to jump to the provided address

[2024-07-13 23:10] Brit: yeah, so make that addr a stub that re enters

[2024-07-13 23:10] Deleted User: yeah and the provided address is a replacement of the original function that jumps to vmentry
like i said for me i would just *seperately* virtualize both functions they can be called independent from each other

[2024-07-13 23:12] elias: [replying to Deleted User: "yeah and the provided address is a replacement of ..."]
how did you implement this in your vm so that it detects callback function pointers and replaces them with vmentry address?

[2024-07-13 23:13] Deleted User: maybe i misunderstand im assuming that the callback is not connected to the vm but its users code that you are virtualizing?

[2024-07-13 23:13] Deleted User: or is it something the vm registers for itself somehow

[2024-07-13 23:18] elias: [replying to Deleted User: "or is it something the vm registers for itself som..."]
yeah what I mean is the protected program running inside the vm registers callbacks with the kernel outside of the vm.

[2024-07-13 23:18] Deleted User: ya like i said the callback is a diff function one sec

[2024-07-13 23:19] Brit: you exit the vm to call the register callback func, you provide the address of a stub that re enters, (this stub is not protected) the stub re enters at the actual protected version of the callback, which will exit again once you're done with whatever you needed the callback for

[2024-07-13 23:19] Deleted User: ```rust
fn callback() {
  callback stuff
}

fn secret_virtualized_function() {
  register_callback(callback);
}
```
if virtualized they both would be something like
```
callback:
push bytecode
jmp vmentry

secret_virtualized_function:
push bytecode
jmp vmentry
```

[2024-07-13 23:19] Brit: so it can deny the handle or let it through say in the case of a obcallback

[2024-07-13 23:20] Deleted User: so the kernel just calls callback which jmps to a vmentry with its seperate bytecode

[2024-07-13 23:20] Deleted User: no extra work needed if u just devirtualize for calls and reenter after the call instruction

[2024-07-13 23:21] Deleted User: [replying to Brit: "you exit the vm to call the register callback func..."]
tis is kindof a diff approach but i guess you could also see the normal virtualized callback as stub so in the end its the same result

[2024-07-13 23:22] Brit: ++

[2024-07-13 23:22] Deleted User: tldr: create an opcode that allows u to include raw instructions in the bytecode (u can encrypt them ofc) to allow supporting not directly supported instructions

[2024-07-13 23:22] Deleted User: make sure to check for relative control flow instructions you dont want those if they arent explicitly supported bcuz that would break your code

[2024-07-13 23:23] elias: yes yes

[2024-07-13 23:23] elias: thank you

[2024-07-13 23:23] elias: I got it

[2024-07-13 23:23] Deleted User: oki ^-^

[2024-07-14 15:16] luci4: Has anyone encountered a `STATUS_PARTIAL_COPY` error when overwriting a member of the PEB?

[2024-07-14 15:16] luci4: I'm trying to overwrite `ImageBaseAddress` but it fails with said error, even though x64dbg shows the area as `RW`

[2024-07-14 15:17] diversenok: It might also mean it failed to read the entire thing from the buffer you provided

[2024-07-14 15:18] luci4: [replying to diversenok: "It might also mean it failed to read the entire th..."]
Surely it can read an `ULONGLONG`?

[2024-07-14 15:18] Brit: it'd be easier to diagnose if you showed code

[2024-07-14 15:19] luci4: [replying to Brit: "it'd be easier to diagnose if you showed code"]
I was removing my schizo debugging, 1 sec

[2024-07-14 15:19] diversenok: [replying to luci4: "Surely it can read an `ULONGLONG`?"]
Not if you pass it by value, for instance

[2024-07-14 15:19] luci4: [replying to diversenok: "Not if you pass it by value, for instance"]
Well it seems to have worked now

[2024-07-14 15:19] luci4: Let me check if its actually been overwritten

[2024-07-14 15:22] luci4: Welp it worked now

[2024-07-14 15:40] diversenok: Well, that just replaces the question of "why doesn't it work" with "why does it work"

[2024-07-14 15:41] diversenok: PEB is indeed writable and it's pretty difficult to make it otherwise

[2024-07-14 15:43] diversenok: If `NtWriteVirtualMemory` returns `STATUS_PARTIAL_COPY` when writing into PEB it means you either try to write (partially) outside or try to read from an invalid address

[2024-07-14 15:44] diversenok: The last might happen if you pass this `ULONGLONG` value directly instead of passing a pointer to the variable

[2024-07-14 15:45] diversenok: But you might also accidentally try to write to where `ImageBaseAddress` points instead of overwriting the field in PEB

[2024-07-14 15:46] Brit: hence, show code

[2024-07-14 15:49] luci4: [replying to diversenok: "The last might happen if you pass this `ULONGLONG`..."]
Passing a pointer fixed it!