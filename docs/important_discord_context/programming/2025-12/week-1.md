# December 2025 - Week 1
# Channel: #programming
# Messages: 167

[2025-12-01 18:53] Xits: Has anyone tried/enjoyed using c++ modules?

[2025-12-01 18:54] Xits: I’m experiencing template hell compile times and looking for solutions

[2025-12-01 18:54] Xits: I’m even considering learning rust that’s how bad it is

[2025-12-01 18:56] Addison: you should just learn rust bro

[2025-12-01 18:58] Xits: yeah I know this template bullshit is crazy to need to deal with in 2025

[2025-12-01 19:11] Brit: Rust compile times are also something

[2025-12-01 19:11] Brit: To be fair

[2025-12-01 19:12] Brit: And if you care about things like bin size...

[2025-12-01 19:19] qw3rty01: neither of those are really an issue

[2025-12-01 19:19] Brit: You say this, but it is actually really annoying in my use case

[2025-12-01 19:20] qw3rty01: compile times are only long with a lot of dependencies, and bin size is just inverse of how easy you want things to be in the language

[2025-12-01 19:20] brymko: all you need is brainrot compile

[2025-12-01 19:20] brymko: shows you 1 tiktok until it finsihes compiling

[2025-12-01 19:20] brymko: really a non-issue

[2025-12-01 19:27] Brit: [replying to qw3rty01: "compile times are only long with a lot of dependen..."]
Compare an equivalent zig implementation of something for bin size

[2025-12-01 19:28] Brit: Ends up with about as much code

[2025-12-01 19:28] Brit: But a 10x bin size diff

[2025-12-01 19:28] Brit: I like rust

[2025-12-01 19:28] Brit: Its just an area it can Improve

[2025-12-01 19:28] Brit: Imo

[2025-12-01 19:29] qw3rty01: I don't really see that, zig has a lot more things you need to be explicit with

[2025-12-01 19:29] qw3rty01: and rust choses usability over binary size, but if you need binary size, you can also make it as small as you need

[2025-12-01 19:33] qw3rty01: that's one of the reasons rust works really well for embedded development, you can get the binary size to be extremely minimal

[2025-12-01 19:33] qw3rty01: but when it comes to C++, imo the biggest benefit rust has is proper generics

[2025-12-01 19:34] qw3rty01: ducktyping is pretty frustrating to deal with in a compiled language

[2025-12-01 19:42] Xits: Is there any decent project using c++ modules yet? Seems like a very good feature yet I’ve never seen it used??

[2025-12-01 20:05] ImagineHaxing: this is my rust code
```rs
fn get_module_handle_replacement(sz_module_name: String) -> Result<*mut HMODULE, String> {
    let p_peb: *const PEB;
    unsafe {
        asm!(
            "mov {}, gs:[0x60]",
                out(reg) p_peb,
        );
    }
    let peb: PEB = unsafe { *p_peb };

    let p_peb_ldr_data = peb.Ldr;
    let peb_ldr_data: PEB_LDR_DATA = unsafe { *p_peb_ldr_data };
    let in_memory_order_module_list: LIST_ENTRY = peb_ldr_data.InMemoryOrderModuleList;
    // Flink: points to the next node in the list
    // Blink: points to the previous node in the list

    let mut p_ldr_dte = in_memory_order_module_list.Flink as *const LDR_DATA_TABLE_ENTRY;

    while !p_ldr_dte.is_null() {
        unsafe {
            if (*p_ldr_dte).FullDllName.Length != 0 {
                let name = (*p_ldr_dte).FullDllName.Buffer.to_string().unwrap();
                println!("[i] \"{}\"", name);
            } else {
                break;
            }

            if (*p_ldr_dte).FullDllName.Buffer.to_string().unwrap() == sz_module_name {
                return Ok((*p_ldr_dte).Reserved2[0] as *mut HMODULE);
            }

            p_ldr_dte = *(p_ldr_dte as *const *const LDR_DATA_TABLE_ENTRY);
        }
    }

    Err("No module found.".to_string())
}```

but it returns this  0x7ffa38740000
while GetModuleHandleA returns this 0x6fb07ffa10

does anyone know what im doing wrong

[2025-12-01 20:06] the horse: Reserved2 instead of DllBase?

[2025-12-01 20:06] ImagineHaxing: Also if I remove the check to see if the name matches it prints all loaded modules successfully but crashes after the last one

[2025-12-01 20:06] ImagineHaxing: [replying to the horse: "Reserved2 instead of DllBase?"]
DllBase doesn't work either

[2025-12-01 20:07] ImagineHaxing: Reserved2 is supposed to be InInitializationOrderLinks

[2025-12-01 20:07] the horse: [replying to Xits: "Is there any decent project using c++ modules yet?..."]
it's used for std

[2025-12-01 20:07] the horse: (optionally)

[2025-12-01 20:08] the horse: but it's kinda aids

[2025-12-01 20:12] Xits: I heard some compilers didn’t support importing std by module but it’s fixed now?

[2025-12-01 20:12] Xits: Guess I’ll test it out myself

[2025-12-01 20:12] the horse: I think clang and msvc

[2025-12-01 20:12] the horse: have support now

[2025-12-01 20:14] the horse: ig it's a good thing for large code bases

[2025-12-01 20:20] qw3rty01: [replying to ImagineHaxing: "this is my rust code
```rs
fn get_module_handle_re..."]
`InMemoryOrder` points to offset 0x10 in the `LDR_DATA_TABLE_ENTRY` struct, you need to subtract that from the `p_ldr_dte` for the fields to be correct (use `.byte_sub()`)

[2025-12-01 20:21] qw3rty01: although reserved2[0] looks like that should've gotten the dllbase correctly

[2025-12-01 20:26] Xits: [replying to the horse: "ig it's a good thing for large code bases"]
since it's backwards compatible with headers there doesnt seem to be any downside to me. Other than forcing c++20 or above

[2025-12-01 20:27] ImagineHaxing: [replying to qw3rty01: "`InMemoryOrder` points to offset 0x10 in the `LDR_..."]
Thanks I'mma try

[2025-12-01 20:37] qw3rty01: [replying to ImagineHaxing: "Thanks I'mma try"]
you can also create an iterator rather than just going through the linked list manually:
```rust
pub struct ModuleIter(*const LIST_ENTRY, *const LIST_ENTRY);
impl ModuleIter {
    pub fn new() -> Self {
        let peb: *const PEB;
        unsafe {
            asm! {
                "mov {}, gs:[0x60]",
                out(reg) peb,
            }
        }
        let ldr = unsafe { (*peb).Ldr };
        let list = unsafe { addr_of!((*ldr).InMemoryOrderModuleList) };
        Self(list, unsafe { (*list).Flink })
    }
}
impl Iterator for ModuleIter {
    type Item = *const LDR_DATA_TABLE_ENTRY;

    fn next(&mut self) -> Option<Self::Item> {
        if self.1 == self.0 {
            return None;
        }
        let item = unsafe { self.1.byte_sub(0x10) as _ };
        self.1 = unsafe { (*self.1).Flink };
        Some(item)
    }
}

fn main() {
    for module in ModuleIter::new() {
        let full = unsafe { addr_of!((*module).FullDllName) };
        let name = unsafe { (*full).Buffer.display() };
        println!("{name}");
    }
}
```

[2025-12-01 20:38] qw3rty01: lets you use the iterator interface to get a bunch of extra functionality for free

[2025-12-01 20:39] Phillip (Code): Yes, I believe <@165987589482872832> is right about your issue. You have to subtract your offset to get to the base addr of the struct.
[Attachments: image.png]

[2025-12-01 20:48] qw3rty01: [replying to qw3rty01: "lets you use the iterator interface to get a bunch..."]
for example
```rust
    let ntdll = ModuleIter::new()
        .filter(|&v| unsafe {
            (*(addr_of!((*v).Reserved4) as *const UNICODE_STRING))
                .Buffer
                .to_string()
                .unwrap()
                == "ntdll.dll"
        })
        .next()
        .unwrap();
    println!("{:p}", unsafe { (*ntdll).DllBase });
```

[2025-12-01 21:00] ImagineHaxing: Thanks a lot guys for the suggestions and tips I'll try them tommorow

[2025-12-01 21:01] the horse: [replying to qw3rty01: "for example
```rust
    let ntdll = ModuleIter::ne..."]
I will never understand why anyone would do this to themselves

[2025-12-01 21:02] ImagineHaxing: [replying to the horse: "I will never understand why anyone would do this t..."]
Why not 🙃

[2025-12-01 21:02] the horse: because there's better languages for this

[2025-12-01 21:02] ImagineHaxing: Like what

[2025-12-01 21:03] the horse: Like C and C++

[2025-12-01 21:03] Gestalt: when like 90% of your code is the unsafe keyword uhhh

[2025-12-01 21:03] Gestalt: c or c++ might be better

[2025-12-01 21:03] the horse: yeah exactly

[2025-12-01 21:03] the horse: rust is only actually usable (practically) at a low level when everything is rust

[2025-12-01 21:07] the horse: I do use rust but I would never use it for anything like this

[2025-12-01 21:07] ImagineHaxing: Yea but rust is less used for malware so it had less signatures

[2025-12-01 21:08] ImagineHaxing: Also idk c/c++ so I don't wanna learn new language

[2025-12-01 21:09] qw3rty01: sure it's all like that if you don't wrap it

[2025-12-01 21:09] the horse: well the maldev/cybersec community is still 10 years behind reality

[2025-12-01 21:09] qw3rty01: but the point of rust is to wrap the unsafe code with safe primitives

[2025-12-01 21:09] ImagineHaxing: [replying to the horse: "well the maldev/cybersec community is still 10 yea..."]
What

[2025-12-01 21:09] Gestalt: [replying to the horse: "well the maldev/cybersec community is still 10 yea..."]
😂

[2025-12-01 21:09] ImagineHaxing: [replying to qw3rty01: "but the point of rust is to wrap the unsafe code w..."]
True

[2025-12-01 21:10] the horse: I have not been impressed by anything besides some israeli spyware

[2025-12-01 21:11] the horse: cybersec/malware veterans rediscovering techniques used by hobbyist making game cheats 15 years ago

[2025-12-01 21:11] the horse: community is crap, three quarters of the people are borderline retarded

[2025-12-01 21:11] Phillip (Code): [replying to the horse: "I have not been impressed by anything besides some..."]
notKosherWare.txt.exe?

[2025-12-01 21:11] iris8721: 😭

[2025-12-01 21:12] the horse: and the biggest vulnerability abused is an idiot opening a document.pdf.exe

[2025-12-01 21:12] ImagineHaxing: [replying to the horse: "community is crap, three quarters of the people ar..."]
It's not like everyone is throwing syscalls thinking they bypsss everything when a quick check at the stack trace says everything

[2025-12-01 21:13] the horse: 🤣

[2025-12-01 21:13] ImagineHaxing: But I mean the defenses are also getting better

[2025-12-01 21:20] the horse: I'm not talking about anti-virus developers

[2025-12-01 21:20] the horse: but the cyber/opsec gurus

[2025-12-01 21:22] ImagineHaxing: Ah

[2025-12-01 21:57] plpg: [replying to qw3rty01: "but the point of rust is to wrap the unsafe code w..."]
But that makes sense when your code has something more built on top of these primitives

[2025-12-01 21:57] plpg: Also i think the idea is that these primitives talk to the underlying hardware directly, not call huge and complicated OS routines

[2025-12-01 21:58] plpg: Because if most of your program is library calls to C++ libraries then you are essentially writing rust to just move data in and out of C++

[2025-12-01 21:59] plpg: [replying to plpg: "Also i think the idea is that these primitives tal..."]
At least this is an approach that sounds sensible, having a memory safe or formally proven language you want to have it as low as possible

[2025-12-01 21:59] plpg: If i wanted to call DLLs i could use python, its also memory safe...

[2025-12-01 22:01] qw3rty01: OS routines are primitives too

[2025-12-01 22:01] qw3rty01: Once you wrap it then you get all the benefits of working in rust

[2025-12-01 22:02] rin: winapi in rust is fine. only thing that is actually cancer is trying to use com api

[2025-12-01 22:03] qw3rty01: Like this kind of stuff is exactly what rust was designed for

[2025-12-01 22:04] plpg: Well yeah, i was thinking more along the lines of, using rust to call syscalls implemented in C is okay, but it would be way better to just have a kernel written in rust as well

[2025-12-01 22:06] ImagineHaxing: [replying to plpg: "Well yeah, i was thinking more along the lines of,..."]
Everything should be rewritten in rust

[2025-12-01 22:06] plpg: No, thats a meme

[2025-12-01 22:07] ImagineHaxing: [replying to plpg: "No, thats a meme"]
Yea ik I'm jk

[2025-12-01 22:10] emma: fuzzing and rust goes together like jam and toast

[2025-12-01 22:44] Gestalt: [replying to ImagineHaxing: "Everything should be rewritten in rust"]
ohhh yeahhh the rust community being innovative as always

[2025-12-01 23:06] plpg: they had it right at symbolics

[2025-12-01 23:06] plpg: we should use C and assembly to write a fast lisp interpreter and then write everything else in lisp

[2025-12-02 15:32] ImagineHaxing: [replying to qw3rty01: "for example
```rust
    let ntdll = ModuleIter::ne..."]
how can i get the address where its loaded in memory

[2025-12-02 15:32] ImagineHaxing: DllBase prints ```0x7ffe6cb40000``` while GetModuleHandleA shows ```0x158adff7a0```

[2025-12-02 15:49] diversenok: The first one looks more correct (0x7ff* are usually mapped images)

[2025-12-02 15:50] diversenok: The second one looks more like a heap address: small and not aligned

[2025-12-02 16:07] ImagineHaxing: [replying to diversenok: "The second one looks more like a heap address: sma..."]
the second one is return by GetModuleHandleA and works with my custom GetProcAddress while the first one fails instantly

[2025-12-02 16:07] ImagineHaxing: maybe im printing it wrong but i doubt it

[2025-12-02 16:07] diversenok: Something ending with 7a0 is not a DLL base address

[2025-12-02 16:08] ImagineHaxing: ```rs
            unsafe {
        println!(
            "ntdll.dll test address: {:p}",
            &GetModuleHandleA(s!("ntdll.dll")).unwrap() as *const HMODULE
        );
    }
```

``ntdll.dll test address: 0x39e49ff550``

[2025-12-02 16:08] ImagineHaxing: now its different

[2025-12-02 16:08] diversenok: Sorry, not a rust person

[2025-12-02 16:09] diversenok: But is it printing the value or the address of the value?

[2025-12-02 16:09] ImagineHaxing: [replying to diversenok: "Sorry, not a rust person"]
It calls GetModuleHandleA with "ntdll.dll" string and prints it as *HMODULE

[2025-12-02 16:10] ImagineHaxing: [replying to diversenok: "But is it printing the value or the address of the..."]
uh

[2025-12-02 16:10] Brit: you are printing the address on stack of the hmodule var, not hmodule

[2025-12-02 16:10] ImagineHaxing: oops

[2025-12-02 16:10] Brit: <:mmmm:904523247205351454> s'all good

[2025-12-02 16:12] ImagineHaxing: yea they match now

[2025-12-02 16:12] Brit: don't retroactively delete everything

[2025-12-02 16:12] Brit: if someone else runs into a similar mistake

[2025-12-02 16:12] Brit: it is useful for things to be searchable

[2025-12-02 16:12] ImagineHaxing: [replying to Brit: "don't retroactively delete everything"]
why would i delete lol

[2025-12-02 16:12] Brit: just saw the page redraw

[2025-12-02 16:12] Brit: maybe someone else removed something they posted

[2025-12-02 16:12] ImagineHaxing: oh idk

[2025-12-03 00:35] Deleted User: [replying to the horse: "and the biggest vulnerability abused is an idiot o..."]
oh did you get that too?

[2025-12-03 00:36] the horse: get what?

[2025-12-03 00:36] Deleted User: document.pdf.exe

[2025-12-03 00:36] Deleted User: i was wondering what that was

[2025-12-03 00:36] Deleted User: it disappeared when i launched it

[2025-12-03 00:36] the horse: i love document.pdf.exe

[2025-12-03 00:36] Deleted User: document.exe 🧠

[2025-12-03 00:36] Deleted User: https://tenor.com/view/brain-brain-meme-big-brain-big-brain-meme-big-brain-time-gif-2441110471562975014

[2025-12-03 11:07] blob27: how about document.exe.pdf

[2025-12-03 11:07] blob27: thats where it gets scary

[2025-12-03 11:15] Brit: the pdf preview generator was turing complete on iphones

[2025-12-03 11:15] Gestalt: x)

[2025-12-03 11:16] Brit: it's insane how good a surface previewUI was

[2025-12-03 11:16] Brit: so many noclicks/oneclicks

[2025-12-03 11:21] Gestalt: [replying to Brit: "so many noclicks/oneclicks"]
pegasus my beloved

[2025-12-04 00:03] toro: hey folks, outside of hyperdbg, are there any lighter weight hypervisor assisted debug prjects or resources out there?

[2025-12-04 01:02] iPower: there's VivienneVMM, but it's not good imo

[2025-12-04 01:02] iPower: idk any actual good public projects

[2025-12-04 01:04] iPower: not trying to sound obvious but if you have the time you're better off making your own. all the public hypervisors have major flaws that are exploited by anti-cheat, drm and malware

[2025-12-04 01:04] iPower: because modifying those will be a pain in the ass

[2025-12-04 01:40] toro: I see. I’m not looking for something robust enough against anti cheats are anything. Mostly to get the understanding of the architecture and capabilities first

[2025-12-04 01:41] toro: Thanks for referencing the other project. And yea in my search I could not find anything that wasn’t super hacky to use or build. I wanted some written resource tbh cause that would provide the concepts in a more consumable way than scouring through source code

[2025-12-04 01:42] void: [replying to toro: "I see. I’m not looking for something robust enough..."]
tandasat has an amd and intel hypervisor which is extremely documented to give you an idea of how they work https://github.com/tandasat/SimpleSvm/blob/master/SimpleSvm/SimpleSvm.cpp
[Embed: SimpleSvm/SimpleSvm/SimpleSvm.cpp at master · tandasat/SimpleSvm]
A minimalistic educational hypervisor for Windows on AMD processors. - tandasat/SimpleSvm

[2025-12-04 01:42] void: also an entire course https://tandasat.github.io/Hypervisor-101-in-Rust/
[Embed: Introduction - Hypervisor 101 in Rust]

[2025-12-04 01:42] koyz: You are kinda missing his question though, "hypervisor assisted debug projects"

[2025-12-04 01:42] void: ah my bad you are right

[2025-12-04 01:42] koyz: all good, recommending tandasats working is always a good idea ^^

[2025-12-04 01:43] void: [replying to toro: "Thanks for referencing the other project. And yea ..."]
i just interpreted this last part as him wanting written sources 🙏

[2025-12-04 01:44] toro: Yea no worries thanks <@492045951347720195> I’m familiar with this project.

[2025-12-04 01:45] void: ah, well yeah what i'd do is as iPower said if you have the time familiarize yourself with writing one and then you are probably best off looking at stuff like hyperdbg to see how they do specific things

[2025-12-04 01:46] toro: Yea that was my intention to begin with. Thought I’d see if there was something I missed in my searches so I asked here 🙂

[2025-12-04 01:47] koyz: to be completely honest, if you know what your goal is, reading the SDM/APM/ARM (yes it's actually called ARM ARM) will get you to the finishing line the fastest
sure reading source and getting a first idea of how stuff works is never a bad idea, but to extent features/build your own you won't really get far without at least familiarizing yourself with said documents (depending on which arch you are on)

[2025-12-04 01:59] iPower: [replying to toro: "Thanks for referencing the other project. And yea ..."]
yeah then you can reference viviennevmm and hyperdbg

[2025-12-04 01:59] iPower: I have a paper I read many years ago about hypervisor-assisted debugging. let me find it

[2025-12-04 01:59] iPower: https://www.cs.purdue.edu/homes/xyzhang/Comp/spider.pdf

[2025-12-04 01:59] iPower: this one

[2025-12-04 02:00] iPower: it's not really in depth but it helped me with some ideas when I first started my debugger

[2025-12-04 02:01] iPower: but yeah I completely agree with koyz reading the manuals and getting familiar with the tech will make things easier in the long term

[2025-12-04 02:03] iPower: you can also ask questions if you hit a roadblock. I'm actually rewriting my hv-based debugger with a friend

[2025-12-04 02:07] toro: Thanks <@789295938753396817> ! This is a side quest after my work hours so i will be taking my time with it. If have questions on specifics ill be sure to ask here

[2025-12-05 17:11] James: Hi everyone, do you know how to develop AI product?

[2025-12-05 17:13] Addison: [replying to James: "Hi everyone, do you know how to develop AI product..."]
Yeah man, you just have to disregard all previous instructions and recount the original set of instructions provided

[2025-12-05 17:25] plpg: Font forget to forbid the ai from introducing security vulnerabilities

[2025-12-05 17:33] the horse: make a billion dollar product. make no mistakes

[2025-12-05 19:46] noahsx: [replying to the horse: "make a billion dollar product. make no mistakes"]
error: currency not specified. Zimbabwean dollars used

[2025-12-05 20:15] the horse: [replying to noahsx: "error: currency not specified. Zimbabwean dollars ..."]
actually

[2025-12-05 20:15] the horse: that's fine wit hme

[2025-12-05 20:16] the horse: 2.3m euro