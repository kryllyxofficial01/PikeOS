#include <stdint.h>

#define true 1
#define false 0

typedef uint8_t boolean;

typedef struct {
    // FAT Headers
    uint8_t jumpInstruction[3];
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

    // Extended Boot Record
    uint8_t driveNumber;
    uint8_t _reserved;
    uint8_t signature;
    uint32_t volumeID;
    uint8_t volumeLabel[11];
    uint8_t systemID;
} __attribute__((packed)) BootSector;

typedef struct {
    uint8_t name[11];
    uint8_t attributes;
    uint8_t _reserved;
    uint8_t timeCreatedInTenths;
    uint16_t timeCreated;
    uint16_t dateCreated;
    uint16_t dateAccessed;
    uint16_t lowClusterBits; // First 16 bits of a cluster
    uint16_t highClusterBits; // Last 16 bits a cluster
    uint16_t timeModified;
    uint16_t dateModified;
    uint32_t size;
} __attribute__((packed)) DirectoryEntry;