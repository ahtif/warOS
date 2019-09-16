# warOS

* Developed and tested on CentOS 7 on DCS lab machines. Anything not mentioned here was installed by default.

* Required software: i686-elf toolchain to cross compile the system as detailed on the OSDev Wiki link: https://wiki.osdev.org/GCC_Cross-Compiler

* Alternatively there seems to be many scripts on github that can automatically download and install an i686-elf toolchain automatically. Available here: https://github.com/search?q=i686-elf. Note, I have not tested any of these myself.

Specific software versions used:
* binutils-2.32
* gcc 4.8.5
* qemu 3.1.0 (need to install i386 targets)
* xorriso 1.5.0

Additionally, need grub-file aliased to grub2-file

The script build.sh can be used to build the project

The script qemu.sh can be used to build and run the project using qemu
