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

void setup()
{
	Serial.begin(115200);

	// This can be set in the IDE no need for ext library
	// system_update_cpu_freq(160);

	Serial.println("\nInternet Radio, (c) GGM, 2022");

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
		if (RemoteCode == KEY_OK && !IsRepeat) DisplayCurrentStation();
	}

	if (StateChanged && ((millis() - LastStateChange) > AUTOSAVE_INTERVAL_MS))
	{
		SaveRadioState();
		StateChanged = false;
		DisplayCurrentStation();
	}

	DisplayDim((millis() - lastKeyTime) > IDLE_DIMM_MS);
	Screensaver((millis() - lastKeyTime) > IDLE_SAVER_MS);

	NetworkJob();
	PlayerJob();
}