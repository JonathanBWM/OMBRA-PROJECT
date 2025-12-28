# August 2025 - Week 3
# Channel: #application-security
# Messages: 39

[2025-08-12 00:42] the horse: https://www.helpnetsecurity.com/2025/08/11/winrar-zero-day-cve-2025-8088/
[Embed: WinRAR zero-day exploited by RomCom hackers in targeted attacks - H...]
CVE-2025-8088, a WinRAR zero-day, is used in spearphishing campaigns against financial, manufacturing, defense, and logistics firms.

[2025-08-12 12:28] Brit: Yes there was

[2025-08-12 12:43] diversenok: I remember a fake 7zip LPE

[2025-08-12 12:44] diversenok: Getting SYSTEM by opening something in 7zip...

[2025-08-12 12:45] diversenok: Funny how they managed to "steal" system permission from 7zip that didn't have them to begin with

[2025-08-12 12:45] safareto: [replying to the horse: "https://www.helpnetsecurity.com/2025/08/11/winrar-..."]
good ol' ADS back at it again

[2025-08-12 13:03] Brit: [replying to diversenok: "I remember a fake 7zip LPE"]
Maybe this was it

[2025-08-12 14:31] AstroB: [replying to Tr1x: "I only mentioned it to say that a company without ..."]
and what is that world? working in the industry?

[2025-08-12 18:08] Pepsi: Those WinRAR/7zip cve's and news coverage are often misleading. In many cases it's just some random code flaw like an integer overflow and there is no evidence at all that it can be exploited in any way and people pretend your computer will instantly get compromised if you don't update your 7zip immediately.

[2025-08-12 18:15] Pepsi: The above mentioned WinRAR vuln seems to be Legitimate tho.

[2025-08-12 20:23] Tr1x: [replying to AstroB: "and what is that world? working in the industry?"]
Zero day brokerages. And in certain parts of the industry yes or starting your own thing

[2025-08-12 20:29] AstroB: [replying to Tr1x: "Zero day brokerages. And in certain parts of the i..."]
but exploiting some internet service will obviously land someone in trouble if caught, right?

[2025-08-12 20:37] Tr1x: [replying to AstroB: "but exploiting some internet service will obviousl..."]
Depends on what you're exploiting, your intentions and if you do damage when researching. It's a big legal gray area, completely legitimate companies that operate within the law purchase these exploits and sell them on. Some say it's protected under free speech for instance as you're only exchanging information and a lot of these brokerages protect the piracy of the seller. With that it makes it insanely difficult to go forward with prosecution or gain any legal grounds.

[2025-08-12 20:39] AstroB: [replying to Tr1x: "Depends on what you're exploiting, your intentions..."]
What? So someone can sell an exploit for a web service, that someone is obviously going to use, and it's legal? WTF

[2025-08-12 20:43] Tr1x: [replying to AstroB: "What? So someone can sell an exploit for a web ser..."]
There is different levels to it but let's say you find a privilege escalation from the browser abusing some v8 engine bullshit, well you can sell that

[2025-08-12 20:53] AstroB: [replying to Tr1x: "There is different levels to it but let's say you ..."]
I know that

[2025-08-12 20:54] AstroB: But what about an exploit for, let's say, microsoft(.)com login?

[2025-08-12 21:48] Tr1x: [replying to AstroB: "But what about an exploit for, let's say, microsof..."]
What exactly are you saying? As in what, accessing any account or something?

[2025-08-12 22:09] AstroB: [replying to Tr1x: "What exactly are you saying? As in what, accessing..."]
Yeah

[2025-08-12 22:09] AstroB: This doesn't seem like a legal thing to sell and if I were to find something like this I'd immediately write to microsoft

[2025-08-12 22:10] AstroB: or whatever website I found the bug in

[2025-08-13 00:19] mtu: it is not

[2025-08-13 00:19] mtu: Vuln research in general is a huge grey area, but it's pretty plainly illegal if the system that you subverted contains information that you don't own

[2025-08-13 00:21] mtu: Like, selling 0day chains in Chrome? I'm sure Google's lawyers would tell you it violates copyright law or something, but you're mostly fine.

Selling 0day chains in GMail? You had to have broken into systems you don't own to test/develop that

[2025-08-14 10:37] roddux: [replying to Pepsi: "Those WinRAR/7zip cve's and news coverage are ofte..."]
nah, the 7zip one was a path traversal one too

[2025-08-14 10:40] roddux: [replying to mtu: "Vuln research in general is a huge grey area, but ..."]
if you’re not running the software yourself, then you can do research only on the terms the platform agree to.

doesn’t stop bad actors from doing it though

[2025-08-14 12:11] Pepsi: [replying to roddux: "nah, the 7zip one was a path traversal one too"]
there isn't the one vuln, there are plenty

[2025-08-14 12:11] Pepsi: I was referring to CVE-2024-11477

[2025-08-14 12:12] Pepsi: everybody screamed RCE and it was a nothingburger

[2025-08-14 12:15] Pepsi: it was a legitimate bug, but it's impact and exploitability was really overstated

[2025-08-14 12:55] mtu: That’s like, most CVEs though

[2025-08-14 12:56] mtu: “This bug is technically RCE because if the application opens a crafted file that can be from the Internet it results in heap memory overflow that might be exploitable”
CVSS: 9.8

[2025-08-14 14:19] roddux: [replying to Pepsi: "there isn't the one vuln, there are plenty"]
oh my bad

[2025-08-14 14:19] roddux: [replying to mtu: "That’s like, most CVEs though"]
rip the days of useful cves

[2025-08-14 14:19] roddux: the moment they became cv padding is the moment they stopped being useful

[2025-08-17 20:38] the horse: Anyone know where I can pay for a support plan for Microsoft since the "free support" is now non-existent?

[2025-08-17 20:39] the horse: It's very nice being ignored weeks on end on e-mails; not having access to call anyone, and stuck in limbo for 5 months

[2025-08-17 22:33] koyz: [replying to the horse: "It's very nice being ignored weeks on end on e-mai..."]
Paid support is even worse, because you have the exact same problems whilst paying money! <:topkek:904522829616263178>

[2025-08-17 22:34] the horse: [replying to koyz: "Paid support is even worse, because you have the e..."]
You don't tip your landlord?