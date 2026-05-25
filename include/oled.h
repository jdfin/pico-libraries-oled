#pragma once

#include <cstdint>
#include <cstring> // memset
// pico
#include "pico/stdlib.h"
// misc
#include "i2c_dev.h"
// oled
#include "font.h"


class Oled
{

public:

    Oled(I2cDev &i2c, uint8_t i2c_addr) :
        _i2c(i2c),
        _i2c_addr(i2c_addr),
        _persist(true) // default: leave it on in destructor
    {
    }

    ~Oled()
    {
        if (!_persist)
            off();
    }

    void init();

    // ~75 usec at 400 KHz
    void on()
    {
        write_cmd(0xaf);
    }

    // ~75 usec at 400 KHz
    void off()
    {
        write_cmd(0xae);
    }

    // Fill rectangle with either foreground or background
    void fill(int col, int row, int wid, int hgt, int val);

    // Fill screen with either foreground or background
    void clear(int val = bg);

    // Set a pixel to either foreground or background color
    void set(int col, int row, int val = fg);

    // Draw character using foreground or background color
    int putc(int col, int row, char ch, const Font *font, int val = fg);

    // Draw string using foreground or background color
    // Note that string terminates with anything nonprintable (isprint false).
    void puts(int col, int row, const char *str, const Font *font, int val = fg);

    // Start writing _image to screen over i2c (using DMA)
    void flush();

    static constexpr int baud = 400'000;

    // foreground/background; 0 is black and 1 is white
    static constexpr int bg = 0;
    static constexpr int fg = 1 - bg;

    // 64 x 128
    static constexpr int rows = 64;
    static constexpr int cols = 128;

private:

    I2cDev &_i2c;
    uint8_t _i2c_addr;

    // true: destructor leaves the display on (default)
    // false: destructor turns off display (takes ~80 us @ 400 KHz)
    bool _persist;

    static_assert(rows % 8 == 0);

    static constexpr int pages = rows / 8;

    // LSB of each uint16_t is 8 pixels of image data, MSB is i2c control
    uint16_t _image[pages][cols];

    // ~75 usec at 400 KHz
    void write_cmd(uint8_t cmd)
    {
        constexpr uint8_t ctrl = 0x00;
        uint8_t xbuf[2] = {ctrl, cmd};
        _i2c.write_sync(_i2c_addr, xbuf, sizeof(xbuf));
    }

    // ~100 usec at 400 KHz
    void write_cmd(uint8_t cmd1, uint8_t cmd2)
    {
        constexpr uint8_t ctrl = 0x00;
        uint8_t xbuf[3] = {ctrl, cmd1, cmd2};
        _i2c.write_sync(_i2c_addr, xbuf, sizeof(xbuf));
    }

}; // class Oled
