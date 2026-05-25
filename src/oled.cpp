
#include <ctype.h>

#include <cstdint>
// pico
#include "pico/stdlib.h"
// misc
#include "i2c_dev.h"
//
#include "oled.h"


// ~800 usec at 400 KHz
void Oled::init()
{
    clear();
    off();                 // display off
    write_cmd(0x20, 0x00); // horizontal addressing mode
    write_cmd(0xa8, 0x3f); // mux ratio 64
    write_cmd(0x8d, 0x14); // charge pump enabled
    write_cmd(0xc8);       // flip vertically
    write_cmd(0xa1);       // flip horizontally
    write_cmd(0xda, 0x12); // alt com pins, no l/r remap
    write_cmd(0xd9, 0xf1); // precharge periods
}


void Oled::fill(int col, int row, int wid, int hgt, int val)
{
    for (int c = col; c < (col + wid); c++)
        for (int r = row; r < (row + hgt); r++)
            set(c, r, val);
}


void Oled::clear(int val)
{
    uint16_t w = (val == 0) ? 0x0000 : 0x00ff;
    for (int p = 0; p < pages; p++)
        for (int c = 0; c < cols; c++)
            _image[p][c] = w;
}


void Oled::set(int col, int row, int val)
{
    if (col < 0 || col >= cols)
        return;
    if (row < 0 || row >= rows)
        return;

    int page = row / 8;
    uint16_t b = 1 << (row % 8);
    if (val != 0)
        _image[page][col] |= b;
    else
        _image[page][col] &= ~b;
}


// (col, row) is the top-left pixel of the character
// ch is the character to put
// font is the font to use
// val is the color to use (bg or fg)
// return width of character rendered
int Oled::putc(int col, int row, char ch, const Font *font, int val)
{
    const uint16_t *pix = font->pixel_data(ch);
    int lft, wid, rgt;
    font->advance(ch, lft, wid, rgt);

    for (int r = 0; r < font->char_rows(); r++) {

        // clear left advance
        for (int c = 0; c < lft; c++)
            set(col + c, row + r, 1 - val);

        // copy pixel data
        for (int c = 0; c < wid; c++) {
            if ((pix[r] & (1 << (wid - c - 1))) != 0)
                set(col + lft + c, row + r, val);
            else
                set(col + lft + c, row + r, 1 - val);
        }

        // clear right advance
        for (int c = 0; c < rgt; c++)
            set(col + lft + wid + c, row + r, 1 - val);

    } // for (int r...)

    return lft + wid + rgt;
}


void Oled::puts(int col, int row, const char *str, const Font *font, int val)
{
    while (isprint(*str)) {
        col += putc(col, row, *str, font, val);
        str++;
    }
}


// This just starts the write using DMA
// Less than 10 usec at 400 KHz
void Oled::flush()
{
    uint16_t buf_0[] = {
        0x0000, 0x00b0,         // start, page 0
        0x0400, 0x0010, 0x0000, // restart, column 0
        0x0440                  // restart, data next
    };
    _i2c.write_async_start(_i2c_addr,                               //
                           buf_0, sizeof(buf_0) / sizeof(buf_0[0]), //
                           &_image[0][0], pages * cols);
}
