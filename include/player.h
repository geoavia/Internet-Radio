#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "main.hpp"

#include <HardwareSerial.h>

#define I2S_DOUT      25 // Preset I2S pins on the ESP32
#define I2S_BCLK      27
#define I2S_LRC       26


HardwareSerial uart(2); // use UART2

void PlayerInit()
{

	audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(PlayerVolume); // 0...21
}

void PlayerJob()
{
if (asyncVolume != PlayerVolume)
	{
		PlayerVolume = asyncVolume;
		audio.setVolume(PlayerVolume);
		SetStateChanged();
	}
}

void UartInit()
{
	Serial.println("UART Init...");
	uart.begin(38400, SERIAL_8N1, 15, 2);
}

void UartCommand(const char *cmd, int param = -1)
{
	String s(cmd);

	if (param >= 0) s+=param;
	Serial.print("Command: ");
	Serial.println(s);

	uart.write(s.c_str());
}

void FMTune(int freq)
{
	UartCommand("AT+FRE=", freq);
}

#endif //__PLAYER_H__