# January 2025 - Week 4
# Channel: #application-security
# Messages: 11

[2025-01-26 17:43] .: https://lkml.indiana.edu/hypermail/linux/kernel/2301.0/01680.html
Detection for Lockheed Martin LMHS hypervisor used in US military fighter jets:
[Attachments: LMHS.cpp]

[2025-01-26 19:23] pinefin: um what

[2025-01-26 20:14] Torph: wait why did they need this feature in the kernel lol

[2025-01-26 20:36] daax: [replying to Torph: "wait why did they need this feature in the kernel ..."]
> This option allows Linux to detect when running on top of the SRE 
> + hypervisor. SRE is a hypervisor targeting environments with high
> + security, isolation, and determinism requirements and can be used
> + in embedded, edge or cloud environments. More details can found at
> + https://www.lockheedmartin.com/en-us/products/Hardened-Security-for-Intel-Processors.html

seems like lots of internal use
[Embed: Hardened Security for Intel Processors]
Lockheed Martin Hardened Security for Intel processors is a virtualized platform that offers a full security run-time solution, which isolates and protects customer domains.

[2025-01-26 20:37] daax: <:kekW:626450502279888906> this is a top picture
[Attachments: IMG_0342.jpg]

[2025-01-26 20:38] daax: where’s the eagle

[2025-01-26 20:54] Torph: [replying to daax: "> This option allows Linux to detect when running ..."]
I underestand why they would want the hypervisor but not sure why they would need to detect if they're running under it from their applications

[2025-01-26 20:54] Torph: [replying to daax: "<:kekW:626450502279888906> this is a top picture"]
thats pretty funny

[2025-01-26 21:08] daax: [replying to Torph: "I underestand why they would want the hypervisor b..."]
utilizing capabilities it enables

[2025-01-26 21:13] Torph: oh like being able to talk to processes in another VM under the hypervisor?

[2025-01-26 21:28] szczcur: [replying to Torph: "oh like being able to talk to processes in another..."]
i imagine much like hyperv there are features that are only available when its running.. this could be things like enabling a built in wireguard-like thing.. idk the features but it says cloud embedded edge. i suppose it could also be for secure data exchange across vm boundary but that seems less likely if theyre trying to ensure isolation