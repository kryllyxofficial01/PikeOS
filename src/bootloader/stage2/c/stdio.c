#include "std/io.h"
#include "x86.h"

void putc(char character) {
    write_char(character, 0);
}

void puts(const char* string) {
    while (*string) {
        putc(*string);
        string++;
    }
}