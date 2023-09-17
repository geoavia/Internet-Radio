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
}

void FMInit()
{
	Serial.println("UART Init...");
	uart.begin(38400, SERIAL_8N1, FM_RX, FM_TX);
}

void FMCommand(const char *cmd, ...)
{
	char scmd[16];
	va_list par;

	vsprintf(scmd, cmd, par);
	Serial.print("Command: ");
	Serial.println(scmd);

	uart.write(scmd);
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
		Serial.printf("Tune to FM: %d\n", freq);
		FMCommand("AT+FRE=%d", freq);
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
		FMCommand("AT+VOL=%02d", vol);
		FMVolume = vol;
		Serial.printf("FM volume: %d\n", FMVolume);
		SetStateChanged();
	}
}

void NextStation(int dir = 1)
{
	int ci = (CurrentRadio == FM_RADIO) ? 
		GetStationIndexByFreq(FMStation.freq) : 
		GetStationIndexByUrl(WebStation.url);
	int n = 0;
	dir = (dir > 0) ? 1 : -1;
	while (n < n_stations)
	{
		ci += dir;
		if (ci < 0) ci = (n_stations - 1);
		else if (ci >= (n_stations - 1)) ci = 0;

		if (IsType(ci, CurrentRadio))
		{
			//FStasyncFreq = Stations[ci].freq;
			//asyncUrl = Stations[ci].url;
			break;
		}
		n++;
	}
}

void SwitchStation(int num)
{

}

void PlayerJob()
{
	if (PlayerAsync.hot)
	{
		if (PlayerAsync.url.length() > 0) 
			PlayWebStation(PlayerAsync.url, PlayerAsync.name);
		else if (PlayerAsync.freq > 0)
			TuneFMStation(PlayerAsync.freq, PlayerAsync.name);
		else if (PlayerAsync.fmvol >= 0)
			SetFMVolume(PlayerAsync.fmvol);
		else if (PlayerAsync.webvol >= 0)
			SetFMVolume(PlayerAsync.webvol);
		PlayerAsync.clear();
	}

}

#endif //__PLAYER_H__