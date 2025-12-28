# May 2024 - Week 3
# Channel: #🔗-research-and-links
# Messages: 8

[2024-05-16 16:58] daax: why are yall deleting messages so often?

[2024-05-16 16:59] daax: <@758032611078701277> <@735210887186481304>

[2024-05-16 16:59] five: huh me?

[2024-05-16 16:59] five: i didnt delete anything

[2024-05-16 16:59] ash: I guess someone did cause they felt like the video and conversation here didnt count as research links??? maybe?

[2024-05-16 17:01] daax: <@&835614130811306014> please do not delete relevant conversations about links/research, or related to the channel.

[2024-05-16 17:07] Torph: [replying to daax: "<@758032611078701277> <@735210887186481304>"]
🤷 wasn't me

[2024-05-16 23:36] stuub: Muraid - Exploiting SQLi in Mura & Masa CMS (CVE-2024-32640)

One of a few new PoC's dropped since my last post here. This one is for automating the vulnerability detection and exploitation of Mura and Masa powered web servers!

Taking leverage of the `contenthistid` HTML Query Parameter in the `_api/json/v1/default` endpoint. Appending an escape sequence `%5c'` to the value stored in the query parameter, allowing us to execute our own SQL payloads.

Integrated with Ghauri for the automation of exploitation on any target vulnerable. 

Enjoy <:pepelove1:836750164461879316> 

https://github.com/Stuub/CVE-2024-32640-SQLI-MuraCMS
[Embed: GitHub - Stuub/CVE-2024-32640-SQLI-MuraCMS: CVE-2024-32640 | Automa...]
CVE-2024-32640 | Automated SQLi Exploitation PoC. Contribute to Stuub/CVE-2024-32640-SQLI-MuraCMS development by creating an account on GitHub.