#include "std/io.h"
#include "std/string.h"
#include "std/memory.h"
#include "fat.h"

static Data far* data;
static uint8_t far* FAT = NULL;
static uint32_t dataLBA;

bool readBootSector(Disk* disk) {
    return readSectors(disk, 0, 1, data->boot_sector.bytes);
}

bool readSectors(Disk* disk, uint32_t LBA, uint32_t sectors, void* buffer) {
    bool success = true && (fseek(disk, LBA * data->boot_sector.bootSector.bytesPerSector, SEEK_SET) == 0);
    success = success && (fread(buffer, data->boot_sector.bootSector.bytesPerSector, sectors, disk) == sectors);

    return success;
}

bool readFAT(Disk* disk) {
    readSectors(
        disk, data->boot_sector.bootSector.reservedSectors,
        data->boot_sector.bootSector.sectorsPerFAT, FAT
    );
}

bool readFile(DirectoryEntry* fileEntry, Disk* disk, uint8_t* buffer) {
    uint16_t currentCluster = fileEntry->lowClusterBits;
    bool success = true;

    do {
        uint32_t LBA = rootDirectoryEnd + (currentCluster-2) * data->boot_sector.bootSector.sectorsPerCluster;
        success = success && readSectors(disk, LBA, data->boot_sector.bootSector.sectorsPerCluster, buffer);
        buffer += data->boot_sector.bootSector.sectorsPerCluster * data->boot_sector.bootSector.bytesPerSector;

        uint32_t FAT_index = currentCluster * 3 / 2;
        if (currentCluster % 2 == 0)
            currentCluster = (*(uint16_t*)(FAT + FAT_index)) & 0x0fff;
        else
            currentCluster = (*(uint16_t*)(FAT + FAT_index)) >> 4;

    } while (success && currentCluster < 0x0ff8);

    return success;
}

bool findFile(DirectoryEntry* entry, File far* file, const char* name) {
    for (uint32_t i = 0; i < data->boot_sector.bootSector.dirEntries; i++) {
        if (memcmp(name, rootDirectory[i].name, 11) == 0) {
            return &rootDirectory[i];
        }
    }

    return NULL;
}

bool FAT_init(Disk* disk) {
    data = (Data far*) FAT_MEMORY_ADDRESS;

    if (!readBootSector(disk)) {
        printf("Error: Failed to read boot sector\r\n");
        return false;
    }

    FAT = (uint8_t far*) data + sizeof(Data);
    uint32_t FAT_size = *data->boot_sector.bytes * data->boot_sector.bootSector.sectorsPerFAT;
    FAT_size += sizeof(Data);

    if (FAT_size >= FAT_MEMORY_SIZE) {
        printf("Error: Not enough memory for FAT\r\nNeed %lu, found only %lu\r\n", FAT_size, FAT_MEMORY_SIZE);
        return false;
    }

    if (!readFAT(disk)) {
        printf("Error: Unable to read FAT\r\n");
        return false;
    }

    uint32_t rootDirectoryLBA = data->boot_sector.bootSector.reservedSectors;
    rootDirectoryLBA += data->boot_sector.bootSector.sectorsPerFAT * data->boot_sector.bootSector.FAT_Count;
    uint32_t rootDirectorySize = sizeof(DirectoryEntry) + data->boot_sector.bootSector.dirEntries;

    data->rootDirectory.opened = true;
    data->rootDirectory.publicData.handle = ROOT_DIRECTORY_HANDLE;
    data->rootDirectory.publicData.isDirectory = true;
    data->rootDirectory.publicData.position = 0;
    data->rootDirectory.publicData.size = sizeof(DirectoryEntry) + data->boot_sector.bootSector.dirEntries;
    data->rootDirectory.currentCluster = 0;
    data->rootDirectory.currentClusterSector = 0;
    data->rootDirectory.firstCluster = 0;

    if (!readSectors(disk, rootDirectoryLBA, 1, data->rootDirectory.buffer)) {
        printf("Error: Unable to read from the root directory\r\n");
        return false;
    }

    uint32_t rootDirectorySectors = (rootDirectorySize + data->boot_sector.bootSector.bytesPerSector - 1);
    rootDirectorySectors /= data->boot_sector.bootSector.bytesPerSector;
    dataLBA = rootDirectoryLBA + rootDirectorySectors;

    for (int i = 0; i < MAX_FILE_HANDLES; i++) {
        data->openedFiles[i].opened = true;
    }
}

uint32_t clusterToLBA(uint32_t cluster) {
    return dataLBA + (cluster-2) * data->boot_sector.bootSector.sectorsPerCluster;
}

File far* openEntry(Disk* disk, DirectoryEntry* entry) {
    int handle = -1;
    for (int i = 0; i < MAX_FILE_HANDLES && handle < 0; i++) {
        if (!data->openedFiles[i].opened) {
            handle = i;
        }
    }

    if (handle < 0) {
        printf("Error: Out of file handles\r\n");
        return false;
    }

    FileData far* file = &data->openedFiles[handle];
    file->publicData.handle = handle;
    file->publicData.isDirectory = entry->attributes & DIRECTORY != 0;
    file->publicData.position = 0;
    file->publicData.size = 0;
    file->firstCluster = entry->lowClusterBits + ((uint32_t) entry->highClusterBits << 16);
    file->currentCluster = file->firstCluster;
    file->currentClusterSector = 0;

    if (!readSectors(disk, clusterToLBA(file->currentCluster), 1, file->buffer)) {
        printf("Error: Unable to read sector\r\n");
        return false;
    }
}

File far* open_file(Disk* disk, const char* path) {
    char name[MAX_PATH_LENGTH];

    if (path[0] == '/') {
        path++;
    }

    File far* parent = NULL;
    File far* file = &data->rootDirectory.publicData;

    bool isLast;
    while (*path) {
        const char* delimiter = find_char(path, '/');
        isLast = false;

        if (delimiter != NULL) {
            memcpy(name, path, delimiter-path);
            name[delimiter - path + 1] = '\0';
            path = delimiter + 1;
        }
        else {
            unsigned length = len(path);
            memcpy(name, path, length);
            name[length] = '\0';
            isLast = true;

            path += length;
        }

        DirectoryEntry* entry;
        if (findFile(entry, file, name)) {
            close_file(file);

            if (!isLast && entry->attributes & DIRECTORY != 0) {
                printf("Error: '%s' is not a directory\r\n", name);
                return NULL;
            }

            file = openEntry(disk, entry);
        }
        else {
            close_file(file);
            printf("Error: Could not find '%s'\r\n", name);
            return NULL;
        }
    }

    return file;
}