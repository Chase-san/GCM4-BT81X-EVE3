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
#define BITMAP_HANDLE(handle)    (0x5000000 | (handle))
#define CELL(cell)               (0x6000000 | (cell))
#define CLEAR_COLOR_RGB(r, g, b) (0x2000000 | ((r) << 16) | ((b) << 8) | (g))
#define CLEAR(c, s, t)           (0x26000000 | ((c) << 2) | ((s) << 1) | (t))
#define COLOR_RGB(r, g, b)       (0x4000000 | ((r) << 16) | ((b) << 8) | (g))
#define DISPLAY()                0x0
#define END()                    0x21000000
#define POINT_SIZE(size)         (0xD000000 | (size))
#define VERTEX2F(x, y)           (0x40000000 | ((x) << 15) | (y))
#define VERTEX2II(x, y, handle, cell) \
  (0x80000000 | ((x) << 21) | ((y) << 12) | ((handle) << 7) | (cell))
#define VERTEX_FORMAT(frac) (0x27000000 | (frac))

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
const bt81xcfg_t rvt70eve3 = {
    /* */
    .hcycle = 1056, .hoffset = 46, .hsync0 = 0,  .hsync1 = 10, .vcycle = 525,
    .voffset = 23,  .vsync0 = 0,   .vsync1 = 10, .swizzle = 0, .pclk = 2,
    .pclk_pol = 1,  .cspread = 0,  .dither = 1,  .hsize = 800, .vsize = 480,
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

/**
 * Display List Memory Offset
 */
uint32_t __dlmo = 0;

inline void dlreset() {
  __dlmo = 0;
}

inline void dl(const uint32_t data) {
  // TODO ensure we are not over display list ram memory limit of 8 KB
  wr32(RAM_DL + __dlmo, data);
  __dlmo += 4;
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

void bt81x_hc_active() {
  host_command(ACTIVE);
}

void bt81x_hc_standby() {
  host_command(STANDBY);
}

void bt81x_hc_clockext() {
  host_command(CLKEXT);
}

void bt81x_hc_clockint() {
  host_command(CLKINT);
}

void bt81x_hc_sleep() {
  host_command(SLEEP);
}

void bt81x_hc_powerdown() {
  host_command(PWRDOWN);
}

void bt81x_hc_reset() {
  host_command(RST_PULSE);
}

void bt81x_init(const bt81xcfg_t *config) {
  bt81x_hc_clockext();
  bt81x_hc_active();
  bt81x_hc_active();
  device_delay(300);

  // wait to be active
  while (rd8(REG_ID) != 0x7C) {
    device_delay(50);
  }

  // wait for reset
  while (rd8(REG_CPURESET) != 0x0) {
    device_delay(50);
  }

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
  wr16(REG_GPIOX_DIR, 0x8000);  // 0x8000

  // Enable Disp (if used)
  wr16(REG_GPIOX, 0x8000);  // 0x8000

  // Backlight PWM frequency
  wr16(REG_PWM_HZ, 0xFA);

  // Backlight PWM duty (on)
  wr8(REG_PWM_DUTY, 32);

  // bt81x_gfx_clear_white();
  // bt81x_gfx_clear();
  // bt81x_demo_loop();
  bt81x_demo_loop();

  // set pclk to half
  wr8(REG_PCLK, config->pclk);
}

void bt81x_gfx_clear() {
  dlreset();
  dl(CLEAR_COLOR_RGB(0, 0, 0));
  dl(CLEAR(1, 1, 1));
  dl(DISPLAY());
  wr8(REG_DLSWAP, DLSWAP_FRAME);
}

void bt81x_gfx_clear_white() {
  dlreset();
  dl(CLEAR_COLOR_RGB(0xFF, 0xFF, 0xFF));
  dl(CLEAR(1, 1, 1));
  dl(DISPLAY());
  wr8(REG_DLSWAP, DLSWAP_FRAME);
}

void bt81x_draw_text(const int16_t x, const int16_t y, const int16_t dx,
                     const int16_t dy, const uint8_t handle, const char *text,
                     const size_t length) {
  int16_t tx = x;
  int16_t ty = y;
  dl(BEGIN(BITMAPS));
  dl(BITMAP_HANDLE(handle));
  for (int i = 0; i < length; ++i) {
    dl(CELL(text[i]));
    dl(VERTEX2F(tx, ty));
    tx += dx;
    ty += dy;
  }
  dl(END());
}

void bt81x_demo_loop() {
  const int16_t br = 20 * 16;
  static int16_t bx = br;
  static int16_t by = br;

  static int8_t dx = 2;
  static int8_t dy = 2;

  dlreset();
  dl(CLEAR_COLOR_RGB(0, 0, 0));
  dl(CLEAR(1, 1, 1));  // clear screen

  dl(COLOR_RGB(240, 16, 16));  // change colour to red
  dl(POINT_SIZE(br));          // set point size to 20 pixels in radius
  dl(BEGIN(POINTS));           // start drawing points
  dl(VERTEX2F(bx, by));        // red point
  dl(END());

  dl(VERTEX_FORMAT(0));             // no subpixel precision please
  dl(COLOR_RGB(0xFF, 0xFF, 0xFF));  // change colour to black
  bt81x_draw_text(350, 240 - 25, 25, 0, 31, "TEST", 4);

  dl(DISPLAY());  // display the image

  wr8(REG_DLSWAP, DLSWAP_FRAME);

  // update ball movement
  bx += dx;
  by += dy;
  if (bx >= 800 * 16 - br || bx <= br) {
    dx = -dx;
  }
  if (by >= 480 * 16 - br || by <= br) {
    dy = -dy;
  }
}
