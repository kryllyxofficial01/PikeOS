ASM=nasm
SRC=src
BUILD=build

$(BUILD)/main_floppy.img: $(BUILD)/main.bin
	cp $(BUILD)/main.bin $(BUILD)/main_floppy.img
	truncate -s 1440k $(BUILD)/main_floppy.img

$(BUILD)/main.bin: $(SRC)/main.asm
	$(ASM) $(SRC)/main.asm -f bin -o $(BUILD)/main.bin