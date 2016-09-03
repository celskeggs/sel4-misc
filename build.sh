#!/bin/bash -e

# these additional packages are required (Arch Linux names): ccache, python2-tempita, qemu

make kernel_elf
if [ ! -e cbuild ]; then mkdir cbuild; cd cbuild; cmake ..; cd ..; fi
cd cbuild
make
make install
cd ..
echo "Use Ctrl-A x to quit qemu"
make simulate-ia32
