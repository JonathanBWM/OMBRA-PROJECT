# April 2025 - Week 4
# Channel: #application-security
# Messages: 13

[2025-04-23 10:23] Timmy: https://sandflysecurity.com/platform/threat-detection/

so... how do they do any of this without software running on the clients? Do they just analyse network traffic?
[Embed: EDR Threat Detection for Linux  |  Sandfly Security]
Learn about the thousands of Linux threats Sandfly can detect from intruders to malware and dangerous security practices. Sandfly deploys instantly.

[2025-04-23 10:31] truckdad: https://docs.sandflysecurity.com/docs/theory-of-operation
https://docs.sandflysecurity.com/docs/protected-system-requirements

[2025-04-23 10:31] truckdad: it seems to me their definition of “agentless” means no _persistent_ running software on the client; it still requires root-level access via SSH at any time to run code on-demand

[2025-04-23 10:39] truckdad: it’s a lot less interesting now, isn’t it 😆

[2025-04-23 10:42] truckdad: [replying to truckdad: "it seems to me their definition of “agentless” mea..."]
i don’t even know if the non-persistent part is true, actually. there could be persistent parts to it, which would make sense from a real-time detection perspective. i suppose the big claim is just that you don’t have to manually install anything?

[2025-04-23 10:46] Timmy: wow

[2025-04-23 10:47] Timmy: didnt see that coming

[2025-04-23 10:47] Timmy: LOL

[2025-04-23 10:48] 25d6cfba-b039-4274-8472-2d2527cb: EDR built on Ansible 🤪

[2025-04-23 11:14] 25d6cfba-b039-4274-8472-2d2527cb: Imagine ur security product connecting through ssh to an already compromised server, surely nothing can go wrong.

[2025-04-23 12:01] Timmy: suuuurelly

[2025-04-23 15:22] JustMagic: https://humzak711.github.io/analyzing_Sandfly
[Embed: Analyzing Sandfly: An agentless Linux EDR solution]
Blog to showcase and share my knowledge on reverse-engineering and low-level programming

[2025-04-26 15:02] OutOfScope: Pardon me, i'm not sure if i'm leaving this in the correct topic or not.

Already reviewing the Zeus malware source code, but couldn't even make it to the "Building" part.

Wondering if any one had similar or related experience to help me out on this one. (Or any one interested to put sime and effort into it, TBH i think it's worth it)

Source Code link: https://github.com/zeustrojancode/Zeus

Thanks in advance.
[Embed: GitHub - zeustrojancode/Zeus: NOT MY CODE! Zeus trojan horse - leak...]
NOT MY CODE! Zeus trojan horse - leaked in 2011, I am not the author. This repository is for study purposes only, do not message me about your lame hacking attempts.  - GitHub - zeustrojancode/Zeus...