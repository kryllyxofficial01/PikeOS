#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "fat.h"

typedef uint8_t bool;

BootSector bootSector;
uint8_t* FAT = NULL;
DirectoryEntry* rootDirectory = NULL;
uint32_t rootDirectoryEnd;

bool readBootSector(FILE* disk) {
    return fread(&bootSector, sizeof(bootSector), 1, disk) > 0;
}

bool readSectors(FILE* disk, uint32_t LBA, uint32_t sectors, void* buffer) {
    bool success = true && (fseek(disk, LBA * bootSector.bytesPerSector, SEEK_SET) == 0);
    success = success && (fread(buffer, bootSector.bytesPerSector, sectors, disk) == sectors);

    return success;
}

bool readFAT(FILE* disk) {
    FAT = (uint8_t*) malloc(bootSector.sectorsPerFAT * bootSector.bytesPerSector);
    return readSectors(disk, bootSector.reservedSectors, bootSector.sectorsPerFAT, FAT);
}

bool readRootDirectory(FILE* disk) {
    uint32_t LBA = bootSector.reservedSectors + bootSector.sectorsPerFAT * bootSector.FAT_Count;
    uint32_t size = sizeof(DirectoryEntry) * bootSector.dirEntries;
    uint32_t sectors = (size / bootSector.bytesPerSector);

    if (size % bootSector.bytesPerSector > 0) {
        sectors++;
    }

    rootDirectoryEnd = LBA + sectors;
    rootDirectory = (DirectoryEntry*) malloc(sectors * bootSector.bytesPerSector);

    return readSectors(disk, LBA, sectors, rootDirectory);
}

bool readFile(DirectoryEntry* fileEntry, FILE* disk, uint8_t* buffer) {
    uint16_t currentCluster = fileEntry->lowClusterBits;
    bool success = true;

    do {
        uint32_t LBA = rootDirectoryEnd + (currentCluster-2) * bootSector.sectorsPerCluster;
        success = success && readSectors(disk, LBA, bootSector.sectorsPerCluster, buffer);
        buffer += bootSector.sectorsPerCluster * bootSector.bytesPerSector;

        uint32_t FAT_index = currentCluster * 3 / 2;
        if (currentCluster % 2 == 0)
            currentCluster = (*(uint16_t*)(FAT + FAT_index)) & 0x0fff;
        else
            currentCluster = (*(uint16_t*)(FAT + FAT_index)) >> 4;

    } while (success && currentCluster < 0x0ff8);

    return success;
}

DirectoryEntry* find_file(const char* name) {
    for (uint32_t i = 0; i < bootSector.dirEntries; i++) {
        if (memcmp(name, rootDirectory[i].name, 11) == 0) {
            return &rootDirectory[i];
        }
    }

    return NULL;
}

int main(int argc, char const* argv[]) {
    if (argc < 3) {
        printf("Syntax: %s <disk image> <file name>\n", argv[0]);
        return -1;
    }

    FILE* disk = fopen(argv[1], "rb");
    if (!disk) {
        fprintf(stderr, "Cannot open disk image %s!\n", argv[1]);
        return -1;
    }

    if (!readBootSector(disk)) {
        fprintf(stderr, "Could not read boot sector!\n");
        return -2;
    }

    if (!readFAT(disk)) {
        fprintf(stderr, "Could not read FAT!\n");
        free(FAT);
        return -3;
    }

    if (!readRootDirectory(disk)) {
        fprintf(stderr, "Could not read FAT!\n");
        free(FAT);
        free(rootDirectory);
        return -4;
    }

    DirectoryEntry* fileEntry = find_file(argv[2]);
    if (!fileEntry) {
        fprintf(stderr, "Could not find file %s!\n", argv[2]);
        free(FAT);
        free(rootDirectory);
        return -5;
    }

    uint8_t* buffer = (uint8_t*) malloc(fileEntry->size + bootSector.bytesPerSector);
    if (!readFile(fileEntry, disk, buffer)) {
        fprintf(stderr, "Could not read file %s!\n", argv[2]);
        free(FAT);
        free(rootDirectory);
        free(buffer);
        return -5;
    }

    for (size_t i = 0; i < fileEntry->size; i++)
    {
        if (isprint(buffer[i])) {
            fputc(buffer[i], stdout);
        }
        else {
            printf("<%02x>", buffer[i]);
        }
    }
    printf("\n");

    free(buffer);
    free(FAT);
    free(rootDirectory);

    return 0;
}