#ifndef __IRADIO_MAIN__
#define __IRADIO_MAIN__

#include <Arduino.h>

#include <Preferences.h>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

#include <SPIFFS.h>

#include <VS1053.h>

#include "cbuf.h"

#define VS1053_CS 25
#define VS1053_DCS 26
#define VS1053_DREQ 27

VS1053 player(VS1053_CS, VS1053_DCS, VS1053_DREQ);

const char *STATIONS_FILE_NAME = "/stations.csv";
const char *STATE_FILE_NAME = "/state.csv";
const char *OTAUP_FILE_NAME = "/otaup.up";

#define MAX_STATIONS 128

// Radio Volume
uint8_t PlayerVolume = 80;
uint8_t asyncVolume = 80;

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

WIFI_NETWORK otaup, curnet, networks[MAX_NETWORKS];

uint n_networks = 0;
uint n_SSID = 0;

Preferences preferences;

#ifdef BOARD_HAS_PSRAM
#define CIRC_BUFFER_SIZE 150000 // Divide by 32 to see how many 2mS samples this can store
#else
#define CIRC_BUFFER_SIZE 10000
#endif
cbuf circBuffer(CIRC_BUFFER_SIZE);


#define READ_BUFFER_SIZE  100
// Internet stream buffer that we copy in chunks to the ring buffer
char readBuffer[READ_BUFFER_SIZE] __attribute__((aligned(4)));

#define MP3_BUFFER_SIZE 32
uint8_t mp3buff[MP3_BUFFER_SIZE] __attribute__((aligned(4)));

String previousUrl = "";

unsigned long LastStateChange = 0;
bool StateChanged = false;

void SetStateChanged()
{
    LastStateChange = millis();
    StateChanged = true;
}

void StartPlayerTask();
bool NetworkConnectRadioUrl(String radio_url);

#endif // __IRADIO_MAIN__