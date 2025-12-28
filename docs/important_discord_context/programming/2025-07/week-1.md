# July 2025 - Week 1
# Channel: #programming
# Messages: 28

[2025-07-02 05:38] ENA: [replying to elias: "Is there a way to enumerate file objects from km w..."]
if you're searching for a specific file object, you can try to open it and check the hresult maybe ?
it's a theory

[2025-07-02 13:56] koyz: https://github.com/dotnet/runtime/issues/117233 how does microsoft even achieve that
[Embed: C# Math.Pow(-1, 2) doesn't output correct value on Windows 11 Insid...]
Description The following is copied with extra bits from https://aka.ms/AAwwjwl (Feedback Hub) to raise awareness of this issue: Hello, I&#39;m a contributor to osu! (https://github.com/ppy/osu). A...

[2025-07-02 14:15] Timmy: jesus

[2025-07-03 14:28] contificate: got tired of encoding inductive datatypes in C, so I've written a generator https://github.com/contificate/summary - it can pretty print the datatypes
[Attachments: 2025-07-03-152752_1920x959_scrot.png]
[Embed: GitHub - contificate/summary: A toy ASDL (Abstract Syntax Descripti...]
A toy ASDL (Abstract Syntax Description Language) intended to make writing compilers in C less tedious. - contificate/summary

[2025-07-04 20:26] Max: Hey im coding a projekt in js/ts and im using prisma...
Im haveing major problems adding a Task that has a 1:m realtion to Group..

Model
```
model Group {
  id          Int     @id @default(autoincrement())
  ownerId     Int
  title       String
  description String?

  owner User @relation("GroupOwner", fields: [ownerId], references: [id])

  userGroups UserGroup[]

  tasks Task[]
}

model Task {
  id          Int       @id @default(autoincrement())
  groupId     Int
  title       String
  description String?
  dueDate     DateTime?
  isDone      Boolean   @default(false)

  group Group @relation(fields: [groupId], references: [id])

  userTasks UserTask[]
}
```

Code
```
const newTask = await prisma.task.create({
            data: {
                title: "Neue Aufgabe",
                description: "Beschreibung der Aufgabe",
                dueDate: new Date("2025-08-01T12:00:00Z").toISOString(),
                groupId: groupId,
            },
        });
```

Can anyone Help?

[2025-07-05 15:25] roddux: [replying to contificate: "got tired of encoding inductive datatypes in C, so..."]
is lambda.h auto-generated ?

[2025-07-05 15:26] roddux: ( also wat is `dune` )

[2025-07-05 15:33] contificate: yeah, `lambda.sm` becomes `lambda.h` and `lambda.c`

[2025-07-05 15:33] contificate: `dune` is the OCaml build system

[2025-07-05 15:34] contificate: it's an OCaml program that parses a file and emits C code

[2025-07-05 15:35] contificate: 
[Attachments: 2025-07-05-163525_1245x663_scrot.png]

[2025-07-05 15:42] roddux: ahh OCaml, nice

[2025-07-05 15:43] roddux: so what defines the `app`, `fun` variables? are they macros in `lambda.h`? 🤔

[2025-07-05 15:43] roddux: love your arena.c btw lol

[2025-07-05 15:44] contificate: the generated construction functions populate them

[2025-07-05 15:44] contificate: I've kept the substructure as anonymous structs

[2025-07-05 15:44] contificate: to avoid people becoming overreliant on generated definitions

[2025-07-05 15:45] contificate: I use the word "ephemeral" to describe the intent of these things

[2025-07-05 15:45] contificate: perfect for an AST or something

[2025-07-05 16:36] bugdigger: Anyone has experience with disabling Windows Kernel DMA protection with a bootkit?

My stupid Blade 16 laptop doesn't have option in BIOS for VT-d, so I thought of maybe writing efi driver that will delete/corrupt ACPI DMAR table? Am I on a good path?

[2025-07-05 16:38] bugdigger: This is my first time writing drivers for uefi so i built edk2 and will use qemu for debugging. Is there some better way of testing my bootkit (when i develop it eventually)?

[2025-07-05 17:42] UJ: [replying to bugdigger: "Anyone has experience with disabling Windows Kerne..."]
Did you try dumping the bios to see how vt-d is set to on/off using IFRExtractor and using something like https://github.com/datasone/setup_var.efi to turn it off if its configurable?

[2025-07-05 18:27] bugdigger: [replying to UJ: "Did you try dumping the bios to see how vt-d is se..."]
Hmm I didn't know about this tool and I see that it says that it can modify stuff hidden from BIOS UI but still I am scared of bricking the laptop.

Thats why I thought that maybe bootkit is better option.

Ah i also thought of using UefiTool, patching the field and then flashing it to SPI chip (i have hardware programmer so this is doable for me) but i am afraid of possibly bricking the laptop.

[2025-07-05 18:30] bugdigger: Btw how can i dump the bios? 
I am also trying to avoid hardware programmer for this.

[2025-07-05 18:31] UJ: I would keep a copy of the current bios grabbed from the chip before you start messing around with this but yeah there is the possibility of bricking here.

[2025-07-05 18:34] UJ: [replying to bugdigger: "Btw how can i dump the bios? 
I am also trying to ..."]
if you just want to explore for now, you can try downloading the bios from the manufacturer's website, open it in uefi tools, dump the setup region and use IFRExtractor to convert it to text and search for vt-d to see if its configurable. it should be. 

But to actually dump the bios, you would use flashrom or Intel FPT. (What cpu do you have?)

[2025-07-05 18:38] bugdigger: [replying to UJ: "if you just want to explore for now, you can try d..."]
i9-14900HX

[2025-07-05 18:48] UJ: Yeah its annoying trying to find that version of the Intel tools maps to which cpu gen but thats a couple gen old now so hopefully the versions you need are out there on sites like winraid etc. (Im sure Matti will chime in as well when he gets on)