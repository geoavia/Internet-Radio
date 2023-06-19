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

#define MAX_VOLUME 21

Audio audio;

// Radio Volume
uint8_t PlayerVolume = 20;
uint8_t asyncVolume = 20;

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

String previousUrl = "";

unsigned long LastStateChange = 0;
bool StateChanged = false;

void SetStateChanged()
{
    LastStateChange = millis();
    StateChanged = true;
}

#endif // __IRADIO_MAIN__