# March 2025 - Week 1
# Channel: #programming
# Messages: 12

[2025-03-02 06:11] T3nb3w: https://x.com/T3nb3w/status/1896078852054823263
[Embed: T3nb3w (@T3nb3w) on X]
🚀 New Blog & PoC: Abusing IDispatch for COM Object Access & PPL Injection

Leveraging STDFONT via IDispatch to inject into PPL processes & access LSASS\. Inspired by James Forshaw's research\!

🔍 Blog

[2025-03-02 06:41] UJ: he's backkkkkkkkkkkk. 

 bro overflowed the buffer to the allocated size and overrode the return address to the functioned that called us. 

https://github.com/SleepTheGod/SSH-RCE

[2025-03-02 06:52] vmx: [replying to UJ: "he's backkkkkkkkkkkk. 

 bro overflowed the buffer..."]
he did it again, what a chad

[2025-03-02 07:17] avx: going to be a big day for him when he finds out about `_ReturnAddress()`/`__builtin_return_address()`

[2025-03-02 08:20] vmx: [replying to avx: "going to be a big day for him when he finds out ab..."]
i believe he left that as an exercise for the reader

[2025-03-02 08:28] 25pwn: does anyone know how to get `libMLIR.so` on nix? fedora has it but i don't think nix does:
```
$ find /nix/store -iname "libLLVM.so"
/nix/store/sj58s6n3rhvp21b4zmh7igbwzgvh8kwz-llvm-19.1.7-lib/lib/libLLVM.so
/nix/store/y4y14m7cn4rfgapgajbv2zzlqrjimyvd-llvm-18.1.8-lib/lib/libLLVM.so
$ find /nix/store -iname "libMLIR.so"
$ find /nix/store -iname "libMLIR*.so"
/nix/store/drvcrx2a179imai2vx5r2rikymq167nf-mlir-19.1.7/lib/libmlir_arm_runner_utils.so
/nix/store/drvcrx2a179imai2vx5r2rikymq167nf-mlir-19.1.7/lib/libmlir_arm_sme_abi_stubs.so
/nix/store/drvcrx2a179imai2vx5r2rikymq167nf-mlir-19.1.7/lib/libmlir_async_runtime.so
/nix/store/drvcrx2a179imai2vx5r2rikymq167nf-mlir-19.1.7/lib/libmlir_c_runner_utils.so
/nix/store/drvcrx2a179imai2vx5r2rikymq167nf-mlir-19.1.7/lib/libmlir_float16_utils.so
/nix/store/drvcrx2a179imai2vx5r2rikymq167nf-mlir-19.1.7/lib/libmlir_runner_utils.so
```

looking at
- [D73130](https://reviews.llvm.org/D73130)
- [D79067](https://reviews.llvm.org/D79067)
- [fedora's llvm.spec](https://src.fedoraproject.org/rpms/llvm/blob/rawhide/f/llvm.spec)
`-DLLVM_BUILD_LLVM_DYLIB=ON` should fix it([afaict nixpkgs doesn't have it](https://github.com/search?q=repo%3ANixOS%2Fnixpkgs+LLVM_BUILD_LLVM_DYLIB&type=code)), but i have no idea how to add the flags "the nix way".

after that I also have to make it visible to the linker using `-L`, but no idea how do that either.
[Embed: Build software better, together]
GitHub is where people build software. More than 150 million people use GitHub to discover, fork, and contribute to over 420 million projects.

[2025-03-02 08:35] avx: [replying to vmx: "i believe he left that as an exercise for the read..."]
ahahaha

[2025-03-02 08:43] sunbather: [replying to UJ: "he's backkkkkkkkkkkk. 

 bro overflowed the buffer..."]
This one's kinda lame, he should at least include `malloc` or at least something about SSH in `vulnerable.c`. Not plausible enough

[2025-03-02 10:29] Timmy: [replying to 25pwn: "does anyone know how to get `libMLIR.so` on nix? f..."]
I think you should be able to define BUILD_SHARED_LIBS=ON through building llvm with this shell https://wiki.nixos.org/wiki/LLVM
[Embed: LLVM]

[2025-03-02 11:58] 25pwn: [replying to Timmy: "I think you should be able to define BUILD_SHARED_..."]
this isn't what i'm looking for, it doesn't seem like "the nix way". i've seen some examples that override some parts of packages but i'm not sure how to override just the cmake flags

[2025-03-02 13:26] Deleted User: hello, i was wondering if you must ensure the IRQL of the current core is above the thread schedulers IRQL after execution of vmptrld. this is so you can initialise the vmcs without context switch happening. because i assume after vmptrld, vmexits can occur, and if vmexit happens before VMCS is fully initialised i assume that could trigger bugcheck

[2025-03-02 21:44] donna🤯: [replying to Deleted User: "hello, i was wondering if you must ensure the IRQL..."]
vmptrld doesnt start vmx operation, vmx operation only begins once vmlaunch is executed