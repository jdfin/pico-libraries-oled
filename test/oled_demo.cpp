
#include <cassert>
#include <cstdint>
#include <cstdio>
// pico
#include "pico/stdio.h"
#include "pico/stdio_usb.h"
#include "pico/stdlib.h"
#include "pico/time.h" // sleep_ms
// pico hardware
#include "hardware/gpio.h"
#include "hardware/i2c.h"
// misc lib
#include "i2c_dev.h"
#include "util.h" // mem_name
// oled lib
#include "chicago_12.h"
#include "font.h"
#include "oled.h"

constexpr uint8_t i2c_addr = 0x3c; // SSD1306: 0x3c or 0x3d
static i2c_inst_t *i2c_dev = i2c1; // RP2040/RP2350: i2c0 or i2c1

static constexpr int sda_gpio = 14;
static constexpr int scl_gpio = 15;
static constexpr int dma_gpio = 16; // asserted during DMA (or -1)

static void do_hello(Oled &oled);
[[maybe_unused]] static void do_dots(Oled &oled);
[[maybe_unused]] static void do_stripes(Oled &oled);
static void do_chars1(Oled &oled);
static void do_chars2(Oled &oled);

static const Font *font = &chicago_12;

static struct {
    const char *name;
    void (*func)(Oled &oled);
} tests[] = {
    {"hello", do_hello},
    //{"dots", do_dots},
    //{"stripes", do_stripes},
    {"chars1", do_chars1},
    {"chars2", do_chars2},
};
static constexpr int tests_max = sizeof(tests) / sizeof(tests[0]);

static void run_test(int test_num);


int main()
{
    if (dma_gpio >= 0) {
        gpio_init(dma_gpio);
        gpio_put(dma_gpio, false);
        gpio_set_dir(dma_gpio, true); // out
    }

    stdio_init_all();

    // wait up to 'wait_us' for usb serial to connect
    constexpr int32_t wait_us = 20'000'000;
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

    int test_num = 0;
    while (true) {
        run_test(test_num);
        test_num = (test_num + 1) % tests_max;
        sleep_ms(5000);
    }

    return 0;

} // int main()


static void run_test(int test_num)
{
    printf("\n");
    printf("%s\n", tests[test_num].name);

    I2cDev i2c(i2c_dev, scl_gpio, sda_gpio, Oled::baud);

    printf("i2c: baud requested=%u actual=%u\n", Oled::baud, i2c.baud());
    sleep_ms(10);

    Oled oled(i2c, i2c_addr);
    oled.init();
    oled.on();
    oled.clear();

    tests[test_num].func(oled);

    if (dma_gpio >= 0)
        gpio_put(dma_gpio, true);

    int32_t t0_us = int32_t(time_us_32());

    oled.flush();

    int32_t t1_us = int32_t(time_us_32());

    I2cDev::Status status;
    while ((status = i2c.write_async_status()) == I2cDev::Status::Busy)
        tight_loop_contents();

    int32_t t2_us = int32_t(time_us_32());

    if (dma_gpio >= 0)
        gpio_put(dma_gpio, false);

    printf("async flush status: %d\n", int(status));
    printf("call to oled.flush: %ld usec\n", t1_us - t0_us);
    printf("i2c bus time: %ld usec\n", t2_us - t1_us);

} // void run_test()


[[maybe_unused]]
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

    const char *msg = "Hello, World!";
    int msg_wid = font->width(msg);

    int x = oled.cols / 2 - msg_wid / 2;
    int y = oled.rows / 2 - font->height() / 2;

    oled.puts(x, y, msg, font);

    // box it

    for (int x = 0; x < oled.cols; x++) {
        oled.set(x, 0);
        oled.set(x, oled.rows - 1);
    }

    for (int y = 0; y < oled.rows; y++) {
        oled.set(0, y);
        oled.set(oled.cols - 1, y);
    }
}


[[maybe_unused]]
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


[[maybe_unused]]
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

    for (int x = 0; x < oled.cols; x += 4)
        for (int y = 0; y < oled.rows; y++)
            oled.set((x + y) % oled.cols, y);
}


// Font generally has these printable characters:
// 0x20  !"#$%&'()*+,-./
// 0x30 0123456789:;<=>?
// 0x40 @ABCDEFGHIJKLMNO
// 0x50 PQRSTUVWXYZ[\]^_
// 0x60 `abcdefghijklmno
// 0x70 pqrstuvwxyz{|}~

// This assumes font->line_hgt <= oled.rows / 4
static constexpr int row0 = 0;
static constexpr int row1 = Oled::rows / 4;
static constexpr int row2 = Oled::rows / 2;
static constexpr int row3 = 3 * Oled::rows / 4;


[[maybe_unused]]
static void do_chars1(Oled &oled)
{
    // Draw characters in font:
    //
    // +--------------------------------+
    // | ABCDEFGHIJKLM                  |
    // | NOPQRSTUVWXYZ                  |
    // | abcdefghijklm                  |
    // | nopqrstuvwxyz                  |
    // +--------------------------------+

    oled.puts(0, row0, "ABCDEFGHIJKLM", font);
    oled.puts(0, row1, "NOPQRSTUVWXYZ", font);
    oled.puts(0, row2, "abcdefghijklm", font);
    oled.puts(0, row3, "nopqrstuvwxyz", font);
}


[[maybe_unused]]
static void do_chars2(Oled &oled)
{
    // Draw characters in font:
    //
    // +--------------------------------+
    // | 0123456789                     |
    // | !"#$%&'()*+                    |
    // | ,-./:;<=>?@                    |
    // | [\]^_`{|}~                     |
    // +--------------------------------+

    oled.puts(0, row0, "0123456789", font);
    oled.puts(0, row1, "!\"#$%&'()*+", font);
    oled.puts(0, row2, ",-./:;<=>?@", font);
    oled.puts(0, row3, "[\\]^_`{|}~", font);
}
