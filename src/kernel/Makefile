ASM?=nasm

BUILD?=build

.PHONY: all kernel clean

all: kernel

kernel: $(BUILD)/kernel/kernel.bin
$(BUILD)/kernel/kernel.bin:
	$(ASM) kernel.asm -f bin -o $(BUILD)/kernel/kernel.bin

clean:
	rm -f $(BUILD)/kernel/kernel.bin