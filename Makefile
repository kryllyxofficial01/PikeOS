ASM=nasm
CC=gcc

SRC=src
BUILD=build
TOOLS=tools

.PHONY: all floppy kernel bootloader clean mkbuild FAT

all: floppy FAT

# Floppy image
floppy: $(BUILD)/floppy.img
$(BUILD)/floppy.img: bootloader kernel
	dd if=/dev/zero of=$(BUILD)/floppy.img bs=512 count=2880
	mkfs.fat -F 12 -n "PikeOS" $(BUILD)/floppy.img
	dd if=$(BUILD)/boot.bin of=$(BUILD)/floppy.img conv=notrunc
	mcopy -i $(BUILD)/floppy.img $(BUILD)/kernel.bin "::kernel.bin"
	mcopy -i $(BUILD)/floppy.img $(TOOLS)/fat/test.txt "::test.txt"

# Bootloader
bootloader: $(BUILD)/boot.bin
$(BUILD)/boot.bin: mkbuild
	$(ASM) $(SRC)/bootloader/boot.asm -f bin -o $(BUILD)/boot.bin

# Kernel
kernel: $(BUILD)/kernel.bin
$(BUILD)/kernel.bin: mkbuild
	$(ASM) $(SRC)/kernel/kernel.asm -f bin -o $(BUILD)/kernel.bin

# Tools
FAT: $(BUILD)/tools/fat
$(BUILD)/tools/fat: mkbuild $(TOOLS)/fat/fat.c
	mkdir -p $(BUILD)/tools
	$(CC) -g $(TOOLS)/fat/fat.c -o $(BUILD)/tools/fat

# Make build directory
mkbuild:
	mkdir -p $(BUILD)

# Clean build directory
clean:
	rm -rf $(BUILD)/*