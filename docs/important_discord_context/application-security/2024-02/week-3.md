# February 2024 - Week 3
# Channel: #application-security
# Messages: 72

[2024-02-14 02:14] stanzi: Would you guys be willing to take this market fit survey for me? My friend and I are kind of sort of building something atm.

https://t6epfju60dp.typeform.com/to/ZHbFbQbC
[Embed: Product Market Fit Survey for Hackercore]
Turn data collection into an experience with Typeform. Create beautiful online forms, surveys, quizzes, and so much more. Try it for FREE.

[2024-02-14 04:28] snowua: Got my first instruction to correctly virtualize for `mov eax, 1`

[2024-02-14 04:28] snowua: Before:
[Attachments: x64dbg_dtjZ6St747.png]

[2024-02-14 04:28] snowua: After:
[Attachments: x64dbg_anbn1YupLj.png]

[2024-02-14 04:30] snowua: 
[Attachments: image.png]

[2024-02-14 04:31] snowua: <:yara_lover:1148745271577157673>

[2024-02-14 16:52] Deleted User: nice but whats up with rsp :o

[2024-02-15 04:14] snowua: Rogue +1 in a handler that I typod

[2024-02-15 11:06] Deleted User: ah i see

[2024-02-16 06:13] Tarkev: Does it make sense to set up SSL between the frontend Docker container and the backend Docker container? Both containers are in the same virtual machine and on the same Docker network.

[2024-02-16 06:13] Tarkev: I have never heard of it but my colleauge thinks it is necessary.

[2024-02-16 11:09] unknowntrojan: sniffing an internal docker net'd require root access to the host at which point youre screwed anyways

[2024-02-16 11:10] unknowntrojan: but you cant go wrong with ssl

[2024-02-16 11:53] mrexodia: [replying to Tarkev: "Does it make sense to set up SSL between the front..."]
no it doesn't make sense imo

[2024-02-16 11:54] mrexodia: unless he's talking about using SSL client authentication (but even then it's dubious)

[2024-02-16 11:58] Tarkev: [replying to unknowntrojan: "but you cant go wrong with ssl"]
It just makes the communication between the containers slower, I guess?

[2024-02-16 11:58] unknowntrojan: yes

[2024-02-16 11:58] Tarkev: [replying to mrexodia: "unless he's talking about using SSL client authent..."]
Could you be more specific please?

[2024-02-16 11:58] unknowntrojan: but not really

[2024-02-16 11:58] unknowntrojan: aes is hardware accelerated

[2024-02-16 11:58] unknowntrojan: ¯\_(ツ)_/¯

[2024-02-16 11:59] unknowntrojan: [replying to Tarkev: "Could you be more specific please?"]
client authentication in ssl is uhhh
for example, you have a web server set up. it has a cert for clients to make a secured connection.
with client auth, the clients have their own certificates the server will validate

[2024-02-16 12:00] unknowntrojan: for example in k8s setups client auth is basically a requirement so that only nodes in the cluster can communicate with one another and no random person can just hop in

[2024-02-16 12:01] Tarkev: Oh thats interesting. Never heard of that either. My bad. Never touched k8s for a loong time.

[2024-02-16 12:02] unknowntrojan: with docker, those internal networks are based on virtual network adapters, no packets leave the system on a physical connection unless they are routed there

[2024-02-16 12:02] unknowntrojan: any packets routed to internal docker ip ranges will stay inside the system

[2024-02-16 12:04] unknowntrojan: although with network stacks, I don't trust anything because they are such a giant, complex system where a lot can go wrong

[2024-02-16 12:06] unknowntrojan: I know that's not particularly helpful but in general I don't use ssl in docker internal networks, if something is exploitable there I won't be the first to notice it, amazon will be

[2024-02-16 12:24] mrexodia: yeah in an internal docker network it makes 0 sense, you are just creating overhead encrypting something that no attacker will ever try to decrypt

[2024-02-16 13:29] unknowntrojan: ^

[2024-02-16 13:37] 25d6cfba-b039-4274-8472-2d2527cb: I'd probs at least support ssl and cert management between the services though. Building shit that relies on a certain system configuration (being on the same vm) might cause issues.

[2024-02-16 13:37] 25d6cfba-b039-4274-8472-2d2527cb: Say u want to deploy elsewhere etc.

[2024-02-16 13:39] unknowntrojan: in that case, a k8s or nomad/consul setup will most likely be the next step, where the security of those internal networks will be guaranteed nonetheless

[2024-02-16 13:39] unknowntrojan: but if not, good point

[2024-02-16 21:56] mrexodia: [replying to 25d6cfba-b039-4274-8472-2d2527cb: "Say u want to deploy elsewhere etc."]
imo, no

[2024-02-16 21:56] mrexodia: if you would deploy it elsewhere you would set up a private VPN and use a reverse proxy to expose whatever relevant parts to the internet

[2024-02-16 21:56] mrexodia: it's exactly the same threat model as far as I can see

[2024-02-16 21:57] mrexodia: the only exception would be if you go full retard and give every service a public IP <:harold:704245193016344596>

[2024-02-16 21:57] mrexodia: but then you have other problems

[2024-02-16 22:35] 25d6cfba-b039-4274-8472-2d2527cb: I mean I personally always configure services to work on a zero trust basis. No service makes an assumption about certain network topology or being on the same clusters etc. unless things are configured to be running on the literal same vm node and such. IaC is abstracted to such a level that things will work securely whether or not services are run on the same host, cluster or even network. Of course firewalls and site-to-site vpn configurations do apply but not really handled at the same level as the service software configuration.

[2024-02-16 22:40] 25d6cfba-b039-4274-8472-2d2527cb: > unless things are configured to be running on the literal same vm node and such
this part obviously encompasses things such as ssl termination being done with a reverse proxy on the same node etc. rather than handling it in the networked application unless there are reasons not to.

[2024-02-16 22:42] mrexodia: I don’t think this idea makes any sense, but you do you

[2024-02-16 22:42] mrexodia: Using self-signed SSL is equivalent to using HTTP anyway

[2024-02-16 22:43] mrexodia: And the great thing about HTTP is that you do not have to deal with this stuff on a per-app basis, just expose the port and done

[2024-02-16 22:45] 25d6cfba-b039-4274-8472-2d2527cb: I mean what is pki. By the same logic why bother authenticating between services if they are in a private network.

[2024-02-16 22:45] 25d6cfba-b039-4274-8472-2d2527cb: self managed secrets are equivalent to no secrets

[2024-02-16 22:47] mrexodia: [replying to 25d6cfba-b039-4274-8472-2d2527cb: "I mean what is pki. By the same logic why bother a..."]
HTTPS isn’t about authentication though (except client certs which nobody uses)

[2024-02-16 22:47] mrexodia: So that’s not the same logic

[2024-02-16 22:48] 25d6cfba-b039-4274-8472-2d2527cb: I mean a major part of it literally is about authenticating the place you are connecting to.

[2024-02-16 22:48] 25d6cfba-b039-4274-8472-2d2527cb: thats why you have CAs

[2024-02-16 22:49] mrexodia: Yeah sure you could set up your own shit there

[2024-02-16 22:49] mrexodia: But the attacker model doesn’t make sense, especially in Docker

[2024-02-16 22:51] mrexodia: You defend against someone impersonating your service on the same internal domain?

[2024-02-16 22:51] jvoisin: just use local sockets?

[2024-02-16 22:51] mrexodia: This means the attacker can already do everything

[2024-02-16 22:52] mrexodia: [replying to jvoisin: "just use local sockets?"]
Yeah, even better

[2024-02-16 22:59] 25d6cfba-b039-4274-8472-2d2527cb: I mean you can say it doesn't make sense. But zero-trust and pkis are a thing for a reason and it isn't just something I'm making up. Network perimeters shouldn't really be in consideration when implementing zero-trust properly.

[2024-02-16 23:00] mrexodia: zero-trust is a buzzword invented by cloud providers, nobody knows what it really means

[2024-02-16 23:01] mrexodia: but when you have a ✅ you should definitely ✅  it

[2024-02-16 23:02] mrexodia: I think you should look at the architecture of your system and make security decisions based on that

[2024-02-16 23:02] 25d6cfba-b039-4274-8472-2d2527cb: Eh not really. It just means authenticate everything.

[2024-02-16 23:02] mrexodia: Not do some bullshit just because someone said it's good practice

[2024-02-16 23:03] root: https://csrc.nist.gov/Pubs/sp/800/207/IPD
[Embed: NIST Special Publication (SP) 800-207 (Withdrawn), Zero Trust Archi...]
Zero Trust is the term for an evolving set of network security paradigms that move network defenses from wide network perimeters to narrowly focusing on individual or small groups of resources. A Zero

[2024-02-16 23:03] mrexodia: xD

[2024-02-16 23:08] root: I think it can be a concept to strive for while also being hijacked by marketing bullshit. They are not mutually exclusive

[2024-02-16 23:08] mrexodia: Yeah but still, does it make sense to use HTTPS for _transport_ here?

[2024-02-16 23:11] 25d6cfba-b039-4274-8472-2d2527cb: Here? No but like I said in the beginning is that I'd not a develop a system that relies on being the same docker host so I can ignore transport/auth security concerns. Makes it much easier when/if complexity increases and you need to change how your architecture needs changes. More nodes, more hosts, different locations etc.

[2024-02-16 23:13] mrexodia: Always good to design everything to be future proof

[2024-02-16 23:13] mrexodia: Gang of four thanks you 🙏

[2024-02-16 23:13] mrexodia: different hosts? yeet it in a VPN

[2024-02-16 23:13] mrexodia: ✅

[2024-02-16 23:14] mrexodia: you just achieved web scale