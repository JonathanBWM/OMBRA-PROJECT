# April 2025 - Week 1
# Channel: #🔗-research-and-links
# Messages: 18

[2025-04-01 00:52] valium: i made a project to render imgui natively on android (without an apk)
https://github.com/valium007/imgui-android
[Embed: GitHub - valium007/imgui-android: Rendering imgui in android using ...]
Rendering imgui in android using native surface. Contribute to valium007/imgui-android development by creating an account on GitHub.

[2025-04-01 00:55] valium: heres another one to inject a shared library into an apk/process that uses opengl to render imgui 
https://github.com/valium007/android-imgui-menu
[Embed: GitHub - valium007/android-imgui-menu: Inject imgui at runtime with...]
Inject imgui at runtime without magisk. Contribute to valium007/android-imgui-menu development by creating an account on GitHub.

[2025-04-01 09:54] unknøwn: [replying to Matti: "this is cool
I was aware of the possibility of doi..."]
Thanks for the info! And yes, would be nice if you could share the reference you mentioned

regarding the false detections: the implementation of the detection "algorithm" is quite naive. It basically assumes that only bootmgfw.efi is loaded and compares the hash of the file on the C drive, if something else then bootmgfw gets loaded too, it will always fail. The detection worked in my setup (VMware and with my motherboard firmware) but didnt do any further testing.

---
also, just as a side note, it’s relatively easy to modify the TCG logs from EFI. While the PCRs still end up being "invalid", you cant really trace back what was actually booted and what caused the mismatch.
[Attachments: image.png]

[2025-04-01 10:04] Matti: [replying to unknøwn: "Thanks for the info! And yes, would be nice if you..."]
ah yeah, my bad, here you go: https://learn.microsoft.com/en-us/windows-hardware/manufacture/desktop/windows-secure-boot-key-creation-and-management-guidance?view=windows-11#24-secure-boot-and-3rd-party-signing

so I only remembered half right - if I'm interpreting this correctly, the exception I mentioned only applies to drivers, not applications, to add some more inconsistency to the mix:
> Any **drivers** that are included in the system firmware image do not need to be re-verified. Being part of the overall system image provides sufficient assurance that the driver is trusted on the PC.
[emphasis mine]

anything other than this seems to fall in the category of boot loaders, which is what the second section is about. again nothing is actually 100% clear here IMO because it could also be interpreted to mean that "boot loaders can be signed using the MS signing certificate [in addition to embedding them in the firmware as mentioned above for drivers]."
[Embed: Windows Secure Boot Key Creation and Management Guidance]

[2025-04-01 10:05] Matti: the link to their driver signing policy is of course memory holed, here is the most recent archive.org: https://web.archive.org/web/20180218102032/https://blogs.msdn.microsoft.com/windows_hardware_certification/2013/12/03/microsoft-uefi-ca-signing-policy-updates/
[Embed: Microsoft UEFI CA Signing policy updates]
This post describes UEFI Signing policy changes. These changes are important to help secure boot meet its security goals and also to maintain a reasonable turnaround time for signing UEFI submissions.

[2025-04-01 10:07] Matti: note that since the blacklotus updates which changed the MS db and KEK certificates, anywhere you see a "2011" certificate mentioned that should *probably* be a "2023" certificate nowadays (see the link I posted earlier with the 25 steps to upgrade your boot manager)

[2025-04-01 10:11] Matti: [replying to Matti: "ah yeah, my bad, here you go: https://learn.micros..."]
it's kind of unfortunate that I have no idea what shitware HP app was loaded on my laptop during boot, but at least going by `TBSLogGenerator.exe -VLF` ("Validate the given raw TCG log event file. If no file is specified, defaults to the existing TCG log."), the second interpretation seems to be correct, i.e. applications are also OK to embed in firmware in order to count as trusted

[2025-04-01 10:13] Matti: signing a DXE driver *file* (in this case I signed efiguard) with a certificate that is in `db` also satisfies TBSLogGenerator, FWIW
[Attachments: image.png]

[2025-04-01 10:14] Matti: idem dito for bitlocker's requirements

[2025-04-01 10:14] Matti: funnily enough that is not true if you sign the optional loader application and use that to start efiguard

[2025-04-05 13:15] Rairii: [replying to Matti: "idem dito for bitlocker's requirements"]
bitlocker tpm-only with secure boot validation would require that PCR7 matches whatever the initially calculated pcr7 was

[2025-04-05 13:16] Rairii: for that, would be PCA2011 or UEFI PCA2023 (plus whatever signed option rom code if applicable)

[2025-04-05 13:17] Rairii: ie, you need to use an exploit in the boot environment to dump the keys, and downgrade attacks (to winload/other second stage boot application) can still work as long as you get code exec with the derived keys still in memory

[2025-04-05 17:54] Matti: I don't know what to tell you other than that this is not correct when the driver is signed by a valid certificate in db (e.g. mine - not a very hard requirement assuming you have SB set up correctly and you actually own the PK and not the other way around ).

This result surprised me somewhat which is why I posted it in the first place

[2025-04-05 21:30] subgraphisomorphism: https://codedefender.io/blog/2025/03/27
[Embed: A Deep Dive into Our LLVM-MSVC Integration]
One of the standout features of BLARE is its Module Extension System, which allows us to statically link additional functions and data generated by llvm-msvc (our inhouse fork of llvm) into a pre-exis

[2025-04-05 21:30] subgraphisomorphism: https://github.com/backengineering/cd-integration-example
[Embed: GitHub - backengineering/cd-integration-example: Example CodeDefend...]
Example CodeDefender/LLVM-MSVC module integration. Contribute to backengineering/cd-integration-example development by creating an account on GitHub.

[2025-04-05 21:58] Pepsi: 
[Attachments: grafik.png]

[2025-04-06 06:25] subgraphisomorphism: lmfao