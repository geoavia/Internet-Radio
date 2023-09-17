#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "main.hpp"

#include <HardwareSerial.h>

#define I2S_DOUT      25 // Preset I2S pins on the ESP32
#define I2S_BCLK      27
#define I2S_LRC       26

#define FM_RX	15
#define FM_TX	2


HardwareSerial uart(2); // use UART2

void SetWebVolume(uint8_t vol);

void PlayerInit()
{
	audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    SetWebVolume(WebVolume);
	PlayWebStation(WebStation.url, WebStation.name);
	StateChanged = false;
}

void FMInit()
{
	Serial.println("UART Init...");
	uart.begin(38400, SERIAL_8N1, FM_RX, FM_TX);
}

void FMCommand(const char *cmd)
{
	Serial.print("Command: ");
	uart.write(cmd);
}

void PlayWebStation(String url, String name)
{
	if (WiFi.isConnected() && (WiFi.getMode() == WIFI_STA))
	{
		audio.stopSong();
		Serial.printf("Tune to URL: '%s'\n", url.c_str());
		if (audio.connecttohost(url.c_str()))
		{
			WebStation.url = url;
			WebStation.name = name;
			FindStationByUrl(url, WebStation);
			DisplayCurrentMode(DM_NORMAL);
			SetStateChanged();
		}
	}

}

void TuneFMStation(uint freq, String name)
{
	if (freq >= MIN_FREQ && freq <= MAX_FREQ) 
	{
		char cmd[16];
		Serial.printf("Tune to FM: %d\n", freq);
		sprintf(cmd, "AT+FRE=%d", freq);
		FMCommand(cmd);
		FMStation.freq = freq;
		FMStation.name = name;
		FindStationByFreq(freq, FMStation);
		DisplayCurrentMode(DM_NORMAL);
		SetStateChanged();
	}
}

void SetWebVolume(uint8_t vol)
{
	if (vol <= MAX_WEB_VOLUME)
	{
		audio.setVolume(vol);
		WebVolume = audio.getVolume();
		Serial.printf("Web volume: %d\n", WebVolume);
		SetStateChanged();
	}
}

void SetFMVolume(uint vol)
{
	if (vol <= MAX_FM_VOLUME)
	{
		char cmd[16];
		sprintf(cmd, "AT+VOL=%02d", vol);
		FMCommand(cmd);
		FMVolume = vol;
		Serial.printf("FM volume: %d\n", FMVolume);
		SetStateChanged();
	}
}

void NextStation(int dir = 1)
{
	Serial.printf("%s station\n", (dir > 0)?"Next":"Previous");
	int ci = (CurrentRadio == FM_RADIO) ? 
		GetStationIndexByFreq(FMStation.freq) : 
		GetStationIndexByUrl(WebStation.url);
	int n = 0;
	dir = (dir > 0) ? 1 : -1;
	while (n < n_stations)
	{
		ci += dir;
		if (ci < 0) ci = (n_stations - 1);
		else if (ci >= n_stations) ci = 0;

		if (IsType(ci, CurrentRadio))
		{
			if (CurrentRadio == FM_RADIO) TuneFMStation(Stations[ci].freq, Stations[ci].name);
			else PlayWebStation(Stations[ci].url, Stations[ci].name);
			break;
		}
		n++;
	}
}

void SwitchStation(uint n)
{
	/// TODO...........
	if (n < n_stations)
	{
		if (Stations[n].freq > 0)
		{
			CurrentRadio = FM_RADIO;
			TuneFMStation(Stations[n].freq, Stations[n].name);
		}
		else
		{
			CurrentRadio = WEB_RADIO;
			PlayWebStation(Stations[n].url, Stations[n].name);
		}
	}
}

void PlayerJob()
{
 	if (async_hot)
	{
		if (async_url.length() > 0) PlayWebStation(async_url, "WEB Station");
		else if (async_freq > 0) TuneFMStation(async_freq, "FM " + String(((float)async_freq)/10));
		else if (async_fmvol >= 0) SetFMVolume(async_fmvol);
		else if (async_webvol >= 0) SetWebVolume(async_webvol);
		async_clear();
	}
}

#endif //__PLAYER_H__