#!/bin/bash
set -xu

CC=clang
CFLAGS="-std=c11 -o2 -g3 -Wall -Wextra --target=riscv32-unknown-elf -fuse-ld=lld -fno-stack-protector -ffreestanding -nostdlib"

$CC $CFLAGS -Wl,-Tkernel.ld -Wl,-Map=kernel.map -o kernel.elf kernel.c common.c

QEMU=qemu-system-riscv32
$QEMU -machine virt -bios none -nographic -serial mon:stdio --no-reboot
qemu-system-riscv32 -machine virt -bios default -nographic -serial mon:stdio --no-reboot -kernel kernel.elf





