#!/bin/sh
set -e
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp sysroot/boot/waros.kernel isodir/boot/waros.kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "waros" {
	multiboot /boot/waros.kernel
}
EOF
grub2-mkrescue -o waros.iso isodir
