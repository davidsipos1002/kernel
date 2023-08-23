#/bin/zsh
echo 'Running fullbuild...'
./clean.sh zap-ext-light16.psf && ./build.sh ../cross-compile/bin && ./gen.sh