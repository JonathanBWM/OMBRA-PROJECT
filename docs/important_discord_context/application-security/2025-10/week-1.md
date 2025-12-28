# October 2025 - Week 1
# Channel: #application-security
# Messages: 15

[2025-10-01 16:26] dullard: [replying to moneytherapy: "if i want to exploit some already patched driver w..."]
Wut ? 

Find the KB update for the fixed version, cross reference with winbindex and you should be able to find the before and after files

From there diff with bindiff and bobs your uncle

[2025-10-01 16:41] moneytherapy: [replying to dullard: "Wut ? 

Find the KB update for the fixed version, ..."]
where do u find KB updates?

[2025-10-01 16:59] dullard: [replying to moneytherapy: "where do u find KB updates?"]
Microsoft msrc update page

[2025-10-01 16:59] dullard: For the CVE

[2025-10-01 17:00] dullard: There’s a list for each OS / type if you scroll down and the download links are on the right

[2025-10-03 18:57] mibho: would appreciate ideas to write about regarding safedisc 2.8.10. <:pepeSmile:786434644085571594> it's NOT a guide on cracking and focus is on obfuscation/software protection. no nanomites (already covered by armadillo guides) or API mangling; ideally regards topics that havent been publically discussed or misconceptions. ive already decided on the following (each will have their own dedicated pages/posts):

1. big picture overview (timeline of events to conceptualize whats happening)
2. custom AES-ctr
3. self-checksumming, self-modifying code linked-list dispatcher (tamper resistant CF)
4. PRNG-driven encoded control flow (tamper resistant CF)
5. Challenge-Response Key Calculation mechanism
6. Examples of abusing "bad" coding practices
  - "out of bounds" indexing via pointer manipulation
  - manual typecasting after passing opaque type
  - swapping/modifying prior frame references
  - switching between passing pointer type and address of that data as an int, then typecasting
7. Stealthy Anti-Tamper Trap (very subtle but not novel; still cool)

[2025-10-03 20:46] szczcur: [replying to mibho: "would appreciate ideas to write about regarding sa..."]
nix 6 and focus on the rest

[2025-10-03 20:47] szczcur: trying to do too much on a blog post just results in a weak post. if you focus on 2-5 and build out the content for those in detail youll have a good post imo

[2025-10-03 20:51] mibho: [replying to szczcur: "trying to do too much on a blog post just results ..."]
each is their own post since theyre that complex <:Charmanderp:785324601047121930>

[2025-10-03 20:51] mibho: would u still recommend not covering those

[2025-10-03 20:52] szczcur: [replying to mibho: "would u still recommend not covering those"]
maybe in that case its fine

[2025-10-03 20:52] szczcur: i thought it was all one post

[2025-10-03 20:53] mibho: yeah thatd be insane <:KEKWCRY:687227788729647110>

[2025-10-03 22:48] UJ: [replying to mibho: "would appreciate ideas to write about regarding sa..."]
don't just focus on what you found. Also detail how you found it, your thought process/reasoning while reversing etc.

[2025-10-04 01:57] mibho: [replying to UJ: "don't just focus on what you found. Also detail ho..."]
gonna be rough w/o rambling so i was planning on addressing what i could in the preface (separate post) and #1 since the remaining parts are heavy on the technical details (i still plan on sharing insight on what wasnt obvious and other observations but not necessarily my thought process)