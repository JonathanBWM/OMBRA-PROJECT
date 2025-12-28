# August 2025 - Week 3
# Channel: #reverse-engineering
# Messages: 20

[2025-08-11 21:00] Lyssa: piss off

[2025-08-11 21:05] Xyrem: <@&835614130811306014>

[2025-08-11 21:05] Xyrem: <:mmmm:904523247205351454>

[2025-08-11 21:06] Lyssa: 
[Attachments: wO1yhHFm.png]

[2025-08-11 21:07] Lyssa: "oh pls help me cheat in valorant pls do this for me for free and I might give the thing to you if you're nice ......"

[2025-08-11 21:08] Matti: [replying to Xyrem: "<@&835614130811306014>"]
thanks

[2025-08-11 21:11] brymko: might've been a good gig for <@839216728008687666> he's recently become jobless

[2025-08-11 21:12] snowua: i hear that <@543396717614333962> is unemployed so this might be a great opportunity for him

[2025-08-11 21:14] the horse: what about justas?

[2025-08-11 21:15] Xyrem: <@733017505072808116> can u help bro out

[2025-08-12 04:23] grb: anybody tried debugging a VM's secure kernel with LiveCloudKd? Im using Hyper-V for my VM, kernel debugging enabled, VBS enabled, TPM enabled, but secure boot is disabled, the host machine also have kernel debugging enabled, i start the VM up, but this is what the LiveCloudKd looks like. From the virtual machines list there, it already looks wrong, i followed the tutorial from here
https://windows-internals.com/secure-kernel-research-with-livecloudkd/
[Attachments: image.png]
[Embed: Secure Kernel Research with LiveCloudKd]

[2025-08-12 19:27] abu: One zero day please

[2025-08-14 16:08] 0xboga: Hey. I have a VM setup of an endpoint running certain malware and it’s CNC . The malware sends a TCP request to the CNC and I want to write a “mock server” running on the endpoint to spoof the reply. Heard of mitmproxy, pydivert amd others.
Practically what I want is that any request from the malware to a specific dest port will go through my python code for inspection, and I’d be able to either answer the tcp request myself, drop it, or forward it to the server based on the contents of the request.
Which tool would be best suited for this?

[2025-08-14 16:18] cinder: [replying to 0xboga: "Hey. I have a VM setup of an endpoint running cert..."]
are you using windows or linux?
is the malware in question trying to contact a CNC server through a domain name?

[2025-08-14 16:19] cinder: if so, consider using the hosts file to force the domain to resolve to the IP of your choosing

[2025-08-14 16:24] cinder: implying of course that the malware in question does not roll it's own DNS resolver which may bypass the hosts file

[2025-08-14 19:46] 0xboga: [replying to cinder: "are you using windows or linux?
is the malware in ..."]
Windows

[2025-08-14 19:47] 0xboga: [replying to cinder: "implying of course that the malware in question do..."]
DNS won’t work, need to proxy based on dest port somehow

[2025-08-14 20:03] cinder: [replying to 0xboga: "DNS won’t work, need to proxy based on dest port s..."]
pydivert could be what you need, your use case is described within the README file

[2025-08-14 20:05] cinder: however I suggest considering setting up something like pfsense or openwrt and handling the redirection in there through firewall rules. pfsense is what I would go with