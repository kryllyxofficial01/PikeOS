ASM=nasm
CC=gcc
WCC=/usr/bin/watcom/binl/wcc
LINKER=/usr/bin/watcom/binl/wlink

SRC=src
BUILD=build
TOOLS=tools

.PHONY: all floppy kernel bootloader clean mkbuild

all: floppy

# Floppy image
floppy: $(BUILD)/floppy.img
$(BUILD)/floppy.img: bootloader kernel
	dd if=/dev/zero of=$(BUILD)/floppy.img bs=512 count=2880
	mkfs.fat -F 12 -n "PikeOS" $(BUILD)/floppy.img
	dd if=$(BUILD)/bootloader/stage1/stage1.bin of=$(BUILD)/floppy.img conv=notrunc
	mcopy -i $(BUILD)/floppy.img $(BUILD)/bootloader/stage2/stage2.bin "::stage2.bin"
	mcopy -i $(BUILD)/floppy.img $(BUILD)/kernel/kernel.bin "::kernel.bin"
#	mcopy -i $(BUILD)/floppy.img $(TOOLS)/fat/test.txt "::test.txt"

# Bootloader
bootloader: stage1 stage2
stage1: $(BUILD)/bootloader/stage1/stage1.bin
stage2: $(BUILD)/bootloader/stage2/stage2.bin

$(BUILD)/bootloader/stage1/stage1.bin: mkbuild
	mkdir -p $(BUILD)/bootloader/stage1
	$(MAKE) -C $(SRC)/bootloader/stage1 BUILD=$(abspath $(BUILD))

$(BUILD)/bootloader/stage2/stage2.bin: mkbuild
	mkdir -p $(BUILD)/bootloader/stage2
	$(MAKE) -C $(SRC)/bootloader/stage2 BUILD=$(abspath $(BUILD))

# Kernel
kernel: $(BUILD)/kernel/kernel.bin
$(BUILD)/kernel/kernel.bin: mkbuild
	mkdir -p $(BUILD)/kernel
	$(MAKE) -C $(SRC)/kernel BUILD=$(abspath $(BUILD))

# Tools
# FAT: $(BUILD)/tools/fat
# $(BUILD)/tools/fat: mkbuild $(TOOLS)/fat/fat.c
#	mkdir -p $(BUILD)/tools
#	$(CC) -g $(TOOLS)/fat/fat.c -o $(BUILD)/tools/fat

# Make build directory
mkbuild:
	mkdir -p $(BUILD)

# Clean build directory
clean:
	$(MAKE) -C $(SRC)/bootloader/stage1 BUILD=$(abspath $(BUILD)) clean
	$(MAKE) -C $(SRC)/bootloader/stage2 BUILD=$(abspath $(BUILD)) clean
	$(MAKE) -C $(SRC)/kernel BUILD=$(abspath $(BUILD)) clean
	rm -rf $(BUILD)/*
