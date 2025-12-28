# May 2025 - Week 1
# Channel: #programming
# Messages: 76

[2025-05-02 19:11] pinefin: https://en.cppreference.com/w/cpp/header/debugging
[Embed: cpp/header/debugging]

[2025-05-02 19:11] pinefin: C++26 is gonna be revolutionary 🔥 🔥 🔥 🔥 🔥 🔥

[2025-05-02 19:11] pinefin: 😭

[2025-05-02 19:11] pinefin: (https://en.cppreference.com/w/cpp/26 full list for reference kek)
[Embed: cpp/26]

[2025-05-02 19:12] pinefin: i think text_encoding will be useful

[2025-05-02 19:13] pinefin: theyre also using constexpr functions more and more now as well

[2025-05-02 19:26] daax: [replying to pinefin: "C++26 is gonna be revolutionary 🔥 🔥 🔥 🔥 🔥 🔥"]
reflection

[2025-05-02 19:26] daax: where is my reflection

[2025-05-02 19:26] daax: my beloved reflection

[2025-05-02 19:28] pinefin: [replying to daax: "reflection"]
the year of the c++ reflection is upon us...

[2025-05-02 19:29] pinefin: (kek)

[2025-05-02 19:33] Brit: I unironically like a lot of the 26 features

[2025-05-02 19:37] pinefin: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p2996r9.html#revision-history

[2025-05-02 19:38] pinefin: ```c++
constexpr std::array types = {^^int, ^^float, ^^double};
constexpr std::array sizes = []{
  std::array<std::size_t, types.size()> r;
  std::views::transform(types, r.begin(), std::meta::size_of);
  return r;
}();
```
what the hell is going on here

[2025-05-02 19:38] pinefin: `^^int`

[2025-05-02 19:39] truckdad: C++/CLI gets the last laugh

[2025-05-02 19:40] pinefin: ```c++
template <typename E>
  requires std::is_enum_v<E>
constexpr std::string enum_to_string(E value) {
  template for (constexpr auto e : std::meta::enumerators_of(^^E)) {
    if (value == [:e:]) {
      return std::string(std::meta::identifier_of(e));
    }
  }

  return "<unnamed>";
}

enum Color { red, green, blue };
static_assert(enum_to_string(Color::red) == "red");
static_assert(enum_to_string(Color(42)) == "<unnamed>");
```

this is neat, i used to use magic_enum for stuff like this

[2025-05-02 19:40] pinefin: i cant tell if this is what they are expecting it to be, or if this is actually new functionality

[2025-05-02 19:40] pinefin: i dig deeper

[2025-05-02 19:40] pinefin: nope

[2025-05-02 19:40] pinefin: this is actual funcvtionality

[2025-05-02 19:41] pinefin: so neat

[2025-05-02 19:41] pinefin: im stuck in c++17 😭

[2025-05-02 19:42] elias: im stuck in legacy msvc c

[2025-05-02 19:42] pinefin: https://godbolt.org/z/sj1aqj8Wj
[Embed: Compiler Explorer - C++ (x86-64 clang (experimental P2996))]
// start 'expand' definition
namespace __impl {
  template<auto... vals>
  struct replicator_type {
    template<typename F>
      constexpr void operator>>(F body) const {
        (body.template oper

[2025-05-02 19:48] pinefin: me likey
[Attachments: Screenshot_2025-05-02_at_2.48.42_PM.png]

[2025-05-02 19:49] pinefin: i think this was said to be proposed or is it actually going to end up in c++26?

[2025-05-02 23:01] the horse: [replying to pinefin: "https://en.cppreference.com/w/cpp/header/debugging"]
🤯 unified int3 intrinsic

[2025-05-02 23:02] the horse: i guess the other funcs are alright

[2025-05-02 23:03] dlima: [replying to pinefin: "```c++
template <typename E>
  requires std::is_en..."]
I just use macro cancer for this most of the time

[2025-05-02 23:03] dlima: Works pretty well

[2025-05-03 00:52] B3NNY: I remember there being an alternative site to vergilius, anyone remmeber what it is?

[2025-05-03 00:55] Yoran: [replying to B3NNY: "I remember there being an alternative site to verg..."]
reactos/wine?

[2025-05-03 00:56] Yoran: closest i know of

[2025-05-03 00:57] B3NNY: I was thinking of https://ntdoc.m417z.com
[Embed: NtDoc - The native NT API online documentation]
NtDoc - The native NT API online documentation

[2025-05-03 02:16] iamwho: i dont have any kernel dev experience but i wonder if we can map one process page tables to the other one in windows? is it possible?

[2025-05-03 09:34] Windy Bug: Not sure what are you trying to do but this may be helpful 

https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/nf-ntifs-kestackattachprocess
[Embed: KeStackAttachProcess function (ntifs.h) - Windows drivers]
The KeStackAttachProcess routine attaches the current thread to the address space of the target process.

[2025-05-03 11:01] Timmy: [replying to iamwho: "i dont have any kernel dev experience but i wonder..."]
https://blog.back.engineering/27/03/2021/
dinner is served
[Embed: Reverse Injector - Merging Address Spaces]
The bottom 256 PML4E’s are what map all code, data, and stacks. The 256th PML4E maps modules such as ntdll.dll, and other loaded modules. As you can foresee, some PML4E index’s overlap. In order to ha

[2025-05-04 15:03] Ruben Mike Litros: what is the point of `UWOP_SET_FPREG` operation code in unwinding? if the fp register is a non-vol register its going to be restored and if its a volatile register its value doesn't matter in unwinding anyway so why does it exist?: https://learn.microsoft.com/en-us/cpp/build/exception-handling-x64?view=msvc-170#unwind-operation-code
[Embed: x64 exception handling]
Overview of Microsoft C++ exception handling conventions on x64.

[2025-05-04 15:05] Ruben Mike Litros: i dont understand

[2025-05-04 15:05] Ruben Mike Litros: oh waitr nvm

[2025-05-04 15:05] truckdad: gone like the wind

[2025-05-04 15:05] Ruben Mike Litros: i was gonna say i dont understand why that matters

[2025-05-04 15:07] truckdad: it's non-volatile in some sense but it's not pushed/popped in the prologue/epilogue

[2025-05-04 15:08] Ruben Mike Litros: [replying to truckdad: "it's non-volatile in some sense but it's not pushe..."]
what?

[2025-05-04 15:08] Ruben Mike Litros: if rbp is being used as frame pointer

[2025-05-04 15:08] Ruben Mike Litros: it will get pushed in prolog right?

[2025-05-04 15:09] Ruben Mike Litros: like in their example code here they use r13 as the frame pointer but they also push r13 before using it as the frame pointer https://learn.microsoft.com/en-us/cpp/build/prolog-and-epilog?view=msvc-170#prolog-code
[Embed: x64 prolog and epilog]
Learn more about: x64 prolog and epilog

[2025-05-04 15:10] truckdad: > If a frame pointer is used in the function, then the stack must be trimmed to its fixed allocation prior to the execution of the epilog. This action is technically not part of the epilog.

[2025-05-04 15:10] truckdad: in that same page

[2025-05-04 15:10] Ruben Mike Litros: [replying to truckdad: "> If a frame pointer is used in the function, then..."]
im not sure what this means

[2025-05-04 15:10] Ruben Mike Litros: doesnt stack get trimmed when u do add rsp, alloc_size?

[2025-05-04 15:19] truckdad: in a fixed stack, sure

[2025-05-04 15:19] truckdad: how would the unwind know where to set the stack pointer at any given time in a non-fixed stack if the frame pointer isn't actually at the base of the frame?

[2025-05-04 15:21] Ruben Mike Litros: a non fixed stack

[2025-05-04 15:21] Ruben Mike Litros: never heard of that before

[2025-05-04 15:22] truckdad: i mean a stack that doesn't have a fixed allocation size, e.g. from `alloca`

[2025-05-04 15:22] truckdad: which is the main time you actually use a frame pointer on x64

[2025-05-04 15:24] Ruben Mike Litros: oh alright i see now

[2025-05-04 15:27] truckdad: [replying to truckdad: "it's non-volatile in some sense but it's not pushe..."]
i was unclear here, i was trying to get at the stack pointer, and how its value can take on the value of the frame pointer without an explicit save

[2025-05-04 16:21] Ruben Mike Litros: but wait a second

[2025-05-04 16:22] Ruben Mike Litros: how does that explain the existence of `UWOP_SET_FPREG` unwind code

[2025-05-04 16:22] Ruben Mike Litros: the fp register is stored in the UNWIND_INFO structure

[2025-05-04 16:24] Ruben Mike Litros: so the unwind would see that fp is being used, load the fp in rsp read the first entry in unwind code array which should be the initial stack allocation, subtract the allocation size by the Frame Register Offset field in UNWIND_INFO and then add that from rsp and then rsp will point to the area where non volatile registers were saved

[2025-05-04 16:24] Ruben Mike Litros: right?

[2025-05-04 16:28] truckdad: > load the fp in rsp read the first entry in unwind code array **which should be the initial stack allocation**
this is not necessarily true, as explained under the `UWOP_SET_FPREG` description

[2025-05-04 16:29] truckdad: the frame pointer may not be at the fixed allocation; it may be at an offset from it

[2025-05-04 16:29] truckdad: as long as the function knows the offset it can work with that, but an exception handler wouldn't know that a priori

[2025-05-04 16:30] Ruben Mike Litros: [replying to truckdad: "the frame pointer may not be at the fixed allocati..."]
what?

[2025-05-04 16:31] Ruben Mike Litros: frame pointer has to be at a fixed allocation no?

[2025-05-04 16:31] truckdad: > Establish the frame pointer register by setting the register to some offset of the current RSP. The offset is equal to the Frame Register offset (scaled) field in the UNWIND_INFO * 16, allowing offsets from 0 to 240. The use of an offset permits establishing a frame pointer that points to the middle of the fixed stack allocation, helping code density by allowing more accesses to use short instruction forms.

[2025-05-04 16:32] Ruben Mike Litros: i dont think i understand

[2025-05-04 16:33] Ruben Mike Litros: the fpreg unwind code doesnt even store any information

[2025-05-04 16:33] Ruben Mike Litros: like what does the unwinder even do when it encounters fpreg unwind code?

[2025-05-04 16:34] truckdad: it stores information temporally, since it establishes the value of the stack pointer onto which the offset is applied

[2025-05-04 16:36] Ruben Mike Litros: [replying to truckdad: "it stores information temporally, since it establi..."]
"since it establishes the value of the stack pointer onto which the offset is applied" doesnt that just have to be whatever the stack pointer was after non volatile registers were pushed and stack space was alloacted?