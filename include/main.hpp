#ifndef __IRADIO_MAIN__
#define __IRADIO_MAIN__

#include <Arduino.h>

#include <Preferences.h>

#include <WiFi.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <Audio.h>

#include <SPIFFS.h>

#include "time.h"

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600*4; // Georgia
const int daylightOffset_sec = 0; // Georgia

const char *STATIONS_FILE_NAME = "/stations.csv";
const char *STATE_FILE_NAME = "/state.csv";

#define MAX_STATIONS 128

#define MAX_WEB_VOLUME 21
#define MAX_FM_VOLUME 30

#define MIN_FREQ	870
#define MAX_FREQ	1080

Audio audio;

// Radio Volume
uint WebVolume = 20;
uint FMVolume = 20;

// saved network credentials
struct RADIO_STATION
{
	uint freq = 0;
	String url = "";
	String name = "";
};

RADIO_STATION WebStation, FMStation, Stations[MAX_STATIONS];

enum RADIO_TYPE
{
	FM_RADIO,
	WEB_RADIO
};

uint n_stations = 0;

RADIO_TYPE CurrentRadio = WEB_RADIO;

// http://wbgo.streamguys.net/thejazzstream - ok
// http://jenny.torontocast.com:8012/stream - sketchy

// saved network credentials
struct WIFI_NETWORK
{
	String ssid = "";
	String password = "";
};

#define MAX_NETWORKS 10

WIFI_NETWORK curnet, networks[MAX_NETWORKS];

uint n_networks = 0;
uint n_SSID = 0;

Preferences preferences;

unsigned long LastStateChange = 0;
bool StateChanged = false;

void SetStateChanged()
{
    LastStateChange = millis();
    StateChanged = true;
}

struct PLAYER_ASYNC
{
	bool hot = false;
	int webvol = -1;
	int fmvol = -1;
	uint freq = 0;
	String url = "";
	String name = "";

	void clear()
	{
		hot = false;
		webvol = fmvol = -1;
		freq = 0;
		url = name = "";
	}

	void SetFreq(uint f, String n)
	{
		clear();
		freq = f;
		name = n;
		hot = true;
	}

	void SetUrl(String u, String n)
	{
		clear();
		url = u;
		name = n;
		hot = true;
	}

	void SetVol(uint v, RADIO_TYPE t)
	{
		clear();
		if (t == FM_RADIO) fmvol = v;
		else webvol = v;
		hot = true;
	}
};

PLAYER_ASYNC PlayerAsync;

void FMCommand(const char *cmd, ...);
void PlayWebStation(String url, String name);
void TuneFMStation(uint freq, String name);
void SetWebVolume(uint8_t vol);
void SetFMVolume(uint vol);

#endif // __IRADIO_MAIN__