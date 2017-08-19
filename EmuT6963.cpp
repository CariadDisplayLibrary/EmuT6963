#include <EmuT6963.h>

#ifndef _PMP_IRQ
#define _PMP_IRQ _PMP_VECTOR
#endif

/*

    90          Display off
---------------------------------------------------
0
0
    42          Graphic home 0x0000
---------------------------------------------------
80
7
    40          Text home 0x0780
---------------------------------------------------
1E
0
    43          Graphic area 0x001E columns
---------------------------------------------------
1E
0
    41          Text area 0x001E columns
---------------------------------------------------
    80          Mode: OR + Internal ROM
---------------------------------------------------

    98          Text off, Graphic on, 
---------------------------------------------------
0
0
    24          Set address pointer 0x0000
---------------------------------------------------
    B0          Auto write
---------------------------------------------------
0
0
0
*/

EmuT6963 *EmuT6963::_reader = NULL;

#define P_D1 1
#define P_D2 0

void __USER_ISR EmuT6963::doTransfer() {
    if (_reader) {
        _reader->processData();
    }
}

void EmuT6963::processData() {
    if (_led != 255) digitalWrite(_led, HIGH);
    if (PMSTATbits.IB0F) {
        if (_mode == 0) {
            _stack[1] = _stack[0];
            _stack[0] = PMDIN & 0xFF;
        } else {
            _frame_data++;
            _buffer[_addr_pointer++] = PMDIN & 0xFF;
            if (_addr_pointer >= 2048) _addr_pointer = 0;
        }
    } else if (PMSTATbits.IB1F) {
        uint8_t cmd = (PMDIN >> 8) & 0xFF;
        switch (cmd) {
            case 0x24:  // Set address pointer
                _addr_pointer = (_stack[P_D2] << 8) | _stack[P_D1];
                break;
            case 0x41:  // Set text area
                _cols = min(32, _stack[P_D1]);
                _width = _cols * 8;
                break;
            case 0x43:  // Set graphic area
                _cols = min(32, _stack[P_D1]);
                _width = _cols * 8;
                break;
            case 0xB0:
                _mode = 1;
                _frame_starts++;
                break;
            case 0xB2:
                _mode = 0;
                _frame_ends++;
                _cols = min(32, _addr_pointer / 64);
                _width = _cols * 8;
                break;
        }
    } else if (PMSTATbits.IB2F) {
        (void)PMDIN;
    } else if (PMSTATbits.IB3F) {
        (void)PMDIN;
    }

    if (PMSTATbits.OB0E) {
        PMDOUT = 0xAFAFAFAF;
    } else if (PMSTATbits.OB1E) {
        PMDOUT = 0xAFAFAFAF;
    } else if (PMSTATbits.OB2E) {
        PMDOUT = 0xAFAFAFAF;
    } else if (PMSTATbits.OB3E) {
        PMDOUT = 0xAFAFAFAF;
    }

    clearIntFlag(_PMP_IRQ);
    if (_led != 255) digitalWrite(_led, LOW);
}

void EmuT6963::initializeDevice() {
    if (_led != 255) {
        pinMode(_led, OUTPUT);
        digitalWrite(_led, LOW);
    }

    _frame_starts = 0;
    _frame_ends = 0;
    _frame_data = 0;

    _reader = this;
    _cols = 30;
    _width = 240;
    _height = 64;
    pinMode(10, INPUT);
    pinMode(3, INPUT);
    pinMode(A0, INPUT);
    TRISE = 0xFFFF;
    PMCON = 0;
    PMMODE = 0;
    PMCONbits.ON         = 0;    // Turn off PMP
    PMCONbits.SIDL        = 0;    // Continue in IDLE mode
    PMCONbits.ADRMUX    = 0b00;    // Separate address and data
    PMCONbits.PMPTTL    = 1;     // TTL Buffers
    PMCONbits.PTWREN    = 1;    // Enable WR
    PMCONbits.PTRDEN    = 1;    // Enable RD
    PMCONbits.CSF         = 0b10;    // CS1 and CS2 are both chip select
    PMCONbits.ALP         = 0;    // Address latch is active low
    PMCONbits.CS2P        = 0;    // CS2 is active low
    PMCONbits.CS1P        = 0;    // CS1 is active low
    PMCONbits.WRSP        = 0;    // WR is active low
    PMCONbits.RDSP        = 0;    // RD is active low
    PMMODEbits.BUSY        = 0;    // Not busy
    PMMODEbits.IRQM        = 0b01;    // Interrupt after every read or write
    PMMODEbits.INCM        = 0b00; // No increment or decrement
    PMMODEbits.MODE16     = 0;    // 8 bit mode
    PMMODEbits.MODE        = 0b01;    // Enhanced slave mode
    PMMODEbits.WAITB    = 1;    // Data wait 1 Tpb
    PMMODEbits.WAITM    = 1;    // Strobe wait 1 Tpb
    PMMODEbits.WAITE    = 0;    // Data hold 1 Tpb
    PMAEN                = 0xC1;    // CS1, CS2, PMA0 and PMA1 are PMP assigned
    PMDOUT = 0;
    setIntVector(_PMP_VECTOR, doTransfer);
    setIntPriority(_PMP_IRQ, 7, 0);
    clearIntFlag(_PMP_IRQ);
    setIntEnable(_PMP_IRQ);
    PMCONbits.ON        = 1;    // Turn the PMP on.
}

color_t EmuT6963::colorAt(int x, int y) {
    if (x >= _width) return 0;
    if (y >= _height) return 0;
    int offset = (y * _cols) + (x / 8);
    int bit = x % 8;
    return ((_buffer[offset] & (0x80 >> bit)) != 0) ? Color::White : Color::Black;
}

void EmuT6963::draw(DisplayCore *dev, int x, int y) {
    dev->openWindow(x, y, _width, _height);
    for (int py = 0; py < _height; py++) {
        for (int px = 0; px < _width; px++) {
            dev->windowData(colorAt(px, py));
        }
    }
    dev->closeWindow();
}

void EmuT6963::draw(DisplayCore *dev, int x, int y, color_t __attribute__((unused)) t) {
    draw(dev, x, y);
}

void EmuT6963::drawTransformed(DisplayCore *dev, int x, int y, int __attribute__((unused)) transform) {
    draw(dev, x, y);
}

void EmuT6963::drawTransformed(DisplayCore *dev, int x, int y, int __attribute__((unused)) transform, color_t __attribute__((unused)) t) {
    draw(dev, x, y);
}


void EmuT6963::stats() {
    Serial.print("Frame Start Count: ");
    Serial.println(_frame_starts);
    Serial.print("Frame End Count:   ");
    Serial.println(_frame_ends);
    Serial.print("Frame Data Count:  ");
    Serial.println(_frame_data);
    Serial.println();
}
