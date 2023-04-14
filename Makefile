ASM=nasm

SRC=src
BUILD=build

$(BUILD)/floppy.img: $(BUILD)/main.bin
	cp $(BUILD)/main.bin $(BUILD)/floppy.img
	truncate -s 1440k $(BUILD)/floppy.img

$(BUILD)/main.bin: $(SRC)/main.asm
	$(ASM) $(SRC)/main.asm -f bin -o $(BUILD)/main.bin