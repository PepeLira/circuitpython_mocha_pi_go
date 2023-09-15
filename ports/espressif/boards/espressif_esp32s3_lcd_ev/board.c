/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Scott Shawcroft for Adafruit Industries
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "supervisor/board.h"
#include "mpconfigboard.h"
#include "shared-bindings/board/__init__.h"
#include "shared-bindings/busio/I2C.h"
#include "shared-bindings/dotclockframebuffer/DotClockFramebuffer.h"
#include "shared-bindings/dotclockframebuffer/__init__.h"
#include "shared-bindings/framebufferio/FramebufferDisplay.h"
#include "shared-bindings/microcontroller/Pin.h"
#include "shared-module/displayio/__init__.h"

static const uint8_t display_init_sequence[] = {
    0xf0, 5, 0x55, 0xaa, 0x52, 0x08, 0x00,
    0xf6, 2, 0x5a, 0x87,
    0xc1, 1, 0x3f,
    0xc2, 1, 0x0e,
    0xc6, 1, 0xf8,
    0xc9, 1, 0x10,
    0xcd, 1, 0x25,
    0xf8, 1, 0x8a,
    0xac, 1, 0x45,
    0xa0, 1, 0xdd,
    0xa7, 1, 0x47,
    0xfa, 4, 0x00, 0x00, 0x00, 0x04,
    0x86, 4, 0x99, 0xa3, 0xa3, 0x51,
    0xa3, 1, 0xee,
    0xfd, 3, 0x3c, 0x3c, 0x00,
    0x71, 1, 0x48,
    0x72, 1, 0x48,
    0x73, 2, 0x00, 0x44,
    0x97, 1, 0xee,
    0x83, 1, 0x93,
    0x9a, 1, 0x72,
    0x9b, 1, 0x5a,
    0x82, 2, 0x2c, 0x2c,
    0xb1, 1, 0x10,
    0x6d, 32, 0x00, 0x1f, 0x19, 0x1a, 0x10, 0x0e, 0x0c, 0x0a, 0x02, 0x07, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x08, 0x01, 0x09, 0x0b, 0x0d, 0x0f, 0x1a, 0x19, 0x1f, 0x00,
    0x64, 16, 0x38, 0x05, 0x01, 0xdb, 0x03, 0x03, 0x38, 0x04, 0x01, 0xdc, 0x03, 0x03, 0x7a, 0x7a, 0x7a, 0x7a,
    0x65, 16, 0x38, 0x03, 0x01, 0xdd, 0x03, 0x03, 0x38, 0x02, 0x01, 0xde, 0x03, 0x03, 0x7a, 0x7a, 0x7a, 0x7a,
    0x66, 16, 0x38, 0x01, 0x01, 0xdf, 0x03, 0x03, 0x38, 0x00, 0x01, 0xe0, 0x03, 0x03, 0x7a, 0x7a, 0x7a, 0x7a,
    0x67, 16, 0x30, 0x01, 0x01, 0xe1, 0x03, 0x03, 0x30, 0x02, 0x01, 0xe2, 0x03, 0x03, 0x7a, 0x7a, 0x7a, 0x7a,
    0x68, 13, 0x00, 0x08, 0x15, 0x08, 0x15, 0x7a, 0x7a, 0x08, 0x15, 0x08, 0x15, 0x7a, 0x7a,
    0x60, 8, 0x38, 0x08, 0x7a, 0x7a, 0x38, 0x09, 0x7a, 0x7a,
    0x63, 8, 0x31, 0xe4, 0x7a, 0x7a, 0x31, 0xe5, 0x7a, 0x7a,
    0x69, 7, 0x04, 0x22, 0x14, 0x22, 0x14, 0x22, 0x08,
    0x6b, 1, 0x07,
    0x7a, 2, 0x08, 0x13,
    0x7b, 2, 0x08, 0x13,
    0xd1, 52, 0x00, 0x00, 0x00, 0x04, 0x00, 0x12, 0x00, 0x18, 0x00, 0x21, 0x00, 0x2a, 0x00, 0x35, 0x00, 0x47, 0x00, 0x56, 0x00, 0x90, 0x00, 0xe5, 0x01, 0x68, 0x01, 0xd5, 0x01, 0xd7, 0x02, 0x36, 0x02, 0xa6, 0x02, 0xee, 0x03, 0x48, 0x03, 0xa0, 0x03, 0xba, 0x03, 0xc5, 0x03, 0xd0, 0x03, 0xe0, 0x03, 0xea, 0x03, 0xfa, 0x03, 0xff,
    0xd2, 52, 0x00, 0x00, 0x00, 0x04, 0x00, 0x12, 0x00, 0x18, 0x00, 0x21, 0x00, 0x2a, 0x00, 0x35, 0x00, 0x47, 0x00, 0x56, 0x00, 0x90, 0x00, 0xe5, 0x01, 0x68, 0x01, 0xd5, 0x01, 0xd7, 0x02, 0x36, 0x02, 0xa6, 0x02, 0xee, 0x03, 0x48, 0x03, 0xa0, 0x03, 0xba, 0x03, 0xc5, 0x03, 0xd0, 0x03, 0xe0, 0x03, 0xea, 0x03, 0xfa, 0x03, 0xff,
    0xd3, 52, 0x00, 0x00, 0x00, 0x04, 0x00, 0x12, 0x00, 0x18, 0x00, 0x21, 0x00, 0x2a, 0x00, 0x35, 0x00, 0x47, 0x00, 0x56, 0x00, 0x90, 0x00, 0xe5, 0x01, 0x68, 0x01, 0xd5, 0x01, 0xd7, 0x02, 0x36, 0x02, 0xa6, 0x02, 0xee, 0x03, 0x48, 0x03, 0xa0, 0x03, 0xba, 0x03, 0xc5, 0x03, 0xd0, 0x03, 0xe0, 0x03, 0xea, 0x03, 0xfa, 0x03, 0xff,
    0xd4, 52, 0x00, 0x00, 0x00, 0x04, 0x00, 0x12, 0x00, 0x18, 0x00, 0x21, 0x00, 0x2a, 0x00, 0x35, 0x00, 0x47, 0x00, 0x56, 0x00, 0x90, 0x00, 0xe5, 0x01, 0x68, 0x01, 0xd5, 0x01, 0xd7, 0x02, 0x36, 0x02, 0xa6, 0x02, 0xee, 0x03, 0x48, 0x03, 0xa0, 0x03, 0xba, 0x03, 0xc5, 0x03, 0xd0, 0x03, 0xe0, 0x03, 0xea, 0x03, 0xfa, 0x03, 0xff,
    0xd5, 52, 0x00, 0x00, 0x00, 0x04, 0x00, 0x12, 0x00, 0x18, 0x00, 0x21, 0x00, 0x2a, 0x00, 0x35, 0x00, 0x47, 0x00, 0x56, 0x00, 0x90, 0x00, 0xe5, 0x01, 0x68, 0x01, 0xd5, 0x01, 0xd7, 0x02, 0x36, 0x02, 0xa6, 0x02, 0xee, 0x03, 0x48, 0x03, 0xa0, 0x03, 0xba, 0x03, 0xc5, 0x03, 0xd0, 0x03, 0xe0, 0x03, 0xea, 0x03, 0xfa, 0x03, 0xff,
    0xd6, 52, 0x00, 0x00, 0x00, 0x04, 0x00, 0x12, 0x00, 0x18, 0x00, 0x21, 0x00, 0x2a, 0x00, 0x35, 0x00, 0x47, 0x00, 0x56, 0x00, 0x90, 0x00, 0xe5, 0x01, 0x68, 0x01, 0xd5, 0x01, 0xd7, 0x02, 0x36, 0x02, 0xa6, 0x02, 0xee, 0x03, 0x48, 0x03, 0xa0, 0x03, 0xba, 0x03, 0xc5, 0x03, 0xd0, 0x03, 0xe0, 0x03, 0xea, 0x03, 0xfa, 0x03, 0xff,
    0x3a, 1, 0x66,
    0x3a, 1, 0x66,
    0x11, 0x80, 120,
    0x29, 0x80, 20
};

static const mcu_pin_obj_t *red_pins[] = {
    &pin_GPIO1, &pin_GPIO2, &pin_GPIO42, &pin_GPIO41, &pin_GPIO40
};
static const mcu_pin_obj_t *green_pins[] = {
    &pin_GPIO21, &pin_GPIO47, &pin_GPIO48, &pin_GPIO45, &pin_GPIO38, &pin_GPIO39
};
static const mcu_pin_obj_t *blue_pins[] = {
    &pin_GPIO10, &pin_GPIO11, &pin_GPIO12, &pin_GPIO13, &pin_GPIO14
};
void board_init(void) {
    dotclockframebuffer_framebuffer_obj_t *framebuffer = &allocate_display_bus_or_raise()->dotclock;
    framebuffer->base.type = &dotclockframebuffer_framebuffer_type;

    common_hal_dotclockframebuffer_framebuffer_construct(
        framebuffer,
        /* de */ &pin_GPIO17,
        /* vsync */ &pin_GPIO3,
        /* hsync */ &pin_GPIO46,
        /* dclk */ &pin_GPIO9,
        /* data */ red_pins, MP_ARRAY_SIZE(red_pins), green_pins, MP_ARRAY_SIZE(green_pins), blue_pins, MP_ARRAY_SIZE(blue_pins),
        /* frequency */ 6500000,
        /* width x height */ 480, 480,
        /* horizontal: pulse, back &  front porch, idle */ 13, 20, 40, false,
        /* vertical: pulse, back &  front porch, idle */ 15, 20, 40, false,
        /* de_idle_high */ false,
        /* pclk_active_high */ true,
        /* pclk_idle_high */ false,
        /* overscan_left */ 0
        );

    framebufferio_framebufferdisplay_obj_t *disp = &allocate_display_or_raise()->framebuffer_display;
    disp->base.type = &framebufferio_framebufferdisplay_type;
    common_hal_framebufferio_framebufferdisplay_construct(
        disp,
        framebuffer,
        0,
        true
        );

    busio_i2c_obj_t *i2c = common_hal_board_create_i2c(0);
    const int i2c_device_address = 32;

    common_hal_busio_i2c_try_lock(i2c);

    {
        uint8_t buf[2] = {3, 0xf1}; // set GPIO direction
        common_hal_busio_i2c_write(i2c, i2c_device_address, buf, sizeof(buf));
    }

    {
        uint8_t buf[2] = {2, 0}; // set all output pins low initially
        common_hal_busio_i2c_write(i2c, i2c_device_address, buf, sizeof(buf));
    }

    common_hal_busio_i2c_unlock(i2c);

    dotclockframebuffer_ioexpander_spi_bus spibus = {
        .bus = i2c,
        .i2c_device_address = i2c_device_address,
        .i2c_write_size = 2,
        .addr_reg_shadow = { .u32 = 1 }, // GPIO data at register 1
        .cs_mask = 0x100 << 1, // data payload is at byte 2
            .mosi_mask = 0x100 << 3,
            .clk_mask = 0x100 << 2,
    };

    dotclockframebuffer_ioexpander_send_init_sequence(&spibus, display_init_sequence, sizeof(display_init_sequence));

}

// Use the MP_WEAK supervisor/shared/board.c versions of routines not defined here.
