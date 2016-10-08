#!/bin/bash -e

# these additional packages are required (Arch Linux names): ccache, python2-tempita, qemu

make kernel_elf
if [ ! -e cbuild ]; then mkdir cbuild; cd cbuild; cmake ..; cd ..; fi
cd cbuild
make install
cd ..
echo "Use Ctrl-A x to quit qemu"
qemu-system-i386 -m 512 -nographic -kernel images/kernel.elf -initrd images/init
