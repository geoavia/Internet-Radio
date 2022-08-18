#ifndef __REMOTE_H__
#define __REMOTE_H__

#define SEND_PWM_BY_TIMER // to disable warning
#define RECORD_GAP_MICROS 12000
#define NO_LED_FEEDBACK_CODE

#include <IRremote.hpp>

#define IR_RECEIVE_PIN 35

#define BUTTON_PIN_OK 35
#define BUTTON_PIN_UP 32
#define BUTTON_PIN_DOWN 0
#define BUTTON_PIN_LEFT 13
#define BUTTON_PIN_RIGHT 14


#define REMOTE_TYPE_1

#ifdef REMOTE_TYPE_1

#define KEY_1 0x45
#define KEY_2 0x46
#define KEY_3 0x47
#define KEY_4 0x44
#define KEY_5 0x40
#define KEY_6 0x43
#define KEY_7 0x7
#define KEY_8 0x15
#define KEY_9 0x9
#define KEY_0 0x19
#define KEY_AST 0x16
#define KEY_HTAG 0xD
#define KEY_OK 0x1C
#define KEY_LEFT 0x8
#define KEY_RIGHT 0x5A
#define KEY_UP 0x18
#define KEY_DOWN 0x52

#else

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

#endif


#define KEY_OK_TO_SLEEP_INTERVAL_MS 3000
#define KEY_REPEAT_INTERVAL_MS 1000
#define KEY_DAMPER_INTERVAL_MS 300

uint16_t RemoteCode = 0;
bool IsRepeat = false;
unsigned long lastKeyTime = 0;
bool IsRemote = false;

bool oldBstate = false;

void ButtonsInit()
{
	pinMode(BUTTON_PIN_OK, INPUT_PULLUP);
	pinMode(BUTTON_PIN_UP, INPUT_PULLUP);
	pinMode(BUTTON_PIN_DOWN, INPUT_PULLUP);
	pinMode(BUTTON_PIN_LEFT, INPUT_PULLUP);
	pinMode(BUTTON_PIN_RIGHT, INPUT_PULLUP);
}

bool ButtonsProcess()
{
	bool bstate = false;

	if (digitalRead(BUTTON_PIN_OK) == LOW)
	{
		RemoteCode = KEY_OK;
		bstate = true;
	}
	
	if (digitalRead(BUTTON_PIN_UP) == LOW)
	{
		RemoteCode = KEY_UP;
		bstate = true;
	}
	if (digitalRead(BUTTON_PIN_DOWN) == LOW)
	{
		RemoteCode = KEY_DOWN;
		bstate = true;
	}
	if (digitalRead(BUTTON_PIN_LEFT) == LOW)
	{
		RemoteCode = KEY_LEFT;
		bstate = true;
	}
	if (digitalRead(BUTTON_PIN_RIGHT) == LOW)
	{
		RemoteCode = KEY_RIGHT;
		bstate = true;
	}

	if (bstate != oldBstate)
	{
		IsRepeat = ((millis() - lastKeyTime) < KEY_DAMPER_INTERVAL_MS);
		if (!IsRepeat)
		{
			oldBstate = bstate;
			//IsRepeat = false;
			if (bstate)
			{
				lastKeyTime = millis();
			}
		}
	}
	else if (bstate) 
	{
		IsRepeat = true;
	}

	return bstate;
}

void RemoteInit()
{
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

		// repeatable commands

		if (IrReceiver.decodedIRData.command == KEY_LEFT ||
			IrReceiver.decodedIRData.command == KEY_RIGHT ||
			IrReceiver.decodedIRData.command == KEY_UP ||
			IrReceiver.decodedIRData.command == KEY_DOWN)
		{
			RemoteCode = IrReceiver.decodedIRData.command;
			IsRepeat = ((millis() - lastKeyTime) <= KEY_REPEAT_INTERVAL_MS);
			lastKeyTime = millis();
			IsRemote = true;
			return true;
		}

		// non repeatable commands
		if (RemoteCode != IrReceiver.decodedIRData.command)
		{
			RemoteCode = IrReceiver.decodedIRData.command;
			IsRepeat = ((millis() - lastKeyTime) <= KEY_REPEAT_INTERVAL_MS);
			lastKeyTime = millis();
			IsRemote = true;
			return true;
		}
		
	}
	return false;
}

#endif //__REMOTE_H__