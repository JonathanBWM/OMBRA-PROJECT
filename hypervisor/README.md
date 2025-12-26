# OmbraHypervisor

Hyper-V hijacking framework. Build via GitHub Actions or Windows.

## Structure
- `usermode/` - Ring-3 loader
- `hypervisor/` - Ring -1 core
- `shared/` - Common headers

## Build
Push to main → GitHub Actions builds on Windows.
Or run `build\build.bat` on Windows with VS2022.
