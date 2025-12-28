# September 2025 - Week 3
# Channel: #reverse-engineering
# Messages: 132

[2025-09-15 13:23] djcivi: hi all

any tips on how to figure out structures passed to a NT function? NtUserRegisterClassExWOW to be exact. 

based on reactOS and ida decomp I vaguely know the types but when I try to call it directly (bypassing user32.dll wrapper) it errors out. Its not 'documented' in phnt or anywhere.

all the arguments are passed on the stack (as pointers) and in x64dbg all I see are just some bytes without context ;/

any tips on approaching stuff like this?

[2025-09-15 14:08] qfrtt: [replying to djcivi: "hi all

any tips on how to figure out structures p..."]
```RTL_ATOM APIENTRY     NtUserRegisterClassExWOW (WNDCLASSEXW *lpwcx, PUNICODE_STRING ClassName, PUNICODE_STRING ClsVersion, PCLSMENUNAME pClassMenuName, DWORD fnID, DWORD Flags, LPDWORD pWow)```

[2025-09-15 14:08] qfrtt: And since you don't know what you should put in there just BP on the user32 func and traceback how it's filled

[2025-09-15 14:13] djcivi: Yeah i just alloced those structs on the stack and manually swapped correct(user32) stack addresses with mine until it broke.
I'm incorrectly initializing the CLSMENUNAME. thanks!

[2025-09-15 14:38] diversenok: ```c
// private
typedef struct tagCLSMENUNAME
{
    PSTR pszClientAnsiMenuName;
    PWSTR pwszClientUnicodeMenuName;
    PUNICODE_STRING pusMenuName;
} CLSMENUNAME, PCLSMENUNAME;

// private
typedef struct tagWOWCLS
{
    ULONG vpszMenu;
    USHORT iClsExtra;
    USHORT hMod16;
} WOWCLS, PWOWCLS;

// private
NTSYSCALLAPI
ULONG
NTAPI
NtUserRegisterClassExWOW(
    _In_ PWNDCLASSEXW lpWndClass,
    _In_ PUNICODE_STRING pstrClassName,
    _In_ PUNICODE_STRING pstrClassNameVer,
    _In_ PCLSMENUNAME pcmn,
    _In_ WORD fnid,
    _In_ DWORD dwFlags,
    _In_opt_ PWOWCLS pwc
    );
```

[2025-09-16 12:41] nidi: can i use a instrumentation callback to capture a syscall and modify the arguments or just cancel it?

[2025-09-16 12:43] diversenok: Instrumentation callbacks execute on syscall return, so a bit too late

[2025-09-16 12:44] nidi: oh..

[2025-09-16 12:55] Brit: it's in the name :^)

[2025-09-16 19:26] mrexodia: [replying to KR: "It *does* have an affect on baseline performance a..."]
you are wrong

[2025-09-17 01:34] daax: [replying to mrexodia: "you are wrong"]
now tell him why <:Kappa:794707301436358686>

[2025-09-17 11:37] mrexodia: assumes facts not in evidence

[2025-09-17 14:08] boulevard: Hi guys. I'm reversing a program and i'm pretty sure it uses VMP. On entry,  the .text section is empty and when program tries to access it it triggers a single step exception and then the .text gets filled with real data and real entry point get's called, is there anyway I can place my hooks before the real entry gets called?

[2025-09-17 14:09] Brit: how about hooking the exception handler?

[2025-09-17 14:12] boulevard: so just 0000000140FEF3EE and guessing its vectored handler? in static analisys it virtualized
[Attachments: image.png]

[2025-09-17 14:24] mrexodia: [replying to boulevard: "so just 0000000140FEF3EE and guessing its vectored..."]
I think it's a regular SEH exception, but you can always hook `KiUserExceptionDispatcher` or `RtlDispatchException` to get all of them

[2025-09-17 14:25] mrexodia: I don't think this is the right place though, since this is just VMPs Hypervisor check. Instead you can try hooking NtProtectVirtualMemory, but with newer VMP versions it will use direct syscalls so it is a bit of a pain

[2025-09-17 14:41] boulevard: yeah you're right, i see calls to rdtsc and cpuid

[2025-09-17 14:44] boulevard: anybody know any other approach i should use?

[2025-09-17 14:52] mrexodia: You can check if the `.text` section is already RWX at this point and change it back to read-only

[2025-09-17 16:22] boulevard: Lol well hooking Wow64PrepareForExecution gives me the message "You cannot run under virtual machine"(idk how it makes sense, I am on a VM, but without the hook it works xD) and RtlDispatchException simply crashes the program

[2025-09-18 01:53] daax: [replying to boulevard: "Lol well hooking Wow64PrepareForExecution gives me..."]
Share your hooking method, please. You’re doing something wrong.

[2025-09-18 09:26] KR: [replying to mrexodia: "assumes facts not in evidence"]
Ok, yeah, I didn't cite myself, that doesn't mean I'm wrong. I don't consider 1~ms frame-time difference significant. 1% lows in some titles do tend to take a hit but that's mostly down to what its hooking / protecting and differs by implementation. But I'm comfortable being wrong if you can find something that contradicts what I said.

Pretty decent video that covers testing methodologies and differences between multiple games.
https://www.youtube.com/watch?v=1VpWKwIjwLk
[Embed: Does Denuvo slow game performance? 7 games benchmarked before and a...]
A benchmark of 7 games before and after they removed Denuvo DRM. Games benchmarked include:

Hitman - 1:17
Abzu - 2:05
Sherlock Holmes the Devil's Daughter - 2:40
Mad Max - 4:11
Agents of Mayhem - 7:1

[2025-09-18 09:42] Mikewind22: How convenient.
They forgot to mention huge memory increase with denuvo.
Also CPU usage differences which do not affect framerates when under 100%.
If you have hardware which hits such limits then FPS drops badly.

[2025-09-18 10:47] KR: [replying to Mikewind22: "How convenient.
They forgot to mention huge memory..."]
They... used a 2nd gen intel CPU and a 1080? Any modern CPU, even lower end ones should greatly surpass that, they also make explicit note that they chose a worse CPU explicitly *because* it's CPU dependent

[2025-09-18 10:48] KR: Also, memory usage is visible throughout every test, the testing methodology holds here.

[2025-09-18 10:52] KR: I'm not defending Denuvo, it's a shitty practice, harms the longevity of games and restricts legitimate user freedoms and while there is a legitimate impact on performance it's rather over-stated

[2025-09-18 12:06] mrexodia: [replying to KR: "Ok, yeah, I didn't cite myself, that doesn't mean ..."]
You cannot benchmark before and after, that’s the fundamental issue (since nobody has access to game builds protected/unprotected). Additionally you make this weird assumption that all the people Denuvo hires cannot see this difference with the most advanced profiling tools _with_ access to the before/after protection builds.

They would be idiots if they saw measurable performance differences in the 300+ hour gameplay testing and benchmarking and then let it go live 😂

[2025-09-18 12:07] mrexodia: Trust me, no protected code runs anywhere near the game loop you would see an insane difference (and yes there have been mistakes in the past, but they showed up instantly and were fixed quickly too)

[2025-09-18 12:11] KR: [replying to mrexodia: "You cannot benchmark before and after, that’s the ..."]
There are several game builds that have both protected and unprotected copies available, for example Mad Max being unprotected on Origin but not Steam despite being the same game build

[2025-09-18 12:12] KR: Which does make comparative analysis possible

[2025-09-18 12:15] mrexodia: [replying to KR: "There are several game builds that have both prote..."]
They are not the same build

[2025-09-18 12:16] mrexodia: Steam API integrations include a call in the game loop and they are compiled with different flags and source files

[2025-09-18 12:17] mrexodia: You realize that Denuvo can see when protected functions are called right?

[2025-09-18 12:18] mrexodia: And don’t forget that publishers often put their own virtualized code in the game

[2025-09-18 12:19] KR: [replying to mrexodia: "Steam API integrations include a call in the game ..."]
I mean, you can argue the platform integrations make a difference but builds compiled on the same day with the same compiler aren't going to differ by any meaningful degree

[2025-09-18 12:19] mrexodia: They are

[2025-09-18 12:20] mrexodia: Unless they are building the exact same thing of course

[2025-09-18 12:20] mrexodia: There is code locality that influences things, actual code differences can trigger massive changes in that

[2025-09-18 12:21] mrexodia: And modern CPUs are extremely sensitive to everything

[2025-09-18 12:21] mrexodia: Plus you have to realize that many benchmarkers found performance _improvements_ on cracked games

[2025-09-18 12:22] mrexodia: Which is hilarious if you understand how the cracks work (they trigger shitloads of exceptions causing 3+ context switches majorly slowing down the functions without actually removing Denuvo code)

[2025-09-18 12:22] KR: [replying to mrexodia: "Plus you have to realize that many benchmarkers fo..."]
Cracking denuvo doesn't *typically* remove the virtualization component, I'm not aware of any crack that has fully de-virtualized a protected build nor do I think it's really that feasible

[2025-09-18 12:24] KR: [replying to mrexodia: "Which is hilarious if you understand how the crack..."]
Isn't that a contradiction to what you said earlier?

> They would be idiots if they saw measurable performance differences in the 300+ hour gameplay testing and benchmarking and then let it go live

[2025-09-18 12:31] mrexodia: [replying to KR: "Cracking denuvo doesn't *typically* remove the vir..."]
I am aware of a few such cracks, but it is not how the majority of them work

[2025-09-18 12:31] mrexodia: [replying to KR: "Isn't that a contradiction to what you said earlie..."]
No? Denuvo doesn't test the cracks to make sure they run with great performance

[2025-09-18 12:32] mrexodia: The fact that cracked copies of the same game have improved measured performance means that the measurement difference is unrelated to Denuvo

[2025-09-18 12:33] mrexodia: (because cracks add VMProtected exception handlers increasing runtime of every Denuvo protected function significantly)

[2025-09-18 18:15] daax: [replying to mrexodia: "Trust me, no protected code runs anywhere near the..."]
Duncan, this guy reads reddit and blog posts that misattribute the perf issues and intentionally seek to drive clicks by giving r/pcgaming an avenue for their pseudo-intellectual / ☝🏼🤓 bs — same with people in the past misunderstanding a bottleneck in a game wasn’t the game devs it was the shitty Intel processor and some errata evoked from a tight loop of pretty normal SIMD instructions.

[2025-09-18 18:20] daax: [replying to KR: "Cracking denuvo doesn't *typically* remove the vir..."]
Barring your opinion on what’s possible or feasible (it is both), what do you believe was the cause of these performance improvements noted in cracked in games?

[2025-09-18 18:24] Mikewind22: [replying to daax: "Barring your opinion on what’s possible or feasibl..."]
Ez
Cracked games removed features, textures, geometry so games run faster.

[2025-09-18 18:25] daax: [replying to Mikewind22: "Ez
Cracked games removed features, textures, geome..."]
Can’t tell if you’re trolling

[2025-09-18 18:30] selfprxvoked: Obviously trolling

[2025-09-18 18:30] daax: [replying to selfprxvoked: "Obviously trolling"]
Not obvious to me. We have experienced some interesting characters in here

[2025-09-18 18:30] selfprxvoked: Me included I suppose

[2025-09-18 18:30] selfprxvoked: <:yea:904521533727342632>

[2025-09-18 18:31] daax: You included in what?

[2025-09-18 18:31] selfprxvoked: Interesting characters that you can't differ from trolling

[2025-09-18 18:32] selfprxvoked: [replying to Mikewind22: "Ez
Cracked games removed features, textures, geome..."]
But anyway, what do you mean cracked games removed textures and "geometry" and what it has to do with Denuvo?

[2025-09-18 18:35] Mikewind22: [replying to selfprxvoked: "But anyway, what do you mean cracked games removed..."]
Advanced cracks often involves repacks of game assets. Removes data encryption, etc.

[2025-09-18 18:35] KR: [replying to daax: "Barring your opinion on what’s possible or feasibl..."]
Depends on the method of the crack? The benefit of de-virtualisation is obvious. However the vast majority of cracks **don't** do this.

[2025-09-18 18:36] daax: [replying to KR: "Depends on the method of the crack? The benefit of..."]
Walk me through the one you know best then.

[2025-09-18 18:36] selfprxvoked: [replying to Mikewind22: "Advanced cracks often involves repacks of game ass..."]
So they removed the encryption and not the assets (?)

[2025-09-18 18:38] Mikewind22: They can do whatever they want. It was very popular in 90' to shrink game data cause of huge cd content not fitting into hdds.

[2025-09-18 18:38] KR: [replying to daax: "Walk me through the one you know best then."]
No.
[Attachments: Screenshot_20250919-043658.png]

[2025-09-18 18:39] daax: [replying to KR: "No."]
Then don’t comment on something you don’t know shit about. It’s incredibly fucking irritating.

[2025-09-18 18:39] daax: The answer was always obvious, regardless of the time.

[2025-09-18 18:42] KR: [replying to daax: "Then don’t comment on something you don’t know shi..."]
Dude, I'm not explaining why code virtualization is an expensive operation. It's self explanatory.

And I'm not going to dig up a bin diff on a crack at 4am to walk you though a crack.

Virtualization as an obfuscation technique **is** expensive. I stand behind that.

[2025-09-18 18:44] KR: And the few Denuvo crack I've seen  as of late typically just hook Win32 calls

[2025-09-18 18:52] Deus Vult: [replying to KR: "Dude, I'm not explaining why code virtualization i..."]
Reread what you said

[2025-09-18 18:55] Deus Vult: [replying to KR: "Dude, I'm not explaining why code virtualization i..."]
It's almost 5am your time not 4am

[2025-09-18 18:56] daax: [replying to KR: "Dude, I'm not explaining why code virtualization i..."]
I’m not arguing about if virtualization is expensive or not, and you had plenty of time in the discussion earlier to do this and explain where those performance improvements might’ve come from wrt: https://discord.com/channels/835610998102425650/835635446838067210/1418210547337269400

I asked for you to tell me what you believe (or *know*) to be the reason for a performance improvement in a cracked game that doesn’t actually remove or disable denuvo code from running (I’m not referring to full/partial devirtualization). The method specified in the linked message is one of the easiest ways to instrument and crack whatever DRM.

[2025-09-18 18:58] KR: [replying to Deus Vult: "It's almost 5am your time not 4am"]
I sent that at 4:42 which is very much still 4am

[2025-09-18 18:58] KR: But yes, it is almost 5am now

[2025-09-18 18:58] Deus Vult: [replying to daax: "I’m not arguing about if virtualization is expensi..."]
My idea was how would a cracked version which still applies these code transformations be any less performative than the original I'm not seeing it or am I missing something

[2025-09-18 18:59] Deus Vult: Original as in the original obfucated and protected version without the "patch"

[2025-09-18 19:01] the horse: a crack might perform better or worse than the protected variant depending on how they approached the cracking process; if it's some very janky hooking that incurs more overhead to get desired values - it might very well run worse; if they found a way to skip over larger chunks of code that would in result reduce the overall overhead (including their changes), the game might run a little bit better

[2025-09-18 19:01] Deus Vult: [replying to KR: "I sent that at 4:42 which is very much still 4am"]
I'm sure you are being ignorant and not rounding, but anyways it's okay not to be correct, I always love when someone corrects me but you aren't you are being defensive for things that you are incorrect about, and when you admit your incompetence it'll actually help you  and you will learn from it.

[2025-09-18 19:01] mrexodia: [replying to mrexodia: "Which is hilarious if you understand how the crack..."]
This is how

[2025-09-18 19:02] mrexodia: [replying to mrexodia: "(because cracks add VMProtected exception handlers..."]
And this

[2025-09-18 19:02] mrexodia: No speculation necessary at all

[2025-09-18 19:03] the horse: i completely agree that a majority of cracks will add overhead rather than reduce it

[2025-09-18 19:03] the horse: +1 on that

[2025-09-18 19:04] mrexodia: But it’s also irrelevant for measuring performance, because there won’t be protected functions anywhere near the game loop

[2025-09-18 19:04] the horse: I think the 'major' scenario in which performance was actually improved was with some ubisoft game (they managed to leak an unprotected beta build which was used to reconstruct the final game exe) which sort of became a major advocacy against drm because the game had vmp+denuvo slapped on it in the final production build

[2025-09-18 19:04] mrexodia: So if you add a Sleep(500) to the protected function your frame times won’t change

[2025-09-18 19:05] daax: [replying to mrexodia: "But it’s also irrelevant for measuring performance..."]
So you’re telling me! that! Denuvo! Doesn’t! Degrade! Performance! By any significant measure?? And that my🌈 kiuserexceptiondispatcher hook ✨ and handlers DONT IMPROVE IT?

[2025-09-18 19:05] daax: https://tenor.com/view/black-guy-poggers-poggers-pog-gif-22576295

[2025-09-18 19:06] mrexodia: [replying to the horse: "I think the 'major' scenario in which performance ..."]
🤷‍♂️ nothing relevant can be derived from beta builds

[2025-09-18 19:06] mrexodia: You should see the insane shit in test builds even weeks before release

[2025-09-18 19:06] the horse: [replying to mrexodia: "🤷‍♂️ nothing relevant can be derived from beta bui..."]
the main game executable was almost identical iirc

[2025-09-18 19:08] Deus Vult: Dist will be much better <:kekw:904522300257345566>

[2025-09-18 19:08] the horse: however; i've never been intimate within this subject

[2025-09-18 19:08] the horse: all my favorite AAA studios use acceleron and I don't play any other game

[2025-09-18 19:09] daax: [replying to the horse: "all my favorite AAA studios use acceleron and I do..."]
so true

[2025-09-18 19:09] daax: 
[Attachments: image.png]

[2025-09-18 19:10] daax: it’s such a selfless protection and only checks 8 unused routines in that build! talk about reducing unnecessary overhead!

[2025-09-18 19:11] the horse: if it checked routines it actually used, it wouldn't be accelerated, duh

[2025-09-19 00:52] GG: do you guys know how to reset these ida graph links ? I have multiple of those
[Attachments: image.png]

[2025-09-19 18:30] nu11sec: [replying to mrexodia: "But it’s also irrelevant for measuring performance..."]
What about context switching between game loop thread and thread with protected function would there be a difference if it wasn’t protected

[2025-09-19 18:32] nu11sec: [replying to GG: "do you guys know how to reset these ida graph link..."]
Right click and select reset graph layout I believe not sure

[2025-09-19 18:33] GG: [replying to nu11sec: "Right click and select reset graph layout I believ..."]
I wish it was that easy

[2025-09-19 18:34] GG: why every feature in ida seems to need to a phd to know where it is

[2025-09-19 19:12] Brit: It literally is that easy

[2025-09-19 19:12] Brit: Resetting the graph layout resets the node positions too

[2025-09-20 20:02] the horse: anyone taken a look at it?
[Attachments: image.png]

[2025-09-21 02:09] 0xf444: [replying to GG: "why every feature in ida seems to need to a phd to..."]
Unraveling the features of IDA would be a great book imo lol

[2025-09-21 10:27] elias: whats your opinion on radare2?

[2025-09-21 10:55] Brit: <:mmmm:904523247205351454>

[2025-09-21 17:55] Mintia: Does anyone know why `MmMapIoSpace` could return NULL when i give it a physical address that i get from `HARDWARE\\RESOURCEMAP\\System Resources\\Physical Memory`?

[2025-09-21 18:34] zeropio: [replying to elias: "whats your opinion on radare2?"]
their binaries are cool for specific tasks

[2025-09-21 18:34] zeropio: but i havent used r2 for anything serious, cool for ctfs tho

[2025-09-21 23:36] the horse: because this is used to disassemble a buffer

[2025-09-21 23:36] the horse: change SIZE_MAX to sizeof(ZydisDecodedInstruction)

[2025-09-21 23:37] the horse: you could heap allocate instruction to 100 * sizeof(ZydisDecodedInstruction) and hold 100 instructions that way

[2025-09-21 23:37] the horse: and operands same way

[2025-09-21 23:37] the horse: in your sample you should use     ZydisDecoderDecodeInstruction

[2025-09-21 23:55] abu: I think somethings wrong with my build lol

[2025-09-21 23:55] the horse: is that 2 instructions or just 1 instruction

[2025-09-21 23:55] the horse: wait yeah

[2025-09-21 23:55] the horse: im retarded

[2025-09-21 23:56] the horse: let me just send you a wrapper

[2025-09-21 23:56] snowua: [replying to the horse: "change SIZE_MAX to sizeof(ZydisDecodedInstruction)"]
It shouldn't be

> The length of the input buffer. Note that this can be bigger than the actual size of the instruction – you don't have to know the size up front. This length is merely used to prevent Zydis from doing out-of-bounds reads on your buffer.

[2025-09-21 23:56] the horse: i have zydispp, capstonepp, icedpp

[2025-09-21 23:56] the horse: pick one

[2025-09-21 23:57] the horse: just use the one in vcpkg

[2025-09-21 23:57] the horse: 
[Attachments: message.txt]

[2025-09-21 23:57] the horse: here

[2025-09-21 23:57] snowua: The issue is you have an instruction which is 5 bytes but youre telling zydis its much larger than that

[2025-09-21 23:57] snowua: [replying to the horse: "change SIZE_MAX to sizeof(ZydisDecodedInstruction)"]
This is also not correct. Its the size of the input buffer

[2025-09-21 23:59] the horse: init:
```
zydis_wrapper::Decoder decoder ( section.data.data ( ), section.data.size ( ), section.base_addr );
decoder.set_ip ( start_addr );

          while ( decoder.can_decode ( ) ) {
            //_mm_prefetch ( reinterpret_cast< const char* >( pe_parser_->buffer_.data() + pe_parser_->rva_to_offset ( decoder.ip ( ) - section.base_addr ) + 64 ), _MM_HINT_T0 );
            uint64_t ip = decoder.ip ( );
            auto instr = decoder.decode ( );
            if ( !instr.is_valid ( ) || is_padding_or_data ( decoder, section ) ) {
              if ( !current_block.instructions.empty ( ) ) {
                current_block.end_addr = current_block.instructions.back ( ).ip ( );
                current_block.size = current_block.end_addr - current_block.start_addr;
                current_func.blocks.push_back ( std::move ( current_block ) );
              }
              break;
            }
}
```

[2025-09-21 23:59] the horse: blablabla