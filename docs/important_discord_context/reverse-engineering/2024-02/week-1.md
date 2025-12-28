# February 2024 - Week 1
# Channel: #reverse-engineering
# Messages: 30

[2024-02-01 02:23] Deleted User: any recommended plugins for IDA?

[2024-02-01 07:10] Magic: Is there a working vm decoder for Themida ?

[2024-02-02 05:04] bowen: ontext

[2024-02-02 06:55] BWA RBX: [replying to Magic: "Is there a working vm decoder for Themida ?"]
You mean devirtualizer

[2024-02-02 08:05] Magic: Yes

[2024-02-02 14:29] Xed0sS: [replying to Ignotus: "Trying to get a decompiled code of a specific meth..."]
Can share this class?

[2024-02-02 14:52] Ignotus: [replying to Xed0sS: "Can share this class?"]
when im on pc sure

[2024-02-02 19:32] Ignotus: [replying to Xed0sS: "Can share this class?"]

[Attachments: U.class]

[2024-02-02 19:37] Ignotus: ```java
String hexEncodedStr = "75C73E17ED54BCBA105BDA0B63AEE6A372263B67440EFEADD1A3112E6BB76D431927973973C10058E199EF97A4F34F498962DDD64E4872065A5DC71220A19A01";

Iterator<Byte> byteIterator = new Iterator<>() {
    private int index = 0;

    @Override
    public boolean hasNext() {
        return index < hexEncodedStr.length();
    }

    @Override
    public Byte next() {
        byte b = (byte) Integer.parseInt(hexEncodedStr.substring(index, index + 2), 16);
        index += 2;
        return b;
    }
};

Predicate<Byte> predicate = new Predicate<>() {
    @Override
    public boolean test(Byte b) {
        if (!byteIterator.hasNext()) {
            return false;
        }
        return b.equals(byteIterator.next());
    }
};

boolean result = T(predicate, 107891122724046L, 64)
```

[2024-02-02 19:37] Ignotus: for those params it should return true, if the string is changed, it should return false

[2024-02-02 19:39] Xed0sS: it's simple flow obfuscation

[2024-02-02 19:42] Ignotus: that's interesting, I did try jadx but it failed to decompile it

[2024-02-02 19:44] Xed0sS: deobfed + cleaned
[Attachments: image.png]

[2024-02-02 19:44] Xed0sS: 
[Attachments: U_deobfed.class]

[2024-02-02 19:45] Xed0sS: [replying to Ignotus: "that's interesting, I did try jadx but it failed t..."]
now it works with cfr/procyon/etc

[2024-02-02 19:57] Ignotus: [replying to Xed0sS: "now it works with cfr/procyon/etc"]
how did u deobf it? if u dont mind me asking

[2024-02-02 19:58] Xed0sS: https://github.com/GraxCode/threadtear
[Embed: GitHub - GraxCode/threadtear: Multifunctional java deobfuscation to...]
Multifunctional java deobfuscation tool suite. Contribute to GraxCode/threadtear development by creating an account on GitHub.

[2024-02-02 19:59] Ignotus: ty

[2024-02-03 14:20] Ignotus: <@854754450772852786> can u use Krakatau with it? for me it throws 
```java
java.util.zip.ZipException: zip file is empty
    at java.util.zip.ZipFile.open(Native Method)
    at java.util.zip.ZipFile.<init>(Unknown Source)
    at java.util.zip.ZipFile.<init>(Unknown Source)
    at java.util.jar.JarFile.<init>(Unknown Source)
    at java.util.jar.JarFile.<init>(Unknown Source)
    at me.nov.threadtear.decompiler.KrakatauBridge.readOutput(KrakatauBridge.java:83)
    at me.nov.threadtear.decompiler.KrakatauBridge.decompile(KrakatauBridge.java:64)
    at me.nov.threadtear.swing.panel.DecompilerPanel.update(DecompilerPanel.java:214)
    at me.nov.threadtear.swing.panel.DecompilerPanel.lambda$reload$2(DecompilerPanel.java:182)
    at me.nov.threadtear.swing.panel.StatusBar$2.doInBackground(StatusBar.java:103)
    at me.nov.threadtear.swing.panel.StatusBar$2.doInBackground(StatusBar.java:100)
    at javax.swing.SwingWorker$1.call(Unknown Source)
    at java.util.concurrent.FutureTask.run(Unknown Source)
    at javax.swing.SwingWorker.run(Unknown Source)
    at java.util.concurrent.ThreadPoolExecutor.runWorker(Unknown Source)
    at java.util.concurrent.ThreadPoolExecutor$Worker.run(Unknown Source)
    at java.lang.Thread.run(Unknown Source)
```

[2024-02-03 17:39] Xed0sS: [replying to Ignotus: "<@854754450772852786> can u use Krakatau with it? ..."]
What is this?

[2024-02-03 17:41] Ignotus: [replying to Xed0sS: "What is this?"]
When I switch to Krakatau decompiler it throws that exception

[2024-02-03 21:34] hxm: i got a naive question : why does CE is so fast at mem scanning ?

[2024-02-03 21:36] not-matthias: [replying to hxm: "i got a naive question : why does CE is so fast at..."]
uses CUDA for pointer scans: https://github.com/cheat-engine/cheat-engine/tree/master/Cheat%20Engine/CUDA%20pointerscan

[2024-02-04 02:59] vendor: no it can’t. ept/npt is used to map that phys range in the root partition to a dummy page filled with 0xFF.

[2024-02-04 04:45] vendor: not that i know of. satoshi has some scripts on github for inspecting SLAT mappings i think.

[2024-02-04 04:47] vendor: https://github.com/tandasat/hvext

[2024-02-04 04:47] vendor: never used it though, i’ve only worked with hvax

[2024-02-04 18:16] Ignotus: Best way to automate patching a java file inside a jar? I need to look for the class based on instructions and then insert some instructions

[2024-02-04 18:17] Ignotus: I saw Recaf has scripting but I cant get it to run
[Attachments: image.png]

[2024-02-04 18:18] Leeky: unzip, look through the classes, zip again - https://asm.ow2.io/ probably works for parsing and modifcation