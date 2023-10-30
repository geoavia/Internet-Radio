#ifndef __REMOTE_H__
#define __REMOTE_H__

#define SEND_PWM_BY_TIMER // to disable warning
//#define RECORD_GAP_MICROS 12000
#define NO_LED_FEEDBACK_CODE
#define DECODE_NEC

#include <IRremote.hpp>

#define KEY_1 0xC
#define KEY_2 0x18
#define KEY_3 0x5E
#define KEY_4 0x8
#define KEY_5 0x1C
#define KEY_6 0x5A
#define KEY_7 0x42
#define KEY_8 0x52
#define KEY_9 0x4A
#define KEY_0 0x16
#define KEY_100 0x19
#define KEY_200 0xD
#define KEY_MINUS 0x7
#define KEY_PLUS 0x15
#define KEY_PREV 0x44
#define KEY_NEXT 0x40
#define KEY_EQ 0x9
#define KEY_PLAYPAUSE 0x43
#define KEY_CH_MINUS 0x45
#define KEY_CH_PLUS 0x47
#define KEY_CH 0x46

#define KEY_OK_TO_SLEEP_INTERVAL_MS 1000
#define KEY_REPEAT_INTERVAL_MS 400
#define KEY_DAMPER_INTERVAL_MS 50

uint16_t RemoteCode = 0;
bool IsRepeat = false;
unsigned long lastKeyTime = 0;
unsigned long lastPressTime = 0;
bool IsRemote = false;

void ButtonsInit()
{
	pinMode(BUTTON_PIN_UP, INPUT); // gpio 35 has internal pullup
	pinMode(BUTTON_PIN_DOWN, INPUT_PULLUP);
	pinMode(BUTTON_PIN_OK, INPUT);
	pinMode(BUTTON_PIN_LEFT, INPUT);
	pinMode(BUTTON_PIN_RIGHT, INPUT);

	// pulled up by the resistor
	// pinMode(IR_RECEIVE_PIN, INPUT_PULLUP);

}

bool ButtonsProcess()
{
	bool pressed = false;
	IsRepeat = false;

	if (digitalRead(BUTTON_PIN_UP) == LOW)
	{
		RemoteCode = KEY_CH_PLUS;
		pressed = true;
	}
	if (digitalRead(BUTTON_PIN_DOWN) == LOW)
	{
		RemoteCode = KEY_CH_MINUS;
		pressed = true;
	}
	if (digitalRead(BUTTON_PIN_OK) == HIGH)
	{
		RemoteCode = KEY_CH;
		pressed = true;
	}
	if (digitalRead(BUTTON_PIN_LEFT) == HIGH)
	{
		RemoteCode = KEY_MINUS;
		pressed = true;
	}
	if (digitalRead(BUTTON_PIN_RIGHT) == HIGH)
	{
		RemoteCode = KEY_PLUS;
		pressed = true;
	}

	if (pressed) 
	{
		if ((millis() - lastPressTime) < KEY_DAMPER_INTERVAL_MS) 
		{
			return false;
		}
		IsRepeat = ((millis() - lastPressTime) < KEY_REPEAT_INTERVAL_MS);
		lastPressTime = millis();
		if (!IsRepeat) 
		{
			lastKeyTime = millis();
		}
	}

	return pressed;
}

void RemoteInit()
{
	// pinMode(IR_RECEIVE_PIN, INPUT_PULLUP);

	IrReceiver.begin(IR_RECEIVE_PIN);
}

bool GetRemoteCode()
{
	if (ButtonsProcess())
	{
		IsRemote = false;
		return true;
	}

	if (IrReceiver.decode())
	{
		
		if (IrReceiver.decodedIRData.protocol == UNKNOWN)
		{
			//IrReceiver.printIRResultShort(&Serial);
			//IrReceiver.printIRResultRawFormatted(&Serial, true);
		}
		else IrReceiver.printIRResultShort(&Serial);
		IrReceiver.resume(); // Receive the next value

		RemoteCode = IrReceiver.decodedIRData.command;
		IsRepeat = (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT);
		//IsRepeat = ((millis() - lastKeyTime) <= KEY_REPEAT_INTERVAL_MS);
		lastKeyTime = millis();
		IsRemote = true;
		return true;
		
	}
	return false;
}

#endif //__REMOTE_H__