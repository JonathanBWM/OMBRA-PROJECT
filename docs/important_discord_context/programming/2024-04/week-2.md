# April 2024 - Week 2
# Channel: #programming
# Messages: 67

[2024-04-12 10:53] Terry: does anyone know if some code exists to relocate an entire function, not just the beginning like some hooking libraries do? I wasnt able to find anything expect these hooking libraries. Otherwise i have a long day ahead me

[2024-04-12 11:02] contificate: Sounds non-trivial honestly

[2024-04-12 11:04] Terry: I mean in theory just fix relative addresses (ez) and then tls, thats kinda where I got/am stuck

[2024-04-12 11:05] Terry: im just not sure exactly how that would work yet

[2024-04-12 11:07] Matti: wdym

[2024-04-12 11:08] Matti: you solved this problem generically *except* for TLS? that's where you got stuck?

[2024-04-12 11:08] Terry: i mean unless i have a major oversight

[2024-04-12 11:08] Terry: it is 7 am for me and im not just waking up

[2024-04-12 11:08] Terry: what else would be problomatic?

[2024-04-12 11:09] Matti: uh, well doing this relocating correctly I guess

[2024-04-12 11:09] Matti: which type of TLS are you referring to, the callbacks array in the PE directory or TLS explicitly referenced from code

[2024-04-12 11:10] Matti: the latter shouldn't need changing

[2024-04-12 11:12] Terry: the thing is, the function is fairly small (320 bytes), and its not going to change besides some data inside it so any function calls or some other static data would be fairly easy to hardcode. there is also only a few function calls so it really shouldnt be that bad. the tls is explcity referenced in the code. i can share the ida f5, if u wanted to take a glance at it

[2024-04-12 11:14] Terry: im not going to be executing under the thread, at least i dont want to

[2024-04-12 11:14] Terry: its not like it really matters, im just doing this for fun

[2024-04-12 11:14] Terry: *ideally* tho i could execute it my "copied" function from a seperate thread

[2024-04-12 11:17] Matti: aha, ok

[2024-04-12 11:17] Terry: I could place a hook and read the tls data tho

[2024-04-12 11:17] Matti: so it's one function, and it's trivial-ish

[2024-04-12 11:17] Terry: yeah.... mostly?

[2024-04-12 11:17] Terry: its majority math

[2024-04-12 11:17] Matti: I was assuming you were trying to solve the problem in general

[2024-04-12 11:17] Matti: ok sure

[2024-04-12 11:17] Terry: no just for 1 specific function

[2024-04-12 11:18] Terry: yeah a generic solution would be...

[2024-04-12 11:18] dullard: [replying to Terry: "does anyone know if some code exists to relocate a..."]
What is the function ? 
If its one thats from a system dll it could be dooable using the EXCEPTION_HANDLER record information

```c
    typedef struct _IMAGE_RUNTIME_FUNCTION_ENTRY {
        DWORD BeginAddress; /* Interested in this */
        DWORD EndAddress;
        union {
            DWORD UnwindInfoAddress;
            DWORD UnwindData;
        } DUMMYUNIONNAME;
    } _IMAGE_RUNTIME_FUNCTION_ENTRY, *_PIMAGE_RUNTIME_FUNCTION_ENTRY;
    
    typedef struct _IMAGE_RUNTIME_FUNCTION_ENTRY RUNTIME_FUNCTION, *PRUNTIME_FUNCTION;
```

Could likely just cut them out using BeginAddress and EndAddress and a jmp at the start and nop the rest

[2024-04-12 11:18] Matti: but I don't see how TLS code would be affected by this relocation

[2024-04-12 11:18] Matti: it would normally be referenced via the PEB

[2024-04-12 11:18] Matti: with the thread's TLS index

[2024-04-12 11:19] Matti: sorry, TEB*

[2024-04-12 11:19] Terry: [replying to dullard: "What is the function ? 
If its one thats from a sy..."]
its not a system dll, but im curious what do u mean exactly?

[2024-04-12 11:20] dullard: See update

[2024-04-12 11:20] Terry: [replying to Matti: "sorry, TEB*"]
yeah i have 0 idea how this will work. the only thing i can think of now is copying the tls data, or pointing my tls to this functions tls

[2024-04-12 11:20] Terry: my tls should be completely empty

[2024-04-12 11:20] contificate: [replying to Terry: "I mean in theory just fix relative addresses (ez) ..."]
a bit more in theory, in all generality

[2024-04-12 11:21] Terry: [replying to dullard: "What is the function ? 
If its one thats from a sy..."]
I already have the function start/end and am able to get it automcaically. Im more interesting in copying the function so I can make some changes too it and execute it

[2024-04-12 11:21] Matti: [replying to Terry: "yeah i have 0 idea how this will work. the only th..."]
but my question is why would the code need changing if all it's doing is accessing some TLS variable

[2024-04-12 11:22] contificate: suppose you were doing this for ARMv7, where a compiler may use a constant pool after the last block in the lowered routine to store large constants (to load pc-relative), you'd need to also relocate that - but yeah practically it may amount to working out the bounds naively, but not possible in general

[2024-04-12 11:22] Matti: are you also giving this code a different thread

[2024-04-12 11:23] Terry: [replying to Matti: "but my question is why would the code need changin..."]
thats not all it doing, it just so happens to use tls. there is a hardcoded IDs which i need to change when I call it

[2024-04-12 11:23] Terry: [replying to Matti: "are you also giving this code a different thread"]
what do you mean? i plan to run  it from my own spawned thread if thats what you mean

[2024-04-12 11:24] Matti: yes, so you mean the thread's TLS index

[2024-04-12 11:24] Matti: [replying to Terry: "what do you mean? i plan to run  it from my own sp..."]
and yes that's what I mean

[2024-04-12 11:25] Terry: Yeah, it uses tls (i think on purpose to make me mad)

```c
v19 = NtCurrentTeb()->ThreadLocalStoragePointer;
    if ( !*(_BYTE *)(v19[TlsIndex] + 20i64) )
      sub_6E23FB0(v19, v16, v17);
    if ( *(_DWORD *)(*((_QWORD *)NtCurrentTeb()->ThreadLocalStoragePointer + (unsigned int)TlsIndex) + 976i64) == 2 )```

[2024-04-12 11:25] Matti: you will need to use your own thread's TLS index instead - if you are expecting data to already be located there (because some other code put it there previously) - well it won't be there

[2024-04-12 11:25] Matti: [replying to Terry: "Yeah, it uses tls (i think on purpose to make me m..."]
yes, that

[2024-04-12 11:25] Terry: [replying to Matti: "you will need to use your own thread's TLS index i..."]
yeah thats what I was saying above, i can hook the function and copy the data

[2024-04-12 11:26] Terry: or point my own thread to this one, im not sure if that breaks anything else tho

[2024-04-12 11:26] Matti: [replying to Terry: "yeah thats what I was saying above, i can hook the..."]
wdym by this

[2024-04-12 11:27] Lewis: [replying to Terry: "Yeah, it uses tls (i think on purpose to make me m..."]
LOL

[2024-04-12 11:27] Terry: [replying to Lewis: "LOL"]
not joking either...

[2024-04-12 11:27] Matti: you can
1. use the original thread's TLS index
or
2. use your thread's TLS index

both are bad in different ways, depending on what the rest of the code is doing

[2024-04-12 11:28] Matti: (2) might work fine if you are also storing the data

[2024-04-12 11:28] Matti: (1) might work fine if the other thread isn't simultaneously accessing its own TLS data too

[2024-04-12 11:31] Terry: yeah this all sounds like a headache. i may be able to just rip the tls out of this function in the first place. at least once its initialized. im going to confirm quickly

[2024-04-12 11:43] Terry: okay it looks like i can just ignore the tls. im just going to have fix some conditional jumps and progmatically search for any refrence in the function and remove it. should be *that* hard i suppose

[2024-04-12 13:22] x86matthew: why are you trying to move a function in the first place? i'm not sure what your original problem is but i'm sure it can be solved in a better way

[2024-04-12 13:23] x86matthew: especially if you're having to resort to "ripping bits out" and hope for the best

[2024-04-12 13:27] Terry: [replying to x86matthew: "why are you trying to move a function in the first..."]
mmm not really. the problem is this function does EXACTLY what i want but its only for 1 ID. there used to be a function that took this id as a param, but it has since been inlined for whatever reason. i have searched through every single function (over 500) which reference what im looking for. due to the heavy amounts of encryption its not worth it to rebuild this function manually. this solution is somehow the easiest

[2024-04-12 13:27] Terry: [replying to x86matthew: "especially if you're having to resort to "ripping ..."]
hey but thats the fun part!

[2024-04-12 13:28] Terry: im doing all this for open source stuff so id be happy to link it after i upload it

[2024-04-12 13:28] Terry: even if there was a better solution this one is pretty unique and im learning a bit so no harm no foul

[2024-04-14 01:45] JustMagic: AFAIK no. Bitfields have very lax handling rules in the standard.

[2024-04-14 09:15] Timmy: try this guy here <@151110446441562112> 
```cpp
union F {
  u64 v;
  struct {
    u64 f1 : 27;
    u64 f2: 37;
  };
};
```

[2024-04-14 11:51] Timmy: I did, didn't notice my error sorry for the random ping xitan

[2024-04-14 20:20] mrexodia: sorry <@151110446441562112>

[2024-04-14 20:20] mrexodia: <:kappa:697728545631371294>