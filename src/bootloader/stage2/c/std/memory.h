#pragma once

#include "int.h"

#define MEMORY_MIN 0x00000500
#define MEMORY_MAX 0x00080000

#define FAT_MEMORY_ADDRESS ((void far*) 0x00500000)
#define FAT_MEMORY_SIZE 0x00010500

#define NULL ((void*)0)

int memcpy(void far* source, const void far* destination, uint16_t length);