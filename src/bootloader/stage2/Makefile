ASM?=nasm

BUILD?=build

.PHONY: all stage2 clean

all: stage2

stage2: $(BUILD)/bootloader/stage2/stage2.bin
$(BUILD)/bootloader/stage2/stage2.bin:
	$(ASM) main.asm -f bin -o $(BUILD)/bootloader/stage2/stage2.bin

clean:
	rm -f $(BUILD)/bootloader/stage2/stage2.bin