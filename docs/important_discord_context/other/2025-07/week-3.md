# July 2025 - Week 3
# Messages: 24

[2025-07-14 02:23] mustarheard: I had a quick question: Do user-mode processes rely solely on the guest page tables for enforcing memory permissions? I read that if EFER.NXE is not set, the NX (No-Execute) bit in the page tables is treated as a reserved bit, and attempting to set it will result in a page fault. Given that, is there any way to bypass this restriction, such as marking memory as RX using a hypervisor, perhaps by leveraging EPT/NPT.  However, as far as I know, with EPT/NPT alone, clearing NX in the guest page tables isn’t enough, and the guest won't be able to execute from that page unless permissions are correctly set in both the guest and EPT/NPT.

[2025-07-14 05:56] Dis: [replying to mustarheard: "I had a quick question: Do user-mode processes rel..."]
so you are going to execute NX area when EFER.NXE is not set?

[2025-07-14 06:00] Dis: first, you're right: enabling EPT.Execute only but not on guest's PTE is invalid, therefore you will cannot execute on that page. I can tell you how to mitigate this problem based on what you are trying to do.

[2025-07-14 06:09] mustarheard: [replying to Dis: "first, you're right: enabling EPT.Execute only but..."]
hey, my goal is to keep the guest PTE as RW while actually executing from that page(yea this might sound dumb. but i was just wondering if this was possible)

[2025-07-14 06:37] Dis: I don't know your situation exactly, but I would emulate that page if I was in your situation. I couldn't think any method to execute without PTE.Execute, even with hypervisor.

[2025-07-15 10:43] xaxax: does anynone knows the differences between eac.sys and eac_eos.sys ?

[2025-07-15 15:08] the horse: EOS runs via Epic Online Services ans the former runs on their own network, Kami (if i remember correctly). Kami is obsolete but servers still run.

[2025-07-15 15:52] Pepsi: it was kamu

[2025-07-15 16:54] xaxax: ok tysm

[2025-07-20 01:47] davi: huh,

[2025-07-20 01:47] davi: so `KERNEL_SECURITY_CHECK_FAILURE` with parameter 1: `EAC`

[2025-07-20 01:47] davi: what kinds of checks would trigger this?

[2025-07-20 01:48] davi: i'm curious because my friend has been playing MCC, without any kind of cheats or weird drivers, and has been getting that bsod. i was thinking ram issue, but i'm uncertain it'd always hit eac or kernel mem to cause it.

[2025-07-20 01:53] Xyrem: https://tenor.com/view/easyanticheat-memey-gif-18874294

[2025-07-20 01:55] davi: starting to suspect some shitty driver like Armoury Crate could be screwing around

[2025-07-20 01:58] raax: [replying to davi: "so `KERNEL_SECURITY_CHECK_FAILURE` with parameter ..."]
i vaguely remember seeing a post somewhere showing EAC use that code to bsod PC on some AC checks if they fail, not confident on that tho

[2025-07-20 02:26] the horse: [replying to raax: "i vaguely remember seeing a post somewhere showing..."]
yup

[2025-07-20 10:36] LabGuy94: [replying to Xyrem: "https://tenor.com/view/easyanticheat-memey-gif-188..."]
"hudsight users have a 30% increased KD across games with our product on average!"

[2025-07-20 13:42] Deleted User: same

[2025-07-20 13:43] Deleted User: i just wanted to play the finals with my friends 🥀

[2025-07-20 15:18] davi: wow, yea, eac sucks

[2025-07-20 15:18] davi: my friend can consistently get crashed by eac

[2025-07-20 15:26] Pepsi: no idea what setups you have, on my system eac never caused any issues besides (justified) gamebans in some instances

[2025-07-20 21:40] jay: same