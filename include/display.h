#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <SPI.h>
#include <TFT_eSPI.h>

#include "images.h"

#ifndef TFT_DISPOFF
#define TFT_DISPOFF 0x28
#endif

#ifndef TFT_SLPIN
#define TFT_SLPIN   0x10
#endif

//initialize the display
TFT_eSPI tft(TFT_WIDTH, TFT_HEIGHT);

bool dimmed = false;
bool saver = false;

enum DISPLAY_MODE {
	DM_NORMAL = 0,
	DM_SIMPLE,
	DM_TIME
};

DISPLAY_MODE DisplayMode = DM_NORMAL;


#define IDLE_DIMM_MS 10000UL
#define IDLE_SAVER_MS 30000UL

// Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
// Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
// Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters

// Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
// Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:-.
// Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.

// FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

void DisplayInit()
{
	Serial.print("Display Init...");

	// enable display from battery
	//pinMode(15,OUTPUT);
	//digitalWrite(15,1);

	tft.init();

	tft.setRotation(3);

	pinMode(TFT_BL, OUTPUT);
	ledcSetup(0, 5000, 8); // 0-15, 5000, 8
	ledcAttachPin(TFT_BL, 0); // TFT_BL, 0 - 15	

	ledcWrite(0, 150);
	

	Serial.println("Done");
}

bool isDisplayDimmed()
{
	return dimmed;
}


void DisplayDim(bool dim)
{
	if (dimmed != dim)
	{
		Serial.print("Dimmed: ");
		Serial.println(dim);
		//digitalWrite(TFT_BL, !dim);
		dimmed = dim;
		ledcWrite(0, dim ? 5 : 150);
	}
}

#define SLEEP_BAR_COUNT 10

bool DisplaySleepBar(int count)
{
	tft.fillScreen(TFT_BLACK);
	tft.setTextColor(TFT_WHITE);
	//tft.stopscroll();
	tft.setTextFont(4);
	tft.setTextSize(1);

	if (count < SLEEP_BAR_COUNT)
	{
		tft.setCursor(0, 40);
		tft.print(F("Keep press to sleep"));
		//
		//
		tft.setTextFont(1);
		tft.setTextSize(2);
		tft.setCursor(50, 80);
		for (size_t i = 0; i < SLEEP_BAR_COUNT; i++)
		{
			if (i < count) 
			{
				tft.setTextColor(TFT_YELLOW);
				tft.print("#");
			}
			else 
			{
				tft.setTextColor(TFT_DARKGREY);
				tft.print(".");
			}
		}
	} 
	else 
	{
		tft.setCursor(20, 50);
		tft.print(F("Going to sleep now"));
		delay(2000);
	}

	//tft.fillScreen(TFT_BLACK);
	return (count >= SLEEP_BAR_COUNT);
	
}

void DisplayRSSI(int x, int y, int32_t rssi, uint16_t color)
{
	int nb = Get4BarsFromRSSI(rssi);
	for (int b = 0; b < nb; b++)
	{
		tft.drawFastHLine(x, y-b*2, b+1, color);
	}
}

void DisplayHeader()
{
	tft.fillScreen(TFT_BLACK);
	tft.pushImage(0, 0, HEADER_WIDTH, HEADER_HEIGHT, header);

	tft.setTextSize(1);
	// tft.setTextColor(TFT_BLUE);
	// tft.drawString("WWW Radio", 0, 0, 4);

	tft.setTextColor(TFT_YELLOW);
	tft.setTextFont(2);
	tft.setCursor(200, 0);
	tft.printf("%.1fv", getVbat());

	tft.setTextColor(TFT_GREEN);
	tft.drawString("ver 1.2", 180, 15, 2);

	tft.setCursor(0, 50);

	tft.setTextFont(1);
	tft.setTextSize(2);
}

bool bTimeDelimiter = false;

void DisplayCurrentMode(DISPLAY_MODE mode)
{
	if (mode == DM_SIMPLE) 
	{
		tft.setTextFont(4);
		Serial.println("----------------DM_SIMPLE");
		tft.fillScreen(TFT_BLACK);
		tft.setTextWrap(false);
		//tft.setTextSize(2);
		tft.setTextColor(TFT_WHITE);
		tft.setCursor(0, 2);
		tft.println("Station #");
		tft.drawFastHLine(0,25,240,TFT_WHITE);
		tft.setCursor(0, 32);
		tft.setTextColor(TFT_YELLOW);
		if (CurrentRadio == WEB_RADIO) tft.println(WebStation.name);
		else tft.println(FMStation.name);
		
		//tft.startscrollright(0x00,0x04);
	} 
	else if (mode == DM_NORMAL) 
	{
		Serial.println("----------------DM_NORMAL");
		//tft.stopscroll();
		DisplayHeader();
		tft.setTextSize(1);
		//tft.setTextWrap(true);
		tft.setTextFont(4);
		tft.setTextColor(TFT_WHITE);
		tft.print(">> Playing ");
		tft.println("Station #");
		tft.setTextColor(TFT_YELLOW);
		if (CurrentRadio == WEB_RADIO) tft.println(WebStation.name); 
		else tft.println(FMStation.name);
		
		tft.setTextWrap(false);
		tft.drawFastHLine(0,100,240,TFT_YELLOW);
		tft.setTextColor(TFT_WHITE);
		tft.setCursor(0, 105);
		tft.printf("IP: %s\n", WiFi.localIP().toString().c_str());
		
	}
	else if (mode == DM_TIME) 
	{
		Serial.println("----------------DM_TIME");
		struct tm timeinfo;
		if (getLocalTime(&timeinfo)) 
		{
			//tft.loadFont(HEADER_FONT);
			//tft.stopscroll();
			tft.fillScreen(TFT_BLACK);
			//tft.setTextWrap(false);
			//tft.setTextSize(4);
			tft.setTextColor(TFT_WHITE);
			tft.setCursor(50, 4);
			tft.setTextFont(7);
			tft.printf("%02d%s%02d", timeinfo.tm_hour, (bTimeDelimiter ? ":" : " "), timeinfo.tm_min);
			//tft.setTextSize(2);
			tft.setTextColor(TFT_GREEN);
			tft.setCursor(55, 60);
			tft.setTextFont(4);
			tft.printf("%2d.%02d.%d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
			//tft.setTextSize(2);
			tft.setTextColor(TFT_YELLOW);
			tft.setCursor(25, 100);
			tft.setTextFont(4);
			tft.printf("Battery: %.1f volts", getVbat());
			bTimeDelimiter = !bTimeDelimiter;
		}
	}

	DisplayMode = mode;
}

void DisplayVolume()
{
	int volume = (CurrentRadio == WEB_RADIO) ? WebVolume : FMVolume;
	int delta = (CurrentRadio == WEB_RADIO) ? 210/MAX_WEB_VOLUME : 210/MAX_FM_VOLUME;
	//if (DisplayMode != DM_NORMAL) DisplayCurrentMode(DM_NORMAL);
	tft.fillRect(0, 115, 240, 20, TFT_BLACK);
	tft.drawRect(24, 119, 212, 12, TFT_DARKGREY);
	tft.fillRect(25, 120, volume*delta, 10, TFT_WHITE);

	tft.setTextSize(1);
	tft.setTextFont(2);
	tft.setCursor(0, 116);
	tft.print(volume);
	
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