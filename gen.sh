#!/bin/zsh

echo 'Generating boot image...'
hdiutil create -fs fat32 -ov -size 50m -volname boot -format UDTO -srcfolder build img/boot.cdr