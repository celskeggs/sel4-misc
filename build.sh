#!/bin/bash -e
make kernel_elf
if [ ! -e cbuild ]; then mkdir cbuild; cd cbuild; cmake ..; cd ..; fi
cd cbuild
make
make install
cd ..
make simulate-ia32
