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
#include "device.h"

#include <Arduino.h>
#include <SPI.h>

void device_init() {
  Serial.begin(9600);
  SPI.begin();
  pinMode(device_CS, OUTPUT);
}

void device_delay(const uint32_t ms) {
  delay(ms);
}

void device_spi_start() {
  SPI.beginTransaction(SPISettings(device_BAUD, MSBFIRST, SPI_MODE0));
  digitalWrite(device_CS, LOW);
}

void device_spi_end() {
  digitalWrite(device_CS, HIGH);
  SPI.endTransaction();
}

void device_spi_read(uint8_t *data, const uint16_t size) {
  for (int i = 0; i < size; ++i) {
    data[i] = SPI.transfer(0);
  }
}

uint8_t device_spi_read8() {
  return SPI.transfer(0);
}

void device_spi_write(const uint8_t *data, const uint16_t size) {
  for (int i = 0; i < size; ++i) {
    SPI.transfer(data[i]);
  }
}

void device_spi_write8(const uint8_t data) {
  SPI.transfer(data);
}

#define LOG_BUFFER_SIZE 128
char log_buffer[LOG_BUFFER_SIZE];

void device_logf(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vsnprintf(log_buffer, LOG_BUFFER_SIZE, format, args);
  Serial.print(log_buffer);
  va_end(args);
}
