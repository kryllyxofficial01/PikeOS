ASM?=nasm
WCC?=/usr/bin/watcom/binl/wcc
LINKER?=/usr/bin/watcom/binl/wlink

ASM_FLAGS=-f obj
WCC_FLAGS=-4 -d3 -s -wx -ms -zl -zq

BUILD?=build/

C_SOURCES=$(wildcard *.c)
C_OBJECTS=$(patsubst %.c, $(BUILD)/stage2/c/%.obj, $(C_SOURCES))
ASM_SOURCES=$(wildcard *.asm)
ASM_OBJECTS=$(patsubst %.asm, $(BUILD)/stage2/asm/%.obj, $(ASM_SOURCES))

.PHONY: all stage2 clean mkbuild

all: stage2

stage2: $(BUILD)/stage2.bin $(ASM_OBJECTS) $(C_OBJECTS)
$(BUILD)/stage2.bin:
	$(LINKER) NAME $(BUILD)/stage2.bin FILE \{$(ASM_OBJECTS) $(C_OBJECTS)\} OPTION MAP=$(BUILD)/stage2.map @linker.lnk

$(BUILD)/stage2/c/%.obj: %.c mkbuild
	$(WCC) -fo=$@ $<

$(BUILD)/stage2/asm/%.obj: %.asm mkbuild
	$(ASM) $(ASM_FLAGS) -o $@ $<

mkbuild:
	mkdir -p $(BUILD)/stage2/c
	mkdir -p $(BUILD)/stage2/asm

clean:
	rm -f $(BUILD)/stage2.bin