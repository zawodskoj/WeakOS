#include <os/ata.h>
#include <os/io.h>
#include <os/interrupt.h>

void ata::init() {
}

void ata::read_sectors(ata_bus bus, ata_drive drive, uint8_t *block, int sector_count, int sector_addr) {
    uint16_t *wblock = reinterpret_cast<uint16_t*>(block);
    
    io::outb((uint16_t) bus + 6, drive == ata_drive::master ? 0xe0 : 0xf0);
    io::outb((uint16_t) bus + 1, 0);
    io::outb((uint16_t) bus + 2, sector_count);
    io::outb((uint16_t) bus + 3, sector_addr & 0xff);
    io::outb((uint16_t) bus + 4, sector_addr >> 8);
    io::outb((uint16_t) bus + 5, sector_addr >> 16);    
    io::outb((uint16_t) bus + 7, 0x20);

    for (int j = 0; j < sector_count; j++) {
        interrupt::wait_irq(bus == ata_bus::primary ? 14 : 15);
        
        for (int i = 0; i < 0x100; i++) wblock[i + j * 0x100] = io::inw((uint16_t) bus);
    }
}

/*void ata::read_sectors(ata_bus bus, ata_drive drive, uint8_t *block, int sector_count, int sector_addr) {
    uint16_t *wblock = reinterpret_cast<uint16_t*>(block);
    
    io::outb((uint16_t) bus + 6, drive == ata_drive::master ? 0xe0 : 0xf0);
    io::outb((uint16_t) bus + 1, 0);
    io::outb((uint16_t) bus + 2, sector_count);
    io::outb((uint16_t) bus + 3, sector_addr & 0xff);
    io::outb((uint16_t) bus + 4, sector_addr >> 8);
    io::outb((uint16_t) bus + 5, sector_addr >> 16);    
    io::outb((uint16_t) bus + 7, 0x20);

    for (int j = 0; j < sector_count; j++) {
        io::wait();
        io::wait();
        io::wait();
        
        while (io::inb((uint16_t) bus + 7) & 0x80);
        while (!(io::inb((uint16_t) bus + 7) & 8));
        
        for (int i = 0; i < 0x100; i++) wblock[i + j * 0x100] = io::inw((uint16_t) bus);
    }
}*/
