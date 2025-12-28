# September 2025 - Week 3
# Channel: #application-security
# Messages: 6

[2025-09-17 22:17] U Mad Bro?: Not sure if this is the right place to ask, but I’ll give it a try!
Do you guys know or have some resources about if it is possible to declare and call JNI method from Java without using the “native” keyword in a way that is compatible with recent Android versions? The purpose is to make it harder to perform trivial hooking as I’ve seen that even with DexGuard the “native” persist in the function signature remains. Furthermore, I cannot decouple Java and Native code by inserting application-specific logic since the native code must be portable between multiple applications.
Many thanks in advance for anyone that will take time to read this!

[2025-09-18 05:30] Timmy: [replying to U Mad Bro?: "Not sure if this is the right place to ask, but I’..."]
iirc (not expert) java 24 has very good native call and memory operations support. they created an alternative to JNI basically

[2025-09-18 05:33] Timmy: close, FFM is replacing JNI since JDK22

[2025-09-18 05:34] Timmy: from what I've heard its a lot more pleasant than jni

[2025-09-18 05:42] Timmy: https://www.happycoders.eu/java/foreign-function-memory-api/
[Embed: Java Foreign Function & Memory API (FFM API)]
Learn how to access libraries of other programming languages from Java using the Foreign Function & Memory API.

[2025-09-18 07:19] U Mad Bro?: Thanks! I’ll surely give it a look