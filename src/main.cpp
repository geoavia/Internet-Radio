#include "main.hpp"

// all in one hpp files (for simplicity)
// preserve order!
#include "helper.h"
#include "remote.h"
#include "data.h"
#include "display.h"
#include "network.h"
#include "player.h"

// state auto save interval
#define AUTOSAVE_INTERVAL_MS 9000

#define BUTTON_PIN_BITMASK (0x200000000) // 2^GPIO

RTC_DATA_ATTR int bootCount = 0;

void setup()
{
	Serial.begin(115200);

	esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);

	// This can be set in the IDE no need for ext library
	// system_update_cpu_freq(160);

	Serial.println("\nInternet Radio, (c) GGM, 2023");

	FMInit();
	// delay(1000);
	// UartCommand("AT+VOL=", 15);
	// delay(1000);
	// UartCommand("AT+FRE=", 1039);
	// delay(1000);
	// UartCommand("AT+BANK=", 10);
	// delay(3000);
	// UartCommand("AT+FRE=", 997);

	//return;


	ButtonsInit();
	RemoteInit();
	DisplayInit();
	DataInit();
	NetworkInit();
	PlayerInit();

	lastKeyTime = millis();
	DisplayCurrentMode(DisplayMode);

	// standalone cpu task 
	//StartPlayerTask();
}

void loop()
{
	//return;
	if (GetRemoteCode())
	{
		if (RemoteCode == KEY_MINUS)
		{
			SetWebVolume(WebVolume-1);
			FMCommand("AT+VOLD");
			DisplayVolume(WebVolume);
		}
		if (RemoteCode == KEY_PLUS)
		{
			SetWebVolume(WebVolume+1);
			FMCommand("AT+VOLU");
			DisplayVolume(WebVolume);
		}
		if (RemoteCode == KEY_CH_MINUS && !IsRepeat) NextStation(-1);
		if (RemoteCode == KEY_CH_PLUS && !IsRepeat) NextStation(1);
		if (RemoteCode == KEY_0) SwitchStation(0);
		if (RemoteCode == KEY_1) SwitchStation(1);
		if (RemoteCode == KEY_2) SwitchStation(2);
		if (RemoteCode == KEY_3) SwitchStation(3);
		if (RemoteCode == KEY_4) SwitchStation(4);
		if (RemoteCode == KEY_5) SwitchStation(5);
		if (RemoteCode == KEY_6) SwitchStation(6);
		if (RemoteCode == KEY_7) SwitchStation(7);
		if (RemoteCode == KEY_8) SwitchStation(8);
		if (RemoteCode == KEY_9) SwitchStation(9);
		if (RemoteCode == KEY_PREV) TuneFMStation(FMStation.freq-1, "Unknown");
		if (RemoteCode == KEY_NEXT) TuneFMStation(FMStation.freq+1, "Unknown");

		if (RemoteCode == KEY_CH)
		{
			if (IsRepeat) 
			{
				if ((millis() - lastKeyTime) > KEY_OK_TO_SLEEP_INTERVAL_MS)
				{
					DisplayZZZ();
					esp_deep_sleep_start();
				}
			}
			else
			{
				if (isDisplayDimmed()) DisplayDim(false);
				else if (IsRemote) DisplayCurrentMode(DM_NORMAL);
				else {
					if (DisplayMode == DM_NORMAL) DisplayCurrentMode(DM_TIME);
					else DisplayCurrentMode(DM_NORMAL);
				}
				FMCommand("AT+BANK=10");
			}
		}
		if (RemoteCode == KEY_EQ) DisplayCurrentMode(DM_TIME);
		if (RemoteCode == KEY_PLAYPAUSE) DisplayCurrentMode(DM_SIMPLE);
	}

	if (StateChanged && ((millis() - LastStateChange) > AUTOSAVE_INTERVAL_MS))
	{
		SaveRadioState();
		StateChanged = false;
		DisplayCurrentMode(DisplayMode);
	}

	DisplayDim((millis() - lastKeyTime) > IDLE_DIMM_MS);
	Screensaver((millis() - lastKeyTime) > IDLE_SAVER_MS);

	PlayerJob();

	audio.loop();
	//delay(50);
}