#include "std/int.h"
#include "std/io.h"
#include "disk.h"
#include "fat.h"

void _cdecl cstart_(uint16_t bootDrive) {
    Disk disk;
    if(!disk_init(&disk, bootDrive)) {
        printf("Disk initiation failed...\r\n");
        goto end;
    }

    if (!FAT_init(&disk)) {
        printf("Failed to initiate FAT system...\r\n");
        goto end;
    }

    File far* file = open_file(&disk, "/");
    DirectoryEntry entry;

    while (readEntry(&disk, file, &entry)) {
        printf(" ");
        for (int i = 0; i < 11; i++) {
            putc(entry.name[i]);
        }
        printf("\r\n");
    }
    close_file(file);

    end: for (;;);
}
