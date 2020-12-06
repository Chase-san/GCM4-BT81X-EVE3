# BT81X Library

This is a library designed to interface the Adafruit Metro M4 Grand Central
with the RVT70AQBFWN00 from Riverdi. This board makes use of a BT816 control
chip along with a 7.0 inch EVE3 Display module.

## Contents

### gcm4-eve3.ino
This is the main file of the software package and not required to use the
library

### bt81x.h / bt81x.c
This performs all actions needed to run and access the device. Display hardware
specific code is stored here.

### device.h / device.cpp
The device file is used to expose certain parts of the device code to the
library code. This handles logging, SPI, and digital pins. This allows the
library to run without specialized knowledge of it's host device.

## Functionality

As of the writing of this document, the library does not currently perform
it's intended function. It is not recommended to use this code for anything.

## License
Copyright (c) 2020, Robert Maupin

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.