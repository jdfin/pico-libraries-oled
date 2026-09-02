
#include <ctype.h>

#include <cassert>
#include <cstdint>
// pico
#include "pico/stdlib.h"
// misc
#include "misc/i2c_dev.h"
//
#include "oled/oled.h"


// ~800 usec at 400 KHz
void Oled::init()
{
    clear();
    off();                 // display off
#if 0 // what does this work for? 128x64?
    write_cmd(0x20, 0x00); // horizontal addressing mode
    write_cmd(0xa8, 0x3f); // mux ratio 64
    write_cmd(0x8d, 0x14); // charge pump enabled
    write_cmd(0xc8);       // flip vertically
    write_cmd(0xa1);       // flip horizontally
    write_cmd(0xda, 0x12); // alt com pins, no l/r remap
    write_cmd(0xd9, 0xf1); // precharge periods
#else
    // from pico-examples
    // works with adafruit 128x32
    uint8_t cmds[] = {
        // clang-format off
        0x00,                       // ssd1306 control byte
        set_disp,                   // set display off
        // memory mapping
        set_mem_mode, 0x00,         // set memory address mode 0 = hor, 1 = ver, 2 = page
        // resolution and layout
        set_disp_start_line,        // set display start line to 0
        set_seg_remap | 0x01,       // set segment re-map, column address 127 is mapped to SEG0
        set_mux_ratio, rows - 1,    // set multiplex ratio
        set_com_out_dir | 0x08,     // set COM (common) output scan direction
        set_disp_offset, 0x00,      // set display offset
        set_com_pin_cfg,            // set COM (common) pins hardware configuration (magic)
        rows == 32 ? 0x02 : 0x12,   // 0x02 for 128x32, 0x12 for 128x64
        // timing and driving scheme
        set_disp_clk_div, 0x80,     // set display clock divide ratio
        set_precharge, 0xf1,        // set pre-charge period (Vcc internally generated on our board)
        set_vcom_desel, 0x30,       // set VCOMH deselect level (0.83xVcc)
        // display
        set_contrast, 0xff,         // set contrast control
        set_entire_on,              // set entire display on to follow RAM content
        set_norm_disp,              // set normal (not inverted) display
        set_charge_pump, 0x14,      // set charge pump
        set_scroll | 0x00,          // deactivate horizontal scrolling if set
        // clang-format on
    };
    write_cmds(cmds, sizeof(cmds));
#endif
}


// Set a pixel to either foreground or background color
void Oled::set(int col, int row, Color color)
{
    set(col, row, color_to_bit(color));
}


void Oled::fill(int col, int row, int wid, int hgt, Color color)
{
    int bit = color_to_bit(color);
    for (int c = col; c < (col + wid); c++)
        for (int r = row; r < (row + hgt); r++)
            set(c, r, bit);
}


void Oled::clear(Color color)
{
    uint16_t word = (color_to_bit(color) == 0) ? 0x0000 : 0x00ff;
    for (int p = 0; p < pages; p++)
        for (int c = 0; c < cols; c++)
            _image[p][c] = word;
}


// (col, row) is character location, subject to _h_align and _v_align
// ch is the character to put
// font is the font to use
// val is the color to use (bg or fg)
// return width of character rendered
int Oled::putc(int col, int row, char ch, Color color)
{
    assert(_font != nullptr);

    // adjust vertical position if not top aligned
    if (_v_align == VAlign::Center)
        row -= _font->height() / 2;
    else if (_v_align == VAlign::Bottom)
        row -= _font->height();

    // adjust horizontal position if not left aligned
    if (_h_align == HAlign::Center)
        col -= _font->width(ch) / 2;
    else if (_h_align == HAlign::Right)
        col -= _font->width(ch);

    return putc_tl(col, row, ch, color_to_bit(color));
}


void Oled::puts(int col, int row, const char *str, Color color)
{
    assert(_font != nullptr);

    // number of lines to draw is 1 + count of newlines
    // e.g. "hello" is 1, "hello\n" is 2, "hello,\nworld" is 2
    int lines = 1;
    for (const char *s = str; *s != '\0'; s++)
        if (*s == '\n')
            lines++;

    // adjust vertical position if not top aligned
    if (_v_align == VAlign::Center)
        row -= (lines * _font->height()) / 2;
    else if (_v_align == VAlign::Bottom)
        row -= (lines * _font->height());

    int bit = color_to_bit(color);

    // print each line
    while (lines > 0) {

        // width of next line to print
        int wid = 0;
        for (const char *s = str; *s != '\0' && *s != '\n'; s++)
            wid += _font->width(*s);

        // adjust horizontal position if not left aligned
        int col0 = col;
        if (_h_align == HAlign::Center)
            col0 -= wid / 2;
        else if (_h_align == HAlign::Right)
            col0 -= wid;

        while (*str != '\0' && *str != '\n')
            col0 += putc_tl(col0, row, *str++, bit);

        if (*str == '\n')
            str++;

        row += _font->height();
        lines--;
    }
}


// This just starts the write using DMA
// Less than 10 usec at 400 KHz
void Oled::flush()
{
    const uint16_t buf[] = {
        0x0000, 0x00b0,         // start, page 0
        0x0400, 0x0010, 0x0000, // restart, column 0
        0x0440                  // restart, data next
    };
    const int buf_cnt = sizeof(buf) / sizeof(buf[0]);
    _i2c.write_start(_i2c_addr, buf, buf_cnt, //
                     &_image[0][0], pages * cols);
}


// Set or clear a pixel.
// Black/White/Foreground/Background has already been mapped to 0 or 1.
void Oled::set(int col, int row, int bit)
{
    assert(bit == 0 || bit == 1);

    if (col < 0 || col >= cols || row < 0 || row >= rows)
        return;

    int page = row / 8;
    uint16_t b = 1 << (row % 8);
    if (bit != 0)
        _image[page][col] |= b;
    else
        _image[page][col] &= ~b;
}


// Draw character, always top-left aligned.
// Black/White/Foreground/Background has already been mapped to 0 or 1.
// Return width of character.
int Oled::putc_tl(int col, int row, char ch, int bit)
{
    assert(bit == 0 || bit == 1);
    assert(_font != nullptr);
    const uint16_t *pix = _font->pixel_data(ch);
    assert(pix != nullptr);
    int wid = _font->width(ch);
    for (int r = 0; r < _font->char_rows(); r++) {
        for (int c = 0; c < wid; c++) {
            if ((pix[r] & (1 << (wid - c - 1))) != 0)
                set(col + c, row + r, bit);
            else
                set(col + c, row + r, 1 - bit);
        }
    }
    return wid;
}
