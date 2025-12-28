# May 2024 - Week 1
# Channel: #🔗-research-and-links
# Messages: 6

[2024-05-01 08:52] Azalea: what can be dome using a double fetch?

[2024-05-01 09:07] Koreos: [replying to Azalea: "what can be dome using a double fetch?"]
See the article above: https://exploits.forsale/24h2-nt-exploit/

[2024-05-01 10:17] Azalea: ah ic

[2024-05-01 23:47] szczcur: [replying to Azalea: "what can be dome using a double fetch?"]
if you can modify the memory between two fetches you can exploit that race condition.. you could turn it into an arb write, or overflow a buffer if the length isn't checked again, escape sandboxes, etc. there's different types like arithmetic races, or binary races. very much worth learning about as any toctou bug is esp between km<->um

[2024-05-03 18:08] [Janna]: I wanted to share some resources! https://www.cs.fsu.edu/~tvhoang/research.html
it's mostly about cryptography but; yea ^_^ have a nice weekend, you who reads this
for example:
- Viet Tung Hoang and Yaobin Shen.   **Security Analysis of NIST CTR-DRBG, CRYPTO 2020,**   pages 218-247.

[2024-05-03 20:41] daax: https://nicolo.dev/en/blog/fairplay-apple-obfuscation/
[Embed: Analysis of Obfuscation Found in Apple FairPlay]
FairPlay comprises a set of algorithms created by Apple for digital rights management (also called DRM, digital rights management). FairPlay is currently used to manage the decryption of iOS applicati