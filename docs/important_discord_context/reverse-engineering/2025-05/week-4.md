# May 2025 - Week 4
# Channel: #reverse-engineering
# Messages: 125

[2025-05-19 03:11] ruan: Is it not possible to the ** ida-pro-mcp server** https://github.com/mrexodia/ida-pro-mcp plugin 
write/edit code directly to the  PseudoCode View open on IDA?

[2025-05-19 09:14] ShekelMerchant: Question for yall. Does regular kdnet debugging sometimes just sh*t itself for no reason?

- ive literally done everything right and handled all loose ends (can go into specifics if anyone interested)
- but besides the point, why is it so f**king garbage, jesus]
- should I just the virtual com port to save time as seen in the pins?

https://learn.microsoft.com/en-us/windows-
hardware/drivers/debugger/setting-up-a-network-debugging-connection

[2025-05-19 10:03] truckdad: honestly no, i have always found it to work very well

[2025-05-19 10:06] truckdad: i regularly keep a connection going for days at a time and still can break in instantly without any trouble

[2025-05-19 10:07] truckdad: (and serve driver files over it with `.kdfiles` also over the course of days, which doesn’t require a break but does require an active connection)

[2025-05-19 10:09] ShekelMerchant: ok interesting

[2025-05-19 10:11] ShekelMerchant: so far ive tried adding my bus params to my dbgsettings but my windbg debugger on my other vm is still waiting to connect 🥲

[2025-05-19 10:14] truckdad: if you’re not relying on dhcp i always disable it in dbgsettings, because it gets rid of the 60-second pause where kdnet tries to find a dhcp server

[2025-05-19 10:14] truckdad: are you specifying an IP address in windbg?

[2025-05-19 10:14] truckdad: note that the kdnet stack is separate from the windows stack, so it won’t have the same IP address as what’s assigned in windows

[2025-05-19 10:14] truckdad: that’s the trap i see many people fall into

[2025-05-19 10:15] truckdad: unless you have a very particular reason to filter to a certain address, i’d just leave it blank

[2025-05-19 10:19] ShekelMerchant: [replying to truckdad: "note that the kdnet stack is separate from the win..."]
Disabling DHCP didnt work, but this may be it, gonna look into this. Thank you good sir 💯

[2025-05-19 10:21] truckdad: there’s not much to look into, just remove the IP address in windbg 😁

[2025-05-19 10:21] truckdad: it notes it’s not required

[2025-05-19 10:21] truckdad: kdnet will end up with a random link-local (169.254) address, but as long as there’s a direct layer 2 connection between kdnet and the debugger, that’s fine

[2025-05-19 10:21] truckdad: (i.e., the packets can be switched, but not routed)

[2025-05-19 10:22] truckdad: for a VM that should be completely fine

[2025-05-19 10:36] ShekelMerchant: [replying to truckdad: "there’s not much to look into, just remove the IP ..."]
I am already omitting my ip from the windbg. but i do have a static ip and gateway setup for both my vm's so im assuming maybe thats conflicting with the kd stack?
```cmd
"C:\Program Files (x86)\Windows Kits\10\Debuggers\x64\windbg.exe" ^
  -k net:port=50000,key=<****>
```

[2025-05-19 10:36] truckdad: no, it’s totally separate, there’s no conflict

[2025-05-19 10:37] truckdad: i’d take a pcap and see whether packets are making it out

[2025-05-19 10:38] truckdad: you should periodically see UDP on port 50000, if that’s what you also configured in your dbgsettings

[2025-05-19 10:39] truckdad: i always stick all my kdnets on their own host-level virtual adapter to keep the traffic out of normal captures and to elide the windows network configuration weirdness around enabling and disabling kdnet, but that’s not really required

[2025-05-19 10:55] ShekelMerchant: [replying to truckdad: "you should periodically see UDP on port 50000, if ..."]
Theres no udp packets coming out of the target vm and running kdnet.exe gives me this weird error, so im assuming i gotta go fix something else then

```cmd
> kdnet.exe
Network debugging is supported by this Microsoft Hypervisor Virtual Machine


KDNET transport initialization failed during a previous boot.  Status = 0xC0000182.
NIC hardware initialization failed.
```

[2025-05-19 10:56] ShekelMerchant: [replying to truckdad: "i always stick all my kdnets on their own host-lev..."]
i will try this, thanks

[2025-05-19 17:44] Matti: [replying to ShekelMerchant: "Theres no udp packets coming out of the target vm ..."]
that's `STATUS_DEVICE_CONFIGURATION_ERROR`

[2025-05-19 17:44] Matti: have you set up busparams for the KD NIC as you're supposed to?

[2025-05-19 17:45] Matti: show the output of `bcdedit /enum {dbgsettings}`

[2025-05-19 17:45] Matti: [replying to truckdad: "i always stick all my kdnets on their own host-lev..."]
this too

[2025-05-19 17:46] Matti: but that makes it even more important to set up busparams correctly so that the HAL knows which device to use for KD

[2025-05-19 20:41] ShekelMerchant: [replying to Matti: "have you set up busparams for the KD NIC as you're..."]
yup, 

```cmd
>bcdedit /enum {dbgsettings}
Debugger Settings
-----------------
identifier              {dbgsettings}
busparams               1.0.0
key                     *****************
debugtype               NET
hostip                  *********
port                    50000
dhcp                    No
```

[2025-05-19 20:55] truckdad: what VM software are you using?

[2025-05-19 20:57] truckdad: i see quite a few similar reports online under qemu

[2025-05-19 21:00] mrexodia: [replying to ShekelMerchant: "yup, 

```cmd
>bcdedit /enum {dbgsettings}
Debugge..."]
I don't think `*********` will work for the `hostip` honestly

[2025-05-19 21:27] Matti: [replying to ShekelMerchant: "yup, 

```cmd
>bcdedit /enum {dbgsettings}
Debugge..."]
are you sure `dhcp no` is what you want? YMMV, and I know others have reported differently, but I've always had to set this to yes for actual kdnet

[2025-05-19 21:27] Matti: `dhcp no` I've only ever used with kdnet USB-EEM and kdnet USB

[2025-05-19 22:00] truckdad: [replying to Matti: "are you sure `dhcp no` is what you want? YMMV, and..."]
are you trying to run kdnet over a routed connection? (rather than a switched connection)

[2025-05-19 22:01] truckdad: DHCP would be needed there (to obtain a routable IP address), but on a switched connection (including a simple adapter shared into a VM) a link-local address that kdnet chooses will suffice

[2025-05-19 22:01] truckdad: i specifically turn off DHCP in that situation (when i’m not running DHCP on that interface) to avoid the discovery phase that slows down boot

[2025-05-19 22:02] truckdad: (the black boot screen before the spinning wheel)

[2025-05-19 22:03] truckdad: if you’re running DHCP on the interface that’d work fine, but i like just assigning static addresses and not dealing with it 🤷‍♀️

[2025-05-19 22:08] Matti: [replying to truckdad: "are you trying to run kdnet over a routed connecti..."]
both actually

[2025-05-19 22:09] Matti: I've had to enable dhcp both in situations where it would logically make sense, and for switched connections

[2025-05-19 22:10] Matti: like I said though, some others I know have reported different results

[2025-05-19 22:10] truckdad: yeah idk, odd

[2025-05-19 22:10] truckdad: i run it exclusively without DHCP

[2025-05-19 22:10] Matti: the only thing that's for certain is that busparams is required

[2025-05-19 22:10] Matti: if it works without setting that, you're just getting by on luck

[2025-05-19 22:12] Matti: [replying to truckdad: "yeah idk, odd"]
what's even weirder to me is that some people need to use a debug cable for kdnet USB-EEM, whereas for me that does not work, and I need to use a standard A -> C supermarket cable

[2025-05-19 22:13] Matti: I have no logical explanation for that

[2025-05-19 22:13] Matti: and it took me a few hours to find out too...

[2025-05-19 22:14] truckdad: do you primarily use USB debugging in the event that something doesn't have a kdnet-compatible NIC?

[2025-05-19 22:14] truckdad: never found a need for it myself

[2025-05-19 22:15] Matti: yeah, so pretty much never

[2025-05-19 22:15] Matti: I tend to avoid it whenever possible

[2025-05-19 22:15] Matti: kdusb (both EHCI and XHCI) has **never** worked for me

[2025-05-19 22:15] Matti: on any system

[2025-05-19 22:15] Matti: literally not even once

[2025-05-19 22:16] Matti: kdnet USB-EEM is slightly better, as in it works on systems that don't have a serial port or PCIe NIC, but do have a USB NIC

[2025-05-19 22:16] Matti: and it does usually work

[2025-05-19 22:16] Matti: it's just not very fun to set up

[2025-05-19 22:17] Matti: so to clarify this must still be a NIC, unlike kdusb where any debug capable port is supposed to work

[2025-05-19 22:18] Matti: I've used it on my AMD 4700S, and on some ARM SoCs

[2025-05-19 22:18] Matti: for the ARM SoCs serial is usually also an option though

[2025-05-20 00:35] ruan: I'm debugging an application on `IDA Pro` which is very small 215kb, but it loads lots of dlls, I have previous decompiled them and saved as `.i64`
when live debugging the process how i could make IDA use/load the decompiled dlls instead of having to go 
`Debugger > Debugger Windows > Modules` right click on each module and then click "Analyze Module"

[2025-05-20 03:44] onyx: guys! do you have any working methods for automatic devirtualization?

[2025-05-20 10:22] onyx: Am I too naive? 😉

[2025-05-20 10:23] Wane: Yes I have it

[2025-05-20 10:23] Wane: https://tenor.com/view/gongyoo-squidgame-squidgame2-netflix-netflix-and-chill-gif-14396779702151948784

[2025-05-20 15:33] ruan: [replying to mrexodia: "I don't think `*********` will work for the `hosti..."]
how i could modify your ida mcp plugin to include the offset when analyzing a function?

[2025-05-20 15:33] ruan: when i ask it to analyze something it output:
```
{
  "address": "0x7ff7b73a0d20",
  "name": "sub_7FF7B73A0D20",
  "size": "0x2ef"
}
```

[2025-05-20 15:41] mrexodia: [replying to ruan: "how i could modify your ida mcp plugin to include ..."]
what is the use case?

[2025-05-20 15:42] ruan: to be able to find the function again when starting a new session
my .i64 got corrupted and i lost the saved functions, all i have now is this log in the vscode chat
but without the offset i cant find it again

[2025-05-20 15:52] mrexodia: 🤔

[2025-05-20 15:52] mrexodia: The `address` is the offset

[2025-05-20 15:55] ruan: i mean the RVA

[2025-05-20 16:37] mrexodia: I don’t think this is a helpful feature, it will just confuse the LLM.

[2025-05-20 16:37] mrexodia: Can add a convert_address function though

[2025-05-23 07:48] Ђинђић: Reverse enginnered a par of Internet Backgammon from windows xp

[2025-05-23 07:49] Ђинђић: Found like, one code that has string of chinese letters and symbols etc.

[2025-05-23 07:49] Ђинђић: Any way i can decode and understand them?

[2025-05-23 07:49] Ђинђић: Like using XOR key that i used in python server i posted on my github?

[2025-05-23 07:52] Ђинђић: I swear its something abput handshake @ client and server since i found there was some string that i have always recieved in that python server and with some decoding with or without xor resulted in first message being ZoNe (for me it actually said smth like b' Zone)

[2025-05-23 07:52] the horse: use flare-floss

[2025-05-23 07:53] Ђинђић: But now, trying to use it translated in C# it doesnt really translate but i need to find how other straings are defined or sent or something.

[2025-05-23 08:40] NSA, my beloved<3: Hey. IDA is able to determine class names and vftables, whereas I could not recover anything related to objects in Ghidra using the default demangling analysis functions, the DemangleAll script, hinting that IDA did not recover objects based on its demangler. Then I tried running the vtable, vftable and RTTI scripts, but all of them say that the binary does not contain a RTTI. Class names can be seen near the vftables in the file. Is it even possible for Ghidra to interpret classes?

[2025-05-23 09:07] x86matthew: [replying to Ђинђић: "I swear its something abput handshake @ client and..."]
https://github.com/selfrender/Windows-Server-2003/blob/master/enduser/zone_internetgames/src/shared/protocols/protocol.h
https://github.com/selfrender/Windows-Server-2003/blob/master/enduser/zone_internetgames/src/shared/protocols/bgmsgs.h
https://github.com/selfrender/Windows-Server-2003/tree/master/enduser/zone_internetgames/src/client/games/backgammon

[2025-05-23 09:10] Ђинђић: [replying to x86matthew: "https://github.com/selfrender/Windows-Server-2003/..."]
What do i do with that following my question?

[2025-05-23 09:16] Ђинђић: They might be useful for later development of stats but not for current handshake and client-status field

[2025-05-23 09:35] x86matthew: i don't think i understand your original question but you said you were reversing the winXP backgammon game

[2025-05-23 09:35] x86matthew: this is the full source and protocol information so it should have everything you need

[2025-05-23 14:12] Ђинђић: About app or server?
I am currently reversing test server to try and bring back the 'nostalgia' of those games. That is just how app worked, not the server really.... but thanks i guess

[2025-05-24 00:07] Windows2000Warrior**: [replying to Ђинђић: "About app or server?
I am currently reversing test..."]
That all i can do with c++ depend on your decompiled client files in github , not sure if that can be a good code for that type of server : https://mega.nz/file/HSByTQRQ#dDNzLmWkU-pKewFZvt_FFvtOh_irikwy1PhbpxBHobQ , i use also wireshark for packets view
[Attachments: copyImage.png]
[Embed: 99.65 MB file on MEGA]

[2025-05-24 04:17] Ђинђић: I need to know how to extract the best in decoding the first package which, somewhere early or in my github server wrote in python was so good that i could start it and get all i need and somehow extract and got message "b'zone"

[2025-05-24 04:18] Ђинђић: Now using C# and just established XOR key to work it all in decoding.

[2025-05-24 10:18] Shanks: [replying to x86matthew: "https://github.com/selfrender/Windows-Server-2003/..."]
He would face legal problems if he used the leaked source code, so I think he wanted to reverse engineer it.

[2025-05-24 21:40] NSA, my beloved<3: Hello. Quick question I could not find any answers for anywhere all day: IDA Pro succesfully identified a vftable and was able to determine the class name based on the RTTI Complete Object Locator address and the demangled name from the RTTI Type Descriptor. However the functions within the vftables have the standard names: `sub_...`. I ran Class Informer but the names stayed the same. Manually renaming each function and appending Class:: to them seems to achieve what I want, however this is a lot of manual work, not to mention that Class:: here is only used as a namespace, the virtual function was not actually put in a Class (or struct?). Could someone point me in the right direction regarding the assignment of functions within a vftable please?

[2025-05-24 22:03] truckdad: how do you know the function belongs to that class and not a base class?

[2025-05-24 22:33] the horse: [replying to NSA, my beloved<3: "Hello. Quick question I could not find any answers..."]
do you know which compiler the app you're reversing was compiled with?

[2025-05-24 22:34] the horse: RTTI is implemented differently and the format differs between compilers, maybe this info is not exposed with gcc for example

[2025-05-25 00:00] kian: to my knowledge, rtti only gives you information on the class like name and what it inherits. it doesn't give you the names of the functions unless they're exported or you have the pdb/map file

[2025-05-25 00:47] f00d: ^

[2025-05-25 02:03] ruan: how to "bookmark" a function on ghidra? like you can put some functions in folders on ida pro

[2025-05-25 05:54] Ђинђић: [replying to Shanks: "He would face legal problems if he used the leaked..."]
What legal problems? Microsoft want to track me down if i got a peice of that source code in my lil journey of reversing?

[2025-05-25 07:08] NSA, my beloved<3: [replying to truckdad: "how do you know the function belongs to that class..."]
The RTTI pointer is located before the first function pointer of the vftable. So whichever mangled name I find in the Type Descriptor within that structure, is the class that overwrote the base class's vftable pointer with the one we are at.

[2025-05-25 07:09] NSA, my beloved<3: [replying to the horse: "RTTI is implemented differently and the format dif..."]
RTTI is not a must, however this binary contains RTTIs.

[2025-05-25 07:10] NSA, my beloved<3: [replying to kian: "to my knowledge, rtti only gives you information o..."]
Yes, exactly. I am not looking for the original names of the virtual functions, just a way to mass assign them to the derived class they belong to.

[2025-05-25 08:11] Shanks: [replying to Ђинђић: "What legal problems? Microsoft want to track me do..."]
Well, I don't think that would happen, that was my belief about you.

[2025-05-25 11:00] truckdad: [replying to NSA, my beloved<3: "The RTTI pointer is located before the first funct..."]
sure, but the vtable can contain base class function pointers if the derived class didn’t override that particular function

[2025-05-25 11:02] NSA, my beloved<3: [replying to truckdad: "sure, but the vtable can contain base class functi..."]
Right, but I'll be able to see that and adjust the vftable functions accordingly.

[2025-05-25 11:02] truckdad: how would you plan on automating that?

[2025-05-25 11:03] NSA, my beloved<3: You tell me.

[2025-05-25 11:03] NSA, my beloved<3: Found this resource: https://reverseengineering.stackexchange.com/questions/11811/how-to-organize-vtables-in-ida-pro

[2025-05-25 11:04] NSA, my beloved<3: Started to re-create the classes and another class that should represent each class' vftable. However I am unable to force the disassembly to interpret a pointer array as a struct. So I can not assign the vftable that IDA found to the struct so it would translate function names.

[2025-05-25 11:18] NSA, my beloved<3: Well, I guess the struct is set for that region, it just doesn't inherint the function names from within the user-defined vftable struct.

[2025-05-25 15:50] ruan: [replying to mrexodia: "what is the use case?"]
is it possible to the agent have access to more than one ida session?
i have two different dlls open on ida and i would like to it have context of both

[2025-05-25 15:50] mrexodia: [replying to ruan: "is it possible to the agent have access to more th..."]
Yes, just put another mcp server in your config. But you should expect it to behave poorly

[2025-05-25 15:53] ruan: [replying to mrexodia: "Yes, just put another mcp server in your config. B..."]
on the second ida instance when i try to use the plugin to start a new mcp server i get: [MCP] Error: Port 13337 is already in use

[2025-05-25 16:07] mrexodia: Ah right, you have to use the idalib headless server

[2025-05-25 16:13] ruan: [replying to mrexodia: "Yes, just put another mcp server in your config. B..."]
which config?
```json
{
  "servers": {
    "ida-mcp-server": {
      "url": "http://127.0.0.1:8744/sse",
      "type": "sse"
    }
  }
}
```i have it this way on vscode

[2025-05-25 16:18] mrexodia: Yeah add another server and name them for the executable you want to analyze

[2025-05-25 23:39] diversenok: Is there a way to start user services on demand?

[2025-05-25 23:40] diversenok: i.e., I can create a `SERVICE_USER_SERVICE`-typed service. On logon, SCM will use it to create `SERVICE_USERSERVICE_INSTANCE`

[2025-05-25 23:41] diversenok: As far as I understand, only user service instances are startable; user services are just templates

[2025-05-25 23:42] diversenok: So is there a way to create and start user service instances on demand, without doing logon?