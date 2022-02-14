#include <Arduino.h>

// all in one hpp files (for simplicity)
// preserve order!
#include "helper.hpp"
#include "remote.hpp"
#include "display.hpp"
#include "data.hpp"
#include "network.hpp"
#include "player.hpp"

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
			}
		}
		if (RemoteCode == KEY_RIGHT)
		{
			if (PlayerVolume < 100) 
			{
				asyncVolume = ++PlayerVolume;
				player.setVolume(PlayerVolume);
				DisplayVolume(PlayerVolume);
			}
		}
		if (RemoteCode == KEY_DOWN && !IsRepeat)
		{
			NextStation(false);
			PlayStation(CurrentStation);
		}
		if (RemoteCode == KEY_UP && !IsRepeat)
		{
			NextStation(true);
			PlayStation(CurrentStation);
		}
		if (RemoteCode == KEY_1)
		{
			SetCurrentStation(1);
			PlayStation(CurrentStation);
		}
		if (RemoteCode == KEY_2)
		{
			SetCurrentStation(2);
			PlayStation(CurrentStation);
		}
		if (RemoteCode == KEY_3)
		{
			SetCurrentStation(3);
			PlayStation(CurrentStation);
		}
		if (RemoteCode == KEY_4)
		{
			SetCurrentStation(4);
			PlayStation(CurrentStation);
		}
		if (RemoteCode == KEY_5)
		{
			SetCurrentStation(5);
			PlayStation(CurrentStation);
		}
		if (RemoteCode == KEY_6)
		{
			SetCurrentStation(6);
			PlayStation(CurrentStation);
		}
		if (RemoteCode == KEY_7)
		{
			SetCurrentStation(7);
			PlayStation(CurrentStation);
		}
		if (RemoteCode == KEY_8)
		{
			SetCurrentStation(8);
			PlayStation(CurrentStation);
		}
		if (RemoteCode == KEY_9)
		{
			SetCurrentStation(9);
			PlayStation(CurrentStation);
		}
		if (RemoteCode == KEY_0)
		{
			SetCurrentStation(0);
			PlayStation(CurrentStation);
		}
	}

	NetworkJob();
	PlayerJob();
}