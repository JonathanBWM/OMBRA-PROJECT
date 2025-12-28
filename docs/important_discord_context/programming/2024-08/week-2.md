# August 2024 - Week 2
# Channel: #programming
# Messages: 252

[2024-08-08 13:25] elias: I'm trying to implement the ARM64 instruction CMP (32 bit, shifted register) in C. Does someone see errors or room for improvement in this code? I'm especially unsure if the NZCV values are correctly calculated

```c
            case MeCmpShf32:

                Rn = REG_LOWER(Rg[INSTR(In, ME_CMP_SHF)->Rn]);
                Rm = REG_LOWER(Rg[INSTR(In, ME_CMP_SHF)->Rm]);
                Im = INSTR(In, ME_CMP_SHF)->Im;

                if (INSTR(In, ME_CMP_SHF)->Sh == 0)
                    Rm <<= Im;
                else if (INSTR(In, ME_CMP_SHF)->Sh == 1)
                    Rm >>= Im;
                else
                    Rm = (INT32)Rm >> Im;

                Value = (INT32)Rn - (INT32)Rm;

                Vcpu->State.N = Value < 0 ? 1 : 0;
                Vcpu->State.Z = Value == 0 ? 1 : 0;
                Vcpu->State.C = (Rn >= Rm) ? 1 : 0;

                Vcpu->State.V = (((INT32)Rn < 0 && (INT32)Rm > 0 && Value >= 0) ||
                    ((INT32)Rn > 0 && (INT32)Rm < 0 && Value < 0)) ? 1 : 0;

                Vcpu->InstructionPointer++;
                break;
```

[2024-08-08 14:06] Hunter: `? 1 : 0;` is unnecessary, also whats with all the casts bleh setup a union for the registers so u could access parts of it without casting or putting macros that mask the values everywhere

[2024-08-08 14:09] Hunter: thats what i would do in cpp since i dont code in C even tho unions are mostly C theyre really useful u might have a diff opinion ¯\_(ツ)_/¯

[2024-08-08 14:28] elias: ur right, thank you for the input

[2024-08-08 15:09] Tetsuo AI: nice code other than the ternary.

[2024-08-08 15:09] Tetsuo AI: i fk up and do that sometimes as well

[2024-08-08 15:11] Tetsuo AI: <:PepeLaugh:1246892517778264239>
[Attachments: for_cherished_user.png]

[2024-08-08 15:13] 5pider: LOL

[2024-08-08 15:37] elias: 😭

[2024-08-08 15:37] elias: ```c
            case MeCmpShf32:

                Rn = REG_LOWER(Rg[Instr.CmpShf.Rn]);
                Rm = REG_LOWER(Rg[Instr.CmpShf.Rm]);
                Im = Instr.CmpShf.Im;

                if (Instr.CmpShf.Sh == 0)
                    Rm <<= Im;
                else if (Instr.CmpShf.Sh == 1)
                    Rm >>= Im;
                else
                    Rm = (INT32)Rm >> Im;

                Value = (INT32)Rn - (INT32)Rm;

                Vcpu->State.N = Value < 0;
                Vcpu->State.Z = Value == 0;
                Vcpu->State.C = (Rn >= Rm);

                Vcpu->State.V = (((INT32)Rn < 0 && (INT32)Rm > 0 && Value >= 0) ||
                    ((INT32)Rn > 0 && (INT32)Rm < 0 && Value < 0));

                Vcpu->InstructionPointer++;
                break;
```

[2024-08-08 15:37] elias: looking better already

[2024-08-08 15:37] donna🤯: honestly I prefer to be exlpicit with the ternarys with TRUE / FALSE

[2024-08-08 15:37] donna🤯: even though it makes no difference

[2024-08-08 15:37] donna🤯: i find it easier to read

[2024-08-08 15:39] donna🤯: [replying to Tetsuo AI: "nice code other than the ternary."]
Ah a fellow tpot dingboard ai ml from scratch in c has joined 😆

[2024-08-08 15:46] Torph: lmao I thought ternary was called unary

[2024-08-08 15:58] Tetsuo AI: [replying to Torph: "lmao I thought ternary was called unary"]
`+` unary <:delet_this:838736832002261043>

[2024-08-08 15:58] Tetsuo AI: `++` ~~twonary~~ unary

[2024-08-08 15:59] diversenok: `+` onenary then

[2024-08-08 17:11] Hunter: id do 
```C
Rn = Rg[Instr.CmpShf.Rn].low/.dword1;
```

[2024-08-08 17:20] luci4: [replying to Tetsuo AI: "`+` unary <:delet_this:838736832002261043>"]
glad to see you here

[2024-08-08 20:14] elias: On ARM64, when a function with more than 8 arguments is called, where is the 9th argument on the stack relative to the stack pointer?

[2024-08-08 20:25] contificate: the stack pointer will point to it on entry

[2024-08-08 20:26] contificate: 10th will be `[sp, 8]` and so on

[2024-08-08 20:28] elias: perfect, thank you a lot

[2024-08-10 08:15] vendor: there are quite a lot of functions in winload that based off the name look like they would be useful for debug printing a message to the screen. before i trial and error calling 30 of these does anyone know one that works?

[2024-08-10 08:15] vendor: just aiming to clear the screen, write a message, leave it a few seconds then call reset

[2024-08-10 09:26] Matti: printing to the screen is gonna be a lot harder to do from winload than printing to a kernel debugger, if that's an option

[2024-08-10 09:27] Matti: if you've got `/bootdebug` enabled you can simply call `BlStatusPrint("value: 0x%p\n", p);` or whatever you want to log, printf style

[2024-08-10 09:28] Matti: it's even exported nowadays

[2024-08-10 09:29] Matti: to print to the screen you'll need to use either EFI runtime services (assuming there's a valid console handle installed to print to) or GOP to draw the text as an image

[2024-08-10 09:31] Matti: personally I do this in efiguard: <https://github.com/Mattiwatti/EfiGuard/blob/master/EfiGuardDxe/EfiGuardDxe.h#L223>
this makes the message go to any debugger directly, and stores it for delayed last-chance printing in the ExitBootServices callback if needed

[2024-08-10 09:31] Matti: I only do the latter for fatal errors

[2024-08-10 10:08] Matti: they're not inconsistent

[2024-08-10 10:08] Matti: the behaviour of things that call into EFI services depends on the current winload execution context

[2024-08-10 10:09] vendor: well this is kind of my situation

[2024-08-10 10:09] Matti: see this comment <https://github.com/Mattiwatti/EfiGuard/blob/master/EfiGuardDxe/PatchNtoskrnl.c#L6>

[2024-08-10 10:09] vendor: before boot services are out i can just use those ezpz

[2024-08-10 10:09] vendor: but once winload has thrown everything around

[2024-08-10 10:09] vendor: gets a bit tricky

[2024-08-10 10:10] vendor: can't use a kernel debugger unfortunately

[2024-08-10 10:10] Matti: do you need these prints to be immediate

[2024-08-10 10:10] Matti: or is buffering and waiting acceptable

[2024-08-10 10:11] vendor: well i normally have 2 situations.

1. system randomly resets, i want to printf debug to see how far it's getting before triple faulting into oblivion

2. i fail an assert and want to print a message before resetting the system.

[2024-08-10 10:12] vendor: and my code is obviously spread across phases of UEFI and windows init

[2024-08-10 10:12] vendor: so it's tricky to know what i can use and when

[2024-08-10 10:14] vendor: also, i knew about BlpArchSwitchContext but i never understood why my hook on winload works, since it should be hit when winload is in it's own context....

[2024-08-10 10:14] vendor: somehow i'm able to detour straight into my UEFI module which shouldn't be mapped

[2024-08-10 10:16] vendor: [replying to Matti: "personally I do this in efiguard: <https://github...."]
actually i like this idea

[2024-08-10 10:16] vendor: this doesn't work for either scenario but it's nice

[2024-08-10 10:16] vendor: will still be useful

[2024-08-10 10:18] Matti: yeah +1 for qemu

[2024-08-10 10:18] vendor: i have my code working on vmware with serial port debugging

[2024-08-10 10:18] vendor: but it's time to go BM xD

[2024-08-10 10:18] vendor: and the code isn't 1:1

[2024-08-10 10:18] Matti: also consider exdi instead of serial, it's basically software JTAG
https://www.andrea-allievi.com/blog/debugging-the-undebuggable-part-1/

[2024-08-10 10:19] Matti: [replying to vendor: "i have my code working on vmware with serial port ..."]
rule number one: never trust vmware

[2024-08-10 10:20] Matti: hell `/bootdebug` doesn't even work in vmware

[2024-08-10 10:20] Matti: with the boot manager that is

[2024-08-10 10:20] Matti: it does with winload

[2024-08-10 10:23] Matti: [replying to vendor: "well i normally have 2 situations.

1. system rand..."]
1. this is a microsecond or maybe millisecond problem
you can't eyeball this, you need a debugger
that also gives you the benefit of an assert with stacktrace instead of a triple fault and reset

[2024-08-10 10:24] vendor: yeah i fear you are right

[2024-08-10 10:24] Matti: 2. yeah that one is probably also not a good fit to dalay
though it depends on the assert

[2024-08-10 10:25] vendor: unrelated but extremely cool

[2024-08-10 10:25] vendor: managed to fully calculate from scratch my PCR0 value

[2024-08-10 10:25] vendor: 
[Attachments: image.png]

[2024-08-10 10:25] vendor: dumped the bios, extracted and hashed these two volumes and the hashes match

[2024-08-10 10:27] Matti: well neat but

[2024-08-10 10:27] Matti: knowing shit about TPMs

[2024-08-10 10:27] Matti: what does this get you?

[2024-08-10 10:27] vendor: nothing xD

[2024-08-10 10:27] vendor: i just sent the measured boot log parsing tool to a friend

[2024-08-10 10:27] vendor: and got side tracked

[2024-08-10 10:29] vendor: it's a shame PCR0 isn't standardized

[2024-08-10 10:29] Matti: I'm guessing it has to be part of the spec that PCR0 matches these volumes, otherwise it wouldn't really have any meaning

[2024-08-10 10:29] Matti: but still, TIL

[2024-08-10 10:29] vendor: because doing anything useful with it must be difficult if you are dealing with a range of motherboards

[2024-08-10 10:29] Matti: [replying to vendor: "it's a shame PCR0 isn't standardized"]
it's not?

[2024-08-10 10:29] vendor: [replying to Matti: "I'm guessing it has to be part of the spec that PC..."]
i don't think it is

[2024-08-10 10:30] Matti: so this is a total lucky find then? ha

[2024-08-10 10:30] Matti: but also, if it's not

[2024-08-10 10:30] vendor: i'll check but i'm pretty sure it's standardized to the extent of "PCR0 is BIOS code"

[2024-08-10 10:30] Matti: then what is its purpose

[2024-08-10 10:30] Matti: mm

[2024-08-10 10:30] vendor: maybe daaximus prime knows

[2024-08-10 10:31] vendor: [replying to Matti: "then what is its purpose"]
well this didn't take too long to figure out and i'd imagine all ASUS boards have the same pattern

[2024-08-10 10:32] vendor: it's probably possible to figure out the calculation logic for other manufactures

[2024-08-10 10:32] vendor: and then you can remote attest the BIOS code

[2024-08-10 10:35] Matti: hmm it doesn't seem to match on my asus motherboard
[Attachments: image.png]

[2024-08-10 10:35] Matti: unless I'm doing something wrong hashing/extracting

[2024-08-10 10:36] Matti: I extracted as-is using uefitool

[2024-08-10 10:36] vendor: parse your latest measured boot log

[2024-08-10 10:36] Matti: uhm uuuhhh

[2024-08-10 10:36] Matti: I of course do this how

[2024-08-10 10:36] vendor: it's not in the SC

[2024-08-10 10:36] vendor: what are the hashes in there?

[2024-08-10 10:38] Matti: don't tell me you're supposed to be able to see the fucking TPM PCR values in windows 11 in the security center

[2024-08-10 10:38] Matti: if so, where/how

[2024-08-10 10:38] Matti: or else, what do I do instead

[2024-08-10 10:38] vendor: no, screenshot

[2024-08-10 10:39] Matti: oh herp

[2024-08-10 10:39] vendor: asking to see the top of the MB log xml xd

[2024-08-10 10:39] Matti: yeah that is still a how question

[2024-08-10 10:39] Matti: but sec

[2024-08-10 10:39] vendor: 
[Attachments: PCPTool.exe, TpmAtt.dll]

[2024-08-10 10:40] vendor: ```PCPTool.exe decodelog C:\Windows\Logs\MeasuredBoot\0000000234-0000000000.log > TPM_log.xml```

[2024-08-10 10:40] Matti: oh god what is this now

[2024-08-10 10:40] vendor: if you want to build yourself <https://github.com/microsoft/TSS.MSR>

[2024-08-10 10:40] vendor: it's from Microsoft

[2024-08-10 10:40] Matti: surely `tpmdiagnostics GatherLogs` will work

[2024-08-10 10:40] vendor: requires wdk 8.1 though 😦

[2024-08-10 10:41] Matti: lol that's not an issue

[2024-08-10 10:41] vendor: [replying to Matti: "surely `tpmdiagnostics GatherLogs` will work"]
hmn i don't have this

[2024-08-10 10:41] vendor: not sure how it works

[2024-08-10 10:41] Matti: 
[Attachments: image.png]

[2024-08-10 10:41] Matti: lemme see

[2024-08-10 10:42] vendor: for me PCR0 is 4 measurements to get the final value

[2024-08-10 10:42] vendor: ```
00000000  05 2b 10 a7 c7 d9 65 41 81 40 2a dd e9 4a f6 3c  |.+.§ÇÙeA.@*ÝéJö<|```

[2024-08-10 10:42] vendor: the first one is this tiny junk

[2024-08-10 10:42] vendor: and the last one is this other junk ``00 00 00 00``

[2024-08-10 10:42] vendor: the middle two are those 2 volumes

[2024-08-10 10:44] Matti: [replying to vendor: "```PCPTool.exe decodelog C:\Windows\Logs\MeasuredB..."]
well this runs into a little snag
[Attachments: image.png]

[2024-08-10 10:45] Matti: lemme reboot, it's possible that the tpmdiagnostics gatherlogs cleared this dir of logs

[2024-08-10 10:46] Matti: or do you literally have something enabled for this

[2024-08-10 10:46] vendor: just secure boot and tpm

[2024-08-10 10:46] Matti: ok, I'm not secure booting atm but I can be

[2024-08-10 10:46] vendor: but we don't need measured boot

[2024-08-10 10:46] vendor: let me give you my script

[2024-08-10 10:46] vendor: actually no

[2024-08-10 10:46] vendor: we do

[2024-08-10 10:46] vendor: nvm

[2024-08-10 10:47] Matti: well there's 86 files in the directory now
that ought to be enough

[2024-08-10 10:49] Matti: ok this XML file is huge but, it does contain the hashes

[2024-08-10 10:49] Matti: 
[Attachments: image.png]

[2024-08-10 10:49] Matti: 
[Attachments: image.png]

[2024-08-10 10:50] vendor: 
[Attachments: image.png]

[2024-08-10 10:50] vendor: ```python
import hashlib
import binascii

def pcr_extend(pcr_value, data):
    data_bytes = binascii.unhexlify(data)
    
    hash_object = hashlib.sha256()
    hash_object.update(data_bytes)
    new_hash_value = hash_object.hexdigest()
    
    pcr_bytes = binascii.unhexlify(pcr_value)
    new_hash_bytes = binascii.unhexlify(new_hash_value)
    hash_object = hashlib.sha256()
    hash_object.update(pcr_bytes)
    hash_object.update(new_hash_bytes)
    
    new_pcr_value = hash_object.hexdigest()
    return new_pcr_value

def calculate_pcr(data_array, initial_pcr_value="0"*64):
    pcr_value = initial_pcr_value

    for data in data_array:
        if isinstance(data, str):
            pcr_value = pcr_extend(pcr_value, data)
        elif isinstance(data, dict) and 'file' in data:
            with open(data['file'], 'rb') as f:
                file_data = f.read()
                file_data_hex = file_data.hex()
                pcr_value = pcr_extend(pcr_value, file_data_hex)
        else:
            raise ValueError("Invalid data type in array")

    return pcr_value

# Example usage:
data_array = [
    "052b10a7c7d9654181402adde94af63c", # CRTM MD5 maybe?
    {'file': 'C:\\Users\\user\\Documents\\bios\\Volume_FFSv2_818A40E1-D82E-497D-A058-D261636A4CB7.vol'},
    {'file': 'C:\\Users\\user\\Documents\\bios\\Volume_FFSv3_4F1C52D3-D824-4D2A-A2F0-EC40C23C5916.vol'},
    "00000000" # EV_Separator
]

final_pcr_value = calculate_pcr(data_array)
print("Final PCR value:", final_pcr_value)```

[2024-08-10 10:50] Matti: ok that is the limit

[2024-08-10 10:50] Matti: no python

[2024-08-10 10:50] vendor: chatgpt wrote it xD

[2024-08-10 10:50] vendor: and it works

[2024-08-10 10:51] Matti: import hashlib
import binascii

[2024-08-10 10:52] Matti: I can already see how well this is going to work

[2024-08-10 10:52] vendor: [replying to vendor: ""]
xDDDD

[2024-08-10 10:52] vendor: it gives the right answer

[2024-08-10 10:52] vendor: it matches the current PCR value

[2024-08-10 10:52] vendor: just pluck the other binary measurements from your parsed log into it

[2024-08-10 10:52] vendor: and fix the file paths

[2024-08-10 10:53] vendor: and pray

[2024-08-10 10:53] vendor: 
[Attachments: image.png]

[2024-08-10 10:53] vendor: there should only be 2 other measurements

[2024-08-10 10:53] vendor: for me PCR0 is just 4, those 2 BIOS blobs and then those 2 tiny buffers

[2024-08-10 10:54] Matti: I already showed you that it matches the FW volume...

[2024-08-10 10:55] Matti: the event record does, that is

[2024-08-10 10:55] Matti: but the output of pcptool matches that of tpmdiagnostics

[2024-08-10 10:56] Matti: everyone is pretty much in agreement about the value of PCR0 AFAICT
[Attachments: image.png]

[2024-08-10 10:56] Matti: so

[2024-08-10 10:56] Matti: the FW volume hashes are part of the input that makes up the PCR0 value for sure

[2024-08-10 10:57] Matti: the python script is supposed to predict the same?

[2024-08-10 10:57] vendor: [replying to Matti: "the python script is supposed to predict the same?"]
yep

[2024-08-10 10:57] Matti: ok then

[2024-08-10 10:58] Matti: great I believe you

[2024-08-10 10:58] vendor: thanks for taking part in pointless curiosity experiment

[2024-08-10 10:58] Matti: lol I was just confused about what you were claiming the actual value of PCR0 was made up of

[2024-08-10 10:58] vendor: PCR0 probably is meaningfully attestable if you put enough effort in

[2024-08-10 10:59] vendor: [replying to Matti: "lol I was just confused about what you were claimi..."]
afaik the spec just says it's bios/firmware code and then it's implementation dependent how and what you actually hash into it

[2024-08-10 11:00] Matti: hmmm

[2024-08-10 11:00] Matti: pretty funny logs
[Attachments: image.png]

[2024-08-10 11:01] vendor: hahahhaa

[2024-08-10 11:01] Matti: I wonder if MS does telemetry on what common firmware signers are

[2024-08-10 11:02] vendor: i mean what doesn't MS do telemetry on

[2024-08-10 11:03] contificate: bill gates trips to various islands

[2024-08-10 11:04] Matti: damn I'm losing
[Attachments: image.png, image.png]

[2024-08-10 11:35] daax: [replying to vendor: "i mean what doesn't MS do telemetry on"]
fun fact if you block all of their traffic at application level on latest windows, to domains and otherwise you might wind up freezing every couple hours for about 10s. only happens when i nuke all their telemetry outlets and windows update.

[2024-08-10 11:37] vendor: [replying to daax: "fun fact if you block all of their traffic at appl..."]
lmao, do you block it externally via network or on the pc?

[2024-08-10 11:47] daax: [replying to vendor: "lmao, do you block it externally via network or on..."]
at first on pc, didn’t care it existed just was testing something and needed all the noise gone then froze randomly throughout the day. now i just keep it as a fun little fuck you to myself because whoever wrote the code that results in this is a dingbat

[2024-08-10 14:48] asz: C:\Windows\System32\TpmTool.exe

[2024-08-10 14:48] asz: also

[2024-08-10 15:20] Matti: [replying to vendor: "well this didn't take too long to figure out and i..."]
ok this idea bothered me a bit because there's no way asus would write the tcg/tpm measurement library themselves
it would have to be either the TPM vendor (so in this case the intel/AMD platform code since it's an ftpm), or the firmware developer, which is AMI, not asus

[2024-08-10 15:20] Matti: turns out it's AMI

[2024-08-10 15:22] Matti: `AmiTcgPlatformPei` is responsible for taking the FW volume hashes and using the TPM to extend them into the PCR

[2024-08-10 15:22] Matti: at least on both our boards (well 1 of  yours and 2 of mine that I checked, so 3 total)

[2024-08-10 15:24] Matti: as usual with AMI the code is horrible
[Attachments: AmiTpm20PlatformPei.c]

[2024-08-10 15:26] Matti: there's no way to tell the difference between
- things that are done because AMI decided to do them (implementation defined, let's say)
- things that are done because they are in the TCG spec, and
- things that are potentially done (or not), but can be overridden by the board manufacturer at build time

[2024-08-10 15:28] Matti: there's 60 `#ifdef`s in this file alone, and due to their insane build system those values can come from pretty much anywhere

[2024-08-10 15:29] Matti: basically the only way to determine for certain whether some branch will be taken in AMI code is to actually compile it and see

[2024-08-10 15:30] elias: wow

[2024-08-10 15:30] elias: i didnt know it was that bad

[2024-08-10 15:30] Matti: so, I would say that this PCR0 calculation is *probably* correct for any AMI motherboard...
...but you can never really know for sure

[2024-08-10 15:31] Matti: [replying to elias: "i didnt know it was that bad"]
yeah AMI code is shocking honestly

[2024-08-10 15:32] Matti: and this is pretty good code by their standards actually, and readable

[2024-08-10 15:32] Matti: unironically

[2024-08-10 15:35] Matti: this sort of code where half of it it is commented out with no explanation given is more  representative honestly
[Attachments: image.png]

[2024-08-10 15:38] elias: what the hell man

[2024-08-10 15:38] Matti: AMI reference code for NCT super IO
[Attachments: image.png]

[2024-08-10 15:38] elias: no wonder there are so many bugs in there

[2024-08-10 15:39] Matti: yeah it's honestly amazing that any of our PCs can even boot

[2024-08-10 15:40] vendor: <@148095953742725120> LOL, nice find, this is super interesting

[2024-08-10 15:46] elias: [replying to Matti: "yeah it's honestly amazing that any of our PCs can..."]
does msi use ami or insyde? 🤔

[2024-08-10 15:46] Matti: AMI

[2024-08-10 15:46] Matti: I believe dell or HP use insyde

[2024-08-10 15:46] Matti: maybe both

[2024-08-10 15:47] elias: rip

[2024-08-10 15:47] elias: i wonder if insyde is any better

[2024-08-10 15:48] Matti: I can't really say
I do have some insyde code but I've never needed to look at it for anything

[2024-08-10 15:49] Matti: I'd say it's not possible that it's worse than AMI code

[2024-08-10 15:50] Matti: it has got to be better than AMI code, because AMI code is such an outrageous outlier

[2024-08-10 15:50] Matti: hut I don't know if it's *good*

[2024-08-10 15:51] Matti: intel and AMD both have pretty high code quality in the platform reference libs, I'd say

[2024-08-10 15:52] Matti: no clue about insyde

[2024-08-10 16:05] Eriktion: If any of you have ever dealt with the windows frame compositor / vram  and how windows interacts with it contact me in dms please

[2024-08-10 16:06] Eriktion: I wanna learn how to render using either by directly writing to the vram or learn how to write to the final frame the compositor produces

[2024-08-10 16:06] elias: Is it possible to force msvc to create a RUNTIME_FUNCTION entry for every function?

[2024-08-10 16:23] Azrael: [replying to Matti: "I believe dell or HP use insyde"]
Well, HP does develop their own firmware for enterprise.

[2024-08-10 16:25] Azrael: But uhh, laptops and whatnot is usually Insyde (or Phoenix). ASUS is one of the only vendors that has AMI for their laptop images as far as I know.

[2024-08-10 16:25] Matti: as does dell

[2024-08-10 16:25] Matti: I meant for consumer boards

[2024-08-10 16:25] Matti: alright

[2024-08-10 16:25] Azrael: Oh yeah, consumer boards are almost always AMI.

[2024-08-10 16:25] Azrael: If we're talking about Intel/AMD's standard lineups.

[2024-08-10 16:26] Matti: nah laptops count

[2024-08-10 16:26] Matti: I just meant... not enterprise

[2024-08-10 16:26] Azrael: Ahh okay.

[2024-08-10 16:26] Azrael: Well, desktop is almost always AMI.

[2024-08-10 16:26] Matti: yeah

[2024-08-10 16:26] Azrael: Laptop is Insyde/Phoenix, or AMI in ASUS case.

[2024-08-10 16:27] Azrael: Acer is strictly Insyde as far as I know.

[2024-08-10 18:08] asz: took a look at last dell firmware at https://dl.dell.com/FOLDER11488292M/1/BIOS_IMG.rcv?uid=dbd94bd5-2cec-4260-4506-74ea575e3553&fn=1

[2024-08-10 18:09] asz: it seems to do the update by WMI!?

[2024-08-10 18:11] Azrael: They do what now?

[2024-08-10 18:16] asz: nah

[2024-08-10 18:17] asz: i think the flashing happen next boot in the dell sos recovery enviroment

[2024-08-10 18:21] asz: SecureBootRecovery.efi havent noticed before

[2024-08-10 18:22] Azrael: [replying to asz: "i think the flashing happen next boot in the dell ..."]
You can't update your UEFI firmware image from within Windows though, can you?

[2024-08-10 18:22] Azrael: It has to pass it down to the underlaying firmware like HP does.

[2024-08-10 18:23] Azrael: And Dell.

[2024-08-10 18:26] asz: there is  uefi capsule service for that

[2024-08-10 18:27] asz: this
[Attachments: capsule.c]

[2024-08-10 18:28] Azrael: [replying to asz: "there is  uefi capsule service for that"]
Yes.

[2024-08-10 19:03] Torph: [replying to Matti: "as usual with AMI the code is horrible"]
lol where did you find this source from
is it just publicly available?

[2024-08-10 19:12] Matti: uhhh yeah, definitely publicly available

[2024-08-10 19:12] Matti: not so sure about legally

[2024-08-10 19:14] Matti: every one of these firmware vendors has a new data breach every 6 months or so

[2024-08-10 19:14] Matti: so that's where these sources tend to come from

[2024-08-10 19:22] Azrael: [replying to Matti: "every one of these firmware vendors has a new data..."]
Nice.

[2024-08-10 19:22] Azrael: Can you hint towards the places to look in?

[2024-08-10 19:28] Matti: look in your DMs

[2024-08-10 19:29] Matti: or maybe not
[Attachments: image.png]

[2024-08-10 19:29] Azrael: I'll add you.

[2024-08-10 19:30] Matti: I'm honoured, thanks

[2024-08-11 11:02] mrexodia: [replying to elias: "Is it possible to force msvc to create a RUNTIME_F..."]
As far as I know that's not possible. Leaf functions do not need a `RUNTIME_FUNCTION` by definition...

[2024-08-11 19:12] Deleted User: https://tenor.com/view/officialyongbok-cat-being-held-like-a-burger-cat-silly-silly-cat-gif-7676949455246341117