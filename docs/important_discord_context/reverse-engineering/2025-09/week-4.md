# September 2025 - Week 4
# Channel: #reverse-engineering
# Messages: 287

[2025-09-22 00:00] the horse: ```cpp
            ZydisMnemonic mnemonic = instr.mnemonic ( );
            if ( mnemonic == ZYDIS_MNEMONIC_MOV || mnemonic == ZYDIS_MNEMONIC_LEA ) {
              uint64_t target = instr.resolve_memory_target ( );
              if ( target && ( target >= base_addrs_.front ( ) && target < base_addrs_.back ( ) + sections_.back ( ).data.size ( ) ) &&
                  !section.executable ) {
                current_func.data_references.insert ( { target, {ip, section_idx} } );
              }
            }
            uint64_t target = 0;
            bool is_branch = false;
            if ( mnemonic == ZYDIS_MNEMONIC_CALL ) {
              target = instr.operand_count ( ) > 0 && instr.operands ( ) [ 0 ].type == ZYDIS_OPERAND_TYPE_MEMORY
                ? get_indirect_call_target ( instr, decoder ) : instr.near_branch_target ( );
              is_branch = true;
            }
            else if ( conditional_branch_bits [ mnemonic / 8 ] & ( 1 << ( mnemonic % 8 ) ) ) {
              target = instr.near_branch_target ( );
              is_branch = true;
            }
            else if ( mnemonic == ZYDIS_MNEMONIC_JMP ) {
              target = instr.operands ( ) [ 0 ].type == ZYDIS_OPERAND_TYPE_MEMORY
                ? get_indirect_call_target ( instr, decoder ) : instr.near_branch_target ( );
              is_branch = true;
            }
            else if ( mnemonic == ZYDIS_MNEMONIC_RET || mnemonic == ZYDIS_MNEMONIC_IRET ) {
              if ( !current_block.instructions.empty ( ) ) {
                current_block.end_addr = instr.ip ( );
                current_block.size = current_block.end_addr - current_block.start_addr;
                current_func.blocks.push_back ( std::move ( current_block ) );
              }
              break;
            }

```

[2025-09-22 00:02] the horse: ```cpp
    [[nodiscard]] inline Instruction decode ( ) noexcept {
      if ( remaining_size_ == 0 || ip_ < base_addr_ ) [[unlikely]] {
        return Instruction ( );
      }
      const uint8_t* current_ptr = data_ + offset_;
      ZydisDecodedInstruction instr;
      ZydisDecodedOperand operands [ ZYDIS_MAX_OPERAND_COUNT ];
      if ( ZYAN_SUCCESS ( ZydisDecoderDecodeFull ( &decoder_, current_ptr, remaining_size_, &instr, operands ) ) ) [[likely]] {
        Instruction result ( instr, operands, ip_ );
        ip_ += instr.length;
        offset_ += instr.length;
        remaining_size_ -= instr.length;
        return result;
      }
      return Instruction ( );
    }
```
it looks ok unless i'm blind

[2025-09-22 00:02] the horse: is there anything in your loop

[2025-09-22 00:02] the horse: or you just single step

[2025-09-22 00:02] the horse: after it

[2025-09-22 00:04] the horse: rebuild the whole project

[2025-09-22 00:04] the horse: what's the exception

[2025-09-22 00:05] the horse: if there is no exception

[2025-09-22 00:05] the horse: check the status

[2025-09-22 00:05] the horse: cuz that might just random shit

[2025-09-22 00:05] the horse: because you didn't default init

[2025-09-22 00:05] the horse: instruction

[2025-09-22 00:08] the horse: rust_offsets.dll

[2025-09-22 00:08] the horse: 💔

[2025-09-22 00:09] the horse: why do you even need to inject into rust for offsets

[2025-09-22 00:09] the horse: L

[2025-09-22 00:10] the horse: https://tenor.com/view/orange-cat-gets-flung-and-explodes-orange-cat-funny-cat-meme-explodes-gif-10706110874965244466

[2025-09-22 00:10] the horse: should use icedpp like a boss

[2025-09-22 00:10] the horse: but yeah try a clean project, pull from vcpkg

[2025-09-22 00:11] the horse: also try heap allocating the buffer

[2025-09-22 00:11] snowua: Turn on ubsan 😞

[2025-09-22 00:11] the horse: [replying to snowua: "Turn on ubsan 😞"]
sorry king, msvc

[2025-09-22 00:11] abu: 🤮

[2025-09-22 00:12] snowua: I have a feeling youre omitting some relevant code which is causing some UB ¯\_(ツ)_/¯

[2025-09-22 00:13] the horse: i've had weird scenarios with zydis

[2025-09-22 00:13] the horse: i'll take capstone over zydis any day i don't need performance

[2025-09-22 00:13] the horse: in which case i'll use iced

[2025-09-22 00:13] the horse: however i am rumbling idiot

[2025-09-22 00:27] the horse: huh

[2025-09-22 00:48] daax: what the vibecode

[2025-09-22 01:31] UJ: you are hardcoding it to meme

[2025-09-22 01:34] UJ: ah i misread this. i thought you were trying to set the address to the entrypoint of your dll to decode it as a sanity check.

[2025-09-22 02:41] the horse: 👏

[2025-09-22 02:43] the horse: i thoutht about that but thought you'd pull on the new projevt like i told u

[2025-09-22 02:43] the horse: (and as you said you would)

[2025-09-22 02:43] the horse: https://tenor.com/view/orange-cat-gets-flung-and-explodes-orange-cat-funny-cat-meme-explodes-gif-10706110874965244466

[2025-09-22 02:44] the horse: pull zydis

[2025-09-22 18:20] lennard.cpp: hi. i want to learn reverse engineering. i tried crackmes but they are either overwhelming or way to easy. i found none in between yet. can anyone link good sources to start with?

[2025-09-22 18:41] nidi: [replying to lennard.cpp: "hi. i want to learn reverse engineering. i tried c..."]
try to reverse old games

[2025-09-22 18:42] nidi: that's what i did and worked for me

[2025-09-22 18:42] nidi: gta sa is fun to work with

[2025-09-22 18:44] pinefin: i'll add that i had the same problem with crackmes when i started as well, i did what nidi is telling you to do and it got me to learn the basics a lot simpler. 

try to go down the path of using tools like x64dbg as well for dynamic reversal

if you dont want to do old games, try out simple desktop applications that arent made with electron

(try and make sure the games/apps you're working on are also not made in c# unless you want to go down that separate rabbit hole of reversal, much easier)

[2025-09-22 18:45] lennard.cpp: thx! ill try that

[2025-09-22 23:16] Edel: source engine games are well documented but some stuff also differs ever since they moved 64bit so you can't just copy&paste (which is good)

[2025-09-22 23:37] lennard.cpp: i think games are still out of my laague

[2025-09-22 23:37] lennard.cpp: i wrote some small programs to familaries myself with asm for now.

[2025-09-22 23:38] lennard.cpp: so what to loops, if statments, classes, ... look like

[2025-09-23 06:09] macwindow: hi, looking for some guides on using windbg for memory

[2025-09-23 06:41] Bony: [replying to Mintia: "Does anyone know why `MmMapIoSpace` could return N..."]
caching issue?

[2025-09-24 06:28] Horsie: Question about using LIEF for binary modification-

[2025-09-24 06:29] Horsie: I have a binary with .text section as follows:
```|Name       | Offset           | Size             | Virtual Address    | Virtual size     | Entropy   | Flags
|CODE       | 0x400            | 0x25a00          | 0x1000             | 0x25958          | 6.50      | CNT_CODE MEM_EXECUTE MEM_READ```

[2025-09-24 06:30] Horsie: Afaik, it should be okay to extend the section to page alignment without causing any relocation issues.

[2025-09-24 06:31] Horsie: So I used LIEF to modify it, even tried setting the physical and virtual sizes of the section manually.
```|CODE       | 0x400            | 0x25a1e          | 0x1000             | 0x26000          | 6.50      |```

[2025-09-24 06:33] Horsie: But I cant seem to understand why, LIEF truncates the content I'm adding at 0x26a00 (hinting towards the original physical size)

[2025-09-24 06:33] Horsie: Even though the modified section reflects the new values.

[2025-09-24 06:34] Horsie: Theres some wack shit going on. I'm using python so perhaps its a bug in the impl? Thinking that because theres some shit from the start of the next section thats duplicated in the area after 0x26a00

[2025-09-24 06:52] Horsie: I'm probably not doing some shit right. God I hate python. I'll just rewrite in c++

[2025-09-24 08:12] abu: LOL

[2025-09-24 09:27] pygrum: [replying to Horsie: "But I cant seem to understand why, LIEF truncates ..."]
26a00? I see a 25a00 there

[2025-09-24 09:28] Horsie: [replying to pygrum: "26a00? I see a 25a00 there"]
size is 25a00, section start at 1000. hence this address

[2025-09-24 09:30] pygrum: what does ur lief code look like

[2025-09-24 09:33] x86matthew: [replying to Horsie: "Afaik, it should be okay to extend the section to ..."]
this is difficult to follow, your messages seem to be alternating between memory layout and file layout

[2025-09-24 09:33] pygrum: ^

[2025-09-24 09:33] x86matthew: are you trying to extend the .text section on disk to take up the remaining (file) padding?

[2025-09-24 09:36] the horse: wouldn't you be able to only write 0x6A8 bytes up to 0x26000

[2025-09-24 09:36] pygrum: I think he’s trying to extend text  on disk up to the next page alignment so he can use as much space before it overruns into the next section in memory

[2025-09-24 09:36] the horse: or actually no; it should be less

[2025-09-24 09:36] the horse: 0x25958 rounded up to file alignment if i'm not mistaken

[2025-09-24 09:37] the horse: what is the default? 0x200? 0x400?

[2025-09-24 09:37] pygrum: Rounded to the next page alignment and default is 0x200 iirc

[2025-09-24 09:37] the horse: if the module is mapped, you should have the entire 0x6A8 bytes available

[2025-09-24 09:37] pygrum: of file alignment

[2025-09-24 09:37] pygrum: Page alignment 1000

[2025-09-24 09:38] the horse: yeah in that case he can only write up to FO 0x25a00

[2025-09-24 09:38] the horse: on disk

[2025-09-24 09:38] pygrum: I think he can go past filealignment on disk

[2025-09-24 09:39] the horse: raw size -> 0x25a00

[2025-09-24 09:39] pygrum: if LIEF is doing it right

[2025-09-24 09:39] the horse: [replying to pygrum: "I think he can go past filealignment on disk"]
not safely though, right?

[2025-09-24 09:39] the horse: well he might be able to adjust it, I guess

[2025-09-24 09:39] the horse: but that would probably require resizing the file

[2025-09-24 09:40] the horse: i'm not familiar with LIEF, but you should be able to do this manually

[2025-09-24 09:40] the horse: have you adjusted the offsets for other sections?

[2025-09-24 09:41] pygrum: I’m assuming the section after code starts at 27000

[2025-09-24 09:42] the horse: shouldn't it start at 0x26000?

[2025-09-24 09:43] the horse: after mapping at least

[2025-09-24 09:43] the horse: from image base

[2025-09-24 09:44] the horse: this is essentially the problem
[Attachments: image.png]

[2025-09-24 09:44] the horse: he can safely overwrite code only adjusting the raw size within the bounds of FA entering .data

[2025-09-24 09:45] the horse: he can safely extend the .text section up to SA; provided he extends the file, adjust the FOs

[2025-09-24 09:46] the horse: I am not sure if LIEF is supposed to fix this up for you

[2025-09-24 09:47] the horse: i tend to do these things manually

[2025-09-24 09:47] pygrum: [replying to the horse: "but that would probably require resizing the file"]
Yeah and LIEF should be doing this properly

[2025-09-24 09:47] pygrum: [replying to the horse: "shouldn't it start at 0x26000?"]
0x1000 is where text starts, plus text size rounded to page alignment = 0x27000

[2025-09-24 09:47] the horse: that shouldn't implicate an additional padding of 0x1000 for the .data section

[2025-09-24 09:47] the horse: oh

[2025-09-24 09:47] the horse: you're right

[2025-09-24 09:47] the horse: mb

[2025-09-24 09:48] the horse: 0x26000 is the virtual size aligned 🙂 (not end of section)

[2025-09-24 09:51] pygrum: [replying to the horse: "if the module is mapped, you should have the entir..."]
Yeah exactly

[2025-09-24 09:52] pygrum: Should be able to write all of that without breaking relocs

[2025-09-24 09:53] pygrum: only if lief works right

[2025-09-24 12:20] Horsie: Sorry folks, got caught up with some stuff.

[2025-09-24 12:22] Horsie: Cant really share the file I'm working on but heres a snippet I wrote in cpp that has the same behavior.

[2025-09-24 12:27] Horsie: ```cpp
#include <cstdint>
#include <cstdio>
#include <memory>
#include <stdio.h>
#include <LIEF/LIEF.hpp>
#include <vector>

int main() {

    std::unique_ptr<LIEF::PE::Binary> my_pe = LIEF::PE::Parser::parse("input.exe");

    my_pe->optional_header().file_alignment(0x1000);
    my_pe->optional_header().section_alignment(0x1000);

    for(auto& sec : my_pe->sections()) {
        if(!sec.has_characteristic(LIEF::PE::Section::CHARACTERISTICS::MEM_EXECUTE)){
            continue;
        }
        std::printf("Section %s (at %llx)\n", sec.name().c_str(), sec.virtual_address());
        std::printf("\tSize is %llx\n", sec.size());
        std::printf("\tVirtual Size is %x\n", sec.virtual_size());

        auto old_raw_size = sec.size();
        auto old_virt_size = sec.virtual_size();
        sec.size(LIEF::align(old_raw_size, 0x1000));
        sec.virtual_size(LIEF::align(old_virt_size, 0x1000));

        sec.add_characteristic(LIEF::PE::Section::CHARACTERISTICS::ALIGN_4096BYTES);

        std::printf("Old content size: %llx\n", sec.content().size());
        std::vector<uint8_t> new_content(sec.content().begin(), sec.content().end());

        // Pad the content to new size
        for(auto _ = sec.virtual_size() - old_virt_size; _--;){
            new_content.push_back(0xFF);
        }
        sec.content(new_content);

        std::printf("New buffer_vec size: %llx\n", new_content.size());
        std::printf("New content size: %llx\n", sec.content().size());
        std::printf("New virtual size: %x\n", sec.virtual_size());
        std::printf("New physical size: %llx\n", sec.size());

    }
    my_pe->write("output.exe");
    
    puts("Fin\n");
}```

[2025-09-24 12:27] Horsie: ```>.\hello.exe
Section CODE (at 1000)
        Size is 25a00
        Virtual Size is 25958
Old content size: 25958
New buffer_vec size: 26000
New content size: 26000
New virtual size: 26000
New physical size: 26000
Fin```

[2025-09-24 12:28] Horsie: <@1021528599171448832> <@943099229126144030> I did realize that my prior message was really badly stated. Hope this provides more context

[2025-09-24 12:28] Horsie: Still facing the same issue.

[2025-09-24 12:29] Horsie: 
[Attachments: image.png]

[2025-09-24 12:30] Horsie: Notice the sudden cut off at 426a00, the data after that and till right before the DATA section, its just content from the start of DATA thats padded there for whatever reason.

[2025-09-24 13:04] x86matthew: i still don't really understand what you're trying to do

[2025-09-24 13:05] x86matthew: idk anything about this LIEF thing but your file alignment is probably 0x200

[2025-09-24 13:05] x86matthew: in which case extending up to 0x426a00 makes sense

[2025-09-24 13:06] x86matthew: you can't extend beyond file alignment on disk without pushing everything back and fixing it up

[2025-09-24 13:06] x86matthew: which is impractical in most cases, just create a new section at the end of the PE if the remaining padding isn't enough for what you need

[2025-09-24 13:07] x86matthew: seems like you are confusing memory alignment and file alignment unless i'm missing something

[2025-09-24 13:14] pygrum: [replying to x86matthew: "which is impractical in most cases, just create a ..."]
in fairness it isn’t too bad extending section content on disk, I think lief does the fix ups for you
they want to increase the size of a section so it uses up its entire last page when it’s loaded in memory

[2025-09-24 13:15] pygrum: am I right <@491503554528542723>

[2025-09-24 13:15] x86matthew: [replying to pygrum: "in fairness it isn’t too bad extending section con..."]
nah it would throw off all rip-relative addresses in the code

[2025-09-24 13:16] Horsie: [replying to pygrum: "in fairness it isn’t too bad extending section con..."]
Yes thats exactly what I wajt to do, so that I can put in some of my own executable code.

[2025-09-24 13:16] Horsie: [replying to x86matthew: "nah it would throw off all rip-relative addresses ..."]
Oh.. I didnt know that

[2025-09-24 13:16] pygrum: [replying to x86matthew: "nah it would throw off all rip-relative addresses ..."]
if you touch the virtual address and sizes yeah, but since all the section start addresses are page aligned they have a bit of leeway

[2025-09-24 13:17] Horsie: I can't add a section at the end because the PE has some mechanism which appends a signature at the end of the file.

I mean sure I can then patch the code. Thus was just a fun experiment to have my code be inside the original text section

[2025-09-24 13:18] Horsie: [replying to x86matthew: "which is impractical in most cases, just create a ..."]
I'm definitely a bit confused in this case. I'm still reading up on how the values in the file vs how the loader interprets them

[2025-09-24 13:19] Horsie: I thought I could sneak in some UB and ride off it. Adding some code at the tail of .text.

[2025-09-24 13:19] x86matthew: [replying to pygrum: "if you touch the virtual address and sizes yeah, b..."]
oh so he's trying to extend beyond existing file alignment but less than page alignment?

[2025-09-24 13:19] x86matthew: i see now

[2025-09-24 13:19] Horsie: [replying to x86matthew: "oh so he's trying to extend beyond existing file a..."]
Yeah

[2025-09-24 13:19] pygrum: [replying to Horsie: "I can't add a section at the end because the PE ha..."]
If you’re adding code try using my tool https://github.com/badhive/stitch
it won’t tell you when the code you add to .text overruns. It’s designed for new sections as <@943099229126144030> suggested which is the best way (you should patch the original text with a jump to your new one)
[Embed: GitHub - badhive/stitch: Rewrite and obfuscate code in compiled bin...]
Rewrite and obfuscate code in compiled binaries. Contribute to badhive/stitch development by creating an account on GitHub.

[2025-09-24 13:20] Horsie: The virtual size of the section is originally 0x250xx. I just want it to be 0x26000. The next section remains at 0x26000

[2025-09-24 13:21] pygrum: I did add logic for certificate data but haven’t tested it fully

[2025-09-24 13:21] Horsie: [replying to pygrum: "If you’re adding code try using my tool https://gi..."]
I'm going to try this now

[2025-09-24 13:21] Horsie: Its been on my todo for some time now. I thought doing what im doing would be easy enough with lief

[2025-09-24 13:21] pygrum: docs are practically non existent sorry just ask if you need a hand

[2025-09-24 13:21] Brit: classic

[2025-09-24 13:23] Horsie: But.. do we still know why this issue I'm facing exists?

[2025-09-24 13:23] Horsie: Sorry if I've missed it

[2025-09-24 13:24] pygrum: haven’t had a chance to read your code, granted I’ve never used LIEF but it might be an impl issue

[2025-09-24 13:24] pygrum: I expected it to handle all that for you

[2025-09-24 13:25] Horsie: [replying to the horse: "this is essentially the problem"]
Oh I just saw this. Hold on still reading

[2025-09-24 13:25] Brit: I have done this exact thing using both pelite and AsmResolver I have a feeling lief should also be able to do this

[2025-09-24 13:25] Brit: but worst case scenario you do it manually

[2025-09-24 13:26] Brit: it's not that bad

[2025-09-24 13:27] Horsie: I'm just trying to figure out where my lapse in judgment is. I'll whip up a diagram.

[2025-09-24 13:28] x86matthew: the concept is fine (now that i understand what you're trying to do)

[2025-09-24 13:28] x86matthew: you're essentially just increasing the file alignment and adjusting everything to match, i've done this manually several times

[2025-09-24 13:28] x86matthew: not sure about the LIEF stuff though

[2025-09-24 13:31] x86matthew: should be easy enough to diagnose with some experimentation

[2025-09-24 13:32] Horsie: Walking back home now. I'll do that in a couple of minutes.

[2025-09-24 20:40] Xits: how do I tell IDA to show me all sections and any header bytes in an ELF object file? for some reason ida just pretends the file starts at the .text section and removes any linker sections from the view

[2025-09-24 20:45] qfrtt: iirc that comes with a plugin

[2025-09-24 21:20] Horsie: [replying to pygrum: "If you’re adding code try using my tool https://gi..."]
If I've read the codecorrectly, new code is added using a new section that's appended?

[2025-09-24 21:21] Horsie: If that's the case then it won't work out for me.

[2025-09-24 21:22] Horsie: Could you help me figure out if its already able to add code to the same section?

[2025-09-24 21:22] pygrum: [replying to Horsie: "If that's the case then it won't work out for me."]
Someone has used it to add code to an existing section

[2025-09-24 21:22] Horsie: [replying to pygrum: "Someone has used it to add code to an existing sec..."]
oh

[2025-09-24 21:22] pygrum: just specify the name of the section eg .code or .text

[2025-09-24 21:22] Horsie: Ah okay. I'm try that. just skimming over the code right now

[2025-09-24 21:23] pygrum: Use X86Code::CreateFunction

[2025-09-24 21:23] pygrum: Then Function::Instrument with the returned function object

[2025-09-24 21:24] Horsie: Sweet. I'll let you know how it goes. Thanks

[2025-09-24 21:34] Brit: It seems really odd that the thing you care about has the ability to check the section count but they did not devise a way to hash their text section.

[2025-09-25 08:36] Garuda: [replying to pygrum: "If you’re adding code try using my tool https://gi..."]
Dayum thats a fantastic and user-friendly library

[2025-09-25 08:36] Garuda: Good job

[2025-09-25 10:26] pygrum: thanks mate 🙂

[2025-09-28 06:14] UJ: unicorn: we have canonical addressing at home
the canonical address:  0x000ff8079471c130

[2025-09-28 06:31] UJ: Doesn't seem related to LAM/TBI, so not sure why it isn't doing reads/writes to the mapped memory using the full address but handling it in the fetch hook seems to resolve the issue.

[2025-09-28 15:22] the horse: Does anyone know how taskmgr checks whether virtualization is enabled?

The only syscall related to it that I can see would be SystemHypervisorDetailInformation; but the SYSTEM_HYPERVISOR_DETAIL_INFORMATION fields are very vague -- which taskmgr doesn't even utilize.

I can't read msrs from usermode; checking hv present bit or hv brand string is also counter-intuitive because a hypervisor might not be running;
[Attachments: image.png]

[2025-09-28 15:24] Brit: I feel like matti would know

[2025-09-28 15:29] the horse: i do see VirtualizationFlags in KUSD

[2025-09-28 15:48] koyz: [replying to the horse: "Does anyone know how taskmgr checks whether virtua..."]
Isn’t this just telling you that Intel VT-x/AMD-V is enabled?

[2025-09-28 15:49] the horse: yes, i'm trying to see how it knows that

[2025-09-28 15:49] the horse: i don't care about an actual hypervisor running

[2025-09-28 15:49] the horse: just if VMX are enabled

[2025-09-28 15:49] koyz: It probably just uses cpuid

[2025-09-28 15:49] the horse: but the vmx bit is just whether the processor supports it

[2025-09-28 15:50] the horse: unless the bios masks it based on on/off state

[2025-09-28 15:50] koyz: Oh yeah right good point

[2025-09-28 15:50] the horse: there's a lot of things to check for an actual hypervisor running

[2025-09-28 15:50] the horse: but I don't see much for virtualization enabled without a hypervisor running

[2025-09-28 15:51] the horse: that you can query from usermode

[2025-09-28 15:51] the horse: on km you can check easily with msrs

[2025-09-28 15:51] koyz: Well now I am also interested <:kekw:904522300257345566> time to check

[2025-09-28 15:52] the horse: taskmgr is major cancer
[Attachments: image.png]

[2025-09-28 15:52] the horse: it might just be a registry key somewhere tbf

[2025-09-28 15:52] the horse: but from import xrefs it doesn't look like it's even using registry for this

[2025-09-28 15:53] the horse: ```cpp
  union {
    UCHAR VirtualizationFlags;
    struct {
      UCHAR ArchStartedInEl2 : 1;
      UCHAR QcSlIsSupported : 1;
    };
  };
```
I'm thinking it might be an undocumented flag in KUSD

[2025-09-28 16:13] koyz: [replying to the horse: "```cpp
  union {
    UCHAR VirtualizationFlags;
  ..."]
The virtualization detection is being setup in `WdcMemoryMonitor::WdcMemoryMonitor` which is quite weird because it's more CPU dependent but whatever
it checks CPUID for the hypervisor presence (especially Hyper-V), or the CPU vendor, it also checks `PF_VIRT_FIRMWARE_ENABLED` with `IsProcessorFeaturePresent`
No idea if IDA is gaslighting me, or why it's happening in the Memory Monitor, but that seems to be the place where they are storing that information

[2025-09-28 16:13] koyz: The actual function that is doing it is `WdcMemoryMonitor::CheckVirtualStatus`

[2025-09-28 16:14] the horse: PF_VIRT_FIRMWARE_ENABLED makes so much sense!

[2025-09-28 16:14] the horse: thank you very much

[2025-09-28 16:14] koyz: no problem :D

[2025-09-28 16:16] the horse: it seems that indeed this is mapped to KUSD 😄

[2025-09-28 16:16] koyz: Well, now we went full circle! <:kekw:904522300257345566>

[2025-09-28 16:17] the horse: 7FFE0289

[2025-09-28 16:18] the horse: ```
UCHAR ProcessorFeatures[64];                                            //0x274
```

[2025-09-28 16:18] the horse: so KUSD->ProcessorFeatures[21] -> virt_enabled

[2025-09-28 16:20] the horse: 🙏 finally
[Attachments: image.png]

[2025-09-28 19:44] slane: Does anyone know of an AI that has no restrictions, for example to create cheats or bypasses?

[2025-09-28 19:50] blob27: [replying to slane: "Does anyone know of an AI that has no restrictions..."]
ANY OF THEM

[2025-09-28 19:50] Loading: you just have to know how to ask them

[2025-09-28 19:50] blob27: LEARN HOW TO SPEAK ENGLISH TO THE BOT IN A PERSUASIVE WAY AND HE WILL DO IT

[2025-09-28 19:50] the horse: Find how they're invoking entry point, and hook that.
Find the allocation, dump the entire process memory, labeled with base addresses.

Note the cheat base address.

Then, find any references to other modules(imports); log these RVAs and what they point to.
create a process as suspended, allocate it at the same base address, fix imports, init static tls and call entry point

[2025-09-28 19:50] Loading: its like speaking to a girl

[2025-09-28 19:50] the horse: this is pretty much universal

[2025-09-28 19:51] the horse: if there is no DRM component in the cheat then this usually always works

[2025-09-28 19:51] the horse: pretty much for every 'shitty cheat'

[2025-09-28 19:51] the horse: and the shittiest ones have a clean dll in the loader and manually map that into the process

[2025-09-28 19:51] blob27: chatgpt gemini grok claude

[2025-09-28 19:51] the horse: which you can just snatch from memory

[2025-09-28 19:51] blob27: all

[2025-09-28 19:52] the horse: [replying to blob27: "chatgpt gemini grok claude"]
claude doesn't allow cheat development

[2025-09-28 19:52] the horse: and frankly apart from general-purpose stuff they will suck at everything

[2025-09-28 19:53] the horse: no

[2025-09-28 19:53] Loading: dude you need to learn basics first

[2025-09-28 19:53] slane: [replying to blob27: "LEARN HOW TO SPEAK ENGLISH TO THE BOT IN A PERSUAS..."]
I tried but I couldn't.

[2025-09-28 19:53] the horse: x64dbg, system informer can help you

[2025-09-28 19:53] the horse: [replying to slane: "I tried but I couldn't."]
AI will not make you a bypass

[2025-09-28 19:53] the horse: nor a cheat

[2025-09-28 19:53] slane: how do you do it?

[2025-09-28 19:53] slane: [replying to the horse: "AI will not make you a bypass"]
like the idea

[2025-09-28 19:53] the horse: how do I do what?

[2025-09-28 19:54] slane: where for example hide the .exe

[2025-09-28 19:54] slane: [replying to the horse: "how do I do what?"]
not you bro

[2025-09-28 19:54] slane: [replying to slane: "how do you do it?"]
<@308624135570063381>

[2025-09-28 19:54] Loading: what do you wanna make cheat for ? which game

[2025-09-28 19:55] slane: [replying to Loading: "what do you wanna make cheat for ? which game"]
not cheat

[2025-09-28 19:55] slane: bypass

[2025-09-28 19:55] Loading: lol

[2025-09-28 19:55] Loading: ai wont help you

[2025-09-28 19:55] blob27: [replying to slane: "<@308624135570063381>"]
dont specifically say you wanna make for example esp aimbot bhop etc but ask the specifics and how to work with the windows apis etc blah blah blah

[2025-09-28 19:56] slane: I don't want AI to do it to me I just want the idea on how to do it

[2025-09-28 19:56] Loading: it might code some function for you like moving mouse from point A to point B in straight line that you need to implement in your code but AI wont spit out complete cheat code or anti cheat bypass

[2025-09-28 19:56] Loading: for AC bypass you need to reverse it first and understand it

[2025-09-28 19:56] Loading: and if you wanna reverse something then first you need to understand how to make it

[2025-09-28 19:57] slane: [replying to Loading: "and if you wanna reverse something then first you ..."]
okok

[2025-09-28 19:57] slane: i’m learning it

[2025-09-28 19:57] slane: but i’m not a pro

[2025-09-28 19:57] slane: 😂

[2025-09-28 19:57] Loading: try unknowncheats but as i said you first need to learn c++ or c# and fifa got anti cheat so you need to reverse that first

[2025-09-28 19:58] slane: i’m looking for a place to learn it

[2025-09-28 19:58] blob27: [replying to slane: "i’m looking for a place to learn it"]
you need to understand memory and how computers work
learncpp
unknowncheats

[2025-09-28 19:58] Loading: if you wanna learn these things you need to learn basics so download some old game without anti cheat that is already reversed and people published some info about it on internet in past then learn from that

[2025-09-28 19:58] slane: i’m already make my own cheat on c++

[2025-09-28 19:58] Loading: then when you get more experience you can try fifa or bypass AC

[2025-09-28 19:59] slane: i need to bypass AC

[2025-09-28 19:59] Loading: need how to reverse first, this could take 2 years easily just to figure out basic stuff

[2025-09-28 20:00] slane: [replying to Loading: "need how to reverse first, this could take 2 years..."]
what is reverse?

[2025-09-28 20:00] Loading: it would be easier for you to get a job and just buy some bypass/cheat from random chinese than spend 1-2 years learning

[2025-09-28 20:00] Loading: [replying to slane: "what is reverse?"]
reverse engineering

[2025-09-28 20:00] slane: where can i learn that

[2025-09-28 20:00] slane: [replying to Loading: "reverse engineering"]
okok

[2025-09-28 20:00] slane: [replying to slane: "where can i learn that"]
??

[2025-09-28 20:01] slane: stop bro

[2025-09-28 20:01] slane: buy it

[2025-09-28 20:01] Loading: as i said earlier 
"if you wanna learn these things you need to learn basics so download some old game without anti cheat that is already reversed and people published some info about it on internet in past then learn from that"

[2025-09-28 20:02] blob27: [replying to slane: "??"]
first understand regular programming so you know the patterns

[2025-09-28 20:02] blob27: then get basic knowledge of assembly

[2025-09-28 20:02] blob27: then start using a disassembler and/or debugger on like ctfs or crackmes or random tiny programs

[2025-09-28 20:02] blob27: and u can see whats boiler plate whats not

[2025-09-28 20:02] blob27: learn what to look out for

[2025-09-28 20:03] Loading: same thing

[2025-09-28 20:03] slane: [replying to blob27: "first understand regular programming so you know t..."]
rn i know c++ phyton and c#

[2025-09-28 20:05] Loading: are you guys ready to spend 1-2 years just by learning basics of reversing/assembly/c or c++ ? i dont think, it would be easier for you to just buy those cheats

[2025-09-28 20:05] slane: [replying to Loading: "are you guys ready to spend 1-2 years just by lear..."]
no i want to learn bro

[2025-09-28 20:05] slane: 😂

[2025-09-28 20:05] Loading: you need passion for these things if you dont have it you will stop after 1-2 weeks

[2025-09-28 20:06] slane: i spend 1 year on learning c++

[2025-09-28 20:06] slane: [replying to Loading: "you need passion for these things if you dont have..."]
i have it

[2025-09-28 20:06] slane: hahahahha

[2025-09-28 20:09] Loading: why would someone do it for you ?

[2025-09-28 20:10] the horse: cracking shit cheats is not rocket science

[2025-09-28 20:10] the horse: you need basic cpp knowledge and some understanding of PEs

[2025-09-28 20:10] pygrum: You’ve been spamming the same thing in other servers stop waiting for a spoonfeed

[2025-09-28 20:10] the horse: 2 weeks of reading and you're more than set in this age to crack a paste

[2025-09-28 20:10] blob27: ok so download x64dbg and crack ida and start trying things, youll learn by trying. when you dont understand something while trying youll realize that you need to research more into that topic

[2025-09-28 20:12] Loading: tbh cracking shitty cheat will be just noping some jumps but bypassing ACs with 0 knowledge will take a lot of time as i said it would be easier for you to spend few hours in mcdonalds and then buy those cheats lol

[2025-09-28 20:14] slane: [replying to Loading: "tbh cracking shitty cheat will be just noping some..."]
bro i’m just asking

[2025-09-28 20:14] slane: i want to learn stop

[2025-09-28 20:15] slane: I'm ready to spend time learning. Thanks for your advice.

[2025-09-28 20:40] Addison: [replying to slane: "I'm ready to spend time learning. Thanks for your ..."]
I think their response to you is mostly out of experience with folks who don't want to put in the effort but say they do to bait people into telling them how to do it for them

[2025-09-28 20:41] Addison: This stuff takes time and a lot of grinding, and requires both expertise and lots of suffering

[2025-09-28 20:42] Addison: Asking from the start about how to get AI tools to explain it to you makes it clear that you're not past the initial hump to realise that AI won't get you anywhere, since it isn't great at giving you real understanding or experience

[2025-09-28 20:43] Addison: Give it some time of working through the problems yourself to understand the problem space, then you'll understand what questions you need to ask ^^

[2025-09-28 20:44] Addison: CTFs and setting your own challenges are great places to start

[2025-09-28 20:45] slane: okok tanks

[2025-09-28 21:14] UJ: [replying to Addison: "This stuff takes time and a lot of grinding, and r..."]
+1 on the suffering.