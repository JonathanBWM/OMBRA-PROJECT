# February 2024 - Week 4
# Channel: #reverse-engineering
# Messages: 78

[2024-02-19 06:01] snowua: damn this binary comes with the intel sdm included
[Attachments: image.png]

[2024-02-19 09:42] estrellas: [replying to asz: "leave the opcodes alone!"]
Fun fact: some forms of INC and DEC aren't supported in x64 due to it's opcode being interpreted as the REX prefix 😆

[2024-02-19 09:42] asz: https://tenor.com/view/dinosaur-dinosaurs-t-rex-tyrannosaurus-rex-roar-gif-27599760

[2024-02-19 18:02] Horsie: Is this some kind of sidechannel mitigation?

[2024-02-19 18:02] Horsie: 
[Attachments: image.png]

[2024-02-19 18:02] Horsie: Its just a bunch of calls that do nothing?

[2024-02-19 18:04] Horsie: Then it ends with 0xDADA
[Attachments: image.png]

[2024-02-19 18:26] Deleted User: 0xGuguGaga

[2024-02-19 22:36] 0x208D9: I'm currently looking for a reason why these two codes can result in a different decompilation output:
```c

#include <stdio.h>

int main()
{
   int year;

   printf("Year : ");
   scanf("%d", &year);

   printf("Year : %d", year);
   return 0;
}
```
and
```c

# include <stdio.h>

int main()
{
    int year;

    printf("Enter year: ");
    scanf("%d", &year);

    printf("Factorial of %d", year);
    
    return 0;
}
```

the decompilation outputs when loaded in ghidra with default analyzer settings are :
```c
undefined8 main(void)
{
  long in_FS_OFFSET;
  uint local_14;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  printf("Year : ");
  __isoc99_scanf(&DAT_0010200c,&local_14);
  printf("Year : %d",(ulong)local_14);
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return 0;
}
```
and
```c
undefined8 main(void)
{
  long in_FS_OFFSET;
  uint local_14;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  printf("Enter year: ");
  __libc_start_main(&DAT_00102011,&local_14);
  printf("Factorial of %d",(ulong)local_14);
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return 0;
}
```
respectively for the first and second program. as we can see the second decompilation is clearly incorrect cuz ` __libc_start_main(&DAT_00102011,&local_14);` which when xrefed will result in a invalid address, i fail to understand why ghidra cant determine the scanf for the second program but can for the first program? the compilation process i used is similar for both `gcc -o binary main.c` and no none of the binaries were obfuscated or packed after compilation

(I asked this in another server as well, but i wanna know what people thinks here, and also maybe cuz the other server isnt responding lmao)

[2024-02-19 22:44] 0x208D9: 
[Attachments: image.png]

[2024-02-19 22:46] qwerty1423: [replying to 0x208D9: "I'm currently looking for a reason why these two c..."]

[Attachments: image.png]

[2024-02-19 22:46] qwerty1423: ghidra bad

[2024-02-19 22:47] 0x208D9: [replying to qwerty1423: ""]
makes sense lmao

[2024-02-19 22:47] 0x208D9: ig ima try debugging why that bug occurs in ghidra

[2024-02-19 22:47] 0x208D9: time to git clone and see source

[2024-02-20 00:51] abu: [replying to Horsie: ""]
I see that whenever kva shadowing is used so I guess it’s something to do with mitigating spectre/meltdown

[2024-02-20 00:53] Deleted User: [replying to 0x208D9: "I'm currently looking for a reason why these two c..."]
?

[2024-02-20 00:53] Deleted User: I dont see anything wrong

[2024-02-20 01:29] Azalea: __libc_start_main

[2024-02-20 01:29] Azalea: vs _isoc99_scanf

[2024-02-20 11:20] jvoisin: [replying to 0x208D9: "ig ima try debugging why that bug occurs in ghidra"]
Use godbolt/digbolt to compare things ?

[2024-02-20 12:52] eternablue: there's also dogbolt to compare disssamblers. although it was pretty slow last time i tried

[2024-02-20 18:17] daax: [replying to Horsie: ""]
Yes, flushes (overflows) the RSB (spectre/meltdown mitigation)

[2024-02-21 17:15] Horsie: "I hope <@609487237331288074>'s CTF probably wouldnt be too weird"
[Attachments: binaryninja_jDNN9Y0uvQ.webm]

[2024-02-21 17:48] naci: 
[Attachments: image.png]

[2024-02-21 17:48] naci: (this may take a while)

[2024-02-21 18:04] qwerty1423: [replying to naci: ""]
yea for me it almost took 2:40 hours to analyze

[2024-02-21 18:37] Horsie: Need to go sleep now but I'm hoping to take a look at it manually and try to figure out some pattern first

[2024-02-21 18:38] Horsie: Looks like a lot of inlined code. Cant always rely on ida to do the analysis for me..

[2024-02-21 18:38] naci: well it looks big

[2024-02-21 18:38] naci: but its actually easy

[2024-02-21 18:41] Horsie: Thanks for the confidence. 👍

[2024-02-21 19:12] Deleted User: [replying to Horsie: ""I hope <@609487237331288074>'s CTF probably would..."]
it's very long (but mostly because essentially all STL methods are inlined + the lazy-importer like code), but if you need some hints lmk

[2024-02-21 19:12] Deleted User: it's sort of guessy (? but straightforward once you find what you need to find

[2024-02-21 19:12] Deleted User: but that's it, might be worth a pending writeup later on :p (mainly because of the amount of anti-analysis shit in there,  you can work around it easily tho)

[2024-02-21 20:43] Horsie: [replying to Deleted User: "it's very long (but mostly because essentially all..."]
Will do. Ty

[2024-02-21 20:43] Horsie: I’ll give it my best attempt due first

[2024-02-22 00:26] MalcomVX: [replying to 0x208D9: "I'm currently looking for a reason why these two c..."]

[Attachments: image.png]

[2024-02-22 00:27] MalcomVX: 
[Attachments: image.png]

[2024-02-22 00:30] MalcomVX: even older versions of ghidra should have decompiled just fine, you might have fudged with the decompilation settings/state somehow

[2024-02-22 00:30] MalcomVX: assigning `__libc_start_main` to `scanf` is just weird for ghidra to do on its own

[2024-02-22 04:59] 0x208D9: [replying to MalcomVX: "even older versions of ghidra should have decompil..."]
newnum : https://dogbolt.org/?id=0f5d0b62-cf93-43de-bbd1-01ef8abfd2b1
numbers : https://dogbolt.org/?id=c0d59e04-6723-4745-ac6d-ccced7eb1439

idk how come dogbolt showing the same thing
[Embed: Decompiler Explorer]
Decompiler Explorer is an interactive online decompiler which shows equivalent C-like output of decompiled programs from many popular decompilers.
[Embed: Decompiler Explorer]
Decompiler Explorer is an interactive online decompiler which shows equivalent C-like output of decompiled programs from many popular decompilers.

[2024-02-22 15:48] MalcomVX: [replying to 0x208D9: "newnum : https://dogbolt.org/?id=0f5d0b62-cf93-43d..."]
which version of ghidra do you have ?

[2024-02-22 17:44] 0x208D9: [replying to MalcomVX: "which version of ghidra do you have ?"]
source compiled latest

[2024-02-23 14:16] _mrfade_: Yo guys I have a fairly basic question I'd like to understand :

In the attached, you can see V13 and V14  set by V6 and V10 respectfully. This the last appearance of both of those variables but I am 90% sure that they are used in the subs above and below them the subs that take (__int64)v12 as a parm. 

Could any shed some light on this ? <:yea:904521533727342632>
[Attachments: image.png]

[2024-02-23 14:17] brymko: press y on the subs and add more parameters

[2024-02-23 14:43] daax: [replying to _mrfade_: "Yo guys I have a fairly basic question I'd like to..."]
Something else in IDA is you can dump to C file (from the File menu option) and it will sometimes resolve a lot of these issues across the bin. The alternative for those in the picture is just to do what brymko said, and then go back to top level func and hit F5 again. Another option is reverse the primary function and track vars and build an idea of what it should look like, then go to pseudoview and see if you can match those expectations. You should never solely rely on pseudoview (imo)

[2024-02-23 14:52] Brit: ^

[2024-02-23 14:52] Brit: "You should never solely rely on pseudoview (imo)" this is incredibly correct and I often see people getting stuck on pseudo

[2024-02-23 14:58] _mrfade_: Thanks I suppose pseudo code is a crutch, I have a dump with the debug symbols as well but it seems to be doing the same thing ? - I'll try the Dump to C file option though thanks.

(Minor note that the below sub with debug symbols is not the same version as my previously shared one in fact it's taken from a mac dump)
[Attachments: image.png]

[2024-02-23 15:07] brymko: idk what happending show disas

[2024-02-23 23:34] MalcomVX: [replying to 0x208D9: "source compiled latest"]
think they do `stable` releases and the `latest`  might be buggy

[2024-02-23 23:35] MalcomVX: in that case it might not be worth trying to figure out why (not something you are likely to see again)

[2024-02-24 17:14] Horsie: Im RE-ing a 3rd party service on windows that creates a named pipe (presumably for IPC with plugin-like clients)

[2024-02-24 17:15] Horsie: From what I know from process hacker and process explorer, only the service holds handles to the pipe so I dont think it has anyone connecting to it

[2024-02-24 17:15] Horsie: That aside- I'm trying to find out how the pipe is being handled and where the data is ultimately being processed

[2024-02-24 17:16] Horsie: Generally speaking, what would be the next step after CreateNamedPipeW, ConnectNamedPipe, ???

[2024-02-24 17:17] Horsie: It spawns a bunch of threads throughout the process and I'm not great with windbg so I'm looking at it in IDA at the moment

[2024-02-24 17:17] Deleted User: Probably something to read from the pipe

[2024-02-24 17:19] Deleted User: ReadFile?

[2024-02-24 17:21] Horsie: I would assume ReadFileExW yeah, since thats what it imports

[2024-02-24 17:21] Horsie: So there shouldnt really be any other api calls between that in the process I assume?

[2024-02-24 17:21] Horsie: I'm a bit lost in the code since im seeing completion routines as well as events being signalled throughout the code so its confusing

[2024-02-24 17:31] diversenok: https://learn.microsoft.com/en-us/windows/win32/ipc/pipe-functions
[Embed: Pipe Functions - Win32 apps]
The following function is used with anonymous pipes.

[2024-02-24 17:31] diversenok: These + read/write

[2024-02-24 17:34] diversenok: Could also be `WaitForSingleObject`/`WaitForMultipleObjects`

[2024-02-24 17:35] diversenok: I'd say the most typical call after  `CreateNamedPipe` probably be `ConnectNamedPipe` since the server wants to wait for clients first

[2024-02-24 17:36] diversenok: And only then `ReadFile`/`WriteFile`

[2024-02-24 19:31] asz: 
[Attachments: image.png]

[2024-02-24 19:31] asz: if didnt know

[2024-02-24 19:36] dullard: icacls works too

[2024-02-24 19:36] dullard: There’s also a pretty nice tool from cyberark called pipe viewer

[2024-02-24 19:40] asz: that do not sound nice at all

[2024-02-24 20:05] dullard: https://github.com/cyberark/PipeViewer
[Embed: GitHub - cyberark/PipeViewer: A tool that shows detailed informatio...]
A tool that shows detailed information about named pipes in Windows - cyberark/PipeViewer

[2024-02-24 20:06] dullard: It’s based on James forshaws NtApiDotNet

[2024-02-25 07:14] Horsie: [replying to diversenok: "Could also be `WaitForSingleObject`/`WaitForMultip..."]
Quite possibly. I'm looking into this now

[2024-02-25 07:14] Horsie: I'm looking at this service which is a bit complex so its been hard to follow statically

[2024-02-25 21:18] Horsie: Btw I figured it out. It was using a indirect call in the io completion routine for readfileex for processing the received data