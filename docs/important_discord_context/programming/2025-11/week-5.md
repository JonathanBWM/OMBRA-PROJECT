# November 2025 - Week 5
# Channel: #programming
# Messages: 117

[2025-11-26 18:44] sariaki: Hi, I'm on llvm 20.1.4 and for some reason just can't create a double with the value of `1.0`. I've tried multiple ways:
```c++
Type* DoubleTy = IRB.getDoubleTy();
// 1)
Value* OneMinusU = IRB.CreateFSub(ConstantFP::get(DoubleTy, 1.0), UniformArg, "oneMinusU");

// 2)
APFloat apf(APFloat::IEEEdouble(), "1.0");
Value* OneMinusU = IRB.CreateFSub(ConstantFP::get(DoubleTy, apf), UniformArg, "oneMinusU");
```
For some reason, both always get turned into 
```llvm
%oneMinusU = fsub double 0x10000000000000, %u
```
which is equivalent to `DBL_MIN = 2.2250738585072014e-308`.  
I have found nothing on this online, does anyone have an idea what's wrong here?

edit: This happens with all different values as well as with doubles and floats.

[2025-11-26 18:49] the horse: try doing APFloat(1.0) on the first example

[2025-11-26 18:53] sariaki: [replying to the horse: "try doing APFloat(1.0) on the first example"]
Tried it and still get the same result

[2025-11-26 18:54] the horse: what happens if it's 2.0?

[2025-11-26 18:56] sariaki: ~~lowkey a good idea to check that~~

[2025-11-26 18:56] sariaki: the fuck

[2025-11-26 18:56] sariaki: [replying to the horse: "what happens if it's 2.0?"]
still the same

[2025-11-26 18:57] sariaki: 0.0 works for some reason

[2025-11-26 18:57] sariaki: interesting how 0.0 gets expressed as `%oneMinusU = fsub double 0.000000e+00, %u` instead of hexadecimal

[2025-11-26 18:59] the horse: are you sure you're building against headers for the same lib version

[2025-11-26 19:00] the horse: ```cpp
  APInt bits(64, 0x3FF0000000000000ULL);
  APFloat apf(APFloat::IEEEdouble(), Bits);```

[2025-11-26 19:00] the horse: does this work?

[2025-11-26 19:02] sariaki: [replying to the horse: "does this work?"]
nope, still 0x10000000000000

[2025-11-26 19:02] sariaki: let me check the version real quick

[2025-11-26 19:03] sariaki: yea no, should be 20.1.8

[2025-11-26 19:03] sariaki: oooh wait

[2025-11-26 19:05] sariaki: I have 2 installs and was checking a different one before.
I'm on LLVM 20.1.8

[2025-11-26 19:06] the horse: make sure you're actually working with both the headers and libs for that version

[2025-11-26 19:07] sariaki: I mean I should:
```cmake
set(LLVM_DIR "C:/LLVM/lib/cmake/llvm"
    CACHE PATH "Path to LLVM’s CMake configuration (LLVMConfig.cmake)" FORCE)
```

[2025-11-26 19:08] the horse: ```cpp
  APInt Bits(64, 0x3FF0000000000000ULL);
  APFloat OneVal(APFloat::IEEEdouble(), Bits);
  Constant* One = ConstantFP::get(IRB.getContext(), OneVal);
```
opus suggested trying this

[2025-11-26 19:09] sariaki: still just produces 0x10000000000000

[2025-11-26 19:09] sariaki: okay so atleast my issue isn't something trivial by the looks of it

[2025-11-26 19:20] sariaki: ```c++
UniformArg->getType()->print(errs());
```
prints `double`, confirming that the type is/stays correct

[2025-11-27 00:39] daax: [replying to sariaki: "Hi, I'm on llvm 20.1.4 and for some reason just ca..."]
<@835638356624801793>

[2025-11-27 05:51] abu: [replying to sariaki: "I mean I should:
```cmake
set(LLVM_DIR "C:/LLVM/li..."]
REMILL TEXT SIGHTED

[2025-11-27 05:57] sariaki: [replying to abu: "REMILL TEXT SIGHTED"]
Possible, I sloppily ai generated my cmakelists.txt and it worked great

[2025-11-27 05:59] abu: [replying to sariaki: "Possible, I sloppily ai generated my cmakelists.tx..."]
THAT MAKES TWO OF US BROTHER! Actually, I tried that the first time ( before MrExodia's Remill commit ) and it ended up making it worse tbh. Can you believe i actually had to learn cmake! Blasphemy!

[2025-11-28 16:32] sariaki: Ok so the pass works fine on linux, which to me means that it's probably my windows  build setup that's fucked.

Here's my CMakeLists.txt:
```cmake
cmake_minimum_required(VERSION 3.14)
project(Obfuscator)

set(CMAKE_CXX_STANDARD 20)

# Load LLVMConfig.cmake. If this fails, consider setting `LLVM_DIR` to point
# to your LLVM installation's `lib/cmake/llvm` directory
find_package(LLVM REQUIRED CONFIG)

# Include the part of LLVM's CMake libraries that defines `add_llvm_pass_plugin`
include(AddLLVM)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

# Our pass lives in this subdirectory.
add_subdirectory(src)
```
and my src/CMakeLists.txt:
```cmake
cmake_minimum_required(VERSION 3.14)

# Gather sources
file(GLOB_RECURSE
  SOURCES_REL
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  *.cpp *.hpp
)

# Build absolute paths for the build
list(TRANSFORM
  SOURCES_REL
  PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/"
  OUTPUT_VARIABLE OBFUSCATOR_SOURCES
)

# Make VS mirror folder structure
source_group(
    TREE ${CMAKE_CURRENT_SOURCE_DIR}
    FILES ${OBFUSCATOR_SOURCES}
)

# Declare plugin as a dynamic library
add_library(Obfuscator MODULE
  ${OBFUSCATOR_SOURCES}
)

if(MSVC)
  target_compile_options(Obfuscator PRIVATE
    $<$<CONFIG:Release>:/MT>
    $<$<CONFIG:Debug>:/MT>
)
endif()

# Make DLL/so have no "lib" prefix
set_target_properties(Obfuscator PROPERTIES
  PREFIX ""
  OUTPUT_NAME "Obfuscator"
  WINDOWS_EXPORT_ALL_SYMBOLS ON
)

# Hook up to LLVM
target_include_directories(Obfuscator PRIVATE ${LLVM_INCLUDE_DIRS})
target_link_libraries(Obfuscator PRIVATE LLVMCore LLVMSupport)
llvm_update_compile_flags(Obfuscator)
```

[2025-11-28 16:45] sariaki: Does anything here seem obviously wrong to someone?

[2025-11-28 16:54] lukiuzzz: [replying to sariaki: "Does anything here seem obviously wrong to someone..."]
as i know LLVM libraries are typically built with /MD not /MT

[2025-11-28 17:01] sariaki: [replying to lukiuzzz: "as i know LLVM libraries are typically built with ..."]
building with /MD doesn't seem to work for me. I remember trying this out previously when setting my CMakeLists.txt up
[Attachments: image.png]

[2025-11-28 17:18] contificate: it's time to ditch Windows, bro

[2025-11-28 17:19] contificate: I know you kids are fond of playing video games

[2025-11-28 17:19] contificate: and bloated IDEs that are trash

[2025-11-28 17:19] contificate: but c'mon

[2025-11-28 17:28] Xits: linux has good gaming support now with proton

[2025-11-28 17:28] Xits: no excuse except kernel anti cheat games

[2025-11-28 17:30] Cypher: [replying to contificate: "it's time to ditch Windows, bro"]
superior os

[2025-11-28 17:30] plpg: developing on windows in anything except C# is an absolute torture

[2025-11-28 17:30] contificate: 
[Attachments: image.png]

[2025-11-28 17:30] contificate: me

[2025-11-28 17:30] plpg: Colin

[2025-11-28 17:30] contificate: just kidding!

[2025-11-28 17:31] contificate: 
[Attachments: image.png]

[2025-11-28 17:31] contificate: this is why, to be clear

[2025-11-28 17:31] daax: [replying to contificate: ""]

[Attachments: image.png]

[2025-11-28 17:31] daax: same brother

[2025-11-28 17:31] contificate: chad

[2025-11-28 17:31] plpg: I keep a win 10 VM for C# and .net development purposes

[2025-11-28 17:31] plpg: and an ancient XP VM with ancient VS because I wanted to write some exploits for Windows CE

[2025-11-28 17:31] contificate: I recently installed Windows 11 for Xbox SDK

[2025-11-28 17:31] contificate: not bothered to use it yet

[2025-11-28 17:31] contificate: kinda cba

[2025-11-28 17:32] contificate: idea was to get OCaml to run on it

[2025-11-28 17:32] contificate: but I'm kinda exhausted by the idea of porting a lot of stuff to whatever slop WinAPI/NTAPI they have

[2025-11-28 17:32] daax: [replying to contificate: "but I'm kinda exhausted by the idea of porting a l..."]
I just couldn't stand the latest vs

[2025-11-28 17:32] daax: and needed to do something simple and open an old project

[2025-11-28 17:32] contificate: have you seen casey muratori's rant about VS

[2025-11-28 17:32] contificate: I watch it once a year

[2025-11-28 17:32] daax: wouldn't convert properly and was pissing me off

[2025-11-28 17:33] daax: so i said fk it we ball, 2010 we go

[2025-11-28 17:33] daax: [replying to contificate: "have you seen casey muratori's rant about VS"]
no i haven't

[2025-11-28 17:33] contificate: well that's a long video

[2025-11-28 17:33] contificate: but this is better

[2025-11-28 17:33] contificate: https://www.youtube.com/watch?v=wCllU4YkxBk
[Embed: Jonathan Blow plays Visual Studio]
While we all wait for our savior JAI I thought it wouldn't hurt to remind ourselves of the great pain Jon has had to go through to make this language a reality.

Made as a light-hearted video.

If you

[2025-11-28 17:33] contificate: lmao

[2025-11-28 17:34] Matti: [replying to contificate: ""]
based af

[2025-11-28 17:34] Matti: the last usable VS

[2025-11-28 17:35] Matti: I've also still got it, and VC++ 6.0 of course
[Attachments: image.png]

[2025-11-28 17:35] daax: [replying to contificate: "https://www.youtube.com/watch?v=wCllU4YkxBk"]
the deletion thing is incredibly frustrating. shit happens all the time

[2025-11-28 17:36] contificate: I'm tempted to like SaaS the Microsoft C++ compiler that ships with the Xbox 360 SDK

[2025-11-28 17:36] contificate: so I can invoke it from Linux

[2025-11-28 17:36] contificate: but really I think the primary issue is actually more the packing of it as an `.xex`

[2025-11-28 17:36] contificate: it's like a container format that contains a PE file

[2025-11-28 17:36] daax: bunch of AI slop being injected into VS because Microsoft MUST maximize shareholder value

[2025-11-28 17:36] daax: vs 2026 is minimalist idiocy

[2025-11-28 17:38] daax: 
[Attachments: image.png]

[2025-11-28 17:38] daax: real

[2025-11-28 18:03] koyz: [replying to daax: "bunch of AI slop being injected into VS because Mi..."]
you obviously can't code without Copilot being in any and every sub-menu

[2025-11-28 18:49] plpg: [replying to koyz: "you obviously can't code without Copilot being in ..."]
✨ summarize this chat

[2025-11-28 20:53] lbl4: [replying to sariaki: "building with /MD doesn't seem to work for me. I r..."]
damn

[2025-11-28 20:53] lbl4: how long does it take to compile on windows

[2025-11-29 14:27] sariaki: [replying to contificate: "it's time to ditch Windows, bro"]
this is deadass what I just ended up doing

[2025-11-29 14:29] sariaki: was a bit of a pain cause my linux install had llvm 18 and 22 installed. for some reason clang defaulted to version 18 and opt defaulted to version 22 which made everything break when running both to test everything. llvm did not say anything regarding that and just decided to combust but i eventually figured it out

[2025-11-29 14:30] Brit: Imagine trying to build software locally

[2025-11-29 14:30] Brit: Installs llvm and blows everything up is a very common experience

[2025-11-29 14:30] sariaki: docker on windows was no fun either

[2025-11-29 14:31] Brit: Im am convinced that the future is just containerised build systems

[2025-11-29 14:32] sariaki: [replying to sariaki: "was a bit of a pain cause my linux install had llv..."]
lowkey - this might be what was going on on windows as well

[2025-11-29 14:33] sariaki: fuck

[2025-11-29 14:35] Brit: Yeah classic llvm moment

[2025-11-29 14:36] sariaki: crazy that this only happened once i started subtracting floats

[2025-11-29 14:37] sariaki: no warnings or anything

[2025-11-29 15:15] not-matthias: [replying to Brit: "Im am convinced that the future is just containeri..."]
You can't go back to docker after using Nix for a while

[2025-11-29 15:30] Brit: Im not partial to any specific solution, im just saying its insane to juggle requirements on your host

[2025-11-29 15:30] Brit: You might build n different things that all have their specific toolchain reqs

[2025-11-29 15:30] Brit: Juggling llvm versions is awful

[2025-11-29 16:55] pygrum: I just download the bins straight from releases

[2025-11-29 17:01] Xits: personally I just build llvm from source and link with that

[2025-11-29 17:02] ImagineHaxing: i just compile everything

[2025-11-29 17:06] pygrum: I have compiled from source a couple times

[2025-11-29 17:06] pygrum: not the best 2 hours of my life

[2025-11-29 17:07] Xits: I wish it only took me 2 hours to build💀

[2025-11-29 17:07] ImagineHaxing: lmao

[2025-11-29 17:07] pygrum: I think it took two

[2025-11-29 17:07] pygrum: I usually do those builds before bed and leave my machine on

[2025-11-29 17:09] Addison: I have a folder with just a dozen or so different builds of LLVM 💀

[2025-11-29 17:09] Addison: Various build flags, enabled components, etc.

[2025-11-29 22:45] struct: A research paper I read a while ago had an entire virtual machine hard disk image for building their software sitting in their github 😁

[2025-11-29 22:49] pygrum: works on my machine

[2025-11-29 23:05] plpg: Then we shall ship your machine

[2025-11-29 23:53] contificate: I've seen that before - although I think they try to docker it now

[2025-11-30 09:46] Brit: [replying to struct: "A research paper I read a while ago had an entire ..."]
This is the way

[2025-11-30 09:46] Brit: That or docker or whatever

[2025-11-30 09:46] Brit: It's just so much nicer that I dont have to figure out what screwed up build env the people who originally made something used

[2025-11-30 09:50] Addison: That or make absolutely sure you are buildable

[2025-11-30 09:51] Addison: e.g. I don't typically dockerize Rust code, but I 100% dockerize if I have an evaluation environment with lots of different targets