# January 2024 - Week 2
# Channel: #programming
# Messages: 43

[2024-01-09 10:11] luci4: Could someone recommend me some resources for learning UEFI programming on Windows? The material I found is pretty old, so I doubt it's still applicable.

[2024-01-09 10:11] luci4: Thanks!

[2024-01-09 10:19] mrexodia: [replying to luci4: "Could someone recommend me some resources for lear..."]
It is still applicable, UEFI doesn't really change much since it's baked into motherboards.

[2024-01-09 10:19] mrexodia: This article has a few links with additional information though: https://secret.club/2022/08/29/bootkitting-windows-sandbox.html
[Embed: Bootkitting Windows Sandbox]
Introduction & Motivation Windows Sandbox is a feature that Microsoft added to Windows back in May 2019. As Microsoft puts it: Windows Sandbox provides a lightweight desktop environment to safely run 

[2024-01-09 10:22] luci4: [replying to mrexodia: "This article has a few links with additional infor..."]
Woah that's so cool. Looks like the ultimate persistence method. Thanks a lot!

[2024-01-09 10:23] mrexodia: [replying to luci4: "Woah that's so cool. Looks like the ultimate persi..."]
It's super easy to detect if you know what you're looking for

[2024-01-09 10:25] luci4: [replying to mrexodia: "It's super easy to detect if you know what you're ..."]
I assumed so, since most popular techniques are easily detected (which is why red team/covert ops require constant R&D)

[2024-01-09 10:27] brymko: covert ops for a rootkit

[2024-01-09 10:27] brymko: <a:peperun:585548014395850772>

[2024-01-09 10:27] mrexodia: That red team rootkit hella fire

[2024-01-09 10:28] brymko: im gonna throw myself outa the window

[2024-01-09 10:28] mrexodia: [replying to brymko: "im gonna throw myself outa the window"]
remember to do it from a high enough floor

[2024-01-09 10:28] luci4: [replying to brymko: "covert ops for a rootkit"]
I meant popular techniques in general

[2024-01-09 10:30] luci4: "It's on MITRE, therefore it's easy to detect" doesn't sound ridiculous to me 🤷‍♂️

[2024-01-09 10:32] brymko: nah like the term covert ops is such a joke in it

[2024-01-09 10:33] luci4: [replying to brymko: "nah like the term covert ops is such a joke in it"]
Yeah, you're right, lol. Should've said... black ops 😎

[2024-01-09 10:33] asz: lol

[2024-01-09 10:33] luci4: Where is that edgy picture when I need it

[2024-01-09 10:33] luci4: 
[Attachments: apps.32777.64998372950229933.77155980-5b6a-4296-88e2-959fda591dee.jpg]

[2024-01-09 10:33] luci4: found it

[2024-01-09 10:36] dullard: [replying to luci4: "Could someone recommend me some resources for lear..."]
<:what:940916054396395521>

[2024-01-09 10:38] vendor: [replying to luci4: "I assumed so, since most popular techniques are ea..."]
man please never use a UEFI bootkit in a red team operation 🤣

[2024-01-09 10:42] vendor: [replying to mrexodia: "This article has a few links with additional infor..."]
i forgot about this, wasn't there a video of it somewhere?

[2024-01-09 10:42] vendor: i can't find it on the github or blog

[2024-01-09 10:44] mrexodia: [replying to vendor: "i forgot about this, wasn't there a video of it so..."]
A video about what? Everything is in the article

[2024-01-09 10:44] vendor: i remember seeing a video where someone automated a build step in visual studio so it launched the sandbox and started debugging their driver. it was very cool.

[2024-01-09 10:48] mrexodia: It certainly wasn't me

[2024-01-09 10:52] asz: i did such scrshot once

[2024-01-10 03:57] Torph: [replying to vendor: "man please never use a UEFI bootkit in a red team ..."]
why? is it just excessive or easy to detect or something?

[2024-01-10 06:30] brymko: there is literally 0 reason todo so and youre fucking with recoverability of livee systems

[2024-01-10 07:20] dullard: Stability/reliability moreso

[2024-01-10 15:16] daax: [replying to Torph: "why? is it just excessive or easy to detect or som..."]
if it’s some corpo red team whatever then yeah you shouldn’t need one tbh

[2024-01-10 21:43] Torph: [replying to brymko: "there is literally 0 reason todo so and youre fuck..."]
oh ok that makes sense

[2024-01-10 23:53] qwerty1423: what is the purpose of "0xF" in the second bracket?
```static char mmnoip8[][0xF] = {.....```

[2024-01-10 23:54] qwerty1423: is it the size of the elements in this array? or its defining the 16th element of something?

[2024-01-10 23:56] Torph: I *think* that means it's an array of `char` arrays with size`0xF` each

[2024-01-11 09:24] vendor: [replying to qwerty1423: "what is the purpose of "0xF" in the second bracket..."]
array of 0xF size arrays. the first size can be omitted because the compiler can infer the size from the initialisation list but the size of the nested array can’t be inferred i guess.

[2024-01-12 02:08] donna🤯: (HOW)

[2024-01-12 02:10] brymko: rwong channel

[2024-01-13 22:07] bocor: hey so i recently got comissioned to do a web site for a family friend. i run a blog myself but its only a hugo static site. anyone have advice or recommendations for building a more 'modern' site with all those lovely js plugins everyone seems to be using?

[2024-01-14 00:55] 25d6cfba-b039-4274-8472-2d2527cb: Definitely depends on what the specs for the site are.

[2024-01-14 01:55] bocor: hardware specs or design wise?

[2024-01-14 01:57] bocor: its not gonna receive much traffic. basically just a portfolio site for his work. hosting it on the same vps my blogs on. 2 older xeon cores w/ 2GB ram