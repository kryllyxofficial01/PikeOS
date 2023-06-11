ASM=nasm

ASM_FLAGS=-f bin

SRC=src
BUILD=build

.PHONY: all floppy kernel bootloader clean common

# Targets
floppy: $(BUILD)/floppy.img
bootloader: $(BUILD)/bootloader/boot.bin
kernel: $(BUILD)/kernel/kernel.bin

# Create the floppy image
$(BUILD)/floppy.img: bootloader kernel
	dd if=/dev/zero of=$(BUILD)/floppy.img bs=512 count=2880
	mkfs.fat -F 12 -n "PIKEOS" $(BUILD)/floppy.img
	dd if=$(BUILD)/bootloader/boot.bin of=$(BUILD)/floppy.img conv=notrunc
	mcopy -i $(BUILD)/floppy.img $(BUILD)/kernel/kernel.bin "::kernel.bin"

# Assemble the bootloader
$(BUILD)/bootloader/boot.bin: common
	mkdir -p $(BUILD)/bootloader
	$(ASM) $(ASM_FLAGS) $(SRC)/bootloader/boot.asm -o $(BUILD)/bootloader/boot.bin

# Assemble the kernel
$(BUILD)/kernel/kernel.bin: common
	mkdir -p $(BUILD)/kernel
	$(ASM) $(ASM_FLAGS) $(SRC)/kernel/kernel.asm -o $(BUILD)/kernel/kernel.bin

# Clean out the build directory
clean:
	rm -rf $(BUILD)/*

# Misc.
common:
	mkdir -p $(BUILD)
