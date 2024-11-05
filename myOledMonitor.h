//--------------------------------------------------------
// myOledMonitor.h
//--------------------------------------------------------
// Simple library to drive (ideaSpark) SSD1306 or ST7789
// display as a scrolling text output monitor.
// Implemented and Tested on ESP32 using default CS(15)

#pragma once

#include <Arduino.h>

#define DRIVER_SSD1306_128x32	0x0100		// I2C
#define DRIVER_SSD1306_128x64	0x0110		// I2C
#define DRIVER_ST7789_320x170	0x0200		// SPI

#define DRIVER_MASK_SSD1306		0x0100
#define DRIVER_MASK_ST7789		0x0200


class myOledMonitor
{
public:

	myOledMonitor(uint16_t driver, int font_size=2, bool with_task=false);

	static void init(int rotation=0, bool with_display=false);

	static void println(const char *format, ...);

};
