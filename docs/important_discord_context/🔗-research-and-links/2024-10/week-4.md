# October 2024 - Week 4
# Channel: #🔗-research-and-links
# Messages: 32

[2024-10-21 05:34] !!! Tar.sh: https://areweanticheatyet.com/

[2024-10-26 04:18] subgraphisomorphism: https://x.com/BackEngineerLab/status/1849956962869592314
[Embed: Back Engineering Labs (@BackEngineerLab) on X]
The next generation of binary obfuscation for the Windows platform is now public\. No source code annotations required, full SEH support,  CET support, ACG compliant\. 

Join the Discord: https://t\.c

[2024-10-26 04:22] dinero: ahaha discord breaks the links thats so tragic

[2024-10-26 14:33] Glatcher: [replying to subgraphisomorphism: "https://x.com/BackEngineerLab/status/1849956962869..."]
Reworked viper?

[2024-10-26 15:01] Matti: https://github.com/PS5Dev/Byepervisor/blob/main/Byepervisor_%20Breaking%20PS5%20Hypervisor%20Security.pdf
[Embed: Byepervisor/Byepervisor_ Breaking PS5 Hypervisor Security.pdf at ma...]
A PS5 hypervisor exploit for 1.xx-2xx firmwares. Contribute to PS5Dev/Byepervisor development by creating an account on GitHub.

[2024-10-26 15:03] Matti: <@753171127903191102> I agree, also eagerly awaiting video of the presentation

[2024-10-26 15:05] Matti: I don't know anything about PS5 myself but from having heard specter talk about this in the past, the biggest issue with this HV escape is that it requires 2.x firmware, which is very very old and therefore also rare

[2024-10-26 15:05] Matti: like, you could buy a 100 used PS5s on ebay and not one might have a FW this old

[2024-10-26 15:07] Matti: but specter of course got a PS5 as soon as possible and prevented the firmware from ever updating, precisely because it is well known that sony (and MS too) often silently patch critical security bugs in firmware updates

[2024-10-26 15:08] Matti: but, if you do have a PS5 with this firmware, from what I understand this opens up the door to homebrew

[2024-10-26 15:09] Matti: and quite likely it will also make it easier to find bugs in newer PS5 FWs to exploit

[2024-10-26 15:11] donna🤯: console exploitation seems really interesting, havent really read into it too much

[2024-10-26 15:11] donna🤯: [replying to Matti: "and quite likely it will also make it easier to fi..."]
yea definitely

[2024-10-26 15:13] donna🤯: this has now got me interested in console exploitation, time to enter the rabbit hole

[2024-10-26 15:15] Matti: I did totally pwn a wii once by following detailed instructions on the zelda exploit on a public wiki 😎

[2024-10-26 15:16] Matti: that game was an investment that paid itself back 10 times over in terms of uh, "homebrew"

[2024-10-26 16:55] Matti: you'd definitely have to ask specter that, not me

[2024-10-26 16:56] Matti: from what I've understood about the PS5 keys system it isn't as simple as dumping 'the keys' for your particular system

[2024-10-26 16:56] Matti: since there are multiple layers of hardware obfuscation involved there

[2024-10-26 16:56] Matti: but, maybe I'm behind and this has been figured out

[2024-10-26 16:58] Matti: mhm, well if the FW update keys are not system specific (why would they be, that seems strange) then yeah I think you're right

[2024-10-26 16:58] Matti: I do wonder how this guy obtained the keys though I have to say

[2024-10-26 16:59] Matti: I remember specter not ironically consiering investing some $50-100K USD in lab tech to decap the CPUs

[2024-10-26 17:00] Matti: yeah, it does sound slightly insane doesn't it

[2024-10-26 17:00] Matti: but he was not joking

[2024-10-26 17:01] Matti: hopefully this guy has found a more viable method

[2024-10-26 17:02] Matti: same

[2024-10-26 17:02] Matti: fortunately specter does have some brains so he arrived at the same conclusion

[2024-10-26 17:04] Matti: np!

[2024-10-26 17:55] subgraphisomorphism: [replying to Glatcher: "Reworked viper?"]
nah

[2024-10-26 17:55] subgraphisomorphism: its bin2bin, targeted towards games/anti cheats/etc.

[2024-10-26 18:47] subgraphisomorphism: [replying to Matti: "https://github.com/PS5Dev/Byepervisor/blob/main/By..."]
very interesting exploit. Its funny those jump tables are in the rdata section, msvc puts them in the .text section with the function its associated with.