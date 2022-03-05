#ifndef __DATA_H__
#define __DATA_H__

#include "main.hpp"

void LoadOTAUP()
{
	File file = SPIFFS.open(OTAUP_FILE_NAME, "r");
	if (file)
	{
		if (file.available())
		{
			otaup.ssid = file.readStringUntil('/');
			otaup.password = file.readStringUntil('\n');
		}
	}
	file.close();
}

void LoadRadioStations()
{
	n_stations = 0;
	File file = SPIFFS.open(STATIONS_FILE_NAME);
	if (file)
	{
		Serial.println("Saved radio stations:");
		while (file.available() && n_stations < MAX_STATIONS)
		{
			Serial.print(n_stations + 1);
			Serial.print(": ");
			Stations[n_stations].url = file.readStringUntil(',');
			Serial.print(Stations[n_stations].url);
			Serial.print(" - ");
			Stations[n_stations].name = file.readStringUntil('\n');
			Serial.println(Stations[n_stations].name);
			n_stations++;
		}
	}
	file.close();
	CurrentStation = Stations[0];
}

void AddStation(String url, String name = "")
{
	if (n_stations < MAX_STATIONS)
	{
		Stations[n_stations].url = url;
		Stations[n_stations].name = name;
		n_stations++;
	}
}

void RemoveStation(uint index)
{
	if (index < n_stations)
	{
		for (uint i = index; i < n_stations - 1; i++)
		{
			Stations[i] = Stations[i + 1];
		}
		n_stations--;
	}
}

void RemoveStationByUrl(String url)
{
	for (uint i = 0; i < n_stations; i++)
	{
		if (Stations[i].url == url)
		{
			RemoveStation(i);
			break;
		}
	}
}

void SaveRadioStations()
{
	File file = SPIFFS.open(STATIONS_FILE_NAME, "w");
	if (file)
	{
		Serial.println("Saving radio stations");
		for (uint i = 0; i < n_stations; i++)
		{
			file.print(Stations[i].url);
			file.print(",");
			file.print(Stations[i].name);
			file.println();
		}
	}
	file.close();
}

void NextStation(bool next = true)
{
	for (uint i = 0; i < n_stations; i++)
		if (Stations[i].url == CurrentStation.url)
		{
			if (next)
			{
				if (i < n_stations - 1)
					CurrentStation = Stations[i + 1];
				else
					CurrentStation = Stations[0];
			}
			else
			{
				if (i > 0)
					CurrentStation = Stations[i - 1];
				else
					CurrentStation = Stations[n_stations - 1];
			}
			break;
		}
}

void SetCurrentStation(uint n)
{
	if (n < n_stations)
		CurrentStation = Stations[n];
}

int GetCurrentStationIndex()
{
	for (uint i = 0; i < n_stations; i++)
		if (Stations[i].url == CurrentStation.url)
			return i;
	return 0;
}

bool FindStationByUrl(String url, RADIO_STATION &station)
{
	for (uint i = 0; i < n_stations; i++)
		if (Stations[i].url == url)
		{
			station = Stations[i];
			return true;
		}
	return false;
}

void LoadRadioState()
{
	File file = SPIFFS.open(STATE_FILE_NAME);
	if (file)
	{
		Serial.println("Loading last state");
		if (file.available())
		{
			SetCurrentStation(file.readStringUntil(',').toInt());
			PlayerVolume = asyncVolume = file.readStringUntil('\n').toInt();
		}
	}
	file.close();
}

void SaveRadioState()
{
	File file = SPIFFS.open(STATE_FILE_NAME, "w");
	if (file)
	{
		Serial.println("Saving state");
		file.print(GetCurrentStationIndex());
		file.print(",");
		file.print(PlayerVolume);
		file.print("\n");
	}
	file.close();

}

void DataInit()
{
	if (!SPIFFS.begin(true))
	{
		Serial.println("An Error has occurred while mounting SPIFFS");
		return;
	}

	LoadRadioStations();
	LoadRadioState();
}


#endif //__DATA_H__