# December 2025 - Week 2
# Channel: #application-security
# Messages: 153

[2025-12-11 19:25] num6: Mea cupla for the noob question: I'm having trouble fully understanding and making a PoC to exploit stuff like buffer, Int, overflows, and stack/heap. I know that certian unsecure C features such as strcpy (vs strscpy) etc. can be vulnerable if they don't limit and sanitize the input. I just don't understand how someone can write the exploit and get a shell fully. Also the prevention features. Should I just re-read Art of Exploitation or is there a new better resource?

[2025-12-11 19:27] num6: Also, I'm having issues with understanding assembly to make sense of it. I know what a few instructions do (e.g. mov and jmp), but not enough to understand what's doing program wise. Is there a good book on learning assembly more and reverse engineering? I've rewatched the youtube videos on some of the fundamental assembly stuff and wrote a program + used GDB to learn more about assembly. Sorry, I'm derailing at this point

[2025-12-11 19:37] Pwnosaur: [replying to num6: "Also, I'm having issues with understanding assembl..."]
Happy to help and explain this stuff, here is the thing you need to understand the assembly a little deeper to understand how the exploitation work
For example a vanilla buffer overflow, the one that targets the stack, it's closely tied to understanding the "call" and "ret" instruction , also understanding that local variables are located on the stack
All that stuff is about understanding the assembly and the program lifecycle in memory , these topics overlap , and there is tons of resources out there

[2025-12-11 19:40] num6: [replying to Pwnosaur: "Happy to help and explain this stuff, here is the ..."]
Right, bececause it's moving return to execute something else with the standard library in the stack.

[2025-12-11 19:40] Pwnosaur: You can start with this book https://beginners.re/ it has a lot of good concepts, I would also suggest to use AI to kind of bridge some and simplfy the concepts (don't rely on it too much because it hallucinates often, so you need to use it wisely )

[2025-12-11 19:45] Pwnosaur: [replying to num6: "Right, bececause it's moving return to execute som..."]
Yes exactly, so basically without any protection mechanisms the overflow works in this way
1. you call a function that is using a insecure strcpy that copies data into a local variable
2. the local variables are placed on the stack frame of the function where they are defined and used (unless using a pointer and that's a different story)
3. if strcpy copies too much data, it starts overwriting other data on the stack downwards, overwriting the return address where this vulnerable function is supposed to return

In a very vanilla naive env without ASLR or any security mechanism, you just need to find an address that has the instruction "jmp esp" which basically jumps to execute what ever on the stack, whatever you have wrote there must be the bytes (opcodes) correlating to the instructions for the CPU to execute

If this sounds too gibberish, go read further, once you have the base concepts it will become much clearer

[2025-12-11 19:45] Pwnosaur: hopefully this helps

[2025-12-11 19:56] plpg: Roughly that. Doing some simple crackmes also helps, you can even write your own vulnerable programs and then learn how to exploit them

[2025-12-11 19:57] plpg: The simplest example doesnt execute the code passed to stack but just overwrites the return address with another known address. Make sure to disable ASLR when testing. That example can be as simple as one .c file with a main(), a vulnerable function and a target function that is never called from the code but nonetheless gets compiled and linked in the final executable

[2025-12-11 19:58] plpg: I can walk you through it if you want <@900494230504505344>

[2025-12-11 20:06] num6: [replying to plpg: "I can walk you through it if you want <@9004942305..."]
No thanks, I'm going to try to do it myself.

[2025-12-11 20:06] plpg: Good luck!

[2025-12-12 01:17] daax: [replying to num6: "Mea cupla for the noob question: I'm having troubl..."]
best source is doing RCA on old versions of modules that had CVEs and trying to reconstruct them. imagine a program that has the following code:
```
char buf[20];
puts(“enter id:”);
gets(buf);
puts(“received”);
puts(buf);
return 0;
```
Assume  nx (dep) and aslr

if you input a large id, it’ll crash with an overflow/segfault. you know you can then write over the return address on the stack at some point so you fill the buffer with indicator sequences like aaaabbbbccccddddeeeeffffababdcdcfefe etc. Eventually you see that there is a specific offset where the ret address is overwritten. you now have control of it. but where do you redirect to? maybe a function calls system(cmd) for arbitrary cmd use, but aslr is on so… we need an info leak, the puts(buf) is an option. 

the attacker would be send N bytes to leak adjacent stack data; parse it and then build a proper payload to leak and then pad to reach ret address and either rop chain to reach the target like winexec or system(cmd) or if there is a known offset from main to the target fn you can calculate like that and overwrite ret. if there are stack canaries you will also include those in info you want to leak so that you can set the stack up to avoid crashing the target

[2025-12-12 01:20] daax: ctfs and looking at old applications + rca to attempt to make a poc will give you the best insight. trivial examples will only go so far, it’s like chess in that you have to do it a bunch and if you are having success with ctfs and doing rca you’ll get better at pattern recongition

[2025-12-12 01:26] daax: this is the output from a ctf i sent someone to help them with this same conceptualization. all of the usual mitigations minus CET were enabled, so you can see what kind of input was piped to the application (rept{%p})
[Attachments: image.png]

[2025-12-12 20:17] pingpong: Has anyone ever seen an exploit for CVE-2017-8823?

[2025-12-12 20:17] pingpong: I'm trying to understand if there is a record for it being actually exploitable or if it's a bunch of bullshit as it seems

[2025-12-12 20:40] elias: if there's no poc chances are high its just a theoretical vulnerability

[2025-12-13 02:58] mtu: That CVE has a crash report tied to it so it was probably possible at some point

[2025-12-13 23:21] Obvious: [replying to plpg: "The simplest example doesnt execute the code passe..."]
How would you call the linked function that is never called (basically how you attain the address of it)? Also, does it start overwriting by the top of the stack when a stack overflow occurs?

[2025-12-13 23:23] Obvious: ```
stack
**-------** 0x0000 7FFF FFFF FFFF
ret addr
---
---
rbp
stack frame
frame
frame
frame
overflow occurs
```
so it then starts to write to 0x0000 7FFF FFFF FFFF and downwards? and thus the return address gets overwritten

[2025-12-13 23:25] plpg: you have to know the address of the function beforehand

[2025-12-13 23:25] plpg: thats the simplest answer

[2025-12-13 23:25] Obvious: how tho?

[2025-12-13 23:25] plpg: thats also why all beginner tutorials written recently tell you to disable ASLR

[2025-12-13 23:26] plpg: you can calculate it from the ELF load address and the offset in .text

[2025-12-13 23:27] Obvious: how would you know it's the function you want to call?

[2025-12-13 23:27] Obvious: well, since you wrote the code for your own means you probably do anyway

[2025-12-13 23:27] Obvious: but this isnt applicable for real word scenarios

[2025-12-13 23:27] plpg: for that you have to analyze the source code or disassembly (in case of crackmes)

[2025-12-13 23:28] plpg: it is, before ASLR was common you had lists of function addresses for each particular version of Windows service pack, as an example

[2025-12-13 23:29] Obvious: doesn't ASLR still hold data in segments?

[2025-12-13 23:29] plpg: it does, but it randomizes the load addresses of segments

[2025-12-13 23:29] Obvious: what i mean by that is:
```
teb,peb
stack
---
heap
```

aslr:
```
stack
teb, peb
heap
dll
```

[2025-12-13 23:29] Obvious: this is my understanding

[2025-12-13 23:30] Obvious: simply put

[2025-12-13 23:30] plpg: I dont know if ASLR randomizes the order of segments

[2025-12-13 23:30] plpg: but it randomizes the load address of each segment by some amount

[2025-12-13 23:31] Obvious: [replying to plpg: "but it randomizes the load address of each segment..."]
You mean each segment useed to be loaded at a fixed address at all times before?

[2025-12-13 23:31] plpg: Yes, the addres it was specified at in the executable file

[2025-12-13 23:31] Obvious: Bruh

[2025-12-13 23:32] Obvious: So you could just read the pe file and find the exact data you needed just by reading that memory

[2025-12-13 23:32] plpg: yeah, the operating system needs to know where to load the code and data from the file

[2025-12-13 23:32] Obvious: I've read into PE and pages etc, but I lack ASLR

[2025-12-13 23:33] Obvious: [replying to Obvious: "```
stack
**-------** 0x0000 7FFF FFFF FFFF
ret ad..."]
Can you answer this?

[2025-12-13 23:34] plpg: ok, forget aslr for now

[2025-12-13 23:34] plpg: ill prepare an example

[2025-12-13 23:36] plpg: this is the program, and its paused in a debugger
[Attachments: obraz.png]

[2025-12-13 23:36] plpg: you have a main that calls a function func

[2025-12-13 23:36] plpg: The debugger paused it when the program entered `func`

[2025-12-13 23:36] plpg: so now the memory dump up there is 8 integers starting at the stack pointer (so it's top of stack and 8 cells up)

[2025-12-13 23:37] plpg: I have a 64 bit machine, so an address is 64 bits, that means two "cells" (i could dump it as longwords but it doesnt matter)

[2025-12-13 23:38] Obvious: cells alias for windows is dword right

[2025-12-13 23:38] plpg: so the first two cells are `0x00007fff ffffdf20`, then you have `0x00005555 555554ca` and then you have `0x00000000 00000001` and so on

[2025-12-13 23:38] Obvious: so 4 bytes

[2025-12-13 23:38] plpg: yeah

[2025-12-13 23:38] plpg: this function has no stack variables

[2025-12-13 23:39] plpg: so on top of stack you now have the previous stack pointer, this is the 7ffff..., then you have the return address, thats 5555.... and then you have the previous frame

[2025-12-13 23:39] plpg: that `0xf7dd224a    0x00007fff` is the previous-previous stack pointer of whatever called main()

[2025-12-13 23:40] plpg: when func is done, it will issue a pop rbp and then a ret
[Attachments: obraz.png]

[2025-12-13 23:41] plpg: `func` is not vulnerable now, but if it was, then you would overwrite the saved rbp value (7fff....) and the return address (5555...) with your own data

[2025-12-13 23:41] plpg: so if your code had some function or a code that you wanted to call, and it was at let's say `000055555555ff00` you would try to write that onto the stack in place of the return address, `func` would go on and then the `ret` would jump back to the overwritten return address

[2025-12-13 23:42] plpg: as for how do you find these addresses

[2025-12-13 23:42] plpg: `objdump -d example.elf` (or any other disassembler) will give me the function offsets in the text section:
[Attachments: obraz.png]

[2025-12-13 23:43] plpg: and you can see how they are related to the 0x0000555555... addresses

[2025-12-13 23:43] plpg: the system just loaded the text section at `0x000055555554000`, so main is at +14bc from there and func is at +14ad

[2025-12-13 23:46] Obvious: [replying to plpg: "so if your code had some function or a code that y..."]
okay I get it, but how do you abuse that vulnerability (assume that vulnerability is a stack overflow)

the way i interpret -> 

stack limit is 1024 bytes
i know exactly a stack overflow would occur if i tried to write 1025 bytes.
and i know if it occurs it will start overwriting the data from the top of the stack.
since the top holds the return value, 
i write 1032 bytes (the last 8 bytes being the malicious function address i want to call that i want to replace with the RET instruction)

[2025-12-13 23:46] plpg: why is the stack limit 1024 bytes?

[2025-12-13 23:46] Obvious: i just assumed it, it doesnt have to be

[2025-12-13 23:47] Obvious: is this the general procedure is what i am asking

[2025-12-13 23:47] plpg: Yeah, exactly

[2025-12-13 23:48] plpg: if you had a function like this
```c
int dontCodeLikeThis(char *s){
  char badbuf[32];
  strcpy(badbuf, s);
  return 0; 
}
```

[2025-12-13 23:48] plpg: then at the moment of strcpy, the first 32 bytes on stack are normal - thats the array. Then you have the saved ESP, and then you have the return address

[2025-12-13 23:49] plpg: so you want the input string to have 32 bytes of dummy padding (to reach the end of buffer), then 8 bytes of dummy padding (for the esp value) and then the next 8 bytes will be copied(*) into the return address.

[2025-12-13 23:49] plpg: (they will not, because in case of this system I am running it on the addresses start with 00 00) and a zero byte is end of string

[2025-12-13 23:50] Brit: honestly just show him the godbolt of the vuln function

[2025-12-13 23:50] Brit: sub rsp, 40

[2025-12-13 23:50] Brit: makes this super clear

[2025-12-13 23:50] Brit: in your example

[2025-12-13 23:50] plpg: [replying to plpg: "if you had a function like this
```c
int dontCodeL..."]
yeah, you can paste this into godbolt and see for yourself

[2025-12-13 23:51] plpg: ```asm
"dontCodeLikeThis":
        push    rbp
        mov     rbp, rsp
        sub     rsp, 48
        mov     QWORD PTR [rbp-40], rdi
        mov     rdx, QWORD PTR [rbp-40]
        lea     rax, [rbp-32]
        mov     rsi, rdx
        mov     rdi, rax
        call    "strcpy"
        mov     eax, 0
        leave
        ret
```

[2025-12-13 23:52] plpg: different compiler, it allocated 48 bytes because the optimizer is not on, but the general idea stands

[2025-12-13 23:53] Brit: its a question of alignment right

[2025-12-13 23:53] Brit: /16 aligned

[2025-12-13 23:54] plpg: fair

[2025-12-13 23:58] Obvious: thanks a lot. now completely off-topic to this one but there's another question i want to ask.

in regards to paging, each address is 64 bits wide on 64-bit architecture.
but we don't have enough memory space to keep addresses of each byte.

so instead, divide and conquer.

a memory region gets an address and we use offsets to access whatever's inside them.

basically page tables live in kernel space and those tables hold addresses of 4KB entries, in this way we utilize memory efficiently.

am  i right?

[2025-12-13 23:58] Brit: is this a question or an affirmation?

[2025-12-13 23:58] plpg: roughly yes

[2025-12-13 23:58] Brit: oh

[2025-12-13 23:59] plpg: the page tables configure the memory management unit that maps the entire 64-bit (huge) address space to the physical address space which is (almost always) smaller

[2025-12-13 23:59] Brit: 
[Attachments: PAGE_2.png]

[2025-12-14 00:00] Brit: something like this

[2025-12-14 00:00] plpg: the mappings do not need to lead to the system ram by the way. you can have mappings to memory mapped devices (like a GPU), or, from userspace, to other virtual memory (like `mmap` does)

[2025-12-14 00:01] plpg: the x86 multi level paging is a bit of a nightmare to explain mmu and virtual memory

[2025-12-14 00:01] plpg: but the general idea is like you say

[2025-12-14 00:05] contificate: [replying to plpg: "this is the program, and its paused in a debugger"]
emacs?

[2025-12-14 00:07] plpg: yes

[2025-12-14 00:07] plpg: eight megabytes and constantly swapping

[2025-12-14 00:09] Obvious: ```

PML4E = CR3 + VA>>39 & 0x1FF (physical address)
PDPE = PML4E & 0x0000 0FFF FFFF F000 + VA >> 30 & 0x1FF
...
...
PTE = PDTE & 0x0000 0FFF FFFF F000 + VA >> 12 & 0x1FF
Physical Addr = PTE & 0x0000 0FFF FFFF F000 + VA & 0x1FF
```

[2025-12-14 00:10] Obvious: Now this physical address is the beginning address of a frame (4KB on most systems)

[2025-12-14 00:10] plpg: dont think about it too much. that only matters when you do OS-dev or bare metal level hacking :D

[2025-12-14 00:11] plpg: also, older families of CPUs will have a slightly different setup with less (or more?) paging levels

[2025-12-14 00:11] Obvious: And you can only write to that frame using offsets from that physical frame base

[2025-12-14 00:12] Obvious: [replying to plpg: "dont think about it too much. that only matters wh..."]
Idk why theres so much indirection

[2025-12-14 00:12] plpg: It is handy for virtualization of all kinds (ex, virtual machines and task switching)

[2025-12-14 00:13] plpg: when the operating system switches tasks, it does not have to move out all memory from the older process and move in the new process, it just swaps a page mapping on one of these levels and suddenly all subsequent levels point to a different physical memory areas

[2025-12-14 00:14] Obvious: I was just learning through paging and I just learned there is something called EPT 🤦‍♂️

[2025-12-14 00:14] plpg: this way all programs can load text to, lets say, 0x5555FFFF, and then the virtual addressing makes sure that the 5555FFFF address seen by process 1 is a completely different physical address in process 2, despite both of these referring to memory at "5555FFFF"

[2025-12-14 00:15] Obvious: you mean each process has their own VAS

[2025-12-14 00:15] plpg: add another paging levels, and you can have a VM hypervisor change the virtual machines by swapping around a whole-OS level of pages (im simplyfying)

[2025-12-14 00:15] plpg: [replying to Obvious: "you mean each process has their own VAS"]
yes

[2025-12-14 00:15] plpg: between processes this VAS is handled by the kernel

[2025-12-14 00:16] plpg: between kernels by hypervisor, and between hypervisors... by more hypervisors

[2025-12-14 00:16] Obvious: [replying to plpg: "when the operating system switches tasks, it does ..."]
!!! great insight, i knew that each process had their own VAS but no way of how the OS knew which process' VA to translate

[2025-12-14 00:17] Obvious: so its related to task switching

[2025-12-14 00:17] plpg: the OS keeps track of the page tables and loads them into the MMU whenever needed

[2025-12-14 00:17] Obvious: [replying to plpg: "the OS keeps track of the page tables and loads th..."]
are VAs generated by the CPU or the loader?

[2025-12-14 00:18] Obvious: i think windows loader sets the region and page table etc

[2025-12-14 00:18] Obvious: and cpu just handles the translation via mmu and tlb

[2025-12-14 00:18] plpg: I would say they are generated by the kernel, whenever a new task/process is spawned the kernel has to find some free memory to allocate it, and then it sets up a page table entry structure for that process

[2025-12-14 00:19] plpg: then when the scheduler decides it's time to run that process it will set up all these PML4E, PDPE, PDE, PTE... values as part of a context switch

[2025-12-14 00:19] plpg: but thats OS dev stuff

[2025-12-14 00:20] plpg: from the userspace you need to know two things, first that the userspace is in a virtual memory space, and second that the kernel can map things in and out of that virtual space

[2025-12-14 00:21] plpg: for example when you call mmap on a file, the kernel will map the file contents to some area in your (process') virtual memory space and give you a pointer to that as the syscall result

[2025-12-14 00:21] plpg: you can have a driver that maps an expansion cards memory space into the userspace when you mmap `/dev/mycard`

[2025-12-14 00:22] plpg: [replying to plpg: "then when the scheduler decides it's time to run t..."]
oh and this is x86 specific. ARM or Risc-V has different names and specifics but the idea is the same

[2025-12-14 00:22] Brit: x86 has this nifty register

[2025-12-14 00:22] Brit: called the CR3

[2025-12-14 00:24] Obvious: [replying to plpg: "for example when you call mmap on a file, the kern..."]
otherwise you would need to copy the file contents in each process right?
this is how DLLs are handled iirc.

they are mapped to VAs but there's only one instance of the whole DLL present in physical memory and if the need arises to W to DLL that process then allocates memory for the copy of that DLL and then perform operations on it

[2025-12-14 00:24] plpg: i recommend reading through the 80386 programming manual, it is pretty short, and because we keep backwards compatiability all the new stuff builds up on the old stuff. So 80386 manual gives you a rough overall view on the basic mechanisms, and then they are expanded in next CPU generations

[2025-12-14 00:25] plpg: [replying to Obvious: "otherwise you would need to copy the file contents..."]
Probably yes. All i can say is that thats how I would do it, I dont know NT internals that well

[2025-12-14 00:26] Obvious: i have no fucking clue why and how i got into all this stuff tbh

[2025-12-14 00:26] Obvious: i was reading a document on PE files

[2025-12-14 00:26] Obvious: then came pages

[2025-12-14 00:26] plpg: https://pdos.csail.mit.edu/6.828/2018/readings/i386.pdf page (lol) 99

[2025-12-14 00:26] plpg: thats the simplest paging system you can have. 2 level paging

[2025-12-14 00:27] plpg: then 3 level paging lets you have several copies of the page directory in memory

[2025-12-14 00:27] contificate: for future ref, many browsers support `#page=`, e..g https://pdos.csail.mit.edu/6.828/2018/readings/i386.pdf#page=99

[2025-12-14 00:27] contificate: too much work to expect bros to navigate

[2025-12-14 00:29] Obvious: [replying to plpg: "https://pdos.csail.mit.edu/6.828/2018/readings/i38..."]
ill look into this, ty. im interested in these topics but do i have any way of putting them into practice? also as i am asking this i do kinda feel it is out of my league to implement this kind of thing as of now even if there were a way of putting it into practice so lol

[2025-12-14 00:29] Obvious: besides OS-dev, no way out?

[2025-12-14 00:29] plpg: not really, if you really want to deal with paging (for some reason) you can try reading some operating system sources or running them in QEMU

[2025-12-14 00:30] plpg: or write your own assembly code that sets up some mappings and debug it in QEMU

[2025-12-14 00:30] plpg: (you could debug it in a real system but I doubt you can find an emulator for a modern CPU for normal money)

[2025-12-14 00:31] plpg: then again there is no reason to deal with this stuff unless you want to write an OS that uses multi level paging

[2025-12-14 00:31] Obvious: its not that i want to deal with paging especially, i want to know if these efforts go in vain or is it worth it

[2025-12-14 00:31] Obvious: besides of course learning how things work

[2025-12-14 00:31] plpg: they give you an idea of how computers work, in most cases

[2025-12-14 00:31] plpg: you dont even need paging to write a multitasking OS

[2025-12-14 00:32] plpg: (it is handy, but not necessary)

[2025-12-14 00:33] Obvious: i see, thank u for ur time xD

[2025-12-14 00:34] plpg: i enjoyed it greatly :D

[2025-12-14 15:05] daax: [replying to Obvious: "and cpu just handles the translation via mmu and t..."]
<https://revers.engineering/mmu-ept-technical-details/>

Might have some info that is helpful to you