# 🐍 SnakePkg


The classic Snake game reimagined as a UEFI Application

*Made for fun and showcasing knowledge of EDK2 and UEFI development*

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![EDK2](https://img.shields.io/badge/EDK2-BSD--2--Clause--Patent-blue.svg)](LICENSE.edk2)
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

### 1. Clone and Setup

```bash
git clone https://github.com/tianocore/edk2.git
cd edk2
git submodule update --init # cf. https://github.com/tianocore/edk2/tree/master?tab=readme-ov-file#submodules

git clone https://github.com/AstonishedLiker/SnakePkg.git
```

### 2. Build and Run

```bash
cd SnakePkg
chmod +x run.sh build.sh
cd ./Scripts

# Examples
./run.sh DEBUG X64
./run.sh DEBUG IA32
./run.sh RELEASE X64
```

### 3. Build Only (No VM)

```bash
# Just compile without launching QEMU
cd ./Scripts
./build.sh DEBUG X64
```

The compiled `.efi` file will be located at:
```
Build/SnakePkg/DEBUG_GCC/X64/Snake.efi
```

## License

This project uses a dual-license approach:

### Original Code (MIT License)

The Snake game application code written for this project is licensed under the **MIT License**.

See [LICENSE](LICENSE) for the full MIT License text.

### EDK2 Components (BSD-2-Clause-Patent License)

This project builds upon and uses components from the EDK-II project, which is licensed under the **BSD-2-Clause-Patent License**. This includes:

- EDK2 libraries
- Build system and tooling
- The [TianoCore bitmap](./Snake/Assets/Logo.bmp.h) (cf. [`MdeModulePkg/Logo/Logo.bmp`](https://github.com/tianocore/edk2/blob/master/MdeModulePkg/Logo/Logo.bmp))
- `TranslateBmpToGopBlt()` function from `BaseBmpSupportLib`, declared in [`Snake/BmpSupport.h`](./Snake/BmpSupport.h)

See [LICENSE.edk2](LICENSE.edk2) for the full BSD-2-Clause-Patent License text.

## Acknowledgments

- [TianoCore EDK-II](https://github.com/tianocore/edk2) - The UEFI development framework
- [UEFI Forum](https://uefi.org/) - UEFI specifications
