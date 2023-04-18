#pragma once

#include "std/int.h"

typedef struct {
    uint8_t ID;
    uint16_t cylinders;
    uint16_t sectors;
    uint16_t heads;
} Disk;

bool disk_init(Disk* disk, uint8_t driveNumber);
bool disk_readSectors(Disk* disk, uint32_t LBA, uint8_t sectors, uint8_t far* buffer);
