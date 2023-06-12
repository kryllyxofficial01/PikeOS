#include "fat.h"

BootSector boot_sector;
uint8_t* FAT = NULL;
DirectoryEntry* root_directory = NULL;
uint32_t root_dir_end;

int main(int argc, char** argv) {
	if (argc < 3) {
        printf("Syntax: %s <disk image> <file name>\n", argv[0]);
        return -1;
    }

    FILE* disk = fopen(argv[1], "rb");
    if (!disk) {
        fprintf(stderr, "Could not open disk image %s!\n", argv[1]);
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
        fprintf(stderr, "Could not read root directory!\n");
        
		free(FAT);
        free(root_directory);
        
		return -4;
    }

    DirectoryEntry* file_entry = findFile(argv[2]);
    if (!file_entry) {
        fprintf(stderr, "Could not find file %s!\n", argv[2]);
        
		free(FAT);
        free(root_directory);
        
		return -5;
    }

	uint8_t* buffer = (uint8_t*) malloc(file_entry->size + boot_sector.bytes_per_sector);
	if (!readFile(disk, file_entry, buffer)) {
		fprintf(stderr, "Could not read from %s!\n", argv[2]);
        
		free(FAT);
        free(root_directory);
		free(buffer);
        
		return -6;
	}

	for (size_t i = 0; i < file_entry->size-1; i++) {
		if (isprint(buffer[i])) {
			fputc(buffer[i], stdout);
		}
		else {
			printf("<%02x>", buffer[i]);
		}
	}
	printf("\n");

	free(FAT);
	free(root_directory);
	free(buffer);

	return 0;
}

bool readBootSector(FILE* disk) {
	return fread(&boot_sector, sizeof(boot_sector), 1, disk) > 0;
}

bool readSectors(FILE* disk, uint32_t LBA, uint32_t count, void* buffer) {
	bool success = true && (fseek(disk, LBA * boot_sector.bytes_per_sector, SEEK_SET) == 0);
    success = success && (fread(buffer, boot_sector.bytes_per_sector, count, disk) == count);

    return success;
}

bool readFAT(FILE* disk) {
	FAT = (uint8_t*) malloc(boot_sector.sectors_per_FAT * boot_sector.bytes_per_sector);
    return readSectors(disk, boot_sector.reserved_sectors, boot_sector.sectors_per_FAT, FAT);
}

bool readRootDirectory(FILE* disk) {
	uint32_t LBA = boot_sector.reserved_sectors + boot_sector.sectors_per_FAT * boot_sector.FAT_count;
    uint32_t size = sizeof(DirectoryEntry) * boot_sector.dir_entry_count;
    uint32_t sectors = size / boot_sector.bytes_per_sector;

    if (size % boot_sector.bytes_per_sector > 0) {
        sectors++;
    }

	root_dir_end = LBA + sectors;
    root_directory = (DirectoryEntry*) malloc(sectors * boot_sector.bytes_per_sector);

    return readSectors(disk, LBA, sectors, root_directory);
}

bool readFile(FILE* disk, DirectoryEntry* file_entry, uint8_t* buffer) {
	uint16_t current_cluster = file_entry->first_cluster_low;

	bool ok = true;
	do {
		uint32_t LBA = root_dir_end + (current_cluster-2) * boot_sector.sectors_per_cluster;
		ok = ok && readSectors(disk, LBA, boot_sector.sectors_per_cluster, buffer);
		buffer += boot_sector.sectors_per_cluster * boot_sector.bytes_per_sector;

		uint32_t FAT_index = current_cluster*3 / 2;
		if (current_cluster % 2 == 0) {
			current_cluster = (*(uint16_t*)(FAT+FAT_index)) & 0x0fff;
		}
		else {
			current_cluster = (*(uint16_t*)(FAT+FAT_index)) >> 4;
		}
	} while (ok && current_cluster < 0x0ff8);

	return ok;
}

DirectoryEntry* findFile(const char* name) {
	for (uint32_t i = 0; i < boot_sector.dir_entry_count; i++) {
		if (memcmp(name, root_directory[i].name, 11) == 0) {
			return &root_directory[i];
		}
	}

	return NULL;
}
