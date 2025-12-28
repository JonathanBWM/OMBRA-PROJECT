# September 2025 - Week 2
# Channel: #programming
# Messages: 151

[2025-09-10 19:20] Aj Topper: When loading my driver on Test mode the code works fine but when using mappers this line causes the driver to crash
pte_base = (idx + 0x1FFFE00ui64) << 39ui64;
https://github.com/MapleSwan/enum_real_dirbase
[Embed: GitHub - MapleSwan/enum_real_dirbase: 从MmPfnData中枚举进程...]
从MmPfnData中枚举进程和页目录基址. Contribute to MapleSwan/enum_real_dirbase development by creating an account on GitHub.

[2025-09-11 10:47] Matti: have you tried a debugger

[2025-09-11 10:48] Matti: well very nearly a rhetorical question I suppose going by the lack of info... so that would be my suggestion

[2025-09-11 10:48] Matti: use a debugger

[2025-09-11 10:59] Matti: [replying to selfprxvoked: "I guess I only use LLMs to rewrite something for m..."]
did you mean
> since [I'm] brazilian, it is a pretty complex language
or
> [brazilian is] a pretty complex language
if the latter, you should probably be aware that your native language is portuguese

[2025-09-11 11:20] Ghostpunisher: Hello everyone

[2025-09-11 12:19] estrellas: [replying to Matti: "did you mean
> since [I'm] brazilian, it is a pret..."]
is it though

[2025-09-11 12:23] Matti: [replying to estrellas: "is it though"]
fair question, only he can answer that really

[2025-09-11 12:24] Matti: but I'm pretty sure it's portuguese and not brazilian

[2025-09-11 12:24] Matti: fake portuguese if you prefer

[2025-09-11 12:38] selfprxvoked: Brazilian and portuguese are very different

[2025-09-11 12:38] selfprxvoked: We are more closely related to galician now rather than portuguese

[2025-09-11 12:42] Matti: OK! well again it's hard for me to disagree since I've never even been to brazil, but there's an awful lot of wikipedia articles to correct if so
e.g. <https://en.wikipedia.org/wiki/Languages_of_Brazil>
or just googling what language is spoken in brazil, and so on

[2025-09-11 12:43] Matti: I was only checking to make sure you weren't accidentally inventing a language

[2025-09-11 12:47] Tequila: [replying to Matti: "OK! well again it's hard for me to disagree since ..."]
98% of brazilians speak Portuguese as their prime language

[2025-09-11 12:47] Brit: https://tenor.com/view/rolblox-devinbooker-gif-18412462

[2025-09-11 12:49] estrellas: well it's called pt-br for some reason

[2025-09-11 13:09] selfprxvoked: [replying to estrellas: "well it's called pt-br for some reason"]
It's not that simple, pt-br has way too many words from the indigenous languages ​​of the original people of Brazil (mandioca, capivara, mingau, etc.), as well as words from the indigenous languages ​​of the African continent (Brazil has the largest black population in the world outside the African continent). Furthermore, we also conjugate verbs differently than all other lusophone languages ​​(from Angola, Portugal, Mozambique, etc.).
In short, yes, brazilian is obviously still portuguese, but it's so different (200 years apart) that you can call it "brazilian", just as you can call it "mexican" which derives from spanish, even though it's still spanish.

[2025-09-11 13:11] selfprxvoked: I'm not saying it is a new language tho, just saying that the dialects are really different from each other that you can refer to it differently 👍

[2025-09-11 13:21] selfprxvoked: [replying to Matti: "fake portuguese if you prefer"]
I guess you can call portuguese from Portugal a fake portuguese since we are 200m and they are only around 10m now?

[2025-09-11 13:22] selfprxvoked: otherwise you're going to call fake english from USA as well?

[2025-09-11 13:22] selfprxvoked: <:mmmm:904523247205351454>

[2025-09-11 13:25] Matti: [replying to selfprxvoked: "otherwise you're going to call fake english from U..."]
yes?

[2025-09-11 13:28] selfprxvoked: yeah then we agree

[2025-09-11 13:28] daax: [replying to selfprxvoked: "otherwise you're going to call fake english from U..."]
The US is barely literate, so yeah

[2025-09-11 13:34] Aj Topper: [replying to Matti: "well very nearly a rhetorical question I suppose g..."]
like winfbg shopwing error in that line bit shift is failing
[Attachments: image.png]

[2025-09-11 13:59] Matti: [replying to Aj Topper: "like winfbg shopwing error in that line bit shift ..."]
true, it is 'failing', but as you can see there's a pretty specific reason for the failure given by the debugger in the form of the bugcheck code and explanation
pte contents = `0900000148a4b121` is further evidence of the same: the PTE for the page is read only (bit 1 for W isn't set)

[2025-09-11 14:00] Matti: so, further questions:
\- what is at the VA in arg 1?
\- (probably not interesting) what is at the ""reserved"" address in arg 3? (this is a trap frame, dump with `dt nt!_KTRAP_FRAME xxx`)
\- how are you mapping this driver exactly? meaning the full source code preferably as this part is kind of complex

[2025-09-11 15:00] Aj Topper: [replying to Matti: "so, further questions:
\- what is at the VA in arg..."]
Why is it working when the driver is loaded in test mode by sc start, the mapper is based on ghostmapper

[2025-09-11 15:32] Matti: yes, that was the original question wasn't it

[2025-09-12 05:25] Pip: [replying to selfprxvoked: "It's not that simple, pt-br has way too many words..."]
Man what

[2025-09-12 05:31] Pip: Portuguese is Portuguese, whether it is spoken in Brasil, or in Portugal, the same way English is English, whether it is spoken in America, or England

[2025-09-12 08:06] dullard: [replying to Pip: "Portuguese is Portuguese, whether it is spoken in ..."]
But the language has differences 🤔 

It’s like saying Catalan is the same as Spanish or Canadian French is the same as French French 😂

[2025-09-12 08:06] dullard: They’re mostly the same but they are different

[2025-09-12 09:08] 0xatul: [replying to Pip: "Portuguese is Portuguese, whether it is spoken in ..."]
google what are dialects

[2025-09-12 14:32] Pip: [replying to 0xatul: "google what are dialects"]
I understand that there are different dialects, but different dialect doesn’t mean it is a different language

[2025-09-12 14:33] Pip: [replying to dullard: "But the language has differences 🤔 

It’s like say..."]
The same way English in England and English in America has differences, but it is still the same language nonetheless

[2025-09-12 14:34] Brit: You realize that there's no hard rules here right?

[2025-09-12 14:35] Brit: Swedish and Danish people speak different languages and yet understand one another

[2025-09-12 14:35] Pip: [replying to Brit: "Swedish and Danish people speak different language..."]
Same for Norwegian

[2025-09-12 14:35] Brit: simillarly Canotnese and Mandarian

[2025-09-12 14:35] Brit: is different enough that while both spoklen in china

[2025-09-12 14:35] Brit: people will not understand one another

[2025-09-12 14:36] Brit: despite being chinese dialects

[2025-09-12 14:37] Pip: [replying to Brit: "people will not understand one another"]
I see your point, but with Portuguese, it is still understood between Portugal and Brasil, there are most definitely differences, but it is most definitely still the same language

[2025-09-12 14:38] Pip: I’d say the most drastic difference would be the accent used, and the slang/phrases used, and how they are different in each country, the same way there is a noticeable difference between the accent and slang/phrases used in England vs America

[2025-09-12 14:38] dullard: How far do you want to go back, technically in europe we all speak the same language as it has origins from latin/greek

[2025-09-12 14:38] dullard: <:Kapp:543420634571735047>

[2025-09-12 14:39] Brit: got a bit culturally enriched with the moorish invasions

[2025-09-12 14:39] Brit: and by the hunnic tribes

[2025-09-12 14:39] Brit: see finnish

[2025-09-12 14:39] Brit: completely unrelated to any other eu language

[2025-09-12 14:39] Pip: Yeah, I have seen Finnish

[2025-09-12 14:40] Pip: Fucks with my head

[2025-09-12 14:41] Pip: [replying to dullard: "How far do you want to go back, technically in eur..."]
I mean, sure if you want to boil it all down and say we all speak the same language because they all have the same origins

[2025-09-12 14:43] Brit: That is also not accurate

[2025-09-12 14:44] Brit: There has been independent instantiation of language accross human history

[2025-09-12 14:44] Pip: Mhm

[2025-09-12 14:44] Brit: a really cute topic to research if you care about this is bedouin sign language

[2025-09-12 14:45] Brit: completely sepparate and independent from ASL

[2025-09-12 14:45] Brit: same with nicaraguan sign language

[2025-09-12 14:46] Pip: Idk, I just thought it was weird for someone to be so intentional about saying Brasilian Portuguese should be called Brasilian

[2025-09-12 14:46] Brit: it's just semiotics

[2025-09-12 14:46] Brit: idk why people get heated about it

[2025-09-12 14:47] Pip: [replying to Brit: "idk why people get heated about it"]
Because it’s wrong

[2025-09-12 14:47] Pip: Mexican isn’t a language

[2025-09-12 14:48] Pip: Brazilian isn’t either

[2025-09-12 14:51] Brit: is creole a language?

[2025-09-12 14:51] Brit: if so how

[2025-09-12 14:51] Brit: if not why not?

[2025-09-12 14:53] Pip: Idk about any of that

[2025-09-12 14:53] dullard: Which creole <:Kapp:543420634571735047>

[2025-09-12 14:53] dullard: bumbaclaaaat

[2025-09-12 14:53] dullard: Oh wait thats patois

[2025-09-12 14:53] dullard: or however you spell it

[2025-09-12 14:54] Pip: I just know that when growing up around Brazilian family and friends, it was always a point of humor when someone called the language “Brazilian”

[2025-09-12 14:56] Brit: [replying to dullard: "Which creole <:Kapp:543420634571735047>"]
exactly

[2025-09-12 15:28] avx: [replying to dullard: "Oh wait thats patois"]

[Attachments: YjkxMTc2YTUz.png]

[2025-09-12 18:15] Addison: I already posted this elsewhere but I'm really quite jazzed about this

[2025-09-12 18:15] Addison: Very degen Rust code

[2025-09-12 18:15] Addison: https://github.com/AFLplusplus/LibAFL/pull/3427

[2025-09-12 18:49] selfprxvoked: [replying to Pip: "Idk about any of that"]
Clearly

[2025-09-12 18:53] selfprxvoked: [replying to Pip: "I see your point, but with Portuguese, it is still..."]
We actually don't. As I said, it is pretty damn hard to understand Portugal's portuguese for us sometimes, requiring subtitles and deep explanation for a lot of words. To give you a better idea of ​​what I'm talking about, Portuguese from the north of Portugal is more difficult to understand than Galician, which is another different language.

As an example, you can call someone as "rapariga" in Portugal's portuguese meanwhile being called that in Brasil's portuguese will be offensive as f\*ck.

[2025-09-12 18:55] selfprxvoked: [replying to Pip: "Portuguese is Portuguese, whether it is spoken in ..."]
> In short, yes, brazilian is obviously still portuguese, but it's so different (200 years apart) that you can call it "brazilian", just as you can call it "mexican" which derives from spanish, even though it's still spanish.

What are you babbling about?

[2025-09-12 18:56] selfprxvoked: [replying to Brit: "is creole a language?"]
I've loved the creole example by the way. It is a perfect example of how complex that topic is and can be.

[2025-09-12 18:59] selfprxvoked: [replying to Brit: "idk why people get heated about it"]
I don't want to delve into this subject any further than I already have as it would be straying too far from the channel's topic, but the main reason is the demonstration of cultural independence of a people who are proud to be a unique nation separate from their colonizers.

[2025-09-12 20:50] Pip: [replying to selfprxvoked: "We actually don't. As I said, it is pretty damn ha..."]
Man your example is applicable for English as well, that doesn't make it not English

[2025-09-12 20:50] Pip: [replying to selfprxvoked: "> In short, yes, brazilian is obviously still port..."]
As I have already said, there are most definitely difference in slang and phrases, but that doesn't make it a different language

[2025-09-12 20:53] selfprxvoked: [replying to Pip: "As I have already said, there are most definitely ..."]
I'm not saying it is?

[2025-09-12 20:53] Addison: lol this is such a dumb convo

[2025-09-12 20:53] Pip: You are considering "Brazilian" another language no?

[2025-09-12 20:53] Addison: It actually is

[2025-09-12 20:53] Pip: F tier ragebait

[2025-09-12 20:53] Addison: To suggest that British vs American is comparable is crazy bro

[2025-09-12 20:55] Pip: In my experience, the difference between Brazilian Portuguese and Portuguese from Portugal is comparable to the difference between American English and English from England

[2025-09-12 21:00] selfprxvoked: [replying to Pip: "In my experience, the difference between Brazilian..."]
It is not, it is very far from different

[2025-09-12 21:00] Pip: [replying to selfprxvoked: "It is not, it is very far from different"]
idk, maybe just you then

[2025-09-12 21:00] selfprxvoked: ?

[2025-09-12 21:00] Pip: I don't struggle all that much when talking to people from Portugal

[2025-09-12 21:01] selfprxvoked: You need to study history to understand more of what was the colonizers approach in both situations, which is far from being close

[2025-09-12 21:01] Pip: The dialect/accent is the "biggest" hurdle when speaking to people from Portugal from me, but 98% of what they are saying is the same as what I am used to hearing

[2025-09-12 21:02] Pip: [replying to selfprxvoked: "You need to study history to understand more of wh..."]
The language stemmed from the language spoken in the northern part of the Kingdom of Galcia, in the County of Portugal no?

[2025-09-12 21:02] Pip: and started from vulgar latin

[2025-09-12 21:03] selfprxvoked: Nope, it is not "vulgar" latin, because "vulgar" in latin means "from the people" and not "vulgar" from the common sense.

[2025-09-12 21:05] Pip: when I say Vulgar Latin I am refering to the literal language called "Vulgar Latin"

[2025-09-12 21:05] selfprxvoked: Again, you're just using personal experience as confirmation bias.

[2025-09-12 21:05] selfprxvoked: [replying to Pip: "when I say Vulgar Latin I am refering to the liter..."]
There's no "Vulgar Latin" language, it was just Latin.

[2025-09-12 21:06] Pip: [replying to selfprxvoked: "Again, you're just using personal experience as co..."]
idk man, sure I am using my own personal experience with my frequent times in Brazil, with family, with friends, as well as reading through the origins of Portuguese out of interest

[2025-09-12 21:06] selfprxvoked: What you're referring to is a separation made by linguistics as a way to proper study the dialects...

[2025-09-12 21:06] Pip: maybe you are just bad at Portuguese?

[2025-09-12 21:06] Matti: [replying to Addison: "To suggest that British vs American is comparable ..."]
english vs american english isn't the best example to be fair

even in NL, some dialects (especially in brabant and limburg) share a lot of the traits he mentioned, like one side (or both sides mutually) not being able to understand the other at times because of pronunciation differences, some words having completely different meanings or not existing in the other dialect, and so on. it's not uncommon to see people from limburg on dutch TV, being subtitled in dutch(!)

[2025-09-12 21:06] Matti: but these are still all considered dutch

[2025-09-12 21:07] Matti: compare with frysian, which is an officially recognised second language in NL that is spoken only in the province of friesland. it is so far removed from dutch that it's completely unintelligible to me as someone who doesn't speak the language

[2025-09-12 21:07] Pip: [replying to Matti: "english vs american english isn't the best example..."]
Right, stark differences or not, they are still the same language

[2025-09-12 21:07] selfprxvoked: [replying to Pip: "maybe you are just bad at Portuguese?"]
Maybe you're just experienced enough to understand lusophone portuguese meanwhile the rest of the brazilians are not?

[2025-09-12 21:07] Pip: Definitely not the case considering I'm only conversationally fluent

[2025-09-12 21:08] Addison: [replying to Matti: "english vs american english isn't the best example..."]
Main difference between a dialect and a language is an army lol

[2025-09-12 21:08] Pip: People that say "Brazilian" is a language get laughed at in my experience

[2025-09-12 21:11] selfprxvoked: [replying to Pip: "People that say "Brazilian" is a language get laug..."]
Yeah, things are changing in Brasil, you can definitely look at the latest Brasil vs "Pernambuco em Pé" (aka Portugal) to see that we still have bitter memories and open wounds about our colonizers, which is the reason people are starting to refer to the brazilian portuguese as just brazilian.

[2025-09-12 21:11] Matti: [replying to Addison: "Main difference between a dialect and a language i..."]
I mean wouldn't americans *definitely* have their own language using this definition? <:kekw:904522300257345566>

[2025-09-12 21:12] Addison: Yeah well they agree that it is English

[2025-09-12 21:12] selfprxvoked: I think this topic was prolonged just enough to understand that it is complicated, right? We can finish being off topic now.

[2025-09-13 00:14] Dyno: 
<:dynoSuccess:314691591484866560> Found and purged 29 messages.

[2025-09-13 19:57] dullard: Dyno is thinking...

[2025-09-13 22:08] R1perXNX: Hello everyone, does anyone know why injecting a page fault exception for a user-mode page from a VM-exit handler causes a BSOD with IRQL_NOT_LESS_OR_EQUAL when the VM-exit is triggered by a read/write to the CR3 register, while other VM-exits allow to inject the pagefault successfully?

My goal is to trigger a page-in on a specific DLL (i think demand paging is on) so I can place EPT hooks, but I can’t inject a page fault if the VM-exit reason is CR3 access. I’ve been able to inject the page fault successfully by checking that CPL = 3 and avoid injecting otherwhise. Maybe Im missing something

[2025-09-13 22:27] R1perXNX: I think i can't generate a page fault from cpl 0 without BSOD even if its for usermode

[2025-09-13 22:48] Brit: yes

[2025-09-13 22:48] Brit: you are at a high irql when you vmexit on cr3 access, cannot pf a usermode page

[2025-09-13 23:36] Xits: is the irql another name for the apic processor priority class? Or are they different

[2025-09-13 23:39] R1perXNX: [replying to Brit: "you are at a high irql when you vmexit on cr3 acce..."]
Can I inject at < DISPATCH ?

[2025-09-13 23:39] R1perXNX: Or I must be at cpl 3

[2025-09-13 23:46] Brit: I think you have to be at cpl3 but I cant check rn

[2025-09-13 23:46] R1perXNX: I’ll check tomorrow

[2025-09-13 23:47] R1perXNX: Thanks 🙏🏻

[2025-09-14 12:31] pbgr: [replying to Xits: "is the irql another name for the apic processor pr..."]
irql's the os's mapping of the apics tpr iirc

[2025-09-14 12:44] donna🤯: [replying to Xits: "is the irql another name for the apic processor pr..."]
Its just an abstraction Windows uses, but its more or less the same thing

[2025-09-14 13:18] W4ZM: i have a question , if we call `SetUnhandledExceptionFilter` to register a `UnhandledExceptionFilter` if the process is being debugged , the exception handler will never be called even if the debugger passes back the exception to the program right ?

[2025-09-14 13:19] W4ZM: unlike VEH wich handles the exception if the debugger passes it back to debuggee

[2025-09-14 15:01] daax: [replying to Xits: "is the irql another name for the apic processor pr..."]
irql is windows’ name for cr8 which is aliased to the apic’s tpr @ offset 80h from apic base.

[2025-09-14 17:42] JustMagic: [replying to W4ZM: "i have a question , if we call `SetUnhandledExcept..."]
No, it will be called.

[2025-09-14 17:47] JustMagic: Both VEH and unhandled exception filter are a user-mode thing implemented by ntdll. If either of them are reached, the user-mode process has been dispatched the exception and is attempting to process it on its own.

[2025-09-14 18:50] UJ: [replying to Xits: "is the irql another name for the apic processor pr..."]
historically? no. 32 bit windows implemented irql in software. even windows on arm now, it might be implemented in software (i'll have to check). modern windows x64 windows, effectively yes (just movs to/from cr8) but as mentioned above, IRQL is a hal abstraction.

[2025-09-14 19:04] UJ: [replying to donna🤯: "Its just an abstraction Windows uses, but its more..."]
did you ever get apic virtualization to work in your hv in a bluepill manner (virtualizing apic for an already running guest)? I recall there was a pr to your hv from someone else that had more apic virt changes as well but that looked like ai slop (and its now gone so im guessing it was)?

[2025-09-14 19:18] W4ZM: [replying to JustMagic: "No, it will be called."]
when the exception occurs,  and  i run the debugger,  in the case of the VEH exception,  the handler gets executed but in the case of `UnhandledExceptionFilter` rip gets stuck where the exception occurred,  and the exception keeps happening , this means ithe handler didnt handle it (when being debugged) , i keep getting first chance then last chance

[2025-09-14 19:31] JustMagic: [replying to W4ZM: "when the exception occurs,  and  i run the debugge..."]
well then place a breakpoint on your handler and see if it gets called?

[2025-09-14 19:31] JustMagic: UnhandledExceptionFilter is just a SEH filter in `RtlUserThreadStart`

[2025-09-14 19:37] W4ZM: [replying to JustMagic: "well then place a breakpoint on your handler and s..."]
it didn't

[2025-09-14 19:43] W4ZM: (im using x64dbg)

[2025-09-14 20:29] JustMagic: [replying to W4ZM: "it didn't"]
`kernelbase!UnhandledExceptionFilter` does not invoke your registered unhandled exception filter if a debugger is present. Use `ntdll!RtlSetUnhandledExceptionFilter` to skip those checks and have no kernelbase in between or scyllahide to trick the process into thinking it has no debugger.

[2025-09-14 20:32] W4ZM: [replying to JustMagic: "`kernelbase!UnhandledExceptionFilter` does not inv..."]
in the case of `ntdll!RtlSetUnhandledExceptionFilter` will the handler be invoked when debugger is present ?

[2025-09-14 20:35] JustMagic: [replying to W4ZM: "in the case of `ntdll!RtlSetUnhandledExceptionFilt..."]
yes, `kernelbase!UnhandledExceptionFilter` is set using `ntdll!RtlSetUnhandledExceptionFilter`