# August 2025 - Week 2
# Channel: #programming
# Messages: 42

[2025-08-05 20:22] daax: 
[Attachments: m7i3htuvp1hf1.jpg]

[2025-08-06 15:41] diversenok: How do I undecorate names like `__FIVectorView_1_Windows__CInternal__CStateRepository__CFileTypeAssociation` into `IVectorView<Windows::Internal::StateRepository::FileTypeAssociation>`?

[2025-08-06 15:42] diversenok: It looks like `UnDecorateSymbolName` uses a different scheme

[2025-08-06 16:18] 0xatul: this is good enough https://github.com/microsoft/llvm/blob/master/lib/Demangle/MicrosoftDemangle.cpp imo
[Embed: llvm/lib/Demangle/MicrosoftDemangle.cpp at master · microsoft/llvm]
Fork of the LLVM Compiler Infrastructure. Contribute to microsoft/llvm development by creating an account on GitHub.

[2025-08-06 16:21] diversenok: I also don't think it's the same format

[2025-08-06 16:22] 0xatul: :pain:

[2025-08-06 17:04] Nats: [replying to diversenok: "It looks like `UnDecorateSymbolName` uses a differ..."]
Thats for traditional MSVC C++ name mangling, not WinRT metadata names

[2025-08-06 17:05] Nats: What you’re looking at is WinRT’s ABI stable naming convention for generic types

[2025-08-06 21:05] diversenok: Is there some helper API to translate them back by any chance?

[2025-08-06 21:06] diversenok: I only saw it done manually in OleViewDotNet

[2025-08-06 21:06] diversenok: https://github.com/tyranid/oleviewdotnet/blob/main/OleViewDotNet/Utilities/WinRTNameUtils.cs#L105
[Embed: oleviewdotnet/OleViewDotNet/Utilities/WinRTNameUtils.cs at main · ...]
A .net OLE/COM viewer and inspector to merge functionality of OleView and Test Container - tyranid/oleviewdotnet

[2025-08-07 09:47] Nats: [replying to diversenok: "I only saw it done manually in OleViewDotNet"]
yeah thats pretty much the state of things

[2025-08-07 09:47] Nats: theres not official Win API that does this reverse translation for you

[2025-08-07 09:47] Nats: microsoft just didnt provide one

[2025-08-07 13:25] diversenok: `combase!WinRT::Metadata::Marshaling::DefaultMetadataFormatStringSourceHelper::UnmangleIdlName` does exactly what I need

[2025-08-07 13:26] diversenok: It's not exported, but I can reproduce its behavior

[2025-08-07 13:29] 0xatul: Not exported, yikes

[2025-08-07 15:01] Brit: Oh no I have to parse a pdb to call it woe is me

[2025-08-07 15:03] diversenok: I honestly hate when people write software that relies on downloading and parsing PDBs on the end machine

[2025-08-07 15:05] diversenok: Even though hardcoding offsets/syscall numbers/etc. is bad, this is even worse

[2025-08-07 15:14] Brit: [replying to diversenok: "I honestly hate when people write software that re..."]
Not that I disagree with this particular case but there's good reasons not to export things that you expect will change

[2025-08-08 14:28] ml: Hi, does anyone have any clue how to execute a TLS callback in a VMP-protected DLL when using manual mapping?

[2025-08-08 14:58] Timmy: https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#tls-callback-functions
[Embed: PE Format - Win32 apps]
This specification describes the structure of executable (image) files and object files under the Windows family of operating systems. These files are referred to as Portable Executable (PE) and Commo

[2025-08-08 14:59] ml: [replying to Timmy: "https://learn.microsoft.com/en-us/windows/win32/de..."]
thx

[2025-08-08 15:00] ml: [replying to Timmy: "https://learn.microsoft.com/en-us/windows/win32/de..."]
I want to clarify that I managed to execute the TLS callback on an unprotected DLL, but the problem arises when the DLL is protected

[2025-08-08 15:07] Timmy: then idk

[2025-08-08 15:10] ml: [replying to Timmy: "then idk"]
Ok, no problem, last question. I read what you sent me and noticed:

DLL_THREAD_ATTACH
A new thread has been created. This notification is sent for all but the first thread.
DLL_THREAD_DETACH
A thread is about to be terminated. This notification is sent for all but the first thread.

So, we need to call the TLS callback again with DLL_THREAD_ATTACH or DLL_THREAD_DETACH whenever a thread is created or destroyed during the DLL life in the target process (it’s written on the website, just to be sure lol)

[2025-08-08 15:12] Timmy: yup

[2025-08-08 15:12] ml: [replying to Timmy: "yup"]
Thanks, that's what I wasn't doing

[2025-08-08 15:21] x86matthew: [replying to ml: "Ok, no problem, last question. I read what you sen..."]
it must also be called by the new thread itself of course

[2025-08-08 15:29] ml: [replying to x86matthew: "it must also be called by the new thread itself of..."]
I expected that, but how can it be done?

[2025-08-08 15:37] selfprxvoked: [replying to ml: "I expected that, but how can it be done?"]
You'll need to include the TLS from your mapped dll in the Loader (which is located in the PEB iirc), basically emulating the windows loader and letting it do the job of calling the tls callbacks for you when new threads are created

[2025-08-08 15:39] selfprxvoked: you can take a look at this code as an example: <https://github.com/DarthTon/Blackbone>

[2025-08-08 15:41] selfprxvoked: but looks like it doesn't support `DLL_THREAD_ATTACH` and `DLL_THREAD_DETACH` based on what it's written in the README.md 🤔

[2025-08-08 15:44] ml: [replying to selfprxvoked: "You'll need to include the TLS from your mapped dl..."]
If I do that, my DLL will appear in the loaded modules list and be removed when it's unloaded, right?

[2025-08-08 15:46] selfprxvoked: well, if you're emulating the loader, yes, it should

[2025-08-08 15:58] ml: [replying to selfprxvoked: "well, if you're emulating the loader, yes, it shou..."]
And if I don't emulate the loader, how do I call the TLS callbacks?

[2025-08-08 16:02] selfprxvoked: [replying to ml: "And if I don't emulate the loader, how do I call t..."]
The TLS callbacks should be called by the Loader, so you'll need to include your TLS in the Loader

[2025-08-08 16:04] ml: [replying to selfprxvoked: "The TLS callbacks should be called by the Loader, ..."]
How could I do that?

[2025-08-08 16:08] selfprxvoked: [replying to ml: "How could I do that?"]
Take a look at `LdrpHandleTlsData`

[2025-08-08 16:13] selfprxvoked: either that or you do manually like in the DarthTon example

[2025-08-09 21:54] elias: did someone here work with WaveRT drivers before?