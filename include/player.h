#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "main.hpp"

#include <Audio.h>
#include <HardwareSerial.h>

static const char DefaultWebStationName[] = "WEB Station";

HardwareSerial uart(2); // use UART2

Audio audio;

//#define SEPARATE_TASK

// Audio as separate task...
#ifdef SEPARATE_TASK

struct audioMessage
{
	uint8_t cmd;
	const char *txt;
	uint32_t value;
	uint32_t ret;
} audioTxMessage, audioRxMessage;

enum : uint8_t
{
	MSG_SET_VOLUME,
	MSG_GET_VOLUME,
	MSG_CONNECTTOHOST,
	MSG_STOPSONG
};

QueueHandle_t audioSetQueue = NULL;
QueueHandle_t audioGetQueue = NULL;

void CreateQueues()
{
	audioSetQueue = xQueueCreate(10, sizeof(struct audioMessage));
	audioGetQueue = xQueueCreate(10, sizeof(struct audioMessage));
}

void audioTask(void *parameter)
{
	Serial.println("Start Aidio Task...");

	CreateQueues();
	if (!audioSetQueue || !audioGetQueue)
	{
		log_e("Queues are not initialized!!!");
		while (true); // endless loop
	}

	struct audioMessage audioRxTaskMessage;
	struct audioMessage audioTxTaskMessage;

	audio.setPinout(I2S_BCLK_PIN, I2S_LRC_PIN, I2S_DOUT_PIN);
	audio.setVolume(WebVolume % (MAX_WEB_VOLUME + 1));

	while (true)
	{
		if (xQueueReceive(audioSetQueue, &audioRxTaskMessage, 1) == pdPASS)
		{
			if (audioRxTaskMessage.cmd == MSG_SET_VOLUME)
			{
				audioTxTaskMessage.cmd = MSG_SET_VOLUME;
				audio.setVolume(audioRxTaskMessage.value);
				audioTxTaskMessage.ret = 1;
				xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
			}
			else if (audioRxTaskMessage.cmd == MSG_CONNECTTOHOST)
			{
				audioTxTaskMessage.cmd = MSG_CONNECTTOHOST;
				audioTxTaskMessage.ret = audio.connecttohost(audioRxTaskMessage.txt);
				xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
			}
			else if (audioRxTaskMessage.cmd == MSG_GET_VOLUME)
			{
				audioTxTaskMessage.cmd = MSG_GET_VOLUME;
				audioTxTaskMessage.ret = audio.getVolume();
				xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
			}
			else if (audioRxTaskMessage.cmd == MSG_STOPSONG)
			{
				audioTxTaskMessage.cmd = MSG_STOPSONG;
				audio.stopSong();
				audioTxTaskMessage.ret = 1;
				xQueueSend(audioGetQueue, &audioTxTaskMessage, portMAX_DELAY);
			}
			else
			{
				log_i("Error");
			}
		}
		audio.loop();
	}
}

audioMessage transmitReceive(audioMessage msg)
{
	xQueueSend(audioSetQueue, &msg, portMAX_DELAY);
	if (xQueueReceive(audioGetQueue, &audioRxMessage, portMAX_DELAY) == pdPASS)
	{
		if (msg.cmd != audioRxMessage.cmd)
		{
			log_e("Wrong reply from message queue");
		}
	}
	return audioRxMessage;
}

#endif // Audio as separate task

void audioInit()
{
#ifdef SEPARATE_TASK
	xTaskCreatePinnedToCore(
		audioTask,			   /* Function to implement the task */
		"audioplay",		   /* Name of the task */
		5000,				   /* Stack size in words */
		NULL,				   /* Task input parameter */
		2 | portPRIVILEGE_BIT, /* Priority of the task */
		NULL,				   /* Task handle. */
		1					   /* Core where the task should run */
	);
#else 
	audio.setPinout(I2S_BCLK_PIN, I2S_LRC_PIN, I2S_DOUT_PIN);
#endif
}

void audioSetVolume(uint8_t vol)
{
#ifdef SEPARATE_TASK
	audioTxMessage.cmd = MSG_SET_VOLUME;
	audioTxMessage.value = vol;
	audioMessage RX = transmitReceive(audioTxMessage);
#else
	audio.setVolume(vol);
#endif
}

uint8_t audioGetVolume()
{
#ifdef SEPARATE_TASK
	audioTxMessage.cmd = MSG_GET_VOLUME;
	audioMessage RX = transmitReceive(audioTxMessage);
	return RX.ret;
#else
	return audio.getVolume();
#endif
}

bool audioConnecttohost(const char *host)
{
#ifdef SEPARATE_TASK
	audioTxMessage.cmd = MSG_CONNECTTOHOST;
	audioTxMessage.txt = host;
	audioMessage RX = transmitReceive(audioTxMessage);
	return RX.ret;
#else
	return audio.connecttohost(host);
#endif
}

void audioStopSong()
{
#ifdef SEPARATE_TASK
	audioTxMessage.cmd = MSG_STOPSONG;
	audioMessage RX = transmitReceive(audioTxMessage);
#else
	audio.stopSong();
#endif
}

///////////////////////////////////////////////////////////////////////////////

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
	SwitchOutput(WEB_RADIO);
	WebStation.url = url;
	WebStation.name = name;
	DisplayCurrentMode(DM_NORMAL, true);
	if (WiFi.isConnected() && (WiFi.getMode() == WIFI_STA))
	{
		Serial.printf("Tune to URL: '%s'\n", url.c_str());
		// try n times...
		for (size_t i = 0; i < 5; i++)
		{
			if (audioConnecttohost(url.c_str()))
			{
				// WebStation.url = url;
				// WebStation.name = name;
				FindStationByUrl(url, WebStation);
				DisplayCurrentMode(DM_NORMAL);
				SetStateChanged();
				return;
			} 
			delay(100);
		}
	}

}

void TuneFMStation(uint freq, String name, bool fout = true)
{
	if (freq < MIN_FREQ) freq = MIN_FREQ;
	if (freq > MAX_FREQ) freq = MAX_FREQ;
	//if (freq >= MIN_FREQ && freq <= MAX_FREQ) 
	{
		if (name.length() == 0) name = "FM "+String(((float)freq)/10);
		audioStopSong();
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

void PlayerInit()
{
	audioInit();
	SetWebVolume(WebVolume);
	SetFMVolume(FMVolume);
	if (CurrentRadio == WEB_RADIO) PlayWebStation(WebStation.url, WebStation.name);
	else TuneFMStation(FMStation.freq, FMStation.name);
	StateChanged = false;
}

void SetWebVolume(uint8_t vol)
{
	if (vol <= MAX_WEB_VOLUME)
	{
		audioSetVolume(vol);
		WebVolume = audioGetVolume();
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
		lastKeyTime = millis();
		if (async_url.length() > 0) PlayWebStation(async_url, DefaultWebStationName);
		else if (async_freq > 0) TuneFMStation(async_freq, "FM " + String(((float)async_freq)/10));
		else if (async_fmvol >= 0) SetFMVolume(async_fmvol);
		else if (async_webvol >= 0) SetWebVolume(async_webvol);
		async_clear();
	}

#ifndef SEPARATE_TASK
	//if (CurrentRadio == WEB_RADIO)
	{
		audio.loop();
	}
#endif
}

/////////////////////////////////////////////////////////////////
// Audio Event Handlers

//void audio_info(const char *info) {}

void audio_showstation(const char *info)
{
	Serial.print("Station: ");
	Serial.println(info);
	if (WebStation.name.length() == 0 || WebStation.name.equals(DefaultWebStationName))
	{
		WebStation.name = info;
		DisplayCurrentMode(DisplayMode);
	}
}
void audio_showstreamtitle(const char *info)
{
	Serial.print("Title: ");
	Serial.println(info);
	WebStation.title = info;
	if (DisplayMode == DM_SIMPLE)
	{
		DisplayCurrentMode(DisplayMode);
	}
}

#endif //__PLAYER_H__