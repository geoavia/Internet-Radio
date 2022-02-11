#ifndef __DATA__
#define __DATA__

#include "SPIFFS.h"

#define MAX_STATIONS 128

// Radio Volume
uint8_t PlayerVolume = 80;

// saved network credentials
struct RADIO_STATION
{
    String url = "";
    String name = "";
};

RADIO_STATION CurrentStation, Stations[MAX_STATIONS];

uint n_stations = 0;

// http://wbgo.streamguys.net/thejazzstream - ok
// http://jenny.torontocast.com:8012/stream - sketchy
// http://157.90.197.157:8010/stream - ok
// http://91.121.155.204:8097/radio.mp3 - ok

void LoadRadioStations()
{
    n_stations = 0;
    File file = SPIFFS.open("/stations.txt");
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

void AddStation(String url, String name)
{
    if (n_stations < MAX_STATIONS)
    {
        Stations[n_stations].url = url;
        Stations[n_stations].name = name;
        n_stations++;
    }
}

void DataInit()
{
    if (!SPIFFS.begin(true))
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    LoadRadioStations();
}

void SaveRadioStations()
{
    File file = SPIFFS.open("/stations.txt", "w");
    if (file)
    {
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

void NextStation(bool next)
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

void SetStationNumber(uint n)
{
    if (n < n_stations)
        CurrentStation = Stations[n];
}

bool GetStationByUrl(String url, RADIO_STATION &station)
{
    for (uint i = 0; i < n_stations; i++)
        if (Stations[i].url == url)
        {
            station = Stations[i];
            return true;
        }
    return false;
}

#endif //__DATA__