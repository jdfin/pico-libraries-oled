#pragma once

#include <cassert>
#include <cstdint>
// pico sdk
#include "pico/stdlib.h"
// misc lib
#include "misc/i2c_dev.h"
// oled lib
#include "oled/font.h"


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
        constexpr uint8_t ctrl = 0x00;
        uint8_t buf[] = { ctrl, 0xaf };
        write_cmds(buf, sizeof(buf));
    }

    // ~75 usec at 400 KHz
    void off()
    {
        constexpr uint8_t ctrl = 0x00;
        uint8_t buf[] = { ctrl, 0xae };
        write_cmds(buf, sizeof(buf));
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

    // 32 x 128 or 64 x 128
    static constexpr int rows = 32;
    static constexpr int cols = 128;
    static_assert(rows == 32 || rows == 64);

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

    void write_cmds(uint8_t *buf, int buf_len)
    {
        // NOTE: the first byte is reserved for the SSD1306 control byte
        assert(buf_len >= 2);
        assert(buf[0] == 0x00); // Co=0, D/C=0
        _i2c.write_timeout_us(_i2c_addr, buf, buf_len);
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

    // command bytes, from pico-examples
    static constexpr uint8_t set_mem_mode = 0x20;
    static constexpr uint8_t set_col_addr = 0x21;
    static constexpr uint8_t set_page_addr = 0x22;
    static constexpr uint8_t set_horiz_scroll = 0x26;
    static constexpr uint8_t set_scroll = 0x2e;

    static constexpr uint8_t set_disp_start_line = 0x40;

    static constexpr uint8_t set_contrast = 0x81;
    static constexpr uint8_t set_charge_pump = 0x8d;

    static constexpr uint8_t set_seg_remap = 0xa0;
    static constexpr uint8_t set_entire_on = 0xa4;
    static constexpr uint8_t set_all_on = 0xa5;
    static constexpr uint8_t set_norm_disp = 0xa6;
    static constexpr uint8_t set_inv_disp = 0xa7;
    static constexpr uint8_t set_mux_ratio = 0xa8;
    static constexpr uint8_t set_disp = 0xae;
    static constexpr uint8_t set_com_out_dir = 0xc0;
    static constexpr uint8_t set_com_out_dir_flip = 0xc0;

    static constexpr uint8_t set_disp_offset = 0xd3;
    static constexpr uint8_t set_disp_clk_div = 0xd5;
    static constexpr uint8_t set_precharge = 0xd9;
    static constexpr uint8_t set_com_pin_cfg = 0xda;
    static constexpr uint8_t set_vcom_desel = 0xdb;

    static constexpr uint8_t write_mode = 0xfe;
    static constexpr uint8_t read_mode = 0xff;

}; // class Oled
