# September 2024 - Week 5
# Channel: #programming
# Messages: 20

[2024-09-26 14:08] brymko: mandetory monologe watch https://www.youtube.com/watch?v=mTa2d3OLXhg
[Embed: DHH Is Right About Everything]
Recorded live on twitch, GET IN 

### Guests
-- DHH | https://x.com/dhh | https://dhh.dk/
-- TJ DeVries | https://x.com/teej_dv | https://www.twitch.tv/teej_dv | https://www.youtube.com/@teej_dv

### 

[2024-09-26 14:59] Torph: 10 steps ahead of you already watched last week

[2024-09-26 15:56] nato: I can override CMAKE_CXX_FLAGS using [variables] but I want to override it for a specific target how can I achieve that using cmkr? There are more CMAKE flags I want to override aswell

[2024-09-26 18:38] mrexodia: [replying to nato: "I can override CMAKE_CXX_FLAGS using [variables] b..."]
As a general rule, never overwrite the CMAKE_CXX_FLAGS variable. If you _require_ certain flags to compile your project you can do something like this:
```toml
[target.foo]
type = "executable"
msvc.compile-options = ["xxx"]
```

[2024-09-26 18:51] nato: [replying to mrexodia: "As a general rule, never overwrite the CMAKE_CXX_F..."]
I was trying to compile without crt and exceptions, can I remove /EHsc flag without overriding CMAKE_CXX_FLAGS?

[2024-09-26 18:57] mrexodia: [replying to nato: "I was trying to compile without crt and exceptions..."]
I did something like this:
```toml
msvc.compile-options = [
    "/GS-",
]
msvc.link-options = [
    "/INCREMENTAL:NO",
    "/ENTRY:EntryPoint",
    "/NODEFAULTLIB",
    "/DEBUG",
    "/PDBALTPATH:%_PDB%",
    "/FILEALIGN:0x1000",
]
```

[2024-09-26 18:57] mrexodia: not sure if `/EHsc-` exists though

[2024-09-27 08:13] nato: [replying to mrexodia: "not sure if `/EHsc-` exists though"]
adding ``/EHs-`` worked, thanks 🙏

[2024-09-27 22:59] daax: https://github.com/CppCon/CppCon2024/blob/main/Presentations/Peering_Forward_Cpps_Next_Decade.pdf
[Embed: CppCon2024/Presentations/Peering_Forward_Cpps_Next_Decade.pdf at ma...]
Contribute to CppCon/CppCon2024 development by creating an account on GitHub.

[2024-09-27 22:59] daax: 
[Attachments: image.png]

[2024-09-28 02:44] Torph: what on earth
that's going to become valid C++?

[2024-09-28 02:55] dinero: 
[Attachments: IMG_1879.png, IMG_1878.png]

[2024-09-28 02:58] dinero: powerful

[2024-09-28 09:51] Brit: metafunc{:^)__proto}

[2024-09-28 09:51] Brit: least insane c++ proposal

[2024-09-28 17:44] sariaki: dude what

[2024-09-28 17:44] sariaki: [replying to daax: ""]
who in their right mind would think that this is a good idea

[2024-09-28 17:47] Brit: genuinely not even that bad

[2024-09-28 17:48] brymko: genuinely abnoxious

[2024-09-28 17:48] Brit: ahuh