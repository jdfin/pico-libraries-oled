#pragma once

#include <stdint.h>

class Font
{
public:

    Font(const char *name,                                //
         uint8_t ascent, uint8_t descent, uint8_t height, //
         uint8_t char_rows, char char_min, char char_max, //
         const uint16_t *data) :
        _name(name),
        _ascent(ascent),
        _descent(descent),
        _height(height),
        _char_rows(char_rows),
        _char_min(char_min),
        _char_max(char_max),
        _data(data)
    {
    }

    const uint16_t *pixel_data(char ch) const;
    void advance(char ch, int &left_adv, int &wid, int &right_adv) const;
    int width(char ch) const;
    int width(const char *str) const;
    int char_rows() const
    {
        return _char_rows;
    }
    int height() const
    {
        return _height;
    }

private:

    const char *_name;
    uint8_t _ascent;
    uint8_t _descent;
    uint8_t _height;
    uint8_t _char_rows; // uint16_t of pixel data for each char
    char _char_min;
    char _char_max;
    const uint16_t *_data;
};


// _data:
//
// Each glyph uses 3 + font._char_rows in the font._data[] array
//
// glyph 0 is at [0 * (3 + _char_rows)]:
// [0] uint16_t adv_left;
// [1] uint16_t width; // 0..15
// [2] uint16_t adv_right;
// [3] uint16_t pixels[_char_rows];
//
// glyph 1 is at [1 * (3 + _char_rows)]:
// [2+_char_rows]   uint16_t adv_left;
// [2+_char_rows+1] uint16_t width;
// [2+_char_rows+2] uint16_t adv_right;
// [2+_char_rows+3] uint16_t pixels[_char_rows];
//
// :
//
// glyph N is at [N * (3 + _char_rows)]:
// +0  uint16_t adv_left;
// +1  uint16_t width;
// +2  uint16_t adv_right;
// +3  uint16_t pixels[_char_rows];
