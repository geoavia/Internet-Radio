#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "main.hpp"

#define I2S_DOUT      25
#define I2S_BCLK      27
#define I2S_LRC       26

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
		//player.setVolume(PlayerVolume);
		SetStateChanged();
	}
}


#endif //__PLAYER_H__