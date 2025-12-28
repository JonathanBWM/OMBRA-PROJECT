# March 2024 - Week 4
# Channel: #application-security
# Messages: 5

[2024-03-20 21:18] stuub: Sticky - AI Powered Privilege Escalation

As the name implies, i present to you my privilege escalation script that harnesses the *wisdom* of AI to automate your priv esc duties. Currently just a prototype I've had an "aha 💡" idea for, written in Python. 

For now, ONLY checks for interesting SUID binaries. 

Expect future developments, like getcap identification, cron jobs, writable files, etc.

The results are requested to OpenAI through curl (trying to be a more reliable request method, the alternative being python libraries which is likely no good for live environments)

The response from GPT will be wrote into a bash script which will be executable to gain elevated privileges.

Give it a whirl, leave feedback, make a pull request, idk, give head pats.

Thanks <:fingerguns:691203085200261141> 

https://github.com/Stuub/Sticky
[Embed: GitHub - Stuub/Sticky]
Contribute to Stuub/Sticky development by creating an account on GitHub.

[2024-03-20 21:30] Deleted User: <@680988534525657176>cant u use python request lib rofl

[2024-03-20 21:48] stuub: [replying to Deleted User: "<@680988534525657176>cant u use python request lib..."]
Yes lol, my laziness turned into developing it around the wrapper openai provides for curl

[2024-03-21 13:55] Deleted User: its like so easier to do with python request lib lol

[2024-03-22 11:07] stuub: [replying to Deleted User: "its like so easier to do with python request lib l..."]
There’s a million ways to hit a nail. I chose to do it this way during initial development, can always change it in future. However it’s not causing harm how It works currently :)