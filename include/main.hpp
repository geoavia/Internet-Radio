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

//#include <driver/adc.h>
#include "esp_adc_cal.h"


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

////////////////////////////////
// Pinout
//
// Audio output relay
//
#define FM_RELAY_PIN		21
#define WEB_RELAY_PIN		22
//
#define IR_RECEIVE_PIN		17
//
// FM board
#define FM_TX_PIN 			2
#define FM_RX_PIN			15
//
// Peripherials power switch
#define PWR_PIN 			12
//
// Buttons
#define BUTTON_PIN_UP		35
#define BUTTON_PIN_DOWN		0
#define BUTTON_PIN_LEFT		39
#define BUTTON_PIN_RIGHT	32
#define BUTTON_PIN_OK		33
//
// External DAC board
#define I2S_DOUT_PIN		25
#define I2S_LRC_PIN			26
#define I2S_BCLK_PIN		27


#define WAKE_PIN_BITMASK (0x8300000000) // 2^GPIO

#define ADC_VBAT 34
#define ADC_EN 14

// State auto save interval
#define AUTOSAVE_INTERVAL_MS 9000


float VBAT = 0; // battery voltage from ESP32 ADC read

void initVbat()
{
	esp_adc_cal_characteristics_t adc_chars;
	esp_adc_cal_value_t val_type = esp_adc_cal_characterize((adc_unit_t)ADC_UNIT_1, (adc_atten_t)ADC_ATTEN_DB_2_5, (adc_bits_width_t)ADC_WIDTH_BIT_12, 1100, &adc_chars);
	pinMode(ADC_VBAT, INPUT)j;
	pinMode(ADC_EN, OUTPUT);
}

float getVbat()
{
	digitalWrite(ADC_EN, HIGH);
	delay(1);
	VBAT = ((float)analogRead(ADC_VBAT)) / 4095.0 * 7.26;
	digitalWrite(ADC_EN, LOW);
	return VBAT;

	// VBAT = ((float)analogRead(ADC_VBAT)) * 3600 / 4095 * 2;
	// return VBAT / 1000;
}

Audio audio;

// Radio Volume
uint WebVolume = 20;
uint FMVolume = 20;

// Station info
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

// Network credentials
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

void FMCommand(const char *cmd);
void PlayWebStation(String url, String name);
void TuneFMStation(uint freq, String name);
void SetWebVolume(uint8_t vol);
void SetFMVolume(uint vol);

#endif // __IRADIO_MAIN__