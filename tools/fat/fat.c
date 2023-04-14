#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "fat.h"

BootSector bootSector;
uint8_t* FAT = NULL;
DirectoryEntry* rootDirectory = NULL;

// Read from a boot sector of a disk
boolean readBootSector(FILE* disk) {
    return fread(&bootSector, sizeof(bootSector), 1, disk) > 0;
}

// Read from multiple sectors of a disk
boolean readSectors(FILE* disk, uint32_t LBA, uint32_t sectors, void* buffer) {
    boolean succeeded = true && fseek(disk, LBA*bootSector.bytesPerSector, SEEK_SET) == 0;
    succeeded = succeeded && fread(buffer, bootSector.bytesPerSector, sectors, disk) == sectors;
    return succeeded;
}

// Read from the File Allocation Tables on a disk
boolean readFAT(FILE* disk) {
    FAT = (uint8_t*) malloc(bootSector.sectorsPerFAT * bootSector.bytesPerSector);
    return readSectors(disk, bootSector.reservedSectors, bootSector.sectorsPerFAT, FAT);
}

// Read from the root directory of a disk
boolean readRootDirectory(FILE* disk) {
    uint32_t LBA = bootSector.reservedSectors + bootSector.sectorsPerFAT * bootSector.FAT_Count;
    uint32_t size = sizeof(DirectoryEntry) * bootSector.dirEntries;
    uint32_t sectors = size / bootSector.bytesPerSector;

    if (size % bootSector.bytesPerSector > 0) {
        sectors++;
    }

    rootDirectory = (DirectoryEntry*) malloc(sectors * bootSector.bytesPerSector);
    return readSectors(disk, LBA, sectors, rootDirectory);
}

DirectoryEntry* findFile(const char* name) {
    for (uint32_t i = 0; i < bootSector.dirEntries; i++) {
        if (memcmp(name, rootDirectory[i].name, 11) == 0) {
            return &rootDirectory[i];
        }
    }

    return NULL;
}

int main(int argc, char const *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Syntax: %s <disk image> <filename>", argv[0]);
        return -1;
    }

    FILE* disk = fopen(argv[1], "rb");

    if (!disk) {
        fprintf(stderr, "Could not read from '%s'", argv[1]);
        return -1;
    }

    if (!readBootSector(disk)) {
        fprintf(stderr, "Could not read from boot sector");
        return -1;
    }

    if (!readFAT(disk)) {
        fprintf(stderr, "Could not read from the FAT");

        free(FAT);
        free(rootDirectory);;

        return -1;
    }

    if (!readRootDirectory(disk)) {
        fprintf(stderr, "Could not read from the root directory");

        free(FAT);
        free(rootDirectory);

        return -1;
    }

    DirectoryEntry* fileEntry = findFile(argv[2]);

    if (!fileEntry) {
        fprintf(stderr, "Could not find the file '%s'", argv[2]);

        free(FAT);
        free(rootDirectory);

        return -1;
    }

    free(FAT);
    free(rootDirectory);

    return 0;
}