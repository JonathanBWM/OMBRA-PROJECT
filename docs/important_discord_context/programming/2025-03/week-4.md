# March 2025 - Week 4
# Channel: #programming
# Messages: 35

[2025-03-23 08:55] rin: can someone link an example of a loader that properly handles tls

[2025-03-23 09:30] the horse: blackbone was pretty decent no?

[2025-03-23 12:17] mrexodia: [replying to rin: "can someone link an example of a loader that prope..."]
Not sure what this means, but it’s pretty easy with mbedtls (IXWebSocket works out of the box)

[2025-03-23 12:18] truckdad: i would venture to say it’s probably thread local storage

[2025-03-23 12:18] truckdad: but i had the same thought in my mind when i first saw it lol

[2025-03-23 12:19] mrexodia: Ahhh lol

[2025-03-23 14:09] Matti: [replying to rin: "can someone link an example of a loader that prope..."]
let me know if you find one

[2025-03-23 14:09] Matti: IMO not even windows handles TLS properly

[2025-03-23 14:10] Matti: see http://www.nynaeve.net/?p=190 for the 8 page explanation on how it is hackplemented
[Embed: Thread Local Storage, part 8: Wrap-up]

[2025-03-23 14:11] Matti: pay attention to the number of varieties there are, and how they all have pretty huge downsides

[2025-03-23 14:13] Matti: this is the only time in my life I've used TLS for something
<https://github.com/x64dbg/ScyllaHide/blob/master/HookLibrary/Tls.h>

[2025-03-23 14:15] daax: [replying to Matti: "let me know if you find one"]
If there’s one that’s public sitting on github, that would be useful to most, but I doubt it.

[2025-03-23 14:15] x86matthew: just store your values in debug registers like some programs in the 90s did <:mmmm:904523247205351454>

[2025-03-23 14:16] Matti: [replying to daax: "If there’s one that’s public sitting on github, th..."]
I hope you're not referring to mine <:lmao3d:611917482105765918>

[2025-03-23 14:16] Matti: I mean, someone's

[2025-03-23 14:16] Matti: I deny writing that code

[2025-03-23 14:17] Matti: btw, the gross new TLS implementation in the loader since vista is the main reason why `/Zc:threadSafeInit-` is such an insanely good flag if you're using MSVC with C++

[2025-03-23 14:20] Matti: things like function local statics get kilobytes of initializer code added to them

[2025-03-23 14:20] Matti: solution: don't use thread local storage

[2025-03-23 14:20] Matti: and let the compiler know

[2025-03-23 14:28] daax: [replying to Matti: "I hope you're not referring to mine <:lmao3d:61191..."]
no no not yours lol. im thinking he's looking for a basic manual mapping implementation (tls cb exec + index mgmt for static/dynamic tls) there's tons of examples that will function as long as edge cases don't crop up; but for a complete implementation i don't think he'll find one that's public.

[2025-03-23 14:29] Matti: I agree

[2025-03-23 14:29] daax: [replying to x86matthew: "just store your values in debug registers like som..."]
that's a special kind of hell

[2025-03-23 14:29] Matti: in fact I strongly doubt any loader but MS's implements TLS "correctly"

[2025-03-23 14:29] Matti: correctly being defined as the way the NT loader implements TLS

[2025-03-23 14:30] daax: [replying to Matti: "in fact I strongly doubt any loader but MS's imple..."]
lol, but what happens if you pasted from them <:Kappa:794707301436358686> cause why would you do it without doing exactly what the NT loader does?

[2025-03-23 14:31] Matti: that's a good question

[2025-03-23 14:31] Matti: usually the answer is because people make mistakes in their code

[2025-03-23 14:31] Matti: or overlook things in the NT loader

[2025-03-23 14:33] Matti: I don't have nearly as strong an opinion on this as I do on usage of TLS in general btw

[2025-03-23 14:33] Matti: because it is poorly implemented in NT, the best use of TLS is no use of TLS

[2025-03-23 14:35] Matti: [replying to Matti: "btw, the gross new TLS implementation in the loade..."]
this flag is also a requirement for restoring XP compatibility, just to give another example

[2025-03-23 14:35] Matti: meaning for a binary that does not even use TLS at all

[2025-03-23 14:36] Matti: the CRT changes made for this broke XP support

[2025-03-23 14:37] Matti: by default anyway, unless you append that flag to undo them