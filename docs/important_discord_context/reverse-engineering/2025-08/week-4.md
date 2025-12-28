# August 2025 - Week 4
# Channel: #reverse-engineering
# Messages: 64

[2025-08-19 08:27] brockade: hey guys, i started building a decompiler for torque2d a few months ago and keep losing motivation so i thought i'd put a note here and then that'll hopefully guilt me into working on it more regularly

[2025-08-19 08:27] brockade: using a combination of the source and reversing the binary ive got a disassembler for the bytecode

[2025-08-19 08:28] brockade: here's the output for the trucktoy module that comes with it:
```Doing function TruckToy::setWheelSpeed @ 10974

OP_FUNC_DECL: TruckToy::setWheelSpeed end 11005 argc 2
OP_SETCURVAR %value
OP_LOADVAR_STR
OP_ADVANCE_STR
OP_SETCURVAR %this
OP_LOADVAR_STR
OP_SETCUROBJECT
OP_SETCURFIELD WheelSpeed
OP_TERMINATE_REWIND_STR
OP_SAVEFIELD_STR
OP_STR_TO_NONE
OP_RETURN

Doing function TruckToy::setWheelFriction @ 11005

OP_FUNC_DECL: TruckToy::setWheelFriction end 11036 argc 2
OP_SETCURVAR %value
OP_LOADVAR_STR
OP_ADVANCE_STR
OP_SETCURVAR %this
OP_LOADVAR_STR
OP_SETCUROBJECT
OP_SETCURFIELD WheelFriction
OP_TERMINATE_REWIND_STR
OP_SAVEFIELD_STR
OP_STR_TO_NONE
OP_RETURN```

[2025-08-19 08:28] brockade: the compiler isn't doing anything too clever so next step is to tidy up the code (it's currently emitting a tuple for each opcode and this should be objects) and then start running some decompiler passes over it

[2025-08-19 08:29] brockade: and then the bytecode changes a little between versions so i then want to port it to the version that "and yet it moves" uses, decompile that and then do a full writeup

[2025-08-20 11:33] 0xboga: Hey, I have a key stored in the registry, seems to be encrypted. The software that uses it passed the key name to CryptAcquireContext then proceeds to call:
CryptGetUserKey
CryptImportKey
CryptDecrypt 

The thing is I need to run code to extract  this key on a remote machine

Is the key the crypt user key api uses stored on disk? I have the disk of the VM where this key is stored

[2025-08-21 05:34] 0xGoose: the keys would be held in memory

[2025-08-21 05:35] 0xGoose: if its still running u can debug to find the call, set breakpoints and just watch the registers

[2025-08-22 15:00] brockade: [replying to brockade: "here's the output for the trucktoy module that com..."]
got some basic state working (string stack, current var/field/object) and can do assignments properly now, so this:
```
OP_FUNC_DECL: TruckToy::setRotateCamera end 11277 argc 2

basic block @ 11260

OP_SETCURVAR %value
OP_LOADVAR_STR
OP_ADVANCE_STR
OP_SETCURVAR %this
OP_LOADVAR_STR
OP_SETCUROBJECT
OP_SETCURFIELD RotateCamera
OP_TERMINATE_REWIND_STR
OP_SAVEFIELD_STR
OP_STR_TO_NONE
OP_RETURN
```

decompiles to this:

```var(%this).RotateCamera = var(%value)```

[2025-08-22 15:02] brockade: code is pretty straightforward:
```
def decompile_basic_block(basic_block):
    string_stack = [None]
    cur_var = None
    cur_object = None
    cur_field = None
    for instr in basic_block:
        if isinstance(instr, OpSetCurVar):
            cur_var = instr.args[0]
            #print(f"Setting cur_var to {cur_var}")
        elif isinstance(instr, OpSetCurField):
            cur_field = instr.args[0]
            #print(f"Setting cur_field to {cur_field}")
        elif isinstance(instr, OpSetCurObject):
            cur_object = string_stack[len(string_stack) - 1]
            #print(f"Setting cur_object to {cur_object}")
        elif isinstance(instr, OpLoadVarStr):
            string_stack[len(string_stack) - 1] = f"var({cur_var})"
            #print(f"Head of string stack is now {string_stack[len(string_stack) - 1]}")
        elif isinstance(instr, OpAdvanceStr):
            string_stack.append(None)
        elif isinstance(instr, OpSaveFieldStr):
            print(f"{cur_object}.{cur_field} = {string_stack[len(string_stack) - 1]}")
        elif isinstance(instr, OpStrToNone):
            # this is a nop in the interpreter
            pass
        elif isinstance(instr, OpTerminateRewindStr):
            string_stack.pop()
            if not len(string_stack):
                raise Exception("Got OpTerminateRewindStr but OpTerminateRewindStr is already size 1")
        else:
            print(f"Not handling opcode {instr}")
    pass
```

[2025-08-23 11:55] ml: Hi, does anyone know how to know if there tls static in a dll file?

[2025-08-23 13:21] brockade: [replying to ml: "Hi, does anyone know how to know if there tls stat..."]
look for a `IMAGE_TLS_DIRECTORY32` or `IMAGE_TLS_DIRECTORY64`

[2025-08-23 13:32] Pepsi: [replying to ml: "Hi, does anyone know how to know if there tls stat..."]
how can you still not have this figured out 😅

[2025-08-23 14:16] Ignotus: is there an ida plugin to rename variables to known function parameter names?

[2025-08-23 14:50] toasts: [replying to Ignotus: "is there an ida plugin to rename variables to know..."]
you could try kaspersky’s hrtng plugin

[2025-08-24 17:03] Plutonium: Can someone help me out in figuring out how to use DeviceIoControl on this case i found in a driver

it's confusing considering it's reading a physical address from PIRP + 24 and writing back to the same address , it has different cases for different buffer sizes
[Attachments: image.png]

[2025-08-24 17:51] Xyrem: [replying to Plutonium: "Can someone help me out in figuring out how to use..."]
press y on PIRP, and set it to _IRP* or PIRP

[2025-08-24 17:53] Xyrem: see the double dereference on MmMapIoSpace? Thats reading PIRP+24 which is SystemBuffer, then it reads it again which contains the physical address of what its trying to map

[2025-08-24 17:54] Xyrem: `*(uint8_t*)(PIO_STACK_LOCATION+8)` refers to _IO_STACK_LOCATION!DeviceIoControl.OutputBufferLength

[2025-08-24 17:56] Xyrem: InputBufferLength is incorrectly named, its actually OutputBufferLength

[2025-08-24 17:59] Plutonium: [replying to Xyrem: "`*(uint8_t*)(PIO_STACK_LOCATION+8)` refers to _IO_..."]
Ah my bad, messed up on reading the structure, i am still confused as to how it's assigning the mapped data back to PIRP + 24 ? , this driver has a call to allocate mdl and return it's address as well , but on reading or writing to that it causes a bsod, i'm not sure if it's a fault in the read on phys mem or the alloc itself

[2025-08-24 18:02] Xyrem: > i am still confused as to how it's assigning the mapped data back to PIRP + 24 ? 
This is because SystemBuffer on PIRP is actually both the InputBuffer and OutputBuffer, the InputBuffer is copied in there, and on return its copied to the OutputBuffer

[2025-08-24 18:03] Plutonium: So i'd need to pass the same variable as input and output buffer ?

[2025-08-24 18:07] Xyrem: well from that screenshot you sent, you'd need to call it like this if you use DeviceIoControl:
```
bool Read1Byte(uint64_t PhysAddr, uint8_t* Data)
{
    return DeviceIoControl(hDriver, IoControlCode, &PhysAddr, 8, Data, 1, 0, 0);
}

uint8_t Value{};
bool Success = Read1Byte(0x2BADD00D, &Value);
```

[2025-08-24 18:10] Xyrem: Gonna assume that for every read size type, it just uses OutputBufferLength. In that case swap "1" with the size you want, as well as ensure that Value's size is the same.

[2025-08-24 18:11] Plutonium: Yeah it does , and i think i got it now how it's assigning the value to data, thnx

[2025-08-24 18:11] Xyrem: np

[2025-08-24 18:17] Deus Vult: [replying to Xyrem: "well from that screenshot you sent, you'd need to ..."]
Isn't that "PhysAddr" a new copy?

[2025-08-24 18:18] Xyrem: [replying to Deus Vult: "Isn't that "PhysAddr" a new copy?"]
Wdym? It's passing a pointer to the "PhysAddr" to DeviceIoControl

[2025-08-24 18:19] Deus Vult: I mean the argument, the way they've set it out it's making a copy of PhysAddr, do you know why they wouldn't just pass as reference?

[2025-08-24 18:20] Deus Vult: I see the use case now

[2025-08-24 18:21] Deus Vult: So they *have* to pass it by raw address

[2025-08-24 18:23] Xyrem: 0x2BADD00D would be loaded into the rcx register, Read1Byte will "copy" it over to the stack and will pass a pointer of the copy to DeviceIoControl. DeviceIoControl expects a pointer to the InputBuffer, same with OutputBuffer which is why the reference

[2025-08-24 18:24] Deus Vult: I'm trying to really wrap my head around this stuff with C++ and actually think about things apologies if that didn't make sense 😆

[2025-08-24 18:24] Plutonium: I'm still rather new to physical memory, would 0x2BADD00D be loaded in memory under any case , like is it a safe value to test the call ?

[2025-08-24 18:25] Xyrem: No not at all lol, its just a dummy value

[2025-08-24 18:26] Xyrem: talking about safety here ^

[2025-08-24 18:26] Plutonium: Yeah thought so , idk why it's still crashing , write call works fine tho

[2025-08-24 18:26] Plutonium: 
[Attachments: image.png]

[2025-08-24 18:26] Xyrem: where does it crash?

[2025-08-24 18:27] Plutonium: when reading the value from my mdl alloc

[2025-08-24 18:27] Xyrem: what mdl alloc

[2025-08-24 18:28] Plutonium: ```C
v8 = *(PHYSICAL_ADDRESS **)(a3 + 24);
  v9 = MmMapIoSpace(*v8, v8[1].LowPart, MmNonCached);
  v10 = v9;
  if ( v9 )
  {
    v11 = 0;
    Mdl = IoAllocateMdl(v9, v8[1].LowPart, 0, 0, 0LL);
    v13 = Mdl;
    if ( Mdl )
    {
      MmBuildMdlForNonPagedPool(Mdl);
      v14 = MmMapLockedPagesSpecifyCache(v13, 1, MmNonCached, 0LL, 0, 0x10u);
```

[2025-08-24 18:28] Deus Vult: <@775688014176452608> so is there any way of preventing a copy even when you pass an immediate value to an argument like that

[2025-08-24 18:30] Xyrem: [replying to Plutonium: "```C
v8 = *(PHYSICAL_ADDRESS **)(a3 + 24);
  v9 = ..."]
I dont think the implementation is even correct for that driver

[2025-08-24 18:30] Deus Vult: I'm so confused with how that function is written why does it want the address of PhysAddr and not the value

[2025-08-24 18:31] Deus Vult: When you pass "0x2BADD00D" isn't it pass the address of this <- instead of the actual value

[2025-08-24 18:31] Xyrem: https://learn.microsoft.com/en-us/windows/win32/api/ioapiset/nf-ioapiset-deviceiocontrol
[Embed: DeviceIoControl function (ioapiset.h) - Win32 apps]
Sends a control code directly to a specified device driver, causing the corresponding device to perform the corresponding operation.

[2025-08-24 18:33] Xyrem: [replying to Deus Vult: "When you pass "0x2BADD00D" isn't it pass the addre..."]
no because it will try to copy "InputBuffer" into SystemBuffer when crafting the IRP, InputBuffer needs to point to something which exists else it will throw a exception which would fail the call. When I referenced "PhysAddr" it will copy the contents of it into SystemBuffer which would be the constant 0x2BADD00D

[2025-08-24 18:33] Xyrem: [replying to Plutonium: "```C
v8 = *(PHYSICAL_ADDRESS **)(a3 + 24);
  v9 = ..."]
Can you check exactly which part it bsods on

[2025-08-24 18:34] Plutonium: Wait sorry i'll give some more context, this is the full func, it's returning the physical address to the allocated mdl

```C
__int64 __fastcall Io_Handler(__int64 a1, __int64 a2, IRP *a3)
{
  int v6; // r9d
  PHYSICAL_ADDRESS *MasterIrp; // r15
  PVOID v9; // rax
  PVOID v10; // r12
  int v11; // edi
  struct _MDL *Mdl; // rax
  struct _MDL *v13; // r14
  PVOID v14; // rdx
  int v15; // eax
  __int64 v16; // rcx
  _DWORD *v17; // rax
  __int64 v18; // rbx

  if ( ((*(_DWORD *)(a2 + 8) - 4) & 0xFFFFFFFB) != 0 )
  {
    a3->IoStatus.Information = 4LL;
    v6 = 146;
LABEL_3:
    return 3221225990LL;
  }
  if ( *(_DWORD *)(a2 + 16) != 12 )
  {
    a3->IoStatus.Information = 12LL;
    v6 = 153;
    goto LABEL_3;
  }
  MasterIrp = (PHYSICAL_ADDRESS *)a3->AssociatedIrp.MasterIrp;
  v9 = MmMapIoSpace(*MasterIrp, MasterIrp[1].LowPart, MmNonCached);
  v10 = v9;
  if ( v9 )
  {
    v11 = 0;
    Mdl = IoAllocateMdl(v9, MasterIrp[1].LowPart, 0, 0, 0LL);
    v13 = Mdl;
    if ( Mdl )
    {
      MmBuildMdlForNonPagedPool(Mdl);
      v14 = MmMapLockedPagesSpecifyCache(v13, 1, MmNonCached, 0LL, 0, 0x10u);
      v15 = *(_DWORD *)(a2 + 8);
      if ( v15 == 4 )
      {
        *(_DWORD *)a3->AssociatedIrp.MasterIrp = (_DWORD)v14;
        v15 = *(_DWORD *)(a2 + 8);
      }
      if ( v15 == 8 )
        *(_QWORD *)a3->AssociatedIrp.MasterIrp = v14;
      v16 = 0LL;
      v17 = (_DWORD *)(a1 + 48);
      while ( *v17 )
      {
        ++v11;
        ++v16;
        v17 += 10;
        if ( v16 >= 256 )
          goto LABEL_19;
      }
      v18 = a1 + 40LL * v11;
      *(_DWORD *)(v18 + 48) = MasterIrp[1].LowPart;
      *(_QWORD *)(v18 + 24) = v10;
      *(_QWORD *)(v18 + 32) = v14;
      *(_QWORD *)(v18 + 40) = v13;
      *(_QWORD *)(v18 + 56) = PsGetCurrentProcessId();
LABEL_19:
      if ( v11 != 256 )
        return 0LL;
    }
  }
  return 3221225473LL;
}```

[2025-08-24 18:34] Plutonium: And i am trying to read/write to that address space

[2025-08-24 18:35] Xyrem: what is a1, a2, a3?

[2025-08-24 18:35] Xyrem: a3 im gonna assume is the IRP

[2025-08-24 18:37] Plutonium: a2 is CurrentStackLocation and a1 is v5 ```v5 = *(_QWORD *)(a1 + 64);```

[2025-08-24 18:37] Plutonium: here a1 is the device_obj

[2025-08-24 18:38] Xyrem: can you first set the correct structure types into IDA for all these variables

[2025-08-24 18:40] Plutonium: I updated it

[2025-08-24 18:41] Deus Vult: [replying to Xyrem: "https://learn.microsoft.com/en-us/windows/win32/ap..."]
Thank you

[2025-08-24 18:41] UJ: The second param of MmMapIoSpace, size, comes from the input structure as well it seems. something like {physical address, size}. so im guessing the input struct for it would be {phys_addr: PHYSICAL_ADDRESS, int32: size}

[2025-08-24 18:42] Xyrem: [replying to Plutonium: "Wait sorry i'll give some more context, this is th..."]
Right from what it looks like, its not reading anything. It's just remapping it into usermode space

[2025-08-24 18:42] UJ: what driver is this? i can take a look

[2025-08-24 18:43] Xyrem: [replying to Xyrem: "well from that screenshot you sent, you'd need to ..."]
What I sent here is only applicable for this https://discord.com/channels/835610998102425650/835635446838067210/1409221651920326656

[2025-08-24 21:52] Aj Topper: so i was working on some shit and using this as a refrence
https://xacone.github.io/eneio-driver.html
it was using a Low Stub method to get cr3 to convert vistual memory to physical(Crafting the Exploit Part)
so i tried but everytime i m getting Could not find nt!HalpLMStub pointer in the Low Stub. The offset may be incorrect for your specific Windows build. but i reversed it with ntoskrnl.exe

so is this perticualr method pacthed.....?
[Embed: Exploiting eneio64.sys through Physical Memory R/W]