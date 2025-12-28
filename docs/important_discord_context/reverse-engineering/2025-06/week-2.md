# June 2025 - Week 2
# Channel: #reverse-engineering
# Messages: 189

[2025-06-02 03:51] 0xatul: [replying to ruan: "theres any shortcut on ida pro to give me the addr..."]
just do `idc.here()` on the console ?

[2025-06-02 13:29] the horse: ICED C++ BINDINGS SOON HOPEFULLY 🙏
[Attachments: image.png]

[2025-06-02 13:30] Brit: a new way to discover emu bugs inshallah

[2025-06-02 13:55] NSA, my beloved<3: Hey! Does anyone have any methods of syncing the cursor between IDA Pro and x64dbg please? ret-sync does not offer such thing and could not find any alternatives. Constantly having to rebase the image in IDA or placing a million breakpoints isn't ideal.

[2025-06-02 14:12] the horse: [replying to NSA, my beloved<3: "Hey! Does anyone have any methods of syncing the c..."]
you could just use relative addressing

[2025-06-02 14:12] the horse: as a ghetto fix

[2025-06-02 14:18] NSA, my beloved<3: [replying to the horse: "you could just use relative addressing"]
Thanks, I'm doing that for the time being.

[2025-06-02 14:22] NSA, my beloved<3: It's insane how you have to use a plugin to grab a single file offset in IDA.

[2025-06-02 14:22] NSA, my beloved<3: It does not offer such functionality be default.

[2025-06-02 14:26] pinefin: ive never worked with something with such an amount of templates, but what i usually do is something like this

```c++
template <typename T>
struct template_test_t {
    T& get() {
        return val;
    }
private:
    T val;
};

//and here is where u place your breakpoint
template <>
uint32_t& template_test_t<uint32_t>::get() {
    printf("hello im in a different function\n");
    return val;
}
```

[2025-06-02 14:27] pinefin: i only recently learned how to do this (lel, i dont work with templates much). idk if it's useful for you

[2025-06-02 14:27] pinefin: just wanted to put that out there incase it helps

[2025-06-02 14:32] pinefin: oh

[2025-06-02 14:33] pinefin: then i have no clue how i can help. your explanation definitely sounds like you're trying to debug debug them

[2025-06-02 14:34] pinefin: good luck!

[2025-06-02 14:56] the horse: [replying to NSA, my beloved<3: "It's insane how you have to use a plugin to grab a..."]
it might just have a command for it

[2025-06-02 14:56] the horse: via idc

[2025-06-02 14:56] the horse: does it not?

[2025-06-02 14:56] the horse: it's a lot more annoying though

[2025-06-02 14:56] NSA, my beloved<3: I don't think so. Googling the topic only revealed one plugin, that does not even work correctly.

[2025-06-02 14:57] the horse: i gotta revive my picanha disassembler 😔

[2025-06-02 15:00] NSA, my beloved<3: What is that?

[2025-06-02 17:31] pinefin: [replying to the horse: "i gotta revive my picanha disassembler 😔"]
KUBERA!

[2025-06-02 17:31] pinefin: [replying to NSA, my beloved<3: "I don't think so. Googling the topic only revealed..."]
imma be honest i keep it at 0x1400000000 and i keep a notepad file. its really tedious and wish there was actually better pipeline between the two

[2025-06-02 17:32] pinefin: [replying to NSA, my beloved<3: "What is that?"]
its one of his projects, i think we're allowed to attach screenshots of which? so im going to

[2025-06-02 17:33] pinefin: 
[Attachments: image.png]

[2025-06-02 17:33] pinefin: old screenshot* but shows u, it has a really high potential for what it is

[2025-06-02 17:34] pinefin: [replying to the horse: "ICED C++ BINDINGS SOON HOPEFULLY 🙏"]
i was thinking about going down this rabbit hole <a:kek:899071675998552094> but lord have mercy good luck...kind share

[2025-06-02 17:53] ruan: I'm working on a python script that writes comments to some lines in the decompiled code view
but the comment is not showing until i set/modify any line in the view myself, looks like a Qt QWidget "update"/"repaint" issue
does someone know a way to force IDA Pro 9.1 refresh the view from a python script?

[2025-06-02 18:51] Brit: you using set_cmt ?

[2025-06-02 18:51] Brit: try this https://python.docs.hex-rays.com/namespaceida__kernwin.html#a532003b12e92e4ba9f5a80023b63248f

[2025-06-02 19:17] ruan: [replying to Brit: "try this https://python.docs.hex-rays.com/namespac..."]
this worked, ty ❤️

[2025-06-02 19:35] Brit: this is if request refresh didnt work

[2025-06-02 19:35] Brit: since that forces it

[2025-06-03 03:00] pinefin: [replying to ruan: "I'm working on a python script that writes comment..."]
wait i didnt know u could write comments in ida python...perfect timing i need this, i need to def go and look into making ida py stuff now

[2025-06-03 03:12] ruan: does someone know a way to stop ida output window from auto scrolling bottom whenever it prints a new msg

[2025-06-03 05:14] the horse: [replying to pinefin: "old screenshot* but shows u, it has a really high ..."]
soon opensourceTM

[2025-06-03 05:15] the horse: after i finish my iced C++ wrapper

[2025-06-03 05:15] the horse: if all goes well it should bring down the time to analyze a 200mb dll from 36s to like 15s

[2025-06-03 05:15] the horse: at least from the current perceived speedup from capstone to iced

[2025-06-03 05:57] James: Typically, pattern based approaches are faulty... But it depends where you implement them. If you could explain how your deobfuscator works in a less abstract way I could give you more advice. 

An example: If you decide that a common pattern to deobfuscate is the following simple example.
```
ADD REG,X
ADD REG,1
=>
ADD REG,(X+1)
```

If you do this at the x86 level it becomes difficult because I could schedule those instructions to be very far apart from one another, making a pattern of raw bytes or x86 instructions impractical.

But if you were to look for an AST with this form at some sort of IR level, that would work much better.

Anyway, something more concrete and then I can say more.

[2025-06-03 06:22] the horse: Implement tests

[2025-06-03 06:22] the horse: you can auto-generate different variants of the same obfuscation with asmjit

[2025-06-03 06:22] the horse: then try to auto-search for the obfuscation pattern, and apply the deobfuscation

[2025-06-03 06:22] the horse: and for clarity, attempt to execute the deobfuscated code with a sandbox or an emulator

[2025-06-03 06:23] the horse: why so?

[2025-06-03 06:23] the horse: If you're pattern-matching, there is likely a consistent pattern

[2025-06-03 06:23] the horse: if you're matching a larger part of a function with unknown data in between, then I can imagine it's more difficult

[2025-06-03 06:23] the horse: if your issue is with the use of other registers or something, that won't be a problem to do tests for (as you can generate these variants with asmjit just fine)

[2025-06-03 06:25] the horse: it's gonna be a bit painful to set up, but you can employ a degree of polymorphism/randomization into the generated functions for more real-world-like code, perhaps employ Vector35's polymorphic shellcode generator

[2025-06-03 06:26] the horse: https://github.com/Vector35/scc
[Embed: GitHub - Vector35/scc]
Contribute to Vector35/scc development by creating an account on GitHub.

[2025-06-03 06:26] the horse: that's pretty much where pattern matching falls apart

[2025-06-03 06:26] the horse: like <@160202062548697108> mentioned

[2025-06-03 06:27] the horse: pattern matching is usually enough for a simple obfuscation engine and/or some annoying disassembly tricks like call+5

[2025-06-03 06:28] the horse: yes, if you don't need it at the moment, you can do without it

[2025-06-03 06:28] the horse: if you have constant substitution patterns which are somewhat continuous in memory, then it should be relatively safe to rewrite

[2025-06-03 06:29] the horse: can you explain the check further

[2025-06-03 06:29] the horse: you mean if the pushed rcx value would be used?

[2025-06-03 06:30] the horse: identify unused registers, preload the value and keep the push

[2025-06-03 06:30] the horse: (conforming to ABI)

[2025-06-03 06:31] the horse: in larger functions and especially functions with control-flow it's a bit more annoying

[2025-06-03 06:31] the horse: if the function is not branching, you have enough space to push and pop/use the stack

[2025-06-03 06:32] the horse: start small

[2025-06-03 06:32] the horse: rewrite safe functions, especially those that do not transfer control-flow

[2025-06-03 06:32] the horse: test for correctness, and if the application continues executing fine

[2025-06-03 06:32] the horse: then you can deobfuscate another outer level

[2025-06-03 06:32] the horse: this will be the least error-prone approach

[2025-06-03 06:33] the horse: tag all functions you touch and the error surface decreases and becomes more debuggable

[2025-06-03 06:33] the horse: eventually you will catch the remaining culprits and rewrite the entire bin, but this stage-based approach would probably be very suitable for you

[2025-06-03 06:34] the horse: especially if you're not lifting to an IR

[2025-06-03 06:34] the horse: if most of the substitution creates such large operations, you have a lot of freedom for safe rewriting

[2025-06-03 06:35] the horse: if you haven't already, I'd minimally recommend writing a executable disassembling engine (not a decoder), to have somewhat reliable function & block detection

[2025-06-03 06:36] the horse: I do not know your current architecture but if you're just doing a linear sweep and trying to deobfuscate any matches this won't be very safe

[2025-06-03 06:37] the horse: you're pushing a finished state constraint for no reason

[2025-06-03 06:37] the horse: and this will make your advances slower

[2025-06-03 06:37] the horse: explore all paths

[2025-06-03 06:38] the horse: you will optimize it out eventually

[2025-06-03 06:39] the horse: or you can emulate

[2025-06-03 06:40] the horse: with concrete/symbolic execution

[2025-06-03 06:41] the horse: well you only have to do it once 😄

[2025-06-03 06:42] the horse: what type of binary is this

[2025-06-03 06:42] the horse: PE?

[2025-06-03 06:42] the horse: did you try using unwind info?

[2025-06-03 06:42] the horse: if the application has runtime_function entries

[2025-06-03 06:42] the horse: you can very reliably detect function starts

[2025-06-03 06:43] the horse: and explore those paths

[2025-06-03 06:44] the horse: do the same with function you get from unwind info

[2025-06-03 06:44] the horse: very good

[2025-06-03 06:44] the horse: I do that in my disasm engine

[2025-06-03 06:45] the horse: you'd emulate from entry point

[2025-06-03 06:45] the horse: https://github.com/momo5502/sogen
[Embed: GitHub - momo5502/sogen: 🪅 Windows User Space Emulator]
🪅 Windows User Space Emulator . Contribute to momo5502/sogen development by creating an account on GitHub.

[2025-06-03 06:46] the horse: ```cpp
  void Disassembler::process_unwind_data ( ) {
    std::println ( "[engine] processing unwind data for function detection" );
    Profiler profiler ( "process_unwind_data" );

    uint64_t image_base = pe_parser_->get_image_base ( );
    auto exception_data = pe_parser_->get_exception_directory ( );

    if ( exception_data.empty ( ) ) {
      std::println ( "[engine - unwind] No exception directory data found." );
      return;
    }

    std::println ( "[engine - unwind] Found {} exception entries.", exception_data.size ( ) );
    std::vector<uint64_t> unwind_candidates;
    unwind_candidates.reserve ( exception_data.size ( ) );
    int valid_count = 0;
    for ( const auto& [runtime_func, unwind_data] : exception_data ) {
      uint32_t begin_rva = runtime_func.begin_address;
      // uint32_t end_rva = entry.runtime_function.end_address; // Could use end address for size hint?

      uint64_t start_addr = image_base + begin_rva;

      if ( is_valid_addr ( start_addr ) && is_executable_addr ( start_addr ) ) {
        if ( !visited_.contains ( start_addr ) ) {
          unwind_candidates.push_back ( start_addr );
          valid_count++;
        }
      }
      else {
        // std::println("[engine - unwind] Skipping invalid/non-exec entry at RVA {:#x} (Abs: {:#x})", begin_rva, start_addr);
      }
    }
```

[2025-06-03 06:46] the horse: ```cpp
  auto Parser::get_exception_directory ( ) const -> ExceptionDirectoryData {
    const auto& exception_dir = pe_info_.data_directories [ 3 ];
    if ( exception_dir.virtual_address == 0 ) {
      return {};
    }

    ExceptionDirectoryData result;
    size_t offset = rva_to_offset ( exception_dir.virtual_address );
    uint32_t entry_count = exception_dir.size / sizeof ( RuntimeFunction );
    result.reserve ( entry_count );

    for ( uint32_t i : std::views::iota ( 0u, entry_count ) ) {
      auto func = read_struct<RuntimeFunction> (
          buffer_, offset + i * sizeof ( RuntimeFunction ) );

      auto resolved_func = resolve_chained_function ( offset, func );
      std::optional<UnwindInfo> unwind_info;
      if ( resolved_func.unwind_info_address ) {
        try {
          unwind_info = read_struct<UnwindInfo> (
              buffer_, rva_to_offset ( resolved_func.unwind_info_address ) );
        }
        catch ( ... ) {
          unwind_info = std::nullopt;
        }
      }

      result.emplace_back ( resolved_func, unwind_info );
    }
    return result;
  }
```

[2025-06-03 06:46] the horse: ```cpp
  auto Parser::resolve_chained_function ( uint64_t base_offset, RuntimeFunction func ) const -> RuntimeFunction {
    if ( !func.unwind_info_address ) {
      return func;
    }

    try {
      auto unwind_info = read_struct<UnwindInfo> (
          buffer_, rva_to_offset ( func.unwind_info_address ) );

      if ( ( unwind_info.flags & 0x4 ) != 0 ) { // UNW_FLAG_CHAININFO
        uint32_t index = unwind_info.count_of_codes;
        if ( ( index & 1 ) != 0 ) {
          index += 1;
        }

        uint64_t chain_offset = rva_to_offset ( func.unwind_info_address ) +
          offsetof ( UnwindInfo, unwind_code ) +
          ( index * sizeof ( UnwindCode ) );

        auto chain_func = read_struct<RuntimeFunction> ( buffer_, chain_offset );
        return resolve_chained_function ( base_offset, chain_func );
      }
    }
    catch ( ... ) {
      return func;
    }

    return func;
  }
```

[2025-06-03 06:47] the horse: sogen supports threads

[2025-06-03 06:47] the horse: so if it creates other threads

[2025-06-03 06:47] the horse: you get the paths from them as well

[2025-06-03 06:47] the horse: would require a lot of modifications though

[2025-06-03 06:47] the horse: interactibility kind of breaks this usually though

[2025-06-03 06:47] the horse: since you won't get past that

[2025-06-03 06:48] the horse: that's where you need symbolic execution

[2025-06-03 06:48] the horse: then you'd need to re-run it a lot of times

[2025-06-03 06:48] the horse: adhesive has an annoying path randomizer with opaque predicates

[2025-06-03 06:48] the horse: (fivem anticheat)

[2025-06-03 06:48] the horse: makes like 16 different paths where half of them are real

[2025-06-03 06:49] the horse: they're identical in semantics but the obfuscated code is different for each one

[2025-06-03 06:49] the horse: very annoying

[2025-06-03 06:49] the horse: I wouldn't even bother with that at all without lifting to LLVM

[2025-06-03 06:50] the horse: https://github.com/NaC-L/Mergen might be of help
[Embed: GitHub - NaC-L/Mergen: Deobfuscation via optimization with usage of...]
Deobfuscation via optimization with usage of LLVM IR and parsing assembly. - NaC-L/Mergen

[2025-06-03 06:50] the horse: it's really a shame almost all the promising projects re-oriented towards unix and dropped windows support

[2025-06-03 06:51] the horse: oof

[2025-06-03 06:51] the horse: luckily mergen is still alive

[2025-06-03 06:51] the horse: to hopefully finally have a stable bin2bin deobfuscator

[2025-06-03 06:53] the horse: yeah realistically you only encounter such problematic bins with games

[2025-06-03 06:54] the horse: question is if static analysis is required in the first place

[2025-06-03 06:54] the horse: if you're trying to evade detections enough data is usually gathered with dynamic methods

[2025-06-03 06:56] the horse: why?

[2025-06-03 06:56] the horse: well honestly using & improving mergen is your best bet atm imo

[2025-06-03 06:57] the horse: if you actually need re-executability it's more annoying

[2025-06-03 06:57] the horse: if you still want to do things your way

[2025-06-03 06:57] the horse: yeah then try to use my tips

[2025-06-03 06:57] the horse: with your deobfuscator

[2025-06-03 06:57] the horse: get more concrete starting points for function-level deobfuscation with unwind data, start small

[2025-06-03 06:57] the horse: and work your way through it

[2025-06-03 06:58] the horse: it will definitely take a long time

[2025-06-03 06:59] the horse: the algorithms needed for a solid optimizer are very hard to write

[2025-06-03 07:00] the horse: I was tweaking some ideas for a couple weeks but decided I need a lot more concrete tooling to even start

[2025-06-03 07:00] the horse: lifting to an SSA-like format is probably what you need most

[2025-06-03 07:00] the horse: and it will allow you to do a lot of optimizations easier

[2025-06-03 07:00] the horse: especially identifying dead code

[2025-06-03 07:01] the horse: yeah the conditions and checks are the hard part

[2025-06-03 07:02] the horse: especially when indirect control flow is introduced

[2025-06-03 07:03] the horse: if you know all the paths sure

[2025-06-03 07:03] the horse: jmp/jcc that you don't know the destination of and it falls apart

[2025-06-03 07:04] the horse: perhaps you could have a purpose for emulation for branch discovery for matched functions

[2025-06-03 07:04] the horse: ah ok

[2025-06-03 17:26] James: Ok the example he provided me in DMs does seem like a good place for pattern matching actually:

```
/// push r9
/// xor r9,r9
/// bt rdi,3F
/// sbb r9,0
/// shrd rdi,r9,30
/// pop r9
/// To:
/// sar rdi,30
```

[2025-06-03 17:26] James: I think it also really depends on the type of obfuscator you're targeting.

[2025-06-03 17:28] James: If you're dealing with something that does just rewriting at the assembly level, they can't really do anything all that complex tbh. They are forced to save/restore registers like the above example, and typically the transforms are simple local rewrites, because anything else is much more difficult to do.

[2025-06-03 17:28] James: A common example is the "junk" code you see in many existing obfuscators where they do some dead register analysis on raw x86 code and decide where they can insert "junk" that won't clobber any live registers.

[2025-06-03 17:30] James: For obfuscators doing things like the above, lifting out of some raw x86 representation is overkill.

[2025-06-03 17:33] James: When you have clear bounds for the transformed expression like above with the PUSH/POP R9, frankly, you might as well not even use the obfuscator...

[2025-06-03 17:33] James: because it's just going to get blasted apart.

[2025-06-03 17:34] James: In fact in that expression there, the only reason IDA won't seemlessly optimize it in is because of that BT and SBB, both of which are doing very obvious things...

[2025-06-03 17:40] Brit: another push reg pop reg with garbage in the middle please the children are starving

[2025-06-03 17:41] Brit: if this is what the debate was about the whole time, you pattern match these away with very little effort

[2025-06-03 17:47] James: It's denuvo supposedly. Which is surprising because those types of expressions are essentially useless.

[2025-06-03 17:50] James: Another note, to be SEH safe they can't be touching any memory or doing division inside of the PUSH/POP ranges...  And only a single PUSH/POP is safe. Any more and you will break the unwinder because if the second PUSH causes a unrecoverable stack overrun, the unwinder will begin unwinding as if there weren't 8 bytes added by the previous push.

[2025-06-03 18:08] x86matthew: [replying to Brit: "another push reg pop reg with garbage in the middl..."]
i saw a funny implementation of this recently

[2025-06-03 18:08] x86matthew: i'm sure the author thought it was very clever

[2025-06-03 18:08] x86matthew: but they failed to take flags into consideration in the junk code

[2025-06-03 18:08] x86matthew: inevitably leading to great results

[2025-06-03 18:41] Brit: ohno

[2025-06-03 18:41] Brit: at least the scatterbrain sample had the pushflags popflags pairs set up

[2025-06-03 18:42] Brit: to restore flags

[2025-06-03 18:42] Brit: pushfq popfq you get the concept

[2025-06-03 18:44] the horse: pushfq popfq 🙏

[2025-06-05 14:02] ShekelMerchant: does a physical network interface card help with kdnet debugging in any way?

[2025-06-05 14:03] ShekelMerchant: ie. if your vm is having trouble working with its virtual nic, will a physical nic solve this?

[2025-06-05 14:18] Horsie: Anyone here familiar with decoding MIDL-related data

[2025-06-05 14:18] Horsie: _MIDL_STUB_DESC->pFormatTypes

[2025-06-05 14:21] Horsie: 
[Attachments: image.png]

[2025-06-05 14:21] Horsie: I have a bunch of data that looks like this. I suspect that its what is being used to describe the structures used in a demarshalling operation.

[2025-06-05 14:22] Horsie: I've gone through the docs, but having a hard time.. <https://learn.microsoft.com/en-us/windows/win32/rpc/rpc-ndr-engine>

[2025-06-05 14:22] diversenok: Yeah, I'm also curious if there are tools for that. Like, it should be possible to do by hand, but I better not

[2025-06-05 14:24] diversenok: If you pass a simple .idl through the MIDL compiler, it will output .h and .c files with these structures, and they have a lot of comments in them

[2025-06-05 14:24] diversenok: These format strings are serialized variable-sized structures

[2025-06-05 14:27] diversenok: Here is an example
```c
static const test_MIDL_PROC_FORMAT_STRING test__MIDL_ProcFormatString =
    {
        0,
        {

    /* Procedure MyRemoteProc */

            0x0,        /* 0 */
            0x48,        /* Old Flags:  */
/*  2 */    NdrFcLong( 0x0 ),    /* 0 */
/*  6 */    NdrFcShort( 0x0 ),    /* 0 */
/*  8 */    NdrFcShort( 0x18 ),    /* x86 Stack size/offset = 24 */
/* 10 */    0x32,        /* FC_BIND_PRIMITIVE */
            0x0,        /* 0 */
/* 12 */    NdrFcShort( 0x0 ),    /* x86 Stack size/offset = 0 */
/* 14 */    NdrFcShort( 0x8 ),    /* 8 */
/* 16 */    NdrFcShort( 0x1a0 ),    /* 416 */
/* 18 */    0x40,        /* Oi2 Flags:  has ext, */
            0x2,        /* 2 */
/* 20 */    0xa,        /* 10 */
            0x1,        /* Ext Flags:  new corr desc, */
/* 22 */    NdrFcShort( 0x0 ),    /* 0 */
/* 24 */    NdrFcShort( 0x0 ),    /* 0 */
/* 26 */    NdrFcShort( 0x0 ),    /* 0 */
/* 28 */    NdrFcShort( 0x0 ),    /* 0 */

    /* Parameter param1 */

/* 30 */    NdrFcShort( 0x48 ),    /* Flags:  in, base type, */
/* 32 */    NdrFcShort( 0x8 ),    /* x86 Stack size/offset = 8 */
/* 34 */    0x8,        /* FC_LONG */
            0x0,        /* 0 */

    /* Parameter outArray */

/* 36 */    NdrFcShort( 0x12 ),    /* Flags:  must free, out, */
/* 38 */    NdrFcShort( 0x10 ),    /* x86 Stack size/offset = 16 */
/* 40 */    NdrFcShort( 0x2 ),    /* Type Offset=2 */

            0x0
        }
    };
```

[2025-06-05 14:28] Horsie: Ooh that would be really good

[2025-06-05 14:29] Horsie: [replying to diversenok: "If you pass a simple .idl through the MIDL compile..."]
This implies that I have the IDL though right?

[2025-06-05 14:29] Horsie: Thats not the case here. I just have the exe :)

[2025-06-05 14:30] diversenok: Indeed, it's not a solution, just an idea of how to approach the process of figuring out how to parse these binary structures

[2025-06-08 02:53] 13X: If when statically analysing a DLL and it has encrypted strings, whats the best way of finding out the runtime decryption routine/function?

[2025-06-08 04:27] 0xboga: <@503274729894051901>  <@394966100892319749> do you guys see our conversation? Did someone delete it?

[2025-06-08 04:28] diversenok: A portion of an earlier conversation also got erased

[2025-06-08 04:29] diversenok: (the part about MIDL decompilation and NDR parsing)

[2025-06-08 04:30] diversenok: Somebody purged too many messages

[2025-06-08 04:46] mtu: rip in pieces

[2025-06-08 08:16] 0xboga: [replying to diversenok: "A portion of an earlier conversation also got eras..."]
So the method of swapping the handle table pointer isn’t as trivial as I thought it would be even with NTQSI.
I need to first find the service’s handle to the said file but NtQueryObject requires the handle to be first duplicated to my context so I can query it, and can’t duplicate since it’s PPL.
I can use the RW to turn off PPL, then if required deal with their OB callback as well, but that’s a lot of work atp. Any ideas for a workaround?

[2025-06-08 08:17] 0xboga: Currently I have all kernel addresses of objects opened by the PPL service, don’t even know the ObjectType. Maybe read the object header? And then the Object type structure?

[2025-06-08 08:46] 0xboga: <@503274729894051901> even if I patch the file object held by the service to set the SharedWrite /Delete explorer fails to delete it?

[2025-06-08 12:02] diversenok: [replying to 0xboga: "Currently I have all kernel addresses of objects o..."]
Wdym you don't know the object type? `NtQuerySystemInformation` tells what it is

[2025-06-08 12:15] diversenok: [replying to 0xboga: "<@503274729894051901> even if I patch the file obj..."]
Crystal ball time. That's not exactly much info to guess what happened. Fails how, fails where? In `NtOpenFile`, in `NtSetInformationFile`? With what status? `STATUS_SHARING_VIOLATION`, `STATUS_ACCESS_DENIED`, `STATUS_CANNOT_DELETE`, `STATUS_FILE_CORRUPT_ERROR`, something else? Are you sure you patched the right field? And what field is the right anyway?

[2025-06-08 22:31] ruan: <@162611465130475520> I see you committed "ida9-improvements" in this extension: https://github.com/ioncodes/idacode

I'm trying to use it to debug python scripts on ida 9.1, but its not working properly
when debugging a script the extension **randomly** crashes and when this happens `vscode` start showing a lot of weird pop up errors "Debug adapter process has terminated unexpectedly (Extension host shut down)"
a lot of things on it seems to stop working like copilot chat, looks like `vscode` loses internet access, them I have to restart `vscode` to it behave normally again

Have you been able to get this extension working?

[2025-06-08 23:40] mrexodia: [replying to ruan: "<@162611465130475520> I see you committed "ida9-im..."]
Worked fine for me on IDA 9.0 🤷‍♂️

[2025-06-08 23:41] mrexodia: Make sure you select the right python version (3.12) with idapyswitch and install the `debugpy` version that works for that

[2025-06-08 23:42] mrexodia: And obviously don't use the release package, but master

[2025-06-08 23:46] ruan: [replying to mrexodia: "And obviously don't use the release package, but m..."]
I'm not using the release package, I have installed it directly from vscode

[2025-06-08 23:56] ruan: I've checked on idapyswitch its using 3.13.3150.1013, what exactly version of 3.12 you're using?