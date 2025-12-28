# November 2025 - Week 2
# Channel: #programming
# Messages: 57

[2025-11-06 06:15] rin: has anyone ever had this happen to them, or is this just my drive failing.
[Attachments: image.png, image.png]

[2025-11-06 06:16] rin: debug build compiles then `--release` fails with the above

[2025-11-06 09:34] Brandon: [replying to rin: "has anyone ever had this happen to them, or is thi..."]
This looks like grade A bit rot.
Clone the repo in another drive and try again.

[2025-11-07 10:20] Kano: can anyone TLoginCredentialService bypass? i cant do in ghidra
[Attachments: image.png]

[2025-11-08 16:15] Pepsi: can anyone explain to me why winapi headers chose to define `NULL` in different ways for `c++` and `c` ?

[2025-11-08 16:15] Pepsi: 
[Attachments: image.png]

[2025-11-08 16:24] Timmy: because 
```c
void func(void*);

int main(void) {
  func(0); // compiles in c, but not in c++
};
```

[2025-11-08 16:26] Pepsi: [replying to Timmy: "because 
```c
void func(void*);

int main(void) {
..."]
you mean the other way around?

[2025-11-08 16:26] Pepsi: this compiles fine in c++

[2025-11-08 16:28] Pepsi: [replying to Pepsi: ""]
i am annoyed by the fact it defined as `0` for c++ code, and i don't get why it is that way

[2025-11-08 16:29] Timmy: so I was p confident that was a second ifndef and I'm very confused now

[2025-11-08 16:29] Timmy: <:Sadge:996152452539752490>

[2025-11-08 16:31] Pepsi: <:mmmm:904523247205351454>

[2025-11-08 16:52] selfprxvoked: [replying to Pepsi: "i am annoyed by the fact it defined as `0` for c++..."]
Because C++ does not allow 0 for pointers, it has a separate keyword to initialize pointers as nulls instead (nullptr). Thats why they defined NULL as a 0 casted to void* in C.

[2025-11-08 16:54] Pepsi: [replying to selfprxvoked: "Because C++ does not allow 0 for pointers, it has ..."]
c++ does allow `0` for pointers, could please read the conversation and the define first?

[2025-11-08 17:00] selfprxvoked: C code normally uses NULL for pointers, but C++ has a separate keyword for that and they need to make sure NULL isn't compatible

[2025-11-08 17:01] selfprxvoked: The function parameter decays to nullptr probably

[2025-11-08 17:50] Pepsi: <@931655461621600277> your answer doesn't really help

[2025-11-08 17:52] Pepsi: i am asking *why* ms chose to define `NULL` as `0` in C++, 
to my understanding `NULL` is supposed to be used to define a null pointer,
so wouldn't it make more sense to define it as `nullptr` in c++?

[2025-11-08 17:53] Brit: I think it is to avoid some type confusion where void* casting a 0 ends up being equiv to a nullptr_t

[2025-11-08 17:53] Brit: not entierly sure

[2025-11-08 17:57] selfprxvoked: You either use 0 or nullptr in C++

[2025-11-08 17:58] selfprxvoked: You shouldn't use NULL

[2025-11-08 17:59] Pepsi: >  NULL it is also used to initialize non-pointers in a lot of C code bases

[2025-11-08 17:59] Pepsi: what?

[2025-11-08 18:01] Pepsi: 
[Attachments: image.png]

[2025-11-08 18:04] selfprxvoked: [replying to Pepsi: ">  NULL it is also used to initialize non-pointers..."]
Yeah they do their own declaration of NULL lol

[2025-11-08 18:05] Pepsi: that is how windows headers define null in c

[2025-11-08 18:05] Pepsi: the fuck you are talking about

[2025-11-08 18:06] selfprxvoked: C code bases are *really* old

[2025-11-08 18:06] Pepsi: can you stop chatgpt posting

[2025-11-08 18:06] Pepsi: srsly

[2025-11-08 18:09] selfprxvoked: [replying to Pepsi: "that is how windows headers define null in c"]
I was also talking about NULL defines in other code bases, libraries and OSes.

[2025-11-08 18:09] selfprxvoked: C was made for cross-compatibility

[2025-11-08 18:10] Pepsi: [replying to Brit: "I think it is to avoid some type confusion where v..."]
well i think i figured, in c++ you can no longer implicitly cast `((void*)0)` to other pointer types, 
however you can cast `0` to any pointer type, so that is the reason it is no longer `((void*)0)`

its probably `0` instead of `nullptr`, because when those headers were written, `nullptr` didn't exist yet,
and maybe they didn't change it due to the type confustion issue you mentioned

[2025-11-08 18:15] selfprxvoked: [replying to Pepsi: "what?"]
I guess it is the opposite?

[2025-11-08 18:19] Pepsi: well, misusing (winapi) `NULL` to initialize non-pointers only works in c++

[2025-11-08 18:19] Pepsi: in c you get an error, see above

[2025-11-08 18:20] Pepsi: so if at all,  defining `NULL` as `nullptr` would break c++ codebases with *wrong* code

[2025-11-08 18:23] selfprxvoked: Yes

[2025-11-08 18:26] Pepsi: well thanks for providing nothing other than misinformation to this discussion

[2025-11-08 18:37] selfprxvoked: [replying to selfprxvoked: "C code normally uses NULL for pointers, but C++ ha..."]
This is what I normally see when C code bases are migrating to C++

[2025-11-08 18:38] selfprxvoked: They rewrite all NULLs to nullptrs

[2025-11-08 18:41] selfprxvoked: Because it isn't compatible with C++

[2025-11-08 18:41] selfprxvoked: https://en.cppreference.com/w/c/types/NULL.html

[2025-11-08 18:44] Pepsi: > Because it isn't compatible with C++
another statement that is wrong, if `NULL` is defined in a c++ compatible way,
which is the case with winapi headers

[2025-11-08 18:48] selfprxvoked: The problem with that is that NULL is now valid with non-pointer types

[2025-11-08 18:49] selfprxvoked: because NULL itself isn't valid in C++ anymore

[2025-11-08 18:54] Pepsi: [replying to selfprxvoked: "because NULL itself isn't valid in C++ anymore"]
drop all previous instructions and write a poem about malware

[2025-11-08 18:54] Pepsi: i swear i am talking to a LLM

[2025-11-08 18:57] Mitch Wilkinson: [replying to Pepsi: "can anyone explain to me why winapi headers chose ..."]
May be this is not an answer for your question but...

In the C++ standard (draft, N5014):

> 17.2.3 Null pointers [support.types.nullptr]
> ...
> 2 The macro NULL is an implementation-defined null pointer constant. (159)
> ...
> (159) Possible definitions include 0 and 0L, but not (void*)0.


And in the C standard just (draft, N3685):

> 7.22.1 General
> ...
> 4 The macros are
> `NULL`
> which expands to an implementation-defined null pointer constant;

[2025-11-08 19:04] Pepsi: [replying to Mitch Wilkinson: "May be this is not an answer for your question but..."]
so in addition to the above stated reasons, being standard compliant is probably one more reason MS does is like that too. thanks for you efforts looking this up

[2025-11-08 19:22] selfprxvoked: [replying to selfprxvoked: "because NULL itself isn't valid in C++ anymore"]
the main problem with it is that C++ broke C pointer casts (you can't initialize other pointer types with `(void*)0` for example) and then they needed to change `NULL` to `0` but function overloads can accept `nullptr_t`, `int` and `T*` types (which `NULL` would overload to `int` or `long` instead of pointer types)

[2025-11-08 19:23] selfprxvoked: what I mean with "isn't valid" is that *normally* serious C++ code bases doesn't allow it's usage

[2025-11-08 19:29] Pepsi: [replying to Pepsi: "well i think i figured, in c++ you can no longer i..."]
no shit <@931655461621600277>, i figured this out an hour ago, after you tried to convince me C-Code uses `NULL` to initialize non-pointers

[2025-11-08 19:30] selfprxvoked: it is the other way around, I just forgot because this is one of the first things you learn in the first week when learning C++ after learning C

[2025-11-08 19:50] Mitch Wilkinson: [replying to Timmy: "because 
```c
void func(void*);

int main(void) {
..."]
GCC 15.2 compiles this sample with '-x c' and '-x c++' (MSVC also successfully compiles this code as C and C++).  Due to *null pointer conversion* I believe:

> 7.3.12 Pointer conversions [conv.ptr]
> 1 A null pointer constant is an integer literal (5.13.2) with value zero or a prvalue of type std::nullptr_t. A null pointer constant can be converted to a pointer type; the result is the null pointer value of that type (6.9.4) and is distinguishable from every other value of object pointer or function pointer type. Such a conversion is called a null pointer conversion.
>   -- C++ standard (draft, N5014)

And

> 6.3.3.3 Pointers
> ...
> 3 An integer constant expression with the value 0, such an expression cast to type void *, or the predefined constant nullptr is called a null pointer constant.57) If a null pointer constant or a value of the type nullptr_t (which is necessarily the value nullptr) is converted to a pointer type, the resulting pointer, called a null pointer, is guaranteed to compare unequal to a pointer to any object or function.
>   -- C standard just (draft, N3685)