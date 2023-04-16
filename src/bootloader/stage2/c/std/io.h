#pragma once

#define PRINTF_NORMAL_STATE 0
#define PRINTF_LENGTH_STATE 1
#define PRINTF_SHORT_LENGTH_STATE 2
#define PRINTF_LONG_LENGTH_STATE 3
#define PRINTF_SPEC_STATE 4

#define PRINTF_DEFAULT_LENGTH 0
#define PRINTF_SHORT_SHORT_LENGTH 1
#define PRINTF_SHORT_LENGTH 2
#define PRINTF_LONG_LENGTH 3
#define PRINTF_LONG_LONG_LENGTH 4

void putc(char);
void puts(const char*);
void _cdecl printf(const char*, ...);
