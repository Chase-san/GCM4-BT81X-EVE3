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
#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <stdint.h>

/**
 * The BT81X Device files are used as a shim between the device platform
 * and the bt81x code. Any needed modifications should be made in these files.
 *
 * Attempts have been made to keep this file as minimum as possible to ease the
 * conversion to other devices.
 */

#ifdef __cplusplus
extern "C" {
#endif

#define BT81X_BAUD 12000000
#define BT81X_CS   4

/**
 * Initializes anythis needed by the other commands to perform thier needed
 * function.
 */
void device_init();

/**
 * Performs a blocking delay of the given duration.
 * @param ms milliseconds to delay
 */
void device_delay(const uint32_t ms);

/**
 * Called at the start of an SPI transaction. Things such as mode, baudrate,
 * and chip select should be performed here.
 */
void device_spi_start();

/**
 * Called after an SPI transaction.
 */
void device_spi_end();

void device_spi_read(uint8_t *data, const uint16_t size);

uint8_t device_spi_read8();

void device_spi_write(const uint8_t *data, const uint16_t size);

void device_spi_write8(const uint8_t data);

void device_logf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* __DEVICE_H__ */
