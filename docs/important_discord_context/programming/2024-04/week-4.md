# April 2024 - Week 4
# Channel: #programming
# Messages: 332

[2024-04-22 03:57] Rubite Devs: does any of you have project maybe i want to be part of the project  of your ideaas

[2024-04-22 04:01] rin: [replying to Rubite Devs: "does any of you have project maybe i want to be pa..."]
hello fed

[2024-04-22 04:02] rin: does anyone have any experience with ChildWindowFromPoint or RealChildWindowFromPoint? they only ever return the parent window handle for me

[2024-04-22 04:56] rin: ```golang
uintptr(point.X)|uintptr(point.Y)<<32
```

[2024-04-22 04:57] rin: this solved the problem, it was just the wrong point

[2024-04-23 15:16] 0x208D9: "Locks the specified region of the process's virtual address space into physical memory, ensuring that subsequent access to the region will not incur a page fault."

explain in layman terms if anyone can

[2024-04-23 15:16] 0x208D9: context : https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtuallock
[Embed: VirtualLock function (memoryapi.h) - Win32 apps]
Locks the specified region of the process's virtual address space into physical memory, ensuring that subsequent access to the region will not incur a page fault.

[2024-04-23 15:23] Brit: It makes sure that the page you specified remains in the working set

[2024-04-23 15:23] Brit: basically a guarantee that the memory doesn't get paged out so long as there's a thread executing that process

[2024-04-23 15:24] Brit: MSDN being particularly backwards with that description because it doesn't actually guarantee anything about physical memory

[2024-04-23 15:24] Brit: And you can think of the working set as the collection of pages that the memory manager keeps live for said program

[2024-04-23 15:24] Brit: because pagefaults are slow

[2024-04-23 15:24] Brit: and if you can avoid them that's good

[2024-04-23 15:30] 25d6cfba-b039-4274-8472-2d2527cb: even more layman: the devious microsoft engineer puts memory content on disk to make it slow and make you buy a new pc

[2024-04-23 15:31] Brit: yes

[2024-04-23 15:31] Brit: paging is a conspiracy between MS and hard drive manafacturers

[2024-04-23 15:31] Brit: to kill drives and make your pc slow

[2024-04-23 15:34] 25d6cfba-b039-4274-8472-2d2527cb: but on the other hand if you have a fast enough drive, swapping might just be enough to convince an apple user that they weren't a bellend for getting a laptop with 8GB of ram. works both ways

[2024-04-23 15:34] 25d6cfba-b039-4274-8472-2d2527cb: 🤪

[2024-04-23 15:35] Brit: matti got it right

[2024-04-23 15:35] Brit: eventually we'll have storage with access time close enough to ram :^)

[2024-04-23 15:35] Brit: optane brained

[2024-04-23 16:03] Matti: speaking of

[2024-04-23 16:03] Matti: https://tweakers.net/aanbod/3615060/intel-optane-dc-p5800x-800gb.html
[Embed: Intel Optane DC P5800X 800GB]

[2024-04-23 16:04] Matti: I already shilled 2 to someone, so 8 remaining

[2024-04-23 16:06] Deleted User: Matti since u hate intel, do I burn this
[Attachments: IMG_0090.jpg]

[2024-04-23 16:07] Matti: I don't hate intel full stop

[2024-04-23 16:07] Matti: only their """CPU""" division really

[2024-04-23 16:07] Matti: however

[2024-04-23 16:07] Matti: that is a subpar SSD abusing the optane brand name

[2024-04-23 16:07] luci4: [replying to Matti: "only their """CPU""" division really"]
how come?

[2024-04-23 16:08] Matti: it's not the worst, but it's like 50 MB for an unreasonable amount of money

[2024-04-23 16:08] Matti: [replying to luci4: "how come?"]
monopoly abuse, anticompetitive practices

[2024-04-23 16:08] Deleted User: [replying to Matti: "that is a subpar SSD abusing the optane brand name"]
this stopped working some time ago for some reason lol

[2024-04-23 16:09] Deleted User: damn that other intel optane it's very expensive

[2024-04-23 16:09] Brit: but it fast doe

[2024-04-23 16:09] Matti: lol you mean the ad?

[2024-04-23 16:09] Matti: I paid 2.2K eur for the same drive when it was new

[2024-04-23 16:10] Matti: this is the cheapest I've seen these, by far

[2024-04-23 16:11] Deleted User: fuckin hell

[2024-04-23 16:11] Matti: oddly enough you can still buy P4800X's (from 2018) new, but almost never P5800X's

[2024-04-23 16:11] Matti: not that it matters, they're not slower second hand and they can't wear out

[2024-04-23 16:12] Matti: no, why

[2024-04-23 16:12] Deleted User: It got resend lol

[2024-04-23 16:12] Deleted User: [replying to Matti: "I paid 2.2K eur for the same drive when it was new"]
now it’s 1.9k euros

[2024-04-23 16:12] Deleted User: hasn’t changed much lol

[2024-04-23 16:12] Deleted User: apparently

[2024-04-23 16:13] Matti: yeah, I've also seen it for 2.5K and 2.7K

[2024-04-23 16:13] Matti: volatile like bitcoin

[2024-04-23 16:13] Matti: but, one day the masses will realise what they are missing out on

[2024-04-23 16:13] Matti: and then the price will soar

[2024-04-23 16:14] Brit: matti about to flip hi drives for billions

[2024-04-23 16:14] Brit: to the moon

[2024-04-23 16:14] Brit: to be fair they are nice drives

[2024-04-23 16:15] Matti: 
[Attachments: image.png]

[2024-04-23 16:15] Brit: dram is the ethot of storage?

[2024-04-23 16:16] Matti: yeah idk I guess they didn't want to overcomplicate the top layer

[2024-04-23 16:16] Matti: but DRAM is also volatile

[2024-04-23 16:17] Brit: the analogy stands

[2024-04-23 16:17] Matti: unlike Intel(R) Optane(TM) PMEM(TM) DIMMs
[Attachments: image.png]

[2024-04-23 16:17] Matti: also named "unoptanables"

[2024-04-23 16:19] Deleted User: LMAO

[2024-04-23 16:22] Matti: https://www.graidtech.com/product/sr-1001/ getting one of these once they come out, assuming the latency penalty isn't too bad with optanes
[Embed: SupremeRAID™ SR-1001 - Graid Technology]
SupremeRAID™ SR-1001 is a PCIe Gen 3 card that supports up to 8 SSDs, and delivers superior performance and flexibility for tower and edge servers, professional workstations, and gaming desktops.

[2024-04-23 16:22] Matti: (TLDR: it's like HW RAID but using a GPU for scheduling)

[2024-04-23 17:22] Azrael: [replying to Matti: "unlike Intel(R) Optane(TM) PMEM(TM) DIMMs"]
Shit, is this NVDIMM-P?

[2024-04-23 17:38] Matti: who knows what it is

[2024-04-23 17:38] Matti: no one can afford them

[2024-04-23 17:39] Matti: also, only C741 chipset server boards support them

[2024-04-23 17:40] Matti: cause they require special UEFI OPROM support

[2024-04-23 17:40] Matti: 
[Attachments: IntelOptanePMemDriver.efi, IntelOptanePMemHii.efi]

[2024-04-23 17:41] Matti: ^ there, saved you the C741 board

[2024-04-23 17:41] Matti: now you just need the $25K for an optane PMEM DIMM

[2024-04-23 17:41] Azrael: Which I obviously have.

[2024-04-23 17:41] Matti: same

[2024-04-23 17:41] Matti: same

[2024-04-23 17:43] Azrael: [replying to Matti: "also, only C741 chipset server boards support them"]
Ouch.

[2024-04-23 17:44] Azrael: [replying to Matti: ""]
I would expect SRAM to be their final layer.

[2024-04-23 17:44] Matti: [replying to Azrael: "I would expect SRAM to be their final layer."]
same

[2024-04-23 17:44] Azrael: I guess they wanted Optane more than anything.

[2024-04-23 17:44] Matti: [replying to Azrael: "Ouch."]
it's not so bad

[2024-04-23 17:44] Matti: you can just flash those two drivers on your own board

[2024-04-23 17:45] Azrael: Oh yeah sure, and then pull out my $25,000, which I have.

[2024-04-23 17:45] Matti: precisely

[2024-04-23 17:58] Deleted User: [replying to Matti: ""]
wtf

[2024-04-23 18:01] Matti: you can just extract these from any C741 BIOS <:thinknow:475800595110821888>

[2024-04-23 18:01] Azrael: Isn't it just hosted on an EEPROM/ROM chip?

[2024-04-23 18:02] Matti: well, that too

[2024-04-23 18:02] Matti: the board's

[2024-04-23 18:02] Azrael: EEPROM isn't necessary for that, but you never know.

[2024-04-23 18:02] Matti: but assuming you haven't got one, it's easier to just download the BIOS for one

[2024-04-23 18:03] Matti: and no it's not stored on any EEPROM on the unoptanables if that's what you meant

[2024-04-23 18:03] Matti: don't ask me why

[2024-04-23 18:03] Azrael: Production costs.

[2024-04-23 18:03] Matti: the regular optane SSDs do have OPROMs on them

[2024-04-23 18:03] Matti: 
[Attachments: Intel_P4800X.rom, Intel_P5800X.rom]

[2024-04-23 18:04] Azrael: OPROM is just ROM with special firmware for BIOS services and such, isn't it?

[2024-04-23 18:04] Matti: an OPROM doesn't technically need to be in ROM necessarily (as far as I know anyway) - though usually of course it is

[2024-04-23 18:05] Matti: think of it as a sort of driver for the BIOS

[2024-04-23 18:05] Matti: or in this case UEFI

[2024-04-23 18:05] Matti: PCIe expansion cards often contain OPROMs too

[2024-04-23 18:05] Matti: e.g. GPUs, NICs

[2024-04-23 18:05] Azrael: [replying to Matti: "think of it as a sort of driver for the BIOS"]
Yeah, initialization service/driver.

[2024-04-23 18:05] Matti: yes

[2024-04-23 18:05] Azrael: [replying to Matti: "e.g. GPUs, NICs"]
Basically anything PCI & extended.

[2024-04-23 18:07] Azrael: [replying to Matti: "the regular optane SSDs do have OPROMs on them"]
Cool stuff.

[2024-04-23 18:08] Matti: the oldest one I've got is for the intel 750

[2024-04-23 18:08] Matti: 
[Attachments: Intel_750_SSDPEDMW400.rom]

[2024-04-23 18:08] Matti: and then there's one for the samsung 950 pro, which is very interesting IMO because it's the only OPROM that comes in both legacy (BIOS) and UEFI versions

[2024-04-23 18:08] Matti: 
[Attachments: Samsung_950_Pro.rom]

[2024-04-23 18:09] Matti: meaning you can boot off this NVMe drive on a legacy BIOS system

[2024-04-23 18:09] Matti: which made it very unusual

[2024-04-23 18:09] Azrael: [replying to Matti: "and then there's one for the samsung 950 pro, whic..."]
That's odd.

[2024-04-23 18:10] Matti: the only NVMe OPROM* I should say

[2024-04-23 18:10] Matti: lots of GPU OPROMs come in both types

[2024-04-23 18:10] Matti: as well as NICs

[2024-04-23 18:11] Matti: but for NVMe it used to kind of be the case that you needed a UEFI OS if you wanted to boot from the drive

[2024-04-23 18:11] Matti: I still own a 950 pro because of this

[2024-04-23 18:12] Matti: I don't need it anymore, but at one point I had NVMe support in my WRK fork but no UEFI support

[2024-04-23 18:17] Azrael: [replying to Matti: "I don't need it anymore, but at one point I had NV..."]
WRK?

[2024-04-23 18:17] Matti: windows research kernel

[2024-04-23 18:18] Matti: it's the server 2003/XP x64 kernel source code

[2024-04-23 18:18] Azrael: [replying to Matti: "windows research kernel"]
Oh yeah.

[2024-04-23 18:18] Azrael: Is that what they're calling it?

[2024-04-23 18:19] Matti: uh well that is its name

[2024-04-23 18:20] Azrael: Understandable, I've never paid attention to it.

[2024-04-23 18:43] irql: the intel optane fan group 🦾

[2024-04-24 16:20] ֆƈǟʀLᏋȶȶ 𝕯ǟռɢᏋʀ: What’s the best auth method in your opinion and why?

[2024-04-24 16:33] 25d6cfba-b039-4274-8472-2d2527cb: answer is: depends

[2024-04-24 16:33] Timmy: it's a convenience <-> security <-> requirements trade-off decision that you're trying to make, there is no such thing as a 'best' here.

[2024-04-24 16:34] Brit: no auth, no code, no db it's the future.

[2024-04-24 16:39] 25d6cfba-b039-4274-8472-2d2527cb: Take any freemium softwares enterprise edition and look at the auth methods behind a paywall. That's probably what everyone is using. (LDAP, SAML2, OpenID connect etc.)

[2024-04-24 16:42] 25d6cfba-b039-4274-8472-2d2527cb: Because if you want a better way of managing users than per service in its own web panel, you're enterprise and need to be billed as such.

[2024-04-24 17:37] dullard: [replying to ֆƈǟʀLᏋȶȶ 𝕯ǟռɢᏋʀ: "What’s the best auth method in your opinion and wh..."]
basic auth over HTTP

[2024-04-24 17:37] 25d6cfba-b039-4274-8472-2d2527cb: [replying to dullard: "basic auth over HTTP"]
<:thonk:883361660100821032>

[2024-04-24 17:39] ֆƈǟʀLᏋȶȶ 𝕯ǟռɢᏋʀ: I was thinking of using IAM with a cloud solution like Azure

[2024-04-24 17:45] 25d6cfba-b039-4274-8472-2d2527cb: [replying to ֆƈǟʀLᏋȶȶ 𝕯ǟռɢᏋʀ: "I was thinking of using IAM with a cloud solution ..."]
https://learn.microsoft.com/en-us/entra/architecture/auth-saml
[Embed: SAML authentication with Microsoft Entra ID - Microsoft Entra]
Architectural guidance on achieving SAML authentication with Microsoft Entra ID

[2024-04-24 17:52] 25d6cfba-b039-4274-8472-2d2527cb: most web software with broader use tends to have saml2 support, so using it as your primary option should be pretty solid, and for your own software you can probs use some pretty solid library to implement it yourself without too much reading of specifications.

[2024-04-25 03:58] ֆƈǟʀLᏋȶȶ 𝕯ǟռɢᏋʀ: Decided to go with Next-Auth instead

[2024-04-25 13:26] Nats: [replying to ֆƈǟʀLᏋȶȶ 𝕯ǟռɢᏋʀ: "What’s the best auth method in your opinion and wh..."]
Auths without passwords

[2024-04-25 13:28] ֆƈǟʀLᏋȶȶ 𝕯ǟռɢᏋʀ: [replying to Nats: "Auths without passwords"]
Why do you think passwordless Auth is the way to go? Do you mean that the user does federated login instead?

[2024-04-25 14:55] repnezz: has anyone had issues  with FltQueryInformationFile / FsRtlGetFileSize before ?
I’m trying to call it in my pre write filter , and it returns 0(EOF.QuadPart) for non empty files I try to overwrite 
if I load my minifilter , then copy a file from another folder to a folder I monitor then the size is correct, issue is with files already there …?

[2024-04-25 14:56] repnezz: filtering NTFS btw ^

[2024-04-25 16:12] Matti: [replying to repnezz: "has anyone had issues  with FltQueryInformationFil..."]
wdym by 0(EOF.QuadPart)

[2024-04-25 16:12] Matti: and which of those two is returning this - or both?

[2024-04-25 16:12] Matti: both of these will return the on disk size of a file

[2024-04-25 16:13] Matti: but you also say you are trying to overwrite this same file

[2024-04-25 16:13] Matti: so - with what mode are you opening it?

[2024-04-25 16:15] Matti: FWIW I've used `FsRtlGetFileSize` loads of times, but never for files I'm writing to

[2024-04-25 16:15] Windy Bug: [replying to repnezz: "has anyone had issues  with FltQueryInformationFil..."]
Maybe it opens it with the truncate flag ? Have you tried retrieving the size in post create ?

[2024-04-25 16:15] Matti: ^ yeah this is why I asked about the open mode

[2024-04-25 16:16] Matti: some open modes will make the file size 0

[2024-04-25 16:16] Matti: also, you are using a synchronous I/O FO right

[2024-04-25 16:20] repnezz: [replying to Matti: "wdym by 0(EOF.QuadPart)"]
Hmm, FltQueryInformationFile with FileStandardInformation returns a struct , which has a member EndOfFile  , my pre write filter is supposed to read the contents from disk and for that I need the size . 
To test it, I wrote a simple Python script that opens the file with open(“test.txt”,”w”)

[2024-04-25 16:20] repnezz: I’m not sure how the Python write thing works under the hood

[2024-04-25 16:21] Matti: "w" will create a 0 byte file ✅

[2024-04-25 16:21] repnezz: Hmm, but the file exists ?

[2024-04-25 16:21] repnezz: Already

[2024-04-25 16:21] Matti: that's possible

[2024-04-25 16:21] Matti: if so, the file will be erased and then a 0 byte file will be created

[2024-04-25 16:22] Matti: what do you want to do instead? append to the existing file?

[2024-04-25 16:22] repnezz: nah, you’re right, it makes sense

[2024-04-25 16:22] repnezz: so what can I do to get the actual size

[2024-04-25 16:23] Matti: uh well FsRtlGetFileSize works

[2024-04-25 16:23] Matti: you just need to not nuke the file beforehand

[2024-04-25 16:23] Matti: or what am I missing

[2024-04-25 16:24] repnezz: [replying to Matti: "you just need to not nuke the file beforehand"]
Well this was just a test…

[2024-04-25 16:25] repnezz: I want my filter driver to be able to handle that case

[2024-04-25 16:25] Matti: the size being 0?

[2024-04-25 16:25] repnezz: [replying to Windy Bug: "Maybe it opens it with the truncate flag ? Have yo..."]
but it won’t work in post create either would it? Because it’s being opened then erased , then a new file is created?

[2024-04-25 16:26] Matti: look

[2024-04-25 16:26] Matti: why are you erasing this file

[2024-04-25 16:26] Matti: in either python or C

[2024-04-25 16:26] Matti: I'm not following what you are trying to do

[2024-04-25 16:27] repnezz: I’m working on a backup mechanism , I want to be able to recover the original content whenever a write is made to a file, in any form

[2024-04-25 16:27] Matti: alright

[2024-04-25 16:28] Matti: https://github.com/microsoft/Windows-driver-samples/tree/main/filesys/miniFilter/delete
[Embed: Windows-driver-samples/filesys/miniFilter/delete at main · microsof...]
This repo contains driver samples prepared for use with Microsoft Visual Studio and the Windows Driver Kit (WDK). It contains both Universal Windows Driver and desktop-only driver samples. - micros...

[2024-04-25 16:28] Matti: this is only for deletions, as the name implies

[2024-04-25 16:29] Matti: but I'm guessing you want to include deletions here.... especially those

[2024-04-25 16:29] Matti: https://github.com/microsoft/Windows-driver-samples/tree/main/filesys/miniFilter/minispy is more comprehensive yet
[Embed: Windows-driver-samples/filesys/miniFilter/minispy at main · microso...]
This repo contains driver samples prepared for use with Microsoft Visual Studio and the Windows Driver Kit (WDK). It contains both Universal Windows Driver and desktop-only driver samples. - micros...

[2024-04-25 16:31] Matti: there's also 'change' which literally tracks writes

[2024-04-25 16:32] Matti: to create a write log of sorts

[2024-04-25 16:32] Matti: lots and lots of FS minifilter samples

[2024-04-25 16:32] Matti: quite a few good ones too

[2024-04-25 16:33] repnezz: Thanks , I will look at them, just lastly - what is the sequence of irps you’d expect from from the Python write ?

[2024-04-25 16:34] Matti: no idea to be honest

[2024-04-25 16:34] Matti: it being python

[2024-04-25 16:34] Matti: but in NT you'd open with truncate

[2024-04-25 16:34] Matti: so it may just do that

[2024-04-25 16:35] Matti: 
[Attachments: image.png]

[2024-04-25 16:35] Brit: I love how they went with supersede instead of replace

[2024-04-25 16:35] Brit: for no reason

[2024-04-25 16:35] Matti: yeah I did mean supersede <:lillullmoa:475778601141403648> I just forgot about the stupid name

[2024-04-25 16:36] Matti: you've been.... superseded 😎

[2024-04-25 16:36] Brit: to be fair, if you say "I wrote a file supersession routine" in your quarterly review that goes down better than the alternative

[2024-04-25 16:37] Matti: err file_overwrite_if* is actually equally likely I guess... hm

[2024-04-25 16:37] Matti: unsure

[2024-04-25 16:37] repnezz: [replying to Matti: "but in NT you'd open with truncate"]
I assume it’s handled by ntfs then? so , wouldn’t what the other guy suggested work? moving it to post create?

[2024-04-25 16:37] Matti: [replying to Brit: "to be fair, if you say "I wrote a file supersessio..."]
that is very true

[2024-04-25 16:38] Matti: [replying to Matti: ""]
the python flags like "w" and such all roughly map to something in this table

[2024-04-25 16:39] Matti: see https://en.cppreference.com/w/cpp/io/c/fopen
[Embed: cpp/io/c/fopen]

[2024-04-25 16:39] Matti: that's for cpp but it's probably the same

[2024-04-25 16:39] Matti: I can't find python docs worth shit

[2024-04-25 16:40] Matti: careful not to read past the table itself, or you will end up ingesting cpp implementation details

[2024-04-25 16:40] Brit: infohazardous material

[2024-04-25 16:41] Matti: the rule of ~~3~~ ~~5~~ 0

[2024-04-25 16:41] Matti: essential cpp knowledge

[2024-04-25 16:50] brymko: there was a german cpp influence who sold the rules as the biggest think u need to know about cpp

[2024-04-25 16:51] Windy Bug: [replying to repnezz: "Thanks , I will look at them, just lastly - what i..."]
I’d expect a create followed by a set information?

[2024-04-25 16:52] Windy Bug: [replying to repnezz: "I assume it’s handled by ntfs then? so , wouldn’t ..."]
move to pre create , look at the create dispostion value. Either FILE_OVERWRITE,
FILE_OVERWRITE_IF or FILE_SUPERSEDE may truncate the file if successful.

[2024-04-25 16:53] Matti: yeah, the file open mode is what causes this

[2024-04-25 16:54] Matti: so the post create is kind of too late to find out that the file was opened in ~~trunc~~supersede mode

[2024-04-25 16:55] Matti: you may also want to watch out for any file opens with FILE_DELETE access, but that's for different reasons

[2024-04-25 16:56] Matti: you can open a file with delete access, and then set a flag so that it will be deleted once the handle is closed

[2024-04-25 16:57] Matti: there are like 4 or 5 ways to delete a file that are pretty obscure

[2024-04-25 16:57] Matti: the posix semantics one is especially good

[2024-04-25 16:58] Matti: but - this is for deleting files, not truncating which is what you were asking about

[2024-04-25 16:59] Matti: I'd expect in most cases where you want to catch the latter you also want to catch the former, so just mentioning it

[2024-04-26 02:07] hxm: <@609487237331288074> is there a way to make use of SEV-SNP to dto execute-only with VMPL permission masks ?

[2024-04-26 09:09] vendor: iirc yes but the idea is stupid

[2024-04-26 09:10] vendor: the extent to which you have to come up with crazy bullshit to get XOM on amd just means it’s not worth it and always comes with compromises.

[2024-04-26 09:11] vendor: the most feasible way i found was using MPK but even that has a million problems

[2024-04-26 09:12] vendor: better off finding different ways (not XOM) to achieve your goals. not every hook has to be inline and hidden globally like that.

[2024-04-26 12:39] daax: [replying to vendor: "the extent to which you have to come up with crazy..."]
I’ve posted a pretty simple method here multiple times but nobody wants to do it (or doesn’t read the response). It’s not difficult at all, and doesn’t use MPK.

[2024-04-26 12:40] vendor: [replying to daax: "I’ve posted a pretty simple method here multiple t..."]
didn't see this, what was it?

[2024-04-26 13:20] daax: [replying to hxm: "<@609487237331288074> is there a way to make use o..."]
yes, i wouldn’t recommend this though as <@519952679657668608> stated. you can use MPK+DRs.

[2024-04-26 13:32] vendor: but good luck enabling mpk + hiding the fact you've enabled it from the guest. literally impossible.

[2024-04-26 13:32] szczcur: [replying to daax: "yes, i wouldn’t recommend this though as <@5199526..."]
yeah w/ DBAM or the SLAT trick you mentioned

[2024-04-26 13:33] vendor: what's this DBAM/SLAT trick? did i miss a convo in here.

[2024-04-26 13:55] vendor: when i last experimented with this stuff MPK was such a pain. you pretty much have no option but to fully shadow the guest's page tables if you want to use it effectively since you need to hide the fact you've set the MPK constant on the PTE in the bits where windows is using for it's own shit + hide the fact you've made supervisor pages user + hide the real guest phys address of the page since MPK only affects virtual address translation so if the guest builds a different PTE to read the guest phys of the tampered page they bypass the execute-only MPK. those are just the things that can go wrong when the guest isn't MPK aware. if the guest actually is MPK aware then somehow you need to throw #UD on rdpkru/wrpkru without actually disabling the CR4 bit otherwise MPK won't work......

[2024-04-26 13:57] vendor: with all those problems + more the mitigations are just tanking performance even more + driving complexity through the roof to the point you would probably have been better off just doing some crappy NX page flipping via SLAT where the performance is bad but at least it's simple to implement. better to shoot yourself in the foot with 9mm rather than 5.56 <:nomore:927764940276772925>

[2024-04-26 23:58] Deleted User: whats the rason why vmware shows "BAD_EDID" for monitor related registry?

[2024-04-27 00:18] daax: [replying to Deleted User: "whats the rason why vmware shows "BAD_EDID" for mo..."]
graphics adapter / driver for vmware being shite

[2024-04-27 00:27] asz: 
[Attachments: OpenIPMI-2.0.31-3.el8.src.rpm]

[2024-04-27 00:27] asz: 
[Attachments: microcode_ctl-20230808-2.20231009.1.el8_9.src.rpm]

[2024-04-27 00:27] daax: [Unit]
Description=IPMI Driver
After=network.target

[Service]
Type=oneshot
RemainAfterExit=yes
ExecStart=/usr/libexec/openipmi-helper start
ExecStop=/usr/libexec/openipmi-helper stop

[Install]
WantedBy=multi-user.target

[2024-04-27 00:31] Deleted User: [replying to daax: "graphics adapter / driver for vmware being shite"]
I haven't tested it with vbox/KVM/qemu. Do u think its the same?

[2024-04-27 00:31] asz: 
[Attachments: 3520p1.cer]

[2024-04-27 00:34] Deleted User: What's that

[2024-04-27 02:00] Matti: kaspersky certificate using ECGOST

[2024-04-27 02:00] Matti: windows doesn't recognise this algo

[2024-04-27 02:01] Matti: 
[Attachments: image.png]

[2024-04-27 02:01] Matti: 
[Attachments: image.png]

[2024-04-27 02:02] Matti: conclusion: don't use the windows certificate shell extension <:kappa:697728545631371294>

[2024-04-27 02:10] asz: pub key length 0 bits heh

[2024-04-27 02:11] Matti: yeah keystore explorer also doesn't implement support for it, see https://github.com/kaikramer/keystore-explorer/issues/235
[Embed: Cannot import ECGOST3410-2012 key pair - algorithm is not supported...]
I get the following exception when trying to import the attached PKCS8 key pair (password 1234): Error org.kse.crypto.CryptoException: Could not check that the private and public keys comprise a ke...

[2024-04-27 02:12] Matti: but at least it doesn't blow up and prints the algo name correctly

[2024-04-27 02:12] Matti: which happens to include the key size too here

[2024-04-27 02:13] Matti: the windows shell extension is just trash

[2024-04-27 03:01] asz: yah

[2024-04-27 15:20] Deleted User: [replying to daax: "graphics adapter / driver for vmware being shite"]
does  some *software* use that to detect if program is in VM?

[2024-04-27 16:39] daax: [replying to Deleted User: "does  some *software* use that to detect if progra..."]
sure, lots of stuff could use it to detect a misconfiguration that is consistent with a VM.

[2024-04-27 16:50] Deleted User: there's bunch of registry entries to identify VM, i am just not sure if its similar accorss different hypervisors

[2024-04-27 16:53] daax: [replying to Deleted User: "there's bunch of registry entries to identify VM, ..."]
there will be many discrepancies across commercial vm platforms

[2024-04-27 17:11] daax: [replying to Deleted User: "I haven't tested it with vbox/KVM/qemu. Do u think..."]
also yes, on QEMU+KVM it will say BAD_EDID if you use default setup and virtualbox, they all do something akin to what i mentioned above which does not provide a proper EDID when queried.

[2024-04-27 17:19] daax: just to clarify, while this is weird, it has happened with the use of external switches like av access switches so it's not a reliable means of detection by itself (in terms of 100% accuracy). there are numerous other ways to determine if you're in a VM that are either 100% reliable and difficult to mitigate.

[2024-04-27 17:22] irql: yea a lot of more reliable methods like daax said -- but if you are curious this is their VMSVGA or "vmware svga"

[2024-04-27 17:22] irql: its documented online, its just a simple graphics pass-through style thing

[2024-04-27 17:23] irql: https://github.com/prepare/vmware-svga/tree/master
[Embed: GitHub - prepare/vmware-svga: not mine, clone from]
not mine, clone from . Contribute to prepare/vmware-svga development by creating an account on GitHub.

[2024-04-27 17:23] daax: [replying to irql: "yea a lot of more reliable methods like daax said ..."]
It will still occur even if you switch to the other options.

[2024-04-27 17:23] irql: so they got their own drivers to run this stuff

[2024-04-27 17:23] irql: [replying to daax: "It will still occur even if you switch to the othe..."]
oh hm

[2024-04-27 17:24] daax: hence why i said all their graphics adapter/drivers are shite.

[2024-04-27 17:24] daax: you have to some annoying hacks to get it working, but it's not worth it because it's not a reliable means anyways.

[2024-04-27 17:24] irql: yea definitely

[2024-04-27 17:25] szczcur: [replying to daax: "you have to some annoying hacks to get it working,..."]
do you think the acpi checks are still reliable despite your rundown of how to modify that info? referring to this <https://revers.engineering/evading-trivial-acpi-checks/>

[2024-04-27 17:31] daax: [replying to szczcur: "do you think the acpi checks are still reliable de..."]
yeah, it's still reliable because you can only modify so much without breaking boot. for example, there are indicators in dsdt that are dead giveaways even if the other checks fail, like VMBB aka vmware virtual battery methods or the NVD device description using VMW0002 and even the setup of that device.

[2024-04-27 17:35] daax: lots of small things / misconfigurations that don't occur on real hardware, and they're present on all the popular vm platforms. ofc you can intercept queries and modify results, yada yada, but you're just not gonna win in the long run. I'm guessing from the way <@456226577798135808> posed the question he is looking at *specific* software that does have pretty comprehensive checking <:Kappa:794707301436358686>

[2024-04-27 19:47] Deleted User: [replying to daax: "just to clarify, while this is weird, it has happe..."]
yea i know, tnx

[2024-04-27 19:48] Deleted User: [replying to daax: "lots of small things / misconfigurations that don'..."]
yes 🤫

[2024-04-27 19:56] Deleted User: [replying to daax: "just to clarify, while this is weird, it has happe..."]
cpuid, redpill, registry entries.. anything else?

[2024-04-27 19:57] Deleted User: redpill technique is from 2004

[2024-04-27 19:57] Deleted User: <:topkek:904522829616263178>

[2024-04-27 20:21] daax: [replying to Deleted User: "redpill technique is from 2004"]
what is redpill technique?

[2024-04-27 20:22] daax: this could encompass like 30 things lol

[2024-04-27 20:22] Deleted User: https://github.com/rootkovska/rootkovska.github.io/blob/master/papers/2004/redpill.md
[Embed: rootkovska.github.io/papers/2004/redpill.md at master · rootkovska/...]
My personal blog and website (see http://blog.invisiblethings.org/) - rootkovska/rootkovska.github.io

[2024-04-27 20:23] Deleted User: based & redpilled

[2024-04-27 20:32] Deleted User: [replying to daax: "this could encompass like 30 things lol"]
what are other things?

[2024-04-27 20:56] daax: [replying to Deleted User: "https://github.com/rootkovska/rootkovska.github.io..."]
<:lmao3d:611917482105765918> this is so whack for the sidt check.
[Attachments: image.png]

[2024-04-27 21:08] Deleted User: [replying to daax: "<:lmao3d:611917482105765918> this is so whack for ..."]
what do u suggest?

[2024-04-27 21:08] Deleted User: for vm detection?

[2024-04-27 21:09] vendor: 2004 xD

[2024-04-27 21:09] vendor: wow

[2024-04-27 21:09] vendor: seems so weird looking at code that executes the stack now

[2024-04-27 21:09] vendor: what a crazy time

[2024-04-27 21:10] daax: [replying to vendor: "seems so weird looking at code that executes the s..."]
right lmao

[2024-04-27 21:11] vendor: im surprised you can still do that on windows

[2024-04-27 21:11] vendor: i looked at a "ctf" binary recently that had the header flags necessary to get an RWX stack

[2024-04-27 21:11] vendor: was surprised that's still accepted

[2024-04-27 21:11] vendor: i get backwards compatability but bruh

[2024-04-27 21:13] luci4: [replying to vendor: "i looked at a "ctf" binary recently that had the h..."]
It's didactical, more than anythkng

[2024-04-27 21:16] daax: [replying to Deleted User: "for vm detection?"]
within the realm of what you posted? sidt, sldt, sgdt, str, xgetbv, cpuid ignoring limit, execution mode abuses, and various other analyses if those are handled. if you want some ideas: <https://secret.club/2020/04/13/how-anti-cheats-detect-system-emulation.html>. there are so many that are effective, even for the custom HVs.

[2024-04-27 21:17] daax: [replying to vendor: "i looked at a "ctf" binary recently that had the h..."]
wild, i feel like ctfs nowadays should have at least 2-3 of the mitigations on by default

[2024-04-27 21:18] Deleted User: [replying to daax: "within the realm of what you posted? sidt, sldt, s..."]
awesome, tyty fr 🙏

[2024-04-27 21:18] szczcur: [replying to daax: "wild, i feel like ctfs nowadays should have at lea..."]
unfortunately there are tons of ctfs that only add one or two, but rwctf and others are pretty good at keeping it real.

[2024-04-27 21:19] szczcur: that said, its not unheard of to stumble upon old legacy bs itw that doesnt have stack canaries or nx enabled

[2024-04-27 21:19] vendor: [replying to daax: "wild, i feel like ctfs nowadays should have at lea..."]
yeah it was some rly basic thing i had to do for university. wasn’t made by them tho - 3rd party platform they use for the security module.

[2024-04-27 21:20] vendor: well didn’t have to do it, you could pick whatever thing you wanted. most just did xss of sqli 101 but i went for the “hardest” binary exploitation and was disappointed xD

[2024-04-27 21:20] vendor: RWX stack and straight up socket recv buffer overflow

[2024-04-27 21:21] szczcur: [replying to vendor: "RWX stack and straight up socket recv buffer overf..."]
sounds like some of the re/vr tasks companies give as "assessments" of your skills prior to hiring

[2024-04-27 21:27] szczcur: 100 percent understand its useful to know if they can do the work. some of the notable cyber places had challenges they suggested are "extremely difficult" and they took abt 2 hours most of the time was spent finding one of the primitives necessary or directing execution to get there. the rest was pretty easy.. and was very similar to what you just said. i suppose thats all subjective though, but was a bit odd to classify it as extremely difficult when it had aslr and stack canaries only

[2024-04-27 21:29] daax: [replying to luci4: "It's didactical, more than anythkng"]
I read this as tactical initially <:lmao3d:611917482105765918> -- but ye, sounds like a reasonable challenge for uni students.

[2024-04-27 21:30] daax: [replying to szczcur: "100 percent understand its useful to know if they ..."]
ye well they probably aren't expecting some sweat like you lol

[2024-04-27 21:30] vendor: [replying to szczcur: "100 percent understand its useful to know if they ..."]
rather that than leetcode 😄

[2024-04-27 21:31] szczcur: [replying to daax: "ye well they probably aren't expecting some sweat ..."]
how many solves did your ctf get that you posted here

[2024-04-27 21:32] daax: iirc only 2

[2024-04-27 21:34] luci4: [replying to daax: "I read this as tactical initially <:lmao3d:6119174..."]
Anything helps! Not a lot of good resources on Windows pwn, unfortunately.

Although pwncollege recently released their fist Windows class, which is great

[2024-04-27 21:34] Deleted User: https://github.com/hacksysteam/HackSysExtremeVulnerableDriver

[2024-04-27 21:35] Deleted User: theres also this https://samsclass.info/126/WI2021_CCC.htm

[2024-04-27 21:36] luci4: [replying to Deleted User: "https://github.com/hacksysteam/HackSysExtremeVulne..."]
I've heard of HackSys before, need to get it done sometime soon

[2024-04-27 21:36] daax: [replying to luci4: "Anything helps! Not a lot of good resources on Win..."]
maybe i should put out some Windows ones and see if some of the other guys in sc that have them will share em too. i know <@285957040688463873> has some pretty 🔥 ones lol.

[2024-04-27 21:36] luci4: Gotta take it one step at a time, I have other stuff on my plate rn

[2024-04-27 21:36] luci4: [replying to daax: "maybe i should put out some Windows ones and see i..."]
I'd really appreciate it!

[2024-04-27 21:37] Deleted User: https://github.com/NullArray/WinKernel-Resources
https://github.com/sam-b/windows_kernel_resources

[2024-04-27 22:38] Deleted User: Daax you should drop another CTF challenge soon :p

[2024-04-27 22:38] Deleted User: That one was fun to solve, maybe a bit annoying but fun nonetheless 🙂

[2024-04-27 22:40] Brit: <@456226577798135808>
[Attachments: VEHMeme.exe]

[2024-04-27 22:41] Brit: I'm not daax, but this should be very slightly easier

[2024-04-27 22:42] Brit: wait I already sent you this

[2024-04-27 22:43] Brit: I don't recally if you solved it

[2024-04-27 22:43] Deleted User: I didn’t, looked at it very briefly

[2024-04-27 22:43] Brit: welp get to work :^)

[2024-04-28 14:28] froj: I’ll take a look at this later, ty for share :)

[2024-04-28 15:37] luci4: I'm gonna try

[2024-04-28 17:11] luci4: [replying to luci4: "I'm gonna try"]
😅

[2024-04-28 17:11] Brit: so? how's it coming lads?

[2024-04-28 17:12] Brit: :^)

[2024-04-28 17:12] Brit: to be fair I think there was only one solve when my mate released it at his uni ctf

[2024-04-28 17:13] luci4: [replying to Brit: "so? how's it coming lads?"]
https://tenor.com/view/green-mile-im-tired-boss-michael-clarke-duncan-gif-5852775606711249238

[2024-04-28 19:58] Xed0sS: Some one knows why on win 11 PAGE_NOACCESS memory showed like "???" in cheat engine?

[2024-04-28 19:59] Xed0sS: But on win 10 - normal bytes and also on win 11 when i try execute that memory (nx spoofed) - process crashed but if memory PAGE_READWRITE - all works fine, wtf?

[2024-04-28 22:03] froj: [replying to Brit: "so? how's it coming lads?"]
Was too hard, now at hospital

[2024-04-28 22:03] froj: (I’ll give it a go in a few days when I’m home)