# April 2024 - Week 2
# Channel: #application-security
# Messages: 71

[2024-04-11 18:07] Tylor: Anybody know what happened to the back engineering caerus project thing

[2024-04-11 18:07] Tylor: https://web.archive.org/web/20230318213029/https://www.caerus.dev/
[Embed: CAERUS - Home]
A modern software protection and DRM solution.

[2024-04-11 18:23] Lewis: [replying to Tylor: "https://web.archive.org/web/20230318213029/https:/..."]
swapped to viper, and now codeprotect or something.

[2024-04-11 18:23] Lewis: https://www.back.engineering/viper
[Embed: Viper - Back Engineering Labs]
A modern product security solution. Protect and secure your software without the hassle.

[2024-04-11 22:29] p1ink0: [replying to Lewis: "https://www.back.engineering/viper"]
Everyone’s too poor to afford it

[2024-04-11 22:38] Lewis: [replying to p1ink0: "Everyone’s too poor to afford it"]
Obviously

[2024-04-11 22:38] Lewis: I have no doubt it's good though

[2024-04-12 01:30] p1ink0: Yeah it’s probably good

[2024-04-12 01:30] p1ink0: The TruDRM feature it has seems like something I want to work on in the future

[2024-04-12 01:34] 25d6cfba-b039-4274-8472-2d2527cb: game cheaters create over engineered rpc with a cool name <:mmmm:772345136037625876>

[2024-04-12 01:34] 25d6cfba-b039-4274-8472-2d2527cb: innovation

[2024-04-12 01:37] Terry: it is a cool name

[2024-04-12 01:51] p1ink0: xD

[2024-04-12 01:51] p1ink0: I love security :D

[2024-04-12 03:25] szczcur: [replying to Lewis: "https://www.back.engineering/viper"]
i dont know who made this, but after looking through the site and their blog.. im not sold. not that that matters, but there are commercial products for drm that exist and are very good, the market is already crowded and this has no proven track record in real world. large companies will go for something like playready or widevine derivative, denuvo or fairplay depending on platform. small companies or developers have the ability to buy licenses for vmprotect ((even with the leaks weve there has been nothing of substance with devirtualization)), themida or code virtualizer. not to mention the countless ways to in-house your own drm with llvm extension.. it’s more expensive in the short term but we had success with implementing our own obfuscation layers over using an external service. 

in my experience many companies will opt for enterprise solutions (this doesnt look like one), or they will roll their own internally. 500/mo for 3 products and licensing support (with no option for demo to test the durability of their licensing scheme).. immediate no from majority of companies. this reads like it was geared toward very profitable and very loose “organizations”

i went and looked at the twitter and associations and it seems like this is definitely a group of uni students building something of decent quality but not something that will compete in the market outside of niche communities like game hacks or warez distro.

[2024-04-12 03:31] Bloombit: [replying to szczcur: "i dont know who made this, but after looking throu..."]
It’s recently launched it looks like so of course there’s less customer backed data it works well. But they’re public testing appears to be real world and working

[2024-04-12 03:35] szczcur: [replying to Bloombit: "It’s recently launched it looks like so of course ..."]
it does not

[2024-04-12 03:36] szczcur: it is binary rewriting which is not something to scoff at, but its not what theyre describing in their platform

[2024-04-12 03:37] szczcur: i know there are a lot of game hacker sympathizers here so ill try not to tilt them, but this will not work in the market unless they differentiate or have some sort of bound over competition

[2024-04-12 03:37] Bloombit: $500/month seems fairly cheap

[2024-04-12 03:37] szczcur: and so far they dont have anything unique about them

[2024-04-12 03:37] Bloombit: If that’s the cost

[2024-04-12 03:38] szczcur: [replying to Bloombit: "$500/month seems fairly cheap"]
your lack of experience is showing. just because companies make $ doesnt mean they will waste it on a dead end, and their 500/mo offer is not what an enterprise would go for.

[2024-04-12 03:38] North: it seems like something those guys just have fun making, no reason not to sell it

[2024-04-12 03:38] szczcur: it isn’t even about the price

[2024-04-12 03:38] North: also they could get some really good connection even if they end up only working with a few companies

[2024-04-12 03:39] Bloombit: [replying to szczcur: "your lack of experience is showing. just because c..."]
I’ve evaluated many solutions 2 years ago for a product I was on. Ended up writing my own but would have evaluated this

[2024-04-12 03:40] szczcur: [replying to North: "also they could get some really good connection ev..."]
im interested in how that turns out. given what i saw on twitter and from their associations i wouldnt touch them with a 10-foot pole. companies do their due diligence, i spent 15 mins digging and there is something off about them

[2024-04-12 03:40] North: ikr i heard they hack games 🤢

[2024-04-12 03:41] Bloombit: [replying to szczcur: "im interested in how that turns out. given what i ..."]
What are you on about right now?

[2024-04-12 03:41] szczcur: [replying to Bloombit: "I’ve evaluated many solutions 2 years ago for a pr..."]
yes, of course, evaluation is important. i dont doubt evaluations will occur if they get it off the ground with obfuscations that jump over the competition

[2024-04-12 03:41] szczcur: [replying to North: "ikr i heard they hack games 🤢"]
believe it or not i used to as well, i have nothing against that. im talking about their contributors online presence.

[2024-04-12 03:41] szczcur: if youre gonna start a company try not to have problematic history follow you*

[2024-04-12 03:43] szczcur: [replying to Bloombit: "What are you on about right now?"]
theyre a bunch of uni kids with racist comments and mildly problematic statements plastered in their history. if theyre smart enough to write something like this they should clean up their history on social media and forums. companies are going to look all around.

[2024-04-12 03:43] North: honestly i kinda doubt they will

[2024-04-12 03:43] szczcur: [replying to North: "honestly i kinda doubt they will"]
the companies looking around?

[2024-04-12 03:52] Terry: [replying to szczcur: "if youre gonna start a company try not to have pro..."]
yeah tons of people already hate the owner/main developer for his past. as mentioned above a simple 10-15 minute search on twitter is all a large corp needs to stay away

[2024-04-12 03:52] Terry: i dont know much about the product, but I think he would have a better chance selling it under a different name lol

[2024-04-12 04:02] Bloombit: I’ll have to look more into it, with that info your tone makes sense now

[2024-04-12 04:02] szczcur: [replying to Terry: "yeah tons of people already hate the owner/main de..."]
whoever it is, they gotta have a differentiating function and have some professionals evaluate them otherwise it might be a slog to get into the market. if they get bought thatll be a win for them anyways.

[2024-04-12 04:04] Terry: [replying to szczcur: "whoever it is, they gotta have a differentiating f..."]
I don't really think they have anything special to get bought

[2024-04-12 04:04] Terry: At least hyperion had some interesting tech when they got bought by roblox

[2024-04-12 04:04] szczcur: idk what hyperion is

[2024-04-12 04:04] Terry: Pretty much the entire project is previous public projects made by the owner that he removed and "polished"

[2024-04-12 04:05] szczcur: but assuming it is another drm

[2024-04-12 04:05] Terry: [replying to szczcur: "idk what hyperion is"]
https://roblox.fandom.com/wiki/Hyperion

[2024-04-12 04:05] Terry: [replying to szczcur: "but assuming it is another drm"]
anti cheat with a "proactive" take

[2024-04-12 04:05] Terry: i just think it a fairly similar comparison, i do get they are different products tho

[2024-04-12 04:44] JustMagic: [replying to Terry: "anti cheat with a "proactive" take"]
anti-tamper*

[2024-04-12 04:47] Terry: nemo (or w.e his name is) said that on a podcast i watched, but yeah whatever the proper terminology is

[2024-04-12 04:48] Terry: did u have any stake in hyperion? or just worked on it

[2024-04-12 04:48] Terry: did u get any robux 💰

[2024-04-12 08:59] vendor: [replying to szczcur: "i dont know who made this, but after looking throu..."]
agree on pretty much everything said but they don't actually have a real product yet so there isn't really anything to judge. they built that thing on the website then quickly realized everything you said and so scrapped it all and started from scratch building a new bin2bin obfuscator with the goal of doing everything in the most "correct" and stable way, hence why they spent like 6 months just on the lifting and re-writing component making support for every possible edge case with all the different types of exceptions. interesting repo with test cases they've aggregated: <https://github.com/backengineering/bintests>

vmprotect, themida and friends have all done an amazing job but have remained pretty static and academic research on obfuscation has come a long way since. there is now quite a big gap between what's available to the public and what's in private solutions - i think there is a good chance this could bridge that gap.

[2024-04-12 12:21] Horsie: [replying to Bloombit: "It’s recently launched it looks like so of course ..."]
Holy shit I think I found the xeroxz alt

[2024-04-12 12:36] Lewis: [replying to Horsie: "Holy shit I think I found the xeroxz alt"]
LOL

[2024-04-12 12:37] szczcur: [replying to vendor: "agree on pretty much everything said but they don'..."]
it’s possible i suppose, i just have a lot of doubt it will bridge anything. binary rewriting and handling edge cases is required for any drm/sbr product to be successful. private r&d companies make these types of things and sell them to companies like google, msft, sony, mil, etc to merge them with existing solutions. the public solutions will always be weaker, and they aren’t going to win contracts at big companies because there are already tried and trusted solutions that exist with a stronger backing and existing contracts/licenses. 

im not saying they won’t do well at all, im saying that they need to significantly differentiate themselves or they will not get anywhere. and their product doesn’t need to be released to make this statement because it applies to all products wanting to compete in a saturated market. differentiate or die, doing something “better” in the process doesn’t mean jack if you don’t get better end results than existing solutions in that domain. you can have the best bin2bin but if you get cracked in a quarter of the time than denuvo, it looks bad. that’s an example, obviously there is nothing released yet.

[2024-04-12 12:52] szczcur: if they’re looking to get bought because of their bin2bin or some other process improvement (vs end result) then that’s different. lots of companies with ‘eh’ products get bought because they have some process improvement that a larger more experienced company could use more effectively. that’s not to say they cant or wont use their own thing effectively, but that’s an alternative if they dont get the results they hope for. which is why i said that getting bought  is a win too.

[2024-04-12 12:57] szczcur: a few ways to win, lots of ways to lose. hopefully they come out a trailblazer, but we’ll just have to wait and see how it turns out. im interested to see an evaluation.

[2024-04-12 13:02] daax: [replying to szczcur: "it’s possible i suppose, i just have a lot of doub..."]
other than your first statement, i agree. there is a lot to do to compete and stand out if that’s the objective. i imagine they will put in the work.

[2024-04-12 13:17] vendor: [replying to szczcur: "it’s possible i suppose, i just have a lot of doub..."]
i don't think they are doing a DRM product. pretty sure it's just going to be a vmp/cv/themida competitor.

[2024-04-12 13:24] szczcur: [replying to vendor: "i don't think they are doing a DRM product. pretty..."]
seems to state they’re doing or planning to do license management and authentication for software distribution, which is DRM.

[2024-04-12 13:24] vendor: [replying to szczcur: "seems to state they’re doing or planning to do lic..."]
the current site can be ignored. that is their old product they realised has no market for the reasons you highlighted above.

[2024-04-12 13:26] vendor: (i think at least, this is just what i've interpreted)

[2024-04-12 13:28] szczcur: [replying to vendor: "the current site can be ignored. that is their old..."]
ah so you do have personal ties. is it expected someone not associated know this? when going to the site the obvious conclusion to draw is that it’s an obfuscation/drm product.

[2024-04-12 13:30] szczcur: [replying to vendor: "(i think at least, this is just what i've interpre..."]
if you have deeper insight then ill take your word for it, just a bit confusing to see the site and then people saying “its not x”

[2024-04-12 13:35] vendor: [replying to szczcur: "ah so you do have personal ties. is it expected so..."]
yes i am friends with some them. idk why they haven't updated the site, maybe they still offer that service but it's not what they are working on. tbh i wouldn't expect their product to launch for another 6 months minimum based off the sounds of progress so no point in advertising it until it exists i guess.

[2024-04-12 13:40] vendor: [replying to szczcur: "im interested in how that turns out. given what i ..."]
but re this: i think them being edgy teens would definitely be an obstacle to big businesses adopting their product. maybe less of an obstacle to being bought out but regardless they are stupid for not cleaning up their act. if they put so much effort into a product and it's actually good just for everyone to avoid it then they will regret it.

[2024-04-12 14:32] 25d6cfba-b039-4274-8472-2d2527cb: Ye I personally found the licensing and rpc based drm to be the most questionable things. I mean if you want always online and serverside code exec you can just make a cloud based saas product, shoving that shit into regular software just makes it non viable for many use cases which might be offline (unless its a game or a cheat for one, where that seems to be standard practice). also these are things that make GDPR compliance a nightmare, so at the bare minimum it should be on-prem hostable for everyone, not just specific contract customers so they can self host the server.

[2024-04-12 21:12] p1ink0: [replying to Terry: "i dont know much about the product, but I think he..."]
That’s true, I’ve heard about very questionable things he’s done

[2024-04-13 02:04] North: What they really gotta do to get the corporate sales is add **AI obfuscation **🧠

[2024-04-14 22:38] p1ink0: [replying to North: "What they really gotta do to get the corporate sal..."]
And AI takes over the PCs