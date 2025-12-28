# January 2024 - Week 5
# Channel: #reverse-engineering
# Messages: 14

[2024-01-29 19:59] Ignotus: [replying to qwerty1423: "``File -> settings -> Build, Execution, Deployment..."]

[Attachments: image.png]

[2024-01-29 19:59] Ignotus: Is it not in community edition?

[2024-01-29 22:38] mrexodia: [replying to Glatcher: "how to gather call stack info during tracing (x64d..."]
Yeah this isn’t supported in the trace

[2024-01-29 22:38] mrexodia: In theory it can be recovered though, since you can search backwards for the call instructions

[2024-01-29 22:39] Glatcher: ow shiet its genious

[2024-01-29 22:39] Glatcher: I even didn't think about it

[2024-01-29 22:39] mrexodia: 🧠

[2024-01-29 22:39] mrexodia: Don’t ask me how to search

[2024-01-29 22:40] Glatcher: I was coding script which just prints stack every time eip changes

[2024-01-29 22:40] mrexodia: That’s not implemented I think <:harold:704245193016344596>

[2024-01-29 22:40] mrexodia: [replying to Glatcher: "I was coding script which just prints stack every ..."]
A plugin would be easier

[2024-01-31 19:34] Ignotus: Trying to get a decompiled code of a specific method [Java], which should not be that complex. Only ~200 lines of bytecode but none of the decompilers I tried were able to get it right. Only Krakatau was able to give me something but it's not functioning correctly. I'm using Bytecode Viewer and tried with the built in Procyon, CRF, JD-GUI, FernFlower and Krakatau. Are there any other decompilers I could try? Or plugins? Some of the decompilers have settings I could change, anything specific I could try? The method only does some number operations with arrays and a string iterator. I dont know Java bytecode that well so I'd like to try everything else before manually decompiling it 🙂

[2024-01-31 21:30] qwerty1423: [replying to Ignotus: "Trying to get a decompiled code of a specific meth..."]
JadX

[2024-01-31 21:30] qwerty1423: needs jre but its a good alternative