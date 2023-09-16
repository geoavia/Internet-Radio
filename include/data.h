#ifndef __DATA_H__
#define __DATA_H__

#include "main.hpp"

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
			Stations[n_stations].freq = file.readStringUntil(',').toInt();
			Stations[n_stations].url = file.readStringUntil(',');
			Serial.print(Stations[n_stations].url);
			Serial.print(" - ");
			Stations[n_stations].name = file.readStringUntil('\n');
			Serial.println(Stations[n_stations].name);
			n_stations++;
		}
	}
	file.close();
}

void AddStation(uint freq, String url, String name = "")
{
	if (n_stations < MAX_STATIONS)
	{
		Stations[n_stations].freq = freq;
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

void RemoveStationByFreq(uint freq)
{
	for (uint i = 0; i < n_stations; i++)
	{
		if (Stations[i].freq == freq)
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
			file.print(Stations[i].freq);
			file.print(",");
			file.print(Stations[i].url);
			file.print(",");
			file.print(Stations[i].name);
			file.println();
		}
	}
	file.close();
}

int GetCurrentStationIndex()
{
	for (uint i = 0; i < n_stations; i++)
		if ((Stations[i].url == CurrentStation.url) && (Stations[i].freq == CurrentStation.freq))
			return i;
	return 0;
}

void NextStation(RADIO_TYPE stt, int dir = 1)
{
	int ci = GetCurrentStationIndex();
	int n = 0;
	dir = (dir > 0) ? 1 : -1;
	while (n < n_stations)
	{
		ci += dir;
		if (ci < 0) ci = (n_stations - 1);
		else if (ci >= (n_stations - 1)) ci = 0;

		if ((stt == FM_RADIO && Stations[ci].freq > 0) ||
			(stt == WEB_RADIO && Stations[ci].freq == 0))
		{
			asyncFreq = Stations[ci].freq;
			asyncUrl = Stations[ci].url;
			break;
		}
		n++;
	}
}

void TuneStation(uint n)
{
	if (n < n_stations) {
		asyncFreq = Stations[n].freq;
		asyncUrl = Stations[n].url;
	}
}

void SetCurrentStation(uint freq, String url, String name)
{
	CurrentStation.freq = freq;
	CurrentStation.url = url;
	CurrentStation.name = name;
}

bool IsCurrent(int i)
{
	return (Stations[i].url == CurrentStation.url && Stations[i].freq == CurrentStation.freq);
}

bool IsType(int i, RADIO_TYPE t)
{
	return ((t == FM_RADIO && Stations[i].freq > 0) || (t == WEB_RADIO && Stations[i].freq == 0));
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

bool FindStationByFreq(uint freq, RADIO_STATION &station)
{
	for (uint i = 0; i < n_stations; i++)
		if (Stations[i].freq == freq)
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
			int i = file.readStringUntil(',').toInt();
			if (i < n_stations)
			{
				asyncFreq = Stations[i].freq;
				asyncUrl = Stations[i].url;
			}
			asyncFMVolume = file.readStringUntil(',').toInt();
			asyncWebVolume = file.readStringUntil('\n').toInt();
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
		file.print(FMVolume);
		file.print(",");
		file.print(WebVolume);
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