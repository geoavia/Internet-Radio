#ifndef __DISPLAY_H__
#define __DISPLAY_H__

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

bool dimmed = false;
bool saver = false;

enum DISPLAY_MODE {
	DM_NORMAL = 0,
	DM_SIMPLE,
	DM_TIME
};

DISPLAY_MODE DisplayMode = DM_NORMAL;


#define IDLE_DIMM_MS 3000
#define IDLE_SAVER_MS 30000

void DisplayDim(bool dim)
{
	if (dimmed != dim)
	{
		display.dim(dim);
		dimmed = dim;
	}
}

void DisplayRSSI(int x, int y, int32_t rssi, uint16_t color)
{
	int nb = Get4BarsFromRSSI(rssi);
	for (int b = 0; b < nb; b++)
	{
		display.drawFastHLine(x, y-b*2, b+1, color);
	}
}

void DisplayHeader()
{
	display.clearDisplay();
	display.setTextSize(2);
	display.setTextColor(SSD1306_WHITE);
	display.setCursor(0, 0);
	display.println(F("WWW Radio"));
	display.setTextSize(1);
	display.println(F("Version: 1.1"));
	display.drawFastHLine(0,28,128,SSD1306_WHITE);
	display.setCursor(0, 32);
	display.display();
}

void DisplayCurrentMode(DISPLAY_MODE mode)
{
	if (mode == DM_SIMPLE) 
	{
		display.clearDisplay();
		display.setTextWrap(false);
		display.setTextSize(2);
		display.setTextColor(SSD1306_WHITE);
		display.setCursor(0, 2);
		if (CurrentStation.name == "Noname")
		{
			display.println("Custom");
		}
		else 
		{
			display.print("Station ");
			display.println(GetCurrentStationIndex());
		}
		display.drawFastHLine(0,25,128,SSD1306_WHITE);
		display.setCursor(0, 32);
		display.println(CurrentStation.name); // todo get from icy-metadata
		display.display();
		display.startscrollright(0x00,0x04);
	} 
	else if (mode == DM_NORMAL) 
	{
		display.stopscroll();
		DisplayHeader();
		display.setTextWrap(true);
		display.print(">> Playing ");
		if (CurrentStation.name == "Noname")
		{
			display.println("Custom");
		}
		else 
		{
			display.print("Station ");
			display.println(GetCurrentStationIndex());
		}
		display.println(CurrentStation.name); // todo get from icy-metadata
		display.display();
		display.setTextWrap(false);
		display.drawFastHLine(0,52,128,SSD1306_WHITE);
		display.setCursor(0, 56);
		display.printf("IP: %s\n", WiFi.localIP().toString().c_str());
		display.display();
	}
	else if (mode == DM_TIME) 
	{
		struct tm timeinfo;
		if (getLocalTime(&timeinfo)) 
		{
			display.stopscroll();
			display.clearDisplay();
			display.setTextWrap(false);
			display.setTextSize(4);
			display.setTextColor(SSD1306_WHITE);
			display.setCursor(6, 4);
			display.printf("%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
			display.setTextSize(2);
			display.setCursor(5, 48);
			display.printf("%2d.%02d.%d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
			display.display();
		}
	}

	DisplayMode = mode;
}

void DisplayVolume(int volume)
{
	if (DisplayMode != DM_NORMAL) DisplayCurrentMode(DM_NORMAL);
	display.fillRect(0, 54, 128, 10, SSD1306_BLACK);    
	display.fillRect(24, 58, volume, 4, SSD1306_WHITE);    
	display.setCursor(2, 56);
	display.print(volume);
	display.display();
}

void DisplayInit()
{
	if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
	{
		Serial.println(F("SSD1306 allocation failed"));
		while(true) delay(1); // Don't proceed, loop forever
	}

	// Show initial display buffer contents on the screen --
	// the library initializes this with an Adafruit splash screen.

	display.setRotation(2);
}

void Screensaver(bool ss)
{
	if (saver != ss)
	{
		DisplayCurrentMode(DM_SIMPLE);
		saver = ss;
	}
}


#endif //__DISPLAY_H__