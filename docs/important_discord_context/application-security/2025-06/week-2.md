# June 2025 - Week 2
# Channel: #application-security
# Messages: 33

[2025-06-02 19:41] th3ma5hatt3r: Hi all. Would anyone be able to point me to a resource that would help me understand how I might be able to “modify the platform firmware in a way that should cause a failure of the integrity check?”

[2025-06-02 19:42] Brit: that sentence without additionally context is just words

[2025-06-02 19:51] th3ma5hatt3r: Very true. I’m trying to test that a platform properly fails its integrity check during boot when the firmware is tampered with. Specifically, I want to modify a firmware binary (like a UEFI image or bootloader), just enough to break the signature or hash check, and then confirm that the system refuses to boot or throws an error. 

I was simply going to modify one of the firmware binaries by flipping a byte, but I was worried about a false positive.

[2025-06-02 20:02] Brit: could you expand what a false positive would mean in this case? a bit flip should be enough to cause a secureboot failiure

[2025-06-03 12:51] dullard: cp FirmwareBlob.bin FirmwareBlob.bin.bak
echo "a'' >> FirmwareBlob.bin

[2025-06-03 12:51] dullard: ggez

[2025-06-03 18:52] Matti: [replying to th3ma5hatt3r: "Very true. I’m trying to test that a platform prop..."]
which integrity check
be more specific

[2025-06-03 18:52] Matti: which platform is this, and what part are you looking to modify that should be detected

[2025-06-03 18:53] Matti: in general a UEFI image and a bootloader are completely different things that are located in different places

[2025-06-03 18:53] Matti: the bootloader isn't firmware

[2025-06-03 18:54] Matti: for the firmware itself, the integrity check(s) and how well they are implemented generally vary a lot from platform to platform and vendor to vendor

[2025-06-03 18:55] Matti: from none (ARM64 SoC dev board) to things like intel bootguard, there's quite a range

[2025-06-03 18:56] Matti: [replying to dullard: "cp FirmwareBlob.bin FirmwareBlob.bin.bak
echo "a''..."]
you do realise this is just about the only way to use echo that does not flip a bit in the original firmware right

[2025-06-03 18:56] Matti: it will only change the filesize

[2025-06-03 18:56] Matti: which depending on the flasher may simply be ignored

[2025-06-03 19:42] dullard: [replying to Matti: "you do realise this is just about the only way to ..."]
Yeah? Isn’t the goal to result in the device which is tryna boot the firmware to fail integrity checks? 

Doesn’t adding junk to the end result in a file which is different but should function as intended ? 

Tbh I’ve gone of the assumption that it’s a dumb hash based check

[2025-06-03 19:46] Matti: yeah, that's all true, but the part you're missing is that SPI ROMs have a certain fixed size

[2025-06-03 19:46] Matti: so you can't take an N byte dump and flash N+1 bytes back to the same ROM

[2025-06-04 10:37] Deleted User: [replying to Matti: "so you can't take an N byte dump and flash N+1 byt..."]
lzma:

[2025-06-04 10:51] Horsie: [replying to Matti: "from none (ARM64 SoC dev board) to things like int..."]
Amd's (evil) PSB once again comfortably ignored 😔

[2025-06-04 16:40] Matti: it's included in the range!

[2025-06-04 16:42] Matti: isn't that amazing, how ranges can include multiple things (often even an infinite number)

[2025-06-04 16:43] Matti: note how I also did not mention apple's whatever it's called again either

[2025-06-04 16:43] Matti: but it is included in the range!

[2025-06-04 19:19] Horsie: Just poking some fun at your comical bias towards Intel :)

[2025-06-04 19:19] Horsie: Dont get all red on me <:yara_lover:1148745271577157673>

[2025-06-05 00:05] Matti: idgi though, it's not like I was complaining about bootguard

[2025-06-05 00:05] Matti: it's just a well known example of a system that wouldn't allow this, assuming it's correctly configured

[2025-06-05 00:06] Matti: you're the only one who's ever mentioned AMD PSB that I know of <:kekw:904522300257345566>

[2025-06-05 00:07] Matti: I'd have to look up what it even is again

[2025-06-05 00:07] Matti: I'm sure I won't be a fan

[2025-06-05 00:08] Matti: I've just either never owned a system with it enabled, or it's useless

[2025-06-06 19:59] eval00: I was looking into psb on amd when I was deep diving into trenchboot