#ifndef __DISPLAY__
#define __DISPLAY__

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET 4        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void DisplayRSSI(int x, int y, int32_t rssi, uint16_t color)
{
    int nb = Get4BarsFromRSSI(rssi);
    for (int b = 0; b < nb; b++)
    {
        display.drawFastHLine(x, y-b*2, b+1, color);
    }
}

void DisplayVolume(int volume)
{
    display.fillRect(14, 60, volume+14, 3, SSD1306_WHITE);    
    display.fillRect(volume+15, 60, 115-volume, 3, SSD1306_BLACK);
    display.display();
}

void DisplayHeader()
{
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(F("WWW Radio"));
    display.setTextSize(1);
    display.println(F("Version: 1.0"));
    display.drawFastHLine(0,28,128,SSD1306_WHITE);
    display.setCursor(0, 32);
    display.display();
}

void DisplayInit()
{
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }

    // Show initial display buffer contents on the screen --
    // the library initializes this with an Adafruit splash screen.

    display.setRotation(2);
}

#endif //__DISPLAY__