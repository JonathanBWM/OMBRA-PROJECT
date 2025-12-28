# December 2025 - Week 3
# Channel: #programming
# Messages: 42

[2025-12-15 22:43] aerodak: In UEFI applications is there any way to block memory cleaning? Due to automated memory free my hook to a ntoskrnl function BSOD's when it is called via inline hook

[2025-12-15 22:58] elias: [replying to aerodak: "In UEFI applications is there any way to block mem..."]
What do you mean by automated memory free?

[2025-12-15 22:59] elias: make sure your hook target function is mapped in efi runtime memory

[2025-12-16 00:15] aerodak: [replying to elias: "make sure your hook target function is mapped in e..."]
If i remember correctly, there is 2 different subsystem builds for EDK2. EFI_APPLICATION and EFI_RUNTIME_DRIVER. According to the sources i searched in web: the memory used by EFI_APPLICATONs are free'd after EfiMain/UefiMain callback

When SetVirtualAddressMap Event is called i am making the hook. when i compile the code with Driver Subsystm it works fine but it should be loaded manually.
EFI applications can be run directly. This is why i am forcing myself to build as EFI_APPLICATION.  i am converting the address with

```cpp
__forceinline VOID* GetVirtual(VOID* physical)
{

    VOID* address = physical;
    gRT->ConvertPointer(0, &address);
    return address;
}
```

Also the hook is made in SetVirtualAddressMap 

```cpp
Utils::CopyMemory(Hooks::originalData, Hooks::function, 15);
UINT8 jump[] = { 0x48, 0x31, 0xc0, 0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xe0 };
*reinterpret_cast<UINT64*>(reinterpret_cast<UINT64>(jump) + 5) = reinterpret_cast<UINT64>(GetVirtual(Hooks::NTOSKRNLCALLBACK));
Utils::CopyMemory(Hooks::newData, jump, 15);
Hooks::Hook();
/* Credits to samueltulach */
```

[2025-12-16 01:08] elias: [replying to aerodak: "If i remember correctly, there is 2 different subs..."]
Your code needs to be inside runtime memory to survive after ExitBootServices. What you could do is include a second stage PE that you load into runtime memory from your boot app or you could manually "re map" your current imagine into a new buffer that you allocate within the runtime memory pool

[2025-12-16 01:10] elias: also I'm not sure what you mean by
> memory used by EFI_APPLICATONs are free'd after EfiMain/UefiMain callback

[2025-12-16 01:10] elias: its actually the ExitBootServices call that implicitly frees your code
[Attachments: image.png]

[2025-12-16 01:13] aerodak: [replying to elias: "also I'm not sure what you mean by
> memory used b..."]
Also i forgot to mention about that:  I need to hook exitbootservices to get correct loaderblock address ( to enumerate modules i need &OSLLOADERBLOCK->LoadOrderListHead ) 

exitbootservices should call original function. because on windows boot windows calls it
[Attachments: image.png]

[2025-12-16 01:15] elias: uh

[2025-12-16 01:15] elias: I'm not sure how that's connected to your original issue

[2025-12-16 23:34] dinero: <@609487237331288074> new job opportunity

[2025-12-16 23:34] dinero: thought you’d like to apply

[2025-12-17 00:23] Matti: [replying to aerodak: "Also i forgot to mention about that:  I need to ho..."]
did you read the first part of his post

[2025-12-17 00:23] Matti: the part in which he answered why your code won't work when linked as application

[2025-12-17 00:24] Matti: what are you going to do with the loader block address after exitbootservices? your memory is freed

[2025-12-17 00:25] Matti: either link as runtime driver, or allocate memory of type runtime code/data, at runtime

[2025-12-17 01:00] the horse: you can remap your app and intercept the loader block to ntoskrnl to get the virtualized runtime services

[2025-12-17 01:00] the horse: there might have to be some fixups because efi runs in physical addressing mode

[2025-12-17 02:07] Matti: the issue is he doesn't seem to understand why the remapping is needed

[2025-12-19 00:39] impl: is it a good idea to disable interrupts when im attaching to a process context using `KeStackAttachProcess` and messing with the process pagetables?

[2025-12-19 12:05] outletproblems: [replying to impl: "is it a good idea to disable interrupts when im at..."]
probably not, depending on what you're doing

[2025-12-19 12:07] outletproblems: if you can guarantee it will succeed and can handle if not, i think theres no consequence

[2025-12-20 11:33] Obvious: Why do so many people copy the same definitions from windows headers instead of including the win header itself? Is it simply to prevent overhead or is there more to it?

[2025-12-20 11:36] abu: [replying to Obvious: "Why do so many people copy the same definitions fr..."]
Sometimes they aren't complete

[2025-12-20 17:04] Timmy: [replying to Obvious: "Why do so many people copy the same definitions fr..."]
the windows headers are very large and can potentially cause conflicts in some scenario's

[2025-12-20 17:05] Brit: WIN32_LEAN_AND_MEAN

[2025-12-20 17:05] Brit: but really this only matters if you are building something massive

[2025-12-20 17:05] Brit: with a lot of translation units

[2025-12-20 17:05] Timmy: [replying to Brit: "WIN32_LEAN_AND_MEAN"]
+ NOMINMAX and that fixes most issues I'm aware of

[2025-12-20 18:18] bac0nneggs: Does anyone have good windows hardening scripts?

[2025-12-20 18:47] dullard: [replying to bac0nneggs: "Does anyone have good windows hardening scripts?"]
HardeningKitty, PrivescCheck (itm4n)

[2025-12-20 18:48] dullard: Then things like SeatBelt, PowerUp.ps1 etc for covering extra stuff

[2025-12-20 18:48] dullard: There’s also DoD STIGs and CIS benchmarks for more regulatory/audit checks

[2025-12-21 20:05] ImagineHaxing: Can someone help me?

Ive allocated RWX at 0x6ac0000

I wrote a rust shellcode to decrypt starting from this address

```rs
#![no_std]
#![no_main]

use core::ptr;

struct RC4 {
    s: [u8; 256],
    i: usize,
    j: usize,
}

impl RC4 {
    // Create a new RC4 instance with the given key
    fn new(key: &[u8]) -> Self {
        let mut s = [0u8; 256];
        let mut j = 0;

        // Initialize state
        for i in 0..256 {
            s[i] = i as u8;
        }

        // Key-Scheduling Algorithm (KSA)
        for i in 0..256 {
            j = (j + s[i] as usize + key[i % key.len()] as usize) % 256;
            s.swap(i, j);
        }

        RC4 { s, i: 0, j: 0 }
    }

    // Generate the next byte of the key stream
    fn next_byte(&mut self) -> u8 {
        self.i = (self.i + 1) % 256;
        self.j = (self.j + self.s[self.i] as usize) % 256;
        self.s.swap(self.i, self.j);
        let t = (self.s[self.i] as usize + self.s[self.j] as usize) % 256;
        self.s[t]
    }

    // Encrypt or decrypt data byte-by-byte
    fn process_byte(&mut self, byte: u8) -> u8 {
        byte ^ self.next_byte()
    }
}

#[panic_handler]
fn panic(_: &core::panic::PanicInfo) -> ! {
    loop {}
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn _start() -> ! {
    let mut rc4 = RC4::new(b"ImagineHaxing");
    let address: u64 = 0x24fe1ed0000;
    let size = 135950;
    unsafe {
        for byte in 0..size {
            let addr = address + byte;
            let unencrypted_byte = ptr::read(addr as *const u8);
            let encrypted_byte = rc4.process_byte(unencrypted_byte);
            ptr::write(addr as *mut u8, encrypted_byte);
        }
        let target_fn: extern "C" fn() = core::mem::transmute(address);
        target_fn();
    }
    loop {}
}
```

and then i strip the bin in order to get the shellcode and modify the address and size to the actual payload (in this case 0x6ac0000 and whatever the size it)

[2025-12-21 20:05] ImagineHaxing: issue is that in theory it should work

[2025-12-21 20:05] ImagineHaxing: but when i check 0x6ac0000 its not decrypted and i see the thread being in a loop

[2025-12-21 20:05] ImagineHaxing: (happens only if it panics or finishes, which i doubt it did if the rc4 didnt decrypt)

[2025-12-21 20:06] ImagineHaxing: first time writing sometehing like this so im kinda clueless

[2025-12-21 20:06] ImagineHaxing: please help

[2025-12-21 20:08] ImagineHaxing: [replying to ImagineHaxing: "Can someone help me?

Ive allocated RWX at 0x6ac00..."]
also i checked this code to be correctly written along with the correct address and size and RWX in the target process and it is

[2025-12-21 22:18] Gestalt: what the fuck

[2025-12-21 22:30] ImagineHaxing: [replying to Gestalt: "what the fuck"]
What what