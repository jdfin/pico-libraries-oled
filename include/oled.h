#pragma once

#include <cassert>
#include <cstdint>
// pico sdk
#include "pico/stdlib.h"
// misc lib
#include "i2c_dev.h"
// oled lib
#include "font.h"


class Oled
{

public:

    Oled(I2cDev &i2c, uint8_t i2c_addr, const Font *font = nullptr) :
        _i2c(i2c),
        _i2c_addr(i2c_addr),
        _persist(true), // default: leave it on in destructor
        _font(font),
        _h_align(HAlign::Left),
        _v_align(VAlign::Top),
        _fg(Color::White)
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

    // One can specify to draw in Black, White, or current foreground/background
    enum class Color { Black = 0, White = 1, Foreground, Background };

    void set_fg(Color fg)
    {
        assert(fg == Color::Black || fg == Color::White);
        _fg = fg;
    }

    // Set a pixel to either foreground or background color
    void set(int col, int row, Color color = Color::Foreground);

    // Fill rectangle
    void fill(int col, int row, int wid, int hgt, Color color);

    // Fill screen with either foreground or background
    void clear(Color color = Color::Background);

    // Text Stuff

    void set_font(const Font *font)
    {
        _font = font;
    }

    enum class HAlign { Left, Center, Right };
    enum class VAlign { Top, Center, Bottom };

    void set_align(HAlign ha, VAlign va)
    {
        _h_align = ha;
        _v_align = va;
    }

    // Draw character using current font and alignment
    int putc(int col, int row, char ch, Color color = Color::Foreground);

    // Draw string using current font and alignment
    void puts(int col, int row, const char *str,
              Color color = Color::Foreground);

    // Start writing _image to screen over i2c (using DMA)
    void flush();

    static constexpr int baud = 400'000;

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

    const Font *_font;

    HAlign _h_align;
    VAlign _v_align;

    // foreground color; background is implied
    Color _fg;

    // LSB of each uint16_t is 8 pixels of image data, MSB is i2c control
    uint16_t _image[pages][cols];

    // ~75 usec at 400 KHz
    void write_cmd(uint8_t cmd)
    {
        constexpr uint8_t ctrl = 0x00;
        uint8_t xbuf[2] = {ctrl, cmd};
        _i2c.write_timeout_us(_i2c_addr, xbuf, sizeof(xbuf));
    }

    // ~100 usec at 400 KHz
    void write_cmd(uint8_t cmd1, uint8_t cmd2)
    {
        constexpr uint8_t ctrl = 0x00;
        uint8_t xbuf[3] = {ctrl, cmd1, cmd2};
        _i2c.write_timeout_us(_i2c_addr, xbuf, sizeof(xbuf));
    }

    int color_to_bit(Color color)
    {
        if (color == Color::Black) {
            return 0;
        } else if (color == Color::White) {
            return 1;
        } else if (color == Color::Foreground) {
            return (_fg == Color::Black) ? 0 : 1;
        } else {
            assert(color == Color::Background);
            return (_fg == Color::Black) ? 1 : 0;
        }
    }

    void set(int col, int row, int bit);

    int putc_tl(int col, int row, char ch, int bit);

}; // class Oled
