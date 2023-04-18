#pragma once

#include "std/int.h"

void _cdecl x86_write_char(char character, uint8_t page);
void _cdecl x86_div(uint64_t dividend, uint32_t divisor, uint64_t* quotient, uint32_t* remainder);
bool _cdecl x86_disk_reset(uint8_t drive);
bool _cdecl x86_disk_read(uint8_t drive, uint16_t cylinder, uint16_t sector, uint16_t head, uint8_t count, uint8_t far* buffer);
bool _cdecl x86_get_drive_params(uint8_t drive, uint8_t* driveType, uint16_t* cylinders, uint16_t* sectors, uint16_t* heads);
