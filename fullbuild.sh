#/bin/zsh
echo 'Running fullbuild...'
./clean.sh glyphs && ./build.sh ../cross-compile/bin && ./gen.sh