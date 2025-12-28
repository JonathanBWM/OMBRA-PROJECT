# March 2024 - Week 3
# Channel: #application-security
# Messages: 4

[2024-03-12 11:42] Timmy: https://youtu.be/9ZpDo7dWO4c
[Embed: EDR = Erase Data Remotely, By Cooking An Unforgettable (Byte) Signa...]
Endpoint security controls are the most essential tool for protecting computer systems from various malware threats. Most of them usually include several layers of detection modules. Among them is the

[2024-03-14 10:36] stuub: Incase we've got any penetration testers here, or any enthusiasts in Web App Pen Testing; I've developed a Python script to spin up a vulnerable web server, following the OWASP vulnerabilities and thought it may be useful to likeminded folk here :) 

All locally hosted, no docker images, no vm's, just Python (for now)

Current OWASP Vulnerabilities available for testing are:
Sensitive Data Exposure (HTTP Logins)
Broken Authentication (Login Page Bruteforcing)
Cross-Site Scripting (Reflective XSS)
Injection (Remote Code Execution)

https://github.com/Stuub/DoomBox/

Thanks <:fingerguns:691203085200261141>
[Embed: GitHub - Stuub/DoomBox]
Contribute to Stuub/DoomBox development by creating an account on GitHub.

[2024-03-14 20:18] stuub: Update:
Doombox is now a fully integrated CTF. 

Thanks to Docker, you can now go from web to root with a nice route aimed for beginners. Passing through broken authentication and remote code execution in web, to reverse shell and multi-level privilege escalation.

3 flags to find, do let me know if you find them! 

Any questions or feedback? drop a message here or in dm’s. Thanks <:pepelove1:836750164461879316>

[2024-03-15 02:14] stuub: Added it to a docker registry for easier access too - Here's the one liner to get it up :)
```└─$ docker run -p 80:80 -d docker.io/stuub/doomed:latest ```