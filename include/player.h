#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "main.hpp"

#include <HardwareSerial.h>

HardwareSerial uart(2); // use UART2

void SetWebVolume(uint8_t vol);

void PlayerInit()
{
	audio.setPinout(I2S_BCLK_PIN, I2S_LRC_PIN, I2S_DOUT_PIN);
    SetWebVolume(WebVolume);
	PlayWebStation(WebStation.url, WebStation.name);
	StateChanged = false;
}

void FMInit()
{
	Serial.println("UART Init...");
	uart.begin(38400, SERIAL_8N1, FM_RX_PIN, FM_TX_PIN);
}

void FMCommand(const char *cmd)
{
	Serial.print("Command: ");
	Serial.println(cmd);
	uart.write(cmd);
}

void SwitchOutput(RADIO_TYPE op)
{
	digitalWrite(WEB_RELAY_PIN, LOW);
	digitalWrite(FM_RELAY_PIN, LOW);
	if (op == FM_RADIO)
	{
		digitalWrite(FM_RELAY_PIN, HIGH);
		delay(50);
		digitalWrite(FM_RELAY_PIN, LOW);
	}
	else
	{
		digitalWrite(WEB_RELAY_PIN, HIGH);
		delay(50);
		digitalWrite(WEB_RELAY_PIN, LOW);
	}
	CurrentRadio = op;
}

void PlayWebStation(String url, String name)
{
	if (WiFi.isConnected() && (WiFi.getMode() == WIFI_STA))
	{
		audio.stopSong();
		Serial.printf("Tune to URL: '%s'\n", url.c_str());
		if (audio.connecttohost(url.c_str()))
		{
			SwitchOutput(WEB_RADIO);
			WebStation.url = url;
			WebStation.name = name;
			FindStationByUrl(url, WebStation);
			DisplayCurrentMode(DM_NORMAL);
			SetStateChanged();
		}
	}

}

void TuneFMStation(uint freq, String name, bool fout = true)
{
	if (freq < MIN_FREQ) freq = MIN_FREQ;
	if (freq > MAX_FREQ) freq = MAX_FREQ;
	//if (freq >= MIN_FREQ && freq <= MAX_FREQ) 
	{
		//audio.stopSong();
		char cmd[16];
		Serial.printf("Tune to FM: %d\n", freq);
		sprintf(cmd, "AT+FRE=%d", freq);
		FMCommand(cmd);
		if (fout || CurrentRadio != FM_RADIO) SwitchOutput(FM_RADIO);
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
	if (n < n_stations)
	{
		if (Stations[n].freq > 0)
		{
			TuneFMStation(Stations[n].freq, Stations[n].name);
		}
		else
		{
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

	if (CurrentRadio == WEB_RADIO) audio.loop();
}

#endif //__PLAYER_H__