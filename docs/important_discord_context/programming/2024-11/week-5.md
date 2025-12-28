# November 2024 - Week 5
# Channel: #programming
# Messages: 13

[2024-11-25 02:11] duk: [replying to InternalHigh: "template enjoyers moving more and more code into h..."]
"slowly" nah ur compile times are tanking at light speed

[2024-11-25 12:32] VTA/APT: yes I use godbolt, very good tool 👍

[2024-11-25 17:43] roddux: is it possible to use godbolt offline yet

[2024-11-25 17:45] roddux: ah yeah they have dockerfiles etc online

[2024-11-26 20:07] Delirium Mode: Asa: Sort of dipping my feet in to directx11, can you inject a dll and spawn a windows desktop window from it, and then draw?

[2024-11-26 22:03] pinefin: [replying to Delirium Mode: Asa: "Sort of dipping my feet in to directx11, can you i..."]
yeah just pass your window handle to where you create your device and swapchain

[2024-11-27 10:40] Timmy: isnt there the explicit template initialization thing where you can tell the compiler to generate the template function explicitly ina translation unit of your choice?

[2024-11-27 21:25] contificate: template specialisation?

[2024-11-27 22:05] Timmy: I now remember where I got that from it was from a cpp weekly vid: https://youtu.be/pyiKhRmvMF4
[Embed: C++ Weekly - Ep 330 - Faster Builds with `extern template` (And How...]
☟☟ Awesome T-Shirts! Sponsors! Books! ☟☟


T-SHIRTS AVAILABLE!

► The best C++ T-Shirts anywhere! https://my-store-d16a2f.creator-spring.com/


WANT MORE JASON?

► My Training Classes: http://emptycra

[2024-11-28 02:09] pinefin: [replying to Timmy: "isnt there the explicit template initialization th..."]
oh! yes! its amazing, this is how stl does things like "is_ptr" and such

[2024-11-28 02:11] pinefin: ```c++
template <typename T>
struct is_pointer {
    static constexpr bool value = false;
};

template <typename T>
struct is_pointer<T*> {
    static constexpr bool value = true;
};

template <typename T>
constexpr bool is_pointer_v = is_pointer<T>::value;
```

its the same concept that you're talking about with functions/methods.

[2024-11-29 11:53] contificate: yeah, traits are/were done this way

[2024-11-30 22:30] 0x208D9: dell