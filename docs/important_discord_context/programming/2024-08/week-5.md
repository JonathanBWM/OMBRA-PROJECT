# August 2024 - Week 5
# Channel: #programming
# Messages: 420

[2024-08-26 01:34] Deleted User: https://github.com/Azvanzed/efi-runner
[Embed: GitHub - Azvanzed/efi-runner: A efi-runner and message logger for v...]
A efi-runner and message logger for vmware. Contribute to Azvanzed/efi-runner development by creating an account on GitHub.

[2024-08-26 01:38] Deleted User: if anyone is in efi dev maybe this will help ^^

[2024-08-26 11:42] abu: Ooo

[2024-08-26 23:38] elias: Is it a requirement for a PE that the .text is the first section in virtual memory?

[2024-08-26 23:42] diversenok: Nope

[2024-08-26 23:43] diversenok: And you don't have to call it .text

[2024-08-26 23:43] diversenok: Or have it at all

[2024-08-26 23:44] 5pider: [replying to elias: "Is it a requirement for a PE that the .text is the..."]
Why should it ? There are a lot of "PE" files that are more or less just .text less and just server as holding on resources or even data related to api sets

[2024-08-26 23:44] 5pider: As far as I even remember

[2024-08-26 23:44] 5pider: but I remember that duncan pointed me to some "dlls" that just kept data where some functions are located and do not contain any kind of code at all

[2024-08-26 23:44] elias: For some reason the loader is refusing to load this PE telling me its not a valid win32 app 🤔
[Attachments: image.png]

[2024-08-26 23:45] diversenok: [replying to 5pider: "but I remember that duncan pointed me to some "dll..."]
`.mui` resources?

[2024-08-26 23:45] mrexodia: [replying to elias: "For some reason the loader is refusing to load thi..."]
probably your file alignment or section alignment is wrong

[2024-08-26 23:45] mrexodia: [replying to diversenok: "`.mui` resources?"]
I think it was the `api-ms` dlls

[2024-08-26 23:45] 5pider: [replying to diversenok: "`.mui` resources?"]
na na. I remember seeing .dll files that are just loaded to get the .rsrc section and doesn't contain any code

[2024-08-26 23:45] mrexodia: they just contain forwarders for everything and don't have an entry point

[2024-08-26 23:46] mrexodia: [replying to 5pider: "na na. I remember seeing .dll files that are just ..."]
those are .mui resources <:kappa:697728545631371294>

[2024-08-26 23:46] 5pider: [replying to mrexodia: "I think it was the `api-ms` dlls"]
yes I think that was the case

[2024-08-26 23:46] 5pider: [replying to mrexodia: "those are .mui resources <:kappa:69772854563137129..."]
AHHH

[2024-08-26 23:46] 5pider: Yeah then I was thinking of api-ms files

[2024-08-26 23:46] 5pider: Got that mixed up

[2024-08-26 23:48] diversenok: Yeah, ApiSet stubs also

[2024-08-26 23:48] diversenok: But .mui DLLs are funny as they always target 32-bit machines

[2024-08-26 23:49] diversenok: One of the examples where a 64-bit program can use `LoadLibrary` on technically 32-bit DLLs

[2024-08-26 23:49] BWA RBX: [replying to elias: "Is it a requirement for a PE that the .text is the..."]
No and if you've looked at any obfuscators like VMProtect you can see that it is not necessary

[2024-08-26 23:52] 5pider: Are you a drunken Irish guy ??

[2024-08-26 23:52] BWA RBX: [replying to 5pider: "Are you a drunken Irish guy ??"]
Yes sir

[2024-08-26 23:52] 5pider: cheers mate

[2024-08-26 23:53] BWA RBX: But you don't have to conform to a standard, the .text header does not always have to be first section that doesn't contain code

[2024-08-26 23:55] 5pider: I think a good example would be that ntoskrnl. It's .text section is not first ? I am on my phone right now so I cant check it but I remember they changed it to mitigate shellcode, exploits, etc. By placing bogus MZ headers in-between the text section and the real header to confuse shellcode that iterate over the memory from the .text back to not header

[2024-08-26 23:57] elias: I have no clue why it doesnt load then

[2024-08-27 00:00] elias: is there a way to get more detailed error information from the loader?

[2024-08-27 00:04] diversenok: Not that I know of; I'd try to make a minimal reproducible example

[2024-08-27 00:05] diversenok: It doesn't have to work, as in, do anything

[2024-08-27 00:05] diversenok: I assume it fails on creating the image section?

[2024-08-27 00:07] elias: I think so, although windbg just tells me `%1 is not a valid Win32 application.`

[2024-08-27 00:11] diversenok: Well, clear the section table to see if it's an issue in the header I guess

[2024-08-27 00:15] BWA RBX: [replying to elias: "I think so, although windbg just tells me `%1 is n..."]
Was gonna say what's happening when you dynamically analyze the PE, have you tried creating a memory dump of it before execution?

[2024-08-27 00:16] BWA RBX: There is probably far more experienced people here but I am just providing ideas or somethings that I would do to try and solve the problem

[2024-08-27 00:30] BWA RBX: Was gonna say that whilst I use a debugger, I try my best to try and look at everything before moving on to memory inspection and then dumping, I haven't got enough dynamic analysis knowledge yet, but for now, I strictly use static analysis. I know that veterans in this sort of shit use dynamic analysis for their own reasons and maybe comply with a methodology, but me personally, rather than handling the sample as quickly as possible through analysis I would rather understand it thoroughly, that's just my 2 cents as a hobbyist, I know the value each has and know dynamic analysis is mostly used by most malware researchers.

[2024-08-27 00:31] BWA RBX: Trying not to butcher that reply cause I am drunk

[2024-08-27 00:43] elias: gonna keep trying tomorrow

[2024-08-27 00:44] elias: after leaving the original .text in the file and insert a new section at the end it works

[2024-08-27 00:52] x86matthew: [replying to elias: "For some reason the loader is refusing to load thi..."]
it's because your sections aren't ordered correctly

[2024-08-27 00:52] x86matthew: they need to be ordered by virtual address

[2024-08-27 00:53] elias: [replying to x86matthew: "it's because your sections aren't ordered correctl..."]
okay, but I also tried putting .text as the last section header and it still wouldn‘t load

[2024-08-27 00:56] x86matthew: why is the first section at 0x2000

[2024-08-27 00:56] x86matthew: have you updated SizeOfHeaders to match this?

[2024-08-27 00:57] x86matthew: my guess is that you've been moving things around and haven't updated SizeOfHeaders to fill the gap

[2024-08-27 00:59] elias: SizeOfHeaders is 0x400

[2024-08-27 01:00] elias: .rdata starts at 0x400

[2024-08-27 01:00] x86matthew: .rdata starts at 0x2000 in your screenshot

[2024-08-27 01:00] x86matthew: it's the virtual address that matters here

[2024-08-27 01:01] elias: <:peepoDetective:570300270089732096>

[2024-08-27 01:01] elias: normally SizeOfHeaders is 0x400 and .text starts at 0x1000 so there would still be a gap

[2024-08-27 01:02] elias: is it a problem if the gap is larger than a page?

[2024-08-27 01:02] x86matthew: that doesn't matter because it's rounded up

[2024-08-27 01:02] x86matthew: you've got a page missing

[2024-08-27 01:02] elias: oooh see

[2024-08-27 01:03] elias: now it makes sense

[2024-08-27 01:03] elias: although I dont understand why the loader considers it a problem if there is a 1 page gap in between

[2024-08-27 01:04] elias: thank you a lot!

[2024-08-27 01:05] diversenok: Actually, is it possible to create a PE that would have a gap in its mapped state? 🤔

[2024-08-27 01:06] elias: apparently not

[2024-08-27 01:07] x86matthew: [replying to diversenok: "Actually, is it possible to create a PE that would..."]
no, gaps aren't allowed between sections afaik

[2024-08-27 01:07] BWA RBX: [replying to diversenok: "Actually, is it possible to create a PE that would..."]
Wouldn't it be possible if you calculated everything correctly or are gaps just non-deterministic here and can't be used

[2024-08-27 01:07] BWA RBX: If it's a case where you know the gap size?

[2024-08-27 01:09] x86matthew: [replying to elias: "now it makes sense"]
to be honest, updating SizeOfHeaders is almost certainly the wrong fix in your case (i'm guessing you've rewritten .text to the end of the binary or something)

[2024-08-27 01:09] x86matthew: but this is the probably cause of the error, and updating the value would "fix" it

[2024-08-27 01:11] elias: yeah Im considering if I should insert a dummy section there or make the following section (.rdata) larger and reduce its Va to cover the previous .text space as well

[2024-08-27 01:11] elias: because yeah I took out the .text and inserted it again at the end

[2024-08-27 01:12] BWA RBX: <@943099229126144030> Why can't someone use a gap region here, say like in a structure where the accesses of a member have a gap why isn't it possibly here to have your own allocations and gap each member why would it mess up the program I am just interested to know why, is the header a set size and we are limited to that or what or is a standard that we have to conform to

[2024-08-27 01:14] diversenok: I'm not sure we are talking about the same gaps at this point

[2024-08-27 01:14] diversenok: I meant having inaccessible pages between sections

[2024-08-27 01:14] BWA RBX: Isn't it in IDA a structure for PE headers

[2024-08-27 01:15] BWA RBX: But also in ida in some structures there are gaps

[2024-08-27 01:15] elias: I think now you live up to your name 😭

[2024-08-27 01:16] elias: [replying to BWA RBX: "But also in ida in some structures there are gaps"]
we are talking about gaps between PE sections, not gaps in the Pe header structures

[2024-08-27 01:20] BWA RBX: [replying to elias: "we are talking about gaps between PE sections, not..."]
I thought you meant implementing a gap in the structures  (Not effecting the members) in IDA Pro of the headers like_IMAGE_DOS_HEADER etc sorry I am drunk must be reading incorrectly

[2024-08-27 01:21] BWA RBX: Not a gap between each section of the header

[2024-08-27 01:21] diversenok: A section of the header?

[2024-08-27 01:26] BWA RBX: [replying to diversenok: "A section of the header?"]
Yes, a section of the header in the PE file

[2024-08-27 01:28] diversenok: Not sure if you mean the section table that describes PE section layout (i.e., how to transform the file between the on-disk and in-memory layout) or something else

[2024-08-27 01:29] BWA RBX: If you'd like further information I mean as an example
.text section contains E.C
.data section contains I.D
.rdata section contains R.O.D

[2024-08-27 01:30] BWA RBX: So each of these

[2024-08-27 01:31] diversenok: Yeah, okay, so there is this table itself (in the headers) and the locations where it points (outside of the header)

[2024-08-27 01:31] diversenok: I guess a question of whether you can put gaps in between is valid for both

[2024-08-27 01:31] BWA RBX: So I was thinking bro that these can't have gaps and have to be specific not related to structures but offsets maybe

[2024-08-27 01:32] BWA RBX: Maybe some alignment shit or something if you where doing some whacky shit would it be possible to add gaps between these sections

[2024-08-27 01:34] BWA RBX: But why would you even want to add a gap to these sections, it wouldn't really matter wouldn't it not?

[2024-08-27 01:34] BWA RBX: .text
gap
.data
gap
.rdata

[2024-08-27 01:35] BWA RBX: I don't see the reasoning

[2024-08-27 01:35] x86matthew: to answer the original question, i'm pretty sure the loader just validates the virtual address of each section in sequence, eg something like:
`if(curr_section->virtualaddress != (last_section->virtualaddress + last_section->size)) return invalid_image;`

[2024-08-27 01:36] x86matthew: with alignment taken into account of course

[2024-08-27 01:37] diversenok: Well, I was asking from the perspective of dumping: i.e., if I have a valid PE mapped as image, could I encounter free or reserved pages within the image limits

[2024-08-27 01:39] diversenok: I guess not

[2024-08-27 01:40] diversenok: Inaccessible due to `PAGE_GUARD` or `PAGE_NOACCESS`... sure

[2024-08-27 01:42] BWA RBX: [replying to x86matthew: "with alignment taken into account of course"]
Just add your logic in this if statement related to the alignment right?

[2024-08-27 01:42] BWA RBX: He'd have to create a separate function that will validate the alignment of the sections?

[2024-08-27 01:43] diversenok: He means that the system probably does that when it tried to load and map a PE file

[2024-08-27 01:44] BWA RBX: I am sure that he would have to include that to validate alignment , but I am skid so idk I am learning from you guys

[2024-08-27 03:00] dinero: plz stop insulting yourself for not fully understanding a concept, asking for help, confirming/denying intuition, etc

[2024-08-27 03:03] dinero: bad habit to build sir

[2024-08-27 03:06] Deleted User: Good advice

[2024-08-27 16:51] twopic: Has anyone here taken a data structures class?

[2024-08-27 16:51] twopic: Im going to be taking one for java

[2024-08-27 17:17] emma: [replying to twopic: "Has anyone here taken a data structures class?"]
yes, lol

[2024-08-27 18:05] Azrael: [replying to twopic: "Has anyone here taken a data structures class?"]
Not yet, in a couple of years probably.

[2024-08-27 18:08] snowua: Too difficult I dropped out

[2024-08-27 18:08] Brit: yikes

[2024-08-27 18:08] snowua: JOKING

[2024-08-27 18:09] snowua: cmon brit 🙄

[2024-08-27 18:09] Brit: I can never tell, you might actually pull smth like that

[2024-08-27 18:09] snowua: <:brainrot:1271606731247456306>

[2024-08-27 18:13] Timmy: people are scary, I never went

[2024-08-27 18:16] brymko: [replying to snowua: "Too difficult I dropped out"]
too easy dropped in

[2024-08-27 18:17] brymko: isn't DS all meme education anyways

[2024-08-27 18:17] brymko: like oooh linked list

[2024-08-27 18:24] elias: usually ds classes are combined with algorithms

[2024-08-27 18:25] elias: stuff like implementing queues can be challenging for cs beginners tho

[2024-08-27 18:26] Deleted User: [replying to brymko: "isn't DS all meme education anyways"]
No?

[2024-08-27 18:27] brymko: maybe im being elitist

[2024-08-27 18:27] Deleted User: Most of it is cringe stupid useless shit, but some of it is actually useful and practical

[2024-08-27 18:28] Deleted User: I've never once in my life used or had to use a multimap or multiset

[2024-08-27 18:28] Deleted User: I find that you should learn DSA in a more ad-hoc manner, rather than learning it all at once. You should learn some algorithms as needed because most likely their purpose is just being an elegant solutions for a very specific problem

[2024-08-27 18:29] Deleted User: For example, "kadanes algorithm" is utterly useless in 99.8% of programming, but if for some reason you had to find the maximum sum / product of a subarray, then it's a very elegant solution

[2024-08-27 19:30] Torph: [replying to brymko: "like oooh linked list"]
lol that's what it was in my intro CS class, but that was just intro not full class on DS. the dedicated DS classes probably have more complex stuff

[2024-08-27 19:33] contificate: [replying to Deleted User: "I find that you should learn DSA in a more ad-hoc ..."]
I prefer the other way - you learn some algorithms just because, then see how your lateral thinking has changed a few weeks later

[2024-08-27 19:58] szczcur: [replying to Deleted User: "I've never once in my life used or had to use a mu..."]
must be nice.. i’ve noticed a lot of personal incredulity when these discussions crop up. there are different problem domains that having even a rudimentary recollection of ds&a will benefit you. the use cases aren’t hyperspecialized either. point being ds&a courses are generally useful.. knowing what data structure offers what benefit will make for much more efficient problem solving.

[2024-08-27 19:59] Brit: what do you mean, everything goes into a std::vector

[2024-08-27 20:01] szczcur: [replying to Deleted User: "For example, "kadanes algorithm" is utterly useles..."]
99.8% of programming is outside of signal processing / computer vision.. hft? don’t think i agree with this

[2024-08-27 20:06] szczcur: [replying to Brit: "what do you mean, everything goes into a std::vect..."]
of course i forgot

[2024-08-27 20:11] Deleted User: [replying to contificate: "I prefer the other way - you learn some algorithms..."]
Meh, most of the time you end up storing a ton of useless information in brain about edge case algorithms of very specific problems that rarely pop up in practice

[2024-08-27 20:12] Deleted User: [replying to szczcur: "99.8% of programming is outside of signal processi..."]
kadanes algorithm is commonly used there?

[2024-08-27 20:12] Deleted User: maybe kadanes algorithm was a bad example, because I have looked at nothing regarding signal processing or computer vision

[2024-08-27 20:13] contificate: [replying to Deleted User: "Meh, most of the time you end up storing a ton of ..."]
not in my experience, there are lots of very useful algorithms to know that don't have this property

[2024-08-27 20:13] Deleted User: and hft somewhat makes sense, but I feel as if most hft is training models to recognize strategies

[2024-08-27 20:13] Deleted User: [replying to contificate: "not in my experience, there are lots of very usefu..."]
but they are significantly more that do

[2024-08-27 20:13] contificate: sounds like leetcode algorithms

[2024-08-27 20:13] Deleted User: yeah

[2024-08-27 20:14] Deleted User: that's mainly what I'm talking about

[2024-08-27 20:14] Deleted User: maybe dsa classes are more pragmatic

[2024-08-27 20:14] contificate: waste of time

[2024-08-27 20:14] Deleted User: yeah

[2024-08-27 20:14] Deleted User: I think learning your trees, graphs, hashmaps, linked lists, and time complexity stuff is quite useful

[2024-08-27 20:14] Deleted User: but like really specific and specialized algorithms and data structures (not necessarily my example data structures from earlier) is just a pure waste of time

[2024-08-27 20:15] Deleted User: and when I think of DSA courses I think of that

[2024-08-27 20:15] Deleted User: I think of leetcode grinding

[2024-08-27 20:15] Deleted User: but now that I think about it, it's probably more oriented towards uni classes

[2024-08-27 20:15] contificate: union-find, tries, balancing trees, aho-corasick, etc. are what it's all about

[2024-08-27 20:16] Deleted User: Meh, just use a hashmap for everything and 95% of the time you'll be alright /s

[2024-08-27 20:16] Deleted User: [replying to contificate: "union-find, tries, balancing trees, aho-corasick, ..."]
Mmmh, I've seen practical industrial examples of all of those but not really union-find, where might you find union-find a lot at?

[2024-08-27 20:17] Deleted User: Union-find, that's the disjoint set data structure, right?

[2024-08-27 20:17] Deleted User: If it is I have seen disjoint sets used quite often for graph-related work

[2024-08-27 20:18] Deleted User: like finding minimal spanning trees, pretty sure that uses disjoint sets

[2024-08-27 20:20] contificate: kruskal's algorithm uses it for MSTs, yes

[2024-08-27 20:20] Deleted User: and I know that spanning trees are quite useful for network topology

[2024-08-27 20:20] contificate: but more generally

[2024-08-27 20:20] Deleted User: [replying to contificate: "kruskal's algorithm uses it for MSTs, yes"]
thought so

[2024-08-27 20:20] contificate: it is the core of equality solvers

[2024-08-27 20:20] Deleted User: Oooh, do elaborate

[2024-08-27 20:20] contificate: like if you were given a bunch of constraints

[2024-08-27 20:20] contificate: `a = b`, `b = c`, `d = a`

[2024-08-27 20:20] contificate: and uh

[2024-08-27 20:20] contificate: `e = f`

[2024-08-27 20:20] contificate: you could determine `a != f`

[2024-08-27 20:21] contificate: so you might think that's a bit contrived

[2024-08-27 20:21] contificate: but algorithms for type inference and logic programming implementations

[2024-08-27 20:21] contificate: are based on unification

[2024-08-27 20:21] contificate: which is effectively equality constraints among subterms

[2024-08-27 20:21] contificate: and is implemented commonly using union-find

[2024-08-27 20:22] Deleted User: [replying to contificate: "so you might think that's a bit contrived"]
Is it used for proof assistance languages as well?

[2024-08-27 20:22] contificate: yeah

[2024-08-27 20:22] Deleted User: Neat neat

[2024-08-27 20:22] Deleted User: Learned a new thing today

[2024-08-27 20:23] contificate: 
[Attachments: 1jyzg8_2.mp4]

[2024-08-27 20:23] contificate: like in this

[2024-08-27 20:24] contificate: the rewriting of type terms in-place is done destructively with union-find based unification

[2024-08-27 20:24] contificate: as opposed to unification that reified substitutions

[2024-08-27 20:24] Deleted User: <@839216728008687666> you sly dog

[2024-08-27 20:24] snowua: misclick

[2024-08-27 20:24] Deleted User: [replying to contificate: "the rewriting of type terms in-place is done destr..."]
neat neat

[2024-08-27 20:25] Deleted User: [replying to snowua: "misclick"]
std::launder and std::get_money I fucking hate you

[2024-08-27 20:31] szczcur: [replying to Deleted User: "but now that I think about it, it's probably more ..."]
what? ds&a is a uni course..

[2024-08-27 20:31] Deleted User: contificate have you seen additional usage to dominator trees besides optimizing compilers and maybe other control flow analysis? I saw your LLVM dominance frontier comment on stack overflow, and was curious about it since im trying to learn more about compiler stuff lol

[2024-08-27 20:32] Deleted User: [replying to szczcur: "what? ds&a is a uni course.."]
yeah yeah, but idk why but I was thinking of like leetcode dsa courses that are sold

[2024-08-27 20:32] Deleted User: that's my bad

[2024-08-27 20:33] Deleted User: but I guess my point also could be reflected upon depending on the contents of the dsa course

[2024-08-27 20:33] Deleted User: some might be more impractical than others, but idk I haven't seen every dsa uni course in the world

[2024-08-27 20:33] daax: <@1033421942910369823> dont get dragged in

[2024-08-27 20:33] daax: <:kekW:626450502279888906>

[2024-08-27 20:35] Deleted User: [replying to Deleted User: "contificate have you seen additional usage to domi..."]
That's actually a good question, I'm curious to know as well. I've only ever seen them used for CFF deobfuscation, SSA construction, and I think like memory leak analysis (Java eclipse editor)

[2024-08-27 20:35] Deleted User: Yeah I also saw it for CFF and SSA construction, it's genuinely based work

[2024-08-27 20:35] Deleted User: Ahh, nvm it was a "memory analyzer"

[2024-08-27 20:37] Deleted User: 
[Attachments: image.png]

[2024-08-27 20:37] Deleted User: welp

[2024-08-27 20:37] contificate: it has uses in circuit design and profilers

[2024-08-27 20:38] Deleted User: oh that's cool as fuck, do you have any literature on it

[2024-08-27 20:38] contificate: well I mean it's just well known that

[2024-08-27 20:38] contificate: if you have a lot of live things in your heap

[2024-08-27 20:38] contificate: it could be that one reference dominates many

[2024-08-27 20:38] contificate: and is keeping things alive artificially

[2024-08-27 20:39] contificate: > Dominators provide information about the origin and the end of reconverging paths in a graph. This information is widely used in CAD applications such as satisfiability checking, equivalence checking, ATPG, technology mapping, decomposition of Boolean functions and power optimization

[2024-08-27 20:41] contificate: but yeah it's mostly just anything that can be formulated as a graph

[2024-08-27 20:41] contificate: I used it for placing nested functions a while back

[2024-08-27 20:41] contificate: but that's more CFG-like fuckery

[2024-08-27 20:43] contificate: dominators are one of those things where

[2024-08-27 20:43] contificate: you have an extremely involved algo that's quite complex, requires many proofs to justify

[2024-08-27 20:43] contificate: then you have a small adjustment to the simplest fixpoint algorithm

[2024-08-27 20:43] contificate: an engineering trick

[2024-08-27 20:43] contificate: and it outperforms the other in practice

[2024-08-27 20:43] contificate: due to constant factors

[2024-08-27 20:43] contificate: in many impls

[2024-08-27 20:51] szczcur: [replying to Deleted User: "some might be more impractical than others, but id..."]
sure alr.. there's a lot of hedging here now as opposed to your initial statement.. could you be more specific about the conditions under which you believe the content of the course would be useless to the majority of the industry? if that's still your opinion.

[2024-08-27 20:54] Deleted User: Really any sort of relation between the aforementioned course and something like a leetcode DSA course that is just riddled with rather specific data structures & algorithms

[2024-08-27 20:55] Deleted User: As I said earlier, as long as it includes your basic primer for DSA then that's fine

[2024-08-27 20:55] Deleted User: Your linked lists, trees, graphs, hashmaps, stack, queue, big O, and whatever else I'm missing

[2024-08-27 20:55] Deleted User: which I think mostly all DSA courses do

[2024-08-27 20:55] Deleted User: but idk for sure

[2024-08-27 20:56] szczcur: [replying to Deleted User: "Really any sort of relation between the aforementi..."]
i'm not familiar with the leetcode dsa stuff. what do they even cover?

[2024-08-27 20:56] Deleted User: [replying to szczcur: "i'm not familiar with the leetcode dsa stuff. what..."]
a lot of useless shit that you never find in practice

[2024-08-27 20:56] contificate: my biggest gripe with many algo courses is they kind of teach recursion like it's some mechanical trick you can do sometimes, rather than the natural and obvious way of expressing many solutions to problems over inductive datatypes

[2024-08-27 20:56] szczcur: i guess im asking what specific ds&a do they cover

[2024-08-27 20:56] Deleted User: it's almost like "sport"

[2024-08-27 20:56] szczcur: [replying to Deleted User: "a lot of useless shit that you never find in pract..."]
like fucking what mate

[2024-08-27 20:56] szczcur: spell it out. i dont know what these courses are lol

[2024-08-27 20:56] Deleted User: I can't think of anything off of the top of my head, but I could go look at a course syllabus

[2024-08-27 20:56] contificate: if you want to know a good leetcode problem that exemplifies the issue

[2024-08-27 20:57] contificate: subtree of another tree

[2024-08-27 20:57] contificate: this is trivial to do poorly

[2024-08-27 20:57] contificate: doing it well is the subject of research papers from the 80s

[2024-08-27 20:57] contificate: so like what have you really done except shown that you can enumate all possible trial and error cases

[2024-08-27 20:58] contificate: https://leetcode.com/problems/subtree-of-another-tree/description/

[2024-08-27 20:58] contificate: leetcode easy

[2024-08-27 20:58] contificate: and yet it's actually a fun problem with lots and lots of literature

[2024-08-27 20:59] Deleted User: [replying to szczcur: "like fucking what mate"]
I still think my kadanes algorithm example was quite good, when I was learning about it I seen no mention of it ever used in practice

[2024-08-27 20:59] Deleted User: You said signal processing and computer vision, can you tell me what codebases use kadanes algorithm?

[2024-08-27 21:00] Deleted User: I don't know much about either field, so it's feasible they do use it but I don't recall those ever being mentioned when I was learning about it

[2024-08-27 21:00] Deleted User: Albeit, I looked at it like 2 years ago

[2024-08-27 21:00] contificate: https://grep.app/search?q=kadanes - find one example that's not a repo containing random algo implementations for sport

[2024-08-27 21:01] Deleted User: this is a fuckin goated website, never seen this before

[2024-08-27 21:02] Deleted User: tbf, most of the codebases that *would* use it are probably not OSS

[2024-08-27 21:02] Deleted User: but idk <:shrug:332268181517238272>

[2024-08-27 21:02] contificate: yeah, grep.app is great, you find out that everyone just copies from papers directly

[2024-08-27 21:02] Deleted User: most of the time that's all they really need to do tbh

[2024-08-27 21:03] szczcur: [replying to contificate: "https://grep.app/search?q=kadanes - find one examp..."]
have you either of you worked on anything involving dsp? the use case for things doesn't require it to be in open source for it to be valid. this is availability bias at its finest. i don't know of an open source use case, but i do know where it is and has been/would be used that i mentioned earlier.

[2024-08-27 21:03] Deleted User: but a really cool website, gonna bookmark that

[2024-08-27 21:03] szczcur: [replying to contificate: "https://leetcode.com/problems/subtree-of-another-t..."]
neat, never looked at leetcode tbh. what's wrong with this in particular?

[2024-08-27 21:03] Deleted User: [replying to szczcur: "have you either of you worked on anything involvin..."]
No, I haven't worked with DSP before

[2024-08-27 21:03] szczcur: [replying to Deleted User: "No, I haven't worked with DSP before"]
Yes I know

[2024-08-27 21:03] contificate: [replying to szczcur: "neat, never looked at leetcode tbh. what's wrong w..."]
the reason I explained

[2024-08-27 21:03] snowua: Its not about memorizing the algorithm, its about being able to problem solve. Sure you can point at an algorithm and say its not common therefore useless. But its not always about that

[2024-08-27 21:04] contificate: it doesn't want a good solution in this case

[2024-08-27 21:04] contificate: this is like a good general problem with many cool approaches

[2024-08-27 21:04] contificate: what it wants is a degenerate solution

[2024-08-27 21:04] contificate: and then the complex problems mostly require tricks

[2024-08-27 21:04] contificate: instead of actual good theoretical foundations

[2024-08-27 21:04] Deleted User: [replying to snowua: "Its not about memorizing the algorithm, its about ..."]
but it becomes a problem whenever you're spending insane amounts of time trying to memorize every little case of when to use things when in reality, it doesn't matter

[2024-08-27 21:04] Deleted User: I think that's the main thing I'm getting at, it doesn't matter

[2024-08-27 21:05] contificate: DSP = digital signals processing?

[2024-08-27 21:05] Deleted User: and then people miss things that actually matter

[2024-08-27 21:05] Deleted User: [replying to contificate: "DSP = digital signals processing?"]
yeah

[2024-08-27 21:05] contificate: what's the relevance here

[2024-08-27 21:05] emma: min cut/max flow go crazy

[2024-08-27 21:05] Deleted User: I guess my example of kadanes algorithm is useful in DSP

[2024-08-27 21:05] contificate: a lot of those people seem to lower problems to like matrix mult and fourier transforms

[2024-08-27 21:05] contificate: and do it in verilog

[2024-08-27 21:05] contificate: so it's harder to grep for

[2024-08-27 21:05] szczcur: [replying to contificate: "instead of actual good theoretical foundations"]
ah ok, in my head i was thinking yeah subtree of tree .. probably good teacher for traversing tree, recursion, maybe ideas of how filesystems are implemented conceptually

[2024-08-27 21:05] contificate: file systems are another great case where you'd want B trees

[2024-08-27 21:06] Deleted User: [replying to Deleted User: "and then people miss things that actually matter"]
You have these CP / leetcode warriors who spend months and months learning and memorizing these quixotic problems and their solutions instead of familiarizing themselves with useful toolchains, practices, and work flows

[2024-08-27 21:07] Deleted User: that's why I think algorithmic interviews are slightly stupid, but only whenever they're done poorly

[2024-08-27 21:07] contificate: not that I'm some industrial expert but

[2024-08-27 21:07] contificate: a lot of what's really useful in industry

[2024-08-27 21:07] contificate: just comes down to knowing your tool well

[2024-08-27 21:07] Deleted User: right

[2024-08-27 21:07] contificate: like I've been forced to become a master at git

[2024-08-27 21:07] contificate: whereas before I used maybe 6 commands

[2024-08-27 21:07] Deleted User: there's bad time complexity code all over google codebases and shit

[2024-08-27 21:07] contificate: now I can just create and commit into repos without using `git` directly

[2024-08-27 21:07] contificate: y'know

[2024-08-27 21:08] szczcur: [replying to Deleted User: "You have these CP / leetcode warriors who spend mo..."]
fair enough. i dont know how leetcode works or what their courses teach so i'll take your word for it

[2024-08-27 21:08] contificate: bad time complexity is to be expected, `n` is usually too small to care

[2024-08-27 21:08] Deleted User: CP / leetcode warriors also seem to be negligent to other code factors outside of speed and memory footprint

[2024-08-27 21:08] contificate: pretty sure many standard libs will use quicksort but use insertion sort if they determine `n` is below some threshold

[2024-08-27 21:08] Deleted User: [replying to contificate: "bad time complexity is to be expected, `n` is usua..."]
yeah, exactly

[2024-08-27 21:08] contificate: I'm one of the people who thinks you should accept upfront cost to enforce a policy you want

[2024-08-27 21:08] Deleted User: [replying to contificate: "pretty sure many standard libs will use quicksort ..."]
most sorting implementations in libraries are hybrid sorts

[2024-08-27 21:09] Deleted User: python uses timsort, which uses different sorting algorithms dependent upon how big the data set is

[2024-08-27 21:09] contificate: exactly, they adapt

[2024-08-27 21:09] Deleted User: C++ uses fuckin "hybrid sorting algorithm"

[2024-08-27 21:09] contificate: we don't talk of C++ in this channel

[2024-08-27 21:09] Brit: sure we do, all useful programing is done in C and it's variants

[2024-08-27 21:10] Brit: everything else is toylangs

[2024-08-27 21:10] Brit: and cope

[2024-08-27 21:10] Deleted User: introsort, that's what it's called

[2024-08-27 21:10] snowua: only useless things are written in rust

[2024-08-27 21:10] Brit: ++

[2024-08-27 21:10] Deleted User: [replying to snowua: "only useless things are written in rust"]
real and true

[2024-08-27 21:10] Deleted User: [replying to snowua: "only useless things are written in rust"]
what!

[2024-08-27 21:10] Deleted User: I love rust

[2024-08-27 21:10] Deleted User: starting to kinda hate it because I'm spamming ``Arc<T>``'s everywhere

[2024-08-27 21:10] Deleted User: probably due to my own incompetence instead of rust being a problem

[2024-08-27 21:11] Brit: .clone()

[2024-08-27 21:11] contificate: you say this but the argument has already been destroyed

[2024-08-27 21:12] Deleted User: [replying to Brit: ".clone()"]
async rust + ``Arc::Clone`` is cheaper <:gigachad:904523979249815573>

[2024-08-27 21:12] szczcur: [replying to contificate: "bad time complexity is to be expected, `n` is usua..."]
true, worth saying though is that bad time complexity can make a world of difference depending on industrial sector.. think automotive or avionics.. radar or telecomm. huge segments of the industry that need to pay attention to these things. not arguing that for most in here i'd imagine itss not a problem, and if it is a problem id hope they would optimize their approach lol

[2024-08-27 21:12] contificate: you would rather be explicit about copying than implicit, so I'd rather the default was to move and clones were explicit

[2024-08-27 21:12] contificate: so clone() everywhere is fine

[2024-08-27 21:12] Deleted User: [replying to contificate: "so clone() everywhere is fine"]
fine in the sense of semantics, but it does get kinda annoying when writing wicked ass UFCS chains

[2024-08-27 21:13] contificate: yeah but you don't prematurely optimise, szc

[2024-08-27 21:13] contificate: you have to have a realistic expectation of `n`

[2024-08-27 21:13] contificate: there are many corners you could cut to write something slightly faster and it'd just devolve your codebase

[2024-08-27 21:13] Deleted User: ```rust
some_operation() // maybe returns a Result<Result<T0>, T1>
    .unwrap()
    .clone()
    .unwrap()
    .clone()
    .something();
```

[2024-08-27 21:13] contificate: like a common example is favouring an array over a set data structure

[2024-08-27 21:14] Brit: [replying to Deleted User: "```rust
some_operation() // maybe returns a Result..."]
if only there was a way to unwrap a result in a function that may fail

[2024-08-27 21:14] Deleted User: [replying to Brit: "if only there was a way to unwrap a result in a fu..."]
``?`` What are you talking about``?``

[2024-08-27 21:14] contificate: I think so

[2024-08-27 21:15] Brit: sorry phone typing

[2024-08-27 21:16] contificate: we only really program OCaml in here

[2024-08-27 21:16] contificate: no time for other shit

[2024-08-27 21:16] szczcur: [replying to contificate: "yeah but you don't prematurely optimise, szc"]
its not called premature optimization. its necessary optimization.. you cant have something be slow in prototype and then test and have catastrophic results. power efficiency is also something that is "prematurely" done..

[2024-08-27 21:16] contificate: I'm just saying in general

[2024-08-27 21:16] contificate: trust me when I tell you I'm well aware of algorithm impls that are of top concern to many sectors of industry

[2024-08-27 21:16] contificate: I'm just saying that the original point is just

[2024-08-27 21:16] contificate: in most cases `n` is small

[2024-08-27 21:17] contificate: a lot of off the shelf algos are higher in constant factors than simper algos for data of that size

[2024-08-27 21:17] contificate: but we prefer them simply because they are easier to maintain

[2024-08-27 21:17] Deleted User: [replying to contificate: "we only really program OCaml in here"]
fuck those camels

[2024-08-27 21:18] Brit: [replying to Deleted User: "``?`` What are you talking about``?``"]
yes I was talking about ?

[2024-08-27 21:19] Deleted User: Yeah, I was being a little witty

[2024-08-27 21:19] twopic: [replying to Azrael: "Not yet, in a couple of years probably."]
Yeah I'm taking one

[2024-08-27 21:19] twopic: [replying to emma: "yes, lol"]
Oh what's it like?

[2024-08-27 21:20] contificate: get on top of content now and just do princeton's course

[2024-08-27 21:20] contificate: https://algs4.cs.princeton.edu/home/

[2024-08-27 21:20] contificate: this is all Java

[2024-08-27 21:20] contificate: starts with union-find

[2024-08-27 21:20] contificate: that's just how based the ivy leagues are

[2024-08-27 21:20] szczcur: [replying to contificate: "trust me when I tell you I'm well aware of algorit..."]
https://tenor.com/view/dr-fate-source-meme-trust-me-gif-25112135

[2024-08-27 21:21] szczcur: took forever to find the right one

[2024-08-27 21:21] contificate: you're just being very condescending

[2024-08-27 21:21] szczcur: im not at all

[2024-08-27 21:21] Azrael: [replying to twopic: "Yeah I'm taking one"]
Sweet.

[2024-08-27 21:21] szczcur: it wasn't a disagreement i just thought it was a statement that fits the meme

[2024-08-27 21:22] Azrael: [replying to szczcur: "https://tenor.com/view/dr-fate-source-meme-trust-m..."]
https://tenor.com/view/metal-gear-rising-metal-gear-rising-revengeance-senator-armstrong-revengeance-i-made-it-the-fuck-up-gif-25029602

[2024-08-27 21:22] contificate: also I take issue with

[2024-08-27 21:22] contificate: > you cant have something be slow in prototype and then test and have catastrophic results.

[2024-08-27 21:22] contificate: because that is exactly what you do

[2024-08-27 21:22] contificate: this is why we have modular interfaces

[2024-08-27 21:22] contificate: so we can replace implementations easily

[2024-08-27 21:22] contificate: oh, the algo is slow? replace it with a better one

[2024-08-27 21:22] contificate: simpy satisfy this extant interface and the rest of the code won't break assuming you implement the algo correctly

[2024-08-27 21:22] szczcur: [replying to contificate: "because that is exactly what you do"]
do that for a device that is an aggregation of things that cost tens of thousands. i bet youll stay employed in that firm.

[2024-08-27 21:23] contificate: that is how it's done

[2024-08-27 21:23] contificate: do you know how many times the dominator algo has been changed in LLVM

[2024-08-27 21:23] szczcur: but if we want condescending: it gets real old defending positions from real experience to arm chair experts who spin their wheels on theoretical nonsense making proclamations that generalize entire industries they have zero experience in.

[2024-08-27 21:24] contificate: what makes us arm chair experts

[2024-08-27 21:24] szczcur: avoid "prematurely optimizing" for something in any of those sectors and let me know.

[2024-08-27 21:24] contificate: that's not what I said

[2024-08-27 21:24] contificate: strawmanning the point

[2024-08-27 21:24] szczcur: no, you generalized and then said "trust me bro"

[2024-08-27 21:24] contificate: let me rephrase to avoid all doubt

[2024-08-27 21:24] contificate: in general, you don't go down rabbit holes prematurely optimising every little thing

[2024-08-27 21:24] szczcur: after telling me "you dont prematurely optimize for ..."

[2024-08-27 21:24] szczcur: you do.

[2024-08-27 21:24] contificate: there is often an upfront cost you accept

[2024-08-27 21:25] contificate: because the performance of your program is often dominated by something else

[2024-08-27 21:25] contificate: sure, you can have the aggregation of marginal gains

[2024-08-27 21:27] contificate: I wasn't even talking about those industries

[2024-08-27 21:27] contificate: you're engaging in whataboutism

[2024-08-27 21:28] emma: [replying to twopic: "Oh what's it like?"]
not too bad

[2024-08-27 21:28] emma: also took it in java

[2024-08-27 21:28] twopic: I don't know how to code java

[2024-08-27 21:28] contificate: here's a simpler example: many networking programs are not compute bound, they're IO-bound

[2024-08-27 21:28] twopic: That's why I'm scared

[2024-08-27 21:28] emma: [replying to twopic: "I don't know how to code java"]
java is simple

[2024-08-27 21:29] emma: not like rust

[2024-08-27 21:29] Deleted User: [replying to contificate: "here's a simpler example: many networking programs..."]
epoll <:trolol:836730473061154832>

[2024-08-27 21:29] contificate: literally

[2024-08-27 21:29] Deleted User: I'm so fucking gay

[2024-08-27 21:29] contificate: there's also known deficiencies in many compilers that haven't been addressed

[2024-08-27 21:30] contificate: do you think that's a good enough example

[2024-08-27 21:30] contificate: of something widely used

[2024-08-27 21:30] Deleted User: nope

[2024-08-27 21:30] twopic: [replying to emma: "java is simple"]
I'll try to look up some guides but Im ass at coding

[2024-08-27 21:30] contificate: I also never said "trust me" about the point you have strawmanned

[2024-08-27 21:30] contificate: I just wanted you to stop condescending me

[2024-08-27 21:31] contificate: as if you think I know nothing of FPGAs and high frequency trading in C++

[2024-08-27 21:31] contificate: I wasn't asking you to trust me about some point you've caricatured

[2024-08-27 21:32] contificate: have you written Ada, by chance?

[2024-08-27 21:32] contificate: do you know what goes into avionics

[2024-08-27 21:34] Brit: [replying to emma: "not like rust"]
? rust is simple too, in fact it holds your hand a lot

[2024-08-27 21:34] Azrael: [replying to twopic: "I don't know how to code java"]
It'll be fine.

[2024-08-27 21:36] emma: [replying to Brit: "? rust is simple too, in fact it holds your hand a..."]
sometimes, but the compiler isn't always good with error messages  :3

[2024-08-27 21:37] contificate: Java is relatively comfy for learning algorithms because you get a lot of building blocks in the standard lib

[2024-08-27 21:37] contificate: of course, the first few tasks will be making those building blocks poorly

[2024-08-27 21:37] contificate: but you can then dispense with your own impl and use the stdlib's

[2024-08-27 21:38] contificate: the domain is also largely unpolluted by irrelevant concerns that pervade implementations in other languages

[2024-08-27 21:38] Deleted User: https://youtu.be/tpaajr0st4U?si=7IVtrfKBEhByflyo
[Embed: Title Fight - "Dizzy" (Full Album Stream)]
Listen to the full album: http://bit.ly/1DBVSbB
"Dizzy" by Title Fight from the album 'Hyperview,' out now
Download on iTunes: http://found.ee/titlefighthyperview

Facebook: https://www.facebook.com/t

[2024-08-27 21:43] twopic: [replying to Azrael: "It'll be fine."]
Yeah

[2024-08-27 21:44] contificate: you can just get started whenever

[2024-08-27 21:44] contificate: there's a lot of undue trepidation in learning algorithms and using unfamiliar programming languages

[2024-08-27 22:32] twopic: [replying to Azrael: "It'll be fine."]
unfortunately I had to drop the class since there were some conflicts

[2024-08-27 22:33] Azrael: [replying to twopic: "unfortunately I had to drop the class since there ..."]
Hmm?

[2024-08-27 22:33] twopic: I'm taking a different cs class

[2024-08-27 22:33] Azrael: Why's that?

[2024-08-27 22:34] twopic: the data class overlapped with one of my labs

[2024-08-27 22:34] Azrael: Oh okay.

[2024-08-27 22:34] twopic: I really want to take that class

[2024-08-27 22:34] twopic: But oh well

[2024-08-27 22:38] contificate: [replying to twopic: "unfortunately I had to drop the class since there ..."]
weak

[2024-08-27 22:54] Torph: [replying to Brit: "? rust is simple too, in fact it holds your hand a..."]
idk man I found the many different types of strings very confusing. also because not all safe programs are expressable in safe rust, you can end up fighting the language a ton if your design falls into that category

[2024-08-27 22:56] Torph: [replying to contificate: "Java is relatively comfy for learning algorithms b..."]
yeah, we used Java for basic algorithms last year. it was useful as a sort of "worry free" environment where you can play with high level concepts while learning to work with a compiler and normal C-style syntax (unlike Python, which was the semester before that)

[2024-08-27 22:59] szczcur: [replying to contificate: "I also never said "trust me" about the point you h..."]
yea the gif when you said “trust me im aware of […]” (https://discord.com/channels/835610998102425650/835664858526646313/1278100913227038791 ) wasn’t meant to serve as some kind of strawman, that was genuinely intended to be a joke. i pictured the meme and laughed and thought it was funny. poorly timed following my response here https://discord.com/channels/835610998102425650/835664858526646313/1278102662864441384, which was genuinely what it felt like and then i moved on mentally but didn’t offer any filler to diffuse, that’s my bad. sorry. 

in any case, yes, i do have experience with avionics systems and i have worked with ada. it’s used in telecomm and radar as i mentioned earlier too. it’s not the whole package though. if youve worked on them before then i retract my earlier statement suggesting inexperience with some/all of the things mentioned. that said, i think we were arguing two different things. i read your message about premature optimization differently than was probably intended, hence why i cited specific areas that you would want to ‘prematurely optimize’.. it is probably a difference in definitions of that between us and a misinterpretation on my end. how i took it was suggesting that optimizing for some constraint/requirement ahead of time is not important, just get x thing working without regard to specifics of some hardware or otherwise.. sure, of course you can get away with that in some places, but in my head that generalization didn’t make much sense when associating them with my previous mention of systems that require strict adherence to something like timings of operations. the algorithms used need to be efficiently implemented from the get go (ime, most of the time).

[2024-08-27 23:02] Torph: average day in secret club

[2024-08-27 23:03] szczcur: [replying to Torph: "average day in secret club"]
dunno what this means

[2024-08-27 23:04] szczcur: also clarifying this is not a continuation of the debate or counter to your points. its an explanation after i went back to see where things derailed.

[2024-08-27 23:04] contificate: Thanks for the clarification. I was just trying to re-centre it around a very general view of programming, as the original discussion seemed to be about the kind of applicability of things you learn by doing leetcodes. Not to discount domains where the name of the game is optimisation. I can see why it's easy to misinterpret what I've said in the way I've said it, the clauses weren't very well delimited. The simple point I was making is that, in general programming, one can yak shave ad infinitum if they believe every little thing must be optimised from the get go - my point being that, in very small cases, I still prefer a slightly more costly solution (in terms of constant factors) if it maintains a nice invariant in the codebase (i.e. continuing to use a very small set (backed by a balanced tree or something) instead of an array of things, even if querying the array may be faster for small `n`).

[2024-08-27 23:06] szczcur: [replying to contificate: "Thanks for the clarification. I was just trying to..."]
understood and yes I would agree with this overall, misinterpretation on my end, ill try to be more thorough with inquiry before making assumptions about what someone is referring to.

[2024-08-29 17:46] archie_uwu: anyone tried to do nested svm here? i'm implementing one for my hypervisor, but once I execute vmrun on the L2 guest, the computer freezes and seemingly gets stuck inside a recursive KiPageFault call - i've tried everything I could think of to troubleshoot this, and just not getting anywhere
since I don't wanna clog the chat too much with windbg logs etc., all the relevant info is here: <https://gist.github.com/Archie-osu/df34beccbc275dbcdd60e22f149b7622>