#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "main.hpp"

void PlayerInit()
{

	// Wait for VS1053 and PAM8403 to power up
	// otherwise the system might not start up correctly
	//delay(3000);

	SPI.begin();

	player.begin();
	player.loadDefaultVs1053Patches();
	player.switchToMp3Mode();
	player.setVolume(PlayerVolume);
}

void PlayerJob()
{
if (asyncVolume != PlayerVolume)
	{
		PlayerVolume = asyncVolume;
		player.setVolume(PlayerVolume);
		SetStateChanged();
	}
}

volatile bool canPlayMusicFromBuffer = false;
TaskHandle_t playMusicTaskHandle;

bool playMusicFromRingBuffer()
{
	bool dataPanic = false;
	if (circBuffer.available() >= MP3_BUFFER_SIZE)
	{
		//if (player.data_request())
		int bytesRead = circBuffer.read((char *)mp3buff, MP3_BUFFER_SIZE);
		if (bytesRead != MP3_BUFFER_SIZE)
		{
			log_w("Only read %db from ring buff", bytesRead);
			dataPanic = true;
		}
		player.playChunk(mp3buff, bytesRead);
	}
	return !dataPanic;
}

void checkBufferForPlaying()
{
	// to allow playing to start without stuttering
	if (circBuffer.available() > CIRC_BUFFER_SIZE / 4)
	{
		// Reset the flag, allowing data to be played, won't get reset again until station change
		canPlayMusicFromBuffer = true;
	}
}

void playMusicTask(void *parameter)
{
	static unsigned long prevMillis = 0;
	while (true)
	{
		if (canPlayMusicFromBuffer)
		{
			if (!playMusicFromRingBuffer())
			{
				canPlayMusicFromBuffer = false;
			};
			// We've queued up more music, no more to do this time slice
			taskYIELD();
		}
		else
		{
			// Otherwise, check whether we now have enough data to start playing
			checkBufferForPlaying();
		}

		// We should check that the stack size allocated was correct. This shows the FREE
		// stack space every X minutes. Assuming we have run all paths it should remain constant.
		// Comment out this code once satisfied that we have allocated the correct stack space.
		if (millis() - prevMillis > (60000 * 5))
		{
// Compiler warning about unused variable if we are not logging this value
#if CORE_DEBUG_LEVEL > ARDUHAL_LOG_LEVEL_DEBUG
			unsigned long remainingStack = uxTaskGetStackHighWaterMark(NULL);
			log_v("Free stack:%lu", remainingStack);
#endif
			prevMillis = millis();
		}
	}
}

void StartPlayerTask()
{
	// Independent Task to play music
	xTaskCreatePinnedToCore(
		playMusicTask,		  /* Function to implement the task */
		"iRadio",			  /* Name of the task */
		1800,				  /* Stack size in words */
		NULL,				  /* Task input parameter */
		1,					  /* Priority of the task - must be higher than 0 (idle)*/
		&playMusicTaskHandle, /* Task handle. */
		1);					  /* Core where the task should run */
}

#endif //__PLAYER_H__