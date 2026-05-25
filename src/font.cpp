
#include <stddef.h>
#include <stdint.h>
#include <ctype.h>
#include "font.h"


// get pointer to pixel data
const uint16_t *Font::pixel_data(char ch) const
{
    if (ch >= _char_min && ch < _char_max) {
        int ci = (int)ch - (int)(_char_min);
        const uint16_t *data = _data + ci * (3 + _char_rows);
        return data + 3;
    } else {
        return nullptr;
    }
}


// get left advance, character width, right advance
void Font::advance(char ch, int &left_adv, int &wid, int &right_adv) const
{
    if (ch >= _char_min && ch < _char_max) {
        int ci = (int)ch - (int)(_char_min);
        const uint16_t *data = _data + ci * (3 + _char_rows);
        left_adv = data[0];
        wid = data[1];
        right_adv = data[2];
    } else {
        left_adv = 0;
        wid = 0;
        right_adv = 0;
    }
}


// get width of character in pixels
int Font::width(char ch) const
{
    if (ch >= _char_min && ch < _char_max) {
        int ci = (int)ch - (int)(_char_min);
        const uint16_t *data = _data + ci * (3 + _char_rows);
        return data[0] + data[1] + data[2];
    } else {
        return 0;
    }
}


// get width of string in pixels up to first nonprintable character
int Font::width(const char *str) const
{
    int w = 0;
    while (isprint((int)(*str)))
        w += width(*str++);
    return w;
}
