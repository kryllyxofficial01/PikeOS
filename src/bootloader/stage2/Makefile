ASM?=nasm
WCC?=/usr/bin/watcom/binl/wcc
LINKER?=/usr/bin/watcom/binl/wlink

WCC_FLAGS?=-4 -d3 -s -wx -ms -zl -zq -za99
ASM_FLAGS?=-f obj

BUILD?=build

C_SOURCES=$(wildcard c/*.c)
C_OBJECTS=$(patsubst c/%.c, $(BUILD)/bootloader/stage2/c/%.obj, $(C_SOURCES))
ASM_SOURCES=$(wildcard asm/*.asm)
ASM_OBJECTS=$(patsubst asm/%.asm, $(BUILD)/bootloader/stage2/asm/%.obj, $(ASM_SOURCES))

.PHONY: all stage2 clean mkbuild

all: stage2

stage2: $(BUILD)/bootloader/stage2/stage2.bin
$(BUILD)/bootloader/stage2/stage2.bin: $(ASM_OBJECTS) $(C_OBJECTS)
	$(LINKER) NAME $(BUILD)/bootloader/stage2/stage2.bin FILE \{$(ASM_OBJECTS) $(C_OBJECTS)\} OPTION MAP=$(BUILD)/bootloader/stage2/stage2.map @linker.lnk

$(BUILD)/bootloader/stage2/c/%.obj: c/%.c mkbuild
	$(WCC) $(WCC_FLAGS) -fo=$@ $<

$(BUILD)/bootloader/stage2/asm/%.obj: asm/%.asm mkbuild
	$(ASM) $(ASM_FLAGS) -o $@ $<

mkbuild:
	mkdir -p $(BUILD)/bootloader/stage2/c
	mkdir -p $(BUILD)/bootloader/stage2/asm

clean:
	rm -f $(BUILD)/bootloader/stage2/stage2.bin