#!/bin/zsh

echo 'Starting QEMU...'
qemu-system-x86_64 -cpu Skylake-Client-v4,vendor=GenuineIntel,pge -monitor stdio -s -S -bios external/uefi/UEFI64.bin -drive file=img/boot.cdr,format=raw -m 4G 
