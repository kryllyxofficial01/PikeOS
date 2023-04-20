#include "std/io.h"
#include "std/string.h"
#include "std/memory.h"
#include "std/ctype.h"
#include "std/utility.h"

#include "fat.h"
#include "disk.h"

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

typedef struct {
    uint8_t buffer[SECTOR_SIZE];
    File public;
    bool opened;
    uint32_t firstCluster;
    uint32_t currentCluster;
    uint32_t currentSectorInCluster;
} FileData;

typedef struct {
    union {
        BootSector bootSector;
        uint8_t bytes[SECTOR_SIZE];
    } boot_sector;

    FileData rootDirectory;
    FileData openedFiles[MAX_FILE_HANDLES];
} Data;

static Data far* data;
static uint8_t far* FAT = NULL;
static uint32_t dataLBA;

bool readBootSector(Disk* disk) {
    return disk_readSectors(disk, 0, 1, data->boot_sector.bytes);
}

bool readFAT(Disk* disk) {
    return disk_readSectors(
        disk, data->boot_sector.bootSector.reservedSectors,
        data->boot_sector.bootSector.sectorsPerFAT, FAT
    );
}

bool FAT_init(Disk* disk) {
    data = (Data far*) FAT_MEMORY_ADDRESS;

    printf("%lx\r\n", data);

    if (!readBootSector(disk)) {
        printf("Error: Failed to read from boot sector\r\n");
        return false;
    }

    FAT = (uint8_t far*) data + sizeof(Data);
    uint32_t FAT_size = data->boot_sector.bootSector.bytesPerSector * data->boot_sector.bootSector.sectorsPerFAT;
    if (sizeof(Data) + FAT_size >= FAT_MEMORY_SIZE) {
        printf("Error: Not enough memory for FAT system\nNeed %lu, but found only %lu\r\n", sizeof(Data) + FAT_size, FAT_MEMORY_SIZE);
        return false;
    }

    if (!readFAT(disk)) {
        printf("Error: Failed to read from FAT\r\n");
        return false;
    }

    uint32_t rootDirectoryLBA = data->boot_sector.bootSector.reservedSectors + data->boot_sector.bootSector.sectorsPerFAT * data->boot_sector.bootSector.FAT_Count;
    uint32_t rootDirectorySize = sizeof(DirectoryEntry) * data->boot_sector.bootSector.dirEntries;

    data->rootDirectory.public.handle = ROOT_DIRECTORY_HANDLE;
    data->rootDirectory.public.isDirectory = true;
    data->rootDirectory.public.position = 0;
    data->rootDirectory.public.size = sizeof(DirectoryEntry) * data->boot_sector.bootSector.dirEntries;
    data->rootDirectory.opened = true;
    data->rootDirectory.firstCluster = rootDirectoryLBA;
    data->rootDirectory.currentCluster = rootDirectoryLBA;
    data->rootDirectory.currentSectorInCluster = 0;

    if (!disk_readSectors(disk, rootDirectoryLBA, 1, data->rootDirectory.buffer)) {
        printf("Error: Failed to read from root directory\r\n");
        return false;
    }

    uint32_t rootDirectorySectors = (rootDirectorySize + data->boot_sector.bootSector.bytesPerSector - 1) / data->boot_sector.bootSector.bytesPerSector;
    dataLBA = rootDirectoryLBA + rootDirectorySectors;

    for (int i = 0; i < MAX_FILE_HANDLES; i++) {
        data->openedFiles[i].opened = false;
    }

    return true;
}

uint32_t clusterToLBA(uint32_t cluster) {
    return dataLBA + (cluster-2) * data->boot_sector.bootSector.sectorsPerCluster;
}

File far* open_entry(Disk* disk, DirectoryEntry* entry) {
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
    file->public.handle = handle;
    file->public.isDirectory = (entry->attributes & DIRECTORY) != 0;
    file->public.position = 0;
    file->public.size = entry->size;
    file->firstCluster = entry->lowClusterBits + ((uint32_t) entry->highClusterBits << 16);
    file->currentCluster = file->firstCluster;
    file->currentSectorInCluster = 0;

    if (!disk_readSectors(disk, clusterToLBA(file->currentCluster), 1, file->buffer)) {
        printf("Error: Failed to read from sectors\r\n");
        return false;
    }

    file->opened = true;
    return &file->public;
}

uint32_t nextCluster(uint32_t currentCluster) {
    uint32_t FAT_index = currentCluster * 3 / 2;

    if (currentCluster % 2 == 0) {
        return (*(uint16_t far*) (FAT + FAT_index)) & 0x0fff;
    }
    else {
        return (*(uint16_t far*) (FAT + FAT_index)) >> 4;
    }
}

uint32_t read_file(Disk* disk, File far* file, uint32_t bytes, void* buffer) {
    FileData far* fileData = (file->handle == ROOT_DIRECTORY_HANDLE)
        ? &data->rootDirectory
        : &data->openedFiles[file->handle];

    uint8_t* dataBuffer = (uint8_t*) buffer;

    if (!fileData->public.isDirectory) {
        bytes = min(bytes, fileData->public.size - fileData->public.position);
    }

    while (bytes > 0) {
        uint32_t leftInBuffer = SECTOR_SIZE - (fileData->public.position % SECTOR_SIZE);
        uint32_t allocated = min(bytes, leftInBuffer);

        memcpy(dataBuffer, fileData->buffer + fileData->public.position % SECTOR_SIZE, allocated);
        dataBuffer += allocated;
        fileData->public.position += allocated;
        bytes -= allocated;

        if (leftInBuffer == allocated) {
            if (fileData->public.handle == ROOT_DIRECTORY_HANDLE) {
                fileData->currentCluster++;

                if (!disk_readSectors(disk, fileData->currentCluster, 1, fileData->buffer)) {
                    printf("Error: Failed to read sectors\r\n");
                    break;
                }
            }
            else {
                if (++fileData->currentSectorInCluster >= data->boot_sector.bootSector.sectorsPerCluster) {
                    fileData->currentSectorInCluster = 0;
                    fileData->currentCluster = nextCluster(fileData->currentCluster);
                }

                if (fileData->currentCluster >= 0xff8) {
                    fileData->public.size = fileData->public.position;
                    break;
                }

                if (!disk_readSectors(disk, clusterToLBA(fileData->currentCluster) + fileData->currentSectorInCluster, 1, fileData->buffer)) {
                    printf("Error: Failed to read sectors\r\n");
                    break;
                }
            }
        }
    }

    return dataBuffer - (uint8_t*) buffer;
}

bool read_entry(Disk* disk, File far* file, DirectoryEntry* entry) {
    return read_file(disk, file, sizeof(DirectoryEntry), entry) == sizeof(DirectoryEntry);
}

void close_file(File far* file) {
    if (file->handle == ROOT_DIRECTORY_HANDLE) {
        file->position = 0;
        data->rootDirectory.currentCluster = data->rootDirectory.firstCluster;
    }
    else {
        data->openedFiles[file->handle].opened = false;
    }
}

bool find_file(Disk* disk, File far* file, const char* name, DirectoryEntry* entry) {
    DirectoryEntry _entry;
    char FAT_name[12];

    memset(FAT_name, ' ', sizeof(FAT_name));
    FAT_name[11] = '\0';

    const char* extension = strchr(name, '.');
    if (extension == NULL) {
        extension = name + 11;
    }

    for (int i = 0; i < 8 && name[i] && name + i < extension; i++) {
        FAT_name[i] = upper(name[i]);
    }

    if (extension != NULL) {
        for (int i = 0; i < 3 && extension[i + 1]; i++) {
            FAT_name[i + 8] = upper(extension[i + 1]);
        }
    }

    while (read_entry(disk, file, &_entry)) {
        if (memcmp(FAT_name, _entry.name, 11) == 0) {
            *entry = _entry;
            return true;
        }
    }

    return false;
}

File far* open_file(Disk* disk, const char* path) {
    char name[MAX_PATH_LENGTH];

    if (path[0] == '/') {
        path++;
    }

    File far* file = &data->rootDirectory.public;

    while (*path) {
        bool isLast = false;
        const char* delimiter = strchr(path, '/');
        if (delimiter != NULL) {
            memcpy(path, name, delimiter - path);
            name[delimiter - path + 1] = '\0';
            path = delimiter + 1;
        }
        else {
            unsigned length = len(path);
            memcpy(path, name, length);
            name[length + 1] = '\0';
            path += length;
            isLast = true;
        }

        DirectoryEntry entry;
        if (find_file(disk, file, name, &entry)) {
            close_file(file);

            if (!isLast && entry.attributes & DIRECTORY == 0) {
                printf("Error: '%s' not a directory\r\n", name);
                return NULL;
            }

            file = open_entry(disk, &entry);
        }
        else {
            close_file(file);
            printf("Error: Could not find '%s'\r\n", name);
            return NULL;
        }
    }

    return file;
}
