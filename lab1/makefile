all:
	clang -mcpu=cortex-a53 --target=aarch64-rpi3-elf -c a.S -o a.o
	clang -mcpu=cortex-a53 --target=aarch64-rpi3-elf -c SimpleShell.c -o SimpleShell.o
	ld.lld -m aarch64elf -T linker.ld -o kernel8.elf a.o SimpleShell.o
	llvm-objcopy --output-target=aarch64-rpi3-elf -O binary kernel8.elf kernel8.img
run:
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -display none -serial null -serial stdio