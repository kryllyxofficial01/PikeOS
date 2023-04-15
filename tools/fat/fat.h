#define true 1
#define false 0

typedef struct {
    // FAT Headers
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

    // Extended Boot Record
    uint8_t driveNumber;
    uint8_t _reserved;
    uint8_t signature;
    uint32_t volumeID;
    uint8_t volumeLabel[11];
    uint8_t systemID[8];

} __attribute__((packed)) BootSector;

typedef struct
{
    uint8_t name[11];
    uint8_t attributes;
    uint8_t _reserved;
    uint8_t createdTimeInTenths;
    uint16_t creationTime;
    uint16_t creationDate;
    uint16_t accessedDate;
    uint16_t highClusterBits;
    uint16_t modifiedTime;
    uint16_t modifiedDate;
    uint16_t lowClusterBits;
    uint32_t size;
} __attribute__((packed)) DirectoryEntry;