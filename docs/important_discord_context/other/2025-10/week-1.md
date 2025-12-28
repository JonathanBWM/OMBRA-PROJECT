# October 2025 - Week 1
# Messages: 47

[2025-10-01 07:40] ozempics winner: does anyone have a list of EAC's deployment/product ids?

[2025-10-01 17:04] the.dude: [replying to ozempics winner: "does anyone have a list of EAC's deployment/produc..."]
are u talking about the ids to download eac modules from the url ? you can just brute force them and count up, u get a response or not from api but years i go i did this cant rember correctly. but then there will be some invalid stuff in there but its easy to clean out of with some code then u have mostly valid ids with there names so on.

[2025-10-01 17:04] the.dude: there some provider already that do this idk

[2025-10-01 17:05] ozempics winner: i've already compiled a little list based of like top 20 most played eac-protected free steam games

[2025-10-01 17:05] ozempics winner: 
[Attachments: Settings.json]

[2025-10-01 17:05] ozempics winner: if anyone needs them ^

[2025-10-01 17:05] the.dude: jeh u can just iteratr from 0 - 100000000 idk as ids then u get responses

[2025-10-01 17:05] the.dude: idk rly how but u will figure i did this years ago

[2025-10-01 17:05] the.dude: but u wont get games list

[2025-10-01 17:05] the.dude: like rust

[2025-10-01 17:05] the.dude: bcs its other eac version

[2025-10-01 17:06] the.dude: [replying to the.dude: "jeh u can just iteratr from 0 - 100000000 idk as i..."]
funyn thing is u dont even get rate limited idk

[2025-10-01 17:06] ozempics winner: [replying to the.dude: "funyn thing is u dont even get rate limited idk"]
yeah that's the case for many anti-cheats

[2025-10-01 17:06] ozempics winner: faceit had the same vulnerability......

[2025-10-01 17:06] the.dude: wait let me check chat historyu

[2025-10-01 17:06] ozempics winner: they're quite quick at patching it tho

[2025-10-01 17:06] ozempics winner: i've reported it to them

[2025-10-01 17:06] the.dude: gonna send u service from mate

[2025-10-01 17:06] ozempics winner: fixed within like a day or two

[2025-10-01 17:07] the.dude: https://hera.dword.xyz/
[Embed: Hera — Anti-Cheat Monitoring Made Simple]
Stay informed on Easy Anti-Cheat, BattlEye, and Riot Vanguard changes in real time. Hera posts, transparent updates to your Discord.

[2025-10-01 17:08] ozempics winner: [replying to the.dude: "https://hera.dword.xyz/"]
dayum

[2025-10-01 17:08] ozempics winner: so someone has already done it before me

[2025-10-01 17:08] ozempics winner: ;c

[2025-10-01 17:09] ozempics winner: i have way more games tho ;p

[2025-10-01 17:09] the.dude: xD

[2025-10-01 17:09] the.dude: jeh i had like 3-4 k or so

[2025-10-01 17:10] ozempics winner: [replying to the.dude: "jeh i had like 3-4 k or so"]
dayum

[2025-10-01 17:10] ozempics winner: i have like 30 rn

[2025-10-01 17:10] the.dude: as said

[2025-10-01 17:10] ozempics winner: not anywhere near 4k 😭

[2025-10-01 17:10] the.dude: u can iterate them u get responses with name

[2025-10-01 17:10] the.dude: if the id exists

[2025-10-01 17:10] the.dude: except few crap data

[2025-10-01 17:10] ozempics winner: [replying to the.dude: "u can iterate them u get responses with name"]
the id's work differently after the acqqusition though

[2025-10-01 17:11] ozempics winner: used to be:

``https://download.eac-cdn.com/api/v1/games/{game_id}/client/{system}/download``

now it is:

``https://modules-cdn.eac-prod.on.epicgames.com/modules/{product_id}/{deployment_id}/{system}``

[2025-10-01 17:12] ozempics winner: and product / deployment id's are some kinds of hashes:

```json
"productid"                                        : "5dcd88f4e2094a698ebffa43438edc33",
"deploymentid"                                    : "47a5a1b2e0f64748a96777920ad97fbd"
```

[2025-10-01 17:14] the.dude: jeh ok well this makes this more complicated 😄

[2025-10-01 17:14] ozempics winner: [replying to the.dude: "jeh ok well this makes this more complicated 😄"]
yeah you gotta download the settings.json file ;c

[2025-10-01 17:14] ozempics winner: and the deployment id's change sometimes

[2025-10-01 17:14] the.dude: <:kekw:904522300257345566>

[2025-10-01 17:14] ozempics winner: i've automated it but still takes ages + can't download it for some paid games i don't own

[2025-10-01 17:14] the.dude: might be way u get that deploy hash somehow

[2025-10-01 17:15] the.dude: idk i have totally given up on that years ago 😄

[2025-10-01 17:15] the.dude: gl

[2025-10-01 17:15] ozempics winner: tyty

[2025-10-01 17:15] ozempics winner: ❤️

[2025-10-02 06:24] dwordxyz: [replying to ozempics winner: "i have way more games tho ;p"]
Yeah, I’m too lazy to push the latest update with more games, and I had no idea people would even use that bot in the first place <:KEKW:913723148086702120>