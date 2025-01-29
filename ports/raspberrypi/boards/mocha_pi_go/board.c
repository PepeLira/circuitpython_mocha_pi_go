// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Pepe
//
// SPDX-License-Identifier: MIT

#include "supervisor/board.h"
#include "mpconfigboard.h"
#include "shared-bindings/busio/SPI.h"
#include "shared-bindings/fourwire/FourWire.h"
#include "shared-module/displayio/__init__.h"
#include "shared-module/displayio/mipi_constants.h"
#include "supervisor/shared/board.h"
#include "shared-bindings/audiopwmio/PWMAudioOut.h"


// display init sequence from CircuitPython library https://github.com/adafruit/Adafruit_CircuitPython_ST7735R/blob/dfae353330cf051d1f31db9e4b681c8d70900cc5/adafruit_st7735r.py
uint8_t display_init_sequence[] = {
    // sw reset
    0x01, 0x80, 0x96,
    // sleep out and delay
    0x11, 0x80, 0xFF,
    // _FRMCTR1
    0xB1, 0x03, 0x01, 0x2C, 0x2D,
    // _FRMCTR2
    0xB2, 0x03, 0x01, 0x2C, 0x2D,
    // _FRMCTR3
    0xB3, 0x06, 0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D,
    // _INVCTR line inversion
    0xB4, 0x01, 0x07,
    // _PWCTR1 GVDD = 4.7V, 1.0uA
    0xC0, 0x03, 0xA2, 0x02, 0x84,
    // _PWCTR2 VGH=14.7V, VGL=-7.35V
    0xC1, 0x01, 0xC5,
    // _PWCTR3 Opamp current small, Boost frequency
    0xC2, 0x02, 0x0A, 0x00,
    0xC3, 0x02, 0x8A, 0x2A,
    0xC4, 0x02, 0x8A, 0xEE,
    // _VMCTR1 VCOMH = 4V, VOML = -1.1V
    0xC5, 0x01, 0x0E,
    // _INVOFF
    0x20, 0x00,
    // _MADCTL bottom to top refresh
    0x36, 0x01, 0x18,
    // COLMOD - 16 bit color
    0x3A, 0x01, 0x05,
    // _GMCTRP1 Gamma
    0xE0, 0x10, 0x02, 0x1C, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2D, 0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10,
    // _GMCTRN1
    0xE1, 0x10, 0x03, 0x1d, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D, 0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10,
    // _NORON
    0x13, 0x80, 0x0A,
    // _DISPON
    0x29, 0x80, 0x64,
    // _MADCTL Default rotation + BGR encoding
    0x36, 0x01, 0xC0,
};


void board_init(void) {
    // Allocate the FourWire bus
    fourwire_fourwire_obj_t *bus = &allocate_display_bus()->fourwire_bus;
    busio_spi_obj_t *spi = &bus->inline_bus;

    // Construct SPI using SCLK=GPIO18, MOSI=GPIO19, no MISO
    common_hal_busio_spi_construct(spi, &pin_GPIO18, &pin_GPIO19, NULL, false);
    common_hal_busio_spi_never_reset(spi);

    // Construct FourWire interface
    // CS pin is NULL because it's tied to ground
    bus->base.type = &fourwire_fourwire_type;
    common_hal_fourwire_fourwire_construct(
        bus,
        spi,
        &pin_GPIO20,  // DC (ddc)
        NULL,         // CS is grounded, so no software control
        &pin_GPIO21,  // RST (drst)
        30000000,     // SPI clock up to ~30MHz
        0,            // polarity
        0             // phase
    );

    // Construct the actual display
    busdisplay_busdisplay_obj_t *display = &allocate_display()->display;
    display->base.type = &busdisplay_busdisplay_type;
    common_hal_busdisplay_busdisplay_construct(
        display,
        bus,
        160,   // width
        128,   // height
        0,     // column start
        0,     // row start
        270,   // rotation in degrees
        16,    // color depth (16-bit / 65K colors)
        false, // grayscale=False
        false, // pixels_in_byte_share_row=False
        1,     // bytes_per_cell=1
        false, // reverse_pixels_in_byte=False
        true,  // reverse_bytes_in_word=True (common on SPI TFT)
        MIPI_COMMAND_SET_COLUMN_ADDRESS,
        MIPI_COMMAND_SET_PAGE_ADDRESS,
        MIPI_COMMAND_WRITE_MEMORY_START,
        display_init_sequence,
        sizeof(display_init_sequence),
        &pin_GPIO22,           // backlight pin (dbl)
        NO_BRIGHTNESS_COMMAND, // no special brightness command
        1.0f,                  // full brightness
        false, // single_byte_bounds
        false, // data_as_commands
        true,  // auto_refresh
        60,    // native_frames_per_second
        true,  // backlight_on_high
        false, // SH1107_addressing
        50000);  // backlight PWM frequency

    board_buzz_obj.base.type = &audiopwmio_pwmaudioout_type;
    common_hal_audiopwmio_pwmaudioout_construct(&board_buzz_obj,
        &pin_GPIO7, NULL, 0x8000);
    never_reset_pin_number(pin_GPIO7.port, pin_GPIO7.number);
}

// Use the MP_WEAK supervisor/shared/board.c versions of routines not defined here.
