# August 2025 - Week 1
# Messages: 13

[2025-08-01 16:58] Vonnegut: [replying to Mitch: "As the guest has to be explicitly enlightened to t..."]
Not exactly hard though is it. IOMMU for sev/tdx works fine. Passthrough is a none issue as guests are aware of the devices via standard methods such as acpi tables

[2025-08-01 17:00] Mitch: [replying to Vonnegut: "Not exactly hard though is it. IOMMU for sev/tdx w..."]
It's a driver basis thing

[2025-08-01 17:00] Mitch: It requires drivers to explicitly support it in certain cases depending on what's used (at least, for networking)

[2025-08-01 17:02] Vonnegut: I'm a little confused as to what the OP was asking now then. We've had passthrough network & block working with SEV-SNP for yonks and the drivers haven't diverged from when it was running without

[2025-08-01 17:05] Vonnegut: If mtu is talking about actually virtual functions (via SR-IOV) for things like block and networking these do exist commercially outside of data centers they're just a bit pricey. Same as vGPU stuff too

[2025-08-02 05:12] varaa: is all that being streamed?

[2025-08-02 05:12] varaa: dma moment

[2025-08-02 06:46] daax: <@660977913599885313> no.

[2025-08-02 15:53] iplaynice: Okidoki. I wasn't aware we weren't allowed to directly post links for that stuff. I won't again.

[2025-08-02 15:56] iplaynice: Can I share pdf for reading materials?

[2025-08-02 19:25] daax: [replying to iplaynice: "Can I share pdf for reading materials?"]
yes, if they’re appropriate and relevant to the channel

[2025-08-02 23:46] iplaynice: They may or may not be... it's not in English, they're in Chinese. For Android Kernel dev

[2025-08-03 09:39] daax: [replying to iplaynice: "They may or may not be... it's not in English, the..."]
yeah that’s fine