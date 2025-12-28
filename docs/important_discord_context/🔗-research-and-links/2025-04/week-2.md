# April 2025 - Week 2
# Channel: #🔗-research-and-links
# Messages: 28

[2025-04-08 09:53] Rairii: [replying to Matti: "I don't know what to tell you other than that this..."]
if your system already needed a driver signed by that cert in db  then yes (can't remember what checks are done when the vmk gets sealed to tpm though)

[2025-04-08 10:18] Matti: well my (windows 11) system obviously doesn't *need* efiguard in any way, but yes, you're right that it would have been loaded prior to any windows installation or FVE taking place

[2025-04-08 10:20] Matti: since most of my older windows installations depend on efiguard in order to boot at all (much less with secure boot on), I have to say I kinda prioritise setting that up over installing windows 11 for the Nth time because windows update broke something again

[2025-04-08 10:23] Matti: so if this part isn't surprising to you (it was to me since it contradicts what MS claims - but let's give them the benefit of the doubt and say the article was written assuming* a standard default PK and MS KEK in place)... then what do you think about the part where this only applies to *driver* files, not applications?

[2025-04-08 10:24] Matti: at the very least I would say the article is extremely confusingly written in this regard

[2025-04-08 10:26] Matti: [replying to Matti: "so if this part isn't surprising to you (it was to..."]
\*except for applications that are located on a firmware volume, since *that* apparently takes priority over *this* rule

[2025-04-08 10:28] Matti: the best explanation I can think of is that MS assumes any EFI application on a FW volume must be the BDS DXE application that starts the boot manager, which would be naive to say the least

[2025-04-08 10:50] Matti: [replying to Matti: "the link to their driver signing policy is of cour..."]
yet another contradiction, assuming this requirement still holds, which I assume is the case...
> 10. Microsoft will not sign EFI submissions that use EFI_IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER. Instead, we recommend transitioning to EFI_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER. This prevents unnecessary use of runtime EFI drivers.
efiguard is in fact a runtime driver (see <https://github.com/Mattiwatti/EfiGuard/blob/master/EfiGuardDxe/EfiGuardDxe.inf#L96>, and <https://github.com/Mattiwatti/EfiGuard/blob/master/EfiGuardDxe/EfiGuardDxe.c#L397> for the explanation for why this is the case, at least by default)

so MS would not have signed this driver (for a lot of reasons probably... but in particular I mean this one), but *me* signing it makes this A-OK?

[2025-04-08 10:53] Matti: I guess there's once again the option to give them the benefit of the doubt and say that this is intentional in order to let db signers choose their own policy... but I'm going to call this an oversight myself

[2025-04-09 10:23] Rairii: [replying to Matti: "I guess there's once again the option to give them..."]
yeah, MS driver signing policy doesnt matter if you're signing stuff yourself and control your keys

[2025-04-09 10:24] Rairii: thats just what MS will auto reject if you try to sign an efi PE through them

[2025-04-09 10:26] Matti: yes lol, I do believe I understand the difference

[2025-04-09 10:28] Matti: what I'm saying is that it seems inconsistent to me to apply the MS signing policy as part of PCR7/BL requirements depending on whether or not a user is able to sign their own drivers (which should always be the assumption even if it's rarely done in reality)

[2025-04-09 19:13] 5pider: really cool project that vector35 just released https://github.com/Vector35/scc
[Embed: GitHub - Vector35/scc]
Contribute to Vector35/scc development by creating an account on GitHub.

[2025-04-09 19:48] pinefin: [replying to 5pider: "really cool project that vector35 just released ht..."]
oh neat

[2025-04-09 19:49] pinefin: > __**Resolve and call Windows functions**__
> *SCC supports the ability to dynamically resolve and call windows functions for you with the right syntax. The following simple example is a popup displaying hello world using MessageBoxA.*
```c
int __stdcall MessageBoxA(HANDLE hwnd, const char* msg, const char* title, uint32_t flags) __import("user32");

int main()
{
    MessageBoxA(NULL, "Hello", "Hello World.", 0);
    return 0;
}
```

[2025-04-09 19:50] pinefin: i wish they provided these examples with outputs of the actual shellcode as well

[2025-04-09 22:05] mrexodia: [replying to pinefin: "i wish they provided these examples with outputs o..."]
You can just run it <:KEKW:912974817295212585>

[2025-04-09 22:17] pinefin: [replying to mrexodia: "You can just run it <:KEKW:912974817295212585>"]
true

[2025-04-12 03:02] nuclearfirefly: [replying to subgraphisomorphism: "https://codedefender.io/blog/2025/03/27"]
when is you guys nestra coming out on the big screen? you guys are keeping it quiet

[2025-04-12 03:08] nuclearfirefly: https://nestra.tech/
[Embed: Nestra]
Commercializing affordable software security worldwide.

[2025-04-12 03:09] nuclearfirefly: roblox blog posts, arm obfuscation,, i want to use it

[2025-04-12 03:09] nuclearfirefly: pls when coming ,?

[2025-04-12 03:10] nuclearfirefly: 
[Attachments: ssySn6C.webp]

[2025-04-12 03:44] subgraphisomorphism: [replying to nuclearfirefly: "when is you guys nestra coming out on the big scre..."]
That's not us

[2025-04-12 03:44] subgraphisomorphism: But we are friends with them

[2025-04-12 03:47] subgraphisomorphism: Its <@879852314540781650> project

[2025-04-12 23:38] atrexus: [replying to subgraphisomorphism: "But we are friends with them"]
🙏