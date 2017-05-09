#pragma once

#include <cstdint>

enum class ata_bus : uint16_t { primary = 0x1f0, secondary = 0x170 };
enum class ata_drive { master, slave };

struct ata {
public:
    static void init();
    static void read_sectors(ata_bus bus, ata_drive drive, uint8_t *block, int sector_count, int sector_addr);
};
