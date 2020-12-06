/**
 * Copyright (c) 2020, Robert Maupin
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include "bt81x.h"

#include "device.h"

#define ROM          0x200000
#define ROM_FONT     0x1E0000
#define ROM_FONTROOT 0x2FFFFC

#define RAM_G          0x0
#define RAM_DL         0x300000
#define RAM_REG        0x302000
#define RAM_CMD        0x308000
#define RAM_ERR_REPORT 0x309800

#define FLASH 0x800000

/* registers */
#define REG_ID                 0x302000
#define REG_FRAMES             0x302004
#define REG_CLOCK              0x302008
#define REG_FREQUENCY          0x30200C
#define REG_RENDERMODE         0x302010
#define REG_SNAPY              0x302014
#define REG_SNAPSHOT           0x302018
#define REG_SNAPFORMAT         0x30201C
#define REG_CPURESET           0x302020
#define REG_TAP_CRC            0x302024
#define REG_TAP_MASK           0x302028
#define REG_HCYCLE             0x30202C
#define REG_HOFFSET            0x302030
#define REG_HSIZE              0x302034
#define REG_HSYNC0             0x302038
#define REG_HSYNC1             0x30203C
#define REG_VCYCLE             0x302040
#define REG_VOFFSET            0x302044
#define REG_VSIZE              0x302048
#define REG_VSYNC0             0x30204C
#define REG_VSYNC1             0x302050
#define REG_DLSWAP             0x302054
#define REG_ROTATE             0x302058
#define REG_OUTBITS            0x30205C
#define REG_DITHER             0x302060
#define REG_SWIZZLE            0x302064
#define REG_CSPREAD            0x302068
#define REG_PCLK_POL           0x30206C
#define REG_PCLK               0x302070
#define REG_TAG_X              0x302074
#define REG_TAG_Y              0x302078
#define REG_TAG                0x30207C
#define REG_VOL_PB             0x302080
#define REG_VOL_SOUND          0x302084
#define REG_SOUND              0x302088
#define REG_PLAY               0x30208C
#define REG_GPIO_DIR           0x302090
#define REG_GPIO               0x302094
#define REG_GPIOX_DIR          0x302098
#define REG_GPIOX              0x30209C
#define REG_INT_FLAGS          0x3020A8
#define REG_INT_EN             0x3020AC
#define REG_INT_MASK           0x3020B0
#define REG_PLAYBACK_START     0x3020B4
#define REG_PLAYBACK_LENGTH    0x3020B8
#define REG_PLAYBACK_READPTR   0x3020BC
#define REG_PLAYBACK_FREQ      0x3020C0
#define REG_PLAYBACK_FORMAT    0x3020C4
#define REG_PLAYBACK_LOOP      0x3020C8
#define REG_PLAYBACK_PLAY      0x3020CC
#define REG_PWM_HZ             0x3020D0
#define REG_PWM_DUTY           0x3020D4
#define REG_MACRO_0            0x3020D8
#define REG_MACRO_1            0x3020DC
#define REG_CMD_READ           0x3020F8
#define REG_CMD_WRITE          0x3020FC
#define REG_CMD_DL             0x302100
#define REG_BIST_EN            0x302174
#define REG_TRIM               0x302180
#define REG_ANA_COMP           0x302184
#define REG_SPI_WIDTH          0x302188
#define REG_DATESTAMP          0x302564
#define REG_CMDB_SPACE         0x302574
#define REG_CMDB_WRITE         0x302578
#define REG_ADAPTIVE_FRAMERATE 0x30257C
#define REG_PLAYBACK_PAUSE     0x3025EC
#define REG_FLASH_STATUS       0x3025F0

/* register values */
#define DLSWAP_LINE                        0x1
#define DLSWAP_FRAME                       0x2
#define ROTATE_LANDSCAPE                   0x0
#define ROTATE_INVERTED_LANDSCAPE          0x1
#define ROTATE_PORTRAIT                    0x2
#define ROTATE_INVERTED_PORTRAIT           0x3
#define ROTATE_MIRRORED_LANDSCAPE          0x4
#define ROTATE_MIRRORED_INVERTED_LANDSCAPE 0x5
#define ROTATE_MIRRORED_PORTRAIT           0x6
#define ROTATE_MIRRORED_INVERTED_PORTRAIT  0x7

/* other registers */
#define CHIP_ID 0x0C0000

/* host commands */
#define ACTIVE       0x0
#define STANDBY      0x41
#define SLEEP        0x42
#define PWRDOWN      0x43  // 0x50
#define CLKEXT       0x44
#define CLKINT       0x48
#define CLKSELA      0x61
#define CLKSELB      0x62
#define RST_PULSE    0x68
#define PINDRIVE     0x70
#define PIN_PD_STATE 0x71

/* display list commands */
#define BEGIN(prim)              (0x1F000000 | (prim))
#define CLEAR_COLOR_RGB(r, g, b) (0x2000000 | ((r) << 16) | ((b) << 8) | (g))
#define CLEAR(c, s, t)           (0x26000000 | ((c) << 2) | ((s) << 1) | (t))
#define COLOR_RGB(r, g, b)       (0x4000000 | ((r) << 16) | ((b) << 8) | (g))
#define DISPLAY()                0x0
#define END()                    0x21000000
#define POINT_SIZE(size)         (0xD000000 | (size))
#define VERTEX2II(x, y, handle, cell) \
  (0x80000000 | ((x) << 21) | ((y) << 12) | ((handle) << 7) | (cell))

/* display list command params */
#define BITMAPS      1
#define POINTS       2
#define LINES        3
#define LINE_STRIP   4
#define EDGE_STRIP_R 5
#define EDGE_STRIP_L 6
#define EDGE_STRIP_A 7
#define EDGE_STRIP_B 8
#define RECTS        9

/* built in configurations */
const bt81xcfg_t bt81x_eve3_70 = {
    1056, /* hcycle */
    246,  /* hoffset */
    210,  /* hsync0 */
    230,  /* hsync1 */
    525,  /* vcycle */
    45,   /* voffset */
    22,   /* vsync0 */
    35,   /* vsync1 */
    0,    /* swizzle */
    2,    /* pclk */
    1,    /* pclk_pol */
    0,    /* cspread */
    1,    /* dither */
    800,  /* hsize */
    480   /* vsize */
};

const bt81xcfg_t bt81x_eve2_70 = {
    928, /* hcycle */
    88,  /* hoffset */
    0,   /* hsync0 */
    48,  /* hsync1 */
    525, /* vcycle */
    32,  /* voffset */
    0,   /* vsync0 */
    3,   /* vsync1 */
    0,   /* swizzle */
    2,   /* pclk */
    1,   /* pclk_pol */
    0,   /* cspread */
    1,   /* dither */
    800, /* hsize */
    480  /* vsize */
};

void wr(const uint32_t address, const uint8_t *data, const uint16_t size) {
  const uint8_t addr[3] = {((address >> 16) & 0x3F) | 0x80,
                           (address >> 8) & 0xFF, address & 0xFF};
  device_spi_start();
  device_spi_write(addr, 3);
  device_spi_write(data, size);
  device_spi_end();
}

void wr8(const uint32_t address, const uint8_t data) {
  wr(address, &data, 1);
}

void wr16(const uint32_t address, const uint16_t data) {
  const uint8_t td[2] = {data & 0xFF, (data >> 8) & 0xFF};
  wr(address, td, 2);
}

void wr32(const uint32_t address, const uint32_t data) {
  const uint8_t td[4] = {data & 0xFF, (data >> 8) & 0xFF, (data >> 16) & 0xFF,
                         (data >> 24) & 0xFF};
  wr(address, td, 4);
}

void rd(const uint32_t address, uint8_t *data, const uint16_t size) {
  const uint8_t addr[4] = {(address >> 16) & 0x3F, (address >> 8) & 0xFF,
                           address & 0xFF, 0};
  device_spi_start();
  device_spi_write(addr, 4);
  device_spi_read(data, size);
  device_spi_end();
}

uint8_t rd8(const uint32_t address) {
  uint8_t data = 0;
  rd(address, &data, 1);
  return data;
}

uint16_t rd16(const uint32_t address) {
  uint8_t data[2] = {0};
  rd(address, data, 2);
  return (data[1] << 8) | data[0];
}

uint32_t rd32(const uint32_t address) {
  uint8_t data[4] = {0};
  rd(address, data, 4);
  return (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
}

// void cmd(const uint32_t cmd);

void wdla(const uint32_t address, const uint32_t data) {
  wr32(RAM_DL + address, data);
}

void host_command(const uint8_t cmd) {
  device_spi_start();
  device_spi_write8(cmd);
  device_spi_write8(0);
  device_spi_write8(0);
  device_spi_end();
}

void host_command_param(const uint8_t cmd, const uint8_t param) {
  device_spi_start();
  device_spi_write8(cmd);
  device_spi_write8(param);
  device_spi_write8(0);
  device_spi_end();
}

uint32_t bt81x_read_chipid() {
  return rd32(0xC0000);
}

uint32_t bt81x_read_frequency() {
  return rd32(REG_FREQUENCY);
}

void bt81x_cmd_active() {
  host_command(ACTIVE);
}

void bt81x_cmd_standby() {
  host_command(STANDBY);
}

void bt81x_cmd_clockext() {
  host_command(CLKEXT);
}

void bt81x_cmd_clockint() {
  host_command(CLKINT);
}

void bt81x_cmd_sleep() {
  host_command(SLEEP);
}

void bt81x_cmd_powerdown() {
  host_command(PWRDOWN);
}

void bt81x_cmd_reset() {
  host_command(RST_PULSE);
}

void bt81x_start() {
  bt81x_cmd_clockext();
  bt81x_cmd_active();
  bt81x_cmd_active();
  device_delay(300);

  // wait to be active
  while (rd8(REG_ID) != 0x7C) {
    device_delay(5);
  }

  // wait for reset
  while (rd8(REG_CPURESET) != 0x0) {
    device_delay(5);
  }
}

void bt81x_configure(const bt81xcfg_t *config) {
  // disable display
  wr16(REG_GPIOX, 0);
  wr8(REG_PCLK, 0);

  // set display values
  wr16(REG_HCYCLE, config->hcycle);
  wr16(REG_HOFFSET, config->hoffset);
  wr16(REG_HSYNC0, config->hsync0);
  wr16(REG_HSYNC1, config->hsync1);
  wr16(REG_VCYCLE, config->vcycle);
  wr16(REG_VOFFSET, config->voffset);
  wr16(REG_VSYNC0, config->vsync0);
  wr16(REG_VSYNC1, config->vsync1);
  wr8(REG_SWIZZLE, config->swizzle);
  wr8(REG_PCLK_POL, config->pclk_pol);
  wr8(REG_CSPREAD, config->cspread);
  wr8(REG_DITHER, config->dither);
  wr16(REG_HSIZE, config->hsize);
  wr16(REG_VSIZE, config->vsize);

  // Set Disp GPIO Direction
  wr16(REG_GPIOX_DIR, 0x8000);

  // Enable Disp (if used)
  wr16(REG_GPIOX, 0x8000);

  // Backlight PWM frequency
  wr16(REG_PWM_HZ, 0xFA);

  // Backlight PWM duty (on)
  wr8(REG_PWM_DUTY, 32);

  bt81x_gfx_clear();

  // set pclk to half
  wr8(REG_PCLK, config->pclk);
}

void bt81x_gfx_clear() {
  wdla(0, CLEAR_COLOR_RGB(0, 0, 0));
  wdla(4, CLEAR(1, 1, 1));
  wdla(8, DISPLAY());
  wr8(REG_DLSWAP, DLSWAP_FRAME);
}

void bt81x_demo_loop() {
  wdla(0, CLEAR(1, 1, 1));                 // clear screen
  wdla(4, BEGIN(BITMAPS));                 // start drawing bitmaps
  wdla(8, VERTEX2II(220, 110, 31, 'T'));   // ascii T in font 31
  wdla(12, VERTEX2II(244, 110, 31, 'E'));  // ascii E
  wdla(16, VERTEX2II(270, 110, 31, 'X'));  // ascii X
  wdla(20, VERTEX2II(299, 110, 31, 'T'));  // ascii T
  wdla(24, END());
  wdla(28, COLOR_RGB(160, 22, 22));     // change colour to red
  wdla(32, POINT_SIZE(320));            // set point size to 20 pixels in radius
  wdla(36, BEGIN(POINTS));              // start drawing points
  wdla(40, VERTEX2II(192, 133, 0, 0));  // red point
  wdla(44, END());
  wdla(48, DISPLAY());  // display the image

  wr8(REG_DLSWAP, DLSWAP_FRAME);
}
