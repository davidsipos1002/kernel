#/bin/zsh
echo 'Running fullbuild...'
./clean.sh && ./build.sh ../cross-compile/bin && ./gen.sh