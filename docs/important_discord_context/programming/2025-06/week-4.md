# June 2025 - Week 4
# Channel: #programming
# Messages: 96

[2025-06-16 00:43] dinero: a kindred spirit

[2025-06-16 00:44] dinero: nothing better then aimless driving

[2025-06-16 07:43] roddux: [replying to contificate: "I don't program any more, just drive about"]
oh you pass your test?

[2025-06-16 07:43] roddux: have a memory of you talking about that not long ago

[2025-06-16 08:24] contificate: yeah, passsed last month

[2025-06-16 08:29] rin: does anyone know why there is a difference between function definitions in windows cargo package vs Microsoft documentation.  I am looking at https://microsoft.github.io/windows-docs-rs/doc/windows/Win32/System/Com/fn.CoCreateInstance.html and https://microsoft.github.io/windows-docs-rs/doc/windows/Win32/System/Com/fn.CoCreateInstance.html, the rust definition is missing `riid` and `ppv` parameters.
[Embed: CoCreateInstance in windows::Win32::System::Com - Rust]
API documentation for the Rust `CoCreateInstance` fn in crate `windows`.

[2025-06-16 08:35] rin: it actually does but i cant read this yet
[Attachments: image.png]

[2025-06-16 11:16] qw3rty01: [replying to rin: "does anyone know why there is a difference between..."]
looks like it automatically provides the riid and ppv based on the output `T`, if you use the windows-sys version, it’s the same definition as msdn

[2025-06-16 11:30] dinero: [replying to contificate: "yeah, passsed last month"]
It’s crazy yall got actually strict tests

[2025-06-16 11:31] dinero: I had a learners permit only as early as 16 but no car.  Then at 21 Florida DMV just gave me my full license  no test or anything

[2025-06-16 11:32] dinero: I’d unironically never driven a car more than once before. Just figured I’d learn by doing and that was 15,000 miles ago 👍

[2025-06-16 11:32] dinero: oh just realized it’s <#835664858526646313> sorry

[2025-06-16 14:37] contificate: yeah and you only have automatic license bro crazy

[2025-06-16 14:38] contificate: but yeah aimless driving is the shig

[2025-06-16 14:38] contificate: Shit

[2025-06-16 14:38] contificate: <@162611465130475520> can you link that llvm thing u mentioned in Poland

[2025-06-16 16:01] mrexodia: [replying to contificate: "<@162611465130475520> can you link that llvm thing..."]
Yes

[2025-06-16 16:02] mrexodia: https://github.com/tpde2/tpde
[Embed: GitHub - tpde2/tpde: A fast framework for writing baseline compiler...]
A fast framework for writing baseline compiler back-ends in C++ - tpde2/tpde

[2025-06-16 19:28] dlima: https://youtu.be/qIDFyhtUMnQ?si=GL-BjU2Wz25RYZsm
[Embed: C++ Weekly - Ep 485 - Variadic Structured Bindings in C++26]
☟☟ Awesome T-Shirts! Sponsors! Books! ☟☟

Upcoming Workshops:
► C++ Best Practices Workshop, C++ On Sea, Folkestone, UK, June 26-27, 2025: https://cpponsea.uk/2025/session/cpp-best-practices

Episode 

[2025-06-16 19:28] dlima: Lol

[2025-06-16 20:03] Horsie: Is there an existing implementation for slicing the Xrefs to/from in ida?

[2025-06-16 20:04] Horsie: I want to find out, given a starting function x, how many ways can function y be reached.

[2025-06-16 20:18] rin: [replying to diversenok: "https://devblogs.microsoft.com/oldnewthing/2013111..."]
found  [this](https://github.com/microsoft/windows-rs/tree/master/crates/samples/windows/shell) rust port on grep.app while trying to re implement my own port if anyone is interested.
[Embed: windows-rs/crates/samples/windows/shell at master · microsoft/wind...]
Rust for Windows. Contribute to microsoft/windows-rs development by creating an account on GitHub.

[2025-06-16 20:27] Horsie: nice! I've lose count how many times I've wanted to interact  with a weird interface and grep. app/sourcegraph helpede locate a poc

[2025-06-16 20:27] Horsie: its saved me countless hours :D

[2025-06-19 21:21] rin: <@165987589482872832> what is your preferred way of handling result cases in rust when dealing with the windows api.

[2025-06-20 00:07] qw3rty01: I usually do a custom library-level error type and convert over like you would with io::Error, or you can just do the lazy way out and have a WinApiError variant and just ? it

[2025-06-20 04:58] dagger: ```cpp
#include <iostream>


int main()
{
    std::string mathematics{};
    std::cout << "Types of math available (type as stated): multiply, divide, add, subtract\n";
    std::cout << "Enter a type of math you would like to do: ";
    std::cin >> mathematics;
    int x{};
    int y{};

    if (mathematics == "multiply" || "divide" || "add" || "subtract") {
        std::cout << "Please enter your first integer you would like to: " << mathematics << ": ";
        std::cin >> x;
        std::cout << "Please enter your second integer you would like to " << mathematics << ": ";
        std::cin >> y;

        if (mathematics == "multiply") {
            std::cout << "The product of " << x << " and " << y << " is " << y * x;
        }
        if (mathematics == "divide") {
            std::cout << "The quotient of " << x << " and " << y << " is " << x / y;
        }
        if (mathematics == "add") {
            std::cout << "The sum of " << x << " and " << y << " is " << y + x;
        }
        if (mathematics == "subtract") {
            std::cout << "The difference of " << x << " and " << y << " is " << x - y;
        }
    }
    else {
        std::cout << "Invalid math operator stated, please restart the program and retry!";
        return 1;
    }

    return 0;
}
```
hey guys this is my first program, its pretty cluttered and let me know your recommendations such as what should i make a function and how can i make it more appealing or more efficient, thank you!

[2025-06-20 09:45] brymko: i mean looks good generally, i'd say don't learn the retarded << and >> syntax, better yet don't learn c++ at all because the comittee is a real world implementation of the CIA handbook on how to sabotage organizations, which in turn gives you a shitty language. Apart from that 
```c++
 if (mathematics == "multiply" || "divide" || "add" || "subtract") 
``` 
this is not a thing, you are checking the const char* pointer value for null. You need to == every string with mathematics like:
```c++
 if (mathematics == "multiply" || mathematics ==  "divide" || mathematics == "add" || mathematics == "subtract") 
```
It's also not needed because you compare against it directly after, which can be a if ... else if ... else if ... else if .... else chain instead.

[2025-06-20 10:14] Timmy: > better yet don't learn c++ at all because the comittee is a real world implementation of the CIA handbook on how to sabotage organizations, which in turn gives you a shitty language
LOL, I mean not wrong, but maybe it's not the best thing to bring up to a newbie? idk. For context in practice this means nobody uses the entire c++ ecosystem and a lot of things in the language and standard library are just plain bad. Large part of the complexity of learning C++ is figuring out what belongs in which category in which situation.

~~std::cout~~ -> `std::println`

[2025-06-20 10:56] contificate: I'd just tabulate the ops, no if chainingnat all

[2025-06-20 10:58] Lasagne: [replying to dagger: "```cpp
#include <iostream>


int main()
{
    std:..."]
https://tenor.com/view/mmm-good-smoke-cigarette-smoking-gif-15715252

[2025-06-20 10:58] Lasagne: bueno

[2025-06-20 11:01] contificate: using `{}` everywhere can be redundant

[2025-06-20 11:01] contificate: as it is in the `std::string` case

[2025-06-20 11:01] contificate: it'll zero the ints, which should be populated later anyway (nevermind you populate them conditionally)

[2025-06-20 11:16] donna🤯: [replying to dagger: "```cpp
#include <iostream>


int main()
{
    std:..."]
I like to return defined status codes such as EXIT_SUCCESS or EXIT_FAILURE rather then 0 and 1 - generally these macros are defined or else define your own

[2025-06-20 11:17] donna🤯: I also like the habit of defining variables together at the start of the scope, in your case you define mathemetics, print something, then define your ints - id define them all together

[2025-06-20 13:54] daax: [replying to Timmy: "> better yet don't learn c++ at all because the co..."]
yeah would agree, a lot of things are good, some things are bad. cpp isn’t going anywhere though

[2025-06-20 14:05] roddux: uhhhhh is a stack array in a `static` function guaranteed to be zero-init?

[2025-06-20 14:07] roddux: like, is this correct?
```c
static int foo(int needle) {
    u8 haystack[32];
    for(int i=0;i<32;i++) if(haystack[i] == needle) return 1;
    return 0;
}
```

I get a warning when I throw `-Wall -Wextra` but idk

[2025-06-20 14:12] Timmy: [replying to roddux: "uhhhhh is a stack array in a `static` function gua..."]
static on a function means it's local to the translation unit it's defined in

[2025-06-20 14:13] roddux: I understand that

[2025-06-20 14:13] roddux: may I take that as 'there are no further guarantees about what `static` means' ?

[2025-06-20 14:13] diversenok: Don't confuse static functions and static variables

[2025-06-20 14:14] roddux: i'm not; i'm specifically asking about a variable _in_ a static function

[2025-06-20 14:14] roddux: my assumption is that the variable is not static, so there's no guarantee of it being zero-init

[2025-06-20 14:14] Timmy: a static function does not have meaning for the codegen of the function body

[2025-06-20 14:16] roddux: in other words then, no, the snippet i posted is not correct

[2025-06-20 14:16] roddux: as the `haystack` is uninitialized

[2025-06-20 14:17] diversenok: `haystack` is just a variable on the stack so not zero-initialized

[2025-06-20 14:17] diversenok: Also you can easily test it

[2025-06-20 14:20] roddux: I did, and I wrote this
```c
#include <stdio.h>
static int foo(int needle) {
    u8 haystack[32];
    for(int i=0;i<32;i++) if(haystack[i] != 0) return 1; // any nonzero
    return 0;
}

int main() {
    if(foo(23)) puts("nonzero");
    return 0;
}
```

and I ran..: `while :; do ./a.out ; done`

I've not had a single hit

[2025-06-20 14:20] roddux: could just be due to stack layout of my example i guess

[2025-06-20 14:21] roddux: but even putting in `-fsanitize=address,undefined` doesn't give any errors

[2025-06-20 14:21] diversenok: Yeah, I suppose you need something to dirty the stack before the call

[2025-06-20 14:22] roddux: oho

[2025-06-20 14:22] roddux: yeah throwing in another function to set stack variables does indeed trigger `found`

[2025-06-20 14:24] roddux: cheers both for being a sounding board, and helping me check i'm not going nuts here 🙂

[2025-06-20 14:24] diversenok: `haystack` would be zero-initialized if you declare the variable as static

[2025-06-20 14:24] diversenok: But declaring a function static doesn't make its variables static

[2025-06-20 14:34] Yoran: ```c
#include <stdio.h>

int main()
{
    static int *ps;
    int *pns;
    if (ps == NULL) printf("Bonkers");
    if (pns == NULL) printf("Conkers");


    return 0;
}
```
output: Bonkers

[2025-06-20 14:35] Yoran: Always good learning something new

[2025-06-20 14:35] Yoran: I wonder why they did that

[2025-06-20 14:36] Yoran: [replying to Yoran: "I wonder why they did that"]
Ahhh, cause you can zero them for free you just put them in the BSS section

[2025-06-20 14:37] diversenok: [replying to Yoran: "I wonder why they did that"]
It's not that they did it explicitly. It's a side effect of where the variable is stored

[2025-06-20 14:37] diversenok: static variables are stored in a dedicated section in executable

[2025-06-20 14:38] roddux: "for free" lol

[2025-06-20 14:38] roddux: it's not free

[2025-06-20 14:38] roddux: the kernel just does it instead

[2025-06-20 14:38] diversenok: Regular local variables are on the stack. At addresses that get reused

[2025-06-20 14:38] Yoran: [replying to roddux: "it's not free"]
Well, yeah

[2025-06-20 14:38] roddux: sorry that was very pedantic

[2025-06-20 14:39] Yoran: [replying to roddux: "sorry that was very pedantic"]
Nah its cool

[2025-06-20 14:39] Yoran: Good clarification

[2025-06-20 14:40] diversenok: [replying to Yoran: "Ahhh, cause you can zero them for free you just pu..."]
It also changes semantics though

[2025-06-20 14:40] diversenok: By reserving storage for the variable, even when the function isn't called

[2025-06-20 14:41] Yoran: right

[2025-06-20 14:42] diversenok: Try calling the function recursively. The same local variable within nested calls can have different values because it's stored on the stack. A static variable will be shared

[2025-06-21 12:56] contificate: [replying to diversenok: "It's not that they did it explicitly. It's a side ..."]
it is explicit

[2025-06-21 12:56] contificate: it's part of the C standard

[2025-06-21 12:59] contificate: 
[Attachments: 2025-06-21-135916_879x300_scrot.png]

[2025-06-21 12:59] contificate: the section being zero initialised is just how they implement the standard - it is an explicit design choice

[2025-06-21 13:00] contificate: in other words, it's not undefined behaviour to query the value of a `static` with a scalar type, as it's meant to be zero'd

[2025-06-21 13:00] contificate: whereas it is to use the value of a local that's not initialised explicitly

[2025-06-21 13:04] diversenok: Okay, I should've been more explicit (pun intended)

[2025-06-21 13:04] diversenok: I meant that local variables not being zero-initialized is an implementation detail

[2025-06-21 13:04] diversenok: They could end up zero, or they could end up something else

[2025-06-21 13:05] diversenok: I suppose choosing not to explicitly make them zero as part of the standard is a design choice

[2025-06-21 13:05] diversenok: But that would be really expensive. So is it really a choice if you cannot reasonably choose another option?

[2025-06-21 13:08] diversenok: As for static variables - sure

[2025-06-21 13:13] contificate: yeah, yeah, I'm just emphasising that

[2025-06-21 13:13] contificate: it's not a mere side effect

[2025-06-21 13:13] contificate: the section is zero'd

[2025-06-21 13:13] contificate: because of the standard

[2025-06-22 03:29] selfprxvoked: [replying to Yoran: "I wonder why they did that"]
The output could also be BonkersConkers, since using `pns` without initializing it's undefined behavior in the C standard