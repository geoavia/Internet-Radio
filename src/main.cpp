#include "main.hpp"

// all in one hpp files (for simplicity)
// preserve order!
#include "helper.hpp"
#include "remote.hpp"
#include "data.hpp"
#include "display.hpp"
#include "network.hpp"
#include "player.hpp"

// state auto save interval
#define AUTOSAVE_INTERVAL_MS 9000

void setup()
{
	Serial.begin(115200);

	// This can be set in the IDE no need for ext library
	// system_update_cpu_freq(160);

	Serial.println("\nInternet Radio, (c) GGM, 2022");

	RemoteInit();
	DisplayInit();
	DataInit();
	NetworkInit();
	PlayerInit();
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
		if (RemoteCode == KEY_DOWN && !IsRepeat)
		{
			NextStation(false);
			PlayCurrentStation();
		}
		if (RemoteCode == KEY_UP && !IsRepeat)
		{
			NextStation(true);
			PlayCurrentStation();
		}
		if (RemoteCode == KEY_1)
		{
			SetCurrentStation(1);
			PlayCurrentStation();
		}
		if (RemoteCode == KEY_2)
		{
			SetCurrentStation(2);
			PlayCurrentStation();
		}
		if (RemoteCode == KEY_3)
		{
			SetCurrentStation(3);
			PlayCurrentStation();
		}
		if (RemoteCode == KEY_4)
		{
			SetCurrentStation(4);
			PlayCurrentStation();
		}
		if (RemoteCode == KEY_5)
		{
			SetCurrentStation(5);
			PlayCurrentStation();
		}
		if (RemoteCode == KEY_6)
		{
			SetCurrentStation(6);
			PlayCurrentStation();
		}
		if (RemoteCode == KEY_7)
		{
			SetCurrentStation(7);
			PlayCurrentStation();
		}
		if (RemoteCode == KEY_8)
		{
			SetCurrentStation(8);
			PlayCurrentStation();
		}
		if (RemoteCode == KEY_9)
		{
			SetCurrentStation(9);
			PlayCurrentStation();
		}
		if (RemoteCode == KEY_0)
		{
			SetCurrentStation(0);
			PlayCurrentStation();
		}
	}

	if (StateChanged && ((millis() - LastStateChange) > AUTOSAVE_INTERVAL_MS))
	{
		SaveRadioState();
		StateChanged = false;
	}

	NetworkJob();
	PlayerJob();
}