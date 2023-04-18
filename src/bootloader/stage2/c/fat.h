#pragma once

#include "std/int.h"
#include "disk.h"

#define SECTOR_SIZE 512
#define MAX_FILE_HANDLES 10
#define ROOT_DIRECTORY_HANDLE -1
#define MAX_PATH_LENGTH 256

#pragma pack(push, 1)
typedef struct {
    uint8_t bootJumpInstruction[3];
    uint8_t OEM_Identifier[8];
    uint16_t bytesPerSector;
    uint8_t sectorsPerCluster;
    uint16_t reservedSectors;
    uint8_t FAT_Count;
    uint16_t dirEntries;
    uint16_t sectors;
    uint8_t mediaDescriptorType;
    uint16_t sectorsPerFAT;
    uint16_t sectorsPerTrack;
    uint16_t heads;
    uint32_t hiddenSectors;
    uint32_t largeSectors;

    uint8_t driveNumber;
    uint8_t _reserved;
    uint8_t signature;
    uint32_t volumeID;
    uint8_t volumeLabel[11];
    uint8_t systemID[8];
} BootSector;
#pragma pack(pop)

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

typedef struct {
    File publicData;
    bool opened;
    uint32_t currentCluster;
    uint32_t currentClusterSector;
    uint32_t firstCluster;
    uint8_t buffer[SECTOR_SIZE];
} FileData;

typedef struct {
    union {
        BootSector bootSector;
        uint8_t bytes[SECTOR_SIZE];
    } boot_sector;

    FileData rootDirectory;
    FileData openedFiles[MAX_FILE_HANDLES];
} Data;

bool FAT_init(Disk* disk);
File far* open_file(Disk* disk, const char* path);
void close_file(File far* file);
uint32_t read_file(Disk* disk, File far* file, uint32_t bytes, void* buffer);
bool readEntry(Disk* disk, File far* file, DirectoryEntry entry);
