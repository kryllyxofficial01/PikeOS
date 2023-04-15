ASM?=nasm

BUILD?=build

.PHONY: all clean

all: stage1

stage1: $(BUILD)/bootloader/stage1/stage1.bin
$(BUILD)/bootloader/stage1/stage1.bin:
	$(ASM) boot.asm -f bin -o $(BUILD)/bootloader/stage1/stage1.bin

clean:
	rm -f $(BUILD)/bootloader/stage1/stage1.bin