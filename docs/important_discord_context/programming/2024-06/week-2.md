# June 2024 - Week 2
# Channel: #programming
# Messages: 9

[2024-06-03 19:37] Matti: [replying to acro: "<@148095953742725120> hello, I'm trying to disable..."]
well that obviously isn't great
still, could you make an issue on the efiguard repository on github instead where this belongs?

[2024-06-03 19:38] Matti: especially including your exact ntoskrnl.exe version number

[2024-06-03 19:41] Matti: note that still doesn't guarantee I'll look into this and/or fix it anytime soon, it just means I won't instantly forget about it again (well I mean I will, but the issue will still be there so I'll also be reminded again sometime after forgetting, and so on until the end of time or maaaybe until I fix it)

[2024-06-03 19:41] acro: alright, thank you

[2024-06-07 11:14] vendor: i really hope im missing a compiler flag here and it's not that msvc's InstCombine pass sucks balls

[2024-06-07 11:14] vendor: <https://godbolt.org/z/ET43doY3K>

[2024-06-07 11:16] vendor: (it gets even worse if you remove the intrinsic <https://godbolt.org/z/bd3PMbsdW>)

[2024-06-09 13:56] ariv3: Does anyone know how to use exceptions in mapped Windows drivers?  I know that in user mode I can add an exception table with the RtlAddFunctionTable function, but how to do it in kernel mode?

[2024-06-09 14:02] mrexodia: [replying to ariv3: "Does anyone know how to use exceptions in mapped W..."]
You cannot, at least not with a PatchGuard bypass