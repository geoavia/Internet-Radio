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

	UartInit();
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

	// standalone cpu task 
	//StartPlayerTask();
}

void loop()
{
	//return;
	if (GetRemoteCode())
	{
		if (RemoteCode == KEY_LEFT)
		{
			if (PlayerVolume > 0) 
			{
				asyncVolume = --PlayerVolume;
				audio.setVolume(PlayerVolume);
				DisplayVolume(PlayerVolume);
				SetStateChanged();
				UartCommand("AT+VOLD");
			}
		}
		if (RemoteCode == KEY_RIGHT)
		{
			if (PlayerVolume < MAX_VOLUME) 
			{
				asyncVolume = ++PlayerVolume;
				audio.setVolume(PlayerVolume);
				DisplayVolume(PlayerVolume);
				SetStateChanged();
				UartCommand("AT+VOLU");
			}
		}
		if (RemoteCode == KEY_DOWN && !IsRepeat) NextStation(false);
		if (RemoteCode == KEY_UP && !IsRepeat) NextStation(true);
		if (RemoteCode == KEY_0) SetCurrentStation(0);
		if (RemoteCode == KEY_1) SetCurrentStation(1);
		if (RemoteCode == KEY_2) SetCurrentStation(2);
		if (RemoteCode == KEY_3) SetCurrentStation(3);
		if (RemoteCode == KEY_4) SetCurrentStation(4);
		if (RemoteCode == KEY_5) SetCurrentStation(5);
		if (RemoteCode == KEY_6) SetCurrentStation(6);
		if (RemoteCode == KEY_7) SetCurrentStation(7);
		if (RemoteCode == KEY_8) SetCurrentStation(8);
		if (RemoteCode == KEY_9) SetCurrentStation(9);
		if (RemoteCode == KEY_OK)
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
				UartCommand("AT+BANK=", 10);
			}
		}
		if (RemoteCode == KEY_HTAG) DisplayCurrentMode(DM_TIME);
		if (RemoteCode == KEY_AST) DisplayCurrentMode(DM_SIMPLE);
	}

	if (StateChanged && ((millis() - LastStateChange) > AUTOSAVE_INTERVAL_MS))
	{
		SaveRadioState();
		StateChanged = false;
		DisplayCurrentMode(DisplayMode);
	}

	DisplayDim((millis() - lastKeyTime) > IDLE_DIMM_MS);
	Screensaver((millis() - lastKeyTime) > IDLE_SAVER_MS);

	NetworkJob();
	PlayerJob();


	audio.loop();
	//delay(50);
}