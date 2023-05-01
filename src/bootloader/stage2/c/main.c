#include "std/int.h"
#include "std/io.h"
#include "disk.h"
#include "fat.h"

void far* data = (void far*) 0x00500200;

void _cdecl cstart_(uint16_t bootDrive) {
    Disk disk;
    if(!disk_init(&disk, bootDrive)) {
        printf("Disk initiation failed...\r\n");
        goto end;
    }

    disk_readSectors(&disk, 19, 1, data);

    if (!FAT_init(&disk)) {
        printf("Failed to initiate FAT system...\r\n");
        goto end;
    }

    File far* fileData = open_file(&disk, "/");
    DirectoryEntry entry;

    while (read_entry(&disk, fileData, &entry)) {
        printf(" ");
        for (int i = 0; i < 11; i++) {
            putc(entry.name[i]);
        }
        printf("\r\n");
    }
    close_file(fileData);

    end: for (;;);
}
