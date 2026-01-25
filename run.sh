#!/bin/bash

ARCH_TARGET=${1:-X64}
BUILD_TARGET=${2:-DEBUG}

echo "Building for $ARCH_TARGET ($BUILD_TARGET)..."

case $ARCH_TARGET in
    X64|IA32)
        ;;
    *)
        echo "Error: Invalid architecture '$ARCH_TARGET'"
        echo "Valid options: X64, IA32"
        exit 1
        ;;
esac

./build.sh $ARCH_TARGET $BUILD_TARGET

if [ $? -ne 0 ]; then
    echo "Build failed! See errors above."
    exit 1
fi 

BUILD_DIR="../Build/SnakePkg/${BUILD_TARGET}_GCC/${ARCH_TARGET}"
ESP_DIR="${BUILD_DIR}/ESP"

case $ARCH_TARGET in
    X64)
        BOOT_FILE="BOOTX64.EFI"
        QEMU_ARCH="x86_64"
        OVMF_ARCH="x64"
        ;;
    IA32)
        BOOT_FILE="BOOTIA32.EFI"
        QEMU_ARCH="i386"
        OVMF_ARCH="ia32"
        ;;
esac

# OVMF Firmware (handles split firmware too)
if [ -f "/usr/share/edk2/${OVMF_ARCH}/OVMF.fd" ]; then
    OVMF_CODE="/usr/share/edk2/${OVMF_ARCH}/OVMF.fd"
    USE_SPLIT=false
elif [ -f "/usr/share/ovmf/${OVMF_ARCH}/OVMF.fd" ]; then
    OVMF_CODE="/usr/share/ovmf/${OVMF_ARCH}/OVMF.fd"
    USE_SPLIT=false
elif [ -f "/usr/share/edk2/${OVMF_ARCH}/OVMF_CODE.fd" ]; then
    OVMF_CODE="/usr/share/edk2/${OVMF_ARCH}/OVMF_CODE.fd"
    OVMF_VARS="/usr/share/edk2/${OVMF_ARCH}/OVMF_VARS.fd"
    USE_SPLIT=true
elif [ -f "/usr/share/ovmf/${OVMF_ARCH}/OVMF_CODE.4m.fd" ]; then
    OVMF_CODE="/usr/share/ovmf/${OVMF_ARCH}/OVMF_CODE.4m.fd"
    OVMF_VARS="/usr/share/ovmf/${OVMF_ARCH}/OVMF_VARS.4m.fd"
    USE_SPLIT=true
else
    echo "Error: OVMF firmware not found for $OVMF_ARCH!"
    echo "Try installing: edk2-ovmf, edk2-aarch64, or edk2-arm"
    exit 1
fi

echo "Using OVMF firmware at: $OVMF_CODE"

rm -rf "$ESP_DIR"
mkdir -p "$ESP_DIR/EFI/BOOT"

if [ ! -f "$BUILD_DIR/Snake.efi" ]; then
    echo "Error: Snake.efi not found at $BUILD_DIR/Snake.efi"
    exit 1
fi

cp "$BUILD_DIR/Snake.efi" "$ESP_DIR/EFI/BOOT/$BOOT_FILE"
echo "Copied Snake.efi to ESP as $BOOT_FILE"

echo "Starting QEMU ($QEMU_ARCH)..."
QEMU_COMMON="-m 512M -net none -vga std"
QEMU_EXTRA="-enable-kvm"

if [ "$USE_SPLIT" = true ]; then
    cp "$OVMF_VARS" "$ESP_DIR/OVMF_VARS.fd"

    qemu-system-${QEMU_ARCH} \
      -drive if=pflash,format=raw,readonly=on,file="$OVMF_CODE" \
      -drive if=pflash,format=raw,file="$ESP_DIR/OVMF_VARS.fd" \
      -drive format=raw,file=fat:rw:$ESP_DIR \
      $QEMU_COMMON $QEMU_EXTRA
else
    qemu-system-${QEMU_ARCH} \
      -bios "$OVMF_CODE" \
      -drive format=raw,file=fat:rw:$ESP_DIR \
      $QEMU_COMMON $QEMU_EXTRA
fi
