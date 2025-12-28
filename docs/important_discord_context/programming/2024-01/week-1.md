# January 2024 - Week 1
# Channel: #programming
# Messages: 82

[2024-01-03 12:32] gogo: Hello. I just finished a 1430 asm lines shellcode for a school project! I need to insert it in a real .exe now. I have only 2 dayst o finish it. Do you know a WORKING existing infector for 32/64 PE please? it is an emergency!

[2024-01-03 12:33] mrexodia: cff explorer

[2024-01-03 12:33] Deleted User: [replying to gogo: "Hello. I just finished a 1430 asm lines shellcode ..."]
how would you inject this into a 32bit executable if your asm is written strictly for x64

[2024-01-03 12:33] Deleted User: if it was 32bit asm, yeah you could execute it in x64

[2024-01-03 12:35] Deleted User: but I'd just suggest opening the file, extending the image and putting your code there, setting the entry point to point to your stub, fixing up the headers and saving it to disk

[2024-01-03 12:36] gogo: [replying to Deleted User: "how would you inject this into a 32bit executable ..."]
I can not but my shellcode is 32 bits and then can be injected in both 32 and 64 bits

[2024-01-03 12:37] Deleted User: [replying to gogo: "I can not but my shellcode is 32 bits and then can..."]
what does your shellcode do?

[2024-01-03 12:37] gogo: [replying to Deleted User: "what does your shellcode do?"]
it download and execute a bigger binary from internet on my own server

[2024-01-03 12:37] Deleted User: You can't do that on x64

[2024-01-03 12:38] gogo: [replying to Deleted User: "You can't do that on x64"]
no problem

[2024-01-03 12:38] Deleted User: you cant syscall in 32bit

[2024-01-03 12:38] gogo: I cna infect only a 32 bits instead

[2024-01-03 12:38] Deleted User: nor can you call 64bit addresses in 32bit, without switching to long mode

[2024-01-03 12:38] gogo: my shellcode is x32

[2024-01-03 12:38] Brit: I need you guys to write my malware for me, this is an emergency

[2024-01-03 12:38] Deleted User: np guyz he said for school

[2024-01-03 12:38] gogo: it is for school

[2024-01-03 12:38] Deleted User: real emergency

[2024-01-03 12:39] Deleted User: https://media.discordapp.net/attachments/1173410970299809792/1189634864140529776/chirpani_ghost_file_140530847.gif?ex=659ee0cd&is=658c6bcd&hm=60d3687fd87581d4c76af40636560b70453db162433e21b326831ecc2789d8f8&

[2024-01-03 12:39] Deleted User: and even if you are writing a real malware, this stuff wont go far without triggering every av in the world lol

[2024-01-03 12:40] gogo: [replying to Deleted User: "and even if you are writing a real malware, this s..."]
no problem

[2024-01-03 12:40] gogo: my teacher told me I can have a good graduation even if detected

[2024-01-03 12:41] Deleted User: use it on your teacher, fix grades

[2024-01-03 12:41] Deleted User: ez pz

[2024-01-03 12:41] Deleted User: https://tenor.com/zbNkZ4lqHj.gif

[2024-01-03 13:10] JustMagic: [replying to gogo: "it is for school"]
what kind of school asks you to write assembly for malware

[2024-01-03 13:10] Brit: school of life <:topkek:904522829616263178>

[2024-01-03 13:17] JustMagic: <@638470686737694730> if it doesn't have to be fancy, you can probably just add a section and set entry point to your shellcode using a few lines of LIEF

[2024-01-03 13:21] gogo: [replying to JustMagic: "<@638470686737694730> if it doesn't have to be fan..."]
yes I had also think to this idea.

[2024-01-03 16:55] donna🤯: [replying to JustMagic: "what kind of school asks you to write assembly for..."]
a cool one😎

[2024-01-03 17:37] Deleted User: man my school is teachin java

[2024-01-03 17:37] Deleted User: it SUCKS

[2024-01-03 17:39] BrightShard: countless other languages tried and failed to fix java's issues

[2024-01-03 17:40] BrightShard: this is clearly because java has no issues so there's nothing to fix <:Trolled:768152256108298240>

[2024-01-03 18:04] contificate: Java is comfy for most of what your school will be doing honestly

[2024-01-03 18:21] mrexodia: [replying to contificate: "Java is comfy for most of what your school will be..."]
C# is the correct way

[2024-01-03 18:49] contificate: mald

[2024-01-03 19:41] daax: [replying to mrexodia: "C# is the correct way"]
D

[2024-01-03 19:41] daax: <:Kappa:794707301436358686>

[2024-01-03 21:17] dullard: [replying to Deleted User: "you cant syscall in 32bit"]
<:what:940916054396395521>

[2024-01-03 21:51] diversenok: WoW64 code doesn't issue syscalls, it calls into the 64-bit ntdll that does that

[2024-01-03 21:51] Deleted User: [replying to BrightShard: "this is clearly because java has no issues so ther..."]
<:gigachad:904523979249815573>

[2024-01-03 22:05] dullard: [replying to diversenok: "WoW64 code doesn't issue syscalls, it calls into t..."]
Hmm, I didn't take WOW64 into consideration, from what I knew that 32bit processes on a 32bit host do use the int syscall handler, no?

[2024-01-03 22:07] diversenok: It might be a different instruction, but sure

[2024-01-03 22:34] daax: [replying to dullard: "<:what:940916054396395521>"]
You can’t use the `syscall` instruction in compat. It will just #UD is what he means

[2024-01-03 22:55] dullard: Yeah okay 👌

[2024-01-03 22:56] dullard: [replying to diversenok: "It might be a different instruction, but sure"]
Yeah it’s `int 2Eh`

[2024-01-04 06:20] 0xatul: [replying to Deleted User: "man my school is teachin java"]
Get a really long monitor kekw

[2024-01-04 07:53] JustMagic: [replying to daax: "You can’t use the `syscall` instruction in compat...."]
Doesn't it work on AMD CPUs through CSTAR?

[2024-01-04 10:56] qwerty1423: hi, i need a python module to code graphical programs with, my focus is on creating the UI for now.
i have only used TKinter but tk programs look like early 2004 windows programs at their best.

[2024-01-04 10:59] Deleted User: [replying to qwerty1423: "hi, i need a python module to code graphical progr..."]
isnt there a imgui binding for python

[2024-01-04 10:59] Deleted User: https://github.com/pyimgui/pyimgui
[Embed: GitHub - pyimgui/pyimgui: Cython-based Python bindings for dear imgui]
Cython-based Python bindings for dear imgui. Contribute to pyimgui/pyimgui development by creating an account on GitHub.

[2024-01-04 11:00] Deleted User: this yea

[2024-01-04 11:03] qwerty1423: [replying to Deleted User: "https://github.com/pyimgui/pyimgui"]
checked the documentations, looks cool, thanks

[2024-01-04 11:04] Deleted User: np

[2024-01-04 23:02] hxm: <@162611465130475520> is there any issue with the recent cmkr changes ? 

```yaml
[variables]
OpenSSL_DIR="/xdev/openssl/build"

[find-package.OpenSSL]
required=true
config=true


[target.nrts]
type = "executable" # static, shared
sources = ["src/*.cpp", "src/*.h", "src/*.hpp"]
include-directories = ["include","${OPENSSL_INCLUDE_DIR}"]
compile-features = ["cxx_std_20"]
link-libraries = ["OpenSSL::SSL", "OpenSSL::Crypto"]
```

why it can not find the include path ? i'm on m1 is there any special thing i can do to debug it further ?

[2024-01-04 23:03] hxm: found this issue only with OpenSSL

[2024-01-04 23:27] mrexodia: [replying to hxm: "<@162611465130475520> is there any issue with the ..."]
What is the exact error? In general you shouldn't set stuff like `OpenSSL_DIR` in the `cmkr.toml`, pass it on the command line

[2024-01-05 22:23] diversenok: Question to COM gurus: many code samples for interacting with WMI use the following sequence of calls: `CoInitializeEx` -> `CoInitializeSecurity` -> `CoCreateInstance(CLSID_WbemLocator)` -> `ConnectServer` -> `CoSetProxyBlanket`. What is the point of `CoInitializeSecurity` here? As I understand, the options we specify in `CoSetProxyBlanket` take precedence, so why would we want to change the settings for the entire process (in a thread-unsafe way) via  `CoInitializeSecurity` when we can (and already do) adjust them on the proxy?

[2024-01-05 22:25] diversenok: I tested, and it seems to work when using only one of either `CoInitializeSecurity` or `CoSetProxyBlanket`

[2024-01-05 22:26] diversenok: I do get why we need at least one (to raise the the impersonation level from identity to impersonate), but what's the point of doing both? Am I missing something?

[2024-01-05 23:05] asz: umm- isnt there something about something have to become initialised that pretty much always done already?

[2024-01-05 23:09] diversenok: I think COM automatically initializes security, the problem is it sets impersonation level to identification which doesn't work for WMI

[2024-01-05 23:09] mrexodia: iirc you also don't need CoInitializeEx (depending on your thread)

[2024-01-05 23:10] diversenok: [replying to mrexodia: "iirc you also don't need CoInitializeEx (depending..."]
Well in UI applications sure; otherwise you still need to choose an apartment/multithreaded mode

[2024-01-05 23:11] mrexodia: But you should just call the functions Microsoft tells you to call in the order they specify. The fact that it happens to work on your machine doesn't mean it will work for everyone

[2024-01-05 23:11] diversenok: `CoCreateInstance` doesn't work if `CoInitializeEx` has never been called in the process

[2024-01-05 23:11] diversenok: MS tells different things in different places

[2024-01-05 23:12] diversenok: There is also an official sample without `CoInitializeSecurity`

[2024-01-05 23:12] diversenok: So `CoInitializeEx` -> `CoCreateInstance(CLSID_WbemLocator)` -> `ConnectServer`

[2024-01-05 23:13] diversenok: The example unfortunately ends there and doesn't do anything else

[2024-01-05 23:13] diversenok: As I said, you do need at least `CoSetProxyBlanket` to set the allowed impersonation level since WMI likes to impersonate its clients

[2024-01-05 23:15] diversenok: So my question is do you need `CoInitializeSecurity` when you use `CoSetProxyBlanket` or not

[2024-01-05 23:16] diversenok: In other words, is there something that `CoSetProxyBlanket` cannot cover (assuming you can call it on the proxy and it supports it) that still requires `CoInitializeSecurity`

[2024-01-05 23:17] diversenok: I don't really like the idea of using `CoInitializeSecurity` in library code because it's a function that affects the entire process according to docs

[2024-01-05 23:17] diversenok: Even `CoInitializeEx` is per-thread

[2024-01-05 23:19] diversenok: So it sound like a bad idea, similar to how setting the process-wide current directory is discouraged if you share the process with other code

[2024-01-05 23:22] diversenok: [replying to mrexodia: "But you should just call the functions Microsoft t..."]
I prefer to try to deviate from the well-established order and known parameter values because it allows finding vulnerabilities 😇

[2024-01-05 23:23] mrexodia: [replying to diversenok: "I prefer to try to deviate from the well-establish..."]
Yeah fair enough in that case <:lmao3d:611917482105765918>

[2024-01-05 23:24] mrexodia: I assumed it was about programming since this is the <#835664858526646313> channel

[2024-01-05 23:25] diversenok: We don't really have a `#windows-internals` channel, so ¯\_(ツ)_/¯

[2024-01-05 23:33] contificate: #memorising-lore