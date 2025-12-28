# April 2024 - Week 1
# Channel: #programming
# Messages: 27

[2024-04-01 13:36] daax: [replying to Deleted User: "for example bitfields are completely ignored"]
optimization has caused weird undefined behavior with bitfields when using msvc cl for me in the past as well, I would say triple check your code for UB elsewhere in your program. bitfields shouldn’t be completely ignored though, but msvc will try to fit bitfields into the underlying type and sometimes wastes whatever left over space

[2024-04-01 13:37] daax: you should double check your bitfields though to make sure they don’t cross the type boundary and the rest of the code for UB because UB elsewhere can lead to UB everywhere

[2024-04-01 15:36] cc_pz: any way to turn off win11 manipulation protection with powershell code or a file that wont get detected by defender

[2024-04-01 15:39] rin: Are you talking about execution policy?

[2024-04-01 15:39] rin: I usually just turn off defender but you can add exceptions also

[2024-04-01 16:05] cc_pz: 
[Attachments: image.png]

[2024-04-01 16:06] cc_pz: to disable this with a command or script

[2024-04-01 16:07] cc_pz: like this command but it wont work unless manipulationprotection is off
Set-MpPreference -DisableRealtimeMonitoring $true

[2024-04-01 16:44] snowua: Is it possible to setup a variadic template function so that you still get intellisense for the constructors of a type? For instance make_shared does this and you still get code completion which shows all the constructor information for that type. 

But if you try to do the setup a similarly templates function which returns new T(std::forward<T>(args)…) you won’t get anything.

On the surface level it makes sense because how could code completion predict what overloads to show. But how does make_shared get this functionality? Would this be some kind of IDE built in exception or is it something I’m missing

[2024-04-01 16:44] toasts: something fishy going on

[2024-04-01 16:46] toasts: [replying to cc_pz: "like this command but it wont work unless manipula..."]
is this to help you sell those cheap fortnite skins you advertise 🥶

[2024-04-01 16:52] Timmy: [replying to snowua: "Is it possible to setup a variadic template functi..."]
very probable that its just hardcoded IDE intellisense

[2024-04-01 16:53] snowua: Yeah i guessed so. Unfortunate

[2024-04-01 16:53] Timmy: jetbrains and vs do that a lot

[2024-04-01 16:54] Timmy: idk about clangd, cuz I cant get that to work

[2024-04-01 20:32] cc_pz: [replying to toasts: "is this to help you sell those cheap fortnite skin..."]
nah i resell from g2g where i find cheap accounts this is just something i do on my free time as i study cyber security

[2024-04-03 22:39] abu: [replying to cc_pz: ""]
obfuscated windows defender?

[2024-04-03 22:40] cc_pz: ye i was wondering if it was possible in powershell to turn it off with some kind of command

[2024-04-03 22:48] abu: idk of any powershell way, i use "sordum" to disable mine

[2024-04-03 22:48] cc_pz: ye u have to download that and run it as admin

[2024-04-03 22:48] cc_pz: then disable it in the sordum application

[2024-04-05 01:27] brymko: forcetailcall

[2024-04-05 08:31] contificate: Well, it must also be a tail call in the first place, subject to other restrictions.

[2024-04-05 08:32] contificate: If brymko is talking about LLVM's `musttail`, he'll be sad to learn it doesn't force anything.

[2024-04-05 08:33] contificate: I think what you're really asking is whether or not GCC will make provisions to emulate a `call` instruction? I don't believe such a thing exists, although it's probably equivalent to jumping with arguments if you modified the target to `add rsp, 8` immediately.

[2024-04-05 08:34] contificate: You can still provide arguments if you use inline assembly, although I'm not going to pretend that inline assembly exists for x64 under MSVC, which is so common to use here.

[2024-04-05 08:48] brymko: [replying to contificate: "If brymko is talking about LLVM's `musttail`, he'l..."]
i am sad