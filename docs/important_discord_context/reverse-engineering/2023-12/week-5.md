# December 2023 - Week 5
# Channel: #reverse-engineering
# Messages: 25

[2023-12-26 20:56] Adonis: Hello guys

[2023-12-26 20:56] Adonis: sorry if the question is dumb

[2023-12-26 20:57] Adonis: but if i have a packed executable that has anti debug all those measures in place

[2023-12-26 20:57] Adonis: and it uses the "WriteFile" function that i want to hook

[2023-12-26 20:57] Adonis: how would i go about doing that if i can't debug the very application

[2023-12-26 20:59] brymko: inject dll

[2023-12-26 21:00] Adonis: Good idea

[2023-12-26 21:00] Adonis: thanks

[2023-12-26 21:25] Adonis: not sure if vmp is the problem but the dll doesn't want to get injected, tried all possible methods

[2023-12-26 21:27] brymko: skill issue

[2023-12-26 21:28] brymko: you can do weird tricks such as start with debugger

[2023-12-26 21:28] brymko: inject dll

[2023-12-26 21:28] brymko: deattach

[2023-12-27 18:30] rase.: When you guys started, did you exclusively reverse with assembly only? I feel like I'm missing out and lacking a strong base because I'm always reaching for F5

[2023-12-27 18:32] naci: no

[2023-12-27 18:59] mibho: may be hot take but nothing wrong with defaulting to f5 (as long as can recognize obvious pseudocode errors) <:pepeShrug:928287319604211732>

[2023-12-27 19:00] mibho: is a big prob if u relying on it to be accurate/correct tho

[2023-12-27 19:28] rase.: I feel like the parenthesis require a somewhat strong base

[2023-12-27 19:47] mibho: [replying to rase.: "I feel like the parenthesis require a somewhat str..."]
100%

[2023-12-27 21:18] mibho: ive been sittin on some research (trial vmp <a:very_sadge:875510137379516426> ) ive done for this but no time so havent written but from what i found: 

(ONLY applies to VMP 3.6 demo )

paid version obviously more sophisticated so theres def additional steps but

1) parse the vm setup routine (fn where all regs pushed then addr for respective bytecode is decrypted)
2) extract encrypted addrs (can make sig for push addr call instructions then verify valid by calculating offset to respective call address) 
3) use addrs from step 2 with parsed vm setup fn to calculate next location of bytecode

[2023-12-27 21:19] mibho: dont know whats the mechanism to determine order in which push call pair should be called

[2023-12-27 21:21] mibho: also demo and paid has same instructions for unpacking portion

[2023-12-27 23:34] mibho: demo one i think static unpacking is doable (depends on how to determine order of push addr call)

[2023-12-27 23:35] mibho: me hoping to look into it after i finish x64dbg trace analyzer

[2023-12-28 08:05] sariaki: Not quite sure what your question is, but symbolic execution is static and should be possible with some packers