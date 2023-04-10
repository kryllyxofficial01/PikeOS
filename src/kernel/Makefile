BUILD?=build/
ASM?=nasm

.PHONY: all kernel clean

all: kernel

stage1: $(BUILD)/kernel.bin
$(BUILD)/kernel.bin:
	$(ASM) kernel.asm -f bin -o $(BUILD)/kernel.bin

clean:
	rm -f $(BUILD)/kernel.bin