ASM=nasm
GXX=gcc

ASM_FLAGS=-f bin
GXX_FLAGS=-g

SRC=src
BUILD=build
TOOLS=tools

.PHONY: all floppy kernel bootloader clean common fat

all: floppy fat

# Targets
floppy: $(BUILD)/floppy.img
bootloader: $(BUILD)/bootloader/boot.bin
kernel: $(BUILD)/kernel/kernel.bin
fat: $(BUILD)/tools/fat

# Create the floppy image
$(BUILD)/floppy.img: bootloader kernel
	dd if=/dev/zero of=$(BUILD)/floppy.img bs=512 count=2880
	mkfs.fat -F 12 -n "PIKEOS" $(BUILD)/floppy.img
	dd if=$(BUILD)/bootloader/boot.bin of=$(BUILD)/floppy.img conv=notrunc
	mcopy -i $(BUILD)/floppy.img $(BUILD)/kernel/kernel.bin "::kernel.bin"
	mcopy -i $(BUILD)/floppy.img $(TOOLS)/fat/test.txt "::test.txt"

# Assemble the bootloader
$(BUILD)/bootloader/boot.bin: common
	mkdir -p $(BUILD)/bootloader
	$(ASM) $(ASM_FLAGS) $(SRC)/bootloader/boot.asm -o $(BUILD)/bootloader/boot.bin

# Assemble the kernel
$(BUILD)/kernel/kernel.bin: common
	mkdir -p $(BUILD)/kernel
	$(ASM) $(ASM_FLAGS) $(SRC)/kernel/kernel.asm -o $(BUILD)/kernel/kernel.bin

$(BUILD)/tools/fat: common $(TOOLS)/fat/fat.c
	mkdir -p $(BUILD)/tools
	$(GXX) $(GXX_FLAGS) $(TOOLS)/fat/fat.c -o $(BUILD)/tools/fat

# Clean out the build directory
clean:
	rm -rf $(BUILD)/*

# Misc.
common:
	mkdir -p $(BUILD)
