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
			Stations[n_stations].freq = file.readStringUntil(',').toInt();
			Stations[n_stations].url = file.readStringUntil(',');
			Stations[n_stations].name = file.readStringUntil('\n');
			Serial.print(n_stations + 1);
			Serial.print(": ");
			if (Stations[n_stations].freq > 0) 
			{
				FMStation = Stations[n_stations];
				Serial.print(String(((float)FMStation.freq)/10));
			}
			else 
			{
				WebStation = Stations[n_stations];
				Serial.print(WebStation.url);
			}
			Serial.print(" - ");
			Serial.println(Stations[n_stations].name);
			n_stations++;
		}
	}
	file.close();
}

void AddStation(uint freq, String url, String name = "")
{
	if (n_stations < MAX_STATIONS)
		if ((freq >= MIN_FREQ && freq <= MAX_FREQ) || url.length() > 0)
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

bool IsType(int i, RADIO_TYPE t)
{
	return ((t == FM_RADIO && Stations[i].freq > 0) || (t == WEB_RADIO && Stations[i].freq == 0));
}

int GetStationIndexByUrl(String url)
{
	for (uint i = 0; i < n_stations; i++)
		if (Stations[i].url == url)
			return i;
	return -1;
}

int GetStationIndexByFreq(uint freq)
{
	for (uint i = 0; i < n_stations; i++)
		if (Stations[i].freq == freq)
			return i;
	return -1;
}


bool FindStationByUrl(String url, RADIO_STATION &station)
{
	int i = GetStationIndexByUrl(url);
	if (i == -1) return false;
	station = Stations[i];
	return true;
}

bool FindStationByFreq(uint freq, RADIO_STATION &station)
{
	int i = GetStationIndexByFreq(freq);
	if (i == -1) return false;
	station = Stations[i];
	return true;
}

void LoadRadioState()
{
	File file = SPIFFS.open(STATE_FILE_NAME);
	RADIO_STATION st;
	if (file)
	{
		Serial.println("Loading last state");
		if (file.available())
		{
			// read current station
			st.freq = file.readStringUntil(',').toInt();
			st.url = file.readStringUntil(',');
			if (st.freq > 0) 
			{
				FMStation = st;
				CurrentRadio = FM_RADIO;
			}
			else 
			{
				WebStation = st;
				CurrentRadio = WEB_RADIO;
			}
			// read volumes
			FMVolume = file.readStringUntil(',').toInt();
			WebVolume = file.readStringUntil('\n').toInt();
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
		if (CurrentRadio == WEB_RADIO)
		{
			file.print("0,");
			file.print(WebStation.url);
		}
		else 
		{
			file.print(FMStation.freq);
			file.print(",");
		}
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