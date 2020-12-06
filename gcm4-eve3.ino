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

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  device_init();

  delay(1000);
  delay(1000);

  Serial.println("Starting Eve3.");

  bt81x_init(&rvt70eve3_alt);
  Serial.printf("Chip ID %08x\n", bt81x_read_chipid());
  Serial.printf("Freq %08d Hz\n", bt81x_read_frequency());

  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  bt81x_demo_loop();
  delay(250);
  digitalWrite(LED_BUILTIN, LOW);
  delay(250);
}
