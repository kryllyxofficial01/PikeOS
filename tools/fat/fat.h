#ifndef _FAT_H
#define _FAT_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef uint8_t bool;

#define true 1
#define false 0

typedef struct {
	uint8_t boot_jump_instruction[3];

	// BIOS Parameter Block (BPB)
	uint8_t oem_identifier[8];
	uint16_t bytes_per_sector;
	uint8_t sectors_per_cluster;
	uint16_t reserved_sectors;
	uint8_t FAT_count;
	uint16_t dir_entry_count;
	uint16_t total_sectors;
	uint8_t media_descriptor_type;
	uint16_t sectors_per_FAT;
	uint16_t sectors_per_track;
	uint16_t heads;
	uint32_t hidden_sectors;
	uint32_t large_sectors;

	// Extended Boot Record (EBR)
	uint8_t drive_number;
	uint8_t reserved;
	uint8_t signature;
	uint32_t volume_ID;
	uint8_t volume_label[11];
	uint8_t system_ID[8];
} __attribute__((packed)) BootSector;

typedef struct {
	uint8_t name[11];
	uint8_t attributes;
	uint8_t _reserved;
	uint8_t creation_time_in_tenths;
	uint16_t creation_time;
	uint16_t creation_date;
	uint16_t date_accessed;
	uint16_t first_cluster_high;
	uint16_t modification_time;
	uint16_t modification_date;
	uint16_t first_cluster_low;
	uint32_t size;
} __attribute__((packed)) DirectoryEntry;

bool readBootSector(FILE* disk);
bool readSectors(FILE* disk, uint32_t LBA, uint32_t count, void* buffer);
bool readFAT(FILE* disk);
bool readRootDirectory(FILE* disk);
bool readFile(FILE* disk, DirectoryEntry* file_entry, uint8_t* buffer);

DirectoryEntry* findFile(const char* name);

#endif
