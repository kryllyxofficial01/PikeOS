#pragma once

#include "std/int.h"
#include "disk.h"

#define SECTOR_SIZE 512
#define MAX_FILE_HANDLES 10
#define ROOT_DIRECTORY_HANDLE -1
#define MAX_PATH_LENGTH 256

#pragma pack(push, 1)
typedef struct {
    uint8_t name[11];
    uint8_t attributes;
    uint8_t _reserved;
    uint8_t createdTimeInTenths;
    uint16_t creationTime;
    uint16_t creationDate;
    uint16_t accessedDate;
    uint16_t highClusterBits;
    uint16_t modifiedTime;
    uint16_t modifiedDate;
    uint16_t lowClusterBits;
    uint32_t size;
} DirectoryEntry;
#pragma pack(pop)

typedef struct {
    int handle;
    bool isDirectory;
    uint32_t position;
    uint32_t size;

} File;

enum Attributes {
    READ_ONLY = 0x01,
    HIDDEN = 0x02,
    SYSTEM = 0x04,
    VOLUME_ID = 0x08,
    DIRECTORY = 0x10,
    ARCHIVE = 0x20,
    FLAGS = READ_ONLY | HIDDEN | SYSTEM | VOLUME_ID
};

bool FAT_init(Disk* disk);
File far* open_file(Disk* disk, const char* path);
uint32_t read_file(Disk* disk, File far* file, uint32_t bytes, void* buffer);
bool readEntry(Disk* disk, File far* file, DirectoryEntry* entry);
void close_file(File far* file);
