# March 2025 - Week 6
# Channel: #🔗-research-and-links
# Messages: 16

[2025-03-31 11:09] Matti: [replying to unknøwn: "https://github.com/unkbyte/measured_boot_poc"]
this is cool
I was aware of the possibility of doing (as the PCR values clearly change when you mess with the boot chain) but I never bothered to write a POC for it

[2025-03-31 11:09] Matti: but, I do seem to be getting false positives on all of my machines so far

[2025-03-31 11:11] Matti: this is my work laptop, it's locked down to the point where it's no longer funny, and msinfo32 OK's SB and PCR7 for bitlocker (which is also enabled)
[Attachments: image.png]

[2025-03-31 11:12] Matti: here's `PrintPcrs` FWIW, as well as the hash comparison for the bootmgfw.efi you're checking
[Attachments: image.png]

[2025-03-31 11:14] Matti: one note re: that part... you should be aware that due to the BlackLotus malware, there are now two 'official'/'reference' versions of bootmgfw.efi
see <https://support.microsoft.com/en-gb/topic/how-to-manage-the-windows-boot-manager-revocations-for-secure-boot-changes-associated-with-cve-2023-24932-41a975df-beb2-40c1-99a3-b3ff139f832d> (warning: very long and tedious)

[2025-03-31 11:15] Matti: after completing the above steps, it is expected that `%windir%\Boot\EFI_EX\bootmgfw_EX.efi` is the file placed on the ESP, not the one in `EFI`

[2025-03-31 11:19] Matti: second - about the `0x80000003` (application) and `0x80000004` (driver) events - these are not necessarily counted as 'tampering' by MS

[2025-03-31 11:20] Matti: what determines whether they are or aren't is a bit ridiculous IMO, but it basically comes down to whether the EFI file is located on a disk volume or a firmware volume (i.e. 'the BIOS')

[2025-03-31 11:21] Matti: I'm at work right now so I haven't got a link handy, but if you want I can find a reference for this later

[2025-03-31 11:24] Matti: [replying to Matti: "here's `PrintPcrs` FWIW, as well as the hash compa..."]
and lastly the decoded log file obtained by
`TBSLogGenerator.exe -LF MeasuredBoot\xxx.log > xxx.txt` for the current boot
[Attachments: 0000000401-0000000001.txt]

[2025-03-31 18:02] pinefin: [replying to Matti: "and lastly the decoded log file obtained by
`TBSLo..."]
pov: im a machine and i read this like a book

```
decent read, nothing too important in the first part of the file. gets a little more juicy towards the middle, stale ending. overall, would read this if i had nothing else to read
```

[2025-03-31 19:05] Matti: [replying to pinefin: "pov: im a machine and i read this like a book

```..."]
I admit it isn't great but it's a lot better than what went in

[2025-03-31 19:06] Matti: <https://learn.microsoft.com/en-us/troubleshoot/windows-client/windows-security/decode-measured-boot-logs-to-track-pcr-changes#use-tbsloggeneratorexe-to-decode-measured-boot-logs> - from OP's repository's references

[2025-03-31 19:07] Matti: here's the tool, in case you don't feel like downloading and installing 3 (HLK) to 30 (VHLK) GB just to get this exe out
[Attachments: ngscb.7z]

[2025-03-31 19:08] Matti: the PCPTool source code linked in the same article was entirely useless on my machines, it failed instantly on every input

[2025-03-31 19:08] Matti: I guess that's why it's the PCP tool