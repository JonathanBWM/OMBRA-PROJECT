# April 2024 - Week 4
# Channel: #🔗-research-and-links
# Messages: 22

[2024-04-22 23:05] stuub: https://github.com/Stuub/RCity-CVE-2024-27198

A PoC i've created to exploit JetBrains TeamCity servers vulnerable to CVE-2024-27199 & 27198.

Achieves:
RCE
Admin Account Creation
Enumeration of Users, UserID, Auth Tokens & Emails
Gathering Server Information
Generating Authorisation Tokens

Check it out, thanks <:pepelove1:836750164461879316>
[Embed: GitHub - Stuub/RCity-CVE-2024-27198: CVE-2024-27198 & CVE-2024-2719...]
CVE-2024-27198 & CVE-2024-27199 PoC - RCE, Admin Account Creation, Enum Users, Server Information - Stuub/RCity-CVE-2024-27198

[2024-04-25 20:20] stuub: Just finished the PoC for CVE-2024-4040. Very scary exploit. Happy hacking <:pepelove1:836750164461879316> https://github.com/Stuub/Crushed-CVE-2024-4040-PoC
[Embed: GitHub - Stuub/CVE-2024-4040-SSTI-LFI-PoC: CVE-2024-4040 CrushFTP S...]
CVE-2024-4040 CrushFTP SSTI LFI & Auth Bypass | Full Server Takeover | Wordlist Support - Stuub/CVE-2024-4040-SSTI-LFI-PoC

[2024-04-25 20:24] jvoisin: [replying to stuub: "Just finished the PoC for CVE-2024-4040. Very scar..."]
why not a metasploit module?

[2024-04-25 20:43] stuub: [replying to jvoisin: "why not a metasploit module?"]
never crossed my mind to do it :)

[2024-04-25 21:12] dullard: [replying to jvoisin: "why not a metasploit module?"]
ruby <:Starege:894092640591708180>

[2024-04-25 21:13] jvoisin: [replying to dullard: "ruby <:Starege:894092640591708180>"]
msf modules, even though they're indeed in ruby, are trivial to write usually. It's mostly copy paste

[2024-04-25 21:13] dullard: I do agree but ruby <:kekw:899071675524591637>

[2024-04-25 21:17] 5pider: doesnt metasploit support python modules lol

[2024-04-26 00:03] stuub: 7K+ CrushFTP hosts on shodan. Reportedly, a hell of a lot are vulnerable. Considering this is a point and shoot PoC, metasploit would actually be more work to run lol

[2024-04-26 11:02] stuub: [replying to stuub: "Just finished the PoC for CVE-2024-4040. Very scar..."]
wordlist support for lfi now :)

[2024-04-26 11:11] jvoisin: [replying to 5pider: "doesnt metasploit support python modules lol"]
but they still strongly prefer ruby.

[2024-04-26 17:32] 0xatul: https://exploits.forsale/24h2-nt-exploit/ damn!

[2024-04-26 18:02] Torph: wow, that's wild

[2024-04-26 18:18] szczcur: [replying to 0xatul: "https://exploits.forsale/24h2-nt-exploit/ damn!"]
<@609487237331288074> lol ExpManufacturingInformation oof

[2024-04-26 18:21] daax: [replying to szczcur: "<@609487237331288074> lol ExpManufacturingInformat..."]
<:PES2_Shrug:513352546341879808> was bound to happen at some point

[2024-04-26 18:23] daax: neat tie together in the write up

[2024-04-26 19:53] vendor: [replying to 0xatul: "https://exploits.forsale/24h2-nt-exploit/ damn!"]
haha that's crazy. in trying to mitigate double fetch issues they introduced a double fetch <:kekw:904522300257345566>

[2024-04-26 20:04] vendor: actually i guess the most interesting take away from that article is that there probably are a handful of places in the windows source that **are** vulnerable to double-fetch but it doesn't manifest in the binaries due to compilers optimizing the reads into one.

[2024-04-26 22:10] daax: [replying to vendor: "haha that's crazy. in trying to mitigate double fe..."]
this is something i've seen recently, they mitigate something and change a group of internal functions then they unintentionally introduce something new. it sort of seems like there is some rushing going on during the W11 overhauls of things.

[2024-04-27 16:47] stuub: [replying to stuub: "Just finished the PoC for CVE-2024-4040. Very scar..."]
Just added a feature to test all tokens extracted from target CrushFTP servers, extracts valid sessions and usernames :)

[2024-04-27 16:55] szczcur: [replying to daax: "this is something i've seen recently, they mitigat..."]
have done a few diffs on the latest versions, they've merged some subsystems or are beginning to. they also updating windef and merging the various components there into a core module. lots of new attack surface is getting introduced.

[2024-04-27 21:53] daax: https://github.com/codyd51/uefirc
[Embed: GitHub - codyd51/uefirc: An IRC client in your motherboard]
An IRC client in your motherboard. Contribute to codyd51/uefirc development by creating an account on GitHub.