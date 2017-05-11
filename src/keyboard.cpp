#include <os/keyboard.h>
#include <os/interrupt.h>
#include <os/io.h>
#include <ustdio.h>

#define PS2_PORT_DATA 0x60
#define PS2_PORT_COMMAND 0x64
#define PS2_CMD_DISABLE_FIRST 0xAD
#define PS2_CMD_DISABLE_SECOND 0xA7
#define PS2_CMD_READ_RAM0 0x20
#define PS2_CMD_WRITE_RAM0 0x60
#define PS2_CMD_CONTROLLER_TEST 0xAA
#define PS2_CMD_CONTROLLER_TEST_REPLY 0x55
#define PS2_CMD_FIRST_TEST 0xAB
#define PS2_CMD_ENABLE_FIRST 0xAE

bool ps2_has_data() {
    return io::inb(PS2_PORT_COMMAND) & 1;
}

void ps2_wait_data() {
    while (true) if (io::inb(PS2_PORT_COMMAND) & 1) break;
}

void ps2_wait_input_ready() {
    while (true) if (!(io::inb(PS2_PORT_COMMAND) & 2)) break;
}

void ps2_initialize() {
    io::outb(PS2_PORT_COMMAND, PS2_CMD_DISABLE_FIRST);
    io::outb(PS2_PORT_COMMAND, PS2_CMD_DISABLE_SECOND);
    
    while (ps2_has_data()) io::inb(PS2_PORT_DATA);
    
    io::outb(PS2_PORT_COMMAND, PS2_CMD_READ_RAM0);
    ps2_wait_data();
    uint8_t ram0 = io::inb(PS2_PORT_DATA);
    
    io::outb(PS2_PORT_COMMAND, PS2_CMD_WRITE_RAM0);
    ps2_wait_input_ready();
    io::outb(PS2_PORT_DATA, ram0 & 0xbd);
    
    io::outb(PS2_PORT_COMMAND, PS2_CMD_CONTROLLER_TEST);
    ps2_wait_data();
    if (io::inb(PS2_PORT_DATA) != PS2_CMD_CONTROLLER_TEST_REPLY) {
        stdio::printf("Bad PS/2 controller");
        while (true);
    }
    
    io::outb(PS2_PORT_COMMAND, PS2_CMD_FIRST_TEST);
    ps2_wait_data();
    if (io::inb(PS2_PORT_DATA) != 0) {
        stdio::printf("Bad PS/2 device");
        while (true);
    }
    
    io::outb(PS2_PORT_COMMAND, PS2_CMD_ENABLE_FIRST);
}

#define PS2_SCANCODE_NORMAL 1
#define PS2_SCANCODE_BREAK 2
#define PS2_SCANCODE_EXTENDED 4
#define PS2_SCANCODE_WIDE 8
#define PS2_SCANCODE_UNKNOWN 0

typedef struct {
    uint16_t type;
    uint16_t scancode;
} ps2_scancode;

ps2_scancode ps2_read() {
    ps2_scancode rval;
    rval.type = PS2_SCANCODE_UNKNOWN;
    if (!ps2_has_data()) return rval;
    uint8_t scancode = io::inb(PS2_PORT_DATA);
    if (scancode & 0x80) {
        if (scancode == 0xe0) {
            ps2_wait_data();
            scancode = io::inb(PS2_PORT_DATA);
            if (scancode == 0xf0) {
                ps2_wait_data();
                scancode = io::inb(PS2_PORT_DATA);
                rval.type = PS2_SCANCODE_BREAK | PS2_SCANCODE_EXTENDED;
                rval.scancode = scancode;
            }
            else {
                rval.type = PS2_SCANCODE_EXTENDED;
                rval.scancode = scancode;
            }
            return rval;
        }
        if (scancode == 0xe1) {
            ps2_wait_data();
            scancode = io::inb(PS2_PORT_DATA);
            if (scancode == 0xf0) {
                ps2_wait_data();
                scancode = io::inb(PS2_PORT_DATA);
                ps2_wait_data();
                io::inb(PS2_PORT_DATA);
                ps2_wait_data();
                rval.type = PS2_SCANCODE_BREAK | PS2_SCANCODE_WIDE;
                rval.scancode = scancode | (io::inb(PS2_PORT_DATA) << 8);
            }
            else {
                ps2_wait_data();
                rval.type = PS2_SCANCODE_WIDE;
                rval.scancode = scancode | (io::inb(PS2_PORT_DATA) << 8);;
            }
            return rval;
        }
        if (scancode == 0xf0) {
            ps2_wait_data();
            rval.type = PS2_SCANCODE_BREAK;
            rval.scancode = io::inb(PS2_PORT_DATA);
            return rval;
        }
        rval.type = PS2_SCANCODE_UNKNOWN;
        rval.scancode = scancode;
        return rval;
    }
    rval.type = PS2_SCANCODE_NORMAL;
    rval.scancode = scancode;
    return rval;
}

static bool waiting = false;
static char chr;
static bool shift;

typedef struct {
    bool shift;
    uint8_t scancode;
    char chr;
} ps2_translated_scancode;

#include "scancodes.inc"

void dispatch_scancode(ps2_scancode scancode, bool shift) {
    if (scancode.type != PS2_SCANCODE_NORMAL) return;
    
    for (unsigned i = 0; i < sizeof(scancodes) / sizeof(scancodes[0]); i++) 
        if (scancodes[i].shift == shift && scancodes[i].scancode == scancode.scancode) {
            chr = scancodes[i].chr;
            waiting = false;
            return;
        }
}

void kbd_interrupt(int irq, uint32_t&) {
    ps2_scancode scancode = ps2_read();
    if (scancode.scancode == 0x12 && (scancode.type & ~PS2_SCANCODE_EXTENDED)) {
        shift = scancode.type != PS2_SCANCODE_BREAK;
    }
    else dispatch_scancode(scancode, shift);
}

void keyboard::init() {
    interrupt::map_irq(1, kbd_interrupt);
    
    ps2_initialize();
}


char keyboard::wait_char() {
    waiting = true;
    while (waiting);
    return chr;
}
