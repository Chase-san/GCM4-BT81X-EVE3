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
#pragma once
#ifndef __BT81X_H__
#define __BT81X_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct bt81xcfg bt81xcfg_t;
struct bt81xcfg {
  uint16_t hcycle;
  uint16_t hoffset;
  uint16_t hsync0;
  uint16_t hsync1;
  uint16_t vcycle;
  uint16_t voffset;
  uint16_t vsync0;
  uint16_t vsync1;
  uint8_t swizzle;
  uint8_t pclk;
  uint8_t pclk_pol;
  uint8_t cspread;
  uint8_t dither;
  uint16_t hsize;
  uint16_t vsize;
};

extern const bt81xcfg_t bt81x_eve3_70;
extern const bt81xcfg_t bt81x_eve2_70;

uint32_t bt81x_read_chipid();
uint32_t bt81x_read_frequency();

void bt81x_cmd_active();
void bt81x_cmd_standby();
void bt81x_cmd_clockext();
void bt81x_cmd_clockint();
void bt81x_cmd_sleep();
void bt81x_cmd_powerdown();
void bt81x_cmd_reset();

void bt81x_start();
void bt81x_configure(const bt81xcfg_t *config);

void bt81x_gfx_clear();
void bt81x_demo_loop();

#ifdef __cplusplus
}
#endif

#endif /* __BT81X_H__ */
