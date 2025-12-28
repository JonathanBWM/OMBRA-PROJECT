# June 2024 - Week 1
# Channel: #reverse-engineering
# Messages: 15

[2024-06-01 01:19] hxm: [replying to mrexodia: "nice work?"]
haha.

[2024-06-01 17:12] serlok🔍: Hi, I am new to reversing windows programs and would like to try and solve a problem while learning re. I have a laptop with a control centre that is extremely bloated and loads unnecessary ads, and it takes a lot of time to get to the settings panel to change my fan speed to max or auto. 

I wanted to ask if this is a good target to start with while learning reversing, and if it would be simple for a beginner to try. I have some experience with binary exploitation. I just want to know if y'all think this is a good idea to start with. I have these following assumptions in my mind that I would like confirmed or debunked: 
1. I probably have to find a function that loads some value to a running process that changes fan speed, and that it wouldnt be too obfuscated or hidden (because I think there is no need for them to obfuscate) 
2. There wouldn't be much problem with experimenting and it wont break my computer messing with fan controls (except if it overheats from my experiments which is understandable)

[2024-06-01 18:26] dullard: [replying to serlok🔍: "Hi, I am new to reversing windows programs and wou..."]
Sure yeah, sounds like a good place to do some exploring, 

it’s very likely a driver which does the actual fan controller stuff and a UM process which does the talking to the driver (you’ll likely find it’s a vuln driver too 😂)

[2024-06-02 05:25] abu: Hey! When it comes to using KASLR on windows, what does it actually do? Looking at the addresses in process hacker it doesn't seem to be randomizing anything. Is this normal behavior?
[Attachments: image.png]

[2024-06-02 06:13] JustMagic: [replying to abu: "Hey! When it comes to using KASLR on windows, what..."]
Did you do a proper reboot? Just clicking shut down won't do a full restart

[2024-06-02 06:27] abu: [replying to JustMagic: "Did you do a proper reboot? Just clicking shut dow..."]
Uhhhhh. I did like 4 restarts by clicking restart. Then shutdown my computer, then powered it back on. This is what is looks like now:
[Attachments: image.png]

[2024-06-02 06:27] abu: [replying to JustMagic: "Did you do a proper reboot? Just clicking shut dow..."]
Is a proper reboot like unplugging computer?

[2024-06-02 06:28] abu: Btw could you send me a picture of what it looks like enabled on someones computer (If you have it on right now that is)

[2024-06-02 06:28] JustMagic: [replying to abu: "Uhhhhh. I did like 4 restarts by clicking restart...."]
I mean the addresses are not the same

[2024-06-02 06:28] JustMagic: I'm not sure what you're expecting to see

[2024-06-02 06:33] abu: Tbh I thought that more executables  would be allocated at a random address like win32k in the picture below:
[Attachments: image.png]

[2024-06-02 06:37] JustMagic: [replying to abu: "Tbh I thought that more executables  would be allo..."]
There are VA spaces for different allocations

[2024-06-02 06:38] JustMagic: You won't have completely random addresses

[2024-06-02 06:39] JustMagic: https://www.vergiliusproject.com/kernels/x64/Windows%2011/23H2%20(2023%20Update)/_MI_SYSTEM_VA_TYPE
[Embed: Vergilius Project]
Take a look into the depths of Windows kernels and reveal more than 60000 undocumented structures.

[2024-06-02 06:39] abu: [replying to JustMagic: "You won't have completely random addresses"]
alright thanks 🙂