# 🐍 SnakePkg


The classic Snake game reimagined as a UEFI Application

*Made for fun and showcasing knowledge of EDK2 and UEFI development*

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](License)
[![EDK2](https://img.shields.io/badge/EDK2-BSD--2--Clause--Patent-blue.svg)](License.edk2)
[![Platform](https://img.shields.io/badge/platform-UEFI-brightgreen.svg)]()
[![Architecture](https://img.shields.io/badge/arch-X64%20%7C%20IA32-orange.svg)]()


## Prerequisites

### Required Packages

Install the necessary packages for building and testing:

#### Arch Linux
```bash
sudo pacman -S edk2-ovmf qemu-full base-devel
```

#### Debian/Ubuntu
```bash
sudo apt update
sudo apt install edk2-ovmf qemu-system-x86 build-essential uuid-dev
```

#### Fedora
```bash
sudo dnf install edk2-ovmf qemu-system-x86 gcc make libuuid-devel
```

### EDK2 Source

You'll need a working EDK2 build environment. Follow the steps below.

## Usage

### 1. Running it on real hardware

#### Prerequisites

- A UEFI-compliant device, with a USB port
- A FAT32-formatted USB storage device (thumbstick, disk, ...)

#### Steps

1. Download the latest `SnakeX64.efi`/`SnakeIA32.efi` (depending on your CPU architecture) from the [Releases Tab](https://github.com/AstonishedLiker/SnakePkg/releases)
2. Rename `SnakeX64.efi`/`SnakeIA32.efi` to `BOOTX64.EFI`/`BOOTIA32.EFI` (again, depending if your CPU is 32-bit or 64-bit)
3. Create a folder named `EFI` at the root of your FAT32-formatted USB storage device
4. Inside that newly-created folder, create another folder named `BOOT`
5. Move the `BOOT<ARCH>.EFI` to the `BOOT` folder
    - The file should be located at `USB Storage Device/EFI/BOOT/`
6. Reboot your PC, then mash the `F12` key to go to the boot menu
7. Select your USB storage device from the list, and enjoy!

**NOTE:** If the application doesn't work, please open an issue and list your PC/Motherboard model, along with its BIOS version. Thanks!

### 2. Clone and Setup

```bash
git clone https://github.com/tianocore/edk2.git
cd edk2
git submodule update --init # cf. https://github.com/tianocore/edk2/tree/master?tab=readme-ov-file#submodules

git clone https://github.com/AstonishedLiker/SnakePkg.git
```

### 3. Build and Run (In QEMU)

```bash
cd SnakePkg
cd ./Scripts
chmod +x run.sh snake_build.sh

# Examples
./run.sh RELEASE X64
```

### 4. Build Only (No VM)

```bash
# Just compile without launching QEMU
cd ./Scripts
./build.sh RELEASE X64
./build.sh RELEASE IA32
```

The compiled `.efi` file will be located at:

```
Build/SnakePkg/DEBUG_GCC5/X64/Snake.efi
```

### 5. Debugging

#### With the command-line

```bash
./run.sh DEBUG X64 # Only X64 builds are supported for debugging
```

In another shell instance:

```bash
lldb

# Once inside LLDB:
(lldb) gdb-remote localhost:1234 # QEMU gdb port
(lldb) command script import "<SNAKEPKG_PATH>/lldb_uefi_helper.py"
(lldb) continue
(lldb) auto_load_symbols <SNAKEPKG_PATH>/../Build/SnakePkg/DEBUG_GCC5/X64/debug.log # Will auto-scan for the `debug.log` file!
```

#### With VS Code

- Install the [LLVM DAP](https://open-vsx.org/extension/llvm-vs-code-extensions/lldb-dap) VS Code extension.
- Select the `64-bit UEFI Debug App` launch target in the Debug Pane, which can be accessed in the VS Code sidebar.
- The symbols should automatically resolve via [`./lldb_uefi_helper.py`](./lldb_uefi_helper.py).

**NOTE:** Breakpoints before the `ImageBase` is outputted to `ConOut` won't be hit!

## License

This project uses a dual-license approach:

### Original Code (MIT License)

The Snake game application code written for this project is licensed under the **MIT License**.

See [the `License` file](./License) for the full MIT License text.

### EDK2 Components (BSD-2-Clause-Patent License)

This project builds upon and uses components from the EDK-II project, which is licensed under the **BSD-2-Clause-Patent License**. This includes:

- EDK2 libraries
- Build system and tooling
- The [TianoCore bitmap](./Snake/Assets/Logo.bmp) (from [`MdeModulePkg/Logo/Logo.bmp`](https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Logo/Logo.bmp))

See [the `License.edk2` file](./License.edk2) for the full BSD-2-Clause-Patent License text.

## Acknowledgments

- [TianoCore EDK-II](https://github.com/tianocore/edk2) - The UEFI development framework
- [UEFI Forum](https://uefi.org/) - UEFI specifications
