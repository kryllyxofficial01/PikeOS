BUILD?=../../../build
ASM?=nasm

.PHONY: all clean

all: stage1

stage1: $(BUILD)/stage1.bin
$(BUILD)/stage1.bin:
	pwd
	$(ASM) boot.asm -f bin -o $(BUILD)/stage1.bin

clean:
	rm -f $(BUILD)/stage1.bin