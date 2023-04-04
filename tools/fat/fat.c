#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef uint8_t bool;
#define true 1
#define false 0

typedef struct {
    uint8_t boot_jump[3];
    uint8_t OEM[8];
    uint16_t sectorBytes;
    uint8_t clusterSectors;
    uint16_t reservedSectors;
    uint8_t FAT_count;
    uint16_t dirEntries;
    uint16_t sectors;
    uint8_t descriptorType;
    uint16_t FAT_sectors;
    uint16_t trackSectors;
    uint16_t heads;
    uint32_t hiddenSectors;
    uint32_t largeSectors;

    uint8_t driveNumber;
    uint8_t _reserved;
    uint8_t signature;
    uint32_t volumeID;
    uint8_t volumeLabel[11];
    uint8_t systemID[8];
} __attribute__((packed)) BootSector;

typedef struct {
    uint8_t name[11];
    uint8_t attributes;
    uint8_t _reserved;
    uint8_t creationTimeInTenths;
    uint16_t creationTime;
    uint16_t creationDate;
    uint16_t accessedDate;
    uint16_t clusterHigh16Bits;
    uint16_t clusterLow16Bits;
    uint16_t modificationTime;
    uint16_t modificationDate;
    uint32_t size;
} __attribute__((packed)) DirectoryEntry;

BootSector bootSector;
uint8_t* FAT = NULL;
DirectoryEntry* rootDirectory = NULL;

bool readSector(FILE* disk) {
    return fread(&bootSector, sizeof(bootSector), 1, disk) > 0;
}

bool readSectors(FILE* disk, uint32_t lba, uint32_t count, void* outBuffer) {
    bool ok;
    ok = true && (fseek(disk, lba*bootSector.sectorBytes, SEEK_SET) == 0);
    ok = ok && (fread(outBuffer, bootSector.sectorBytes, count, disk) == count);
    return ok;
}

bool readFAT(FILE* disk) {
    FAT = (uint8_t*) malloc(bootSector.FAT_sectors*bootSector.sectorBytes);
    return readSectors(disk, bootSector.reservedSectors, bootSector.FAT_sectors, FAT);
}

bool readRootDir(FILE* disk) {
    uint32_t lba = bootSector.reservedSectors + bootSector.FAT_sectors*bootSector.FAT_count;
    uint32_t size = sizeof(DirectoryEntry) * bootSector.dirEntries;
    uint32_t sectors = size / bootSector.sectorBytes;
    sectors += sectors % bootSector.sectorBytes > 0 ? 1 : 0;

    rootDirectory = (DirectoryEntry*) malloc(sectors*bootSector.sectorBytes);
    return readSectors(disk, lba, sectors, rootDirectory);
}

DirectoryEntry* findFile(const char* name) {
    for (uint32_t i = 0; i < bootSector.dirEntries; i++) {
        if (memcmp(name, rootDirectory[i].name, 11) == 0) {
            return &rootDirectory[i];
        }
    }

    return NULL;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Syntax: %s <disk image> <file name>\n", argv[0]);
        return -1;
    }

    FILE* disk = fopen(argv[1], "rb");

    if (!disk) {
        fprintf(stderr, "Cannot read disk image '%s'\n", argv[0]);
        return -1;
    }

    if (!readSector(disk)) {
        fprintf(stderr, "Could not read boot sector\n");
        return -1;
    }

    if (!readFAT(disk)) {
        fprintf(stderr, "Could not read FAT\n");
        free(FAT);
        return -1;
    }

    if (!readRootDir(disk)) {
        fprintf(stderr, "Could not read root directory\n");
        free(FAT);
        free(rootDirectory);
        return -1;
    }

    DirectoryEntry* fileEntry = findFile(argv[2]);
    if (!fileEntry) {
        fprintf(stderr, "Could not find '%s'\n", argv[2]);
        free(FAT);
        free(rootDirectory);
        return -1;
    }

    free(FAT);
    free(rootDirectory);
    return 0;
}