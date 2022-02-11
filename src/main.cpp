#include <Arduino.h>

// all in one hpp files (for simplicity)
// preserve order!
#include "helper.hpp"
#include "remote.hpp"
#include "display.hpp"
#include "data.hpp"
#include "network.hpp"
#include "player.hpp"

void setup()
{
    Serial.begin(115200);

    // This can be set in the IDE no need for ext library
    // system_update_cpu_freq(160);

    Serial.println("\nInternet Radio, (c) GGM, 2022");

    RemoteInit();
    DisplayInit();
    DataInit();
    NetworkInit();
    PlayerInit();
}

void loop()
{
    if (GetRemoteCode())
    {
        if (RemoteCode == KEY_DOWN)
        {
            if (PlayerVolume > 0) PlayerVolume--;
            player.setVolume(PlayerVolume);
            DisplayVolume(PlayerVolume);
        }
        if (RemoteCode == KEY_UP)
        {
            if (PlayerVolume < 100) PlayerVolume++;
            player.setVolume(PlayerVolume);
            DisplayVolume(PlayerVolume);
        }
        if (RemoteCode == KEY_LEFT && !IsRepeat)
        {
            NextStation(false);
            PlayStation(CurrentStation);
        }
        if (RemoteCode == KEY_RIGHT && !IsRepeat)
        {
            NextStation(true);
            PlayStation(CurrentStation);
        }
        if (RemoteCode == KEY_1)
        {
            SetStationNumber(1);
            PlayStation(CurrentStation);
        }
        if (RemoteCode == KEY_2)
        {
            SetStationNumber(2);
            PlayStation(CurrentStation);
        }
        if (RemoteCode == KEY_3)
        {
            SetStationNumber(3);
            PlayStation(CurrentStation);
        }
        if (RemoteCode == KEY_4)
        {
            SetStationNumber(4);
            PlayStation(CurrentStation);
        }
        if (RemoteCode == KEY_5)
        {
            SetStationNumber(5);
            PlayStation(CurrentStation);
        }
        if (RemoteCode == KEY_6)
        {
            SetStationNumber(6);
            PlayStation(CurrentStation);
        }
        if (RemoteCode == KEY_7)
        {
            SetStationNumber(7);
            PlayStation(CurrentStation);
        }
        if (RemoteCode == KEY_8)
        {
            SetStationNumber(8);
            PlayStation(CurrentStation);
        }
        if (RemoteCode == KEY_9)
        {
            SetStationNumber(9);
            PlayStation(CurrentStation);
        }
        if (RemoteCode == KEY_0)
        {
            SetStationNumber(0);
            PlayStation(CurrentStation);
        }
    }

    NetworkJob();
    PlayerJob();
}