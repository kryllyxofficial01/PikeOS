#pragma once

#include "int.h"

#define MEMORY_MIN 0x00000500
#define MEMORY_MAX 0x00080000

#define FAT_MEMORY_ADDRESS 0x00500000L
#define FAT_MEMORY_SIZE 0x00010000

#define NULL ((void*)0)

void far* memcpy(const void far* source, void far* destination, uint16_t length);
void far* memset(void far* pointer, int value, uint16_t size);
bool memcmp(const void far* pointer1, const void far* pointer2, uint16_t length);
