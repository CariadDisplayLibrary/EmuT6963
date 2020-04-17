#ifndef _EMU_T6963_H
#define _EMU_T6963_H

#include <DisplayCore.h>

class EmuT6963 : public Image {
    private:
        volatile uint8_t _buffer[32*64];
        uint8_t _cols;
        int _addr_pointer = 0;
        int _mode = 0;
        uint8_t _stack[2];
        uint8_t _led;
        static EmuT6963 *_reader;

        uint32_t _frame_starts;
        uint32_t _frame_ends;
        uint32_t _frame_data;

    public:
        EmuT6963(uint8_t l) : _led(l) {}
        EmuT6963() : _led(255) {}

        void initializeDevice();
        static void __USER_ISR doTransfer();
        void processData();

        void setPixel(int __attribute__((unused)) x, int __attribute__((unused)) y, color_t __attribute__((unused)) c) {}
        void setRotation(uint8_t __attribute__((unused)) r) {}
        void displayOn() {}
        void displayOff() {}
        void invertDisplay(bool __attribute__((unused)) i) {}
        
        color_t colorAt(int x, int y);

        void draw(DisplayCore *dev, int x, int y);
        void draw(DisplayCore *dev, int x, int y, color_t t);
        void drawTransformed(DisplayCore *dev, int x, int y, int transform);
        void drawTransformed(DisplayCore *dev, int x, int y, int transform, color_t t);

        void draw(DisplayCore &dev, int x, int y) { draw(&dev, x, y); }
        void draw(DisplayCore &dev, int x, int y, color_t t) { draw(&dev, x, y, t); }
        void drawTransformed(DisplayCore &dev, int x, int y, int transform) { drawTransformed(&dev, x, y, transform); }
        void drawTransformed(DisplayCore &dev, int x, int y, int __attribute__((unused)) transform, color_t t) { drawTransformed(&dev, x, y, t); }

        void stats();
};

#endif
