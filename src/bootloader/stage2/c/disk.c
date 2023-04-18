#include "std/int.h"
#include "disk.h"
#include "x86.h"

void LBA_to_CHS(Disk* disk, uint32_t LBA, uint16_t* cylinder, uint16_t* sector, uint16_t* head) {
    *sector = LBA % disk->sectors + 1;
    *cylinder = (LBA / disk->heads) / disk->heads;
    *head = (LBA / disk->heads) % disk->heads;
}

bool disk_init(Disk* disk, uint8_t driveNumber) {
    uint8_t driveType;
    uint16_t cylinders, sectors, heads;

    if (!x86_get_drive_params(disk->ID, &driveType, &cylinders, &sectors, &heads)) {
        return false;
    }

    disk->ID = driveNumber;
    disk->cylinders = cylinders;
    disk->sectors = sectors;
    disk->heads = heads;

    return true;
}

bool disk_readSectors(Disk* disk, uint32_t LBA, uint8_t sectors, uint8_t far* buffer) {
    uint16_t cylinder, sector, head;
    LBA_to_CHS(disk, LBA, &cylinder, &sector, &head);

    for (int i = 0; i< 3; i++) {
        if (x86_disk_read(disk->ID, cylinder, sector, head, sectors, buffer)) {
            return true;
        }

        x86_disk_reset(disk->ID);
    }

    return false;
}
