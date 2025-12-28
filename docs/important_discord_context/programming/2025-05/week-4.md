# May 2025 - Week 4
# Channel: #programming
# Messages: 29

[2025-05-22 03:57] Xits: Anyone know a repo with an example of validating the types of variadic args vs a const format string at compile time in c++

[2025-05-22 03:58] Xits: I’m stuck on trying to convert a const std::string to a compile time tuple of characters. My idea was to iterate both simultaneously using the fold expression

[2025-05-22 04:21] JustMagic: [replying to Xits: "I’m stuck on trying to convert a const std::string..."]
std::string? I don't think that will ever work

[2025-05-22 04:26] Xits: ah alright. im a newb at template 😦

[2025-05-22 04:26] Xits: is there a correct way to do it? if its just a const char* instead?

[2025-05-22 04:26] JustMagic: yes, it's possible with a const char*

[2025-05-22 05:01] Xits: ```
    template<std::size_t... Is>
    static constexpr auto stringToTuple(const char* str, std::index_sequence<Is...>) {
        return std::make_tuple(str[Is]...);
    }

    template <typename Char, typename Arg>
    static constexpr void ValidateArgs_internal(Char c, Arg a) {

        switch(c){
            case 'i':
                if(!std::is_same<decltype(a), int>()){
                    throw std::invalid_argument("arg is not type int");
                }
                break;
            default:
                throw std::invalid_argument("bad type");
        }
    }

    template <typename Char, typename... Args>
    static constexpr void ValidateArgs_unpack_args(Char s, Args... args) {
        ValidateArgs_internals(s, args...);
    }

    template <typename... Args>
    static constexpr void ValidateArgs(const char* format, Args... args) {
        std::tuple<Args...> argTuple(args...);
        constexpr auto num_args = sizeof...(args);
        constexpr auto format_sz = sizeof(*format);

        const auto strTuple = stringToTuple(format, std::make_index_sequence<format_sz>());

        if (format_sz > num_args){
            throw std::invalid_argument("too few arguments");
        }else if (format_sz < num_args) {
            throw std::invalid_argument("too many arguments");
        }

        std::apply([&](auto &&... strargs) { ValidateArgs_internal(strargs..., args); }, strTuple);
    }
```
how far off is this?

[2025-05-22 05:02] Xits: i think I converted the const char* to a compiletime tuple but really lost trying to "unpack" it

[2025-05-22 06:07] JustMagic: [replying to Xits: "```
    template<std::size_t... Is>
    static con..."]
you don't need the template cancer for this anymore

[2025-05-22 06:07] JustMagic: ~~https://godbolt.org/z/sxn9zn3Y7~~ https://godbolt.org/z/9sbrdGWT9 <- this one will actually work properly for your use case.

[2025-05-22 07:37] Xits: why is this not a constexpression anymore? https://godbolt.org/z/9dberEe14
[Embed: Compiler Explorer - C++ (x86-64 gcc (trunk))]
constexpr void parse(std::string_view format)
{
    for(auto i = 0u; i &lt; format.length(); i++){
        switch(format[i]){
            case 'i':
                if(!std::is_same&lt;int, decltype(a)

[2025-05-22 07:38] Xits: I was trying to pass the class templated args to parse and iterate it along with the string_view

[2025-05-22 22:22] Xits: I figured something out eventually 😭 https://godbolt.org/z/W5E773oqG
[Embed: Compiler Explorer - C++ (x86-64 gcc (trunk))]
//base case for no args left 
template&lt;std::size_t N, typename... Args&gt;
consteval void validate_internal(const char (&amp;s)[N]){
    //throw "in the base case";
}

template&lt;std::size_t N, ty

[2025-05-24 13:39] space: anyone here got any experience on hosting socks5 proxies? i have a few bots running on ubuntu servers and whenever I want to connect to them on my local pc I'd like to keep the server's ip. For that I'd need them exposed as socks5 proxies

is it hard to do? could someone link me somewhere I could understand a little bit more about it? I have 0 knowledge on those kind of stuff

[2025-05-24 14:30] mrexodia: [replying to space: "anyone here got any experience on hosting socks5 p..."]
Might be easier to set up a Wireguard VPN on the server in that case if the goal is to have all the traffic routed through there

[2025-05-24 14:30] mrexodia: Another option is `tinyproxy`, which is super easy to set up

[2025-05-24 14:35] space: [replying to mrexodia: "Might be easier to set up a Wireguard VPN on the s..."]
yeah I found smth called dante and wireproxy, I'll try it later

[2025-05-24 14:35] space: ty

[2025-05-24 14:36] space: tinyproxy seems to be http tho, those I mentioned ^ are socks5

[2025-05-24 14:52] mrexodia: [replying to space: "tinyproxy seems to be http tho, those I mentioned ..."]
yes but socks5 is not appropriate for your stated application

[2025-05-24 14:52] mrexodia: you should use a vpn on the network layer of your VMs

[2025-05-24 21:34] roddux: [replying to mrexodia: "you should use a vpn on the network layer of your ..."]
https://tenor.com/view/youknow-you-gif-19056787

[2025-05-25 08:11] Timmy: [replying to space: "anyone here got any experience on hosting socks5 p..."]
Tailscale with an exit node might be your solution.

[2025-05-25 21:37] roddux: tailscale is wireguard for people who cannot configure wireguard

[2025-05-25 21:42] truckdad: or just don't want to

[2025-05-25 21:43] truckdad: i'm technically capable of going into all of my devices to add a new endpoint or setting up a TURN-style relay to connect two devices behind symmetric NATs but i'm not particularly interested in spending time on either

[2025-05-25 21:43] truckdad: 🤷

[2025-05-25 21:44] truckdad: i agree it probably isn't the best fit here, though

[2025-05-25 21:46] 0xdeluks: [replying to roddux: "tailscale is wireguard for people who cannot confi..."]
real, my wireguard setup was hella weird so i just went with tailscale (cant port forward so i had wireguard with gsocket lol)