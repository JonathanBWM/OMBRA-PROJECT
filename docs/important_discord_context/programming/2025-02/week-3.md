# February 2025 - Week 3
# Channel: #programming
# Messages: 395

[2025-02-10 07:16] vendor: [replying to Xits: "Wouldnt that not work if I catch it after mov rax,..."]
you would just include those two instructions in your bad region

[2025-02-10 13:34] elias: Is Patchguard enabled when testsigning is on? <:peepoDetective:570300270089732096> People are telling me contradictory things

[2025-02-10 13:37] Timmy: haven't heard corelated things. I have heard that when you enable the debug bcd setting patchguard is disabled

[2025-02-10 13:37] elias: yeah debug is off

[2025-02-10 13:37] Timmy: but don't use me as a resource me <:pepega:521656574507220992>

[2025-02-10 13:38] zeropio: [replying to elias: "Is Patchguard enabled when testsigning is on? <:pe..."]
peeps at unknowncheats says that testsigning disable pg

[2025-02-10 13:39] zeropio: but why dont you try it yourself? write something to trigger pg

[2025-02-10 13:39] elias: [replying to zeropio: "peeps at unknowncheats says that testsigning disab..."]
I know, but someone else tells me PG is enabled

[2025-02-10 13:40] Timmy: [replying to zeropio: "but why dont you try it yourself? write something ..."]
isn't it like a really annoying thing to do because pg bsod triggers different from the detections

[2025-02-10 13:40] Timmy: idk what the time window is tho

[2025-02-10 13:41] dullard: Differs for different windows versions iirc

[2025-02-10 13:41] elias: yes

[2025-02-10 13:42] zeropio: [replying to Timmy: "idk what the time window is tho"]
it should take some time, but i think you can force it with int 0x20

[2025-02-10 13:42] zeropio: havent tried it myself tho

[2025-02-10 13:42] zeropio: so i can be wrong

[2025-02-10 13:48] Timmy: hmm KiSwInterrupt

[2025-02-10 13:48] Timmy: time to see what that's about

[2025-02-10 16:42] pinefin: matti can prob tell u about it

[2025-02-10 16:42] pinefin: i know he used it in efiguard

[2025-02-10 16:42] pinefin: cause i was wondering what it did

[2025-02-10 16:42] pinefin: https://github.com/Mattiwatti/EfiGuard/blob/5bfa808faa1890e5267536130637c4eb337ce5b3/EfiGuardDxe/PatchNtoskrnl.c#L49
[Embed: EfiGuard/EfiGuardDxe/PatchNtoskrnl.c at 5bfa808faa1890e526753613063...]
Disable PatchGuard and Driver Signature Enforcement at boot time - Mattiwatti/EfiGuard

[2025-02-10 17:53] daax: [replying to elias: "Is Patchguard enabled when testsigning is on? <:pe..."]
yes

[2025-02-10 17:54] daax: you can verify this by enabling test signing and hooking KeCancelTimer (within scope of PG). you will bugcheck within a few minutes. however, it’s disabled when you enable debug.

[2025-02-10 17:59] daax: [replying to Timmy: "isn't it like a really annoying thing to do becaus..."]
modifications to functions/pdata will generate pretty quick results.

[2025-02-11 13:27] x86matthew: show fopen call

[2025-02-11 13:28] x86matthew: my guess is you're not opening the file as binary

[2025-02-11 13:28] x86matthew: ie "r" not "rb"

[2025-02-11 13:28] x86matthew: what is the value of e_lfanew then?

[2025-02-11 13:29] x86matthew: both according to your code and the actual value

[2025-02-11 13:33] x86matthew: +0x3c is e_lfanew but please use a hex dump, not ida code dump lol

[2025-02-11 13:35] x86matthew: yes that looks correct, it matches the details in your ida screenshot

[2025-02-11 13:35] x86matthew: what is your code doing between fopen and your function? need more context

[2025-02-11 13:35] x86matthew: post it all ideally

[2025-02-11 13:45] x86matthew: the signature stuff is weird but it should work at least

[2025-02-11 13:45] x86matthew: how/where are you printing e_lfanew?

[2025-02-11 13:46] reversem3: parsing a file looking for a DOS sig?

[2025-02-11 13:47] x86matthew: print the value of `sizeof(IMAGE_DOS_HEADER)`

[2025-02-11 13:47] x86matthew: it sounds like you've implemented the struct yourself

[2025-02-11 13:47] x86matthew: your alignment might be off

[2025-02-11 13:48] x86matthew: should be 0x40 from memory

[2025-02-11 13:50] x86matthew: yeah that's it then

[2025-02-11 13:50] x86matthew: the struct needs to be packed correctly

[2025-02-11 14:06] x86matthew: it's not weird, that's to be expected

[2025-02-11 14:06] x86matthew: you need to explicitly pack the struct by 16-bits

[2025-02-11 14:24] x86matthew: the base nt headers are 4 byte packed

[2025-02-11 14:24] x86matthew: you shouldn't really be doing it like this though

[2025-02-11 14:24] x86matthew: why not just use the provided headers? it handles all of this for you

[2025-02-11 14:27] naci: use https://github.com/can1357/linux-pe

[2025-02-11 14:33] x86matthew: yeah, but 4 should work for dos headers too and also be correct for nt headers

[2025-02-11 14:57] x86matthew: you need to debug it

[2025-02-11 14:57] x86matthew: or start by printing some values as basic sanity checks

[2025-02-11 14:58] x86matthew: sizeof(IMAGE_DOS_HEADER) == 0x40

[2025-02-11 14:58] x86matthew: &DOS.e_lfanew - &DOS == 0x3C

[2025-02-11 15:00] x86matthew: but again if you're struggling with this you should use a premade library

[2025-02-11 15:26] Brit: [replying to naci: "use https://github.com/can1357/linux-pe"]
this is genuinely good, and would fit your needs

[2025-02-11 17:15] Deleted User: Decryption is working, but I can't get the boku popCalc payload to run apparently

All I'm doing is basically just a CreateFiber inject:
https://github.com/ustayready/tradecraft/blob/master/offensive-security/code-injection-process-injection/executing-shellcode-with-createfiber.md

It runs the calc just fine before the encryption-decryption
[Attachments: decrypt_calc.PNG, create_fiber.PNG]
[Embed: tradecraft/offensive-security/code-injection-process-injection/exec...]
Red Teaming Tradecraft. Contribute to ustayready/tradecraft development by creating an account on GitHub.

[2025-02-11 17:16] DIMM: Why not load the library using loadlibraryex

[2025-02-11 17:16] Deleted User: (didn't mean to interrupt, myb)

[2025-02-11 17:17] DIMM: If you use DONT_RESOLVE_DLL_REFERENCES you'll have a DLL ready to go (it's just that the IAT isn't fixed)

[2025-02-11 17:17] DIMM: All the sizes will work

[2025-02-11 17:17] DIMM: Ahh

[2025-02-11 17:18] DIMM: Hmm well I'm not experienced w linux so idk  then

[2025-02-11 17:18] DIMM: But why would u parse PE files on Linux?

[2025-02-11 17:20] DIMM: 🤔

[2025-02-11 17:22] DIMM: Okay well

[2025-02-11 17:22] DIMM: you can just pragma pack if the alignment is wrong couldn't you

[2025-02-11 17:33] DIMM: You could also try a char array

[2025-02-11 22:14] sync: [replying to Deleted User: "Decryption is working, but I can't get the boku po..."]
if it doesnt pop up then decryption is wrong ?

[2025-02-11 22:15] sync: cause if it works without decryption.. then you found your culprit

[2025-02-11 22:16] sync: i would try to check if the shellcode is actually 1:1 to the decrypted and checking whats diff

[2025-02-11 22:26] dlima: [replying to Deleted User: "Decryption is working, but I can't get the boku po..."]
What’s a boku popcalc anyways

[2025-02-11 22:26] dlima: Is that just the msfvenom calc payload written by that boku dude

[2025-02-11 22:27] dlima: Or rewritten i should say

[2025-02-11 23:45] Deleted User: for some reason I added an extra digit to the key, which is why it was fucking up
[Attachments: 500iq.PNG]

[2025-02-11 23:45] Deleted User: 500iq

[2025-02-11 23:46] Deleted User: I love missing the tiniest details that causes me complete havoc

[2025-02-11 23:46] x86matthew: [replying to Deleted User: "500iq"]
added an extra digit again?

[2025-02-11 23:46] Deleted User: no

[2025-02-11 23:46] x86matthew: lol

[2025-02-11 23:46] Deleted User: GOD no

[2025-02-11 23:47] rin: ```c
typedef struct _IMAGE_RESOURCE_DIRECTORY_ENTRY {
    union {
        struct {
            DWORD NameOffset:31;
            DWORD NameIsString:1;
        } DUMMYSTRUCTNAME;
        DWORD   Name;
        WORD    Id;
    } DUMMYUNIONNAME;
    union {
        DWORD   OffsetToData;
        struct {
            DWORD   OffsetToDirectory:31;
            DWORD   DataIsDirectory:1;
        } DUMMYSTRUCTNAME2;
    } DUMMYUNIONNAME2;
} IMAGE_RESOURCE_DIRECTORY_ENTRY, *PIMAGE_RESOURCE_DIRECTORY_ENTRY;
```

[2025-02-11 23:47] rin: not a c++ programmer but how does this struct work if the values in the union are different sizes?

[2025-02-11 23:48] rin: for context I am translating it to golang that's why I need to know the size

[2025-02-11 23:48] qw3rty01: it's the size of the largest field

[2025-02-12 01:24] rin: does anyone have a good blog on the resource section. I am struggling to find a good explanation

[2025-02-12 01:28] rin: more specifically how to parse it

[2025-02-12 01:36] Deleted User: not sure on parsing codewise, but does this help at all:
[Attachments: 20250211_193024.jpg, 20250211_193104.jpg, 20250211_193351.jpg]

[2025-02-12 01:46] Deleted User: (highly recommend this book btw for anyone who wants to learn the origins of Windows architecture)

[2025-02-12 01:46] rin: yeah I know how to parse the IMAGE_RESOURCE_DIRECTORY its just a struct but getting the actual resource from the first entry is where I am stuck at

[2025-02-12 01:51] Deleted User: from the looks of it, apparently you have to get IMAGE_RESOURCE_DATA_ENTRY through IMAGE_RESOURCE_DIRECTORY

[2025-02-12 01:51] rin: I know

[2025-02-12 01:52] Deleted User: I (clearly) haven't parsed it myself, so I can't exactly help you on the code part

[2025-02-12 01:53] rin: 
[Attachments: image.png]

[2025-02-12 01:53] rin: 
[Attachments: image.png]

[2025-02-12 01:55] rin: the lower 31 bits are suppose to be an offset to the next structure or something idk. Im looking at it in pe bear and I am retarded

[2025-02-12 01:55] Brit: if only we had pe parsing code on the internet

[2025-02-12 01:55] rin: [replying to Brit: "if only we had pe parsing code on the internet"]
true

[2025-02-12 01:56] f00d: [replying to rin: "does anyone have a good blog on the resource secti..."]
https://learn.microsoft.com/en-us/previous-versions/ms809762(v=msdn.10)#pe-file-resources
[Embed: Peering Inside the PE: A Tour of the Win32 Portable Executable File...]

[2025-02-12 01:58] f00d: [replying to rin: "the lower 31 bits are suppose to be an offset to t..."]
right here
[Attachments: opera_OGlQ8uwenc.png]

[2025-02-12 01:59] Deleted User: [replying to f00d: "right here"]
isn't that what I just showed-

[2025-02-12 01:59] f00d: idk i don't look at images of a book

[2025-02-12 02:00] f00d: ye ur right lmao

[2025-02-12 02:00] Deleted User: <:kekw:904522300257345566>

[2025-02-12 02:02] rin: kind of cancer you can't reply to your own messages anymore. but If you look at my first screenshot I parsed the IMAGE_RESOURCE_DIRECTORY_ENTRY(s)

[2025-02-12 02:02] rin: just after that which I think your first link explains

[2025-02-12 02:11] Deleted User: [replying to rin: "kind of cancer you can't reply to your own message..."]
wdym you cant reply to your own messages, since when

[2025-02-12 02:11] Deleted User: [replying to Deleted User: "wdym you cant reply to your own messages, since wh..."]
.

[2025-02-12 02:12] rin: lol true, didn't click the three dots

[2025-02-12 02:12] rin: <:mmmm:904523247205351454>

[2025-02-12 02:12] Deleted User: 😭

[2025-02-12 02:12] Deleted User: gg

[2025-02-12 02:23] f00d: [replying to rin: "kind of cancer you can't reply to your own message..."]
it's right there in the article, if `NameIsString` field is set then `NameOffset` is RVA to `IMAGE_RESOURCE_DIR_STRING_U` otherwise it's an `Id` field. same goes for `DataIsDirectory`, if set it's RVA to `IMAGE_RESOURCE_DIRECTORY` (subdir) otherwise it's RVA to `IMAGE_RESOURCE_DATA_ENTRY`.

[2025-02-12 02:23] f00d: note that RVA is relative to the start of the resources

[2025-02-12 12:30] Matti: well in general you need to add the base to an RVA to get the full VA, but it sounds like you're aware of what an RVA is

[2025-02-12 12:31] Matti: but an offset is not the same as an RVA, usually due to section alignment differences

[2025-02-12 12:33] Matti: so if you have an *offset*, you need to convert it to an RVA first to work with a mapped image
but, if you are *only* working with a file buffer (as in: a buffer returned from ReadFile) then you'll probably want to use offsets, not RVAs

[2025-02-12 12:34] Matti: in general IMO there aren't very many use cases for working with file buffers directly instead of section mappings, the latter is usually just more convenient

[2025-02-12 12:35] Matti: these don't have to be literal sections, you could use VirtualAlloc perfectly fine as well with more effort, the main point is to have RVAs that work like you'd expect them to

[2025-02-12 12:41] Matti: `let dir_size = size_of::<ImageLoadConfigDirectory>();`
this line is dangerous and probably going to crash most PE files that are not from ~2024 or later

[2025-02-12 12:41] Matti: you'lll want the size as given by the load config directory itself

[2025-02-12 12:41] Matti: not the size of the struct at the time of compilation

[2025-02-12 12:42] Matti: because the load config dir gets expanded with new fields often

[2025-02-12 12:42] Matti: there is another more insidious gotcha that you may or may not care about for what you're doing here

[2025-02-12 12:43] Matti: but, there are actually two sizes of the load config directory in a PE

[2025-02-12 12:43] Matti: the one I just mentioned, in the load config dir itself (if one exists), and the size of the directory as given by the PE headers

[2025-02-12 12:44] Matti: **these have to be the same value**

[2025-02-12 12:44] Matti: if they're not, windows will refuse to run the exe

[2025-02-12 12:44] Matti: I'm not kidding

[2025-02-12 12:45] Matti: this was a bug in UPX once that I spent quite some time hunting down

[2025-02-12 12:46] Matti: in general I'd check the size of the load config dir from the PE directories first, to make sure it is at least >= offset of whatever field you're interested in

[2025-02-12 12:46] Matti: if it's 0 or less than the size of the size member itself, you're done

[2025-02-12 12:47] Matti: if it's >= required size, you can proceed to compute the load config dir VA, read the size field, verify it's the same as the other size given, and then proceed to read the actual field

[2025-02-12 12:56] Matti: ```c
ULONG
RvaToOffset(
    _In_ PIMAGE_NT_HEADERS NtHeaders,
    _In_ ULONG Rva
    )
{
    PIMAGE_SECTION_HEADER SectionHeaders = IMAGE_FIRST_SECTION(NtHeaders);
    for (USHORT i = 0; i < NtHeaders->FileHeader.NumberOfSections; ++i)
    {
        if (Rva >= SectionHeaders->VirtualAddress &&
            Rva < SectionHeaders->VirtualAddress + SectionHeaders->SizeOfRawData)
        {
            return Rva - SectionHeaders->VirtualAddress +
                        SectionHeaders->PointerToRawData;
        }
        SectionHeaders++;
    }
    return 0;
}
```
btw, here is a convenience rva to offset conversion (not rust-qualified enough, so sorry about the C but you get the idea)

[2025-02-12 12:57] Matti: if you don't have `PIMAGE_NT_HEADERS ` either you can get them with `RtlImageNtHeader[Ex]`

[2025-02-12 12:57] Matti: from the base

[2025-02-12 16:27] sync: [replying to Deleted User: "for some reason I added an extra digit to the key,..."]
lol

[2025-02-12 16:27] sync: happens

[2025-02-12 16:27] sync: that’s crazy though

[2025-02-12 16:27] sync: 😭

[2025-02-12 16:27] sync: i mean me personally i’ve spent hours debugging why a function wasn’t returning anything

[2025-02-12 16:28] sync: then to figure out that i forgot to write the return statement

[2025-02-12 16:28] sync: my brain literally magically popped it in when i was reading the code, then compiler warnings popped up

[2025-02-12 16:37] Deleted User: this is the 3rd most embarrassing moment I've had code-wise
1st was that horrible WriteFile code I attempted back in 2023
2nd was forgetting how hex format looks a few days ago
..and now it's not knowing how to debug <:mmmm:904523247205351454>

[2025-02-12 16:40] avx: less talk more do then

[2025-02-12 16:41] Deleted User: real
I will probably take a break after pushing this project out to work on basics again

[2025-02-12 16:52] avx: [replying to avx: "less talk more do then"]
.

[2025-02-12 16:52] avx: as a wise man once said

[2025-02-12 17:13] sync: [replying to Deleted User: "this is the 3rd most embarrassing moment I've had ..."]
tbh you should just buy chatgpt

[2025-02-12 17:13] sync: it helps with basic stuff like this

[2025-02-12 17:13] Brit: this is not a good idea

[2025-02-12 17:13] sync: but double check what it says and don’t paste blindly cause it’s a tool not a all-in-one programmer

[2025-02-12 17:13] Brit: it just moves the problem deeper

[2025-02-12 17:14] sync: [replying to Brit: "it just moves the problem deeper"]
not if used correctly

[2025-02-12 17:14] Deleted User: [replying to sync: "tbh you should just buy chatgpt"]
I used it to make the hex print function, but I had to adjust it because it was fucking up a piece

[2025-02-12 17:14] Brit: llm usage rots your brain

[2025-02-12 17:14] Deleted User: [replying to Brit: "llm usage rots your brain"]
but also this

[2025-02-12 17:14] sync: [replying to Brit: "llm usage rots your brain"]
if you rely on it yeh

[2025-02-12 17:14] Brit: that's literally what you told him to do

[2025-02-12 17:15] sync: no, i said to double check what it says and not paste blindly

[2025-02-12 17:15] sync: if you solely rely on it ur cooked

[2025-02-12 17:15] Deleted User: I'm trying to break out of the habit of using it at all tbh

[2025-02-12 17:15] Deleted User: it's fucking up my critical thinking

[2025-02-12 17:15] sync: i mean if you don’t have any of the basics

[2025-02-12 17:16] sync: then yeah don’t use it

[2025-02-12 17:16] Deleted User: yes

[2025-02-12 17:16] Deleted User: [replying to sync: "i mean if you don’t have any of the basics"]
this

[2025-02-12 17:16] sync: like

[2025-02-12 17:16] sync: i usually use it to write boilerplate code

[2025-02-12 17:16] sync: but it’s not like i make it think on itself

[2025-02-12 17:16] sync: shit if you make it think alone it’s gg

[2025-02-12 17:16] sync: you’re gonna get C— not c++

[2025-02-12 17:17] Deleted User: no like deadass
I couldn't make a function that adds forward slashes to my hexcode without it

[2025-02-12 17:18] Deleted User: that's..not good

[2025-02-12 17:19] Deleted User: I literally have no excuses, I just have to practice on my own

[2025-02-12 17:19] sync: yeh

[2025-02-12 17:19] sync: i was lucky to start out c++  before LLMs existed

[2025-02-12 17:20] 5pider: I learned what pointers are thanks to chatgpt

[2025-02-12 17:21] Deleted User: it is **very** good at explaining things

[2025-02-12 17:22] Deleted User: I've just been relying on it too much for the actual code
...and pasting, as we have seen

[2025-02-12 17:33] dlima: [replying to 5pider: "I learned what pointers are thanks to chatgpt"]
Same lmao

[2025-02-12 17:33] dlima: And a bunch of other shit

[2025-02-12 17:33] dlima: I started programming when chatgpt first started popping

[2025-02-12 17:33] dlima: Learned a bunch of shit that way

[2025-02-12 17:34] N: Just rely on your instincts when you code.

[2025-02-12 17:34] N: That’s it

[2025-02-12 17:34] dlima: True

[2025-02-12 17:34] Deleted User: my first lang was Javascript in 2018, then I did Python 2020-2022
then I started C in 2023, when chatGPT came out

the rest is history

[2025-02-12 17:35] dlima: Damn

[2025-02-12 17:35] Brit: [replying to 5pider: "I learned what pointers are thanks to chatgpt"]
I refuse to believe this wasn't in jest

[2025-02-12 17:35] Brit: that's insanity

[2025-02-12 17:35] dlima: I think hes joking but im not

[2025-02-12 17:35] Brit: sad

[2025-02-12 17:35] Brit: very sad

[2025-02-12 17:35] dlima: LLMs are very good at explaining stuff

[2025-02-12 17:35] dlima: [replying to Brit: "very sad"]
How

[2025-02-12 17:35] dlima: They’re good as a learning tool

[2025-02-12 17:35] Brit: I don't think I agree

[2025-02-12 17:35] dlima: Just don’t use them exclusively

[2025-02-12 17:35] Brit: I think they stifle creativity and critical thought

[2025-02-12 17:36] dlima: That’s why I don’t use them to actually write code

[2025-02-12 17:36] Brit: that's not even the point

[2025-02-12 17:36] dlima: I just ask it questions about concepts I don’t understand

[2025-02-12 17:37] dlima: Plus programming with llms is boring asf

[2025-02-12 17:37] dlima: I’d rather do it myself it’s more fun that way

[2025-02-12 17:38] Brit: the issue is that it'll just hallucinate or give you information about a thing without any indication that the confidence in that being a correct approach isn't actually all that high

[2025-02-12 17:38] Brit: it's great for trivial things

[2025-02-12 17:38] Brit: but that's about it

[2025-02-12 17:39] dlima: [replying to Brit: "the issue is that it'll just hallucinate or give y..."]
Yeah that happens for topics with little to no information available

[2025-02-12 17:39] dlima: Ur right

[2025-02-12 17:39] dlima: Gotta be careful

[2025-02-12 17:41] x86matthew: [replying to Brit: "I think they stifle creativity and critical though..."]
when you see `Credits: ChatGPT` in a project you know it's gonna be shit

[2025-02-12 17:42] Brit: that's another thing

[2025-02-12 17:42] Brit: it's an indicator of slop

[2025-02-12 17:42] Brit: and that's when they admit to having used chatgpt

[2025-02-12 17:43] Brit: but you'll see some code or just plain english in some projects / code / papers and you can immediately tell that someone slapped some prompt into some llm

[2025-02-12 17:43] 5pider: [replying to Brit: "I refuse to believe this wasn't in jest"]
LMFAOOO

[2025-02-12 17:44] 5pider: no i am just fucking around

[2025-02-12 17:44] Brit: "we delve" "Comprehensive and intricate"

[2025-02-12 17:44] Brit: <:nomore:927764940276772925>

[2025-02-12 17:44] Brit: anyway, learn things for yourself, don't nuke your brain with llm overuse

[2025-02-12 17:45] Brit: can't make a mental model of something if you've outsourced knowledge of the thing to some black box you pay 20 bucks for a month

[2025-02-12 17:45] 5pider: LLM tend to halucinate on certain topics. the most it was useful to me was when i was working with Qt but also then it sometimes makes shit up so i am going back to good old docs with grep.app searching

[2025-02-12 17:45] Brit: learn to use rg instead of gpt honestly

[2025-02-12 17:46] dlima: 😢

[2025-02-12 17:46] dlima: Guys am I an llmtard

[2025-02-12 17:46] 5pider: [replying to Brit: "learn to use rg instead of gpt honestly"]
what is rg ?

[2025-02-12 17:46] Brit: ripgrep

[2025-02-12 17:47] Brit: grep but fast instead of slow

[2025-02-12 17:47] dlima: [replying to 5pider: "what is rg ?"]
Oldhead tool

[2025-02-12 17:47] 5pider: [replying to Brit: "grep but fast instead of slow"]
no i meant i use https://grep.app/
[Embed: Code Search | Grep by Vercel]
Search for code, files, and paths across half a million public GitHub repositories.

[2025-02-12 17:47] Brit: no I know

[2025-02-12 17:47] 5pider: i mostly use it to see APIs in code as examples

[2025-02-12 17:47] 5pider: ah

[2025-02-12 17:55] dlima: I need to switch off jetbrains

[2025-02-12 17:55] dlima: Im so tired of this slow piece of shit

[2025-02-12 17:55] dlima: I just need to find something with good refactoring and im off this

[2025-02-12 18:02] Deleted User: [replying to Brit: "anyway, learn things for yourself, don't nuke your..."]
noted

[2025-02-12 18:08] Deleted User: [replying to Brit: "but you'll see some code or just plain english in ..."]
reminds me of this:
https://github.com/SleepTheGod/Windows-Atom-Table-Hijacking/blob/main/exploit.c
[Embed: Windows-Atom-Table-Hijacking/exploit.c at main · SleepTheGod/Window...]
A privilege escalation vulnerability exists in Windows due to a flaw in the implementation of the Atom Table. An attacker could exploit this vulnerability by injecting malicious code into the Atom ...

[2025-02-12 18:16] avx: [replying to Brit: ""we delve" "Comprehensive and intricate""]
we delve into the comprehensive and intricate world of pointers

[2025-02-12 18:18] Deleted User: "in this emerging digital age"

[2025-02-12 18:19] BWA RBX: [replying to Deleted User: "reminds me of this:
https://github.com/SleepTheGod..."]
Hate this drug addict

[2025-02-12 18:20] Deleted User: [replying to BWA RBX: "Hate this drug addict"]
we found some..very odd prompts from him some months ago

[2025-02-12 18:21] Deleted User: yeah <:yea:904521533727342632>

[2025-02-12 18:21] mishap: [replying to Deleted User: "reminds me of this:
https://github.com/SleepTheGod..."]
wow it just magically crosses address spaces <:topkek:904522829616263178>

[2025-02-12 18:22] Deleted User: [replying to mishap: "wow it just magically crosses address spaces <:top..."]
I had a stroke trying to figure out the offset shit he was attempting for atom tables before we found it was ChatGPT

[2025-02-12 18:22] x86matthew: [replying to mishap: "wow it just magically crosses address spaces <:top..."]
crosses OS boundaries too! windows to linux!

[2025-02-12 18:24] Deleted User: you don't

[2025-02-12 18:24] contificate: misunderstood genius

[2025-02-12 18:24] Brit: this could be you

[2025-02-12 18:24] BWA RBX: [replying to contificate: "misunderstood genius"]
https://tenor.com/view/good-will-hunting-gif-27068727

[2025-02-12 18:24] Brit: you only need to get a crack habbit

[2025-02-12 18:25] Deleted User: hard pass

[2025-02-12 18:25] Deleted User: my point remains

[2025-02-12 18:26] contificate: ```c
    if (!InjectShellCode(pid, moduleName, functionSymbol)) {
        return 1;
    }

    return 0;
```
idiot!!! just fucking
```
return InjectShellCode(pid, moduleName, functionSymbol);
```
💯 lmaooo learn to code!!!1 flex on his ass!!1 lmaooo

[2025-02-12 18:26] contificate: lmaoooo

[2025-02-12 18:26] contificate: can this ugy _even_ code??

[2025-02-12 18:26] 5pider: [replying to contificate: "```c
    if (!InjectShellCode(pid, moduleName, fun..."]
you dont understand. this is pure genius code

[2025-02-12 18:26] Deleted User: [replying to contificate: "can this ugy _even_ code??"]
..it was chatGPT

[2025-02-12 18:26] contificate: can this guy EVEN code?

[2025-02-12 18:26] Deleted User: [replying to Deleted User: "..it was chatGPT"]
^

[2025-02-12 18:27] mishap: [replying to contificate: "```c
    if (!InjectShellCode(pid, moduleName, fun..."]
this would return false on success sir

[2025-02-12 18:27] contificate: close enough!

[2025-02-12 18:27] contificate: invert it!

[2025-02-12 18:27] mishap: ```c
return !InjectShellCode(pid, moduleName, functionSymbol);
```
you just can't handle the genius

[2025-02-12 18:27] contificate: tbh

[2025-02-12 18:27] contificate: `CreateRemoteThread` is like

[2025-02-12 18:27] contificate: a malware author's dream

[2025-02-12 18:28] contificate: not been given a satisfying reason why this is so trivial to do

[2025-02-12 18:28] mishap: not really, it's trivial for anything to catch it

[2025-02-12 18:28] contificate: sure but it's still a dream

[2025-02-12 18:28] contificate: 12yr olds can write shitware

[2025-02-12 18:28] contificate: that does things on Windows

[2025-02-12 18:28] mishap: a dream for detection engineers finding new samples to signature

[2025-02-12 18:29] mishap: "Oh look, another one fell for the bait"

[2025-02-12 18:29] x86matthew: [replying to contificate: "`CreateRemoteThread` is like"]
you joke but this guy literally discovered CreateRemoteThread

[2025-02-12 18:29] x86matthew: https://clumsylulz.medium.com/windows-dll-injection-0day-1d99a9fa7023
[Embed: Windows DLL Injection 0day]

[2025-02-12 18:29] x86matthew: same guys blog

[2025-02-12 18:29] mishap: [replying to x86matthew: "https://clumsylulz.medium.com/windows-dll-injectio..."]
ok, I've seen enough to wipe this person from my memory and mind

[2025-02-12 18:30] elias: [replying to x86matthew: "https://clumsylulz.medium.com/windows-dll-injectio..."]
<:yea:904521533727342632>

[2025-02-12 18:30] elias: I wonder what hes trying to achieve with this

[2025-02-12 18:30] Deleted User: this is still nightmare-inducing
[Attachments: 20250212_122954.jpg]

[2025-02-12 18:31] Deleted User: bro was SPECIFIC specific

[2025-02-12 18:35] rin: [replying to x86matthew: "https://clumsylulz.medium.com/windows-dll-injectio..."]
this guy literally does meth

[2025-02-12 18:35] rin: or used to

[2025-02-12 18:36] avx: [replying to x86matthew: "https://clumsylulz.medium.com/windows-dll-injectio..."]

[Attachments: when-youre-tired-of-that-one-person-in-the-office-who-is-v0-44rbkwfjt70d1.png]

[2025-02-12 18:36] Deleted User: HAHAHAHAAAHAA

[2025-02-12 18:45] broeder: ahahahaaaa https://clumsylulz.medium.com/working-discord-remote-code-execution-c08e447bbeb6
[Embed: WORKING DISCORD REMOTE CODE EXECUTION]
Working windows discord rce https://sleepthegod.github.io/discord_rce.html

[2025-02-12 18:45] broeder: 
[Attachments: image.png]

[2025-02-12 18:49] Timmy: [replying to Deleted User: "this is still nightmare-inducing"]
<:OMEGALUL:662670462215782440> did the llm not work that day?

[2025-02-12 18:50] Loading: why would u post something like this on medium lol

[2025-02-12 19:27] naci: [replying to Deleted User: "reminds me of this:
https://github.com/SleepTheGod..."]
i remember someone was trying to execute this code recently

[2025-02-12 20:11] UJ: bro is temu can1357.

[2025-02-12 20:15] Brit: offensive to can

[2025-02-12 20:15] Brit: honestly

[2025-02-12 20:17] naci: turkish federation of devirtualizing vmprotect

[2025-02-12 20:34] sunbather: Is there more lore on this guy? I am impressed

[2025-02-12 20:57] dullard: SleepTheGod ? Or can 😂

[2025-02-12 20:57] dullard: Can is turkish smart man

[2025-02-12 20:57] dullard: SleepTheGod is a tweaker retard 😂

[2025-02-12 21:00] dlima: [replying to Deleted User: "reminds me of this:
https://github.com/SleepTheGod..."]
Why is that his profile pic

[2025-02-12 21:01] dlima: Negative aura

[2025-02-12 21:01] dlima: Bro looks like those 35 year olds that still use snapchat to seem cool to the youngins

[2025-02-12 21:09] rin: [replying to dlima: "Bro looks like those 35 year olds that still use s..."]
Surprisingly close

[2025-02-12 22:29] Deleted User: [replying to dlima: "Bro looks like those 35 year olds that still use s..."]
linkedin fraud

[2025-02-12 22:30] Deleted User: making bs up to get hired or something idk i hope he doesnt believe whatever he posts

[2025-02-12 22:32] Deleted User: [replying to Timmy: "<:OMEGALUL:662670462215782440> did the llm not wor..."]
don't ask me
it's **that** mf's prompt

[2025-02-12 22:43] sunbather: [replying to dullard: "SleepTheGod ? Or can 😂"]
SleepTheGod. I feel like he must have really good schizo ramblings. The type of stuff you read three times and still say "what the fuck is he trying to say"

[2025-02-12 22:46] contificate: probs a legend

[2025-02-12 22:53] Deleted User: [replying to contificate: "probs a legend"]
and on an FBI watchlist

[2025-02-12 22:54] Brit: why

[2025-02-12 22:54] Brit: the man is literally posting crackhead hallucinations

[2025-02-12 22:54] Brit: there's nothing there

[2025-02-12 22:54] Deleted User: that prompt screams groomer <:mmmm:904523247205351454>

[2025-02-12 22:55] Brit: ah

[2025-02-12 22:55] Brit: yee

[2025-02-12 22:55] Brit: probably

[2025-02-12 22:56] sunbather: [replying to Brit: "the man is literally posting crackhead hallucinati..."]
I find those amusing. It's a small challenge to try to decipher completely insane ramblings.

[2025-02-12 22:56] sunbather: Also I've never seen crackhead hallucinations embodied in code. It's kinda fun

[2025-02-12 22:59] naci: yall just hate him, this is peak programming, i cant wait for delving into world of the complex ai 0day generating ! #2025

[2025-02-12 23:04] Deleted User: can't wait to spend all my time learning how to do Exploit Development AI-free just to have chadGPT do it in 5 seconds by December

[2025-02-12 23:34] dlima: [replying to sunbather: "SleepTheGod. I feel like he must have really good ..."]
I need more lore on this dude

[2025-02-12 23:35] dlima: First time i’m hearing about him

[2025-02-12 23:54] Deleted User: I don't think I _want_ to know more about this guy 😭😭😭

[2025-02-12 23:55] Deleted User: I've seen enough <:kekw:904522300257345566>

[2025-02-13 00:07] dlima: [replying to Deleted User: "I don't think I _want_ to know more about this guy..."]
Nah i need more info

[2025-02-13 00:07] dlima: New lolcow just dropped

[2025-02-13 00:07] dlima: Infosec version of lowtiergod

[2025-02-13 00:08] Deleted User: [replying to dlima: "Nah i need more info"]
bro wants a whole-ass Turkey Tom video on this mf

[2025-02-13 00:13] dlima: [replying to Deleted User: "bro wants a whole-ass Turkey Tom video on this mf"]
I do

[2025-02-13 01:14] emma: *removes the polymorphic*

[2025-02-13 01:27] dlima: [replying to emma: "*removes the polymorphic*"]
🤣🤣

[2025-02-13 01:27] dlima: If you know you know

[2025-02-13 01:27] dlima: That’s the ultimate infosec lolcow actually

[2025-02-13 02:33] dlima: Has anyone here written a hobby os kernel before?

[2025-02-13 02:33] dlima: I’m looking for good resources

[2025-02-13 02:34] dlima: Preferably for x86_64

[2025-02-13 02:35] Deleted User: I was gonna ask protected or real mode, but

[2025-02-13 02:39] dlima: [replying to Deleted User: "I was gonna ask protected or real mode, but"]
Well there’s a lot more resources for that

[2025-02-13 02:39] Deleted User: yeah

[2025-02-13 02:39] dlima: Like for writing a 32 bit kernel

[2025-02-13 02:40] dlima: So far I’ve managed to identity map my kernel and enter long mode (i’m using GRUB so I started in protected mode initially)

[2025-02-13 02:41] dlima: But I’m not too sure on where to go from here

[2025-02-13 02:41] dlima: Never written a kernel before evidently

[2025-02-13 02:42] Deleted User: why x64 again

[2025-02-13 02:45] dlima: [replying to Deleted User: "why x64 again"]
Wdym

[2025-02-13 02:45] dlima: Like why do I want to write a 64 bit one as opposed to 32?

[2025-02-13 02:46] Deleted User: yeah (I'm just curious)

[2025-02-13 02:46] dlima: [replying to Deleted User: "yeah (I'm just curious)"]
Idk cuz fuck it lol

[2025-02-13 02:46] dlima: Why not

[2025-02-13 02:47] Deleted User: LMAO, that's basically how I made my 1st ASM project

[2025-02-13 02:47] dlima: We’re all crazy people doing things that have no purpose might as well make it a bit more interesting

[2025-02-13 02:47] Deleted User: osdev has this

[2025-02-13 02:47] Deleted User: https://wiki.osdev.org/Creating_a_64-bit_kernel
[Embed: Creating a 64-bit kernel]

[2025-02-13 02:48] dlima: [replying to Deleted User: "https://wiki.osdev.org/Creating_a_64-bit_kernel"]

[Attachments: IMG_9071.png]

[2025-02-13 02:48] dlima: ☹️

[2025-02-13 02:48] Deleted User: ah

[2025-02-13 02:48] Deleted User: F

[2025-02-13 02:49] dlima: Don’t know how relevant it is based on that disclaimer BUT I will still take a look when I get home

[2025-02-13 02:51] N: [replying to dlima: "Don’t know how relevant it is based on that discla..."]
go home quick

[2025-02-13 02:52] Deleted User: if I had to guess, you'd basically just be replicating this with 64-bit registers
https://github.com/repnz/simple-os/tree/master/OperatingSystem/source
[Embed: simple-os/OperatingSystem/source at master · repnz/simple-os]
Simple Protected Mode Kernel for i386. Contribute to repnz/simple-os development by creating an account on GitHub.

[2025-02-13 03:14] daax: [replying to dlima: "Has anyone here written a hobby os kernel before?"]
https://littleosbook.github.io

[2025-02-13 03:14] daax: https://os.phil-opp.com/
[Embed: Writing an OS in Rust]
This blog series creates a small operating system in the Rust programming language. Each post is a small tutorial and includes all needed code.

[2025-02-13 03:15] daax: <https://github.com/cfenollosa/os-tutorial>

[2025-02-13 03:15] daax: [replying to dlima: "So far I’ve managed to identity map my kernel and ..."]
Should be sufficient to get you started.

[2025-02-13 03:16] dlima: [replying to daax: "https://littleosbook.github.io"]
Thank you very much! 🙏

[2025-02-13 04:07] Deleted User: [replying to daax: "https://os.phil-opp.com/"]
rust mentioned.,

[2025-02-13 04:08] Deleted User: that website is awesome

[2025-02-13 07:27] avx: [replying to Deleted User: "if I had to guess, you'd basically just be replica..."]
stop making guesses ngl

[2025-02-13 07:29] Deleted User: stop saying ngl that implies u do lie on a regular basis!

[2025-02-13 07:30] Deleted User: <:33:1333236172738007173>

[2025-02-13 07:30] avx: I do

[2025-02-13 08:14] sync: [replying to Deleted User: "stop saying ngl that implies u do lie on a regular..."]
he tells white lies to make people feel good!

[2025-02-13 08:18] .Ξ 𐌔 C Ó: hello, new here. was wondering if anyone has any interest in an EFT project. I have source for an outdated C# project and am looking to learn some things.

[2025-02-13 08:19] Deleted User: wrong server

[2025-02-13 08:19] .Ξ 𐌔 C Ó: https://tenor.com/view/umm-um-ummm-ummok-puppet-gif-17038634

[2025-02-13 12:19] Deleted User: [replying to avx: "stop making guesses ngl"]
you right, ig

[2025-02-13 12:25] avx: I'm bakki

[2025-02-13 12:25] avx: guess again

[2025-02-13 23:36] idalen: anything to add to this list?
[Attachments: image.png]

[2025-02-13 23:38] contificate: I prefer the metric of GB/s for caches

[2025-02-13 23:38] contificate: you see that L1 is in the order of like 200+ GB/s

[2025-02-13 23:38] contificate: but very small

[2025-02-13 23:38] contificate: this is advertised in memtest, for example

[2025-02-13 23:39] contificate: 
[Attachments: 2025-02-13-233843_527x260_scrot.png]

[2025-02-13 23:39] contificate: nobody knows what a nanosecond is

[2025-02-13 23:39] contificate: people have a feel for GB/s

[2025-02-14 00:39] Matti: GB/s is easier to compare than 10 different types of fractions of a second (or worse, using ns for everything just to show that there's orders of magnitude of changes involved.... because that's such a difficult concept to understand otherwise or something)

[2025-02-14 00:40] Matti: but, any type of memory is always making a tradeoff between bandwidth and latency

[2025-02-14 00:41] Matti: some of the things listed definitely should be expressed in time units IMO, basically anything above 1 ms

[2025-02-14 00:43] Matti: sometimes latency is the most important thing because at some point it becomes noticeable on human timescales

[2025-02-14 00:43] Matti: which makes anything with high latency fucking horrible to use

[2025-02-14 00:45] Matti: there are also uses cases where the inverse is true ofc and where bandwidth >>> latency

[2025-02-14 00:46] Matti: but I think the list is going for the "how noticeable is this operation" factor, which I think is a fine approach

[2025-02-14 05:12] Torph: [replying to idalen: "anything to add to this list?"]
maybe time for some math operations like add/sub vs mul/div compared to their SIMD variants

[2025-02-16 06:36] WEMUSTDIG: [replying to rin: "this guy literally does meth"]
dont we all