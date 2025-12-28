# May 2024 - Week 5
# Channel: #programming
# Messages: 77

[2024-05-27 20:47] ash: for fun and exploration I am writing my own array-backed allocator and I am having a hard time figuring out alignment, because the standard doesn't specify it and it depends on implementation, given a function similar to calloc that takes size of element and number of elements (in my case: 
`alloc_with(Allocator* a, size_t size, size_t count)`
how can I determine their alignment correctly?

[2024-05-27 20:47] ash: is the alignment even tied to the size in any way as far as the standard is concerned? or would my best bet be to just align everything to `alignof(max_align_t)`?

[2024-05-27 20:49] ash: optionally let's say I don't care about the standard and its fine as long as it works just on linux, could I align it to just `size` in if it is smaller than `max_align_t` and to `max_align_t` otherwise?

[2024-05-28 02:01] ash: I guess I'll just align to `max_align_t` and call it a day but I'd still be grateful for further insight if anyone is more knowledgable about this than me

[2024-05-28 04:14] Torph: I'm not familiar with how the standard allocators work, but if you want to be super safe id just align to 0x10 boundaries. as far as I'm aware nothing requires alignment to boundaries larger than 128-bit? but i've also never heard of `max_align_t` or done much research in this area... so y'know, grain of salt

[2024-05-29 19:34] repnezz: how would you guys manage a pool allocation made in a create minifilter callback that needs to be used up until close?
It leaks during unload , probably because by the time I unload some allocations have been made for files that havent been closed

[2024-05-29 19:52] Matti: wdym you 'unload allocations'

[2024-05-29 19:53] Matti: oh wait, I think I get it after re-reading

[2024-05-29 19:54] Matti: isn't this part of the struct you pass to fltmgr when registering?

[2024-05-29 19:54] Matti: there's a table with function pointers for all kinds of events, including (IIRC) pre and post unload

[2024-05-29 19:55] Matti: I know this all sounds kinda vague but I'm trying to avoid opening the fltmgr msdn docs

[2024-05-29 19:58] repnezz: hmm yes you have the unload callback , but that means i need to somehow keep track of all allocations made (maybe in a list) and traverse the list on unlod to free each remaining allocation

its possible but i was hoping for a more s”built-in” way

[2024-05-29 20:00] repnezz: Like, since when using FltAllocatePool… you associate it with an instance , it would be cool that when the driver that owns the instance unloads the filter manager will free all allocations made by that instance , probably a stretch tho

[2024-05-29 20:00] repnezz: But ive been told to never underestimate the filter manager ….

[2024-05-29 20:01] Matti: I think you're supposed to/able to do this through InstanceTeardownStartCallback
> The InstanceTeardownStartCallback routine must:
> 
>     Call FltCompletePendedPreOperation for each I/O operation that was pended in the minifilter driver's preoperation callback routine to complete the operation or return control of the operation to the filter manager.
>     Not pend any new I/O operations. If the minifilter driver uses a callback data queue, it must call FltCbdqDisable to disable it.
>     Call FltCompletePendedPostOperation for each I/O operation that was pended in the minifilter driver's postoperation callback routine to return control of the operation to the filter manager.
> 
> The InstanceTeardownStartCallback routine can optionally do the following to allow the minifilter driver to unload as quickly as possible:
> 
>     Close any opened files.
>     Ensure that worker threads perform only the minimum necessary to complete processing of outstanding work items.
>     Call FltCancelIo to cancel any I/O operations that were initiated by the minifilter driver.
>     Stop queuing new work items.

[2024-05-29 20:02] Matti: then when InstanceTeardownCompleteCallback gets called, if you have any file handles that you opened yourself, you can close those tthere

[2024-05-29 20:02] Matti: <https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/fltkernel/nc-fltkernel-pflt_instance_teardown_callback>

[2024-05-29 20:02] Matti: so, this is an operation that can block

[2024-05-29 20:02] Matti: until the IO is actually completed

[2024-05-29 20:03] Matti: also

[2024-05-29 20:03] Matti: are you using driver verifier?

[2024-05-29 20:03] Matti: if not, definitely enable it and enable I/O verification

[2024-05-29 20:03] Matti: chances are good it'll literally just tell you what you're doing wrong

[2024-05-29 20:06] Matti: oh, also see FLTT_RELATED_OBJECTS - https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/fltkernel/ns-fltkernel-_flt_related_objects
[Embed: _FLT_RELATED_OBJECTS (fltkernel.h) - Windows drivers]
The FLT_RELATED_OBJECTS structure contains opaque pointers for the objects associated with an operation.

[2024-05-29 20:06] Matti: this is passsd to the QueryTeardownCallback and the TeaardownCallbacks

[2024-05-29 20:07] Windy Bug: [replying to repnezz: "hmm yes you have the unload callback , but that me..."]
It sounds like what you’d want is a FileObject context. you can attach the context to a file object on pre create and it will be automatically freed by the system in close . if you need to recursively free structure members you can set up a context cleanup callback

[2024-05-29 20:08] Matti: if you need the context, call FltGetContexts on the FLT_RELATED_OBJECTS

[2024-05-29 20:08] Matti: if you need the file object - it's in there if there is one associated witth the requestt

[2024-05-29 20:09] Matti: [replying to Windy Bug: "It sounds like what you’d want is a FileObject con..."]
yeah I was kinda assuming he was using contexts

[2024-05-29 20:10] Matti: but if not, that is the way to do it

[2024-05-29 20:11] Matti: in addition to using the callbacks I mentioned to then clean up any contexts if needed

[2024-05-29 20:11] Matti: [replying to repnezz: "hmm yes you have the unload callback , but that me..."]
so in other words, don't do this

[2024-05-29 20:15] Matti: > The FLT_RELATED_OBJECTS structure is allocated by the filter manager and contains opaque pointers for the objects associated with an I/O operation or an instance setup or teardown operation.
> 
> The contents of the FLT_RELATED_OBJECTS structure are set by the filter manager. Minifilter drivers cannot directly modify the contents of this structure. However, if a minifilter driver modifies the target instance or target file object for an I/O operation in the FLT_IO_PARAMETER_BLOCK structure for the operation, the filter manager modifies the value of the corresponding Instance or FileObject member of the FLT_RELATED_OBJECTS structure that is passed to lower minifilter drivers. For more information, see Modifying the Parameters for an I/O Operation.
> 
> A minifilter driver receives a pointer to an FLT_RELATED_OBJECTS structure as the FltObjects input parameter to the following callback routine types:
> 
> PFLT_POST_OPERATION_CALLBACK
> 
> PFLT_PRE_OPERATION_CALLBACK
> 
> PFLT_INSTANCE_SETUP_CALLBACK
> 
> PFLT_INSTANCE_QUERY_TEARDOWN_CALLBACK
> 
> PFLT_INSTANCE_TEARDOWN_CALLBACK

I'd say you can pretty much take this to mean that all of these callbacks should be implementetd

[2024-05-29 20:15] JustMagic: My dude just wants garbage collection in kernel

[2024-05-29 20:16] Matti: I mean, the urge to make a linked list of thousands of file object pointers is obviously strong...

[2024-05-29 20:16] Matti: but still, like don't do it

[2024-05-29 20:19] Matti: apart from being slow, it's also gonna be broken in some way that will cause constant BSODs for reasons like 'fltmgr already decided to do that for you because of X'
where X is obviously explained somewhere in the 2000 pages of fltmgr docs

[2024-05-29 20:24] Matti: https://github.com/microsoft/Windows-driver-samples/tree/main/filesys/miniFilter/ctx this is a very simple sample driver that I reallly recommend taking a look at
[Embed: Windows-driver-samples/filesys/miniFilter/ctx at main · microsoft/W...]
This repo contains driver samples prepared for use with Microsoft Visual Studio and the Windows Driver Kit (WDK). It contains both Universal Windows Driver and desktop-only driver samples. - micros...

[2024-05-29 20:25] Matti: if you look at the other samples in fs/minifilter, you'll see that actually almost all of them make use of contexts

[2024-05-29 20:26] Matti: any time anything needs to be stored about some file, other object, IRP...

[2024-05-29 20:28] Matti: last two link dumps:
<https://learn.microsoft.com/en-gb/windows-hardware/drivers/ifs/managing-contexts>
<https://learn.microsoft.com/en-gb/windows-hardware/drivers/ifs/managing-contexts-in-a-minifilter-driver>

[2024-05-29 20:32] Matti: note that contexts can be released, deleted and freed, and those are all different operations

[2024-05-29 20:32] Matti: not all of those should be manually done by  you

[2024-05-29 20:32] Matti: this table is surprisingly informative
[Attachments: image.png]

[2024-05-29 20:33] Matti: [replying to Matti: "are you using driver verifier?"]
finally... answer this question

[2024-05-29 20:34] Matti: I mean I already know the answer so I guess what I'm saying is, go enable it

[2024-05-29 20:35] repnezz: First of all thanks for all the info

And yes , that’s how I spotted the leak in the first place and it indicated allocations in create when running !verifier 3

[2024-05-29 20:35] Matti: hmm alright!

[2024-05-29 20:35] Matti: sorry for assuming

[2024-05-29 20:35] Matti: you get +1 rep for using verifier

[2024-05-29 20:40] repnezz: One last thing, should I just trust close to happen before I unload? (With the context being freed when the file object is torn down and refcount is 0)

Iirc the unload routine will just hang until all contexts are released , but what if some usermode service holds the file object forever ?

[2024-05-29 20:44] Matti: I'm not 100% sure of the answer to this tbh
but I think it is that: minifilter unloading does not necessarily require all of its file object references to be closed by every driver/process in the system
it only requires that your driver has closed what it has opened

[2024-05-29 20:46] Matti: the filter manager can detach your driver from an IRP's driver stack on the fly to adjust for the unload I'm pretty sure

[2024-05-29 20:47] repnezz: but I need the file objects I attached contexts to , to be closed  - so the contexts will get freed , and so will  my allocations  : )

[2024-05-29 20:49] repnezz: I think it would make most sense if it does hang until all file objects that have a context attached to them are torn down, or at least the contexts drop to 0 refcount

[2024-05-29 20:57] Matti: yes but like, you are able (or required really) to release a context that you got back from FltAllocateContext

[2024-05-29 20:57] Matti: that's what the callbacks are for

[2024-05-29 20:57] Matti: to allow you to do this at the appropriate time

[2024-05-29 20:58] Matti: which is probably before the unload callback

[2024-05-29 20:59] Matti: use the FltGetContexts API with the provided related objects parameter

[2024-05-29 20:59] Matti: then simply release it

[2024-05-29 21:00] Matti: then its ref count will be 0, and fltmgr will free its allocation

[2024-05-29 21:00] Matti: if you are allocating something manually in addition to this using e.g. ExAllocatePool, well then obviously free that first and then release the context

[2024-05-29 21:00] Matti: am I missing something?

[2024-05-29 21:01] Matti: idt a file object close is required anywhere to support unloading, again excepting any files you opened yourself

[2024-05-29 21:03] Windy Bug: [replying to repnezz: "One last thing, should I just trust close to happe..."]
You are overthinking this atp

[2024-05-29 21:03] Windy Bug: Its not that complicated

[2024-05-29 21:05] Windy Bug: you allocate a context , set it on an object , release the allocate reference . match every get with a release

[2024-05-29 21:06] Windy Bug: “Given that FltSetContext will attach a context to a data structure and add a reference count, what are the cases that will detach it? There are, in fact, four such cases.
The attached to system structure is about to go away. For example, when the file system calls FsRtlTeardownPer StreamContexts as part of tearing down the FCB, the Filter Manager will detach any attached stream contexts and dereference them.
The filter instance associated with the context is being detached.  Again taking the stream context example, during instance teardown after the InstanceTeardown callbacks have been made the filter manager will detach any stream contexts associated with this instance from their associated ADVANCED_FCB_HEADER and dereference them.
The minifilter itself detaches the context by calling FltDeleteContext or one of its variants.
The minifilter itself detaches the context by calling the appropriate FltSetXXXContext functions, and specifying FLT_SET_CONTEXT_REPLACE_ IF_EXISTS. In this case the old context is not dereferenced, which means it will not go away before the caller has a chance to inspect it. Therefore, the caller has to perform the dereference.”

[2024-05-29 21:06] Windy Bug: Filter manager handles it for you…

[2024-05-31 21:19] RedPawn (無意味さん): Are there any resources on advanced bitwise oneliners?  I'm doing something like regex golf, where I'm trying to do stuff with the least amount of operations possible.

[2024-05-31 21:37] roddux: [replying to RedPawn (無意味さん): "Are there any resources on advanced bitwise onelin..."]
🤔 i wouldn’t have thought bitwise operations was a big enough topic to have an “advanced” article

[2024-05-31 21:42] RedPawn (無意味さん): Advanced/tricks/hacks, whatever you want to call it.

[2024-05-31 21:43] RedPawn (無意味さん): I'm aware of this:
https://graphics.stanford.edu/~seander/bithacks.html

[2024-05-31 21:44] RedPawn (無意味さん): But a lot of the solutions use loops or conditions, which isn't allowed in the game i'm playing.

[2024-05-31 21:45] RedPawn (無意味さん): I've already solved them, I just want to improve my score.

[2024-05-31 22:13] fvrmatteo: [replying to RedPawn (無意味さん): "I've already solved them, I just want to improve m..."]
Have you considered attempting to synthesize a minimal solution via component-based synthesis? If you don't know what I'm talking about, you can take a look here: https://fitzgeraldnick.com/2020/01/13/synthesizing-loop-free-programs.html. If you are familiar with LLVM and can compile your one-liners to LLVM bitcode, then you can try applying Souper (https://github.com/google/souper) to it to obtain a minimal solution with the list of input components you specify in the query.
[Embed: GitHub - google/souper: A superoptimizer for LLVM IR]
A superoptimizer for LLVM IR. Contribute to google/souper development by creating an account on GitHub.