ASM=nasm

ASM_FLAGS=-f bin

SRC=src
BUILD=build

$(BUILD)/floppy.img: $(BUILD)/main.bin
	cp $(BUILD)/main.bin $(BUILD)/floppy.img
	truncate -s 1440k $(BUILD)/floppy.img

$(BUILD)/main.bin: $(SRC)/main.asm
	$(ASM) $(ASM_FLAGS) $(SRC)/main.asm -o $(BUILD)/main.bin
