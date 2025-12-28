# May 2024 - Week 4
# Channel: #application-security
# Messages: 4

[2024-05-21 22:34] vendor: https://github.com/HigherOrderCO/bend
[Embed: GitHub - HigherOrderCO/Bend: A massively parallel, high-level progr...]
A massively parallel, high-level programming language - HigherOrderCO/Bend

[2024-05-21 22:36] vendor: i had a feeling this might be exciting for <#835656787154960384> and the last bit of the guide file confirms what i had in mind.

```
Of course, you would absolutely not want to sort numbers like that, specially when mutable arrays exist. But there are many algorithms that can not be implemented easily with buffers. Evolutionary and genetic algorithms, proof checkers, compilers, interpreters. For the first time ever, you can implement these algorithms as high-level functions, in a language that runs on GPUs. That's the magic of Bend!
```

[2024-05-21 22:42] vendor: i’m wondering if this would make new n-bit MBA equivalence proofs feasible. granted there are definitely more intelligent ways to try and fix existing provers but could throwing more (GPU parallelised) compute make 32+ bit exhaustive proofs feasible?

[2024-05-21 22:43] vendor: not sure what operators they can do and just how big the integers can get. seems 24 bit is the current limit but they say will go bigger soon.