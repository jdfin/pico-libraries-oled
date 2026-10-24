
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
// pico
#include "pico/stdio.h"
#include "pico/stdio_usb.h"
#include "pico/stdlib.h"
#include "pico/time.h" // sleep_ms
// pico hardware
#include "hardware/gpio.h"
#include "hardware/i2c.h"
// misc lib
#include "misc/i2c_dev.h"
#include "misc/util.h" // mem_name
// oled lib
#include "oled/chicago_12.h"
#include "oled/font.h"
#include "oled/oled.h"

constexpr uint8_t i2c_addr = 0x3c; // SSD1306: 0x3c or 0x3d
static i2c_inst_t *i2c_dev = i2c0; // RP2040/RP2350: i2c0 or i2c1

static constexpr int sda_gpio = 16;
static constexpr int scl_gpio = 17;

// easily comment out some of the demo functions
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

static void do_hello(Oled &oled);
static void do_fullscreen(Oled &oled);
static void do_dots(Oled &oled);
static void do_stripes(Oled &oled);
static void do_chars1a(Oled &oled);
static void do_chars1b(Oled &oled);
static void do_chars2a(Oled &oled);
static void do_chars2b(Oled &oled);
static void do_double(Oled &oled, const char *s);
static void do_hello_2x(Oled &oled);
static void do_chars1a_2x(Oled &oled);
static void do_chars1b_2x(Oled &oled);
static void do_chars2a_2x(Oled &oled);
static void do_chars2b_2x(Oled &oled);
static void do_chars2c_2x(Oled &oled);
static void do_chars2d_2x(Oled &oled);
static void do_chars2e_2x(Oled &oled);

static const Font *font = &chicago_12;

static struct {
    const char *name;
    void (*func)(Oled &oled);
} demo[] = {
    {"hello", do_hello},           //
//  {"fullscreen", do_fullscreen}, //
//  {"dots", do_dots},             //
//  {"stripes", do_stripes},       //
//  {"chars1a", do_chars1a},       //
//  {"chars1b", do_chars1b},       //
//  {"chars2a", do_chars2a},       //
//  {"chars2b", do_chars2b},       //
    {"hello_2x", do_hello_2x},     //
    {"chars1a_2x", do_chars1a_2x}, //
    {"chars1b_2x", do_chars1b_2x}, //
    {"chars2a_2x", do_chars2a_2x}, //
    {"chars2b_2x", do_chars2b_2x}, //
    {"chars2c_2x", do_chars2c_2x}, //
    {"chars2d_2x", do_chars2d_2x}, //
    {"chars2e_2x", do_chars2e_2x}, //
};
static constexpr int demo_max = sizeof(demo) / sizeof(demo[0]);

static void run_demo(int demo_num);


int main()
{
    stdio_init_all();

    // wait up to 'wait_us' for usb serial to connect
    constexpr int32_t wait_us = 0; //30'000'000;
    int32_t start_us = int32_t(time_us_32());
    while (!stdio_usb_connected() &&
           (int32_t(time_us_32()) - start_us) <= wait_us)
        tight_loop_contents();

    sleep_ms(10);

    printf("\n");
    printf("oled_demo\n");
    printf("\n");

    // show that the font data stays in flash
    printf("font @ %p (%s), font.data = %p (%s)\n", font, mem_name(font),
           font->pixel_data('A'), mem_name(font->pixel_data('A')));
    printf("\n");

    int demo_num = 0;
    while (true) {
        run_demo(demo_num);
        demo_num = (demo_num + 1) % demo_max;
        sleep_ms(5000);
    }

    return 0;

} // int main()


static void run_demo(int demo_num)
{
    printf("\n");
    printf("%s\n", demo[demo_num].name);

    I2cDev i2c(i2c_dev, scl_gpio, sda_gpio, Oled::baud);

    printf("i2c: baud requested=%u actual=%u\n", Oled::baud, i2c.baud());
    sleep_ms(10);

    Oled oled(i2c, i2c_addr);
    oled.init();
    //oled.set_fg(Oled::Color::Black);
    oled.on();
    oled.clear();

    demo[demo_num].func(oled);

    int32_t t0_us = int32_t(time_us_32());

    oled.flush();

    int32_t t1_us = int32_t(time_us_32());

    I2cDev::Status status;
    while ((status = i2c.write_status()) == I2cDev::Status::Busy)
        tight_loop_contents();

    int32_t t2_us = int32_t(time_us_32());

    printf("async flush status: %d\n", int(status));
    printf("call to oled.flush: %ld usec\n", t1_us - t0_us);
    printf("i2c bus time: %ld usec\n", t2_us - t1_us);

} // void run_demo()


static void do_hello(Oled &oled)
{
    // Say hi:
    //
    // +--------------------------------+
    // |********************************|
    // |*                              *|
    // |*        Hello, World!         *|
    // |*                              *|
    // |********************************|
    // +--------------------------------+

    // box
    for (int col = 0; col < oled.cols; col++) {
        oled.set(col, 0);
        oled.set(col, oled.rows - 1);
    }
    for (int row = 0; row < oled.rows; row++) {
        oled.set(0, row);
        oled.set(oled.cols - 1, row);
    }

    oled.set_align(Oled::HAlign::Center, Oled::VAlign::Center);
    oled.set_font(font);

    const char *msg = "Hello, World!";

    oled.puts(oled.cols / 2, oled.rows / 2, msg);
}


static void do_fullscreen(Oled &oled)
{
    // Fill screen. This can be used to measure maximum power consumption.
    //
    //          100%              50% horizontal          50% vertical
    // +--------------------+ +--------------------+ +--------------------+
    // |********************| |********************| |* * * * * * * * * * |
    // |********************| |                    | |* * * * * * * * * * |
    // |********************| |********************| |* * * * * * * * * * |
    // |********************| |                    | |* * * * * * * * * * |
    // +--------------------+ +--------------------+ +--------------------+
    //
    // My screen (128x64) draws:
    //   0.5 mA with no pixels on
    //   10 mA with half of them on
    //   17 mA with all pixels on
    //
    // Vertical stripes (10 mA) draws about 0.2 mA more than horizontal
    // stripes (9.8 mA), curious....

    static int pattern = 0;
    constexpr int pattern_cnt = 3;

    if (pattern == 0) {
        // all pixels on
        oled.clear(Oled::Color::White);

    } else if (pattern == 1) {
        // horizontal stripes, half pixels on
        for (int r = 0; r < oled.rows; r += 2)
            oled.fill(0, r, oled.cols, 1, Oled::Color::White);

    } else if (pattern == 2) {
        // vertical stripes, half pixels on
        for (int c = 0; c < oled.cols; c += 2)
            oled.fill(c, 0, 1, oled.rows, Oled::Color::White);
    }

    pattern = (pattern + 1) % pattern_cnt;
}


static void do_dots(Oled &oled)
{
    // Single pixel in each corner:
    //
    // +--------------------------------+
    // |*                              *|
    // |                                |
    // |                                |
    // |                                |
    // |*                              *|
    // +--------------------------------+

    oled.set(0, 0);
    oled.set(0, oled.rows - 1);
    oled.set(oled.cols - 1, 0);
    oled.set(oled.cols - 1, oled.rows - 1);
}


static void do_stripes(Oled &oled)
{
    // Stripes:
    //
    // +--------------------------------+
    // |\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ |
    // | \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \|
    // |\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ |
    // | \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \|
    // |\ \ \ \ \ \ \ \ \ \ \ \ \ \ \ \ |
    // +--------------------------------+

    for (int col = 0; col < oled.cols; col += 4)
        for (int row = 0; row < oled.rows; row++)
            oled.set((col + row) % oled.cols, row);
}


// Font generally has these printable characters:
// 0x20  !"#$%&'()*+,-./
// 0x30 0123456789:;<=>?
// 0x40 @ABCDEFGHIJKLMNO
// 0x50 PQRSTUVWXYZ[\]^_
// 0x60 `abcdefghijklmno
// 0x70 pqrstuvwxyz{|}~


static void do_chars1a(Oled &oled)
{
    // Draw characters in font:
    //
    // +---------------------+
    // |                     |
    // |    ABCDEFGHIJKLM    |
    // |    NOPQRSTUVWXYZ    |
    // |                     |
    // +---------------------+

    oled.set_font(font);
    oled.set_align(Oled::HAlign::Center, Oled::VAlign::Center);
    oled.puts(Oled::cols / 2, Oled::rows / 2,
              "ABCDEFGHIJKLM\n"
              "NOPQRSTUVWXYZ");
}


static void do_chars1b(Oled &oled)
{
    // Draw characters in font:
    //
    // +---------------------+
    // |                     |
    // |    abcdefghijklm    |
    // |    nopqrstuvwxyz    |
    // |                     |
    // +---------------------+

    oled.set_font(font);
    oled.set_align(Oled::HAlign::Center, Oled::VAlign::Center);
    oled.puts(Oled::cols / 2, Oled::rows / 2,
              "abcdefghijklm\n"
              "nopqrstuvwxyz");
}


static void do_chars2a(Oled &oled)
{
    // Draw characters in font:
    //
    // +--------------------+
    // |                    |
    // |  !"#$%&'()*+,-./:  |
    // |  ;<=>?@[\]^_`{|}~  |
    // |                    |
    // +--------------------+

    oled.set_font(font);
    oled.set_align(Oled::HAlign::Center, Oled::VAlign::Center);
    oled.puts(Oled::cols / 2, Oled::rows / 2,
              "!\"#$%&'()*+,-./:\n"
              ";<=>?@[\\]^_`{|}~");
}


static void do_chars2b(Oled &oled)
{
    // Draw characters in font:
    //
    // +--------------------+
    // |                    |
    // |     0123456789     |
    // |     <special>      |
    // |                    |
    // +--------------------+

    char msg[32];
    char *msg_eos = msg + sizeof(msg) - 1; // must always be '\0' here
    memset(msg, '\0', sizeof(msg));
    strcpy(msg,
           "0123456789\n");
    char *msg_end;
    for (msg_end = msg; *msg_end != '\0'; msg_end++)
        ;

    // append anything below 0x20 that has width > 0 and has any pixels set
    for (char c = 0x00; c < 0x20; c++) {
        if (font->width(c) == 0)
            continue;
        const uint16_t *pd = font->pixel_data(c);
        for (int i = 0; i < font->char_rows(); i++) {
            if (pd[i] != 0) {
                *msg_end++ = c;
                break;
            }
        }
        if (msg_end >= msg_eos)
            break;
    }

    oled.set_font(font);
    oled.set_align(Oled::HAlign::Center, Oled::VAlign::Center);
    oled.puts(Oled::cols / 2, Oled::rows / 2, msg);
}


static void do_double(Oled &oled, const char *s)
{
    // One line of double-size text:
    //
    // +--------------------+
    // |                    |
    // |      <string>      |
    // |                    |
    // +--------------------+

    oled.set_font(font);
    oled.set_double(true);
    oled.set_align(Oled::HAlign::Center, Oled::VAlign::Center);
    oled.puts(Oled::cols / 2, Oled::rows / 2, s);
    oled.set_double(false);
}


static void do_hello_2x(Oled &oled)
{
    do_double(oled, "Hello!");
}


static void do_chars1a_2x(Oled &oled)
{
    do_double(oled, "ABCDEFGH");
}


static void do_chars1b_2x(Oled &oled)
{
    do_double(oled, "abcdefgh");
}


static void do_chars2a_2x(Oled &oled)
{
    do_double(oled, "%&()+-*/");
}


static void do_chars2b_2x(Oled &oled)
{
    do_double(oled, "<=>?@[]^_");
}


static void do_chars2c_2x(Oled &oled)
{
    do_double(oled, "4,321.0");
}


static void do_chars2d_2x(Oled &oled)
{
    do_double(oled, "9,876.5");
}


static void do_chars2e_2x(Oled &oled)
{
    do_double(oled, "3.792 mV");
}


#pragma GCC diagnostic pop
