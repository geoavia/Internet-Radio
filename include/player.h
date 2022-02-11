#ifndef __PLAYER__
#define __PLAYER__

#include <VS1053.h>

#define VS1053_CS 25   //5
#define VS1053_DCS 26  //16
#define VS1053_DREQ 27 //4

VS1053 player(VS1053_CS, VS1053_DCS, VS1053_DREQ);

uint8_t mp3buff[32] __attribute__((aligned(4)));

void PlayStation(RADIO_STATION station)
{
    CurrentStation = station;
    if (NetworkConnectRadioUrl(CurrentStation.url))
    {
        DisplayHeader();
        display.setTextWrap(true);
        display.println("Now Playing:");
        display.println(CurrentStation.name); // todo get from icy-metadata
        display.display();
        display.setTextWrap(false);
        display.drawFastHLine(0,52,128,SSD1306_WHITE);
        display.setCursor(0, 56);
        display.printf("IP: %s\n", WiFi.localIP().toString().c_str());
        display.display();
    }
}

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

    PlayStation(CurrentStation);
}

void PlayerJob()
{
    if (WiFi.isConnected())
    {
        if (!client.connected())
        {
            // reconnecting
            NetworkConnectRadioUrl(CurrentStation.url);
        }

        if (client.available() > 0)
        {
            uint8_t bytesread = client.read(mp3buff, 32);
            player.playChunk(mp3buff, bytesread);
        }
    }
}

#endif //__PLAYER__