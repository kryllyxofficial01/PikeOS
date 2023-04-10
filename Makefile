ASM=nasm
WCC=/usr/bin/watcom/binl/wcc
LD=/usr/bin/watcom/binl/wlink

SRC=src
BUILD=build
TOOLS=tools

.PHONY: all floppy kernel bootloader clean mkbuild

all: floppy

# Floppy Image
floppy: $(BUILD)/floppy.img
$(BUILD)/floppy.img: bootloader kernel
	dd if=/dev/zero of=$(BUILD)/floppy.img bs=512 count=2880
	mkfs.fat -F 12 -n "PikeOS" $(BUILD)/floppy.img
	dd if=$(BUILD)/bootloader.bin of=$(BUILD)/floppy.img conv=notrunc
	mcopy -i $(BUILD)/floppy.img $(BUILD)/stage2.bin "::stage2.bin"
	mcopy -i $(BUILD)/floppy.img $(BUILD)/kernel.bin "::kernel.bin"

# Bootloader
bootloader: stage1 stage2

stage1: $(BUILD)/stage1.bin
$(BUILD)/stage1.bin: mkbuild
	$(MAKE) -C $(SRC)/bootloader/stage1 BUILD_DIR=$(abspath $(BUILD))

stage2: $(BUILD)/stage2.bin
$(BUILD)/stage2.bin: mkbuild
	$(MAKE) -C $(SRC)/bootloader/stage2 BUILD_DIR=$(abspath $(BUILD))

# Kernel
kernel: $(BUILD)/kernel.bin
$(BUILD)/kernel.bin: mkbuild
	$(MAKE) -C $(SRC)/kernel BUILD_DIR=$(abspath $(BUILD))

# Misc.
mkbuild:
	mkdir -p $(BUILD)

clean:
	$(MAKE) -C $(SRC)/bootloader/stage1 BUILD_DIR=$(abspath $(BUILD)) clean
	$(MAKE) -C $(SRC)/bootloader/stage2 BUILD_DIR=$(abspath $(BUILD)) clean
	$(MAKE) -C $(SRC)/kernel BUILD_DIR=$(abspath $(BUILD)) clean
	rm -rf $(BUILD)/*

# fat: $(BUILD)/tools/fat
# $(BUILD)/tools/fat: mkbuild $(TOOLS)/fat/fat.c
# 	mkdir -p $(BUILD)/tools
# 	gcc -g -o $(BUILD)/tools/fat $(TOOLS)/fat/fat.c