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

void PlayerInit()
{
	audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(WebVolume); // 0...21
}

void FMInit()
{
	Serial.println("UART Init...");
	uart.begin(38400, SERIAL_8N1, FM_RX, FM_TX);
}

void FMCommand(const char *cmd, int param = -1)
{
	String s(cmd);

	if (param >= 0) s+=param;
	Serial.print("Command: ");
	Serial.println(s);

	uart.write(s.c_str());
}

void FMTune(int freq)
{
	FMCommand("AT+FRE=", freq);
}

void PlayerJob()
{
if (asyncWebVolume != WebVolume)
	{
		WebVolume = asyncWebVolume;
		audio.setVolume(WebVolume);
		Serial.printf("Web volume: %d\n", WebVolume);
		SetStateChanged();
	}

if (asyncFMVolume != FMVolume)
	{
		FMVolume = asyncFMVolume;
		char vol[10];
		sprintf(vol, "AT+VOL=%02d", FMVolume);
		FMCommand(vol);
		SetStateChanged();
	}
}



#endif //__PLAYER_H__