#!/bin/zsh

echo 'Cleaning working directory...'
rm -rf build
rm -rf img
mkdir img
mkdir build
mkdir build/kernel
mkdir build/efi
mkdir build/efi/boot
mkdir build/config
mkdir build/init
cp external/bootloader/BOOTX64.EFI build/efi/boot/BOOTX64.EFI
cp external/bootloader/boot.cfg build/config/boot.cfg
cp external/fonts/${1} build/init/${1}