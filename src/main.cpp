#include <Arduino.h>

#include "helper.h"
#include "remote.h"
#include "display.h"
#include "data.h"
#include "network.h"
#include "player.h"

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
            NetworkConnectRadioUrl(CurrentStation.url);
        }
        if (RemoteCode == KEY_RIGHT && !IsRepeat)
        {
            NextStation(true);
            NetworkConnectRadioUrl(CurrentStation.url);
        }
        if (RemoteCode == KEY_1)
        {
            SetStationNumber(1);
            NetworkConnectRadioUrl(CurrentStation.url);
        }
        if (RemoteCode == KEY_2)
        {
            SetStationNumber(2);
            NetworkConnectRadioUrl(CurrentStation.url);
        }
        if (RemoteCode == KEY_3)
        {
            SetStationNumber(3);
            NetworkConnectRadioUrl(CurrentStation.url);
        }
        if (RemoteCode == KEY_4)
        {
            SetStationNumber(4);
            NetworkConnectRadioUrl(CurrentStation.url);
        }
        if (RemoteCode == KEY_5)
        {
            SetStationNumber(5);
            NetworkConnectRadioUrl(CurrentStation.url);
        }
        if (RemoteCode == KEY_6)
        {
            SetStationNumber(6);
            NetworkConnectRadioUrl(CurrentStation.url);
        }
        if (RemoteCode == KEY_7)
        {
            SetStationNumber(7);
            NetworkConnectRadioUrl(CurrentStation.url);
        }
        if (RemoteCode == KEY_8)
        {
            SetStationNumber(8);
            NetworkConnectRadioUrl(CurrentStation.url);
        }
        if (RemoteCode == KEY_9)
        {
            SetStationNumber(9);
            NetworkConnectRadioUrl(CurrentStation.url);
        }
        if (RemoteCode == KEY_0)
        {
            SetStationNumber(0);
            NetworkConnectRadioUrl(CurrentStation.url);
        }
    }

    NetworkJob();
    PlayerJob();
}