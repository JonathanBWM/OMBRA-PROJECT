# December 2025 - Week 3
# Messages: 77

[2025-12-15 01:32] vanessa: [replying to noahsx: "yeah placed hooks on KeRegisterNmiCallback, never ..."]
hey im wondering do u still have any logs from this?

[2025-12-15 01:43] Addison: epic username

[2025-12-15 02:05] BloodBerry: Girl hacker?)

[2025-12-15 02:05] the horse: ladyboy 😍

[2025-12-15 02:06] BloodBerry: [replying to the horse: "ladyboy 😍"]
Omg no way

[2025-12-15 02:06] Addison: [replying to BloodBerry: "Girl hacker?)"]
nah bro the discord username not the nick

[2025-12-15 07:49] noahsx: [replying to vanessa: "hey im wondering do u still have any logs from thi..."]
i do not, the whole point is that none of them came through

[2025-12-15 07:49] noahsx: i can spin up siege later and check again for you if youd like

[2025-12-15 07:50] noahsx: Ill hook HalSendNmi and KeRegisterNmiCallback on a hvci system

[2025-12-15 13:48] vanessa: [replying to noahsx: "i can spin up siege later and check again for you ..."]
yh id  appreciate t hat

[2025-12-16 09:37] noahsx: [replying to vanessa: "yh id  appreciate t hat"]
im away right now so i cant for a while

[2025-12-16 09:38] noahsx: but again, that research was in like august

[2025-12-16 09:38] noahsx: So only 4 months ago

[2025-12-16 22:25] glut: anyone have any info on how hyperion's control flow guard works?

[2025-12-16 22:25] glut: haven't been paying much attention to roblox, thought i'd ask here for some info as it doesn't seem like anyone has posted publicly about it

[2025-12-16 22:26] glut: IIRC they don't rely on the default cfg implementation

[2025-12-17 16:15] glut: if anyone cares about this, i found the cfg process function and will reverse it once i have the time to

just curious if somebody could enlighten me with some info 🙂 
```cpp
00df7d50    int64_t hyperion::cfg::process(int64_t virtualAddress @ rax)
00df7d50    {
00df7d50        if (virtualAddress <= 0x7fffffffffff && TEST_BITQ((uint64_t)*(uint8_t*)(hyperion::cfg::bitMap + (virtualAddress >> 0xf)), virtualAddress >> 0xc & 7))
❓00df7d83            /* jump -> virtualAddress */
00df7d83        
00df7d85        /* tailcall */
00df7d85        return sub_df7cd0();
00df7d50    }```
```cpp
00df7cd0    int64_t sub_df7cd0(int64_t virtualAddress @ rax)
00df7cd0    {
00df7cd0        int32_t* rbx;
00df7d0b        uint16_t* rsi;
00df7d0b        uint64_t rdi;
00df7d0b        int64_t r13;
00df7d0b        int64_t r14;
00df7d0b        long double x87_r0;
00df7d0b        long double x87_r3;
00df7d0b        hyperion::cfg::handle(virtualAddress, rbx, rsi, rdi, r13, r14, x87_r0, x87_r3);
❓00df7d40        /* jump -> virtualAddress */
00df7cd0    }
```

[2025-12-17 16:16] glut: a lot of encryption lol
[Attachments: zmaNPnH.cpp]

[2025-12-17 16:20] glut: i'm not sure what they do with the bitmap, most likely encrypt the pointer (if that's possible?) since there are no xrefs to it in the handler

[2025-12-18 01:09] Atom: [replying to glut: "if anyone cares about this, i found the cfg proces..."]
hyperion::cfg::handle is most probably report

[2025-12-18 15:26] glut: [replying to Atom: "hyperion::cfg::handle is most probably report"]
yeah of course, there's way more to it

[2025-12-18 15:26] glut: managed to find an older build of hyperion & located the cfg handler, it was way simpler

[2025-12-18 15:29] glut: funnily enough, roblox lets u run older client versions (not play though) & managed to find a whole vector of whitelisted modules in the cfg handler

[2025-12-19 07:47] bananaaaaaaaa: [replying to glut: "anyone have any info on how hyperion's control flo..."]
its a trick they use to find unwhitelisted memory in the game

[2025-12-19 07:49] bananaaaaaaaa: a bitmap is used as a cache for faster lookups, and if a new address is encountered it gets forwarded to a bigger function which actually checks if its whitelisted

[2025-12-19 07:50] bananaaaaaaaa: if it ever hits unwhitelisted memory hyperion will crash your game after a random amount of time

[2025-12-19 07:52] bananaaaaaaaa: [replying to glut: "a lot of encryption lol"]
this is obfuscated, if you clean up the junk & opaque branches you will see the actual logic is much simpler

[2025-12-19 15:35] glut: [replying to bananaaaaaaaa: "this is obfuscated, if you clean up the junk & opa..."]
yeah makes sense

[2025-12-19 15:36] glut: i think i'm required to look at this lol, or some way to sort of whitelist my allocations

[2025-12-19 15:36] glut: tried taking a shortcut by using a RW code cave & changing it's protection to RWX

[2025-12-19 15:37] glut: which ofc, works but CFG fucks me up when i try to do anything (ex: vft swap, calling some engine funcs) (inline asm is fine unless i call a function in the RWX code cafve)

[2025-12-19 15:38] glut: but yeah, i'll spend my time cleaning the function up & continue reversing it

[2025-12-19 15:38] glut: if i'm not wrong their cache is integrity checked (?) so even if i added to my allocations to their cache i'd crash

[2025-12-19 20:47] bananaaaaaaaa: [replying to glut: "yeah makes sense"]
there were some good blogs about hyperion but it looks like they were removed

[2025-12-19 20:47] bananaaaaaaaa: https://web.archive.org/web/20251019060013/https://blog.nestra.tech/reverse-engineering-hyperion-bypassing-control-flow-guard/
[Embed: Reverse Engineering Hyperion: Bypassing Control Flow Guard]
In this post, we will analyze how Hyperion modifies Windows' Control Flow Guard (CFG) to enforce its own validation checks. Instead of relying on the default CFG implementation, Hyperion overwrites it

[2025-12-19 20:48] bananaaaaaaaa: [replying to glut: "if i'm not wrong their cache is integrity checked ..."]
i doubt the bitmap is

[2025-12-19 20:52] bananaaaaaaaa: it gets read & written to very often

[2025-12-19 23:43] glut: [replying to bananaaaaaaaa: "it gets read & written to very often"]
i see, thank you, i'll look into it further

[2025-12-19 23:44] glut: [replying to bananaaaaaaaa: "https://web.archive.org/web/20251019060013/https:/..."]
yeah thank you lol i saw that today on UC

<https://gist.github.com/atrexus/11e71d6b245e99a545c05021f72bc992>
```cpp
    // Method 1.2:
    //  Here we "cache" a region of memory, by adding it to the cache. Once CFG encounters an address in the provided region, it will think
    //  that it has already processed it and skip it.
    void cache( std::uintptr_t base, const std::size_t size ) noexcept
    {
        // Get the control flow guard cache
        const auto cfg_cache = *reinterpret_cast< std::uintptr_t* >( GetModuleHandleA( "RobloxPlayerBeta.dll" ) + cfg_cache );

        if ( cfg_cache )
        {
            // Align the base address
            base &= -cfg_cache_alignment;

            // Add all pages to the cache
            for ( auto pg = base; pg < base + size; pg += cfg_cache_alignment )
            {
                const auto entry = cfg_cache + ( pg >> 0x13 );
                
                *reinterpret_cast< std::uint32_t* >( entry ) |= 1 << ( ( pg >> 0x10 & 7 ) % 0x20 );
            }     
        }
    }```
interesting stuff, that's what i thought of doing

[2025-12-19 23:46] glut: also on the newest hyperion build for some reason the bitmap has xrefs

[2025-12-19 23:46] glut: i saw 2 in the handler

[2025-12-19 23:46] glut: didnt appear for me before

[2025-12-19 23:47] glut: i did not clean up anything (opaque branches, junk, dead code & etc)

[2025-12-20 00:19] bananaaaaaaaa: [replying to glut: "i did not clean up anything (opaque branches, junk..."]
its pretty easy to do for the CFG routine since its so small

[2025-12-20 00:19] bananaaaaaaaa: you can do it all by hand

[2025-12-20 00:20] glut: i can tell lol

[2025-12-20 00:20] glut: it looks like it hasnt changed much since that blog post

[2025-12-20 00:20] glut: just a bunch of obfuscation

[2025-12-20 00:21] bananaaaaaaaa: yeah

[2025-12-20 00:21] bananaaaaaaaa: its decent, it will stop most beginners

[2025-12-20 00:21] glut: [replying to glut: "yeah thank you lol i saw that today on UC

<https:..."]
interesting stuff xd, im not surprised someone has already done this either

[2025-12-20 00:22] glut: not sure how i've just seen that post

[2025-12-20 00:22] bananaaaaaaaa: there used to be a whole collection of blogs on there

[2025-12-20 00:22] bananaaaaaaaa: but for some reason they all got deleted

[2025-12-20 00:23] glut: yeah i can tell i went to the blog page

[2025-12-20 00:23] glut: its a shame they got deleted

[2025-12-20 00:26] bananaaaaaaaa: [replying to glut: "its a shame they got deleted"]
the archive still has them so

[2025-12-20 00:26] bananaaaaaaaa: you can read them all

[2025-12-20 00:27] bananaaaaaaaa: the CFG impl was very sneeky

[2025-12-20 00:27] glut: how do i see who made the blogs?

[2025-12-20 00:27] glut: is it the same guy from the gist

[2025-12-20 00:27] bananaaaaaaaa: uhhh

[2025-12-20 00:27] bananaaaaaaaa: i think so

[2025-12-20 00:28] bananaaaaaaaa: 
[Attachments: image.png]

[2025-12-20 00:28] bananaaaaaaaa: ye

[2025-12-20 00:28] bananaaaaaaaa: atrexus

[2025-12-20 00:28] glut: any clue if he has more posts outside of that blog? he posts interesting stuff

[2025-12-20 00:28] glut: id assume he posts outside of that blog xd

[2025-12-20 00:29] bananaaaaaaaa: no clue

[2025-12-20 00:29] bananaaaaaaaa: https://github.com/atrexus
[Embed: atrexus - Overview]
atrexus has 10 repositories available. Follow their code on GitHub.

[2025-12-20 00:29] bananaaaaaaaa: thats his github

[2025-12-20 00:31] glut: https://github.com/atrexus/ws-watcher

[2025-12-20 00:31] glut: oh cool lol

[2025-12-20 00:31] glut: reminds me of this https://gist.github.com/nmulasmajic/de68e1016862024a964220d6a7f1a602

[2025-12-20 00:31] glut: https://atrexus.github.io/posts/abusing-the-process-working-set

[2025-12-20 00:31] glut: ooo

[2025-12-21 23:38] >_: So, I am just a web programmer, and I decided to make cheats for games just for fun. I spent a couple of weeks learning the Win32 library, but I was surprised to find out that games have anti-cheat systems. These anti-cheat systems can detect you very easily, and I realized that I would need to program at the kernel level. Now I feel confused because I don’t really know what I should do next.