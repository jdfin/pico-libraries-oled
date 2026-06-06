
#include "oled/font.h"

#include <ctype.h>
#include <stddef.h>
#include <stdint.h>


// pointer to pixel data
const uint16_t *Font::pixel_data(char ch) const
{
    if (ch >= 0x80)
        return nullptr;
    const uint16_t *data = _data + int(ch) * (1 + _char_rows);
    return data + 1;
}


// width of character in pixels
int Font::width(char ch) const
{
    if (ch >= 0x80)
        return 0;
    const uint16_t *data = _data + int(ch) * (1 + _char_rows);
    return data[0];
}


// width of string in pixels, up to first nonprintable character
int Font::width(const char *str) const
{
    int w = 0;
    while (isprint(int(*str)))
        w += width(*str++);
    return w;
}
