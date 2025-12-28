# July 2025 - Week 5
# Channel: #programming
# Messages: 20

[2025-07-28 06:47] dullard: [replying to the horse: "left/right/center doesn't look good"]
Put them on the right and try stack the buttons on top of each other

[2025-07-28 06:47] dullard: Ah <@1340811848022360096> suggested well

[2025-07-28 08:35] the horse: [replying to dullard: "Put them on the right and try stack the buttons on..."]
ended up with this
[Attachments: image.png, image.png]

[2025-07-28 08:38] the horse: [replying to Timmy: "why not use something like uptime kuma instead of ..."]
I got paid for it

[2025-07-28 08:38] the horse: ¯\_(ツ)_/¯

[2025-07-28 08:40] the horse: and he wanted the whole thing in rust (web, backend, service) and I wanted to learn a bit more rust 👍 
+ we need some more analytics

[2025-07-29 16:31] sariaki: Hi, I'm new to LLVM and wanted to ask:

do phi nodes and alloca+store/loads not accomplish the same thing? (that being sorta mutable variables in an SSA language)
if yes, then what's the reason for both being implemented in LLVM? Also in what situation is which of the two preferred?

Concretely, I'm dealing with a loop and was wondering which of the two i should use for updating variables (iterator etc.)

[2025-07-29 17:46] Brit: If it can be a phi node it should be a phi node

[2025-07-29 17:47] Brit: [replying to sariaki: "Hi, I'm new to LLVM and wanted to ask:

do phi nod..."]
https://longfangsong.github.io/en/mem2reg-made-simple/

[2025-07-29 17:48] Brit: This is abt mem 2 reg which essentially transforms alloca to phi nodes

[2025-07-29 17:49] Brit: Because alloca will be on stack whereas phinodes if they fit in a reg will be put in a reg

[2025-07-29 18:11] sariaki: i see. thank you!

[2025-07-30 13:22] contificate: the major takeaway being frontends should just construct code that expects mem2reg

[2025-07-30 13:22] contificate: hence you don't need to really construct SSA to construct LLVM IR - even though the IR is always in SSA, because the memory isn't (allocas)

[2025-07-30 18:49] sariaki: Is there any way to force LLVM to generate a float as a double?
I'm just calling an LLVM function that takes a double as a param and when compiling the IL, I get the following: 

```
C:\Users\sariaki\Documents\code\py\JuFo-2026\examples>clang ./example_obf.ll
./example_obf.ll:323:33: error: floating point constant does not have type 'double'
  323 |   %p0 = call double @exp(double 0xR1AEC95BFF04578)
      |                                 ^
1 error generated.```

This is my code: ```c++
const auto Lambda = ConstantFP::get(IRB.getDoubleTy(), (double)0.42069);
Value* NegLambda = IRB.CreateFNeg(Lambda);
Value* P0 = IRB.CreateCall(ExpFn, { NegLambda }, "p0");
```
For some reason, `Lambda` gets turned into a bfloat16 even though i specify the type as a double.

[2025-07-31 08:26] sariaki: this appears to be an IR emitting issue i guess?
```c++
Lambda->getType()->print(errs());
``` prints `double`

[2025-07-31 08:33] Yoran: [replying to sariaki: "this appears to be an IR emitting issue i guess?
`..."]
Maybe LLVM treats floats as doubles but keeps the float precision or smth?

[2025-07-31 08:33] sariaki: Nah, this is just the IR text representation being wrong.

edit: emitting bitcode ACTUALLY works.

[2025-07-31 08:34] sariaki: ¯\_(ツ)_/¯

[2025-07-31 08:34] Yoran: Gotcha