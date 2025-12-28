# June 2025 - Week 6
# Channel: #🔗-research-and-links
# Messages: 2

[2025-06-30 17:59] mrexodia: # June 2025: Type System and Modernization
Excited to announce a [major new release of x64dbg](https://releases.x64dbg.com), the open-source user mode debugger for Windows. For those new to the project, x64dbg is designed to make reverse engineering and malware analysis faster and more intuitive. This release marks a significant step forward, overhauling our core type system and modernizing our entire toolchain to bring you a more powerful and stable debugging experience.
## ✨ Revamped Type System
In previous versions, analyzing data structures was a tedious, manual process and many features were not supported. This release adds support for bitfields, enums and anonymous types, which allows all types in the Windows SDK to be represented and displayed.

The [ManyTypes](https://github.com/notpidgey/ManyTypes) plugin by snow (who also drove this revamp) allows you to import C header files and see the results directly in x64dbg. While we plan to streamline this workflow even further in future updates, this is a huge leap in making data inspection easier.

This isn't just about convenience; it's about speed. We've introduced **drastic performance improvements** to the struct widget, so you can now browse deeply nested pointers and large data structures without the lag.

We’ve also added a host of quality-of-life improvements:
- **Interactive type selection**: While selecting the type you will instantly see what the data looks like, enabling a more interactive workflow.
- **Smarter Displays:** Character arrays are now automatically rendered as strings, saving you an extra step.
- **Better Integration:** You can now invoke the "Display type" action directly from the register and stack views, making it easier than ever to inspect data on the fly.
- **AVX-512 Support**: View and manipulate AVX-512 register on supported systems.
- **Revamped Launcher Setup**: The launcher now has checkboxes and an uninstall option thanks to G3ph4z <:give:798922993287561246>

## 🔮 Looking Ahead: A Cross-Platform Future
We're excited to share a glimpse into our experimental `cross-platform` branch. This is our testbed for separating core GUI widgets into a reusable library, with the goal of bringing x64dbg's powerful tools to all major platforms (Windows, macOS, and Linux).

Current experimental tools include:
- A simple **hex viewer** that uses the ImHex Pattern Language to visualize data structures.
- A **minidump viewer** to browse `.dmp` files on any platform.
- A **remote table** tool that showcases fetching data over a high-latency network.

These tools are still in early development and not part of the release, but they represent a critical step toward a more versatile and platform-independent future for our components.
[Embed: GitHub - notpidgey/ManyTypes: x64dbg typeparsing plugin with Window...]
x64dbg typeparsing plugin with Windows types. Contribute to notpidgey/ManyTypes development by creating an account on GitHub.

[2025-06-30 18:22] mrexodia: 
[Attachments: 19-01-53-types-update.mp4]