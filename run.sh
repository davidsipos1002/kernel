#!/bin/zsh

echo 'Starting QEMU...'
qemu-system-x86_64 -cpu Nehalem-v2 -monitor stdio -bios external/uefi/UEFI64.bin -drive file=img/boot.cdr,format=raw -vga std -full-screen -m 4G 
