#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <SPI.h>
#include <TFT_eSPI.h>

#ifndef TFT_DISPOFF
#define TFT_DISPOFF 0x28
#endif

#ifndef TFT_SLPIN
#define TFT_SLPIN   0x10
#endif

#define ADC_EN          14
#define ADC_PIN         34

#define SCREEN_WIDTH 135 // LCD display width, in pixels
#define SCREEN_HEIGHT 240 // LCD display height, in pixels

//initialize the display
TFT_eSPI display(135, 240);

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


void DisplayZZZ()
{
	display.fillScreen(TFT_WHITE);
	//display.stopscroll();
	display.setTextSize(4);
	display.setCursor(30, 25);
	display.print(F("Z"));
	////display.display();
	delay(500);
	display.setTextSize(3);
	display.setCursor(55, 15);
	display.print(F("Z"));
	////display.display();
	delay(500);
	display.setTextSize(2);
	display.setCursor(75, 5);
	display.print(F("Z"));
	////display.display();
	delay(500);
	display.setTextSize(1);
	display.setCursor(88, 0);
	display.print(F("Z"));
	////display.display();
	delay(500);
	display.fillScreen(TFT_BLACK);
	////display.display();
}

void DisplayDim(bool dim)
{
	if (dimmed != dim)
	{
		//display.dim(dim);
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
	display.fillScreen(TFT_BLACK);
	display.setTextSize(2);
	display.setTextColor(TFT_WHITE);
	display.setCursor(0, 0);
	display.println(F("WWW Radio"));
	display.setTextSize(1);
	display.println(F("Version: 1.1"));
	display.drawFastHLine(0,28,128,TFT_WHITE);
	display.setCursor(0, 32);


	//display.display();
}

void DisplayCurrentMode(DISPLAY_MODE mode)
{
	if (mode == DM_SIMPLE) 
	{
		display.fillScreen(TFT_BLACK);
		display.setTextWrap(false);
		display.setTextSize(2);
		display.setTextColor(TFT_WHITE);
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
		display.drawFastHLine(0,25,128,TFT_WHITE);
		display.setCursor(0, 32);
		display.println(CurrentStation.name); // todo get from icy-metadata
		//display.display();
		//display.startscrollright(0x00,0x04);
	} 
	else if (mode == DM_NORMAL) 
	{
		//display.stopscroll();
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
		//display.display();
		display.setTextWrap(false);
		display.drawFastHLine(0,52,128,TFT_WHITE);
		display.setCursor(0, 56);
		display.printf("IP: %s\n", WiFi.localIP().toString().c_str());
		//display.display();
	}
	else if (mode == DM_TIME) 
	{
		struct tm timeinfo;
		if (getLocalTime(&timeinfo)) 
		{
			//display.stopscroll();
			display.fillScreen(TFT_BLACK);
			display.setTextWrap(false);
			display.setTextSize(4);
			display.setTextColor(TFT_WHITE);
			display.setCursor(6, 4);
			display.printf("%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
			display.setTextSize(2);
			display.setCursor(5, 48);
			display.printf("%2d.%02d.%d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
			//display.display();
		}
	}

	DisplayMode = mode;
}

void DisplayVolume(int volume)
{
	if (DisplayMode != DM_NORMAL) DisplayCurrentMode(DM_NORMAL);
	display.fillRect(0, 54, 128, 10, TFT_BLACK);
	display.fillRect(24, 58, volume, 4, TFT_WHITE);
	display.setCursor(2, 56);
	display.print(volume);
	//display.display();
}

void DisplayInit()
{
	Serial.print("Display Init...");
	display.init();

	display.setRotation(1);
	
	Serial.println("Done");
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