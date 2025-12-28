# June 2024 - Week 5
# Channel: #programming
# Messages: 19

[2024-06-24 20:57] SYAZ: Is it possible to obtain the actual buffer memory address of a named pipe when a process connects to it, in order to directly read from or write to it? For example 123.exe has \Device\NamedPipe\Local\Example and I want to read from it without creating connection to the named pipe myself.

[2024-06-24 21:54] diversenok: Not via a public API

[2024-06-24 21:54] diversenok: My guess is the buffer is probably not even static and instead gets allocated on demand when there is data to store

[2024-06-25 23:15] 0x208D9: any good resources to learn about Qt with cpp?

[2024-06-26 09:40] mrexodia: [replying to 0x208D9: "any good resources to learn about Qt with cpp?"]
Docs are pretty good

[2024-06-26 09:42] 0x208D9: [replying to mrexodia: "Docs are pretty good"]
i mean the api changes pretty heavily and its kinda hard to migrate my app from qt4 to qt6

[2024-06-26 09:42] 0x208D9: thats what i was mostly looking for

[2024-06-26 09:46] mrexodia: I would do Qt4 -> Qt5 first and then start thinking about Qt6

[2024-06-26 09:46] mrexodia: honestly the API didn't change that much

[2024-06-26 09:46] mrexodia: https://wiki.qt.io/Transition_from_Qt_4.x_to_Qt5
[Embed: Transition from Qt 4.x to Qt5]

[2024-06-26 09:46] mrexodia: https://www.kdab.com/porting-from-qt-4-to-qt-5/
[Embed: Porting from Qt 4 to Qt 5]
Porting from Qt 4 to Qt 5 is intentionally easy. There has been a conscious effort throughout the development of Qt 5 so far to maintain source compatibility compared to Qt 4. Unlike the port from Qt 

[2024-06-26 09:47] mrexodia: https://doc.qt.io/qt-6/portingguide.html
[Embed: Porting to Qt 6 | Qt 6.7]
Guide for porting Qt 5 applications to Qt 6.

[2024-06-26 09:47] mrexodia: https://www.qt.io/blog/references-and-hints-about-porting-from-qt5-to-qt6
[Embed: A Collection of References and Hints about Porting from Qt 5 to Qt 6]
This blogpost should help all users considering to migrate their development from Qt5 to Qt6. It provides a selection of references to Qt documentation as well as various community resources

[2024-06-27 10:26] 0x208D9: ^^ Thankssssss

[2024-06-30 13:01] elias: How is the VMProtect 'rolling encryption' performed at transpile time? Walking the entire code and all possible branches?

[2024-06-30 13:43] contificate: Do keys cross block boundaries?

[2024-06-30 13:44] contificate: If not, it's likely that the initial key is always statically known (the address of the first virtual instruction in a block) and is only valid for a straightline sequence (the block) it governs over.

[2024-06-30 13:45] contificate: It would be more involved - but not impossible - to have some key method that ensures control flow integrity by factoring in predecessors.

[2024-06-30 16:29] mrexodia: [replying to elias: "How is the VMProtect 'rolling encryption' performe..."]
iirc it resets at a jump