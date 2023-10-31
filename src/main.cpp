#ifdef ENABLE_DEBUG
  #define DEBUG_ESP_PORT Serial
  #define NODEBUG_WEBSOCKETS
  #define NDEBUG
#endif

#include "main.hpp"

// preserve order!
#include "helper.h"
#include "remote.h"
#include "data.h"
#include "display.h"
#include "network.h"
#include "player.h"

RTC_DATA_ATTR int bootCount = 0;

int sleepBar = 0;
unsigned long sleepBarTime = 0;
unsigned long secondsFromMillis = 0;

void setup()
{
	//Serial.begin(115200);

	initVbat();

	pinMode(FM_RELAY_PIN, OUTPUT);
	pinMode(WEB_RELAY_PIN, OUTPUT);
	pinMode(PWR_PIN, OUTPUT);
	
	// power up peripherials
	digitalWrite(PWR_PIN, HIGH);
	delay(200);

	esp_sleep_enable_ext1_wakeup(WAKE_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);

	// This can be set in the IDE no need for ext library
	// system_update_cpu_freq(160);

	Serial.println("\nInternet Radio, (c) GGM, 2023");

	SwitchOutput(WEB_RADIO);

	FMInit();

	ButtonsInit();
	RemoteInit();
	DisplayInit();
	DataInit();
	NetworkInit();
	PlayerInit();

	// GPIO0 strange behaviour fix
	pinMode(BUTTON_PIN_DOWN, INPUT_PULLUP);

	DisplayCurrentMode(DisplayMode);

	// standalone cpu task 
	//StartPlayerTask();
}

void loop()
{
	if (GetRemoteCode())
	{
		if (IsCode(KEY_MINUS, false))
		{
			if (CurrentRadio == WEB_RADIO) SetWebVolume(WebVolume-1);
			else SetFMVolume(FMVolume-1);
			//FMCommand("AT+VOLD");
			DisplayVolume();
		}
		if (IsCode(KEY_PLUS, false))
		{
			if (CurrentRadio == WEB_RADIO) SetWebVolume(WebVolume+1);
			else SetFMVolume(FMVolume+1);
			//FMCommand("AT+VOLU");
			DisplayVolume();
		}
		if (IsCode(KEY_CH_MINUS)) NextStation(-1);
		if (IsCode(KEY_CH_PLUS)) NextStation(1);
		if (IsCode(KEY_0)) SwitchStation(0);
		if (IsCode(KEY_1)) SwitchStation(1);
		if (IsCode(KEY_2)) SwitchStation(2);
		if (IsCode(KEY_3)) SwitchStation(3);
		if (IsCode(KEY_4)) SwitchStation(4);
		if (IsCode(KEY_5)) SwitchStation(5);
		if (IsCode(KEY_6)) SwitchStation(6);
		if (IsCode(KEY_7)) SwitchStation(7);
		if (IsCode(KEY_8)) SwitchStation(8);
		if (IsCode(KEY_9)) SwitchStation(9);
		if (IsCode(KEY_PREV)) TuneFMStation(FMStation.freq-1, "", false);
		if (IsCode(KEY_NEXT)) TuneFMStation(FMStation.freq+1, "", false);

		if (IsCode(KEY_CH, false))
		{
			if (IsRepeat) 
			{
				if ((millis() - lastKeyTime) > KEY_OK_TO_SLEEP_INTERVAL_MS)
				{
					sleepBarTime = millis();
					sleepBar++;
					if (DisplaySleepBar(sleepBar)) 
					{
						shutdown();
					}
				}
			}
			else
			{
				sleepBar = 0;
				if (isDisplayDimmed()) DisplayDim(false);
				else if (IsRemote) DisplayCurrentMode(DM_NORMAL);
				else {
					if (DisplayMode == DM_NORMAL) DisplayCurrentMode(DM_TIME);
					else DisplayCurrentMode(DM_NORMAL);
				}
				//FMCommand("AT+BANK=10");
			}
		}
		if (IsCode(KEY_EQ)) DisplayCurrentMode(DM_TIME);
		if (IsCode(KEY_PLAYPAUSE)) DisplayCurrentMode(DM_SIMPLE);
	}
	else if (RemoteCode == 0 && sleepBar > 0 && ((millis() - sleepBarTime) > KEY_REPEAT_DELAY_MS))
	{
		sleepBar = 0;
		DisplayCurrentMode(DM_NORMAL);
	}

	if (sleepBar == 0)
	{
		if (StateChanged && ((millis() - LastStateChange) > AUTOSAVE_INTERVAL_MS))
		{
			SaveRadioState();
			StateChanged = false;
			DisplayCurrentMode(DisplayMode);
		}

		DisplayDim((millis() - lastKeyTime) > IDLE_DIMM_MS);
		Screensaver((millis() - lastKeyTime) > IDLE_SAVER_MS);

		if (millis()/1000 > secondsFromMillis)
		{
			if (DisplayMode == DM_TIME) DisplayCurrentMode(DisplayMode);
			secondsFromMillis = (millis()/1000);
		}
	}

	PlayerJob();

	//delay(1);
}