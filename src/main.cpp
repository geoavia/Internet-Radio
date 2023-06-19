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

#define BUTTON_PIN_BITMASK (0x800000000) // 2^GPIO

RTC_DATA_ATTR int bootCount = 0;

void print_wakeup_reason() 
{
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

void print_GPIO_wake_up()
{
  uint64_t GPIO_reason = esp_sleep_get_ext1_wakeup_status();
  Serial.printf("GPIO that triggered the wake up: GPIO %lx\n", GPIO_reason);
}

void setup()
{
	Serial.begin(115200);

	print_wakeup_reason();
	print_GPIO_wake_up();
	esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ALL_LOW);

	// This can be set in the IDE no need for ext library
	// system_update_cpu_freq(160);

	Serial.println("\nInternet Radio, (c) GGM, 2022");

	ButtonsInit();
	RemoteInit();
	DisplayInit();
	DataInit();
	NetworkInit();
	//PlayerInit();

	// standalone cpu task 
	//StartPlayerTask();
}

void loop()
{
	if (GetRemoteCode())
	{
		if (RemoteCode == KEY_LEFT)
		{
			if (PlayerVolume > 0) 
			{
				asyncVolume = --PlayerVolume;
				player.setVolume(PlayerVolume);
				DisplayVolume(PlayerVolume);
				SetStateChanged();
			}
		}
		if (RemoteCode == KEY_RIGHT)
		{
			if (PlayerVolume < 100) 
			{
				asyncVolume = ++PlayerVolume;
				player.setVolume(PlayerVolume);
				DisplayVolume(PlayerVolume);
				SetStateChanged();
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
				if (IsRemote) DisplayCurrentMode(DM_NORMAL);
				else {
					if (DisplayMode == DM_NORMAL) DisplayCurrentMode(DM_TIME);
					else DisplayCurrentMode(DM_NORMAL);
				}
			}
		}
		if (RemoteCode == KEY_HTAG) DisplayCurrentMode(DM_TIME);
		if (RemoteCode == KEY_AST) DisplayCurrentMode(DM_SIMPLE);
	}

	if (StateChanged && ((millis() - LastStateChange) > AUTOSAVE_INTERVAL_MS))
	{
		SaveRadioState();
		StateChanged = false;
		if (DisplayMode == DM_NORMAL) DisplayCurrentMode(DM_NORMAL);
	}

	DisplayDim((millis() - lastKeyTime) > IDLE_DIMM_MS);
	Screensaver((millis() - lastKeyTime) > IDLE_SAVER_MS);

	//NetworkJob();
	//PlayerJob();

	//delay(50);
}