#pragma once

#include <cstdint>

class Font
{

public:

    Font(const char *name,                                //
         uint8_t ascent, uint8_t descent, uint8_t height, //
         uint8_t char_rows, const uint16_t *data) :
        _name(name),
        _ascent(ascent),
        _descent(descent),
        _height(height),
        _char_rows(char_rows),
        _data(data)
    {
    }

    const uint16_t *pixel_data(char ch) const;

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
    const uint16_t *_data;
};


// _data array:
//
// Each glyph uses:
//   uint16_t[1] with the width in pixels of the character
//   uint16_t[font._char_rows] with the pixel data
//
// glyph 0 is at [0 * (1 + _char_rows)]:
// [0] uint16_t width; // 0..16
// [1] uint16_t pixels[_char_rows];
//
// glyph 1 is at [1 * (1 + _char_rows)]:
// [1+_char_rows+0] uint16_t width;
// [1+_char_rows+1] uint16_t pixels[_char_rows];
//
// :
//
// glyph N is at [N * (1 + _char_rows)]:
// +0  uint16_t width;
// +1  uint16_t pixels[_char_rows];
