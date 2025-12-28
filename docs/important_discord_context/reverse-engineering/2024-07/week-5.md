# July 2024 - Week 5
# Channel: #reverse-engineering
# Messages: 64

[2024-07-30 08:43] 0xatul: [replying to mrexodia: "You can also swallow (like you did with CrowdStrik..."]
Does your debugger swallow || exceptions || ?

[2024-07-30 10:01] Taks: [replying to 0xatul: "Does your debugger swallow || exceptions || ?"]
https://tenor.com/view/skinenr-gif-20761908

[2024-07-30 12:12] marc: hey, do you guys have any tips for reversing an application with the goal of finding about cryptographic operations? there's a big native application which I believe does some AES encryption on some files, but it has no symbols and has lots of functions, so I'm trying to look for encryption-related strings and working from there.
I have thought about doing something in a debugger, breaking when a certain file is read, and trying to look for any crypto-related API calls, or looking for new memory allocations and setting hardware breakpoints on them to see when they are written to, but I haven't found anything so far.
since the encryption functions would (likely) be internal I'm not really sure if there's an easy way to identify them

[2024-07-30 12:13] marc: I'm pretty much hoping to either find hardcoded keys or at least find how they are generated

[2024-07-30 12:18] Timmy: prefacing this with saying I've never done this: I'd go look at an AES implementation that you expect is being used and find some of the unique values in the stuff it uses with the key tables.

[2024-07-30 12:19] elias: aes uses a matrix with fixed values in the mixcolumns base operation (with some implementations at least)

[2024-07-30 12:19] elias: you could try scanning for this matrix and see where its referenced in the code

[2024-07-30 12:20] marc: hmmmmm

[2024-07-30 12:21] Timmy: noct said what I wanted to say, but in the good way of saying it <:OMEGALUL:662670462215782440>

[2024-07-30 12:21] marc: [replying to elias: "you could try scanning for this matrix and see whe..."]
this seems promising but I'll have to look more into it

[2024-07-30 12:21] marc: tbh I wouldn't even know how to start looking for it

[2024-07-30 12:21] marc: not really sure what kind of pattern to look for

[2024-07-30 12:21] marc: but I'm guessing maybe compiling another simpler app that uses the same algo I believe gets used, and comparing it or something

[2024-07-30 12:24] elias: https://github.com/kokke/tiny-AES-c/blob/master/aes.c
[Embed: tiny-AES-c/aes.c at master · kokke/tiny-AES-c]
Small portable AES128/192/256 in C. Contribute to kokke/tiny-AES-c development by creating an account on GitHub.

[2024-07-30 12:24] elias: you see aes uses some static values

[2024-07-30 12:24] elias: alternatively you can scan for rcon

[2024-07-30 12:37] marc: thanks

[2024-07-30 12:37] marc: tried looking for those but not matches unfortunately

[2024-07-30 12:56] luci4: [replying to marc: "tried looking for those but not matches unfortunat..."]
Have you tried capa? It looks for encryption constants and can tell you where they were utilized

[2024-07-30 12:56] luci4: I used it recently with great results

[2024-07-30 12:57] marc: will try now 👍

[2024-07-30 12:57] marc: I'm finally finding some interesting functions, but I'm not quite there yet

[2024-07-30 12:57] luci4: [replying to marc: "will try now 👍"]
Tell me how it goes! Don't forget the -vv flag

[2024-07-30 12:57] luci4: 😄

[2024-07-30 13:13] marc: lol it didn't even print anything

[2024-07-30 13:13] marc: "analyzing program..."
* dies *

[2024-07-30 13:13] wallaby: [replying to marc: "hey, do you guys have any tips for reversing an ap..."]
If the application implements the AES algo you might also want to try `binwalk`, which can detect the standard AES substitution tables (a.k.a. s-boxes) and give their offsets

[2024-07-30 13:14] marc: oh I had no idea binwalk could detect sboxes

[2024-07-30 13:16] wallaby: aye, used it recently on some thumb2 firmware, works a treat!

[2024-07-30 14:01] luci4: [replying to marc: ""analyzing program..."
* dies *"]
Lol that's interesting, it worked perfectly for me

[2024-07-30 14:53] expy: [replying to marc: "hey, do you guys have any tips for reversing an ap..."]
tried PEiD & "Krypto ANALyzer"?

[2024-07-30 14:54] expy: is there a phnt.h for structs like EPROCESS compatible with winddk.h?

[2024-07-31 07:37] marc: [replying to expy: "tried PEiD & "Krypto ANALyzer"?"]
thanks, I didn't know about it

[2024-07-31 07:37] marc: it's giving me paths to go through 😄

[2024-07-31 11:28] elias: I have a question about IDA Pro licenses

[2024-07-31 11:29] elias: does the first option not include an ARM decompiler?
[Attachments: Screenshot_2024-07-31_132858.png]

[2024-07-31 11:43] Timmy: That'd be nice but no iirc

[2024-07-31 11:44] elias: so if I wanted to decompile x64 and ARM, I would first have to buy the first license for 2k and then the second one for 3k on top? 🤔

[2024-07-31 11:44] elias: that sounds stupid

[2024-07-31 11:45] Timmy: <:topkek:904522829616263178>

[2024-07-31 11:45] Timmy: I agree

[2024-07-31 11:47] elias: imagine binary ninja offers it combined for $80 😭

[2024-07-31 11:47] elias: (for students)

[2024-07-31 11:52] elias: [replying to elias: "does the first option not include an ARM decompile..."]
wait does the first license include any decompiler at all??

[2024-07-31 11:52] elias: or I need to spend 2k for the license and then 6k for x64 and ARM64?

[2024-07-31 11:53] elias: this is insanity

[2024-07-31 12:04] Saturnalia: [replying to elias: "wait does the first license include any decompiler..."]
no

[2024-07-31 12:34] Brit: [replying to elias: "imagine binary ninja offers it combined for $80 😭"]
Yeah but then you have to use binja

[2024-07-31 12:35] Brit: Although I hear it's better now

[2024-07-31 12:44] elias: its not bad imo

[2024-07-31 12:44] elias: but not as good as ida

[2024-07-31 13:00] Brit: I mean I was using it two weeks ago and it was quite badTM when dealing with anything non standard

[2024-07-31 15:12] zeropio: [replying to elias: "wait does the first license include any decompiler..."]
I think you have the x64 cloud decompiler, same as the free version

[2024-07-31 15:18] elias: ah that would be great

[2024-07-31 19:57] Matti: [replying to Brit: "Although I hear it's better now"]
I tried it the other day
if it's better now, I don't want to know what it was like before

[2024-07-31 19:58] Matti: took an hour to load ntoskrnl before I could see the pseudocode for a function that (I thought) hex rays did a poor job of decompiling

[2024-07-31 19:58] Matti: binja redefined 'poor job' for me there

[2024-07-31 20:14] SomethingElse: i tried to use it for Mach-O's but then switched to hopper

[2024-07-31 20:15] Brit: [replying to Matti: "I tried it the other day
if it's better now, I don..."]
Is the other day pre or post the recent patch that allegedly really improved hlil / decomp

[2024-07-31 20:16] Matti: yesterday

[2024-07-31 20:16] Brit: I'm asking because I've had issues with the latest version prior to that patch and Im currently busy touching grass so I couldn't Czech 🇨🇿  it since

[2024-07-31 20:16] Brit: Ah, sad

[2024-07-31 23:20] elias: Does vmprotect protect data like strings in an interesting way?

[2024-07-31 23:33] daax: [replying to elias: "Does vmprotect protect data like strings in an int..."]
not really