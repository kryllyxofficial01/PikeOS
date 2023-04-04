ASM=nasm
SRC=src
BUILD=build

.PHONY: all floppy kernel bootloader clean mkbuild

# Create floppy image
floppy: $(BUILD)/floppy.img
$(BUILD)/floppy.img: bootloader kernel
	dd if=/dev/zero of=$(BUILD)/floppy.img bs=512 count=2880
	mkfs.fat -F 12 -n "PikeOS" $(BUILD)/floppy.img
	dd if=$(BUILD)/bootloader.bin of=$(BUILD)/floppy.img conv=notrunc
	mcopy -i $(BUILD)/floppy.img $(BUILD)/kernel.bin "::kernel.bin"

# Run the bootloader
bootloader: $(BUILD)/bootloader.bin
$(BUILD)/bootloader.bin: mkbuild
	$(ASM) $(SRC)/bootloader/boot.asm -f bin -o $(BUILD)/bootloader.bin

# Start the kernel
kernel: $(BUILD)/kernel.bin
$(BUILD)/kernel.bin: mkbuild
	$(ASM) $(SRC)/kernel/main.asm -f bin -o $(BUILD)/kernel.bin

# Create the the build directory
mkbuild:
	mkdir -p $(BUILD)

# Clean out the build directory
clean:
	rm -rf $(BUILD)/*