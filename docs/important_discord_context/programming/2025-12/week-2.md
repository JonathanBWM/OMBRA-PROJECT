# December 2025 - Week 2
# Channel: #programming
# Messages: 5

[2025-12-08 17:19] Addison: see <#902892977284841503>

[2025-12-14 12:50] ml: Hi, does anyone know how to fix this when unloading a manual mapped dll?
[Attachments: message_3.txt]

[2025-12-14 13:54] ml: btw the crash start at: Exception levée à 0x0000029455BD7481 dans javaw.exe : 0xC0000005 : Violation d'accès lors de la lecture de l'emplacement 0x0000029446BA0000.

[2025-12-14 14:58] daax: [replying to ml: "Hi, does anyone know how to fix this when unloadin..."]
unloading a manually mapped modules is memset(base,0,image_size). there is no *unloading* once it’s mapped, only zero and free the associated memory.

[2025-12-14 18:52] ml: [replying to daax: "unloading a manually mapped modules is memset(base..."]
Yes that what i have did and:
```Exception levée à 0x000000019063AB80 dans javaw.exe : 0xC0000005 : Violation d'accès lors de l'exécution à l'emplacement 0x000000019063AB80.```


```     000000019063ab80()    Inconnu
     KernelBase.dll!00007ffbbc1e8546()    Inconnu
     jvm.dll!00007ffb49017be5()    Inconnu
     jvm.dll!00007ffb4900c5da()    Inconnu
     jvm.dll!00007ffb491d34d4()    Inconnu
     jvm.dll!00007ffb491d3656()    Inconnu
     jvm.dll!00007ffb48cf26c6()    Inconnu
     jvm.dll!00007ffb48ba2123()    Inconnu
     jvm.dll!00007ffb48ff2c78()    Inconnu
     jvm.dll!00007ffb48b653b9()    Inconnu
     jvm.dll!00007ffb49023024()    Inconnu
     jvm.dll!00007ffb48bb8c39()    Inconnu
     jvm.dll!00007ffb48bb7c61()    Inconnu
     jvm.dll!00007ffb48b4406d()    Inconnu
     jvm.dll!00007ffb48bc75d8()    Inconnu
     jvm.dll!00007ffb48bc572c()    Inconnu
     jvm.dll!00007ffb4918bd4c()    Inconnu
     jvm.dll!00007ffb491862ea()    Inconnu
     jvm.dll!00007ffb49019d55()    Inconnu
     ucrtbase.dll!thread_start<unsigned int (__cdecl*)(void *),1>()    Inconnu
     kernel32.dll!00007ffbbd337374()    Inconnu
>    ntdll.dll!RtlUserThreadStart()    Inconnu
```